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

#include <qglobal.h>

#if defined (Q_OS_UNIX)
#include <limits.h>  //compatability issue. GCC 2.96 doesn't have limits include
#else
#include <limits> // Python tries to redefine some of this stuff, so include first
#endif

#include "pythonscript.h"
#include "globals.h"
#include "maps.h"
#include "network.h"
#include "wpconsole.h"

// Library Includes
#include <qfile.h>

// Extension includes
#include "python/utilities.h"
#include "python/target.h"

void cPythonScript::addKeyword( UINT16 data )
{
	speechKeywords_.push_back( data );
}

void cPythonScript::addWord( const QString &data )
{
	speechWords_.push_back( data );
}

void cPythonScript::addRegexp( const QRegExp &data )
{
	speechRegexp_.push_back( data );
}

bool cPythonScript::canHandleSpeech( const QString &text, const QValueVector< UINT16 >& keywords )
{
	// Check keywords first.
	for( QValueVector< UINT16 >::const_iterator iter1 = keywords.begin(); iter1 != keywords.end(); ++iter1 )
		for( QValueVector< UINT16 >::const_iterator iter2 = speechKeywords_.begin(); iter2 != speechKeywords_.end(); ++iter2 )
		{
			if( *iter1 == *iter2 )
				return true;
		}

	for( QValueVector< QString >::const_iterator iter3 = speechWords_.begin(); iter3 != speechWords_.end(); ++iter3 )
		if( text.contains( *iter3 ) )
			return true;

	for( QValueVector< QRegExp >::const_iterator iter4 = speechRegexp_.begin(); iter4 != speechRegexp_.end(); ++iter4 )
		if( text.contains( *iter4 ) )
			return true;

	return false;
}

void cPythonScript::unload( void )
{
	if( !codeModule )
		return;

	PyObject* method = PyObject_GetAttrString( codeModule, "onUnload" ); 
		
	if( !method )
	{
		PyErr_Clear();
		Py_DECREF( codeModule );
		codeModule = 0;
		return;
	}

	if ( !PyCallable_Check( method ) )
	{
		Py_DECREF( method );
		Py_DECREF( codeModule );
		codeModule = 0;
	}
		
	PyObject* result = PyObject_CallObject( method, NULL );
	PyReportError();
	Py_XDECREF( result ); // void
	Py_DECREF( method );
	Py_DECREF( codeModule );
	codeModule = 0;

}

// Find our module name
bool cPythonScript::load( const cElement *element )
{
	// Initialize it
	codeModule = NULL;
	catchAllSpeech_ = false;

	QString name = element->text();

	if( name.isNull() )
		return false;

	setName( name );

	QString moduleName = element->text();

	if( moduleName.isNull() || moduleName.isEmpty() )
		return false;

	codeModule = PyImport_ImportModule( const_cast<char*>(moduleName.latin1()) );

	if( !codeModule )
	{
		clConsole.ProgressFail();

		if( PyErr_Occurred() )
			PyErr_Print();

		clConsole.send( QString( "\nError while compiling module [" + moduleName + "]\n" ) );
		clConsole.PrepareProgress( "Continuing loading" );
		return false;
	}

	// Call the load Function
	PyObject* method = PyObject_GetAttrString( codeModule, "onLoad" ); 
		
	if ( method )
	{
		if ( PyCallable_Check( method ) )
		{
			PyObject* result = PyObject_CallObject( method, NULL );
			PyReportError();
			Py_XDECREF( result );
		}
		Py_DECREF( method );
	}
		
	handleSpeech_ = PyObject_HasAttrString( codeModule, "onSpeech" );
	return true;
}

//========================== OVERRIDDEN DEFAULT EVENTS
bool cPythonScript::onServerstart()
{
	PyHasMethod( "onServerstart" )
	return PyEvalMethod( "onServerstart", PyTuple_New( 0 ) );
}

bool cPythonScript::onUse( P_CHAR User, P_ITEM Used )
{
	PyHasMethod( "onUse" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( User ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Used ) );

	return PyEvalMethod( "onUse", tuple );
}

bool cPythonScript::onSingleClick( P_ITEM Item, P_CHAR Viewer )
{
	PyHasMethod( "onSingleClick" )
	
	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetItemObject( Item ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );

	return PyEvalMethod( "onSingleClick", tuple );
}

bool cPythonScript::onSingleClick( P_CHAR Character, P_CHAR Viewer )
{
	PyHasMethod( "onSingleClick" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );
	
	return PyEvalMethod( "onSingleClick", tuple );
}

bool cPythonScript::onLogout( P_CHAR Character )
{
	PyHasMethod( "onLogout" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	return PyEvalMethod( "onLogout", tuple );
}

bool cPythonScript::onLogin( P_CHAR Character )
{
	PyHasMethod( "onLogin" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	return PyEvalMethod( "onLogin", tuple );
}

bool cPythonScript::onCollide( P_CHAR Character, P_ITEM Obstacle )
{
	PyHasMethod( "onCollide" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Obstacle ) );

	return PyEvalMethod( "onCollide", tuple );
}

bool cPythonScript::onWalk( P_CHAR Character, UINT8 Direction, UINT8 Sequence )
{
	PyHasMethod( "onWalk" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Direction ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( Sequence ) );

	return PyEvalMethod( "onWalk", tuple );
}

// if this events returns true (handeled) then we should not display the text
bool cPythonScript::onTalk( P_CHAR Character, char speechType, UINT16 speechColor, UINT16 speechFont, const QString &Text, const QString &Lang )
{
	PyHasMethod( "onTalk" )

	PyObject *tuple = PyTuple_New( 6 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( speechType ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( speechColor ) );
	PyTuple_SetItem( tuple, 3, PyInt_FromLong( speechFont ) );
	PyTuple_SetItem( tuple, 4, PyString_FromString( Text.ascii() ) );
	PyTuple_SetItem( tuple, 5, PyString_FromString( Lang.ascii() ) );
	
	return PyEvalMethod( "onTalk", tuple );
}

bool cPythonScript::onWarModeToggle( P_CHAR Character, bool War )
{
	PyHasMethod( "onWarModeToggle" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, ( War ? PyInt_FromLong( 1 ) : PyInt_FromLong( 0 ) ) );

	return PyEvalMethod( "onWarModeToggle", tuple );
}

bool cPythonScript::onHelp( P_CHAR Character )
{
	PyHasMethod( "onHelp" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	return PyEvalMethod( "onHelp", tuple );
}


bool cPythonScript::onChat( P_CHAR Character )
{
	PyHasMethod( "onChat" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	return PyEvalMethod( "onChat", tuple );
}


bool cPythonScript::onSkillUse( P_CHAR Character, UINT8 Skill )
{
	PyHasMethod( "onSkillUse" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Skill ) );

	return PyEvalMethod( "onSkillUse", tuple );
}

bool cPythonScript::onSkillGain( P_CHAR Character, UINT8 Skill, INT32 min, INT32 max, bool success )
{
	PyHasMethod( "onSkillGain" )

	PyObject *tuple = PyTuple_New( 5 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Skill ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( min ) );
	PyTuple_SetItem( tuple, 3, PyInt_FromLong( max ) );
	PyTuple_SetItem( tuple, 4, PyInt_FromLong( success ) );

	return PyEvalMethod( "onSkillGain", tuple );
}

bool cPythonScript::onStatGain( P_CHAR Character, UINT8 stat, INT8 amount )
{
	PyHasMethod( "onStatGain" )

	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( stat ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( amount ) );

	return PyEvalMethod( "onStatGain", tuple );
}

bool cPythonScript::onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id )
{
	PyHasMethod( "onContextEntry" )

	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	
	if( isItemSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyTuple_SetItem( tuple, 2, PyInt_FromLong( id ) );

	return PyEvalMethod( "onContextEntry", tuple );
}

bool cPythonScript::onShowContextMenu( P_CHAR pChar, cUObject *pObject )
{
	PyHasMethod( "onShowContextMenu" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	
	if( isItemSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	return PyEvalMethod( "onShowContextMenu", tuple );
}

bool cPythonScript::onShowToolTip( P_CHAR pChar, cUObject *pObject, cUOTxTooltipList* tooltip )
{
	PyHasMethod( "onShowToolTip" )
	
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );

	if( isItemSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetItemObject( (P_ITEM)pObject ) );
	else if( isCharSerial( pObject->serial() ) )
		PyTuple_SetItem( tuple, 1, PyGetCharObject( (P_CHAR)pObject ) );

	PyTuple_SetItem( tuple, 2, PyGetTooltipObject( tooltip ) );

	return PyEvalMethod( "onShowToolTip", tuple );
}

unsigned int cPythonScript::onDamage( P_CHAR pChar, unsigned char type, unsigned int amount, cUObject *source )
{
	if( !codeModule )
		return amount;

	PyObject *method = PyObject_GetAttrString( codeModule, "onDamage" );
	
	if( !method )
		return amount;

	if ( !PyCallable_Check( method ) )
	{
		Py_DECREF( method );
		return amount;
	}

	PyObject *args = PyTuple_New( 4 );
	
	PyTuple_SetItem( args, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( args, 1, PyInt_FromLong( type ) );
	PyTuple_SetItem( args, 2, PyInt_FromLong( amount ) );
	
	if( source && source->isItem() )
		PyTuple_SetItem( args, 3, PyGetItemObject( dynamic_cast< P_ITEM >( source ) ) );
	else
		PyTuple_SetItem( args, 3, PyGetCharObject( dynamic_cast< P_CHAR >( source ) ) );


	PyObject *returnValue = PyObject_CallObject( method, args ); 
	
	PyReportError();
	Py_DECREF( args );
	Py_DECREF( method );
	
	if( !returnValue ) 
		return amount; 
	if ( !PyInt_Check( returnValue ) )
	{
		Py_DECREF( returnValue );
		return amount;
	}

	amount = PyInt_AsLong( returnValue );
	Py_DECREF( returnValue );
	return amount;	
}

bool cPythonScript::onCastSpell( cPlayer *player, unsigned int spell )
{
	PyHasMethod( "onCastSpell" )

	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( player) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( spell ) );

	return PyEvalMethod( "onCastSpell", tuple );
}

bool cPythonScript::onCreate( cUObject *object, const QString &definition )
{
	PyHasMethod( "onCreate" )
	
	PyObject *tuple = PyTuple_New( 2 );

	if( object->isChar() )
		PyTuple_SetItem( tuple, 0, PyGetCharObject( (P_CHAR)object ) );
	else if( object->isItem() )
		PyTuple_SetItem( tuple, 0, PyGetItemObject( (P_ITEM)object ) );

	PyTuple_SetItem( tuple, 1, PyString_FromString( definition.latin1() ) );
	
	return PyEvalMethod( "onCreate", tuple );
}

bool cPythonScript::onSpeech( cUObject *listener, P_CHAR talker, const QString &text, const QValueVector< UINT16 >& keywords )
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

	return PyEvalMethod( "onSpeech", tuple );
}

bool cPythonScript::onDropOnChar( P_CHAR pChar, P_ITEM pItem )
{
	PyHasMethod( "onDropOnChar" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );

	return PyEvalMethod( "onDropOnChar", tuple );
}

bool cPythonScript::onDropOnItem( P_ITEM pCont, P_ITEM pItem )
{
	PyHasMethod( "onDropOnItem" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetItemObject( pCont ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );

	return PyEvalMethod( "onDropOnItem", tuple );
}

bool cPythonScript::onDropOnGround( P_ITEM pItem, const Coord_cl &pos )
{
	PyHasMethod( "onDropOnGround" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetItemObject( pItem ) );
	PyTuple_SetItem( tuple, 1, PyGetCoordObject( pos ) );

	return PyEvalMethod( "onDropOnGround", tuple );
}

bool cPythonScript::onPickup( P_CHAR pChar, P_ITEM pItem )
{
	PyHasMethod( "onPickup" )
	
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );

	return PyEvalMethod( "onPickup", tuple );
}

bool cPythonScript::onCommand( cUOSocket *socket, const QString &name, const QString &args )
{
	PyHasMethod( "onCommand" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetSocketObject( socket ) );
	PyTuple_SetItem( tuple, 1, PyString_FromString( name.latin1() ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( args.latin1() ) );

	return PyEvalMethod( "onCommand", tuple );
}

bool cPythonScript::onShowPaperdoll( P_CHAR pChar, P_CHAR pOrigin )
{
	PyHasMethod( "onShowPaperdoll" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( pOrigin ) );

	return PyEvalMethod( "onShowPaperdoll", tuple );
}

bool cPythonScript::onDeath( P_CHAR pChar )
{
	PyHasMethod( "onDeath" )
	
	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 1 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	return PyEvalMethod( "onDeath", tuple );
}

bool cPythonScript::onShowSkillGump( P_CHAR pChar )
{
	PyHasMethod( "onShowSkillGump" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 1 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );

	return PyEvalMethod( "onShowSkillGump", tuple );
}

bool cPythonScript::onWorldSave( cDBDriver *connection )
{
	PyHasMethod( "onWorldSave" )
	PyObject *tuple = PyTuple_New( 0 );
	return PyEvalMethod( "onWorldSave", tuple );
}

bool cPythonScript::onWorldLoad( cDBDriver *connection )
{
	PyHasMethod( "onWorldLoad" )
	PyObject *tuple = PyTuple_New( 0 );
	return PyEvalMethod( "onWorldSave", tuple );
}

bool cPythonScript::onWearItem( P_PLAYER pPlayer, P_CHAR pChar, P_ITEM pItem, unsigned char layer )
{
	PyHasMethod( "onWearItem" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 4 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pPlayer ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 2, PyGetItemObject( pItem ) );
	PyTuple_SetItem( tuple, 3, PyInt_FromLong( layer ) );

	return PyEvalMethod( "onWearItem", tuple );
}

bool cPythonScript::onEquip( P_CHAR pChar, P_ITEM pItem, unsigned char layer )
{
	PyHasMethod( "onEquip" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( layer ) );

	return PyEvalMethod( "onEquip", tuple );
}

bool cPythonScript::onUnequip( P_CHAR pChar, P_ITEM pItem, unsigned char layer )
{
	PyHasMethod( "onUnequip" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pItem ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( layer ) );

	return PyEvalMethod( "onUnequip", tuple );
}

bool cPythonScript::onBookUpdateInfo( P_CHAR pChar, P_ITEM pBook, const QString &author, const QString &title )
{
	PyHasMethod( "onBookUpdateInfo" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 4 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pBook ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( author.latin1() ) );
	PyTuple_SetItem( tuple, 3, PyString_FromString( title.latin1() ) );

	return PyEvalMethod( "onBookUpdateInfo", tuple );
}

bool cPythonScript::onBookRequestPage( P_CHAR pChar, P_ITEM pBook, unsigned short page )
{
	PyHasMethod( "onBookRequestPage" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 3 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pBook ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( page ) );

	return PyEvalMethod( "onBookRequestPage", tuple );
}

bool cPythonScript::onBookUpdatePage( P_CHAR pChar, P_ITEM pBook, unsigned short page, const QString &content )
{
	PyHasMethod( "onBookUpdatePage" )

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 4 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( pBook ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( page ) );
	PyTuple_SetItem( tuple, 3, PyString_FromString( content.latin1() ) );

	return PyEvalMethod( "onBookUpdatePage", tuple );
}

QString cPythonScript::onShowPaperdollName( P_CHAR pChar, P_CHAR pOrigin )
{
	if( !codeModule ) 
		return QString::null; 

	PyObject* method = PyObject_GetAttrString( codeModule, "onShowPaperdollName" ); 
	
	if ( !method )
		return QString::null;
	
	if( !PyCallable_Check( method ) ) 
	{
		Py_DECREF( method );
		return QString::null; 
	}

	// Create our args for the python function
	PyObject *tuple = PyTuple_New( 2 );
	PyTuple_SetItem( tuple, 0, PyGetCharObject( pChar ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( pOrigin ) );

	PyObject *returnValue = PyObject_CallObject( method, tuple ); 
	PyReportError();
	Py_DECREF( tuple );
	Py_DECREF( method );

	if( !returnValue ) 
		return QString::null; 
	if( !PyString_Check( returnValue ) ) 
	{
		Py_DECREF( returnValue );
		return QString::null;
	}
	
	QString result = PyString_AsString( returnValue );
	Py_DECREF( returnValue );
	return result;
}


