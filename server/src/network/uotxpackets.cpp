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

#include "uotxpackets.h"
#include "uopacket.h"

void cUOTxShardList::addServer( Q_UINT16 serverIndex, QString serverName, Q_UINT8 serverFull, Q_INT8 serverTimeZone, Q_UINT32 serverIp )
{
	// Increase the server-count
	// Offset: 4
	setShort( 4, getShort( 4 ) + 1 );

	Q_INT32 offset = rawPacket.count();
	rawPacket.resize( rawPacket.count() + 40 ); // 40 byte per server
	setShort( offset, serverIndex );

	if( serverName.length() > 31 ) 
		serverName = serverName.left( 31 );

	strcpy( &rawPacket.data()[ offset + 2 ], serverName.latin1() );

	rawPacket[ offset + 34 ] = serverFull;
	rawPacket[ offset + 35 ] = serverTimeZone;
	setInt( offset + 36, serverIp );
}
