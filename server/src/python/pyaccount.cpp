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

#include <qstring.h>
#include <qvaluevector.h>
#include <limits.h>

#include "engine.h"
#include "utilities.h"
#include "../accounts.h"
#include "../basechar.h"
#include "../md5.h"
#include "../config.h"

#include "../player.h"

/*!
	The object for Wolfpack cAccount items
*/
typedef struct
{
	PyObject_HEAD;
	cAccount* account;
} wpAccount;

// Forward Declarations
static PyObject* wpAccount_getAttr( wpAccount* self, char* name );
static int wpAccount_setAttr( wpAccount* self, char* name, PyObject* value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpAccountType =
{
	PyObject_HEAD_INIT( &wpAccountType )
	0,
	"wpaccount",
	sizeof( wpAccountType ),
	0,
	wpDealloc,
	0,
	( getattrfunc ) wpAccount_getAttr,
	( setattrfunc ) wpAccount_setAttr,

};

/*!
	Removes this account
*/
static PyObject* wpAccount_delete( wpAccount* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( self->account == 0 )
		return PyFalse();

	QValueVector<P_PLAYER> chars = self->account->caracterList();
	for ( uint i = 0; i < chars.size(); ++i )
		chars[i]->remove();

	self->account->remove();
	self->account = 0;

	return PyTrue();
}

/*!
	Blocks the account from logging into the system
*/
static PyObject* wpAccount_block( wpAccount* self, PyObject* args )
{
	Q_UNUSED( args );

	self->account->setBlocked( true );
	return PyTrue();
}

/*!
	unBlocks the account from logging into the system
*/
static PyObject* wpAccount_unblock( wpAccount* self, PyObject* args )
{
	Q_UNUSED( args );
	self->account->setBlocked( false );
	return PyTrue();
}

static PyObject* wpAccount_addcharacter( wpAccount* self, PyObject* args )
{
	if ( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_PLAYER pChar = dynamic_cast<P_PLAYER>( getArgChar( 0 ) );

	if ( pChar )
	{
		pChar->setAccount( self->account );
	}

	return PyTrue();
}

static PyObject* wpAccount_removecharacter( wpAccount* self, PyObject* args )
{
	if ( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_PLAYER pChar = dynamic_cast<P_PLAYER>( getArgChar( 0 ) );

	if ( pChar )
	{
		pChar->setAccount( 0 );
	}

	return PyTrue();
}

static PyObject* wpAccount_authorized( wpAccount* self, PyObject* args )
{
	if ( !checkArgStr( 0 ) || !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QCString group = getArgStr( 0 ).latin1();
	QCString action = getArgStr( 1 ).latin1();

	if ( self->account->authorized( group, action ) )
		return PyTrue();
	else
		return PyFalse();
}

static PyObject* wpAccount_checkpassword( wpAccount* self, PyObject* args )
{
	char* password;

	if ( !PyArg_ParseTuple( args, "es:account.checkpassword(password)", "utf-8", &password ) )
	{
		return 0;
	}

	bool authorized;

	if ( Config::instance()->hashAccountPasswords() )
	{
		authorized = cMd5::fastDigest( password ) == self->account->password();
	}
	else
	{
		authorized = password == self->account->password();
	}

	PyMem_Free( password );

	return authorized ? PyTrue() : PyFalse();
}

static PyMethodDef wpAccountMethods[] =
{
	{ "authorized", ( getattrofunc ) wpAccount_authorized, METH_VARARGS, "Checks if the account is authorized to perform a given action." },
	{ "delete", ( getattrofunc ) wpAccount_delete, METH_VARARGS, "Delete this account." },
	{ "block", ( getattrofunc ) wpAccount_block, METH_VARARGS, "Shortcut for blocking the account." },
	{ "unblock", ( getattrofunc ) wpAccount_unblock, METH_VARARGS, "Shortcut for unblocking the account." },
	{ "addcharacter", ( getattrofunc ) wpAccount_addcharacter, METH_VARARGS, "Adds a character to this account." },
	{ "removecharacter", ( getattrofunc ) wpAccount_removecharacter, METH_VARARGS, "Removes a character from this account." },
	{ "checkpassword", ( getattrofunc ) wpAccount_checkpassword, METH_VARARGS, "Checks the password and regards md5 hashes." },
	{ NULL, NULL, 0, NULL }
};

static PyObject* wpAccount_getAttr( wpAccount* self, char* name )
{
	if ( !self->account )
	{
		return 0;
	}

	if ( !strcmp( name, "acl" ) )
		return PyString_FromString( self->account->acl().latin1() );
	else if ( !strcmp( name, "name" ) )
		return PyString_FromString( self->account->login().latin1() );
	else if ( !strcmp( name, "multigems" ) )
	{
		if ( self->account->isMultiGems() )
		{
			Py_INCREF( Py_True );
			return Py_True;
		}
		else
		{
			Py_INCREF( Py_False );
			return Py_False;
		}
	}
	else if ( !strcmp( name, "password" ) )
		return PyString_FromString( self->account->password().latin1() );
	else if ( !strcmp( name, "flags" ) )
		return PyInt_FromLong( self->account->flags() );
	else if ( !strcmp( name, "characters" ) )
	{
		PyObject* list = PyList_New( 0 );
		QValueVector<P_PLAYER> characters = self->account->caracterList();
		for ( uint i = 0; i < characters.size(); ++i )
			PyList_Append( list, PyGetCharObject( characters[i] ) );
		return list;
	}
	else if ( !strcmp( name, "lastlogin" ) )
	{
		if ( !self->account->lastLogin().isValid() )
			return PyString_FromString( "Unknown" );
		else
			return PyString_FromString( self->account->lastLogin().toString().latin1() );
	}
	else if ( !strcmp( name, "blockuntil" ) )
	{
		if ( self->account->blockedUntil() > QDateTime::currentDateTime() )
			return PyString_FromString( self->account->blockedUntil().toString().latin1() );
		else
			return PyString_FromString( "" );
	}
	else if ( !strcmp( name, "inuse" ) )
	{
		if ( self->account->inUse() )
			return PyTrue();
		else
			return PyFalse();
	}
	else if ( !strcmp( name, "rank" ) )
	{
		return PyInt_FromLong( self->account->rank() );
	}
	else
	{
		return Py_FindMethod( wpAccountMethods, ( PyObject * ) self, name );
	}
}

static int wpAccount_setAttr( wpAccount* self, char* name, PyObject* value )
{
	if ( !strcmp( name, "acl" ) && PyString_Check( value ) )
		self->account->setAcl( PyString_AsString( value ) );
	else if ( !strcmp( name, "multigems" ) )
	{
		if ( PyObject_IsTrue( value ) )
		{
			self->account->setMultiGems( true );
		}
		else
		{
			self->account->setMultiGems( false );
		}
	}
	else if ( !strcmp( name, "password" ) && PyString_Check( value ) )
		self->account->setPassword( PyString_AsString( value ) );
	else if ( !strcmp( name, "flags" ) && PyInt_Check( value ) )
		self->account->setFlags( PyInt_AsLong( value ) );
	else if ( !strcmp( name, "blockuntil" ) && PyString_Check( value ) )
	{
		QDateTime datetime = QDateTime::fromString( PyString_AsString( value ), Qt::ISODate );
		if ( datetime > QDateTime::currentDateTime() )
		{
			self->account->setBlockUntil( datetime );
		}
	}

	return 0;
}

bool checkWpAccount( PyObject* object )
{
	if ( !object )
		return false;
	return ( object->ob_type == &wpAccountType );
}

PyObject* PyGetAccountObject( cAccount* account )
{
	if ( !account )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	wpAccount* cObject = PyObject_New( wpAccount, &wpAccountType );
	cObject->account = account;
	return ( PyObject * ) ( cObject );
}

cAccount* getWpAccount( PyObject* wpaccount )
{
	if ( !wpaccount )
		return 0;

	if ( !checkWpAccount( wpaccount ) )
		return 0;

	wpAccount* cObject = ( wpAccount* ) wpaccount;
	return cObject->account;
}
