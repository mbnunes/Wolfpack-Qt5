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

#include "wolfpack.h"
#include "wpdefmanager.h"
#include <qdom.h>
#include <qfile.h>

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
		if( !Node.attributes().contains( QString( "file" ) ) )
			return;

		// Get the filename and import it
		ImportSections( Node.attribute("file") );
		return;
	}

	// Get the Node ID
	if( !Node.attributes().contains( QString( "id" ) ) )
		return;

	QString NodeID = Node.attribute("id");

	// IF's for all kind of nodes
	// <item id="xx">
	// <script id="xx">
	// <npc id="xx">
	if( NodeName == "item" )
		Items[ NodeID ] = Node;
	else if( NodeName == "script" )
		Scripts[ NodeID ] = Node;
	else if( NodeName == "npc" )
		NPCs[ NodeID ] = Node;
	else if( NodeName == "menu" )
		Menus[ NodeID ] = Node;
	else if( NodeName == "resource" )
		Resources[ NodeID ] = Node;
	else if( NodeName == "itemlist" )
		ItemLists[ NodeID ] = Node;
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
		clConsole.send( FileName.ascii() );
		clConsole.send( "!\n" );
		return false;
	}

	QString errorMessage;
	int errorLine, errorColumn;
	
	if( !Document.setContent( &File, &errorMessage, &errorLine, &errorColumn ) ) 
	{
        File.close();
        
		clConsole.ProgressFail();
		clConsole.send( QString("Unable to parse file %1\nError:%2(%3:%4)\n").arg(FileName).arg(errorMessage).arg(errorLine).arg(errorColumn).latin1() );

		return false;
	}

    	File.close();

	// Get the first document node and start to process it
	QDomElement Definitions = Document.documentElement();
	QDomNodeList NodeList = Definitions.childNodes();

	// Process all nodes
	for( UI32 i = 0; i < NodeList.count(); i++ )
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
	clearNodes( ItemLists );
	clearNodes( Menus );
	clearNodes( Resources );
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
	clConsole.PrepareProgress( "Loading Definitions" );

	if( ImportSections( "definitions.xml" )	)
		clConsole.ProgressDone();
	else
		clConsole.ProgressFail();
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

	case WPDT_ITEMLIST:
		ListPointer = &ItemLists;
		break;

	case WPDT_MENU:
		ListPointer = &Menus;
		break;

	case WPDT_RESOURCE:
		ListPointer = &Resources;
		break;
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

	case WPDT_RESOURCE:
		ListPointer = &Resources;
		break;

	case WPDT_ITEMLIST:
		ListPointer = &ItemLists;
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

const QString processNode( QDomElement &Node )
{
	if( Node.nodeName() == "namelist" )
	{
		// Get the namelist and select a random name!
		// ...
		QString selectedName = Node.text();
		return selectedName;
	}
	else if( Node.nodeName() == "random" )
	{
		if( Node.attributes().contains("min") && Node.attributes().contains("max") )
			return QString("%1").arg( RandomNum( Node.attributeNode("min").nodeValue().toInt(), Node.attributeNode("max").nodeValue().toInt() ) );
		else if( Node.attributes().contains("list") )
		{
			QStringList RandValues = QStringList::split(",", Node.attributeNode("list").nodeValue());
			return RandValues[ RandomNum(0,RandValues.size()-1) ];
		}
		else if( Node.attributes().contains("dice") )
			return QString("%1").arg(rollDice(Node.attributeNode("dice").nodeValue()));
		else
			return QString("0");
	}
	else if( Node.nodeName() == "colorlist" )
	{
		QString Value = QString();
		if( Node.hasChildNodes() ) //random i.e.
			for( int i = 0; i < Node.childNodes().count(); i++ )
			{
				if( Node.childNodes().item( i ).isText() )
					Value += Node.childNodes().item( i ).toText().data();
				else if( Node.childNodes().item( i ).isElement() )
					Value += processNode( Node.childNodes().item( i ).toElement() );
			}
		else
			Value = QString("%1").arg(addrandomcolor( NULL, (char*)Node.nodeValue().latin1() ));

		return Value;
	}

	if( !Node.hasChildNodes() )
		return Node.text();

	// Process the childnodes
	QDomNodeList childNodes = Node.childNodes();

	for( int i = 0; i < childNodes.count(); i++ )
	{
		if( !childNodes.item( i ).isElement() )
			continue;

		return processNode( Node );
	}

	return "";
}