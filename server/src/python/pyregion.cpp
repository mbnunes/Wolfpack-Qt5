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

/*
	\object region
	\description This object type represents a region within the ultima online world.
*/
typedef struct
{
	PyObject_HEAD;
	cTerritory* pRegion;
	bool frozen; // unused yet
	Coord_cl pos; // unused yet
} wpRegion;

// Forward Declarations
static PyObject* wpRegion_getAttr( wpRegion* self, char* name );
static int wpRegion_setAttr( wpRegion* self, char* name, PyObject* value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpRegionType =
{
	PyObject_HEAD_INIT( NULL )
	0,
	"wpregion",
	sizeof( wpRegionType ),
	0,
	wpDealloc,
	0,
	( getattrfunc ) wpRegion_getAttr,
	( setattrfunc ) wpRegion_setAttr,

};

static PyMethodDef wpRegionMethods[] =
{
	{ NULL, NULL, 0, NULL }
};

static PyObject* wpRegion_getAttr( wpRegion* self, char* name )
{
	/*
		\rproperty region.parent This property represents the parent region of this region. If there is no parent region this 
		property contains None, otherwise another region object for the parent region.
	*/
	if ( !strcmp( name, "parent" ) )
	{
		// Check if valid region
		cTerritory* pRegion = dynamic_cast<cTerritory*>( self->pRegion->parent() );
		return PyGetRegionObject( pRegion );
	}
	/*
		\rproperty region.children This property contains a tuple of region objects for the subregions within this region.
	*/
	else if ( !strcmp( name, "children" ) )
	{
		QValueVector<cBaseRegion*> children = self->pRegion->children();
		PyObject* tuple = PyTuple_New( children.size() );
		for ( uint i = 0; i < children.size(); ++i )
		{
			cTerritory* pRegion = dynamic_cast<cTerritory*>( children[i] );
			PyTuple_SetItem( tuple, i, PyGetRegionObject( pRegion ) );
		}
		return tuple;
	}
	/*
		\rproperty region.rectangles This property is a tuple of tuples containing x1, y1, x2 and y2 for the rectangles that define
		the area of this region.
	*/
	else if ( !strcmp( name, "rectangles" ) )
	{
		QValueVector<cBaseRegion::rect_st> rectangles = self->pRegion->rectangles();
		PyObject* tuple = PyTuple_New( rectangles.size() );
		for ( uint i = 0; i < rectangles.size(); ++i )
		{
			PyObject* subtuple = PyTuple_New( 4 );
			PyTuple_SetItem( subtuple, 0, PyInt_FromLong( rectangles[i].x1 ) );
			PyTuple_SetItem( subtuple, 1, PyInt_FromLong( rectangles[i].y1 ) );
			PyTuple_SetItem( subtuple, 2, PyInt_FromLong( rectangles[i].x2 ) );
			PyTuple_SetItem( subtuple, 3, PyInt_FromLong( rectangles[i].y2 ) );

			PyTuple_SetItem( tuple, i, subtuple );
		}
		return tuple;
	}
	/*
		\rproperty region.name The name of this region.
	*/
	else if ( !strcmp( name, "name" ) )
		return QString2Python( self->pRegion->name() );
	/*
		\rproperty region.midilist A list of midi sounds to be played for this region.
	*/
	else if ( !strcmp( name, "midilist" ) )
		return QString2Python( self->pRegion->midilist() );
	/*
		\rproperty region.guardowner The name of the guardowner for this region.
	*/
	else if ( !strcmp( name, "guardowner" ) )
		return QString2Python( self->pRegion->guardOwner() );

	// Flags
	/*
		\rproperty region.guarded This boolean flag indicates whether the region is guarded or not.
	*/
	else if ( !strcmp( name, "guarded" ) )
		return PyInt_FromLong( self->pRegion->isGuarded() ? 1 : 0 );
	/*
		\rproperty region.nomark This boolean flag indicates whether runes aren't markable in this region or not.
	*/
	else if ( !strcmp( name, "nomark" ) )
		return PyInt_FromLong( self->pRegion->isNoMark() ? 1 : 0 );
	/*
		\rproperty region.nogate This boolean flag indicates whether gates in or out of this region are allowed.
	*/
	else if ( !strcmp( name, "nogate" ) )
		return PyInt_FromLong( self->pRegion->isNoGate() ? 1 : 0 );
	/*
		\rproperty region.norecallout This boolean flag indicates whether recalling out of this region is allowed.
	*/
	else if ( !strcmp( name, "norecallout" ) )
		return PyInt_FromLong( self->pRegion->isNoRecallOut() ? 1 : 0 );
	/*
		\rproperty region.norecallin This boolean flag indicates whether recalling into this region is allowed.
	*/
	else if ( !strcmp( name, "norecallin" ) )
		return PyInt_FromLong( self->pRegion->isNoRecallIn() ? 1 : 0 );
	/*
		\rproperty region.noagressivemagic This boolean flag indicates whether agressive magic is forbidden in this region or not.
	*/
	else if ( !strcmp( name, "noagressivemagic" ) )
		return PyInt_FromLong( self->pRegion->isNoAgressiveMagic() ? 1 : 0 );
	/*
		\rproperty region.noagressivemagic This boolean flag indicates whether magic is forbidden in this region or not.
	*/
	else if ( !strcmp( name, "antimagic" ) )
		return PyInt_FromLong( self->pRegion->isAntiMagic() ? 1 : 0 );
	/*
		\rproperty region.cave This boolean flag indicates that this region is underground.
	*/
	else if ( !strcmp( name, "cave" ) )
		return PyInt_FromLong( self->pRegion->isCave() ? 1 : 0 );
	/*
		\rproperty region.nomusic This boolean flag indicates that no music should be played in this region.
	*/
	else if ( !strcmp( name, "nomusic" ) )
		return PyInt_FromLong( self->pRegion->isNoMusic() ? 1 : 0 );

	return Py_FindMethod( wpRegionMethods, ( PyObject * ) self, name );
}

static int wpRegion_setAttr( wpRegion* self, char* name, PyObject* value )
{
	Q_UNUSED( self );
	Q_UNUSED( name );
	Q_UNUSED( value );
	// Regions have no changeable attributes yet
	return 0;
}

bool checkWpRegion( PyObject* object )
{
	return ( object->ob_type == &wpRegionType );
}

PyObject* PyGetRegionObject( cTerritory* pRegion )
{
	if ( !pRegion )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	wpRegion* returnVal = PyObject_New( wpRegion, &wpRegionType );
	returnVal->pRegion = pRegion;
	return ( PyObject * ) returnVal;
}

cTerritory* getWpRegion( PyObject* pObj )
{
	if ( pObj->ob_type != &wpRegionType )
		return NULL;

	return ( ( wpRegion * ) pObj )->pRegion;
}
