//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#include "pythonscript.h"
#include "globals.h"
#include "maps.h"
#include "network.h"
#include "console.h"

// Library Includes
#include <qfile.h>
#include <qglobal.h>

// Extension includes
#include "python/utilities.h"
#include "python/target.h"

// Keep this in Synch with the Enum on the header file
static char *eventNames[] =
{
	"onUse",
	"onSingleClick",
	"onCollide",
	"onWalk",
	"onCreate",
	"onTalk",
	"onWarModeToggle",
	"onLogin",
	"onLogout",
	"onHelp",
	"onChat",
	"onSkillUse",
	"onSkillGain",
	"onStatGain",
	"onShowPaperdoll",
	"onShowSkillGump",
	"onDeath",
	"onShowPaperdollName",
	"onContextEntry",
	"onShowContextMenu",
	"onShowTooltip",
	"onCHLevelChange",
	"onSpeech",
	"onWearItem",
	"onEquip",
	"onUnequip",
	"onDropOnChar",
	"onDropOnItem",
	"onDropOnGround",
	"onPickup",
	"onCommand",
	"onBookUpdateInfo",
	"onBookRequestPage",
	"onBookUpdatePage",
	"onDamage",
	"onCastSpell",
	"onTrade",
	"onTradeStart",
	"onBulletinBoard",
	"onDelete",
	"onSwing",
	"onShowStatus",
	"onChangeRegion",
	"onAttach",
	"onDetach",
	0
};

cPythonScript::cPythonScript() : loaded(false)
{
	codeModule = 0;
	for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
		events[i] = 0;
	}
}

cPythonScript::~cPythonScript()
{
	if ( loaded )
		unload();
}

void cPythonScript::unload( void )
{
	loaded = false;
	
	// Free Cached Events
	for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
		if (events[i]) {
			Py_XDECREF(events[i]);
			events[i] = 0;
		}
	}

	callEventHandler("onUnload");
	
	Py_XDECREF(codeModule);
	codeModule = 0;
}

// Find our module name
bool cPythonScript::load(const QString &name)
{
	if (name.isEmpty()) {
		return false;
	}

	setName(name);

	codeModule = PyImport_ImportModule(const_cast<char*>(name.latin1()));

	if(!codeModule) {
		reportPythonError(name);
		return false;
	}

	// Call the onLoad event
	callEventHandler("onLoad", 0, true);

	if (PyErr_Occurred()) {
		reportPythonError(name_);		
	}

	// Cache Event Functions
	for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
		if (PyObject_HasAttrString(codeModule, eventNames[i])) {
			events[i] = PyObject_GetAttrString(codeModule, eventNames[i]);

			if (events[i] && !PyCallable_Check(events[i])) {
				Console::instance()->log( LOG_ERROR, QString( "Script %1 has non callable event: %1" ).arg( eventNames[ i ] ) );

				Py_DECREF(events[i]);
				events[i] = 0;
			}
		}
	}

	loaded = true;
	return true;
}

bool cPythonScript::isLoaded() const
{
	return loaded;
}

PyObject* cPythonScript::callEvent( ePythonEvent event, PyObject *args, bool ignoreErrors )
{
	PyObject *result = 0;

	if( event < EVENT_COUNT && events[ event ] )
	{
		result = PyObject_CallObject( events[ event ], args );

		if( !ignoreErrors )
			reportPythonError( name_ );
	}

	return result;
}

PyObject* cPythonScript::callEvent( const QString &name, PyObject *args, bool ignoreErrors )
{
	PyObject *result = 0;

	if( codeModule && !name.isEmpty() && PyObject_HasAttrString( codeModule, const_cast< char* >( name.latin1() ) ) )
	{
		PyObject *event = PyObject_GetAttrString( codeModule, const_cast< char* >( name.latin1() ) );

		if( event && PyCallable_Check( event ) )
		{
			result = PyObject_CallObject( event, args );

			if( !ignoreErrors )
				reportPythonError( name_ );
		}
	}

	return result;
}

bool cPythonScript::callEventHandler( ePythonEvent event, PyObject *args, bool ignoreErrors )
{
	PyObject *result = callEvent( event, args, ignoreErrors );
	bool handled = false;

	if( result )
	{
		handled = PyObject_IsTrue( result ) == 0 ? false : true;
		Py_DECREF( result );					
	}

	return handled;
}

bool cPythonScript::callEventHandler( const QString &name, PyObject* args, bool ignoreErrors )
{
	PyObject* result = callEvent( name, args, ignoreErrors );
	bool handled = false;

	if( result )
	{
		handled = PyObject_IsTrue( result ) == 0 ? false : true;
		Py_DECREF( result );					
	}

	return handled;
}

// Standard Handler for Python ScriptChains assigned to objects
bool cPythonScript::callChainedEventHandler( ePythonEvent event, cPythonScript** chain, PyObject* args )
{
	bool handled = false;

	if( chain )
	{
		// Measure
		unsigned int count = reinterpret_cast< unsigned int >( chain[0] );
		cPythonScript **copy = new cPythonScript*[ count ];
		for( unsigned int j = 0; j < count; ++j )
			copy[j] = chain[j+1];

		// Find a valid handler function
		for( unsigned int i = 0; i < count; ++i )
		{
			PyObject *result = copy[i]->callEvent(event, args);
			
			if( result )
			{
				if( PyObject_IsTrue( result ) )
				{
					handled = true;
					Py_DECREF( result );
					break;
				}

				Py_DECREF( result );
			}
		}

		delete [] copy;
	}

	return handled;
}

PyObject *cPythonScript::callChainedEvent( ePythonEvent event, cPythonScript **chain, PyObject *args )
{
	PyObject *result = 0;

	if( chain )
	{
		// Measure
		unsigned int count = reinterpret_cast< unsigned int >( chain[0] );
		cPythonScript **copy = new cPythonScript*[ count ];
		for( unsigned int j = 0; j < count; ++j )
			copy[j] = chain[j+1];

		// Find a valid handler function
		for( unsigned int i = 0; i < count; ++i )
		{
			result = copy[i]->callEvent( event, args );
			
			if( result )
				break;
		}

		delete [] copy;
	}

	return result;
}

bool cPythonScript::canChainHandleEvent( ePythonEvent event, cPythonScript **chain )
{
	if (!chain) {
		return false;
	}

	bool result = false;

	if( event < EVENT_COUNT )
	{
		unsigned int count = reinterpret_cast< unsigned int >( *(chain++) );
		
		for( unsigned int i = 0; i < count; ++i )
		{
			if( chain[i]->canHandleEvent( event ) )
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

stError *cPythonScriptable::setProperty(const QString &name, const cVariant &value) {
	// No settable properties are available for this class
    PROPERTY_ERROR(-1, QString( "Property not found: '%1'" ).arg(name))
}

stError *cPythonScriptable::getProperty(const QString &name, cVariant &value) const {
	GET_PROPERTY("classname", className());
	PROPERTY_ERROR(-1, QString( "Property not found: '%1'" ).arg(name))
}

bool cPythonScriptable::implements(const QString &name) const {
	if (name == cPythonScriptable::className()) {
		return true;
	} else {
		return false;
	}
}

const char *cPythonScriptable::className() const {
	return "pythonscriptable";
}
