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

#if defined (__unix__)
#include <limits.h>  //compatability issue. GCC 2.96 doesn't have limits include
#else
#include <limits> // Python tries to redefine some of this stuff, so include first
#endif

#include "utilities.h"
#include "../network/uosocket.h"

/*!
	Struct for WP Python Sockets
*/
typedef struct {
    PyObject_HEAD;
	cUOSocket *pSock;
} wpSocket;

// Forward Declarations
PyObject *wpSocket_getAttr( wpSocket *self, char *name );
int wpSocket_setAttr( wpSocket *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python chars
*/
static PyTypeObject wpSocketType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPSocket",
    sizeof(wpSocketType),
    0,
    wpDealloc,				
    0,								
    (getattrfunc)wpSocket_getAttr,
    (setattrfunc)wpSocket_setAttr,
};

PyObject* PyGetSocketObject( cUOSocket *socket )
{
	if( !socket )
		return Py_None;

	wpSocket *rVal = PyObject_New( wpSocket, &wpSocketType );
	rVal->pSock = socket;

	if( rVal )
		return (PyObject*)rVal;
	else
		return Py_None;
}

/*!
	Disconnects the socket.
*/
PyObject* wpSocket_disconnect( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	self->pSock->socket()->close();
	return PyTrue;
}

/*!
	Sends a system message to the socket
*/
PyObject* wpSocket_sysmessage( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for socket.sysmessage is 1" );
		return PyFalse;
	}

	QString message = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	UINT16 color = 0x37;
	UINT16 font = 3;

	if( PyTuple_Size( args ) > 1 && checkArgInt( 1 ) )
		color = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );

	if( PyTuple_Size( args ) > 2 && checkArgInt( 2 ) )
		font = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );

	self->pSock->sysMessage( message, color, font );

	return PyTrue;
}

/*!
	Sends speech of a given object to the socket
*/
PyObject* wpSocket_sendspeech( wpSocket* self, PyObject* args )
{
	// Needed/Allowed arugments:
	// First Argument: Source
	// Second Argument: Speech
	// optional:
	// Third Argument: Color
	// Fourth Argument: SpeechType
	if( !self->pSock || PyTuple_Size( args ) < 2 || !checkArgStr( 1 ) || !checkArgObject( 2 ) ) 
		return PyFalse;

	cUObject *object = NULL;
	QString speech( PyString_AsString( PyTuple_GetItem( args, 1 ) ) );
	UINT16 color = 0x3b2;
	UINT16 font = 3;
	UINT8 type = 0;
	cUOTxUnicodeSpeech::eSpeechType eType;

	object = getWpItem( PyTuple_GetItem( args, 0 ) );

	if( !object )
		object = getWpChar( PyTuple_GetItem( args, 0 ) );

	if( !object )
		return PyFalse;

	if( PyTuple_Size( args ) > 2 && checkArgInt( 2 ) )
		color = getArgInt( 2 );

	if( PyTuple_Size( args ) > 3 && checkArgInt( 3 ) )
		font = getArgInt( 3 );

	if( PyTuple_Size( args ) > 4 && checkArgInt( 4 ) )
		type = getArgInt( 4 );

	switch( type )
	{
	case 1:
		eType = cUOTxUnicodeSpeech::Broadcast;
		break;
	case 2:
		eType = cUOTxUnicodeSpeech::Emote;
		break;
	case 6:
		eType = cUOTxUnicodeSpeech::System;
		break;
	case 8:
		eType = cUOTxUnicodeSpeech::Whisper;
		break;
	case 9:
		eType = cUOTxUnicodeSpeech::Yell;
		break;
	case 0:
	default:
		eType = cUOTxUnicodeSpeech::Regular;
		break;
	};

	self->pSock->showSpeech( object, speech, color, font, eType );

	return PyTrue;
}

static PyMethodDef wpSocketMethods[] = 
{
    { "sysmessage",			(getattrofunc)wpSocket_sysmessage, METH_VARARGS, "Sends a system message to the char." },
	{ "sendspeech",			(getattrofunc)wpSocket_sendspeech, METH_VARARGS, "Sends raw speech to the socket." },
	{ "disconnect",			(getattrofunc)wpSocket_disconnect, METH_VARARGS, "Disconnects the socket." },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpSocket_getAttr( wpSocket *self, char *name )
{
	if( !strcmp( name, "player" ) )
		return PyGetCharObject( self->pSock->player() );
	else
		return Py_FindMethod( wpSocketMethods, (PyObject*)self, name );
}

int wpSocket_setAttr( wpSocket *self, char *name, PyObject *value )
{
	return 0;
}

