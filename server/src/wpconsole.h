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

#if !defined(__WPCONSOLE_H__)
#define __WPCONSOLE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iosfwd>
#include <cstdarg>
#include <cstdio>
#include "qstringlist.h"

// Third Party includes

// Forward class declaration
class QString;
class WPConsole_cl;
class WPDefaultScript;

// Wolfpack Includes


// Class definitions

enum WPC_ColorKeys 
{
	WPC_NORMAL = 0,
	WPC_RED,
	WPC_GREEN,
	WPC_YELLOW,
	WPC_WHITE,
};

// I'll use defines here
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_NOTICE 3
#define LOG_FATAL 4

class WPConsole_cl
{
private:
	QStringList linebuffer_;
	QString incompleteLine_;
public:
	QStringList linebuffer() const { return linebuffer_; }

	// Constructor
	WPConsole_cl();
	// Destructor
	~WPConsole_cl();
	void enabled(bool);
	void setStreams(std::istream *in, std::ostream *out, std::ostream *error, std::ostream *log);

	// Send a message to the console
	void send(char* szMessage, ...);
	void send(const QString &sMessage);


	// Log a message
	void log( UINT8 logLevel, const QString &message );

	// Flag an error
	void error(char* szMessage, ...);
	void error(const QString&);

        // Get input from the console
	UI08 getkey(void);

	// Prepare a 
	// xxxxx -----------------------[         ]
	// line
	void PrepareProgress( const QString &sMessage );
	void ProgressDone( void );
	void ProgressFail( void );
	void ProgressSkip( void );

	void ChangeColor( WPC_ColorKeys Color );
	void setConsoleTitle( const QString& data );

private:

	std::istream *inputstrm;
	std::ostream *outputstrm;
	std::ostream *errorstrm;
	std::ostream *logstrm;
	bool bEnabled;
};
//==========================================================================================

#endif
