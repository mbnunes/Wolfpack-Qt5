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

#if !defined(__WPSCRIPTMANAGER_H__)
#define __WPSCRIPTMANAGER_H__

// System Includes
#include <qmap.h>
#include <qvaluevector.h>

// Forward definitions
class cPythonScript;
class QString;

#define OBJECT_OBJECT	1
#define OBJECT_ITEM		2
#define OBJECT_CHAR		3

#define EVENT_USE					1
#define EVENT_SINGLECLICK			2
#define EVENT_COLLIDE				3
#define EVENT_WORLDSAVE				4
#define EVENT_WORLDLOAD				5
#define EVENT_WALK					6
#define EVENT_CREATE				7
#define EVENT_LOGIN					8
#define EVENT_LOGOUT				9
#define EVENT_TALK					10
#define EVENT_WARMODETOGGLE			11
#define EVENT_HELP					12
#define EVENT_CHAT					13
#define EVENT_SKILLUSE				14
#define EVENT_SELECTCONTEXTMENU		15
#define EVENT_REQUESTCONTEXTMENU	16
#define EVENT_DROPONCHAR			17
#define EVENT_DROPONITEM			18
#define EVENT_DROPONGROUND			19
#define EVENT_PICKUP				20
#define EVENT_SPEECH				21
#define EVENT_BEGINCAST				22
#define EVENT_ENDCAST				23
#define EVENT_SPELLCHECKTARGET		24
#define EVENT_SPELLSUCCESS			25
#define EVENT_SPELLFAILURE			26
#define EVENT_SHOWTOOLTIP			27
#define EVENT_SKILLGAIN				28
#define EVENT_DAMAGE				29
#define EVENT_SHOWPAPERDOLL			30
#define EVENT_SHOWSKILLGUMP			31
#define EVENT_STATGAIN				32
#define EVENT_CASTSPELL				33
#define EVENT_CHLEVELCHANGE			34

class cScriptManager  
{
private:
	QMap< QString, cPythonScript* > Scripts;

	// map< OBJECT, map< EVENT, vector< cPythonScript
	QMap< UINT32, QMap< UINT32, QValueVector< cPythonScript* > > > globalhooks;

	// map< QString, cPythonScript >
	QMap< QString, cPythonScript* > commandhooks;

public:
	typedef QMap< QString, cPythonScript* >::iterator iterator;

	virtual ~cScriptManager();

	cPythonScript* find( const QString& Name ) const;
	void add( const QString& Name, cPythonScript *Script );
	void remove( const QString& Name );

	void load( void );
	void reload( void );
	void unload( void );

	void onServerStart(); // Call the onServerStart Event
	void onServerStop(); // Call the onServerEnd Event

	void addCommandHook( const QString &command, cPythonScript *script );
	void addGlobalHook( UINT32 object, UINT32 event, cPythonScript *script );
	void clearGlobalHooks() { globalhooks.clear(); }
	void clearCommandHooks() { commandhooks.clear(); }
	cPythonScript *getCommandHook( const QString &command );
	const QValueVector< cPythonScript* > getGlobalHooks( UINT32 object, UINT32 event ) const;
};

#endif
