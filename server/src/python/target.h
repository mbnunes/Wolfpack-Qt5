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

#ifndef __TARGET_H__
#define __TARGET_H__

#include "../defines.h"
#include "utilities.h"
#include "../targetrequests.h"

typedef struct {
    PyObject_HEAD;
	Coord_cl pos;
	UINT16 model;
	SERIAL object;
} wpTarget;

PyObject *wpTarget_getAttr( wpTarget *self, char *name )
{	
	if( !strcmp( name, "pos" ) )
		return PyGetCoordObject( self->pos );
	else if( !strcmp( name, "model" ) )
		return PyInt_FromLong( self->model );
	else if( !strcmp( name, "item" ) )
	{
		if( isItemSerial( self->object ) )
			return PyGetItemObject( FindItemBySerial( self->object ) );
	}
	else if( !strcmp( name, "char" ) )
		if( isCharSerial( self->object ) )
			return PyGetCharObject( FindCharBySerial( self->object ) );

	return PyFalse;
}

static PyTypeObject wpTargetType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpTarget",
    sizeof(wpTargetType),
    0,
	wpDealloc,
    0,
    (getattrfunc)wpTarget_getAttr,
    0,
    0,
    0,
    0,
    0,
};

PyObject *PyGetTarget( cUORxTarget *target, UINT8 map )
{
	if( !target )
		return Py_None;

	wpTarget *returnVal = PyObject_New( wpTarget, &wpTargetType );
	
	Coord_cl pos;
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();
	pos.map = map;

	returnVal->pos = pos;
	returnVal->object = target->serial();
	returnVal->model = target->model();

	return (PyObject*)returnVal;
}

class cPythonTarget: public cTargetRequest
{
private:
	QString responsefunc, timeoutfunc, cancelfunc;
	PyObject *args;
public:
	cPythonTarget( QString _responsefunc, QString _timeoutfunc, QString _cancelfunc, PyObject *_args ):
	  responsefunc( _responsefunc ), timeoutfunc( _timeoutfunc ), cancelfunc( _cancelfunc ), args( _args )
	  {
		  tuple_incref( args );
	  }

	bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// Try to call the python function
		// Get everything before the last dot
		if( responsefunc.contains( "." ) )
		{
			// Find the last dot
			INT32 position = responsefunc.findRev( "." );
			QString sModule = responsefunc.left( position );
			QString sFunction = responsefunc.right( responsefunc.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( socket->player() ) );
					PyTuple_SetItem( p_args, 1, args );
					PyTuple_SetItem( p_args, 2, PyGetTarget( target, socket->player()->pos.map ) );

					PyEval_CallObject( pFunc, p_args );

					if( PyErr_Occurred() )
						PyErr_Print();
				}
			}
		}

		tuple_decref( args );
		return true;
	}

	void timedout( cUOSocket *socket )
	{
		if( !timeoutfunc.isNull() && !timeoutfunc.isEmpty() && timeoutfunc.contains( "." ) )
		{
			// Find the last dot
			INT32 position = timeoutfunc.findRev( "." );
			QString sModule = timeoutfunc.left( position );
			QString sFunction = timeoutfunc.right( timeoutfunc.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *args = PyTuple_New( 1 );
					PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

					PyEval_CallObject( pFunc, args );

					if( PyErr_Occurred() )
						PyErr_Print();
				}
			}
		}

		tuple_decref( args );
	}

	void canceled( cUOSocket *socket )
	{
		if( !cancelfunc.isNull() && !cancelfunc.isEmpty() && cancelfunc.contains( "." ) )
		{
			// Find the last dot
			INT32 position = cancelfunc.findRev( "." );
			QString sModule = cancelfunc.left( position );
			QString sFunction = cancelfunc.right( cancelfunc.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *args = PyTuple_New( 1 );
					PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

					PyEval_CallObject( pFunc, args );

					if( PyErr_Occurred() )
						PyErr_Print();
				}
			}
		}

		tuple_decref( args );
	}
};

#endif
