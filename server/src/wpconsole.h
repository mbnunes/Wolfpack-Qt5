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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================
#if !defined(__WPCONSOLE_H__)
#define __WPCONSOLE_H__

// Platform specifics
#include "platform.h"


// System Includes

#include <iostream>
#include <string>
#include <cstdarg>
#include <cstdio>

using namespace std;


// Third Party includes



//Forward class declaration

class WPConsole_cl;

//Wolfpack Includes


//Class definitions

class WPConsole_cl
{
public:
	// Constructor
	WPConsole_cl();
	// Destructor
	~WPConsole_cl();

	void enabled(bool);
	void setStreams(istream *in, ostream *out, ostream *error, ostream *log);

	// Send a message to the console
	void send(char* szMessage, ...);
	void send(string sMessage);


	// Log a message
	void log(char* szMessage, ...);
	void log(string sMessage);

	// Flag an error
	void error(char* szMessage, ...);
	void error(string sMessage);

        // Get input from the console
	UI08 getkey(void);

private:

	istream *inputstrm;
	ostream *outputstrm;
	ostream *errorstrm;
	ostream *logstrm;

	bool bEnabled;
};
//==========================================================================================

#endif
