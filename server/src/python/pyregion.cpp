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

#include "engine.h"
#include "utilities.h"
#include "../territories.h"

#include <vector>

/*!
	The object for Wolfpack Python items
*/
typedef struct {
    PyObject_HEAD;
	cTerritory *pRegion;
	bool frozen; // unused yet
	Coord_cl pos; // unused yet
} wpRegion;

// Forward Declarations
static PyObject *wpRegion_getAttr( wpRegion *self, char *name );
static int wpRegion_setAttr( wpRegion *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpRegionType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpregion",
    sizeof(wpRegionType),
    0,
    wpDealloc,
    0,
    (getattrfunc)wpRegion_getAttr,
    (setattrfunc)wpRegion_setAttr,
};

static PyMethodDef wpRegionMethods[] =
{
    { NULL, NULL, 0, NULL }
};

static PyObject *wpRegion_getAttr( wpRegion *self, char *name )
{
	if( !strcmp( name, "parent" ) )
	{
		// Check if valid region
		cTerritory *pRegion = dynamic_cast< cTerritory* >( self->pRegion->parent() );
		return PyGetRegionObject( pRegion );
	}
	else if( !strcmp( name, "children" ) )
	{
		QValueVector< cBaseRegion* > children = self->pRegion->children();
		PyObject *tuple = PyTuple_New( children.size() );
		for( uint i = 0; i < children.size(); ++i )
		{
			cTerritory *pRegion = dynamic_cast< cTerritory* >( children[i] );
			PyTuple_SetItem( tuple, i, PyGetRegionObject( pRegion ) );
		}
		return tuple;
	}
	// Return a Tuple of Tuples
	else if( !strcmp( name, "rectangles" ) )
	{
		QValueVector< cBaseRegion::rect_st > rectangles = self->pRegion->rectangles();
		PyObject *tuple = PyTuple_New( rectangles.size() );
		for( uint i = 0; i < rectangles.size(); ++i )
		{
			PyObject *subtuple = PyTuple_New( 4 );
			PyTuple_SetItem( subtuple, 0, PyInt_FromLong( rectangles[i].x1 ) );
			PyTuple_SetItem( subtuple, 1, PyInt_FromLong( rectangles[i].y1 ) );
			PyTuple_SetItem( subtuple, 2, PyInt_FromLong( rectangles[i].x2 ) );
			PyTuple_SetItem( subtuple, 3, PyInt_FromLong( rectangles[i].y2 ) );

			PyTuple_SetItem( tuple, i, subtuple );
		}
		return tuple;
	}
	else if( !strcmp( name, "name" ) )
		return QString2Python(self->pRegion->name());
	else if( !strcmp( name, "midilist" ) )
		return QString2Python(self->pRegion->midilist());
	else if( !strcmp( name, "guardowner" ) )
		return QString2Python(self->pRegion->guardOwner());
	else if( !strcmp( name, "rainchance" ) )
		return PyInt_FromLong( self->pRegion->rainChance() );
	else if( !strcmp( name, "snowchance" ) )
		return PyInt_FromLong( self->pRegion->snowChance() );

	// Flags
	else if( !strcmp( name, "guarded" ) )
		return PyInt_FromLong( self->pRegion->isGuarded() ? 1 : 0 );
	else if( !strcmp( name, "nomark" ) )
		return PyInt_FromLong( self->pRegion->isNoMark() ? 1 : 0 );
	else if( !strcmp( name, "nogate" ) )
		return PyInt_FromLong( self->pRegion->isNoGate() ? 1 : 0 );
	else if( !strcmp( name, "norecallout" ) )
		return PyInt_FromLong( self->pRegion->isNoRecallOut() ? 1 : 0 );
	else if( !strcmp( name, "norecallin" ) )
		return PyInt_FromLong( self->pRegion->isNoRecallIn() ? 1 : 0 );
	else if( !strcmp( name, "recallshield" ) )
		return PyInt_FromLong( self->pRegion->isRecallShield() ? 1 : 0 );
	else if( !strcmp( name, "noagressivemagic" ) )
		return PyInt_FromLong( self->pRegion->isNoAgressiveMagic() ? 1 : 0 );
	else if( !strcmp( name, "antimagic" ) )
		return PyInt_FromLong( self->pRegion->isAntiMagic() ? 1 : 0 );
	else if( !strcmp( name, "validescortregion" ) )
		return PyInt_FromLong( self->pRegion->isValidEscortRegion() ? 1 : 0 );
	else if( !strcmp( name, "cave" ) )
		return PyInt_FromLong( self->pRegion->isCave() ? 1 : 0 );
	else if( !strcmp( name, "nomusic" ) )
		return PyInt_FromLong( self->pRegion->isNoMusic() ? 1 : 0 );

	return Py_FindMethod( wpRegionMethods, (PyObject*)self, name );
}

static int wpRegion_setAttr( wpRegion *self, char *name, PyObject *value )
{
	Q_UNUSED(self);
	Q_UNUSED(name);
	Q_UNUSED(value);
	// Regions have no changeable attributes yet
	return 0;
}

bool checkWpRegion( PyObject *object )
{
	return ( object->ob_type == &wpRegionType );
}

PyObject* PyGetRegionObject( cTerritory *pRegion )
{
	if( !pRegion )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	wpRegion *returnVal = PyObject_New( wpRegion, &wpRegionType );
	returnVal->pRegion = pRegion;
	return (PyObject*)returnVal;
}

cTerritory* getWpRegion( PyObject *pObj )
{
	if( pObj->ob_type != &wpRegionType )
		return NULL;

	return ((wpRegion*)pObj)->pRegion;
}
