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

#include "utilities.h"
#include "../network/uosocket.h"
#include "target.h"
#include "gump.h"

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
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "wpsocket",
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
	Q_UNUSED(args);
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
		PyErr_BadArgument();
		return NULL;
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
	Sends a localized message to the socket
*/
PyObject* wpSocket_clilocmessage( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT16 color = 0x37;
	UINT16 font = 3;

	QString params( "" );

	if( checkArgStr( 1 ) )
		params = getArgStr( 1 );

	if( checkArgInt( 2 ) )
		color = getArgInt( 2 );

	if( checkArgInt( 3 ) )
		font = getArgInt( 3 );

	// Object
	cUObject *object = 0;

	if( checkArgChar( 4 ) )
		object = getArgChar( 4 );
	else if( checkArgItem( 4 ) )
		object = getArgItem( 4 );

	self->pSock->clilocMessage( getArgInt( 0 ), params, color, font, object );

	return PyTrue;
}

/*!
	Sends speech of a given object to the socket
*/
PyObject* wpSocket_showspeech( wpSocket* self, PyObject* args )
{
	// Needed/Allowed arugments:
	// First Argument: Source
	// Second Argument: Speech
	// optional:
	// Third Argument: Color
	// Fourth Argument: SpeechType
	if( !self->pSock || !checkArgStr( 1 ) || !checkArgObject( 0 ) ) 
	{
		PyErr_BadArgument();
		return NULL;
	}

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

/*!
	Attachs a target request to the socket.
*/
PyObject* wpSocket_attachtarget( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	// Collect Data
	QString responsefunc = getArgStr( 0 );
	QString cancelfunc, timeoutfunc;
	UINT16 timeout = 0;
	PyObject *targetargs = 0;

	// If Second argument is present, it has to be a tuple
	if( PyTuple_Size( args ) > 1 && PyList_Check( PyTuple_GetItem( args, 1 ) ) )
		targetargs = PyList_AsTuple( PyTuple_GetItem( args, 1 ) );

	if( !targetargs )
		targetargs = PyTuple_New( 0 );

	if( checkArgStr( 2 ) )
		cancelfunc = getArgStr( 2 );

	if( checkArgStr( 3 ) && checkArgInt( 4 ) )
	{
		timeoutfunc = getArgStr( 3 );
		timeout = getArgInt( 4 );
	}

	cPythonTarget *target = new cPythonTarget( responsefunc, timeoutfunc, cancelfunc, targetargs );
	target->setTimeout( uiCurrentTime + timeout );
	self->pSock->attachTarget( target );

	return PyTrue;
}

/*!
	Sends a gump to the socket. This function is used internally only.
*/
PyObject* wpSocket_sendgump( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	// Parameters:
	// x, y, nomove, noclose, nodispose, serial, type, layout, text, callback, args
	if( PyTuple_Size( args ) != 11 )
		return PyFalse;

	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) || !checkArgInt( 3 ) ||
		!checkArgInt( 4 ) || !checkArgInt( 5 ) || !checkArgInt( 6 ) || !PyList_Check( PyTuple_GetItem( args, 7 ) ) ||
		!PyList_Check( PyTuple_GetItem( args, 8 ) ) || !checkArgStr( 9 ) || !PyList_Check( PyTuple_GetItem( args, 10 ) ) )
		return PyFalse;

	INT32 x = getArgInt( 0 );
	INT32 y = getArgInt( 1 );
	bool nomove = getArgInt( 2 );
	bool noclose = getArgInt( 3 );
	bool nodispose = getArgInt( 4 );
	UINT32 serial = getArgInt( 5 );
	UINT32 type = getArgInt( 6 );
	PyObject *layout = PyTuple_GetItem( args, 7 );
	PyObject *texts = PyTuple_GetItem( args, 8 );
	QString callback = getArgStr( 9 );
	PyObject *py_args = PyList_AsTuple( PyTuple_GetItem( args, 10 ) );

	cPythonGump *gump = new cPythonGump( callback, py_args );
	if( serial )
		gump->setSerial( serial );

	if( type )
		gump->setType( type );

	gump->setNoClose( noclose );
	gump->setNoMove( nomove );
	gump->setNoDispose( nodispose );

	INT32 i;
	for( i = 0; i < PyList_Size( layout ); ++i )
	{
		if( PyString_Check( PyList_GetItem( layout, i ) ) )
			gump->addRawLayout( PyString_AsString( PyList_GetItem( layout, i ) ) );
		else
			gump->addRawLayout( "" );
	}

	for( i = 0; i < PyList_Size( texts ); ++i )
	{
		if( PyString_Check( PyList_GetItem( texts, i ) ) )
			gump->addRawText( PyString_AsString( PyList_GetItem( texts, i ) ) );
		else
			gump->addRawText( "" );
	}

	self->pSock->send( gump );

	return PyInt_FromLong( gump->serial() );
}

/*!
	Closes a gump that has been sent to the client using it's
	serial.
*/
PyObject* wpSocket_closegump( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;
	
	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	cUOTxCloseGump closeGump;
	closeGump.setButton( 0 );
	closeGump.setType( getArgInt( 0 ) );
	self->pSock->send( &closeGump );

	return PyTrue;
}

/*!
	Resends the world around this socket.
*/
PyObject* wpSocket_resendworld( wpSocket* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pSock )
		return PyFalse;
	self->pSock->resendWorld( false );
	return PyTrue;
}

/*!
	Resends the player only.
*/
PyObject* wpSocket_resendplayer( wpSocket* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pSock )
		return PyFalse;
	self->pSock->resendPlayer( false );
	return PyTrue;
}

/*!
	Sends a container and it's content to a socket.
*/
PyObject* wpSocket_sendcontainer( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;
	
	if( !checkArgItem( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	if( !getArgItem( 0 ) )
		return PyFalse;

	self->pSock->sendContainer( getArgItem( 0 ) );

	return PyTrue;
}

/*!
	Sends a packet to this socket.
*/
PyObject* wpSocket_sendpacket( wpSocket* self, PyObject* args )
{
	if( PyTuple_Size( args ) != 1 )
	{
		PyErr_BadArgument();
		return 0;
	}

	PyObject *list = PyTuple_GetItem( args, 0 );

	if( !PyList_Check( list ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	// Build a packet
	int packetLength = PyList_Size( list );

	QByteArray buffer( packetLength );

	for( int i = 0; i < packetLength; ++i )
		buffer[i] = PyInt_AsLong( PyList_GetItem( list, i ) );

	cUOPacket packet( buffer );
	self->pSock->send( &packet );

	return PyTrue;
}

/*!
	Returns the custom tag passed
*/
PyObject* wpSocket_gettag( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return Py_None;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	cVariant value = self->pSock->tags().get( key );

	if( value.type() == cVariant::String )
		return PyString_FromString( value.asString().latin1() );
	else if( value.type() == cVariant::Int )
		return PyInt_FromLong( value.asInt() );

	return Py_None;
}

/*!
	Sets a custom tag
*/
PyObject* wpSocket_settag( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) || ( !checkArgStr( 1 ) && !checkArgInt( 1 )  ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );

	self->pSock->tags().remove( key );

	if( checkArgStr( 1 ) )
		self->pSock->tags().set( key, cVariant( getArgStr( 1 ) ) );
	else if( checkArgInt( 1 ) )
		self->pSock->tags().set( key, cVariant( (int)getArgInt( 1 ) ) );

	return PyTrue;
}

/*!
	Checks if a certain tag exists
*/
PyObject* wpSocket_hastag( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	
	return self->pSock->tags().get( key ).isValid() ? PyTrue : PyFalse;
}

/*!
	Deletes a given tag
*/
PyObject* wpSocket_deltag( wpSocket* self, PyObject* args )
{
	if( !self->pSock )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	self->pSock->tags().remove( key );

	return PyTrue;
}

static PyMethodDef wpSocketMethods[] = 
{
    { "sysmessage",			(getattrofunc)wpSocket_sysmessage, METH_VARARGS, "Sends a system message to the char." },
    { "clilocmessage",		(getattrofunc)wpSocket_clilocmessage, METH_VARARGS, "Sends a localized message to the socket." },
	{ "showspeech",			(getattrofunc)wpSocket_showspeech, METH_VARARGS, "Sends raw speech to the socket." },
	{ "disconnect",			(getattrofunc)wpSocket_disconnect, METH_VARARGS, "Disconnects the socket." },
	{ "attachtarget",		(getattrofunc)wpSocket_attachtarget,  METH_VARARGS, "Adds a target request to the socket" },
	{ "sendgump",			(getattrofunc)wpSocket_sendgump,	METH_VARARGS, "INTERNAL! Sends a gump to this socket." },
	{ "closegump",			(getattrofunc)wpSocket_closegump,	METH_VARARGS, "Closes a gump that has been sent to the client." },
	{ "resendworld",		(getattrofunc)wpSocket_resendworld,  METH_VARARGS, "Sends the surrounding world to this socket." },
	{ "resendplayer",		(getattrofunc)wpSocket_resendplayer,  METH_VARARGS, "Resends the player only." },
	{ "sendcontainer",		(getattrofunc)wpSocket_sendcontainer,  METH_VARARGS, "Sends a container to the socket." },
	{ "sendpacket",			(getattrofunc)wpSocket_sendpacket,		METH_VARARGS, "Sends a packet to this socket." },
	{ "gettag",				(getattrofunc)wpSocket_gettag,	METH_VARARGS,	"Gets a tag from a socket." },
	{ "settag",				(getattrofunc)wpSocket_settag,	METH_VARARGS,	"Sets a tag to a socket." },
	{ "hastag",				(getattrofunc)wpSocket_hastag,	METH_VARARGS,	"Checks if a socket has a specific tag." },
	{ "dettag",				(getattrofunc)wpSocket_deltag,	METH_VARARGS,	"Delete specific tag." },
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
	Q_UNUSED(self);
	Q_UNUSED(name);
	Q_UNUSED(value);
	return 0;
}

