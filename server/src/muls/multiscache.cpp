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

#include "multiscache.h"
#include "../defines.h"
#include "../exceptions.h"
#include "../serverconfig.h"

// Library Includes
#include <qvaluevector.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qdir.h>

#include <algorithm>

MultiDefinition::MultiDefinition() : width( 0 ), height( 0 ), left(0), top(0), right(0), bottom(0)
{
}

void MultiDefinition::setItems( const QValueVector<multiItem_st>& items )
{
	// try to sort
	if ( items.empty() )
		return;
	unsigned int i = 0;
	for ( ; i < items.size(); ++i )
	{
		if ( items[i].x < left )
			left = items[i].x;
		else if ( items[i].x > right )
			right = items[i].x;

		if ( items[i].y < top )
			top = items[i].y;
		else if ( items[i].y > bottom )
			bottom = items[i].y;
	}

	// by now we have the dimensions.
	this->width = abs(right - left) + 1;
	this->height = abs(bottom - top) + 1;

	// copy into grid
	grid.resize(width * height);
	for ( i = 0; i < items.size(); ++i) {
		unsigned int index = (items[i].y - top) * width + (items[i].x - left);
		if (index >= 0 && index < grid.size()) {
			grid[index].append(items[i]);
		}
	}

	entries = items;
}

const QValueVector<multiItem_st> &MultiDefinition::itemsAt(int x, int y) {
	unsigned int index = (x - left) + (y - top)  * width;
	static QValueVector<multiItem_st> emptyGrid;

	if (index < 0 || index >= grid.size()) {
		return emptyGrid;
	} else {
		return grid[index];
	}
}

/*!
	Checks if the given pair of \a x and \a y coordinates
	are inside this Multi. The coordinates should be relative to
	the multi's center (0,0)
*/
bool MultiDefinition::inMulti( short x, short y )
{
	if ( abs( x ) > this->width || abs( y ) > this->height )
		for ( unsigned int i = 0; i < entries.size(); ++i )
			if ( entries[i].x == x && entries[i].y == y )
				return true;
	return false;
}

signed char MultiDefinition::multiHeight( short x, short y, short z ) const
{
	for ( uint j = 0; j < entries.size(); ++j )
	{
		if ( entries[j].visible && ( x + entries[j].x == x ) && ( y + entries[j].y == y ) )
		{
			const int tmpTop = z + entries[j].z;
			if ( tmpTop <= z + MaxZstep && tmpTop >= z - 1 )
			{
				return entries[j].z;
			}
			else if ( tmpTop >= z - MaxZstep && tmpTop < z - 1 )
			{
				return entries[j].z;
			}
		}
	}
	return ILLEGAL_Z;
}

/*!
	Returns this Multi's entries as found in the multis.mul file
*/
QValueVector<multiItem_st> MultiDefinition::getEntries() const
{
	return entries;
}

/*****************************************************************************
  MultiCache member functions
 *****************************************************************************/

/*!
	\class cMultiCache multiscache.h

	\brief cMultiCache is responsable for loading and parsing multi definitions
	stored into "multi.idx" and "multi.mul" files.

	This class is prepared for BigEndian/LittleEndian portability.

	\ingroup UO File Handlers
	\ingroup mainclass
*/


/*!
	Constructs a cMultiCache object
*/
cMultiCache::~cMultiCache()
{
}

void cMultiCache::unload()
{
	// Clear existing definitions
	while ( multis.begin() != multis.end() )
	{
		delete multis.begin().data();
		multis.erase( multis.begin() );
	}

	cComponent::unload();
}

void cMultiCache::reload()
{
	unload();
	load();
}

/*!
	Parses and loads multi definitions
*/
void cMultiCache::load()
{
	QString basePath = Config::instance()->mulPath();

	QFile indexFile( basePath + "multi.idx" );
	if ( !indexFile.open( IO_ReadOnly ) )
		throw wpException( QString( "Error opening file %1 for reading." ).arg( basePath + "multi.idx" ) );

	QDataStream indexStream( &indexFile );
	indexStream.setByteOrder( QDataStream::LittleEndian );

	QFile multiFile( basePath + "multi.mul" );
	if ( !multiFile.open( IO_ReadOnly ) )
		throw wpException( QString( "Error opening file %1 for reading." ).arg( basePath + "multi.mul" ) );

	struct
	{
		Q_INT32 start;
		Q_INT32 length;
		Q_INT32 unknown;
	} indexData;
	ushort currentID = 0;
	while ( !indexStream.atEnd() )
	{
		indexFile.at( currentID * 12 );
		indexStream >> indexData.start;
		indexStream >> indexData.length;
		indexStream >> indexData.unknown;

		if ( indexData.start == -1 ) // empty record?
		{
			++currentID;
			continue;
		}

		QValueVector<multiItem_st> items;
		items.reserve( indexData.length / 12 );

		multiFile.at( indexData.start );
		QDataStream multiStream( &multiFile );
		multiStream.setByteOrder( QDataStream::LittleEndian );

		for ( int i = 0; i < indexData.length / 12; ++i )
		{
			multiItem_st item;
			multiStream >> item.tile;
			multiStream >> item.x;
			multiStream >> item.y;
			multiStream >> item.z;
			Q_UINT8 empty;
			multiStream >> empty; // ????
			Q_UINT32 isVisible = 0;
			multiStream >> isVisible;
			item.visible = isVisible > 0 ? true : false;

			if ( item.visible ) // we ignore invisible items (?)
				items.push_back( item );
		}
		MultiDefinition* multi = new MultiDefinition;
		multi->setItems( items );

		multis.insert( currentID++, multi );
	}

	cComponent::load();
}

/*!
	Retrieves the multi's definition of a given ID.
*/
MultiDefinition* cMultiCache::getMulti( ushort id )
{
	QMap<ushort, MultiDefinition*>::const_iterator it = multis.find( id );
	if ( it != multis.end() )
		return it.data();
	else
		return 0;
}


