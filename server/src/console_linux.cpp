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

#include "console.h"

class cConsoleThread : public QThread
{
protected:
	virtual void run()
	{
		try
		{
			while( serverState < SHUTDOWN )
			{
				char c = getch();
				
				if( c > 0 && serverState == RUNNING )
				{
					Console::instance()->handleCommand( QChar( c ) );			
				}
				else
				{	
					Sleep( 100 );
				}
			}
		}
		// If there is any error: Quit.
		// It's better to have no console input
		// than a deadlocking server.
		catch( ... )
		{
		}
	}
};

cConsoleThread *thread = 0;

void cConsole::start()
{
	thread = new cConsoleThread;
	thread->start();
}

void cConsole::poll()
{
	// Normally we would check if there is a command in the command queue and execute it
}

void cConsole::stop()
{
	thread->wait();
	delete thread;
}


void cConsole::setConsoleTitle( const QString& data )
{
#if defined(Q_OS_WIN32)
	SetConsoleTitle( data.latin1() );
#endif
}

//=========================================================================================
// Change the console Color
void cConsole::ChangeColor( WPC_ColorKeys Color )
{
#if defined(Q_OS_UNIX)
QString cb = "\e[0m";
		switch( Color )
		{
		case WPC_GREEN: cb = "\e[1;32m";
			break;
		case WPC_RED:	cb = "\e[1;31m";
			break;
		case WPC_YELLOW:cb = "\e[1;33m";
			break;
		case WPC_NORMAL:cb = "\e[0m";
			break;
		case WPC_WHITE:	cb = "\e[1;37m";
			break;
		default: cb = "\e[0m";

		}
		send( cb );
#elif defined(Q_OS_WIN32)
		HANDLE ConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
		UI16 ColorKey = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;

		switch( Color )
		{
		case WPC_GREEN:
			ColorKey = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			break;

		case WPC_RED:
			ColorKey = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		case WPC_YELLOW:
			ColorKey = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		case WPC_NORMAL:
			ColorKey = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
			break;

		case WPC_WHITE:
			ColorKey = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		default:
			break;
		}

		SetConsoleTextAttribute( ConsoleHandle, ColorKey );
#endif
}

//========================================================================================
// Send a message to the console
void cConsole::send(const QString &sMessage)
{
	if( outputstrm != NULL )
	{
		(*outputstrm) << sMessage.latin1();
		flush( *outputstrm );
	}

	if( sMessage.contains( "\n" ) )
	{
#if defined(Q_OS_UNIX) && 0 
		sMessage.replace("\e[0m", "");
	  	sMessage.replace("\e[1;32m", "");
		sMessage.replace("\e[1;31m", "");
		sMessage.replace("\e[1;33m", "");
		sMessage.replace("\e[1;37m", "");
#endif
		incompleteLine_.append( sMessage ); // Split by \n
		QStringList lines = QStringList::split( "\n", incompleteLine_, true );

		// Insert all except the last element
		for( int i = 0; i < lines.count()-1; ++i )
			linebuffer_.push_back( lines[i] );

		incompleteLine_ = lines[ lines.count() - 1 ];
	}
	else
	{
		incompleteLine_.append( sMessage );
	}
}
