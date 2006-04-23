/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
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
#include "uosocket.h"
#include "../basechar.h"
#include "../player.h"
#include "../exceptions.h"

// Library Includes
#include <QStringList>
#include <QList>
#include <QTcpServer>


class cNetwork::cNetworkPrivate
{
public:
	QList<cUOSocket*> uoSockets;
	QList<cUOSocket*> loginSockets;
	QMutableListIterator<cUOSocket*> internalIterator;
	QTcpServer* loginServer_;
	QTcpServer* gameServer_;
	QMutex mutex;

	cNetworkPrivate() : internalIterator( uoSockets )
	{
		loginServer_ = 0;
		gameServer_ = 0;
	}

	~cNetworkPrivate()
	{
		foreach (cUOSocket *socket, uoSockets) {
			delete socket;
		}
		foreach (cUOSocket *socket, loginSockets) {
			delete socket;
		}
		delete loginServer_;
		delete gameServer_;
	}
};


cNetwork::cNetwork() : d( new cNetworkPrivate )
{
}

cNetwork::~cNetwork()
{
	delete d;
}

void cNetwork::incomingGameServerConnection()
{
	QTcpSocket* socket = d->gameServer_->nextPendingConnection();
	cUOSocket *uosocket = new cUOSocket( socket );
	d->uoSockets.append( uosocket );
	connect( uosocket, SIGNAL(disconnected()), this, SLOT(partingGameServerConnection()) );
	// Notify the admin
	uosocket->log( tr( "Client '%1' connected to game server.\n" ).arg( socket->peerAddress().toString() ) );
}

void cNetwork::incomingLoginServerConnection()
{
	QTcpSocket* socket = d->loginServer_->nextPendingConnection();
	cUOSocket *uosocket = new cUOSocket( socket );
	d->loginSockets.append( uosocket );

	connect( uosocket, SIGNAL(disconnected()), this, SLOT(partingLoginServerConnection()) );
	// Notify the admin
	uosocket->log( tr( "Client '%1' connected to login server.\n" ).arg( socket->peerAddress().toString() ) );
}

void cNetwork::partingLoginServerConnection()
{
	cUOSocket* uoSocket = qobject_cast<cUOSocket *>( sender() );
	uoSocket->disconnect();
	d->loginSockets.removeAll( uoSocket );
	uoSocket->deleteLater();
	uoSocket->log( tr( "Client '%1' disconnected from loginserver.\n" ).arg( uoSocket->ip()) );
}

void cNetwork::partingGameServerConnection()
{
	cUOSocket* uoSocket = qobject_cast<cUOSocket *>(sender());
	uoSocket->disconnect();
	d->uoSockets.removeAll( uoSocket );
	uoSocket->deleteLater();
	// account info?
	uoSocket->log( tr( "Client '%1' disconnected from gameserver.\n" ).arg( uoSocket->ip()) );
}

// Load IP Blocking rules
void cNetwork::load()
{
	if ( Config::instance()->enableLogin() )
	{
		d->loginServer_ = new QTcpServer( this );
		if ( !d->loginServer_->listen( QHostAddress::Any, Config::instance()->loginPort() ) )
			throw wpException( tr("Unable to listen to port %1, port may be already in use").arg(Config::instance()->loginPort()) );
		connect( d->loginServer_, SIGNAL(newConnection()), this, SLOT(incomingLoginServerConnection()));
		Console::instance()->send( tr( "\nLoginServer running on port %1\n" ).arg( Config::instance()->loginPort() ) );
		QList<ServerList_st> serverList = Config::instance()->serverList();
		if ( serverList.size() < 1 )
			Console::instance()->log( LOG_WARNING, tr( "LoginServer enabled but there no Game server entries found\n Check your wolfpack.xml settings\n" ) );
		else
		{
			for ( QList<ServerList_st>::iterator it = serverList.begin(); it != serverList.end(); ++it )
				Console::instance()->send( tr("\t%1 using address %2\n").arg( (*it).sServer, (*it).address.toString() ) );
		}
	}

	if ( Config::instance()->enableGame() )
	{
		d->gameServer_ = new QTcpServer( this );
		if ( !d->gameServer_->listen( QHostAddress::Any, Config::instance()->gamePort() ) )
			throw wpException( tr("Unable to listen to port %1, port may be already in use").arg(Config::instance()->gamePort()) );
		connect( d->gameServer_, SIGNAL(newConnection()), this, SLOT(incomingGameServerConnection()));
		Console::instance()->send( tr( "\nGameServer running on port %1\n" ).arg( Config::instance()->gamePort() ) );
	}

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
	// Disconnect all connected sockets
	QList<cUOSocket*> socketList = d->uoSockets;
	d->uoSockets.clear();
	foreach (cUOSocket *socket, socketList) 
	{
		socket->disconnect();
		delete socket;
	}

	socketList = d->loginSockets;
	d->loginSockets.clear();
	foreach (cUOSocket *socket, d->loginSockets) 
	{
		socket->disconnect();
		delete socket;
	}

	// Process derrefered deletion of cUOSockets before deleting the parent QTcpSocket to avoid double deletion.
	QCoreApplication::processEvents();
	if ( d->loginServer_ )
	{
		delete d->loginServer_;
		d->loginServer_ = 0;
	}

	if ( d->gameServer_ )
	{
		delete d->gameServer_;
		d->gameServer_ = 0;
	}

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

quint32 cNetwork::count()
{
	return d->uoSockets.count();
}

QListIterator<cUOSocket*> cNetwork::getIterator()
{
	return QListIterator<cUOSocket*>( d->uoSockets );
}

QList<cUOSocket*> cNetwork::sockets() const
{
	return d->uoSockets;
}

void cNetwork::broadcast( const QString& message, quint16 color, quint16 font )
{
	foreach ( cUOSocket* socket, d->uoSockets )
	{
		socket->sysMessage( message, color, font );
	}
}
