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
#include "../coord.h"

/*!
	The object for Wolfpack Coord items
*/
typedef struct {
    PyObject_HEAD;
	Coord_cl coord;
} wpCoord;

// Forward Declarations
PyObject *wpCoord_getAttr( wpCoord *self, char *name );
int wpCoord_setAttr( wpCoord *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpCoordType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPCoord",
    sizeof(wpCoordType),
    0,
    wpDealloc,				
    0,								
    (getattrfunc)wpCoord_getAttr,
    (setattrfunc)wpCoord_setAttr,
};

PyObject *wpCoord_distance( wpCoord *self, PyObject *args )
{
	// Check if the paramter is a coordinate
	if( !checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
	{
		return PyInt_FromLong( -1 );
	}
	else
	{
		Coord_cl pos = getWpCoord( PyTuple_GetItem( args, 0 ) );

		// Calculate the distance
		return PyInt_FromLong( self->coord.distance( pos ) );
	}
}

static PyMethodDef wpCoordMethods[] = 
{
	{ "distance",	(getattrofunc)wpCoord_distance, METH_VARARGS, "Whats the distance between Point A and Point B" },
};

PyObject *wpCoord_getAttr( wpCoord *self, char *name )
{
	if( !strcmp( name, "x" ) )
		return PyInt_FromLong( self->coord.x );
	else if( !strcmp( name, "y" ) )
		return PyInt_FromLong( self->coord.y );
	else if( !strcmp( name, "z" ) )
		return PyInt_FromLong( self->coord.z );
	else if( !strcmp( name, "map" ) )
		return PyInt_FromLong( self->coord.map );
	else
		return Py_FindMethod( wpCoordMethods, (PyObject*)self, name );
}

int wpCoord_setAttr( wpCoord *self, char *name, PyObject *value )
{
	// I only have integer params in mind
	if( !PyInt_Check( value ) )
		return 1;

	if( !strcmp( name, "x" ) )
		self->coord.x = PyInt_AsLong( value );
	else if( !strcmp( name, "y" ) )
		self->coord.y = PyInt_AsLong( value );
	else if( !strcmp( name, "z" ) )
		self->coord.z = PyInt_AsLong( value );
	else if( !strcmp( name, "map" ) )
		self->coord.map = PyInt_AsLong( value );

	return 0;
}

bool checkWpCoord( PyObject *object )
{
	return ( object->ob_type == &wpCoordType );
}

PyObject *PyGetCoordObject( const Coord_cl& coord )
{
	wpCoord *cObject = PyObject_New( wpCoord, &wpCoordType );
	cObject->coord = coord;
    return (PyObject*)( cObject );	
}

Coord_cl getWpCoord( PyObject* object )
{
	Coord_cl coord;

	if( object->ob_type == &wpCoordType )
		coord = ((wpCoord*)object)->coord;

	return coord;
}
