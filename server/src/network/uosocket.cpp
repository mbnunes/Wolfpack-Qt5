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
	//cout << "Sending packet:" << endl;
	//packet->print( &cout );
	netio->sendPacket( _socket, packet, ( _state != SS_LOGGINGIN ) );
}

// Tries to recieve and process a packet
void cUOSocket::recieve()
{
	cUOPacket *packet = netio->recvPacket( _socket );

	if( !packet )
		return;

	Q_UINT8 packetId = (*packet)[0];

	// Disconnect harmful clients
	if( ( _state == SS_CONNECTING ) && ( packetId != 0x80 ) && ( packetId != 0x91 ) )
	{
		cUOTxDenyLogin *denyLogin = new cUOTxDenyLogin( DL_BADCOMMUNICATION );
		send( denyLogin );
		delete denyLogin;
		_socket->close();
		return;
	}

	// Relay it to the handler functions
	switch( packetId )
	{
	case 0x80:
		handleLoginRequest( static_cast< cUORxLoginRequest* >( packet ) ); break;
	case 0xA4:
		handleHardwareInfo( static_cast< cUORxHardwareInfo* >( packet ) ); break;
	case 0xA0:
		handleSelectShard( static_cast< cUORxSelectShard* >( packet ) ); break;
	case 0x91:
		handleServerAttach( static_cast< cUORxServerAttach* >( packet ) ); break;
	case 0x73:
		send( packet ); break; // For pings we just bounce the packet back
	case 0x83:
		handleDeleteCharacter( static_cast< cUORxDeleteCharacter* >( packet ) ); break;
	case 0x5D:
		handlePlayCharacter( static_cast< cUORxPlayCharacter* >( packet ) ); break;
	default:
		//cout << "Recieved packet: " << endl;
		//packet->print( &cout );
		break;
	}
	
	delete packet;
}

// Packet Handler
void cUOSocket::handleLoginRequest( cUORxLoginRequest *packet )
{
	// has to check for username/password here (normally)
	cout << "Trying to access the server via: " << packet->username().latin1() << "/" << packet->password().latin1() << endl;

	cUOPacket *denyPacket = NULL;

	// TODO: Insert authentication code here

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

	cUOTxShardList *shardList = new cUOTxShardList;
	shardList->addServer( 0, "My Server", 10, 0, 0x7F000001 );
	send( shardList );
	delete shardList;
}

void cUOSocket::handleHardwareInfo( cUORxHardwareInfo *packet )
{
	// Do something with the retrieved hardware information here
	// > Hardware Log ??
	QString hardwareMsg = QString( "Hardware: %1 Processors [Type: %2], %2 MB RAM, %3 MB Harddrive" ).arg( packet->processorCount() ).arg( packet->processorType() ).arg( packet->memoryInMb() ).arg( packet->largestPartitionInMb() );
	cout << hardwareMsg.latin1() << endl;
}

void cUOSocket::handleSelectShard( cUORxSelectShard *packet )
{
	cout << "User selected shard: " << packet->shardId() << endl;

	// Relay him - save an auth-id so we recog. him when he's relay locally
	_uniqueId = rand() % 0xFFFFFFFF;

	cUOTxRelayServer *relay = new cUOTxRelayServer;
	relay->setServerIp( 0x7F000001 );
	relay->setServerPort( 2593 );
	relay->setAuthId( _uniqueId );
	send( relay );
	delete relay;
}

void cUOSocket::handleServerAttach( cUORxServerAttach *packet )
{
	// From this point our output is compressed (!)
	_state = SS_LOGGEDIN;

	// We either have to recheck our login here or 
	// use the auth-id (not safe!!)
	// but for testing/debugging we'll assume that it's safe to continue

	sendCharList( packet->username() );
}

void cUOSocket::sendCharList()
{
	cUOTxCharTownList *charList = new cUOTxCharTownList;
	charList->addCharacter( "This" );
	charList->addCharacter( "is" );
	charList->addCharacter( "just" );
	charList->addCharacter( "a" );
	charList->addCharacter( "test" );

	charList->addTown( 0, "Britain", "Britain" );
	charList->compile();
	send( charList );
	delete charList;
}

void cUOSocket::handleDeleteCharacter( cUORxDeleteCharacter *packet )
{
	cout << "Trying to delete character with id " << packet->index() << endl;

	cUOTxUpdateCharList *update = new cUOTxUpdateCharList;
	update->setCharacter( 0, "You deleted me!" );
	send( update );
}

void cUOSocket::handlePlayCharacter( cUORxPlayCharacter *packet )
{
	cout << "User is trying to play character " << packet->character().latin1() << " [" << packet->password().latin1() << "]" << endl;
	cout << "in slot " << packet->slot() << endl;
}
