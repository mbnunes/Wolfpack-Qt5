/*#include "PyExtensions.h"
#include "wolfpack.h"

// TESTING FUNCTION
void WPy_Test( void )
{
	// Add us to the Python Search Path	
	PyObject *SysModule = PyImport_ImportModule( "sys" );
	PyObject *list = PyObject_GetAttrString( SysModule, "path" );
	PyList_Append( list, PyString_FromString( "D:\\Python21" ) );
	PyList_Append( list, PyString_FromString( "E:\\WolfPack\\Scripting" ) );

	PyObject *CodeModule = PyImport_ImportModule( "WolfPack.mytest" );

	if( CodeModule == NULL )
	{
		cout << "Error compiling the script" << endl;
		PyErr_Print();
		return;
	}

	//PyObject *Result = 
	PyObject_CallMethod( CodeModule, "onUse", NULL );
}

// This function intializes the Python extension functions  
// and exposes them to the engine
void WPy_InitExtensions( void )
{
	PyImport_AddModule( "Wolfpack" );
	Py_InitModule( "Wolfpack", WPy_WolfpackMethods);
}

// Displays a message on the console
PyObject *WPy_ConsoleMessage( PyObject *self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return Py_False;

	PyObject *Output = PyTuple_GetItem( args, 0 );

	if( Output == NULL )
		return Py_False;

	clConsole.send( PyString_AS_STRING( Output ) );

	return Py_None;
}*/