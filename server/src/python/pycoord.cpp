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

#include "utilities.h"
#include "../coord.h"
#include "../walking.h"

/*
	\object coord
	\description This object type represents a coordinate in the ultima online world.
	Use the coord function in the <module id="wolfpack">wolfpack</module> module to create an instance of this object type.
*/
typedef struct
{
	PyObject_HEAD;
	Coord_cl coord;
} wpCoord;

// Return a string representation for a coord object.
static PyObject *wpCoord_str(wpCoord *object) {
	const Coord_cl &pos = object->coord;
	return PyString_FromFormat("%i,%i,%i,%i", pos.x, pos.y, (int)pos.z, pos.map);
}


// Forward Declarations
static PyObject* wpCoord_getAttr( wpCoord* self, char* name );
static int wpCoord_setAttr( wpCoord* self, char* name, PyObject* value );
static int wpCoord_compare(PyObject *a, PyObject *b);

/*!
	The typedef for Wolfpack Python items
*/
PyTypeObject wpCoordType = {
	PyObject_HEAD_INIT( NULL )
	0,
	"WPCoord",
	sizeof( wpCoordType ),
	0,
	wpDealloc,
	0,
	( getattrfunc ) wpCoord_getAttr,
	( setattrfunc ) wpCoord_setAttr,
	(cmpfunc)wpCoord_compare,
	0,
	0,
	0,
	0,
	0,
	0, // Call
	(reprfunc)wpCoord_str
};

static int wpCoord_compare(PyObject *a, PyObject *b) {
	// Both have to be coordinates
	if (a->ob_type != &wpCoordType || b->ob_type != &wpCoordType)
		return -1;

	const Coord_cl &posa = ((wpCoord*)a)->coord;
	const Coord_cl &posb = ((wpCoord*)b)->coord;
	
	if (posa.x != posb.x || posa.y != posb.y || posa.z != posb.z || posa.map != posb.map) {
		return -1;
	} else {
		return 0;
	}
}

/*
	\method coord.distance
	\param pos A <object id="coord">coord</object> object.
	\return An integer value.
	\description This method measures the distance between this coordinate
	and the given <object id="coord">coord</object> coord object. 
	The return value is -1 if the distance is infinite, otherwise the distance
	in tiles is returned.
*/
static PyObject* wpCoord_distance( wpCoord* self, PyObject* args )
{
	// Check if the paramter is a coordinate
	if ( !checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
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

/*
	\method coord.direction
	\param pos A <object id="coord">coord</object> object.
	\return An integer value.
	\description This method calculates the direction from this coordinate to the
	given one. If no direction can be determined, -1 is returned.
*/
static PyObject* wpCoord_direction( wpCoord* self, PyObject* args )
{
	// Check if the paramter is a coordinate
	if ( !checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
	{
		return PyInt_FromLong( -1 );
	}
	else
	{
		Coord_cl pos = getWpCoord( PyTuple_GetItem( args, 0 ) );

		// Calculate the distance
		return PyInt_FromLong( self->coord.direction( pos ) );
	}
}

/*
	\method coord.validspawnspot
	\return A boolean value.
	\description This method returns true if this coordinate is a valid spawn spot for a monster, character or item.
	Otherwise it returns false.
*/
static PyObject* wpCoord_validspawnspot( wpCoord* self, PyObject* args )
{
	Q_UNUSED( args );
	return Movement::instance()->canLandMonsterMoveHere( self->coord ) ? PyTrue() : PyFalse();
}

/*
	\method coord.lineofsight
	\param coord Another <object id="coord">coord</object> object.
	\param targetheight The height of the line of sight target.
	Can be 0.
	\param touch A boolean value. Defaults to false.
	\description Returns true if an object at the given coordinate with the given height can be seen from this coordinate.
	Touch determines whether the target needs to be touched or only be seen.
*/
static PyObject* wpCoord_lineofsight( wpCoord* self, PyObject* args )
{
	Coord_cl pos;
	ushort targetheight;
	char touch = 0;
	if ( !PyArg_ParseTuple( args, "O&i|b:coord.lineofsight(coord, targetheight, [touch=0])", &PyConvertCoord, &pos, &targetheight, &touch ) )
		return 0;
	if ( self->coord.lineOfSight( pos, targetheight, touch ) )
		return Py_True;
    return Py_False;
}

static PyMethodDef wpCoordMethods[] =
{
	{ "distance",	( getattrofunc ) wpCoord_distance, METH_VARARGS, "Whats the distance between Point A and Point B" },
	{ "direction", ( getattrofunc ) wpCoord_direction, METH_VARARGS, NULL },
	{ "validspawnspot",	( getattrofunc ) wpCoord_validspawnspot, METH_VARARGS, NULL },
	{ "lineofsight", ( getattrofunc ) wpCoord_lineofsight, METH_VARARGS, NULL },
	{ 0, 0, 0, 0 }
};

static PyObject* wpCoord_getAttr( wpCoord* self, char* name )
{
	/*
		\property coord.x This is the x component of the coordinate.
	*/
	if ( !strcmp( name, "x" ) )
		return PyInt_FromLong( self->coord.x );
	/*
		\property coord.y This is the y component of the coordinate.
	*/
	else if ( !strcmp( name, "y" ) )
		return PyInt_FromLong( self->coord.y );
	/*
		\property coord.z This is the z component of the coordinate.
	*/
	else if ( !strcmp( name, "z" ) )
		return PyInt_FromLong( self->coord.z );
	/*
		\property coord.map This is the map this coordinate is on.
	*/
	else if ( !strcmp( name, "map" ) )
		return PyInt_FromLong( self->coord.map );
	else
		return Py_FindMethod( wpCoordMethods, ( PyObject * ) self, name );
}

static int wpCoord_setAttr( wpCoord* self, char* name, PyObject* value )
{
	// I only have integer params in mind
	if ( !PyInt_Check( value ) )
		return 1;

	if ( !strcmp( name, "x" ) )
		self->coord.x = PyInt_AsLong( value );
	else if ( !strcmp( name, "y" ) )
		self->coord.y = PyInt_AsLong( value );
	else if ( !strcmp( name, "z" ) )
		self->coord.z = PyInt_AsLong( value );
	else if ( !strcmp( name, "map" ) )
		self->coord.map = PyInt_AsLong( value );

	return 0;
}

int PyConvertCoord( PyObject* object, Coord_cl* pos )
{
	if ( object->ob_type != &wpCoordType )
	{
		PyErr_BadArgument();
		return 0;
	}

	*pos = ( ( wpCoord * ) object )->coord;
	return 1;
}

bool checkWpCoord( PyObject* object )
{
	return ( object->ob_type == &wpCoordType );
}

PyObject* PyGetCoordObject( const Coord_cl& coord )
{
	wpCoord* cObject = PyObject_New( wpCoord, &wpCoordType );
	cObject->coord = coord;
	return ( PyObject * ) ( cObject );
}

Coord_cl getWpCoord( PyObject* object )
{
	Coord_cl coord;

	if ( object->ob_type == &wpCoordType )
		coord = ( ( wpCoord * ) object )->coord;

	return coord;
}
