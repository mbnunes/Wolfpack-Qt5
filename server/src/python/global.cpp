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

#include "../itemsmgr.h"
#include "../globals.h"
#include "../network/uosocket.h"
#include "../network/uotxpackets.h"
#include "../wpconsole.h"
#include "../TmpEff.h"
#include "../newmagic.h"
#include "../mapobjects.h"
#include "../territories.h"
#include "../maps.h"
#include "../tilecache.h"
#include "../accounts.h"
#include "../commands.h"
#include "../wpscriptmanager.h"
#include "../wpdefmanager.h"
#include "../wpdefaultscript.h"

#include "utilities.h"
#include "tempeffect.h"

// Library Includes
#include <qdatetime.h>

/*!
	Sends a string to the wolfpack console.
*/
PyObject* wpConsole_send( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == Py_None )
		return PyFalse;

	clConsole.send( PyString_AS_STRING( pyMessage ) );

	return PyTrue;
}

/*!
	Sends a progress-bar to the wolfpack console
*/
PyObject* wpConsole_progress( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == NULL )
		return PyFalse;

	clConsole.PrepareProgress( PyString_AS_STRING( pyMessage ) );

	return PyInt_FromLong( 1 );
}

/*!
	Sends a [done] progress section to the console
*/
PyObject* wpConsole_progressDone( PyObject* self, PyObject* args )
{
	clConsole.ProgressDone();
	return PyInt_FromLong( 1 );
}

/*!
	Sends a [fail] progress section to the console
*/
PyObject* wpConsole_progressFail( PyObject* self, PyObject* args )
{
	clConsole.ProgressFail();
	return PyInt_FromLong( 1 );
}

/*!
	Sends a [skip] progress section to the console
*/
PyObject* wpConsole_progressSkip( PyObject* self, PyObject* args )
{
	clConsole.ProgressSkip();
	return PyInt_FromLong( 1 );
}

/*!
	Returns a list of Strings (the linebuffer)
*/
PyObject* wpConsole_getbuffer( PyObject* self, PyObject* args )
{	
	QStringList linebuffer = clConsole.linebuffer();
	PyObject *list = PyList_New( linebuffer.count() );

	for( int i = 0; i < linebuffer.count(); ++i )
		if( linebuffer[i].isNull() )
			PyList_SetItem( list, i, PyString_FromString( "" ) );
		else
			PyList_SetItem( list, i, PyString_FromString( linebuffer[i].latin1() ) );

	return list;
}


/*!
	wolfpack.console
	Initializes wolfpack.console
*/
static PyMethodDef wpConsole[] = 
{
    { "send",			wpConsole_send,			METH_VARARGS, "Prints something to the wolfpack console" },
	{ "progress",		wpConsole_progress,		METH_VARARGS, "Prints a .....[xxxx] block" },
	{ "progressDone",	wpConsole_progressDone,	METH_VARARGS, "Prints a [done] block" },
	{ "progressFail",	wpConsole_progressFail,	METH_VARARGS, "Prints a [fail] block" },
	{ "progressSkip",	wpConsole_progressSkip,	METH_VARARGS, "Prints a [skip] block" },
	{ "getbuffer",		wpConsole_getbuffer,	METH_VARARGS, "Gets the linebuffer of the console" },
    { NULL, NULL, 0, NULL } // Terminator
};

/*!
	Gets the seconds of the current uo time
*/
PyObject* wpTime_second( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().second() );
}

/*!
	Gets the minutes of the current uo time
*/
PyObject* wpTime_minute( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().minute() );
}

/*!
	Gets the hours of the current uo time
*/
PyObject* wpTime_hour( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().hour() );
}

/*!
	Gets the current day
*/
PyObject* wpTime_day( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.date().day() );
}

/*!
	Gets the current month
*/
PyObject* wpTime_month( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.date().month() );
}

/*!
	Gets the current year
*/
PyObject* wpTime_year( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.date().year() );
}

/*!
	Gets a timestamp of the current uo time
*/
PyObject* wpTime_timestamp( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uoTime.time().elapsed() );
}

/*!
	Methods for handling UO Time from within python
*/
static PyMethodDef wpTime[] = 
{
	{ "second",		wpTime_second,		METH_VARARGS, "Returns the current time-seconds" },
	{ "minute",		wpTime_minute,		METH_VARARGS, "Returns the current time-minutes" },
	{ "hour",			wpTime_hour,		METH_VARARGS, "Returns the current time-hour" },
	{ "day",			wpTime_day,			METH_VARARGS, "Returns the current date-day" },
	{ "month",			wpTime_month,		METH_VARARGS, "Returns the current date-month" },
	{ "year",			wpTime_year,		METH_VARARGS, "Returns the current date-year" },
	{ "timestamp",		wpTime_timestamp,	METH_VARARGS, "Returns the current timestamp" },
    { NULL, NULL, 0, NULL } // Terminator
};

////////////////////////////////////////////////////////////////////////////
// GLOBALS
// Global methods found in wolfpack.
// Examples: wolfpack.additem() wolfpack.addnpc()
////////////////////////////////////////////////////////////////////////////

/*!
	Adds an item
	Argument is just the def-section
*/
PyObject* wpAdditem( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}


	P_ITEM pItem = Items->createScriptItem( PyString_AsString( PyTuple_GetItem( args, 0 ) ) );
	return PyGetItemObject( pItem );
}

/*!
	Adds a npc
*/
PyObject* wpAddnpc( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 2 || !checkArgStr( 0 ) || !checkWpCoord( PyTuple_GetItem( args, 1 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	Coord_cl pos = getWpCoord( PyTuple_GetItem( args, 1 ) );
	P_CHAR pChar = cCharStuff::createScriptNpc( getArgStr( 0 ), pos );

	return PyGetCharObject( pChar ); 
}

/*!
	Creates an item object based on the 
	passed serial
*/
PyObject* wpFinditem( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	SERIAL serial = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	return PyGetItemObject( FindItemBySerial( serial ) );
}

/*!
	Creates a char object based on the 
	passed serial
*/
PyObject* wpFindchar( PyObject* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	SERIAL serial = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	return PyGetCharObject( FindCharBySerial( serial ) );
}

/*!
	Adds a tempeffect
*/
PyObject* wpAddtimer( PyObject* self, PyObject* args )
{
	// Three arguments
	if( PyTuple_Size( args ) < 3 || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT32 expiretime = getArgInt( 0 );
	QString function = getArgStr( 1 );
	PyObject *py_args = PyList_AsTuple( PyTuple_GetItem( args, 2 ) );

	cPythonEffect *effect = new cPythonEffect( function, py_args );
	
	// Should we save this effect?
	if( checkArgInt( 3 ) && getArgInt( 3 ) != 0 ) 
		effect->setSerializable( true );
	else
		effect->setSerializable( false );
	
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );

	return PyFalse;
}

/*!
	Tries to find a region for a specific position.
*/
PyObject* wpRegion( PyObject* self, PyObject* args )
{
	// Three arguments
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyGetRegionObject( cAllTerritories::getInstance()->region( getArgInt( 0 ), getArgInt( 1 ), getArgInt(2) ) );
}

/*!
	Returns the time in ms since the last server-start
	used for object-delays and others
*/
PyObject* wpCurrenttime( PyObject* self, PyObject* args )
{
	return PyInt_FromLong( uiCurrentTime );
}

/*!
	Returns a list of Static item at a given position
*/
PyObject *wpStatics( PyObject* self, PyObject* args )
{
	// Minimum is x, y, map
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	
	bool exact = true;
	
	if( checkArgInt( 3 ) && getArgInt( 3 ) == 0 )
		exact = false;
	
	StaticsIterator iter = Map->staticsIterator( Coord_cl( getArgInt( 0 ), getArgInt( 1 ), 0, getArgInt( 2 ) ), exact );
	
	PyObject *list = PyList_New( 0 );
	UINT32 xBlock = getArgInt( 0 ) / 8;
	UINT32 yBlock = getArgInt( 1 ) / 8;

	while( !iter.atEnd() )
	{
		// Create a Dictionary
		PyObject *dict = PyDict_New();

		PyDict_SetItemString( dict, "id", PyInt_FromLong( iter->itemid ) );
		PyDict_SetItemString( dict, "x", PyInt_FromLong( ( xBlock * 8 ) + iter->xoff ) );
		PyDict_SetItemString( dict, "y", PyInt_FromLong( ( yBlock * 8 )+ iter->yoff ) );
		PyDict_SetItemString( dict, "z", PyInt_FromLong( iter->zoff ) );

		PyList_Append( list, dict );
		iter++;
	}

	return list;
}

/*!
	Returns a list of items at a given position (sector)
*/
PyObject *wpItems( PyObject* self, PyObject* args )
{
	// Minimum is x, y, map
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	
	bool exact = true;
	
	if( checkArgInt( 3 ) && getArgInt( 3 ) == 0 )
		exact = false;
	
	Coord_cl pos( getArgInt( 0 ), getArgInt( 1 ), 0, getArgInt( 2 ) );
	RegionIterator4Items iter( pos );

	PyObject *list = PyList_New( 0 );
	UINT32 xBlock = getArgInt( 0 ) / 8;
	UINT32 yBlock = getArgInt( 1 ) / 8;

	for( iter.Begin(); !iter.atEnd(); iter++ )
	{
		P_ITEM pItem = iter.GetData();

		if( pItem->pos.map != pos.map )
			continue;

		if( exact && pItem->pos.x != pos.x && pItem->pos.y != pos.y )
			continue;

		PyList_Append( list, PyGetItemObject( pItem ) );
	}

	return list;
}

/*!
	Returns information about a given map cell.
*/
PyObject *wpMap( PyObject* self, PyObject* args )
{
	// Minimum is x, y, map
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	map_st mTile = Map->seekMap( Coord_cl( getArgInt( 0 ), getArgInt( 1 ), 0, getArgInt( 2 ) ) );

	PyObject *dict = PyDict_New();
	PyDict_SetItemString( dict, "id", PyInt_FromLong( mTile.id ) );
	PyDict_SetItemString( dict, "z", PyInt_FromLong( mTile.z ) );
	return dict;
}

/*!
	Returns the tiledata information for a item id.
*/
PyObject *wpTiledata( PyObject* self, PyObject* args )
{
	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	tile_st tile = TileCache::instance()->getTile( getArgInt( 0 ) );

	PyObject *dict = PyDict_New();
	PyDict_SetItemString( dict, "name", PyString_FromString( tile.name ) );
	PyDict_SetItemString( dict, "height", PyInt_FromLong( tile.height ) );
	PyDict_SetItemString( dict, "weight", PyInt_FromLong( tile.weight ) );
	PyDict_SetItemString( dict, "layer", PyInt_FromLong( tile.layer ) );
	PyDict_SetItemString( dict, "animation", PyInt_FromLong( tile.animation ) );
	PyDict_SetItemString( dict, "quantity", PyInt_FromLong( tile.quantity ) );
	PyDict_SetItemString( dict, "unknown1", PyInt_FromLong( tile.unknown1 ) );
	PyDict_SetItemString( dict, "unknown2", PyInt_FromLong( tile.unknown2 ) );
	PyDict_SetItemString( dict, "unknown3", PyInt_FromLong( tile.unknown3 ) );
	PyDict_SetItemString( dict, "unknown4", PyInt_FromLong( tile.unknown4 ) );
	PyDict_SetItemString( dict, "unknown5", PyInt_FromLong( tile.unknown5 ) );
	PyDict_SetItemString( dict, "flag1", PyInt_FromLong( tile.flag1 ) );
	PyDict_SetItemString( dict, "flag2", PyInt_FromLong( tile.flag2 ) );
	PyDict_SetItemString( dict, "flag3", PyInt_FromLong( tile.flag3 ) );
	PyDict_SetItemString( dict, "flag4", PyInt_FromLong( tile.flag4 ) );
	PyDict_SetItemString( dict, "wet", PyInt_FromLong( tile.isWet() ) );
	PyDict_SetItemString( dict, "blocking", PyInt_FromLong( tile.isBlocking() ) );
	PyDict_SetItemString( dict, "floor", PyInt_FromLong( tile.isRoofOrFloorTile() ) );
	
	return dict;
}

/*!
	Returns information about a certain spell
*/
PyObject *wpSpell( PyObject* self, PyObject* args )
{
	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	stNewSpell *sInfo = NewMagic->findSpell( getArgInt( 0 ) );

	if( !sInfo )
		return PyFalse;

	PyObject *dict = PyDict_New();
	PyDict_SetItemString( dict, "name", PyString_FromString( sInfo->name ) );
	PyDict_SetItemString( dict, "mantra", PyString_FromString( sInfo->mantra ) );
	PyDict_SetItemString( dict, "target", PyString_FromString( sInfo->target ) );
	PyDict_SetItemString( dict, "booklow", PyInt_FromLong( sInfo->booklow ) );
	PyDict_SetItemString( dict, "bookhigh", PyInt_FromLong( sInfo->bookhigh ) );
	PyDict_SetItemString( dict, "scrolllow", PyInt_FromLong( sInfo->scrolllow ) );
	PyDict_SetItemString( dict, "scrollhigh", PyInt_FromLong( sInfo->scrollhigh ) );
	PyDict_SetItemString( dict, "delay", PyInt_FromLong( sInfo->delay ) );
	PyDict_SetItemString( dict, "scroll", PyInt_FromLong( sInfo->scroll ) );
	PyDict_SetItemString( dict, "action", PyInt_FromLong( sInfo->action ) );
	PyDict_SetItemString( dict, "targets", PyInt_FromLong( sInfo->targets ) );
	PyDict_SetItemString( dict, "flags", PyInt_FromLong( sInfo->flags ) );
	PyDict_SetItemString( dict, "mana", PyInt_FromLong( sInfo->mana ) );

	// Reagents
	PyDict_SetItemString( dict, "ginseng", PyInt_FromLong( sInfo->reagents.ginseng ) );
	PyDict_SetItemString( dict, "bloodmoss", PyInt_FromLong( sInfo->reagents.bloodmoss ) );
	PyDict_SetItemString( dict, "mandrake", PyInt_FromLong( sInfo->reagents.mandrake ) );
	PyDict_SetItemString( dict, "blackpearl", PyInt_FromLong( sInfo->reagents.blackpearl ) );
	PyDict_SetItemString( dict, "spidersilk", PyInt_FromLong( sInfo->reagents.spidersilk ) );
	PyDict_SetItemString( dict, "sulfurash", PyInt_FromLong( sInfo->reagents.sulfurash ) );
	PyDict_SetItemString( dict, "garlic", PyInt_FromLong( sInfo->reagents.garlic ) );
	PyDict_SetItemString( dict, "nightshade", PyInt_FromLong( sInfo->reagents.nightshade ) );

	return dict;
}

/*!
	Returns a stringlist out of the definitions.
*/
PyObject *wpList( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QStringList list = DefManager->getList( getArgStr( 0 ) );
	PyObject *pylist = PyList_New( list.count() );

	for( int i = 0; i < list.count(); ++i )
		PyList_SetItem( pylist, i, PyString_FromString( list[i].latin1() ) );

	return pylist;
}

/*!
	wolfpack
	Initializes wolfpack
*/
static PyMethodDef wpGlobal[] = 
{
    { "additem",		wpAdditem,		METH_VARARGS, "Adds an item with the specified script-section" },
	{ "addnpc",			wpAddnpc,		METH_VARARGS, "Adds a npc with the specified script-section" },
	{ "finditem",		wpFinditem,		METH_VARARGS, "Tries to find an item based on it's serial" },
	{ "findchar",		wpFindchar,		METH_VARARGS, "Tries to find a char based on it's serial" },
	{ "addtimer",		wpAddtimer,		METH_VARARGS, "Adds a timed effect" },
	{ "region",			wpRegion,		METH_VARARGS, "Gets the region at a specific position" },
	{ "currenttime",	wpCurrenttime,	METH_VARARGS, "Time in ms since server-start" },
	{ "statics",		wpStatics,		METH_VARARGS, "Returns a list of static-item at a given position" },
	{ "map",			wpMap,			METH_VARARGS, "Retruns a dictionary with information about a given map tile" },
	{ "items",			wpItems,		METH_VARARGS, "Returns a list of items in a specific sector." },
	{ "tiledata",		wpTiledata,		METH_VARARGS, "Returns the tiledata information for a given tile stored on the server." },
	{ "spell",			wpSpell,		METH_VARARGS, "Returns information about a certain spell." },
	{ "list",			wpList,			METH_VARARGS, "Returns a list defined in the definitions as a Python List" },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	Adds a speech keyword to a wolfpack script object.
*/
PyObject *wpSpeechAddKeyword( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) || !checkArgInt( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	WPDefaultScript *script = ScriptManager->find( getArgStr( 0 ) );

	if( !script )
	{
		PyErr_Warn( PyExc_Warning, "Use of unknown script-id." );
		return PyFalse;
	}
	
	script->addKeyword( getArgInt( 1 ) );

	return PyTrue;
}

/*!
	Adds a speech trigger-word to a wolfpack script object.
*/
PyObject *wpSpeechAddWord( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) || !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	WPDefaultScript *script = ScriptManager->find( getArgStr( 0 ) );

	if( !script )
	{
		PyErr_Warn( PyExc_Warning, "Use of unknown script-id." );
		return PyFalse;
	}
	
	script->addWord( getArgStr( 1 ) );

	return PyTrue;
}

/*!
	Adds a speech regular expression to a wolfpack script object.
*/
PyObject *wpSpeechAddRegexp( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) || !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	WPDefaultScript *script = ScriptManager->find( getArgStr( 0 ) );

	if( !script )
	{
		PyErr_Warn( PyExc_Warning, "Use of unknown script-id." );
		return PyFalse;
	}
	
	script->addRegexp( QRegExp( getArgStr( 1 ) ) );

	return PyTrue;
}

/*!
	Specifies if a speech script should fetch all speech events and not just one case.
*/
PyObject *wpSpeechSetCatchAll( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) || !checkArgInt( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	WPDefaultScript *script = ScriptManager->find( getArgStr( 0 ) );

	if( !script )
	{
		PyErr_Warn( PyExc_Warning, "Use of unknown script-id." );
		return PyFalse;
	}
	
	script->setCatchAllSpeech( getArgInt( 1 ) != 0 );

	return PyTrue;
}

/*!
	wolfpack.speech
	speech related functions
*/
static PyMethodDef wpSpeech[] = 
{
    { "addKeyword",		wpSpeechAddKeyword, METH_VARARGS, "Adds a keyword to a specific speech script." },
	{ "addWord",		wpSpeechAddWord,	METH_VARARGS, "Adds a triggerword to a specific speech script." },
	{ "addRegexp",		wpSpeechAddRegexp, METH_VARARGS, "Adds a regular expression to a specific speech script." },
	{ "setCatchAll",	wpSpeechSetCatchAll, METH_VARARGS, "Specifies if a speech script should fetch all speech events and not just one case." },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	Finds an Account object.
 */
PyObject *wpAccountsFind( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	AccountRecord* account = Accounts::instance()->getRecord( getArgStr( 0 ) );
	return PyGetAccountObject( account );
}

/*!
	Gets a list of Account names.
 */
PyObject *wpAccountsList( PyObject* self, PyObject* args )
{
	PyObject *list = PyList_New( 0 );

	cAccounts::const_iterator it = Accounts::instance()->begin();
	while( it != Accounts::instance()->end() )
	{
		QString login = (*it)->login();
		if( login != QString::null )
			PyList_Append( list, PyString_FromString( login.latin1() ) );
		++it;
	}

	return list;
}

/*!
	Gets a list of ACL names.
 */
PyObject *wpAccountsAcls( PyObject* self, PyObject* args )
{
	PyObject *list = PyList_New( 0 );

	QMap< QString, cAcl* >::const_iterator it = cCommands::instance()->aclbegin();
	while( it != cCommands::instance()->aclend() )
	{
		QString name = it.key();
		if( name != QString::null )
			PyList_Append( list, PyString_FromString( name.latin1() ) );
		++it;
	}

	return list;
}

/*!
	Returns an ACL as a dictionary.
 */
PyObject *wpAccountsAcl( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) )
		return Py_None;

	cAcl *acl = cCommands::instance()->getACL( getArgStr( 0 ) );
	if( !acl )
		return Py_None;

	PyObject *dict = PyDict_New();
	
	QMap< QString, QMap< QString, bool > >::iterator git;
	for( git = acl->groups.begin(); git != acl->groups.end(); ++git )
	{
		PyObject *dict2 = PyDict_New();

		for( QMap< QString, bool >::iterator it = (*git).begin(); it != (*git).end(); ++it )
			PyDict_SetItem( dict2, PyString_FromString( it.key().latin1() ), it.data() ? PyTrue : PyFalse );

		PyDict_SetItem( dict, PyString_FromString( git.key().latin1() ), dict2 );
	}

	return dict;
}

/*!
	Creates an account (username + password is enough)
 */
PyObject *wpAccountsAdd( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) && !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QString login = getArgStr( 0 );
	QString password = getArgStr( 1 );

	if( login.length() < 1 && password.length() < 1 )
		return PyFalse;

	AccountRecord *account = Accounts::instance()->getRecord( login );

	if( account )
		return PyFalse;

	account = Accounts::instance()->createAccount( login, password );
	return PyGetAccountObject( account );
}

/*!
	wolfpack.accounts
	account related functions
*/
static PyMethodDef wpAccounts[] = 
{
    { "find",		wpAccountsFind, METH_VARARGS, "Finds an account object." },
	{ "list",		wpAccountsList, METH_VARARGS, "Gets a list of Account names." },
	{ "acls",		wpAccountsAcls,	METH_VARARGS, "Gets a list of valid ACL names." },
	{ "acl",		wpAccountsAcl,	METH_VARARGS, "Returns an acl as a double dictionary." },
	{ "add",		wpAccountsAdd,	METH_VARARGS, "Creates an account." },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	This initializes the wolfpack.console extension
*/
void init_wolfpack_globals()
{
	PyObject *wpNamespace = Py_InitModule( "_wolfpack", wpGlobal );

	PyObject *mConsole = Py_InitModule( "_wolfpack.console", wpConsole );
    PyObject_SetAttrString( wpNamespace, "console", mConsole );

	PyObject *mSpeech = Py_InitModule( "_wolfpack.speech", wpSpeech );
    PyObject_SetAttrString( wpNamespace, "speech", mSpeech );

	PyObject *mAccounts = Py_InitModule( "_wolfpack.accounts", wpAccounts );
    PyObject_SetAttrString( wpNamespace, "accounts", mAccounts );

	PyObject *mTime = Py_InitModule( "_wolfpack.time", wpTime );
    PyObject_SetAttrString( wpNamespace, "time", mTime );
}

void tuple_incref( PyObject *object )
{
	Py_INCREF( object );
	if( PyTuple_Check( object ) )
	{
		for( UINT32 i = 0; i < PyTuple_Size( object ); ++i )
			tuple_incref( PyTuple_GetItem( object, i ) );
	}
}

void tuple_decref( PyObject *object )
{	
	if( PyTuple_Check( object ) )
	{
		for( UINT32 i = 0; i < PyTuple_Size( object ); ++i )
			tuple_decref( PyTuple_GetItem( object, i ) );
	}
	Py_DECREF( object );
}
