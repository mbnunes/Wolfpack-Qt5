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
#include "wpconsole.h"
#include "wpdefaultscript.h"

// Library Includes
#include "qstring.h"

#ifndef __unix__
#include <windows.h>
#endif

#include <iostream>

using namespace std;

//========================================================================================
// Constructor
WPConsole_cl::WPConsole_cl()
{
	bEnabled = true;
	// do nothing at the moment
	setStreams(&cin, &cout, &cerr, &cout);
}

//========================================================================================
// Destuctor
WPConsole_cl::~WPConsole_cl()
{
	// Clean up any terminal settings
}

void WPConsole_cl::enabled(bool bState)
{
	bEnabled = bState;
}

void WPConsole_cl::setStreams(istream *in, ostream *out, ostream *error, ostream *log)
{
	inputstrm  = in;
	outputstrm = out;
	errorstrm  = error;
	logstrm    = log;
}

//========================================================================================
// Send a char string to the console
void WPConsole_cl::send(char* szMessage, ...)
{
	va_list argptr;
	char msg[512];
	va_start(argptr, szMessage);
	vsnprintf(msg, 512, szMessage, argptr);
	va_end(argptr);
	
	QString sMessage(msg);
	send(sMessage);
}

//========================================================================================
// Send a message to the console
void WPConsole_cl::send(const QString &sMessage)
{
	if( sMessage.contains( "\n" ) )
	{
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

	if( outputstrm != NULL )
	{
		(*outputstrm) << sMessage.latin1();
		flush( *outputstrm );
	}
}

//========================================================================================
void WPConsole_cl::log( UINT8 logLevel, const QString &message )
{
	switch( logLevel )
	{
	case LOG_ERROR:
	case LOG_FATAL:
		ChangeColor( WPC_RED );
		send( "ERROR" );
		ChangeColor( WPC_NORMAL );
		break;
	case LOG_NOTICE:
		ChangeColor( WPC_WHITE );
		send( "NOTICE" );
		ChangeColor( WPC_NORMAL );
		break;
	case LOG_WARNING:
		ChangeColor( WPC_YELLOW );
		send( "WARNING" );
		ChangeColor( WPC_NORMAL );
		break;
	}

	send( ": " + message + "\n" );
}

//========================================================================================
// Send a char string to the error
void WPConsole_cl::error(char* szMessage, ...)
{
	va_list argptr;
	char msg[512];
	va_start(argptr, szMessage);
	vsnprintf(msg, 512,szMessage, argptr);
	va_end(argptr);
	
	QString sMessage(msg);
	error(sMessage);
}
//========================================================================================
// Send a message to the console
void WPConsole_cl::error(const QString& sMessage)
{
	if (errorstrm != NULL)
		(*errorstrm) << sMessage;
}

//=========================================================================================
// Get input from the console
UI08 WPConsole_cl::getkey(void)
{
	UI08 key = 0;

//	if (cin.peek())
//		key = cin.get();
	
	return key;
}

//=========================================================================================
// Prepare a "progess" line
void WPConsole_cl::PrepareProgress( const QString &sMessage )
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
void WPConsole_cl::ProgressDone( void )
{
	send( "\b\b\b\b\b" ); // Go 5 Characters back
	ChangeColor( WPC_GREEN );
	send( "done" );
	ChangeColor( WPC_NORMAL );
	send( "]\n" );
}

//=========================================================================================
// Print "Fail"
void WPConsole_cl::ProgressFail( void )
{
	send( "\b\b\b\b\b" ); // Go 5 Characters back
	ChangeColor( WPC_RED );
	send( "fail" );
	ChangeColor( WPC_NORMAL );
	send( "]\n" );
}

//=========================================================================================
// Print "Skip" (maps etc.)
void WPConsole_cl::ProgressSkip( void )
{
	send( "\b\b\b\b\b" ); // Go 5 Characters back
	ChangeColor( WPC_YELLOW );
	send( "skip" );
	ChangeColor( WPC_NORMAL );
	send( "]\n" );
}

//=========================================================================================
// Change the console Color
void WPConsole_cl::ChangeColor( WPC_ColorKeys Color )
{
#if defined(__unix__)
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
#endif
	#ifndef __unix__
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

void WPConsole_cl::setConsoleTitle( const QString& data )
{
#ifndef __unix__
	SetConsoleTitle( data.latin1() );
#endif
}


