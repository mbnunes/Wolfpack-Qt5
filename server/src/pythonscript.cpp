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

#include <qglobal.h>

#if defined (Q_OS_UNIX)
#include <limits.h>  //compatability issue. GCC 2.96 doesn't have limits include
#else
#include <limits> // Python tries to redefine some of this stuff, so include first
#endif

#include "pythonscript.h"
#include "globals.h"
#include "maps.h"
#include "network.h"
#include "console.h"

// Library Includes
#include <qfile.h>

// Extension includes
#include "python/utilities.h"
#include "python/target.h"

cPythonScript::cPythonScript()
{
	codeModule = 0;
	for( unsigned int i = 0; i < EVENT_COUNT; ++i )
		events[i] = 0;
}

cPythonScript::~cPythonScript()
{
}

void cPythonScript::unload( void )
{
	// Free Cached Events
	for( unsigned int i = 0; i < EVENT_COUNT; ++i )
	{
		if( events[ i ] )
		{
			Py_XDECREF( events[ i ] );
			events[ i ] = 0;
		}
	}

	callEventHandler( "onUnload" );	

	Py_XDECREF( codeModule );
	codeModule = 0;
}

// Find our module name
bool cPythonScript::load( const cElement *element )
{
	QString name = element->text();

	if( name.isEmpty() )
		return false;

	setName( name );

	codeModule = PyImport_ImportModule( const_cast< char* >( name.latin1() ) );

	if( !codeModule )
	{
		Console::instance()->ProgressFail();
		reportPythonError( name );
		Console::instance()->PrepareProgress( "Continuing loading" );
		return false;
	}

	// Call the onLoad event
	callEventHandler( "onLoad", 0, true );

	if( PyErr_Occurred() )
	{
		Console::instance()->ProgressFail();
		reportPythonError( name_ );
		Console::instance()->PrepareProgress( "Continuing loading" );
	}

	// Cache Event Functions
	for( unsigned int i = 0; i < EVENT_COUNT; ++i )
	{
		if( !PyObject_HasAttrString( codeModule, eventNames[ i ] ) )
			continue;

		events[i] = PyObject_GetAttrString( codeModule, eventNames[ i ] );

		if( events[i] && !PyCallable_Check( events[i] ) )
		{
			Console::instance()->ProgressFail();
			Console::instance()->log( LOG_ERROR, QString( "Script %1 has non callable event: %1" ).arg( eventNames[ i ] ) );
			Console::instance()->PrepareProgress( "Continuing loading" );

			Py_DECREF( events[i] );
			events[i] = 0;
		}
	}

	return true;
}

PyObject *cPythonScript::callEvent( ePythonEvent event, PyObject *args, bool ignoreErrors )
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

PyObject *cPythonScript::callEvent( const QString &name, PyObject *args, bool ignoreErrors )
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

bool cPythonScript::callEventHandler( const QString &name, PyObject *args, bool ignoreErrors )
{
	PyObject *result = callEvent( name, args, ignoreErrors );
	bool handled = false;

	if( result )
	{
		handled = PyObject_IsTrue( result ) == 0 ? false : true;
		Py_DECREF( result );					
	}

	return handled;
}


// Standard Handler for Python ScriptChains assigned to objects
bool cPythonScript::callChainedEventHandler( ePythonEvent event, cPythonScript **chain, PyObject *args )
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
			PyObject *result = copy[i]->callEvent( event, args );
			
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
