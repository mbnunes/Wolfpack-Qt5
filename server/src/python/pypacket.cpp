//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

#include <qstring.h>

#include "utilities.h"
#include "pypacket.h"
#include "../network/uopacket.h"
#include "../network/uosocket.h"

// Object Information
struct wpPacket
{
    PyObject_HEAD;
	cUOPacket *packet;
};

static PyObject *wpPacket_getattr( PyObject *self, char *name );

// Object Destructor
static void wpPacketDestructor( PyObject *obj )
{
	delete ( (wpPacket*)obj )->packet;
	PyObject_Del( obj );
}

// Static Type Information
static PyTypeObject wpPacketType =
{
    PyObject_HEAD_INIT( &PyType_Type )
    0,
    "wppacket",
    sizeof( wpPacketType ),
    0,
	wpPacketDestructor,
	0,
    wpPacket_getattr,
};

// Resize the packet
static PyObject *wpPacket_resize( PyObject *self, PyObject *args )
{
	int size;

	if( !PyArg_ParseTuple( args, "i:wppacket.resize( size )", &size ) )
		return 0;

	( (wpPacket*)self )->packet->resize( (unsigned short)size );

	Py_INCREF( Py_None );
	return Py_None;
}

// Set a byte
static PyObject *wpPacket_setbyte( PyObject *self, PyObject *args )
{
	unsigned int pos, value;

	if( !PyArg_ParseTuple( args, "ii:wppacket.setbyte( position, value )", &pos, &value ) )
		return 0;

	( *( (wpPacket*)self )->packet )[ (unsigned short)pos ] = (char)value;

	Py_INCREF( Py_None );
	return Py_None;
}

// Set a short
static PyObject *wpPacket_setshort( PyObject *self, PyObject *args )
{
	unsigned int pos, value;

	if( !PyArg_ParseTuple( args, "ii:wppacket.setshort( position, value )", &pos, &value ) )
		return 0;

	( (wpPacket*)self )->packet->setShort( (unsigned short)pos, (unsigned short)value );

	Py_INCREF( Py_None );
	return Py_None;
}

// Set an integer
static PyObject *wpPacket_setint( PyObject *self, PyObject *args )
{
	unsigned int pos, value;

	if( !PyArg_ParseTuple( args, "ii:wppacket.setint( position, value )", &pos, &value ) )
		return 0;

	( (wpPacket*)self )->packet->setInt( (unsigned short)pos, (unsigned int)value );

	Py_INCREF( Py_None );
	return Py_None;
}

// Set raw data in the packet buffer
static PyObject *wpPacket_setascii( PyObject *self, PyObject *args )
{
	int pos, size;
	char *buffer;
		
	if( !PyArg_ParseTuple( args, "ies#:wppacket.setbuffer( position, value )", &pos, "ascii", &buffer, &size ) )
		return 0;

	( (wpPacket*)self )->packet->setAsciiString( (unsigned short)pos, buffer, size );

	PyMem_Free( buffer );

	Py_INCREF( Py_None );
	return Py_None;
}

// Set data in unicode encoding
static PyObject *wpPacket_setunicode( PyObject *self, PyObject *args )
{
	int pos;
	char *buffer;
		
	if( !PyArg_ParseTuple( args, "ies:wppacket.setunicode( position, value )", &pos, "utf-8", &buffer ) )
		return 0;

	QString string = QString::fromUtf8( buffer );
	
	( (wpPacket*)self )->packet->setUnicodeString( pos, string, string.length() );

	PyMem_Free( buffer );

	Py_INCREF( Py_None );
	return Py_None;
}

// Send the packet
static PyObject *wpPacket_send( PyObject *self, PyObject *args )
{
	cUOSocket *socket;

	if( !PyArg_ParseTuple( args, "O&:wppacket.send( socket )", &PyConvertSocket, &socket ) )
		return 0;

	socket->send( ( (wpPacket*)self )->packet );

	Py_INCREF( Py_None );
	return Py_None;
}

// List of Methods
PyMethodDef wpPacketMethods[] = 
{
	{ "resize",		wpPacket_resize,			METH_VARARGS,	NULL },
	{ "setbyte",	wpPacket_setbyte,			METH_VARARGS,	NULL },
	{ "setshort",	wpPacket_setshort,			METH_VARARGS,	NULL },
	{ "setint",		wpPacket_setint,			METH_VARARGS,	NULL },
	{ "setascii",	wpPacket_setascii,			METH_VARARGS,	NULL },
	{ "setunicode",	wpPacket_setunicode,		METH_VARARGS,	NULL },
	{ "send",		wpPacket_send,				METH_VARARGS,	NULL },
	{ NULL, NULL, 0, NULL }
};

static PyObject* wpPacket_getattr( PyObject* self, char* name )
{
	return Py_FindMethod( wpPacketMethods, self, name );
}

// Object Constructor
PyObject *CreatePyPacket( unsigned char id, unsigned short size )
{
	wpPacket *obj = (wpPacket*)PyObject_New( wpPacket, &wpPacketType );

	obj->packet = new cUOPacket( id, size );

	return (PyObject*)obj;
}
