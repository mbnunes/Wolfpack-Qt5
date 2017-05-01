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

// basics.h: interface for the basics functions.
#if !defined(__BASICS_H__)
#define __BASICS_H__

// Platfrom specifics
#include "platform.h"

#include <functional>
#include <algorithm>
#include <QMap>
#include <QFile>
#include <QByteArray>

#define WRITER_BUFFERSIZE 1048576

// Forward definitions
class Coord;
class QString;

// sereg : roll dices d&d style
int rollDice( const QString& dicePattern );

bool parseCoordinates( const QString& input, Coord& coord, bool ignoreZ = false );

int RandomNum( int nLowNum, int nHighNum );

QString hex2dec( const QString& value );

float RandomFloatNum( float nLowNum, float nHighNum );
unsigned int getNormalizedTime();
uint elfHash(const char * name);

// Swap the value in place
inline void swapBytes( unsigned int& data )
{
	data = ( ( data & 0xFF ) << 24 ) | ( ( data & 0xFF00 ) << 8 ) | ( ( data & 0xFF0000 ) >> 8 ) | ( ( data & 0xFF000000 ) >> 24 );
}

inline void swapBytes( double& value )
{
	unsigned char * ptr = ( unsigned char * ) &value;
	std::swap( ptr[0], ptr[7] );
	std::swap( ptr[1], ptr[6] );
	std::swap( ptr[2], ptr[5] );
	std::swap( ptr[3], ptr[4] );
}

inline void swapBytes( int& data )
{
	data = ( ( data & 0xFF ) << 24 ) | ( ( data & 0xFF00 ) << 8 ) | ( ( data & 0xFF0000 ) >> 8 ) | ( ( data & 0xFF000000 ) >> 24 );
}

inline void swapBytes( unsigned short& data )
{
	data = ( ( data & 0xFF00 ) >> 8 ) | ( ( data & 0xFF ) << 8 );
}

inline void swapBytes( short& data )
{
	data = ( ( data & 0xFF00 ) >> 8 ) | ( ( data & 0xFF ) << 8 );
}

class cBufferedWriter
{
private:
	class cBufferedWriterPrivate *d;

	void writeIntUnbuffered( unsigned int data );
	void writeShortUnbuffered( unsigned short data );
	void writeByteUnbuffered( unsigned char data );
	void writeBoolUnbuffered( bool data );
	void writeUtf8Unbuffered( const QString& data );
	void writeAsciiUnbuffered( const QByteArray& data );
	void writeRawUnbuffered( const void* data, unsigned int size );
	void writeDoubleUnbuffered( double data );

public:
	cBufferedWriter( const QByteArray& magic, unsigned int version );
	~cBufferedWriter();

	void open( const QString& filename );
	void close();
	void flush();

	void writeInt( unsigned int data );
	void writeShort( unsigned short data );
	void writeByte( unsigned char data );
	void writeBool( bool data );
	void writeUtf8( const QString& data );
	void writeAscii( const QByteArray& data );
	void writeRaw( const void* data, unsigned int size );
	void writeDouble( double data );

	unsigned int position();
	unsigned int version();
	void setSkipSize( unsigned char type, unsigned int skipsize );
	void setObjectCount( unsigned int count );
	unsigned int objectCount();
};

class cBufferedWriterPrivate
{
public:
	QFile file;
	unsigned int version;
	QByteArray magic;
	char *buffer;
	unsigned int bufferpos;
	QMap<QByteArray, unsigned int> dictionary;
	QMap<unsigned char, unsigned int> skipmap;
	QMap<unsigned char, QString> typemap;
	unsigned int lastStringId;
	unsigned int objectCount;
};

class cBufferedReader
{
private:
	class cBufferedReaderPrivate *d;
	QString error_;

public:
	cBufferedReader( const QByteArray& magic, unsigned int version );
	~cBufferedReader();

	void open( const QString& filename );
	void close();
	unsigned int version();

	unsigned int readInt();
	unsigned short readShort();
	unsigned char readByte();
	bool readBool();
	double readDouble();
	QString readUtf8();
	QByteArray readAscii( bool nodictionary = false );
	void readRaw( void* data, int size );

	unsigned int position();
	const QMap<unsigned char, QByteArray>& typemap();
	unsigned int getSkipSize( unsigned char type );

	unsigned int objectCount();

	inline void setError( const QString& error )
	{
		error_ = error;
	}

	inline bool hasError()
	{
		return !error_.isNull();
	}

	inline const QString& error()
	{
		return error_;
	}
};

inline void cBufferedWriter::writeInt( unsigned int data )
{
#if (Q_BYTE_ORDER != Q_LITTLE_ENDIAN)
	swapBytes( data );
#endif

	if ( d->bufferpos > WRITER_BUFFERSIZE - sizeof( data ) ) {
		flush();
	}

	unsigned int *ptr = (unsigned int*)(d->buffer + d->bufferpos);
	*ptr = data;
	d->bufferpos += sizeof(data);
}

inline void cBufferedWriter::writeIntUnbuffered( unsigned int data )
{
#if (Q_BYTE_ORDER != Q_LITTLE_ENDIAN)
	swapBytes( data );
#endif

	flush();
	d->file.write( ( char * ) &data, sizeof( data ) );
}

inline void cBufferedWriter::writeShort( unsigned short data )
{
#if (Q_BYTE_ORDER != Q_LITTLE_ENDIAN)
	swapBytes( data );
#endif

	if ( d->bufferpos > WRITER_BUFFERSIZE - sizeof( data ) )
	{
		flush();
	}

	unsigned short *ptr = (unsigned short*)(d->buffer + d->bufferpos);
	*ptr = data;
	d->bufferpos += sizeof(data);
}

inline void cBufferedWriter::writeShortUnbuffered( unsigned short data )
{
#if (Q_BYTE_ORDER != Q_LITTLE_ENDIAN)
	swapBytes( data );
#endif

	flush();
	d->file.write( ( char * ) &data, sizeof( data ) );
}

inline void cBufferedWriter::writeBool( bool data )
{
	writeByte( data ? 1 : 0 );
}

inline void cBufferedWriter::writeBoolUnbuffered( bool data )
{
	writeByteUnbuffered( data ? 1 : 0 );
}

inline void cBufferedWriter::writeByte( unsigned char data )
{
	if ( d->bufferpos >= WRITER_BUFFERSIZE ) {
		flush();
	}

	d->buffer[d->bufferpos++] = (char)data;
}

inline void cBufferedWriter::writeByteUnbuffered( unsigned char data )
{
	flush();
	d->file.write( ( const char * ) &data, sizeof( data ) );
}

inline void cBufferedWriter::writeUtf8( const QString& data )
{
	writeAscii( data.toUtf8() );
}

inline void cBufferedWriter::writeUtf8Unbuffered( const QString& data )
{
	writeAsciiUnbuffered( data.toUtf8() );
}

inline void cBufferedWriter::writeAscii( const QByteArray& data )
{
	QMap<QByteArray, unsigned int>::iterator it = d->dictionary.find( data );

	if ( it != d->dictionary.end() )
	{
		writeInt( it.value() );
	}
	else
	{
		d->dictionary.insert( data, ++d->lastStringId );
		writeInt( d->lastStringId );
	}
}

inline void cBufferedWriter::writeAsciiUnbuffered( const QByteArray& data )
{
	QMap<QByteArray, unsigned int>::iterator it = d->dictionary.find( data );

	if ( it != d->dictionary.end() )
	{
		writeIntUnbuffered( it.value() );
	}
	else
	{
		d->dictionary.insert( data, ++d->lastStringId );
		writeIntUnbuffered( d->lastStringId );
	}
}

inline void cBufferedWriter::writeRaw( const void* data, unsigned int size )
{
	// Flush out entire blocks if neccesary until we dont
	// overflow the buffer anymore, then just append
	unsigned int pos = 0;

	while ( d->bufferpos + size >= ( unsigned int ) WRITER_BUFFERSIZE )
	{
		unsigned int bspace = WRITER_BUFFERSIZE - d->bufferpos;

		// Try putting in some bytes of the remaining data
		if ( bspace != 0 )
		{
			memcpy( d->buffer + d->bufferpos, ( unsigned char * ) data + pos, bspace );
			d->bufferpos = WRITER_BUFFERSIZE;
			pos += bspace;
			size -= bspace;
		}

		flush();
	}

	// There are still some remaining bytes of our data
	if ( size != 0 )
	{
		memcpy( d->buffer + d->bufferpos, ( unsigned char * ) data + pos, size );
		d->bufferpos += size;
	}
}

inline void cBufferedWriter::writeRawUnbuffered( const void* data, unsigned int size )
{
	flush();
	d->file.write( ( const char * ) data, size );
}

inline void cBufferedWriter::writeDouble( double value )
{
#if (Q_BYTE_ORDER != Q_LITTLE_ENDIAN)
	// Inplace Swapping (data is a copy anyway)
	swapBytes( value );
#endif

	writeRaw( &value, sizeof( value ) );
}

inline void cBufferedWriter::writeDoubleUnbuffered( double value )
{
#if (Q_BYTE_ORDER != Q_LITTLE_ENDIAN)
	// Inplace Swapping (data is a copy anyway)
	swapBytes( value );
#endif

	writeRawUnbuffered( &value, sizeof( value ) );
}

#endif

