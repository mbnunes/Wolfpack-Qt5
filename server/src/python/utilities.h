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

#if !defined( __UTILITIES_H__ )
#define __UTILITIES_H__

#include "engine.h"
#include "pyerrors.h"
#include <qstring.h>

#include "../typedefs.h"

class cUOSocket;
class cItem;
class cBaseChar;
class Coord_cl;
class cAccount;
class cTerritory;
class cUOTxTooltipList;
class cMulti;
class AbstractAI;

typedef cItem* P_ITEM;
typedef cMulti* P_MULTI;

/*!
	Things commonly used in other python-definition
	source-files.
*/
inline PyObject* PyFalse()
{
	Py_INCREF( Py_False );
	return Py_False;
}

inline PyObject* PyTrue()
{
	Py_INCREF( Py_True );
	return Py_True;
}

#define PyHasMethod(a) if( codeModule == NULL ) return false; if( !PyObject_HasAttrString( codeModule, a ) ) return false;

// Setting and getting item/char properties
#define setIntProperty( identifier, property ) if( !strcmp( name, identifier ) ) self->property = PyInt_AS_LONG( value );
#define setStrProperty( identifier, property ) if( !strcmp( name, identifier ) ) self->property = PyString_AS_STRING( value );
#define getIntProperty( identifier, property ) if( !strcmp( name, identifier ) ) return PyInt_FromLong( self->property );
#define getStrProperty( identifier, property ) if( !strcmp( name, identifier ) ) return PyString_FromString( self->property );

void wpDealloc( PyObject* self );

PyObject* PyGetTooltipObject( cUOTxTooltipList* );

int PyConvertObject( PyObject* object, cUObject** uobject );

bool checkWpSocket( PyObject* object );
PyObject* PyGetSocketObject( cUOSocket* );
cUOSocket* getWpSocket( PyObject* object );
int PyConvertSocket( PyObject* object, cUOSocket** sock );

bool checkWpCoord( PyObject* object );
int PyConvertCoord( PyObject* object, Coord_cl* pos );
PyObject* PyGetCoordObject( const Coord_cl& coord );
Coord_cl getWpCoord( PyObject* object );

bool checkWpItem( PyObject* object );
PyObject* PyGetItemObject( P_ITEM );
P_ITEM getWpItem( PyObject* );
int PyConvertItem( PyObject*, P_ITEM* item );

PyObject* PyGetObjectObject( cUObject* );

bool checkWpChar( PyObject* object );
PyObject* PyGetCharObject( P_CHAR );
P_CHAR getWpChar( PyObject* );
int PyConvertChar( PyObject* object, P_CHAR* character );
int PyConvertPlayer( PyObject* object, P_PLAYER* player );

bool checkWpAccount( PyObject* object );
PyObject* PyGetAccountObject( cAccount* );
cAccount* getWpAccount( PyObject* );

bool checkWpRegion( PyObject* object );
PyObject* PyGetRegionObject( cTerritory* );
cTerritory* getWpRegion( PyObject* );

bool checkWpMulti( PyObject* object );
PyObject* PyGetMultiObject( P_MULTI );
P_MULTI getWpMulti( PyObject* );

bool checkWpAI( PyObject* object );
PyObject* PyGetAIObject( AbstractAI* );
AbstractAI* getWpAI( PyObject* );

// Argument checks
#define checkArgObject( id ) ( PyTuple_Size( args ) > id && ( checkWpItem( PyTuple_GetItem( args, id ) ) || checkWpChar( PyTuple_GetItem( args, id ) ) ) )
#define checkArgChar( id ) ( PyTuple_Size( args ) > id && checkWpChar( PyTuple_GetItem( args, id ) ) )
#define checkArgItem( id ) ( PyTuple_Size( args ) > id && checkWpItem( PyTuple_GetItem( args, id ) ) )
#define checkArgCoord( id ) ( PyTuple_Size( args ) > id && checkWpCoord( PyTuple_GetItem( args, id ) ) )
#define getArgCoord( id ) getWpCoord( PyTuple_GetItem( args, id ) )
#define getArgItem( id ) getWpItem( PyTuple_GetItem( args, id ) )
#define getArgChar( id ) getWpChar( PyTuple_GetItem( args, id ) )
#define checkArgInt( id ) ( PyTuple_Size( args ) > id && PyInt_Check( PyTuple_GetItem( args, id ) ) )
#define getArgInt( id ) PyInt_AsLong( PyTuple_GetItem( args, id ) )
#define checkArgStr( id ) ( PyTuple_Size( args ) > id && ( PyString_Check( PyTuple_GetItem( args, id ) ) || PyUnicode_Check( PyTuple_GetItem( args, id ) ) ) )
#define checkArgUnicode( id ) ( PyTuple_Size( args ) > id && PyUnicode_Check( PyTuple_GetItem( args, id ) ) )
#define getArgStr( id ) (PyString_Check(PyTuple_GetItem(args,id)) ? QString(PyString_AsString(PyTuple_GetItem(args, id))) : QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(PyTuple_GetItem(args,id))))
#define getArgUnicode( id ) PyUnicode_AsUnicode( PyTuple_GetItem( args, id ) )
#define getUnicodeSize( id ) PyUnicode_GetSize( PyTuple_GetItem( args, id ) )
#define checkArgAccount( id ) ( PyTuple_Size( args ) > id && checkWpAccount( PyTuple_GetItem( args, id ) ) )
#define checkArgRegion( id ) ( PyTuple_Size( args ) > id && checkWpRegion( PyTuple_GetItem( args, id ) ) )
#define getArgRegion( id ) getWpRegion( PyTuple_GetItem( args, id ) )
#define getArgAccount( id ) getWpAccount( PyTuple_GetItem( args, id ) )
#define checkArgMulti( id ) ( PyTuple_Size( args ) > id && checkWpMulti( PyTuple_GetItem( args, id ) ) )
#define getArgMulti( id ) getWpMulti( PyTuple_GetItem( args, id ) )

inline PyObject* QString2Python( const QString& string )
{
	if ( string.isEmpty() )
	{
		return PyUnicode_FromWideChar( L"", 0 );
	}
	else
	{
		return PyUnicode_FromUnicode( ( Py_UNICODE * ) string.ucs2(), string.length() );
	}
}

inline QString Python2QString( PyObject* object )
{
	if ( PyUnicode_Check( object ) )
	{
		return QString::fromUcs2( ( ushort * ) PyUnicode_AS_UNICODE( object ) );
	}
	else if ( PyString_Check( object ) )
	{
		return QString::fromLocal8Bit( PyString_AsString( object ) );
	}
	else if ( PyInt_Check( object ) )
	{
		return QString::number( PyInt_AsLong( object ) );
	}
	else if ( PyFloat_Check( object ) )
	{
		return QString::number( PyFloat_AsDouble( object ) );
	}
	else
	{
		return QString::null;
	}
}

#endif
