/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

// Platform Includes
#include "../platform.h"

//Wolfpack Includes
#include "network.h"
#include "../serverconfig.h"

#include "../console.h"
#include "../inlines.h"
#include "asyncnetio.h"
#include "uosocket.h"
#include "listener.h"
#include "../basechar.h"
#include "../player.h"
#include "../exceptions.h"

// Library Includes
#include <qstringlist.h>
#include <qmutex.h>


class cNetwork::cNetworkPrivate
{
public:
	QPtrList<cUOSocket> uoSockets;
	QPtrList<cUOSocket> loginSockets;
	cAsyncNetIO* netIo_;
	cListener* loginServer_;
	cListener* gameServer_;
	QMutex mutex;

	cNetworkPrivate()
	{
		loginSockets.setAutoDelete( true );
		uoSockets.setAutoDelete( true );
		netIo_ = new cAsyncNetIO;
		loginServer_ = 0;
		gameServer_ = 0;
	}

	~cNetworkPrivate()
	{
		delete loginServer_;
		delete gameServer_;
		delete netIo_;
	}
};


cNetwork::cNetwork() : d( new cNetworkPrivate )
{
}

cNetwork::~cNetwork()
{
	delete d;
}

void cNetwork::poll( void )
{
	lock();

	// Check for new Connections (LoginServer)
	if ( d->loginServer_ && d->loginServer_->haveNewConnection() )
	{
		QSocketDevice* socket = d->loginServer_->getNewConnection();
		d->netIo_->registerSocket( socket, true );
		cUOSocket* uosocket = new cUOSocket( socket );
		d->loginSockets.append( uosocket );

		// Notify the admin
		uosocket->log( tr( "Client connected to login server (%1).\n" ).arg( socket->peerAddress().toString() ) );
	}

	// Check for new Connections (GameServer)
	if ( d->gameServer_ && d->gameServer_->haveNewConnection() )
	{
		QSocketDevice* socket = d->gameServer_->getNewConnection();
		d->netIo_->registerSocket( socket, false );
		cUOSocket* uosocket = new cUOSocket( socket );
		d->loginSockets.append( uosocket );

		// Notify the admin
		uosocket->log( tr( "Client connected to game server (%1).\n" ).arg( socket->peerAddress().toString() ) );
	}

	// fast return
	if ( !d->uoSockets.isEmpty() || !d->loginSockets.isEmpty() )
	{
		// Check for new Packets
		cUOSocket* uoSocket = 0;
		for ( uoSocket = d->uoSockets.first(); uoSocket; uoSocket = d->uoSockets.next() )
		{
			// Check for disconnected sockets
			if ( uoSocket->socket()->error() != QSocketDevice::NoError || !uoSocket->socket()->isValid() || !uoSocket->socket()->isWritable() || uoSocket->socket()->isInactive() || !uoSocket->socket()->isOpen() )
			{
				uoSocket->log( tr( "Client disconnected.\n" ) );
				uoSocket->disconnect();
				d->netIo_->unregisterSocket( uoSocket->socket() );
				d->uoSockets.remove( uoSocket );
			}
			else
			{
				try
				{
					uoSocket->recieve();

					if ( Server::instance()->time() % 500 == 0 ) // Once every 0.5 Seconds
						uoSocket->poll();
				}
				catch ( wpException e )
				{
					uoSocket->log( LOG_PYTHON, e.error() + "\n" );
					uoSocket->log( LOG_ERROR, tr( "Disconnecting due to an unhandled exception.\n" ) );
					uoSocket->disconnect();
				}
			}
		}

		for ( uoSocket = d->loginSockets.first(); uoSocket; uoSocket = d->loginSockets.next() )
		{
			if ( uoSocket->socket()->error() != QSocketDevice::NoError || !uoSocket->socket()->isValid() || !uoSocket->socket()->isOpen() )
			{
				uoSocket->log( tr( "Client disconnected.\n" ) );
				d->netIo_->unregisterSocket( uoSocket->socket() );
				d->loginSockets.remove();
				continue;
			}
			else
				uoSocket->recieve();

			if ( uoSocket->state() == cUOSocket::InGame )
			{
				d->uoSockets.append( d->loginSockets.take() );
			}
		}
	}

	unlock();
}

// Load IP Blocking rules
void cNetwork::load()
{
	if ( Config::instance()->enableLogin() )
	{
		d->loginServer_ = new cListener( Config::instance()->loginPort() );
		d->loginServer_->start();
		Console::instance()->send( tr( "LoginServer running on port %1\n" ).arg( Config::instance()->loginPort() ) );
		if ( Config::instance()->serverList().size() < 1 )
			Console::instance()->log( LOG_WARNING, tr( "LoginServer enabled but there no Game server entries found\n Check your wolfpack.xml settings" ) );
	}

	if ( Config::instance()->enableGame() )
	{
		d->gameServer_ = new cListener( Config::instance()->gamePort() );
		d->gameServer_->start();
		Console::instance()->send( tr( "GameServer running on port %1\n" ).arg( Config::instance()->gamePort() ) );
	}

	d->netIo_->start();
	cComponent::load();
}

// Reload IP Blocking rules
void cNetwork::reload()
{
	unload();
	load();
}

// Unload IP Blocking rules
void cNetwork::unload()
{
	if ( d->loginServer_ )
	{
		d->loginServer_->cancel();
		d->loginServer_->wait();
		delete d->loginServer_;
		d->loginServer_ = 0;
	}

	if ( d->gameServer_ )
	{
		d->gameServer_->cancel();
		d->gameServer_->wait();
		delete d->gameServer_;
		d->gameServer_ = 0;
	}

	// Disconnect all connected sockets
	cUOSocket* socket;
	for ( socket = d->uoSockets.first(); socket; socket = d->uoSockets.next() )
	{
		socket->disconnect();
		d->netIo_->unregisterSocket( socket->socket() );
	}
	d->uoSockets.clear();

	for ( socket = d->loginSockets.first(); socket; socket = d->loginSockets.next() )
	{
		socket->disconnect();
		d->netIo_->unregisterSocket( socket->socket() );
	}
	d->loginSockets.clear();

	d->netIo_->cancel();
	d->netIo_->wait();

	cComponent::unload();
}

void cNetwork::lock()
{
	d->mutex.lock();
}

void cNetwork::unlock()
{
	d->mutex.unlock();
}

cAsyncNetIO* cNetwork::netIo()
{
	return d->netIo_;
}

cUOSocket* cNetwork::first()
{
	return d->uoSockets.first();
}

cUOSocket* cNetwork::next()
{
	return d->uoSockets.next();
}

Q_UINT32 cNetwork::count()
{
	return d->uoSockets.count();
}

QPtrListIterator<cUOSocket> cNetwork::getIterator()
{
	return QPtrListIterator<cUOSocket>( d->uoSockets );
}

void cNetwork::broadcast( const QString& message, Q_UINT16 color, Q_UINT16 font )
{
	for ( cUOSocket*socket = d->uoSockets.first(); socket; socket = d->uoSockets.next() )
	{
		socket->sysMessage( message, color, font );
	}
}
