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

#include "utilities.h"
#include "../globals.h"
#include "../network/uosocket.h"
#include "../network/uotxpackets.h"
#include "../junk.h"
#include "../wpconsole.h"
#include "../TmpEff.h"
#include "../mapobjects.h"

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
	if( PyTuple_Size( args ) < 1 || !PyString_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for wolfpack.additem is 1\n" );
		return Py_None;
	}


	P_ITEM pItem = Items->createScriptItem( PyString_AsString( PyTuple_GetItem( args, 0 ) ) );
	return PyGetItemObject( pItem );
}

/*!
	Adds a npc
*/
PyObject* wpAddnpc( PyObject* self, PyObject* args )
{
	return PyTrue;
}

/*!
	Creates an item object based on the 
	passed serial
*/
PyObject* wpFinditem( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for wolfpack.finditem is 1\n" );
		return Py_None;
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
		clConsole.send( "Minimum argument count for wolfpack.findchar is 1\n" );
		return Py_None;
	}

	SERIAL serial = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	return PyGetCharObject( FindCharBySerial( serial ) );
}

/*!
	Adds a tempeffect with the following arguments:
	wolfpack.addtimer( "callback.function", Timeout in Secs (could be float..), ( subtuple with args ) );
*/
PyObject* wpAddtimer( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 3 )
	{
		clConsole.send( "Minimum argument count for wolfpack.addtimer is 2\n" );
		return PyFalse;
	}

	if( !PyString_Check( PyTuple_GetItem( args, 0 ) ) || 
		( !PyInt_Check( PyTuple_GetItem( args, 1 ) ) && 
		PyFloat_Check( PyTuple_GetItem( args, 1 ) ) ) ||
		!PyTuple_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		clConsole.send( "Wrong arguments passed to addTimer, needed: string, int/float, tuple\n");
		return PyFalse;
	}

	QString funcName = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	UINT32 expireTime;

	if( PyFloat_Check( PyTuple_GetItem( args, 1 ) ) )
		expireTime = PyFloat_AsDouble( PyTuple_GetItem( args, 1 ) ) * 1000;
	else
		expireTime = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );

	PyObject *sArgs = PyTuple_GetItem( args, 2 );
	Py_INCREF( sArgs );

	cPythonEffect *tmpEff = new cPythonEffect;
	tmpEff->setFunctionName( funcName );
	tmpEff->setArgs( sArgs );
	tmpEff->setExpiretime_ms( expireTime );

	cTempEffects::getInstance()->insert( tmpEff );

	return PyTrue;
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
	{ "currenttime",	wpCurrenttime,	METH_VARARGS, "Time in ms since server-start" },
    { NULL, NULL, 0, NULL } // Terminator
};

/*!
	This initializes the wolfpack.console extension
*/
void init_wolfpack_globals()
{
	PyObject *wpNamespace = Py_InitModule( "wolfpack", wpGlobal );
	
	for( UINT8 i = 0; i < ALLSKILLS; ++i )
		PyModule_AddIntConstant( wpNamespace, skillname[ i ], i );

	PyObject *mConsole = Py_InitModule( "wolfpack.console", wpConsole );
    PyObject_SetAttrString( wpNamespace, "console", mConsole );

	PyObject *mTime = Py_InitModule( "wolfpack.time", wpTime );
    PyObject_SetAttrString( wpNamespace, "time", mTime );
}

/*
PyObject* PyWPServer_shutdown( PyObject* self, PyObject* args );
PyObject* PyWPServer_save( PyObject* self, PyObject* args );

PyObject* PyWPItems_findbyserial( PyObject* self, PyObject* args );
PyObject* PyWPItems_add( PyObject* self, PyObject* args );

PyObject* PyWPChars_findbyserial( PyObject* self, PyObject* args );

PyObject* PyWPMovement_deny( PyObject* self, PyObject* args );
PyObject* PyWPMovement_accept( PyObject* self, PyObject* args );

PyObject* PyWP_clients( PyObject* self, PyObject* args );

PyObject* PyWPMap_gettile( PyObject* self, PyObject* args );
PyObject* PyWPMap_getheight( PyObject* self, PyObject* args );

static PyMethodDef WPGlobalMethods[] = 
{
    { "console_send",		PyWPConsole_send,		METH_VARARGS, "Prints something to the wolfpack console" },
	{ "console_progress",	PyWPConsole_progress,	METH_VARARGS, "Prints a .....[xxxx] block" },
	{ "console_printDone",	PyWPConsole_printDone,	METH_VARARGS, "Prints a [done] block" },
	{ "console_printFail",	PyWPConsole_printFail,	METH_VARARGS, "Prints a [fail] block" },
	{ "console_printSkip",	PyWPConsole_printSkip,	METH_VARARGS, "Prints a [skip] block" },

	// .map
	{ "map_gettile",		PyWPMap_gettile,		METH_VARARGS, "Get's a maptile" },
	{ "map_getheight",		PyWPMap_getheight,		METH_VARARGS, "Get's the height of the map at the specified point" },

	// .server
	{ "server_shutdown",	PyWPServer_shutdown,	METH_VARARGS, "Shuts the server down" },
	{ "server_save",		PyWPServer_save,		METH_VARARGS, "Saves the worldstate" },
	
	// .items
	{ "items_add",			PyWPItems_add,			METH_VARARGS, "Adds an item by it's ID specified in the definition files" },
	{ "items_findbyserial",	PyWPItems_findbyserial,	METH_VARARGS, "Finds an item by it's serial" },

	// .chars	
	{ "chars_findbyserial",	PyWPChars_findbyserial,	METH_VARARGS, "Finds an char by it's serial" },

	// .movement
	{ "movement_accept",	PyWPMovement_accept,	METH_VARARGS, "Accepts the movement of the character" },
	{ "movement_deny",		PyWPMovement_deny,		METH_VARARGS, "Denies the movement of a character" },

	// .clients
	{ "clients",			PyWP_clients,			METH_VARARGS, "Retrieves the clientS object" },

    { NULL, NULL, 0, NULL }
};


//========================================= WPChar

//========================================= WPClients

typedef struct {
    PyObject_HEAD;
} Py_WPClients;

int Py_WPClientsLength( Py_WPClients *self );
PyObject *Py_WPClientsGet( Py_WPClients *self, int Num );

static PySequenceMethods Py_WPClientsSequence = {
	(inquiry)Py_WPClientsLength,	
	0,		
	0,		
	(intargfunc)Py_WPClientsGet
};

static PyTypeObject Py_WPClientsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPClients",
    sizeof(Py_WPClientsType),
    0,
    Py_WPDealloc,					
    0,								
    0,								
    0,								
    0,								
    0,								
    0,								
    &Py_WPClientsSequence,			
};

//================================= WPContent

typedef struct {
    PyObject_HEAD;
	cItem *Item;
} Py_WPContent;

int Py_WPContentLength( Py_WPContent *self );
PyObject *Py_WPContentGet( Py_WPContent *self, int Num );

static PySequenceMethods Py_WPContentSequence = {
	(inquiry)Py_WPContentLength,
	0,
	0,
	(intargfunc)Py_WPContentGet,
	0,
	0,
	0,
	0,
};

static PyTypeObject Py_WPContentType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPContent",
    sizeof(Py_WPContentType),
    0,
    Py_WPDealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &Py_WPContentSequence,
    0,
    0,
};

//================================= WPEquipment

typedef struct {
    PyObject_HEAD;
	cChar *Char;
} Py_WPEquipment;

int Py_WPEquipmentLength( Py_WPEquipment *self );
PyObject *Py_WPEquipmentGet( Py_WPEquipment *self, int Num );

static PySequenceMethods Py_WPEquipmentSequence = {
	(inquiry)Py_WPEquipmentLength,
	0,
	0,
	(intargfunc)Py_WPEquipmentGet,
	0,
	0,
	0,
	0,
};

static PyTypeObject Py_WPEquipmentType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPEquipment",
    sizeof(Py_WPEquipmentType),
    0,
    Py_WPDealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &Py_WPEquipmentSequence,
    0,
    0,
};

//================== TARGETTING
#include "wptargetrequests.h"
#include "targeting.h"

typedef struct {
    PyObject_HEAD;
	PKGx6C targetInfo;
} Py_WPTarget;

PyObject *Py_WPTargetGetAttr( Py_WPTarget *self, char *name );

static PyTypeObject Py_WPTargetType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPTarget",
    sizeof(Py_WPTargetType),
    0,
    Py_WPDealloc,
    0,
    (getattrfunc)Py_WPTargetGetAttr,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

class cPythonTarget: public cTargetRequest
{
protected:
	PyObject *callback_;
	PyObject *arguments_;

public:
	cPythonTarget( PyObject *callback, PyObject *arguments );
	virtual ~cPythonTarget() {};

	virtual void responsed( UOXSOCKET socket, PKGx6C targetInfo );
	virtual void timedout( UOXSOCKET socket );
};
*/