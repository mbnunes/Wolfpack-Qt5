//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#include "../basics.h"
#include "../globals.h"
#include "../network/uosocket.h"
#include "../network/uotxpackets.h"
#include "../console.h"
#include "../guilds.h"
#include "../uotime.h"
#include "../TmpEff.h"
#include "../sectors.h"
#include "../territories.h"
#include "../maps.h"
#include "../tilecache.h"
#include "../accounts.h"
#include "../commands.h"
#include "../multi.h"
#include "../scriptmanager.h"
#include "../wpdefmanager.h"
#include "../pythonscript.h"
#include "../verinfo.h"
#include "../globals.h"
#include "../items.h"
#include "../network.h"
#include "../srvparams.h"
#include "../basechar.h"
#include "../player.h"
#include "../npc.h"
#include "../targetrequests.h"
#include "../basedef.h"
#include "../wolfpack.h"

#include "pypacket.h"
#include "regioniterator.h"
#include "utilities.h"
#include "tempeffect.h"
#include "worlditerator.h"

// Library Includes
#include <qdatetime.h>
#include <qmutex.h>

PyObject *PyGetObjectObject( cUObject *object )
{
	if( dynamic_cast< P_ITEM >( object ) )
		return PyGetItemObject( (P_ITEM)object );
	else if( dynamic_cast< P_CHAR >( object ) )
		return PyGetCharObject( (P_CHAR)object );

	return 0;
}

static QStringList getFlagNames(unsigned char flag1, unsigned char flag2, unsigned char flag3, unsigned char flag4)
{
#define FLAG_STUB( a, b, c ) if( a & b ) flags.push_back( tr( c ) )
	QStringList flags;

	// Flag 1
	FLAG_STUB( flag1, 0x01, "background" );
	FLAG_STUB( flag1, 0x02, "weapon" );
	FLAG_STUB( flag1, 0x04, "transparent" );
	FLAG_STUB( flag1, 0x08, "translucent" );
	FLAG_STUB( flag1, 0x10, "wall" );
	FLAG_STUB( flag1, 0x20, "damaging" );
	FLAG_STUB( flag1, 0x40, "impassable" );
	FLAG_STUB( flag1, 0x80, "wet" );

	// Flag 2
	//FLAG_STUB( flag2, 0x01, "unknown1" ); 
	FLAG_STUB( flag2, 0x02, "surface" );
	FLAG_STUB( flag2, 0x04, "stairs" );
	FLAG_STUB( flag2, 0x08, "stackable" );
	FLAG_STUB( flag2, 0x10, "window" );
	FLAG_STUB( flag2, 0x20, "no shoot" );
	FLAG_STUB( flag2, 0x40, "a" );
	FLAG_STUB( flag2, 0x80, "an" );

	// Flag 3
	FLAG_STUB( flag3, 0x01, "internal" ); 
	FLAG_STUB( flag3, 0x02, "foliage" );
	FLAG_STUB( flag3, 0x04, "partial hue" );
	//FLAG_STUB( flag3, 0x08, "unknown2" );
	FLAG_STUB( flag3, 0x10, "map" );
	FLAG_STUB( flag3, 0x20, "container" );
	FLAG_STUB( flag3, 0x40, "wearable" );
	FLAG_STUB( flag3, 0x80, "lightsource" );

	// Flag 4
	FLAG_STUB( flag4, 0x01, "animation" ); 
	FLAG_STUB( flag4, 0x02, "no diagonal" );
	//FLAG_STUB( flag4, 0x04, "unknown3" );
	FLAG_STUB( flag4, 0x08, "armor" );
	FLAG_STUB( flag4, 0x10, "roof" );
	FLAG_STUB( flag4, 0x20, "door" );
	FLAG_STUB( flag4, 0x40, "stair back" );
	FLAG_STUB( flag4, 0x80, "stair right" );

	return flags;
#undef FLAG_STUB
}

/*!
	Log a string
*/
static PyObject* wpConsole_log( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	char loglevel;
	char *text;

	if( !PyArg_ParseTuple( args, "bs:wolfpack.console.log( loglevel, text )", &loglevel, &text ) )
		return 0;

	Console::instance()->log( (eLogLevel)loglevel, text );

	return PyTrue;
}

/*!
	Sends a string to the wolfpack console.
*/
static PyObject* wpConsole_send( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if( PyTuple_Size( args ) < 1 )
		return PyFalse;

	PyObject *pyMessage = PyTuple_GetItem( args, 0 );

	if( pyMessage == Py_None )
		return PyFalse;

	Console::instance()->send( PyString_AS_STRING( pyMessage ) );

	return PyTrue;
}

/*!
	Sends a progress-bar to the wolfpack console
*/
static PyObject* wpConsole_sendprogress( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	char *message;
	if (!PyArg_ParseTuple(args, "es", "utf-8", &message)) {
		return 0;
	}

	Console::instance()->sendProgress(QString::fromUtf8(message));

	PyMem_Free(message);

	Py_INCREF(Py_None);
	return Py_None;
}

/*!
	Sends a [done] progress section to the console
*/
static PyObject* wpConsole_senddone( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	Console::instance()->sendDone();
	Py_INCREF(Py_None);
	return Py_None;
}

/*!
	Sends a [fail] progress section to the console
*/
static PyObject* wpConsole_sendfail( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	Console::instance()->sendFail();
	Py_INCREF(Py_None);
	return Py_None;
}

/*!
	Sends a [skip] progress section to the console
*/
static PyObject* wpConsole_sendskip( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	Console::instance()->sendSkip();
	Py_INCREF(Py_None);
	return Py_None;
}

/*!
	Returns a list of Strings (the linebuffer)
*/
static PyObject* wpConsole_getbuffer( PyObject* self, PyObject* args )
{	
	Q_UNUSED(self);	
	Q_UNUSED(args);
	QStringList linebuffer = Console::instance()->linebuffer();
	PyObject *list = PyList_New( linebuffer.count() );

	for( uint i = 0; i < linebuffer.count(); ++i )
		if( linebuffer[i].isNull() )
			PyList_SetItem( list, i, PyString_FromString( "" ) );
		else
			PyList_SetItem( list, i, PyString_FromString( linebuffer[i].latin1() ) );

	return list;
}

static PyObject* wpConsole_shutdown( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	keeprun = 0;

	return PyTrue;
}

/*!
	wolfpack.console
	Initializes wolfpack.console
*/
static PyMethodDef wpConsole[] = 
{
	{ "send",		wpConsole_send,			METH_VARARGS,	"Prints something to the wolfpack console" },
	{ "progress",		wpConsole_sendprogress,		METH_VARARGS,	"Prints a .....[xxxx] block" },
	{ "progressDone",	wpConsole_senddone,		METH_NOARGS,	"Prints a [done] block" },
	{ "progressFail",	wpConsole_sendfail,		METH_NOARGS,	"Prints a [fail] block" },
	{ "progressSkip",	wpConsole_sendskip,		METH_NOARGS,	"Prints a [skip] block" },
	{ "getbuffer",		wpConsole_getbuffer,		METH_NOARGS,	"Gets the linebuffer of the console" },
	{ "log",		wpConsole_log,			METH_VARARGS,	NULL },
	{ "shutdown",		wpConsole_shutdown,		METH_NOARGS,	"Shut the server down" },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	Gets the minutes of the current uo time
*/
static PyObject* wpTime_minute( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	return PyInt_FromLong( UoTime::instance()->minute() );
}

/*!
	Gets the hours of the current uo time
*/
static PyObject* wpTime_hour( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	return PyInt_FromLong( UoTime::instance()->hour() );
}

static PyObject* wpTime_days( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	return PyInt_FromLong( UoTime::instance()->days() );
}

/*!
	Gets the elapsed minutes since initialization of the game time.
*/
static PyObject* wpTime_minutes( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	return PyInt_FromLong(UoTime::instance()->getMinutes());
}

static PyObject* wpTime_currentlightlevel( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	return PyInt_FromLong( SrvParams->worldCurrentLevel() );
}

/*!
	Methods for handling UO Time from within python
*/
static PyMethodDef wpTime[] = 
{
	{ "minute",				wpTime_minute,				METH_NOARGS, "Returns the current time-minutes" },
	{ "hour",				wpTime_hour,				METH_NOARGS, "Returns the current time-hour" },
	{ "days",				wpTime_days,				METH_NOARGS, "Returns the current date-day" },
	{ "minutes",			wpTime_minutes,				METH_NOARGS, "Returns the current timestamp" },
	{ "currentlightlevel",	wpTime_currentlightlevel,	METH_NOARGS, "Returns the current light level" },
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
static PyObject* wpAdditem( PyObject* self, PyObject* args )
{
	char *definition;

	if (!PyArg_ParseTuple(args, "s:wolfpack.additem(def)", &definition)) {
		return 0;
	}

	P_ITEM pItem = cItem::createFromScript(definition);
	return PyGetItemObject(pItem);
}

/*!
	Adds a npc
*/
static PyObject* wpAddnpc( PyObject* self, PyObject* args )
{
	char *definition;
	Coord_cl pos;

	if (!PyArg_ParseTuple(args, "sO&:wolfpack.addnpc(def, pos)", &definition, &PyConvertCoord, &pos)) {
		return 0;
	}

	P_CHAR pChar = cNPC::createFromScript(getArgStr(0), pos);

	return PyGetCharObject( pChar ); 
}

/*!
	Creates an item object based on the 
	passed serial
*/
static PyObject* wpFinditem( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	

	SERIAL serial = INVALID_SERIAL;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.finditem", &serial ) )
		return 0;

	return PyGetItemObject( FindItemBySerial( serial ) );
}

/*!
	Returns a list of guilds.
*/
static PyObject *wpGuilds(PyObject *self, PyObject *args)
{
	Q_UNUSED(args);
	PyObject *list = PyList_New(0);

	for (cGuilds::iterator it = Guilds::instance()->begin(); it != Guilds::instance()->end(); ++it)
		PyList_Append(list, it.data()->getPyObject());

	return list;
}

/*!
	Returns the guild registered under the given serial.
*/
static PyObject* wpFindguild( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	

	SERIAL serial;

	if ( !PyArg_ParseTuple( args, "i:wolfpack.findguild(serial)", &serial ) )
		return 0;

	cGuild *guild = Guilds::instance()->findGuild(serial);

	if (guild) {
		return guild->getPyObject();
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}

/*!
	Creates a char object based on the 
	passed serial
*/
static PyObject* wpFindchar( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	SERIAL serial = INVALID_SERIAL;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.findchar", &serial ) )
		return 0;

	return PyGetCharObject( FindCharBySerial( serial ) );
}

/*!
	Find a multi based on its position.
 */
static PyObject* wpFindmulti( PyObject* self, PyObject* args ) {
	Coord_cl coord;

	if (!PyArg_ParseTuple(args, "O&:wolfpack.findmulti(pos)", &PyConvertCoord, &coord)) {
		return 0;
	}

	cMulti *multi = cMulti::find(coord);

	if (!multi) {
		Py_INCREF(Py_None);
		return Py_None;
	} else {
		return multi->getPyObject();
	}
}

/*!
	Adds a tempeffect
*/
static PyObject* wpAddtimer( PyObject* self, PyObject* args )
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
static PyObject* wpRegion( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Three arguments
	int x = 0, y = 0, map = 0;
	if ( !PyArg_ParseTuple( args, "iii:wolfpack.region", &x, &y, &map ) )
		return 0;

	return PyGetRegionObject( AllTerritories::instance()->region( x, y, map ) );
}

/*!
	Returns the time in ms since the last server-start
	used for object-delays and others
*/
static PyObject* wpCurrenttime( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyInt_FromLong( uiCurrentTime );
}

/*!
	Returns a list of Static item at a given position
*/
static PyObject *wpStatics( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Minimum is x, y, map
	uint x = 0, y = 0, map = 0;
	uchar exact = 0;
	if ( !PyArg_ParseTuple( args, "iii|b:wolfpack.statics", &x, &y, &map, &exact ) )
		return 0;
	
	StaticsIterator iter = Map->staticsIterator( Coord_cl( x, y, 0, map ), exact );
	
	PyObject *list = PyList_New( 0 );
	UINT32 xBlock = x / 8;
	UINT32 yBlock = y / 8;

	while( !iter.atEnd() )
	{
		// Create a Dictionary
		PyObject *dict = PyDict_New();

		PyDict_SetItemString( dict, "id", PyInt_FromLong( iter->itemid ) );
		PyDict_SetItemString( dict, "x", PyInt_FromLong( ( xBlock * 8 ) + iter->xoff ) );
		PyDict_SetItemString( dict, "y", PyInt_FromLong( ( yBlock * 8 ) + iter->yoff ) );
		PyDict_SetItemString( dict, "z", PyInt_FromLong( iter->zoff ) );

		PyList_Append( list, dict );
		iter++;
	}

	return list;
}

/*!
	Returns a list of all items serials 
*/
static PyObject *wpAllItemsSerials( PyObject* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);

	cItemIterator iter;
	PyObject *list = PyList_New( 0 );
	for( P_ITEM pItem = iter.first(); pItem; pItem = iter.next() )
		PyList_Append( list, PyInt_FromLong( pItem->serial() ) );

	return list;
}

/*!
	Returns an iterator for all items in the world
*/
static PyObject *wpAllItemsIterator( PyObject* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
	return PyGetItemIterator();
}

/*!
	Returns an iterator for all items in the world
*/
static PyObject *wpAllCharsIterator( PyObject* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
	return PyGetCharIterator();
}

/*!
	Returns a list of all chars serials 
*/
static PyObject *wpAllCharsSerials( PyObject* self, PyObject* args )
{
	Q_UNUSED(args);
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
static PyObject *wpItems( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Minimum is x, y, map
	uint x = 0, y = 0, map = 0, range = 1;
	if ( !PyArg_ParseTuple( args, "iii|i:wolfpack.items", &x, &y, &map, &range ) )
		return 0;
	
	Coord_cl pos( x, y, 0, map );
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
static PyObject *wpChars( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Minimum is x, y, map
	uint x = 0, y = 0, map = 0, range = 1;
	if ( !PyArg_ParseTuple( args, "iii|i:wolfpack.chars", &x, &y, &map, &range ) )
		return 0;
	
	Coord_cl pos( x, y, 0, map );
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
static PyObject *wpEffect( PyObject* self, PyObject* args )
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
static PyObject *wpMap( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	// Minimum is x, y, map
	uint x = 0, y = 0, map = 0;
	if ( !PyArg_ParseTuple( args, "iii:wolfpack.map", &x, &y, &map ) )
		return 0;

	map_st mTile = Map->seekMap( Coord_cl( x, y, 0, map ) );

	PyObject *dict = PyDict_New();
	PyDict_SetItemString( dict, "id", PyInt_FromLong( mTile.id ) );
	PyDict_SetItemString( dict, "z", PyInt_FromLong( mTile.z ) );
	return dict;
}

static PyObject* wpHasMap( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	uint map = 0;
	if( !PyArg_ParseTuple( args, "i:wolfpack.hasmap", &map ) )
		return 0;

	return Map->hasMap( map ) ? PyTrue : PyFalse;
}

static PyObject *wpLanddata(PyObject *self, PyObject *args) {
	Q_UNUSED(self);
	unsigned int tileid;

	if (!PyArg_ParseTuple(args, "i:wolfpack.landdata(id)", &tileid)) {
		return 0;
	}

	land_st tile = TileCache::instance()->getLand(tileid);

	PyObject *dict = PyDict_New();
	PyDict_SetItemString(dict, "name", PyString_FromString(tile.name));
	PyDict_SetItemString(dict, "unknown1", PyInt_FromLong(tile.unknown1));
	PyDict_SetItemString(dict, "unknown2", PyInt_FromLong(tile.unknown2));
	PyDict_SetItemString( dict, "flag1", PyInt_FromLong( tile.flag1 ) );
	PyDict_SetItemString( dict, "flag2", PyInt_FromLong( tile.flag2 ) );
	PyDict_SetItemString( dict, "flag3", PyInt_FromLong( tile.flag3 ) );
	PyDict_SetItemString( dict, "flag4", PyInt_FromLong( tile.flag4 ) );
	PyDict_SetItemString( dict, "wet", PyInt_FromLong( tile.isWet() ) );
	PyDict_SetItemString( dict, "blocking", PyInt_FromLong( tile.isBlocking() ) );
	PyDict_SetItemString( dict, "floor", PyInt_FromLong( tile.isRoofOrFloorTile() ) );

	QString flags = getFlagNames(tile.flag1, tile.flag2, tile.flag3, tile.flag4).join(",");
	if (flags.isNull()) {
		PyDict_SetItemString( dict, "flagnames", PyString_FromString("") );
    } else {
		PyDict_SetItemString( dict, "flagnames", PyString_FromString(flags.latin1()) );
	}

	return dict;
}

/*!
	Returns the tiledata information for a item id.
*/
static PyObject *wpTiledata( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	uint tileid = 0;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.tiledata(id)", &tileid ) )
		return 0;

	tile_st tile = TileCache::instance()->getTile( tileid );

	PyObject *dict = PyDict_New();

	// test if item is defined
	if( !strlen( tile.name ) )
	{
		PyDict_SetItemString( dict, "name", PyString_FromString( "unknown" ) );
		PyDict_SetItemString( dict, "flag1", PyInt_FromLong( 0 ) );
		PyDict_SetItemString( dict, "flag2", PyInt_FromLong( 0 ) );
		PyDict_SetItemString( dict, "flag3", PyInt_FromLong( 0 ) );
		PyDict_SetItemString( dict, "flag4", PyInt_FromLong( 0 ) );
	}
	else
	{
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

		QString flags = getFlagNames(tile.flag1, tile.flag2, tile.flag3, tile.flag4).join(",");
		if (flags.isNull()) {
			PyDict_SetItemString( dict, "flagnames", PyString_FromString("") );
        } else {
			PyDict_SetItemString( dict, "flagnames", PyString_FromString(flags.latin1()) );
		}
	}
	
	return dict;
}

/*!
	Returns a stringlist out of the definitions.
*/
static PyObject *wpList( PyObject* self, PyObject* args )
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
static PyObject *wpRegisterGlobal( PyObject* self, PyObject* args )
{
	unsigned int event;
	const char *scriptName;

	if( !PyArg_ParseTuple( args, "is:wolfpack.registerglobal", &event, &scriptName ) )
		return 0;

	cPythonScript *script = ScriptManager::instance()->find( scriptName );

	if( script == 0 )
	{
		PyErr_SetString( PyExc_RuntimeError, "Unknown script." );
		return 0;
	}

	if( event >= EVENT_COUNT )
	{
		PyErr_SetString( PyExc_RuntimeError, "Unknown script." );
		return 0;
	}

	ScriptManager::instance()->setGlobalHook( (ePythonEvent)event, script );
	return PyTrue;
}

/*!
	Registers a global command hook.
*/
static PyObject *wpRegisterCommand( PyObject* self, PyObject* args )
{
	const char *command;
	PyObject *function;

	if( !PyArg_ParseTuple( args, "sO:wolfpack.registercommand", &command, &function ) )
		return 0;

	if( !PyCallable_Check( function ) )
	{
		PyErr_SetString( PyExc_TypeError, "wolfpack.registercommand( command, function ): function has to be a callable object" );
		return 0;
	}

	Py_INCREF( function );
	ScriptManager::instance()->setCommandHook( command, function );
	return PyTrue;
}

/*!
	Registers a global packet hook.
*/
static PyObject *wpRegisterPacketHook( PyObject* self, PyObject* args )
{
	unsigned char packet;
	PyObject *function;

	if (!PyArg_ParseTuple(args, "bO:wolfpack.registerpackethook(packet, handler)", &packet, &function)) {
		return 0;
	}

	if (!PyCallable_Check(function)) {
		PyErr_SetString(PyExc_TypeError, "A callable object was expected.");
		return 0;
	}

	cUOSocket::registerPacketHandler(packet, function);
	return PyTrue;
}

/*!
	Coord object creation
*/
static PyObject *wpCoord( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	uint x = 0, y = 0, z = 0;
	uchar map = 0;
	if( !PyArg_ParseTuple( args, "iiib:wolfpack.coord", &x, &y, &z, &map ) )
		return 0;

	Coord_cl pos( x, y, z, map );

	return PyGetCoordObject( pos );
}

static PyObject *wpAddMulti(PyObject* self, PyObject* args) {
	char *definition;

	if (!PyArg_ParseTuple(args, "s:wolfpack.addmulti(def)", &definition)) {
		return 0;
	}

	cMulti *multi = cMulti::createFromScript(definition);
	
	if (multi) {
		return multi->getPyObject();
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}

/*!
	Returns uptime of server in seconds
*/
static PyObject* wpServerUptime( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyInt_FromLong( uiCurrentTime / MY_CLOCKS_PER_SEC );
}

/*!
	Returns the server version string
*/
static PyObject* wpServerVersion( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyString_FromString(QString("%1 %2 %3").arg(productString()).arg(productBeta()).arg(productVersion()).latin1());
}

/*!
	Returns the current real date/time
*/
static PyObject* wpCurrentdatetime( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);	
	Q_UNUSED(args);
	return PyString_FromString( QDateTime::currentDateTime().toString() );
}

/*!
	Returns if the server is in starting state
*/
static PyObject* wpIsStarting( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	if( serverState == STARTUP )
		return PyTrue;
	else
		return PyFalse;
}

/*!
	Returns if the server is in running state
*/
static PyObject* wpIsRunning( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	if( serverState == RUNNING )
		return PyTrue;
	else
		return PyFalse;
}

/*!
	Returns if the server is in reload state
*/
static PyObject* wpIsReloading( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	if( serverState == SCRIPTRELOAD )
		return PyTrue;
	else
		return PyFalse;
}

/*!
	Returns if the server is in closing state
*/
static PyObject* wpIsClosing( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
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

	return PyGetCharRegionIterator( xBlock, yBlock, map );
}

static PyObject* wpItemBlock( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	unsigned int xBlock, yBlock;
	unsigned char map;

	if( !PyArg_ParseTuple( args, "iib:wolfpack.itemblock", &xBlock, &yBlock, &map ) )
		return 0;

	return PyGetItemRegionIterator( xBlock, yBlock, map );
}

static PyObject* wpCharRegion( PyObject* self, PyObject* args )
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

static PyObject *wpNewguild(PyObject *self, PyObject *args) 
{
	Q_UNUSED(args);
	cGuild *guild = new cGuild(true);
	Guilds::instance()->registerGuild(guild);
	return guild->getPyObject();
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

static PyObject* wpTickcount( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyInt_FromLong( getNormalizedTime() );
}

static PyObject* wpCharBase( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	unsigned short id;

	if( !PyArg_ParseTuple( args, "h:wolfpack.charbase( id )", &id ) )
		return 0;

	cCharBaseDef *def = BaseDefManager::instance()->getCharBaseDef( id );

	if( !def )
	{
		Py_XINCREF( Py_None );
		return Py_None;
	}

	// Create a Dictionary
	PyObject *dict = PyDict_New();

	PyDict_SetItemString( dict, "basesound", PyInt_FromLong( def->basesound() ) );
	PyDict_SetItemString( dict, "soundmode", PyInt_FromLong( def->soundmode() ) );
	PyDict_SetItemString( dict, "shrinked", PyInt_FromLong( def->shrinked() ) );
	PyDict_SetItemString( dict, "flags", PyInt_FromLong( def->flags() ) );
	PyDict_SetItemString( dict, "type", PyInt_FromLong( def->type() ) );

	return dict;
}

static PyObject* wpCharCount( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyLong_FromLong( World::instance()->charCount() );
}

static PyObject* wpItemCount( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyLong_FromLong( World::instance()->itemCount() );
}

static PyObject* wpPacket( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	unsigned int id, size;

	if( !PyArg_ParseTuple( args, "ii:wolfpack.packet", &id, &size ) )
		return 0;

	return CreatePyPacket( (unsigned char)id, (unsigned short)size );
}

static PyObject* wpQueueAction( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	
	unsigned int type = 0;

	if( !PyArg_ParseTuple( args, "i:wolfpack.queueaction( type )", &type ) )
		return 0;
	
	queueAction( (eActionType)type );

	return PyInt_FromLong( 1 );
}

static PyObject *wpGetDefinition(PyObject *self, PyObject *args) {
	unsigned int type;
	char *name;

	if (!PyArg_ParseTuple(args, "Ies:getdefinition(type, id)", &type, "utf-8", &name)) {
		return 0;
	}

	const cElement *element = DefManager->getDefinition((eDefCategory)type, QString::fromUtf8(name));

	PyMem_Free(name);

	if (element) {
		return const_cast<cElement*>(element)->getPyObject();	
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}

static PyObject *wpGetDefinitions(PyObject *self, PyObject *args) {
	unsigned int type;
	
	if (!PyArg_ParseTuple(args, "I:getdefinitions(type)", &type)) {
		return 0;
	}

	const QValueVector<cElement*> elements = DefManager->getDefinitions((eDefCategory)type);
	QStringList sections = DefManager->getSections((eDefCategory)type);

	PyObject *result = PyTuple_New(elements.size() + sections.size());
	
	uint i = 0;
	for (; i < elements.size(); ++i) {
		PyTuple_SetItem(result, i, elements[i]->getPyObject());
	}

	for (uint j = 0; j < sections.size(); ++j) {
		cElement *element = const_cast<cElement*>(DefManager->getDefinition((eDefCategory)type, sections[j]));
		PyTuple_SetItem(result, i++, element->getPyObject());
	}

	return result;
}

static PyObject *wpLock(PyObject *self, PyObject *args) {
	Q_UNUSED(self);
	Q_UNUSED(args);
	lockDataMutex();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *wpUnlock(PyObject *self, PyObject *args) {
	Q_UNUSED(self);
	Q_UNUSED(args);
	unlockDataMutex();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *wpCallEvent(PyObject *self, PyObject *args) {
	char *script;
	unsigned int event;
	PyObject *eventargs;

	if (!PyArg_ParseTuple(args, "sIO!:wolfpack.callevent(scriptname, event, args)", 
		&script, &event, &PyTuple_Type, &eventargs)) {
		return 0;
	}

	cPythonScript *pythonscript = ScriptManager::instance()->find(script);

	if (!pythonscript) {
		PyErr_SetString(PyExc_ValueError, "You tried to access an unknown script.");
		return 0;
	}

	PyObject *result = pythonscript->callEvent((ePythonEvent)event, eventargs);

	if (!result) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	return result;
}

static PyObject *wpHasEvent(PyObject *self, PyObject *args) {
	char *script;
	unsigned int event;

	if (!PyArg_ParseTuple(args, "sI:wolfpack.hasevent(scriptname, event)", &script, &event)) {
		return 0;
	}

	cPythonScript *pythonscript = ScriptManager::instance()->find(script);

	if (!pythonscript) {
		PyErr_SetString(PyExc_ValueError, "You tried to access an unknown script.");
		return 0;
	}

	bool result = pythonscript->canHandleEvent((ePythonEvent)event);

	if (result) {
		Py_INCREF(Py_True);
		return Py_True;
	} else {
		Py_INCREF(Py_False);
		return Py_False;
	}
}

static PyObject *wpCallNamedEvent(PyObject *self, PyObject *args) {
	char *script;
	char *event;
	PyObject *eventargs;

	if (!PyArg_ParseTuple(args, "ssO!:wolfpack.callevent(scriptname, event, args)", 
		&script, &event, &PyTuple_Type, &eventargs)) {
		return 0;
	}

	cPythonScript *pythonscript = ScriptManager::instance()->find(script);

	if (!pythonscript) {
		PyErr_SetString(PyExc_ValueError, "You tried to access an unknown script.");
		return 0;
	}

	PyObject *result = pythonscript->callEvent(event, eventargs);

	if (!result) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	return result;
}

static PyObject *wpHasNamedEvent(PyObject *self, PyObject *args) {
	char *script;
	char *event;

	if (!PyArg_ParseTuple(args, "ss:wolfpack.hasnamedevent(scriptname, eventname)", &script, &event)) {
		return 0;
	}

	cPythonScript *pythonscript = ScriptManager::instance()->find(script);

	if (!pythonscript) {
		PyErr_SetString(PyExc_ValueError, "You tried to access an unknown script.");
		return 0;
	}

	bool result = pythonscript->canHandleEvent(event);

	if (result) {
		Py_INCREF(Py_True);
		return Py_True;
	} else {
		Py_INCREF(Py_False);
		return Py_False;
	}
}

/*!
	wolfpack
	Initializes wolfpack
*/
static PyMethodDef wpGlobal[] = 
{
	{ "callevent",			wpCallEvent,					METH_VARARGS, "Call an event in a script and return the result." },
	{ "hasevent",			wpHasEvent,						METH_VARARGS, "If the given script has the given event. Return true." },
	{ "callnamedevent",		wpCallEvent,					METH_VARARGS, "Call an event in a script and return the result." },
	{ "hasnamedevent",		wpHasEvent,						METH_VARARGS, "If the given script has the given event. Return true." },
	{ "getdefinition",		wpGetDefinition,				METH_VARARGS, "Gets a certain definition by it's id." },
	{ "getdefinitions",		wpGetDefinitions,				METH_VARARGS, "Gets all definitions by type." },
	{ "packet",				wpPacket,						METH_VARARGS, NULL },
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
	{ "guilds",				wpGuilds,						METH_VARARGS, 0},
	{ "findguild",			wpFindguild,					METH_VARARGS, 0},
	{ "findchar",			wpFindchar,						METH_VARARGS, "Tries to find a char based on it's serial" },
	{ "findmulti",			wpFindmulti,					METH_VARARGS, "Tries to find a multi based on it's position" },
	{ "addtimer",			wpAddtimer,						METH_VARARGS, "Adds a timed effect" },
	{ "effect",				wpEffect,						METH_VARARGS, "Shows a graphical effect." },
	{ "region",				wpRegion,						METH_VARARGS, "Gets the region at a specific position" },
	{ "currenttime",		wpCurrenttime,					METH_NOARGS, "Time in ms since server-start" },
	{ "newguild",			wpNewguild,						METH_VARARGS, 0},
	{ "statics",			wpStatics,						METH_VARARGS, "Returns a list of static-item at a given position" },
	{ "map",				wpMap,							METH_VARARGS, "Returns a dictionary with information about a given map tile" },
	{ "hasmap",				wpHasMap,						METH_VARARGS, "Returns true if the map specified is present"	},
	{ "items",				wpItems,						METH_VARARGS, "Returns a list of items in a specific sector." },
	{ "itemiterator",		wpAllItemsIterator,				METH_NOARGS,  "Returns an iterator for all items in the world."	},
	{ "chariterator",		wpAllCharsIterator,				METH_NOARGS,  "Returns an iterator for all chars in the world."	},
	{ "chars",				wpChars,						METH_VARARGS, "Returns a list of chars in a specific sector." },
	{ "allcharsserials",	wpAllCharsSerials,				METH_VARARGS, "Returns a list of all chars serials" },
	{ "allitemsserials",	wpAllItemsSerials,				METH_VARARGS, "Returns a list of all items serials" },
	{ "landdata",			wpLanddata,						METH_VARARGS, "Returns the landdata information for a given tile stored on the server." },
	{ "tiledata",			wpTiledata,						METH_VARARGS, "Returns the tiledata information for a given tile stored on the server." },
	{ "coord",				wpCoord,						METH_VARARGS, "Creates a coordinate object from the given parameters (x,y,z,map)." },
	{ "addmulti",			wpAddMulti,						METH_VARARGS, "Creates a multi object by given type CUSTOMHOUSE, HOUSE, BOAT." },
	{ "list",				wpList,							METH_VARARGS, "Returns a list defined in the definitions as a Python List" },
	{ "registerglobal",		wpRegisterGlobal,				METH_VARARGS, "Registers a global script hook." },
	{ "registerpackethook", wpRegisterPacketHook,			METH_VARARGS, "Registers a packet hook." },
	{ "registercommand",	wpRegisterCommand,				METH_VARARGS, "Registers a global command hook." },
	{ "serveruptime",		wpServerUptime,					METH_NOARGS, "Returns uptime of server in seconds." },
	{ "serverversion",		wpServerVersion,				METH_NOARGS, "Returns the server version string." },
	{ "currentdatetime",	wpCurrentdatetime,				METH_NOARGS, "Returns the current real date/time" },
	{ "isstarting",			wpIsStarting,					METH_NOARGS, "Returns if the server is in starting state" },
	{ "isrunning",			wpIsRunning,					METH_NOARGS, "Returns if the server is in running state" },
	{ "isreloading",		wpIsReloading,					METH_NOARGS, "Returns if the server is in reload state" },
	{ "isclosing",			wpIsClosing,					METH_NOARGS, "Returns if the server is in closing state" },
	{ "tickcount",			wpTickcount,					METH_NOARGS, "Returns the current Tickcount on Windows" },
	{ "charbase",			wpCharBase,						METH_VARARGS, NULL },
	{ "queueaction",		wpQueueAction,					METH_VARARGS, NULL },
	{ "charcount",			wpCharCount,					METH_NOARGS,  "Returns the number of chars in the world" },
	{ "itemcount",			wpItemCount,					METH_NOARGS,  "Returns the number of items in the world" },
	{ NULL, NULL, 0, NULL } // Terminator
};

static PyObject *wpSocketsFirst( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyGetSocketObject( cNetwork::instance()->first() );
}

static PyObject *wpSocketsNext( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyGetSocketObject( cNetwork::instance()->next() );  
}

/*!
	Retrieves the number of currently connected sockets
*/
static PyObject *wpSocketsCount( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	return PyInt_FromLong( cNetwork::instance()->count() );
}

/*!
	wolfpack.sockets
	Socket iteration
*/
static PyMethodDef wpSockets[] = 
{
	{ "first",	wpSocketsFirst,	METH_NOARGS, "Returns the first connected socket." },
	{ "next",	wpSocketsNext,	METH_NOARGS, "Returns the next connected socket." },
	{ "count",	wpSocketsCount,	METH_NOARGS, "Returns the number of connected sockets." },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	Finds an Account object.
 */
static PyObject *wpAccountsFind( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	cAccount* account = Accounts::instance()->getRecord( getArgStr( 0 ) );
	return PyGetAccountObject( account );
}

/*!
	Gets a list of Account names.
 */
static PyObject *wpAccountsList( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
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
static PyObject *wpAccountsAcls( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	PyObject *list = PyList_New( 0 );

	QMap< QString, cAcl* >::const_iterator it = Commands::instance()->aclbegin();
	while( it != Commands::instance()->aclend() )
	{
		QString name = it.key();
		if( !name.isEmpty() )
			PyList_Append( list, PyString_FromString( name ) );
		++it;
	}

	return list;
}

/*!
	Returns an ACL as a dictionary.
 */
static PyObject *wpAccountsAcl( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	if( !checkArgStr( 0 ) )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	cAcl *acl = Commands::instance()->getACL( getArgStr( 0 ) );
	if( !acl )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	PyObject *dict = PyDict_New();
	
	QMap< QString, QMap< QString, bool > >::const_iterator git;
	for( git = acl->groups.begin(); git != acl->groups.end(); ++git )
	{
		PyObject *dict2 = PyDict_New();

		for( QMap< QString, bool >::const_iterator it = (*git).begin(); it != (*git).end(); ++it )
			PyDict_SetItem( dict2, PyString_FromString( it.key() ), it.data() ? PyTrue : PyFalse );

		PyDict_SetItem( dict, PyString_FromString( git.key() ), dict2 );
	}

	return dict;
}

/*!
	Creates an account (username + password is enough)
 */
static PyObject *wpAccountsAdd( PyObject* self, PyObject* args )
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

	cAccount *account = Accounts::instance()->getRecord( login );

	if( account )
		return PyFalse;

	account = Accounts::instance()->createAccount( login, password );
	return PyGetAccountObject( account );
}

/*!
	Reload accounts.
 */
static PyObject *wpAccountsReload( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	Accounts::instance()->reload();
	return PyTrue;
}

/*!
	Save accounts.
 */
static PyObject *wpAccountsSave( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	Accounts::instance()->save();
	return PyTrue;
}

/*!
	wolfpack.accounts
	account related functions
*/
static PyMethodDef wpAccounts[] = 
{
    { "find",		wpAccountsFind,		METH_VARARGS, "Finds an account object." },
	{ "list",		wpAccountsList,		METH_NOARGS, "Gets a list of Account names." },
	{ "acls",		wpAccountsAcls,		METH_NOARGS, "Gets a list of valid ACL names." },
	{ "acl",		wpAccountsAcl,		METH_VARARGS, "Returns an acl as a double dictionary." },
	{ "add",		wpAccountsAdd,		METH_VARARGS, "Creates an account." },
	{ "save",		wpAccountsSave,		METH_NOARGS, "Save accounts." },
	{ "reload",		wpAccountsReload,	METH_NOARGS, "Reload accounts." },
	{ NULL, NULL, 0, NULL } // Terminator
};

/*!
	Reads the boolean entry specified by key and group. 
	The key is created if it doesn't exist, using the default argument.
	If an error occurs the settings are left unchanged and FALSE is returned; 
	otherwise TRUE is returned
*/
static PyObject *wpSettingsGetBool( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	char *pyGroup, *pyKey, pyDef, create = 0;
	if ( !PyArg_ParseTuple(args, "ssb|b:getBool(group, key, default, create)", &pyGroup, &pyKey, &pyDef, &create ) )
		return 0;

	return SrvParams->getBool( pyGroup, pyKey, pyDef, create ) ? PyTrue : PyFalse;
}

/*!
	Writes the boolean entry value into specified key and group. 
	The key is created if it doesn't exist. Any previous value is overwritten by value. 
	If an error occurs the settings are left unchanged and FALSE is returned; 
	otherwise TRUE is returned
*/
static PyObject *wpSettingsSetBool( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	char *pyGroup, *pyKey, pyValue;
	if ( !PyArg_ParseTuple(args, "ssb:setBool(group, key, value)", &pyGroup, &pyKey, &pyValue ) )
		return 0;

	SrvParams->setBool( pyGroup, pyKey, pyValue );
	return PyTrue;
}

/*!
	Reads the numeric entry specified by key and group. 
	The key is created if it doesn't exist using the default argument, provided
	that \a create argument is true. 
	If an error occurs the settings are left unchanged and FALSE is returned; 
	otherwise TRUE is returned
*/
static PyObject *wpSettingsGetNumber( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	char *pyGroup, *pyKey, create = 0;
	int pyDef;
	if ( !PyArg_ParseTuple(args, "ssi|b:getNumber(group, key, default, create)", &pyGroup, &pyKey, &pyDef, &create ) )
		return 0;

	return PyInt_FromLong( SrvParams->getNumber( pyGroup, pyKey, pyDef, create ) );
}

/*!
	Writes the numeric entry value into specified key and group. 
	The key is created if it doesn't exist. Any previous value is overwritten by value. 
	If an error occurs the settings are left unchanged and FALSE is returned; 
	otherwise TRUE is returned
*/
static PyObject *wpSettingsSetNumber( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	char *pyGroup, *pyKey;
	int pyValue;
	if ( !PyArg_ParseTuple(args, "ssi:setNumber(group, key, value)", &pyGroup, &pyKey, &pyValue ) )
		return 0;

	SrvParams->setNumber( pyGroup, pyKey, pyValue );
	return PyTrue;
}

/*!
	getString( group, key, default, create )
	Reads the string entry specified by key and group. 
	The key is created if it doesn't exist using the default argument, provided that
	\a create argument is true.
	If an error occurs the settings are left unchanged and FALSE is returned; 
	otherwise TRUE is returned
*/
static PyObject *wpSettingsGetString( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	char *pyGroup, *pyKey, *pyDef, create = 0;
	if ( !PyArg_ParseTuple(args, "sss|b:getString(group, key, default, create)", &pyGroup, &pyKey, &pyDef, &create ) )
		return 0;
	
	return PyString_FromString( SrvParams->getString( pyGroup, pyKey, pyDef, create ) );
}

/*!
	Writes the string entry value into specified key and group. 
	The key is created if it doesn't exist. Any previous value is overwritten by value. 
	If an error occurs the settings are left unchanged and FALSE is returned; 
	otherwise TRUE is returned
*/
static PyObject *wpSettingsSetString( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	char *pyGroup, *pyKey, *pyValue;
	if ( !PyArg_ParseTuple(args, "sss:setString(group, key, value)", &pyGroup, &pyKey, &pyValue ) )
		return 0;

	SrvParams->setString( pyGroup, pyKey, pyValue );
	return PyTrue;
}

/*!
	Reloads wolfpack.xml
*/
static PyObject* wpSettingsReload( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	SrvParams->reload();
	return PyTrue;
}

/*!
	Saves wolfpack.xml
*/
static PyObject* wpSettingsSave( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);
	Q_UNUSED(args);
	SrvParams->flush();
	return PyTrue;
}

/*!
	wolfpack.settings
	wolfpack.xml config related functions
*/
static PyMethodDef wpSettings[] = 
{
    { "getBool",		wpSettingsGetBool,		METH_VARARGS, "Reads a boolean value from wolfpack.xml." },
	{ "setBool",		wpSettingsSetBool,		METH_VARARGS, "Sets a boolean value to wolfpack.xml." },
	{ "getNumber",		wpSettingsGetNumber,	METH_VARARGS, "Gets a numeric value from wolfpack.xml." },
	{ "setNumber",		wpSettingsSetNumber,	METH_VARARGS, "Sets a numeric value to wolfpack.xml." },
	{ "getString",		wpSettingsGetString,	METH_VARARGS, "Reads a string value from wolfpack.xml." },
	{ "setString",		wpSettingsSetString,	METH_VARARGS, "Writes a string value to wolfpack.xml." },
	{ "reload",			wpSettingsReload,		METH_NOARGS, "Reloads wolfpack.xml." },
	{ "save",			wpSettingsSave,			METH_NOARGS, "Saves changes made to wolfpack.xml"	},
	{ NULL, NULL, 0, NULL } // Terminator
};

static PyObject* wpOptionsGetOption( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	QString arg_key = getArgStr( 0 );
	QString arg_def = getArgStr( 1 );

	QString value;

	World::instance()->getOption( arg_key, value, arg_def );

	return PyString_FromString( value );
}

static PyObject* wpOptionsSetOption( PyObject* self, PyObject* args )
{
	Q_UNUSED(self);

	QString arg_key = getArgStr( 0 );
	QString arg_val = getArgStr( 1 );

	World::instance()->setOption( arg_key, arg_val );

	return PyTrue;
}

/*!
	wolfpack.options
	config using the settings table
*/
static PyMethodDef wpOptions[] = 
{
	{ "getOption",		wpOptionsGetOption,		METH_VARARGS, "Reads a string value from the database." },
	{ "setOption",		wpOptionsSetOption,		METH_VARARGS, "Sets a string value and a key to the database." },
	{ NULL, NULL, 0, NULL } // Terminator
};

static PyObject *wpQuery(PyObject *self, PyObject *args) {
	char *query;

	if (!PyArg_ParseTuple(args, "es:wolfpack.database.query(query)", "utf-8", &query)) {
		return 0;
	}

	cDBResult result;
	
	try {
		result = persistentBroker->query(query);
	} catch (QString e) {
		PyMem_Free(query);
		PyErr_SetString(PyExc_RuntimeError, e.latin1());
		return 0;
	} catch(...) {
		PyMem_Free(query);
		PyErr_SetString(PyExc_RuntimeError, "An error occured while querying the database.");
		return 0;
	}
	
	PyMem_Free(query);
	
	return (new cDBResult(result))->getPyObject();
}

static PyObject *wpExecute(PyObject *self, PyObject *args) {
	char *query;

	if (!PyArg_ParseTuple(args, "es:wolfpack.database.execute(query)", "utf-8", &query)) {
		return 0;
	}

	try {
		persistentBroker->executeQuery(query);
	} catch (QString e) {
		PyMem_Free(query);
		PyErr_SetString(PyExc_RuntimeError, e.latin1());
		return 0;
	} catch(...) {
		PyMem_Free(query);
		PyErr_SetString(PyExc_RuntimeError, "An error occured while querying the database.");
		return 0;
	}

	PyMem_Free(query);
	return PyTrue;
}

static PyObject *wpDriver(PyObject *self, PyObject *args) 
{
	Q_UNUSED(args);
	unsigned int database;

	if (!PyArg_ParseTuple(args, "I:wolfpack.database.driver(database)", &database))
		return 0;

	QString driver = "unknown";

	if (database == 1)
		driver = SrvParams->accountsDriver();
	else if (database == 2) 
		driver = SrvParams->databaseDriver();

	return PyString_FromString(driver.latin1());
}

static PyObject *wpClose(PyObject *self, PyObject *args) 
{
	try 
	{
		persistentBroker->disconnect();
	}
	catch (...) 
	{
		PyErr_SetString(PyExc_RuntimeError, "Error while disconnecting from the database.");
		return 0;
	}

	return PyTrue;
}

static PyObject *wpOpen(PyObject *self, PyObject *args) {
	unsigned int database;

	if (!PyArg_ParseTuple(args, "I:wolfpack.database.open(database)", &database)) {
		return 0;
	}

	try {
		if (database == 1) {
			persistentBroker->connect(SrvParams->accountsHost(), SrvParams->accountsName(), 
				SrvParams->accountsUsername(), SrvParams->accountsPassword());
		} else if (database == 2) {
			persistentBroker->connect(SrvParams->databaseHost(), SrvParams->databaseName(), 
				SrvParams->databaseUsername(), SrvParams->databasePassword());
		}
	} catch (QString e) {
		PyErr_SetString(PyExc_RuntimeError, e.latin1());
		return 0;
	} catch (...) {
		PyErr_SetString(PyExc_RuntimeError, "Error while connecting to the database.");
		return 0;
	}

	return PyTrue;
}

static PyMethodDef wpDatabase[] = {
	{ "query",				wpQuery,	METH_VARARGS, "Executes a sql query and returns the result." },
	{ "execute",			wpExecute,	METH_VARARGS, "Executes a sql query and dont return a result." },
	{ "driver",				wpDriver,	METH_VARARGS, "Returns the name of the database driver used." },
	{ "close",				wpClose,	METH_VARARGS, "Closes the database." },
	{ "open",				wpOpen,		METH_VARARGS, "Opens the database." },
	{ 0, 0, 0, 0 }
};

/*!
	This initializes the _wolfpack namespace and it's sub extensions
*/
void init_wolfpack_globals()
{
	PyObject *wpNamespace = Py_InitModule( "_wolfpack", wpGlobal );

	PyObject *mConsole = Py_InitModule( "_wolfpack.console", wpConsole );
    PyObject_SetAttrString( wpNamespace, "console", mConsole );

	PyObject *mAccounts = Py_InitModule( "_wolfpack.accounts", wpAccounts );
    PyObject_SetAttrString( wpNamespace, "accounts", mAccounts );

	PyObject *mSockets = Py_InitModule( "_wolfpack.sockets", wpSockets );
    PyObject_SetAttrString( wpNamespace, "sockets", mSockets );

	PyObject *mTime = Py_InitModule( "_wolfpack.time", wpTime );
    PyObject_SetAttrString( wpNamespace, "time", mTime );

	PyObject *mSettings = Py_InitModule( "_wolfpack.settings", wpSettings );
	PyObject_SetAttrString( wpNamespace, "settings", mSettings );
	
	PyObject *mOptions = Py_InitModule( "_wolfpack.options", wpOptions );
	PyObject_SetAttrString( wpNamespace, "options", mOptions );

	PyObject *mDatabase = Py_InitModule( "_wolfpack.database", wpDatabase );
	PyObject_SetAttrString( wpNamespace, "database", mDatabase );

	// Try to import the wolfpack module and add some integer constants
	/*PyObject *module;
	
	module = PyImport_ImportModule("wolfpack.consts");
	
	if (!module) {
		reportPythonError("wolfpack.consts");
		return;
	}

	Py_DECREF(module);*/
}

int PyConvertObject(PyObject *object, cUObject **uoobject) {
	if (checkWpChar(object)) {
		*uoobject = getWpChar(object);
	} else if(checkWpItem(object)) {
		*uoobject = getWpItem(object);
	} else {
		PyErr_SetString(PyExc_TypeError, "Object expected.");
		return 0;
	}   
	
	return 1;
}
