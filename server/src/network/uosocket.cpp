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

	packet->print( &cout );

	// Disconnect harmful clients
	if( ( _state == SS_CONNECTING ) && ( packetId != 0x80 ) && ( packetId != 0x91 ) )
	{
		cUOTxDenyLogin denyLogin( DL_BADCOMMUNICATION );
		send( &denyLogin );
		_socket->close();
		return;
	}

	// Relay it to the handler functions
	switch( packetId )
	{
	case 0x02: // just want to walk a little.
		{
			cUOPacket moveOk(3);
			moveOk[1] = (*packet)[2];
			send( &moveOk );
		}
	case 0x80:
		handleLoginRequest( dynamic_cast< cUORxLoginRequest* >( packet ) ); break;
	case 0xA4:
		handleHardwareInfo( dynamic_cast< cUORxHardwareInfo* >( packet ) ); break;
	case 0xA0:
		handleSelectShard( dynamic_cast< cUORxSelectShard* >( packet ) ); break;
	case 0x91:
		handleServerAttach( dynamic_cast< cUORxServerAttach* >( packet ) ); break;
	case 0x73:
		send( packet ); break; // For pings we just bounce the packet back
	case 0x83:
		handleDeleteCharacter( dynamic_cast< cUORxDeleteCharacter* >( packet ) ); break;
	case 0x5D:
		handlePlayCharacter( dynamic_cast< cUORxPlayCharacter* >( packet ) ); break;
	default:
		//cout << "Recieved packet: " << endl;
		packet->print( &cout );
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

	cUOTxRelayServer relay;
	relay.setServerIp( 0x7F000001 );
	relay.setServerPort( 2593 );
	relay.setAuthId( _uniqueId );
	send( &relay );
}

void cUOSocket::handleServerAttach( cUORxServerAttach *packet )
{
	// From this point our output is compressed (!)
	_state = SS_LOGGEDIN;

	// We either have to recheck our login here or 
	// use the auth-id (not safe!!)
	// but for testing/debugging we'll assume that it's safe to continue

	sendCharList();
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

	// Minimum Requirements for log in
	// a) Set the map the user is on
	// b) Set the client features
	// c) Confirm the Login
	// d) Start the Game
	// E) Set the Game Time

	cUOTxClientFeatures *clientFeatures = new cUOTxClientFeatures;
	clientFeatures->setLbr( true );
	clientFeatures->setT2a( true );
	send( clientFeatures );
	delete clientFeatures;

	// Confirm the Login
	cUOTxConfirmLogin *confirmLogin = new cUOTxConfirmLogin;

	// Set our data here
	confirmLogin->setSerial( 0x00000001 );
	confirmLogin->setBody( 0x190 );
	confirmLogin->setDirection( 0x00 );
	confirmLogin->setX( 1000 );
	confirmLogin->setY( 1000 );
	confirmLogin->setZ( 20 );

	// Set the unknown data
	confirmLogin->setUnknown3( 0x007f0000 );
	confirmLogin->setUnknown4( 0x00000007 );
	confirmLogin->setUnknown5( "\x60\x00\x00\x00\x00\x00\x00" );

	send( confirmLogin );
	delete confirmLogin;

	// Change the map after the client knows about the char
	cUOTxChangeMap *changeMap = new cUOTxChangeMap;
	changeMap->setMap( MT_TRAMMEL );
	send( changeMap );
	delete changeMap;

	// Start the game!
	cUOTxStartGame *startGame = new cUOTxStartGame;
	send( startGame );
	delete startGame;

	// Send the gametime
	cUOTxGameTime *gameTime = new cUOTxGameTime;
	gameTime->setTime( 12, 19, 3 );
	send( gameTime );
	delete gameTime;
}

