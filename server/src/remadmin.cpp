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

// Raw Telnet Implementation done by DarkStorm
// NOTE: No Telnet Commands are supported. Somehow putty screwed up.
// Reference: http://www.scit.wlv.ac.uk/~jphb/comms/telnet.html

#include "remadmin.h"

// Library Includes
#include "qsocketdevice.h"
#include "qstringlist.h"

// Wolfpack Includes
#include "accounts.h"
#include "globals.h"
#include "verinfo.h"

// Private Socket
class PrivateSocket
{
public:
	enum socketState
	{ 
		TelnetCommand, 
		SendingUsername, 
		SendingPassword, 
		Connected
	};
	socketState state;
	socketState previousState;
	QString username;
	QString line;
	QSocketDevice socket;
	UINT8 buffer[128]; // For Telnet Commands
	UINT8 bufferSize;
	UINT8 escapeSequence;
	bool bufferIAC;

public:			// Methods
	PrivateSocket( int fd );

	void send( const QString& msg, bool newline = true );

	// Telnet Methods
	bool tryTelnetCommand( UINT8 data );
};

// Defines for the Telnet Interface
#define TELNET_SE 240
#define TELNET_NOP 241
#define TELNET_DM 242
#define TELNET_BRK 243
#define TELNET_IP 244
#define TELNET_AO 245
#define TELNET_AYT 246
#define TELNET_EC 247
#define TELNET_EL 248
#define TELNET_GA 249
#define TELNET_SB 250
#define TELNET_WILL 251
#define TELNET_DO 252
#define TELNET_WONT 253
#define TELNET_DONT 254
#define TELNET_IAC 255

#define MODE_ECHO 1
#define MODE_SGA 3
#define MODE_STATUS 5
#define MODE_TIMINGMARK 6
#define MODE_TERMINALTYPE 24
#define MODE_WINDOWSIZE 31
#define MODE_TERMINALSPEED 32
#define MODE_REMOTEFLOWCONTROL 33
#define MODE_LINEMODE 34
#define MODE_ENV 36
#define MODE_NEWENV 39

PrivateSocket::PrivateSocket( int fd ) : socket( fd, QSocketDevice::Stream ), 
previousState(SendingUsername), state(SendingUsername), bufferSize( 0 ), bufferIAC( false ),
escapeSequence( 0 )
{
	memset( &buffer[0], 0, 128 );
	socket.setBlocking( false );
}

bool PrivateSocket::tryTelnetCommand( UINT8 data )
{
	// If we are not in TelnetCommand mode, check out if the
	// received byte was a Telnet Command
	if( state != TelnetCommand && data != TELNET_IAC )
		return false;

	// IAC = Entering Telnet Command Mode
	if( data == TELNET_IAC && state != TelnetCommand )
	{
		bufferSize = 1;
		buffer[0] = data;
		previousState = state; // Save old State
		state = TelnetCommand;
		return true;
	}

	// TELNET_IAC are sent twice if they are data
	if( data == TELNET_IAC && !bufferIAC )
	{
		bufferIAC = true;
		return true;
	}

	// Whenever we come here we are no longer buffering a 255
	bufferIAC = false;

	// Append to our Buffer
	buffer[bufferSize++] = data;

	// Check if a Command was completely received
	if( bufferSize >= 3 )
	{
		switch( buffer[1] )
		{
			case TELNET_WILL:
			case TELNET_WONT:
			case TELNET_DO:
			case TELNET_DONT:
				state = previousState;
				bufferSize = 0;
				return true;
				
			// Starting a Subnegotiation
			case TELNET_SB:
				// Is the Negotiation already done?
				if( buffer[bufferSize-1] == TELNET_SE )
				{
					state = previousState;
					bufferSize = 0;
				}

				return true;

			// Command is invalid or unknown
			default:
				bufferSize = 0;
				state = previousState;
				return true;
		}
	}

	// If we reached this point, there was no command
	// Check if we are overflowing our buffer
	if( bufferSize == 31 )
	{
		state = previousState;
		bufferSize = 0;
	}

	return true;
}

void PrivateSocket::send( const QString& msg, bool newline )
{
	socket.writeBlock( msg.latin1(), msg.length() );
	if( newline )
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
		const QString version = wp_version.verstring.c_str();

		socket->send( "===============================================================" );
		socket->send( "|   )      (\\_     | Welcome to Wolfpack " + version, false );
		// 21 - Length of Version
		QString spaces;
		for( int i = 0; i < 21 - version.length(); ++i )
			spaces.append( " " );
		spaces.append( "|" );

		socket->send( spaces );
		socket->send( "|  ((    _/{  \"-;  | http://www.wpdev.org                     |" );
		socket->send( "|   )).-' {{ ;'`   |                                          |" );
		socket->send( "|  ( (  ;._ \\\\ ctr | Turn off Local Echo and Line Editing!    |" );
		socket->send( "===============================================================" );
		socket->send( "" );			

		socket->send( "Username: ", false ); // ask for username.
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
		
		if( !socket->socket.atEnd() )
		{
			UINT8 ch = socket->socket.getch();

			if( socket->tryTelnetCommand( ch ) )
				continue;

			// Special Case = Escape Sequence
			if( socket->escapeSequence > 0 )
			{
				
			}

			// Remote Admin functionalities
			switch ( ch )
			{
			// Backspace
			case 8:
				// Is there even something to delete?
				if( socket->state != PrivateSocket::SendingPassword && socket->line.length() > 0 )
					socket->socket.putch( ch );
				socket->line = socket->line.left( socket->line.length() - 1 );
				break;
			case 127:
				socket->socket.putch( 0xff );
				socket->socket.putch( 0xf8 );
				break;
			// Escape Sequence
			case 27:
				socket->escapeSequence = 1; // Start of Escape Sequence received
				
			case 9 : tryCompleteCommand( socket );	break;
			case 13: executeCommand( socket );		break;
			default:
				// Only "readable" characters here
				if( ch < 32 || ch > 126 )
					continue;
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
	// Clear the old line
	socket->send( "\n\r", false );

	QStringList tokens = QStringList::split( " ", socket->line );

	switch ( socket->state )
	{
	case PrivateSocket::SendingUsername:
		socket->username = socket->line;
		socket->state    = PrivateSocket::SendingPassword;
		socket->send( "Password: ", false );
		socket->line = "";
		return; // Prevent it from showing the prompt
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

	socket->line = "";
	sendPrompt( socket ); // ready for next command.
}

