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
#include "item.h"
#include "char.h"
#include "socket.h"
#include "../chars.h"
#include "../prototypes.h"
#include "../junk.h"

extern cCharStuff *Npcs;

/*!
	Struct for WP Python Chars
*/
typedef struct {
    PyObject_HEAD;
	P_CHAR pChar;
} wpChar;

// Forward Declarations
PyObject *wpChar_getAttr( wpChar *self, char *name );
int wpChar_setAttr( wpChar *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python chars
*/
static PyTypeObject wpCharType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPChar",
    sizeof(wpCharType),
    0,
    wpDealloc,				
    0,								
    (getattrfunc)wpChar_getAttr,
    (setattrfunc)wpChar_setAttr,
};

PyObject* PyGetCharObject( P_CHAR pChar )
{
	if( !pChar )
		return Py_None;

	wpChar *returnVal = PyObject_New( wpChar, &wpCharType );
	returnVal->pChar = pChar;
	return (PyObject*)returnVal;
}

// Methods

/*!
	Resends the character.
*/
PyObject* wpChar_update( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->resend( false );

	return PyTrue;
}

/*!
	Removes the character from view.
*/
PyObject* wpChar_removefromview( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->removeFromView( false );

	return PyTrue;
}

/*!
	Displays a message to the character if connected.
*/
PyObject* wpChar_message( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( ( PyTuple_Size( args ) < 1 ) || !PyString_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for char.message is 1" );
		return PyFalse;
	}

	QString message = PyString_AsString( PyTuple_GetItem( args, 0 ) );

	if( PyTuple_Size( args ) == 2 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
		self->pChar->message( message, PyInt_AsLong( PyTuple_GetItem( args, 1 ) ) );
	else
		self->pChar->message( message );

	return PyTrue;
}

/*!
	Moves the char to the specified location
*/
PyObject* wpChar_moveto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	// Gather parameters
	Coord_cl pos = self->pChar->pos;

	if( PyTuple_Size( args ) <= 1 )
	{
		clConsole.send( "Minimum argument count for item.moveto is 2" );
		return PyFalse;
	}

	// X,Y
	if( PyTuple_Size( args ) >= 2 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyFalse;

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	}

	// Z
	if( PyTuple_Size( args ) >= 3 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 2 ) ) )
			return PyFalse;

		pos.z = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );
	}

	// PLANE
	if( PyTuple_Size( args ) >= 4 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 3 ) ) )
			return PyFalse;

		pos.plane = PyInt_AsLong( PyTuple_GetItem( args, 3 ) );
	}

	self->pChar->moveTo( pos );

	return PyTrue;
}

/*!
	Plays a soundeffect originating from the char
*/
PyObject* wpChar_soundeffect( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for char.soundeffect is 1\n" );
		return PyFalse;
	}

	if( PyTuple_Size( args ) > 1 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) && !PyInt_AsLong( PyTuple_GetItem( args, 1 ) ) )
		self->pChar->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ), false );
	else
		self->pChar->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) );

	return PyTrue;
}

/*!
	Returns the distance towards a given object or position
*/
PyObject* wpChar_distance( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyInt_FromLong( -1 );

	// Probably an object
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *pObj = PyTuple_GetItem( args, 0 );

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( pItem->pos.distance( self->pChar->pos ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( pChar->pos.distance( self->pChar->pos ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pChar->pos;

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( self->pChar->pos.distance( pos ) );
	}

	clConsole.send( "Minimum argment count for char.distance is 1\n" );
	return PyInt_FromLong( -1 );
}

static PyMethodDef wpCharMethods[] = 
{
	{ "moveto",			(getattrofunc)wpChar_moveto, METH_VARARGS, "Moves the character to the specified location." },
    { "update",			(getattrofunc)wpChar_update, METH_VARARGS, "Sends the char to all clients in range." },
	{ "removefromview", (getattrofunc)wpChar_removefromview, METH_VARARGS, "Removes the char from all surrounding clients." },
	{ "message",		(getattrofunc)wpChar_message, METH_VARARGS, "Displays a message above the characters head - only visible for the player." },
	{ "soundeffect",	(getattrofunc)wpChar_soundeffect, METH_VARARGS, "Plays a soundeffect for the character." },
	{ "distance",		(getattrofunc)wpChar_distance, METH_VARARGS, "Distance to another object or a given position." },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpChar_getAttr( wpChar *self, char *name )
{
	QString field = name;

	getStrProperty( "name", pChar->name.c_str() )
	else getStrProperty( "orgname", pChar->orgname().latin1() )
	else getStrProperty( "title", pChar->title().latin1() )
	else getIntProperty( "serial", pChar->serial )
	else getIntProperty( "body", pChar->id() )
	else getIntProperty( "xbody", pChar->xid )
	else getIntProperty( "skin", pChar->skin() )
	else getIntProperty( "xskin", pChar->xskin() )
	
	else getIntProperty( "health", pChar->hp )
	else getIntProperty( "stamina", pChar->stm )
	else getIntProperty( "mana", pChar->mn )

	else getIntProperty( "str", pChar->st )
	else getIntProperty( "dex", pChar->effDex() )
	else getIntProperty( "int", pChar->in )

	else getIntProperty( "x", pChar->pos.x )
	else getIntProperty( "y", pChar->pos.y )
	else getIntProperty( "z", pChar->pos.z )
	else getIntProperty( "plane", pChar->pos.plane )

	else getIntProperty( "direction", pChar->dir )
	else getIntProperty( "flags2", pChar->priv2 )
	else getIntProperty( "hidamage", pChar->hidamage )
	else getIntProperty( "lodamage", pChar->lodamage )

	else if( !strcmp( "socket", name ) )
		return PyGetSocketObject( self->pChar->socket() );

	else if( !strcmp( "equipment", name ) )
	{
		/*Py_WPEquipment *returnVal = PyObject_New( Py_WPEquipment, &Py_WPEquipmentType );
		returnVal->pChar = self->pChar; // Never forget that
		return (PyObject*)returnVal;*/
		return Py_None;
	}

	// Base skill
	else for( UINT8 i = 0; i < ALLSKILLS; ++i )
		if( field.upper() == skillname[ i ] ) 
			return PyInt_FromLong( self->pChar->baseSkill( i ) );

	// If no property is found search for a method
	return Py_FindMethod( wpCharMethods, (PyObject*)self, name );
}

int wpChar_setAttr( wpChar *self, char *name, PyObject *value )
{
	setStrProperty( "name", pChar->name )
	else if ( !strcmp( "orgname", name) )
		self->pChar->setOrgname( PyString_AS_STRING( value ) );
	else if ( !strcmp( "title", name) )
		self->pChar->setTitle( PyString_AS_STRING( value ) );
	else setIntProperty( "serial", pChar->serial )
	
	else if( !strcmp( "body", name ) )
		self->pChar->setId( PyInt_AS_LONG( value ) );
	
	else setIntProperty( "xbody", pChar->xid )
	else if ( !strcmp( "skin", name ) )
		self->pChar->setSkin( PyInt_AS_LONG(value ) );
	else if ( !strcmp( "xskin", name ) )
		self->pChar->setXSkin( PyInt_AS_LONG(value ) );
	
	else setIntProperty( "health", pChar->hp )
	else setIntProperty( "stamina", pChar->stm )
	else setIntProperty( "mana", pChar->mn )

	else setIntProperty( "str", pChar->st )
	
	else if( !strcmp( "dex", name ) )
		self->pChar->setDex( PyInt_AS_LONG( value ) );
	
	else setIntProperty( "int", pChar->in )
	else setIntProperty( "direction", pChar->dir )
	else setIntProperty( "flags2", pChar->priv2 )
	else setIntProperty( "hidamage", pChar->hidamage )
	else setIntProperty( "lodamage", pChar->lodamage )

	return 0;
}

P_CHAR getWpChar( PyObject *pObj )
{
	if( pObj->ob_type != &wpCharType )
		return 0;

	wpChar *item = (wpChar*)( pObj );
	return item->pChar;
}
