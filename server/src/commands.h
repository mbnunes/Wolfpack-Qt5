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

// Wolfpack Includes
#include "wpdefaultscript.h"

// Library Includes
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

class cUOSocket;

struct stCommand
{
	const char *name;
	void (*command)( cUOSocket*, const QString&, QStringList& );	
};

class cAcl
{
public:
	QString name;
	QMap< QString, QMap< QString, bool > > groups;
};

// ACL:
// Group -> Command -> Permitted

class cCommands : public QObject
{
	Q_OBJECT
private:
	QMap< QString, cAcl* > _acls;
	QMap< QString, WPDefaultScript > _customcommands;
	static stCommand commands[];
public:
	void addCustomCommand( const QString &name, WPDefaultScript *script );
	void clearCustomCommands();

	// Command processing system
	void process( cUOSocket *socket, const QString &command );
	void dispatch( cUOSocket *socket, const QString &command, QStringList &arguments );

	static cCommands *instance()
	{
		static cCommands instance_;
		return &instance_;
	}

	// Privlevel System
	void loadACLs( void );
	cAcl *getACL( const QString& );
};

inline cAcl *cCommands::getACL( const QString& key )
{
	if( _acls.find( key ) == _acls.end() )
		return 0;
	else
		return _acls.find( key ).data();
}


#endif

