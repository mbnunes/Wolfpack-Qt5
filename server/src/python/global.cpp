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

#include "../itemsmgr.h"
#include "../globals.h"
#include "../network/uosocket.h"
#include "../network/uotxpackets.h"
#include "../wpconsole.h"
#include "../TmpEff.h"
#include "../mapobjects.h"
#include "../territories.h"

#include "utilities.h"
#include "tempeffect.h"

/*!
	Sends a string to the wolfpack console.
*/
PyObject* wpConsole_send( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == Py_None )
		return PyFalse;

	clConsole.send( PyString_AS_STRING( pyMessage ) );

	return PyTrue;
}

/*!
	Sends a progress-bar to the wolfpack console
*/
PyObject* wpConsole_progress( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == NULL )
		return PyFalse;

	clConsole.PrepareProgress( PyString_AS_STRING( pyMessage ) );

	return PyInt_FromLong( 1 );
}

/*!
	Sends a [done] progress section to the console
*/
PyObject* wpConsole_progressDone( PyObject* self, PyObject* args )
{
	clConsole.ProgressDone();
	return PyInt_FromLong( 1 );
}

/*!
	Sends a [fail] progress section to the console
*/
PyObject* wpConsole_progressFail( PyObject* self, PyObject* args )
{
	clConsole.ProgressFail();
	return PyInt_FromLong( 1 );
}

/*!
	Sends a [skip] progress section to the console
*/
PyObject* wpConsole_progressSkip( PyObject* self, PyObject* args )
{
	clConsole.ProgressSkip();
	return PyInt_FromLong( 1 );
}

/*!
	wolfpack.console
	Initializes wolfpack.console
*/
static PyMethodDef wpConsole[] = 
{
    { "send",			wpConsole_send,			METH_VARARGS, "Prints something to the wolfpack console" },
	{ "progress",		wpConsole_progress,		METH_VARARGS, "Prints a .....[xxxx] block" },
	{ "progressDone",	wpConsole_progressDone,	METH_VARARGS, "Prints a [done] block" },
	{ "progressFail",	wpConsole_progressFail,	METH_VARARGS, "Prints a [fail] block" },
	{ "progressSkip",	wpConsole_progressSkip,	METH_VARARGS, "Prints a [skip] block" },
    { NULL, NULL, 0, NULL } // Terminator
};

/*!
	Gets the seconds of the current uo time
*/
PyObject* wpTime_second( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().second() );
}

/*!
	Gets the minutes of the current uo time
*/
PyObject* wpTime_minute( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().minute() );
}

/*!
	Gets the hours of the current uo time
*/
PyObject* wpTime_hour( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().hour() );
}

/*!
	Gets the current day
*/
PyObject* wpTime_day( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.date().day() );
}

/*!
	Gets the current month
*/
PyObject* wpTime_month( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.date().month() );
}

/*!
	Gets the current year
*/
PyObject* wpTime_year( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.date().year() );
}

/*!
	Gets a timestamp of the current uo time
*/
PyObject* wpTime_timestamp( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().elapsed() );
}

/*!
	Methods for handling UO Time from within python
*/
static PyMethodDef wpTime[] = 
{
	{ "second",		wpTime_second,		METH_VARARGS, "Returns the current time-seconds" },
	{ "minute",		wpTime_minute,		METH_VARARGS, "Returns the current time-minutes" },
	{ "hour",			wpTime_hour,		METH_VARARGS, "Returns the current time-hour" },
	{ "day",			wpTime_day,			METH_VARARGS, "Returns the current date-day" },
	{ "month",			wpTime_month,		METH_VARARGS, "Returns the current date-month" },
	{ "year",			wpTime_year,		METH_VARARGS, "Returns the current date-year" },
	{ "timestamp",		wpTime_timestamp,	METH_VARARGS, "Returns the current timestamp" },
    { NULL, NULL, 0, NULL } // Terminator
};

////////////////////////////////////////////////////////////////////////////
// GLOBALS
// Global methods found in wolfpack.
// Examples: wolfpack.additem() wolfpack.addnpc()
////////////////////////////////////////////////////////////////////////////

/*!
	Adds an item
	Argument is just the def-section
*/
PyObject* wpAdditem( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}


	P_ITEM pItem = Items->createScriptItem( PyString_AsString( PyTuple_GetItem( args, 0 ) ) );
	return PyGetItemObject( pItem );
}

/*!
	Adds a npc
*/
PyObject* wpAddnpc( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 2 || !checkArgStr( 0 ) || !checkWpCoord( PyTuple_GetItem( args, 1 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	Coord_cl pos = getWpCoord( PyTuple_GetItem( args, 1 ) );
	P_CHAR pChar = cCharStuff::createScriptNpc( getArgStr( 0 ), pos );

	return PyGetCharObject( pChar ); 
}

/*!
	Creates an item object based on the 
	passed serial
*/
PyObject* wpFinditem( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	SERIAL serial = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	return PyGetItemObject( FindItemBySerial( serial ) );
}

/*!
	Creates a char object based on the 
	passed serial
*/
PyObject* wpFindchar( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	SERIAL serial = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	return PyGetCharObject( FindCharBySerial( serial ) );
}

/*!
	Adds a tempeffect
*/
PyObject* wpAddtimer( PyObject* self, PyObject* args )
{
	// Three arguments
	if( PyTuple_Size( args ) != 3 || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT32 expiretime = getArgInt( 0 );
	QString function = getArgStr( 1 );
	PyObject *py_args = PyList_AsTuple( PyTuple_GetItem( args, 2 ) );

	cPythonEffect *effect = new cPythonEffect( function, py_args );
	
	// Should we save this effect?
	if( checkArgInt( 3 ) && getArgInt( 3 ) != 0 ) 
		effect->setSerializable( true );
	else
		effect->setSerializable( false );
	
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );

	return PyFalse;
}

/*!
	Tries to find a region for a specific position.
*/
PyObject* wpRegion( PyObject* self, PyObject* args )
{
	// Three arguments
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyGetRegionObject( cAllTerritories::getInstance()->region( getArgInt( 0 ), getArgInt( 1 ) ) );
}

/*!
	Returns the time in ms since the last server-start
	used for object-delays and others
*/
PyObject* wpCurrenttime( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uiCurrentTime );
}

/*!
	wolfpack
	Initializes wolfpack
*/
static PyMethodDef wpGlobal[] = 
{
    { "additem",		wpAdditem,		METH_VARARGS, "Adds an item with the specified script-section" },
	{ "addnpc",			wpAddnpc,		METH_VARARGS, "Adds a npc with the specified script-section" },
	{ "finditem",		wpFinditem,		METH_VARARGS, "Tries to find an item based on it's serial" },
	{ "findchar",		wpFindchar,		METH_VARARGS, "Tries to find a char based on it's serial" },
	{ "addtimer",		wpAddtimer,		METH_VARARGS, "Adds a timed effect" },
	{ "region",			wpRegion,		METH_VARARGS, "Gets the region at a specific position" },
	{ "currenttime",	wpCurrenttime,	METH_VARARGS, "Time in ms since server-start" },
    { NULL, NULL, 0, NULL } // Terminator
};

/*!
	This initializes the wolfpack.console extension
*/
void init_wolfpack_globals()
{
	PyObject *wpNamespace = Py_InitModule( "_wolfpack", wpGlobal );

	PyObject *mConsole = Py_InitModule( "_wolfpack.console", wpConsole );
    PyObject_SetAttrString( wpNamespace, "console", mConsole );

	PyObject *mTime = Py_InitModule( "_wolfpack.time", wpTime );
    PyObject_SetAttrString( wpNamespace, "time", mTime );
}

void tuple_incref( PyObject *object )
{
	Py_INCREF( object );
	if( PyTuple_Check( object ) )
	{
		for( UINT32 i = 0; i < PyTuple_Size( object ); ++i )
			tuple_incref( PyTuple_GetItem( object, i ) );
	}
}

void tuple_decref( PyObject *object )
{	
	if( PyTuple_Check( object ) )
	{
		for( UINT32 i = 0; i < PyTuple_Size( object ); ++i )
			tuple_decref( PyTuple_GetItem( object, i ) );
	}
	Py_DECREF( object );
}
