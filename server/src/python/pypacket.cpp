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

#include <qstring.h>

#include "utilities.h"
#include "pypacket.h"
#include "../network/uopacket.h"
#include "../network/uosocket.h"

/*
	\object packet
	\description This object type represents a packet received from the client or one that should
	be sent to the client.
*/
struct wpPacket
{
	PyObject_HEAD;
	cUOPacket* packet;
	bool borrowed;
};

static PyObject* wpPacket_getattr( PyObject* self, char* name );

// Object Destructor
static void wpPacketDestructor( PyObject* obj )
{
	wpPacket* ppacket = ( wpPacket* ) obj;
	// Delete the packet if the pointer wasn't borrowed
	if ( !ppacket->borrowed )
	{
		delete ppacket->packet;
	}
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

/*
	\method packet.resize
	\param size The new size in bytes of the packet.
	\description This method resizes the packet and either truncates to the new size or
	fills newly added bytes with zeros.
*/
static PyObject* wpPacket_resize( PyObject* self, PyObject* args )
{
	int size;

	if ( !PyArg_ParseTuple( args, "i:wppacket.resize( size )", &size ) )
		return 0;

	( ( wpPacket * ) self )->packet->resize( ( unsigned short ) size );

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.setbyte
	\param offset The byte offset within the packet.
	\param value The integer value that should be set.
	\description This method sets a byte (8 bit) value within the packet.
*/
static PyObject* wpPacket_setbyte( PyObject* self, PyObject* args )
{
	unsigned int pos, value;

	if ( !PyArg_ParseTuple( args, "ii:wppacket.setbyte( position, value )", &pos, &value ) )
		return 0;

	( *( ( wpPacket * ) self )->packet )[( unsigned short ) pos] = ( char ) value;

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.setshort
	\param offset The byte offset within the packet.
	\param value The integer value that should be set.
	\description This method sets a short (16 bit) value within the packet.
*/
static PyObject* wpPacket_setshort( PyObject* self, PyObject* args )
{
	unsigned int pos, value;

	if ( !PyArg_ParseTuple( args, "ii:wppacket.setshort( position, value )", &pos, &value ) )
		return 0;

	( ( wpPacket * ) self )->packet->setShort( ( unsigned short ) pos, ( unsigned short ) value );

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.setint
	\param offset The byte offset within the packet.
	\param value The integer value that should be set.
	\description This method sets a integer (32 bit) value within the packet.
*/
static PyObject* wpPacket_setint( PyObject* self, PyObject* args )
{
	unsigned int pos, value;

	if ( !PyArg_ParseTuple( args, "ii:wppacket.setint( position, value )", &pos, &value ) )
		return 0;

	( ( wpPacket * ) self )->packet->setInt( ( unsigned short ) pos, ( unsigned int ) value );

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.getbyte
	\param offset The byte offset within the packet.
	\return An integer value.
	\description This methods get an 8-bit value from the packet.
*/
static PyObject* wpPacket_getbyte( PyObject* self, PyObject* args )
{
	unsigned int pos;

	if ( !PyArg_ParseTuple( args, "i:wppacket.getbyte( position )", &pos ) )
		return 0;

	return PyInt_FromLong( ( ( unsigned char ) ( ( *( ( wpPacket * ) self )->packet )[( unsigned short ) pos] ) ) );
}

/*
	\method packet.getshort
	\param offset The byte offset within the packet.
	\return An integer value.
	\description This methods get a 16-bit value from the packet.
*/
static PyObject* wpPacket_getshort( PyObject* self, PyObject* args )
{
	unsigned int pos;

	if ( !PyArg_ParseTuple( args, "i:wppacket.getshort( position )", &pos ) )
		return 0;

	return PyInt_FromLong( ( ( wpPacket * ) self )->packet->getShort( ( unsigned short ) pos ) );
}

/*
	\method packet.getint
	\param offset The byte offset within the packet.
	\return An integer value.
	\description This methods get a 32-bit value from the packet.
*/
static PyObject* wpPacket_getint( PyObject* self, PyObject* args )
{
	unsigned int pos;

	if ( !PyArg_ParseTuple( args, "i:wppacket.getint( position )", &pos ) )
		return 0;

	return PyInt_FromLong( ( ( wpPacket * ) self )->packet->getInt( ( unsigned short ) pos ) );
}

/*
	\method packet.setascii
	\param offset The byte offset within the packet.
	\param value The string value.
	\description This method copies an ASCII string into the packet including the null termination byte.
*/
static PyObject* wpPacket_setascii( PyObject* self, PyObject* args )
{
	unsigned short pos;
	char* buffer;

	if ( !PyArg_ParseTuple( args, "Hs:wppacket.setbuffer(position, value)", &pos, &buffer ) )
		return 0;

	cUOPacket* packet = ( ( wpPacket* ) self )->packet;
	unsigned int length = strlen( buffer ) + 1;

	if ( pos + length > packet->size() )
	{
		PyErr_SetString( PyExc_IOError, "String longer than packet size." );
		return 0;
	}
	else
	{
		packet->setAsciiString( pos, buffer, length );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.setunicode
	\param offset The byte offset within the packet.
	\param value The unicode or utf-8 value.
	\description This method copies an unicode string into the packet including the null termination byte.
*/
static PyObject* wpPacket_setunicode( PyObject* self, PyObject* args )
{
	int pos;
	char* buffer;

	if ( !PyArg_ParseTuple( args, "ies:wppacket.setunicode( position, value )", &pos, "utf-8", &buffer ) )
		return 0;

	QString string = QString::fromUtf8( buffer );

	( ( wpPacket * ) self )->packet->setUnicodeString( pos, string, string.length() );

	PyMem_Free( buffer );

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.getunicode
	\param offset The byte offset within the packet.
	\param length The maximum length.
	\description This method gets an unicode string from the packet.
*/
static PyObject* wpPacket_getunicode( PyObject* self, PyObject* args )
{
	int pos;
	int length;

	if ( !PyArg_ParseTuple( args, "ii:wppacket.getunicode(position, length)", &pos, &length ) )
	{
		return 0;
	}

	cUOPacket* packet = ( ( wpPacket* ) self )->packet;
	QString string = packet->getUnicodeString( pos, length );

	return QString2Python( string );
}

/*
	\method packet.getascii
	\param offset The byte offset within the packet.
	\param length The maximum length.
	\description This method gets an ASCII string from the packet.
*/
static PyObject* wpPacket_getascii( PyObject* self, PyObject* args )
{
	int pos;
	int length;

	if ( !PyArg_ParseTuple( args, "ii:wppacket.getascii(position, length)", &pos, &length ) )
	{
		return 0;
	}

	cUOPacket* packet = ( ( wpPacket* ) self )->packet;
	QCString string = packet->getAsciiString( pos, length );

	if ( string.isEmpty() )
	{
		return PyString_FromString( "" );
	}
	else
	{
		return PyString_FromString( string.data() );
	}
}

/*
	\method packet.send
	\param socket A <object id="socket">socket</object> object.
	\description This method sends this packet to a given socket.
*/
static PyObject* wpPacket_send( PyObject* self, PyObject* args )
{
	cUOSocket* socket;

	if ( !PyArg_ParseTuple( args, "O&:wppacket.send( socket )", &PyConvertSocket, &socket ) )
		return 0;

	socket->send( ( ( wpPacket * ) self )->packet );

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method packet.dump
	\return A string.
	\description This method creates a dump of the packet and returns it as a human readable string.
*/
static PyObject* wpPacket_dump( PyObject* self, PyObject* args )
{
	Q_UNUSED( args );
	QCString dump = cUOPacket::dump( ( ( wpPacket* ) self )->packet->uncompressed() );

	return PyString_FromString( dump.data() );
}

// List of Methods
PyMethodDef wpPacketMethods[] =
{
	{"resize",		wpPacket_resize,		METH_VARARGS,	NULL},
	{"setbyte",		wpPacket_setbyte,		METH_VARARGS,	NULL},
	{"setshort",	wpPacket_setshort,		METH_VARARGS,	NULL},
	{"setint",		wpPacket_setint,		METH_VARARGS,	NULL},
	{"getascii",	wpPacket_getascii,		METH_VARARGS,   NULL},
	{"getunicode",	wpPacket_getunicode,	METH_VARARGS,   NULL},
	{"setascii",	wpPacket_setascii,		METH_VARARGS,	NULL},
	{"setunicode",	wpPacket_setunicode,	METH_VARARGS,	NULL},
	{"send",		wpPacket_send,			METH_VARARGS,	NULL},
	{"dump",		wpPacket_dump,			METH_VARARGS,	NULL},
	{"getbyte",		wpPacket_getbyte,		METH_VARARGS,	NULL},
	{"getshort",	wpPacket_getshort,		METH_VARARGS,	NULL},
	{"getint",		wpPacket_getint,		METH_VARARGS,	NULL},
	{NULL, NULL, 0, NULL}
};

static PyObject* wpPacket_getattr( PyObject* self, char* name )
{
	/*
	\rproperty packet.size The current size of this packet.
	*/
	if ( !strcmp( name, "size" ) )
	{
		return PyInt_FromLong( ( ( wpPacket * ) self )->packet->size() );
	}
	else
	{
		return Py_FindMethod( wpPacketMethods, self, name );
	}
}

// Object Constructor
PyObject* CreatePyPacket( unsigned char id, unsigned short size )
{
	wpPacket* obj = ( wpPacket* ) PyObject_New( wpPacket, &wpPacketType );

	obj->packet = new cUOPacket( id, size );
	obj->borrowed = false;

	return ( PyObject * ) obj;
}

PyObject* CreatePyPacket( cUOPacket* packet )
{
	wpPacket* obj = ( wpPacket* ) PyObject_New( wpPacket, &wpPacketType );

	obj->packet = packet;
	obj->borrowed = true;

	return ( PyObject * ) obj;
}
