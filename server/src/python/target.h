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

#ifndef __TARGET_H__
#define __TARGET_H__

#include "../defines.h"
#include "utilities.h"
#include "../targetrequests.h"

/*
	\object target
	\description This object type represents the clients response to a target request.
*/
typedef struct
{
	PyObject_HEAD;
	Coord_cl pos;
	Q_UINT16 model;
	SERIAL object;
} wpTarget;

static PyObject* wpTarget_getAttr( wpTarget* self, char* name )
{
	/*
		\rproperty target.pos The value of this property is a <object id="coord">coord</object> object representing the location of the targetted object or ground tile.
		Remember that this value should not be trusted.
	*/
	if ( !strcmp( name, "pos" ) )
		return PyGetCoordObject( self->pos );
	/*
		\rproperty target.model If a static tile has been targetted by the client, this property contains the art id of the targetted static tile. If a character has been targetted,
		this property contains the body id of the targetted character. Please note that this value is sent by the client and not determined by the server.
	*/
	else if ( !strcmp( name, "model" ) )
		return PyInt_FromLong( self->model );
	/*
		\rproperty target.item If a valid item has been targetted, this property contains an <object id="item">item</object> object for the targetted item.
	*/
	else if ( !strcmp( name, "item" ) )
	{
		if ( isItemSerial( self->object ) )
			return PyGetItemObject( FindItemBySerial( self->object ) );
	}
	/*
		\rproperty target.char If a valid character has been targetted, this property contains a <object id="char">char</object> object for the targetted character.
	*/
	else if ( !strcmp( name, "char" ) )
		if ( isCharSerial( self->object ) )
			return PyGetCharObject( FindCharBySerial( self->object ) );

	Py_RETURN_FALSE;
}

static PyTypeObject wpTargetType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpTarget",
sizeof( wpTargetType ),
0,
wpDealloc,
0,
( getattrfunc ) wpTarget_getAttr,
0,
0,
0,
0,
0,

};

static PyObject* PyGetTarget( cUORxTarget* target, Q_UINT8 map )
{
	if ( !target )
	{
		Py_RETURN_NONE;
	}

	wpTarget* returnVal = PyObject_New( wpTarget, &wpTargetType );

	Coord_cl pos;
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();
	pos.map = map;

	returnVal->pos = pos;
	returnVal->object = target->serial();
	returnVal->model = target->model();

	return ( PyObject * ) returnVal;
}

class cPythonTarget : public cTargetRequest
{
private:
	QString responsefunc,
	timeoutfunc,
	cancelfunc;
	PyObject* args;
public:
	cPythonTarget( QString _responsefunc, QString _timeoutfunc, QString _cancelfunc, PyObject* _args ) : responsefunc( _responsefunc ), timeoutfunc( _timeoutfunc ), cancelfunc( _cancelfunc ), args( _args )
	{
		Py_INCREF( args );
	}

	bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		// Try to call the python function
		// Get everything before the last dot
		if ( responsefunc.contains( "." ) )
		{
			// Find the last dot
			INT32 position = responsefunc.findRev( "." );
			QString sModule = responsefunc.left( position );
			QString sFunction = responsefunc.right( responsefunc.length() - ( position + 1 ) );

			PyObject* pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );

			if ( pModule )
			{
				PyObject* pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );

				if ( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject* p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( socket->player() ) );
					PyTuple_SetItem( p_args, 1, args );
					PyTuple_SetItem( p_args, 2, PyGetTarget( target, socket->player()->pos().map ) );

					PyObject* result = PyEval_CallObject( pFunc, p_args );
					Py_XDECREF( result );
					reportPythonError( sModule );
				}

				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}

		Py_DECREF( args );
		return true;
	}

	void timedout( cUOSocket* socket )
	{
		if ( !timeoutfunc.isNull() && !timeoutfunc.isEmpty() && timeoutfunc.contains( "." ) )
		{
			// Find the last dot
			INT32 position = timeoutfunc.findRev( "." );
			QString sModule = timeoutfunc.left( position );
			QString sFunction = timeoutfunc.right( timeoutfunc.length() - ( position + 1 ) );

			PyObject* pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );

			if ( pModule )
			{
				PyObject* pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );

				if ( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject* args = PyTuple_New( 1 );
					PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

					PyObject* result = PyEval_CallObject( pFunc, args );
					Py_XDECREF( result );
					reportPythonError( sModule );
				}

				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}

		Py_DECREF( args );
	}

	void canceled( cUOSocket* socket )
	{
		if ( !cancelfunc.isNull() && !cancelfunc.isEmpty() && cancelfunc.contains( "." ) )
		{
			// Find the last dot
			INT32 position = cancelfunc.findRev( "." );
			QString sModule = cancelfunc.left( position );
			QString sFunction = cancelfunc.right( cancelfunc.length() - ( position + 1 ) );

			PyObject* pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );

			if ( pModule )
			{
				PyObject* pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );

				if ( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject* args = PyTuple_New( 1 );
					PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

					PyObject* result = PyEval_CallObject( pFunc, args );
					Py_XDECREF( result );

					reportPythonError( sModule );
				}

				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}

		Py_DECREF( args );
	}
};

#endif
