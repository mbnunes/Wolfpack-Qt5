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
#include "../serverconfig.h"

#include "../player.h"

/*
	\object account
	\description This object type represents a user account.
*/
typedef struct
{
	PyObject_HEAD;
	cAccount* account;
} wpAccount;

static PyObject *wpAccount_str(wpAccount *self) {
	return QString2Python(self->account->login());
}

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
	0,
	0,
	0,
	0,
	0,
	0,
	0, // Call
	(reprfunc)wpAccount_str
};

/*
	\method account.delete
	\description Removes this account and all characters attached to it.
*/
static PyObject* wpAccount_delete( wpAccount* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( self->account == 0 )
		Py_RETURN_FALSE;

	QValueVector<P_PLAYER> chars = self->account->caracterList();
	for ( uint i = 0; i < chars.size(); ++i )
		chars[i]->remove();

	self->account->remove();
	self->account = 0;

	Py_RETURN_TRUE;
}

/*
	\method account.block
	\description Blocks this account.
*/
static PyObject* wpAccount_block( wpAccount* self, PyObject* args )
{
	Q_UNUSED( args );

	self->account->setBlocked( true );
	Py_RETURN_TRUE;
}

/*
	\method account.unblock
	\description Unblock this account.
*/
static PyObject* wpAccount_unblock( wpAccount* self, PyObject* args )
{
	Q_UNUSED( args );
	self->account->setBlocked( false );
	Py_RETURN_TRUE;
}

/*
	\method account.addcharacter
	\param player The player that should be added to this account.
	\description Add a player character to this account.
*/
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

	Py_RETURN_TRUE;
}

/*
	\method account.removecharacter
	\param player The player that should be removed from this account.
	\description Removes a player from this account.
*/
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

	Py_RETURN_TRUE;
}

/*
	\method account.authorized
	\param group The ACL group that should be checked for.
	\param action The action in the given ACL group that should be checked.
	\return True if the account may perform the given action. False otherwise.
	\description Checks if the account may perform a given action based on its ACL.
*/
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
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\method account.checkpassword
	\param password The given password.
	\description Checks if the password of the account matches. Automatically takes MD5 hashing into account.
	\return True if the password is correct, false otherwise.
*/
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
	/*
		\rproperty account.name The name of this account.
	*/
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
	/*
		\rproperty account.characters A list of <object id="CHAR">char</object> objects. This list contains
		all characters assigned to this account.
	*/
	else if ( !strcmp( name, "characters" ) )
	{
		PyObject* list = PyList_New( 0 );
		QValueVector<P_PLAYER> characters = self->account->caracterList();
		for ( uint i = 0; i < characters.size(); ++i )
			PyList_Append( list, PyGetCharObject( characters[i] ) );
		return list;
	}
	/*
		\rproperty account.lastlogin The last login date of this account or
		"Unknown" if it's unknown.
	*/
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
	/*
		\rproperty account.inuse Indicates whether this account is currently in use.
	*/
	else if ( !strcmp( name, "inuse" ) )
	{
		if ( self->account->inUse() )
			Py_RETURN_TRUE;
		else
			Py_RETURN_FALSE;
	}
	/*
		\rproperty account.rank Returns the integer rank of this account. This is inherited by the ACL of
		this account.
	*/
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
	/*
		\property account.acl The name of the ACL used to check the permissions of this account.
	*/
	if ( !strcmp( name, "acl" ) && PyString_Check( value ) )
		self->account->setAcl( PyString_AsString( value ) );
	/*
		\property account.multigems Indicates whether Multis should be sent as Worldgems to this account.
	*/
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
	/*
		\property account.password The password of this account. Please note that if MD5 hashing is activated,
		this property will only return the hashed password. But when setting this property you don't need to
		specify the MD5 hashed password as it will be automatically converted.
	*/
	else if ( !strcmp( name, "password" ) && PyString_Check( value ) )
		self->account->setPassword( PyString_AsString( value ) );
	/*
		\property account.flags This property provides direct access to the flags of this account. Possible flags
		are:
		<code>0x00000001 blocked
		0x00000002 allmove
		0x00000004 allshow
		0x00000008 showserials
		0x00000010 pagenotify
		0x00000020 staff - gm mode on/off
		0x00000040 multigems on/off</code>
	*/
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
		Py_RETURN_NONE;
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
