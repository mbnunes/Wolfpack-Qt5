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

#include "engine.h"

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
#include "../verinfo.h"
#include "../globals.h"
#include "../items.h"
#include "../network.h"
#include "../multis.h"
#include "../house.h"
#include "../boats.h"
#include "../srvparams.h"
#include "../chars.h"
#include "../basechar.h"
#include "../player.h"
#include "../npc.h"
#include "../targetrequests.h"

#include "regioniterator.h"
#include "utilities.h"
#include "tempeffect.h"

// Library Includes
#include <qdatetime.h>
#include <qmutex.h>


/*!
	Sends a string to the wolfpack console.
*/
PyObject* wpConsole_send( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
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
	Q_UNUSED(self);	
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
PyObject* wpConsole_progressDone( PyObject* self )
{
	Q_UNUSED(self);	
	clConsole.ProgressDone();
	return PyInt_FromLong( 1 );
}

/*!
	Sends a [fail] progress section to the console
*/
PyObject* wpConsole_progressFail( PyObject* self )
{
	Q_UNUSED(self);	
	clConsole.ProgressFail();
	return PyInt_FromLong( 1 );
}

/*!
	Sends a [skip] progress section to the console
*/
PyObject* wpConsole_progressSkip( PyObject* self )
{
	Q_UNUSED(self);	
	clConsole.ProgressSkip();
	return PyInt_FromLong( 1 );
}

/*!
	Returns a list of Strings (the linebuffer)
*/
PyObject* wpConsole_getbuffer( PyObject* self )
{	
	Q_UNUSED(self);	
	QStringList linebuffer = clConsole.linebuffer();
	PyObject *list = PyList_New( linebuffer.count() );

	for( uint i = 0; i < linebuffer.count(); ++i )
		if( linebuffer[i].isNull() )
			PyList_SetItem( list, i, PyString_FromString( "" ) );
		else
			PyList_SetItem( list, i, PyString_FromString( linebuffer[i].latin1() ) );

	return list;
}

extern QStringList commandQueue;
extern QMutex commandMutex;
PyObject* wpConsole_reloadScripts( PyObject* self )
{
	Q_UNUSED(self);
	// Temporary implementation while thread comunication is not done
	QMutexLocker lock(&commandMutex);

	if ( !secure )
		commandQueue.push_back( "R" );
	else
	{
		commandQueue.push_back("S");
		commandQueue.push_back("R");
		commandQueue.push_back("S");
	}
	
	return PyInt_FromLong( 1 );
}

/*!
	wolfpack.console
	Initializes wolfpack.console
*/
static PyMethodDef wpConsole[] = 
{
    { "send",			wpConsole_send,							METH_VARARGS, "Prints something to the wolfpack console" },
	{ "progress",		wpConsole_progress,						METH_VARARGS, "Prints a .....[xxxx] block" },
	{ "progressDone",	(PyCFunction)wpConsole_progressDone,	METH_NOARGS, "Prints a [done] block" },
	{ "progressFail",	(PyCFunction)wpConsole_progressFail,	METH_NOARGS, "Prints a [fail] block" },
	{ "progressSkip",	(PyCFunction)wpConsole_progressSkip,	METH_NOARGS, "Prints a [skip] block" },
	{ "getbuffer",		(PyCFunction)wpConsole_getbuffer,		METH_NOARGS, "Gets the linebuffer of the console" },
	{ "reloadScripts",	(PyCFunction)wpConsole_reloadScripts,	METH_NOARGS, "Reloads Scripts and Definitions"	},
    { NULL, NULL, 0, NULL } // Terminator
};

/*!
	Gets the seconds of the current uo time
*/
PyObject* wpTime_second( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.time().second() );
}

/*!
	Gets the minutes of the current uo time
*/
PyObject* wpTime_minute( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.time().minute() );
}

/*!
	Gets the hours of the current uo time
*/
PyObject* wpTime_hour( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.time().hour() );
}

/*!
	Gets the current day
*/
PyObject* wpTime_day( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.date().day() );
}

/*!
	Gets the current month
*/
PyObject* wpTime_month( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.date().month() );
}

/*!
	Gets the current year
*/
PyObject* wpTime_year( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.date().year() );
}

/*!
	Gets a timestamp of the current uo time
*/
PyObject* wpTime_timestamp( PyObject* self )
{
	Q_UNUSED(self);	
	return PyInt_FromLong( uoTime.time().elapsed() );
}

/*!
	Methods for handling UO Time from within python
*/
static PyMethodDef wpTime[] = 
{
	{ "second",			(PyCFunction)wpTime_second,		METH_NOARGS, "Returns the current time-seconds" },
	{ "minute",			(PyCFunction)wpTime_minute,		METH_NOARGS, "Returns the current time-minutes" },
	{ "hour",			(PyCFunction)wpTime_hour,		METH_NOARGS, "Returns the current time-hour" },
	{ "day",			(PyCFunction)wpTime_day,		METH_NOARGS, "Returns the current date-day" },
	{ "month",			(PyCFunction)wpTime_month,		METH_NOARGS, "Returns the current date-month" },
	{ "year",			(PyCFunction)wpTime_year,		METH_NOARGS, "Returns the current date-year" },
	{ "timestamp",		(PyCFunction)wpTime_timestamp,	METH_NOARGS, "Returns the current timestamp" },
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
	Q_UNUSED(self);	
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
	Q_UNUSED(self);	
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
	Q_UNUSED(self);	
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
	Q_UNUSED(self);
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	SERIAL serial = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	return PyGetCharObject( FindCharBySerial( serial ) );
}

/*
 * Creates a multi object based on the passed pos
 */
PyObject* wpFindmulti( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	if( PyTuple_Size( args ) < 1 )
	{
		PyErr_BadArgument();
		return NULL;
	}
	P_MULTI pMulti = NULL;
	if(  !checkArgCoord( 0 ) )
		if( !checkArgInt( 0 ) )
		{
			PyErr_BadArgument();
			return NULL;
		}
		else
			pMulti = dynamic_cast< cMulti* >( FindItemBySerial( getArgInt( 0 ) ) );
	else
		pMulti = cMulti::findMulti( getArgCoord( 0 ) );

	return PyGetMultiObject( pMulti );

}

/*!
	Adds a tempeffect
*/
PyObject* wpAddtimer( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
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
	Q_UNUSED(self);
	// Three arguments
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyGetRegionObject( AllTerritories::instance()->region( getArgInt( 0 ), getArgInt( 1 ), getArgInt(2) ) );
}

/*!
	Returns the time in ms since the last server-start
	used for object-delays and others
*/
PyObject* wpCurrenttime( PyObject* self )
{
	Q_UNUSED(self);
	return PyInt_FromLong( uiCurrentTime );
}

/*!
	Returns a list of Static item at a given position
*/
PyObject *wpStatics( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
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
	Returns a list of all items serials 
*/
PyObject *wpAllItemsSerials( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	cItemIterator iter;
	PyObject *list = PyList_New( 0 );
	for( P_ITEM pItem = iter.first(); pItem; pItem = iter.next() )
		PyList_Append( list, PyInt_FromLong( pItem->serial() ) );

	return list;
}

/*!
	Returns a list of all chars serials 
*/
PyObject *wpAllCharsSerials( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	cCharIterator iter;
	PyObject *list = PyList_New( 0 );

	for( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
		PyList_Append( list, PyInt_FromLong( pChar->serial() ) );

	return list;
}
/*!
	Returns a list of items at a given position (sector)
*/
PyObject *wpItems( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Minimum is x, y, map
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	
	UINT32 range = 1;

	if( checkArgInt( 3 ) && getArgInt( 3 ) > 1 )
		range = getArgInt( 3 );
	
	Coord_cl pos( getArgInt( 0 ), getArgInt( 1 ), 0, getArgInt( 2 ) );
	RegionIterator4Items iter( pos, range );

	PyObject *list = PyList_New( 0 );
	for( iter.Begin(); !iter.atEnd(); iter++ )
	{
		P_ITEM pItem = iter.GetData();

		if( pItem )
			PyList_Append( list, PyGetItemObject( pItem ) );
	}

	return list;
}

/*!
	Returns a list of chars at a given position (sector)
*/
PyObject *wpChars( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Minimum is x, y, map
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	
	UINT32 range = 1;
	
	if( checkArgInt( 3 ) && getArgInt( 3 ) > 1 )
		range = getArgInt( 3 );
	
	Coord_cl pos( getArgInt( 0 ), getArgInt( 1 ), 0, getArgInt( 2 ) );
	RegionIterator4Chars iter( pos, range );

	PyObject *list = PyList_New( 0 );
	for( iter.Begin(); !iter.atEnd(); iter++ )
	{
		P_CHAR pChar = iter.GetData();

		if( pChar )
			PyList_Append( list, PyGetCharObject( pChar ) );
	}

	return list;
}

/*!
	Shows a graphical effect at a certain position
*/
PyObject *wpEffect( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	
	// effect-id, position, speed, duration
	if( !checkArgInt( 0 ) || !checkArgCoord( 1 ) || !checkArgInt( 2 ) || !checkArgInt( 3 ) )
	{
		PyErr_BadArgument();
		return 0;
	}	

	cUOTxEffect effect;
	effect.setType( ET_STAYSOURCEPOS );
	effect.setId( getArgInt( 0 ) );
	effect.setSourcePos( getArgCoord( 1 ) );
	effect.setDuration( getArgInt( 3 ) );
	effect.setSpeed( getArgInt( 2 ) );

	Coord_cl displaypos = getArgCoord( 1 );

	cUOSocket *mSock;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && mSock->player()->pos().distance( displaypos ) <= mSock->player()->visualRange() )
			mSock->send( &effect );
	}

	return PyTrue;
}

/*!
	Returns information about a given map cell.
*/
PyObject *wpMap( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
	PyDict_SetItemString( dict, "flagnames", PyString_FromString( getFlagNames( tile ).join(",") ) );
	
	return dict;
}

/*!
	Returns information about a certain spell
*/
PyObject *wpSpell( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
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
	Q_UNUSED(self);
	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QStringList list = DefManager->getList( getArgStr( 0 ) );
	PyObject *pylist = PyList_New( list.count() );

	for( uint i = 0; i < list.count(); ++i )
		PyList_SetItem( pylist, i, PyString_FromString( list[i].latin1() ) );

	return pylist;
}

/*!
	Registers a global script hook.
*/
PyObject *wpRegisterGlobal( PyObject* self, PyObject* args )
{
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgStr( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	UINT32 object = getArgInt( 0 );
	UINT32 event = getArgInt( 1 );
	QString scriptName = getArgStr( 2 );

	WPDefaultScript *script = ScriptManager->find( scriptName );

	if( script == 0 )
	{
		PyErr_SetString( PyExc_RuntimeError, "Unknown script." );
		return 0;
	}

	ScriptManager->addGlobalHook( object, event, script );
	return PyTrue;
}

/*!
	Registers a global command hook.
*/
PyObject *wpRegisterCommand( PyObject* self, PyObject* args )
{
	if( !checkArgStr( 0 ) || !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QString command = getArgStr( 0 );
	QString scriptName = getArgStr( 1 );

	WPDefaultScript *script = ScriptManager->find( scriptName );

	if( script == 0 )
	{
		PyErr_SetString( PyExc_RuntimeError, "Unknown script." );
		return 0;
	}

	ScriptManager->addCommandHook( command, script );
	return PyTrue;
}

/*!
	Coord object creation
*/
PyObject *wpCoord( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) || !checkArgInt( 3 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	Coord_cl pos;
	pos.x = getArgInt( 0 );
	pos.y = getArgInt( 1 );
	pos.z = getArgInt( 2 );
	pos.map = getArgInt( 3 );

	return PyGetCoordObject( pos );
}

/*!
	Multi object creation
*/
PyObject *wpMulti( PyObject* self, PyObject* args )
{
	Q_UNUSED( self);

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	uint type = getArgInt( 0 );
	P_MULTI pMulti = NULL;

	switch( type )
	{
		case CUSTOMHOUSE: pMulti = dynamic_cast< cMulti* >( new cHouse( true ) );  // Custom house have serial for caching purposes
			break;
		case HOUSE: pMulti = dynamic_cast< cMulti* >( new cHouse() );  // Common house
			break;
		case BOAT: pMulti = dynamic_cast< cMulti* >( new cBoat() );
			break;
		default:
			break;
	}

	return PyGetMultiObject( pMulti );
}

/*!
	Returns uptime of server in seconds
*/
PyObject* wpServerUptime( PyObject* self )
{
	Q_UNUSED(self);
	return PyInt_FromLong( uiCurrentTime / MY_CLOCKS_PER_SEC );
}

/*!
	Returns the server version string
*/
PyObject* wpServerVersion( PyObject* self )
{
	Q_UNUSED(self);
	wp_version_info wpversioninfo;
	return PyString_FromString( QString("%1 %2 %3").arg( wpversioninfo.productstring.c_str() ).arg( wpversioninfo.betareleasestring.c_str() ).arg( wpversioninfo.verstring.c_str() ).latin1() );
}

/*!
	Returns the current real date/time
*/
PyObject* wpCurrentdatetime( PyObject* self )
{
	Q_UNUSED(self);	
	return PyString_FromString( QDateTime::currentDateTime().toString() );
}

/*!
	Returns if the server is in starting state
*/
PyObject* wpIsStarting( PyObject* self )
{
	Q_UNUSED(self);
	if( serverState == STARTUP )
		return PyTrue;
	else
		return PyFalse;
}

/*!
	Returns if the server is in running state
*/
PyObject* wpIsRunning( PyObject* self )
{
	Q_UNUSED(self);
	if( serverState == RUNNING )
		return PyTrue;
	else
		return PyFalse;
}

/*!
	Returns if the server is in reload state
*/
PyObject* wpIsReloading( PyObject* self )
{
	Q_UNUSED(self);
	if( serverState == SCRIPTRELOAD )
		return PyTrue;
	else
		return PyFalse;
}

/*!
	Returns if the server is in closing state
*/
PyObject* wpIsClosing( PyObject* self )
{
	Q_UNUSED(self);
	if( serverState == SHUTDOWN )
		return PyTrue;
	else
		return PyFalse;
}

static PyObject* wpCharBlock( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	unsigned int xBlock, yBlock;
	unsigned char map;

	if( !PyArg_ParseTuple( args, "iib:wolfpack.charblock", &xBlock, &yBlock, &map ) )
		return 0;


	return PyGetCharRegionIterator( x1, y1, x2, y2, map );
}

static PyObject* wpItemBlock( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	unsigned int x1, y1, x2, y2;
	unsigned char map;

	if( !PyArg_ParseTuple( args, "iiiib:wolfpack.charregion", &x1, &y1, &x2, &y2, &map ) )
		return 0;

	return PyGetCharRegionIterator( x1, y1, x2, y2, map );
}

static PyObject* wpItemRegion( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	unsigned int x1, y1, x2, y2;
	unsigned char map;

	if( !PyArg_ParseTuple( args, "iiiib:wolfpack.itemregion", &x1, &y1, &x2, &y2, &map ) )
		return 0;

	return PyGetItemRegionIterator( x1, y1, x2, y2, map );
}

static PyObject* wpNewItem( PyObject *self, PyObject *args )
{
	char createSerial = 1;

	if( !PyArg_ParseTuple( args, "|b:wolfpack.newitem", &createSerial ) )
		return 0;

	P_ITEM pItem = new cItem;
	
	if( createSerial )
		pItem->Init( true );

	return PyGetItemObject( pItem );
}

static PyObject* wpNewNpc( PyObject *self, PyObject *args )
{
	char createSerial = 1;

	if( !PyArg_ParseTuple( args, "|b:wolfpack.newnpc", &createSerial ) )
		return 0;

	P_NPC pNpc = new cNPC;
	
	if( createSerial )
		pNpc->Init( true );

	return PyGetCharObject( pNpc );
}

static PyObject* wpNewPlayer( PyObject *self, PyObject *args )
{
	char createSerial = 1;

	if( !PyArg_ParseTuple( args, "|b:wolfpack.newplayer", &createSerial ) )
		return 0;

	P_PLAYER pPlayer = new cPlayer;
	
	if( createSerial )
		pPlayer->Init( true );

	return PyGetCharObject( pPlayer );
}

static PyObject* wpTickcount( PyObject* self )
{
	Q_UNUSED(self);
#if defined( Q_OS_WIN32 )
	return PyInt_FromLong( GetTickCount() );
#else
	return PyInt_FromLong( 0 );
#endif
}

/*!
	wolfpack
	Initializes wolfpack
*/
static PyMethodDef wpGlobal[] = 
{
	{ "charblock",			wpCharBlock,					METH_VARARGS, NULL },
	{ "itemblock",			wpItemBlock,					METH_VARARGS, NULL },
	{ "charregion",			wpCharRegion,					METH_VARARGS, NULL },
	{ "itemregion",			wpItemRegion,					METH_VARARGS, NULL },
	{ "additem",			wpAdditem,						METH_VARARGS, "Adds an item with the specified script-section" },
	{ "newnpc",				wpNewNpc,						METH_VARARGS, "Creates an entirely new npc." },
	{ "newitem",			wpNewItem,						METH_VARARGS, "Creates an entirely new item." },
	{ "newplayer",			wpNewPlayer,					METH_VARARGS, "Creates an entirely new player." },
	{ "addnpc",				wpAddnpc,						METH_VARARGS, "Adds a npc with the specified script-section" },
	{ "finditem",			wpFinditem,						METH_VARARGS, "Tries to find an item based on it's serial" },
	{ "findchar",			wpFindchar,						METH_VARARGS, "Tries to find a char based on it's serial" },
	{ "findmulti",			wpFindmulti,					METH_VARARGS, "Tries to find a multi based on it's position" },
	{ "addtimer",			wpAddtimer,						METH_VARARGS, "Adds a timed effect" },
	{ "effect",				wpEffect,						METH_VARARGS, "Shows a graphical effect." },
	{ "region",				wpRegion,						METH_VARARGS, "Gets the region at a specific position" },
	{ "currenttime",		(PyCFunction)wpCurrenttime,		METH_NOARGS, "Time in ms since server-start" },
	{ "statics",			wpStatics,						METH_VARARGS, "Returns a list of static-item at a given position" },
	{ "map",				wpMap,							METH_VARARGS, "Retruns a dictionary with information about a given map tile" },
	{ "items",				wpItems,						METH_VARARGS, "Returns a list of items in a specific sector." },
	{ "chars",				wpChars,						METH_VARARGS, "Returns a list of chars in a specific sector." },
	{ "allcharsserials",	wpAllCharsSerials,				METH_VARARGS, "Returns a list of all chars serials" },
	{ "allitemsserials",	wpAllItemsSerials,				METH_VARARGS, "Returns a list of all items serials" },
	{ "tiledata",			wpTiledata,						METH_VARARGS, "Returns the tiledata information for a given tile stored on the server." },
	{ "coord",				wpCoord,						METH_VARARGS, "Creates a coordinate object from the given parameters (x,y,z,map)." },
	{ "multi",				wpMulti,						METH_VARARGS, "Creates a multi object by given type CUSTOMHOUSE, HOUSE, BOAT." },
	{ "spell",				wpSpell,						METH_VARARGS, "Returns information about a certain spell." },
	{ "list",				wpList,							METH_VARARGS, "Returns a list defined in the definitions as a Python List" },
	{ "registerglobal",		wpRegisterGlobal,				METH_VARARGS, "Registers a global script hook." },
	{ "registercommand",	wpRegisterCommand,				METH_VARARGS, "Registers a global command hook." },
	{ "serveruptime",		(PyCFunction)wpServerUptime,	METH_NOARGS, "Returns uptime of server in seconds." },
	{ "serverversion",		(PyCFunction)wpServerVersion,	METH_NOARGS, "Returns the server version string." },
	{ "currentdatetime",	(PyCFunction)wpCurrentdatetime,	METH_NOARGS, "Returns the current real date/time" },
	{ "isstarting",			(PyCFunction)wpIsStarting,		METH_NOARGS, "Returns if the server is in starting state" },
	{ "isrunning",			(PyCFunction)wpIsRunning,		METH_NOARGS, "Returns if the server is in running state" },
	{ "isreloading",		(PyCFunction)wpIsReloading,		METH_NOARGS, "Returns if the server is in reload state" },
	{ "isclosing",			(PyCFunction)wpIsClosing,		METH_NOARGS, "Returns if the server is in closing state" },
	{ "tickcount",			(PyCFunction)wpTickcount,		METH_NOARGS, "Returns the current Tickcount on Windows" },
	{ NULL, NULL, 0, NULL } // Terminator
};

PyObject *wpSocketsFirst( PyObject* self )
{
	Q_UNUSED(self);
	return PyGetSocketObject( cNetwork::instance()->first() );
}

PyObject *wpSocketsNext( PyObject* self )
{
	Q_UNUSED(self);
	return PyGetSocketObject( cNetwork::instance()->next() );  
}

PyObject *wpSocketsCount( PyObject* self )
{
	Q_UNUSED(self);
	return PyInt_FromLong( cNetwork::instance()->count() );
}

/*!
	wolfpack.sockets
	Socket iteration
*/
static PyMethodDef wpSockets[] = 
{
	{ "first",			(PyCFunction)wpSocketsFirst,	METH_NOARGS, "Returns the first connected socket." },
	{ "next",			(PyCFunction)wpSocketsNext,		METH_NOARGS, "Returns the next connected socket." },
	{ "count",			(PyCFunction)wpSocketsCount,	METH_NOARGS, "Returns the number of connected sockets." },
	{ NULL, NULL, 0, NULL } // Terminator
};


/*!
	Adds a speech keyword to a wolfpack script object.
*/
PyObject *wpSpeechAddKeyword( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
PyObject *wpAccountsList( PyObject* self )
{
	Q_UNUSED(self);
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
PyObject *wpAccountsAcls( PyObject* self )
{
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
	Q_UNUSED(self);
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
	Reload accounts.
 */
PyObject *wpAccountsReload( PyObject* self )
{
	Q_UNUSED(self);
	Accounts::instance()->reload();
	return PyTrue;
}

/*!
	Save accounts.
 */
PyObject *wpAccountsSave( PyObject* self )
{
	Q_UNUSED(self);
	Accounts::instance()->save();
	return PyTrue;
}

/*!
	wolfpack.accounts
	account related functions
*/
static PyMethodDef wpAccounts[] = 
{
    { "find",		wpAccountsFind,					METH_VARARGS, "Finds an account object." },
	{ "list",		(PyCFunction)wpAccountsList,	METH_NOARGS, "Gets a list of Account names." },
	{ "acls",		(PyCFunction)wpAccountsAcls,	METH_NOARGS, "Gets a list of valid ACL names." },
	{ "acl",		wpAccountsAcl,					METH_VARARGS, "Returns an acl as a double dictionary." },
	{ "add",		wpAccountsAdd,					METH_VARARGS, "Creates an account." },
	{ "save",		(PyCFunction)wpAccountsSave,	METH_NOARGS, "Save accounts." },
	{ "reload",		(PyCFunction)wpAccountsReload,	METH_NOARGS, "Reload accounts." },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	Reads a boolean value from wolfpack.xml
 */
PyObject *wpSettingsGetBool( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if( !checkArgStr( 0 ) && !checkArgStr( 1 ) && !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyInt_FromLong( SrvParams->getBool( getArgStr(0), getArgStr(1), getArgInt(2) ) );
}

/*!
	Writes a boolean value to wolfpack.xml
*/
PyObject *wpSettingsSetBool( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if ( !checkArgStr(0) && !checkArgStr( 1 ) && !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return PyFalse;
	}
	SrvParams->setBool( getArgStr(0), getArgStr(1), getArgInt(2) );
	return PyTrue;
}

/*!
	Reads a numeric value from wolfpack.xml
 */
PyObject *wpSettingsGetNumber( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if( !checkArgStr( 0 ) && !checkArgStr( 1 ) && !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	return PyInt_FromLong( SrvParams->getNumber( getArgStr(0), getArgStr(1), getArgInt(2) ) );
}

/*!
	Writes a boolean value to wolfpack.xml
*/
PyObject *wpSettingsSetNumber( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if ( !checkArgStr(0) && !checkArgStr( 1 ) && !checkArgInt( 2 ) )
	{
		PyErr_BadArgument();
		return PyFalse;
	}
	SrvParams->setNumber( getArgStr(0), getArgStr(1), getArgInt(2) );
	return PyTrue;
}

/*!
	Reads a numeric value from wolfpack.xml
 */
PyObject *wpSettingsGetString( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if( !checkArgStr( 0 ) && !checkArgStr( 1 ) && !checkArgStr( 2 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	return PyString_FromString( SrvParams->getString( getArgStr(0), getArgStr(1), getArgStr(2) ) );
}

/*!
	Writes a boolean value to wolfpack.xml
*/
PyObject *wpSettingsSetString( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if ( !checkArgStr(0) && !checkArgStr( 1 ) && !checkArgStr( 2 ) )
	{
		PyErr_BadArgument();
		return PyFalse;
	}
	SrvParams->setString( getArgStr(0), getArgStr(1), getArgStr(2) );
	return PyTrue;
}

/*!
	Reloads wolfpack.xml
*/
PyObject* wpSettingsReload( PyObject* self )
{
	Q_UNUSED(self);
	SrvParams->reload();
	return PyTrue;
}

/*!
	Saves wolfpack.xml
*/
PyObject* wpSettingsSave( PyObject* self )
{
	Q_UNUSED(self);
	SrvParams->flush();
	return PyTrue;
}

/*!
	wolfpack.settings
	wolfpack.xml config related functions
*/
static PyMethodDef wpSettings[] = 
{
    { "getBool",		wpSettingsGetBool,				METH_VARARGS, "Reads a boolean value from wolfpack.xml." },
	{ "setBool",		wpSettingsSetBool,				METH_VARARGS, "Sets a boolean value to wolfpack.xml." },
	{ "getNumber",		wpSettingsGetNumber,			METH_VARARGS, "Gets a numeric value from wolfpack.xml." },
	{ "setNumber",		wpSettingsSetNumber,			METH_VARARGS, "Sets a numeric value to wolfpack.xml." },
	{ "getString",		wpSettingsGetString,			METH_VARARGS, "Reads a string value from wolfpack.xml." },
	{ "setString",		wpSettingsSetString,			METH_VARARGS, "Writes a string value to wolfpack.xml." },
	{ "reload",			(PyCFunction)wpSettingsReload,	METH_NOARGS, "Reloads wolfpack.xml." },
	{ "save",			(PyCFunction)wpSettingsSave,	METH_NOARGS, "Saves changes made to wolfpack.xml"	},
	{ NULL, NULL, 0, NULL } // Terminator
};


/*!
	This initializes the _wolfpack namespace and it's sub extensions
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

	PyObject *mSockets = Py_InitModule( "_wolfpack.sockets", wpSockets );
    PyObject_SetAttrString( wpNamespace, "sockets", mSockets );

	PyObject *mTime = Py_InitModule( "_wolfpack.time", wpTime );
    PyObject_SetAttrString( wpNamespace, "time", mTime );

	PyObject *mSettings = Py_InitModule( "_wolfpack.settings", wpSettings );
	PyObject_SetAttrString( wpNamespace, "settings", mSettings );
}
