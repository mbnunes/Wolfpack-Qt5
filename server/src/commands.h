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
#if !defined( __COMMANDS_H__ )
#define __COMMANDS_H__

// Library Includes
#include <map>
#include "qstring.h"
#include "qstringlist.h"

using namespace std;

class cUOSocket;

struct stCommand
{
	const char *name;
	void (*command)( cUOSocket*, const QString&, QStringList& );	
};

struct stPrivEntry
{
	QStringList commands;
	bool		implicit;
	
	stPrivEntry() { implicit = true; };
};

class cCommands
{
private:
	map< QString, stPrivEntry > privlvl_commands;
	static stCommand commands[];
public:
	// Command processing system
	void process( cUOSocket *socket, const QString &command );
	void dispatch( cUOSocket *socket, const QString &command, QStringList &arguments );

	static cCommands *instance()
	{
		static cCommands instance_;
		return &instance_;
	}

	// Privlevel System
	void addCmdToPrivLvl( QString privlvl, QString command );
	void rmvCmdFromPrivLvl( QString privlvl, QString command );
	bool containsCmd( QString privlvl, QString command );
	void loadPrivLvlCmds( void );
};

#endif