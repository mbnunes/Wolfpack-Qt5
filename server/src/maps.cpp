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


#include "maps.h"
#include "classes.h" // due to illegal_z
#include "tilecache.h"
#include "mapobjects.h"
#include "multiscache.h"

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

class MapsPrivate
{
public:
	// About map?.mul
	QFile mapfile;
	uint width;
	uint height;
	QIntCache<map_st> mapCache;
	QIntCache<QValueVector<staticrecord> > staticsCache;

	QFile idxfile;
	QFile staticsfile;
	
	MapsPrivate(QString index, QString map, QString statics);
	map_st seekMap( ushort x, ushort y );
};

/*****************************************************************************
  MapsPrivate member functions
 *****************************************************************************/

MapsPrivate::MapsPrivate( QString index, QString map, QString statics )
{
	idxfile.setName( index );
	if ( !idxfile.open( IO_ReadOnly ) )
		qFatal( QString("Couldn't open file %1").arg( index ) );

	mapfile.setName( map );
	if ( !mapfile.open( IO_ReadOnly ) )
		qFatal( QString("Couldn't open file %1").arg( map ) );
	
	staticsfile.setName( statics );
	if ( !staticsfile.open( IO_ReadOnly ) )
		qFatal( QString("Couldn't open file %1").arg( statics ) );
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


Maps::Maps( const QString& path ) : basePath( path )
{
}

Maps::~Maps()
{
	for ( iterator it = d.begin(); it != d.end(); ++it )
		delete it.data();
}

bool Maps::registerMap( uint id, const QString& mapfile, uint mapwidth, uint mapheight, const QString& staticsfile, const QString& staticsidx )
{
	MapsPrivate* p = new MapsPrivate( basePath + QDir::separator() + staticsidx, basePath + QDir::separator() + mapfile, basePath + QDir::separator() + staticsfile );
	p->height = mapheight;
	p->width  = mapwidth;
	d.insert( id, p );
	return true;
}

map_st Maps::seekMap( uint id, ushort x, ushort y ) const
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		return map_st();
	return it.data()->seekMap( x, y );
}

map_st Maps::seekMap( const Coord_cl& p ) const
{
	return seekMap( p.map, p.x, p.y );
}

signed char Maps::mapElevation( const Coord_cl& p ) const
{
	map_st map = seekMap( p );
	// make sure nothing can move into black areas
	if (430 == map.id || 475 == map.id || 580 == map.id || 610 == map.id ||
		611 == map.id || 612 == map.id || 613 == map.id)
		return illegal_z;
	/*! 
	\internal
	\note maybe the above if could be better checked thru flags. Or even better yet,
	simply return map.z and leave the responsability of checking walkability to the calee
	*/
	return map.z;
}

uint Maps::mapTileHeight( uint id ) const
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		return 0;
	return it.data()->height;
}

uint Maps::mapTileWidth( uint id ) const
{
	const_iterator it = d.find( id );
	if ( it == d.end() )
		return 0;
	return it.data()->width;
}

signed char Maps::mapAverageElevation( const Coord_cl& p ) const
{
	// first thing is to get the map where we are standing
	map_st map1 = seekMap( p );
	//id = map1.id;
	// if this appears to be a valid land id, <= 2 is invalid
	if (map1.id > 2 && illegal_z != mapElevation(p))
	{
		// get three other nearby maps to decide on an average z?
		INT8 map2z = mapElevation( p + Coord_cl( 1, 0, 0 ) );
		INT8 map3z = mapElevation( p + Coord_cl( 0, 1, 0 ) );
		INT8 map4z = mapElevation( p + Coord_cl( 1, 1, 0 ) );
		
		INT8 testz = 0;
		if (abs(map1.z - map4z) <= abs(map2z - map3z))
		{
			if (illegal_z == map4z)
				testz = map1.z;
			else
			{
				testz = (INT8)((map1.z + map4z) >> 1);
				if (testz%2<0) --testz;
				// ^^^ Fix to make it round DOWN, not just in the direction of zero
			}
		} else {
			if (illegal_z == map2z || illegal_z == map3z)
				testz = map1.z;
			else
			{
				testz = (signed char)((map2z + map3z) >> 1);
				if (testz%2<0) --testz;
				// ^^^ Fix to make it round DOWN, not just in the direction of zero
			}
		}
		return testz;
	}
	return illegal_z;
}

signed char Maps::dynamicElevation(const Coord_cl& pos) const
{
	//int z = illegal_z;
	signed char z = illegal_z;
	RegionIterator4Items ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if(mapitem->isMulti())
			{
				MultiDefinition* def = MultisCache->getMulti( mapitem->id() - 0x4000 );
				if ( def )
				{
					z = def->multiHeight( pos.x, pos.y, pos.z );
					z += mapitem->pos.z + 1;
				// this used to do a z++, but that doesn't take INT32o account the fact that
				// the itemp[] the multi was based on has its own elevation
				}
			}
			if ( ( mapitem->pos.x == pos.x ) && ( mapitem->pos.y == pos.y ) && ( !mapitem->isMulti() ) )
			{
				INT8 ztemp = mapitem->pos.z + TileCache::instance()->tileHeight( mapitem->id() );
				if ((ztemp <= pos.z + MaxZstep) && (ztemp > z))
				{
					z = ztemp;
				}
				
			}
		}
	}
	return z;
}

signed char Maps::staticTop(const Coord_cl& pos)
{
	signed char top = illegal_z;
	unsigned long loopexit = 0;

	StaticsIterator msi = this->staticsIterator(pos);
	while ( !msi.atEnd() )
	{
		signed char tempTop = msi->zoff + TileCache::instance()->tileHeight(msi->itemid);
		if ((tempTop <= pos.z + MaxZstep) && (tempTop > top))
		{
			top = tempTop;
		}
	}
	return top;
}	

// Return new height of player who walked to X/Y but from OLDZ
signed char Maps::height(const Coord_cl& pos)
{
	// let's check in this order.. dynamic, static, then the map
	signed char dynz = dynamicElevation(pos);
	if (illegal_z != dynz)
		return dynz;

	signed char staticz = staticTop(pos);
	if (illegal_z != staticz)
		return staticz;

	return mapElevation(pos);
}


StaticsIterator Maps::staticsIterator(uint id, ushort x, ushort y, bool exact /* = true */ )
{
	iterator it = d.find( id );
	if ( it == d.end() )
		throw std::bad_exception();
	return StaticsIterator( x, y, it.data(), exact );
}

StaticsIterator Maps::staticsIterator( const Coord_cl& p, bool exact /* = true */ )
{
	return staticsIterator( p.map, p.x, p.y, exact );
}

/*****************************************************************************
  StaticsIterator member functions
 *****************************************************************************/

StaticsIterator::StaticsIterator( ushort x, ushort y, MapsPrivate* d, bool exact /* = true */ )
{
	baseX = x / 8;
	baseY = y / 8;
	pos = 0;

	load(d, x, y, exact);
}

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
