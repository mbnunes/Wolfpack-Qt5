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

#include "platform.h"

#include "contextmenu.h"
#include "pythonscript.h"
#include "uobject.h"
#include "wpdefmanager.h"
#include "scriptmanager.h"
#include "player.h"

cConMenu::cConMenu( const cElement *Tag )
{
	applyDefinition( Tag );
	recreateEvents();
}

void cConSingleOption::setOption( const cElement *Tag )
{
	this->tag_ = Tag->getAttribute( "tag" ).toUShort();
	this->intlocid_ = Tag->getAttribute( "intlocid" ).toUShort();
	this->msgid_ = Tag->getAttribute( "msgid" ).toUShort();
}

void cConMenuOptions::addOption( const cElement *Tag )
{
	cConSingleOption SingleOption;
	SingleOption.setOption( Tag );
	this->options_.push_back( SingleOption );
}

void cConMenuOptions::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	
	if( TagName == "option" ) 
		addOption ( Tag );
}

void cConMenu::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	cConMenuOptions tOptions;
	
	tOptions.applyDefinition( Tag );
	
	if( TagName == "access" && Tag->hasAttribute( "acl" ) )
	{
		tOptions.processNode( Tag );
		options_.insert( Tag->getAttribute( "acl" ), tOptions );
		tOptions.deleteAll();
	}
	else if( TagName == "events" )
	{
		eventList_ = QStringList::split( ",", Tag->getValue() );
		recreateEvents();
	}

}

void cConMenu::recreateEvents( void )
{
	scriptChain.clear();
	// Walk the eventList and recreate 
	QStringList::const_iterator myIter( eventList_.begin() );
	for( ; myIter != eventList_.end(); ++myIter )
	{
		cPythonScript *myScript = ScriptManager->find( *myIter );

		// Script not found
		if( myScript == NULL )
			continue;

		scriptChain.push_back( myScript );
	}
}

bool cConMenu::onContextEntry( cPlayer *Caller, cUObject *Target, Q_UINT16 Tag ) const
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		if ( scriptChain[ i ]->onContextEntry( Caller, Target, Tag ) )
			return true;
	}

	return false;
}

const cConMenuOptions* cConMenu::getOptionsByAcl( const QString& acl ) const
{
	QMap< QString, cConMenuOptions >::const_iterator it = options_.find( acl );
	if( it != options_.end() )
		return &it.data();
	
	return 0;
}

void cAllConMenus::load( void )
{
	QStringList sections = DefManager->getSections( WPDT_CONTEXTMENU );
	QStringList::const_iterator it = sections.begin();
	while( it != sections.end() )
	{
		const cElement* section = DefManager->getDefinition( WPDT_CONTEXTMENU, (*it) );

		if( section )
		{
			menus_.insert( (*it), cConMenu( section ) );
		}
		++it;
	}
	
}

bool cAllConMenus::menuExists( const QString& bindmenu ) const
{
	QMap< QString, cConMenu >::const_iterator it( menus_.find( bindmenu ) );
	return it != menus_.end();
}

const cConMenuOptions* cAllConMenus::getMenuOptions( const QString& bindmenu, const QString& acl ) const
{
	QMap< QString, cConMenu >::const_iterator it = menus_.find( bindmenu );
	if ( it != menus_.end() ) 	
		return it.data().getOptionsByAcl( acl );
	return 0;
}

const cConMenu* cAllConMenus::getMenu( const QString& bindmenu, const QString& acl ) const
{
	Q_UNUSED(acl);
	QMap< QString, cConMenu >::const_iterator it( menus_.find( bindmenu ) );
	if ( it != menus_.end() )
		return &it.data();

	return 0;
}

void cAllConMenus::reload( void )
{
	menus_.clear();
	load();
}
