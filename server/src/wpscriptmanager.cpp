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
#include "qstring.h"

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
	// First unload, then reload
	unload();
	reloadPython();
	load();

	// After reloading all scripts we *need* to recreate all script-pointers 
	// assigned to scripted items and characters
	// because all of them got invalidated while relaoding
	clConsole.PrepareProgress( "Recreating assigned Script-pointers" );

	AllItemsIterator iter_items;
	
	for( iter_items.Begin(); !iter_items.atEnd(); ++iter_items )
	{
		P_ITEM pi = iter_items.GetData();
		
		if( pi )
			pi->recreateEvents();
	}

	AllCharsIterator iter_chars;
	
	for( iter_chars.Begin(); !iter_chars.atEnd(); ++iter_chars )
	{
		P_CHAR pc = iter_chars.GetData();
		
		if( pc )
			pc->recreateEvents();
	}

	clConsole.ProgressDone();
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
}

void WPScriptManager::load( void )
{
	clConsole.PrepareProgress( "Loading Script Manager" );

	// Load the XML Script
	QStringList SectionList = DefManager->getSections( WPDT_SCRIPT );

	UI32 ScriptsLoaded = 0;

	// Each Section is a Script identifier
	for( UI32 i = 0; i < SectionList.count(); ++i )
	{
		const QDomElement *NodePtr = DefManager->getSection( WPDT_SCRIPT, SectionList[ i ] );

		if( !NodePtr->attributes().contains( QString( "type" ) ) )
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
