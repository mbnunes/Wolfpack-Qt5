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

#ifndef __GUMP_H__
#define __GUMP_H__

#include "utilities.h"
#include "../gumps.h"

/*!
	Response object. 
	This object is used to represent the users choice.
*/
typedef struct {
    PyObject_HEAD;
	gumpChoice_st *response;
} wpGumpResponse;

PyObject *wpGumpResponse_getAttr( wpGumpResponse *self, char *name )
{	
	if( !strcmp( name, "button" ) )
		return PyInt_FromLong( self->response->button );
	else if( !strcmp( name, "text" ) )
	{
		// Create a dictionary
		PyObject *dict = PyDict_New();

		std::map< unsigned short, QString >::iterator iter = self->response->textentries.begin();
		for( ; iter != self->response->textentries.end(); ++iter )
			PyDict_SetItem( dict, PyInt_FromLong( iter->first ), PyString_FromString( iter->second.latin1() ) );

		return dict;
	}
	else if( !strcmp( name, "switches" ) )
	{
		// Create a list
		PyObject *list = PyList_New( self->response->switches.size() );
		for( uint i = 0; i < self->response->switches.size(); ++i )
			PyList_SetItem( list, i, PyInt_FromLong( self->response->switches[ i ] ) );
		return list;
	}

	return PyFalse;
}

static PyTypeObject wpGumpResponseType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "wpGumpResponse",
    sizeof(wpGumpResponseType),
    0,
	wpDealloc,
    0,
    (getattrfunc)wpGumpResponse_getAttr,
    0,
    0,
    0,
    0,
    0,
};

PyObject *PyGetGumpResponse( gumpChoice_st &response )
{
	wpGumpResponse *returnVal = PyObject_New( wpGumpResponse, &wpGumpResponseType );
	returnVal->response = new gumpChoice_st;
	returnVal->response->button = response.button;
	returnVal->response->switches = response.switches;
	returnVal->response->textentries = response.textentries;

	return (PyObject*)returnVal;
}

/*!
	Internally used class for Python based Gumps
*/
class cPythonGump: public cGump
{
private:
	QString callback;
	PyObject *args;
public:
	cPythonGump( const QString &_callback, PyObject *_args ):
		callback( _callback ), args( _args )
	{
		// Increase ref-count for argument list
		tuple_incref( args );
	}
	
	void handleResponse( cUOSocket* socket, gumpChoice_st choice )
	{
		// Call the response function (and pass it a response-object)
		// Try to call the python function
		// Get everything before the last dot
		if( callback.contains( "." ) )
		{
			// Find the last dot
			INT32 position = callback.findRev( "." );
			QString sModule = callback.left( position );
			QString sFunction = callback.right( callback.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( socket->player() ) );
					PyTuple_SetItem( p_args, 1, args );
					PyTuple_SetItem( p_args, 2, PyGetGumpResponse( choice ) );

					PyEval_CallObject( pFunc, p_args );

					if( PyErr_Occurred() )
						PyErr_Print();
				}
			}
		}

		tuple_decref( args );
	}
};

#endif
