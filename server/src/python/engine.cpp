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

#include "../wpconsole.h"
#include "../globals.h"

// Library includes
#include "qdom.h"
#include "qstring.h"
#include "qfile.h"
#include "Python.h"

// Forward declaration for wolfpack extension function
void init_wolfpack_globals();

/*!
	Stops the python interpreter
*/
void stopPython( void )
{
	Py_Finalize();
}

/*!
	Starts the python interpreter
*/
void startPython( int argc, char* argv[] )
{
	clConsole.PrepareProgress( "Starting Python interpreter" );
	
	Py_SetProgramName( argv[ 0 ] );
	Py_SetPythonHome( "python" ); // Subdirectory "python" is the base path

	Py_NoSiteFlag = 1;

	Py_Initialize(); // Initialize python finally
	PySys_SetArgv( argc, argv );

	// Modify our search-path
	PyObject *sysModule = PyImport_ImportModule( "sys" );
	PyObject *searchPath = PyObject_GetAttrString( sysModule, "path" );
	
	// Sorry but we can't use our DefManager for this
	QDomDocument Document( "python" );
	QFile File( "python.xml" );

    if ( !File.open( IO_ReadOnly ) )
	{
		clConsole.ProgressSkip();
	
		clConsole.send( "Unable to open python.xml!\n" );
		return;
	}

    if ( !Document.setContent( &File ) ) {
        File.close();
        
		clConsole.ProgressSkip();
		clConsole.send( "Unable to parse python.xml" );

		return;
	}

	File.close();

	QDomElement pythonSettings = Document.documentElement();
	QDomNodeList nodeList = pythonSettings.childNodes();

	for( UI08 i = 0; i < nodeList.count(); i++ )
	{
		if( !nodeList.item( i ).isElement() )
			continue;

		QDomElement element = nodeList.item( i ).toElement();
		
		if( element.nodeName() != "searchpath" )
			continue;

		PyList_Append( searchPath, PyString_FromString( element.text().ascii() ) );
	}
	
	// Import site now
	PyObject *m = PyImport_ImportModule("site");
	if( m == NULL )
	{
		clConsole.ProgressFail();
		if( PyErr_Occurred() )
			PyErr_Print();
		return;
	}
	else
	{
		Py_XDECREF( m );
    }
	
	try
	{
		
		init_wolfpack_globals(); // Init wolfpack extensions
	}
	catch( ... )
	{
		clConsole.ProgressFail();
		clConsole.send( "Failed to initialize the python extension modules\n" );
		return;
	}

	clConsole.ProgressDone();
}

void reloadPython()
{
	PyObject *sysModule = PyImport_ImportModule( "sys" );
	PyObject *modules = PyObject_GetAttrString( sysModule, "modules" );

	// This is a dictionary, so iterate trough it and reload all contained modules
	PyObject *mList = PyDict_Items( modules );

	for( INT32 i = 0; i < PyList_Size( mList ); ++i )
		PyImport_ReloadModule( PyList_GetItem( mList, i ) );
}
