/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

// Wolfpack Includes
#include "dbdriver.h"
#include "python/utilities.h"

// Library Includes
#include <QtGlobal>

#include <QString>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

/*****************************************************************************
  cDBDriver member functions
 *****************************************************************************/

struct wpDbResult
{
	PyObject_HEAD;
	QSqlQuery result;
};

static PyObject* wpDbResult_getAttr( wpDbResult* self, char* name );

/*
	\object dbresult
	\description This object type represents the response from the database to a query.
*/
PyTypeObject wpDbResultType =
{
PyObject_HEAD_INIT( NULL )
0,
"dbresult",
sizeof( wpDbResultType ),
0,
0,
0,
( getattrfunc ) wpDbResult_getAttr,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/*
	\method dbresult.free
	\description This function frees the resources allocated by this dbresult object. Always call this
	when you no longer need this object.
*/
static PyObject* wpDbResult_free( wpDbResult* /*self*/, PyObject* args )
{
	Q_UNUSED( args );
	Py_RETURN_NONE;
}

/*
	\method dbresult.fetchrow
	\return A boolean value.
	\description Fetch a new row from the database and return false if the end of the result set was reached.
*/
static PyObject* wpDbResult_fetchrow( wpDbResult* self, PyObject* args )
{
	Q_UNUSED( args );
	bool result = self->result.next();

	if ( result )
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\method dbresult.getint
	\param position The position of the integer value you want to get.
	\return An integer value.
	\description Get an integer value from the row at the given position.
	Please be careful with this function. If you specify an invalid position
	it can lead to a crash of your server.
*/
static PyObject* wpDbResult_getint( wpDbResult* self, PyObject* args )
{
	unsigned int pos;
	if ( !PyArg_ParseTuple( args, "I:dbresult.getint(position)", &pos ) )
	{
		return 0;
	}
	return PyInt_FromLong( self->result.value( pos ).toInt() );
}

/*
	\method dbresult.getstring
	\param position The position of the string value you want to get.
	\return A string value.
	\description Get a string value from the row at the given position.
	Please be careful with this function. If you specify an invalid position
	it can lead to a crash of your server.
*/
static PyObject* wpDbResult_getstring( wpDbResult* self, PyObject* args )
{
	unsigned int pos;
	if ( !PyArg_ParseTuple( args, "I:dbresult.getstring(position)", &pos ) )
	{
		return 0;
	}

	QString value = self->result.value(pos).toString();

	return QString2Python( value );
}

static PyMethodDef wpDbResultMethods[] =
{
{ "free", ( getattrofunc ) wpDbResult_free, METH_VARARGS, 0 },
{ "fetchrow", ( getattrofunc ) wpDbResult_fetchrow, METH_VARARGS, 0 },
{ "getint", ( getattrofunc ) wpDbResult_getint, METH_VARARGS, 0 },
{ "getstring", ( getattrofunc ) wpDbResult_getstring, METH_VARARGS, 0 },
{ 0, 0, 0, 0 }
};

static PyObject* wpDbResult_getAttr( wpDbResult* self, char* name )
{
	return Py_FindMethod( wpDbResultMethods, ( PyObject * ) self, name );
}

PyObject* getPyObjectFromQSqlQuery( QSqlQuery& t )
{
	wpDbResult* returnVal = PyObject_New( wpDbResult, &wpDbResultType );
	returnVal->result = t;
	return ( PyObject * ) returnVal;
}
