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

#if defined (Q_OS_UNIX)
#include <limits.h>  //compatability issue. GCC 2.96 doesn't have limits include
#else
#include <limits> // Python tries to redefine some of this stuff, so include first
#endif

#include "wppythonscript.h"
#include "../globals.h"
#include "../maps.h"
#include "../network.h"

// Library Includes
#include "qfile.h"

// Extension includes
#include "utilities.h"
#include "target.h"

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

	codeModule = 0;
}

// Find our module name
void WPPythonScript::load( const QDomElement &Data )
{
	// Initialize it
	codeModule = NULL;
	catchAllSpeech_ = false;

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

	// Call the load Function
	if( PyObject_HasAttr( codeModule, PyString_FromString( "onLoad" ) ) ) 
	{
		PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onLoad" ) ); 
		
		if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) 
			return; 
		
		PyObject_CallObject( method, NULL ); 
		PyReportError();
	}

	handleSpeech_ = PyObject_HasAttr( codeModule, PyString_FromString( "onSpeech" ) );
}

//========================== OVERRIDDEN DEFAULT EVENTS
bool WPPythonScript::onServerstart()
{
	PyHasMethod( "onServerstart" )

	PyObject *tuple = PyTuple_New( 0 );

	PyEvalMethod( "onServerstart" )
}

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

bool WPPythonScript::onLogout( P_CHAR Character )
{
	PyHasMethod( "onLogout" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyEvalMethod( "onLogout" )
}

bool WPPythonScript::onLogin( P_CHAR Character )
{
	PyHasMethod( "onLogin" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyEvalMethod( "onLogin" )
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
	
	if( isItemSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyTuple_SetItem( tuple, 2, PyInt_FromLong( id ) );

	PyEvalMethod( "onContextEntry" )
}

bool WPPythonScript::onShowContextMenu( P_CHAR pChar, cUObject *pObject )
{
	PyHasMethod( "onShowContextMenu" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	
	if( isItemSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyEvalMethod( "onShowContextMenu" )
}

bool WPPythonScript::onShowToolTip( P_CHAR pChar, cUObject *pObject, cUOTxTooltipList* tooltip )
{
	PyHasMethod( "onShowToolTip" )
	
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );

	if( isItemSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyTuple_SetItem( tuple, 2, PyGetTooltipObject( tooltip ) );

	PyEvalMethod( "onShowToolTip" )
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

bool WPPythonScript::onSpellCheckTarget( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget *target )
{
	PyHasMethod( "onSpellCheckTarget" )
	
	PyObject *tuple = PyTuple_New( 4 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( type ) );
	PyTuple_SetItem( tuple, 3, PyGetTarget( target, pMage->pos().map ) );

	PyEvalMethod( "onSpellCheckTarget" )
}

bool WPPythonScript::onSpellSuccess( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget *target )
{
	PyHasMethod( "onSpellSuccess" )
	
	PyObject *tuple = PyTuple_New( 4 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( type ) );
	PyTuple_SetItem( tuple, 3, PyGetTarget( target, pMage->pos().map ) );

	PyEvalMethod( "onSpellSuccess" )
}

bool WPPythonScript::onSpellFailure( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget *target )
{
	PyHasMethod( "onSpellFailure" )
	
	PyObject *tuple = PyTuple_New( 4 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pMage ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( type ) );
	PyTuple_SetItem( tuple, 3, PyGetTarget( target, pMage->pos().map ) );
	
	PyEvalMethod( "onSpellFailure" )
}

bool WPPythonScript::onCreate( cUObject *object, const QString &definition )
{
	PyHasMethod( "onCreate" )
	
	PyObject *tuple = PyTuple_New( 2 );

	if( object->isChar() )
		PyTuple_SetItem( tuple, 0, PyGetCharObject( (P_CHAR)object ) );
	else if( object->isItem() )
		PyTuple_SetItem( tuple, 0, PyGetItemObject( (P_ITEM)object ) );

	PyTuple_SetItem( tuple, 1, PyString_FromString( definition.latin1() ) );
	
	PyEvalMethod( "onCreate" )
}

bool WPPythonScript::onSpeech( cUObject *listener, P_CHAR talker, const QString &text, const QValueVector< UINT16 >& keywords )
{
	PyHasMethod( "onSpeech" )

	PyObject *tuple = PyTuple_New( 4 ); // Create our args for the python function
	if( isItemSerial( listener->serial() ) )
		PyTuple_SetItem( tuple, 0, PyGetItemObject( (P_ITEM)listener ) );
	else 
		PyTuple_SetItem( tuple, 0, PyGetCharObject( (P_CHAR)listener ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( talker ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( text.latin1() ) );
	
	// Convert the keywords into a list
	PyObject *list = PyList_New( 0 );
	for( QValueVector< UINT16 >::const_iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
		PyList_Append( list, PyInt_FromLong( *iter ) );


	PyTuple_SetItem( tuple, 3, list );

	PyEvalMethod( "onSpeech" )
}

bool WPPythonScript::onDropOnChar( P_CHAR pChar, P_ITEM pItem )
{
	PyHasMethod( "onDropOnChar" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );

	PyEvalMethod( "onDropOnChar" )
}

bool WPPythonScript::onDropOnItem( P_ITEM pCont, P_ITEM pItem )
{
	PyHasMethod( "onDropOnItem" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetItemObject( pCont ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );

	PyEvalMethod( "onDropOnItem" )
}

bool WPPythonScript::onDropOnGround( P_ITEM pItem, const Coord_cl &pos )
{
	PyHasMethod( "onDropOnGround" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetItemObject( pItem ) );
	PyTuple_SetItem( tuple, 1, PyGetCoordObject( pos ) );

	PyEvalMethod( "onDropOnGround" )
}

bool WPPythonScript::onPickup( P_CHAR pChar, P_ITEM pItem )
{
	PyHasMethod( "onPickup" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );

	PyEvalMethod( "onPickup" )
}

bool WPPythonScript::onCommand( cUOSocket *socket, const QString &name, const QString &args )
{
	PyHasMethod( "onCommand" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetSocketObject( socket ) );
	PyTuple_SetItem( tuple, 1, PyString_FromString( name.latin1() ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( args.latin1() ) );

	PyEvalMethod( "onCommand" )
}

bool WPPythonScript::onShowPaperdoll( P_CHAR pChar, P_CHAR pOrigin )
{
	PyHasMethod( "onShowPaperdoll" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( pOrigin ) );

	PyEvalMethod( "onShowPaperdoll" )
}

QString WPPythonScript::onShowPaperdollName( P_CHAR pChar, P_CHAR pOrigin )
{
	if( codeModule == NULL ) 
		return (char*)0; 
	if( !PyObject_HasAttr( codeModule, PyString_FromString( "onShowPaperdollName" ) ) ) 
		return (char*)0;

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( pOrigin ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onShowPaperdollName" ) ); 
	if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) 
		return (char*)0; 
	
	PyObject *returnValue = PyObject_CallObject( method, tuple ); 
	PyReportError(); 
	if( returnValue == NULL ) 
		return (char*)0; 
	if( !PyString_Check( returnValue ) ) 
		return (char*)0; 
	
	return PyString_AsString( returnValue );
}

