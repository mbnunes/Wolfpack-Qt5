// WPPythonScript.cpp: Implementierung der Klasse WPPythonScript.
//
//////////////////////////////////////////////////////////////////////

#include "wppythonscript.h"
#include "worldmain.h"
#include "globals.h"
#include "junk.h"

#define PyFalse PyInt_FromLong( 0 )
#define PyTrue PyInt_FromLong( 1 )
#define PyReturnCheck(a) if( a == NULL ) return false; if( !PyInt_Check( a ) ) return false; if( PyInt_AsLong( a ) == 1 ) return true; else return false;
#define PyMethodCheck(a) if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) return false;
#define PyMethodCheckVoid(a) if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) return;
#define PyHasMethod(a) if( !PyObject_HasAttr( codeModule, PyString_FromString( a ) ) ) return false;
#define PyHasMethodVoid(a) if( !PyObject_HasAttr( codeModule, PyString_FromString( a ) ) ) return;

// Setting and getting item properties
#define setItemIntProperty( identifier, property ) if( !strcmp ( name, #identifier ) ) self->Item->property = PyInt_AS_LONG( value );
#define setItemStrProperty( identifier, property ) if( !strcmp ( name, #identifier ) ) self->Item->property = PyString_AS_STRING( value );

#define getItemIntProperty( identifier, property ) if( !strcmp( name, #identifier ) ) return PyInt_FromLong( self->Item->property );
#define getItemStrProperty( identifier, property ) if( !strcmp( name, #identifier ) ) return PyString_FromString( self->Item->property );

// If an error occured, report it
inline void PyReportError( void )
{
	if( PyErr_Occurred() )
		PyErr_Print();
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

			if( codeModule == NULL )
			{
				clConsole.send( QString( "Error while compiling the module: " + moduleName ).ascii() );
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

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onUse" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}

bool WPPythonScript::onShowItemName( P_ITEM Item, P_CHAR Viewer )
{
	PyHasMethod( "onShowItemName" )
	
	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetItemObject( Item ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onShowItemName" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}

bool WPPythonScript::onShowCharName( P_CHAR Character, P_CHAR Viewer )
{
	PyHasMethod( "onShowCharName" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Viewer ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onShowCharName" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}

void WPPythonScript::onCollideItem( P_CHAR Character, P_ITEM Obstacle )
{
	PyHasMethodVoid( "onCollideItem" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Obstacle ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onCollideItem" ) );

	PyMethodCheckVoid( method ) // Checks if the method is callable and exists
	PyObject_CallObject( method, tuple );
	PyReportError();
}

void WPPythonScript::onCollideChar( P_CHAR Character, P_CHAR Obstacle )
{
	PyHasMethodVoid( "onCollideChar" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Obstacle ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onCollideChar" ) );

	PyMethodCheckVoid( method ) // Checks if the method is callable and exists

	PyObject_CallObject( method, tuple );
	PyReportError();
}

bool WPPythonScript::onWalk( P_CHAR Character, UI08 Direction, UI08 Sequence )
{
	PyHasMethod( "onWalk" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Direction ) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong( Sequence ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onWalk" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}


// if this events returns true (handeled) then we should not display the text
bool WPPythonScript::onTalk( P_CHAR Character, QString Text )
{
	PyHasMethod( "onTalk" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyString_FromString( Text.ascii() ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onTalk" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}


void WPPythonScript::onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text )
{
	PyHasMethodVoid( "onTalkToNPC" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Talker ) );
	PyTuple_SetItem( tuple, 1, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( Text.ascii() ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onTalkToNPC" ) );
	
	PyMethodCheckVoid( method ) // Checks if the method is callable and exists

	PyObject_CallObject( method, tuple );
	PyReportError();
}

void WPPythonScript::onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text )
{
	PyHasMethodVoid( "onTalkToItem" )

	PyObject *tuple = PyTuple_New( 3 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Talker ) );
	PyTuple_SetItem( tuple, 1, PyGetItemObject( Item ) );
	PyTuple_SetItem( tuple, 2, PyString_FromString( Text.ascii() ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onTalkToItem" ) );

	PyMethodCheckVoid( method ) // Checks if the method is callable and exists

	PyObject_CallObject( method, tuple );
	PyReportError();
}


void WPPythonScript::onWarModeToggle( P_CHAR Character, bool War )
{
	PyHasMethodVoid( "onWarModeToggle" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, ( War ? PyInt_FromLong( 1 ) : PyInt_FromLong( 0 ) ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onWarModeToggle" ) );

	PyMethodCheckVoid( method ) // Checks if the method is callable and exists

	PyObject_CallObject( method, tuple );
	PyReportError();
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


void WPPythonScript::onEnterWorld( P_CHAR Character )
{
	PyHasMethodVoid( "onEnterWorld" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onEnterWorld" ) );

	PyMethodCheckVoid( method ) // Checks if the method is callable and exists

	PyObject_CallObject( method, tuple );
	PyReportError();
}


bool WPPythonScript::onHelp( P_CHAR Character )
{
	PyHasMethod( "onHelp" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onHelp" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}


bool WPPythonScript::onChat( P_CHAR Character )
{
	PyHasMethod( "onChat" )

	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onChat" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
}


bool WPPythonScript::onSkillUse( P_CHAR Character, UI08 Skill )
{
	PyHasMethod( "onSkillUse" )

	PyObject *tuple = PyTuple_New( 2 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong( Skill ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onSkillUse" ) );

	PyMethodCheck( method ) // Checks if the method is callable and exists

	PyObject *returnValue = PyObject_CallObject( method, tuple );
	PyReportError();

	PyReturnCheck( returnValue ) // Checks the return value
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
	getItemStrProperty( "name", name.c_str() )
	else getItemIntProperty( "id", id() )
	else getItemStrProperty( "name", name.c_str() )
	else getItemStrProperty( "name2", name2.c_str() )
	else getItemIntProperty( "color", color )
	else getItemIntProperty( "amount", amount )
	else getItemIntProperty( "amount2", amount2 )
	else getItemIntProperty( "serial", serial )
	else getItemIntProperty( "x", pos.x )
	else getItemIntProperty( "y", pos.y )
	else getItemIntProperty( "z", pos.z )
	else getItemIntProperty( "plane", pos.plane )
	else getItemIntProperty( "layer", layer )
	else getItemIntProperty( "itemhand", itmhand )
	else getItemIntProperty( "type", type )
	else getItemIntProperty( "type2", type2 )
	// CONTAINER!!
	else getItemIntProperty( "oldx", oldpos.x )
	else getItemIntProperty( "oldy", oldpos.y )
	else getItemIntProperty( "oldz", oldpos.z )
	else getItemIntProperty( "oldplane", oldpos.plane )
	else getItemIntProperty( "oldlayer", oldlayer )
	else getItemIntProperty( "weight", weight )
	else getItemIntProperty( "more1", more1 )
	else getItemIntProperty( "more2", more2 )
	else getItemIntProperty( "more3", more3 )
	else getItemIntProperty( "more4", more4 )
	else getItemIntProperty( "moreb1", moreb1 )
	else getItemIntProperty( "moreb2", moreb2 )
	else getItemIntProperty( "moreb3", moreb3 )
	else getItemIntProperty( "moreb4", moreb4 )
	else getItemIntProperty( "morex", morex )
	else getItemIntProperty( "morey", morey )
	else getItemIntProperty( "morez", morez )
	else getItemIntProperty( "doordir", doordir )
	else getItemIntProperty( "dooropen", dooropen )
	// PILEABLE
	else getItemIntProperty( "dye", dye )
	else getItemIntProperty( "corpse", corpse )
	else getItemIntProperty( "defense", def )
	else getItemIntProperty( "lodamage", lodamage )
	else getItemIntProperty( "hidamage", hidamage )
	else getItemIntProperty( "weaponskill", wpsk )
	else getItemIntProperty( "health", hp )
	else getItemIntProperty( "maxhealth", maxhp )
	else getItemIntProperty( "strength", st )
	else getItemIntProperty( "dexterity", dx )
	else getItemIntProperty( "intelligence", in )
	else getItemIntProperty( "strength2", st2 )
	else getItemIntProperty( "dexterity2", dx2 )
	else getItemIntProperty( "intelligence2", in2 )
	else getItemIntProperty( "speed", spd )
	else getItemIntProperty( "smelt", smelt )
	else getItemIntProperty( "secureIt", secureIt )
	else getItemIntProperty( "moveable", magic )
	else getItemIntProperty( "gatetime", gatetime )
	else getItemIntProperty( "gatenumber", gatenumber )
	else getItemIntProperty( "decaytime", decaytime )
	// ownserial
	else getItemIntProperty( "visible", visible )
	// spanserial
	else getItemIntProperty( "dir", dir ) // lightsource type
	else getItemIntProperty( "priv", priv ) 
	else getItemIntProperty( "value", value ) 
	else getItemIntProperty( "restock", restock )
	else getItemIntProperty( "value", value ) 
	else getItemIntProperty( "disabled", disabled )
	else getItemStrProperty( "disabledmsg", disabledmsg.c_str() ) 
	else getItemIntProperty( "poisoned", poisoned ) 
	else getItemStrProperty( "murderer", murderer.c_str() ) 
	else getItemIntProperty( "murdertime", murdertime ) 
	else getItemIntProperty( "rank", rank ) 
	else getItemStrProperty( "creator", creator.c_str() ) 
	else getItemIntProperty( "good", good ) 
	else getItemIntProperty( "madewith", madewith ) 
	else getItemStrProperty( "desc", desc.c_str() ) 
	else getItemIntProperty( "carve", carve ) 
	else getItemIntProperty( "spawnregion", spawnregion )

	// If no property is found search for a method
	return Py_FindMethod( Py_WPItemMethods, (PyObject*)self, name );
}

int Py_WPItemSetAttr( Py_WPItem *self, char *name, PyObject *value )
{
	if( !strcmp( name, "id" ) )
		self->Item->setId( PyInt_AS_LONG( value ) );

	else setItemStrProperty( "name", name )
	else setItemStrProperty( "name2", name2 )
	else setItemIntProperty( "color", color )
	else setItemIntProperty( "amount", amount )
	else setItemIntProperty( "amount2", amount2 )
	else setItemIntProperty( "serial", serial )
	else setItemIntProperty( "x", pos.x )
	else setItemIntProperty( "y", pos.y )
	else setItemIntProperty( "z", pos.z )
	else setItemIntProperty( "plane", pos.plane )
	else setItemIntProperty( "layer", layer )
	else setItemIntProperty( "itemhand", itmhand )
	else setItemIntProperty( "type", type )
	else setItemIntProperty( "type2", type2 )
	// CONTAINER!!
	else setItemIntProperty( "oldx", oldpos.x )
	else setItemIntProperty( "oldy", oldpos.y )
	else setItemIntProperty( "oldz", oldpos.z )
	else setItemIntProperty( "oldplane", oldpos.plane )
	else setItemIntProperty( "oldlayer", oldlayer )
	else setItemIntProperty( "weight", weight )
	else setItemIntProperty( "more1", more1 )
	else setItemIntProperty( "more2", more2 )
	else setItemIntProperty( "more3", more3 )
	else setItemIntProperty( "more4", more4 )
	else setItemIntProperty( "moreb1", moreb1 )
	else setItemIntProperty( "moreb2", moreb2 )
	else setItemIntProperty( "moreb3", moreb3 )
	else setItemIntProperty( "moreb4", moreb4 )
	else setItemIntProperty( "morex", morex )
	else setItemIntProperty( "morey", morey )
	else setItemIntProperty( "morez", morez )
	else setItemIntProperty( "doordir", doordir )
	else setItemIntProperty( "dooropen", dooropen )
	// PILEABLE
	else setItemIntProperty( "dye", dye )
	else setItemIntProperty( "corpse", corpse )
	else setItemIntProperty( "defense", def )
	else setItemIntProperty( "lodamage", lodamage )
	else setItemIntProperty( "hidamage", hidamage )
	else setItemIntProperty( "weaponskill", wpsk )
	else setItemIntProperty( "health", hp )
	else setItemIntProperty( "maxhealth", maxhp )
	else setItemIntProperty( "strength", st )
	else setItemIntProperty( "dexterity", dx )
	else setItemIntProperty( "intelligence", in )
	else setItemIntProperty( "strength2", st2 )
	else setItemIntProperty( "dexterity2", dx2 )
	else setItemIntProperty( "intelligence2", in2 )
	else setItemIntProperty( "speed", spd )
	else setItemIntProperty( "smelt", smelt )
	else setItemIntProperty( "secureIt", secureIt )
	else setItemIntProperty( "moveable", magic )
	else setItemIntProperty( "gatetime", gatetime )
	else setItemIntProperty( "gatenumber", gatenumber )
	else setItemIntProperty( "decaytime", decaytime )
	// ownserial
	else setItemIntProperty( "visible", visible )
	// spanserial
	else setItemIntProperty( "dir", dir ) // lightsource type
	else setItemIntProperty( "priv", priv ) 
	else setItemIntProperty( "value", value ) 
	else setItemIntProperty( "restock", restock )
	else setItemIntProperty( "value", value ) 
	else setItemIntProperty( "disabled", disabled )
	else setItemStrProperty( "disabledmsg", disabledmsg ) 
	else setItemIntProperty( "poisoned", poisoned ) 
	else setItemStrProperty( "murderer", murderer ) 
	else setItemIntProperty( "murdertime", murdertime ) 
	else setItemIntProperty( "rank", rank ) 
	else setItemStrProperty( "creator", creator ) 
	else setItemIntProperty( "good", good ) 
	else setItemIntProperty( "madewith", madewith ) 
	else setItemStrProperty( "desc", desc ) 
	else setItemIntProperty( "carve", carve ) 
	else setItemIntProperty( "spawnregion", spawnregion ) 
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

PyObject *Py_WPCharGetAttr( Py_WPChar *self, char *name )
{
	if( !strcmp( name, "name" ) )
		return PyString_FromString( self->Char->name.c_str() );	

	else if( !strcmp( name, "orgname" ) )
		return PyString_FromString( self->Char->orgname.c_str() );	

	else if( !strcmp( name, "title" ) )
		return PyString_FromString( self->Char->orgname.c_str() );	

	else if( !strcmp( name, "serial" ) )
		return PyInt_FromLong( self->Char->serial );

	// Body, XBody, Skin, XSkin
	else if( !strcmp( name, "body" ) )
		return PyInt_FromLong( self->Char->id() );

	else if( !strcmp( name, "xbody" ) )
		return PyInt_FromLong( self->Char->xid );

	else if( !strcmp( name, "skin" ) )
		return PyInt_FromLong( self->Char->skin );
	
	else if( !strcmp( name, "xskin" ) )
		return PyInt_FromLong( self->Char->xskin );

	// Health, Stamina, Mana
	else if( !strcmp( name, "health" ) )
		return PyInt_FromLong( self->Char->hp );

	else if( !strcmp( name, "stamina" ) )
		return PyInt_FromLong( self->Char->stm );

	else if( !strcmp( name, "mana" ) )
		return PyInt_FromLong( self->Char->mn );

	// Strength, Dexterity, Intelligence
	else if( !strcmp( name, "strength" ) )
		return PyInt_FromLong( self->Char->st );

	else if( !strcmp( name, "dexterity" ) )
		return PyInt_FromLong( self->Char->effDex() );

	else if( !strcmp( name, "intelligence" ) )
		return PyInt_FromLong( self->Char->in );

	// X,Y,Z,(W)
	else if( !strcmp( name, "x" ) )
		return PyInt_FromLong( self->Char->pos.x );

	else if( !strcmp( name, "y" ) )
		return PyInt_FromLong( self->Char->pos.y );

	else if( !strcmp( name, "z" ) )
		return PyInt_FromLong( self->Char->pos.z );

	else if( !strcmp( name, "plane" ) )
		return PyInt_FromLong( self->Char->pos.plane );

	else if( !strcmp( name, "direction" ) )
		return PyInt_FromLong( self->Char->dir );

	else if( !strcmp( name, "incognito" ) )
		return self->Char->incognito ? PyTrue : PyFalse;

	else if( !strcmp( name, "polymorph" ) )
		return self->Char->polymorph ? PyTrue : PyFalse;

	// Flags ?
	else if( !strcmp( name, "flags2" ) )
		return PyInt_FromLong( self->Char->priv2 );

	// HiDamage,LoDamage
	else if( !strcmp( name, "hidamage" ) )
		return PyInt_FromLong( self->Char->hidamage );

	else if( !strcmp( name, "lodamage" ) )
		return PyInt_FromLong( self->Char->lodamage );

	// If no property is found search for a method
	return Py_FindMethod( Py_WPCharMethods, (PyObject*)self, name );
}

int Py_WPCharSetAttr( Py_WPChar *self, char *name, PyObject *value )
{


	return 0;
}

PyObject* Py_WPChar_update( Py_WPChar* self, PyObject* args )
{
	return PyTrue;
}

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
 
