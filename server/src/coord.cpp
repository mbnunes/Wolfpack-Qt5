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

// Include files
#include "coord.h"
#include "player.h"
#include "network/network.h"
#include "mapobjects.h"
#include "items.h"
#include "inlines.h"
#include "muls/multiscache.h"
#include "muls/maps.h"
#include "multi.h"

#include "muls/tilecache.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"
#include "console.h"

// System Includes
#include <math.h>
#include <set>

inline QValueList<Coord> getPointList(const Coord &origin, const Coord &target) {
	// Create a list of coordinates we are going to "touch" when looking
	// from point a to point b
	QValueList<Coord> pointList;

	int xDiff = target.x - origin.x;
	int yDiff = target.y - origin.y;
	int zDiff = target.z - origin.z;

	// Calculate the length of the X,Y diagonal
	double xyDiagonal = sqrt( (double)( xDiff * xDiff + yDiff * yDiff ) );
		
	// Calculate the length of the second diagonal
	double lineLength;
	if (zDiff != 0) {
		lineLength = sqrt( xyDiagonal * xyDiagonal + (double)( zDiff * zDiff ) );
	} else {
		lineLength = xyDiagonal;
	}

	// Calculate the stepsize for each coordinate
	double xStep = xDiff / lineLength;
	double yStep = yDiff / lineLength;
    double zStep = zDiff / lineLength;

	// Initialize loop variables
	double currentY = origin.y;
	double currentZ = origin.z;
	double currentX = origin.x;

	Coord pos = origin;

	while (isBetween(currentX, target.x, origin.x) && isBetween(currentY, target.y, origin.y) && isBetween(currentZ, target.z, origin.z)) {
		pos.x = roundInt(currentX);
		pos.y = roundInt(currentY);
		pos.z = roundInt(currentZ);

		if (pointList.count() == 0 || pointList.last() != pos) {
			pointList.append(pos);
		}

		// Jump to the next set of coordinates.
		currentX += xStep;
		currentY += yStep;
        currentZ += zStep;
	}

	// Add the target to the end of the pointlist if it's not already
	// there
	if (pointList.count() != 0 && pointList.last() != target) {
		pointList.append(target);
	}

	return pointList;
}

// A small structure used to check for blocking dynamics at the given position
struct stBlockingItem {
	unsigned short id;
	int bottom;
	int top;
	bool maptile;
};

// Get blocking tiles at the given x,y,map coordinate
void getBlockingTiles( const Coord &pos, QValueList<stBlockingItem> &items )
{
	stBlockingItem item;

	// Maptiles first
	Maps::instance()->mapTileSpan(pos, item.id, item.bottom, item.top);
	item.maptile = true;

    // Only include this maptile if it's relevant for our line of sight
	if (item.id != 2 && item.id != 0x1DB && (item.id < 0x1AE || item.id > 0x1B5)) {
		items.append(item);
	}

	item.maptile = false;

	// Search for statics at the same position
	StaticsIterator statics = Maps::instance()->staticsIterator(pos, true);

	// Find blocking statics
	for (; !statics.atEnd(); ++statics) {
		const staticrecord &sitem = *statics;
		
		tile_st tile = TileCache::instance()->getTile(sitem.itemid);

		if (tile.flag2 & 0x30) {
			item.bottom = sitem.zoff;
			// Bridges are only half as high
			item.top = item.bottom + ((tile.flag2 & 0x04) ? (tile.height / 2) : tile.height);
			item.id = sitem.itemid;
			items.append(item);
        }
	}

	// Search for items at the given location
	MapItemsIterator itemIter = MapObjects::instance()->listItemsAtCoord( pos );
	for( P_ITEM pItem = itemIter.first(); pItem; pItem = itemIter.next() )
	{
		// If the item is invisible or a multi, skip past it.
		if( pItem->isMulti() )
			continue;

		tile_st tile = TileCache::instance()->getTile( pItem->id() );

		// Window and noshoot tiles block
		if( tile.flag2 & 0x30 )
		{
			item.id = pItem->id();
			item.bottom = pItem->pos().z;
			// Bridges are only half as high
			item.top = item.bottom + ( ( tile.flag2 & 0x04 ) ? ( tile.height / 2 ) : tile.height );
			items.append( item );
		}
	}

	// Check for multis around the area
	MapMultisIterator multis = MapObjects::instance()->listMultisInCircle( pos, BUILDRANGE );

	// Check if there is an intersecting item for this multi
	for( P_MULTI pMulti = multis.first(); pMulti; pMulti = multis.next() )
	{
		// Get all items for this multi
		MultiDefinition *data = MultiCache::instance()->getMulti( pMulti->id() - 0x4000 );
		if( data )
		{
			QValueVector<multiItem_st> mitems = data->getEntries();
			QValueVector<multiItem_st>::iterator it;

			for( it = mitems.begin(); it != mitems.end(); ++it )
			{
				multiItem_st mitem = *it;

				// Skip this multi tile if it's not at the position we need it to be
				if( !mitem.visible || pMulti->pos().x + mitem.x != pos.x || pMulti->pos().y + mitem.y != pos.y )
				{
					continue;
				}

				tile_st tile = TileCache::instance()->getTile( mitem.tile );

				// Has to be blocking
				if( tile.flag2 & 0x30 )
				{
					item.bottom = mitem.z + pMulti->pos().z;
					item.top = item.bottom + ( ( tile.flag2 & 0x04 ) ? ( tile.height / 2 ) : tile.height );
					item.id = mitem.tile;
					items.append( item );
				}
			}
		}
	}
}

// Check for blocking tiles at the given position
inline bool checkBlockingTiles( const QValueList<stBlockingItem> &items, const Coord &pos, const Coord &target )
{
	// Iterate trough all blocking tiles
	QValueList<stBlockingItem>::const_iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		stBlockingItem item = *it;

		// 0x244 tiles are handled differently. If they're the only 
		// tile at the xy position, they're blocking.
		if (item.maptile && item.id == 0x244 && items.count() != 1) {
			continue;
		}

		// Do we intersect the blocking tile?
		if (pos.z >= item.bottom && pos.z <= item.top) {
			// If the blocking item is within our target area, forget about it.
			if (pos.x != target.x || pos.y != target.y || item.bottom > target.z || item.top < target.z) {
				return true;
			}
		}
	}

	return false;
}

// Check the line of sight from a source to a target coordinate.
bool Coord::lineOfSight(const Coord &target, bool debug) const {
	// If the target is out of range, save cpu time by not calculating the
	// line of sight
	if (map != target.map || distance(target) > 25) {
        return false;
	}

	// LoS always succeeds for the same points
	if (*this == target) {
		return true;
	}

	QValueList<Coord> pointList = getPointList(*this, target);

	int lastX = -1, lastY = -1;
	QValueList<stBlockingItem> blockingItems;

	QValueList<Coord>::const_iterator it;
	for (it = pointList.begin(); it != pointList.end(); ++it) {
		Coord point = *it;

		// Get a fresh tile-list
		if (point.x != lastX || point.y != lastY) {
			blockingItems.clear();
			getBlockingTiles(point, blockingItems);
			lastX = point.x;
			lastY = point.y;
		}

		// Check if there are blocking map, static or dynamic items.
		bool blocked = checkBlockingTiles(blockingItems, point, target);

		// Play an effect for the tile
		if (blocked) {
			if (debug) {
				point.effect(0x181D, 10, 50, 0x21);
			}
			return false;
		} else if (debug) {
			point.effect(0x181D, 10, 50, 0x44);
		}
	}

	return true;
}

Coord Coord::null( 0xFFFF, 0xFFFF, 0xFF, 0xFF );

Coord::Coord( void )
{
	x = y = z = map = 0;
}

Coord::Coord( const Coord& clCoord )
{
	( *this ) = clCoord;
}

Coord::Coord( UI16 uiX, UI16 uiY, SI08 siZ, UI08 uiMap, UI08 )
{
	x = uiX;
	y = uiY;
	z = siZ;
	map = uiMap;
	//plane = uiPlane;
}


/*!
 * Calculates the distance between 2 coordinates. If they are in different planes and maps, the
 * return value is infinite (0xFFFFFFFF). The exception is if one of the planes is COORD_ALL_PLANES
 *
 * @param &src : Coordinate to compare
 *
 * @return UI32  : distance result
 */
unsigned int Coord::distance( const Coord& src ) const
{
	UI32 uiResult = ~0U; // Initialize with *infinite*
	if ( map == src.map )
	{
		RF64 rfResult = sqrt( static_cast<RF64>( ( x - src.x ) * ( x - src.x ) + ( y - src.y ) * ( y - src.y )/* + (z - src.z)*(z - src.z)*/ ) );
		//		RF64 rfResult = sqrt(static_cast<RF64>((x * src.x + y * src.y + z * src.z)));
		uiResult = static_cast<UI32>( floor( rfResult ) ); // truncate
	}
	return uiResult;
}

UI32 Coord::distance( const Coord& a, const Coord& b )
{
	return a.distance( b );
}

// Operators
Coord& Coord::operator=( const Coord& clCoord )
{
	x = clCoord.x;
	y = clCoord.y;
	z = clCoord.z;
	map = clCoord.map;
	return ( *this );
}

bool Coord::operator==( const Coord& src ) const
{
	return ( x == src.x && y == src.y && z == src.z && map == src.map );
}

bool Coord::operator!=( const Coord& src ) const
{
	return !( x == src.x && y == src.y && z == src.z && map == src.map );
}

Coord Coord::operator+( const Coord& src ) const
{
	return Coord( this->x + src.x, this->y + src.y, this->z + src.z, this->map );
}

Coord Coord::operator-( const Coord& src ) const
{
	return Coord( this->x - src.x, this->y - src.y, this->z - src.z, this->map );
}

void Coord::effect( Q_UINT16 id, Q_UINT8 speed, Q_UINT8 duration, Q_UINT16 hue, Q_UINT16 renderMode ) const
{
	cUOTxEffect effect;
	effect.setType( ET_STAYSOURCEPOS );
	effect.setSourcePos( ( *this ) );
	effect.setId( id );
	effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket* mSock = 0;
	for ( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
	{
		if ( mSock->player() && ( mSock->player()->pos().distance( ( *this ) ) <= mSock->player()->visualRange() ) )
			mSock->send( &effect );
	}
}

// Calculates the direction from one location to another
unsigned int Coord::direction( const Coord& dest ) const
{
	unsigned int dir;
	short xdif, ydif;

	xdif = dest.x - x;
	ydif = dest.y - y;

	if ( ( xdif == 0 ) && ( ydif < 0 ) )
		dir = 0;
	else if ( ( xdif > 0 ) && ( ydif < 0 ) )
		dir = 1;
	else if ( ( xdif > 0 ) && ( ydif == 0 ) )
		dir = 2;
	else if ( ( xdif > 0 ) && ( ydif > 0 ) )
		dir = 3;
	else if ( ( xdif == 0 ) && ( ydif > 0 ) )
		dir = 4;
	else if ( ( xdif < 0 ) && ( ydif > 0 ) )
		dir = 5;
	else if ( ( xdif < 0 ) && ( ydif == 0 ) )
		dir = 6;
	else if ( ( xdif < 0 ) && ( ydif < 0 ) )
		dir = 7;
	else
		dir = 8;

	return dir;
}

Coord Coord::losCharPoint(bool eye) const {
	Coord result = *this;
	result.z += (eye ? 15 : 10);
	return result;
}

Coord Coord::losItemPoint(unsigned short id) const {
	Coord result = *this;

	tile_st tile = TileCache::instance()->getTile(id);
	result.z += tile.height / 2 + 1;

	return result;
}

Coord Coord::losMapPoint() const {
	Coord result = *this;

    int bottom, top;
	unsigned short id;
	Maps::instance()->mapTileSpan(result, id, bottom, top);

	// Use the top
	if (result.z >= bottom && result.z <= top) {		
		result.z = top;
	}

	return result;
}

Coord Coord::losTargetPoint(cUORxTarget *target, unsigned char map) {
	SERIAL serial = target->serial();
	P_ITEM pItem = World::instance()->findItem(serial);
	P_CHAR pChar = World::instance()->findChar(serial);

	if (pItem) {
		pItem = pItem->getOutmostItem();

		if (pItem->container() && pItem->container()->isChar()) {
			return pItem->container()->pos().losCharPoint();
		} else {
			return pItem->pos().losItemPoint(pItem->id());
		}
	} else if (pChar) {
		return pChar->pos().losCharPoint();
	} else {
		return Coord(target->x(), target->y(), target->z(), map);
	}
}
