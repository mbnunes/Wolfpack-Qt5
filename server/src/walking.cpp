/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "walking.h"
#include "mapobjects.h"
#include "serverconfig.h"
#include "network/network.h"
#include "muls/maps.h"
#include "muls/tilecache.h"
#include "territories.h"
#include "network/uosocket.h"
#include "muls/multiscache.h"
#include "muls/tilecache.h"
#include "items.h"
#include "multi.h"
#include "basechar.h"
#include "npc.h"
#include "player.h"

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
//  	 P_M = Property for Movement

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
#define P_M_MAX_Z_BLOCKS	14

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
// fur  		  : 1999.10.27 - rewrite of walking.cpp with documentation!
//  			  : 1999.10.27 - ripped apart walking into smaller functions
// Tauriel  	  : 1999.03.06 - For all of the region stuff
// knoxos   	  : 2000.08.?? - For finally making use of the flags, and height blocking
// DarkStorm	  : 2002.06.19 - Cleaning up the mess

// To clear things up, we only need to care about
// blocking items anyway. So we don't need 90% of the data
// previously gathered. What we need is a check if the character
// Can walk the tile, the tile's height and if the tile's a stair
struct stBlockItem
{
	Q_INT8 z;
	Q_UINT8 height;
	bool walkable;
	bool maptile;

	stBlockItem() : z( -128 ), height( 0 ), walkable( false ), maptile( false )
	{
	}
};

// Keep in mind that this only get's called when
// the tile we're walking on is impassable
bool checkWalkable( P_CHAR pChar, Q_UINT16 tileId )
{
	if (!pChar) {
		return false;
	}

	Q_UNUSED( pChar );
	Q_UNUSED( tileId );
	return false;
}

struct compareTiles : public std::binary_function<stBlockItem, stBlockItem, bool>
{
	bool operator()( stBlockItem a, stBlockItem b )
	{
		// If the items have the same top, the one with the surface flag has precedence
		int itemTopA = a.height + a.z;
		int itemTopB = b.height + b.z;

		if (itemTopA == itemTopB) {
			if (a.height == 0 && a.walkable) {
				return true;
			}

			if (b.height == 0 && b.walkable) {
				return false;
			}
		}

		return ( itemTopA > itemTopB );
	}
};

// The highest items will be @ the beginning
// While walking we always will try the highest first.
vector< stBlockItem > getBlockingItems( P_CHAR pChar, const Coord& pos )
{
	vector<stBlockItem> blockList;
	make_heap( blockList.begin(), blockList.end(), compareTiles() );

	// Process the map at that position
	stBlockItem mapBlock;
	mapBlock.maptile = true;
	mapBlock.z = Maps::instance()->mapAverageElevation( pos );
	mapBlock.height = 0;

	// TODO: Calculate the REAL average Z Value of that Map Tile here! Otherwise clients will have minor walking problems.
	map_st mapCell = Maps::instance()->seekMap( pos );
	//mapBlock.z = mapCell.z;
	land_st mapTile = TileCache::instance()->getLand( mapCell.id );

	// If it's not impassable it's automatically walkable
	if ( !( mapTile.flag1 & 0x40 ) )
		mapBlock.walkable = true;
	else
		mapBlock.walkable = checkWalkable( pChar, mapCell.id );

	if ( mapCell.id != 0x02 )
	{
		blockList.push_back( mapBlock );
		push_heap( blockList.begin(), blockList.end(), compareTiles() );
	}

	// Now for the static-items
	StaticsIterator staIter = Maps::instance()->staticsIterator( pos, true );
	for ( ; !staIter.atEnd(); ++staIter )
	{
		tile_st tTile = TileCache::instance()->getTile( staIter->itemid );

		// Here is decided if the tile is needed
		// It's uninteresting if it's NOT blocking
		// And NOT a bridge/surface
		if ( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
			continue;

		stBlockItem staticBlock;
		staticBlock.z = staIter->zoff;

		// If we are a surface we can always walk here, otherwise check if
		// we are special
		if ( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
			staticBlock.walkable = true;
		else
			staticBlock.walkable = checkWalkable( pChar, staIter->itemid );

		// If we are a stair only the half height counts (round up)
		if ( tTile.flag2 & 0x04 )
			staticBlock.height = ( Q_UINT8 ) ( ( tTile.height ) / 2 );
		else
			staticBlock.height = tTile.height;

		blockList.push_back( staticBlock );
		push_heap( blockList.begin(), blockList.end(), compareTiles() );
	}

	// We are only interested in items at pos
	// todo: we could impliment blocking for items on the adjacent sides 
	// during a diagonal move here, but this has yet to be decided.

	MapItemsIterator iIter = MapObjects::instance()->listItemsAtCoord( pos );  
	for ( P_ITEM pItem = iIter.first(); pItem; pItem = iIter.next() )
	{
		if ( pChar && pChar->isDead() )
		{
			// Doors can be passed by ghosts
			if ( pItem->hasScript( "door" ) )
			{
				continue;
			}
		}

		tile_st tTile = TileCache::instance()->getTile( pItem->id() );

		// See above for what the flags mean
		if ( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
			continue;

		stBlockItem blockItem;
		blockItem.height = (tTile.flag2 & 0x04) ? (tTile.height / 2) : tTile.height;
		blockItem.z = pItem->pos().z;

		// Once again: see above for a description of this part
		if ( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
			blockItem.walkable = true;
		else
			blockItem.walkable = checkWalkable( pChar, pItem->id() );

		blockList.push_back( blockItem );
		push_heap( blockList.begin(), blockList.end(), compareTiles() );
	}
	
	
	// deal with the multis now, or not.
	// 18 has been tested with castle sides and corners...
	MapMultisIterator iter = MapObjects::instance()->listMultisInCircle( pos, 18 );
	for( cMulti *pMulti = iter.first(); pMulti; pMulti = iter.next() )
	{
		MultiDefinition* def = MultiCache::instance()->getMulti( pMulti->id() - 0x4000 );
		if ( !def )
			continue;

		QValueVector<multiItem_st> multi = def->getEntries();
		
		for( unsigned int j = 0; j < multi.size(); ++j )
		{
			if ( multi[j].visible && ( pMulti->pos().x + multi[j].x == pos.x ) && ( pMulti->pos().y + multi[j].y == pos.y ) )
			{
				tile_st tTile = TileCache::instance()->getTile( multi[j].tile );
				if ( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
					continue;

				stBlockItem blockItem;
				blockItem.height = (tTile.flag2 & 0x04) ? (tTile.height / 2) : tTile.height;
				blockItem.z = pMulti->pos().z + multi[j].z;

				if ( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
					blockItem.walkable = true;
				else
					blockItem.walkable = checkWalkable( pChar, pMulti->id() );

				blockList.push_back( blockItem );
				push_heap( blockList.begin(), blockList.end(), compareTiles() );
			}
		}
		continue;
	}
	
	// Now we need to evaluate dynamic items [...] (later)  ??
	sort_heap( blockList.begin(), blockList.end(), compareTiles() );

	return blockList;

};

// May a character walk here ? 
// If yes we auto. set the new z value for pos
bool mayWalk( P_CHAR pChar, Coord& pos )
{
	// Go trough the array top-to-bottom and check
	// If we find a tile to walk on
	vector<stBlockItem> blockList = getBlockingItems( pChar, pos );
	bool found = false;
	Q_UINT32 i;
	bool priviledged = false;
	Q_INT32 oldz = pos.z;

	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );

	priviledged = player && player->isGM();

	for ( i = 0; i < blockList.size(); ++i )
	{
		stBlockItem item = blockList[i];
		Q_INT32 itemTop = ( item.z + item.height );

		// If we found something to step on and the next tile
		// below would block us, use the good one instead
		if (found && (itemTop > pos.z - P_M_MAX_Z_BLOCKS || !item.walkable)) {
			break;
		}

		// If we encounter any object with itemTop <= pos.z which is NOT walkable
		// Then we can as well just return false as while falling we would be
		// blocked by that object
		if ( !item.walkable && !priviledged && itemTop <= pos.z )
			return false;

		if ( item.walkable || priviledged ) {
			// If the top of the item is within our max-climb reach
			// then the first check passed. in addition we need to
			// check if the "bottom" of the item is reachable
			// I would say 2 is a good "reach" value for the bottom
			// of any item
			if ( itemTop < pos.z + P_M_MAX_Z_CLIMB && itemTop >= pos.z - P_M_MAX_Z_FALL ) {
				// We already found something to step on above.
				// See if it's easier to step down.
				if (found && abs(oldz - pos.z) < abs(oldz - itemTop)) {
					break;
				}

				pos.z = itemTop;
				found = true;

				if (item.height > 1) {
					break;
				}

				// break; - We can't break here anymore since we have to check if the ground would be easier
				// to step on
			// Climbing maptiles is 5 tiles easier
			} else if ( item.maptile && itemTop < pos.z + P_M_MAX_Z_CLIMB + 5 && itemTop >= pos.z - P_M_MAX_Z_FALL ) {
				// We already found something to step on above.
				// See if it's easier to step down.
				if (found && abs(oldz - pos.z) < abs(oldz - itemTop)) {
					break;
				}

				pos.z = itemTop;
				found = true;
				break;
			} else if ( itemTop < pos.z ) {
				// We already found something to step on above.
				// See if it's easier to step down.
				if (found && abs(oldz - pos.z) < abs(oldz - itemTop)) {
					break;
				}

				pos.z = itemTop;
				found = true;
				break;
			}
		}
	}

	if ( priviledged )
	{
		return true;
	}

	// If we're still at the same position
	// We didn't find anything to step on
	if ( !found )
		return false;

	// Another loop *IS* needed here (at least that's what i think)
	for ( i = 0; i < blockList.size(); ++i )
	{
		// So we know about the new Z position we are moving to
		// Lets check if there is enough space ABOVE that position (at least 15 z units)
		// If there is ANY impassable object between pos.z and pos.z + 15 we can't walk here
		stBlockItem item = blockList[i];
		Q_INT8 itemTop = ( item.z + item.height );

		// If the item is below what we step on, ignore it
		if (itemTop <= pos.z) {
			continue;
		}

		// Does the top of the item looms into our space
		// Like before 15 is the assumed height of ourself
		// Use the new position here.
		if ( ( itemTop > pos.z ) && ( itemTop < pos.z + P_M_MAX_Z_BLOCKS ) )
			return false;

		// Or the bottom ?
		// note: the following test was commented out.  by putting the code back in,
		// npcs stop wandering through the walls of multis.  I am curious if this code
		// has other (negative) affects besides that.
		 if ( ( item.z > oldz ) && ( item.z < oldz + P_M_MAX_Z_BLOCKS / 2 ) )
			return false;

		// Or does it spread the whole range ?
		if ( ( item.z <= oldz ) && ( itemTop >= oldz + P_M_MAX_Z_BLOCKS / 2 ) )
			return false;

		// Is it at the new position ?
		 if ( ( item.z >= pos.z ) && ( item.z < pos.z + P_M_MAX_Z_BLOCKS ) )
			return false;
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
	Coord dPos = pChar->pos();

	if ( pItem->onCollide( pChar ) )
		return true;

	return false;
}

/*!
	Sends items which came in range and handles collisions with teleporters
	or damaging items.
*/
void handleItems( P_CHAR pChar, const Coord& oldpos )
{
	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );

	MapItemsIterator iter = MapObjects::instance()->listItemsInCircle( pChar->pos(), VISRANGE );
	for( P_ITEM pItem = iter.first(); pItem; pItem = iter.next() )
	{
		// Check for item collisions here.
		if ( pChar->pos().x == pItem->pos().x && pChar->pos().y == pItem->pos().y )
		{
			if ( pItem->pos().z >= pChar->pos().z - 15 && pItem->pos().z <= pChar->pos().z + 15 )
			{
				if ( handleItemCollision( pChar, pItem ) )
				{
					break;
				}
			}
		}

		// If we are a connected player then send new items
		if( player && player->socket() )
		{
			UI32 oldDist = oldpos.distance( pItem->pos() );
			if( oldDist >= player->visualRange() )
			{
				// was out of range before and now is in range
				pItem->update( player->socket() );
			}
		}
	}
}

/*!
	Sends multis which came in range and handles collisions with them.
*/
void handleMultis( P_CHAR pChar, const Coord& oldpos )
{
	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );

	MapMultisIterator multis = MapObjects::instance()->listMultisInCircle( pChar->pos(), BUILDRANGE );
	for( P_MULTI multi = multis.first(); multi; multi = multis.next() )
	{
		// TODO: handle multi collisions here.

		// If we are a connected player then send new multis
		if( player && player->socket() )
		{
			UI32 oldDist = oldpos.distance( multi->pos() );
			if( oldDist >= BUILDRANGE )
			{
				// was out of range before and now is in range
				multi->update( player->socket() );
			}
		}
	}
}

/*!
	This handles if a character actually tries to walk (NPC & Player)
*/
bool cMovement::Walking( P_CHAR pChar, Q_UINT8 dir, Q_UINT8 sequence )
{
	if ( !pChar )
		return false;

	// Scripting
	if ( pChar->onWalk( dir, sequence ) )
		return false;

	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );

	// Is the sequence in order ?
	if ( player && player->socket() && !verifySequence( player->socket(), sequence ) )
		return false;

	// If checking for weight is more expensive, shouldn't we check for frozen first?
	if ( pChar->isFrozen() )
	{
		if ( player && player->socket() )
			player->socket()->denyMove( sequence );
		return false;
	}

	// save our original location before we even think about moving
	const Coord oldpos( pChar->pos() );

	// If the Direction we're moving to is different from our current direction
	// We're turning and NOT moving into a specific direction
	// Clear the running flag here (!)
	// If the direction we're moving is already equal to our current direction
	bool running = dir & 0x80;
	dir = dir & 0x7; // Remove all unneeded stuff

	pChar->setRunning(running);

	bool turning = dir != pChar->direction();

	// This happens if we're moving
	if ( !turning )
	{
		// Note: Do NOT use the copy constructor as it'll create a reference
		Coord newCoord = calcCoordFromDir( dir, pChar->pos() );

		// Check if the stamina parameters
		if ( player && !consumeStamina( player, running ) )
		{
			if ( player->socket() )
				player->socket()->denyMove( sequence );
			return false;
		}

		// Check if we're going to collide with characters
		if( player )
		{
			// Player vs characters
			if( player->socket() && player->pos().map == 0 && !player->account()->isStaff() )
			{
				// Currently hard-limiting collisions to Felucca; this should be a server option!
				MapCharsIterator charCollisions = MapObjects::instance()->listCharsAtCoord( newCoord );
				for( P_CHAR them = charCollisions.first(); them; them = charCollisions.next() )
				{
					if( them == player )
						continue;

					P_PLAYER otherplayer = dynamic_cast<P_PLAYER>( them );
					if( otherplayer && otherplayer->account()->isStaff() )
						continue; // no collisions against the staff

					if( wpAbs<SI08>( newCoord.z - them->pos().z ) < P_M_MAX_Z_CLIMB )
					{
						// to push another char we must have maximum stamina
						if( player->stamina() >= player->maxStamina() )
						{
							player->socket()->clilocMessage( them->isHidden() ? 1019043 : 1019042 );
							player->setStamina( player->stamina() - 10 );
							break;
						}
						else
						{
							player->socket()->denyMove( sequence );
							return false;
						}
					}
				}
			}
		}
		else
		{
			// NPC vs characters
			P_NPC npc = dynamic_cast<P_NPC>( pChar );
			if( npc && CheckForCharacterAtXYZ( pChar, newCoord ) )
			{
				npc->clearPath();
				return false;
			}
		}

		// Check if the char can move to those new coordinates
		// It is going to automatically calculate the new coords (!)
		if ( !mayWalk( pChar, newCoord ) )
		{
			if ( player && player->socket() )
				player->socket()->denyMove( sequence );
			return false;
		}
		else
		{
			if ( player && player->socket() )
				player->socket()->allowMove( sequence );
		}

		// We moved so let's update our location
		pChar->moveTo( newCoord );
		pChar->setStepsTaken( pChar->stepsTaken() + 1 );
		pChar->setLastMovement( Server::instance()->time() );
		checkStealth( pChar ); // Reveals the user if neccesary
	}
	else
	{
		if ( player && player->socket() )
			player->socket()->allowMove( sequence );
	}

	// do all of the following regardless of whether turning or moving i guess
	// set the player direction to contain only the cardinal direction bits
	pChar->setDirection( dir );

	Coord upperLeft = pChar->pos() + Coord(- (VISRANGE + 1), - (VISRANGE + 1));
	Coord lowerRight = pChar->pos() + Coord(VISRANGE + 1, VISRANGE + 1);

	MapCharsIterator ri = MapObjects::instance()->listCharsInRect(upperLeft, lowerRight);
	for( P_CHAR observer = ri.first(); observer; observer = ri.next() )
	{
		if( observer == pChar )
			continue;

		bool wasVisible = observer->pos().distance(oldpos) <= VISRANGE; // We were previously in range
		bool isVisible = player->dist(observer) <= VISRANGE; // We are now in range

		// If we are a player, send us new characters
		if( player && player->socket() ) {
			// Send the observer to us if he was previously not visible and came into range recently
			if( !wasVisible && isVisible ) {
				player->socket()->sendChar(observer);
			}
		}

		// Send our movement to the observer
		P_PLAYER otherplayer = dynamic_cast<P_PLAYER>(observer);

		if (!otherplayer || !otherplayer->socket()) {
			continue; // Skip this character, it's a player.
			// TODO: OnSeePlayer, OnLoosePlayer
		}

		if (wasVisible) {
			if (isVisible) {
				otherplayer->socket()->updateChar(pChar); // We walked inside the visible range
			} else {
				otherplayer->socket()->removeObject(pChar); // We walked out of visible range
			}

		} else if (isVisible) {
			otherplayer->socket()->sendChar(pChar); // We walked into visible range
		}
	}

	// If we really moved handle teleporters and new items
	if ( !turning )
	{
		handleItems( pChar, oldpos );
		handleMultis( pChar, oldpos );
		handleTeleporters( pChar, oldpos );
	}

	return true;
}

bool cMovement::CheckForCharacterAtXYZ( P_CHAR pc, const Coord& pos )
{
	MapCharsIterator ri = MapObjects::instance()->listCharsAtCoord( pos );
	for( P_CHAR pChar = ri.first(); pChar; pChar = ri.next() )
	{
		if( pChar != pc && !pChar->isHidden() && !pChar->isInvisible() )
		{
			// x=x,y=y, and distance btw z's <= MAX STEP
			if( wpAbs<SI08>( pChar->pos().z - pos.z ) <= P_M_MAX_Z_CLIMB )
			{
				return true;
			}
		}
	}

	return false;
}

// if we have a valid socket, see if we need to deny the movement request because of
// something to do with the walk sequence being out of sync.
bool cMovement::verifySequence( cUOSocket* socket, Q_UINT8 sequence ) throw()
{
	if ( ( socket->walkSequence() == 0 && sequence != 0 ) )
	{
		return false;
	}

	// Simply ignore this packet.
	// It's out of sync
	if ( socket->walkSequence() != sequence )
	{
		socket->denyMove( sequence );
		return false;
	}

	return true;
}

void cMovement::checkStealth( P_CHAR pChar )
{
	if ( pChar->isHidden() && !pChar->isInvisible() )
	{
		// We have not enough steps left
		if ( pChar->stealthedSteps() <= 0 )
		{
			pChar->unhide();
		}
		else
		{
			pChar->setStealthedSteps( pChar->stealthedSteps() - 1 );
		}
	}
}

void cMovement::handleTeleporters( P_CHAR pc, const Coord& oldpos )
{
	cTerritory* territory = pc->region();

	if ( !territory )
		Territories::instance()->check( pc );

	if ( territory && pc->pos() != oldpos )
	{
		if ( territory->haveTeleporters() )
		{
			Coord destination = pc->pos();
			if ( territory->findTeleporterSpot( destination ) )
			{
				bool quick = pc->pos().map != destination.map;
				pc->removeFromView( false );
				pc->moveTo( destination );
				pc->resend( false );
				P_PLAYER player = dynamic_cast<P_PLAYER>( pc );
				if ( player && player->socket() )
				{
					player->socket()->resendPlayer( quick );
					player->socket()->resendWorld();
				}
			}
		}
	}
}

/*!
	Calculates a new position out of the old position
	and the direction we're moving to.
*/
Coord cMovement::calcCoordFromDir( Q_UINT8 dir, const Coord& oldCoords )
{
	Coord newCoords( oldCoords );

	// We're not switching the running flag
	switch ( dir & 0x07 )
	{
	case 0x00:
		newCoords.y--; break;
	case 0x01:
		newCoords.y--; newCoords.x++; break;
	case 0x02:
		newCoords.x++; break;
	case 0x03:
		newCoords.y++; newCoords.x++; break;
	case 0x04:
		newCoords.y++; break;
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
bool cMovement::consumeStamina( P_PLAYER pChar, bool running ) {
	// Dead people and gms don't care about weight
	if (pChar->isDead() || pChar->isGMorCounselor()) {
		return true;
	}

	// Calculate the stones we weight too much
	int overWeight = ( int )( pChar->weight() - pChar->maxWeight() );
	bool mounted = pChar->atLayer(cBaseChar::Mount) != 0;
	bool update = false;

	// We carry too much
	if (overWeight > 0) {
		// How much stamina we loose
		int amount = 5 + (overWeight / 25);

		// Only one third loss if mounted
		if (mounted) {
			amount = amount / 3;
		}

		// Double loss if running
		if (running) {
			amount = amount * 2;
		}

		// Set the new stamina
		pChar->setStamina( wpMax<Q_INT16>( 0, pChar->stamina() - amount ), false );
		update = true;

		// We are overloaded
		if (pChar->stamina() == 0) {
			pChar->socket()->updateStamina();
			pChar->socket()->clilocMessage(500109);
			return false;
		}
	}

	// If we have less than 10% stamina left, we loose
	// stamina more quickly
	if ( ( pChar->stamina() * 100 ) / wpMax<ushort>( 1, pChar->maxStamina() ) < 10 )
	{
		pChar->setStamina( wpMax<Q_INT16>( 0, pChar->stamina() - 1 ), false );
		update = true;
	}

	// We can't move anymore because we are exhausted
	if ( pChar->stamina() == 0 ) {
		pChar->socket()->updateStamina();
		pChar->socket()->clilocMessage( 500110 );
		return false;
	}

	// Normally reduce stamina every few steps
	if ( pChar->stepsTaken() % ( mounted ? 48 : 16 ) == 0 ) {
		pChar->setStamina( wpMax<Q_INT16>( 0, pChar->stamina() - 1 ) );
		update = true;
	}

	if (update) {
		pChar->socket()->updateStamina();
	}

	return true;
}

Q_UINT16 DynTile( const Coord& pos )
{
	MapItemsIterator ri = MapObjects::instance()->listItemsInCircle( pos, 18 );
	for( P_ITEM mapitem = ri.first(); mapitem; mapitem = ri.next() )
	{
		if ( mapitem->isMulti() )
		{
			MultiDefinition* def = MultiCache::instance()->getMulti( mapitem->id() - 0x4000 );
			if ( !def )
				return 0;

			QValueVector<multiItem_st> multi = def->getEntries();
			for ( Q_UINT32 j = 0; j < multi.size(); ++j )
			{
				if ( ( multi[j].visible && ( mapitem->pos().x + multi[j].x == pos.x ) && ( mapitem->pos().y + multi[j].y == pos.y ) && ( abs( mapitem->pos().z + multi[j].z - pos.z ) <= 1 ) ) )
				{
					return multi[j].tile;
				}
			}
		}
		else if( mapitem->pos() == pos )
		{
			return mapitem->id();
		}
	}
	return ( Q_UINT16 ) - 1;
}

bool cMovement::canLandMonsterMoveHere( Coord& pos ) const
{
	if ( pos.x >= ( Maps::instance()->mapTileWidth( pos.map ) * 8 ) || pos.y >= ( Maps::instance()->mapTileHeight( pos.map ) * 8 ) )
		return false;

	int oldz = pos.z;

	// Go trough the array top-to-bottom and check
	// If we find a tile to walk on
	vector<stBlockItem> blockList = getBlockingItems( 0, pos );
	bool found = false;
	Q_UINT32 i;

	for ( i = 0; i < blockList.size(); ++i )
	{
		stBlockItem item = blockList[i];
		Q_INT8 itemTop = ( item.z + item.height );

		// If we encounter any object with itemTop <= pos.z which is NOT walkable
		// Then we can as well just return false as while falling we would be
		// blocked by that object
		if ( !item.walkable && itemTop < pos.z )
			return false;

		if ( item.walkable ) {
			// If the top of the item is within our max-climb reach
			// then the first check passed. in addition we need to
			// check if the "bottom" of the item is reachable
			// I would say 2 is a good "reach" value for the bottom
			// of any item
			if ( itemTop < pos.z + P_M_MAX_Z_CLIMB && itemTop >= pos.z - P_M_MAX_Z_FALL ) {
				pos.z = itemTop;
				found = true;
				break;
			// Climbing maptiles is 5 tiles easier
			} else if ( item.maptile && itemTop < pos.z + P_M_MAX_Z_CLIMB + 5 && itemTop >= pos.z - P_M_MAX_Z_FALL ) {
				pos.z = itemTop;
				found = true;
				break;
			} else if ( itemTop < pos.z ) {
				pos.z = itemTop;
				found = true;
				break;
			}
		}
	}

	// If we're still at the same position
	// We didn't find anything to step on
	if ( !found )
		return false;

	// Another loop *IS* needed here (at least that's what i think)
	for ( i = 0; i < blockList.size(); ++i )
	{
		// So we know about the new Z position we are moving to
		// Lets check if there is enough space ABOVE that position (at least 15 z units)
		// If there is ANY impassable object between pos.z and pos.z + 15 we can't walk here
		stBlockItem item = blockList[i];
		Q_INT8 itemTop = ( item.z + item.height );

		// If the item is below what we step on, ignore it
		if (itemTop <= pos.z) {
			continue;
		}

		// Does the top of the item looms into our space
		// Like before 15 is the assumed height of ourself
		// Use the new position here.
		if ( ( itemTop > pos.z ) && ( itemTop < pos.z + P_M_MAX_Z_BLOCKS ) )
			return false;

		// Or the bottom ?
		// note: the following test was commented out.  by putting the code back in,
		// npcs stop wandering through the walls of multis.  I am curious if this code
		// has other (negative) affects besides that.
		 if ( ( item.z > oldz ) && ( item.z < oldz + P_M_MAX_Z_BLOCKS / 2 ) )
			return false;

		// Or does it spread the whole range ?
		if ( ( item.z <= oldz ) && ( itemTop >= oldz + P_M_MAX_Z_BLOCKS / 2 ) )
			return false;
	}

	return true;
}
