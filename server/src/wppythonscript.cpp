// WPPythonScript.cpp: Implementierung der Klasse WPPythonScript.
//
//////////////////////////////////////////////////////////////////////

#include "WPPythonScript.h"
#include "worldmain.h"
#include "globals.h"
#include "junk.h"

#define PyFalse PyInt_FromLong( 0 )
#define PyTrue PyInt_FromLong( 1 )
#define PyReturnCheck(a) if( a == NULL ) return false; if( !PyInt_Check( a ) ) return false; if( PyInt_AsLong( a ) == 1 ) return true; else return false;
#define PyMethodCheck(a) if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) return false;
#define PyMethodCheckVoid(a) if( ( method == NULL ) || ( !PyCallable_Check( method ) ) ) return;

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
	PyObject *tuple = PyTuple_New( 1 ); // Create our args for the python function
	PyTuple_SetItem( tuple, 0, PyGetCharObject( Character ) );

	PyObject* method = PyObject_GetAttr( codeModule, PyString_FromString( "onEnterWorld" ) );

	PyMethodCheckVoid( method ) // Checks if the method is callable and exists

	PyObject_CallObject( method, tuple );
	PyReportError();
}


bool WPPythonScript::onHelp( P_CHAR Character )
{
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
	if( !strcmp( name, "name" ) )
		return PyString_FromString( self->Item->name.c_str() );	

	else if( !strcmp( name, "id" ) )
		return PyInt_FromLong( self->Item->id() );

	else if( !strcmp( name, "serial" ) )
		return PyInt_FromLong( self->Item->serial );

	// If no property is found search for a method
	return Py_FindMethod( Py_WPItemMethods, (PyObject*)self, name );
}

int Py_WPItemSetAttr( Py_WPItem *self, char *name, PyObject *value )
{
	return 0;
}

// This will send the item to all surrounding clients
PyObject* Py_WPItem_update( PyObject* self, PyObject* args )
{
	return NULL;
}

// This will delete the object and it's reference
PyObject* Py_WPItem_delete( PyObject* self, PyObject* args )
{
	return NULL;
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
 