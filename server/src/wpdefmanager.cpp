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
#include "wpconsole.h"

// Library Includes
#include <qdom.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>


// Method for processing one node
void WPDefManager::ProcessNode( const QDomElement& Node )
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

	DefSections *Sections = 0; // Defaults to Items

	// IF's for all kind of nodes
	// <item id="xx">
	// <script id="xx">
	// <npc id="xx">
	if( NodeName == "item" )
		Sections = &Items;
	else if( NodeName == "script" )
		Sections = &Scripts;
	else if( NodeName == "npc" )
		Sections = &NPCs;
	else if( NodeName == "menu" )
		Sections = &Menus;
	else if( NodeName == "spell" )
		Sections = &Spells;
	else if( NodeName == "list" )
		Sections = &StringLists;
	else if( NodeName == "acl" )
		Sections = &PrivLevels;
	else if( NodeName == "spawnregion" )
		Sections = &SpawnRegions;
	else if( NodeName == "region" )
		Sections = &Regions;
	else if( NodeName == "multi" )
		Sections = &Multis;
	else if( NodeName == "text" )
		Sections = &Texts;
	else if( NodeName == "startitems" )
		Sections = &StartItems;
	else if( NodeName == "location" )
		Sections = &Locations;
	else if( NodeName == "skill" )
		Sections = &Skills;
	else if( NodeName == "action" )
		Sections = &Actions;
	else if( NodeName == "make" )
		Sections = &MakeSections;
	else if( NodeName == "makeitem" )
		Sections = &MakeItems;
	else if( NodeName == "useitem" )
		Sections = &UseItems;
	else if( NodeName == "skillcheck" )
		Sections = &SkillChecks;
	else if( NodeName == "define" )
		Sections = &Defines;
	else if( NodeName == "resource" )
		Sections = &Resources;
 	else if( NodeName == "contextmenu" )	
 		Sections = &ContextMenus;

	if( !Sections )
		return;

	if( Sections->find( NodeID ) != Sections->end() )
	{
		clConsole.ChangeColor( WPC_YELLOW );
		clConsole.send( "Warning: " );
		clConsole.ChangeColor( WPC_NORMAL );
		clConsole.send( tr( "Duplicate %1: %2\n" ).arg( NodeName ).arg( NodeID ) );
	}

    Sections->insert( NodeID, Node );
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
	nodeMap.clear(); // Delete it's contents
}

void WPDefManager::unload( void )
{
	// Clear all nodes inside our local tree
	clearNodes( this->Items );
	clearNodes( this->Scripts );
	clearNodes( this->NPCs );
	clearNodes( this->StringLists );
	clearNodes( this->Menus );
	clearNodes( this->Spells );
	clearNodes( this->PrivLevels );
	clearNodes( this->SpawnRegions );
	clearNodes( this->Regions );
	clearNodes( this->Multis );
	clearNodes( this->Texts );
	clearNodes( this->StartItems );
	clearNodes( this->Locations );
	clearNodes( this->Skills );
	clearNodes( this->Actions );
	clearNodes( this->MakeSections );
	clearNodes( this->MakeItems );
	clearNodes( this->UseItems );
	clearNodes( this->SkillChecks );
	clearNodes( this->Defines );
	clearNodes( this->Resources );
 	clearNodes( this->ContextMenus );

}

void WPDefManager::unload( WPDEF_TYPE t )
{
	DefSections *ListPointer;

	switch( t )
	{
	case WPDT_ITEM:			ListPointer = &Items;			break;
	case WPDT_SCRIPT:		ListPointer = &Scripts;			break;
	case WPDT_NPC:			ListPointer = &NPCs;			break;
	case WPDT_LIST:			ListPointer = &StringLists;		break;
	case WPDT_MENU:			ListPointer = &Menus;			break;
	case WPDT_SPELL:		ListPointer = &Spells;			break;
	case WPDT_PRIVLEVEL:	ListPointer = &PrivLevels;		break;
	case WPDT_SPAWNREGION:	ListPointer = &SpawnRegions;	break;
	case WPDT_REGION:		ListPointer = &Regions;			break;
	case WPDT_MULTI:		ListPointer = &Multis;			break;
	case WPDT_TEXT:			ListPointer = &Texts;			break;
	case WPDT_STARTITEMS:	ListPointer = &StartItems;		break;
	case WPDT_LOCATION:		ListPointer = &Locations;		break;
	case WPDT_SKILL:		ListPointer = &Skills;			break;
	case WPDT_ACTION:		ListPointer = &Actions;			break;
	case WPDT_MAKESECTION:	ListPointer = &MakeSections;	break;
	case WPDT_MAKEITEM:		ListPointer = &MakeItems;		break;
	case WPDT_USEITEM:		ListPointer = &UseItems;		break;
	case WPDT_SKILLCHECK:	ListPointer = &SkillChecks;		break;
	case WPDT_DEFINE:		ListPointer = &Defines;			break;
	case WPDT_RESOURCE:		ListPointer = &Resources;		break;
	case WPDT_CONTEXTMENU:	ListPointer = &ContextMenus;	break;
	default:				return ;
	};

	clearNodes( *ListPointer );
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
const QDomElement *WPDefManager::getSection( WPDEF_TYPE Type, const QString& Section ) const
{
	const DefSections *ListPointer;

	switch( Type )
	{
	case WPDT_ITEM:			ListPointer = &Items;			break;
	case WPDT_SCRIPT:		ListPointer = &Scripts;			break;
	case WPDT_NPC:			ListPointer = &NPCs;			break;
	case WPDT_LIST:			ListPointer = &StringLists;		break;
	case WPDT_MENU:			ListPointer = &Menus;			break;
	case WPDT_SPELL:		ListPointer = &Spells;			break;
	case WPDT_PRIVLEVEL:	ListPointer = &PrivLevels;		break;
	case WPDT_SPAWNREGION:	ListPointer = &SpawnRegions;	break;
	case WPDT_REGION:		ListPointer = &Regions;			break;
	case WPDT_MULTI:		ListPointer = &Multis;			break;
	case WPDT_TEXT:			ListPointer = &Texts;			break;
	case WPDT_STARTITEMS:	ListPointer = &StartItems;		break;
	case WPDT_LOCATION:		ListPointer = &Locations;		break;
	case WPDT_SKILL:		ListPointer = &Skills;			break;
	case WPDT_ACTION:		ListPointer = &Actions;			break;
	case WPDT_MAKESECTION:	ListPointer = &MakeSections;	break;
	case WPDT_MAKEITEM:		ListPointer = &MakeItems;		break;
	case WPDT_USEITEM:		ListPointer = &UseItems;		break;
	case WPDT_SKILLCHECK:	ListPointer = &SkillChecks;		break;
	case WPDT_DEFINE:		ListPointer = &Defines;			break;
	case WPDT_RESOURCE:		ListPointer = &Resources;		break;
	case WPDT_CONTEXTMENU:	ListPointer = &ContextMenus;	break;
	default:				return 0;
	};

	return &( ListPointer->find( Section ).data() );
}

// Returns a list of section-names found
QStringList WPDefManager::getSections( WPDEF_TYPE Type ) const
{
	const DefSections *ListPointer;
	switch( Type )
	{
	case WPDT_ITEM:			ListPointer = &Items;			break;
	case WPDT_SCRIPT:		ListPointer = &Scripts;			break;
	case WPDT_NPC:			ListPointer = &NPCs;			break;
	case WPDT_MENU:			ListPointer = &Menus;			break;
	case WPDT_SPELL:		ListPointer = &Spells;			break;
	case WPDT_LIST:			ListPointer = &StringLists;		break;
	case WPDT_PRIVLEVEL:	ListPointer = &PrivLevels;		break;
	case WPDT_SPAWNREGION:	ListPointer = &SpawnRegions;	break;
	case WPDT_REGION:		ListPointer = &Regions;			break;
	case WPDT_MULTI:		ListPointer = &Multis;			break;
	case WPDT_TEXT:			ListPointer = &Texts;			break;
	case WPDT_STARTITEMS:	ListPointer = &StartItems;		break;
	case WPDT_LOCATION:		ListPointer = &Locations;		break;
	case WPDT_SKILL:		ListPointer = &Skills;			break;
	case WPDT_ACTION:		ListPointer = &Actions;			break;
	case WPDT_MAKESECTION:	ListPointer = &MakeSections;	break;
	case WPDT_MAKEITEM:		ListPointer = &MakeItems;		break;
	case WPDT_USEITEM:		ListPointer = &UseItems;		break;
	case WPDT_SKILLCHECK:	ListPointer = &SkillChecks;		break;
	case WPDT_DEFINE:		ListPointer = &Defines;			break;
	case WPDT_RESOURCE:		ListPointer = &Resources;		break;
	case WPDT_CONTEXTMENU:	ListPointer = &ContextMenus;	break;
	default:		// Return an empty list
		return QStringList();
	};

	DefSections::const_iterator MyIter;
	QStringList SectionList;
	for( MyIter = ListPointer->begin(); MyIter != ListPointer->end(); ++MyIter )
	{
		SectionList.push_back( MyIter.key() );
	}

	return SectionList;
}

QString	WPDefManager::getRandomListEntry( const QString& ListSection ) const
{
	QStringList list = this->getList( ListSection );
	if( list.isEmpty() )
		return QString();
	else
		return list[ RandomNum( 0, list.size()-1 ) ];
}

QStringList	WPDefManager::getList( const QString& ListSection ) const
{
	const QDomElement* DefSection = this->getSection( WPDT_LIST, ListSection );
	QStringList list;
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
					++i;
				}
			}
			childNode = childNode.nextSibling();
		}
	}
	return list;
}

QString WPDefManager::getText( const QString& TextSection ) const
{
	const QDomElement* DefSection = this->getSection( WPDT_TEXT, TextSection );
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

