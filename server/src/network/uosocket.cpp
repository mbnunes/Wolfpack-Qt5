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
#include "../mapobjects.h"
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
#include "../contextmenu.h"
#include "../TmpEff.h"
#include "../newmagic.h"

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
	_socket->resetStatus();
}

/*!
  Destructs the cUOSocket instance.
*/
cUOSocket::~cUOSocket(void)
{
	delete _socket;
	delete targetRequest;

	QMap< SERIAL, cGump* >::iterator it = gumps.begin();
	while( it != gumps.end() )
	{
		delete it.data();
		it++;
	}
}

/*!
  Sends \a packet to client.
*/
void cUOSocket::send( cUOPacket *packet ) const
{
	// Don't send when we're already disconnected
	if( !_socket || !_socket->isOpen() )
		return;

	cNetwork::instance()->netIo()->sendPacket( _socket, packet, ( _state != LoggingIn ) );

	// Once send, flush if in Debug mode
#if defined(_DEBUG)
	cNetwork::instance()->netIo()->flush( _socket );
#endif
}

/*!
  Sends \a gump to client.
*/
void cUOSocket::send( cGump *gump )
{
	if( gump->serial() == INVALID_SERIAL )
	{
		while( gump->serial() == INVALID_SERIAL || ( gumps.find( gump->serial() ) != gumps.end() ) )
			gump->setSerial( RandomNum( 0x10000000, 0x1000FFFF ) ); 
		// I changed this, everything between 0x10000000 and 0x1000FFFF is randomly generated)
	}
	// Remove/Timeout the old one first
	else if( gumps.find( gump->serial() ) != gumps.end() )
	{
		cGump *pGump = gumps.find( gump->serial() ).data();
		if( pGump )
		{
			gumps.erase( gumps.find( pGump->serial() ) );
			delete pGump;
		}
	}

	gumps.insert( gump->serial(), gump );

	QString layout = gump->layout().join( "" );
	Q_UINT32 gumpsize = 24 + layout.length();
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
		clConsole.send( QString( "[%1] Idle Disconnected" ).arg( _socket->peerAddress().toString() ) );
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
	case 0x75:
		handleRename( dynamic_cast< cUORxRename* >( packet ) ); break;
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
	case 0xA7:
		handleGetTip( dynamic_cast< cUORxGetTip* >( packet ) ); break;
	case 0xAD:
		handleSpeechRequest( dynamic_cast< cUORxSpeechRequest* >( packet ) ); break;
	case 0xB1:
		handleGumpResponse( dynamic_cast< cUORxGumpResponse* >( packet ) ); break;
	case 0xB8:
		handleProfile( dynamic_cast< cUORxProfile* >( packet ) ); break;
	case 0xBD:
		_version = dynamic_cast< cUORxSetVersion* >( packet )->version(); break;
	case 0xBF:
		handleMultiPurpose( dynamic_cast< cUORxMultiPurpose* >( packet ) ); break;
	case 0xC8:
		handleUpdateRange( dynamic_cast< cUORxUpdateRange* >( packet ) ); break;
	case 0x95:
		handleDye( dynamic_cast< cUORxDye* >( packet ) ); break;
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
	if( _account )
	{
		sendCharList();
		return;
	}
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
	{
		_player->onLogout();

		cUOSocket* mSocket = 0;
		QPtrListIterator<cUOSocket> it( cNetwork::instance()->getIterator() );
		while ( ( mSocket = it.current() ) )
		{
			++it;
			if ( mSocket == this || !SrvParams->joinMsg() || !mSocket->player() || !mSocket->player()->isGMorCounselor() )
				continue;
			mSocket->sysMessage( tr("%1 left the world!").arg( _player->name.latin1() ), 0x25 );
		}		
		_player->setSocket( NULL );
	}

	cNetwork::instance()->netIo()->flush( _socket );
	_socket->close();

	if( _player )
	{
		if( _player->region() && _player->region()->isGuarded() )
			_player->setHidden( 1 );
		else
		{
			cDelayedHideChar* pTmpEff = new cDelayedHideChar( _player->serial );
			pTmpEff->setExpiretime_s( SrvParams->quittime() );
			TempEffects::instance()->insert( pTmpEff );
		}
		_player->resend( true );
	}
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
		charList->addCharacter( characters.at(i)->name.latin1() );

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
		cCharStuff::DeleteChar( pChar ); // Does everything for us
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

	if( _account->inUse() )
	{
		cUOTxDenyLogin denyLogin;
		denyLogin.setReason( DL_INUSE );
		send( &denyLogin );
		return;
	}

	_account->setInUse( true );
	playChar( characters.at(packet->slot()) );

	if( _player )
		_player->onLogin();
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

	// We're now playing this char
	pChar->setHidden( 0 ); // Unhide us (logged out)
	setPlayer( pChar );

	// This needs to be sent once
	cUOTxConfirmLogin confirmLogin;
	confirmLogin.fromChar( pChar );
	confirmLogin.setUnknown3( 0x007f0000 );
	confirmLogin.setUnknown4( 0x00000007 );
	confirmLogin.setUnknown5( "\x60\x00\x00\x00\x00\x00\x00" );
	send( &confirmLogin );

	// Which map are we on
	cUOTxChangeMap changeMap;
	changeMap.setMap( pChar->pos.map );
	send( &changeMap );

	// Send us our player and send the rest to us as well.
	pChar->resend();
	resendWorld( false );

	cUOTxWarmode warmode;
	warmode.setStatus( pChar->war() );
	send( &warmode );

	if( pChar->targ() != INVALID_SERIAL )
	{
		cUOTxAttackResponse attack;
		attack.setSerial( pChar->targ() );
		send( &attack );
	}

	// Start the game!
	cUOTxStartGame startGame;
	send( &startGame );

	// Send the gametime
	cUOTxGameTime gameTime;
	gameTime.setTime( uoTime.time().hour(), uoTime.time().minute(), uoTime.time().second() );
	send( &gameTime );
	
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock != this && SrvParams->joinMsg() && mSock->player() && mSock->player()->isGMorCounselor() )
			mSock->sysMessage( tr("%1 entered the world!").arg( pChar->name.latin1() ), 0x48 );
}

/*!
	Checks the \a username and \a password pair. If accepted, initializes this socket
	to track that account, otherwise a proper cUOTxDenyLogin packet is sent. This method
	will create the account if SrvParams->autoAccountCreate() evaluates to true
	\sa cUOTxDenyLogin
*/
bool cUOSocket::authenticate( const QString &username, const QString &password )
{
	cAccounts::enErrorCode error = cAccounts::NoError;
	AccountRecord* authRet = Accounts::instance()->authenticate( username, password, &error );

	// Reject login
	if( !_account && error != cAccounts::NoError )
	{
		cUOTxDenyLogin denyPacket;

		switch( error )
		{
		case cAccounts::LoginNotFound:
			if ( SrvParams->autoAccountCreate() )
			{
				authRet = Accounts::instance()->createAccount( username, password );
				_account = authRet;
				
				clConsole.send( QString( "[%2] Account autocreated: '%1'\n" ).arg( username ).arg( _socket->peerAddress().toString() ) );
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

		clConsole.send( QString( "[%2] Failed to log in as '%1'\n" ).arg( username ).arg( _socket->peerAddress().toString() ) );
		send( &denyPacket );
	}
	else if( error == cAccounts::NoError )
	{
		clConsole.send( QString( "[%2] Identified as '%1'\n" ).arg( username ).arg( _socket->peerAddress().toString() ) );
	}

	_account = authRet;

	return ( error == cAccounts::NoError );
}

// Processes a create character request
// Notes from Lord Binaries packet documentation:
#define cancelCreate( message ) cUOTxDenyLogin denyLogin; denyLogin.setReason( DL_BADCOMMUNICATION ); send( &denyLogin ); sysMessage( message ); disconnect(); return;

/*!
  This method handles Character create request packet types.
  \sa cUORxCreateChar
*/
void cUOSocket::handleCreateChar( cUORxCreateChar *packet )
{
	// Several security checks
	if ( !_account )
	{
		clConsole.send( tr("%1 sent cUORxCreateChar without being authenticated, disconnecting. Probably a crypt client").arg(this->_socket->peerAddress().toString()) );
		this->_socket->close();
		return;
	}
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
		clConsole.send( tr( "%1 is trying to create char with wrong skills: %1 [%2%] %3 [%4%] %5 [%6%]" ).arg( _account->login() ).arg( packet->skillId1() ).arg( packet->skillValue1() ).arg( packet->skillId2() ).arg( packet->skillValue2() ).arg( packet->skillId3() ).arg( packet->skillValue3() ) );
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
	pChar->setPriv2( SrvParams->defaultpriv2() );

	pChar->name = packet->name().latin1();
	
	pChar->setSkin( packet->skinColor() );
	pChar->setXSkin( packet->skinColor() );

	pChar->moveTo( startLocations[ packet->startTown() ].pos );
	pChar->setDir(4);

	pChar->setId( ( packet->gender() == 1 ) ? 0x191 : 0x190 );
	pChar->setXid( pChar->id() );

	pChar->setSt( packet->strength() );
	pChar->setHp( pChar->st() );

	pChar->setDex( packet->dexterity() );
	pChar->setStm( pChar->effDex() );

	pChar->setIn( packet->intelligence() );
	pChar->setMn( pChar->in() );

	pChar->setBaseSkill( packet->skillId1(), packet->skillValue1()*10 );
	pChar->setBaseSkill( packet->skillId2(), packet->skillValue2()*10 );
	pChar->setBaseSkill( packet->skillId3(), packet->skillValue3()*10 );
	Skills->updateSkillLevel( pChar, packet->skillId1() );
	Skills->updateSkillLevel( pChar, packet->skillId2() );
	Skills->updateSkillLevel( pChar, packet->skillId3() );

	CharsManager::instance()->registerChar( pChar );

	// Create the char equipment (JUST the basics !!)
	P_ITEM pItem = new cItem;
	pItem->Init();

	// Shirt
	pItem->setId( 0x1517 );
	pItem->setColor( packet->shirtColor() );
	pItem->setMaxhp( RandomNum( 25, 50 ) );
	pItem->setHp( pItem->maxhp() );
	pChar->addItem( cChar::Shirt, pItem );	
	pItem->dye = 1;
	pItem->priv |= 0x02;
	ItemsManager::instance()->registerItem( pItem );

	pItem = new cItem;
	pItem->Init();

	// Skirt or Pants
	pItem->setId( ( packet->gender() != 0 ) ? 0x1516 : 0x152E );
	pItem->setColor( packet->pantsColor() );
	pItem->setMaxhp( RandomNum( 25, 50 ) );
	pItem->setHp( pItem->maxhp() );
	pChar->addItem( cChar::Pants, pItem );
	pItem->dye = 1;
	pItem->priv |= 0x02;
	ItemsManager::instance()->registerItem( pItem );

	// Hair & Beard
	if( packet->hairStyle() )
	{
		pItem = new cItem;
		pItem->Init();

		pItem->dye = 1;
		pItem->priv |= 0x02;
		pItem->setId( packet->hairStyle() );
		pItem->setColor( packet->hairColor() );
		pChar->addItem( cChar::Hair, pItem );
		ItemsManager::instance()->registerItem( pItem );
	}

	if( packet->beardStyle() )
	{
		pItem = new cItem;
		pItem->Init();

		pItem->setId( packet->beardStyle() );
		pItem->priv |= 0x02;
		pItem->setColor( packet->beardColor() );
		pChar->addItem( cChar::FacialHair, pItem );
		ItemsManager::instance()->registerItem( pItem );
	}

	// Backpack + Bankbox autocreate
	pItem = pChar->getBankBox();
	pItem = pChar->getBackpack();
	
	pChar->setAccount( _account );

	UI08 skillid = 0xFF;
	if( packet->skillValue1() > packet->skillValue2() )
	{
		if( packet->skillValue1() > packet->skillValue3() )
			skillid = packet->skillId1();
		else
			skillid = packet->skillId3();
	}
	else
	{
		if( packet->skillValue2() > packet->skillValue3() )
			skillid = packet->skillId2();
		else
			skillid = packet->skillId3();
	}

	pChar->giveNewbieItems( skillid );

	// Start the game with the newly created char -- OR RELAY HIM !!
    playChar( pChar );
}

/*!
  This method sends a system \a message at the botton of the screen
  \sa cUOTxUnicodeSpeech
*/
void cUOSocket::sysMessage( const QString &message, Q_UINT16 color, UINT16 font ) const
{
	if( message.isEmpty() )
		return;
	// Color: 0x0037
	cUOTxUnicodeSpeech speech;
	speech.setSource( 0xFFFFFFFF );
	speech.setModel( 0xFFFF );
	speech.setFont( font );
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
		charList.setCharacter( i, characters.at(i)->name.latin1() );

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
	if( !_player )
		return;

	// Check if it's a singleclick on items or chars
	if( isCharSerial( packet->serial() ) )
	{
		P_CHAR pChar = FindCharBySerial( packet->serial() );

		if( !pChar )
			return;

		if( !pChar->onSingleClick( _player ) )
			pChar->showName( this );
	}
	else
	{
		P_ITEM pItem = FindItemBySerial( packet->serial() );

		if( !pItem )
			return;

		if( !pItem->onSingleClick( _player ) )
			pItem->showName( this );
	}
}

/*!
  This method handles cUORxMultiPorpuse packet types.
  \sa cUORxMultiPorpuse
*/
void cUOSocket::handleMultiPurpose( cUORxMultiPurpose *packet ) 
{ 
	if ( !packet ) // Happens if it's not inherited from cUORxMultiPurpose
		return;

	switch( packet->subCommand() ) 
	{ 
	case cUORxMultiPurpose::setLanguage: 
		handleSetLanguage( dynamic_cast< cUORxSetLanguage* >( packet ) ); break; 
	case cUORxMultiPurpose::contextMenuRequest: 
		handleContextMenuRequest( dynamic_cast< cUORxContextMenuRequest* >( packet ) ); break; 
		break;
	case cUORxMultiPurpose::contextMenuSelection: 
		handleContextMenuSelection( dynamic_cast< cUORxContextMenuSelection* >( packet ) ); break; 
	default:
		return;
		packet->print( &cout ); // Dump the packet 
	}; 
} 

void cUOSocket::handleContextMenuSelection( cUORxContextMenuSelection *packet ) 
{ 
	P_CHAR pChar;
	P_ITEM pItem;
	const cConMenu *menu;

	Q_UINT16 Tag = packet->EntryTag();
	
	
	pItem = FindItemBySerial( packet->serial() );
	if ( pItem )
	{
		menu = ContextMenus::instance()->getMenu( pItem->bindmenu(), this->player()->account()->acl() );
		if ( !menu ) 
			return;
		menu->onContextEntry( this->player(), pItem, Tag );
	} 
	else 
	{
		pChar = FindCharBySerial( packet->serial() );
		if( !pChar )
			return;
		menu = ContextMenus::instance()->getMenu( pChar->bindmenu(), this->player()->account()->acl() );
		if ( !menu ) 
			return;
		menu->onContextEntry( this->player(), pChar, Tag );
	}
} 

// Show a context menu
/*!
  This method handles cUORxContextMenuRequest packet types.
  \sa cUORxContextMenuRequest
*/
void cUOSocket::handleContextMenuRequest( cUORxContextMenuRequest *packet ) 
{ 
	cUObject *clicked = FindItemBySerial( packet->serial() );
	if ( clicked == 0 ) clicked = FindCharBySerial( packet->serial() );
	
	if (!clicked || clicked->bindmenu().isEmpty() )
		return;
	
	if( !ContextMenus::instance()->MenuExist( clicked->bindmenu() ) ) 
	{
		clicked->setBindmenu(QString::null);
		return;
	}
	
	QString acl = this->account()->acl(); 
	QString bindmenu = clicked->bindmenu();
	
	cUOTxContextMenu menu; 
	menu.setSerial ( packet->serial() ); 
	
	const cConMenuOptions *tOptions = ContextMenus::instance()->getMenuOptions( bindmenu, acl );
	
	if ( !tOptions )
		return;
	
	Q_UINT16 Tag, IntlocID, MsgID;
	
	for (int i = 0; i < tOptions->getOptions().size(); i++) 
	{
		Tag = tOptions->getOptions()[i].getTag();
		IntlocID = tOptions->getOptions()[i].getIntlocID();
		MsgID = tOptions->getOptions()[i].getMsgID();
		
		menu.addEntry ( Tag, IntlocID, MsgID ); 
	}
	send( &menu ); 
} 

/*!
  This method prints \a message on top of \a object using the given \a color and \a speechType
  \sa cUObject, cUOTxUnicodeSpeech, cUOTxUnicodeSpeech::eSpeechType
*/
void cUOSocket::showSpeech( const cUObject *object, const QString &message, Q_UINT16 color, Q_UINT16 font, cUOTxUnicodeSpeech::eSpeechType speechType ) const
{
	cUOTxUnicodeSpeech speech;
	speech.setSource( object->serial );
	speech.setName( object->name.latin1() );
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
	Movement::instance()->Walking( _player, packet->direction(), packet->key());
}

void cUOSocket::resendPlayer( bool quick )
{
	if( !_player )
		return;

	// Pause
	cUOTxPause pause;
	pause.pause();
	send( &pause );

	// Make sure we switch client when this changes
	if( !quick )
	{
		cUOTxChangeMap changeMap; 
		changeMap.setMap( _player->pos.map );
		send( &changeMap );
	}

	cUOTxDrawPlayer drawPlayer;
	drawPlayer.fromChar( _player );
	send( &drawPlayer );

	// Resend our equipment
	cChar::ContainerContent container(_player->content());
	cChar::ContainerContent::const_iterator it (container.begin());
	cChar::ContainerContent::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pItem = *it;
		if( !pItem )
			continue;

		// Only send visible layers
		// 0x19 = horse layer
		// -> Shop containers need to be send
		if( pItem->layer() > 0x19 && pItem->layer() != 0x1A && pItem->layer() != 0x1B && pItem->layer() != 0x1C )
			continue;

		cUOTxCharEquipment equipment;
		equipment.fromItem( pItem );
		send( &equipment );
	}

	// Set the warmode status
	if( !quick )
	{
		cUOTxWarmode warmode;
		warmode.setStatus( _player->war() );
		send( &warmode );
	}

	// Resume
	cUOTxPause resume;
	resume.resume();
	send( &resume );

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
	if( !_player || !_player->account() )
		return;

	if( !pChar->isNpc() && !pChar->socket() && !_player->account()->isAllShow() )
		return;

	if( ( pChar->isHidden() || ( pChar->dead() && !pChar->war() ) ) && !_player->isGMorCounselor() )
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
/*	UI32 y;

	QString motdText = DefManager->getText( "MOTD" );
	y = motdText.length() + 10;
	
	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=2;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;
	Xsend(s, updscroll, 10);
	
	Xsend(s, (char*)motdText.latin1(), motdText.length() );*/

	if ( packet->isTip() )
	{
		UI32 tip = packet->lastTip();
		UI32 y = 10;

		if( tip == 0 ) 
			tip = 1; 

		QStringList tipList = DefManager->getList( "TIPS" );
		if( tipList.size() == 0 )
			return;
		else if( tip > tipList.size() )
			tip = tipList.size();

		QString tipText = DefManager->getText( tipList[ tip-1 ] );
		cUOTxTipWindow packet;
		packet.setType( cUOTxTipWindow::Tip );
		packet.setNumber( tip );
		packet.setMessage( tipText );
		send( &packet );	
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
	bool update = false;

	if( packet->warmode() != _player->war() )
		update = true;

	_player->setWar( packet->warmode() );

	// Stop fighting immedeately
	if( !_player->war() )
	{
		_player->setTarg( INVALID_SERIAL );
		_player->setSwingTarg( INVALID_SERIAL );
		_player->setTimeOut( 0 );
	}

	cUOTxWarmode warmode;
	warmode.setStatus( packet->warmode() ? 1 : 0 );
	send( &warmode );

	playMusic();
	_player->disturbMed();

	// Something changed
	if( update )
	{
		if( _player->dead() && _player->war() ) 
			_player->resend( false );
		else if( _player->dead() && !_player->war() )
		{
			_player->removeFromView( false );
			_player->resend( false );
		}			
		else
			_player->update( true );
	}
}

void cUOSocket::playMusic()
{
	if( !_player );
		return;

	cTerritory* Region = _player->region();
	UINT32 midi = 0;

	if( _player->war() )
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
			if( pCont->morex() == 1 ) 
				gump = 0x4a;
			break;

		default:
			if( ( ( pCont->id() & 0xFF00 ) >> 8 ) == 0x3E )
				gump = 0x4C;
			break;
	}

	// If its one of the "invisible" layers send an equip item packet first
	if( pCont->layer() == 0x1D )
	{
		cUOTxCharEquipment equipment;
		equipment.fromItem( pCont );
		send( &equipment );
	}

	// Draw the container
	cUOTxDrawContainer dContainer;
	dContainer.setSerial( pCont->serial );
	dContainer.setGump( gump );
	send( &dContainer );

	// Add all items to the container
	cUOTxItemContent itemContent;
	INT32 count = 0;

	cItem::ContainerContent container = pCont->content();
	cItem::ContainerContent::const_iterator it(container.begin());
	cItem::ContainerContent::const_iterator end(container.end());
	for( ; it != end; ++it )
	{
		P_ITEM pItem = *it;
		
		if( !pItem )
			continue;

		itemContent.addItem( pItem );
		++count;
	}

	if( pCont->objectID() == "CORPSE" )
	{
		cCorpse *pCorpse = dynamic_cast< cCorpse* >( pCont );

		if( !pCorpse )
			return;

		if( pCorpse->hairStyle() )
		{
			itemContent.addItem( 0x4FFFFFFE, pCorpse->hairStyle(), pCorpse->hairColor(), 0, 0, 1, pCorpse->serial );
			++count;
		}

		if( pCorpse->beardStyle() )
		{			
			itemContent.addItem( 0x4FFFFFFF, pCorpse->beardStyle(), pCorpse->beardColor(), 0, 0, 1, pCorpse->serial );
			++count;
		}
	}

	// Only send if there is content
	if( count )
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
		cancelTarget();
	}
}

void cUOSocket::attachTarget( cTargetRequest *request )
{
	// Let the old one time out
	if( targetRequest && targetRequest != request )
	{
		targetRequest->canceled( this );
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
		targetRequest->canceled( this );
		delete targetRequest;
	}

	targetRequest = request;

	cUOTxPlace target;
	target.setTargSerial( 1 );
	target.setModelID( multiid - 0x4000 );
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
		targetRequest->canceled( this );
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
	if( _player->cell() > 0 )
	{
		sysMessage( tr( "There is no fighting in the jail cells!" ) );
		send( &attack );
		return;
	}

	// Player is dead
	if( _player->dead() )
	{
		if( SrvParams->persecute() )
		{
			_player->setTarg( pc_i->serial );
			if( _player->targ() != INVALID_SERIAL ) 
				Skills->Persecute( this );
		} 
		else
			sysMessage( tr( "You are dead and cannot do that." ) );

		send( &attack );
		return;
	}

	// Attacking ghosts is not possible
	if( pc_i->dead() || pc_i->hp() <= 0 )
	{
		sysMessage( tr( "That person is already dead!" ) );
		send( &attack );
		return;
	}

	// Playervendors are invulnerable
	if( pc_i->npcaitype() == 17 ) 
	{
		sysMessage( tr( "%1 cannot be harmed." ).arg( pc_i->name ) );
		send( &attack );
		return;
	}

	_player->setTarg( pc_i->serial );
	_player->unhide();
	_player->disturbMed();

	// Accept the attack
	attack.setSerial( pc_i->serial );
	send( &attack );

	// NPC already has a target 
	// (And so is already fighting and should've been attacked by someone else)
	if( pc_i->targ() != INVALID_SERIAL )
	{
		pc_i->setAttacker( _player->serial );
		pc_i->resetAttackFirst();
	}

	_player->setAttackFirst();
	_player->setAttacker(pc_i->serial);
	_player->turnTo( pc_i );

	// The person being attacked is guarded by pets ?
	if( pc_i->guarded() )
	{
		RegionIterator4Chars cIter( pc_i->pos );
		for( cIter.Begin(); !cIter.atEnd(); cIter++ )
		{
			P_CHAR toCheck = cIter.GetData();
			if( toCheck->owner() == pc_i && toCheck->npcaitype() == 32 && toCheck->inRange( _player, 10 ) && toCheck->targ() == INVALID_SERIAL )
				toCheck->attackTarget( _player );
				// This was: npcattacktarget( _player, toCheck );
		}
	}

	if( pc_i->inGuardedArea() && SrvParams->guardsActive() )
	{
		if( pc_i->isPlayer() && pc_i->isInnocent() && GuildCompare( _player, pc_i ) == 0 ) //REPSYS
		{
			criminal( _player );
			Combat::spawnGuard( _player, pc_i, _player->pos );
		}
		else if( pc_i->isNpc() && pc_i->isInnocent() && !pc_i->isHuman() && pc_i->npcaitype() != 4 )
		{
			criminal( _player );
			Combat::spawnGuard( _player, pc_i, _player->pos );
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
			pc_i->attackTarget( _player );
		}
		else if ((pc_i->isNpc() || pc_i->tamed()) && !pc_i->war() && pc_i->npcaitype() != 4) // changed from 0x40 to 4, cauz 0x40 was removed LB
		{
			pc_i->toggleCombat();
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

				if( pc_i->targ() == INVALID_SERIAL )
					pc_i->attackTarget( _player );

				if( !pc_i->tamed() && pc_i->isHuman() )
					pc_i->talk( tr( "Help! Guards! Tis a murder being commited!" ) );
			}
		}
	}

	// Send the "You see %1 attacking %2" string to all surrounding sockets
	// Except the one being attacked
	QString message = tr( "*You see %1 attacking %2!" ).arg(_player->name.latin1()).arg(pc_i->name.latin1());
	for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
		if( s->player() && s != this && s->player()->inRange( _player, s->player()->VisRange() ) && s->player() != pc_i )
			s->showSpeech( _player, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );

	// Send an extra message to the victim
	if( pc_i->socket() )
		pc_i->socket()->showSpeech( _player, tr( "You see %1 attacking you" ).arg( _player->name.latin1() ), 0x26, 3, cUOTxUnicodeSpeech::Emote );
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

	RegionIterator4Items itIterator( _player->pos );
	for( itIterator.Begin(); !itIterator.atEnd(); itIterator++ )
	{
		P_ITEM pItem = itIterator.GetData();
		pItem->update( this );
	}

	RegionIterator4Chars chIterator( _player->pos, _player->VisRange() );

	for( chIterator.Begin(); !chIterator.atEnd(); chIterator++ )
	{
		P_CHAR pChar = chIterator.GetData();
		if( !pChar || pChar == _player )
			continue;

		if( clean )
		{
			cUOTxRemoveObject rObject;
			rObject.setSerial( pChar->serial );
			send( &rObject );
		}
		
		// Hidden
		if( pChar->isHidden() && !_player->isGMorCounselor() )
			continue;

		// Logged out
		if( pChar->account() && !pChar->socket() && !_player->account()->isAllShow() )
			continue;

		cUOTxDrawChar drawChar;
		drawChar.fromChar( pChar );
		drawChar.setHighlight( pChar->notority( _player ) );
		send( &drawChar );
	}
}

void cUOSocket::resync()
{
	resendPlayer( false );
}

P_ITEM cUOSocket::dragging() const
{
	if( !_player )
		return 0;

	return _player->atLayer( cChar::Dragging );
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
		update.setCurrent( pChar->stm() );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( (UINT16)((pChar->stm()/pChar->effDex())*100) );
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
		update.setMaximum( pChar->in() );
		update.setCurrent( pChar->mn() );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( (UINT16)((pChar->mn()/pChar->in())*100) );
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
		update.setMaximum( pChar->st() );
		update.setCurrent( pChar->hp() );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( (UINT16)((pChar->hp()/pChar->st())*100) );
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

	// TODO: extended packet information
	sendStats.setFullMode( pChar == _player, true /*_version.left(1).toInt() == 3*/ );

	// Dont allow rename-self
	sendStats.setAllowRename( ( ( pChar->owner() == _player && !pChar->isHuman() ) || _player->isGM() ) && ( _player != pChar ) );
	
	sendStats.setMaxHp( pChar->st() );
	sendStats.setHp( pChar->hp() );

	sendStats.setName( pChar->name.latin1() );
	sendStats.setSerial( pChar->serial );
		
	// Set the rest - and reset if nec.
	if( pChar == _player )
	{
		sendStats.setStamina( pChar->stm() );
		sendStats.setMaxStamina( pChar->effDex() );
		sendStats.setMana( pChar->mn() );
		sendStats.setMaxMana( pChar->in() );
		sendStats.setStrength( pChar->st() );
		sendStats.setDexterity( pChar->effDex() );
		sendStats.setIntelligence( pChar->in() );
		sendStats.setWeight( pChar->weight() / 10 );
		sendStats.setGold( pChar->CountBankGold() + pChar->CountGold() );
		sendStats.setArmor( pChar->calcDefense( ALLBODYPARTS ) );
		sendStats.setSex( true );
		
	//	if( sendStats[2] == 0x46 )
	//	{
			sendStats.setPets( _player->followers().size() );
			sendStats.setMaxPets( 0xFF );
			sendStats.setStatCap( SrvParams->statcap() );
	//	}
	}

	send( &sendStats );
}

bool cUOSocket::inRange( cUOSocket* socket ) const
{
	if ( !socket || !socket->player() || !_player )
		return false;
	return ( socket->player()->pos.distance( _player->pos ) < socket->player()->VisRange() );
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
			QMap<int, QString> content_ = pBook->content();
			QStringList lines = packet->lines();
			
			QString toInsert = QString();
			QStringList::const_iterator it = lines.begin();
			while( it != lines.end() )
			{
				toInsert += (*it)+"\n";
				++it;
			}

			UINT16 n = packet->page();
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
	if( !_player )
		return;

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
	// Cast Spell (out of spellbook)
	case 0x56:
	case 0x27:
	case 0x39:
		{
			NewMagic->castSpell( _player, packet->action().toInt()-1 );
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

	QMap< SERIAL, cGump* >::iterator it( gumps.find( packet->serial() ) );
	if ( it == gumps.end() )
	{
		clConsole.send( tr("Invalid gump response packet received from %1").arg(_account->login()) );
		return;
	}

	cGump* pGump = it.data();
	
	if( pGump )
	{
		pGump->handleResponse( this, packet->choice() );
		delete pGump;
		gumps.erase( it );
	}
}

void cUOSocket::sendVendorCont( P_ITEM pItem )
{
	// Only allowed for pItem's contained by a character
	cUOTxItemContent itemContent;
	cUOTxVendorBuy vendorBuy;
	vendorBuy.setSerial( pItem->serial );

	cItem::ContainerContent container = pItem->content();
	cItem::ContainerContent::const_iterator it( container.begin() );
	cItem::ContainerContent::const_iterator end( container.end() );
	for( Q_INT32 i = 0; it != end; ++it, ++i )
	{
		P_ITEM mItem = *it;

		if( mItem )
		{
			// For Layer 0x1A restock is the amount currently in stock
			if( pItem->layer() == 0x1A && mItem->restock <= 0 )
				continue;

			itemContent.addItem( mItem->serial, mItem->id(), mItem->color(), i, i, ( pItem->layer() == 0x1A ) ? mItem->restock : mItem->amount(), pItem->serial );
			vendorBuy.addItem( mItem->value, mItem->getName() );
		}
	}

	send( &itemContent );
	send( &vendorBuy );
}

void cUOSocket::sendBuyWindow( P_CHAR pVendor )
{
	P_ITEM pBought = pVendor->atLayer( cChar::BuyNoRestockContainer );
	P_ITEM pStock = pVendor->atLayer( cChar::BuyRestockContainer );
	
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
	player()->setLockSkill( packet->skill(), packet->lock() );
}

/*
thanks to codex, see 
http://www.wpdev.org/modules.php?op=modload&name=phpBB2&file=viewtopic&t=1117&sid=44a576c488c79ba923295eae549bed42
for more information
*/
void cUOSocket::clilocMessage( const Q_INT16 FileID, const Q_UINT16 MsgID, const QString &params, const Q_UINT16 color, const Q_UINT16 font, cUObject *object )
{
	cUOTxClilocMsg msg;

	if( object != 0 )
	{
		msg.setSerial( object->serial );
		msg.setType( cUOTxClilocMsg::OnObject );
		msg.setName( object->name.latin1() );
	}
	else
	{
		msg.setSerial( 0xFFFF );
		msg.setType( cUOTxClilocMsg::LowerLeft );
		msg.setName( "System" );
	}
	msg.setBody( 0xFF );
	msg.setHue( color );
	msg.setFont( font );
	msg.setMsgNum( FileID*1000+MsgID+1000001 );
	msg.setParams( params );

	send( &msg );
}

void cUOSocket::cancelTarget()
{
	cUOTxTarget target;
	target.setTargSerial( 0 );
	target[6] = 3; // Cursor Type
	send( &target );

	if( targetRequest )
	{
		targetRequest->canceled( this );
		delete targetRequest;
		targetRequest = 0;
	}
}

void cUOSocket::updateLightLevel( UINT8 level )
{
	if( _player )
	{
		cUOTxLightLevel pLight;

		if( SrvParams->worldFixedLevel() != 255 )
			pLight.setLevel( SrvParams->worldFixedLevel() );

		else if( _player->fixedlight() != 255 )
			pLight.setLevel( _player->fixedlight() );

		//else if( indungeon( _player ) )
		//	pLight.setLevel( SrvParams->dungeonLightLevel() );

		else if( level != 0xFF )
			pLight.setLevel( level );

		else
			pLight.setLevel( SrvParams->worldCurrentLevel() );

		send( &pLight );
	}
}

/*!
	Class for handling a dye request (for dying dye-tubs).
*/
class cDyeTubDyeTarget: public cTargetRequest
{
private:
	UINT16 _color;
public:
	cDyeTubDyeTarget( UINT16 color ) { _color = color; }

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		P_ITEM pItem = FindItemBySerial( target->serial() );

		if( !pItem )
			return true;

		if( pItem->getOutmostChar() != socket->player() && !( pItem->isInWorld() && pItem->inRange( socket->player(), 4 ) ) )
		{
			socket->sysMessage( tr( "You can't reach this object to dye it." ) );
			return true;
		}

		if( pItem->type() != 406 )
		{
			socket->sysMessage( tr( "You can only dye dye tubs with this." ) );
			return true;
		}

		pItem->setColor( _color );
		pItem->update();

		return true;
	}
};

void cUOSocket::handleDye( cUORxDye* packet )
{
	if( !_player )
		return;

	P_ITEM pItem = FindItemBySerial( packet->serial() );
	
	if( !pItem || pItem->type() != 405 )
		return;

	// Check if there is someone cheating
	if( packet->color() < 2 && packet->color() > 0x3E9 )
	{
		sysMessage( tr( "You can't dye in this kind of color." ) );
		return;
	}

	// Ok, now show the client a target to select the dye-tub we want
	// to dye.
	attachTarget( new cDyeTubDyeTarget( packet->color() ) );
}

void cUOSocket::handleProfile( cUORxProfile *packet )
{
	if( !_player )
		return;

	// Send the requested profile
	if( packet->size() <= 8 )
	{
		P_CHAR pChar = FindCharBySerial( packet->serial() );

		if( pChar )
		{
			cUOTxProfile profile;
			profile.setSerial( packet->serial() );
			profile.setInfo( pChar->name, pChar->title(), pChar->profile() );
			send( &profile );
		}
	}
	// Check if it is a update request
	else if( packet->command() )
	{
		// You can only change your own profile
		if( packet->serial() != _player->serial )
			sysMessage( tr( "You can only change your own profile" ) );
		else
			_player->setProfile( packet->text() );
	}
}

void cUOSocket::handleRename( cUORxRename* packet )
{
	if( !_player )
		return;

	// If we are no GM we can only rename characters
	// we own (only pets here!)
	if( packet->serial() == _player->serial )
		sysMessage( tr( "You can't rename yourself" ) );
	else
	{
		P_CHAR pChar = FindCharBySerial( packet->serial() );

		if( pChar && pChar->owner() == _player && !pChar->isHuman() )
		{
			pChar->name = packet->name();

			if( pChar->name.length() > 29 )
			{
				pChar->name = pChar->name.left( 29 );
				sysMessage( tr( "This name was too long, i truncated it to 29 characters." ) );
			}
		}
		else
			sysMessage( tr( "You can't rename this." ) );
	}
}
