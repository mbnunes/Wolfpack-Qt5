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
#include <map>
#include <qmap.h>

// Forward definitions
class WPDefaultScript;
class QString;

#define OBJECT_OBJECT	1
#define OBJECT_ITEM		2
#define OBJECT_CHAR		3

#define EVENT_USE					0x00000001
#define EVENT_SINGLECLICK			0x00000002
#define EVENT_COLLIDE				0x00000004
#define EVENT_UNUSED1				0x00000004
#define EVENT_WALK					0x00000010
#define EVENT_CREATE				0x00000020
#define EVENT_LOGIN					0x00000040
#define EVENT_LOGOUT				0x00000080
#define EVENT_TALK					0x00000100
#define EVENT_WARMODETOGGLE			0x00000200
#define EVENT_HELP					0x00000400
#define EVENT_CHAT					0x00000800
#define EVENT_SKILLUSE				0x00001000
#define EVENT_SELECTCONTEXTMENU		0x00002000
#define EVENT_REQUESTCONTEXTMENU	0x00004000
#define EVENT_DROPONCHAR			0x00008000
#define EVENT_DROPONITEM			0x00010000
#define EVENT_DROPONGROUND			0x00020000
#define EVENT_PICKUP				0x00040000
#define EVENT_SPEECH				0x00080000
#define EVENT_BEGINCAST				0x00100000
#define EVENT_ENDCAST				0x00200000
#define EVENT_SPELLCHECKTARGET		0x00400000
#define EVENT_SPELLSUCCESS			0x00800000
#define EVENT_SPELLFAILURE			0x01000000
#define EVENT_SHOWTOOLTIP			0x02000000

class WPScriptManager  
{
private:
	std::map< QString, WPDefaultScript* > Scripts;

	// map< OBJECT, map< EVENT, vector< WPDefaultScript
	QMap< UINT32, QMap< UINT32, QValueVector< WPDefaultScript* > > > globalhooks;

	// map< QString, WPDefaultScript >
	QMap< QString, WPDefaultScript* > commandhooks;

public:
	typedef std::map< QString, WPDefaultScript* >::iterator iterator;

	virtual ~WPScriptManager();

	WPDefaultScript* find( const QString& Name ) const;
	void add( const QString& Name, WPDefaultScript *Script );
	void remove( const QString& Name );

	void load( void );
	void reload( void );
	void unload( void );

	void onServerStart(); // Call the onServerStart Event
	void onServerStop(); // Call the onServerEnd Event

	void addCommandHook( const QString &command, WPDefaultScript *script );
	void addGlobalHook( UINT32 object, UINT32 event, WPDefaultScript *script );
	void clearGlobalHooks() { globalhooks.clear(); }
	void clearCommandHooks() { commandhooks.clear(); }
	WPDefaultScript *getCommandHook( const QString &command );
	const QValueVector< WPDefaultScript* > getGlobalHooks( UINT32 object, UINT32 event );
};

#endif
