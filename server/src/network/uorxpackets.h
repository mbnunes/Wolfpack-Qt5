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

#ifndef __UO_RXPACKETS__
#define __UO_RXPACKETS__

#include "uopacket.h"
#include "qcstring.h"
#include "qstring.h"

cUOPacket *getUOPacket( const QByteArray &data );

// 0x80: Login Request
class cUORxLoginRequest: public cUOPacket
{
public:
	cUORxLoginRequest( const QByteArray &data ): cUOPacket( data ) {}
	QString username( void ) { return &rawPacket.data()[1]; }
	QString password( void ) { return &rawPacket.data()[31]; }
};

// 0xA4: Hardware Info
class cUORxHardwareInfo: public cUOPacket
{
public:
	cUORxHardwareInfo( const QByteArray &data ): cUOPacket( data ) {}
	Q_UINT8 processorType( void ) { return rawPacket[ 0x01 ]; }
	Q_UINT16 processorSpeed( void ) { return getShort( 0x02 ); }
	Q_UINT8 processorCount( void ) { return rawPacket[ 0x04 ]; }
	// QString directory -- unicode ? -- > 20 bytes useless trash
	// Video Card descritor: -- unicode ? -- > 20 bytes useless trash
	// ??? > another 20 byte trash field
	// ??? > ANOTHER 20 byte trash field
	Q_UINT16 memoryInMb( void ) { return getShort( 0x85 ); }
	Q_UINT16 largestPartitionInMb( void ) { return getShort( 0x87 ); }
	Q_INT32 timezoneBias( void ) { return getInt( 0x8C ); }
};

// 0xA0: Select Shard
class cUORxSelectShard: public cUOPacket
{
public:
	cUORxSelectShard( const QByteArray &data ): cUOPacket( data ) {}
	Q_UINT16 shardId( void ) { return getShort( 1 ); }
};

// 0x91: Charlist Request
class cUORxServerAttach: public cUOPacket
{
public:
	cUORxServerAttach( const QByteArray &data ): cUOPacket( data ) {}
	Q_UINT32 authId( void ) { return getInt( 1 ); }
	QString username( void ) { return &rawPacket.data()[2]; }
	QString password( void ) { return &rawPacket.data()[32]; }
};

// 0x73 Ping
class cUORxPing: public cUOPacket
{
public:
	cUORxPing( const QByteArray &data ): cUOPacket( data ) {}
};

// 0x83 Delete Character
class cUORxDeleteCharacter: public cUOPacket
{
public:
	cUORxDeleteCharacter( const QByteArray &data ): cUOPacket( data ) {}

	QString password( void ) { return &rawPacket.data()[1]; }
	Q_UINT32 index( void ) { return getInt( 31 ); }
	Q_UINT32 ip( void ) { return getInt( 35 ); }
};

// 0x5D Play Character
class cUORxPlayCharacter: public cUOPacket
{
public:
	cUORxPlayCharacter( const QByteArray &data ): cUOPacket( data ) {}

	QString character( void ) { return &rawPacket.data()[5]; }
	QString password( void ) { return &rawPacket.data()[35]; }
	Q_UINT32 slot( void ) { return getInt( 65 ); }
	Q_UINT32 ip( void ) { return getInt( 69 ); }
};

#endif