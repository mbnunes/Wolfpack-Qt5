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
* Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
*/

#include "engine.h"
#include "utilities.h"
#include "../spawnregions.h"
#include "../uobject.h"

/*
\object spawnregion
\description This object type represents a spawnregion within the Ultima Online world.
*/
typedef struct
{
	PyObject_HEAD;
	cSpawnRegion* pRegion;
} wpSpawnRegion;

// Forward Declarations
static PyObject* wpSpawnRegion_getAttr( wpSpawnRegion* self, char* name );
static int wpSpawnRegion_setAttr( wpSpawnRegion* self, char* name, PyObject* value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpSpawnRegionType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpspawnregion",
sizeof( wpSpawnRegionType ),
0,
wpDealloc,
0,
( getattrfunc ) wpSpawnRegion_getAttr,
( setattrfunc ) wpSpawnRegion_setAttr,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/*
\method spawnregion.add
\param object
\description Add the given object to this spawnregion.
*/
static PyObject* wpSpawnRegion_add( wpSpawnRegion *self, PyObject *args) {
	cUObject *object;

	if (!PyArg_ParseTuple(args, "O&:spawnregion.add(obj)", &PyConvertObject, &object)) {
		return 0;
	}

	object->setSpawnregion(self->pRegion);
	Py_RETURN_NONE;
}

/*
\method spawnregion.remove
\param serial
\description Remove the given object from a spawnregion, freeing the slot for a new item
to be spawned.
*/
static PyObject* wpSpawnRegion_remove( wpSpawnRegion* self, PyObject* args )
{
	SERIAL serial;
	if ( !PyArg_ParseTuple( args, "i:spawnregion.remove(serial)", &serial ) )
	{
		return 0;
	}

	QPtrList<cUObject> objects;
	if ( isItemSerial( serial ) )
	{
		objects = self->pRegion->spawnedItems(); // Copy
	}
	else
	{
		objects = self->pRegion->spawnedNpcs(); // Copy
	}

	cUObject *object;
	for ( object = objects.first(); object; object = objects.next() )
	{
		if ( object->serial() == serial )
			object->remove();
	}
	Py_RETURN_NONE;
}

/*
\method spawnregion.respawn
\description Issue a spawn cycle of this spawnregion.
*/
static PyObject* wpSpawnRegion_respawn( wpSpawnRegion* self, PyObject* args )
{
	self->pRegion->reSpawn();
	Py_RETURN_NONE;
}

/*
\method spawnregion.clear
\description Clear this spawnregion.
*/
static PyObject* wpSpawnRegion_clear( wpSpawnRegion* self, PyObject* args )
{
	self->pRegion->deSpawn();
	Py_RETURN_NONE;
}

static PyMethodDef wpSpawnRegionMethods[] =
{
{ "add",				( getattrofunc ) wpSpawnRegion_add,	METH_VARARGS, NULL },
{ "remove",				( getattrofunc ) wpSpawnRegion_remove,	METH_VARARGS, NULL },
{ "respawn",			( getattrofunc ) wpSpawnRegion_respawn,	METH_VARARGS, NULL },
{ "clear",				( getattrofunc ) wpSpawnRegion_clear,	METH_VARARGS, NULL },
{ NULL, NULL, 0, NULL }
};

static PyObject* wpSpawnRegion_getAttr( wpSpawnRegion* self, char* name )
{
	/*
	\rproperty spawnregion.id
	*/
	if ( !strcmp( name, "id" ) )
	{
		return QString2Python( self->pRegion->id() );
	}
	/*
	\rproperty spawnregion.maxitems The maximum number of items spawned by this region.
	*/
	else if ( !strcmp( name, "maxitems" ) )
	{
		return PyInt_FromLong( self->pRegion->maxItems() );
	}
	/*
	\rproperty spawnregion.maxnpcs The maximum number of npcs spawned by this region.
	*/
	else if ( !strcmp( name, "maxnpcs" ) )
	{
		return PyInt_FromLong( self->pRegion->maxNpcs() );
	}
	/*
	\rproperty spawnregion.spawnednpcs The number of npcs currently spawned in this region.
	*/
	else if ( !strcmp( name, "spawnednpcs" ) )
	{
		return PyInt_FromLong( self->pRegion->npcs() );
	}
	/*
	\rproperty spawnregion.spawneditems The number of items currently spawned in this region.
	*/
	else if ( !strcmp( name, "spawneditems" ) )
	{
		return PyInt_FromLong( self->pRegion->items() );
	}
	/*
	\rproperty spawnregion.items A list of serials for the items currently spawned in this region.
	*/
	else if ( !strcmp( name, "items" ) )
	{
		QPtrList<cUObject> objects = self->pRegion->spawnedItems();
		PyObject* list = PyList_New( objects.count() );
		cUObject *object;
		int offset = 0;

		for ( object = objects.first(); object; object = objects.next() )
			PyList_SetItem( list, offset++, PyInt_FromLong( object->serial() ) );
		return list;
	}
	/*
	\rproperty spawnregion.npcs A list of serials for the npcs currently spawned in this region.
	*/
	else if ( !strcmp( name, "npcs" ) )
	{
		QPtrList<cUObject> objects = self->pRegion->spawnedNpcs();
		PyObject* list = PyList_New( objects.count() );
		cUObject *object;
		int offset = 0;

		for ( object = objects.first(); object; object = objects.next() )
			PyList_SetItem( list, offset++, PyInt_FromLong( object->serial() ) );
		return list;
	}
	/*
	\property spawnregion.active Indicates whether this spawnregion is currently active.
	*/
	else if (!strcmp( name, "active") ) {
		if (self->pRegion->active()) {
			Py_RETURN_TRUE;
		} else {
			Py_RETURN_FALSE;
		}
	}

	return Py_FindMethod( wpSpawnRegionMethods, ( PyObject * ) self, name );
}

static int wpSpawnRegion_setAttr( wpSpawnRegion* self, char* name, PyObject* value )
{
	Q_UNUSED( self );
	Q_UNUSED( name );
	Q_UNUSED( value );

	if (!strcmp(name, "active")) {
		if (PyObject_IsTrue(value)) {
			self->pRegion->setActive(true);
		} else {
			self->pRegion->setActive(false);
		}
	}

	return 0;
}

bool checkWpSpawnRegion( PyObject* object )
{
	return ( object->ob_type == &wpSpawnRegionType );
}

PyObject* PyGetSpawnRegionObject( cSpawnRegion* pRegion )
{
	if ( !pRegion )
	{
		Py_RETURN_NONE;
	}

	wpSpawnRegion* returnVal = PyObject_New( wpSpawnRegion, &wpSpawnRegionType );
	returnVal->pRegion = pRegion;
	return ( PyObject * ) returnVal;
}

cSpawnRegion* getWpSpawnRegion( PyObject* pObj )
{
	if ( pObj->ob_type != &wpSpawnRegionType )
		return NULL;

	return ( ( wpSpawnRegion * ) pObj )->pRegion;
}
