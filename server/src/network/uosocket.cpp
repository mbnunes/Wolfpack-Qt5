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

#include "uosocket.h"
#include "uopacket.h"
#include "uotxpackets.h"
#include "asyncnetio.h"

#include <conio.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

extern cAsyncNetIO* netio;

// Send a packet to our ioHandle
void cUOSocket::send( cUOPacket *packet )
{
	packet->print( &cout );
	netio->sendPacket( _socket, packet );
}

// Tries to recieve and process a packet
void cUOSocket::recieve()
{
	cUOPacket *packet = netio->recvPacket( _socket );

	if( !packet )
		return;

	Q_UINT8 packetId = (*packet)[0];

	switch( packetId )
	{
	case 0x80:
		handleLoginRequest( static_cast< cUORxLoginRequest* >( packet ) ); break;
	default:
		packet->print( &cout );
	}
	
	delete packet;
}

// Packet Handler
void cUOSocket::handleLoginRequest( cUORxLoginRequest *packet )
{
	// has to check for username/password here (normally)
	cout << "Trying to access the server via: " << packet->username().latin1() << "/" << packet->password().latin1() << endl;

	cUOPacket *denyPacket = NULL;

	if( packet->username() != "admin" )
		denyPacket = new cUOTxDenyLogin( DL_NOACCOUNT );
	else if( packet->password() != "admin" )
		denyPacket = new cUOTxDenyLogin( DL_BADPASSWORD );

	// Reject login
	if( denyPacket )
	{
		send( denyPacket );
		delete denyPacket;
		return;
	}

	// Accept login and send serverlist
	/*cUOTxAcceptLogin *acceptLogin = new cUOTxAcceptLogin;
	netio->sendPacket( _socket, acceptLogin );
	delete acceptLogin;*/

	cUOTxShardList *shardList = new cUOTxShardList;
	shardList->addServer( 0, "Mein Server", 10, 0, 0x7F000001 );
	send( shardList );
	delete shardList;
}
