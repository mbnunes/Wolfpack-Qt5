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
#include "../spawnregions.h"
#include "../uobject.h"

/*
\object region
\description This object type represents a region within the ultima online world.
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
		0,
};

/*
\method spawnregion.removeitem
\description 
\param serial
*/
static PyObject* wpSpawnRegion_removeitem( wpSpawnRegion* self, PyObject* args )
{
	SERIAL serial;
	if ( !PyArg_ParseTuple( args, "i:spawnregion.removeitem(serial)", &serial ) )
	{
		return 0;
	}

	QPtrList<cUObject> items = self->pRegion->spawnedItems(); // Copy
	cUObject *object;

	for (object = items.first(); object; object = items.next()) 
	{
		if ( object->serial() == serial )
			object->remove();
	}
	Py_RETURN_NONE;
}

/*
\method spawnregion.removenpc
\description 
\param serial
*/
static PyObject* wpSpawnRegion_removenpc( wpSpawnRegion* self, PyObject* args )
{
	SERIAL serial;
	if ( !PyArg_ParseTuple( args, "i:spawnregion.removenpc(serial)", &serial ) )
	{
		return 0;
	}

	QPtrList<cUObject> items = self->pRegion->spawnedNpcs(); // Copy
	cUObject *object;

	for (object = items.first(); object; object = items.next()) 
	{
		if ( object->serial() == serial )
			object->remove();
	}
	Py_RETURN_NONE;
}

static PyMethodDef wpSpawnRegionMethods[] =
{
	{ "removeitem",			( getattrofunc ) wpSpawnRegion_removeitem, METH_VARARGS, NULL },
	{ "removenpc",			( getattrofunc ) wpSpawnRegion_removenpc, METH_VARARGS, NULL },
	{ NULL, NULL, 0, NULL }
};

static PyObject* wpSpawnRegion_getAttr( wpSpawnRegion* self, char* name )
{
	/*
	\rproperty spawnregion.id
	*/
	if ( !strcmp( name, "id" ) )
	{
		return QString2Python( self->pRegion->name() );
	}
	/*
	\rproperty spawnregion.maxitemspawn
	*/
	else if ( !strcmp( name, "maxitemspawn" ) )
	{
		return PyInt_FromLong( self->pRegion->maxItems() );
	}
	/*
	\rproperty spawnregion.maxnpcspawn
	*/
	else if ( !strcmp( name, "maxnpcspawn" ) )
	{
		return PyInt_FromLong( self->pRegion->maxNpcs() );
	}
	/*
	\rproperty spawnregion.curnpcspawn
	*/
	else if ( !strcmp( name, "curnpcspawn" ) )
	{
		return PyInt_FromLong( self->pRegion->npcs() );
	}
	/*
	\rproperty spawnregion.curitemspawn
	*/
	else if ( !strcmp( name, "curitemspawn" ) )
	{
		return PyInt_FromLong( self->pRegion->items() );
	}
	/*
	\rproperty spawnregion.npcspawnlist
	*/
	else if ( !strcmp( name, "npcspawnlist" ) )
	{
		QStringList sections = self->pRegion->npcSections();
		PyObject* tuple = PyTuple_New( sections.size() );
		for ( uint i = 0; i < sections.size(); ++i )
		{
			PyTuple_SetItem( tuple, i, QString2Python( sections[i] ) );
		}
		return tuple;
	}
	/*
	\rproperty spawnregion.itemspawnlist
	*/
	else if ( !strcmp( name, "itemspawnlist" ) )
	{
		QStringList sections = self->pRegion->itemSections();
		PyObject* tuple = PyTuple_New( sections.size() );
		for ( uint i = 0; i < sections.size(); ++i )
		{
			PyTuple_SetItem( tuple, i, QString2Python( sections[i] ) );
		}
		return tuple;
	}
	/*
	\rproperty spawnregion.spawneditems
	*/
	else if ( !strcmp( name, "spawneditems" ) )
	{
		QPtrList<cUObject> objects = self->pRegion->spawnedItems();
		PyObject* list = PyList_New( objects.count() );
		cUObject *object;

		for (object = objects.first(); object; object = objects.next() )
			PyList_Append( list, PyInt_FromLong( object->serial() ) );
		return list;
	}
	/*
	\rproperty spawnregion.spawneditems
	*/
	else if ( !strcmp( name, "spawnednpcs" ) )
	{
		QPtrList<cUObject> objects = self->pRegion->spawnedNpcs();
		PyObject* list = PyList_New( objects.count() );
		cUObject *object;

		for (object = objects.first(); object; object = objects.next() )
			PyList_Append( list, PyInt_FromLong( object->serial() ) );
		return list;
	}

	return Py_FindMethod( wpSpawnRegionMethods, ( PyObject * ) self, name );
}

static int wpSpawnRegion_setAttr( wpSpawnRegion* self, char* name, PyObject* value )
{
	Q_UNUSED( self );
	Q_UNUSED( name );
	Q_UNUSED( value );
	// SpawnRegions have no changeable attributes yet
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
