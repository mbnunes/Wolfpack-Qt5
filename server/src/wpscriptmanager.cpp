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
