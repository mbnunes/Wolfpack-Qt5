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

#include <limits.h>

#include "Python.h"
#include "utilities.h"
#include "../accounts.h"
#include "qvaluevector.h"

/*!
	The object for Wolfpack Coord items
*/
typedef struct {
    PyObject_HEAD;
	AccountRecord *account;
} wpAccount;

// Forward Declarations
PyObject *wpAccount_getAttr( wpAccount *self, char *name );
int wpAccount_setAttr( wpAccount *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpAccountType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpaccount",
    sizeof(wpAccountType),
    0,
    wpDealloc,				
    0,								
    (getattrfunc)wpAccount_getAttr,
    (setattrfunc)wpAccount_setAttr,
};

PyObject *wpAccount_getAttr( wpAccount *self, char *name )
{
	if( !strcmp( name, "acl" ) )
		return PyString_FromString( self->account->acl().latin1() );
	else if( !strcmp( name, "name" ) )
		return PyString_FromString( self->account->login().latin1() );
	else if( !strcmp( name, "password" ) )
		return PyString_FromString( self->account->password().latin1() );
	else if( !strcmp( name, "flags" ) )
		return PyInt_FromLong( self->account->flags() );
	else if( !strcmp( name, "characters" ) )
	{
		PyObject *list = PyList_New( 0 );
		QValueVector< cChar* > characters = self->account->caracterList();
		for( int i = 0; i < characters.size(); ++i )
			PyList_Append( list, PyGetCharObject( characters[i] ) );
		return list;
	}

	return 0;
}

int wpAccount_setAttr( wpAccount *self, char *name, PyObject *value )
{
/*	// I only have integer params in mind
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
*/
	return 0;
}

bool checkWpAccount( PyObject *object )
{
	if( !object )
		return false;
	return ( object->ob_type == &wpAccountType );
}

PyObject* PyGetAccountObject( AccountRecord *account )
{
	if( !account )
		return Py_None;

	wpAccount *cObject = PyObject_New( wpAccount, &wpAccountType );
	cObject->account = account;
    return (PyObject*)( cObject );	
}

AccountRecord* getWpAccount( PyObject *wpaccount )
{
	if( !wpaccount )
		return 0;

	if( !checkWpAccount( wpaccount ) )
		return 0;

	wpAccount *cObject = (wpAccount*)wpaccount;
	return cObject->account;
}
