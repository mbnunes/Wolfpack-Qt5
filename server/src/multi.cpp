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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "multi.h"
#include "definitions.h"
#include "territories.h"
#include "muls/multiscache.h"
#include "muls/maps.h"
#include "muls/tilecache.h"
#include "world.h"
#include "log.h"
#include "console.h"
#include "basechar.h"
#include "mapobjects.h"
#include "timers.h"

void cMulti::remove()
{
	cUObject* object;
	for ( object = objects.first(); object; object = objects.next() )
	{
		object->setMulti( 0 );
	}
	objects.clear();

	cItem::remove();
}

cMulti::cMulti()
{
	serial_ = INVALID_SERIAL;
}

cMulti::~cMulti()
{
}

PyObject* cMulti::getPyObject()
{
	return cItem::getPyObject();
}

void cMulti::buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions )
{
	cItem::buildSqlString( objectid, fields, tables, conditions );
}

const char* cMulti::className() const
{
	return "multi";
}

stError* cMulti::setProperty( const QString& name, const cVariant& value )
{
	return cItem::setProperty( name, value );
}

PyObject* cMulti::getProperty( const QString& name )
{
	return cItem::getProperty( name );
}

void cMulti::moveTo( const Coord& pos )
{
	cUObject::moveTo( pos );
}

cMulti* cMulti::createFromScript( const QString& id )
{
	cMulti* multi = 0;

	const cElement* section = Definitions::instance()->getDefinition( WPDT_MULTI, id );

	if ( section )
	{
		multi = new cMulti;
		multi->setSerial( World::instance()->findItemSerial() );
		multi->setBaseid( id.latin1() );
		multi->applyDefinition( section );
		cDelayedOnCreateCall* onCreateCall = new cDelayedOnCreateCall( multi, id );
		Timers::instance()->insert( onCreateCall );
	}
	else
	{
		Console::instance()->log( LOG_ERROR, tr( "Unable to create unscripted item: %1\n" ).arg( id ) );
	}

	return multi;
}

void cMulti::addObject( cUObject* object )
{
	objects.remove( object );
	objects.append( object );
}

void cMulti::removeObject( cUObject* object )
{
	objects.remove( object );
}

bool cMulti::inMulti( const Coord& pos )
{
	// Seek tiles with same x,y as pos
	// Seek for tile which z value <= pos.z + 5 && z value >= pos.z - 5
	MultiDefinition* multi = MultiCache::instance()->getMulti( id_ - 0x4000 );

	if ( !multi )
	{
		return false;
	}

	QValueVector<multiItem_st> items = multi->getEntries();
	QValueVector<multiItem_st>::iterator it;
	for ( it = items.begin(); it != items.end(); ++it )
	{
		if ( !it->visible )
		{
			continue;
		}

		if ( pos_.x + it->x != pos.x || pos_.y + it->y != pos.y )
		{
			continue;
		}

		if ( pos_.z + it->z >= pos.z - 5 && pos_.z + it->z <= pos.z + 5 )
		{
			return true;
		}
	}

	return false;
}

static FactoryRegistration<cMulti> registration( "cMulti" );

unsigned char cMulti::classid;

cMulti* cMulti::find( const Coord& pos )
{
	// Multi Range = BUILDRANGE
	MapMultisIterator multis = MapObjects::instance()->listMultisInCircle( pos, BUILDRANGE );

	cMulti *multi;
	for ( multi = multis.first(); multi; multi = multis.next() )
	{
		if ( multi && multi->inMulti( pos ) )
		{
			break;
		}
	}

	return multi;
}

void cMulti::save( cBufferedWriter& writer )
{
	cItem::save( writer );

	// Save objects within this multi *after* the multi
	cUObject* object;
	for ( object = objects.first(); object; object = objects.next() )
	{
		object->save( writer );
	}
}

bool cMulti::canPlace( const Coord& pos, unsigned short multiid, QPtrList<cUObject>& moveOut, unsigned short yard )
{
	MultiDefinition *multi = MultiCache::instance()->getMulti( multiid );

	if ( !multi )
	{
		return false;
	}

	moveOut.setAutoDelete( false );

	// Get the boundaries and build a list sorted by x,y
	int left = multi->getLeft();
	int right = multi->getRight();
	int bottom = multi->getBottom();
	int top = multi->getTop();
	int height = multi->getHeight();
	int width = multi->getWidth();

	QValueList<Coord> borderList; // a list of points around the foundation that need to be clear of impassables
	QValueList<Coord> yardList; // a list of points in the yard (front/back of the house that needs to be clear)

	for ( int x = 0; x < width; ++x )
	{
		for ( int y = 0; y < height; ++y )
		{
			Coord point = pos + Coord( x + left, y + top );
			bool hasBase = false; // Has this multi tile a base below the floor?

			// See if there are any tiles at that position
			const QValueVector<multiItem_st> &multiItems = multi->itemsAt( x + left, y + top );

			if ( multiItems.size() == 0 )
			{
				continue; // Skip this tile since there are no items here
			}

			cTerritory *region = Territories::instance()->region( point );
			if ( region && region->isNoHousing() )
			{
				return false; // No housing is allowed in this region
			}

			// Collect data for the intersect checks
			StaticsIterator statics = Maps::instance()->staticsIterator( point );
			MapItemsIterator items = MapObjects::instance()->listItemsAtCoord( point );
			int top, bottom;
			unsigned short landId;
			Maps::instance()->mapTileSpan( point, landId, bottom, top );

			// Check every tile of the multi at the current position
			// The following algorithm is more or less a ripoff of RunUOs idea.
			for ( unsigned int i = 0; i < multiItems.size(); ++i )
			{
				multiItem_st multiItem = multiItems[i];
				tile_st tile = TileCache::instance()->getTile( multiItem.tile );

				// Calculcate the spawn of the tile
				int itemBottom = point.z + multiItem.z;
				// if the tile is a surface, someone has to be able to stand on it.
				int itemTop = itemBottom + tile.height + ( ( ( tile.flag2 & 0x02 ) != 0 ) ? 16 : 0 );

				// There is special handling for floor tiles
				bool baseTile = multiItem.z == 0 && ( tile.flag1 & 0x10 ) != 0;
				bool isHovering = true; // This tile has not yet something to "stand" on

				if ( baseTile )
					hasBase = true;

				// Does the multi item intersect a land-tile?
				if ( ( itemTop < top && itemTop >= bottom ) || ( itemBottom < top && itemBottom >= bottom ) )
				{
					return false;
				}

				// Since houses can only be built on land and not on items, it's enough to check for
				// a solid foundation here
				if ( pos.z != bottom || pos.z != top )
				{
					return false;
				}

				// Check if the multi item is interfering with a static tile at the same position
				statics.reset();
				while ( !statics.atEnd() )
				{
					const staticrecord &staticTile = ( statics++ ).data();
					tile_st staticInfo = TileCache::instance()->getTile( staticTile.itemid );

					int staticBottom = staticTile.zoff;
					int staticTop = staticBottom + staticInfo.height;

					// The tile intersects a static tile
					if ( ( itemTop < staticTop && itemTop >= staticBottom ) || ( itemBottom < staticTop && itemBottom >= staticBottom ) )
					{
						bool impassable = ( staticInfo.flag1 & 0x40 ) != 0;
						bool background = ( staticInfo.flag1 & 0x01 ) != 0;
						bool surface = ( staticInfo.flag2 & 0x02 ) != 0;

						// A normally blocking tile is intersecting our multi
						if ( impassable || ( !background && surface ) )
						{
							return false;
						}
					}
				}

				// Do the same check (as above) with movable items, but make sure that movable items
				// are moved out of the house
				for ( P_ITEM pItem = items.first(); pItem; pItem = items.next() )
				{
					tile_st itemInfo = TileCache::instance()->getTile( pItem->id() );

					int dynamicBottom = pItem->pos().z;
					int dynamicTop = dynamicBottom + itemInfo.height;

					// Only handle the tile if it is intersecting the multi
					if ( ( itemTop < dynamicTop && itemTop >= dynamicBottom ) || ( itemBottom < dynamicTop && itemBottom >= dynamicBottom ) )
					{
						// Move the item out of the multi space if possible
						if ( ( pItem->movable() == 0 && itemInfo.weight != 255 ) || pItem->movable() == 1 )
						{
							moveOut.append( pItem );
						}
						else
						{
							bool impassable = ( itemInfo.flag1 & 0x40 ) != 0;
							bool background = ( itemInfo.flag1 & 0x01 ) != 0;
							bool surface = ( itemInfo.flag2 & 0x02 ) != 0;

							// A normally blocking tile is intersecting our multi
							if ( impassable || ( !background && surface ) )
							{
								return false;
							}
						}
					}
				}

				// Moves mobiles inside the multi out to the ban location
				MapCharsIterator chars = MapObjects::instance()->listCharsAtCoord( point );
				for ( P_CHAR pChar = chars.first(); pChar; pChar = chars.next() )
				{
					// Move them ALWAYS out, they could be trapped by the castle
					// otherwise (or other strange multi forms)
					moveOut.append( pChar );
				}

				// To keep roads house free, here's a specialized check for roads
				if ( ( landId >= 0x71 && landId <= 0x8c ) || ( landId >= 0x14c && landId <= 0x14f ) || ( landId >= 0x161 && landId <= 0x174 ) || ( landId >= 0x1f0 && landId <= 0x1f3 ) || ( landId >= 0x26e && landId <= 0x279 ) || ( landId >= 0x27e && landId <= 0x281 ) || ( landId >= 0x324 && landId <= 0x3ac ) || ( landId >= 0x597 && landId <= 0x5a6 ) || ( landId >= 0x637 && landId <= 0x63a ) || ( landId >= 0x67d && landId <= 0x6a0 ) || ( landId >= 0x7ae && landId <= 0x7b1 ) || ( landId >= 0x442 && landId <= 0x479 ) || ( landId >= 0x501 && landId <= 0x510 ) || ( landId >= 0x009 && landId <= 0x015 ) || ( landId >= 0x150 && landId <= 0x15c ) )
				{
					return false; // Road Blocked
				}

				// For houses (they have a base you know...)
				// we collect another list of points around the house that need to be checked
				if ( hasBase )
				{
					int xOffset, yOffset;

					// We have to do two loops since the yard size does play a role here
					// but not for the border
					for ( xOffset = -1; xOffset <= 1; ++xOffset )
					{
						for ( yOffset = -yard; yOffset <= yard; ++yOffset )
						{
							Coord pos = point + Coord( xOffset, yOffset );

							if ( !yardList.contains( pos ) )
							{
								yardList.push_back( pos ); // Put this point into the yard checklist if it's not there
							}
						}
					}

					for ( xOffset = -1; xOffset <= 1; ++xOffset )
					{
						for ( yOffset = -1; yOffset <= 1; ++yOffset )
						{
							Coord pos = point + Coord( xOffset, yOffset );

							// Only do the following if the current tiles position differs from the
							// check position.
							if ( xOffset != 0 || yOffset != 0 )
							{
								// The border list should not contain tiles that are actually below the multis
								// floor and hence not visible and covered by a walkable multi tile.
								// So what we do here is check if within 8 z units of the multis floor there is a
								// walkable tile above the border tile
								int multiX = x + xOffset; // Offset to the upper left corner of the multi
								int multiY = y + yOffset; // Offset to the upper left corner of the multi
								bool found = false; // Assume there is no such tile

								// Only do this check if the to-be-checked tile is really within the multi
								// boundaries
								if ( multiX >= 0 && multiY >= 0 && multiX < width && multiY < height )
								{
									// Get the multi tiles at the to-check position
									const QValueVector<multiItem_st> &tiles = multi->itemsAt( multiX + left, multiY + right );
									QValueVector<multiItem_st>::const_iterator it;
									for ( it = tiles.begin(); it != tiles.end(); ++it )
									{
										if ( it->z > 8 )
										{
											continue; // Skip the tile if its above the base (2nd floor etc.)
										}

										// Get the tiledata info for this tile if it's below z8
										tile_st tileInfo = TileCache::instance()->getTile( it->tile );
										bool surface = ( tileInfo.flag2 & 0x02 ) != 0;

										if ( tileInfo.height == 0 && surface )
										{
											found = true;
											break;
										}
									}

									// We found a tile that we could stand on. So it's not neccesary to check
									// that the house stands on something walkable here.
									if ( found )
									{
										continue;
									}
								}

								// Add the tile to the list of border tiles.
								if ( !borderList.contains( pos ) )
								{
									borderList.append( pos );
								}
							}
						}
					}
				}
			}
		}
	}

	QValueList<Coord>::const_iterator it;

	// Now check all the accumulated border tiles
	for ( it = borderList.begin(); it != borderList.end(); ++it )
	{
		map_st mapTile = Maps::instance()->seekMap( *it );
		land_st mapTileInfo = TileCache::instance()->getLand( mapTile.id );

		// Impassable map tiles are not allowed nearby
		bool impassable = ( mapTileInfo.flag1 & 0x40 ) != 0;
		if ( impassable )
		{
			return false;
		}

		// Get Static Tiles
		StaticsIterator statics = Maps::instance()->staticsIterator( *it );
		while ( !statics.atEnd() )
		{
			const staticrecord &staticItem = statics.data();
			tile_st staticInfo = TileCache::instance()->getTile( staticItem.itemid );

			bool impassable = ( staticInfo.flag1 & 0x40 ) != 0;
			bool background = ( staticInfo.flag1 & 0x01 ) != 0;
			bool surface = ( staticInfo.flag2 & 0x02 ) != 0;

			// The tile is only of importance if it's not below the multi
			if ( ( staticItem.zoff > ( ( *it ).z + 2 ) ) && ( impassable || ( !background && surface ) ) )
			{
				return false; // A normally blocking tile is intersecting our multi border
			}

			statics++;
		}

		// Do the same check (as above) with dynamic items
		MapItemsIterator items = MapObjects::instance()->listItemsAtCoord( *it );
		for ( P_ITEM pItem = items.first(); pItem; pItem = items.next() )
		{
			tile_st itemInfo = TileCache::instance()->getTile( pItem->id() );

			// Move the item out of the multi space if possible
			if ( ( pItem->movable() == 0 && itemInfo.weight != 255 ) || pItem->movable() == 1 )
				continue;

			if ( pItem->pos().z <= ( ( *it ).z + 2 ) )
				continue; // Does not interfere with the border

			bool impassable = ( itemInfo.flag1 & 0x40 ) != 0;
			bool background = ( itemInfo.flag1 & 0x01 ) != 0;
			bool surface = ( itemInfo.flag2 & 0x02 ) != 0;

			// A normally blocking tile is intersecting our multi
			if ( impassable || ( !background && surface ) )
				return false;
		}
	}

	// The yard has to be free of any multis at that position
	for ( it = yardList.begin(); it != yardList.end(); ++it )
	{
		// Search for multis in the region
		MapMultisIterator multis = MapObjects::instance()->listMultisInCircle( *it, 18 );
		for ( cMulti*multi = multis.first(); multi; multi = multis.next() )
		{
			if ( multi->inMulti( *it ) )
			{
				// This is a simplified check but it should be sufficient.
				return false;
			}
		}
	}

	return true;
}
