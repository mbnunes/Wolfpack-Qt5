//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
#include <qapplication.h>

using namespace std;

cScriptManager::cScriptManager()
{
	for( unsigned int i = 0; i < EVENT_COUNT; ++i )
		hooks[i] = 0;
}

cScriptManager::~cScriptManager()
{
	unload();
}

cPythonScript* cScriptManager::find( const QCString &name )
{
	cScriptManager::iterator it = scripts.find( name );
	
	if ( it != scripts.end() )
		return it.data();
	else
		return 0;
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
	cItemIterator iter_items;
	
	for( P_ITEM pItem = iter_items.first(); pItem; pItem = iter_items.next() )
		pItem->recreateEvents();

	cCharIterator iter_chars;
	
	for( P_CHAR pChar = iter_chars.first(); pChar; pChar = iter_chars.next() )
		pChar->recreateEvents();

	serverState = RUNNING;
}

// Unload all scripts
void cScriptManager::unload()
{
	cScriptManager::iterator it;
	
	for( it = scripts.begin(); it != scripts.end(); ++it )
	{
		it.data()->unload();
		delete it.data();
	}

	scripts.clear();

	for( unsigned int i = 0; i < EVENT_COUNT; ++i )
	{
		hooks[i] = 0;
	}

	QMap< QCString, PyObject* >::iterator itc;

	for( itc = commandhooks.begin(); itc != commandhooks.end(); ++itc )
	{
		Py_XDECREF( itc.data() );
	}

	commandhooks.clear();
}

void cScriptManager::load()
{
	Console::instance()->PrepareProgress( "Loading ScriptManager" );

	// Each Section is a Script identifier
	const QValueVector< cElement* > &sections = DefManager->getDefinitions( WPDT_SCRIPT );

	unsigned int i;

	for( i = 0; i < sections.size(); ++i )
	{
		const cElement *element = sections[i];
		
		if( scripts.contains( element->text().latin1() ) )
		{
			Console::instance()->ProgressFail();
			Console::instance()->PrepareProgress( "Continuing..." );
			Console::instance()->log( LOG_WARNING, QString( "Duplicate Script: %1" ).arg( element->text().utf8() ) );
			continue;			
		}

		
		cPythonScript *script = new cPythonScript;
		scripts.replace( element->text().utf8(), script );
		script->load( element );
	}

	Console::instance()->ProgressDone();
	Console::instance()->send( QString( "%1 Script(s) loaded\n" ).arg( i ) );
}

void cScriptManager::onServerStart()
{
	cScriptManager::iterator it;

	for( it = scripts.begin(); it != scripts.end(); ++it )
	{
		it.data()->callEventHandler( "onServerStart" );
	}
}

void cScriptManager::onServerStop()
{
	cScriptManager::iterator it;

	for( it = scripts.begin(); it != scripts.end(); ++it )
	{
		it.data()->callEventHandler( "onServerStop" );
	}
}

PyObject *cScriptManager::getCommandHook( const QCString &command )
{
	PyObject *result = 0;

	if( commandhooks.contains( command.lower() ) )
	{
		result = commandhooks[ command.lower() ];
	}

	return result;
}

cPythonScript *cScriptManager::getGlobalHook( ePythonEvent event )
{
	cPythonScript *result = 0;

	if( event < EVENT_COUNT )
	{
		result = hooks[ event ];
	}

	return result;
}

void cScriptManager::setCommandHook( const QCString &command, PyObject *object )
{
	if( commandhooks.contains( command.lower() ) )
	{
		Py_DECREF( commandhooks[ command.lower() ] );
	}

	commandhooks.replace( command.lower(), object );
}

void cScriptManager::setGlobalHook( ePythonEvent event, cPythonScript *script )
{
	if( event < EVENT_COUNT )
		hooks[event] = script;
}
