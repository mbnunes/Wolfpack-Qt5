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

//##ModelId=3C5D9290034D
class WPConsole_cl
{
public:
	// Constructor
	//##ModelId=3C5D929102F3
	WPConsole_cl();
	// Destructor
	//##ModelId=3C5D929103C5
	~WPConsole_cl();

	//##ModelId=3C5D929103D9
	void enabled(bool);
	//##ModelId=3C5D929200B9
	void setStreams(istream *in, ostream *out, ostream *error, ostream *log);

	// Send a message to the console
	//##ModelId=3C5D92920115
	void send(char* szMessage, ...);
	//##ModelId=3C5D92920132
	void send(string sMessage);


	// Log a message
	//##ModelId=3C5D92920150
	void log(char* szMessage, ...);
	//##ModelId=3C5D9292016E
	void log(string sMessage);

	// Flag an error
	//##ModelId=3C5D92920179
	void error(char* szMessage, ...);
	//##ModelId=3C5D929201A0
	void error(string sMessage);

        // Get input from the console
	//##ModelId=3C5D929201B4
	UI08 getkey(void);

private:

	//##ModelId=3C5D92A603C5
	istream *inputstrm;
	//##ModelId=3C5D92A7010A
	ostream *outputstrm;
	//##ModelId=3C5D92A70133
	ostream *errorstrm;
	//##ModelId=3C5D92A7015B
	ostream *logstrm;

	//##ModelId=3C5D92A7016E
	bool bEnabled;
};
//==========================================================================================

#endif
