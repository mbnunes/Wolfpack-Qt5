//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

// Wolfpack Includes
#include "console.h"
#include "pythonscript.h"
#include "log.h"
#include "globals.h"
#include "world.h"
#include "network.h"
#include "srvparams.h"
#include "player.h"
#include "accounts.h"
#include "inlines.h"
#include "wolfpack.h"

// Library Includes
#include <qstring.h>
#include <qglobal.h>
#include <qthread.h>

#if defined(Q_OS_WIN32) 
# include <windows.h>
# include <conio.h>
#endif

#if defined(Q_OS_UNIX)
     #include <sys/types.h>
     #include <sys/time.h>
     #include <unistd.h>
#else

#endif

using namespace std;

//========================================================================================
// Constructor
cConsole::cConsole()
{
	bEnabled = true;
}

//========================================================================================
// Destuctor
cConsole::~cConsole()
{
	// Clean up any terminal settings
}

void cConsole::enabled(bool bState)
{
	bEnabled = bState;
}

//========================================================================================
void cConsole::log( UINT8 logLevel, const QString &message )
{
	// Legacy Code
	QString msg = message;
	
	if( msg.endsWith( "\n" ) )
		msg = msg.left( msg.length() - 1 );

	Log::instance()->print( (eLogLevel)logLevel, msg + "\n" );
}

//=========================================================================================
// Get input from the console
UI08 cConsole::getkey(void)
{
	UI08 key = 0;

//	if (cin.peek())
//		key = cin.get();
	
	return key;
}

//=========================================================================================
// Prepare a "progess" line
void cConsole::PrepareProgress( const QString &sMessage )
{
	UI08 PrintedChars = sMessage.length() + 1; // one spacer

	send( sMessage.right(59) + " " );

	ChangeColor( WPC_WHITE );
	// Fill up the remaining chars with "....."
	for( UI08 i = 0; i < 60 - PrintedChars; i++ )
		send( "." );

	ChangeColor( WPC_NORMAL );

	send( " [____]" );
}

//=========================================================================================
// Print Progress Done
void cConsole::ProgressDone( void )
{
	send( "\b\b\b\b\b" ); // Go 5 Characters back
	ChangeColor( WPC_GREEN );
	send( "done" );
	ChangeColor( WPC_NORMAL );
	send( "]\n" );
}

//=========================================================================================
// Print "Fail"
void cConsole::ProgressFail( void )
{
	send( "\b\b\b\b\b" ); // Go 5 Characters back
	ChangeColor( WPC_RED );
	send( "fail" );
	ChangeColor( WPC_NORMAL );
	send( "]\n" );
}

//=========================================================================================
// Print "Skip" (maps etc.)
void cConsole::ProgressSkip( void )
{
	send( "\b\b\b\b\b" ); // Go 5 Characters back
	ChangeColor( WPC_YELLOW );
	send( "skip" );
	ChangeColor( WPC_NORMAL );
	send( "]\n" );
}


bool cConsole::handleCommand( const QString &command, bool silentFail )
{
	cUOSocket *mSock;
	int i;
	char c = command.latin1()[0];
	c = toupper(c);

	if( c == 'S' )
	{
		secure = !secure;

		if( !secure )
			Console::instance()->send("WOLFPACK: Secure mode disabled. Press ? for a commands list.\n");
		else
			Console::instance()->send("WOLFPACK: Secure mode re-enabled.\n");

		return true;
	}

	// Allow Help in Secure Mode
	if( secure && c != '?' )
	{
		Console::instance()->send( "WOLFPACK: Secure mode prevents keyboard commands! Press 'S' to disable.\n" );
		return false;
	}

	switch( c )
	{
	case 'Q':
		Console::instance()->send("WOLFPACK: Immediate Shutdown initialized!\n");
		keeprun=0;
		break;

	case '#':
		World::instance()->save();				
		SrvParams->flush();
		break;

	case 'W':
		Console::instance()->send( "Current Users in the World:\n" );

		mSock = cNetwork::instance()->first();
		i = 0;
		
		for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
			if( mSock->player() )
				Console::instance()->send( QString("%1) %2 [%3]\n").arg(++i).arg(mSock->player()->name()).arg(QString::number( mSock->player()->serial(), 16) ) );
		}

		Console::instance()->send( tr("Total Users Online: %1\n").arg(cNetwork::instance()->count()) );
		break;
	case 'A': //reload the accounts file
		Accounts::instance()->reload();
		break;
	case 'R':
		reloadScripts();
		break;
	case '?':
		Console::instance()->send("Console commands:\n");
		Console::instance()->send("	Q: Shutdown the server.\n");
		Console::instance()->send("	# - Save world\n" );
		Console::instance()->send("	W - Display logged in characters\n" );
		Console::instance()->send("	A - Reload accounts\n" );
		Console::instance()->send("	R - Reload scripts\n" );
		Console::instance()->send("	S - Toggle Secure mode " );
		if( secure )
			Console::instance()->send( "[enabled]\n" );
		else
			Console::instance()->send( "[disabled]\n" );
		Console::instance()->send( "	? - Commands list (this)\n" );
		Console::instance()->send( "End of commands list.\n" );
		break;
	default:
		break;
	}

	return true;
}

void cConsole::queueCommand( const QString &command )
{
	commandMutex.lock();
	commandQueue.push_back( command );
	commandMutex.unlock();
}

