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

#include "Python.h"

#include "../items.h"
#include "../tilecache.h"
#include "../prototypes.h"
#include "../wolfpack.h"
#include "../wpscriptmanager.h"
#include "../itemid.h"
#include "../spellbook.h"
#include "../multis.h"
#include "../network/uotxpackets.h"

extern cAllItems *Items;

#include "utilities.h"
#include "content.h"
#include "tempeffect.h"

/*!
	The object for Wolfpack Python items
*/
typedef struct {
	PyObject_HEAD;
	P_MULTI pMulti;
} wpMulti;

// Forward Declarations
PyObject *wpMulti_getAttr( wpMulti *self, char *name );
int wpMulti_setAttr( wpMulti *self, char *name, PyObject *value );
int wpMulti_compare( PyObject *a, PyObject *b );

/*!
	The typedef for Wolfpack Python multis
*/
static PyTypeObject wpMultiType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"wpmulti",
	sizeof(wpMultiType),
	0,
	wpDealloc,				
	0,								
	(getattrfunc)wpMulti_getAttr,
	(setattrfunc)wpMulti_setAttr,
	wpMulti_compare,
};

PyObject* PyGetMultiObject( P_MULTI pMulti )
{
	if( pMulti == NULL )
		return Py_None;

	wpMulti *returnVal = PyObject_New( wpMulti, &wpMultiType );
	returnVal->pMulti = pMulti;
	return (PyObject*)returnVal;
}

// Method declarations

/*!
	Resends the multi to all clients in range
*/
PyObject* wpMulti_update( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	self->pMulti->update();

	return PyTrue;
}

/*!
	Sends custom house to client
*/
PyObject* wpMulti_sendcustomhouse( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pMulti || self->pMulti->free || !self->pMulti->ishouse() )
		return PyFalse;
	
	if( PyTuple_Size( args ) < 1 || !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
	P_CHAR player = getArgChar( 0 );

	self->pMulti->sendCH( player->socket() );

	return PyTrue;
}

/*! 
	Adds a tile to the custom house
*/
PyObject* wpMulti_addchtile( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pMulti || self->pMulti->free || !self->pMulti->ishouse() )
		return PyFalse;
	
	if( PyTuple_Size( args ) < 4 || !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) || !checkArgInt( 3 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
	UINT16 model;
	Coord_cl pos;
	
	model = getArgInt( 0 );
	pos.x = getArgInt( 1 );
	pos.y = getArgInt( 2 );
	pos.z = getArgInt( 3 );
	
	self->pMulti->addCHTile( model, pos );

	return PyTrue;
}
/*!
	Removes the multi
*/
PyObject* wpMulti_delete( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	self->pMulti->del();
	Items->DeleItem( self->pMulti );

	return PyTrue;
}

/*!
	Moves the multi to the specified location
*/
PyObject* wpMulti_moveto( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( PyTuple_Size( args ) == 1 && checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
	{
		self->pMulti->moveTo( getWpCoord( PyTuple_GetItem( args, 0 ) ) );
		return PyTrue;
	}

	// Gather parameters
	Coord_cl pos = self->pMulti->pos();

	if( PyTuple_Size( args ) <= 1 )
	{
		PyErr_BadArgument();
		return NULL;
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

	// MAP
	if( PyTuple_Size( args ) >= 4 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 3 ) ) )
			return PyFalse;

		pos.map = PyInt_AsLong( PyTuple_GetItem( args, 3 ) );
	}

	self->pMulti->moveTo( pos );

	return PyTrue;
}


/*!
	Removes the multi from all clients in range
*/
PyObject* wpMulti_removefromview( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( !checkArgInt( 0 ) || getArgInt( 0 ) == 0 )
		self->pMulti->removeFromView( false );
	else
		self->pMulti->removeFromView( true );

	return PyTrue;
}

/*!
	Plays a soundeffect originating from the multi
*/
PyObject* wpMulti_soundeffect( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	self->pMulti->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) );

	return PyTrue;
}

/*
 * Adds a temp effect to this multi.
 */
PyObject* wpMulti_addtimer( wpMulti* self, PyObject* args )
{
	if( (PyTuple_Size( args ) < 3 && PyTuple_Size( args ) > 4) || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
	UINT32 expiretime = getArgInt( 0 );
	QString function = getArgStr( 1 );
	PyObject *py_args = PyList_AsTuple( PyTuple_GetItem( args, 2 ) );
	cPythonEffect *effect = new cPythonEffect( function, py_args );
	if( checkArgInt( 3 ) && getArgInt( 3 ) != 0 )
		effect->setSerializable( true );
	else
		effect->setSerializable( false );
	effect->setDest( self->pMulti->serial() );
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );
	return PyTrue;
}

/*!
	Returns the custom tag passed
*/
PyObject* wpMulti_gettag( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return Py_None;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	cVariant value = self->pMulti->tags().get( key );

	if( value.type() == cVariant::String )
	{
		QString strValue = value.asString();

		if( !strValue.isNull() )
			return PyString_FromString( strValue.latin1() );
		else
			return PyString_FromString( "" );
	}
	else if( value.type() == cVariant::Int )
		return PyInt_FromLong( value.asInt() );

	return Py_None;
}

/*!
	Sets a custom tag
*/
PyObject* wpMulti_settag( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) || ( !checkArgStr( 1 ) && !checkArgInt( 1 )  ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );

	self->pMulti->tags().remove( key );

	if( checkArgStr( 1 ) )
		self->pMulti->tags().set( key, cVariant( QString( getArgStr( 1 ) ) ) );
	else if( checkArgInt( 1 ) )
		self->pMulti->tags().set( key, cVariant( (int)getArgInt( 1 ) ) );

	return PyTrue;
}

/*!
	Checks if a certain tag exists
*/
PyObject* wpMulti_hastag( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	
	return self->pMulti->tags().get( key ).isValid() ? PyTrue : PyFalse;
}

/*!
	Deletes a given tag
*/
PyObject* wpMulti_deltag( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	self->pMulti->tags().remove( key );

	return PyTrue;
}

PyObject* wpMulti_ismulti( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);	
	Q_UNUSED(self);	
	return PyTrue;
}

/*
 * is this multi a house ?
 */
PyObject* wpMulti_ishouse( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
	return self->pMulti->ishouse() ? PyTrue : PyFalse;
}

/*
 * is this multi a boat ?
 */
PyObject* wpMulti_isboat( wpMulti* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
	return self->pMulti->isboat() ? PyTrue : PyFalse;
}

/*!
	Adds an item to this multi.
*/
PyObject* wpMulti_additem( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;

	if( !checkArgItem( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_ITEM pItem = getArgItem( 0 );

	self->pMulti->addItem( pItem );

	return PyTrue;
}

/*
 * Returns the list of the chars in this multi
 */
PyObject* wpMulti_chars( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;
	QValueList< SERIAL > chars = self->pMulti->chars();
	QValueList< SERIAL >::iterator it = chars.begin();
	PyObject* list = PyList_New( chars.size() );
	while( it != chars.end() )
	{
		P_CHAR pChar = FindCharBySerial( *it );
		if( pChar )
			PyList_Append( list, PyGetCharObject( pChar ) );
		it ++;
	}
	return list;
}

/*
 * Returns the list of the items in this multi
 */
PyObject* wpMulti_items( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;
	QValueList< SERIAL > items = self->pMulti->items();
	QValueList< SERIAL >::iterator it = items.begin();
	PyObject* list = PyList_New( items.size() );
	while( it != items.end() )
	{
		P_ITEM pItem = FindItemBySerial( *it );
		if( pItem )
			PyList_Append( list, PyGetItemObject( pItem ) );
		it ++;
	}
	return list;
}

/*
 * Returns the friends list of this multi
 */
PyObject* wpMulti_friends( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;
	std::vector< SERIAL > friends = self->pMulti->friends();
	std::vector< SERIAL >::iterator it = friends.begin();
	PyObject* list = PyList_New( friends.size() );
	while( it != friends.end() )
	{
		P_CHAR pChar = FindCharBySerial( *it );
		if( pChar )
			PyList_Append( list, PyGetCharObject( pChar ) );
		it ++;
	}
	return list;
}

/*
 * Returns the bans list of this multi
 */
PyObject* wpMulti_bans( wpMulti* self, PyObject* args )
{
	if( !self->pMulti || self->pMulti->free )
		return PyFalse;
	std::vector< SERIAL > bans = self->pMulti->bans();
	std::vector< SERIAL >::iterator it = bans.begin();
	PyObject* list = PyList_New( bans.size() );
	while( it != bans.end() )
	{
		P_CHAR pChar = FindCharBySerial( *it );
		if( pChar )
			PyList_Append( list, PyGetCharObject( pChar ) );
		it ++;
	}
	return list;
}

static PyMethodDef wpMultiMethods[] = 
{
	{ "additem",			(getattrofunc)wpMulti_additem, METH_VARARGS, "Adds an item to this multi." },
    { "update",			(getattrofunc)wpMulti_update, METH_VARARGS, "Sends the multi to all clients in range." },
	{ "removefromview",	(getattrofunc)wpMulti_removefromview, METH_VARARGS, "Removes the multi from the view of all in-range clients." },
	{ "delete",				(getattrofunc)wpMulti_delete, METH_VARARGS, "Deletes the multi and the underlying reference." },
	{ "moveto",				(getattrofunc)wpMulti_moveto, METH_VARARGS, "Moves the multi to the specified location." },
	{ "soundeffect",		(getattrofunc)wpMulti_soundeffect, METH_VARARGS, "Sends a soundeffect to the surrounding sockets." },
	{ "addtimer",			(getattrofunc)wpMulti_addtimer, METH_VARARGS, "Attaches a timer to this object." },
	{ "chars",				(getattrofunc)wpMulti_chars, METH_VARARGS, "Returns the list of the chars in this multi." },
	{ "items",				(getattrofunc)wpMulti_items, METH_VARARGS, "Returns the list of the items in this multi." },
	{ "friends",			(getattrofunc)wpMulti_friends, METH_VARARGS, "Returns the friends list of this multi." },
	{ "bans",				(getattrofunc)wpMulti_friends, METH_VARARGS, "Returns the ban list of this multi." },
	{ "addchtile",			(getattrofunc)wpMulti_addchtile, METH_VARARGS, "Adds a tile to the custom house." },

	// Tag System
	{ "gettag",				(getattrofunc)wpMulti_gettag, METH_VARARGS, "Gets a tag assigned to a specific item." },
	{ "settag",				(getattrofunc)wpMulti_settag, METH_VARARGS, "Sets a tag assigned to a specific item." },
	{ "hastag",				(getattrofunc)wpMulti_hastag, METH_VARARGS, "Checks if a certain item has the specified tag." },
	{ "deltag",				(getattrofunc)wpMulti_deltag, METH_VARARGS, "Deletes the specified tag." },

	// Is*? Functions
	{ "ismulti",			(getattrofunc)wpMulti_ismulti, METH_VARARGS, "Is this an multi?" },
	{ "ishouse",			(getattrofunc)wpMulti_ishouse, METH_VARARGS, "Is this a house?" },
	{ "isboat",				(getattrofunc)wpMulti_isboat, METH_VARARGS, "Is this a boat?" },
    { NULL, NULL, 0, NULL }
};

// Getters + Setters

PyObject *wpMulti_getAttr( wpMulti *self, char *name )
{
	if( !strcmp( "events", name ) )
	{
		QStringList events = QStringList::split( ",", self->pMulti->eventList() );
		PyObject *list = PyList_New( events.count() );
		for( INT32 i = 0; i < events.count(); ++i )
			PyList_SetItem( list, i, PyString_FromString( events[i].latin1() ) );
		return list;
	}
	else
	{
		cVariant result;
		stError *error = self->pMulti->getProperty( name, result );

		if( !error )
		{
			PyObject *obj = 0;

			switch( result.type() )
			{
			case cVariant::Char:
				obj = PyGetCharObject( result.toChar() );
				break;
			case cVariant::Item:
				obj = PyGetItemObject( result.toItem() );
				break;
			case cVariant::Long:
			case cVariant::Int:
				obj = PyInt_FromLong( result.toInt() );
				break;
			case cVariant::String:
				if( result.toString().isNull() )
					obj = PyString_FromString( "" );
				else
					obj = PyString_FromString( result.toString().latin1() );
				break;
			case cVariant::Double:
				obj = PyFloat_FromDouble( result.toDouble() );
				break;
			case cVariant::Coord:
				obj = PyGetCoordObject( result.toCoord() );
				break;
			}

			if( !obj )
			{
				PyErr_Format( PyExc_ValueError, "Unsupported Property Type: %s", result.typeName() );
				return 0;
			}

			return obj;
		}
		else
			delete error;
	}	
	
	return Py_FindMethod( wpMultiMethods, (PyObject*)self, name );
}

int wpMulti_setAttr( wpMulti *self, char *name, PyObject *value )
{
	if( !strcmp( "events", name ) )
	{
		if( !PyList_Check( value ) )
		{
			PyErr_BadArgument();
			return -1;
		}

		self->pMulti->clearEvents();
		int i;
		for( i = 0; i < PyList_Size( value ); ++i )
		{
			if( !PyString_Check( PyList_GetItem( value, i ) ) )
				continue;

			WPDefaultScript *script = ScriptManager->find( PyString_AsString( PyList_GetItem( value, i ) ) );
			if( script )
				self->pMulti->addEvent( script );
		}
	}
	else
	{
		cVariant val;
		if( PyString_Check( value ) )
			val = cVariant( PyString_AsString( value ) );
		else if( PyInt_Check( value ) )
			val = cVariant( PyInt_AsLong( value ) );
		else if( checkWpItem( value ) )
			val = cVariant( getWpItem( value ) );
		else if( checkWpChar( value ) )
			val = cVariant( getWpChar( value ) );
		else if( checkWpCoord( value ) )
			val = cVariant( getWpCoord( value ) );
		else if( PyFloat_Check( value ) )
			val = cVariant( PyFloat_AsDouble( value ) );

		if( !val.isValid() )
		{
			if( value->ob_type )
				PyErr_Format( PyExc_TypeError, "Unsupported object type: %s", value->ob_type->tp_name );
			else
				PyErr_Format( PyExc_TypeError, "Unknown object type" );
			return 0;
		}

		stError *error = self->pMulti->setProperty( name, val );

		if( error )
		{
			PyErr_Format( PyExc_TypeError, "Error while setting attribute '%s': %s", name, error->text.latin1() );
			delete error;
			return 0;
		}
	}

	return 0;
}

P_MULTI getWpMulti( PyObject *pObj )
{
	if( pObj->ob_type != &wpMultiType )
		return 0;

	wpMulti *multi = (wpMulti*)( pObj );
	return multi->pMulti;
}

bool checkWpMulti( PyObject *pObj )
{
	if( pObj->ob_type != &wpMultiType )
		return false;
	else
		return true;
}

int wpMulti_compare( PyObject *a, PyObject *b )
{
	// Both have to be characters
	if( a->ob_type != &wpMultiType || b->ob_type != &wpMultiType ) 
		return -1;

	P_ITEM pA = getWpMulti( a );
	P_ITEM pB = getWpMulti( b );

	return !( pA == pB );
}
