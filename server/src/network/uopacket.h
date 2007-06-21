/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2007 by holders identified in AUTHORS.txt
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

#if !defined(__UOPACKET_H__)
#define __UOPACKET_H__

#include <QByteArray>

//#include <iosfwd>

// Forward declaration
class QString;

class cUOPacket
{
private:
	QByteArray compressedBuffer;
	QByteArray rawPacket;

protected:
	bool haveCompressed;
	bool noswap;
	void init();
	void compress();
	void assign( cUOPacket& );
	void setRawData( int, const char*, int );

public:
	cUOPacket( const QByteArray& );
	cUOPacket( qint32 );
	cUOPacket( cUOPacket& );
	cUOPacket( quint8, qint32 );
	int size() const;
	int count() const;
	virtual ~cUOPacket();

	virtual  QByteArray compressed();
	virtual  QByteArray uncompressed()
	{
		return rawPacket;
	}
	void resize( int );
	int getInt( int ) const;
	short getShort( int ) const;
	QByteArray getAsciiString( int, int = 0 ) const;
	QString getUnicodeString( int, int ) const;
	void setInt( int, uint );
	void setShort( int, unsigned short );
	void setUnicodeString( int, const QString&, int, bool swapbytes = false );
	void setAsciiString( int, const char*, int );
	static QByteArray dump( const QByteArray& );

	// Operators
	char& operator []( int );
	char operator []( int ) const;
	cUOPacket& operator=( cUOPacket& p );
};

// Inline members

/*!
  Overloaded version for const objects.
*/
inline char cUOPacket::operator[]( int index ) const
{
	return rawPacket.at( index );
}

//#define WP_CPU_REQUIRES_DATA_ALIGNMENT
/* This template safely copies a value in from an untyped byte buffer to a typed value.
* (Make sure WP_CPU_REQUIRES_DATA_ALIGNMENT is defined if you are on a CPU
* that doesn't like non-word-aligned data reads and writes)
*/
template <typename T>
inline void wpCopyIn( T& dest, const void* source )
{
#ifdef WP_CPU_REQUIRES_DATA_ALIGNMENT
	memcpy( &dest, source, sizeof( dest ) );
#else
	dest = *( ( const T * ) source );
#endif
}

/** This template safely copies a value in from a typed value to an untyped byte buffer.
* (Make sure WP_CPU_REQUIRES_DATA_ALIGNMENT is defined if you are on a CPU
*  that doesn't like non-word-aligned data reads and writes)
*/
template <typename T>
inline void wpCopyOut( void* dest, const T& source )
{
#ifdef WP_CPU_REQUIRES_DATA_ALIGNMENT
	memcpy( dest, &source, sizeof( source ) );
#else
	*( ( T * ) dest ) = source;
#endif
}


#endif // __UOPACKET_H__

