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
#include "platform.h"
#include "contextmenu.h"

#include "debug.h"
#undef  DBGFILE
#define DBGFILE "contextmenu.cpp"

void cConSingleOption::setOption( const QDomElement &Tag )
{
	this->tag_ = Tag.attribute( "tag" ).toUShort();
	this->intlocid_ = Tag.attribute( "intlocid" ).toUShort();
	this->msgid_ = Tag.attribute( "msgid" ).toUShort();
	this->scriptname_ = Tag.attribute( "script" );
}

void cConMenuOptions::addOption( const QDomElement &Tag )
{
	cConSingleOption SingleOption;
	SingleOption.setOption( Tag );
	this->options_.push_back( SingleOption );
}

void cConMenuOptions::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	
	if( TagName == "option" ) 
		addOption ( Tag );
}

void cConMenu::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	cConMenuOptions tOptions;
	
	tOptions.applyDefinition( Tag );
	
	if( TagName == "access" && Tag.hasAttribute( "acl" ))
	{
		tOptions.processNode( Tag );
		options_.insert( Tag.attribute( "acl" ), tOptions );
		tOptions.deleteAll();
	}
}


cConMenu::cConMenu(const QDomElement &Tag)
{
	applyDefinition( Tag );
}

const cConMenuOptions* cConMenu::getOptionsByAcl( QString acl ) const
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
		const QDomElement* DefSection = DefManager->getSection( WPDT_CONTEXTMENU, (*it) );
		if( !DefSection->isNull() )
		{
			menus_.insert( (*it), cConMenu( *DefSection ) );
		}
		++it;
	}
	
}

bool cAllConMenus::MenuExist( QString bindmenu )
{
	QMap< QString, cConMenu >::iterator it = menus_.find( bindmenu );
	if ( it != menus_.end() ) 
		return true;
	return false;
}
const cConMenuOptions* cAllConMenus::getMenu( QString bindmenu, QString acl ) const
{
	QMap< QString, cConMenu >::const_iterator it = menus_.find( bindmenu );
	if ( it != menus_.end() ) 	
		return it.data().getOptionsByAcl( acl );

	return 0;
}

void cAllConMenus::reload( void )
{
	menus_.clear();
	load();
}
