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


#include "maps.h"
#include "tilecache.h"
#include "sectors.h"
#include "multiscache.h"
#include "defines.h"
#include "items.h"

// Library Includes
#include <qstring.h>
#include <qintcache.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qdir.h>

// System includes
#include <algorithm>
#include <exception>
#include <assert.h>
#include <math.h>

class MapsPrivate
{
public:
	uint width;
	uint height;
	QIntCache<map_st> mapCache;
	QIntCache<QValueVector<staticrecord> > staticsCache;

	QFile mapfile;
	QFile idxfile;
	QFile staticsfile;
	
	MapsPrivate( const QString& index, const QString& map, const QString& statics) throw(wpFileNotFoundException);
	map_st seekMap( ushort x, ushort y );
};

/*****************************************************************************
  MapsPrivate member functions
 *****************************************************************************/

MapsPrivate::MapsPrivate( const QString& index, const QString& map, const QString& statics ) throw(wpFileNotFoundException)
{
	idxfile.setName( index );
	if ( !idxfile.open( IO_ReadOnly ) )
		throw wpFileNotFoundException( QString("Couldn't open file %1").arg( index ) );

	mapfile.setName( map );
	if ( !mapfile.open( IO_ReadOnly ) )
		throw wpFileNotFoundException( QString("Couldn't open file %1").arg( map ) );
	
	staticsfile.setName( statics );
	if ( !staticsfile.open( IO_ReadOnly ) )
		throw wpFileNotFoundException( QString("Couldn't open file %1").arg( statics ) );
	staticsCache.setAutoDelete( true );
	mapCache.setAutoDelete( true );
}

map_st MapsPrivate::seekMap( ushort x, ushort y )
{
	const ushort x1 = x / 8, y1 = y / 8, x2 = x % 8, y2 = y % 8;
	const uint record = ( x1 * height * 196 ) + ( y1 * 196 ) + ( y2 * 24 ) + ( x2 * 3 ) + 4;

	map_st* result = mapCache.find( record );
	bool shouldDelete = false;
	if ( !result )
	{
		result = new map_st;
		mapfile.at( record );
		QDataStream stream( &mapfile );
		stream.setByteOrder( QDataStream::LittleEndian );
		stream >> (*result).id;
		stream >> (*result).z;
		shouldDelete = !mapCache.insert( record, result );
	}

	if ( shouldDelete )
	{
		map_st dummy = *result;
		delete result;
		return dummy;
	}
	else
		return *result;
}

/*****************************************************************************
  Maps member functions
 *****************************************************************************/

/*!
  \class Maps maps.h

  \brief The Maps class is responsable for handling Map queries

  \ingroup UO File Handlers
  \ingroup mainclass
  \sa cTileCache
*/

/*!
	Constructs a Maps class, and uses \a path as it's base
	path to search for the map files registered thru registerMap()
	\sa registerMap
*/
Maps::Maps( const QString& path ) : basePath( path )
{
}

/*!
	Destroy the Maps instance and frees allocated memory
*/
Maps::~Maps()
{
	for ( iterator it = d.begin(); it != d.end(); ++it )
		delete it.data();
}

/*!
	Registers a map id and corresponding file to be accessible to Wolfpack
	It will try it best to match the filenames in a case insensitive way,
	since it's case sometimes varies and it might became an anoyance configuring the
	server under Linux.
*/
bool Maps::registerMap( uint id, const QString& mapfile, uint mapwidth, uint mapheight, const QString& staticsfile, const QString& staticsidx )
{
	try {
		QDir baseFolder( basePath );
		QStringList files = baseFolder.entryList();
		QString staticsIdxName, mapFileName, staticsFileName;
		for ( QStringList::const_iterator it = files.begin(); it != files.end(); ++it )
		{
			if ( (*it).lower() == staticsidx.lower() )
				staticsIdxName = *it;
			if ( (*it).lower() == mapfile.lower() )
				mapFileName = *it;
			if ( (*it).lower() == staticsfile.lower() )
				staticsFileName = *it;
		}

		MapsPrivate* p = new MapsPrivate( basePath + staticsIdxName, basePath + mapFileName, basePath + staticsFileName );
		p->height = mapheight;
		p->width  = mapwidth;
		d.insert( id, p );
		return true;
	} catch ( wpFileNotFoundException& e ) {
		qWarning( e.error() );
		return false;
	}
}

/*!
	Returns true if the \a id map is present, false otherwise
*/
bool Maps::hasMap( uint id ) const
{
	return d.contains( id );
}

/*!
	Seeks for a map record (map_st) in the given map \a id, at the given \a x, \a y 
	coordinates.
	\sa map_st
*/
map_st Maps::seekMap( uint id, ushort x, ushort y ) const
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		return map_st();
	return it.data()->seekMap( x, y );
}

/*!
	\overload
	Overloaded method, like the above, but takes coordinates and map id out of
	a Coord_cl instance.
	\sa Coord_cl
	\sa map_st
*/
map_st Maps::seekMap( const Coord_cl& p ) const
{
	return seekMap( p.map, p.x, p.y );
}

/*!
	Returns the elevation (z) of map tile located at \a p.
*/
signed char Maps::mapElevation( const Coord_cl& p ) const
{
	map_st map = seekMap( p );
	// make sure nothing can move into black areas
	if (430 == map.id || 475 == map.id || 580 == map.id || 610 == map.id ||
		611 == map.id || 612 == map.id || 613 == map.id)
		return ILLEGAL_Z;
	/*! 
	\internal
	\note maybe the above if could be better checked thru flags. Or even better yet,
	simply return map.z and leave the responsability of checking walkability to the calee
	*/
	return map.z;
}

/*!
	Returns the height ( max. y value ) of \a id map
*/
uint Maps::mapTileHeight( uint id ) const
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		return 0;
	return it.data()->height;
}

/*!
	Returns the width ( max. x value ) of \a id map
*/
uint Maps::mapTileWidth( uint id ) const
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		return 0;
	return it.data()->width;
}

/*!
	Returns the average elevation (z) of coordinates \a p and it's
	neighbor tiles.
	This method does not take into account dynamic objects that might
	be placed in those coordinates, instead it only looks at the map file.
	The optional parameters \a top and \a botton are respectively the highest
	and lowerst values that composes the average
*/
signed char Maps::mapAverageElevation( const Coord_cl& p, int* top /* = 0 */, int* botton /* = 0 */  ) const
{
	// first thing is to get the map where we are standing
	map_st map1 = seekMap( p );
	//id = map1.id;
	// if this appears to be a valid land id, <= 2 is invalid
	if (map1.id > 2 && ILLEGAL_Z != mapElevation(p))
	{
		// get three other nearby titles to decide on an average z?
		INT8 map2z = mapElevation( p + Coord_cl( 1, 0, 0 ) );
		INT8 map3z = mapElevation( p + Coord_cl( 0, 1, 0 ) );
		INT8 map4z = mapElevation( p + Coord_cl( 1, 1, 0 ) );
		
		INT8 testz = 0;
		if (abs(map1.z - map4z) <= abs(map2z - map3z))
		{
			if (ILLEGAL_Z == map4z)
				testz = map1.z;
			else // round down.
				testz = (signed char)( floor( ( map1.z + map4z ) / 2.0 ) );
		} 
		else 
		{
			if (ILLEGAL_Z == map2z || ILLEGAL_Z == map3z)
				testz = map1.z;
			else // round down
				testz = (signed char)( floor( ( map2z + map3z ) / 2.0 ) );
		}
		if ( top )
		{
			*top = map1.z;
			if ( map2z > *top )
				*top = map2z;
			if ( map3z > *top )
				*top = map3z;
			if ( map4z > *top )
				*top = map4z;
		}
		if ( botton )
		{
			*botton = map1.z;
			if ( map2z < *botton )
				*botton = map2z;
			if ( map3z < *botton )
				*botton = map3z;
			if ( map4z < *botton )
				*botton = map4z;
		}
		return testz;
	}
	return ILLEGAL_Z;
}

bool Maps::canFit( int x, int y, int z, int map, int height ) const
{
	if ( x < 0 || y < 0 || x >= mapTileWidth(map) * 8 || y >= mapTileHeight(map) * 8 )
		return false;

	map_st map1 = seekMap( map, x, y );
	land_st land = TileCache::instance()->getLand( map1.id );
	if ( land.isBlocking() )
		return false; // There is something here.
	
	// check statics too
	StaticsIterator StaticTiles = staticsIterator( Coord_cl( x, y, z, map ) );
	for ( ; !StaticTiles.atEnd(); ++StaticTiles )
	{
		tile_st tile = TileCache::instance()->getTile(StaticTiles->itemid);
		if ( ( tile.isBlocking() ) && StaticTiles->zoff + tile.height > z )
			return false;
	}

	return true;
}

signed char Maps::dynamicElevation(const Coord_cl& pos) const
{
	//int z = ILLEGAL_Z;
	signed char z = ILLEGAL_Z;
	RegionIterator4Items ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if(mapitem->isMulti())
			{
				MultiDefinition* def = MultiCache::instance()->getMulti( mapitem->id() - 0x4000 );
				if ( def )
				{
					z = def->multiHeight( pos.x, pos.y, pos.z );
					z += mapitem->pos().z + 1;
				// this used to do a z++, but that doesn't take INT32o account the fact that
				// the itemp[] the multi was based on has its own elevation
				}
			}
			if ( ( mapitem->pos().x == pos.x ) && ( mapitem->pos().y == pos.y ) && ( !mapitem->isMulti() ) )
			{
				const INT8 ztemp = mapitem->pos().z + TileCache::instance()->tileHeight( mapitem->id() );
				if ((ztemp <= pos.z + MaxZstep) && (ztemp > z))
				{
					z = ztemp;
				}
			}
		}
	}
	return z;
}

signed char Maps::staticTop(const Coord_cl& pos) const
{
	signed char top = ILLEGAL_Z;
	unsigned long loopexit = 0;

	StaticsIterator msi = this->staticsIterator(pos);
	while ( !msi.atEnd() )
	{
		signed char tempTop = msi->zoff + TileCache::instance()->tileHeight(msi->itemid);
		if ( (tempTop <= pos.z + MaxZstep) && (tempTop > top) )
		{
			top = tempTop;
		}
		++msi;
	}
	return top;
}	

// Return new height of player who walked to X/Y but from OLDZ
signed char Maps::height(const Coord_cl& pos)
{
	// let's check in this order.. dynamic, static, then the map
	signed char dynz = dynamicElevation(pos);
	if (ILLEGAL_Z != dynz)
		return dynz;

	signed char staticz = staticTop(pos);
	if (ILLEGAL_Z != staticz)
		return staticz;

	return mapElevation(pos);
}


StaticsIterator Maps::staticsIterator(uint id, ushort x, ushort y, bool exact /* = true */ ) const throw (wpException)
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		throw wpException(QString("[Maps::staticsIterator line %1] map id(%2) not registered!").arg(__LINE__).arg(id) );
	return StaticsIterator( x, y, it.data(), exact );
}

StaticsIterator Maps::staticsIterator( const Coord_cl& p, bool exact /* = true */ ) const throw (wpException)
{
	return staticsIterator( p.map, p.x, p.y, exact );
}

/*****************************************************************************
  StaticsIterator member functions
 *****************************************************************************/

/*!
	\class StaticsIterator maps.h

	\brief The StaticIterator class allows iterating thru the statics file's
	data ( statics?.mul )

	This class is independent of the running machine endianess.

	\ingroup UO File Handlers
	\ingroup mainclass
	\sa cTileCache
	\sa Maps
*/

/*!
	\internal
	Constructs a StaticsIterator class.
	\sa registerMap
*/
StaticsIterator::StaticsIterator( ushort x, ushort y, MapsPrivate* d, bool exact /* = true */ )
{
	baseX = x / 8;
	baseY = y / 8;
	pos = 0;
	
	if ( baseX < d->width && baseY < d->height )
		load(d, x, y, exact);
}

/*!
	\internal
	Loads the data from cache or from file if it's not avaliable in cache.
	This method is independent of the running machine's endianess.
*/
void StaticsIterator::load( MapsPrivate* mapRecord, ushort x, ushort y, bool exact )
{
	const uint indexPos = (baseX * mapRecord->height + baseY ) * 12;
	assert ( indexPos < 0x8000000 ); // dam, breaks our assumption

	UINT32 cachePos;
	if( exact )
		cachePos = ( x * y ) | 0x80000000;
	else
		cachePos = baseX * baseY;

	QValueVector<staticrecord>* p = mapRecord->staticsCache.find( cachePos );
	
	if ( !p )
	{ // Well, unfortunally we will be forced to read the file :(
		struct 
		{
			Q_UINT32 offset;
			Q_UINT32 blocklength;
		} indexStructure;

		mapRecord->idxfile.at(indexPos);
		QDataStream idxStream( &mapRecord->idxfile );
		idxStream.setByteOrder( QDataStream::LittleEndian );
		idxStream >> indexStructure.offset;
		idxStream >> indexStructure.blocklength;

		if ( indexStructure.offset == 0xFFFFFFFF )
			return; // No statics for this block

		mapRecord->staticsfile.at( indexStructure.offset );
		QDataStream staticStream( &mapRecord->staticsfile );
		staticStream.setByteOrder( QDataStream::LittleEndian );

		const uint remainX = x % 8;
		const uint remainY = y % 8;
		for ( Q_UINT32 i = 0; i < indexStructure.blocklength / 7; ++i )
		{
			staticrecord r;
			staticStream >> r.itemid;
			staticStream >> r.xoff;
			staticStream >> r.yoff;
			staticStream >> r.zoff;
			Q_UINT16 unknown;
			staticStream >> unknown;
			if ( exact )
			{
				if ( r.xoff == remainX && r.yoff == remainY )
					staticArray.push_back( r );
			}
			else
				staticArray.push_back( r );
		}
		
		// update cache;
		QValueVector<staticrecord>* temp = new QValueVector<staticrecord>(staticArray);
		if ( !mapRecord->staticsCache.insert( cachePos, temp ) )
			delete temp;
	}
	else
	{
		staticArray = *p;
	}
}

void StaticsIterator::inc()
{
	if ( pos < staticArray.size() )
		++pos;
}

void StaticsIterator::dec()
{
	if ( pos > 0 )
		--pos;
}
