/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#include "platform.h"

#include "contextmenu.h"
#include "pythonscript.h"
#include "uobject.h"
#include "definitions.h"
#include "scriptmanager.h"
#include "player.h"

/*!
	Builds a context menu out of the definition scripts
*/
void cContextMenu::processNode( const cElement* Tag )
{
	QString TagName = Tag->name();

	if ( TagName == "scripts" )
	{
		scripts_ = Tag->value();
	}
	else if ( TagName == "option" )
	{
		bool ok;
		ushort msgid = Tag->getAttribute( "msgid" ).toUShort();
		ushort tag = Tag->getAttribute( "tag" ).toUShort();
		ushort color = Tag->getAttribute( "color" ).toUShort( &ok );
		if ( !ok )
			color = 0x7FE0; // Default
		bool checkenabled = Tag->getAttribute( "checkenabled", "false" ).lower() == "true";
		bool checkvisible = Tag->getAttribute( "checkvisible", "false" ).lower() == "true";
		entries_.push_back( new cContextMenuEntry( msgid, tag, color, checkvisible, checkenabled ) );
	}
}

/*!
	Deletes all menu entries, should only be called from the Context Menu manager
*/
void cContextMenu::disposeEntries()
{
	const_iterator it( entries_.begin() );
	for ( ; it != entries_.end(); ++it )
		delete * it;
}

/*!
	Handles response from client invoking all the scripts binded to this menu's
	given \a entry.
*/
void cContextMenu::onContextEntry( cPlayer* from, cUObject* target, ushort entry )
{
	if ( scriptChain_.isEmpty() || entries_.size() <= entry )
		return;

	PyObject* args = Py_BuildValue( "O&O&h", PyGetCharObject, from, PyGetObjectObject, target, entries_[entry]->scriptTag() );
	for ( cPythonScript*script = scriptChain_.first(); script; script = scriptChain_.next() )
	{
		script->callEventHandler( EVENT_CONTEXTENTRY, args );
	}
}

/*!
	Calls script handler to check whether the given entry of the menu
	should be sent or not to the client
*/
bool cContextMenu::onCheckVisible( cPlayer* from, cUObject* target, ushort entry )
{
	if ( scriptChain_.isEmpty() || entries_.size() <= entry )
		return true;

	bool returnValue = true;
	PyObject* args = Py_BuildValue( "NNH", from->getPyObject(), target->getPyObject(), entries_[entry]->scriptTag() );
	for ( cPythonScript*script = scriptChain_.first(); script; script = scriptChain_.next() )
	{
		PyObject* obj = script->callEvent( EVENT_CONTEXTCHECKVISIBLE, args );
		if ( obj )
		{
			if ( !PyObject_IsTrue( obj ) )
				returnValue = false;
			Py_DECREF( obj );
		}

		if ( !returnValue )
			return false;
	}
	return true;
}

/*!
	Calls script handler to check whether the given entry of the menu
	should be flagged as enabled or not to the client
*/
bool cContextMenu::onCheckEnabled( cPlayer* from, cUObject* target, ushort entry )
{
	if ( scriptChain_.isEmpty() || entries_.size() <= entry )
		return true;

	bool returnValue = true;
	PyObject* args = Py_BuildValue( "O&O&h", PyGetCharObject, from, PyGetObjectObject, target, entries_[entry]->scriptTag() );
	for ( cPythonScript*script = scriptChain_.first(); script; script = scriptChain_.next() )
	{
		PyObject* obj = script->callEvent( EVENT_CONTEXTCHECKENABLED, args );
		if ( obj )
		{
			if ( !PyObject_IsTrue( obj ) )
				returnValue = false;
			Py_XDECREF( obj );
		}

		if ( !returnValue )
			return false;
	}
	return true;
}

/*!
	Reloads associated Python scripts
*/
void cContextMenu::recreateEvents()
{
	scriptChain_.clear();
	// Walk the eventList and recreate
	QStringList eventList = QStringList::split( ",", scripts_ );
	QStringList::const_iterator myIter( eventList.begin() );
	for ( ; myIter != eventList.end(); ++myIter )
	{
		cPythonScript* myScript = ScriptManager::instance()->find( ( *myIter ).latin1() );

		// Script not found
		if ( myScript == NULL )
			continue;

		scriptChain_.append( myScript );
	}
}

/*!
	Loads and pre-builds all context menus
*/
void cAllContextMenus::load( void )
{
	QStringList sections = Definitions::instance()->getSections( WPDT_CONTEXTMENU );
	QStringList::const_iterator it = sections.begin();
	for ( ; it != sections.end(); ++it )
	{
		const cElement* section = Definitions::instance()->getDefinition( WPDT_CONTEXTMENU, ( *it ) );

		if ( section )
		{
			cContextMenu* menu = new cContextMenu;
			menu->applyDefinition( section );
			menu->recreateEvents();
			menus_.insert( ( *it ), menu );
		}
	}
	cComponent::load();
}

/*!
	Checks if a given menu exists or not, indexed by name
*/
bool cAllContextMenus::menuExists( const QString& bindmenu ) const
{
	const_iterator it( menus_.find( bindmenu ) );
	return it != menus_.end();
}

/*!
	Retrieves the menu
*/
cContextMenu* cAllContextMenus::getMenu( const QString& bindmenu ) const
{
	const_iterator it( menus_.find( bindmenu ) );
	if ( it != menus_.end() )
	{
		return it.data(); // returns a copy of the menu
	}
	return 0;
}

void cAllContextMenus::unload()
{
	const_iterator it( menus_.begin() );
	for ( ; it != menus_.end(); ++it )
	{
		it.data()->disposeEntries();
		delete it.data();
	}
	cComponent::unload();
}

void cAllContextMenus::reload( void )
{
	unload();
	load();
}
