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

#include "../network/uotxpackets.h"
#include "../network/uosocket.h"
#include "objectcache.h"
#include "../basechar.h"
#include "../player.h"

typedef struct
{
	PyObject_HEAD;
	cUOTxTooltipList* list;
} wpTooltip;

// Note: Must be of a different type to cause more then 1 template instanciation
class cTooltipCache : public cObjectCache<wpTooltip, 50>
{
} tooltipCache;

static void clearTooltipCache()
{
	tooltipCache.clear();
}

static CleanupAutoRegister reg( &clearTooltipCache );

// Forward Declarations
static PyObject* wpTooltip_getAttr( wpTooltip* self, char* name );
static int wpTooltip_setAttr( wpTooltip* self, char* name, PyObject* value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpTooltipType =
{
	PyObject_HEAD_INIT( NULL )
	0, "wpTooltip", sizeof( wpTooltipType ), 0, wpDealloc, 0, ( getattrfunc ) wpTooltip_getAttr, ( setattrfunc ) wpTooltip_setAttr, 
};

static PyObject* wpTooltip_add( wpTooltip* self, PyObject* args )
{
	char* params;
	unsigned int id;

	if ( !PyArg_ParseTuple( args, "Ies:tooltip.add(id, params)", &id, "utf-8", &params ) )
	{
		return 0;
	}

	self->list->addLine( id, QString::fromUtf8( params ) );
	PyMem_Free( params );
	return PyTrue();
}

static PyObject* wpTooltip_reset( wpTooltip* self, PyObject* args )
{
	self->list->resize( 19 );
	self->list->setShort( 1, 19 );
	return PyTrue();
}

static PyMethodDef wpTooltipMethods[] =
{
	{"reset", ( getattrofunc ) wpTooltip_reset, METH_VARARGS, "Resets the tooltip packet."}, {"add", ( getattrofunc ) wpTooltip_add, METH_VARARGS, "Adds new line to tooltip packet."}, { NULL, NULL, 0, NULL }
};

PyObject* PyGetTooltipObject( cUOTxTooltipList* tooltip )
{
	if ( !tooltip )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	wpTooltip* object = tooltipCache.allocObj( &wpTooltipType );
	object->list = tooltip;
	return ( PyObject * ) object;
}

static PyObject* wpTooltip_getAttr( wpTooltip* self, char* name )
{
	if ( !strcmp( name, "id" ) )
		return PyInt_FromLong( self->list->getInt( 11 ) );
	else if ( !strcmp( name, "serial" ) )
		return PyInt_FromLong( self->list->getInt( 5 ) );
	else
		return Py_FindMethod( wpTooltipMethods, ( PyObject * ) self, name );
}

static int wpTooltip_setAttr( wpTooltip* self, char* name, PyObject* value )
{
	if ( !PyInt_Check( value ) )
	{
		return 1;
	}

	if ( !strcmp( name, "id" ) )
	{
		self->list->setInt( 11, PyInt_AsLong( value ) );
	}
	else if ( !strcmp( name, "serial" ) )
	{
		self->list->setInt( 5, PyInt_AsLong( value ) );
	}

	return 0;
}

