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

#ifndef __UO_TXPACKETS__
#define __UO_TXPACKETS__

// Library includes
#include "qcstring.h"
#include "qstring.h"

#include "uopacket.h"

// Packets which are sent always uncompressed
class cUORawPacket: public cUOPacket
{
public:
	cUORawPacket( Q_UINT32 size ): cUOPacket( size ) {}
	virtual QByteArray compressed() { return rawPacket; }
};

enum eDenyLogin
{
	DL_NOACCOUNT = 0x00,
	DL_INUSE,
	DL_BLOCKED,
	DL_BADPASSWORD,
	DL_BADCOMMUNICATION
};

// Packet to deny Login
class cUOTxDenyLogin: public cUORawPacket
{
public:
	cUOTxDenyLogin( eDenyLogin reason ): cUORawPacket( 2 )
	{
		rawPacket[0] = (Q_UINT8)0x82;
		rawPacket[1] = reason;
	}
};

class cUOTxAcceptLogin: public cUORawPacket
{
public:
	cUOTxAcceptLogin(): cUORawPacket( 3 )
	{
		rawPacket[0] = (Q_UINT8)0x81;
	}
};

class cUOTxShardList: public cUORawPacket
{
public:
	cUOTxShardList(): cUORawPacket( 6 )
	{
		rawPacket[0] = (Q_UINT8)0xA8;
		rawPacket[3] = (Q_UINT8)0xFF;
	}

	void addServer( Q_UINT16 serverIndex, QString serverName, Q_UINT8 serverFull = 0, Q_INT8 serverTimeZone = 0, Q_UINT32 serverIp = 0 );
};

#endif