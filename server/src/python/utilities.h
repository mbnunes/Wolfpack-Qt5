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

#if !defined( __UTILITIES_H__ )
#define __UTILITIES_H__

#include "engine.h"
#include "pyerrors.h"

#include "../typedefs.h"

class cUOSocket;
class cItem;
class cBaseChar;
class Coord_cl;
class cAccount;
class cTerritory;
class cUOTxTooltipList;
class cMulti;
class cNPC_AI;

typedef cItem* P_ITEM;
typedef cMulti* P_MULTI;

/*!
	Things commonly used in other python-definition
	source-files.
*/
#define PyFalse PyInt_FromLong( 0 )
#define PyTrue PyInt_FromLong( 1 )
#define PyHasMethod(a) if( codeModule == NULL ) return false; if( !PyObject_HasAttrString( codeModule, a ) ) return false;

// Setting and getting item/char properties
#define setIntProperty( identifier, property ) if( !strcmp( name, identifier ) ) self->property = PyInt_AS_LONG( value );
#define setStrProperty( identifier, property ) if( !strcmp( name, identifier ) ) self->property = PyString_AS_STRING( value );
#define getIntProperty( identifier, property ) if( !strcmp( name, identifier ) ) return PyInt_FromLong( self->property );
#define getStrProperty( identifier, property ) if( !strcmp( name, identifier ) ) return PyString_FromString( self->property );

/*!
	This function checks if there has been an error and tries to print it out.
  */
inline void PyReportError( void )
{
	if( PyErr_Occurred() )
		PyErr_Print();
}

inline void wpDealloc( PyObject* self )
{
    PyObject_Del( self );
}

PyObject *PyGetTooltipObject( cUOTxTooltipList* );

bool checkWpSocket( PyObject *object );
PyObject *PyGetSocketObject( cUOSocket* );
cUOSocket *getWpSocket( PyObject* object );
int PyConvertSocket( PyObject *object, cUOSocket** sock );

bool checkWpCoord( PyObject *object );
int PyConvertCoord( PyObject *object, Coord_cl* pos );
PyObject *PyGetCoordObject( const Coord_cl& coord );
Coord_cl getWpCoord( PyObject* object );

bool checkWpItem( PyObject *object );
PyObject* PyGetItemObject( P_ITEM );
P_ITEM getWpItem( PyObject* );

bool checkWpChar( PyObject *object );
PyObject* PyGetCharObject( P_CHAR );
P_CHAR getWpChar( PyObject* );

bool checkWpAccount( PyObject *object );
PyObject* PyGetAccountObject( cAccount* );
cAccount* getWpAccount( PyObject* );

bool checkWpRegion( PyObject *object );
PyObject* PyGetRegionObject( cTerritory* );
cTerritory* getWpRegion( PyObject* );

bool checkWpMulti( PyObject *object );
PyObject* PyGetMultiObject( P_MULTI );
P_MULTI getWpMulti( PyObject* );

bool checkWpAI( PyObject *object );
PyObject* PyGetAIObject( cNPC_AI* );
cNPC_AI* getWpAI( PyObject* );

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
#define checkArgStr( id ) ( PyTuple_Size( args ) > id && PyString_Check( PyTuple_GetItem( args, id ) ) )
#define getArgStr( id ) PyString_AsString( PyTuple_GetItem( args, id ) )
#define checkArgAccount( id ) ( PyTuple_Size( args ) > id && checkWpAccount( PyTuple_GetItem( args, id ) ) )
#define checkArgRegion( id ) ( PyTuple_Size( args ) > id && checkWpRegion( PyTuple_GetItem( args, id ) ) )
#define getArgRegion( id ) getWpRegion( PyTuple_GetItem( args, id ) )
#define getArgAccount( id ) getWpAccount( PyTuple_GetItem( args, id ) )
#define checkArgMulti( id ) ( PyTuple_Size( args ) > id && checkWpMulti( PyTuple_GetItem( args, id ) ) )
#define getArgMulti( id ) getWpMulti( PyTuple_GetItem( args, id ) )

#endif
