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

#include "wpdefmanager.h"
#include "globals.h"
#include "prototypes.h"
#include "basics.h"
#include "inlines.h"

// Library Includes
#include "qdom.h"
#include "qfile.h"
#include "qregexp.h"

// Method for processing one node
void WPDefManager::ProcessNode( QDomElement Node )
{
	if( Node.isNull() ) 
		return;

	QString NodeName = Node.nodeName();

	if( NodeName == "include" )
	{
		// Try to get the filename
		// <include file="data\npcs.xml" \>
		if( !Node.attributes().contains( "file" ) )
			return;

		// Get the filename and import it
		ImportSections( Node.attribute("file") );
		return;
	}

	// Get the Node ID
	if( !Node.attributes().contains( "id" ) )
		return;

	QString NodeID = Node.attribute("id");

	// IF's for all kind of nodes
	// <item id="xx">
	// <script id="xx">
	// <npc id="xx">
	if( NodeName == "item" )
		Items.insert( NodeID, Node );
	else if( NodeName == "script" )
		Scripts.insert( NodeID, Node );
	else if( NodeName == "npc" )
		NPCs.insert( NodeID, Node );
	else if( NodeName == "menu" )
		Menus.insert( NodeID, Node );
	else if( NodeName == "spell" )
		Spells.insert( NodeID, Node );
	else if( NodeName == "list" )
		StringLists.insert( NodeID, Node );
	else if( NodeName == "acl" )
		PrivLevels.insert( NodeID, Node );
	else if( NodeName == "spawnregion" )
		SpawnRegions.insert( NodeID, Node );
	else if( NodeName == "region" )
		Regions.insert( NodeID, Node );
	else if( NodeName == "multi" )
		Multis.insert( NodeID, Node );
	else if( NodeName == "text" )
		Texts.insert( NodeID, Node );
	else if( NodeName == "startitems" )
		StartItems.insert( NodeID, Node );
	else if( NodeName == "location" )
		Locations.insert( NodeID, Node );
	else if( NodeName == "skill" )
		Skills.insert( NodeID, Node );
	else if( NodeName == "action" )
		Actions.insert( NodeID, Node );
	else if( NodeName == "make" )
		MakeSections.insert( NodeID, Node );
	else if( NodeName == "makeitem" )
		MakeItems.insert( NodeID, Node );
	else if( NodeName == "useitem" )
		UseItems.insert( NodeID, Node );
	else if( NodeName == "skillcheck" )
		SkillChecks.insert( NodeID, Node );
	else if( NodeName == "define" )
		Defines.insert( NodeID, Node );
	else if( NodeName == "resource" )
		Resources.insert( NodeID, Node );
 	else if( NodeName == "contextmenu" )	
 		ContextMenus.insert( NodeID, Node );

}

// Recursive Function for Importing Script Sections
bool WPDefManager::ImportSections( const QString& FileName )
{    
	QDomDocument Document( "definitions" );
	QFile File( FileName );

    if ( !File.open( IO_ReadOnly ) )
	{
		clConsole.ProgressFail();
	
		clConsole.send( "Unable to open " );
		clConsole.send( FileName );
		clConsole.send( "!\n" );
		return false;
	}

	QString errorMessage;
	int errorLine, errorColumn;
	
	clConsole.PrepareProgress( tr( "Parsing %1" ).arg( FileName ) );

	QByteArray data = File.readAll();

	if( !Document.setContent( data, false, &errorMessage, &errorLine, &errorColumn ) ) 
	{
        File.close();
        
		clConsole.ProgressFail();
		clConsole.send( QString("Unable to parse file %1\nError:%2(%3:%4)\n").arg(FileName).arg(errorMessage).arg(errorLine).arg(errorColumn) );

		return false;
	}

	clConsole.ProgressDone();

    File.close();

	// Get the first document node and start to process it
	QDomElement Definitions = Document.documentElement();
	QDomNodeList NodeList = Definitions.childNodes();

	// Process all nodes
	for( UI32 i = 0; i < NodeList.count(); ++i )
	{
		if( NodeList.item( i ).isElement() )
		{
			ProcessNode( NodeList.item( i ).toElement() );
		}
	}

	return true;
}

// Clears nodes within the specified nodeMap
void clearNodes( DefSections &nodeMap )
{
	DefSections::iterator myIter;

	for( myIter = nodeMap.begin(); myIter != nodeMap.end(); ++myIter )
	{
		QDomElement node = myIter.data();
		node.clear(); // Delete all subnodes - i'm unsure if this delets all data
	}

	nodeMap.clear(); // Delete it's contents
}

void WPDefManager::unload( void )
{
	// Clear all nodes inside our local tree
	clearNodes( Items );
	clearNodes( Scripts );
	clearNodes( NPCs );
	clearNodes( StringLists );
	clearNodes( Menus );
	clearNodes( Spells );
	clearNodes( PrivLevels );
	clearNodes( SpawnRegions );
	clearNodes( Regions );
	clearNodes( Multis );
	clearNodes( Texts );
	clearNodes( StartItems );
	clearNodes( Locations );
	clearNodes( Skills );
	clearNodes( Actions );
	clearNodes( MakeSections );
	clearNodes( MakeItems );
	clearNodes( UseItems );
	clearNodes( SkillChecks );
	clearNodes( Defines );
	clearNodes( Resources );
 	clearNodes( ContextMenus );

}

void WPDefManager::reload( void )
{
	// First unload then reload
	unload();

	// Load them once again
	load();
}

// Load the Definitions
void WPDefManager::load( void )
{
	ImportSections( "definitions.xml" );
}

// Returns one Section
QDomElement *WPDefManager::getSection( WPDEF_TYPE Type, QString Section )
{
	DefSections *ListPointer;

	switch( Type )
	{
	case WPDT_ITEM:
		ListPointer = &Items;
		break;
	
	case WPDT_SCRIPT:
		ListPointer = &Scripts;
		break;

	case WPDT_NPC:
		ListPointer = &NPCs;
		break;

	case WPDT_LIST:
		ListPointer = &StringLists;
		break;

	case WPDT_MENU:
		ListPointer = &Menus;
		break;

	case WPDT_SPELL:
		ListPointer = &Spells;
		break;

	case WPDT_PRIVLEVEL:
		ListPointer = &PrivLevels;
		break;

	case WPDT_SPAWNREGION:
		ListPointer = &SpawnRegions;
		break;

	case WPDT_REGION:
		ListPointer = &Regions;
		break;

	case WPDT_MULTI:
		ListPointer = &Multis;
		break;

	case WPDT_TEXT:
		ListPointer = &Texts;
		break;

	case WPDT_STARTITEMS:
		ListPointer = &StartItems;
		break;

	case WPDT_LOCATION:
		ListPointer = &Locations;
		break;

	case WPDT_SKILL:
		ListPointer = &Skills;
		break;

	case WPDT_ACTION:
		ListPointer = &Actions;
		break;

	case WPDT_MAKESECTION:
		ListPointer = &MakeSections;
		break;

	case WPDT_MAKEITEM:
		ListPointer = &MakeItems;
		break;

	case WPDT_USEITEM:
		ListPointer = &UseItems;
		break;

	case WPDT_SKILLCHECK:
		ListPointer = &SkillChecks;
		break;

	case WPDT_DEFINE:
		ListPointer = &Defines;
		break;

	case WPDT_RESOURCE:
		ListPointer = &Resources;
		break;

	case WPDT_CONTEXTMENU:
 		ListPointer = &ContextMenus;
 		break;

	default:
		return 0;
	};

	return &( ListPointer->find( Section ).data() );
}

// Returns a list of section-names found
QStringList WPDefManager::getSections( WPDEF_TYPE Type )
{
	QStringList SectionList;
	DefSections *ListPointer;
	DefSections::iterator MyIter;

	switch( Type )
	{
	case WPDT_ITEM:
		ListPointer = &Items;
		break;
	
	case WPDT_SCRIPT:
		ListPointer = &Scripts;
		break;

	case WPDT_NPC:
		ListPointer = &NPCs;
		break;

	case WPDT_MENU:
		ListPointer = &Menus;
		break;

	case WPDT_SPELL:
		ListPointer = &Spells;
		break;

	case WPDT_LIST:
		ListPointer = &StringLists;
		break;

	case WPDT_PRIVLEVEL:
		ListPointer = &PrivLevels;
		break;

	case WPDT_SPAWNREGION:
		ListPointer = &SpawnRegions;
		break;

	case WPDT_REGION:
		ListPointer = &Regions;
		break;

	case WPDT_MULTI:
		ListPointer = &Multis;
		break;

	case WPDT_TEXT:
		ListPointer = &Texts;
		break;

	case WPDT_STARTITEMS:
		ListPointer = &StartItems;
		break;

	case WPDT_LOCATION:
		ListPointer = &Locations;
		break;

	case WPDT_SKILL:
		ListPointer = &Skills;
		break;

	case WPDT_ACTION:
		ListPointer = &Actions;
		break;

	case WPDT_MAKESECTION:
		ListPointer = &MakeSections;
		break;

	case WPDT_MAKEITEM:
		ListPointer = &MakeItems;
		break;

	case WPDT_USEITEM:
		ListPointer = &UseItems;
		break;

	case WPDT_SKILLCHECK:
		ListPointer = &SkillChecks;
		break;

	case WPDT_DEFINE:
		ListPointer = &Defines;
		break;

	case WPDT_RESOURCE:
		ListPointer = &Resources;
		break;

	case WPDT_CONTEXTMENU:
 		ListPointer = &ContextMenus;
 		break;

	default:
		// Return an empty list
		return SectionList;
	};

	for( MyIter = ListPointer->begin(); MyIter != ListPointer->end(); ++MyIter )
	{
		SectionList.push_back( MyIter.key() );
	}

	return SectionList;
}

QString	WPDefManager::getRandomListEntry( QString ListSection )
{
	QStringList list = this->getList( ListSection );
	if( list.isEmpty() )
		return QString();
	else
		return list[ RandomNum( 0, list.size()-1 ) ];
}

QStringList	WPDefManager::getList( QString ListSection )
{
	QDomElement* DefSection = this->getSection( WPDT_LIST, ListSection );
	QStringList list = QStringList();
	QString data;

	if( !DefSection->isNull() )
	{
		QDomNode childNode = DefSection->firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				// Using the nodename is a very very bad habit
				// if the name of the node is "item" then
				// use the node value instead
				QDomElement childTag = childNode.toElement();
				
				if( childTag.nodeName() == "item" )
					data = childTag.text();
				else
					data = childTag.nodeName();

				int mult = childTag.attribute( "mult" ).toInt();
				if( mult <= 0 )
					mult = 1;
				int i = 0;
				while( i < mult )
				{
					list.push_back( data );
					i++;
				}
			}
			childNode = childNode.nextSibling();
		}
	}
	return list;
}

QString WPDefManager::getText( QString TextSection )
{
	QDomElement* DefSection = this->getSection( WPDT_TEXT, TextSection );
	if( DefSection->isNull() )
		return QString();
	else
	{
		QString text = DefSection->text();
		if( text.left( 1 ) == "\n" || text.left( 0 ) == "\r" )
			text = text.right( text.length()-1 );
		text = text.replace( QRegExp( "\\t" ), "" );
		if( text.right( 1 ) == "\n" || text.right( 1 ) == "\r" )
			text = text.left( text.length()-1 );
		return text;
	}
}

