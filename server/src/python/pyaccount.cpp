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

#include <limits.h>

#include "Python.h"
#include "utilities.h"
#include "../accounts.h"
#include "../chars.h"
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

PyObject *wpAccount_delete( wpAccount *self, PyObject *args )
{
	Q_UNUSED(args);
	if( self->account == 0 )
		return PyFalse;

	QValueVector< cChar* > chars = self->account->caracterList();
	for( uint i = 0; i < chars.size(); ++i )
		cCharStuff::DeleteChar( chars[i] );

	self->account->remove();
	self->account = 0;

	return PyTrue;
}

PyObject *wpAccount_block( wpAccount *self, PyObject *args )
{
	Q_UNUSED(args);
	if( self->account == 0 )
		return PyFalse;

	self->account->setBlocked( true );
	return PyTrue;
}

PyObject *wpAccount_unblock( wpAccount *self, PyObject *args )
{
	Q_UNUSED(args);
	if( self->account == 0 )
		return PyFalse;

	self->account->setBlocked( false );
	return PyTrue;
}

PyObject *wpAccount_addcharacter( wpAccount *self, PyObject *args )
{
	if( !self->account )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_CHAR pChar = getArgChar( 0 );

	if( pChar )
	{
		pChar->setAccount( self->account );
	}

	return PyTrue;
}

PyObject *wpAccount_removecharacter( wpAccount *self, PyObject *args )
{
	if( !self->account )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_CHAR pChar = getArgChar( 0 );

	if( pChar )
	{
		pChar->setAccount( 0 );
	}

	return PyTrue;
}

PyObject *wpAccount_authorized( wpAccount *self, PyObject *args )
{
	if( !self->account )
		return PyFalse;

	if( !checkArgStr( 0 ) || !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QString group = getArgStr( 0 );
	QString action = getArgStr( 1 );

	if( self->account->authorized( group, action ) )
		return PyTrue;
	else
		return PyFalse;
}

static PyMethodDef wpAccountMethods[] = 
{
	{ "authorized", (getattrofunc)wpAccount_authorized, METH_VARARGS, "Checks if the account is authorized to perform a given action." },
	{ "delete", (getattrofunc)wpAccount_delete, METH_VARARGS, "Delete this account." },
	{ "block", (getattrofunc)wpAccount_block, METH_VARARGS, "Shortcut for blocking the account." },
	{ "unblock", (getattrofunc)wpAccount_unblock, METH_VARARGS, "Shortcut for unblocking the account." },
	{ "addcharacter", (getattrofunc)wpAccount_addcharacter, METH_VARARGS, "Adds a character to this account." },
	{ "removecharacter", (getattrofunc)wpAccount_removecharacter, METH_VARARGS, "Removes a character from this account." },
    { NULL, NULL, 0, NULL }
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
		for( uint i = 0; i < characters.size(); ++i )
			PyList_Append( list, PyGetCharObject( characters[i] ) );
		return list;
	}
	else if( !strcmp( name, "lastlogin" ) )
		return PyString_FromString( self->account->lastLogin().toString().latin1() );
	else
		return Py_FindMethod( wpAccountMethods, (PyObject*)self, name );
}

int wpAccount_setAttr( wpAccount *self, char *name, PyObject *value )
{
	if( !strcmp( name, "acl" ) && PyString_Check( value ) )
		self->account->setAcl( PyString_AsString( value ) );
	else if( !strcmp( name, "password" ) && PyString_Check( value ) )
		self->account->setPassword( PyString_AsString( value ) );
	else if( !strcmp( name, "flags" ) && PyInt_Check( value ) )
		self->account->setFlags( PyInt_AsLong( value ) );

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
