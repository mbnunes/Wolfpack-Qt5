/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2026 by holders identified in AUTHORS.txt
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

#include "uopindex.h"
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QMap>
#include <zlib.h>

cUOPIndex::cUOPIndex()
	: valid_( false ), totalSize_( 0 )
{
}

cUOPIndex::~cUOPIndex()
{
	close();
}

void cUOPIndex::close()
{
	valid_ = false;
	totalSize_ = 0;
	chunks_.clear();
}

quint64 cUOPIndex::hashLittle2( const QString& s )
{
	QByteArray utf8 = s.toLatin1();
	return hashLittle2( utf8.constData(), utf8.size() );
}

quint64 cUOPIndex::hashLittle2( const char* s, int length )
{
	quint32 a, b, c;
	a = b = c = 0xDEADBEEF + static_cast<quint32>( length );

	int k = 0;

	while ( length > 12 )
	{
		a += static_cast<quint8>( s[k] );
		a += ( static_cast<quint32>( static_cast<quint8>( s[k + 1] ) ) ) << 8;
		a += ( static_cast<quint32>( static_cast<quint8>( s[k + 2] ) ) ) << 16;
		a += ( static_cast<quint32>( static_cast<quint8>( s[k + 3] ) ) ) << 24;
		b += static_cast<quint8>( s[k + 4] );
		b += ( static_cast<quint32>( static_cast<quint8>( s[k + 5] ) ) ) << 8;
		b += ( static_cast<quint32>( static_cast<quint8>( s[k + 6] ) ) ) << 16;
		b += ( static_cast<quint32>( static_cast<quint8>( s[k + 7] ) ) ) << 24;
		c += static_cast<quint8>( s[k + 8] );
		c += ( static_cast<quint32>( static_cast<quint8>( s[k + 9] ) ) ) << 8;
		c += ( static_cast<quint32>( static_cast<quint8>( s[k + 10] ) ) ) << 16;
		c += ( static_cast<quint32>( static_cast<quint8>( s[k + 11] ) ) ) << 24;

		a -= c; a ^= ( ( c << 4 ) | ( c >> 28 ) ); c += b;
		b -= a; b ^= ( ( a << 6 ) | ( a >> 26 ) ); a += c;
		c -= b; c ^= ( ( b << 8 ) | ( b >> 24 ) ); b += a;
		a -= c; a ^= ( ( c << 16 ) | ( c >> 16 ) ); c += b;
		b -= a; b ^= ( ( a << 19 ) | ( a >> 13 ) ); a += c;
		c -= b; c ^= ( ( b << 4 ) | ( b >> 28 ) ); b += a;

		length -= 12;
		k += 12;
	}

	if ( length > 0 )
	{
		switch ( length )
		{
			case 12: c += ( static_cast<quint32>( static_cast<quint8>( s[k + 1] ) ) ) << 24; [[fallthrough]];
			case 11: c += ( static_cast<quint32>( static_cast<quint8>( s[k + 10] ) ) ) << 16; [[fallthrough]];
			case 10: c += ( static_cast<quint32>( static_cast<quint8>( s[k + 9] ) ) ) << 8; [[fallthrough]];
			case 9:  c += static_cast<quint8>( s[k + 8] ); [[fallthrough]];
			case 8:  b += ( static_cast<quint32>( static_cast<quint8>( s[k + 7] ) ) ) << 24; [[fallthrough]];
			case 7:  b += ( static_cast<quint32>( static_cast<quint8>( s[k + 6] ) ) ) << 16; [[fallthrough]];
			case 6:  b += ( static_cast<quint32>( static_cast<quint8>( s[k + 5] ) ) ) << 8; [[fallthrough]];
			case 5:  b += static_cast<quint8>( s[k + 4] ); [[fallthrough]];
			case 4:  a += ( static_cast<quint32>( static_cast<quint8>( s[k + 3] ) ) ) << 24; [[fallthrough]];
			case 3:  a += ( static_cast<quint32>( static_cast<quint8>( s[k + 2] ) ) ) << 16; [[fallthrough]];
			case 2:  a += ( static_cast<quint32>( static_cast<quint8>( s[k + 1] ) ) ) << 8; [[fallthrough]];
			case 1:  a += static_cast<quint8>( s[k] ); break;
		}

		c ^= b; c -= ( ( b << 14 ) | ( b >> 18 ) );
		a ^= c; a -= ( ( c << 11 ) | ( c >> 21 ) );
		b ^= a; b -= ( ( a << 25 ) | ( a >> 7 ) );
		c ^= b; c -= ( ( b << 16 ) | ( b >> 16 ) );
		a ^= c; a -= ( ( c << 4 ) | ( c >> 28 ) );
		b ^= a; b -= ( ( a << 14 ) | ( a >> 18 ) );
		c ^= b; c -= ( ( b << 24 ) | ( b >> 8 ) );
	}

	return ( static_cast<quint64>( b ) << 32 ) | static_cast<quint64>( c );
}

bool cUOPIndex::open( QFile* file, const QString& pattern )
{
	close();

	if ( !file || !file->isOpen() || !file->isReadable() )
		return false;

	if ( !file->seek( 0 ) )
		return false;

	QDataStream ds( file );
	ds.setByteOrder( QDataStream::LittleEndian );

	quint32 magic = 0;
	quint32 version = 0;
	quint32 signature = 0;
	quint64 nextTable = 0;
	quint32 blockCapacity = 0;
	quint32 totalFiles = 0;

	ds >> magic >> version >> signature >> nextTable >> blockCapacity >> totalFiles;

	if ( magic != 0x0050594D ) // "MYP\0"
		return false;

	// Precalculate hash lookup table for "build/{pattern}/{index:08d}.dat"
	// Also check fallback with .bin if needed
	QMap<quint64, int> hashToChunk;
	int maxChunks = qMax( static_cast<int>( totalFiles ), 512 );
	for ( int i = 0; i < maxChunks; ++i )
	{
		QString chunkName = QString( "build/%1/%2.dat" )
			.arg( pattern.toLower() )
			.arg( i, 8, 10, QLatin1Char( '0' ) );
		QByteArray ascii = chunkName.toLatin1();
		quint64 h = hashLittle2( ascii.constData(), ascii.size() );
		hashToChunk.insert( h, i );
	}

	QMap<int, Chunk> indexedChunks;
	QVector<Chunk> fallbackChunks;

	while ( nextTable != 0 && nextTable < static_cast<quint64>( file->size() ) )
	{
		if ( !file->seek( nextTable ) )
			break;

		quint32 filesCount = 0;
		quint64 nextBlock = 0;
		ds >> filesCount >> nextBlock;

		for ( quint32 i = 0; i < filesCount; ++i )
		{
			quint64 offset = 0;
			quint32 headerLength = 0;
			quint32 compressedLength = 0;
			quint32 decompressedLength = 0;
			quint64 fileHash = 0;
			quint32 dataHash = 0;
			quint16 flag = 0;

			ds >> offset >> headerLength >> compressedLength >> decompressedLength
			   >> fileHash >> dataHash >> flag;

			if ( offset == 0 )
				continue;

			quint32 len = ( flag == 1 ) ? compressedLength : decompressedLength;
			Chunk c;
			c.fileOffset = offset + headerLength;
			c.length = len;
			c.virtualOffset = 0;

			if ( hashToChunk.contains( fileHash ) )
			{
				int chunkIdx = hashToChunk.value( fileHash );
				indexedChunks.insert( chunkIdx, c );
			}
			else
			{
				fallbackChunks.append( c );
			}
		}

		nextTable = nextBlock;
	}

	if ( !indexedChunks.isEmpty() )
	{
		int count = indexedChunks.size();
		chunks_.reserve( count );
		qint64 currentVirtOffset = 0;
		for ( int i = 0; i < count; ++i )
		{
			if ( !indexedChunks.contains( i ) )
			{
				close();
				return false;
			}
			Chunk c = indexedChunks.value( i );
			c.virtualOffset = currentVirtOffset;
			currentVirtOffset += c.length;
			chunks_.append( c );
		}
		totalSize_ = currentVirtOffset;
		valid_ = true;
		return true;
	}

	if ( !fallbackChunks.isEmpty() )
	{
		qint64 currentVirtOffset = 0;
		for ( int i = 0; i < fallbackChunks.size(); ++i )
		{
			Chunk c = fallbackChunks[i];
			c.virtualOffset = currentVirtOffset;
			currentVirtOffset += c.length;
			chunks_.append( c );
		}
		totalSize_ = currentVirtOffset;
		valid_ = true;
		return true;
	}

	return false;
}

qint64 cUOPIndex::lookup( qint64 virtualOffset ) const
{
	if ( !valid_ || chunks_.isEmpty() || virtualOffset < 0 || virtualOffset >= totalSize_ )
		return -1;

	int low = 0;
	int high = chunks_.size() - 1;
	int best = -1;

	while ( low <= high )
	{
		int mid = low + ( high - low ) / 2;
		if ( chunks_[mid].virtualOffset <= virtualOffset )
		{
			best = mid;
			low = mid + 1;
		}
		else
		{
			high = mid - 1;
		}
	}

	if ( best >= 0 )
	{
		const Chunk& c = chunks_[best];
		if ( virtualOffset < c.virtualOffset + c.length )
		{
			return static_cast<qint64>( c.fileOffset ) + ( virtualOffset - c.virtualOffset );
		}
	}

	return -1;
}

QByteArray cUOPIndex::extractEntry( QFile* file, quint64 chunkOffset, quint32 compressedLength, quint32 decompressedLength, quint16 flag )
{
	if ( !file || !file->seek( chunkOffset ) )
		return QByteArray();

	QByteArray compressed = file->read( compressedLength );
	if ( static_cast<quint32>( compressed.size() ) != compressedLength )
		return QByteArray();

	if ( flag == 0 )
	{
		return compressed;
	}
	else if ( flag == 1 )
	{
		QByteArray decompressed;
		decompressed.resize( decompressedLength );
		uLongf destLen = decompressedLength;
		int res = ::uncompress( reinterpret_cast<Bytef*>( decompressed.data() ), &destLen, reinterpret_cast<const Bytef*>( compressed.constData() ), compressedLength );
		if ( res != Z_OK )
		{
			return QByteArray();
		}
		decompressed.resize( static_cast<int>( destLen ) );
		return decompressed;
	}

	return QByteArray();
}
