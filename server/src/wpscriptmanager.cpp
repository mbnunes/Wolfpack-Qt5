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

#include "wolfpack.h"
#include "wpscriptmanager.h"
#include "wpdefmanager.h"
#include "wpdefaultscript.h"

#include "python/wppythonscript.h"
#include "python/engine.h"

// Library Includes
#include <qstring.h>

using namespace std;

WPScriptManager::~WPScriptManager()
{
	map< QString, WPDefaultScript* >::iterator ScriptIterator;

	for( ScriptIterator = Scripts.begin(); ScriptIterator != Scripts.end(); ++ScriptIterator )
	{
		delete( ScriptIterator->second );
	}
}


WPDefaultScript* WPScriptManager::find( const QString& Name ) const
{
	map< QString, WPDefaultScript* >::const_iterator it = Scripts.find( Name );
	if ( it != Scripts.end() )
		return (*it).second;
	else
		return 0;
}

void WPScriptManager::add( const QString& Name, WPDefaultScript *Script )
{
	remove( Name );

	Script->setName( Name );

	Scripts.insert( make_pair(Name, Script) );
}

void WPScriptManager::remove( const QString& Name )
{
	iterator it = Scripts.find( Name );
	if( it != Scripts.end() )
	{
		delete (*it).second;
		Scripts.erase( it );
	}
}

void WPScriptManager::reload( void )
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
	clConsole.PrepareProgress( "Recreating assigned Script-pointers" );

	cItemIterator iter_items;
	
	for( P_ITEM pItem = iter_items.first(); pItem; pItem = iter_items.next() )
		pItem->recreateEvents();

	cCharIterator iter_chars;
	
	for( P_CHAR pChar = iter_chars.first(); pChar; pChar = iter_chars.next() )
		pChar->recreateEvents();

	clConsole.ProgressDone();
	serverState = RUNNING;
}

// Unload all scripts
void WPScriptManager::unload( void )
{
	WPScriptManager::iterator myIter;

	for( myIter = Scripts.begin(); myIter != Scripts.end(); ++myIter )
	{
		myIter->second->unload();
		delete myIter->second;
	}

	Scripts.clear();
	globalhooks.clear();
	commandhooks.clear();
}

void WPScriptManager::load()
{
	clConsole.PrepareProgress( "Loading Script Manager" );

	// Load the XML Script
	QStringList SectionList = DefManager->getSections( WPDT_SCRIPT );

	UI32 ScriptsLoaded = 0;

	// Each Section is a Script identifier
	for( UI32 i = 0; i < SectionList.count(); ++i )
	{
		const QDomElement *NodePtr = DefManager->getSection( WPDT_SCRIPT, SectionList[ i ] );

		if( !NodePtr || NodePtr->isNull() || !NodePtr->attributes().contains( QString( "type" ) ) )
			continue;

		QString ScriptType = NodePtr->attributes().namedItem( QString( "type" ) ).nodeValue();

		WPDefaultScript *Script = 0;

		// Decide upon the Constructor based on the script-type
		if( ScriptType == "default" )
			Script = new WPDefaultScript;
		else if( ScriptType == "python" )
			Script = new WPPythonScript;
		else
			continue;
	
		add( SectionList[ i ].latin1(), Script );
		Script->load( *NodePtr );
		++ScriptsLoaded;
	}

	clConsole.ProgressDone();
	clConsole.send( QString("%1 Script(s) loaded successfully\n").arg(ScriptsLoaded) );
}

void WPScriptManager::addCommandHook( const QString &command, WPDefaultScript *script )
{
	commandhooks[ command.upper() ] = script;
}

void WPScriptManager::addGlobalHook( UINT32 object, UINT32 event, WPDefaultScript *script )
{
	// Find first
	QValueVector< WPDefaultScript* > &vec = globalhooks[ object ][ event ];
	QValueVector< WPDefaultScript* >::const_iterator it = vec.begin();

	for( ; it != vec.end(); ++it )
	{
		if( *it == script )
			return;
	}	

	globalhooks[ object ][ event ].push_back( script );
}

WPDefaultScript *WPScriptManager::getCommandHook( const QString &command )
{
	QMap< QString, WPDefaultScript* >::const_iterator it( commandhooks.find( command ) );

	if( it == commandhooks.end() )
		return 0;

	return *it;
}

const QValueVector< WPDefaultScript* > WPScriptManager::getGlobalHooks( UINT32 object, UINT32 event )
{
	return globalhooks[ object ][ event ];  
}

void WPScriptManager::onServerStart()
{
	map< QString, WPDefaultScript* >::iterator ScriptIterator;

	for( ScriptIterator = Scripts.begin(); ScriptIterator != Scripts.end(); ++ScriptIterator )
	{
		ScriptIterator->second->onServerstart();
	}	
}

void WPScriptManager::onServerStop()
{
}
