// WPDefManager.cpp: Implementierung der Klasse WPDefManager.
//
//////////////////////////////////////////////////////////////////////

#include "wolfpack.h"
#include "wpdefmanager.h"
#include <qdom.h>
#include <qfile.h>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

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
bool WPDefManager::ImportSections( QString FileName )
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

    if ( !Document.setContent( &File ) ) {
        File.close();
        
		clConsole.ProgressFail();

		clConsole.send( "Unable to parse " );
		clConsole.send( FileName.ascii() );
		clConsole.send( "!\n" );

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
			//QDomNode *myNode = &NodeList.item( i ).cloneNode();
			ProcessNode( NodeList.item( i ).cloneNode().toElement() );
		}
	}

	return true;
}

// Load the Definitions
void WPDefManager::Load( void )
{
	clConsole.PrepareProgress( "Loading Definitions" );

	if( ImportSections( QString( "definitions.xml" ) ) )	
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

	//return &(*ListPointer)[ Section ];
	return &( ListPointer->find( Section ).data() );
	//return ListPointer->find( Section )->second;
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
