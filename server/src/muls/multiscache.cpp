/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#include "multiscache.h"
#include "uopindex.h"
#include "../defines.h"
#include "../exceptions.h"
#include "../serverconfig.h"
#include "../console.h"
#include "../log.h"

// Library Includes
#include <QFile>
#include <QDataStream>
#include <QDir>

#include <algorithm>

MultiDefinition::MultiDefinition() : width( 0 ), height( 0 ), left( 0 ), top( 0 ), right( 0 ), bottom( 0 )
{
}

void MultiDefinition::setItems( const QList<multiItem_st>& items )
{
	// try to sort
	if ( items.empty() )
		return;
	int i = 0;
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
	this->width = abs( right - left ) + 1;
	this->height = abs( bottom - top ) + 1;

	// copy into grid
	grid.resize( width * height );
	for ( i = 0; i < items.size(); ++i )
	{
		int index = ( items[i].y - top ) * width + ( items[i].x - left );
		if ( index < grid.size() )
		{
			grid[index].append( items[i] );
		}
	}

	entries = items;
}

const QList<multiItem_st>& MultiDefinition::itemsAt( int x, int y )
{
	int index = ( x - left ) + ( y - top ) * width;
	static QList<multiItem_st> emptyGrid;
	if ( index >= grid.size() )
	{
		return emptyGrid;
	}
	else
	{
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
		for ( int i = 0; i < entries.size(); ++i )
			if ( entries[i].x == x && entries[i].y == y )
				return true;
	return false;
}

signed char MultiDefinition::multiHeight( short x, short y, short z ) const
{
	for ( int j = 0; j < entries.size(); ++j )
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
QList<multiItem_st> MultiDefinition::getEntries() const
{
	return entries;
}

/*****************************************************************************
  MultiCache member functions
 *****************************************************************************/

/*!
	\class cMultiCache multiscache.h

	\brief cMultiCache is responsible for loading and parsing multi definitions
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
		delete multis.begin().value();
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
	QFile multiFile( basePath + "multi.mul" );

	if ( indexFile.open( QIODevice::ReadOnly ) && multiFile.open( QIODevice::ReadOnly ) )
	{
		QByteArray indexBytes = indexFile.readAll();
		QByteArray multiBytes = multiFile.readAll();
		indexFile.close();
		multiFile.close();

		int totalRecords = indexBytes.size() / 12;
		const char* idxPtr = indexBytes.constData();
		const char* mulPtr = multiBytes.constData();
		int mulSize = multiBytes.size();

		for ( int currentID = 0; currentID < totalRecords; ++currentID )
		{
			int offset = currentID * 12;
			qint32 start = *reinterpret_cast<const qint32*>( idxPtr + offset );
			qint32 length = *reinterpret_cast<const qint32*>( idxPtr + offset + 4 );

			if ( start <= 0 || length <= 0 || ( start + length ) > mulSize )
			{
				continue;
			}

			bool is16Byte = ( length % 16 == 0 );
			int recordSize = is16Byte ? 16 : 12;
			int count = length / recordSize;

			QList<multiItem_st> items;
			items.reserve( count );

			const char* recPtr = mulPtr + start;
			for ( int i = 0; i < count; ++i )
			{
				const char* itemPtr = recPtr + ( i * recordSize );
				qint16 tileVal = *reinterpret_cast<const qint16*>( itemPtr );
				qint16 xVal = *reinterpret_cast<const qint16*>( itemPtr + 2 );
				qint16 yVal = *reinterpret_cast<const qint16*>( itemPtr + 4 );
				qint16 zVal = *reinterpret_cast<const qint16*>( itemPtr + 6 );

				multiItem_st item;
				item.tile = static_cast<quint16>( tileVal );
				item.x = xVal;
				item.y = yVal;
				item.z = static_cast<qint8>( zVal );

				if ( is16Byte )
				{
					quint16 flags = *reinterpret_cast<const quint16*>( itemPtr + 8 );
					item.visible = ( flags != 0 || tileVal != 1 );
				}
				else
				{
					quint32 isVisible = *reinterpret_cast<const quint32*>( itemPtr + 8 );
					item.visible = isVisible > 0;
				}

				if ( item.visible && item.tile != 1 )
				{
					items.push_back( item );
				}
			}

			MultiDefinition* multi = new MultiDefinition;
			multi->setItems( items );
			multis.insert( static_cast<ushort>( currentID ), multi );
		}
		Console::instance()->log( LOG_NOTICE, tr( "Loaded %1 multis from multi.idx/multi.mul.\n" ).arg( multis.size() ) );
	}
	else
	{
		// Fallback to MultiCollection.uop
		QDir baseFolder( basePath );
		QStringList files = baseFolder.entryList();
		QString uopFileName;
		for ( QStringList::const_iterator it = files.begin(); it != files.end(); ++it )
		{
			if ( ( *it ).toLower() == "multicollection.uop" )
			{
				uopFileName = *it;
				break;
			}
		}

		if ( uopFileName.isEmpty() )
		{
			throw wpException( QString( "Error opening multi.idx / multi.mul or MultiCollection.uop in %1." ).arg( basePath ) );
		}

		loadUOP( basePath + uopFileName );
	}

	cComponent::load();
}

void cMultiCache::loadUOP( const QString& uopPath )
{
	QFile file( uopPath );
	if ( !file.open( QIODevice::ReadOnly ) )
		throw wpException( QString( "Error opening file %1 for reading." ).arg( uopPath ) );

	QDataStream ds( &file );
	ds.setByteOrder( QDataStream::LittleEndian );

	quint32 magic = 0, version = 0, signature = 0;
	quint64 nextTable = 0;
	ds >> magic >> version >> signature >> nextTable;

	if ( magic != 0x0050594D )
		throw wpException( QString( "Invalid UOP file format: %1" ).arg( uopPath ) );

	// Precalculate hash lookup table for build/multicollection/{id:06d}.bin
	QMap<quint64, ushort> hashToId;
	for ( int i = 0; i < 0x10000; ++i )
	{
		QString name = QString( "build/multicollection/%1.bin" ).arg( i, 6, 10, QLatin1Char( '0' ) );
		QByteArray ascii = name.toLatin1();
		quint64 h = cUOPIndex::hashLittle2( ascii.constData(), ascii.size() );
		hashToId.insert( h, static_cast<ushort>( i ) );
	}

	while ( nextTable != 0 && nextTable < static_cast<quint64>( file.size() ) )
	{
		if ( !file.seek( nextTable ) )
			break;

		quint32 filesCount = 0;
		quint64 nextBlock = 0;
		ds >> filesCount >> nextBlock;

		for ( quint32 i = 0; i < filesCount; ++i )
		{
			quint64 offset = 0;
			quint32 headerLength = 0, compressedLength = 0, decompressedLength = 0;
			quint64 fileHash = 0;
			quint32 dataHash = 0;
			quint16 flag = 0;

			ds >> offset >> headerLength >> compressedLength >> decompressedLength
			   >> fileHash >> dataHash >> flag;

			if ( offset == 0 || decompressedLength == 0 || !hashToId.contains( fileHash ) )
				continue;

			ushort multiId = hashToId.value( fileHash );
			qint64 savedPos = file.pos();

			QByteArray data = cUOPIndex::extractEntry( &file, offset + headerLength, compressedLength, decompressedLength, flag );
			file.seek( savedPos );

			if ( data.size() < 8 )
				continue;

			QDataStream dataStream( data );
			dataStream.setByteOrder( QDataStream::LittleEndian );

			quint32 unknown = 0, count = 0;
			dataStream >> unknown >> count;

			QList<multiItem_st> items;
			for ( quint32 j = 0; j < count; ++j )
			{
				multiItem_st item;
				quint16 tile = 0;
				qint16 x = 0, y = 0, z = 0;
				quint16 flagg = 0;
				quint32 clilocsCount = 0;

				dataStream >> tile >> x >> y >> z >> flagg >> clilocsCount;

				item.tile = tile;
				item.x = x;
				item.y = y;
				item.z = static_cast<qint8>( z );
				item.visible = true;

				for ( quint32 c = 0; c < clilocsCount; ++c )
				{
					quint32 cliloc = 0;
					dataStream >> cliloc;
				}

				items.push_back( item );
			}

			MultiDefinition* multi = new MultiDefinition;
			multi->setItems( items );
			multis.insert( multiId, multi );
		}

		nextTable = nextBlock;
	}

	Console::instance()->log( LOG_NOTICE, tr( "Loaded %1 multis from MultiCollection.uop.\n" ).arg( multis.size() ) );
}

/*!
	Retrieves the multi's definition of a given ID.
*/
MultiDefinition* cMultiCache::getMulti( ushort id )
{
	QMap<ushort, MultiDefinition*>::const_iterator it = multis.find( id );
	if ( it != multis.end() )
		return it.value();
	else
		return 0;
}


