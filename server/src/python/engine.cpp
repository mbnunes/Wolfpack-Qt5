//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#include "engine.h"

#include "../srvparams.h"
#include "../console.h"
#include "../globals.h"
#include "../log.h"

// Library includes
#include <qdom.h>
#include <qstring.h>
#include <qfile.h>
#include <qwaitcondition.h>

// Forward declaration for wolfpack extension function
void init_wolfpack_globals();

/*!
	Stops the python interpreter
*/
void stopPython( void )
{
	// Give the Python Threads time to finalize
	Py_BEGIN_ALLOW_THREADS
		QWaitCondition waitCondition;
		waitCondition.wait(100);
	Py_END_ALLOW_THREADS
	Py_Finalize();
}

/*!
	Starts the python interpreter
*/
void startPython(int argc, char* argv[]) {
	Py_SetProgramName( argv[ 0 ] );

	Py_NoSiteFlag = 1; // No import because we need to set the search path first

	Py_Initialize();
	PySys_SetArgv( argc, argv );

	// Modify our search-path
	PyObject *searchpath = PySys_GetObject( "path" );
	
	QStringList elements = QStringList::split( ";", SrvParams->getString( "General", "Python Searchpath", "./scripts;.", true ) );
	
	// Prepend our items to the searchpath
	for (int i = elements.count() - 1; i >= 0; --i) {
		PyList_Insert( searchpath, 0, PyString_FromString( elements[i].latin1() ) );
	}
	
	// Import site now
	PyObject *m = PyImport_ImportModule("site");
	Py_XDECREF(m);
	
	// Try changing the stderr + stdout pointers
	PyObject *file = PyFile_FromString("python.log", "w");
	
	if (file) {
		Py_INCREF(file);
		PySys_SetObject("stderr", file);
		Py_INCREF(file);
		PySys_SetObject("stdout", file);
		Py_DECREF(file);
	}

	try {
		init_wolfpack_globals();
	} catch (...) {
		Console::instance()->send( "Failed to initialize the python extension modules\n" );
	}
}

/*!	
	Reloads Python interpreter and restarts loaded modules
*/
void reloadPython()
{
	PyObject *sysModule = PyImport_ImportModule( "sys" );
	PyObject *modules = PyObject_GetAttrString( sysModule, "modules" );

	// This is a dictionary, so iterate trough it and reload all contained modules
	PyObject *mList = PyDict_Items( modules );

	for( INT32 i = 0; i < PyList_Size( mList ); ++i )
		PyImport_ReloadModule( PyList_GetItem( mList, i ) );
}

void reportPythonError( QString moduleName )
{
	// Print the Error
	if( PyErr_Occurred() )
	{
		PyObject *exception, *value, *traceback;

		PyErr_Fetch( &exception, &value, &traceback );
		PyErr_NormalizeException( &exception, &value, &traceback );

		// Set sys. variables for exception tracking
		PySys_SetObject( "last_type", exception );
		PySys_SetObject( "last_value", value );
		PySys_SetObject( "last_traceback", traceback );

		PyObject *exceptionName = PyObject_GetAttrString( exception, "__name__" );

		// Do we have a detailed description of the error ?
		PyObject *error = value != 0 ? PyObject_Str( value ) : 0;

		if( !error )
		{
			if( !moduleName.isNull() )
			{
				Console::instance()->log( LOG_ERROR, QString( "An error occured while compiling \"%1\": %2" ).arg( moduleName ).arg( PyString_AsString( exceptionName ) ) );
			}
			else
			{
				Console::instance()->log( LOG_ERROR, QString( "An error occured: %1" ).arg( PyString_AsString( exceptionName ) ) );
			}
		}
		else
		{
			if( !moduleName.isNull() )
			{
				Console::instance()->log( LOG_ERROR, QString( "An error occured in \"%1\": %2" ).arg( moduleName ).arg( PyString_AsString( exceptionName ) ) );
			}
			else
			{
				Console::instance()->log( LOG_ERROR, QString( "An error occured: %1" ).arg( PyString_AsString( exceptionName ) ) );
			}

			Console::instance()->log( LOG_PYTHON, QString( "%1: %2" ).arg( PyString_AsString( exceptionName ) ).arg( PyString_AsString( error ) ), false );
			Py_XDECREF( error );
		}

		// Don't print a traceback for syntax errors
		if( PyErr_GivenExceptionMatches( exception, PyExc_SyntaxError ) )
		{
			Py_XDECREF( traceback );
			traceback = 0;
		}

		// Dump a traceback
		while( traceback )
		{
			if( !PyObject_HasAttrString( traceback, "tb_frame" ) )
				break;

			PyObject *frame = PyObject_GetAttrString( traceback, "tb_frame" );

			if( !PyObject_HasAttrString( frame, "f_code" ) )
			{
				Py_XDECREF( frame );
				break;
			}

			PyObject *code = PyObject_GetAttrString( frame, "f_code" );

			if( !PyObject_HasAttrString( code, "co_filename" ) || !PyObject_HasAttrString( code, "co_name" ) )
			{
				Py_XDECREF( frame );
				Py_XDECREF( code );
				break;
			}

			PyObject *pyFilename = PyObject_GetAttrString( code, "co_filename" );
			PyObject *pyFunction = PyObject_GetAttrString( code, "co_name" );

			QString filename = PyString_AsString( pyFilename );
			QString function = PyString_AsString( pyFunction );

			Py_XDECREF( pyFilename );
			Py_XDECREF( pyFunction );

			Py_XDECREF( code );
			Py_XDECREF( frame );

			PyObject *pyLine = PyObject_GetAttrString( traceback, "tb_lineno" );
			unsigned int line = PyInt_AsLong( pyLine );
			Py_XDECREF( pyLine );

			// Print it 
			Console::instance()->log( LOG_PYTHON, QString( "File '%1',%2 in '%3'" ).arg( filename ).arg( line ).arg( function ), false );

			// Switch Frames
			PyObject *newtb = PyObject_GetAttrString( traceback, "tb_next" );
			Py_XDECREF( traceback );
			traceback = newtb;
		}

		Py_XDECREF( exceptionName );
		Py_XDECREF( exception );
		Py_XDECREF( value );
		Py_XDECREF( traceback );
	}
}
