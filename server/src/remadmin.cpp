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

#include "remadmin.h"

// Library Includes
#include "qsocketdevice.h"
#include "qstringlist.h"

// Wolfpack Includes
#include "accounts.h"
#include "globals.h"
#include "junk.h"

// Private Socket
class PrivateSocket
{
public:
	enum { SendingUsername, SendingPassword, Connected } state;
	QString username;
	QString line;
	QSocketDevice socket;

public:			// Methods
	PrivateSocket( int fd );
	void send( const QString& msg );
};

PrivateSocket::PrivateSocket( int fd ) : socket( fd, QSocketDevice::Stream ), state(SendingUsername)
{
	socket.setBlocking( false );
}

void PrivateSocket::send( const QString& msg )
{
	socket.writeBlock( msg.latin1(), msg.length() );
	socket.writeBlock( "\n\r", 2 );
}

static const QString prompt("[Wolfpack]# ");
QSocketDevice* RemoteAdmin::listenningSocket = 0;
QPtrList<PrivateSocket> RemoteAdmin::sockets;

RemoteAdmin::~RemoteAdmin()
{
	delete listenningSocket;
	listenningSocket = 0;
}

void RemoteAdmin::initialize( Q_UINT16 port, bool verbose /* = false */ )
{
	listenningSocket = new QSocketDevice;
	listenningSocket->setBlocking( false );
	listenningSocket->bind( static_cast<Q_UINT32>(0), port );
	listenningSocket->listen( 1 );
	sockets.setAutoDelete( true ); 
}

void RemoteAdmin::stop()
{
	sockets.clear();
	delete listenningSocket;
	listenningSocket = 0;
}

RemoteAdmin* RemoteAdmin::instance()
{
	static RemoteAdmin theInstance;
	return &theInstance;
}

void RemoteAdmin::processNextEvent()
{
	// First, let's see if there is anyone at the door.
    PrivateSocket *socket;
	int fd = listenningSocket->accept();
	if ( fd > -1 )
	{
		socket = new PrivateSocket(fd);
		socket->send("Login: "); // ask for username.
		sockets.append( socket );
	}


    for ( socket = sockets.first(); socket; socket = sockets.next() )
	{
		// First check if it's valid.
		if ( socket->socket.error() != QSocketDevice::NoError )
		{
			sockets.remove();
			continue;
		}
		
		int ch = socket->socket.getch();
		if ( ch > 0 )
		{
			switch ( ch )
			{
			case 9 : tryCompleteCommand( socket );	break;
			case 13: executeCommand( socket );		break;
			default:
				if ( socket->state != PrivateSocket::SendingPassword )
					socket->socket.putch( ch );
				socket->line.append( ch );
			}
		}
	}
}

void RemoteAdmin::sendPrompt( PrivateSocket* socket )
{
	socket->socket.writeBlock( prompt.latin1(), prompt.length() );
}

void RemoteAdmin::tryCompleteCommand( PrivateSocket* socket )
{
	sendPrompt( socket );
	socket->send( socket->line );
}

void RemoteAdmin::executeCommand( PrivateSocket* socket )
{
	QStringList tokens = QStringList::split( " ", socket->line );

	switch ( socket->state )
	{
	case PrivateSocket::SendingUsername:
		socket->username = socket->line;
		socket->state    = PrivateSocket::SendingPassword;
		socket->socket.writeBlock("Password: ", 11);
		break;
	case PrivateSocket::SendingPassword:
		{
			AccountRecord* account = Accounts::instance()->authenticate( socket->username, socket->line );
			if ( account == 0 )
			{
				socket->send("Access Denied");
				socket->socket.close(); // No second chances here.
				return;
			}
			if ( !account->authorized("RemoteAccess", "login") )
			{
				socket->send("Access Denied");
				socket->socket.close();
				return;
			}
			socket->state = PrivateSocket::Connected;
			sendPrompt( socket );
		}
		break;
	case PrivateSocket::Connected:
		
		break;
	}

	sendPrompt( socket ); // ready for next command.
}

