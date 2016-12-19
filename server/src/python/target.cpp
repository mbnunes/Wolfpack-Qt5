/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2016 by holders identified in AUTHORS.txt
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

#include "target.h"

static PyObject* wpTarget_getAttr( wpTarget* self, char* name )
{
	/*
		\rproperty target.pos The value of this property is a <object id="coord">coord</object> object representing the location of the targetted object or ground tile.
		Remember that this value should not be trusted.
	*/
	if ( !strcmp( name, "pos" ) )
	{
		if ( !self->py_pos )
			self->py_pos = PyGetCoordObject( self->pos );

		Py_INCREF( self->py_pos );
		return self->py_pos;
	}
	/*
		\rproperty target.model If a static tile has been targetted by the client, this property contains the art id of the targetted static tile. If a character has been targetted,
		this property contains the body id of the targetted character. Please note that this value is sent by the client and not determined by the server.
	*/
	else if ( !strcmp( name, "model" ) )
	{
		return PyInt_FromLong( self->model );
	}
	/*
		\rproperty target.item If a valid item has been targetted, this property contains an <object id="item">item</object> object for the targetted item.
	*/
	else if ( !strcmp( name, "item" ) )
	{
		if ( isItemSerial( self->object ) )
		{
			if ( !self->py_obj )
				self->py_obj = PyGetItemObject( FindItemBySerial( self->object ) );

			Py_INCREF( self->py_obj );
			return self->py_obj;
		}
	}
	/*
		\rproperty target.char If a valid character has been targetted, this property contains a <object id="char">char</object> object for the targetted character.
	*/
	else if ( !strcmp( name, "char" ) )
	{
		if ( isCharSerial( self->object ) )
		{
			if ( !self->py_obj )
				self->py_obj = PyGetCharObject( FindCharBySerial( self->object ) );

			Py_INCREF( self->py_obj );
			return self->py_obj;
		}
	}
	Py_RETURN_FALSE;
}


static void wpTarget_Dealloc( wpTarget* self )
{
	Py_XDECREF( self->py_pos );
	Py_XDECREF( self->py_obj );

	wpDealloc( ( PyObject * ) self );
}


PyTypeObject wpTargetType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpTarget",
sizeof( wpTargetType ),
0,
( destructor ) wpTarget_Dealloc,
0,
( getattrfunc ) wpTarget_getAttr,
0,
0,
0,
0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

};


static PyObject* PyGetTarget( cUORxTarget* target, quint8 map )
{
	if ( !target )
	{
		Py_RETURN_NONE;
	}

	wpTarget* returnVal = PyObject_New( wpTarget, &wpTargetType );

	Coord pos;
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();
	pos.map = map;

	returnVal->pos = pos;
	returnVal->object = target->serial();
	returnVal->model = target->model();

	returnVal->py_pos = NULL;
	returnVal->py_obj = NULL;

	return ( PyObject * ) returnVal;
}

bool cPythonTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	PythonFunction function( responsefunc, true );
	if ( function.isValid() )
	{
		// Create our Argument list
		PyObject* p_args = PyTuple_New( 3 );
		PyTuple_SetItem( p_args, 0, PyGetCharObject( socket->player() ) );
		Py_INCREF( args );
		PyTuple_SetItem( p_args, 1, args );
		PyTuple_SetItem( p_args, 2, PyGetTarget( target, socket->player()->pos().map ) );

		PyObject* result = function( p_args );
		Py_XDECREF( result );
	}
	return true;
}

void cPythonTarget::timedout( cUOSocket* socket )
{
	PythonFunction function( timeoutfunc, true );
	if ( function.isValid() )
	{
		// Create our Argument list
		PyObject* args = PyTuple_New( 1 );
		PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

		PyObject* result = function( args );
		Py_XDECREF( result );
	}
}

void cPythonTarget::canceled( cUOSocket* socket )
{
	PythonFunction function( cancelfunc, true );
	if ( function.isValid() )
	{
		// Create our Argument list
		PyObject* args = PyTuple_New( 1 );
		PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

		PyObject* result = function( args );
		Py_XDECREF( result );
	}
}
