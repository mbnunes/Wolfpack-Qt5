//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================
#include "Python.h"
#include "utilities.h"

#include "../network/uotxpackets.h"
#include "../network/uosocket.h"


typedef struct {
    PyObject_HEAD;
	cUOTxTooltipList *list;
} wpTooltip;

// Forward Declarations
PyObject *wpTooltip_getAttr( wpTooltip *self, char *name );
int wpTooltip_setAttr( wpTooltip *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpTooltipType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpTooltip",
    sizeof(wpTooltipType),
    0,
    wpDealloc,				
    0,								
    (getattrfunc)wpTooltip_getAttr,
    (setattrfunc)wpTooltip_setAttr,
};

PyObject *wpTooltip_add( wpTooltip *self, PyObject *args );
PyObject *wpTooltip_send( wpTooltip *self, PyObject *args );

static PyMethodDef wpTooltipMethods[] = 
{
	{ "add", (getattrofunc)wpTooltip_add, METH_VARARGS, "Adds new line to tooltip packet" },
  	{ "send", (getattrofunc)wpTooltip_send, METH_VARARGS, "Sends tooltip packet" },
    { NULL, NULL, 0, NULL }
};

PyObject *wpTooltip_send( wpTooltip *self, PyObject *args )
{
	if( !self->list )
		return false;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
	
	P_CHAR player = getArgChar( 0 );
	player->socket()->send( self->list );

    return PyTrue;
}
PyObject *wpTooltip_add( wpTooltip *self, PyObject *args )
{
	if( !self->list )
		return false;

	if( ( !checkArgInt( 0 ) ) || ( !checkArgStr( 1 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT32 listid = getArgInt( 0 );
	QString params = getArgStr( 1 );
	
	self->list->addLine( listid, params );

	return PyTrue;
}

PyObject* PyGetTooltipObject( cUOTxTooltipList *tooltip )
{
	if( !tooltip )
		return Py_None;

	wpTooltip *cObject = PyObject_New( wpTooltip, &wpTooltipType );
	cObject->list = tooltip;
    return (PyObject*)( cObject );	
}

PyObject *wpTooltip_getAttr( wpTooltip *self, char *name )
{
	if( !strcmp( name, "id" ) )
		return PyInt_FromLong( self->list->getInt( 11 ) );
	else if( !strcmp( name, "serial" ) )
		return PyInt_FromLong( self->list->getInt( 5 ) );
	else
		return Py_FindMethod( wpTooltipMethods, (PyObject*)self, name );
}

int wpTooltip_setAttr( wpTooltip *self, char *name, PyObject *value )
{
	if( !PyInt_Check( value ) )
		return 1;

	if( !strcmp( name, "id" ) )
		self->list->setInt( 11, PyInt_AsLong( value ) );
	else if( !strcmp( name, "serial" ) )
		self->list->setInt( 5, PyInt_AsLong( value ) );

	return 0;
}

