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

// Boost.Python
#include <boost/python.hpp>

#include "engine.h"
#include "utilities.h"

#include "../serverconfig.h"
#include "../console.h"
#include "../log.h"

// Library includes
#include <QString>
#include <QFile>
#include <QWaitCondition>
#include <QCoreApplication>
#include <QList>

using namespace boost::python;

// Python Functions
QList<PythonFunction*> PythonFunction::instances;

class cCleanupHandlers
{
private:
	QList<fnCleanupHandler> cleanupHandler;

public:
	void call()
	{
		QList<fnCleanupHandler>::iterator it;
		for ( it = cleanupHandler.begin(); it != cleanupHandler.end(); ++it )
		{
			( *it ) ();
		}
	}

	void reg( fnCleanupHandler handler )
	{
		cleanupHandler.append( handler );
	}
};

typedef Singleton<cCleanupHandlers> CleanupHandlers;

void registerCleanupHandler( fnCleanupHandler handler )
{
	CleanupHandlers::instance()->reg( handler );
}

CleanupAutoRegister::CleanupAutoRegister( fnCleanupHandler handler )
{
	registerCleanupHandler( handler );
}

// Forward declaration for wolfpack extension function
void init_wolfpack_globals();


/*!
	Stops the python interpreter
*/
static void stopPython()
{
	// Give the Python Threads time to finalize
	Py_BEGIN_ALLOW_THREADS
	QMutex m;
	m.lock();
	QWaitCondition waitCondition;
	waitCondition.wait( &m, 500 );
	Py_END_ALLOW_THREADS

	// We have to be sure that all memory
	// is freed here.
	// Call all cleanup handlers
	CleanupHandlers::instance()->call();

	Py_Finalize();
}

/*!
	Starts the python interpreter
*/
extern "C"  void init_wolfpack();
static void startPython( QStringList arguments )
{
	using namespace boost::python;
    Py_SetProgramName( arguments.at(0).toLocal8Bit().data());

	Py_NoSiteFlag = 1; // No import because we need to set the search path first

	PyImport_AppendInittab( "_wolfpack", init_wolfpack );
	Py_Initialize();
#if PY_VERSION_HEX > 0x02040000
	if ( !PyEval_ThreadsInitialized() )
#endif
		PyEval_InitThreads();

    char** argv = new char*[arguments.size()];

    for(int i=0;i<arguments.size();i++)
    {
        argv[i] = arguments[i].toLocal8Bit().data();
    }

    PySys_SetArgv( arguments.size(), argv);

	// Modify our search-path
	list searchPath = extract<list>( object( handle<>( borrowed( PySys_GetObject( "path" ) ) ) ) );

	QStringList elements = Config::instance()->getString( "General", "Python Searchpath", "./scripts;.", true ).split( ";" );

	// Prepend our items to the searchpath
	for ( int i = elements.count() - 1; i >= 0; --i )
	{
		searchPath.insert( 0, str( elements[i].toLatin1() ) );
	}

	// Import site now
	object siteModule = extract<object>( PyImport_ImportModule( "site" ) );

	// Try changing the stderr + stdout pointers
	PyObject* file = PyFile_FromString( "python.log", "w" );

	if ( file )
	{
		Py_INCREF( file );
		PySys_SetObject( "stderr", file );
		Py_INCREF( file );
		PySys_SetObject( "stdout", file );
		Py_DECREF( file );
	}

/*	try
	{
		init_wolfpack_globals();
	}
	catch ( ... )
	{
		Console::instance()->send( QString( "Failed to initialize the python extension modules\n" ) );
	}
*/
}

/*!
	Reloads Python interpreter and restarts loaded modules
*/


void reportPythonError( const QString& moduleName )
{
	using namespace boost::python;
	// Print the Error
	if ( PyErr_Occurred() )
	{
		PyObject* exception,* value,* traceback;

		PyErr_Fetch( &exception, &value, &traceback );
		PyErr_NormalizeException( &exception, &value, &traceback );

		// Set sys. variables for exception tracking
		PySys_SetObject( "last_type", exception );
		PySys_SetObject( "last_value", value );
		PySys_SetObject( "last_traceback", traceback );

		object exceptionName( ( handle<>( PyObject_GetAttrString( exception, "__name__" ) ) ) );

		// Do we have a detailed description of the error ?
		PyObject* error = value != 0 ? PyObject_Str( value ) : 0;

		if ( !error )
		{
			if ( !moduleName.isNull() )
			{
				Console::instance()->log( LOG_ERROR, tr( "An error occured while compiling \"%1\": %2" ).arg( moduleName ).arg( extract<QString>( exceptionName ) ) );
			}
			else
			{
				Console::instance()->log( LOG_ERROR, tr( "An error occured: %1" ).arg( extract<QString>( exceptionName ) ) );
			}
		}
		else
		{
			if ( !moduleName.isNull() )
			{
				Console::instance()->log( LOG_ERROR, tr( "An error occured in \"%1\": %2" ).arg( moduleName ).arg( extract<QString>( exceptionName ) ) );
			}
			else
			{
				Console::instance()->log( LOG_ERROR, tr( "An error occured: %1" ).arg( extract<QString>( exceptionName ) ) );
			}

			Console::instance()->log( LOG_PYTHON, QString( "%1: %2" ).arg( extract<QString>( exceptionName ) ).arg( PyString_AsString( error ) ), false );
			Py_XDECREF( error );
		}

		// Don't print a traceback for syntax errors
		if ( PyErr_GivenExceptionMatches( exception, PyExc_SyntaxError ) )
		{
			Py_XDECREF( traceback );
			traceback = 0;
		}

		// Dump a traceback
		while ( traceback )
		{
			if ( !PyObject_HasAttrString( traceback, "tb_frame" ) )
				break;

			PyObject* frame = PyObject_GetAttrString( traceback, "tb_frame" );

			if ( !PyObject_HasAttrString( frame, "f_code" ) )
			{
				Py_XDECREF( frame );
				break;
			}

			PyObject* code = PyObject_GetAttrString( frame, "f_code" );

			if ( !PyObject_HasAttrString( code, "co_filename" ) || !PyObject_HasAttrString( code, "co_name" ) )
			{
				Py_XDECREF( frame );
				Py_XDECREF( code );
				break;
			}

			PyObject* pyFilename = PyObject_GetAttrString( code, "co_filename" );
			PyObject* pyFunction = PyObject_GetAttrString( code, "co_name" );

			QString filename = PyString_AsString( pyFilename );
			QString function = PyString_AsString( pyFunction );

			Py_XDECREF( pyFilename );
			Py_XDECREF( pyFunction );

			Py_XDECREF( code );
			Py_XDECREF( frame );

			PyObject* pyLine = PyObject_GetAttrString( traceback, "tb_lineno" );
			unsigned int line = PyInt_AsLong( pyLine );
			Py_XDECREF( pyLine );

			// Print it
			Console::instance()->log( LOG_PYTHON, tr( "File '%1',%2 in '%3'" ).arg( filename ).arg( line ).arg( function ), false );

			// Switch Frames
			PyObject* newtb = PyObject_GetAttrString( traceback, "tb_next" );
			Py_XDECREF( traceback );
			traceback = newtb;
		}

		Py_XDECREF( exception );
		Py_XDECREF( value );
		Py_XDECREF( traceback );
	}
}

void wpDealloc( PyObject* self )
{
	PyObject_Del( self );
}

void registerConverters(); // from converters.cpp

cPythonEngine::cPythonEngine()
{
	registerConverters();
}

cPythonEngine::~cPythonEngine()
{
}

void cPythonEngine::load()
{
    startPython( QCoreApplication::arguments() );
	cComponent::load();
}

void cPythonEngine::unload()
{
	stopPython();
	cComponent::unload();
}

QString cPythonEngine::version()
{
	return Py_GetVersion();
}
