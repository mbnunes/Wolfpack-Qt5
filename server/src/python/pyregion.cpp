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

#include "Python.h"
#include "utilities.h"
#include "../territories.h"

/*!
	The object for Wolfpack Python items
*/
typedef struct {
    PyObject_HEAD;
	cTerritory *pRegion;
} wpRegion;

// Forward Declarations
PyObject *wpRegion_getAttr( wpRegion *self, char *name );
int wpRegion_setAttr( wpRegion *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpRegionType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPRegion",
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

PyObject *wpRegion_getAttr( wpRegion *self, char *name )
{
	return Py_FindMethod( wpRegionMethods, (PyObject*)self, name );
}

int wpRegion_setAttr( wpRegion *self, char *name, PyObject *value )
{
	return 0;
}

bool checkWpRegion( PyObject *object )
{
	return ( object->ob_type == &wpRegionType );
}

PyObject* PyGetRegionObject( cTerritory *pRegion )
{
	if( !pRegion )
		return Py_None;

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
