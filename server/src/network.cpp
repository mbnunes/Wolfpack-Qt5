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

// Platform Includes
#include "platform.h"

//Wolfpack Includes
#include "network.h"
#include "srvparams.h"
#include "network/asyncnetio.h"
#include "network/uosocket.h"
#include "network/listener.h"

// Library Includes
#include "qstringlist.h"

#undef  DBGFILE
#define DBGFILE "Network.cpp"

// Authenticate return codes

#define LOGIN_NOT_FOUND -3
#define BAD_PASSWORD -4
#define ACCOUNT_BANNED -5
#define ACCOUNT_WIPE -6

cNetwork *cNetwork::instance_;

cNetwork::cNetwork( void )
{
	loginSockets.setAutoDelete( true );
	uoSockets.setAutoDelete( true );
	listener_ = new cListener( SrvParams->port() );
	netIo_ = new cAsyncNetIO;

	listener_->start();
	netIo_->start();
}

cNetwork::~cNetwork( void )
{
	netIo_->cancel();
	listener_->cancel();

	netIo_->join();
	listener_->join(); // Wait for them to stop

	delete netIo_;
	delete listener_;
}

void cNetwork::poll( void )
{
	// Check for new Connections
	if ( listener_->haveNewConnection() )
	{
		QSocketDevice *socket = listener_->getNewConnection(); 
		netIo_->registerSocket( socket );
		loginSockets.append( new cUOSocket(socket) );

		// Notify the admin
		clConsole.send( QString( "[%1] Client connected\n" ).arg( socket->peerAddress().toString() ) );
	}

	// fast return
	if( uoSockets.isEmpty() && loginSockets.isEmpty() )
		return;

	// Check for new Packets
	cUOSocket* uoSocket = 0;
	for ( uoSocket = uoSockets.first(); uoSocket; uoSocket = uoSockets.next() )
	{
		// Check for disconnected sockets
		if ( uoSocket->socket()->error() != QSocketDevice::NoError || !uoSocket->socket()->isValid() || !uoSocket->socket()->isWritable() || uoSocket->socket()->isInactive() || !uoSocket->socket()->isOpen() )
		{
			clConsole.send( tr( "[%1] Client disconnected\n" ).arg( uoSocket->socket()->peerAddress().toString() ) );			
			uoSocket->disconnect();
			netIo_->unregisterSocket( uoSocket->socket() );			
			uoSockets.remove( uoSocket );
		}
		else
		{
			if( uiCurrentTime % 100 == 0 ) // Just every 100 ticks
				uoSocket->poll();

			uoSocket->recieve();
		}
	}

	for ( uoSocket = loginSockets.first(); uoSocket; uoSocket = loginSockets.next())
	{
		if( uoSocket->socket()->error() != QSocketDevice::NoError )
		{
			clConsole.send( tr( "[%1] Client disconnected\n" ).arg( uoSocket->socket()->peerAddress().toString() ) );
			netIo_->unregisterSocket( uoSocket->socket() );
			loginSockets.remove();
			continue;
		}
		else
			uoSocket->recieve();

		if ( uoSocket->state() == cUOSocket::InGame )
		{
			uoSockets.append( loginSockets.take() );
		}
	}
}

// Code for wrapping old Stuff
void cNetwork::xSend( int s, const void *point, int length, int test) // Buffering send function
{
	cUOSocket *socket = uoSockets.at(s);
	
	if( !socket )
		return;

	QByteArray data( length );
	memcpy( data.data(), point, length );
	cUOPacket packet( data );

	socket->send( &packet );
}

// Load IP Blocking rules
void cNetwork::load( void )
{
}

// Reload IP Blocking rules
void cNetwork::reload( void )
{
	unload();
	load();
}

// Unload IP Blocking rules
void cNetwork::unload( void )
{
//	hosts_deny.clear();
}

// This is junk code to interface with the old junk code. Don't relay on that.
// It's ugly and slower.
UOXSOCKET cNetwork::getuoSocketsIndex( const cUOSocket* data )
{
	return uoSockets.findRef( data );	
}

UOXSOCKET calcSocketFromChar(P_CHAR pc)
{
	if ( !pc || !pc->socket() || pc->npc() )
	{
		return -1;
	}
	return cNetwork::instance()->getuoSocketsIndex( pc->socket() );
}

void cNetwork::broadcast( const QString &message, UINT16 color, UINT16 font )
{
	cUOSocket *mSock = uoSockets.first();
	while( mSock )
	{
		mSock->sysMessage( message, color, font );
		mSock = uoSockets.next();
	}
}
