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

// Wolfpack Includes

#include "wpconsole.h"

// Method Implementations

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
	
	string sMessage(msg);
	send(sMessage);
}

//========================================================================================
// Send a message to the console
void WPConsole_cl::send(string sMessage)
{
	if (outputstrm != NULL)
	{
		(*outputstrm) << sMessage;
		flush((*outputstrm));
	}
}
//========================================================================================
// Send a char string to the log
void WPConsole_cl::log(char* szMessage, ...)
{
	va_list argptr;
	char msg[512];

	va_start(argptr, szMessage);
	vsnprintf(msg, 512, szMessage, argptr);
	va_end(argptr);
	
	string sMessage(msg);
	log(sMessage);
}
//========================================================================================
// Send a message to the log
void WPConsole_cl::log(string sMessage)
{
	if (logstrm != NULL)
		(*logstrm) << sMessage;

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
	
	string sMessage(msg);
	error(sMessage);
}
//========================================================================================
// Send a message to the console
void WPConsole_cl::error(string sMessage)
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


