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
#include "../corpse.h"
#include "../accounts.h"
#include "../globals.h"
#include "../junk.h"
#include "../territories.h"
#include "../regions.h"
#include "../structs.h"
#include "../speech.h"
#include "../commands.h"
#include "../classes.h"
#include "../srvparams.h"
#include "../wpdefmanager.h"
#include "../walking.h"
#include "../speech.h"
#include "../guildstones.h"
#include "../combat.h"
#include "../books.h"
#include "../gumps.h"
#include "../skills.h"

//#include <conio.h>
#include <iostream>
#include <stdlib.h>

using namespace std;


/*****************************************************************************
  cUOSocket member functions
 *****************************************************************************/

/*!
  \class cUOSocket uosocket.h

  \brief The cUOSocket class provides an abstraction of Ultima Online network 
  connected sockets. It works with Application level packets provided by \sa cAsyncNetIO.

  \ingroup network
  \ingroup mainclass
*/

/*!
  Constructs a cUOSocket attached to \a sDevice system socket.
  Ownership of \a sDevice will be transfered to cUOSocket, that is,
  cUOSocket will call delete on the given pointer when it's destructed.
*/
cUOSocket::cUOSocket( QSocketDevice *sDevice ): 
		_walkSequence( 0xFF ), lastPacket( 0xFF ), _state( LoggingIn ), _lang( "ENU" ),
		targetRequest(0), _account(0), _player(0), _rxBytes(0), _txBytes(0), _socket( sDevice )
{
}

/*!
  Destructs the cUOSocket instance.
*/
cUOSocket::~cUOSocket(void)
{
	delete _socket;
	delete targetRequest;

	std::map< SERIAL, cGump* >::iterator it = gumps.begin();
	while( it != gumps.end() )
	{
		delete it->second;
		it++;
	}
}

/*!
  Sends \a packet to client.
*/
void cUOSocket::send( cUOPacket *packet )
{
	// Don't send when we're already disconnected
	if( !_socket || !_socket->isOpen() )
		return;

	cNetwork::instance()->netIo()->sendPacket( _socket, packet, ( _state != LoggingIn ) );
}

/*!
  Sends \a gump to client.
*/
void cUOSocket::send( cGump *gump )
{
	if( gump->serial() == INVALID_SERIAL )
	{
		while( gump->serial() == INVALID_SERIAL || ( gumps.find( gump->serial() ) != gumps.end() ) )
			gump->setSerial( RandomNum( 1, 0xDDDDDDDC ) ); 
		// randomnum takes int not uint... if 2nd val is above 0xdddddddd
		// it will interpret it as negative... internally it checks
		// if 1st < 2nd val, so it ended up in an infinite loop as soon
		// as 2 gumps were in the queue
		// plz dont touch this or replace it with 0xFFFFFFFE (= -2) again!
	}
	// Remove/Timeout the old one first
	else if( gumps.find( gump->serial() ) != gumps.end() )
	{
		cGump *pGump = gumps.find( gump->serial() )->second;
		if( pGump )
		{
			gumps.erase( gumps.find( pGump->serial() ) );
			delete pGump;
		}
	}

	gumps.insert( make_pair( gump->serial(), gump ) );

	QString layout = gump->layout().join( "" );
	Q_UINT32 gumpsize = 21 + layout.length() + 2;
	QStringList text = gump->text();
	QStringList::const_iterator it = text.begin();
	while( it != text.end() )
	{
		gumpsize += (*it).length() * 2 + 2;
		it++;
	}
	cUOTxGumpDialog uoPacket( gumpsize );

	uoPacket.setSerial( gump->serial() );
	uoPacket.setType( gump->type() );
	uoPacket.setX( gump->x() );
	uoPacket.setY( gump->y() );
	uoPacket.setContent( layout, text );

	send( &uoPacket );
	//uoPacket.print( &cout ); // for debugging
}

/*!
  Tries to receive and dispatch a packet.
*/
void cUOSocket::recieve()
{
	cUOPacket *packet = cNetwork::instance()->netIo()->recvPacket( _socket );

	if( !packet )
		return;

	Q_UINT8 packetId = (*packet)[0];

	// After two pings we idle-disconnect
	if( lastPacket == 0x73 && packetId == 0x73 )
	{
		clConsole.send( QString( "Socket idle-kicked [%1]" ).arg( _socket->peerAddress().toString() ) );
		disconnect();
		return;
	}

	// Disconnect harmful clients
	if( ( _account < 0 ) && ( packetId != 0x80 ) && ( packetId != 0x91 ) )
	{
		clConsole.send( QString( "Communication Error [%1 instead of 0x80|0x91] [%2]\n" ).arg( packetId, 2, 16 ).arg( _socket->peerAddress().toString() ) );
		cUOTxDenyLogin denyLogin;
		denyLogin.setReason( DL_BADCOMMUNICATION );
		send( &denyLogin );
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
	case 0x05:
		handleRequestAttack( dynamic_cast< cUORxRequestAttack*> (packet) );
		break;
	case 0x06:
		handleDoubleClick( dynamic_cast< cUORxDoubleClick*>( packet ) );
		break;
	case 0x07:
		cDragItems::getInstance()->grabItem( this, dynamic_cast< cUORxDragItem* >( packet ) ); break;
	case 0x08:
		cDragItems::getInstance()->dropItem( this, dynamic_cast< cUORxDropItem* >( packet ) ); break;
	case 0x09:
		handleRequestLook( dynamic_cast< cUORxRequestLook* >( packet ) ); break;
	case 0x12:
		handleAction( dynamic_cast< cUORxAction* >( packet ) ); break;
	case 0x13:
		cDragItems::getInstance()->equipItem( this, dynamic_cast< cUORxWearItem* >( packet ) ); break;
	case 0x22:
		resync(); break;
	case 0x2C:
		/* Resurrection menu */ break;
	case 0x34:
		handleQuery( dynamic_cast< cUORxQuery* >( packet ) ); break;
	case 0x3A:
		handleSkillLock( dynamic_cast< cUORxSkillLock* >( packet ) ); break;
	case 0x3B:
		Trade->buyaction( this, dynamic_cast< cUORxBuy* >( packet ) ); break;
	case 0x5D:
		handlePlayCharacter( dynamic_cast< cUORxPlayCharacter* >( packet ) ); break;
	case 0x66:
		handleBookPage( dynamic_cast< cUORxBookPage* >( packet ) ); break;
	case 0x6c:
		handleTarget( dynamic_cast< cUORxTarget* >( packet ) ); break;
	case 0x72:
		handleChangeWarmode( dynamic_cast< cUORxChangeWarmode* >( packet ) ); break;
	case 0x73:
		break; // Pings are handeled
	case 0x80:
		handleLoginRequest( dynamic_cast< cUORxLoginRequest* >( packet ) ); break;
	case 0x83:
		handleDeleteCharacter( dynamic_cast< cUORxDeleteCharacter* >( packet ) ); break;
	case 0x91:
		handleServerAttach( dynamic_cast< cUORxServerAttach* >( packet ) ); break;
	case 0x93:
		handleUpdateBook( dynamic_cast< cUORxUpdateBook* >( packet ) ); break;
	case 0x9B:
		handleHelpRequest( dynamic_cast< cUORxHelpRequest* >( packet ) ); break;
	case 0xA0:
		handleSelectShard( dynamic_cast< cUORxSelectShard* >( packet ) ); break;
	case 0xA4:
		handleHardwareInfo( dynamic_cast< cUORxHardwareInfo* >( packet ) ); break;
	case 0xAD:
		handleSpeechRequest( dynamic_cast< cUORxSpeechRequest* >( packet ) ); break;
	case 0xB1:
		handleGumpResponse( dynamic_cast< cUORxGumpResponse* >( packet ) ); break;
	case 0xBD:
		_version = dynamic_cast< cUORxSetVersion* >( packet )->version(); break;
	case 0xBF:
		handleMultiPurpose( dynamic_cast< cUORxMultiPurpose* >( packet ) ); break;
	case 0xC8:
		handleUpdateRange( dynamic_cast< cUORxUpdateRange* >( packet ) ); break;
	default:
		//cout << "Recieved packet: " << endl;
		packet->print( &cout );
		break;
	}
	
	delete packet;
}

/*!
  This method handles cUORxLoginRequest packet types.
  \sa cUORxLoginRequest
*/
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
		shardList->addServer( i, shards[i].sServer, 0, shards[i].uiTime, shards[i].ip );
	
	send( shardList );
	delete shardList;
}

/*!
  This method handles cUORxHardwareInfo packet types.
  \sa cUORxHardwareInfo
*/
void cUOSocket::handleHardwareInfo( cUORxHardwareInfo *packet )
{
	// Do something with the retrieved hardware information here
	// > Hardware Log ??
	//QString hardwareMsg = QString( "Hardware: %1 Processors [Type: %2], %2 MB RAM, %3 MB Harddrive" ).arg( packet->processorCount() ).arg( packet->processorType() ).arg( packet->memoryInMb() ).arg( packet->largestPartitionInMb() );
	//cout << hardwareMsg.latin1() << endl;
}

/*!
  This method provides a way of interrupting the client's connection
  to this server.
*/
void cUOSocket::disconnect( void )
{
	if( _account )
		_account->setInUse( false );

	if( _player )
		_player->setSocket( NULL );

	cNetwork::instance()->netIo()->flush( _socket );
	_socket->close();
}

/*!
  This method handles cUORxSelectShard packet types.
  \sa cUORxSelectShard
*/
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
	relay->setServerIp( shards[ packet->shardId() ].ip );
	relay->setServerPort( shards[ packet->shardId() ].uiPort );
	relay->setAuthId( _uniqueId );
	send( relay );
	delete relay;
}

/*!
  This method handles cUORxServerAttach packet types.
  \sa cUORxServerAttach
*/
void cUOSocket::handleServerAttach( cUORxServerAttach *packet )
{
	// Re-Authenticate the user !!
	if( !_account && !authenticate( packet->username(), packet->password() ) )
		disconnect();
	else
	{
		sendCharList();
		_account->setInUse( true );
	}
}

/*!
  This method sends the list of Characters this account have
  during the login process. 
  \sa cUOTxCharTownList
*/
void cUOSocket::sendCharList()
{
	cUOTxCharTownList *charList = new cUOTxCharTownList;
	QValueVector< cChar* > characters = _account->caracterList();

	// Add the characters
	Q_UINT8 i = 0;
	for(; i < characters.size(); ++i )
		charList->addCharacter( characters.at(i)->name.c_str() );

	// Add the Starting Locations
	vector< StartLocation_st > startLocations = SrvParams->startLocation();
	for( i = 0; i < startLocations.size(); ++i )
		charList->addTown( i, startLocations[i].name, startLocations[i].name );

	charList->compile();
	send( charList );
	delete charList;
}

/*!
  This method handles cUORxDeleteCharacter packet types.
  It will also resend the updated character list
  \sa cUORxDeleteCharacter
  \todo Implement delete code to cUOSocket::handleDeleteCharacter
*/
void cUOSocket::handleDeleteCharacter( cUORxDeleteCharacter *packet )
{
	QValueVector< P_CHAR > charList = _account->caracterList();

	if( packet->index() >= charList.size() )
	{
		cUOTxDenyLogin dLogin;
		dLogin.setReason( DL_BADCOMMUNICATION );
		send( &dLogin );
		return;
	}

	P_CHAR pChar = charList[ packet->index() ];

	if( pChar )
	{
		Npcs->DeleteChar( pChar ); // Does everything for us
		_account->removeCharacter( pChar );
	}

	updateCharList();
}

/*!
  This method handles cUORxPlayCharacter packet types.
  \sa cUORxPlayCharacter
*/
void cUOSocket::handlePlayCharacter( cUORxPlayCharacter *packet )
{
	// Check the character the user wants to play
	QValueVector< cChar* > characters = _account->caracterList();

	if( packet->slot() >= characters.size() )
	{
		cUOTxDenyLogin denyLogin;
		denyLogin.setReason( DL_BADCOMMUNICATION );
		send( &denyLogin );
		return;
	}

	playChar( characters.at(packet->slot()) );
}

// Set up the neccesary stuff to play
void cUOSocket::playChar( P_CHAR pChar )
{
	if( !pChar )
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
	setPlayer( pChar );
	resendWorld( false );
	pChar->resend( false ); // Send us to others
}

bool cUOSocket::authenticate( const QString &username, const QString &password )
{
	// Log
	clConsole.send( QString( "Trying to log in as %1 using password %2 [%3]\n" ).arg( username ).arg( password ).arg( _socket->peerAddress().toString() ) );

	cAccounts::enErrorCode error = cAccounts::NoError;
	AccountRecord* authRet = Accounts->authenticate( username, password, &error );

	// Reject login
	if( error != cAccounts::NoError )
	{
		cUOTxDenyLogin denyPacket;

		switch( error )
		{
		case cAccounts::LoginNotFound:
			if ( SrvParams->autoAccountCreate() )
			{
				authRet = Accounts->createAccount( username, password );
				_account = authRet;
				return true;
			}
			else
				denyPacket.setReason( DL_NOACCOUNT );
			break;
		case cAccounts::BadPassword:
			denyPacket.setReason( DL_BADPASSWORD ); break;
		case cAccounts::Wipped:
		case cAccounts::Banned:
			denyPacket.setReason( DL_BLOCKED ); break;
		case cAccounts::AlreadyInUse:
			denyPacket.setReason( DL_INUSE ); break;
		};

		clConsole.send( QString( "Bad Authentication [%1]\n" ).arg( _socket->peerAddress().toString() ) );
		send( &denyPacket );
	}

	_account = authRet;

	return ( error == cAccounts::NoError );
}

// Processes a create character request
// Notes from Lord Binaries packet documentation:
#define cancelCreate( message ) cUOTxDenyLogin denyLogin; denyLogin.setReason( DL_BADCOMMUNICATION ); send( &denyLogin ); sysMessage( message ); disconnect(); return;

/*!
  This method handles cUORxCreateChar packet types.
  \sa cUORxCreateChar
*/
void cUOSocket::handleCreateChar( cUORxCreateChar *packet )
{
	// Several security checks
	QValueVector<cChar*> characters = _account->caracterList();

    // If we have more than 5 characters
	if( characters.size() >= 5 )
	{
		clConsole.send( tr( "%1 is trying to create char but has more than 5 characters" ).arg( _account->login() ) );
		cancelCreate( tr("You already have more than 5 characters") )
	}

	// Check the stats
	Q_UINT16 statSum = ( packet->strength() + packet->dexterity() + packet->intelligence() );
	if( statSum > 80 )
	{
		clConsole.send( tr( "%1 is trying to create char with wrong stats: %2" ).arg( _account->login() ).arg( statSum ) );
		cancelCreate( tr( "Invalid Character stat sum: %1" ).arg( statSum ) )
	}

	// Every stat needs to be below 60
	if( ( packet->strength() > 60 ) || ( packet->dexterity() > 60 ) || ( packet->intelligence() > 60 ) )
	{
		clConsole.send( tr( "%1 is trying to create char with wrong stats: %2 [str] %3 [dex] %4 [int]" ).arg( _account->login() ).arg( packet->strength() ).arg( packet->dexterity() ).arg( packet->intelligence() ) );
		cancelCreate( tr("Invalid Character stats") )
	}

	// Check the skills
	if( ( packet->skillId1() >= ALLSKILLS ) || ( packet->skillValue1() > 50 ) ||
		( packet->skillId2() >= ALLSKILLS ) || ( packet->skillValue2() > 50 ) ||
		( packet->skillId3() >= ALLSKILLS ) || ( packet->skillValue3() > 50 ) ||
		( packet->skillValue1() + packet->skillValue2() + packet->skillValue3() > 100 ) )
	{
		QString failMessage = tr( "%1 is trying to create char with wrong skills: %1 [%2%] %3 [%4%] %5 [%6%]" ).arg( _account->login() ).arg( packet->skillId1() ).arg( packet->skillValue1() ).arg( packet->skillId2() ).arg( packet->skillValue2() ).arg( packet->skillId3() ).arg( packet->skillValue3() );
		clConsole.send( failMessage );
		cancelCreate( tr("Invalid Character skills") )
	}

	// Check Hair
	if( packet->hairStyle() && ( !isHair( packet->hairStyle() ) || !isHairColor( packet->hairColor() ) ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong hair %2 [Color: %3]" ).arg( _account->login() ).arg( packet->hairStyle() ).arg( packet->hairColor() ) );
		cancelCreate( tr("Invalid hair") )
	}

	// Check Beard
	if( packet->beardStyle() && ( !isBeard( packet->beardStyle() ) || !isHairColor( packet->beardColor() ) ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong beard %2 [Color: %3]" ).arg( _account->login() ).arg( packet->beardStyle() ).arg( packet->beardColor() ) );
		cancelCreate( tr("Invalid beard") )
	}

	// Check color for pants and shirt
	if( !isNormalColor( packet->shirtColor() ) || !isNormalColor( packet->pantsColor() ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong shirt [%2] or pant [%3] color" ).arg( _account->login() ).arg( packet->shirtColor() ).arg( packet->pantsColor() ) );
		cancelCreate( tr("Invalid shirt or pant color") )
	}

	// Check the start location
	vector< StartLocation_st > startLocations = SrvParams->startLocation();
	if( packet->startTown() > startLocations.size() )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong start location: %2" ).arg( _account->login() ).arg( packet->startTown() ) );
		cancelCreate( tr("Invalid start location") )
	}

	// Finally check the skin
	if( !isSkinColor( packet->skinColor() ) )
	{
		clConsole.send( tr( "%1 is trying to create a char with wrong skin color: %2" ).arg( _account->login() ).arg( packet->skinColor() ) );
		cancelCreate( tr("Invalid skin color") )
	}

	// FINALLY create the char
	P_CHAR pChar = new cChar;
	pChar->Init();
	
	pChar->setPriv( SrvParams->defaultpriv1() );
	pChar->priv2 = SrvParams->defaultpriv2();

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

	pChar->setBaseSkill( packet->skillId1(), packet->skillValue1()*10 );
	pChar->setBaseSkill( packet->skillId2(), packet->skillValue2()*10 );
	pChar->setBaseSkill( packet->skillId3(), packet->skillValue3()*10 );
	Skills->updateSkillLevel( pChar, packet->skillId1() );
	Skills->updateSkillLevel( pChar, packet->skillId2() );
	Skills->updateSkillLevel( pChar, packet->skillId3() );

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
	giveNewbieItems( packet );
	
	Weight->NewCalc( pChar );

	// Start the game with the newly created char -- OR RELAY HIM !!
    playChar( pChar );
}

/*!
  This method handles cUORxHardwareInfo packet types creating the
  required newbie items.
  \sa cUORxHardwareInfo
*/
void cUOSocket::giveNewbieItems( cUORxCreateChar *packet, Q_UINT8 skill ) 
{
	QDomElement *startItems = DefManager->getSection( WPDT_STARTITEMS, ( skill == 0xFF ) ? QString("default") : QString::number( skill ) );

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

/*!
  This method sends a system \a message at the botton of the screen
  \sa cUOTxUnicodeSpeech
*/
void cUOSocket::sysMessage( const QString &message, Q_UINT16 color, UINT16 font )
{
	// Color: 0x0037
	cUOTxUnicodeSpeech speech;
	speech.setSource( 0xFFFFFFFF );
	speech.setModel( 0xFFFF );
	speech.setFont( font );
	speech.setLanguage( "ENU" ); // Standard server language >> Change later
	speech.setColor( color );
	speech.setType( cUOTxUnicodeSpeech::System );
	speech.setName( "System" );
	speech.setText( message );
	send( &speech );
}

void cUOSocket::updateCharList()
{
	cUOTxUpdateCharList charList;
	QValueVector<cChar*> characters = _account->caracterList();

	// Add the characters
	for( Q_UINT8 i = 0; i < characters.size(); ++i )
		charList.setCharacter( i, characters.at(i)->name.c_str() );

	send( &charList );
}

// Sends either a stat or a skill packet
/*!
  This method handles cUORxQuery packet types.
  \sa cUORxQuery
*/
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
		sendStatWindow( pChar );
	}
}

/*!
  This method handles cUORxUpdateRange packet types.
  \sa cUORxUpdateRange
*/
void cUOSocket::handleUpdateRange( cUORxUpdateRange *packet )
{
	if( packet->range() > 20 )
		return; // Na..

	//if( _player )
	//	_player->VisRange = packet->range();
}

/*!
  This method handles cUORxRequestLook packet types.
  \sa cUORxRequestLook
*/
void cUOSocket::handleRequestLook( cUORxRequestLook *packet )
{
	/*cUOTxPopupMenu popup;
	popup.setSerial( packet->serial() );
	popup.addEntry( 0, 1, true );
	popup.addEntry( 1, 2, false );
	popup.addEntry( 2, 3, false );
	send( &popup );
	return;*/

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
		pItem->showName( this );
	}
}

/*!
  This method handles cUORxMultiPorpuse packet types.
  \sa cUORxMultiPorpuse
*/
void cUOSocket::handleMultiPurpose( cUORxMultiPurpose *packet )
{
	cUOPacket *mPacket = packet->packet();

	switch( packet->subCommand() )
	{
	case 0x0B:
		handleSetLanguage( dynamic_cast< cUORxSetLanguage* >( mPacket ) ); break;
	case 0x13:
		handleContextMenuRequest( dynamic_cast< cUORxContextMenuRequest* >( mPacket ) ); break;
	default:
		packet->print( &cout ); // Dump the packet
	};
}

// Show a context menu
/*!
  This method handles cUORxContextMenuRequest packet types.
  \sa cUORxContextMenuRequest
*/
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

/*!
  This method prints \a message on top of \a object using the given \a color and \a speechType
  \sa cUObject, cUOTxUnicodeSpeech, cUOTxUnicodeSpeech::eSpeechType
*/
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

/*!
  This method sends an moviment acknowleadge allowing the client to move.
  \sa cUOTxAcceptMove.
*/
void cUOSocket::allowMove( Q_UINT8 sequence )
{
	cUOTxAcceptMove acceptMove;
	acceptMove.setSequence( sequence );
	send( &acceptMove );

	_walkSequence = ( sequence < 255 ) ? sequence : 0;
}

/*!
  This method informs the client that the requested movement is not permited.
  \sa cUOTxDenyMove.
*/
void cUOSocket::denyMove( Q_UINT8 sequence )
{
	cUOTxDenyMove deny;
	deny.fromChar( _player );
	deny.setSequence( sequence );
	send( &deny );

	_walkSequence = ( sequence < 255 ) ? sequence : 0;
}

/*!
  This method handles cUORxWalkRequest packet types.
  \sa cUORxWalkRequest
*/
void cUOSocket::handleWalkRequest( cUORxWalkRequest* packet )
{
	Movement->Walking( _player, packet->direction(), packet->key());
}

void cUOSocket::resendPlayer()
{
	if( !_player )
		return;

	cUOTxDrawPlayer drawPlayer;
	drawPlayer.fromChar( _player );
	send( &drawPlayer );

	// Reset the walking sequence
	_walkSequence = 0xFF;
}

void cUOSocket::updateChar( P_CHAR pChar )
{
	if( !_player )
		return;

	cUOTxUpdatePlayer updatePlayer;
	updatePlayer.fromChar( pChar );
	updatePlayer.setHighlight( pChar->notority( _player ) );
	send( &updatePlayer );
}

// Sends a foreign char including equipment
void cUOSocket::sendChar( P_CHAR pChar )
{
	if( !_player )
		return;

	// Then completely resend it
	cUOTxDrawChar drawChar;
	drawChar.fromChar( pChar );
	drawChar.setHighlight( pChar->notority( _player ) );
	send( &drawChar );
}

/*!
  This method handles cUORxSetLanguage packet types.
  \sa cUORxSetLanguage
*/
void cUOSocket::handleSetLanguage( cUORxSetLanguage* packet )
{
	_lang = packet->language();
}

void cUOSocket::setPlayer( P_CHAR pChar )
{
	// If we're already playing a char and chaning reset the socket status of that
	// player
	if( !pChar && !_player )
		return;

	// If the player is changing
	if( pChar && ( pChar != _player ) )
	{
		if( _player )
			_player->setSocket( NULL );

		_player = pChar;
		_player->setSocket( this );
	}

	resendPlayer(); // Set our location

	// Set the warmode status
	cUOTxWarmode warmode;
	warmode.setStatus( _player->war );
	send( &warmode );

	// Send our equipment
	vector< SERIAL > equipment = contsp.getData( _player->serial );
	for( Q_UINT32 i = 0; i < equipment.size(); ++i )
	{
		P_ITEM pItem = FindItemBySerial( equipment[i] );
		
		if( !pItem )
			continue;

		cUOTxCharEquipment cEquipment;
		cEquipment.fromItem( pItem );
		send( &cEquipment );
	}

	_state = InGame;
}

/*!
  This method handles cUORxSpeechRequest packet types.
  \sa cUORxSpeechRequest
*/
void cUOSocket::handleSpeechRequest( cUORxSpeechRequest* packet )
{
	if( !_player )
		return;

	// Check if it's a command, then dispatch it to the command system
	// if it's normal speech send it to the normal speech dispatcher
	QString speech = packet->message();
	UINT16 color = packet->color();
	UINT16 font = packet->font();
	UINT16 type = packet->type() & 0x3f; // Pad out the Tokenized speech flag

	// There is one special case. if the user has the body 0x3db and the first char
	// of the speech is = then it's always a command
	if( ( _player->id() == 0x3DB ) && speech.startsWith( SrvParams->commandPrefix() ) )
		cCommands::instance()->process( this, speech.right( speech.length()-1 ) );
	else if( speech.startsWith( SrvParams->commandPrefix() ) )
		cCommands::instance()->process( this, speech.right( speech.length()-1 ) );
	else
		Speech->talking( _player, speech, color, type );
}
	
/*!
  This method handles cUORxDoubleClick packet types.
  \sa cUORxDoubleClick
*/
void cUOSocket::handleDoubleClick( cUORxDoubleClick* packet )
{
	if ( isCharSerial(packet->serial() ) )
		showPaperdoll( this, FindCharBySerial( packet->serial() ), packet->keyboard() );
	else
	{
		dbl_click_item(this, packet->serial() );
	}
}

/*!
  This method handles cUORxGetTip packet types.
  \sa cUORxGetTip
*/
void cUOSocket::handleGetTip( cUORxGetTip* packet )
{
	if ( packet->isTip() )
	{
		tips( this, packet->lastTip() );
	}
}

void cUOSocket::sendPaperdoll( P_CHAR pChar )
{
	cUOTxOpenPaperdoll oPaperdoll;
	oPaperdoll.fromChar( pChar );
	send( &oPaperdoll );
}

/*!
  This method handles cUORxChangeWarmode packet types.
  \sa cUORxChangeWarmode
*/
void cUOSocket::handleChangeWarmode( cUORxChangeWarmode* packet )
{
	_player->targ = INVALID_SERIAL;
	_player->war = packet->warmode();

	cUOTxWarmode warmode;
	warmode.setStatus( packet->warmode() ? 1 : 0 );
	send( &warmode );

	playMusic();
	_player->disturbMed();

	if( _player->dead ) 
		_player->resend( false );
	else
		_player->update();
}

void cUOSocket::playMusic()
{
	if( !_player );
		return;

	cTerritory* Region = cAllTerritories::getInstance()->region( _player->region );
	UINT32 midi = 0;

	if( _player->war )
		midi = DefManager->getRandomListEntry( "MIDI_COMBAT" ).toInt();

	else if( Region )
		midi = DefManager->getRandomListEntry( Region->midilist() ).toInt();

	if( midi )
	{
		cUOTxPlayMusic pMusic;
		pMusic.setId( midi );
		send( &pMusic );
	}
}

void cUOSocket::sendContainer( P_ITEM pCont )
{
	if( !pCont )
		return;

	// Get the container gump
	UINT16 gump = 0x3D;

	switch( pCont->id() )
	{
		case 0x0E75:					// Backpack
		case 0x0E79:					// Box/Pouch
			gump = 0x3C; break;

		case 0x0E76:					// Leather Bag
			gump = 0x3D; break;

		case 0x0E77:					// Barrel
		case 0x0E7A:					// Square Basket
		case 0x0E7F:					// Keg
			gump = 0x3E; break;

		case 0x0E7C:					// Silver Chest
			gump = 0x4A; break;

		case 0x0E7D:					// Wooden Box
			gump = 0x43; break;

		case 0x0E3D:					// Large Wooden Crate
		case 0x0E3C:					// Large Wooden Crate
		case 0x0E3F:					// Small Wooden Crate
		case 0x0E3E:					// Small Wooden Crate
		case 0x0E7E:					// Wooden Crate
			gump = 0x44; break;

		case 0x0E80:					// Brass Box
			gump = 0x4B; break;

		case 0x0E40:					// Metal & Gold Chest
		case 0x0E41:					// Metal & Gold Chest
			gump = 0x42; break;

		case 0x0E43:					// Wooden & Gold chest
		case 0x0E42:					// Wooden & Gold Chest
			gump = 0x49; break;

		case 0x0990:					// Round Basket
			gump = 0x41; break;

		case 0x09B2:					// Backpack 2
			gump = 0x3C; break;

		case 0x09AA:					// Wooden Box
			gump = 0x43; break;

		case 0x09A8:					// Metal Box
			gump = 0x40; break;

		case 0x09AB:					// Metal/Silver Chest
			gump = 0x4A; break;

		case 0x09A9:					// Small Wooden Crate
			gump = 0x44; break;
			
		case 0x2006:					// Coffin
			gump = 0x09; break;

		case 0x0A97:					// Bookcase
		case 0x0A98:					// Bookcase
		case 0x0A99:					// Bookcase
		case 0x0A9a:					// Bookcase
		case 0x0A9b:					// Bookcase
		case 0x0A9c:					// Bookcase
		case 0x0A9d:					// Bookcase
		case 0x0A9e:					// Bookcase
			gump = 0x4d; break;

		case 0x0A4d:					// Fancy Armoire
		case 0x0A51:					// Fancy Armoire
		case 0x0A4c:					// Fancy Armoire
		case 0x0A50: 					// Fancy Armoire
			gump = 0x4e; break;

		case 0x0A4f:					// Wooden Armoire
		case 0x0A53:					// Wooden Armoire
		case 0x0A4e:					// Wooden Armoire
		case 0x0A52:					// Wooden Armoire
			gump = 0x4f; break;

		case 0x0A30:					// chest of drawers (fancy)
		case 0x0A38:					// chest of drawers (fancy)
			gump = 0x48; break;

		case 0x0A2c:					// chest of drawers (wood)
		case 0x0A34:					// chest of drawers (wood)
		case 0x0A3c:					// Dresser
		case 0x0A3d:					// Dresser
		case 0x0A44:					// Dresser
		case 0x0A35:					// Dresser
			gump = 0x51; break;

		case 0x0Ab2: 
			if( pCont->morex == 1 ) 
				gump = 0x4a;
			break;

		default:
			if( ( ( pCont->id() & 0xFF00 ) >> 8 ) == 0x3E )
				gump = 0x4C;
			break;
	}

	// Draw the container
	cUOTxDrawContainer dContainer;
	dContainer.setSerial( pCont->serial );
	dContainer.setGump( gump );
	send( &dContainer );

	// Add all items to the container
	cUOTxItemContent itemContent;

	vector< SERIAL > content = contsp.getData( pCont->serial );
	for( UINT32 i = 0; i < content.size(); ++i )
	{
		P_ITEM pItem = FindItemBySerial( content[ i ] );
		
		if( !pItem )
			continue;

		itemContent.addItem( pItem );
	}

	if( pCont->objectID() == "CORPSE" )
	{
		cCorpse *pCorpse = dynamic_cast< cCorpse* >( pCont );

		if( !pCorpse )
			return;

		if( pCorpse->hairStyle() )
		{
			itemContent.addItem( 0x4FFFFFFE, pCorpse->hairStyle(), pCorpse->hairColor(), 0, 0, 1, pCorpse->serial );
		}

		if( pCorpse->beardStyle() )
		{			
			itemContent.addItem( 0x4FFFFFFF, pCorpse->beardStyle(), pCorpse->beardColor(), 0, 0, 1, pCorpse->serial );
		}
	}

	send( &itemContent );
}

void cUOSocket::removeObject( cUObject *object )
{
	cUOTxRemoveObject rObject;
	rObject.setSerial( object->serial );
	send( &rObject );
}

// if flags etc. have changed
void cUOSocket::updatePlayer()
{
	if( !_player )
		return;

	cUOTxUpdatePlayer pUpdate;
	pUpdate.fromChar( _player );
	pUpdate.setHighlight( _player->notority( _player ) );
	send( &pUpdate );
}

// Do periodic stuff for this socket
void cUOSocket::poll()
{
	// TODO: check for timed out target requests herei
	if( targetRequest && ( targetRequest->timeout() > 1 ) && targetRequest->timeout() < uiCurrentTime )
	{
		targetRequest->timedout( this );
		delete targetRequest;
		targetRequest = 0;
	}
}

void cUOSocket::attachTarget( cTargetRequest *request )
{
	// Let the old one time out
	if( targetRequest )
	{
		targetRequest->timedout( this );
		delete targetRequest;
	}

	// attach the new one
	targetRequest = request;

	cUOTxTarget target;
	target.setTargSerial( 1 );
	target.setAllowGround( true ); // Not sure how to handle this
	send( &target );
}

void cUOSocket::attachTarget( cTargetRequest *request, UINT16 multiid )
{
	if( multiid < 0x4000 )
		return;

	if( targetRequest )
	{
		targetRequest->timedout( this );
		delete targetRequest;
	}

	targetRequest = request;

	cUOTxPlace target;
	target.setTargSerial( 1 );
	target.setModelID( 0x4000 - multiid );
	send( &target );
}

/*!
  This method handles cUORxTarget packet types.
  \sa cUORxTarget
*/
void cUOSocket::handleTarget( cUORxTarget *packet )
{
	if( !_player )
		return;
	
	if( !targetRequest )
		return;

	// Check if there really was a response or if it just was canceled
	if( !packet->serial() && ( ( packet->x() == 0xFFFF ) || ( packet->y() == 0xFFFF ) || ( (UINT8)packet->z() == 0xFF ) ) )
		targetRequest->timedout( this );
	else
	{
		if ( targetRequest->responsed( this, packet ) )
		{
			delete targetRequest;
			targetRequest = 0;
		}
		else
			attachTarget( targetRequest ); // Resend target.
	}

}

/*!
  This method handles cUORxRequestAttack packet types.
  \sa cUORxRequestAttack
*/
void cUOSocket::handleRequestAttack( cUORxRequestAttack* packet )
{
	// If we dont set any serial the attack is rejected
	cUOTxAttackResponse attack;

	P_CHAR pc_i = FindCharBySerial( packet->serial() );
	if( !pc_i ) 
	{
		send( &attack );
		return;
	}

	// No Fighting in jail
	if( _player->cell > 0 )
	{
		sysMessage( tr( "There is no fighting in the jail cells!" ) );
		send( &attack );
		return;
	}

	// Player is dead
	if( _player->dead )
	{
		if( SrvParams->persecute() )
		{
			_player->targ = pc_i->serial;
			if( _player->targ != INVALID_SERIAL ) 
				Skills->Persecute( this );
		} 
		else
			sysMessage( tr( "You are dead and cannot do that." ) );

		send( &attack );
		return;
	}

	// Attacking ghosts is not possible
	if( pc_i->dead || pc_i->hp <= 0 )
	{
		sysMessage( tr( "That person is already dead!" ) );
		send( &attack );
		return;
	}

	// Playervendors are invulnerable
	if( pc_i->npcaitype() == 17 ) 
	{
		sysMessage( tr( "%1 cannot be harmed." ).arg( pc_i->name.c_str() ) );
		send( &attack );
		return;
	}

	_player->targ = pc_i->serial;
	_player->unhide();
	_player->disturbMed();

	// Accept the attack
	attack.setSerial( pc_i->serial );
	send( &attack );

	// NPC already has a target
	if( pc_i->targ != INVALID_SERIAL )
	{
		pc_i->attacker = _player->serial;
		pc_i->resetAttackFirst();
	}

	_player->setAttackFirst();
	_player->attacker = pc_i->serial;
	_player->turnTo( pc_i );

	// The person being attacked is guarded by pets ?
	if( pc_i->guarded() )
	{
		RegionIterator4Chars cIter( pc_i->pos );
		for( cIter.Begin(); !cIter.atEnd(); cIter++ )
		{
			P_CHAR toCheck = cIter.GetData();
			if( pc_i->Owns( toCheck ) && toCheck->npcaitype() == 32 && toCheck->inRange( _player, 10 ) )
				npcattacktarget( toCheck, _player );
				// This was: npcattacktarget( _player, toCheck );
		}
	}

	if( pc_i->inGuardedArea() && SrvParams->guardsActive() )
	{
		if( pc_i->isPlayer() && pc_i->isInnocent() && GuildCompare( _player, pc_i ) == 0 ) //REPSYS
		{
			criminal( _player );
			Combat->SpawnGuard( _player, pc_i, _player->pos );
		}
		else if( pc_i->isNpc() && pc_i->isInnocent() && !pc_i->isHuman() && pc_i->npcaitype() != 4 )
		{
			criminal( _player );
			Combat->SpawnGuard( _player, pc_i, _player->pos );
		}
		else if( pc_i->isNpc() && pc_i->isInnocent() && pc_i->isHuman() && pc_i->npcaitype() != 4 )
		{
			pc_i->talk( "Help! Guards! I've been attacked!" );
			criminal( _player );
			callguards(pc_i);
		}
		else if( pc_i->isNpc() && pc_i->npcaitype() == 4 )
		{
			criminal( _player );
			npcattacktarget( pc_i, _player );
		}
		else if ((pc_i->isNpc() || pc_i->tamed()) && !pc_i->war && pc_i->npcaitype() != 4) // changed from 0x40 to 4, cauz 0x40 was removed LB
		{
			npcToggleCombat( pc_i );
			pc_i->setNextMoveTime();
		}
		else
		{
			pc_i->setNextMoveTime();
		}
	}
	else // not a guarded area
	{
		if( pc_i->isInnocent() )
		{
			if( pc_i->isPlayer() && GuildCompare( _player, pc_i ) == 0 )
			{
				criminal( _player );
			}
			else if( pc_i->isNpc() )
			{
				criminal( _player );
				npcattacktarget( pc_i, _player );

				if( !pc_i->tamed() && pc_i->isHuman() )
					pc_i->talk( tr( "Help! Guards! Tis a murder being commited!" ) );
			}
		}
	}

	// Send the "You see %1 attacking %2" string to all surrounding sockets
	// Except the one being attacked
	QString message = tr( "*You see %1 attacking %2!" ).arg(_player->name.c_str()).arg(pc_i->name.c_str());
	for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
		if( s->player() && s != this && s->player()->inRange( _player, s->player()->VisRange ) && s->player() != pc_i )
			s->showSpeech( _player, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );

	// Send an extra message to the victim
	if( pc_i->socket() )
		pc_i->socket()->showSpeech( _player, tr( "You see %1 attacking you" ).arg( _player->name.c_str() ), 0x26, 3, cUOTxUnicodeSpeech::Emote );
}

void cUOSocket::soundEffect( UINT16 soundId, cUObject *source )
{
	if( !_player )
		return;

	cUOTxSoundEffect sound;
	sound.setSound( soundId );
	
	if( !source )
		sound.setCoord( _player->pos );
	else
		sound.setCoord( source->pos );

	send( &sound );
}

void cUOSocket::resendWorld( bool clean )
{
	if( !_player )
		return;

	cUOTxRemoveObject rObject;
	cUOTxSendItem sendItem;

	RegionIterator4Items itIterator( _player->pos );
	for( itIterator.Begin(); !itIterator.atEnd(); itIterator++ )
	{
		P_ITEM pItem = itIterator.GetData();
		pItem->update( this );
	}

	RegionIterator4Chars chIterator( _player->pos );
	cUOTxDrawChar drawChar;

	for( chIterator.Begin(); !chIterator.atEnd(); chIterator++ )
	{
		P_CHAR pChar = chIterator.GetData();
		if( !pChar || pChar == _player )
			continue;

		if( pChar->pos.distance( _player->pos ) > _player->VisRange )
			continue;

		if( clean )
		{
			rObject.setSerial( pChar->serial );
			send( &rObject );
		}
		
		drawChar.fromChar( pChar );
		drawChar.setHighlight( pChar->notority( _player ) );
		send( &drawChar );
	}
}

void cUOSocket::resync()
{
	resendPlayer();
	sendChar( _player );
}

P_ITEM cUOSocket::dragging() const
{
	P_CHAR pChar = _player;

	if( !pChar )
		return 0;

	vector< SERIAL > equipment = contsp.getData( pChar->serial );

	for( UI32 i = 0; i < equipment.size(); i++ )
	{
		P_ITEM pItem = FindItemBySerial( equipment[ i ] );

		if( pItem && ( pItem->layer() == 0x1E ) )
			return pItem;
	}

	return 0;
}

void cUOSocket::bounceItem( P_ITEM pItem, eBounceReason reason )
{
	cUOTxBounceItem bounce;
	bounce.setReason( reason );
	send( &bounce );

	// Only bounce it back if it's on the hand of the char
	if( dragging() == pItem )
	{
		pItem->toBackpack( player() );
		
		if( pItem->isInWorld() )
		{
			// TODO: Send a drop-soundeffect if we were forced to drop it to ground
			for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
				mSock->soundEffect( 0x42, pItem );
		}
		else
			soundEffect( 0x57, pItem );

	}
}

void cUOSocket::updateStamina( P_CHAR pChar )
{
	if( !pChar )
		pChar = _player;

	if( !pChar )
		return;

	cUOTxUpdateStamina update;
	
	if( pChar == _player )
	{
		update.setMaximum( pChar->effDex() );
		update.setCurrent( pChar->stm );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( (UINT16)((pChar->stm/pChar->effDex())*100) );
	}

	send( &update );
}

void cUOSocket::updateMana( P_CHAR pChar )
{
	if( !pChar )
		pChar = _player;

	if( !pChar )
		return;

	cUOTxUpdateMana update;
	
	if( pChar == _player )
	{
		update.setMaximum( pChar->in );
		update.setCurrent( pChar->mn );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( (UINT16)((pChar->mn/pChar->in)*100) );
	}

	send( &update );
}

void cUOSocket::updateHealth( P_CHAR pChar )
{
	if( !pChar )
		pChar = _player;

	if( !pChar )
		return;

	cUOTxUpdateHealth update;
	
	if( pChar == _player )
	{
		update.setMaximum( pChar->st );
		update.setCurrent( pChar->hp );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( (UINT16)((pChar->hp/pChar->st)*100) );
	}

	send( &update );
}

void cUOSocket::sendStatWindow( P_CHAR pChar )
{
	if( !pChar )
		pChar = _player;

	if( !pChar )
		return;

	// For other chars we only send the basic stats
	cUOTxSendStats sendStats;
	sendStats.setAllowRename( _player->Owns( pChar ) || _player->isGM() );
	
	sendStats.setMaxHp( pChar->st );
	sendStats.setHp( pChar->hp );

	sendStats.setName( pChar->name.c_str() );
	sendStats.setSerial( pChar->serial );
		
	sendStats.setFullMode( pChar == _player );

	// Set the rest - and reset if nec.
	if( pChar == _player )
	{
		sendStats.setStamina( pChar->stm );
		sendStats.setMaxStamina( pChar->effDex() );
		sendStats.setMana( pChar->mn );
		sendStats.setMaxMana( pChar->in );
		sendStats.setStrength( pChar->st );
		sendStats.setDexterity( pChar->effDex() );
		sendStats.setIntelligence( pChar->in );
		sendStats.setWeight( pChar->weight() );
		sendStats.setGold( pChar->CountBankGold() + pChar->CountGold() );
		sendStats.setArmor( Combat->CalcDef( pChar, 0 ) ); // TODO: Inaccurate			
	}

	send( &sendStats );
}

bool cUOSocket::inRange( cUOSocket* socket ) const
{
	if ( !socket || !socket->player() || !_player )
		return false;
	return ( socket->player()->pos.distance( _player->pos ) < socket->player()->VisRange );
}

void cUOSocket::handleBookPage( cUORxBookPage* packet )
{
	cBook* pBook = dynamic_cast< cBook* >(FindItemBySerial( packet->serial() ));
	if( pBook )
	{
		if( packet->numOfLines() == (UINT16)-1 )
		{
			// simple page request
			pBook->readPage( this, packet->page() );
		}
		else if( pBook->writeable() )
		{
			// page write request
			QStringList content_ = pBook->content();
			QStringList lines = packet->lines();
			
			QString toInsert = QString();
			QStringList::const_iterator it = lines.begin();
			while( it != lines.end() )
			{
				toInsert += (*it);
				it++;
			}

			UINT16 n = packet->page();
			while( content_.size() < n )
				content_.push_back( "" );

			content_[ n - 1 ] = toInsert;
			pBook->setContent( content_ );
		}
	}	
}

void cUOSocket::handleUpdateBook( cUORxUpdateBook* packet )
{
	cBook* pBook = dynamic_cast< cBook* >(FindItemBySerial( packet->serial() ));
	if( pBook && pBook->writeable() )
	{
		pBook->setAuthor( packet->author() );
		pBook->setTitle( packet->title() );
	}
}

void cUOSocket::sendSkill( UINT16 skill )
{
	if( !_player )
		return;

	cUOTxUpdateSkill pUpdate;
	pUpdate.setId( skill );
	pUpdate.setValue( _player->skill( skill ) );
	pUpdate.setRealValue( _player->baseSkill( skill ) );
	send( &pUpdate );
}

void cUOSocket::handleAction( cUORxAction *packet )
{
	switch( packet->type() )
	{
	// Skill use
	case 0x24:
		{
			QStringList skillParts = QStringList::split( " ", packet->action() );
			if( skillParts.count() > 1 )
				Skills->SkillUse( this, skillParts[0].toInt() );
		}
		break;
	}
}

void cUOSocket::handleGumpResponse( cUORxGumpResponse* packet )
{
	if( gumps.find( packet->serial() ) == gumps.end() )
	{
		sysMessage( tr( "Unexpected button input" ) );
		return;
	}

	cGump* pGump = gumps.find( packet->serial() )->second;
	
	if( pGump )
	{
		pGump->handleResponse( this, packet->choice() );
		delete pGump;
		gumps.erase( gumps.find( packet->serial() ) );
	}
}

void cUOSocket::sendVendorCont( P_ITEM pItem )
{
	cUOTxItemContent itemContent;
	cUOTxVendorBuy vendorBuy;
	vendorBuy.setSerial( pItem->serial );

	vector< SERIAL > content = contsp.getData( pItem->serial );

	for( UINT32 i = 0; i < content.size(); ++i )
	{
		P_ITEM mItem = FindItemBySerial( content[i] );

		if( mItem )
		{
			if( mItem->restock <= 0 )
				continue;

			itemContent.addItem( mItem->serial, mItem->id(), mItem->color(), i, i, mItem->restock, pItem->serial );
			vendorBuy.addItem( mItem->value, mItem->getName() );
		}
	}

	send( &itemContent );
	send( &vendorBuy );
}

void cUOSocket::sendBuyWindow( P_CHAR pVendor )
{
	P_ITEM pBought = pVendor->GetItemOnLayer( 0x1C );
	P_ITEM pStock = pVendor->GetItemOnLayer( 0x1A );

	if( pBought )
		sendVendorCont( pBought );

	if( pStock )
		sendVendorCont( pStock );

	cUOTxDrawContainer drawContainer;
	drawContainer.setSerial( pVendor->serial );
	drawContainer.setGump( 0x30 );
	send( &drawContainer );
}

void cUOSocket::handleHelpRequest( cUORxHelpRequest* packet )
{
	cHelpGump* pGump = new cHelpGump( this->player()->serial );
	send( pGump );
}

void cUOSocket::handleSkillLock( cUORxSkillLock* packet )
{
	player()->lockSkill[ packet->skill() ] = packet->lock();
}
