//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "walking.h"
#include "wolfpack.h"
#include "debug.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "network.h"
#include "classes.h"
#include "maps.h"
#include "tilecache.h"
#include "territories.h"
#include "network/uosocket.h"
#include "multiscache.h"
#include "tilecache.h"
#include "weight.h"

// Library Includes
#include <qvaluevector.h>

// System Includes
#include <algorithm>

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

// Maximum Search Depth: Iterations to calculate
#define P_PF_MSD		5
// Maximum Return Value: Number of steps to return (Replaces PATHNUM)
// NOTE: P_PF_MRV CANNOT EXCEED THE VALUE OF PATHNUM FOR THE TIME BEING
#define P_PF_MRV		2
// Maximum Influence Range: Tiles to left/right of path to account for
#define P_PF_MIR		5
// Maximum Blocked Range: MIR to use if character is stuck
#define P_PF_MBR		10
// Minimum Flee Distance: MFD
#define P_PF_MFD		15

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
		mapBlock.walkable = checkWalkable( pChar, Map->seekMap( pos ).id );

	blockList.push_back( mapBlock );
	push_heap( blockList.begin(), blockList.end(), compareTiles() );

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
			MultiDefinition* def = MultisCache->getMulti( pItem->id() - 0x4000 );
			if ( !def )
				continue;
			QValueVector<multiItem_st> multi = def->getEntries();
			unsigned int j;
			for ( j = 0; j < multi.size(); ++j)
			{
				if ( multi[j].visible && ( pItem->pos.x + multi[j].x == pos.x) && ( pItem->pos.y + multi[j].y == pos.y ) )
				{
					tile_st tTile = TileCache::instance()->getTile( multi[j].tile );
					if( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
						continue;

					stBlockItem blockItem;
					blockItem.height = tTile.height;
					blockItem.z = pItem->pos.z + multi[j].z;

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
		if( ( pItem->pos.x != pos.x ) || ( pItem->pos.y != pos.y ) || ( pItem->pos.map != pos.map ) )
			continue;

		tile_st tTile = TileCache::instance()->getTile( pItem->id() );

		// Se above for what the flags mean
		if( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
			continue;

		stBlockItem blockItem;
		blockItem.height = tTile.height;
		blockItem.z = pItem->pos.z;

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
	Coord_cl dPos = pChar->pos;

	if( pItem->onCollide( pChar ) )
		return true;

	// Decide what to do on type first.
	switch( pItem->type() )
	{
	// Gate
	case 51:
	case 52:
		dPos.x = pItem->morex;
		dPos.y = pItem->morey;
		dPos.z = pItem->morez;

		// Soundeffect before teleport
		pChar->soundEffect( 0x1FE );

		// Teleport us
		pChar->removeFromView( false );
		pChar->moveTo( dPos );
		pChar->resend( false );

		// And soundeffect after
		pChar->soundEffect( 0x1FE );
		staticeffect( pChar, 0x37, 0x2A, 0x09, 0x06 );

		// Teleport pets
		RegionIterator4Chars iter( pChar->pos );
		for( iter.Begin(); !iter.atEnd(); iter++ )
		{
			P_CHAR pPet = iter.GetData();
			if( pPet->isNpc() && ( pPet->ftarg() == pChar->serial ) )
			{
				if( pPet->inRange( pItem, 4 ) )
				{
					pPet->removeFromView( false );
					pPet->moveTo( dPos );
					pPet->resend( false );
				}
			}
		}
		return true;
	};

	switch( pItem->id() )
	{
	// Fire Field
	case 0x3996:
	case 0x398c:
		if( !Magic->CheckResist( NULL, pChar, 4 ) )
			Magic->MagicDamage( pChar, pItem->morex/3000 );
		pChar->soundEffect( 0x208 );
		return false;

	//Poison field
	case 0x3915:
	case 0x3920:
		if( !Magic->CheckResist( NULL, pChar, 5 ) )
			Magic->PoisonDamage( pChar, 1 );
		pChar->soundEffect( 0x208 );
		return false;

	// Para field
	case 0x3979:
	case 0x3967:
		if( !Magic->CheckResist( NULL, pChar, 6 ) )
			tempeffect( pChar, pChar, 1, 0, 0, 0 );
		pChar->soundEffect( 0x204 );
		return false;
	};

	return false;
}

/*!
	Sends items which came in range and
	handles collisions with teleporters
	or damaging items.
*/
void handleItems( P_CHAR pChar, const Coord_cl &oldpos )
{
	cUOSocket *socket = pChar->socket();

//	teleporters( pChar );

	RegionIterator4Items iter( pChar->pos );
	for( iter.Begin(); !iter.atEnd(); iter++ )
	{
		// Check if the item got newly in range
		P_ITEM pItem = iter.GetData();

		if( !pItem )
			continue;

		// Check for item collisions here.
		if( ( pChar->pos.x == pItem->pos.x ) && ( pChar->pos.y == pItem->pos.y ) && ( pItem->pos.z > pChar->pos.z ) && ( pItem->pos.z <= pChar->pos.z + 5 ) )
		{
			if( handleItemCollision( pChar, pItem ) );
			break;
		}

		// If we are a connected player then send new items
		if( socket )
		{
			UI32 oldDist = oldpos.distance( pItem->pos );
			UI32 newDist = pChar->pos.distance( pItem->pos );

			// Was out of range before and now is in range
			if( ( oldDist >= pChar->VisRange() ) && ( newDist <= pChar->VisRange() ) )
			{
				pItem->update( socket );
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

	if( !isValidDirection( dir ) )
	{
		//pChar->pathnum += PATHNUM;
		pChar->setPathNum( pChar->pathnum() + PATHNUM );
		return;
	}

	cUOSocket *socket = pChar->socket();

	// Is the sequence in order ?
	if( socket && !verifySequence( socket, sequence ) )
		return;

	// If checking for weight is more expensive, shouldn't we check for frozen first?
	if( pChar->isFrozen() )
	{
		if( socket )
			socket->denyMove( sequence );
		return;
	}

	// save our original location before we even think about moving
	const Coord_cl oldpos( pChar->pos );
	
	// If the Direction we're moving to is different from our current direction
	// We're turning and NOT moving into a specific direction
	// Clear the running flag here (!)
	// If the direction we're moving is already equal to our current direction
	bool running = dir & 0x80;
	dir = dir & 0x7F; // Remove the running flag

	// This happens if we're moving
	if( dir == pChar->dir() )
	{
		// Note: Do NOT use the copy constructor as it'll create a reference
		Coord_cl newCoord = calcCoordFromDir( dir, pChar->pos );

		// Check if the stamina parameters
		if( !consumeStamina( socket, pChar, running ) )
		{
			socket->denyMove( sequence );
			return;
		}
	
		// Check for Characters in our way
		if( !checkObstacles( socket, pChar, newCoord, running ) )
		{
			socket->denyMove( sequence );
			return;
		}

		checkStealth( pChar ); // Reveals the user if neccesary

		// Check if the char can move to those new coordinates
		// It is going to automatically calculate the new coords (!)
		if( !mayWalk( pChar, newCoord ) )
		{
			if( socket )
				socket->denyMove( sequence );
			else if( pChar->isNpc() )
//				pChar->pathnum += P_PF_MRV;
				pChar->setPathNum( pChar->pathnum() + P_PF_MRV);

			return;
		}
        
		// Check if we're going to collide with characters
		if( pChar->isNpc() && CheckForCharacterAtXYZ( pChar, newCoord ) )
		{
//			pChar->pathnum += P_PF_MRV;
			pChar->setPathNum( pChar->pathnum() + P_PF_MRV);
			return;
		}

		// Lets check if we entered or left a multi
		cMulti* pOldMulti = dynamic_cast< cMulti* >( FindItemBySerial( pChar->multis ) );
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
		cAllTerritories::getInstance()->check( pChar );

		handleItems( pChar, oldpos );
		/*HandleTeleporters(pChar, socket, oldpos);
		HandleWeatherChanges(pChar, socket);
		HandleItemCollision(pChar, socket, amTurning);*/
	}

	// do all of the following regardless of whether turning or moving i guess
	// set the player direction to contain only the cardinal direction bits
	pChar->setDir(dir);
	
	if( socket )
		socket->allowMove( sequence );

	RegionIterator4Chars ri( pChar->pos );

	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pChar_vis = ri.GetData();

		if( pChar_vis && ( pChar_vis != pChar ) && ( !pChar->dead() || pChar->war() || pChar_vis->isGM() ) && ( !pChar->isHidden() || pChar_vis->isGM() ) )
			sendWalkToOther( pChar_vis, pChar, oldpos );
	}
	
	// keep on checking this even if we just turned, because if you are taking damage
	// for standing here, lets keep on dishing it out. if we pass whether we actually
	// moved or not we can optimize things some
}

// Thyme 07/28/00
// Here's how I'm going to use it for now.  Movement Type may be used for races, that's why
// I put it as an integer.  Here are the values I'm going to use:
// GM Body  0x01
// Player   0x02
// Bird     0x20 (basically, a fish and an NPC, so I could use 0xc0, but I don't wanna)
// NPC      0x40
// Fish     0x80 (So they can swim!)
// I left a gap between Player and NPC because someone may want to implement race
// restrictions...
short int cMovement::CheckMovementType(P_CHAR pc)
{
	// Am I a GM Body?
	if( pc->isGMorCounselor() || pc->dead() )
		return P_C_IS_GM_BODY;

	// Am I a player?
	if (pc->isPlayer())
		return P_C_IS_PLAYER;

	// Change this to a flag in NPC.scp

	short int retval = P_C_IS_NPC;
	switch ( pc->id() )
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
		if (pc_i != NULL)
		{
			if (pc_i != pc && (online(pc_i) || pc_i->isNpc()))
			{
				// x=x,y=y, and distance btw z's <= MAX STEP
				if ((pc_i->pos.x == pos.x) && (pc_i->pos.y == pos.y) && (abs(pc_i->pos.z-pos.z) <= P_M_MAX_Z_CLIMB))
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
	if( pChar->stealth() != -1 ) 
	{
		pChar->setStealth( -1 );
		pChar->setHidden( 0 );
		pChar->resend( false );
	}

	// Don't regenerate stamina while running
	pChar->setRegen2( uiCurrentTime + ( SrvParams->staminarate() * MY_CLOCKS_PER_SEC ) );
	pChar->setRunning( pChar->running() + 1 );
	
	// If we're running on our feet, check for stamina loss
	// Crap
	if( !pChar->dead() && !pChar->atLayer( cChar::Mount ) && pChar->running() > ( SrvParams->runningStamSteps() ) * 2 )
	{
		// The *2 it's because i noticed that a step(animation) correspond to 2 walking calls
		// ^^ WTF?
		pChar->setRunning( 0 );
//		pChar->stm--;
		tempshort = pChar->stm();
		pChar->setStm( --tempshort );
		socket->updateStamina();
	}

	if( pChar->war() && pChar->targ() != INVALID_SERIAL )
		pChar->setTimeOut(uiCurrentTime + ( MY_CLOCKS_PER_SEC * 2 ) ); // 2 Second timeout
}

void cMovement::checkStealth( P_CHAR pChar )
{
	if( pChar->hidden() && !pChar->isHiddenPermanently() )
	{
		if( pChar->stealth() != -1 )
		{
			pChar->setStealth( pChar->stealth() + 1 );
			if( pChar->stealth() > ( ( SrvParams->maxStealthSteps() * pChar->skill( STEALTH ) ) / 1000 ) )
			{
				if( pChar->socket() )
					pChar->socket()->sysMessage( tr( "You have been revealed." ) );
				pChar->setStealth( -1 );
				pChar->setHidden( 0 );
				pChar->resend( false );
				if( pChar->socket() )
					pChar->socket()->updatePlayer();
			}
		}
		else
		{
			if( pChar->socket() )
				pChar->socket()->sysMessage( tr( "You have been revealed." ) );
			pChar->setHidden( 0 );
			pChar->resend( false );
			if( pChar->socket() )
				pChar->socket()->updatePlayer();
		}
	}
}

void cMovement::GetBlockingMap( const Coord_cl pos, unitile_st *xyblock, int &xycount)
{
	int mapid = 0;
	signed char mapz = Map->mapElevation(pos);  //Map->AverageMapElevation(x, y, mapid);
	if (mapz != illegal_z)
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
				if ((mapitem->pos.x == position.x) && (mapitem->pos.y == position.y))
				{
					tile_st tile = TileCache::instance()->getTile( mapitem->id() );
					xyblock[xycount].type=1;
					xyblock[xycount].basez=mapitem->pos.z;
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
				(abs(mapitem->pos.x - position.x)<=BUILDRANGE)&&
				(abs(mapitem->pos.y - position.y)<=BUILDRANGE)
				)
			{
				MultiDefinition* def = MultisCache->getMulti( mapitem->id() - 0x4000 );
				if ( !def )
					continue;
				QValueVector<multiItem_st> multi = def->getEntries();
				for (int j = 0; j < multi.size(); ++j)
				{
					if (multi[j].visible && (mapitem->pos.x+multi[j].x == position.x) && (mapitem->pos.y+multi[j].y == position.y))
					{
						tile_st tile = TileCache::instance()->getTile( multi[j].tile );
						xyblock[xycount].type = 2;
						xyblock[xycount].basez = multi[j].z + mapitem->pos.z;
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

void cMovement::sendWalkToOther( P_CHAR pChar, P_CHAR pWalker, const Coord_cl& oldpos )
{
	// What we need to decide here is if we
	// Need to UPDATE the player (so it's walking on the screen)
	// Or if we need to CREATE the player if it was not in 
	// The visibility range of our Viewer

	cUOSocket *socket = pWalker->socket();
	cUOSocket *visSocket = pChar->socket();

	// If both are not connected it's useless to send updates
	if( !socket && !visSocket )
		return;

	// We can see the target and didn't see it before
	Q_UINT32 newDistance = pChar->pos.distance( pWalker->pos );
	Q_UINT32 oldDistance = pChar->pos.distance( oldpos );
	
	// We dont see him, he doesn't see us
	if( ( newDistance > pChar->VisRange() ) && ( newDistance > pWalker->VisRange() ) )
		return;

	// Someone got into our range
	if( socket && ( newDistance <= pWalker->VisRange() ) && ( oldDistance > pWalker->VisRange() ) )
		socket->sendChar( pChar );

	// This guy is already known to us
	// So we dont need to send anything as HE didn't walk
	//if( socket && ( newDistance <= pWalker->VisRange ) && ( oldDistance <= pWalker->VisRange ) )
	//	socket->updateChar( pChar );

	// We got into somone elses Range
	if( visSocket && ( newDistance <= pChar->VisRange() ) && ( oldDistance > pChar->VisRange() ) )
		visSocket->sendChar( pWalker );

	// We are already known to this guy
	if( visSocket && ( newDistance <= pChar->VisRange() ) && ( oldDistance <= pChar->VisRange() ) )
		visSocket->updateChar( pWalker );
}

// see if we should mention that we shove something out of the way
void cMovement::outputShoveMessage( P_CHAR pChar, cUOSocket *socket, const Coord_cl& oldpos )
{
	if( !socket )
		return;

	const int visibleRange = VISRANGE;
	signed short tempshort;

	RegionIterator4Chars ri( pChar->pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR mapChar = ri.GetData();

		if( !mapChar || mapChar == pChar || ( !online( mapChar ) && !mapChar->isNpc() ) )
			continue;

		// If it's not on the same position it's useless to check
		if( mapChar->pos != pChar->pos )
			continue;

		if( ( pChar->id() == 0x3DB ) || ( pChar->id() == 0x192 ) || ( pChar->id() == 0x193 ) || !pChar->isGMorCounselor() )
			continue;

		// Trigger the event for the "stumbled apon" character
		if( pChar->onCollideChar( mapChar ) )
			continue;

		if( mapChar->onCollideChar( pChar ) )
			continue;

		if( mapChar->isHidden() && !mapChar->dead() && !mapChar->isInvul() && !mapChar->isGM() )
		{
			if( socket )
				socket->sysMessage( tr( "Being perfectly rested, you shoved something invisible out of the way." ) );

		    pChar->setStm( QMAX( pChar->stm() - 4, 0 ) );
			updatestats( pChar, 2 );
		}
		else if( !mapChar->isHidden() && !mapChar->dead() && (!(mapChar->isInvul())) &&(!(mapChar->isGM()))) // ripper..GMs and ghosts dont get shoved.)
		{
			if( socket )
				socket->sysMessage( tr( "Being perfectly rested, you shove %1 out of the way." ).arg( mapChar->name ) );
			
			pChar->setStm( QMAX( ( tempshort = pChar->stm() ) - 4, 0 ) );
			updatestats( pChar, 2 );
		}
		else if( !mapChar->isGMorCounselor() && !mapChar->isInvul() ) //A normal player (No priv1(Not a gm))
		{
			if( socket )
				socket->sysMessage( "Being perfectly rested, you shove something invisible out of the way." );

			pChar->setStm( QMAX( pChar->stm() - 4, 0 ) );
			updatestats( pChar, 2 );
		}
	}
}

// This is called whenever a char *HAS* already moved
// It will handle Collisions with items and sending them as well
void cMovement::handleItemCollision( P_CHAR pChar )
{
/*	// lets cache these vars in advance
	const int visibleRange = VISRANGE;
	const short int newx = pc->pos.x;
	const short int newy = pc->pos.y;
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
						if( ( mapitem->id() == 0x407C ) || ( mapitem->id() == 0x407D ) || ( mapitem->id() == 0x407E ) )
						{
							if ((abs(newx-mapitem->pos.x)==BUILDRANGE)||(abs(newy-mapitem->pos.y)==BUILDRANGE))
							{
								senditem(socket, mapitem);
							}
						}
						else
						{
							signed int oldd = Distance(oldx, oldy, mapitem->pos.x, mapitem->pos.y);
							signed int newd = Distance(newx, newy, mapitem->pos.x, mapitem->pos.y);
							
							//if ((abs(newx-mapitem->pos.x) == visibleRange ) || (abs(newy-mapitem->pos.y) == visibleRange ))
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

					UI16 tileID = mapitem->id();

					// split out the x,y,z check so we can use else ifs for faster item id checking
					if( ( tileID == 0x3996 ) || ( tileID == 0x398C ) )
					{//Fire Field
// Thyme 2000.09.15
// At the request of Abaddon
// Thyme BEGIN
//						if ((mapitem->pos.x == newx) && (mapitem->pos.y == newy) && (mapitem->pos.z==pc->pos.z))
						if ( ( mapitem->pos.x == newx ) &&
							 ( mapitem->pos.y == newy ) &&
							 ( pc->pos.z >= mapitem->pos.z ) &&
							 ( pc->pos.z <= ( mapitem->pos.z + 5 ) ) )
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
						if ((mapitem->pos.x == newx) && (mapitem->pos.y == newy) && (mapitem->pos.z==pc->pos.z))
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
						if ((mapitem->pos.x == newx) && (mapitem->pos.y == newy) && (mapitem->pos.z==pc->pos.z))
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
						if ((mapitem->pos.x== newx) && (mapitem->pos.y == newy))
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

void cMovement::HandleTeleporters(P_CHAR pc, UOXSOCKET socket, const Coord_cl& oldpos)
{	
}

/********* start of LB's no rain & snow in buildings stuff ***********/
void cMovement::HandleWeatherChanges(P_CHAR pc, UOXSOCKET socket)
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
			if (x!=illegal_z)
				x=illegal_z; // check for dynamic buildings
			if (x==1)
				x = illegal_z; // 1 seems to be the multi-borders
			
			//printf("x: %i\n",x);
			// ah hah! this was a bug waiting to happen if not already, we have overloaded the use of the
			// variable k, which used to hold the socket
//			int k = noweather[socket];
			if (inDungeon || i || x!= illegal_z )
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
	/*
    for (int i=0;i<now;i++)
    {
        if ((perm[i]) && (inrange1p(pc, currchar[i])))
        {
			P_CHAR pc_check = currchar[i];
			LongToCharPtr(pc->serial, &extmove[1]);
			ShortToCharPtr(pc->id(),  &extmove[5]);
            extmove[7] = (unsigned char)(pc->pos.x>>8);
            extmove[8] = (unsigned char)(pc->pos.x%256);
            extmove[9] = (unsigned char)(pc->pos.y>>8);
            extmove[10] = (unsigned char)(pc->pos.y%256);
            extmove[11] = pc->dispz;
            extmove[12] = (unsigned char)(pc->dir&0x7F);

			ShortToCharPtr(pc->skin(), &extmove[13]);


            if (pc->war) 
				extmove[15]=0x40; 
			else 
				extmove[15]=0x00;
            if (pc->hidden()) extmove[15] |= 0x80;
            if (pc->poisoned()) extmove[15] |= 0x04; //AntiChrist -- thnx to SpaceDog
            const int guild = GuildCompare( pc, currchar[i] );
            if (pc->kills > SrvParams->maxkills() ) extmove[16]=6; // ripper
            //if (pc->npcaitype==0x02) extmove[16]=6; else extmove[16]=1;
            //chars[i].flag=0x04;       // everyone should be blue on default
            else if (guild==1)//Same guild (Green)
                extmove[16]=2;
            else if (guild==2) // Enemy guild.. set to orange
                extmove[16]=5;
            //                  else if( !chars[i].npc && ( chars[i].priv&1 || chars[i].priv&80 ) )
            //                          extmove[16] = 7;
            else {
                switch(pc->flag())
                {//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
                case 0x01: extmove[16]=6; break;// If a bad, show as red.
                case 0x04: extmove[16]=1; break;// If a good, show as blue.
                case 0x08: extmove[16]=2; break; //green (guilds)
                case 0x10: extmove[16]=5; break;//orange (guilds)
                default:extmove[16]=3; break;//grey
                }
            }

            if (!pc->war)
            {
                //                              pc->attacker=INVALID_SERIAL;
                pc->targ = INVALID_SERIAL;
            }
            cNetwork::instance()->xSend(i, extmove, 17, 0);
        }
    }*/
}

// type is npcwalk mode ( 0 for normal, 1 for box, 2 for circle )
void cMovement::randomNpcWalk( P_CHAR pChar, Q_UINT8 dir, Q_UINT8 type )   
{
	// If we're not moving at all that could be -1
	if( !isValidDirection( dir ) )
		return;

	// We will precheck that here
	if( pChar->isFrozen() )
		return;

	Coord_cl newCoord = calcCoordFromDir( dir, pChar->pos );

	// When the circle or box is not set yet reset the npcwalking setting
    if(	( ( type == 1 ) && ( pChar->fx1() == -1 ) || ( pChar->fx2() == -1 ) || ( pChar->fy1() == -1 ) || ( pChar->fy2() == -1 ) ) ||
		( ( type == 2 ) && ( pChar->fx1() == -1 ) || ( pChar->fx2() == -1 ) || ( pChar->fy1() == -1 ) ) )
	{
		pChar->setNpcWander( 0 );
		type = 0;
	}
    
	// If we either have to walk in a box or a circle we'll check if the new direction
	// is outside of our bounds
	if( ( type == 1 ) && !checkBoundingBox( newCoord, pChar->fx1(), pChar->fy1(), pChar->fx2(), pChar->fy2() ) )
		return;

	if( ( type == 2 ) && !checkBoundingCircle( newCoord, pChar->fx1(), pChar->fy1(), pChar->fx2() ) )
		return;

	Walking( pChar, dir&0x07, 0xFF );
}

// Ok, I'm going to babble here, but here's my thinking process...
// Max Heuristic is 5 (for now) and I'm not concerned about walls... I'll add that later
// Easiest way I think would be for recursive call for now... Will change later if need be
// pathfind will call itself with new stuff... as long as the distance get's shorter
// We have to take into consideration if the NPC is going to a point or to a character
// if we don't want NPCs to walk over each other, this has to be known, because the NPC
// that's walking will never reach destination if it's another character.
// We must pass, that way if we get chardist=1 then we're ok.  We are basically searching
// for the shortest path, which is always a diagonal line, followed by lateral to target
// (barring obstacles) On calculation, for the FIRST step, we need to know if a character
// is there or not, then after that no biggie because if so we can just recalc the path if
// something is blocking. If we don't check on that first step, NPCs will get stuck behind horses
// and stuff... Kinda exploitable if I'm on a horse attacking, then step off and behind to hide
// while I heal. The first thing we need to do when walking is determine if i'm blocked... then
// if I'm an NPC, recalculate my path and step... I'm also gonna take out the path structure
// in chars_st... all we need is to hold the directions, not the x and y... Hopefully this will
// save memory.
void cMovement::PathFind(P_CHAR pc, unsigned short gx, unsigned short gy)
{
	// Make sure this is a valid character before proceeding
	if ( !pc )
		return;

	// Make sure the character has taken used all of their previously saved steps
	if ( pc->pathnum() < P_PF_MRV ) 
		return;

	path_st newpath[P_PF_MIR];
	pc->setPathNum(0);

	for ( int pn = 0 ; pn < P_PF_MRV ; pn++ )
	{
		newpath[pn].x = newpath[pn].y = 0;
		int pf_neg = ( ( rand() % 2 ) ? 1 : -1 );
		int pf_dir = Direction( pc->pos.x, pc->pos.y, gx, gy );
		for ( int i = 0 ; i < 8 ; i++ )
		{
			pf_neg *= -1;
			pf_dir += ( i * pf_neg );
			Coord_cl newCoord = pc->pos;

			if( mayWalk( pc, newCoord ) )
			{
				if ( ( pn < P_PF_MRV ) && CheckForCharacterAtXYZ( pc, newCoord ) )
					continue;

				newpath[pn].x = newCoord.x;
				newpath[pn].y = newCoord.y;
				break;
			}
		}
		if ( ( newpath[pn].x == 0 ) && ( newpath[pn].y == 0 ) )
		{
			pc->setPathNum(P_PF_MRV);
			break;
		}
	}

	for ( int i = 0 ; i < P_PF_MRV ; i++ )
	{
//		pc->path[i].x = newpath[i].x;
		pc->setPathX(i, newpath[i].x);

//		pc->path[i].y = newpath[i].y;
		pc->setPathY(i, newpath[i].y);

	}
}

// This processes a NPC movement poll
void cMovement::NpcMovement( unsigned int currenttime, P_CHAR pc_i )
{
    int dnpctime=0;
    if( !pc_i->isNpc() || ( pc_i->npcmovetime() > currenttime ) )
		return;

	// If we are fighting and not fleeing move toward our target if neccesary
	if( pc_i->war() && pc_i->npcWander() != 5 )
    {
        P_CHAR pc_attacker = FindCharBySerial( pc_i->targ() ); // This was wrong - we want to move towards our target not our attacker

        if( pc_attacker )
        {
			// Only move in the direction of the target
			// No special pathfinding
			if( pc_attacker->pos.distance( pc_i->pos ) > 1 && ( pc_attacker->socket() || pc_attacker->isNpc() ) )
            {
				//PathFind( pc_i, pc_attacker->pos.x, pc_attacker->pos.y );
                //UINT8 dir = chardirxyz( pc_i, pc_i->path[pc_i->pathnum].x, pc_i->path[pc_i->pathnum].y );
                //if( ( pc_i->dir & 0x07 ) == ( dir & 0x07 ) )
				//	pc_i->pathnum++;
                
				// This is a temporary fix. 
				UINT8 dir = chardir( pc_i, pc_attacker );
				Coord_cl newCoord = calcCoordFromDir( dir, pc_i->pos );
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
			pc_i->setTarg( INVALID_SERIAL );
		return;
    }

	UINT8 j = RandomNum( 0, 32 );

	switch( pc_i->npcWander() )
    {
    case 1: // Follow the follow target
		{
			P_CHAR pc_target = FindCharBySerial( pc_i->ftarg() );

	        if( !pc_target )
				return;

		    if( pc_target->socket() || pc_target->isNpc() )
			{
				if ( chardist( pc_i, pc_target ) > 1 )
				{
					//PathFind( pc_i, pc_target->pos.x, pc_target->pos.y );
	                //UINT8 dir = chardirxyz(pc_i, pc_i->path[pc_i->pathnum].x, pc_i->path[pc_i->pathnum].y);
					//UINT8 dir = chardirxyz(pc_i, pc_i->pathX( pc_i->pathnum() ), pc_i->pathY(pc_i->pathnum()) );
					//pc_i->pathnum++;
					//pc_i->setPathNum( pc_i->pathnum() + 1 );
					// Disabled Path finding until a good algorythm is found
					UINT8 dir = chardir( pc_i, pc_target );

			        Walking( pc_i, dir, 0xFF );
				}

				// Has the Escortee reached the destination ??
/*				if( !pc_target->dead && ( pc_i->questDestRegion() == pc_i->region ) )
				{
					// Pay the Escortee and free the NPC
					MsgBoardQuestEscortArrive( pc_i, calcSocketFromChar( pc_target ) );
				}*/
	        }
		}
        break;
    case 2: // Wander freely, avoiding obstacles.
        if( j < 8 || j > 32) 
			dnpctime = 5;
        if( j > 7 && j < 33) // Let's move in the same direction lots of the time.  Looks nicer.
            j = pc_i->dir();
        randomNpcWalk( pc_i, j, 0 );
        break;
    case 3: // Wander freely, within a defined box
        if (j<8 || j>32) dnpctime=5;
        if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
            j=pc_i->dir();

        randomNpcWalk(pc_i,j,1);
        break;
    case 4: // Wander freely, within a defined circle
        if (j<8 || j>32) dnpctime=5;
        if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
            j=pc_i->dir();
        randomNpcWalk(pc_i,j,2);
        break;
    case 5: // Flee
		{
			P_CHAR pc_k = FindCharBySerial(pc_i->targ());
			if (pc_k == NULL) return;
			
			if ( chardist(pc_i, pc_k) < P_PF_MFD )
			{
				// calculate a x,y to flee towards
				int mydist = P_PF_MFD - chardist( pc_i, pc_k) + 1;
				j = chardirxyz( pc_i, pc_k->pos.x, pc_k->pos.y );
				Coord_cl fleeCoord = calcCoordFromDir( j, pc_i->pos );

				if ( fleeCoord != pc_i->pos )
				{
					Q_INT8 xfactor = 0;
					Q_INT8 yfactor = 0;

					if( fleeCoord.x < pc_i->pos.x )
						xfactor = -1;
					else
						xfactor = 1;

					if( fleeCoord.y < pc_i->pos.y )
						xfactor = -1;
					else
						xfactor = 1;

					fleeCoord.x += ( xfactor * mydist );
					fleeCoord.y += ( yfactor * mydist );
				}
			
				PathFind( pc_i, fleeCoord.x, fleeCoord.y );
//				j = chardirxyz(pc_i, pc_i->path[ pc_i->pathnum ].x, pc_i->path[ pc_i->pathnum ].y);
				j = chardirxyz(pc_i, pc_i->pathX( pc_i->pathnum() ), pc_i->pathY( pc_i->pathnum() ) );
//				pc_i->pathnum++;
				pc_i->setPathNum( pc_i->pathnum() + 1 );
				Walking( pc_i, j, 256 );
			}
			else
			{ // wander freely... don't just stop because I'm out of range.
				j=rand()%40;
				if (j<8 || j>32) dnpctime=5;
				if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
					j=pc_i->dir();
				randomNpcWalk(pc_i,j,0);
			}
		}
		break;
	}

	pc_i->setNextMoveTime();
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
	Checks if the character is overloaded
	this function is deprecataed and will
	most likely be removed soon.
*/
inline bool cMovement::isOverloaded( P_CHAR pc )
{
	if ( !pc->dead() && !pc->isNpc() && !pc->isGMorCounselor() )
		if( !Weight->CheckWeight( pc ) || ( pc->stm() < 3 ) )
			return true;
	return false;	
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
bool cMovement::consumeStamina( cUOSocket *socket, P_CHAR pChar, bool running )
{
	// TODO: Stamina loss is disabled for now -- Weight system needs to be rediscussed
	return true;

	// Weight percent
	UINT32 allowedWeight = ( pChar->st() * WEIGHT_PER_STR ) + 30;
	UINT8 load = pChar->weight() / allowedWeight;

	if( running )
		load *= 2;

	// 200% load is too much
	if( load >= 200 )
	{
		socket->sysMessage( tr( "You are too overloaded to move." ) );
		return false;
	}

	// 20% overweight = ( 0.20 * 0.10 ) * (Weight carrying) = Stamina needed to move
	INT32 overweight = load - 100;

	// We're not overloaded so we dont need additional stamina
	if( overweight < 0 )
		return true;

	INT32 requiredStamina = (INT32)((double)( (double)overweight * 0.10f ) * (double)pChar->weight());
	
	if( pChar->stm() < requiredStamina ) 
	{
		pChar->talk( tr( "You are too exhausted to move" ) );
		return false;
	}
}

/*!
  This checks the new tile we're moving to
  for Character we could eventually bump into.
*/
bool cMovement::checkObstacles( cUOSocket *socket, P_CHAR pChar, const Coord_cl &newPos, bool running )
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
				MultiDefinition* def = MultisCache->getMulti( mapitem->id() - 0x4000 );
				if ( !def )
					return 0;
				QValueVector<multiItem_st> multi = def->getEntries();
				for( UINT32 j = 0; j < multi.size(); ++j )
				{
					if( ( multi[j].visible && ( mapitem->pos.x + multi[j].x == pos.x ) && ( mapitem->pos.y + multi[j].y == pos.y )
						&& ( abs( mapitem->pos.z + multi[j].z - pos.z ) <= 1 ) ) )
					{
						return multi[j].tile;
					}
				}
			}
			else if ( mapitem->pos == pos )
				return mapitem->id();
        }    
		
    }
	return (UINT16)-1;
}

bool cMovement::canLandMonsterMoveHere( const Coord_cl& pos ) const
{
	if( pos.x >= ( Map->mapTileWidth(pos.map) * 8 ) || pos.y >= ( Map->mapTileHeight(pos.map) * 8 ) )
		return false;
    const signed char elev = Map->height( pos );
	Coord_cl target = pos;
	target.z = elev;
	if (illegal_z == elev)
		return false;

	// is it too great of a difference z-value wise?
	if (pos.z != illegal_z)
	{
		// you can climb MaxZstep, but fall up to 15
		if (elev - pos.z > MaxZstep)
			return false;
		else if (pos.z - elev > 15)
			return false;
	}

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
	
	while ( !msi.atEnd() )
	{
		tile_st tile = TileCache::instance()->getTile( msi->itemid );
		const INT32 elev = msi->zoff + cTileCache::tileHeight(tile);
		if( (elev >= pos.z) && (msi->zoff <= pos.z ) )
		{
			if (tile.isBlocking() || tile.isWet()) 
				return false;
		}
	}
    return true;
}
