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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "utilities.h"
#include "../network/uosocket.h"
#include "../network/uotxpackets.h"
#include "target.h"
#include "gump.h"

/*
	\object socket
	\description This object represents a connected client.
*/
typedef struct
{
	PyObject_HEAD;
	cUOSocket* pSock;
} wpSocket;

// Forward Declarations
static PyObject* wpSocket_getAttr( wpSocket* self, char* name );
static int wpSocket_setAttr( wpSocket* self, char* name, PyObject* value );

/*!
	The typedef for Wolfpack Python chars
*/
static PyTypeObject wpSocketType =
{
PyObject_HEAD_INIT( &PyType_Type )
0,
"wpsocket",
sizeof( wpSocketType ),
0,
wpDealloc,
0,
( getattrfunc ) wpSocket_getAttr,
( setattrfunc ) wpSocket_setAttr,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

};

PyObject* PyGetSocketObject( cUOSocket* socket )
{
	if ( !socket )
	{
		Py_RETURN_NONE;
	}

	wpSocket* rVal = PyObject_New( wpSocket, &wpSocketType );
	rVal->pSock = socket;

	if ( rVal )
		return ( PyObject * ) rVal;

	Py_RETURN_NONE;
}

/*
	\method socket.disconnect
	\description Disconnects this client from the server.
*/
static PyObject* wpSocket_disconnect( wpSocket* self, PyObject* args )
{
	Q_UNUSED( args );
	self->pSock->socket()->close();
	Py_RETURN_NONE;
}

/*
	\method socket.sysmessage
	\description Sends a sysmessage to the client.
	\param message The message text. Unicode or ASCII.
	\param color Defaults to 0x3b2. The message color.
	\param font The message font. Defaults to 3.
*/
static PyObject* wpSocket_sysmessage( wpSocket* self, PyObject* args )
{
	char* message;
	unsigned short color = 0x3b2;
	unsigned short font = 3;

	if ( !PyArg_ParseTuple( args, "es|HH:socket.sysmessage(message, color, font)", "utf-8", &message, &color, &font ) )
	{
		return 0;
	}

	self->pSock->sysMessage( QString::fromUtf8( message ), color, font );
	PyMem_Free( message );

	Py_RETURN_NONE;
}

/*
	\method socket.clilocmessage
	\description Sends a localized message to the client.
	\param message The id of the localized message.
	\param params The parameters that should be parsed into the localized message.
	Defaults to an empty string.
	\param color The color of the localized message. Defaults to 0x3b2.
	\param font The font of the message. Defaults to 3.
	\param source The source of the message. The message will be a system message if
	this is None. Defaults to None.
	\param affix A text that should be appended to the localized message. Defaults to an
	empty string.
	\param dontmove If true, the message will not move with the source. Defaults to false.
	\param prepend If true, the affix will be prepended instead of appended.
*/
static PyObject* wpSocket_clilocmessage( wpSocket* self, PyObject* args )
{
	unsigned int clilocid;
	char* params = 0;
	unsigned short color = 0x3b2;
	unsigned short font = 3;
	PyObject* psource = Py_None;
	char* affix = 0;
	unsigned char dontmove = 0;
	unsigned char prepend = 0;

	if ( !PyArg_ParseTuple( args, "I|esHHOesBB:socket.clilocmessage"
		"(messageid, [params], [color], [font], [source], [affix], [dontmove], [prepend])", &clilocid, "utf-8", &params, &color, &font, &psource, "utf-8", &affix, &dontmove, &prepend ) )
	{
		return 0;
	}

	cUObject* source = 0;

	if ( psource != Py_None )
	{
		PyConvertObject( psource, &source );
	}

	if ( affix != 0 )
	{
		self->pSock->clilocMessageAffix( clilocid, QString::fromUtf8( params ), QString::fromUtf8( affix ), color, font, source, dontmove, prepend );
		PyMem_Free( affix );
	}
	else
	{
		self->pSock->clilocMessage( clilocid, QString::fromUtf8( params ), color, font, source );
	}

	if ( params != 0 )
	{
		PyMem_Free( params );
	}

	Py_RETURN_NONE;
}

/*
	\method socket.showspeech
	\description Sends an advanced text message to this client.
	\param source The source of the message. Should not be None.
	\param message The text of the message. Unicode or ASCII.
	\param color The color of the message. Defaults to 0x3b2.
	\param font The font of the message. Defaults to 3.
	\param type The message type. Defaults to 0.
*/
static PyObject* wpSocket_showspeech( wpSocket* self, PyObject* args )
{
	cUObject* object;
	char* message;
	unsigned short color = 0x3b2;
	unsigned short font = 3;
	unsigned char type = 0;

	if ( !PyArg_ParseTuple( args, "O&es|HHB:socket.showspeech"
		"(source, message, [color], [font], [type])", &PyConvertObject, &object, "utf-8", &message, &color, &font, &type ) )
	{
		return 0;
	}

	self->pSock->showSpeech( object, QString::fromUtf8( message ), color, font, ( cUOTxUnicodeSpeech::eSpeechType ) type );
	PyMem_Free( message );
	Py_RETURN_NONE;
}

/*
	\method socket.attachtarget
	\description Send a target request to this client.
	\param callback The full name (including the module) of a python function
	that will be called when the client selects a target. The function needs
	to have the following prototype (name may differ):
	<code>def callback(player, arguments, target):
	&nbsp;&nbsp;pass</code>
	- <code>player</code> The <object id="CHAR">char</object> object of the player sending the target response.
	- <code>arguments</code> The arguments passed to attachtarget. This is converted to a tuple.
	- <code>target</code> A <object id="TARGET">target</object> object representing the target.

	\param args A list of arguments that are passed to the callback function. This defaults to an empty list.
	\param cancelcallback The full name of a python function that should be called when the target is canceled.
	Defaults to an empty string.
	\param timeoutcallback The full name of a python function that should be called when the target times out.
	\param timeout The timeout of this target in miliseconds. Defaults to zero which means the target doesn't
	time out at all.
*/
static PyObject* wpSocket_attachtarget( wpSocket* self, PyObject* args )
{
	char* responsefunc;
	PyObject* targetargs = 0;
	char* cancelfunc = 0;
	char* timeoutfunc = 0;
	unsigned int timeout = 0;

	if ( !PyArg_ParseTuple( args, "s|O!ssI:socket.attachtarget"
		"(callback, [args], [cancelcallback], [timeoutcallback], [timeout])", &responsefunc, &PyList_Type, &targetargs, &cancelfunc, &timeoutfunc, &timeout ) )
	{
		return 0;
	}

	if ( targetargs )
	{
		targetargs = PyList_AsTuple( targetargs );
	}
	else
	{
		targetargs = PyTuple_New( 0 );
	}

	cPythonTarget* target = new cPythonTarget( responsefunc, timeoutfunc, cancelfunc, targetargs );

	if ( timeout )
	{
		target->setTimeout( Server::instance()->time() + timeout );
	}

	self->pSock->attachTarget( target );
	Py_RETURN_NONE;
}

static PyObject* wpSocket_attachitemtarget( wpSocket* self, PyObject* args )
{
	char* responsefunc;
	PyObject* items;
	PyObject* targetargs = 0;
	char* cancelfunc = 0;
	char* timeoutfunc = 0;
	unsigned int timeout = 0;
	int xoffset,
	yoffset,
	zoffset;

	if ( !PyArg_ParseTuple( args, "sO!iii|O!ssI:socket.attachitemtarget"
		"(callback, [items], [args], [cancelcallback], [timeoutcallback], [timeout])", &responsefunc, &PyList_Type, &items, &xoffset, &yoffset, &zoffset, &PyList_Type, &targetargs, &cancelfunc, &timeoutfunc, &timeout ) )
	{
		return 0;
	}

	if ( targetargs )
	{
		targetargs = PyList_AsTuple( targetargs );
	}
	else
	{
		targetargs = PyTuple_New( 0 );
	}

	std::vector<stTargetItem> targetitems;

	// Evaluate the given items
	for ( int i = 0; i < PyList_Size( items ); ++i )
	{
		PyObject* listitem = PyList_GetItem( items, i );

		// Has to be another list
		if ( PyList_Check( listitem ) )
		{
			// id, xoffset, yoffset, zoffset, hue
			if ( PyList_Size( listitem ) == 5 )
			{
				PyObject* id = PyList_GetItem( listitem, 0 );
				PyObject* ixoffset = PyList_GetItem( listitem, 1 );
				PyObject* iyoffset = PyList_GetItem( listitem, 2 );
				PyObject* izoffset = PyList_GetItem( listitem, 3 );
				PyObject* hue = PyList_GetItem( listitem, 4 );

				if ( PyInt_Check( id ) && PyInt_Check( ixoffset ) && PyInt_Check( iyoffset ) && PyInt_Check( izoffset ) && PyInt_Check( hue ) )
				{
					stTargetItem targetitem;
					targetitem.id = PyInt_AsLong( id );
					targetitem.xOffset = PyInt_AsLong( ixoffset );
					targetitem.yOffset = PyInt_AsLong( iyoffset );
					targetitem.zOffset = PyInt_AsLong( izoffset );
					targetitem.hue = PyInt_AsLong( hue );
					targetitems.push_back( targetitem );
				}
			}
		}
	}

	cPythonTarget* target = new cPythonTarget( responsefunc, timeoutfunc, cancelfunc, targetargs );

	if ( timeout )
	{
		target->setTimeout( Server::instance()->time() + timeout );
	}

	self->pSock->attachTarget( target, targetitems, xoffset, yoffset, zoffset );
	Py_RETURN_NONE;
}

static PyObject* wpSocket_attachmultitarget( wpSocket* self, PyObject* args )
{
	char* responsefunc;
	unsigned short multiid;
	PyObject* targetargs;
	char* cancelfunc = 0;
	char* timeoutfunc = 0;
	unsigned int timeout = 0;
	short xoffset, yoffset, zoffset;

	if ( !PyArg_ParseTuple( args, "sHO!hhh|ssI:socket.attachmultitarget"
		"(callback, multi, args, xoffset, yoffset, zoffset, [cancelcallback], [timeoutcallback], [timeout])", &responsefunc, &multiid, &PyList_Type, &targetargs, &xoffset, &yoffset, &zoffset, &cancelfunc, &timeoutfunc, &timeout ) )
	{
		return 0;
	}

	targetargs = PyList_AsTuple( targetargs );

	cPythonTarget* target = new cPythonTarget( responsefunc, timeoutfunc, cancelfunc, targetargs );

	if ( timeout ) {
		target->setTimeout( Server::instance()->time() + timeout );
	}

	self->pSock->attachTarget( target, 0x4000 + multiid, xoffset, yoffset, zoffset );
	Py_RETURN_NONE;
}

/*!
	Begins CH customization
*/
static PyObject* wpSocket_customize( wpSocket* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pSock )
		return PyFalse();

	if ( !checkArgItem( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
	P_ITEM signpost = getArgItem( 0 );

	cUOTxStartCustomHouse custom;
	custom.setSerial( signpost->getTag( "house" ).toInt() ); // Morex of signpost contain serial of house
	self->pSock->send( &custom );
	Py_RETURN_NONE;
}


// DEPRECATED
static PyObject* wpSocket_sendgump( wpSocket* self, PyObject* args )
{
	// Parameters:
	// x, y, nomove, noclose, nodispose, serial, type, layout, text, callback, args
	int x,
	y;
	bool nomove,
	noclose,
	nodispose;
	unsigned int serial, type;
	PyObject* layout,* texts,* py_args;
	char* callback;

	if ( !PyArg_ParseTuple( args, "iiBBBIIO!O!sO!:socket.sendgump", &x, &y, &nomove, &noclose, &nodispose, &serial, &type, &PyList_Type, &layout, &PyList_Type, &texts, &callback, &PyList_Type, &py_args ) )
	{
		return 0;
	}

	// Convert py_args to a tuple
	py_args = PyList_AsTuple( py_args );

	cPythonGump* gump = new cPythonGump( callback, py_args );
	if ( serial )
		gump->setSerial( serial );

	if ( type )
		gump->setType( type );

	gump->setX( x );
	gump->setY( y );
	gump->setNoClose( noclose );
	gump->setNoMove( nomove );
	gump->setNoDispose( nodispose );

	Q_INT32 i;
	for ( i = 0; i < PyList_Size( layout ); ++i )
	{
		PyObject* item = PyList_GetItem( layout, i );

		if ( PyString_Check( item ) )
		{
			gump->addRawLayout( PyString_AsString( item ) );
		}
		else if ( PyUnicode_Check( item ) )
		{
			gump->addRawLayout( Python2QString( item ) );
		}
		else
		{
			gump->addRawLayout( "" );
		}
	}

	for ( i = 0; i < PyList_Size( texts ); ++i )
	{
		PyObject* item = PyList_GetItem( texts, i );

		if ( PyString_Check( item ) )
		{
			gump->addRawText( PyString_AsString( item ) );
		}
		else if ( PyUnicode_Check( item ) )
		{
			gump->addRawText( Python2QString( item ) );
		}
		else
		{
			gump->addRawText( "" );
		}
	}

	self->pSock->send( gump );

	return PyInt_FromLong( gump->serial() );
}

/*
	\method socket.closegump
	\description Closes a gump sent to the client.
	\param type The type id of the gump(s) to close.
	\param buttonid Which button id should the client send as a response for the closed gumps.
	Defaults to 0.
*/
static PyObject* wpSocket_closegump( wpSocket* self, PyObject* args )
{
	unsigned int type;
	unsigned int button = 0;

	if ( !PyArg_ParseTuple( args, "i|i:socket.closegump(type, [button])", &type, &button ) )
	{
		return 0;
	}

	cUOTxCloseGump closeGump;
	closeGump.setButton( button );
	closeGump.setType( type );
	self->pSock->send( &closeGump );

	Py_RETURN_NONE;
}

/*
	\method socket.resendworld
	\description Resends the world to this socket. This method should be called
	whenever you move the player to a new area of the world.
	\param clean Should all objects being sent to the client be removed from it first.
	Defaults to 0. This parameter has a use for resending multis in the area.
*/
static PyObject* wpSocket_resendworld( wpSocket* self, PyObject* args )
{
	unsigned char clean = 0;
	if ( !PyArg_ParseTuple( args, "|B:socket.resendworld([clean=0])", &clean ) )
	{
		return 0;
	}

	self->pSock->resendWorld( clean != 0 );

	Py_RETURN_NONE;
}

/*
	\method socket.resendplayer
	\description Resend the information about the player to this socket only.
*/
static PyObject* wpSocket_resendplayer( wpSocket* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pSock )
		return PyFalse();
	self->pSock->resendPlayer( false );
	Py_RETURN_NONE;
}

/*
	\method socket.sendcontainer
	\description Sends the content and gump of a container to this socket.
	\param container The <object id="ITEM">item</object> object of the container
	that should be sent to the client.
*/
static PyObject* wpSocket_sendcontainer( wpSocket* self, PyObject* args )
{
	if ( !self->pSock )
		return PyFalse();

	if ( !checkArgItem( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	if ( !getArgItem( 0 ) )
		return PyFalse();

	self->pSock->sendContainer( getArgItem( 0 ) );

	Py_RETURN_NONE;
}

/*
	\method socket.sendobject
	\description Send an object to this socket only.
	\param object The <object id="ITEM">item</object> or <object id="char">char</object> char object that should
	be sent to this socket.
*/
static PyObject* wpSocket_sendobject( wpSocket* self, PyObject* args )
{
	if ( !self->pSock )
		Py_RETURN_NONE;

	cUObject *object;

	if (!PyArg_ParseTuple(args, "O&:socket.sendobject( object )", &PyConvertObject, &object)) {
		return 0;
	}

	P_ITEM pItem = dynamic_cast<P_ITEM>(object);

	if (pItem) {
		pItem->update(self->pSock);
	} else {
		P_CHAR pChar = dynamic_cast<P_CHAR>(object);
		if (pChar) {
			self->pSock->sendChar(pChar);
		}
	}

	Py_RETURN_NONE;
}

/*
	\method socket.removeobject
	\description Remove an object from this socket only.
	\param object The <object id="ITEM">item</object> or <object id="char">char</object> char object that should
	be removed from the view of this socket.
*/
static PyObject* wpSocket_removeobject( wpSocket* self, PyObject* args )
{
	if ( !self->pSock )
		Py_RETURN_NONE;

	cUObject *object;

	if (!PyArg_ParseTuple(args, "O&:socket.removeobject( object )", &PyConvertObject, &object)) {
		return 0;
	}

	if (object) {
		self->pSock->removeObject(object);
	}

	Py_RETURN_NONE;
}

// DEPRECATED
static PyObject* wpSocket_sendpacket( wpSocket* self, PyObject* args )
{
	if ( PyTuple_Size( args ) != 1 )
	{
		PyErr_BadArgument();
		return 0;
	}

	PyObject* list = PyTuple_GetItem( args, 0 );

	if ( !PyList_Check( list ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	// Build a packet
	int packetLength = PyList_Size( list );

	QByteArray buffer( packetLength );

	for ( int i = 0; i < packetLength; ++i )
		buffer[i] = PyInt_AsLong( PyList_GetItem( list, i ) );

	cUOPacket packet( buffer );
	self->pSock->send( &packet );

	Py_RETURN_NONE;
}

/*
	\method socket.sendpaperdoll
	\description Sends the paperdoll of character to this client.
	\param char The character whose paperdoll should be sent to the client.

*/
static PyObject* wpSocket_sendpaperdoll( wpSocket* self, PyObject* args )
{
	if ( !self->pSock )
		return PyFalse();

	if ( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	if ( !getArgChar( 0 ) )
		return PyFalse();

	self->pSock->sendPaperdoll( getArgChar( 0 ) );

	Py_RETURN_NONE;
}

/*
	\method socket.useitem
	\description Forces the player assigned to this socket to use a given item.
	\param item The item that should be activated.

*/
static PyObject* wpSocket_useitem( wpSocket* self, PyObject* args )
{
	P_ITEM item;

	if ( !PyArg_ParseTuple( args, "O&:socket.useitem(item)", &PyConvertItem, &item ) )
	{
		return 0;
	}

	if ( self->pSock->useItem( item ) )
	{
		Py_RETURN_TRUE;
	}
	else
	{
		Py_RETURN_FALSE;
	}
}

/*
	\method socket.gettag
	\description Get a custom tag attached to the socket.
	Please keep in mind these tags are temporary in nature.
	When the socket disconnects, the tag will be gone.
	\return None if there is no such tag, the tag value otherwise.
	Possible return types are: unicode (string), float, integer.
	\param name The name of the tag.
*/
static PyObject* wpSocket_gettag( wpSocket* self, PyObject* args )
{
	if ( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	cVariant value = self->pSock->tags().get( key );

	if ( value.type() == cVariant::StringType )
		return QString2Python(value.toString());
	else if ( value.type() == cVariant::IntType )
		return PyInt_FromLong( value.asInt() );
	else if ( value.type() == cVariant::DoubleType )
		return PyFloat_FromDouble( value.asDouble() );

	Py_RETURN_NONE;
}

/*
	\method socket.settag
	\description Set a custom tag on the object.
	\param name The name of the tag.
	Please keep in mind these tags are temporary in nature.
	When the socket disconnects, the tag will be gone.
	\param value The value of the tag. Possible value types
	are string, unicode, float and integer.
*/
static PyObject* wpSocket_settag( wpSocket* self, PyObject* args )
{
	char* key;
	PyObject* object;

	if ( !PyArg_ParseTuple( args, "sO:char.settag( name, value )", &key, &object ) )
		return 0;

	if ( PyString_Check( object ) || PyUnicode_Check( object ) )
	{
		self->pSock->tags().set( key, cVariant( Python2QString( object ) ) );
	}
	else if ( PyInt_Check( object ) )
	{
		self->pSock->tags().set( key, cVariant( ( int ) PyInt_AsLong( object ) ) );
	}
	else if ( PyFloat_Check( object ) )
	{
		self->pSock->tags().set( key, cVariant( ( double ) PyFloat_AsDouble( object ) ) );
	}

	Py_RETURN_NONE;
}

/*
	\method socket.hastag
	\description Check if the socket has a certain custom tag attached to it.
	\return True if the tag is present. False otherwise.
	\param name The name of the tag.
*/
static PyObject* wpSocket_hastag( wpSocket* self, PyObject* args )
{
	if ( !self->pSock )
		return PyFalse();

	if ( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );

	return self->pSock->tags().has( key ) ? PyTrue() : PyFalse();
}

/*
	\method socket.deltag
	\description Deletes a tag attached to the socket.
	\param name The name of the tag.
*/
static PyObject* wpSocket_deltag( wpSocket* self, PyObject* args )
{
	if ( !self->pSock )
		return PyFalse();

	if ( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	self->pSock->tags().remove( key );

	Py_RETURN_NONE;
}

/*
	\method socket.resendstatus
	\description Resends the status window to this socket.
	Use this for updating weight and other properties.
*/
static PyObject* wpSocket_resendstatus( wpSocket* self, PyObject* args )
{
	Q_UNUSED( args );
	self->pSock->sendStatWindow();
	Py_RETURN_NONE;
}

/*
	\method socket.updatelightlevel
	\description Send the current lightlevel to this socket.
	Use this for updating the lightlevel after you change the
	lightbonus property on character objects.
*/
static PyObject* wpSocket_updatelightlevel( wpSocket* self, PyObject* args )
{
	Q_UNUSED( args );
	self->pSock->updateLightLevel();
	Py_RETURN_NONE;
}

/*
	\method socket.questarrow
	\param show This boolean flag indicates whether the questarrow should be shown.
	\param x The x position the quest arrow points to.
	\param y The y position the quest arrow points to.
	\description This method shows a quest arrow that points to a given location on the map.
*/
static PyObject* wpSocket_questarrow( wpSocket* self, PyObject* args )
{
	int show;
	int x = 0;
	int y = 0;

	if ( !PyArg_ParseTuple( args, "i|ii:socket.questarrow( show, x, y )", &show, &x, &y ) )
		return 0;

	self->pSock->sendQuestArrow( show, x, y );
	Py_RETURN_NONE;
}

/*
	\method socket.log
	\param level The loglevel for this event. Check <library id="wolfpack.consts">wolfpack.consts</library> for constants.
	\param text The text that should be logged.
	\description Sends a given text to the logfile and console and prepends a timestamp and the socket id.
*/
static PyObject* wpSocket_log( wpSocket* self, PyObject* args )
{
	char loglevel;
	char* text;

	if ( !PyArg_ParseTuple( args, "bs:socket.log( loglevel, text )", &loglevel, &text ) )
		return 0;

	self->pSock->log( ( eLogLevel ) loglevel, text );
	Py_RETURN_NONE;
}

/*
	\method socket.updateplayer
	\description Resend the player of this socket only.
	Handle with care.
*/
static PyObject* wpSocket_updateplayer( wpSocket* self, PyObject* args )
{
	Q_UNUSED( args );
	self->pSock->updatePlayer();
	Py_RETURN_NONE;
}

/*
	\method socket.updateskill
	\param skill The skill id.
	\description Resends the value of one single skill.
*/
static PyObject* wpSocket_updateskill( wpSocket* self, PyObject* args )
{
	unsigned short skill;

	if ( !PyArg_ParseTuple( args, "H:socket.updateskill(skill)", &skill ) )
	{
		return 0;
	}

	self->pSock->sendSkill( skill );
	Py_RETURN_NONE;
}

/*
	\method socket.denymove
	\param sequence The id of the sequence that has been denied.
	\description Send a deny move packet and bounce the character back to its current position.
	Use this in the onWalk event.
*/
static PyObject *wpSocket_denymove( wpSocket* self, PyObject* args ) {
	unsigned char sequence;

	if (!PyArg_ParseTuple( args, "b:socket.denymove(sequence)", &sequence)) {
		return 0;
	}

	self->pSock->denyMove(sequence);
	Py_RETURN_NONE;
}

static PyMethodDef wpSocketMethods[] =
{
{ "useitem",			( getattrofunc ) wpSocket_useitem, METH_VARARGS, NULL },
{ "updateskill",		( getattrofunc ) wpSocket_updateskill, METH_VARARGS, NULL },
{ "updateplayer",		( getattrofunc ) wpSocket_updateplayer, METH_VARARGS, NULL },
{ "questarrow",			( getattrofunc ) wpSocket_questarrow, METH_VARARGS, NULL },
{ "sysmessage",			( getattrofunc ) wpSocket_sysmessage, METH_VARARGS, "Sends a system message to the char." },
{ "clilocmessage",		( getattrofunc ) wpSocket_clilocmessage, METH_VARARGS, "Sends a localized message to the socket." },
{ "showspeech",			( getattrofunc ) wpSocket_showspeech, METH_VARARGS, "Sends raw speech to the socket." },
{ "disconnect",			( getattrofunc ) wpSocket_disconnect, METH_VARARGS, "Disconnects the socket." },
{ "attachtarget",		( getattrofunc ) wpSocket_attachtarget,  METH_VARARGS, "Adds a target request to the socket" },
{ "attachmultitarget",	( getattrofunc ) wpSocket_attachmultitarget,  METH_VARARGS, "Adds a multi target request to the socket" },
{ "attachitemtarget",	( getattrofunc ) wpSocket_attachitemtarget,  METH_VARARGS, "Adds a target request to the socket" },
{ "sendgump",			( getattrofunc ) wpSocket_sendgump,	METH_VARARGS, "INTERNAL! Sends a gump to this socket." },
{ "closegump",			( getattrofunc ) wpSocket_closegump,	METH_VARARGS, "Closes a gump that has been sent to the client." },
{ "resendworld",		( getattrofunc ) wpSocket_resendworld,  METH_VARARGS, "Sends the surrounding world to this socket." },
{ "resendplayer",		( getattrofunc ) wpSocket_resendplayer,  METH_VARARGS, "Resends the player only." },
{ "sendcontainer",		( getattrofunc ) wpSocket_sendcontainer,  METH_VARARGS, "Sends a container to the socket." },
{ "sendobject",			( getattrofunc ) wpSocket_sendobject,  METH_VARARGS, NULL },
{ "removeobject",		( getattrofunc ) wpSocket_removeobject,  METH_VARARGS, NULL },
{ "sendpacket",			( getattrofunc ) wpSocket_sendpacket,		METH_VARARGS, "Sends a packet to this socket." },
{ "sendpaperdoll",		( getattrofunc ) wpSocket_sendpaperdoll,	METH_VARARGS,	"Sends a char's paperdool to this socket."	},
{ "gettag",				( getattrofunc ) wpSocket_gettag,	METH_VARARGS,	"Gets a tag from a socket." },
{ "settag",				( getattrofunc ) wpSocket_settag,	METH_VARARGS,	"Sets a tag to a socket." },
{ "hastag",				( getattrofunc ) wpSocket_hastag,	METH_VARARGS,	"Checks if a socket has a specific tag." },
{ "deltag",				( getattrofunc ) wpSocket_deltag,	METH_VARARGS,	"Delete specific tag." },
{ "resendstatus",		( getattrofunc ) wpSocket_resendstatus, METH_VARARGS,	"Resends the status windows to this client." },
{ "customize",			( getattrofunc ) wpSocket_customize, METH_VARARGS,	"Begin house customization." },
{ "log",				( getattrofunc ) wpSocket_log, METH_VARARGS, NULL },
{ "updatelightlevel",   ( getattrofunc ) wpSocket_updatelightlevel, METH_VARARGS, NULL },
{ "denymove",			( getattrofunc ) wpSocket_denymove, METH_VARARGS, NULL },
{ NULL, NULL, 0, NULL }
};

// Getters & Setters
static PyObject* wpSocket_getAttr( wpSocket* self, char* name )
{
	/*
		\rproperty socket.player The <object id="CHAR">char</object> object for the player played by this socket. May be None.
	*/
	if ( !strcmp( name, "player" ) )
		return PyGetCharObject( self->pSock->player() );
	/*
		\rproperty socket.screenwidth The width of the game window in pixels as sent by the client.
	*/
	else if ( !strcmp( name, "screenwidth" ) )
	{
		return PyInt_FromLong( self->pSock->screenWidth() );
	}
	/*
		\rproperty socket.screenheight The height of the game window in pixels as sent by the client.
	*/
	else if ( !strcmp( name, "screenheight" ) )
	{
		return PyInt_FromLong( self->pSock->screenHeight() );
	}
	else if ( !strcmp( name, "walksequence" ) )
	{
		return PyInt_FromLong( self->pSock->walkSequence() );
	}
	/*
		\rproperty socket.account An <object id="account">account</object> object for the account used by this socket. Should not be None.
	*/
	else if ( !strcmp( name, "account" ) )
	{
		return PyGetAccountObject( self->pSock->account() );
	}
	/*
		\rproperty socket.address A string containing the IP address this socket is connected to.
	*/
	else if ( !strcmp( name, "address" ) )
	{
		return QString2Python( self->pSock->ip() );
	}
	else
	{
		return Py_FindMethod( wpSocketMethods, ( PyObject * ) self, name );
	}
}

static int wpSocket_setAttr( wpSocket* self, char* name, PyObject* value )
{
	Q_UNUSED( self );
	if ( !strcmp( name, "walksequence" ) && PyInt_Check( value ) )
	{
		self->pSock->setWalkSequence( PyInt_AsLong( value ) );
		return 0;
	}
	return 1;
}

int PyConvertSocket( PyObject* object, cUOSocket** sock )
{
	if ( object->ob_type != &wpSocketType )
	{
		PyErr_BadArgument();
		return 0;
	}

	*sock = ( ( wpSocket * ) object )->pSock;
	return 1;
}
