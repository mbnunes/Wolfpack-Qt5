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
#include "../network.h"

// Wolfpack Includes
#include "../accounts.h"
#include "../globals.h"
#include "../junk.h"
#include "../structs.h"
#include "../srvparams.h"

#include <conio.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

// Send a packet to our ioHandle
void cUOSocket::send( cUOPacket *packet )
{
	// Don't send when we're already disconnected
	if( !_socket->isOpen() )
		return;

	cNetwork::instance()->netIo()->sendPacket( _socket, packet, ( _state != SS_LOGGINGIN ) );
}

// Tries to recieve and process a packet
void cUOSocket::recieve()
{
	cUOPacket *packet = cNetwork::instance()->netIo()->recvPacket( _socket );

	if( !packet )
		return;

	Q_UINT8 packetId = (*packet)[0];


	// After two pings we idle-disconnect
	if( lastPacket == 0x73 && packetId == 0x73 )
	{
		clConsole.send( QString( "Socket idle-kicked [%1]" ).arg( _socket->address().toString() ) );
		disconnect();
		return;
	}

	// Disconnect harmful clients
	if( ( _account < 0 ) && ( packetId != 0x80 ) && ( packetId != 0x91 ) )
	{
		clConsole.send( QString( "Communication Error [%1 instead of 0x80|0x91] [%2]\n" ).arg( packetId, 2, 16 ).arg( _socket->address().toString() ) );
		cUOTxDenyLogin *denyLogin = new cUOTxDenyLogin( DL_BADCOMMUNICATION );
		send( denyLogin );
		disconnect();
		return;
	}

	// Relay it to the handler functions
	switch( packetId )
	{
	case 0x00:
		handleCreateChar( static_cast< cUORxCreateChar* >( packet ) ); break;
	case 0x01: // Disconnect Notification recieved, should NEVER happen as it's unused now
		disconnect(); break;
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
		break; // Pings are handeled
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
	// If we dont authenticate disconnect us
	if( !authenticate( packet->username(), packet->password() ) )
	{
		disconnect();
		return;
	}

	// Otherwise build the shard-list
	cUOTxShardList *shardList = new cUOTxShardList;

	vector< ServerList_st > shards = SrvParams->serverList();
	
	for( Q_UINT8 i = 0; i < shards.size(); ++i )
		shardList->addServer( i, shards[i].sServer, shards[i].uiFull, shards[i].uiTime, shards[i].sIP );
	
	send( shardList );
	delete shardList;
}

void cUOSocket::handleHardwareInfo( cUORxHardwareInfo *packet )
{
	// Do something with the retrieved hardware information here
	// > Hardware Log ??
	//QString hardwareMsg = QString( "Hardware: %1 Processors [Type: %2], %2 MB RAM, %3 MB Harddrive" ).arg( packet->processorCount() ).arg( packet->processorType() ).arg( packet->memoryInMb() ).arg( packet->largestPartitionInMb() );
	//cout << hardwareMsg.latin1() << endl;
}

void cUOSocket::disconnect( void )
{
	_socket->close();
}

void cUOSocket::handleSelectShard( cUORxSelectShard *packet )
{
	// Relay him - save an auth-id so we recog. him when he relays locally
	vector< ServerList_st > shards = SrvParams->serverList();

	if( packet->shardId() >= shards.size() )
	{
		disconnect();
		return;
	}
	
	_uniqueId = rand() % 0xFFFFFFFF;
	
	cUOTxRelayServer *relay = new cUOTxRelayServer;
	relay->setServerIp( shards[ packet->shardId() ].sIP );
	relay->setServerPort( shards[ packet->shardId() ].uiPort );
	relay->setAuthId( _uniqueId );
	send( relay );
	delete relay;
}

void cUOSocket::handleServerAttach( cUORxServerAttach *packet )
{
	// From this point our output is compressed (!)
	_state = SS_LOGGEDIN;

	// Re-Authenticate the user !!
	if( _account < 0 )
	{
		if( !authenticate( packet->username(), packet->password() ) )
		{
			disconnect();
			return;
		}
	}

	// We either have to recheck our login here or 
	// use the auth-id (not safe!!)
	// but for testing/debugging we'll assume that it's safe to continue
	sendCharList();
}

void cUOSocket::sendCharList()
{
	cUOTxCharTownList *charList = new cUOTxCharTownList;
	vector< P_CHAR > characters = Accounts->characters( _account );

	// Add the characters
	for( Q_UINT8 i = 0; i < characters.size(); ++i )
		charList->addCharacter( characters[ i ]->name.c_str() );

	// Add the Starting Locations
	vector< StartLocation_st > startLocations = SrvParams->startLocation();
	for( Q_UINT8 i = 0; i < startLocations.size(); ++i )
		charList->addTown( i, startLocations[i].name, startLocations[i].name );

	charList->compile();
	send( charList );
	delete charList;
}

void cUOSocket::handleDeleteCharacter( cUORxDeleteCharacter *packet )
{
	//cout << "Trying to delete character with id " << packet->index() << endl;

	cUOTxUpdateCharList *update = new cUOTxUpdateCharList;
	update->setCharacter( 0, "You deleted me!" );
	send( update );
}

void cUOSocket::handlePlayCharacter( cUORxPlayCharacter *packet )
{
	//cout << "User is trying to play character " << packet->character().latin1() << " [" << packet->password().latin1() << "]" << endl;
	//cout << "in slot " << packet->slot() << endl;

	// Minimum Requirements for log in
	// a) Set the map the user is on
	// b) Set the client features
	// c) Confirm the Login
	// d) Start the Game
	// e) Set the Game Time

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

bool cUOSocket::authenticate( const QString &username, const QString &password )
{
	// Log
	clConsole.send( QString( "Trying to log in as %1 using password %2 [%3]\n" ).arg( username ).arg( password ).arg( _socket->address().toString() ) );

	Q_INT32 authRet = Accounts->Authenticate( username, password );

	cUOPacket *denyPacket = NULL;

	switch( authRet )
	{
	case LOGIN_NOT_FOUND:
		denyPacket = new cUOTxDenyLogin( DL_NOACCOUNT ); break;
	case BAD_PASSWORD:
		denyPacket = new cUOTxDenyLogin( DL_BADPASSWORD ); break;
	case ACCOUNT_WIPE:
	case ACCOUNT_BANNED:
		denyPacket = new cUOTxDenyLogin( DL_BLOCKED ); break;
	};

	// Reject login
	if( denyPacket )
	{
		clConsole.send( QString( "Bad Authentication [%1]\n" ).arg( _socket->address().toString() ) );
		send( denyPacket );
		delete denyPacket;
		disconnect();
		return false;
	}	

	_account = authRet;
	return true;
}

// Processes a create character request
// Notes from Lord Binaries packet documentation:

void cUOSocket::handleCreateChar( cUORxCreateChar *packet )
{
	// Several security checks
	vector< P_CHAR > characters = Accounts->characters( _account );

	if( characters.size() >= 5 )
	{

	}

}

void cUOSocket::sysMessage( const QString &message, Q_UINT16 )
{
	// Color: 0x0037
	cUOTxUnicodeSpeech *speech = new cUOTxUnicodeSpeech;
	speech->setSource( 0xFFFFFFFF );
	speech->setModel( 0xFFFF );
	speech->setFont( 3 );
	speech->setLanguage( "ENU" ); // Standard server language >> Change later
	speech->setColor( 0x0037 );
	speech->setType( cUOTxUnicodeSpeech::System );
	speech->setName( "System" );
	
}

