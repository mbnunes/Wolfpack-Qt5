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

#include "wpscriptmanager.h"
#include "wpdefmanager.h"
#include "wolfpack.h"
#include <qstring.h>

WPScriptManager::WPScriptManager()
{

}

WPScriptManager::~WPScriptManager()
{
	std::map< QString, WPDefaultScript* >::iterator ScriptIterator;

	for( ScriptIterator = Scripts.begin(); ScriptIterator != Scripts.end(); ++ScriptIterator )
	{
		delete( ScriptIterator->second );
	}
}


WPDefaultScript* WPScriptManager::FindScript( QString Name )
{
	return Scripts[ Name ];
}

void WPScriptManager::AddScript( QString Name, WPDefaultScript *Script )
{
	RemoveScript( Name );

	Script->setName( Name );

	Scripts[ Name ] = Script;
}

void WPScriptManager::RemoveScript( QString Name )
{
	if( Scripts.find( Name ) != Scripts.end() )
	{
		delete Scripts[ Name ];
		Scripts.erase( Name );
	}
}

void WPScriptManager::Load( void )
{
	clConsole.PrepareProgress( "Loading Script Manager" );

	// Load the XML Script
	QStringList SectionList = DefManager->getSections( WPDT_SCRIPT );

	UI32 ScriptsLoaded = 0;

	// Each Section is a Script identifier
	for( UI32 i = 0; i < SectionList.count(); i++ )
	{
		QDomNode *NodePtr = DefManager->getSection( WPDT_SCRIPT, SectionList[ i ] );
		QDomElement Node = NodePtr->toElement();

		WPDefaultScript *Script;

		if( !Node.attributes().contains( QString( "type" ) ) )
			continue;

		QString ScriptType = Node.attributes().namedItem( QString( "type" ) ).nodeValue();

		// Decide upon the Constructor based on the script-type
		if( ScriptType == "default" )
			Script = new WPDefaultScript;
		else if( ScriptType == "python" )
			Script = (WPDefaultScript*)new WPPythonScript;
		else
			continue;
	
		Script->load( Node );
		AddScript( SectionList[ i ].ascii(), Script );
		ScriptsLoaded++;
	}

	clConsole.ProgressDone();
	QString Message;
	Message.sprintf( "%d Script(s) loaded successfully\n", ScriptsLoaded );
	
	clConsole.send( Message.ascii() );
}
