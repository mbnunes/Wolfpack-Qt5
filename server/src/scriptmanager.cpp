//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#include "qapplication.h"
#include "scriptmanager.h"
#include "wpdefmanager.h"
#include "basechar.h"
#include "globals.h"
#include "console.h"
#include "world.h"
#include "items.h"
#include "pythonscript.h"
#include "python/engine.h"

// Library Includes
#include <qstring.h>
#include <qregexp.h>

using namespace std;

cScriptManager::~cScriptManager()
{
	QMap< QString, cPythonScript* >::iterator ScriptIterator;

	for( ScriptIterator = Scripts.begin(); ScriptIterator != Scripts.end(); ++ScriptIterator )
	{
		delete( ScriptIterator.data() );
	}
}


cPythonScript* cScriptManager::find( const QString& Name ) const
{
	QMap< QString, cPythonScript* >::const_iterator it = Scripts.find( Name );
	if ( it != Scripts.end() )
		return it.data();
	else
		return 0;
}

void cScriptManager::add( const QString& Name, cPythonScript *Script )
{
	remove( Name );

	Script->setName( Name );

	Scripts.insert( Name, Script );
}

void cScriptManager::remove( const QString& Name )
{
	iterator it = Scripts.find( Name );
	if( it != Scripts.end() )
	{
		delete it.data();
		Scripts.erase( it );
	}
}

void cScriptManager::reload( void )
{
	serverState = SCRIPTRELOAD;
	// First unload, then reload
	unload();

	// Stop + Restart Python
	stopPython();
	startPython( qApp->argc(), qApp->argv(), true );

	load();

	// After reloading all scripts we *need* to recreate all script-pointers 
	// assigned to scripted items and characters
	// because all of them got invalidated while relaoding
	Console::instance()->PrepareProgress( "Recreating assigned Script-pointers" );

	cItemIterator iter_items;
	
	for( P_ITEM pItem = iter_items.first(); pItem; pItem = iter_items.next() )
		pItem->recreateEvents();

	cCharIterator iter_chars;
	
	for( P_CHAR pChar = iter_chars.first(); pChar; pChar = iter_chars.next() )
		pChar->recreateEvents();

	Console::instance()->ProgressDone();
	serverState = RUNNING;
}

// Unload all scripts
void cScriptManager::unload( void )
{
	cScriptManager::iterator myIter;

	for( myIter = Scripts.begin(); myIter != Scripts.end(); ++myIter )
	{
		myIter.data()->unload();
		delete myIter.data();
	}

	Scripts.clear();
	globalhooks.clear();
	commandhooks.clear();
}

void cScriptManager::load()
{
	Console::instance()->PrepareProgress( "Loading Script Manager" );

	// Load the XML Script
	UI32 ScriptsLoaded = 0;

	// Each Section is a Script identifier
	const QValueVector< cElement* > &sections = DefManager->getDefinitions( WPDT_SCRIPT );

	for( unsigned int i = 0; i < sections.size(); ++i )
	{
		const cElement *element = sections[i];

		cPythonScript *script = new cPythonScript;
		add( element->text(), script );
		script->load( element );

		++ScriptsLoaded;
	}

	Console::instance()->ProgressDone();
	Console::instance()->send( QString("%1 Script(s) loaded\n").arg(ScriptsLoaded) );
}

void cScriptManager::addCommandHook( const QString &command, cPythonScript *script )
{
	commandhooks[ command.upper() ] = script;
}

void cScriptManager::addGlobalHook( UINT32 object, UINT32 event, cPythonScript *script )
{
	// Find first
	QValueVector< cPythonScript* > &vec = globalhooks[ object ][ event ];
	QValueVector< cPythonScript* >::const_iterator it = vec.begin();

	for( ; it != vec.end(); ++it )
	{
		if( *it == script )
			return;
	}	

	globalhooks[ object ][ event ].push_back( script );
}

cPythonScript *cScriptManager::getCommandHook( const QString &command )
{
	QMap< QString, cPythonScript* >::const_iterator it( commandhooks.find( command ) );

	if( it == commandhooks.end() )
		return 0;

	return *it;
}

const QValueVector< cPythonScript* > cScriptManager::getGlobalHooks( UINT32 object, UINT32 event ) const
{
	return globalhooks[ object ][ event ];  
}

void cScriptManager::onServerStart()
{
	QMap< QString, cPythonScript* >::iterator ScriptIterator;

	for( ScriptIterator = Scripts.begin(); ScriptIterator != Scripts.end(); ++ScriptIterator )
	{
		ScriptIterator.data()->onServerstart();
	}	
}

void cScriptManager::onServerStop()
{
}
