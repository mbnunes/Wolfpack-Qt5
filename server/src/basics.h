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

// basics.h: interface for the basics functions.
#if !defined(__BASICS_H__)
#define __BASICS_H__

// Platfrom specifics
#include "platform.h"

#include <functional>
#include <qmap.h>

// Forward definitions
class Coord_cl;
class QString;

// sereg : roll dices d&d style
int rollDice( const QString& dicePattern );

bool parseCoordinates( const QString& input, Coord_cl& coord );

int RandomNum( int nLowNum, int nHighNum );

QString hex2dec( const QString& value );

float RandomFloatNum( float nLowNum, float nHighNum );
unsigned int getNormalizedTime();

template <class T>
struct destroy_obj : std::unary_function<T, void>
{
	void operator()( T& d ) const
	{
		delete d;
		d = 0;
	}
};

class QCString;

class cBufferedWriter
{
private:
	class cBufferedWriterPrivate *d;

public:
	cBufferedWriter( const QCString& magic, unsigned int version );
	~cBufferedWriter();

	void open( const QString& filename );
	void close();
	void flush();

	void writeInt( unsigned int data, bool unbuffered = false );
	void writeShort( unsigned short data, bool unbuffered = false );
	void writeByte( unsigned char data, bool unbuffered = false );
	void writeUtf8( const QString& data, bool unbuffered = false );
	void writeAscii( const QCString& data, bool unbuffered = false );
	void writeRaw( const void* data, unsigned int size, bool unbuffered = false );
	void writeDouble( double data, bool unbuffered = false );

	unsigned int position();
	unsigned int version();
	void setSkipSize( unsigned char type, unsigned int skipsize );
	void setObjectCount( unsigned int count );
	unsigned int objectCount();
};

class cBufferedReader
{
private:
	class cBufferedReaderPrivate *d;

public:
	cBufferedReader( const QCString& magic, unsigned int version );
	~cBufferedReader();

	void open( const QString& filename );
	void close();
	unsigned int version();

	unsigned int readInt();
	unsigned short readShort();
	unsigned char readByte();
	double readDouble();
	QString readUtf8();
	QCString readAscii( bool nodictionary = false );
	void readRaw( void* data, unsigned int size );

	unsigned int position();
	const QMap<unsigned char, QCString>& typemap();
	unsigned int getSkipSize( unsigned char type );

	unsigned int objectCount();
};

#endif
