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

#include "qhostaddress.h"

#include "uosocket.h"
#include "uopacket.h"
#include "uotxpackets.h"
#include "asyncnetio.h"
#include "../network.h"
#include "../chars.h"

// Wolfpack Includes
#include "../accounts.h"
#include "../globals.h"
#include "../junk.h"
#include "../structs.h"
#include "../srvparams.h"
#include "../wpdefmanager.h"
#include "../walking2.h"

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

	cNetwork::instance()->netIo()->sendPacket( _socket, packet, ( _state != LoggingIn ) );
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

	// Switch to encrypted mode if one of the advanced packets is recieved
	if( packetId == 0x91 )
		_state = LoggedIn;

	// Relay it to the handler functions
	switch( packetId )
	{
	case 0x00:
		handleCreateChar( dynamic_cast< cUORxCreateChar* >( packet ) ); break;
	case 0x01: // Disconnect Notification recieved, should NEVER happen as it's unused now
		disconnect(); break;
	case 0x02: // just want to walk a little.
		handleWalkRequest( dynamic_cast< cUORxWalkRequest* >( packet ) );
		break;
	case 0x80:
		handleLoginRequest( dynamic_cast< cUORxLoginRequest* >( packet ) ); break;
	case 0xA4:
		handleHardwareInfo( dynamic_cast< cUORxHardwareInfo* >( packet ) ); break;
	case 0xA0:
		handleSelectShard( dynamic_cast< cUORxSelectShard* >( packet ) ); break;
	case 0x91:
		handleServerAttach( dynamic_cast< cUORxServerAttach* >( packet ) ); break;
	case 0x34:
		handleQuery( dynamic_cast< cUORxQuery* >( packet ) ); break;
	case 0x73:
		break; // Pings are handeled
	case 0x83:
		handleDeleteCharacter( dynamic_cast< cUORxDeleteCharacter* >( packet ) ); break;
	case 0x5D:
		handlePlayCharacter( dynamic_cast< cUORxPlayCharacter* >( packet ) ); break;
	case 0xC8:
		handleUpdateRange( dynamic_cast< cUORxUpdateRange* >( packet ) ); break;
	case 0x09:
		handleRequestLook( dynamic_cast< cUORxRequestLook* >( packet ) ); break;
	case 0xBF:
		handleMultiPurpose( dynamic_cast< cUORxMultiPurpose* >( packet ) ); break;
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
	Q_UINT8 i = 0;
	for(; i < characters.size(); ++i )
		charList->addCharacter( characters[ i ]->name.c_str() );

	// Add the Starting Locations
	vector< StartLocation_st > startLocations = SrvParams->startLocation();
	for( i = 0; i < startLocations.size(); ++i )
		charList->addTown( i, startLocations[i].name, startLocations[i].name );

	charList->compile();
	send( charList );
	delete charList;
}

void cUOSocket::handleDeleteCharacter( cUORxDeleteCharacter *packet )
{
	// TODO: Implement code here
	updateCharList();
}

// When a user selects a character to play this is the handler
void cUOSocket::handlePlayCharacter( cUORxPlayCharacter *packet )
{
	// Check the character the user wants to play
	vector< P_CHAR > characters = Accounts->characters( _account );

	if( packet->slot() >= characters.size() )
	{
		cUOTxDenyLogin denyLogin( DL_BADCOMMUNICATION );
		send( &denyLogin );
		return;
	}

	_player = characters[ packet->slot() ];
	playChar();
}

// Set up the neccesary stuff to play
void cUOSocket::playChar( P_CHAR pChar )
{
	if( pChar == NULL )
		pChar = _player;

	// Minimum Requirements for log in
	// a) Set the map the user is on
	// b) Set the client features
	// c) Confirm the Login
	// d) Start the Game
	// e) Set the Game Time

	cUOTxClientFeatures clientFeatures;
	clientFeatures.setLbr( true );
	clientFeatures.setT2a( true );
	send( &clientFeatures );

	// Confirm the Login
	cUOTxConfirmLogin confirmLogin;
	confirmLogin.fromChar( pChar );
	confirmLogin.setBody( 0x3DB );
		
	confirmLogin.setUnknown3( 0x007f0000 );
	confirmLogin.setUnknown4( 0x00000007 );
	confirmLogin.setUnknown5( "\x60\x00\x00\x00\x00\x00\x00" );

	send( &confirmLogin );

	// Change the map after the client knows about the char
	cUOTxChangeMap changeMap;
	changeMap.setMap( pChar->pos.map );
	send( &changeMap );

	// Start the game!
	cUOTxStartGame startGame;
	send( &startGame );

	// Send the gametime
	cUOTxGameTime gameTime;
	gameTime.setTime( 12, 19, 3 );
	send( &gameTime );

	// We're now playing this char:
	pChar->setSocket( this );
	_player = pChar;
	_state  = InGame;
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
#define cancelCreate( message ) cUOTxDenyLogin denyLogin( DL_BADCOMMUNICATION ); send( &denyLogin ); sysMessage( message ); disconnect(); return;

void cUOSocket::handleCreateChar( cUORxCreateChar *packet )
{
	// Several security checks
	vector< P_CHAR > characters = Accounts->characters( _account );

    // If we have more than 5 characters
	if( characters.size() >= 5 )
	{
		clConsole.send( tr( "%1 is trying to create char but has more than 5 characters" ).arg( Accounts->findByNumber( _account ) ) );
		cancelCreate( tr("You already have more than 5 characters") )
	}

	clConsole.send( cUOPacket::dump( packet->uncompressed() ) );

	// Check the stats
	Q_UINT16 statSum = ( packet->strength() + packet->dexterity() + packet->intelligence() );
	if( statSum > 80 )
	{
		clConsole.send( tr( "%1 is trying to create char with wrong stats: %2" ).arg( Accounts->findByNumber( _account ) ).arg( statSum ) );
		cancelCreate( tr( "Invalid Character stat sum: %1" ).arg( statSum ) )
	}

	// Every stat needs to be below 60
	if( ( packet->strength() > 60 ) || ( packet->dexterity() > 60 ) || ( packet->intelligence() > 60 ) )
	{
		clConsole.send( tr( "%1 is trying to create char with wrong stats: %2 [str] %3 [dex] %4 [int]" ).arg( Accounts->findByNumber( _account ) ).arg( packet->strength() ).arg( packet->dexterity() ).arg( packet->intelligence() ) );
		cancelCreate( tr("Invalid Character stats") )
	}

	// Check the skills
	if( ( packet->skillId1() >= ALLSKILLS ) || ( packet->skillValue1() > 50 ) ||
		( packet->skillId2() >= ALLSKILLS ) || ( packet->skillValue2() > 50 ) ||
		( packet->skillId3() >= ALLSKILLS ) || ( packet->skillValue3() > 50 ) ||
		( packet->skillValue1() + packet->skillValue2() + packet->skillValue3() > 100 ) )
	{
		QString failMessage = tr( "%1 is trying to create char with wrong skills: %1 [%2%] %3 [%4%] %5 [%6%]" ).arg( Accounts->findByNumber( _account ) ).arg( packet->skillId1() ).arg( packet->skillValue1() ).arg( packet->skillId2() ).arg( packet->skillValue2() ).arg( packet->skillId3() ).arg( packet->skillValue3() );
		clConsole.send( failMessage );
		cancelCreate( tr("Invalid Character skills") )
	}

	// Check Hair
	if( packet->hairStyle() && ( !isHair( packet->hairStyle() ) || !isHairColor( packet->hairColor() ) ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong hair %2 [Color: %3]" ).arg( Accounts->findByNumber( _account ) ).arg( packet->hairStyle() ).arg( packet->hairColor() ) );
		cancelCreate( tr("Invalid hair") )
	}

	// Check Beard
	if( packet->beardStyle() && ( !isBeard( packet->beardStyle() ) || !isHairColor( packet->beardColor() ) ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong beard %2 [Color: %3]" ).arg( Accounts->findByNumber( _account ) ).arg( packet->beardStyle() ).arg( packet->beardColor() ) );
		cancelCreate( tr("Invalid beard") )
	}

	// Check color for pants and shirt
	if( !isNormalColor( packet->shirtColor() ) || !isNormalColor( packet->pantsColor() ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong shirt [%2] or pant [%3] color" ).arg( Accounts->findByNumber( _account ) ).arg( packet->shirtColor() ).arg( packet->pantsColor() ) );
		cancelCreate( tr("Invalid shirt or pant color") )
	}

	// Check the start location
	vector< StartLocation_st > startLocations = SrvParams->startLocation();
	if( packet->startTown() > startLocations.size() )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong start location: %2" ).arg( Accounts->findByNumber( _account ) ).arg( packet->startTown() ) );
		cancelCreate( tr("Invalid start location") )
	}

	// Finally check the skin
	if( !isSkinColor( packet->skinColor() ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong skin color: %2" ).arg( Accounts->findByNumber( _account ) ).arg( packet->skinColor() ) );
		cancelCreate( tr("Invalid skin color") )
	}

	// FINALLY create the char
	P_CHAR pChar = new cChar;
	pChar->Init();
	
	pChar->setPriv( SrvParams->defaultpriv1() );
	pChar->priv2 = SrvParams->defaultpriv2();

	if( _account == 0 )
	{
		pChar->setPriv( 0xE7 );
		pChar->setPrivLvl("admin");
		pChar->setMenupriv( -1 );
	}
	else
		pChar->setPrivLvl( "player" );

	pChar->name = packet->name().latin1();
	
	pChar->setSkin( packet->skinColor() );
	pChar->setXSkin( packet->skinColor() );

	pChar->moveTo( startLocations[ packet->startTown() ].pos );
	pChar->dir = 4;

	pChar->setId( ( packet->gender() == 1 ) ? 0x191 : 0x190 );
	pChar->xid = pChar->id();

	pChar->st = packet->strength();
	pChar->hp = pChar->st;

	pChar->setDex( packet->dexterity() );
	pChar->stm = pChar->effDex();

	pChar->in = packet->intelligence();
	pChar->mn = pChar->in;

	pChar->setBaseSkill( packet->skillId1(), packet->skillValue1() );
	pChar->setBaseSkill( packet->skillId2(), packet->skillValue2() );
	pChar->setBaseSkill( packet->skillId3(), packet->skillValue3() );
	cCharsManager::getInstance()->registerChar( pChar );

	// Create the char equipment (JUST the basics !!)
	P_ITEM pItem = new cItem;
	pItem->Init();

	// Shirt
	pItem->setId( 0x1517 );
	pItem->setLayer( 0x05 );
	pItem->setColor( packet->shirtColor() );
	pItem->setContSerial( pChar->serial );
	pItem->dye = 1;
	pItem->priv |= 0x02;
	cItemsManager::getInstance()->registerItem( pItem );

	pItem = new cItem;
	pItem->Init();

	// Skirt or Pants
	pItem->setId( ( packet->gender() != 0 ) ? 0x1516 : 0x152E );
	pItem->setLayer( 0x04 );
	pItem->setColor( packet->pantsColor() );
	pItem->setContSerial( pChar->serial );
	pItem->dye = 1;
	pItem->priv |= 0x02;
	cItemsManager::getInstance()->registerItem( pItem );

	// Hair & Beard
	if( packet->hairStyle() )
	{
		pItem = new cItem;
		pItem->Init();

		pItem->dye = 1;
		pItem->priv |= 0x02;
		pItem->setId( packet->hairStyle() );
		pItem->setLayer( 11 );
		pItem->setColor( packet->hairColor() );
		pItem->setContSerial( pChar->serial );
		cItemsManager::getInstance()->registerItem( pItem );
	}

	if( packet->beardStyle() )
	{
		pItem = new cItem;
		pItem->Init();

		pItem->setId( packet->beardStyle() );
		pItem->priv |= 0x02;
		pItem->setLayer( 16 );
		pItem->setColor( packet->beardColor() );
		pItem->setContSerial( pChar->serial );
		cItemsManager::getInstance()->registerItem( pItem );
	}

	// Backpack + Bankbox autocreate
	pItem = pChar->getBankBox();
	pItem = pChar->getBackpack();
	
	pChar->setAccount( _account );
	_player = pChar;

	giveNewbieItems( packet );
	
	// Start the game with the newly created char -- OR RELAY HIM !!
    playChar();

	/*cUOTxRelayServer relayServer;
	QHostAddress hostAddress;
	hostAddress.setAddress( "127.0.0.1" );
	relayServer.setServerIp( hostAddress.ip4Addr() );
	relayServer.setServerPort( 2593 );
	send( &relayServer );*/
}

void cUOSocket::giveNewbieItems( cUORxCreateChar *packet, Q_UINT8 skill ) 
{
	QDomElement *startItems = DefManager->getSection( WPDT_STARTITEMS, ( skill = 0xFF ) ? "default" : QString::number( skill ) );

	// No Items defined
	if( !startItems )
		return;

	// Just one type of node: item
	QDomElement node = startItems->firstChild().toElement();

	while( !node.isNull() )
	{
		if( node.nodeName() == "item" )
		{
			P_ITEM pItem = Items->createScriptItem( node.attribute( "id" ) );

			if( pItem )
			{
				pItem->applyDefinition( node );
				// Put it into the backpack
				P_ITEM backpack = _player->getBackpack();
				backpack->AddItem( pItem );
			}
		}
		else if( node.nodeName() == "bankitem" )
		{
			P_ITEM pItem = Items->createScriptItem( node.attribute( "id" ) );

			if( pItem )
			{
				pItem->applyDefinition( node );
				// Put it into the bankbox
				P_ITEM bankbox = _player->getBankBox();
				bankbox->AddItem( pItem );
			}
		}
		else if( node.nodeName() == "equipment" )
		{
			P_ITEM pItem = Items->createScriptItem( node.attribute( "id" ) );

			if( pItem )
			{
				pItem->applyDefinition( node );
				// Put it onto the char
				pItem->setContSerial( _player->serial );
				_player->giveItemBonus( pItem );
			}
		}

		node = node.nextSibling().toElement();
	}
}

void cUOSocket::sysMessage( const QString &message, Q_UINT16 color )
{
	// Color: 0x0037
	cUOTxUnicodeSpeech speech;
	speech.setSource( 0xFFFFFFFF );
	speech.setModel( 0xFFFF );
	speech.setFont( 3 );
	speech.setLanguage( "ENU" ); // Standard server language >> Change later
	speech.setColor( color );
	speech.setType( cUOTxUnicodeSpeech::System );
	speech.setName( "System" );
	speech.setText( message );

	clConsole.send( cUOPacket::dump( speech.uncompressed() ) );

	send( &speech );
}

void cUOSocket::updateCharList()
{
	cUOTxUpdateCharList charList;
	vector< P_CHAR > characters = Accounts->characters( _account );

	// Add the characters
	for( Q_UINT8 i = 0; i < characters.size(); ++i )
		charList.setCharacter( i, characters[ i ]->name.c_str() );

	send( &charList );
}

// Sends either a stat or a skill packet
void cUOSocket::handleQuery( cUORxQuery *packet )
{
	P_CHAR pChar = FindCharBySerial( packet->serial() );

	if( !pChar )
		return;

	// Skills of other people can only be queried as a gm
	if( packet->type() == cUORxQuery::Skills )
	{
		if( ( pChar != _player ) && !_player->isGM() )
			return;

		// Send a full skill update
		cUOTxSendSkills skillList;
		skillList.fromChar( pChar );
		send( &skillList );
	}
	else if( packet->type() == cUORxQuery::Stats )
	{
		// For other chars we only send the basic stats
		cUOTxSendStats sendStats;
		sendStats.setAllowRename( _player->Owns( pChar ) || _player->isGM() );
		
		sendStats.setMaxHp( 100 );
		sendStats.setHp( (pChar->hp/pChar->st)*100 );

		sendStats.setName( pChar->name.c_str() );
		sendStats.setSerial( pChar->serial );
		
		sendStats.setFullMode( pChar == _player );

		// Set the rest - and reset if nec.
		if( pChar == _player )
		{
			sendStats.setHp( pChar->hp );
			sendStats.setMaxHp( pChar->st );
			sendStats.setStrength( pChar->st );
			sendStats.setDexterity( pChar->effDex() );
			sendStats.setIntelligence( pChar->in );
		}

		send( &sendStats );
	}
}

void cUOSocket::handleUpdateRange( cUORxUpdateRange *packet )
{
	clConsole.send( cUOPacket::dump( packet->uncompressed() ) );
	_viewRange = packet->range();
}

void cUOSocket::handleRequestLook( cUORxRequestLook *packet )
{
	// Check if it's a singleclick on items or chars
	if( isCharSerial( packet->serial() ) )
	{
		P_CHAR pChar = FindCharBySerial( packet->serial() );

		if( !pChar )
			return;

		pChar->showName( this );
	}
	else
	{
		P_ITEM pItem = FindItemBySerial( packet->serial() );

		if( !pItem )
			return;
	}
}

void cUOSocket::handleRequestUse( cUORxRequestUse *packet )
{
}

void cUOSocket::handleMultiPurpose( cUORxMultiPurpose *packet )
{
	switch( packet->subCommand() )
	{
	case 0x13:
		handleContextMenuRequest( dynamic_cast< cUORxContextMenuRequest* >( packet ) ); break;
	default:
		packet->print( &cout ); // Dump the packet
	};
}

// Show a context menu
void cUOSocket::handleContextMenuRequest( cUORxContextMenuRequest *packet )
{
	// The dumps below didn't have ANY effect so it's removed for now

	// Send a dummy popup menu
	/*cUOTxContextMenu menu;
	menu.addEntry( 0x17EB, 0x0001 );
	send( &menu );*/

	/*const char *pData = "\xbf\x00\x2c\x00\x14\x00\x01\x00\x20\x65\xb9\x05\x00\x0a\x17\xeb\x00\x20\xff\xff\x01\x93\x18\x08\x00\x00\x00\x6e\x17\xd7\x00\x00\x00\x6f\x17\xd8\x00\x00\x00\xcf\x17\x77\x00\x01";
	//const char *pData = "\xbf\x00\x1a\x00\x14\x00\x01\x01\x88\xad\x4b\x02\x00\x0a\x17\xeb\x00\x20\xff\xff\x01\x2e\x18\x01\x00\x00";
	QByteArray data( 0x2c );
	memcpy( data.data(), pData, 0x2c );
	cUOPacket cPacket( data );
	send( &cPacket );*/

	//0000: bf 00 2c 00 14 00 01 00 20 65 b9 05 00 0a 17 eb : ..,..... e......
	//0010: 00 20 ff ff 01 93 18 08 00 00 00 6e 17 d7 00 00 : . .........n....
	//0020: 00 6f 17 d8 00 00 00 cf 17 77 00 01 -- -- -- -- : .o.......w..

	//0000: bf 00 1a 00 14 00 01 01 88 ad 4b 02 00 0a 17 eb : ..........K.....
	//0010: 00 20 ff ff 01 2e 18 01 00 00 -- -- -- -- -- -- : . ........

}

void cUOSocket::showSpeech( cUObject *object, const QString &message, Q_UINT16 color, Q_UINT16 font, cUOTxUnicodeSpeech::eSpeechType speechType )
{
	cUOTxUnicodeSpeech speech;
	speech.setSource( object->serial );
	speech.setName( object->name.c_str() );
	speech.setFont( font );
	speech.setColor( color );
	speech.setText( message );
	speech.setType( speechType );
	send( &speech );
}

void cUOSocket::handleWalkRequest( cUORxWalkRequest* packet )
{
//	cUOPacket moveOk(0x22, 3);
	Movement->Walking( this->_player, packet->direction(), packet->key());
//	moveOk[1] = packet->key();
//	send( &moveOk );
}
