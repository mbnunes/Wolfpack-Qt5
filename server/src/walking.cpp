//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "walking.h"
#include "sectors.h"
#include "srvparams.h"
#include "network.h"
#include "maps.h"
#include "tilecache.h"
#include "territories.h"
#include "network/uosocket.h"
#include "multiscache.h"
#include "tilecache.h"
#include "multis.h"
#include "basechar.h"
#include "npc.h"
#include "player.h"
#include "globals.h"
#include "inlines.h"
#include "world.h"

// Library Includes
#include <qvaluevector.h>

// System Includes
#include <algorithm>

using namespace std;

// DEFINES (Some of these should probably be moved to typedefs.h in the future)

// These are defines that I'll use. I have a history of working with properties, so that's why
// I'm using custom definitions here versus what may be defined in the other includes.
// NOTE: P = Property, P_C = Property for Characters, P_PF = Property for Pathfinding
//       P_M = Property for Movement

#define P_C_PRIV_GM			0x01	// GM Privilege Bit
#define P_C_PRIV_COUNSELOR	0x80	// Counselor Privilege Bit
#define P_C_PRIV2_FROZEN	0x02	// Character Frozen Bit
#define P_C_IS_GM_BODY		0x01	// Bits for different movement types
#define P_C_IS_PLAYER		0x02
#define P_C_IS_BIRD			0x20
#define P_C_IS_NPC			0x40
#define P_C_IS_FISH			0x80

// Ok, I played with this some, and in some places in T2A, the max height that I should be allowed
// to climb is actually around 14. So, I'm gonna use a different var up here, and if you
// don't agree with me, just give it the value MaxZstep [9] (defined in typedefs.h)

#define P_M_MAX_Z_CLIMB		14
#define P_M_MAX_Z_INFLUENCE	15
#define P_M_MAX_Z_FALL		20 // You can fall 20 tiles ofcourse !!
#define P_M_MAX_Z_BLOCKS	15

// These are the debugging defines

// These are the errors that we should want to see. RARELY results in any console spam unless
// someone is trying to use an exploit or is sending invalid data directly to the server.
#define DEBUG_WALK_ERROR	1

#define DEBUG_NPCWALK		0
#define DEBUG_WALK			0
#define DEBUG_PATHFIND		0

// Ok, as a part of the cleanup effort of the code, I'm seperating everything into sections...
// This is my attempt to rewriting the walking code. I'm going to take the code and documentation
// that others before me have used and incorporate my little (big?) fixes and comments.
// Many thanks to all of the previous contributors, and I hope these changes help out.
// fur            : 1999.10.27 - rewrite of walking.cpp with documentation!
//                : 1999.10.27 - ripped apart walking into smaller functions
// Tauriel        : 1999.03.06 - For all of the region stuff
// knoxos         : 2000.08.?? - For finally making use of the flags, and height blocking
// DarkStorm	  : 2002.06.19 - Cleaning up the mess

// To clear things up, we only need to care about 
// blocking items anyway. So we don't need 90% of the data
// previously gathered. What we need is a check if the character 
// Can walk the tile, the tile's height and if the tile's a stair
struct stBlockItem
{
	INT8 z;
	UINT8 height;
	bool walkable;

	stBlockItem(): walkable( false ), height( 0 ), z( -128 ) {}
};

// Keep in mind that this only get's called when 
// the tile we're walking on is impassable
bool checkWalkable( P_CHAR pChar, UINT16 tileId )
{
	return false;
}

struct compareTiles : public std::binary_function<stBlockItem, stBlockItem, bool>
{
	bool operator()(stBlockItem a, stBlockItem b)
	{
		return ( (a.height+a.z) > (b.height+b.z) );
	}
};

// The highest items will be @ the beginning
// While walking we always will try the highest first.
vector< stBlockItem > getBlockingItems( P_CHAR pChar, const Coord_cl &pos )
{
	vector< stBlockItem > blockList;
	make_heap( blockList.begin(), blockList.end(), compareTiles() );

	// Process the map at that position
	stBlockItem mapBlock;
	mapBlock.z = Map->mapAverageElevation( pos );
	mapBlock.height = 0;

	// TODO: Calculate the REAL average Z Value of that Map Tile here! Otherwise clients will have minor walking problems.
	map_st mapCell = Map->seekMap( pos );
	mapBlock.z = mapCell.z;
	land_st mapTile = TileCache::instance()->getLand( mapCell.id );

	// If it's not impassable it's automatically walkable
	if (!(mapTile.flag1 & 0x40))
		mapBlock.walkable = true;
	else
		mapBlock.walkable = checkWalkable( pChar, mapCell.id );

	if (mapCell.id != 0x02) {
		blockList.push_back( mapBlock );
		push_heap( blockList.begin(), blockList.end(), compareTiles() );
	}

    // Now for the static-items
	StaticsIterator staIter = Map->staticsIterator( pos, true );
	for( ; !staIter.atEnd(); ++staIter )
	{
		tile_st tTile = TileCache::instance()->getTile( staIter->itemid );

		// Here is decided if the tile is needed
		// It's uninteresting if it's NOT blocking
		// And NOT a bridge/surface
		if( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
			continue;

		stBlockItem staticBlock;
		staticBlock.z = staIter->zoff;

		// If we are a surface we can always walk here, otherwise check if
		// we are special
		if( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
			staticBlock.walkable = true;
		else
			staticBlock.walkable = checkWalkable( pChar, staIter->itemid );

		// If we are a stair only the half height counts
		if( tTile.flag2 & 0x04 )
			staticBlock.height = (UINT8)( tTile.height / 2 );
		else
			staticBlock.height = tTile.height;

		blockList.push_back( staticBlock );
		push_heap( blockList.begin(), blockList.end(), compareTiles() );
	}

	RegionIterator4Items iIter( pos );
	for( iIter.Begin(); !iIter.atEnd(); iIter++ )
	{
		P_ITEM pItem = iIter.GetData();

		if( !pItem )
			continue;

		if( pItem->id() >= 0x4000 )
		{
			MultiDefinition* def = MultiCache::instance()->getMulti( pItem->id() - 0x4000 );
			if ( !def )
				continue;
			QValueVector<multiItem_st> multi = def->getEntries();
			unsigned int j;
			for ( j = 0; j < multi.size(); ++j)
			{
				if ( multi[j].visible && ( pItem->pos().x + multi[j].x == pos.x) && ( pItem->pos().y + multi[j].y == pos.y ) )
				{
					tile_st tTile = TileCache::instance()->getTile( multi[j].tile );
					if( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
						continue;

					stBlockItem blockItem;
					blockItem.height = tTile.height;
					blockItem.z = pItem->pos().z + multi[j].z;

					if( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
						blockItem.walkable = true;
					else
						blockItem.walkable = checkWalkable( pChar, pItem->id() );

					blockList.push_back( blockItem );
					push_heap( blockList.begin(), blockList.end(), compareTiles() );				
				}
			}
			continue;
		} else if (pChar->isDead()) {
			// Doors can be passed by ghosts
			if (pItem->hasEvent("door")) {
				continue;
			}
		}

		// They need to be at the same x,y,plane coords
		if( ( pItem->pos().x != pos.x ) || ( pItem->pos().y != pos.y ) || ( pItem->pos().map != pos.map ) )
			continue;

		tile_st tTile = TileCache::instance()->getTile( pItem->id() );

		// Se above for what the flags mean
		if( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
			continue;

		stBlockItem blockItem;
		blockItem.height = tTile.height;
		blockItem.z = pItem->pos().z;

		// Once again: see above for a description of this part
		if( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
			blockItem.walkable = true;
		else
			blockItem.walkable = checkWalkable( pChar, pItem->id() );

		blockList.push_back( blockItem );
		push_heap( blockList.begin(), blockList.end(), compareTiles() );
	}

	// Now we need to evaluate dynamic items [...] (later)
	// TODO: Multis here
	sort_heap( blockList.begin(), blockList.end(), compareTiles() );

	return blockList;
};

// May a character walk here ?
// If yes we auto. set the new z value for pos
bool mayWalk( P_CHAR pChar, Coord_cl &pos )
{
	// Go trough the array top-to-bottom and check
	// If we find a tile to walk on
	vector< stBlockItem > blockList = getBlockingItems( pChar, pos );
	bool found = false;
	UINT32 i;
	bool priviledged = false;

	P_PLAYER player = dynamic_cast<P_PLAYER>(pChar);

	priviledged = player && player->isGM();
	
	for( i = 0; i < blockList.size(); ++i )
	{
		stBlockItem item = blockList[i];
		INT8 itemTop = ( item.z + item.height );

		// If we encounter any object with itemTop <= pos.z which is NOT walkable
		// Then we can as well just return false as while falling we would be
		// blocked by that object
		if( !item.walkable && !priviledged && itemTop < pos.z)
			return false;

		// If the top of the item is within our max-climb reach
		// then the first check passed. in addition we need to 
		// check if the "bottom" of the item is reachable
		// I would say 2 is a good "reach" value for the bottom
		// of any item
		if( (item.walkable || priviledged) && ( itemTop <= pos.z + P_M_MAX_Z_CLIMB ) && ( itemTop >= pos.z - P_M_MAX_Z_FALL ) /*&& ( item.z <= pos.z + 2 )*/ )
		{
			pos.z = itemTop;
			found = true;
			break;
		}
	}

	if (priviledged)
	{
		return true;
	}

	// If we're still at the same position
	// We didn't find anything to step on
	if( !found )
		return false;

	// Another loop *IS* needed here (at least that's what i think)
	for( i = 0; i < blockList.size(); ++i )
	{
		// So we know about the new Z position we are moving to
		// Lets check if there is enough space ABOVE that position (at least 15 z units)
		// If there is ANY impassable object between pos.z and pos.z + 15 we can't walk here
		stBlockItem item = blockList[i];
		INT8 itemTop = ( item.z + item.height );

		// Does the top of the item looms into our space
		// Like before 15 is the assumed height of ourself
		if( ( itemTop > pos.z ) && ( itemTop < pos.z + P_M_MAX_Z_BLOCKS ) )
			return false;

		// Or the bottom ?
		if( ( item.z > pos.z ) && ( item.z < pos.z + P_M_MAX_Z_BLOCKS ) )
			return false;

		// Or does it spread the whole range ?
		if( ( item.z <= pos.z ) && ( itemTop >= pos.z + P_M_MAX_Z_BLOCKS ) )
			return false;

		// If the Item Tops are already below our current position exit the
		// loop, we're sorted by those !
		if( itemTop <= pos.z )
			break;
	}

	// All Checks passed
	return true;
}

/*!
	This is called when a character
	collides with an item and this
	checks for special effects the item could have.
*/
bool handleItemCollision( P_CHAR pChar, P_ITEM pItem )
{
	Coord_cl dPos = pChar->pos();

	if( pItem->onCollide( pChar ) )
		return true;

	return false;
}

/*!
	Sends items which came in range and
	handles collisions with teleporters
	or damaging items.
*/
void handleItems( P_CHAR pChar, const Coord_cl &oldpos )
{
	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );
//	teleporters( pChar );

	RegionIterator4Items iter( pChar->pos() );
	for( iter.Begin(); !iter.atEnd(); iter++ )
	{
		// Check if the item got newly in range
		P_ITEM pItem = iter.GetData();

		if( !pItem )
			continue;

		// Check for item collisions here.
		if( ( pChar->pos().x == pItem->pos().x ) && ( pChar->pos().y == pItem->pos().y ) && ( pItem->pos().z <= pChar->pos().z ) && ( pItem->pos().z >= pChar->pos().z - 15 ) )
		{
			if( handleItemCollision( pChar, pItem ) )
				break;
		}
		
		// If we are a connected player then send new items
		if( player && player->socket() )
		{
			UI32 oldDist = oldpos.distance( pItem->pos() );
			UI32 newDist = pChar->pos().distance( pItem->pos() );

			// Was out of range before and now is in range
			if( ( oldDist >= player->visualRange() ) && ( newDist < player->visualRange() ) )
			{
				pItem->update( player->socket() );
			}
		}		
	}
}

/*!
	This handles if a character actually tries to walk (NPC & Player)
*/
void cMovement::Walking( P_CHAR pChar, Q_UINT8 dir, Q_UINT8 sequence )
{
	if( !pChar )
		return;

	// Scripting
	if( pChar->onWalk( dir, sequence ) )
		return;

/*	if( !isValidDirection( dir ) )
	{
		pChar->setPathNum( pChar->pathnum() + PATHNUM );
		return;
	}*/

	P_PLAYER player = dynamic_cast<P_PLAYER>(pChar);

	// Is the sequence in order ?
	if( player && player->socket() && !verifySequence( player->socket(), sequence ) )
		return;

	// If checking for weight is more expensive, shouldn't we check for frozen first?
	if( pChar->isFrozen() )
	{
		if( player && player->socket() )
			player->socket()->denyMove( sequence );
		return;
	}

	// save our original location before we even think about moving
	const Coord_cl oldpos( pChar->pos() );
	
	// If the Direction we're moving to is different from our current direction
	// We're turning and NOT moving into a specific direction
	// Clear the running flag here (!)
	// If the direction we're moving is already equal to our current direction
	bool running = dir & 0x80;
	dir = dir & 0x7F; // Remove the running flag

	bool turning = dir != pChar->direction();

	// This happens if we're moving
	if (!turning) {
		// Note: Do NOT use the copy constructor as it'll create a reference
		Coord_cl newCoord = calcCoordFromDir( dir, pChar->pos() );

		// Check if the stamina parameters
		if( player && !consumeStamina( player, running ) )
		{
			if( player->socket() )
				player->socket()->denyMove( sequence );
			return;
		}
	
		// Check for Characters in our way
		if( !checkObstacles( pChar, newCoord, running ) )
		{
			if( player && player->socket() )
				player->socket()->denyMove( sequence );
			return;
		}

		// Check if the char can move to those new coordinates
		// It is going to automatically calculate the new coords (!)
		if (!mayWalk(pChar, newCoord))
		{
			if( player && player->socket() )
				player->socket()->denyMove( sequence );
			return;
		} else {
			if (player && player->socket())
				player->socket()->allowMove(sequence);
		}
        
		// Check if we're going to collide with characters
		if( !player && CheckForCharacterAtXYZ( pChar, newCoord ) )
		{
			P_NPC npc = dynamic_cast<P_NPC>( pChar );
			if ( npc )
			{
				npc->clearPath();
			}
			return;
		}

		// Lets check if we entered or left a multi
		cMulti* pOldMulti = dynamic_cast< cMulti* >( FindItemBySerial( pChar->multis() ) );
		cMulti* pNewMulti = cMulti::findMulti( newCoord );
		if( pOldMulti != pNewMulti )
		{
			if( pOldMulti )
				pOldMulti->removeChar( pChar );

			if( pNewMulti )
				pNewMulti->addChar( pChar );
		}

		// We moved so let's update our location
		pChar->moveTo(newCoord);
		pChar->setLastMovement(uiCurrentTime);		
		checkStealth( pChar ); // Reveals the user if neccesary
	} else {
		if( player && player->socket() )
			player->socket()->allowMove( sequence );
	}

	// do all of the following regardless of whether turning or moving i guess
	// set the player direction to contain only the cardinal direction bits
	pChar->setDirection(dir);
	
	RegionIterator4Chars ri( pChar->pos() );
	for( ri.Begin(); !ri.atEnd(); ri++ ) {
		P_CHAR observer = ri.GetData();
		
		if (observer == pChar) {
			continue;
		}

		unsigned int distance = observer->pos().distance(oldpos);

		// If we are a player, send us new characters
		if (player && player->socket()) {
			if (distance > player->visualRange()) {
				player->socket()->sendChar(observer); // We were previously out of range.
			}
		}

		// Send our movement to the observer
		P_PLAYER otherplayer = dynamic_cast<P_PLAYER>(observer);

		if (otherplayer && otherplayer->socket()) {            
			if (distance > otherplayer->visualRange()) {
				otherplayer->socket()->sendChar(pChar); // Previously we were out of range 
			} else {
				otherplayer->socket()->updateChar(pChar); // Previously we were already known
			}
		}
	}

	// If we really moved handle teleporters and new items
	if (!turning) {
		handleItems( pChar, oldpos );
		HandleTeleporters( pChar, oldpos );
	}
}

short int cMovement::CheckMovementType(P_CHAR pc)
{
	if( pc->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pc);
		// Am I a GM Body?
		if( pp->isGMorCounselor() || pc->isDead() )
			return P_C_IS_GM_BODY;
		// Am I a player?
		else
			return P_C_IS_PLAYER;
	}
	// Change this to a flag in NPC definition

	short int retval = P_C_IS_NPC;
	switch ( pc->body() )
	{
	case 0x0010 : // Water Elemental
		retval = P_C_IS_FISH;
		break;
	case 0x005F : // Kraken
	case 0x0096 : // Dolphin
	case 0x0097 : // Sea Serpent
		retval = P_C_IS_FISH;
		break;
	default : // Regular NPC
		break;
	}
	return retval;
}

bool cMovement::CheckForCharacterAtXYZ(P_CHAR pc, const Coord_cl &pos )
{
	RegionIterator4Chars ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pc_i = ri.GetData();
		if( pc_i != NULL )
		{
			if( pc_i != pc && !pc_i->isHidden() && !pc_i->isInvisible() )
			{
				// x=x,y=y, and distance btw z's <= MAX STEP
				if ((pc_i->pos().x == pos.x) && (pc_i->pos().y == pos.y) && (abs(pc_i->pos().z-pos.z) <= P_M_MAX_Z_CLIMB))
				{
					return true;
				}
			}
		}
	}
	return false;
}

// GMs can go *EVERYWHERE*
bool cMovement::CanGMWalk( unitile_st xyb )
{
	return true;
}

bool cMovement::CanNPCWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;

	tile_st newTile = TileCache::instance()->getTile( blockid );

	// khpae : blocking tile !!
	if (xyb.flag1 & 0x40) {
		return false;
	}
	if ( newTile.isRoofOrFloorTile() )
		return true;
	
	if ( xyb.type == 0 )
		return true;

	if ( xyb.flag2 & 0x06 )
		return true;

	return false;
}

bool cMovement::CanPlayerWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;

	tile_st newTile = TileCache::instance()->getTile( blockid );

	if (xyb.flag1 & 0x40 || xyb.flag2 & 0x06 || xyb.type == 0 ) 
	{
		return false;
	}
	if ( newTile.isRoofOrFloorTile() )
		return true;

	return false;
}

bool cMovement::CanFishWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;
	
	if ( TileCache::instance()->getTile(blockid).isWet() )
		return true;

	// Can they walk/swim on water tiles?
	if ( blockid > 0x00A7 && blockid < 0x00AC )
		return true;
	if ( blockid > 0x1796 && blockid < 0x179D )
		return true;
	if ( blockid > 0x346D && blockid < 0x3486 )
		return true;
	if ( blockid > 0x3493 && blockid < 0x34AC )
		return true;
	if ( blockid > 0x34B7 && blockid < 0x34CB )
		return true;

	// Can they walk/swim on water ripples and splashes?
	if ( blockid > 0x34D0 && blockid < 0x34D6 )
		return true;
	if ( blockid > 0x352C && blockid < 0x3531 )
		return true;

	// Can they walk/swim on/up waterfalls?
	if ( blockid > 0x34EC && blockid < 0x3529 )
		return true;

	return false;
}

// needs testing... not totally accurate, but something to hold place.
bool cMovement::CanBirdWalk(unitile_st xyb)
{
	return ( CanNPCWalk(xyb) || CanFishWalk(xyb) );
}

// if we have a valid socket, see if we need to deny the movement request because of
// something to do with the walk sequence being out of sync.
bool cMovement::verifySequence( cUOSocket *socket, Q_UINT8 sequence ) throw() {
	if ((socket->walkSequence() == 0 && sequence != 0)) {
		return false;
	}

	// Simply ignore this packet. 
	// It's out of sync
	if (socket->walkSequence() != sequence) {
		socket->denyMove(sequence);
		return false;
	}

	return true;
}

// This only gets called when running
void cMovement::checkRunning( cUOSocket *socket, P_CHAR pChar, Q_UINT8 dir )
{
	signed short tempshort;
	
	// Running automatically stops stealthing
	if( pChar->stealthedSteps() != -1 ) 
	{
		// Simply update the statistics for us
        bool update = pChar->isHidden();

		pChar->setStealthedSteps(-1);
		pChar->setHidden(false);
		pChar->resend(false);
	}

	// Don't regenerate stamina while running
	pChar->setRegenStaminaTime(uiCurrentTime + floor(pChar->getStaminaRate() * 1000));
	pChar->setRunningSteps(pChar->runningSteps() + 1);
	
	// If we're running on our feet, check for stamina loss
	// Crap
	if( !pChar->isDead() && !pChar->atLayer( cBaseChar::Mount ) && pChar->runningSteps() > ( SrvParams->runningStamSteps() ) * 2 )
	{
		// The *2 it's because i noticed that a step(animation) correspond to 2 walking calls
		// ^^ WTF?
		pChar->setRunningSteps( 0 );
		pChar->setStamina( pChar->stamina() - 1 );
		socket->updateStamina();
	}
}

void cMovement::checkStealth( P_CHAR pChar )
{
	if( pChar->isHidden() && !pChar->isInvisible() )
	{
		if( pChar->stealthedSteps() != -1 )
		{
			pChar->setStealthedSteps( pChar->stealthedSteps() + 1 );
			if( pChar->stealthedSteps() > ( ( SrvParams->maxStealthSteps() * pChar->skillValue( STEALTH ) ) / 1000 ) )
			{
				pChar->setStealthedSteps(-1);
				pChar->setHidden(false);
				pChar->resend(false);
				pChar->sysmessage( tr( "You have been revealed." ) );
			}
		}
		else
		{
			pChar->setHidden(false);
			pChar->resend(false);
			pChar->sysmessage( tr( "You have been revealed." ) );
		}
	}
}

void cMovement::GetBlockingMap( const Coord_cl pos, unitile_st *xyblock, int &xycount)
{
	int mapid = 0;
	signed char mapz = Map->mapElevation(pos);  //Map->AverageMapElevation(x, y, mapid);
	if (mapz != ILLEGAL_Z)
	{
		land_st land = TileCache::instance()->getLand( mapid );
	
		xyblock[xycount].type=0;
		xyblock[xycount].basez = mapz;
		xyblock[xycount].id = mapid;
		xyblock[xycount].flag1=land.flag1;
		xyblock[xycount].flag2=land.flag2;
		xyblock[xycount].flag3=land.flag3;
		xyblock[xycount].flag4=land.flag4;
		xyblock[xycount].height=0;
		xyblock[xycount].weight=255;
		xycount++;
	}
}

void cMovement::GetBlockingStatics(const Coord_cl pos, unitile_st *xyblock, int &xycount)
{
	StaticsIterator msi = Map->staticsIterator(pos);
 	while (!msi.atEnd())
	{
		tile_st tile = TileCache::instance()->getTile( msi->itemid );
		xyblock[xycount].type	= 2;
		xyblock[xycount].basez	= msi->zoff;
		xyblock[xycount].id		= msi->itemid;
		xyblock[xycount].flag1	= tile.flag1;
		xyblock[xycount].flag2	= tile.flag2;
		xyblock[xycount].flag3	= tile.flag3;
		xyblock[xycount].flag4	= tile.flag4;
		xyblock[xycount].height	= tile.height;
		xyblock[xycount].weight = 255;
		++xycount;
		++msi;
	}
}

void cMovement::GetBlockingDynamics(const Coord_cl position, unitile_st *xyblock, int &xycount)
{
	RegionIterator4Items ri(position);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if( !mapitem->isMulti() )
			{
				if ((mapitem->pos().x == position.x) && (mapitem->pos().y == position.y))
				{
					tile_st tile = TileCache::instance()->getTile( mapitem->id() );
					xyblock[xycount].type=1;
					xyblock[xycount].basez=mapitem->pos().z;
					xyblock[xycount].id=mapitem->id();
					xyblock[xycount].flag1=tile.flag1;
					xyblock[xycount].flag2=tile.flag2;
					xyblock[xycount].flag3=tile.flag3;
					xyblock[xycount].flag4=tile.flag4;
					xyblock[xycount].height=tile.height;
					xyblock[xycount].weight=tile.weight;
					xycount++;
				}
			}
			else if (
				(abs(mapitem->pos().x - position.x)<=BUILDRANGE)&&
				(abs(mapitem->pos().y - position.y)<=BUILDRANGE)
				)
			{
				MultiDefinition* def = MultiCache::instance()->getMulti( mapitem->id() - 0x4000 );
				if ( !def )
					continue;
				QValueVector<multiItem_st> multi = def->getEntries();
				for (uint j = 0; j < multi.size(); ++j)
				{
					if (multi[j].visible && (mapitem->pos().x+multi[j].x == position.x) && (mapitem->pos().y+multi[j].y == position.y))
					{
						tile_st tile = TileCache::instance()->getTile( multi[j].tile );
						xyblock[xycount].type = 2;
						xyblock[xycount].basez = multi[j].z + mapitem->pos().z;
						xyblock[xycount].id = multi[j].tile;
						xyblock[xycount].flag1 = tile.flag1;
						xyblock[xycount].flag2 = tile.flag2;
						xyblock[xycount].flag3 = tile.flag3;
						xyblock[xycount].flag4 = tile.flag4;
						xyblock[xycount].height = tile.height;
						xyblock[xycount].weight = 255;
						++xycount;
					}
				}
			}
		}
	}
} //- end of itemcount for loop

void cMovement::HandleTeleporters(P_CHAR pc, const Coord_cl& oldpos)
{
	cTerritory* territory = pc->region();

	if( !territory )
		AllTerritories::instance()->check( pc );

	if( territory && pc->pos() != oldpos )
	{
		if ( territory->haveTeleporters() )
		{
			Coord_cl destination = pc->pos();
			if ( territory->findTeleporterSpot( destination ) )
			{
				bool quick = pc->pos().map != destination.map;
				pc->removeFromView(false);
				pc->moveTo(destination);
				pc->resend(false);
				P_PLAYER player = dynamic_cast<P_PLAYER>(pc);
				if (player && player->socket()) {
					player->socket()->resendPlayer(quick);
					player->socket()->resendWorld();
				}
			}
		}
	}
}

/*! 
	Checks if the direction we're moving to is valid.
*/
inline bool cMovement::isValidDirection( Q_UINT8 dir ) 
{
	return ( dir == ( dir & 0x87 ) );
}

/*!
	Calculates a new position out of the old position
	and the direction we're moving to.
*/
Coord_cl cMovement::calcCoordFromDir( Q_UINT8 dir, const Coord_cl& oldCoords )
{
	Coord_cl newCoords(oldCoords);

	// We're not switching the running flag
	switch( dir&0x07 )
	{
	case 0x00:
		newCoords.y--;                break;
	case 0x01:
		newCoords.y--; newCoords.x++; break;
	case 0x02:
		               newCoords.x++; break;
	case 0x03:
		newCoords.y++; newCoords.x++; break;
	case 0x04:
		newCoords.y++;                break;
	case 0x05:
		newCoords.y++; newCoords.x--; break;
	case 0x06:
		               newCoords.x--; break;
	case 0x07:
		newCoords.y--; newCoords.x--; break;
	};

	return newCoords;
}

/*!
  Calculates the amount of Stamina needed for a move of the
  passed character.
*/
bool cMovement::consumeStamina( P_PLAYER pChar, bool running )
{
	// TODO: Stamina loss is disabled for now -- Weight system needs to be rediscussed
//	return true;

	// Weight percent
	float allowedWeight = ( pChar->strength() * WEIGHT_PER_STR ) + 30;
	float load = ceilf( ( pChar->weight() / allowedWeight ) * 100) / 100;

	if( running )
		load = ceilf( load * 200 ) / 100;

	// 200% load is too much
	if( load >= 200 )
	{
		pChar->socket()->sysMessage( tr( "You are too overloaded to move." ) );
		return false;
	}

	// 20% overweight = ( 0.20 * 0.10 ) * (Weight carrying) = Stamina needed to move
	float overweight = load - 100;

	// We're not overloaded so we dont need additional stamina
	if( overweight < 0 )
		return true;

	float requiredStamina = ceilf( (float)((double)( (double)overweight * 0.10f ) * (double)pChar->weight()) * 100 ) / 100;
	
	if( pChar->stamina() < requiredStamina ) 
	{
		pChar->talk( tr( "You are too exhausted to move" ) );
		return false;
	}

	return true;
}

/*!
  This checks the new tile we're moving to
  for Character we could eventually bump into.
*/
bool cMovement::checkObstacles( P_CHAR pChar, const Coord_cl &newPos, bool running )
{
	// TODO: insert code here
	return true;
}

UINT16 DynTile( const Coord_cl &pos )
{
	RegionIterator4Items ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_ITEM mapitem = ri.GetData();
		if( mapitem )
		{
			if( mapitem->isMulti() )
			{
				MultiDefinition* def = MultiCache::instance()->getMulti( mapitem->id() - 0x4000 );
				if ( !def )
					return 0;
				QValueVector<multiItem_st> multi = def->getEntries();
				for( UINT32 j = 0; j < multi.size(); ++j )
				{
					if( ( multi[j].visible && ( mapitem->pos().x + multi[j].x == pos.x ) && ( mapitem->pos().y + multi[j].y == pos.y )
						&& ( abs( mapitem->pos().z + multi[j].z - pos.z ) <= 1 ) ) )
					{
						return multi[j].tile;
					}
				}
			}
			else if ( mapitem->pos() == pos )
				return mapitem->id();
        }    
		
    }
	return (UINT16)-1;
}

bool cMovement::canLandMonsterMoveHere( const Coord_cl& pos ) const
{
	if( pos.x >= ( Map->mapTileWidth(pos.map) * 8 ) || pos.y >= ( Map->mapTileHeight(pos.map) * 8 ) )
		return false;

    const signed char elev = Map->mapElevation( pos );
	Coord_cl target = pos;
	target.z = elev;
	if (ILLEGAL_Z == elev)
		return false;

    // get the tile id of any dynamic tiles at this spot
	Coord_cl mPos = pos;
	mPos.z = elev;
    const INT32 dt = DynTile( mPos );
	
    // if there is a dynamic tile at this spot, check to see if its a blocker
    // if it does block, might as well INT16-circuit and return right away
    if ( dt >= 0 )
	{
		tile_st tile = TileCache::instance()->getTile(dt);
		if ( tile.isBlocking() || tile.isWet() )
			return false;
	}
	
    // if there's a static block here in our way, return false
	StaticsIterator msi = Map->staticsIterator( pos );
	
	while( !msi.atEnd() )
	{
		tile_st tile = TileCache::instance()->getTile( msi->itemid );
		const INT32 elev = msi->zoff + cTileCache::tileHeight(tile);
		if( (elev >= pos.z) && (msi->zoff <= pos.z ) )
		{
			if (tile.isBlocking() || tile.isWet()) 
				return false;
		}
		msi++;
	}
    return true;
}
