//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#if defined (__unix__)
#include <limits.h>  //compatability issue. GCC 2.96 doesn't have limits include
#else
#include <limits> // Python tries to redefine some of this stuff, so include first
#endif

#include "wppythonscript.h"
#include "../worldmain.h"
#include "../globals.h"
#include "../junk.h"
#include "../maps.h"
#include "../network.h"

// Library Includes
#include "qfile.h"

// Extension includes
#include "utilities.h"

/*
inline PyObject* PyGetTargetObject( cUOTxTarget *target )
{
	Py_WPTarget *returnVal = PyObject_New( Py_WPTarget, &Py_WPTargetType );
	returnVal->targetInfo = Target;
	return (PyObject*)returnVal;
}*/

void WPPythonScript::unload( void )
{
	if( codeModule == NULL )
		return;

	if( PyObject_HasAttr( codeModule, PyString_FromString( "onUnload" ) ) ) 
	{
		PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onUnload" ) ); 
		
		if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) 
			return; 
		
		PyObject_CallObject( method, NULL ); 
		PyReportError();
	}

	codeModule = PyImport_ReloadModule( codeModule );
}

// Find our module name
void WPPythonScript::load( const QDomElement &Data )
{
	// Initialize it
	codeModule = NULL;

	QString moduleName = Data.attribute( "module" );

	if( moduleName.isNull() || moduleName.isEmpty() )
		return;

	// Compile the codemodule
	char moduleNameStr[1024]; // Just to be sure
	strcpy( &moduleNameStr[ 0 ], moduleName.latin1() );

	codeModule = PyImport_ImportModule( moduleNameStr );

	if( codeModule == NULL )
	{
		clConsole.ProgressFail();

		if( PyErr_Occurred() )
			PyErr_Print();

		clConsole.send( QString( "\nError while compiling module [" + moduleName + "]\n" ) );
		clConsole.PrepareProgress( "Continuing loading" );
		return;
	}
}

//========================== OVERRIDDEN DEFAULT EVENTS
bool WPPythonScript::onUse( P_CHAR User, P_ITEM Used )
{
	PyHasMethod( "onUse" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( User ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Used ) );

	PyEvalMethod( "onUse" )
}

bool WPPythonScript::onSingleClick( P_ITEM Item, P_CHAR Viewer )
{
	PyHasMethod( "onSingleClick" )
	
	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetItemObject( Item ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );

	PyEvalMethod( "onSingleClick" )
}

bool WPPythonScript::onSingleClick( P_CHAR Character, P_CHAR Viewer )
{
	PyHasMethod( "onSingleClick" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );
	
	PyEvalMethod( "onSingleClick" )
}

bool WPPythonScript::onCollideItem( P_CHAR Character, P_ITEM Obstacle )
{
	PyHasMethod( "onCollideItem" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Obstacle ) );

	PyEvalMethod( "onCollideItem" )
}

bool WPPythonScript::onCollideChar( P_CHAR Character, P_CHAR Obstacle )
{
	PyHasMethod( "onCollideChar" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Obstacle ) );

	PyEvalMethod( "onCollideChar" )
}

bool WPPythonScript::onWalk( P_CHAR Character, UI08 Direction, UI08 Sequence )
{
	PyHasMethod( "onWalk" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Direction ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( Sequence ) );

	PyEvalMethod( "onWalk" )
}

// if this events returns true (handeled) then we should not display the text
bool WPPythonScript::onTalk( P_CHAR Character, char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang )
{
	PyHasMethod( "onTalk" )

	PyObject *tuple = PyTuple_New( 6 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( speechType ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( speechColor ) );
	PyTuple_SetItem( tuple, 3, PyInt_FromLong( speechFont ) );
	PyTuple_SetItem( tuple, 4, PyString_FromString( Text.ascii() ) );
	PyTuple_SetItem( tuple, 5, PyString_FromString( Lang.ascii() ) );
	
	PyEvalMethod( "onTalk" )
}


bool WPPythonScript::onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text )
{
	PyHasMethod( "onTalkToNPC" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Talker ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( Text.ascii() ) );

	PyEvalMethod( "onTalkToNPC" )
}

bool WPPythonScript::onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text )
{
	PyHasMethod( "onTalkToItem" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Talker ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Item ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( Text.ascii() ) );

	PyEvalMethod( "onTalkToItem" )
	PyReportError();
}


bool WPPythonScript::onWarModeToggle( P_CHAR Character, bool War )
{
	PyHasMethod( "onWarModeToggle" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, ( War ? PyInt_FromLong( 1 ) : PyInt_FromLong( 0 ) ) );

	PyEvalMethod( "onWarModeToggle" )
}

bool WPPythonScript::onHelp( P_CHAR Character )
{
	PyHasMethod( "onHelp" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyEvalMethod( "onHelp" )
}


bool WPPythonScript::onChat( P_CHAR Character )
{
	PyHasMethod( "onChat" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyEvalMethod( "onChat" )
}


bool WPPythonScript::onSkillUse( P_CHAR Character, UI08 Skill )
{
	PyHasMethod( "onSkillUse" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Skill ) );

	PyEvalMethod( "onSkillUse" )
}

bool WPPythonScript::onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id )
{
	PyHasMethod( "onContextEntry" )

	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	
	if( isItemSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyTuple_SetItem( tuple, 2, PyInt_FromLong( id ) );

	PyEvalMethod( "onContextEntry" )
}

bool WPPythonScript::onShowContextMenu( P_CHAR pChar, cUObject *pObject )
{
	PyHasMethod( "onShowContextMenu" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	
	if( isItemSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyEvalMethod( "onShowContextMenu" )
}

bool WPPythonScript::onBeginCast( P_CHAR pMage, UINT8 spell, UINT8 type )
{
	PyHasMethod( "onBeginCast" )
	
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( type ) );

	PyEvalMethod( "onBeginCast" )
}

bool WPPythonScript::onEndCast( P_CHAR pMage, UINT8 spell, UINT8 type )
{
	PyHasMethod( "onEndCast" )
	
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( type ) );

	PyEvalMethod( "onEndCast" )
}

bool WPPythonScript::onSpellTarget( P_CHAR pMage, UINT8 spell, UINT8 type, cUObject *pObject, const Coord_cl &pos, UINT16 model )
{
	PyHasMethod( "onSpellTarget" )
	
	PyObject *tuple = PyTuple_New( 6 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( spell ) );
	
	// Check what kind of data we have here
	if( pObject && isCharSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 3, PyGetCharObject( (P_CHAR)pObject ) );
	else if( pObject && isItemSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 3, PyGetItemObject( (P_ITEM)pObject ) );
	else if( !pObject )
		PyTuple_SetItem( tuple, 3, Py_None );

	PyTuple_SetItem( tuple, 4, PyGetCoordObject( pos ) );
	PyTuple_SetItem( tuple, 5, PyInt_FromLong( model ) );

	PyEvalMethod( "onSpellTarget" )
}

bool WPPythonScript::onSpellSuccess( P_CHAR pMage, UINT8 spell, UINT8 type, cUObject *pObject, const Coord_cl &pos, UINT16 model )
{
	PyHasMethod( "onSpellSuccess" )
	
	PyObject *tuple = PyTuple_New( 6 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( spell ) );
	
	// Check what kind of data we have here
	if( pObject && isCharSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 3, PyGetCharObject( (P_CHAR)pObject ) );
	else if( pObject && isItemSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 3, PyGetItemObject( (P_ITEM)pObject ) );
	else if( !pObject )
		PyTuple_SetItem( tuple, 3, Py_None );

	PyTuple_SetItem( tuple, 4, PyGetCoordObject( pos ) );
	PyTuple_SetItem( tuple, 5, PyInt_FromLong( model ) );

	PyEvalMethod( "onSpellSuccess" )
}

bool WPPythonScript::onSpellFailure( P_CHAR pMage, UINT8 spell, UINT8 type, cUObject *pObject, const Coord_cl &pos, UINT16 model )
{
	PyHasMethod( "onSpellFailure" )
	
	PyObject *tuple = PyTuple_New( 6 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( spell ) );
	
	// Check what kind of data we have here
	if( pObject && isCharSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 3, PyGetCharObject( (P_CHAR)pObject ) );
	else if( pObject && isItemSerial( pObject->serial ) )
		PyTuple_SetItem( tuple, 3, PyGetItemObject( (P_ITEM)pObject ) );
	else if( !pObject )
		PyTuple_SetItem( tuple, 3, Py_None );

	PyTuple_SetItem( tuple, 4, PyGetCoordObject( pos ) );
	PyTuple_SetItem( tuple, 5, PyInt_FromLong( model ) );

	PyEvalMethod( "onSpellFailure" )
}

//============ wolfpack.server
/*PyObject* PyWPServer_shutdown( PyObject* self, PyObject* args )
{
	keeprun = 0;

	return PyTrue;
}

PyObject* PyWPServer_save( PyObject* self, PyObject* args )
{
	cwmWorldState->savenewworld( "binary" );

	return PyTrue;
}

PyObject* PyWPItems_findbyserial( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return Py_None;

	PyObject *pySerial = PyTuple_GetItem( args, 0 );

	if( pySerial == NULL )
		return Py_None;

	SERIAL serial;

	// Integer Serial
	if( PyInt_Check( pySerial ) )
		serial = PyInt_AsLong( pySerial );
	// String serial
	else if( PyString_Check( pySerial ) )
	{
		QString qSerial( PyString_AS_STRING( pySerial ) );
		serial = qSerial.toUInt( 0, 16 );
	}
	// Invalid Serial
	else
		return Py_None;

	P_ITEM rItem = FindItemBySerial( serial );

	if( rItem == NULL )
		return Py_None;

	return PyGetItemObject( rItem );
}

//============ wolfpack.map.gettile
PyObject* PyWPMap_gettile( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) != 2 )
		return Py_None;

	Coord_cl mapCoord;
	mapCoord.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	mapCoord.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	
	map_st mapTile = Map->SeekMap( mapCoord );

	return PyInt_FromLong( mapTile.id );
}

//============ wolfpack.map.getheight
PyObject* PyWPMap_getheight( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) != 2 )
		return Py_None;

	Coord_cl mapCoord;
	mapCoord.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	mapCoord.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	
	map_st mapTile = Map->SeekMap( mapCoord );

	return PyInt_FromLong( mapTile.z );
}

//============ wolfpack.chars
PyObject* PyWPChars_findbyserial( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return Py_None;

	PyObject *pySerial = PyTuple_GetItem( args, 0 );

	if( pySerial == NULL )
		return Py_None;

	SERIAL serial;

	// Integer Serial
	if( PyInt_Check( pySerial ) )
		serial = PyInt_AsLong( pySerial );
	// String serial
	else if( PyString_Check( pySerial ) )
	{
		QString qSerial( PyString_AS_STRING( pySerial ) );
		serial = qSerial.toUInt( 0, 16 );
	}
	// Invalid Serial
	else
		return Py_None;

	P_CHAR rChar = FindCharBySerial( serial );

	if( rChar == NULL )
		return Py_None;

	return PyGetCharObject( rChar );
}

//============ wolfpack.clients
PyObject* PyWP_clients( PyObject* self, PyObject* args )
{
	// Construct a clients object
	Py_WPClients *clients = PyObject_New( Py_WPClients, &Py_WPClientsType );

	if( clients == NULL )
		return Py_None;

	return (PyObject*)clients;
}

// ============================ class: char

PyObject* Py_WPChar_lineofsight( Py_WPChar* self, PyObject* args )
{
	// For one object check if it's a char or an item
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	Coord_cl source = self->Char->pos;
	Coord_cl destination;

	// Line of sight either to an item or to a char
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *object = PyTuple_GetItem( args, 0 );

		if( !strcmp( object->ob_type->tp_name, "char" ) )
			destination = ((Py_WPChar*)object)->Char->pos;
		else if( !strcmp( object->ob_type->tp_name, "item" ) )
			destination = ((Py_WPItem*)object)->Item->pos;
		else
			return PyFalse;
	}
	// To another location
	else if( PyTuple_Size( args ) == 3 )
	{
		destination.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		destination.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
		destination.z = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );
	}
	else
		return PyFalse;

	if( lineOfSight( source, destination, 1|2|4|8|16|32 ) )
		return PyTrue;
	else 
		return PyFalse;
}

PyObject* Py_WPChar_distance( Py_WPChar* self, PyObject* args )
{
	// For one object check if it's a char or an item
	if( PyTuple_Size( args ) < 1 )
		return PyInt_FromLong( -1 );

	Coord_cl source = self->Char->pos;
	Coord_cl destination;

	// Line of sight either to an item or to a char
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *object = PyTuple_GetItem( args, 0 );

		if( !strcmp( object->ob_type->tp_name, "char" ) )
			destination = ((Py_WPChar*)object)->Char->pos;
		else if( !strcmp( object->ob_type->tp_name, "item" ) )
			destination = ((Py_WPItem*)object)->Item->pos;
		else
			return PyInt_FromLong( -1 );
	}
	// To another location
	else if( PyTuple_Size( args ) == 3 )
	{
		destination.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		destination.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
		destination.z = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );
	}
	else
		return PyInt_FromLong( -1 );

	return PyInt_FromLong( destination.distance( source ) );
}

PyObject* Py_WPChar_requesttarget( Py_WPChar* self, PyObject* args )
{
	if( !self->Char->socket() )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	// Three arguments maximum
	// 1: callback function
	// 2: arguments for that function
	// 3: timeout
	PyObject *callbackName = PyTuple_GetItem( args, 0 );

	// Get the real thing
	QString fullName( PyString_AsString( callbackName ) );
	QStringList nameParts = QStringList::split( ".", fullName );

	if( nameParts.size() < 2 )
	{
		clConsole.send( "You have to pass a full qualified function name to requesttarget\n" );
		return PyFalse;
	}

	// All except the last are the module name
	QString functionName = nameParts.back();
	nameParts.pop_back();
	QString moduleName = nameParts.join( "." );

	PyObject *module = PyImport_Import( PyString_FromString( moduleName.ascii() ) );

	if( module == NULL )
	{
		clConsole.send( "Invalid module name %s", moduleName.ascii() );
		return PyFalse;
	}

	PyObject *callback = PyObject_GetAttr( module, PyString_FromString( functionName.ascii() ) );

	if( ( callback == NULL ) || ( !PyCallable_Check( callback ) ) )
	{
		clConsole.send( "Invalid function name %s", functionName.ascii() );
		return PyFalse;
	}

	PyObject *arguments;
	
	if( PyTuple_Size( args ) > 1 )
		arguments = PyTuple_GetItem( args, 1 );
	else
		arguments = PyTuple_New( 0 );

	cPythonTarget *target = new cPythonTarget( callback, arguments );

	if( ( PyTuple_Size( args ) == 3 ) && ( PyInt_Check( PyTuple_GetItem( args, 2 ) ) ) )
		target->setTimeout( PyInt_AS_LONG( PyTuple_GetItem( args, 2 ) ) );
	
	// Send target to client
	self->Char->socket()->attachTarget( target );

	return PyTrue;
}

//================= CLIENTS
int Py_WPClientsLength( Py_WPClients *self )
{
	return now;
}


PyObject *Py_WPClientsGet( Py_WPClients *self, int Num )
{
	if( Num > now )
		return Py_None;

	Py_WPClient *returnVal = PyObject_New( Py_WPClient, &Py_WPClientType );
	returnVal->Socket = Num;
	
	return (PyObject*)returnVal;
}

//================ CLIENT
PyObject *Py_WPClientGetAttr( Py_WPClient *self, char *name )
{
	if( !strcmp( name, "char" ) )
		return PyGetCharObject( currchar[ self->Socket ] );
	else
		return Py_FindMethod( Py_WPClientMethods, (PyObject*)self, name );
}

PyObject* Py_WPClient_disconnect( Py_WPClient* self, PyObject* args )
{
	//cNetwork::instance()->Disconnect( self->Socket );
	return PyTrue;
}

PyObject* Py_WPClient_send( Py_WPClient* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 2 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );
	int len = PyInt_AS_LONG( PyTuple_GetItem( args, 0 ) );

	if( pyMessage == Py_None )
		return PyFalse;

	char *Message = PyString_AS_STRING( pyMessage );

	Xsend( self->Socket, Message, len );

	// send the buffer
	//cNetwork::instance()->xSend( self->Socket, buffer, buffLen, 0 );

	return PyTrue;
}

//================ CONTENT
int Py_WPContentLength( Py_WPContent *self )
{
	return contsp.getData( self->Item->serial ).size();
}

PyObject *Py_WPContentGet( Py_WPContent *self, int Num )
{
	vector< SERIAL > contItems = contsp.getData( self->Item->serial );

	if( ( Num < 0 ) || ( Num >= contItems.size() ) )
		return Py_None;

	return PyGetItemObject( FindItemBySerial( contItems[ Num ] ) );  
}

//================ EQUIPMENT 

// we have a FIXED amount !!
int Py_WPEquipmentLength( Py_WPEquipment *self )
{
	return MAXLAYERS; 
}

PyObject *Py_WPEquipmentGet( Py_WPEquipment *self, int Num )
{
	if( ( Num < 0 ) || ( Num > MAXLAYERS ) )
		return Py_None;

	vector< SERIAL > equipItems = contsp.getData( self->Char->serial );

	return PyGetItemObject( FindItemBySerial( equipItems[ Num ] ) );  	
}

//================= TARGETTING

PyObject *Py_WPTargetGetAttr( Py_WPTarget *self, char *name )
{
	// Return what we've targetted
	if( !strcmp( name, "target" ) )
	{
		if( self->targetInfo.Tserial != 0x00000000 )
		{
			if( self->targetInfo.Tserial >= 0x40000000 )
				return PyString_FromString( "item" );
			else
				return PyString_FromString( "char" );
		}
		else if( self->targetInfo.model == 0 )
			return PyString_FromString( "ground" );
		else
			return PyString_FromString( "static" );
	}
	// Return an item object
	else if( !strcmp( name, "item" ) )
	{
		if( ( self->targetInfo.Tserial == 0x00000000 ) || ( self->targetInfo.Tserial < 0x40000000 ) )
			return Py_None;  
		else 
			return PyGetItemObject( FindItemBySerial( self->targetInfo.Tserial ) ); 
	}
	// Return a char object
	else if( !strcmp( name, "char" ) )
	{
		if( ( self->targetInfo.Tserial == 0x00000000 ) || ( self->targetInfo.Tserial >= 0x40000000 ) )
			return Py_None;  
		else 
			return PyGetCharObject( FindCharBySerial( self->targetInfo.Tserial ) ); 
	}
	else if( !strcmp( name, "x" ) )
	{
		return PyInt_FromLong( self->targetInfo.TxLoc );
	}
	else if( !strcmp( name, "y" ) )
	{
		return PyInt_FromLong( self->targetInfo.TyLoc );
	}
	else if( !strcmp( name, "z" ) )
	{
		return PyInt_FromLong( self->targetInfo.TzLoc );
	}
	else if( !strcmp( name, "static" ) )
	{
		return PyInt_FromLong( self->targetInfo.model );
	}

	return Py_None;
}

cPythonTarget::cPythonTarget( PyObject *callback, PyObject *arguments )
{
	callback_ = callback;
	
	// INCREF for the arguments so they dont get garbage collected
	arguments_ = PyTuple_New( PyTuple_Size( arguments ) );

	for( UI08 i = 0; i < PyTuple_Size( arguments ); i++ )
	{
		PyObject *mObject = PyTuple_GetItem( arguments, i );
		Py_INCREF( mObject );
		PyTuple_SetItem( arguments_, i, mObject );
	}

	Py_INCREF( arguments_ );
}

// Call the codeobject
void cPythonTarget::responsed( UOXSOCKET socket, PKGx6C targetInfo )
{
	if( !PyCallable_Check( callback_ ) )
		return;

	// Set the target-information
	PyObject *arguments = PyTuple_New( PyTuple_Size( arguments_ ) + 1 );

	PyObject *target = PyGetTargetObject( targetInfo );
	Py_INCREF( target );
	PyTuple_SetItem( arguments, 0, target );

	UI32 i = 0;

	for( i = 1; i <= PyTuple_Size( arguments_ ); i++ )
	{
		PyTuple_SetItem( arguments, i, PyTuple_GetItem( arguments_, i-1 ) );
	}

	PyObject_CallObject( callback_, arguments );

	// DECREF as the arguments are no longer needed
	for( i = 0; i < PyTuple_Size( arguments ); i++ )
		Py_DECREF( PyTuple_GetItem( arguments, i ) );

	if( PyErr_Occurred() )
		PyErr_Print();
}

// Free our arguments
void cPythonTarget::timedout( UOXSOCKET socket )
{
	// DECREF as the arguments are no longer needed
	for( UI08 i = 0; i < PyTuple_Size( arguments_ ); i++ )
		Py_DECREF( PyTuple_GetItem( arguments_, i ) );
}*/

