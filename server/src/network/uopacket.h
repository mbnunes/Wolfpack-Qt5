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

#if !defined(__UOPACKET_H__)
#define __UOPACKET_H__

#include "qcstring.h"
#include "qstring.h"

#include <iostream>

using namespace std;

// Forward declaration
class cUORxMultiPurpose;

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
	void resize( uint );
	void setRawData( uint, const char*, uint );

public:
	cUOPacket( QByteArray );
	cUOPacket( Q_UINT32 );
	cUOPacket( cUOPacket& );
	cUOPacket( Q_UINT8, Q_UINT32 );
	virtual ~cUOPacket() {}
	uint	size() const;
	uint    count() const;

	virtual QByteArray compressed();
	virtual QByteArray uncompressed() { return rawPacket; }
	int		getInt( uint ) const;
	short	getShort( uint) const;
	QString getAsciiString( uint, uint = 0 ) const;
	QString getUnicodeString( uint, uint ) const;
	void	setInt( uint, uint );
	void	setShort( uint, ushort );
	void	setUnicodeString( uint, QString&, uint );
	void	setAsciiString( uint, const char*, uint );
	static	QString dump( const QByteArray& );

	virtual void print( ostream* );

	// Operators
	char& operator []( uint );
	char  operator []( uint ) const;
	cUOPacket& operator=( cUOPacket& p );

};

// Inline members

/*!
  Overloaded version for const objects.
*/
inline char cUOPacket::operator[] ( unsigned int index ) const
{
	return rawPacket.at( index );
}


#endif // __UOPACKET_H__

