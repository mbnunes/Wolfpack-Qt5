//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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

#undef  DBGFILE
#define DBGFILE "walking.cpp"

unitile_st xyblock[XYMAX];


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

	// TODO: Calculate the REAL average Z Value of that Map Tile here! Otherwise clients will have minor walking problems.
	mapBlock.z = Map->mapElevation( pos );

	map_st mapCell = Map->seekMap( pos );
	land_st mapTile = TileCache::instance()->getLand( mapCell.id );

	// If it's not impassable it's automatically walkable
	if( !(mapTile.flag1 & 0x40) )
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
	
	for( i = 0; i < blockList.size(); ++i )
	{
		stBlockItem item = blockList[i];
		INT8 itemTop = ( item.z + item.height );

		// If we encounter any object with itemTop <= pos.z which is NOT walkable
		// Then we can as well just return false as while falling we would be
		// blocked by that object
		if( !item.walkable && ( itemTop < pos.z ) )
			return false;

		// If the top of the item is within our max-climb reach
		// then the first check passed. in addition we need to 
		// check if the "bottom" of the item is reachable
		// I would say 2 is a good "reach" value for the bottom
		// of any item
		if( item.walkable && ( itemTop <= pos.z + P_M_MAX_Z_CLIMB ) && ( itemTop >= pos.z - P_M_MAX_Z_FALL ) /*&& ( item.z <= pos.z + 2 )*/ )
		{
			pos.z = itemTop;
			found = true;
			break;
		}
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
		if( ( pChar->pos().x == pItem->pos().x ) && ( pChar->pos().y == pItem->pos().y ) && ( pItem->pos().z >= pChar->pos().z ) && ( pItem->pos().z <= pChar->pos().z + 5 ) )
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
			if( ( oldDist >= player->visualRange() ) && ( newDist <= player->visualRange() ) )
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

	// This happens if we're moving
	if( dir == pChar->direction() )
	{
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

		checkStealth( pChar ); // Reveals the user if neccesary

		// Check if the char can move to those new coordinates
		// It is going to automatically calculate the new coords (!)
		if( ( !player || !player->isGM() ) && !mayWalk( pChar, newCoord ) )
		{
			if( player && player->socket() )
				player->socket()->denyMove( sequence );
			return;
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
		pChar->moveTo( newCoord );
		AllTerritories::instance()->check( pChar );

		handleItems( pChar, oldpos );
		HandleTeleporters( pChar, oldpos );
	}

	// do all of the following regardless of whether turning or moving i guess
	// set the player direction to contain only the cardinal direction bits
	pChar->setDirection(dir);
	
	if( player && player->socket() )
		player->socket()->allowMove( sequence );
	
	RegionIterator4Chars ri( pChar->pos() );
	for( ri.Begin(); !ri.atEnd(); ri++ ) {
		P_CHAR observer = ri.GetData();

		// If we are a player, send us new characters
		if (player && player->socket()) {
			if (observer->pos().distance(player->pos()) <= player->visualRange() && 
				observer->pos().distance(oldpos) > player->visualRange()) {
					player->socket()->sendChar(observer);
				}
		}

		// Send our movement to the observer
		P_PLAYER otherplayer = dynamic_cast<P_PLAYER>(observer);

		if (otherplayer && otherplayer->socket() && otherplayer->canSee(pChar)) {            
			if (otherplayer->pos().distance(oldpos) > otherplayer->visualRange()) {
				otherplayer->socket()->sendChar(pChar); // Previously we were out of range 
			} else {
				otherplayer->socket()->updateChar(pChar); // Previously we were already known
			}
		}
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
	switch ( pc->bodyID() )
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
bool cMovement::verifySequence( cUOSocket *socket, Q_UINT8 sequence ) throw()
{
	if( !sequence )
		socket->setWalkSequence( 0xFF );

	if ( ( socket->walkSequence() + 1 != sequence ) && ( sequence != 0xFF ) && ( socket->walkSequence() != 0xFF ) )
	{
		socket->denyMove( sequence );
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
		pChar->resend(false, true);

		if (update && socket) {
			cUOTxUpdatePlayer updatePlayer;
			updatePlayer.fromChar(pChar);
            socket->send(&updatePlayer);
		}
	}

	// Don't regenerate stamina while running
	pChar->setRegenStaminaTime( uiCurrentTime + ( SrvParams->staminarate() * MY_CLOCKS_PER_SEC ) );
	pChar->setRunningSteps( pChar->runningSteps() + 1 );
	
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

	if( pChar->isAtWar() && pChar->combatTarget() != INVALID_SERIAL )
		pChar->setNextHitTime(uiCurrentTime + ( MY_CLOCKS_PER_SEC * 2 ) ); // 2 Second timeout
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
				pChar->resend(false, true);

				if (pChar->objectType() == enPlayer) {
					P_PLAYER pp = dynamic_cast<P_PLAYER>(pChar);
				
					if (pp->socket()) {
						pp->socket()->sysMessage( tr( "You have been revealed." ) );

						cUOTxUpdatePlayer updatePlayer;
						updatePlayer.fromChar(pChar);
						pp->socket()->send(&updatePlayer);
					}
				}
			}
		}
		else
		{
			pChar->setHidden(false);
			pChar->resend(false, true);

			if (pChar->objectType() == enPlayer) {
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pChar);
			
				if (pp->socket()) {
					pp->socket()->sysMessage( tr( "You have been revealed." ) );

					cUOTxUpdatePlayer updatePlayer;
					updatePlayer.fromChar(pChar);
					pp->socket()->send(&updatePlayer);
				}
			}
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

// This is called whenever a char *HAS* already moved
// It will handle Collisions with items and sending them as well
void cMovement::handleItemCollision( P_CHAR pChar )
{
/*	// lets cache these vars in advance
	const int visibleRange = VISRANGE;
	const short int newx = pc->pos().x;
	const short int newy = pc->pos().y;
	const short int oldx = GetXfromDir(pc->dir + 4, newx);
	const short int oldy = GetYfromDir(pc->dir + 4, newy);
	
	// - Tauriel's region stuff 3/6/99
	const int StartGrid = cMapObjects::getInstance()->StartGrid(pc->pos);

	int checkgrid = 0;
	for (int increment = 0; increment < 3; increment++)
	{
		checkgrid = StartGrid + (increment * cMapObjects::getInstance()->GetColSize());
		for( int i = 0; i < 3; i++ )
		{
			P_ITEM mapitem = NULL;

			cRegion::raw vecEntries = cMapObjects::getInstance()->GetCellEntries(checkgrid+i, enItemsOnly);
			cRegion::rawIterator it = vecEntries.begin();
			for (; it != vecEntries.end(); ++it)
			{
				mapitem = FindItemBySerial(*it);
				if (mapitem != NULL)
				{
					if (!amTurning)
					{
						// is the item a building on the BUILDRANGE?
						if( ( mapitem->bodyID() == 0x407C ) || ( mapitem->bodyID() == 0x407D ) || ( mapitem->bodyID() == 0x407E ) )
						{
							if ((abs(newx-mapitem->pos().x)==BUILDRANGE)||(abs(newy-mapitem->pos().y)==BUILDRANGE))
							{
								senditem(socket, mapitem);
							}
						}
						else
						{
							signed int oldd = Distance(oldx, oldy, mapitem->pos().x, mapitem->pos().y);
							signed int newd = Distance(newx, newy, mapitem->pos().x, mapitem->pos().y);
							
							//if ((abs(newx-mapitem->pos().x) == visibleRange ) || (abs(newy-mapitem->pos().y) == visibleRange ))
							if (newd == visibleRange)
							{
								if( ( !mapitem->visible ) || ( ( mapitem->visible ) && ( currchar[socket]->isGM() ) ) )// we're a GM, or not hidden
									senditem(socket, mapitem);
							}
							if ( ( oldd == visibleRange ) && ( newd == ( visibleRange + 1 ) ) )
							{
								// item out of range for trigger
							}
						}
					}

					// Make our trigger check
					if( pc->onCollide( mapitem ) )
						continue;

					if( mapitem->onCollide( pc ) )
						continue;

					UI16 tileID = mapitem->bodyID();

					// split out the x,y,z check so we can use else ifs for faster item id checking
					if( ( tileID == 0x3996 ) || ( tileID == 0x398C ) )
					{//Fire Field
// Thyme 2000.09.15
// At the request of Abaddon
// Thyme BEGIN
//						if ((mapitem->pos().x == newx) && (mapitem->pos().y == newy) && (mapitem->pos().z==pc->pos().z))
						if ( ( mapitem->pos().x == newx ) &&
							 ( mapitem->pos().y == newy ) &&
							 ( pc->pos().z >= mapitem->pos().z ) &&
							 ( pc->pos().z <= ( mapitem->pos().z + 5 ) ) )
// Thyme END
						{
							if (!Magic->CheckResist(NULL, pc, 4))
							{
								Magic->MagicDamage(pc, mapitem->morex/300);
							}
							soundeffect2(pc, 0x0208);
						}
					}
					
					else if( ( tileID == 0x3915 ) || ( tileID == 0x3920 ) )
					{//Poison field
						if ((mapitem->pos().x == newx) && (mapitem->pos().y == newy) && (mapitem->pos().z==pc->pos().z))
						{
							if (!Magic->CheckResist(NULL, pc, 5))
							{
								Magic->PoisonDamage(pc, 1);
							}
							soundeffect2(pc, 0x0208);
						}
					}
					
					else if( ( tileID == 0x3979 ) || ( tileID == 0x3967 ) )
					{//Para Field
						if ((mapitem->pos().x == newx) && (mapitem->pos().y == newy) && (mapitem->pos().z==pc->pos().z))
						{
							if (!Magic->CheckResist(NULL, pc, 6))
							{
								tempeffect(pc, pc, 1, 0, 0, 0);
							}
							soundeffect2(pc, 0x0204);
						}
					}
					else if( !mapitem->isMulti() )
					{
						// look for item triggers, this was moved from CrazyXYBlockStuff()
						if ((mapitem->pos().x== newx) && (mapitem->pos().y == newy))
						{
							if (mapitem->trigger!=0)
							{
								if ((mapitem->trigtype==1)&&(!pc->dead))
								{
									if (!mapitem->disabled)
									{
										Trig->triggerwitem(socket, mapitem, 1);  //When player steps on a trigger
									}
									else // see if disabled trigger can be re-enabled
									{
										if( ( mapitem->disabled != 0 ) && ( ( mapitem->disabled <= uiCurrentTime ) || (overflow)))
										{
											mapitem->disabled = 0;	// re-enable it
											Trig->triggerwitem( socket, mapitem, 1 );
										}
									}
								}
							}
						}
					}

					// always check for new items if we actually moved
					
				}
			}
		}
	}*/
}

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
				pc->moveTo( destination );
				pc->resend();
			}
		}
	}
}

/********* start of LB's no rain & snow in buildings stuff ***********/
void cMovement::HandleWeatherChanges(P_CHAR pc, cUOSocket* socket)
{
/*	if (pc->isPlayer() && online(pc)) // check for being in buildings (for weather) only for PC's
	{
		// ok, this is already a bug, because the new weather stuff doesn't use this global
		// for the weather.
		if( wtype != 0 ) // check only neccasairy if it rains or snows ...
		{
			int inDungeon = indungeon(pc); // dung-check
			bool i = Map->IsUnderRoof(pc->pos); // static check

			// dynamics-check
			int x = Map->dynamicElevation(pc->pos);
			if (x!=ILLEGAL_Z)
				x=ILLEGAL_Z; // check for dynamic buildings
			if (x==1)
				x = ILLEGAL_Z; // 1 seems to be the multi-borders
			
			//printf("x: %i\n",x);
			// ah hah! this was a bug waiting to happen if not already, we have overloaded the use of the
			// variable k, which used to hold the socket
//			int k = noweather[socket];
			if (inDungeon || i || x!= ILLEGAL_Z )
				noweather[socket] = 1;
			else
				noweather[socket] = 0; // no rain & snow in static buildings+dungeons;
//			if (k - noweather[c] !=0)
//				weather(socket, 0); // if outside-inside changes resend weather ...
		}
	}
*/
}

void cMovement::CombatWalk(P_CHAR pc) // Only for switching to combat mode
{
	pc->update();
}

// type is npcwalk mode ( 0 for normal, 1 for box, 2 for circle )
void cMovement::randomNpcWalk( P_NPC pChar, Q_UINT8 dir, Q_UINT8 type )   
{
	// If we're not moving at all that could be -1
	if( !isValidDirection( dir ) )
		return;

	// We will precheck that here
	if( pChar->isFrozen() )
		return;

	Coord_cl newCoord = calcCoordFromDir( dir, pChar->pos() );

	// When the circle or box is not set yet reset the npcwalking setting
    if(	
		( type == 1 && ( ( pChar->wanderX1() == -1 ) || ( pChar->wanderX2() == -1 ) || ( pChar->wanderY1() == -1 ) || ( pChar->wanderY2() == -1 ) ) ) ||
		( type == 2 && pChar->wanderRadius() == -1 ) 
		)
	{
		pChar->setWanderType( enHalt );
		type = 0;
	}

	// If we should wander around in a circle and the middle of our circle is not yet set, let's reset it to our current position
	if( type == 2 && pChar->wanderX1() == -1 && pChar->wanderY1() == -1 )
	{
		pChar->setWanderX1( pChar->pos().x );
		pChar->setWanderY1( pChar->pos().y );
	}
    
	// If we either have to walk in a box or a circle we'll check if the new direction
	// is outside of our bounds
	if( ( type == 1 ) && !checkBoundingBox( newCoord, pChar->wanderX1(), pChar->wanderY1(), pChar->wanderX2(), pChar->wanderY2() ) )
		return;

	if( ( type == 2 ) && !checkBoundingCircle( newCoord, pChar->wanderX1(), pChar->wanderY1(), pChar->wanderX2() ) )
		return;

	Walking( pChar, dir&0x07, 0xFF );
}

// This processes a NPC movement poll
void cMovement::NpcMovement( unsigned int currenttime, P_NPC pc_i )
{
/*    int dnpctime=0;
    if( !pc_i || ( pc_i->nextMoveTime() > currenttime ) )
		return;

	// If we are fighting and not fleeing move toward our target if neccesary
	if( pc_i->isAtWar() )
    {
        P_CHAR pc_attacker = FindCharBySerial( pc_i->combatTarget() ); // This was wrong - we want to move towards our target not our attacker

        if( pc_attacker )
        {
			// Only move in the direction of the target
			// No special pathfinding
			if( pc_attacker->dist( pc_i ) > 1 && 
				( pc_attacker->objectType() == enPlayer && dynamic_cast<P_PLAYER>(pc_attacker)->socket() ) || 
				pc_attacker->objectType() == enNPC )
            {
				//PathFind( pc_i, pc_attacker->pos().x, pc_attacker->pos().y );
                //UINT8 dir = chardirxyz( pc_i, pc_i->path[pc_i->pathnum].x, pc_i->path[pc_i->pathnum].y );
                //if( ( pc_i->dir & 0x07 ) == ( dir & 0x07 ) )
				//	pc_i->pathnum++;
                
				// This is a temporary fix. 
				UINT8 dir = chardir( pc_i, pc_attacker );
				Coord_cl newCoord = calcCoordFromDir( dir, pc_i->pos() );
				if( mayWalk( pc_i, newCoord ) )
					Walking( pc_i, dir, 0xFF );
            }
			else
			{   
				// if I'm within distance, clear my path... for attacking only.
				// This happens too if our target is no longer valid
				// pc_i->pathnum += P_PF_MRV;
			}
	    }
		else
			pc_i->setCombatTarget( INVALID_SERIAL );
		return;
    }

	UINT8 j = RandomNum( 0, 32 );

	switch( pc_i->wanderType() )
    {
    case enFollowTarget: // Follow the follow target
		{
			P_CHAR pc_target = FindCharBySerial( pc_i->wanderFollowTarget() );

	        if( !pc_target )
				return;

		    if( ( pc_target->objectType() == enPlayer && dynamic_cast<P_PLAYER>(pc_target)->socket() ) || 
				pc_target->objectType() == enNPC )
			{
				if( pc_i->dist( pc_target ) <= SrvParams->pathfindFollowRadius() && pc_i->pathHeuristic( pc_i->pos(), pc_target->pos() ) > SrvParams->pathfindFollowMinCost() )
				{
					if( SrvParams->pathfind4Follow() )
					{
						Coord_cl nextmove = pc_i->nextMove();
						// check if we already have calculated a path
						// and if the destination still is within cost range.
						if( !pc_i->hasPath() ||
							pc_i->pathHeuristic( pc_i->pathDestination(), pc_target->pos() ) > SrvParams->pathfindFollowMinCost() ||
							!mayWalk( pc_i, nextmove ) )
						{
							pc_i->findPath( pc_target->pos(), SrvParams->pathfindFollowMinCost() );
							nextmove = pc_i->nextMove();
						}

						if( nextmove.x != 0xFFFF )
						{
							int dir = chardirxyz( pc_i, nextmove.x, nextmove.y );
							pc_i->setDirection( dir );

							if( pc_i->dist( pc_target ) > ( SrvParams->pathfindFollowRadius() / 2 ) )
								dir |= 0x80;

							pc_i->popMove();
							Walking( pc_i, dir, 0xFF );
						}
						else
						{
							pc_i->setWanderType( enHalt );
						}
					}
					else
					{
						Walking( pc_i, chardirxyz( pc_i, pc_target->pos().x, pc_target->pos().y ), 0xFF );
					}
				}
	        }
		}
        break;
    case enFreely: // Wander freely, avoiding obstacles.
        if( j < 8 || j > 32) 
			dnpctime = 5;
        if( j > 7 && j < 33) // Let's move in the same direction lots of the time.  Looks nicer.
            j = pc_i->direction();
        randomNpcWalk( pc_i, j, 0 );
        break;
    case enRectangle: // Wander freely, within a defined box
        if (j<8 || j>32) dnpctime=5;
        if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
            j=pc_i->direction();

        randomNpcWalk(pc_i,j,1);
        break;
    case enCircle: // Wander freely, within a defined circle
        if (j<8 || j>32) dnpctime=5;
        if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
            j=pc_i->direction();
        randomNpcWalk(pc_i,j,2);
        break;
/*    case 5: // Flee
		{
			Coord_cl nextmove = pc_i->nextMove();
			if( pc_i->hasPath() && mayWalk( pc_i, nextmove ) )
			{
				pc_i->popMove();
				Walking( pc_i, chardirxyz( pc_i, nextmove.x, nextmove.y ), 0xFF );
				break;
			}

			P_CHAR pc_k = FindCharBySerial( pc_i->combatTarget() );
			if( !pc_k ) 
				return;

			if ( pc_k->dist(pc_i) < SrvParams->pathfindFleeRadius() )
			{
				// calculate a x,y to flee towards
				int mydist = SrvParams->pathfindFleeRadius() - pc_k->dist( pc_i ) + 1;
				j = chardirxyz( pc_i, pc_k->pos().x, pc_k->pos().y );
				Coord_cl fleeCoord = calcCoordFromDir( j, pc_i->pos() );

				if ( fleeCoord != pc_i->pos() )
				{
					Q_INT8 xfactor = 0;
					Q_INT8 yfactor = 0;

					if( fleeCoord.x < pc_i->pos().x )
						xfactor = -1;
					else
						xfactor = 1;

					if( fleeCoord.y < pc_i->pos().y )
						xfactor = -1;
					else
						xfactor = 1;

					fleeCoord.x += ( xfactor * mydist );
					fleeCoord.y += ( yfactor * mydist );
				}
			
				fleeCoord.z = Map->mapAverageElevation( fleeCoord );
				pc_i->findPath( fleeCoord, 2 );
				Coord_cl nextmove = pc_i->nextMove();
				int dir = chardirxyz( pc_i, nextmove.x, nextmove.y );
				pc_i->setDirection( dir );
				pc_i->popMove();
				Walking( pc_i, dir, 0xFF );
			}
			else
			{ // wander freely... don't just stop because I'm out of range.
				j=rand()%40;
				if (j<8 || j>32) dnpctime=5;
				if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
					j=pc_i->direction();
				randomNpcWalk(pc_i,j,0);
			}
		}
		break;*/
	// Try to find your way to a specified position
/*	case enGoToPosition:
#pragma message("Implement pathfinding for this!")
		if( pc_i->pos().map != pc_i->wanderDestination().map )
		{
			pc_i->setWanderType( enHalt );
		}
		else
		{
			Coord_cl dest( pc_i->wanderDestination() );

			if( dest.x == 0 && dest.y == 0  && dest.z == 0 )
				pc_i->setWanderType( enHalt );
			else if( dest.distance( pc_i->pos() ) == 0 )
			{
				pc_i->setWanderType( enHalt );
			}
			else
			{
				UINT8 dir = chardirxyz( pc_i, dest.x, dest.y );
		        Walking( pc_i, dir, 0xFF );
			}
		}		
	}

	pc_i->setNextMoveTime();*/
}

// Function      : cMovement::CanCharWalk()
// Written by    : Dupois
// Revised by    : Thyme
// Revision Date : 2000.09.17
// Purpose       : Check if a character can walk to a specified x,y location
// Method        : Modified the old CheckWalkable function so that it can be utilized throughout
// the walking code. Ever wonder why NPCs can walk through walls and stuff in combat mode? This
// is the fix, plus more.
short int cMovement::Direction(short int sx, short int sy, short int dx, short int dy)
{
	int dir,xdif,ydif;
	
	xdif = dx - sx;
	ydif = dy - sy;
	
	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=-1;
	
	return dir;
}

bool cMovement::checkBoundingBox(const Coord_cl pos, int fx1, int fy1, int fx2, int fy2)
{
	if (pos.x>=((fx1<fx2)?fx1:fx2) && pos.x<=((fx1<fx2)?fx2:fx1))
		if (pos.y>=((fy1<fy2)?fy1:fy2) && pos.y<=((fy1<fy2)?fy2:fy1))
			if (pos.z == -1 || abs(pos.z - Map->height(pos))<=5)
				return true;
			return false;
}

bool cMovement::checkBoundingCircle(const Coord_cl pos, int fx1, int fy1, int radius)
{
	if ( (pos.x-fx1)*(pos.x-fx1) + (pos.y-fy1)*(pos.y-fy1) <= radius * radius)
		if (pos.z == -1 || abs(pos.z-Map->height(pos))<=5)
			return true;
		return false;
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
