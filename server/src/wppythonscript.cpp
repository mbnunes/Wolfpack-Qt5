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

#include "wppythonscript.h"
#include "worldmain.h"
#include "globals.h"
#include "junk.h"
#include "qfile.h"

#define PyFalse PyInt_FromLong( 0 )
#define PyTrue PyInt_FromLong( 1 )
#define PyHasMethod(a) if( codeModule == NULL ) return false; if( !PyObject_HasAttr( codeModule, PyString_FromString( a ) ) ) return false;

// Method Calling Macro!
#define PyEvalMethod(a) PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( a ) ); if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) return false; PyObject *returnValue = PyObject_CallObject( method, tuple ); PyReportError(); if( returnValue == NULL ) return false; if( !PyInt_Check( returnValue ) ) return false; if( PyInt_AsLong( returnValue ) == 1 ) return true; else return false;

// Setting and getting item/char properties
#define setIntProperty( identifier, property ) if( !strcmp( name, identifier ) ) self->property = PyInt_AS_LONG( value );
#define setStrProperty( identifier, property ) if( !strcmp( name, identifier ) ) self->property = PyString_AS_STRING( value );
#define getIntProperty( identifier, property ) if( !strcmp( name, identifier ) ) return PyInt_FromLong( self->property );
#define getStrProperty( identifier, property ) if( !strcmp( name, identifier ) ) return PyString_FromString( self->property );

// If an error occured, report it
inline void PyReportError( void )
{
	if( PyErr_Occurred() )
		PyErr_Print();
}

inline PyObject* PyGetTargetObject( PKGx6C Target )
{
	Py_WPTarget *returnVal = PyObject_New( Py_WPTarget, &Py_WPTargetType );
	returnVal->targetInfo = Target;
	return (PyObject*)returnVal;
}

inline PyObject* PyGetItemObject( P_ITEM Item )
{
	if( Item == NULL )
		return Py_None;

	Py_WPItem *returnVal = PyObject_New( Py_WPItem, &Py_WPItemType );
	returnVal->Item = Item;
	return (PyObject*)returnVal;
}

inline PyObject* PyGetCharObject( P_CHAR Char )
{
	if( Char == NULL )
		return Py_None;

	Py_WPChar *returnVal = PyObject_New( Py_WPChar, &Py_WPCharType );
	returnVal->Char = Char;
	return (PyObject*)returnVal;
}

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

// Find our module name
void WPPythonScript::load( const QDomElement &Data )
{
	// Initialize it
	codeModule = NULL;

	QDomNodeList nodeList = Data.childNodes();

	if( nodeList.count() < 1 )
		return;

	// Walk all settings
	for( UI08 i = 0; i < nodeList.count(); i++ )
	{
		if( !nodeList.item( i ).isElement() )
			continue;

		QDomElement element = nodeList.item( i ).toElement();

		if( element.nodeName() == "module" )
		{
			if( !element.attributes().contains( "name" ) )
				continue;

			QString moduleName = element.attributeNode( "name" ).nodeValue();

			// Compile the codemodule
			char moduleNameStr[1024]; // Just to be sure
			strcpy( &moduleNameStr[ 0 ], moduleName.ascii() );

			codeModule = PyImport_ImportModule( moduleNameStr );

			/*if( PyErr_Occurred() )
			{
				PyErr_Print(); // Hm, seems to simple... does it ?
				//PyObject *errType, *errValue, *errTrace;
				//PyErr_Fetch( &errType, &errValue, &errTrace );

				//if( errType != NULL && errValue != NULL ) 
				//	savelog( QString( "%1 %2" ).arg( PyString_AS_STRING( errType ) ).arg( PyString_AS_STRING( errValue ) ).ascii(), "python.log" );
			}*/

			if( codeModule == NULL )
			{
				clConsole.ProgressFail();

				if( PyErr_Occurred() )
					PyErr_Print();

				clConsole.send( QString( "\nError while compiling module [" + moduleName + "]\n" ).ascii() );
				clConsole.PrepareProgress( "Continuing loading" );
				return;
			}
		}
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

bool WPPythonScript::onShowItemName( P_ITEM Item, P_CHAR Viewer )
{
	PyHasMethod( "onShowItemName" )
	
	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetItemObject( Item ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );

	PyEvalMethod( "onShowItemName" )
}

bool WPPythonScript::onShowCharName( P_CHAR Character, P_CHAR Viewer )
{
	PyHasMethod( "onShowCharName" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );
	
	PyEvalMethod( "onShowItemName" )
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


// Is the Client version already known to us here ???
bool WPPythonScript::onConnect( UOXSOCKET Socket, const QString &IP, const QString &Username, const QString &Password )
{
	// This really does nothing yet...

	return false;
}


bool WPPythonScript::onDisconnect( UOXSOCKET Socket, QString IP )
{
	// Look above
	return false;
}


bool WPPythonScript::onEnterWorld( P_CHAR Character )
{
	PyHasMethod( "onEnterWorld" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyEvalMethod( "onEnterWorld" )
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

//========================== Initialize Python extensions
void initPythonExtensions( void )
{
	PyImport_AddModule( "_wolfpack" );
	Py_InitModule( "_wolfpack", WPGlobalMethods );
}

//============ wolfpack.server
PyObject* PyWPServer_shutdown( PyObject* self, PyObject* args )
{
	keeprun = 0;

	return PyTrue;
}

PyObject* PyWPServer_save( PyObject* self, PyObject* args )
{
	cwmWorldState->savenewworld( "binary" );

	return PyTrue;
}

//============ wolfpack.items
PyObject* PyWPItems_add( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return Py_None;

	// The first argument is always the item-id
	QString itemID( PyString_AsString( PyTuple_GetItem( args, 0 ) ) );

	P_ITEM nItem = Items->createScriptItem( itemID );
	
	if( nItem == NULL )
		return Py_None;

	// Now we can determine what to do with the item
	// wolfpack.items.add( "myitem", x, y, z, [plane] )
	if( PyTuple_Size( args ) == 2 )
	{
		PyObject *object = PyTuple_GetItem( args, 1 );

		// Equip the item if possible, else pack it into the chars backpack and if the char doesn't have one pack it to the ground
		if( !strcmp( object->ob_type->tp_name, "char" ) )
		{
			P_CHAR wearer = ((Py_WPChar*)object)->Char;

			equipItem( wearer, nItem );
		}
		else if( !strcmp( object->ob_type->tp_name, "item" ) )
		{
			P_ITEM container = ((Py_WPItem*)object)->Item;
			
			// Put the item into the container
			nItem->SetContSerial( container->serial );
			RefreshItem( nItem );
		}
	}
	else if( PyTuple_Size( args ) == 4 )
	{
		Coord_cl mapCoord;
		mapCoord.x = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
		mapCoord.y = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );
		mapCoord.z = PyInt_AsLong( PyTuple_GetItem( args, 3 ) );

		nItem->moveTo( mapCoord );
		RefreshItem( nItem );
	}

	return PyGetItemObject( nItem ); 
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

//============ wolfpack.console
PyObject* PyWPConsole_send( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == Py_None )
		return PyFalse;

	clConsole.send( PyString_AS_STRING( pyMessage ) );

	return PyTrue;
}

PyObject* PyWPConsole_progress( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == NULL )
		return PyFalse;

	clConsole.PrepareProgress( PyString_AS_STRING( pyMessage ) );

	return PyInt_FromLong( 1 );
}

PyObject* PyWPConsole_printDone( PyObject* self, PyObject* args )
{
	clConsole.ProgressDone();
	return PyInt_FromLong( 1 );
}

PyObject* PyWPConsole_printFail( PyObject* self, PyObject* args )
{
	clConsole.ProgressFail();
	return PyInt_FromLong( 1 );
}

PyObject* PyWPConsole_printSkip( PyObject* self, PyObject* args )
{
	clConsole.ProgressSkip();
	return PyInt_FromLong( 1 );
}

PyObject* PyWPMovement_deny( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 2 ) 
		return PyFalse;

	Py_WPChar *Char = (Py_WPChar*)PyTuple_GetItem( args, 0 );
	P_CHAR pc = Char->Char;
	UI08 sequence = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	UOXSOCKET k = calcSocketFromChar( pc );

	char walkdeny[9] = "\x21\x00\x01\x02\x01\x02\x00\x01";
	
	walkdeny[1] = sequence;
	walkdeny[2] = pc->pos.x>>8;
	walkdeny[3] = pc->pos.x%256;
	walkdeny[4] = pc->pos.y>>8;
	walkdeny[5] = pc->pos.y%256;
	walkdeny[6] = pc->dir;
	walkdeny[7] = pc->dispz;
	Network->xSend( k, walkdeny, 8, 0 );
	walksequence[k] = -1;

	return PyTrue;
}

PyObject* PyWPMovement_accept( PyObject* self, PyObject* args )
{
	return PyFalse;
}

// ============================ class: item

PyObject *Py_WPItemGetAttr( Py_WPItem *self, char *name )
{
	getStrProperty( "name", Item->name().ascii() )
	else getIntProperty( "id", Item->id() )
	else getStrProperty( "name2", Item->name2().ascii() )
	else getIntProperty( "color", Item->color() )
	else getIntProperty( "amount", Item->amount() )
	else getIntProperty( "amount2", Item->amount2() )
	else getIntProperty( "serial", Item->serial )
	else getIntProperty( "x", Item->pos.x )
	else getIntProperty( "y", Item->pos.y )
	else getIntProperty( "z", Item->pos.z )
	else getIntProperty( "plane", Item->pos.plane )
	else getIntProperty( "layer", Item->layer() )
	else getIntProperty( "itemhand", Item->itemhand() )
	else getIntProperty( "type", Item->type )
	else getIntProperty( "type2", Item->type2 )
	
	// What we're contained in
	else if( !strcmp( "container", name ) )
	{
		if( self->Item->contserial == 0xFFFFFFFF )
			return Py_None;
		else if( isCharSerial( self->Item->contserial ) )
			return PyGetCharObject( FindCharBySerial( self->Item->contserial ) );
		else 
			return PyGetItemObject( FindItemBySerial( self->Item->contserial ) ); 
	}

	// What we contain
	else if( !strcmp( "content", name ) )
	{
		Py_WPContent *returnVal = PyObject_New( Py_WPContent, &Py_WPContentType );
		returnVal->Item = self->Item; // Never forget that
		return (PyObject*)returnVal;
	}

	else getIntProperty( "oldx", Item->oldpos.x )
	else getIntProperty( "oldy", Item->oldpos.y )
	else getIntProperty( "oldz", Item->oldpos.z )
	else getIntProperty( "oldplane", Item->oldpos.plane )
	else getIntProperty( "oldlayer", Item->oldlayer )
	else getIntProperty( "weight", Item->weight )
	else getIntProperty( "more1", Item->more1 )
	else getIntProperty( "more2", Item->more2 )
	else getIntProperty( "more3", Item->more3 )
	else getIntProperty( "more4", Item->more4 )
	else getIntProperty( "moreb1", Item->moreb1 )
	else getIntProperty( "moreb2", Item->moreb2 )
	else getIntProperty( "moreb3", Item->moreb3 )
	else getIntProperty( "moreb4", Item->moreb4 )
	else getIntProperty( "morex", Item->morex )
	else getIntProperty( "morey", Item->morey )
	else getIntProperty( "morez", Item->morez )
	else getIntProperty( "doordir", Item->doordir )
	else getIntProperty( "dooropen", Item->dooropen )
	// PILEABLE
	else getIntProperty( "dye", Item->dye )
	else getIntProperty( "corpse", Item->corpse )
	else getIntProperty( "defense", Item->def )
	else getIntProperty( "lodamage", Item->lodamage )
	else getIntProperty( "hidamage", Item->hidamage )
	else getIntProperty( "weaponskill", Item->wpsk )
	else getIntProperty( "health", Item->hp )
	else getIntProperty( "maxhealth", Item->maxhp )
	else getIntProperty( "strength", Item->st )
	else getIntProperty( "dexterity", Item->dx )
	else getIntProperty( "intelligence", Item->in )
	else getIntProperty( "strength2", Item->st2 )
	else getIntProperty( "dexterity2", Item->dx2 )
	else getIntProperty( "intelligence2", Item->in2 )
	else getIntProperty( "speed", Item->spd )
	else getIntProperty( "smelt", Item->smelt )
	else getIntProperty( "secureIt", Item->secureIt )
	else getIntProperty( "moveable", Item->magic )
	else getIntProperty( "gatetime", Item->gatetime )
	else getIntProperty( "gatenumber", Item->gatenumber )
	else getIntProperty( "decaytime", Item->decaytime )
	// ownserial
	else getIntProperty( "visible", Item->visible )
	// spanserial
	else getIntProperty( "dir", Item->dir ) // lightsource type
	else getIntProperty( "priv", Item->priv ) 
	else getIntProperty( "value", Item->value ) 
	else getIntProperty( "restock", Item->restock )
	else getIntProperty( "value", Item->value ) 
	else getIntProperty( "disabled", Item->disabled )
	else getStrProperty( "disabledmsg", Item->disabledmsg.c_str() ) 
	else getIntProperty( "poisoned", Item->poisoned ) 
	else getStrProperty( "murderer", Item->murderer().ascii() ) 
	else getIntProperty( "murdertime", Item->murdertime ) 
	else getIntProperty( "rank", Item->rank ) 
	else getStrProperty( "creator", Item->creator.c_str() ) 
	else getIntProperty( "good", Item->good ) 
	else getIntProperty( "madewith", Item->madewith ) 
	else getStrProperty( "desc", Item->desc.c_str() ) 
	else getIntProperty( "carve", Item->carve ) 
	else getIntProperty( "spawnregion", Item->spawnregion )

	// If no property is found search for a method
	return Py_FindMethod( Py_WPItemMethods, (PyObject*)self, name );
}

int Py_WPItemSetAttr( Py_WPItem *self, char *name, PyObject *value )
{
	if( !strcmp( name, "id" ) )
		self->Item->setId( PyInt_AS_LONG( value ) );

	//else setStrProperty( "name", Item->name )
	else if( !strcmp( "name", name ) )
		self->Item->setName( PyString_AS_STRING( value ) ); 

	else if( !strcmp( "name2", name ) )
		self->Item->setName2( PyString_AS_STRING( value ) );
	
	else if( !strcmp( "color", name ) )
		self->Item->setColor( PyInt_AS_LONG( value ) );

	else if( !strcmp( "amount", name ) )
		self->Item->setAmount( PyInt_AS_LONG( value ) );

	else if( !strcmp( "amount2", name ) )
		self->Item->setAmount2( PyInt_AS_LONG( value ) );

	else setIntProperty( "serial", Item->serial )
	else setIntProperty( "x", Item->pos.x )
	else setIntProperty( "y", Item->pos.y )
	else setIntProperty( "z", Item->pos.z )
	else setIntProperty( "plane", Item->pos.plane )
	
	else if( !strcmp( name, "layer" ) )
		self->Item->setLayer( PyInt_AS_LONG( value ) );
	
	else if( !strcmp( name, "itemhand" ) )
		self->Item->setItemhand( PyInt_AS_LONG( value ) );

	else setIntProperty( "type", Item->type )
	else setIntProperty( "type2", Item->type2 )
	// CONTAINER!!
	else setIntProperty( "oldx", Item->oldpos.x )
	else setIntProperty( "oldy", Item->oldpos.y )
	else setIntProperty( "oldz", Item->oldpos.z )
	else setIntProperty( "oldplane", Item->oldpos.plane )
	else setIntProperty( "oldlayer", Item->oldlayer )
	else setIntProperty( "weight", Item->weight )
	else setIntProperty( "more1", Item->more1 )
	else setIntProperty( "more2", Item->more2 )
	else setIntProperty( "more3", Item->more3 )
	else setIntProperty( "more4", Item->more4 )
	else setIntProperty( "moreb1", Item->moreb1 )
	else setIntProperty( "moreb2", Item->moreb2 )
	else setIntProperty( "moreb3", Item->moreb3 )
	else setIntProperty( "moreb4", Item->moreb4 )
	else setIntProperty( "morex", Item->morex )
	else setIntProperty( "morey", Item->morey )
	else setIntProperty( "morez", Item->morez )
	else setIntProperty( "doordir", Item->doordir )
	else setIntProperty( "dooropen", Item->dooropen )
	// PILEABLE
	else setIntProperty( "dye", Item->dye )
	else setIntProperty( "corpse", Item->corpse )
	else setIntProperty( "defense", Item->def )
	else setIntProperty( "lodamage", Item->lodamage )
	else setIntProperty( "hidamage", Item->hidamage )
	else setIntProperty( "weaponskill", Item->wpsk )
	else setIntProperty( "health", Item->hp )
	else setIntProperty( "maxhealth", Item->maxhp )
	else setIntProperty( "strength", Item->st )
	else setIntProperty( "dexterity", Item->dx )
	else setIntProperty( "intelligence", Item->in )
	else setIntProperty( "strength2", Item->st2 )
	else setIntProperty( "dexterity2", Item->dx2 )
	else setIntProperty( "intelligence2", Item->in2 )
	else setIntProperty( "speed", Item->spd )
	else setIntProperty( "smelt", Item->smelt )
	else setIntProperty( "secureIt", Item->secureIt )
	else setIntProperty( "moveable", Item->magic )
	else setIntProperty( "gatetime", Item->gatetime )
	else setIntProperty( "gatenumber", Item->gatenumber )
	else setIntProperty( "decaytime", Item->decaytime )
	// ownserial
	else setIntProperty( "visible", Item->visible )
	// spanserial
	else setIntProperty( "dir", Item->dir ) // lightsource type
	else setIntProperty( "priv", Item->priv ) 
	else setIntProperty( "value", Item->value ) 
	else setIntProperty( "restock", Item->restock )
	else setIntProperty( "value", Item->value ) 
	else setIntProperty( "disabled", Item->disabled )
	else setStrProperty( "disabledmsg", Item->disabledmsg ) 
	else setIntProperty( "poisoned", Item->poisoned ) 

	else if( !strcmp( name, "murderer" ) )
		self->Item->setMurderer( PyString_AS_STRING( value ) );

	else setIntProperty( "murdertime", Item->murdertime ) 
	else setIntProperty( "rank", Item->rank ) 
	else setStrProperty( "creator", Item->creator ) 
	else setIntProperty( "good", Item->good ) 
	else setIntProperty( "madewith", Item->madewith ) 
	else setStrProperty( "desc", Item->desc ) 
	else setIntProperty( "carve", Item->carve ) 
	else setIntProperty( "spawnregion", Item->spawnregion ) 
	// incognito

	return 0;
}

// This will send the item to all surrounding clients
PyObject* Py_WPItem_update( Py_WPItem* self, PyObject* args )
{
	RefreshItem( self->Item );
	return PyTrue;
}

// This is *USELESS* Amount = 0 should do it
PyObject* Py_WPItem_delete( Py_WPItem* self, PyObject* args )
{
	Items->DeleItem( self->Item );
	self = (Py_WPItem*)Py_None;
	return PyTrue;
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

	if( line_of_sight( -1, source, destination, 1|2|4|8|16|32 ) )
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

PyObject *Py_WPCharGetAttr( Py_WPChar *self, char *name )
{
	getStrProperty( "name", Char->name.c_str() )
	else getStrProperty( "orgname", Char->orgname().latin1() )
	else getStrProperty( "title", Char->title().latin1() )
	else getIntProperty( "serial", Char->serial )
	else getIntProperty( "body", Char->id() )
	else getIntProperty( "xbody", Char->xid )
	else getIntProperty( "skin", Char->skin() )
	else getIntProperty( "xskin", Char->xskin() )
	
	else getIntProperty( "health", Char->hp )
	else getIntProperty( "stamina", Char->stm )
	else getIntProperty( "mana", Char->mn )

	else getIntProperty( "strength", Char->st )
	else getIntProperty( "dexterity", Char->effDex() )
	else getIntProperty( "intelligence", Char->in )

	else getIntProperty( "x", Char->pos.x )
	else getIntProperty( "y", Char->pos.y )
	else getIntProperty( "z", Char->pos.z )
	else getIntProperty( "plane", Char->pos.plane )

	else getIntProperty( "direction", Char->dir )
	else getIntProperty( "flags2", Char->priv2 )
	else getIntProperty( "hidamage", Char->hidamage )
	else getIntProperty( "lodamage", Char->lodamage )

	else if( !strcmp( "equipment", name ) )
	{
		Py_WPEquipment *returnVal = PyObject_New( Py_WPEquipment, &Py_WPEquipmentType );
		returnVal->Char = self->Char; // Never forget that
		return (PyObject*)returnVal;
	}

	// If no property is found search for a method
	return Py_FindMethod( Py_WPCharMethods, (PyObject*)self, name );
}

int Py_WPCharSetAttr( Py_WPChar *self, char *name, PyObject *value )
{
	setStrProperty( "name", Char->name )
	else if ( !strcmp( "orgname", name) )
		self->Char->setOrgname( PyString_AS_STRING( value ) );
	else if ( !strcmp( "title", name) )
		self->Char->setTitle( PyString_AS_STRING( value ) );
	else setIntProperty( "serial", Char->serial )
	
	else if( !strcmp( "body", name ) )
		self->Char->setId( PyInt_AS_LONG( value ) );
	
	else setIntProperty( "xbody", Char->xid )
	else if ( !strcmp( "skin", name ) )
		self->Char->setSkin( PyInt_AS_LONG(value ) );
	else if ( !strcmp( "xskin", name ) )
		self->Char->setXSkin( PyInt_AS_LONG(value ) );
	
	else setIntProperty( "health", Char->hp )
	else setIntProperty( "stamina", Char->stm )
	else setIntProperty( "mana", Char->mn )

	else setIntProperty( "strength", Char->st )
	
	else if( !strcmp( "dexterity", name ) )
		self->Char->setDex( PyInt_AS_LONG( value ) );
	
	else setIntProperty( "intelligence", Char->in )

	else setIntProperty( "x", Char->pos.x )
	else setIntProperty( "y", Char->pos.y )
	else setIntProperty( "z", Char->pos.z )
	else setIntProperty( "plane", Char->pos.plane )

	else setIntProperty( "direction", Char->dir )
	else setIntProperty( "flags2", Char->priv2 )
	else setIntProperty( "hidamage", Char->hidamage )
	else setIntProperty( "lodamage", Char->lodamage )

	return 0;
}

// Resend the char to all connected players
PyObject* Py_WPChar_update( Py_WPChar* self, PyObject* args )
{
	teleport( self->Char );
	return PyTrue;
}

// Send the character a message
PyObject* Py_WPChar_message( Py_WPChar* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == NULL )
		return PyFalse;

	npctalk( calcSocketFromChar( self->Char ), self->Char, PyString_AS_STRING( pyMessage ), 0 );

	return PyTrue;
}

PyObject* Py_WPChar_sysmessage( Py_WPChar* self, PyObject* args )
{
	if( !online( self->Char ) )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == NULL )
		return PyFalse;

	if( PyTuple_Size( args ) == 2 )
	{
		PyObject *color = PyTuple_GetItem( args, 1 );
		if( !PyInt_Check( color ) )
			return PyFalse; // Second argument is not an integer

		sysmessage( calcSocketFromChar( self->Char ), PyInt_AS_LONG( color ), PyString_AS_STRING( pyMessage ) );
	}
	else
		sysmessage( calcSocketFromChar( self->Char ), PyString_AS_STRING( pyMessage ) );

	return PyTrue;
}

PyObject* Py_WPChar_requesttarget( Py_WPChar* self, PyObject* args )
{
	if( !online( self->Char ) )
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
	attachTargetRequest( calcSocketFromChar( self->Char ), target );

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
	Network->Disconnect( self->Socket );
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
	//Network->xSend( self->Socket, buffer, buffLen, 0 );

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
}

