/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include <boost/python.hpp>
#include "engine.h"

#include "../basics.h"
#include "../network/network.h"
#include "../network/uosocket.h"
#include "../network/uotxpackets.h"
#include "../console.h"
#include "../guilds.h"
#include "../uotime.h"
#include "../timers.h"
#include "../mapobjects.h"
#include "../territories.h"
#include "../muls/maps.h"
#include "../muls/tilecache.h"
#include "../accounts.h"
#include "../commands.h"
#include "../multi.h"
#include "../scriptmanager.h"
#include "../muls/multiscache.h"
#include "../definitions.h"
#include "../pythonscript.h"
#include "../verinfo.h"
#include "../exportdefinitions.h"

#include "../items.h"
#include "../serverconfig.h"
#include "../basechar.h"
#include "../player.h"
#include "../npc.h"
#include "../targetrequests.h"
#include "../basedef.h"

#include "pyaction.h"
#include "pypacket.h"
#include "regioniterator.h"
#include "utilities.h"
#include "tempeffect.h"
#include "worlditerator.h"
#include "pyspawnregion.h"

// Library Includes
#include <QDateTime>
#include <QMutex>
#include <QCoreApplication>

using namespace boost::python;

PyObject* PyGetObjectObject( cUObject* object )
{
	if ( !object )
	{
		Py_RETURN_NONE;
	}

	if ( dynamic_cast<P_ITEM>( object ) )
		return PyGetItemObject( ( P_ITEM ) object );
	else if ( dynamic_cast<P_CHAR>( object ) )
		return PyGetCharObject( ( P_CHAR ) object );

	Py_RETURN_NONE;
}

static QStringList getFlagNames( unsigned char flag1, unsigned char flag2, unsigned char flag3, unsigned char flag4 )
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

/*
\function wolfpack.console.log
\param loglevel The loglevel for this message. See the "Log Constants" in <module id="wolfpack.consts">wolfpack.consts</module> for
details.
\param text The text of the message.
\description Sends a string to the console and the logfile.
*/
void wpConsole_log(int loglevel, QString text )
{
	Console::instance()->log( ( eLogLevel ) loglevel, text );
}

/*
\function wolfpack.console.send
\param text The text of the message.
\description Prints a string on the server console.
*/
void wpConsole_send( QString text )
{
	Console::instance()->send( text );
}

/*
\function wolfpack.console.sendprogress
\param text The progress identifier.
\description Prints a string on the console that identifies the start of a progress display.
*/
void wpConsole_sendprogress( QString message )
{
	Console::instance()->sendProgress( message );
}

/*
\function wolfpack.console.senddone
\description Indicates that a printed progress action is now done.
*/
void wpConsole_senddone()
{
	Console::instance()->sendDone();
}

/*
\function wolfpack.console.sendfail
\description Indicates that a printed progress action has failed.
*/
void wpConsole_sendfail()
{
	Console::instance()->sendFail();
}

/*
\function wolfpack.console.sendskip
\description Indicates that a printed progress action has been skipped.
*/
void wpConsole_sendskip()
{
	Console::instance()->sendSkip();
}

/*
\function wolfpack.console.getbuffer
\return A list of the strings that have been sent to the console previously.
*/
QString wpConsole_getbuffer()
{
	return Console::instance()->linebuffer();
}

/*
\function wolfpack.console.shutdown
\description Shutdown the server.
*/
void wpConsole_shutdown()
{
	Server::instance()->cancel();
}

/*
	\function wolfpack.time.minute
	\return Integer value ranging from 0 to 59.
	\description Returns the current minute of the hour in ingame time.
*/
int wpTime_minute()
{
	return UoTime::instance()->minute();
}

/*
	\function wolfpack.time.hour
	\return Integer value ranging from 0 to 23.
	\description Returns the current hour of the day in ingame time.
*/
int wpTime_hour()
{
	return UoTime::instance()->hour();
}

/*
	\function wolfpack.time.days
	\return Integer value.
	\description Returns the current day in ingame time. There are no years in ingame time but
	you are free to implement them yourself in custom calendar and clock items.
*/
int wpTime_days()
{
	return UoTime::instance()->days();
}

/*
	\function wolfpack.time.minutes
	\return Integer value.
	\description Returns the minutes since the world was initialized.
*/
int wpTime_minutes()
{
	return UoTime::instance()->getMinutes();
}

/*
	\function wolfpack.time.currentlightlevel
	\return Integer value.
	\description Returns the current global lightlevel of the world. This value range is between 0 and 30
*/
int wpTime_currentlightlevel()
{
	return Config::instance()->worldCurrentLevel();
}

/*
\function wolfpack.time.currenttime
\return An interger value.
\description This function returns the current time since the server start in miliseconds.
*/
unsigned int wpCurrenttime()
{
	return Server::instance()->time();
}

/*
	\function wolfpack.additem
	\param definition The id of the item definition to create the item from.
	\return An <object id="item">item</object> object or None.
	\description Creates a new item from a given definition id and assigns a new serial to it.
*/
static PyObject* wpAdditem( PyObject* /*self*/, PyObject* args )
{
	char* definition;

	if ( !PyArg_ParseTuple( args, "s:wolfpack.additem(def)", &definition ) )
	{
		return 0;
	}

	if ( !World::instance()->isLoaded() )
	{
		PyErr_Format(PyExc_RuntimeError, "Trying to create an item before the world has loaded is not supported." );
		Py_RETURN_NONE;
	}


	P_ITEM pItem = cItem::createFromScript( definition );
	return PyGetItemObject( pItem );
}

/*
	\function wolfpack.addnpc
	\param definition The id of the npc definition to create the npc from.
	\param pos A <object id="COORD">coord</object> object representing the coordinate where
	the new npc should be created. This has to be a valid location on a valid map.
	\return A <object id="char">char</object> object for the newly created npc.
	\description Creates a new npc from the given definition and moves it to the given position.
*/
static PyObject* wpAddnpc( PyObject* /*self*/, PyObject* args )
{
	char* definition;
	Coord pos;

	if ( !PyArg_ParseTuple( args, "sO&:wolfpack.addnpc(def, pos)", &definition, &PyConvertCoord, &pos ) )
	{
		return 0;
	}

	if ( !World::instance()->isLoaded() )
	{
		PyErr_Format(PyExc_RuntimeError, "Trying to create an npc before the world has loaded is not supported." );
		Py_RETURN_NONE;
	}

	P_CHAR pChar = cNPC::createFromScript( getArgStr( 0 ), pos );

	return PyGetCharObject( pChar );
}

/*
	\function wolfpack.finditem
	\param serial The item serial, an integer value.
	\return An <object id="item">item</object> object if the item was found, None otherwise.
	\description Tries to find an item with the given serial and returns an item object for it. If
	no item with the given serial is found, None is silently returned.
*/
static PyObject* wpFinditem( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	SERIAL serial = INVALID_SERIAL;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.finditem", &serial ) )
		return 0;

	return PyGetItemObject( FindItemBySerial( serial ) );
}

/*
	\function wolfpack.guilds
	\return A list of <object id="guild">guild</object> objects.
	\description Returns a list of all registered guilds in the world.
*/
static PyObject* wpGuilds( PyObject* /*self*/, PyObject* /*args*/ )
{
	PyObject* list = PyList_New( 0 );

	for ( cGuilds::iterator it = Guilds::instance()->begin(); it != Guilds::instance()->end(); ++it )
		PyList_AppendStolen( list, it.value()->getPyObject() );

	return list;
}

/*
	\function wolfpack.findguild
	\param id The guild id to look for, an integer value.
	\return A <object id="guild">guild</object> object or None.
	\description Tries to find a guild with the given id and returns a guild object for it.
	Returns None if no guild could be found.
*/
static PyObject* wpFindguild( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	SERIAL serial;

	if ( !PyArg_ParseTuple( args, "i:wolfpack.findguild(serial)", &serial ) )
		return 0;

	cGuild* guild = Guilds::instance()->findGuild( serial );

	if ( guild )
	{
		return guild->getPyObject();
	}
	else
	{
		Py_RETURN_NONE;
	}
}

/*
	\function wolfpack.findchar
	\param serial The character serial.
	\return A <object id="char">char</object> object or None.
	\description Tries to find a player or npc with the given serial and returns a character
	object. If no character with the given serial could be found it returns None.
*/
static PyObject* wpFindchar( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	SERIAL serial = INVALID_SERIAL;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.findchar", &serial ) )
		return 0;

	return PyGetCharObject( FindCharBySerial( serial ) );
}

/*
	\function wolfpack.findmulti
	\param pos A coordinate object representing the location to look for a multi.
	\return An <object id="item">item</object> object or None.
	\description Tries to find a multi at the given position. Returns None if none can be found.
*/
static PyObject* wpFindmulti( PyObject* /*self*/, PyObject* args )
{
	Coord coord;

	if ( !PyArg_ParseTuple( args, "O&:wolfpack.findmulti(pos)", &PyConvertCoord, &coord ) )
	{
		return 0;
	}

	cMulti* multi = cMulti::find( coord );

	if ( !multi )
	{
		Py_RETURN_NONE;
	}
	else
	{
		return multi->getPyObject();
	}
}

/*
	\function wolfpack.addtimer
	\param expiretime The delay in miliseconds after which this timer should be triggered.
	\param function The function that should be called when this timer is triggered.

	The function should have the following prototype:
	<code>def expire(object, args):
	&nbsp;&nbsp;pass</code>
	Although you can choose any name you want. The object parameter for the trigger function is always
	None when you use <code>wolfpack.addtimer</code> to add the timer.
	\param args This should be a list of custom arguments that will be passed on to the timer expire function.
	If you don't want the timer to be saved you can pass on any type of obejcts. However. You should not pass
	character, item or guild objects directly but their serials instead. Use finditem, findguild and findchar to
	get a new object in the expirefunction afterwards.
	\param serializable Defaults to false. If this boolean parameter is true, the timer will be saved on worldsaves
	and will be triggered even if the server is restarted in between.
	\description This function will schedule a function to be executed later.
*/
static PyObject* wpAddtimer( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	quint32 expiretime;
	PyObject* function;
	PyObject* arguments;
	uchar persistent = 0;

	if ( !PyArg_ParseTuple( args, "iOO!|B:item.addtimer", &expiretime, &function, &PyList_Type, &arguments, &persistent ) )
		return 0;

	PythonFunction* toCall = 0;
	if ( !PyCallable_Check( function ) )
	{
		QString func = extract<QString>( function );
		if ( func.isNull() )
		{
			PyErr_SetString( PyExc_TypeError, "Bad argument on addtimer callback type" );
			return 0;
		}
		Console::instance()->log( LOG_WARNING, tr("Using deprecated string as callback identifier [%1]").arg(func) );
		toCall = new PythonFunction( func );

	        if (!toCall || !toCall->isValid()) {
	                PyErr_Format(PyExc_RuntimeError, "The function callback you specified was invalid: %s", func.toLatin1().constData() );
	                return 0;
	        }

	}
	else
		toCall = new PythonFunction( function );

	if (!toCall || !toCall->isValid()) {
		PyErr_SetString(PyExc_RuntimeError, "The function callback you specified was invalid.");
		return 0;
	}

	PyObject* py_args = PyList_AsTuple( arguments );
	cPythonEffect* effect = new cPythonEffect( toCall, py_args );
	Py_DECREF(py_args);

	// Should we save this effect?
	effect->setSerializable( persistent != 0 );
	effect->setExpiretime_ms( expiretime );
	Timers::instance()->insert( effect );

	Py_RETURN_NONE;
}

/*
	\function wolfpack.region
	\param x The x component of the coordinate.
	\param y The y component of the coordinate.
	\param map Which map should be looked on.
	<code>0 - Felucca
	1 - Trammel
	2 - Ilshenar
	3 - Malas
	4 - Tokuno</code>
	\return A <object id="region">region</object> object or None if there is no region at the given coordinates.
	\description Finds a region at the given coordinates.
*/
static PyObject* wpRegion( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	// Three arguments
	int x = 0,
	y = 0,
	map = 0;
	if ( !PyArg_ParseTuple( args, "iii:wolfpack.region", &x, &y, &map ) )
		return 0;

	return PyGetRegionObject( Territories::instance()->region( x, y, map ) );
}

/*
	\function wolfpack.spawnregion
	\param id The spawnregion id.
	\return A <object id="spawnregion">spawnregion</object> object or None if there is no region with the given name.
	\description Finds a spawnregion with the given id.
*/
static PyObject* wpSpawnregion( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	// Three arguments
	char *name;
	if ( !PyArg_ParseTuple( args, "s:wolfpack.spawnregion", &name ) )
		return 0;

	cSpawnRegion *spawn = SpawnRegions::instance()->region( name );
	return PyGetSpawnRegionObject( spawn );
}

/*
	\function wolfpack.statics
	\param x The x component of the coordinate.
	\param y The y component of the coordinate.
	\param map The map of the coordinate.
	\param exact Defaults to false. If this boolean parameter is true,
	not the entire 8x8 static block matching the coordinate is returned,
	but only the tiles that are exactly at the given coordinate.
	\return Returns a tuple of found static items.
	The static items are represented by 5-tuples containing the following items:
	<code>0: The item id of the static item.
	1: The absolute x coordinate of the static item.
	2: The absolute y coordinate of the static item.
	3: The z position of the static item.
	4: The color of the static item.</code>
	\description This function searches for static tiles at the given coordinate and returns its findings.
*/
static PyObject* wpStatics( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	// Minimum is x, y, map
	int x = 0,
	y = 0,
	map = 0;
	uchar exact = 0;
	if ( !PyArg_ParseTuple( args, "iii|b:wolfpack.statics", &x, &y, &map, &exact ) )
		return 0;

	if ( map == 0xff || !Maps::instance()->hasMap( map ) )
	{
		PyErr_Format( PyExc_RuntimeError, "Unable to access unknown map %u.", map );
		return 0;
	}

	int maxX = Maps::instance()->mapTileWidth( map ) * 8;
	int maxY = Maps::instance()->mapTileHeight( map ) * 8;

	if ( x < 0 )
		x = 0;
	if ( x >= maxX )
		x = maxX - 1;
	if ( y < 0 )
		y = 0;
	if ( y >= maxY )
		y = maxY - 1;

	StaticsIterator iter = Maps::instance()->staticsIterator( Coord( x, y, 0, map ), exact );

	PyObject* tuple = PyTuple_New( iter.count() ); // Create a tuple instead of a list (faster)!
	const quint32 xBlockStart = (x / 8) * 8;
	const quint32 yBlockStart = (y / 8) * 8;

	unsigned int i = 0; // The offset within the tuple
	unsigned int count = iter.count();

	// Make absolutly sure, that we'll never store more objects
	// than allowed in a tuple
	for ( ; !iter.atEnd() && i < count; ++iter, ++i )
	{
		// Create a tuple with the following keys:
		// 1. The id of the static item.
		// 2. The absolute x coordinate of the static item.
		// 3. The absolute y coordinate of the static item.
		// 4. The z position of the static item.
		// 5. The color of the static item.
		PyObject* item = PyTuple_New(5);
        PyTuple_SetItem(item, 0, PyInt_FromLong( iter->itemid ) );
		PyTuple_SetItem(item, 1, PyInt_FromLong( xBlockStart + iter->xoff ) );
		PyTuple_SetItem(item, 2, PyInt_FromLong( yBlockStart + iter->yoff ) );
		PyTuple_SetItem(item, 3, PyInt_FromLong( iter->zoff ) );
		PyTuple_SetItem(item, 4, PyInt_FromLong( iter->color ) );
		PyTuple_SetItem(tuple, i, item );
	}

	return tuple;
}

/*
	\function wolfpack.itemiterator
	\return An <object id="ITEMITERATOR">itemiterator</object> object.
	\description This function creates an itemiterator object to allow iteration
	over all items registered in the world.
*/
static PyObject* wpAllItemsIterator( PyObject* self, PyObject* args )
{
	Q_UNUSED( args );
	Q_UNUSED( self );
	return PyGetItemIterator();
}

/*
	\function wolfpack.chariterator
	\return A <object id="CHARITERATOR">chariterator</object> object.
	\description This function creates a chariterator object to allow iteration
	over all characters registered in the world.
*/
static PyObject* wpAllCharsIterator( PyObject* self, PyObject* args )
{
	Q_UNUSED( args );
	Q_UNUSED( self );
	return PyGetCharIterator();
}

/*
	\function wolfpack.items
	\param x The x component of the coordinate.
	\param y The y component of the coordinate.
	\param map The map to look on.
	\param range Defaults to 0.
	The range of the circle to search in
	\return A list of <object id="item">item</object> objects.
	\description This function searches for dynamic items (no static items) at the given
	coordinate and in the given range and returns a list of found item objects.
*/
static PyObject* wpItems( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	// Minimum is x, y, map
	uint x = 0,
	y = 0,
	map = 0,
	range = 0;
	if ( !PyArg_ParseTuple( args, "iii|i:wolfpack.items", &x, &y, &map, &range ) )
		return 0;

	Coord pos( x, y, 0, map );

	MapItemsIterator iter;
	if ( range > 0 )
	{
		iter = MapObjects::instance()->listItemsInCircle( pos, range );
	}
	else
	{
		iter = MapObjects::instance()->listItemsAtCoord( pos ); // Suppose this is faster
	}

	PyObject* list = PyList_New( 0 );
	for ( P_ITEM pItem = iter.first(); pItem; pItem = iter.next() )
	{
		PyList_AppendStolen( list, PyGetItemObject( pItem ) );
	}

	return list;
}

/*
	\function wolfpack.chars
	\param x The x component of the coordinate.
	\param y The y component of the coordinate.
	\param map The map to look on.
	\param range Defaults to 1.
	This is the range in which the server should search for characters. Please remember that this is not
	a circle.
	\param offline Defaults to False. Indicates whether logged out characters should be included.
	\return A list of <object id="char">char</object> objects.
	\description This function searches for characters at the given
	coordinate and in the given range and returns a list of found objects.
*/
static PyObject* wpChars( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	// Minimum is x, y, map
	uint x = 0,
	y = 0,
	map = 0,
	range = 1;
	PyObject *offline = Py_False;
	if ( !PyArg_ParseTuple( args, "iii|iO:wolfpack.chars", &x, &y, &map, &range, &offline ) )
		return 0;

	Coord pos(x, y, 0, map);

	MapCharsIterator iter = MapObjects::instance()->listCharsInCircle( pos, range, PyObject_IsTrue( offline ) != 0 );

	PyObject* list = PyList_New( 0 );
	for ( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
	{
		PyList_AppendStolen( list, pChar->getPyObject() );
	}

	return list;
}

/*
	\function wolfpack.canplace
	\param pos The position.
	\param id The display id of the multi (0x0000 to 0x4000).
	\param yard The size of the multis yard.
	\return True if the given multi can be placed by a player at the given location.
	\description This function checks if a multi can be placed at a certain location by players.
*/
static PyObject* wpCanPlace( PyObject* self, PyObject* args )
{
	Q_UNUSED( self ); // Warning Fix
	Coord pos;
	unsigned short multiid;
	unsigned short yard;
	if ( !PyArg_ParseTuple( args, "O&hh:wolfpack.canplace(coord, id, yard)", &PyConvertCoord, &pos, &multiid, &yard ) )
	{
		return 0;
	}

	QList<cUObject*> moveOut; // List of objects to move out

	PyObject *result = PyTuple_New( 2 );

	if ( cMulti::canPlace( pos, multiid, moveOut, yard ) )
	{
		Py_INCREF( Py_True );
		PyTuple_SET_ITEM( result, 0, Py_True );

		PyObject *list = PyTuple_New( moveOut.count() );
		int i = 0;
		foreach ( cUObject* obj, moveOut )
		{
			PyTuple_SET_ITEM( list, i++, obj->getPyObject() );
		}
		PyTuple_SET_ITEM( result, 1, list );
	}
	else
	{
		Py_INCREF( Py_True );
		PyTuple_SET_ITEM( result, 0, Py_False );
		PyTuple_SET_ITEM( result, 1, PyTuple_New( 0 ) );
	}

	return result;
}

/*
	\function wolfpack.canplaceboat
	\param pos The position.
	\param id The display id of the multi (0x0000 to 0x4000).
	\param yard The size of the multis yard.
	\return True if the given multi can be placed by a player at the given location.
	\description This function checks if a multi can be placed at a certain location by players.
*/
static PyObject* wpCanPlaceBoat( PyObject* self, PyObject* args )
{
	Q_UNUSED( self ); // Warning Fix
	Coord pos;
	unsigned short multiid;
	if ( !PyArg_ParseTuple( args, "O&h:wolfpack.canplaceboat(coord, id)", &PyConvertCoord, &pos, &multiid ) )
	{
		return 0;
	}

	QList<cUObject*> moveOut; // List of objects to move out

	PyObject *result = PyTuple_New( 2 );

	if ( cMulti::canPlaceBoat( pos, multiid, moveOut ) )
	{
		Py_INCREF( Py_True );
		PyTuple_SET_ITEM( result, 0, Py_True );

		PyObject *list = PyTuple_New( moveOut.count() );
		int i = 0;
		foreach ( cUObject* obj, moveOut )
		{
			PyTuple_SET_ITEM( list, i++, obj->getPyObject() );
		}
		PyTuple_SET_ITEM( result, 1, list );
	}
	else
	{
		Py_INCREF( Py_True );
		PyTuple_SET_ITEM( result, 0, Py_False );
		PyTuple_SET_ITEM( result, 1, PyTuple_New( 0 ) );
	}

	return result;
}

/*
\function wolfpack.canboatmoveto
\param boat the boat object.
\param pos The position.
\return True if the given multi can move to position.
\description This function checks if a multi can advance to target position.
*/
static PyObject* wpCanBoatMoveTo( PyObject* self, PyObject* args )
{
	Q_UNUSED( self ); // Warning Fix
	Coord pos;
	cItem* boat;

	if ( !PyArg_ParseTuple( args, "O&O&:wolfpack.canboatmoveto(boat, coord)", &PyConvertItem, &boat, &PyConvertCoord, &pos ) )
	{
		return 0;
	}

	cMulti* multi = dynamic_cast<cMulti*>( boat );
	if ( !multi )
	{
		PyErr_BadArgument();
		return 0;
	}

	if ( multi->canBoatMoveTo( pos ) )
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\function wolfpack.effect
	\param id The art id of the effect that should be shown.
	\param pos A <object id="coord">coord</object> object representing the position where the effect should be shown.
	\param duration The duration of the effect. This is an integer value.
	\param speed This integer value indicates the animation speed that should be used.
	\description This function shows a graphical effect at a given position to all players in range.
	The exact scale of the duration and speed parameters are unknown and passed on directly to the client.
	You'll have to experiment to find suitable values.
*/
static PyObject* wpEffect( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	// effect-id, position, speed, duration
	if ( !checkArgInt( 0 ) || !checkArgCoord( 1 ) || !checkArgInt( 2 ) || !checkArgInt( 3 ) )
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

	Coord displaypos = getArgCoord( 1 );

	cUOSocket* mSock;
	QList<cUOSocket*> sockets = Network::instance()->sockets();
	foreach ( mSock, sockets )
	{
		if ( mSock->player() && mSock->player()->pos().distance( displaypos ) <= mSock->player()->visualRange() )
			mSock->send( &effect );
	}

	Py_RETURN_NONE;
}

/*
	\function wolfpack.map
	\param x The x component of the coordinate.
	\param y The y component of the coordinate.
	\param map The map to look on.
	\return A dictionary with the following keys:
	- <code>id</code> The landtile art id for the maptile.
	- <code>z</code> The height of the map at the given position. Please note that this
	is the height stored in the datafile and not the real height which is also influenced by
	the surrounding maptiles.
	\description This function returns a dictionary with information about a tile of the map
	at the given coordinate.
*/
static PyObject* wpMap( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	// Minimum is x, y, map
	uint x = 0,
	y = 0,
	map = 0;
	if ( !PyArg_ParseTuple( args, "iii:wolfpack.map", &x, &y, &map ) )
		return 0;

	map_st mTile = Maps::instance()->seekMap( Coord( x, y, 0, map ) );

	PyObject* dict = PyDict_New();
	PyDict_SetStolenItem( dict, "id", PyInt_FromLong( mTile.id ) );
	PyDict_SetStolenItem( dict, "z", PyInt_FromLong( mTile.z ) );
	return dict;
}

/*
	\function wolfpack.hasmap
	\param map The map id.
	\return True or False.
	\description This function returns true if the given map id is available and valid.
*/
static PyObject* wpHasMap( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	uint map = 0;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.hasmap", &map ) )
		return 0;

	return Maps::instance()->hasMap( map ) ? PyTrue() : PyFalse();
}

/*
	\function wolfpack.landdata
	\param id The landtile id.
	\return A dictionary with the following keys:
	- <code>name</code> The name of the tile.
	- <code>unknown1</code>
	- <code>unknown2</code>
	- <code>flag1</code>
	- <code>flag2</code>
	- <code>flag3</code>
	- <code>flag4</code>
	- <code>wet</code> This value is true if the tile contains water.
	- <code>blocking</code> This value is true if the tile is not walkable.
	- <code>floor</code> This value is true if the landtile is a walkable roof or floor.
	\description This function retrieves information about a given landtile id from the tiledata.
*/
static PyObject* wpLanddata( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	unsigned int tileid;

	if ( !PyArg_ParseTuple( args, "i:wolfpack.landdata(id)", &tileid ) )
	{
		return 0;
	}

	land_st tile = TileCache::instance()->getLand( tileid );

	PyObject* dict = PyDict_New();
	PyDict_SetStolenItem( dict, "name", PyString_FromString( tile.name ) );
	PyDict_SetStolenItem( dict, "unknown1", PyInt_FromLong( tile.unknown1 ) );
	PyDict_SetStolenItem( dict, "unknown2", PyInt_FromLong( tile.unknown2 ) );
	PyDict_SetStolenItem( dict, "flag1", PyInt_FromLong( tile.flag1 ) );
	PyDict_SetStolenItem( dict, "flag2", PyInt_FromLong( tile.flag2 ) );
	PyDict_SetStolenItem( dict, "flag3", PyInt_FromLong( tile.flag3 ) );
	PyDict_SetStolenItem( dict, "flag4", PyInt_FromLong( tile.flag4 ) );
	PyDict_SetStolenItem( dict, "wet", PyInt_FromLong( tile.isWet() ) );
	PyDict_SetStolenItem( dict, "blocking", PyInt_FromLong( tile.isBlocking() ) );
	PyDict_SetStolenItem( dict, "floor", PyInt_FromLong( tile.isRoofOrFloorTile() ) );

	QString flags = getFlagNames( tile.flag1, tile.flag2, tile.flag3, tile.flag4 ).join( "," );
	if ( flags.isNull() )
	{
		PyDict_SetStolenItem( dict, "flagnames", PyString_FromString( "" ) );
	}
	else
	{
		PyDict_SetStolenItem( dict, "flagnames", PyString_FromString( flags.toLatin1() ) );
	}

	return dict;
}

/*
	\function wolfpack.tiledata
	\param id The item id.
	\return A dictionary with the following keys:
	- <code>name</code> The name of the tile.
	- <code>height</code> The height of the tile.
	- <code>weight</code> The weight of the tile. This value is not used internally but can be used to check if the item is movable by the client.
	Items with a weight of 255 are not movable by default.
	- <code>layer</code> The layer this item will be equipped on if equippable.
	- <code>animation</code> The id of the animation that will be shown when this item is equipped.
	- <code>quantity</code>
	- <code>unknown1</code>
	- <code>unknown2</code>
	- <code>unknown3</code>
	- <code>unknown4</code>
	- <code>unknown5</code>
	- <code>flag1</code>
	- <code>flag2</code>
	- <code>flag3</code>
	- <code>flag4</code>
	- <code>wet</code> This value is true if the tile contains water.
	- <code>blocking</code> This value is true if the tile is not walkable.
	- <code>floor</code> This value is true if the tile is a walkable roof or floor.
	\description This function retrieves information about a given art tile from the tiledata.
*/
static PyObject* wpTiledata( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	uint tileid = 0;
	if ( !PyArg_ParseTuple( args, "i:wolfpack.tiledata(id)", &tileid ) )
		return 0;

	tile_st tile = TileCache::instance()->getTile( tileid );

	PyObject* dict = PyDict_New();

	// test if item is defined
	if ( !strlen( tile.name ) )
	{
		PyDict_SetStolenItem( dict, "name", PyString_FromString( "unknown" ) );
		PyDict_SetStolenItem( dict, "flag1", PyInt_FromLong( 0 ) );
		PyDict_SetStolenItem( dict, "flag2", PyInt_FromLong( 0 ) );
		PyDict_SetStolenItem( dict, "flag3", PyInt_FromLong( 0 ) );
		PyDict_SetStolenItem( dict, "flag4", PyInt_FromLong( 0 ) );
	}
	else
	{
		PyDict_SetStolenItem( dict, "name", PyString_FromString( tile.name ) );
		PyDict_SetStolenItem( dict, "height", PyInt_FromLong( tile.height ) );
		PyDict_SetStolenItem( dict, "weight", PyInt_FromLong( tile.weight ) );
		PyDict_SetStolenItem( dict, "layer", PyInt_FromLong( tile.layer ) );
		PyDict_SetStolenItem( dict, "animation", PyInt_FromLong( tile.animation ) );
		PyDict_SetStolenItem( dict, "quantity", PyInt_FromLong( tile.quantity ) );
		PyDict_SetStolenItem( dict, "unknown1", PyInt_FromLong( tile.unknown1 ) );
		PyDict_SetStolenItem( dict, "unknown2", PyInt_FromLong( tile.unknown2 ) );
		PyDict_SetStolenItem( dict, "unknown3", PyInt_FromLong( tile.unknown3 ) );
		PyDict_SetStolenItem( dict, "unknown4", PyInt_FromLong( tile.unknown4 ) );
		PyDict_SetStolenItem( dict, "unknown5", PyInt_FromLong( tile.unknown5 ) );
		PyDict_SetStolenItem( dict, "flag1", PyInt_FromLong( tile.flag1 ) );
		PyDict_SetStolenItem( dict, "flag2", PyInt_FromLong( tile.flag2 ) );
		PyDict_SetStolenItem( dict, "flag3", PyInt_FromLong( tile.flag3 ) );
		PyDict_SetStolenItem( dict, "flag4", PyInt_FromLong( tile.flag4 ) );
		PyDict_SetStolenItem( dict, "wet", PyInt_FromLong( tile.isWet() ) );
		PyDict_SetStolenItem( dict, "blocking", PyInt_FromLong( tile.isBlocking() ) );
		PyDict_SetStolenItem( dict, "floor", PyInt_FromLong( tile.isRoofOrFloorTile() ) );

		QString flags = getFlagNames( tile.flag1, tile.flag2, tile.flag3, tile.flag4 ).join( "," );
		if ( flags.isNull() )
		{
			PyDict_SetStolenItem( dict, "flagnames", PyString_FromString( "" ) );
		}
		else
		{
			PyDict_SetStolenItem( dict, "flagnames", PyString_FromString( flags.toLatin1() ) );
		}
	}

	return dict;
}

/*
	\function wolfpack.list
	\param id The id of the list in the definitions.
	\return A list of strings.
	\description This function retrives a list from the definitions and returns it as a list of strings.
*/
static PyObject* wpList( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	if ( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QStringList list = Definitions::instance()->getList( getArgStr( 0 ) );
	PyObject* pylist = PyTuple_New( list.count() );

	for ( uint i = 0; i < list.count(); ++i )
		PyTuple_SetItem( pylist, i, QString2Python( list[i] ) );

	return pylist;
}

/*
	\function wolfpack.registerglobal
	\param event An integer constant for the event that should be hooked.
	Take a look at the "Event Constants" in <module id="wolfpack.consts">wolfpack.consts</module> for details.
	\param script The name of a script that should be notified about the given event.
	\description This function registers a script as a global hook for one given event type. Whenever the
	event is triggered, the given script will be called first. Please note that there can only be one
	hook per event at a time, but a script can hook more than one event to itself.
*/
static PyObject* wpRegisterGlobal( PyObject* /*self*/, PyObject* args )
{
	unsigned int event;
	const char* scriptName;

	if ( !PyArg_ParseTuple( args, "is:wolfpack.registerglobal", &event, &scriptName ) )
		return 0;

	cPythonScript* script = ScriptManager::instance()->find( scriptName );

	if ( script == 0 )
	{
		PyErr_SetString( PyExc_RuntimeError, "Unknown script." );
		return 0;
	}

	if ( event >= EVENT_COUNT )
	{
		PyErr_SetString( PyExc_RuntimeError, "Unknown script." );
		return 0;
	}

	ScriptManager::instance()->setGlobalHook( ( ePythonEvent ) event, script );

	Py_RETURN_NONE;
}

/*
	\function wolfpack.registercommand
	\param command The name of the command as a string.
	\param function The function that should handle the command. This is not a string but a callable
	python function object. The function should have the following prototype:
	<code>
	def mycommand(socket, command, arguments):
	&nbsp;&nbsp;pass
	</code>
	Where socket is the <object id="socket">socket</object> object the command was issued from,
	command is the name of the command that has been issued if you want to use a single function
	for multiple commands and arguments is a string containing the full list of arguments passed
	to the command.
	\description This function registers a python handler for the given command name. Using this function
	you can implement custom ingame commands.
*/
static PyObject* wpRegisterCommand( PyObject* /*self*/, PyObject* args )
{
	const char* command;
	PyObject* function;

	if ( !PyArg_ParseTuple( args, "sO:wolfpack.registercommand", &command, &function ) )
		return 0;

	if ( !PyCallable_Check( function ) )
	{
		PyErr_SetString( PyExc_TypeError, "wolfpack.registercommand( command, function ): function has to be a callable object" );
		return 0;
	}

	Py_INCREF( function );
	ScriptManager::instance()->setCommandHook( command, function );

	Py_RETURN_NONE;
}

/*
	\function wolfpack.registerpackethook
	\param packetid The id of the packet you want to hook.
	\param handler The function that should handle the packet. This is not a string but a callable
	python function object. The function should have the following prototype:
	<code>
	def mycommand(socket, packet):
	&nbsp;&nbsp;pass
	</code>
	Where socket is the <object id="socket">socket</object> object the packet was sent from,
	packet is a <object id="packet">packet</object> object representing the received packet.
	\description This function registers a python handler for a given packet id. If you don't
	want the core to handle the packet after your handling function was called, you have to return
	True in your function, otherwise return False.
*/
static PyObject* wpRegisterPacketHook( PyObject* /*self*/, PyObject* args )
{
	unsigned char packet;
	PyObject* function;

	if ( !PyArg_ParseTuple( args, "bO:wolfpack.registerpackethook(packet, handler)", &packet, &function ) )
	{
		return 0;
	}

	if ( !PyCallable_Check( function ) )
	{
		PyErr_SetString( PyExc_TypeError, "A callable object was expected." );
		return 0;
	}

	cUOSocket::registerPacketHandler( packet, function );
	Py_RETURN_TRUE;
}

/*
	\function wolfpack.coord
	\param x The x component of the coordinate.
	\param y The y component of the coordinate.
	\param z The z component of the coordinate.
	\param map The map component of the coordinate.
	\return A <object id="coord">coord</object> object.
	\description This function creates a new coord object from the
	given coordinate components.
*/
static PyObject* wpCoord( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	uint x = 0,
	y = 0,
	z = 0;
	uchar map = 0;
	if ( !PyArg_ParseTuple( args, "iiib:wolfpack.coord", &x, &y, &z, &map ) )
		return 0;

	Coord pos( x, y, z, map );

	return PyGetCoordObject( pos );
}

/*
	\function wolfpack.addmulti
	\param definition The id of the multi definition.
	\return An <object id="item">item</object> object or None.
	\description Creates a new multi from a given definition id and assigns a new serial to it.
	Please note that multis are represented by the item object.
*/
static PyObject* wpAddMulti( PyObject* /*self*/, PyObject* args )
{
	char* definition;

	if ( !PyArg_ParseTuple( args, "s:wolfpack.addmulti(def)", &definition ) )
	{
		return 0;
	}

	cMulti* multi = cMulti::createFromScript( definition );

	if ( multi )
	{
		return multi->getPyObject();
	}
	else
	{
		Py_RETURN_NONE;
	}
}

/*
	\function wolfpack.serverversion
	\return A string.
	\description Returns the server version.
*/
static PyObject* wpServerVersion( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyString_FromString( QString( "%1 %2 %3" ).arg( productString() ).arg( productBeta() ).arg( productVersion() ).toLatin1() );
}

/*
	\function wolfpack.isstarting
	\return True if the server is in startup phase, false otherwise.
*/
static PyObject* wpIsStarting( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	if ( Server::instance()->getState() == STARTUP )
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\function wolfpack.isrunning
	\return True if the server is in running phase, false otherwise.
*/
static PyObject* wpIsRunning( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	if ( Server::instance()->getState() == RUNNING )
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\function wolfpack.isreloading
	\return True if the server is reloading, false otherwise.
*/
static PyObject* wpIsReloading( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	if ( Server::instance()->getState() == SCRIPTRELOAD )
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\function wolfpack.isclosing
	\return True if the server is shutting down, false otherwise.
*/
static PyObject* wpIsClosing( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	if ( Server::instance()->getState() == SHUTDOWN )
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
	\function wolfpack.charregion
	\param x1 The left edge of the rectangle.
	\param y1 The upper edge of the rectangle.
	\param x2 The right edge of the rectangle.
	\param y2 The bottom edge of the rectangle.
	\param map The map to search on.
	\param offline Defaults to false, indicates whether offline characters should be included.
	\return A <object id="charregioniterator">charregioniterator</object> oject.
	\description This function returns an object that will allow you to iterate over characters
	in the given rectangle on the given map.
*/
static PyObject* wpCharRegion( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	unsigned int x1, y1, x2, y2;
	unsigned char map;
	PyObject *offline = Py_False;

	if ( !PyArg_ParseTuple( args, "iiiib|O:wolfpack.charregion", &x1, &y1, &x2, &y2, &map, &offline ) )
		return 0;

	return PyGetCharRegionIterator( x1, y1, x2, y2, map, PyObject_IsTrue( offline ) != 0 );
}

/*
	\function wolfpack.itemregion
	\param x1 The left edge of the rectangle.
	\param y1 The upper edge of the rectangle.
	\param x2 The right edge of the rectangle.
	\param y2 The bottom edge of the rectangle.
	\param map The map to search on.
	\return A <object id="itemregioniterator">itemregioniterator</object> oject.
	\description This function returns an object that will allow you to iterate over items
	in the given rectangle on the given map.
*/
static PyObject* wpItemRegion( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	unsigned int x1, y1, x2, y2;
	unsigned char map;

	if ( !PyArg_ParseTuple( args, "iiiib:wolfpack.itemregion", &x1, &y1, &x2, &y2, &map ) )
		return 0;

	return PyGetItemRegionIterator( x1, y1, x2, y2, map );
}

/*
	\function wolfpack.newitem
	\param createserial Defaults to true. This boolean value will indicate whether a serial
	will be assigned to the item. If you plan to reset the serial of the item anyway, you
	can set this parameter to false.
	\return An <object id="item">item</object> object for the newly created item.
	\description This function will create a new item object without using a definition.
	Please use this function wisely. It is better to use the additem function whenever possible instead.
*/
static PyObject* wpNewItem( PyObject* /*self*/, PyObject* args )
{
	char createSerial = 1;

	if ( !PyArg_ParseTuple( args, "|b:wolfpack.newitem", &createSerial ) )
		return 0;

	if ( !World::instance()->isLoaded() )
	{
		PyErr_Format(PyExc_RuntimeError, "Trying to create an item before the world has loaded is not supported." );
		Py_RETURN_NONE;
	}

	P_ITEM pItem = new cItem;

	if ( createSerial )
		pItem->Init( true );

	return PyGetItemObject( pItem );
}

/*
	\function wolfpack.newnpc
	\param createserial Defaults to true. This boolean value will indicate whether a serial
	will be assigned to the npc. If you plan to reset the serial of the npc anyway, you
	can set this parameter to false.
	\return A <object id="char">char</object> object for the newly created npc.
	\description This function will create a new npc without using a definition.
	Please use this function wisely. It is better to use the addnpc function whenever possible instead.
*/
static PyObject* wpNewNpc( PyObject* /*self*/, PyObject* args )
{
	char createSerial = 1;

	if ( !PyArg_ParseTuple( args, "|b:wolfpack.newnpc", &createSerial ) )
		return 0;

	if ( !World::instance()->isLoaded() )
	{
		PyErr_Format(PyExc_RuntimeError, "Trying to create an npc before the world has loaded is not supported." );
		Py_RETURN_NONE;
	}

	P_NPC pNpc = new cNPC;

	if ( createSerial )
		pNpc->Init( true );

	return PyGetCharObject( pNpc );
}

/*
	\function wolfpack.newguild
	\return A <object id="guild">guild</object> object.
	\description This function will create a new guild.
*/
static PyObject* wpNewguild( PyObject* /*self*/, PyObject* /*args*/ )
{
	cGuild* guild = new cGuild( true );
	Guilds::instance()->registerGuild( guild );
	return guild->getPyObject();
}

/*
	\function wolfpack.newplayer
	\param createserial Defaults to true. This boolean value will indicate whether a serial
	will be assigned to the player. If you plan to reset the serial of the player anyway, you
	can set this parameter to false.
	\return A <object id="char">char</object> object for the newly created player.
	\description This function will create a new player.
	Please use this function wisely, it is better to let the server handle the creation of new player
	characters.
*/
static PyObject* wpNewPlayer( PyObject* /*self*/, PyObject* args )
{
	char createSerial = 1;

	if ( !PyArg_ParseTuple( args, "|b:wolfpack.newplayer", &createSerial ) )
		return 0;

	P_PLAYER pPlayer = new cPlayer;

	if ( createSerial )
		pPlayer->Init( true );

	return PyGetCharObject( pPlayer );
}

/*
	\function wolfpack.tickcount
	\return An integer value.
	\description This function calculates the current normalized time and returns it.
	It is often faster to use the wolfpack.time.currenttime function instead, but if you need
	an accurate value for timing or similar tasks, use this function instead.
*/
static PyObject* wpTickcount( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyInt_FromLong( getNormalizedTime() );
}

/*
	\function wolfpack.charcount
	\return An integer value.
	\description This function returns the number of registered characters in the world.
*/
static PyObject* wpCharCount( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyLong_FromLong( World::instance()->charCount() );
}

/*
	\function wolfpack.npccount
	\return An integer value.
	\description This function returns the number of registered NPCs in the world.
*/
static PyObject* wpNpcCount( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyLong_FromLong( World::instance()->npcCount() );
}

/*
	\function wolfpack.playercount
	\return An integer value.
	\description This function returns the number of registered player characters in the world.
*/
static PyObject* wpPlayerCount( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyLong_FromLong( World::instance()->playerCount() );
}

/*
	\function wolfpack.itemcount
	\return An integer value.
	\description This function returns the number of registered items in the world.
*/
static PyObject* wpItemCount( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyLong_FromLong( World::instance()->itemCount() );
}

/*
	\function wolfpack.packet
	\param id The packet id.
	\param size The packet size in byte.
	\return A <object id="packet">packet</object> object.
	\description This function creates a new packet object with the given size and sets the first byte of the packet to the given packet id.
*/
static PyObject* wpPacket( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	unsigned int id, size;

	if ( !PyArg_ParseTuple( args, "ii:wolfpack.packet", &id, &size ) )
		return 0;

	if ( size == 0 )
		Py_RETURN_NONE;

	return CreatePyPacket( ( unsigned char ) id, ( unsigned short ) size );
}

/*
	\function wolfpack.queueaction
	\param action The action you want to queue.
	Take a look at the "Action Constants" in the <module id="wolfpack.consts">wolfpack.consts</module> module.
	\description This function queues an action to be executed in the next iteration of the mainloop.
*/
void wpQueueAction( unsigned int action )
{
	Server::instance()->queueAction( ( enActionType ) action );
}

/*
	\function wolfpack.queuecode
	\param function A python code object.
	\param arguments A tuple.
	\description This function queues a python code object to be executed in the main thread.
	This funciton is useful if you want to access wolfpack internals from within another
	python thread. Use this function to queue code that should be executed synchronized with the
	main thread.
*/
static PyObject* wpQueueCode( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );

	PyObject *code,* pargs;

	if ( !PyArg_ParseTuple( args, "OO!:wolfpack.queuecode( code, args )", &code, &PyTuple_Type, &pargs ) )
		return 0;

	Server::instance()->queueAction( new cActionPythonCode( code, pargs ) );

	Py_RETURN_NONE;
}

/*
	\function wolfpack.exportdefinitions
	\param filename The filename of the category database.
	\description This function behaves like the exportdefinitions command. It exports a WPGM compatible database
	to the given filename.
*/
void wpExportDefinitions( const QString& filename )
{
	cDefinitionExporter exporter;
	exporter.generate(filename);
}

/*
	\function wolfpack.getdefinition
	\param type The definition type.
	Use one of the "Definition Constants" from <module id="wolfpack.consts">wolfpack.consts</module>.
	\param id A string representing the id of the desired definition section.
	\return None if the section could not be found or an <object id="element">element</object> object otherwise.
	\description This function tries to find a section with the given type and id in the definitions and returns
	an element object to access its data.
*/
static PyObject* wpGetDefinition( PyObject* /*self*/, PyObject* args )
{
	unsigned int type;
	char* name;

	if ( !PyArg_ParseTuple( args, "Ies:getdefinition(type, id)", &type, "utf-8", &name ) )
	{
		return 0;
	}

	const cElement* element = Definitions::instance()->getDefinition( ( eDefCategory ) type, QString::fromUtf8( name ) );

	PyMem_Free( name );

	if ( element )
	{
		return const_cast<cElement*>( element )->getPyObject();
	}
	else
	{
		Py_RETURN_NONE;
	}
}

/*
	\function wolfpack.getdefinitions
	\param type The definition type.
	Use one of the "Definition Constants" from <module id="wolfpack.consts">wolfpack.consts</module>.
	\return A tuple of strings.
	\description This function will compile the ids of all sections of a given type.
*/
static PyObject* wpGetDefinitions( PyObject* /*self*/, PyObject* args )
{
	unsigned int type;

	if ( !PyArg_ParseTuple( args, "I:wolfpack.getdefinitions(type)", &type ) )
	{
		return 0;
	}

	const QList<cElement*> elements = Definitions::instance()->getDefinitions( ( eDefCategory ) type );
	QStringList sections = Definitions::instance()->getSections( ( eDefCategory ) type );

	PyObject* result = PyTuple_New( elements.size() + sections.size() );

	uint i = 0;
	for ( ; i < elements.size(); ++i )
	{
		PyTuple_SetItem( result, i, elements[i]->getPyObject() );
	}

	for ( uint j = 0; j < sections.size(); ++j )
	{
		cElement* element = const_cast<cElement*>( Definitions::instance()->getDefinition( ( eDefCategory ) type, sections[j] ) );
		PyTuple_SetItem( result, i++, element->getPyObject() );
	}

	return result;
}

/*
	\function wolfpack.definitionsiterator
	\param type The definition type.
	Use one of the "Definition Constants" from <module id="wolfpack.consts">wolfpack.consts</module>.
	\return A definitionsiterator object.
	\description This function will create and return a definitionsiterator object for the given definition type.
*/
static PyObject* wpDefinitionsIterator( PyObject* /*self*/, PyObject* args )
{
	unsigned int type;

	if ( !PyArg_ParseTuple( args, "I:wolfpack.definitionsiterator(type)", &type ) ) {
		return 0;
	}

	return PyGetDefinitionIterator((eDefCategory)type);
}

/*
	\function wolfpack.callevent
	\param script The id of the script containing the event.
	\param event The numeric constant for the event you want to call.
	See the "Event Constants" in <module id="wolfpack.consts">wolfpack.consts</module> for details.
	\param args A tuple containing the arguments for the event handler.
	\return The return value from the event handler is passed trough.
	\description This function calls an event handler in the given script.
*/
static PyObject* wpCallEvent( PyObject* /*self*/, PyObject* args )
{
	char* script;
	unsigned int event;
	PyObject* eventargs;

	if ( !PyArg_ParseTuple( args, "sIO!:wolfpack.callevent(scriptname, event, args)", &script, &event, &PyTuple_Type, &eventargs ) )
	{
		return 0;
	}

	cPythonScript* pythonscript = ScriptManager::instance()->find( script );

	if ( !pythonscript )
	{
		PyErr_SetString( PyExc_ValueError, "You tried to access an unknown script." );
		return 0;
	}

	PyObject* result = pythonscript->callEvent( ( ePythonEvent ) event, eventargs );

	if ( !result )
	{
		Py_RETURN_NONE;
	}

	return result;
}

/*
	\function wolfpack.hasevent
	\param script The id of the script containing the event.
	\param event The numeric constant for the event you want to check for.
	See the "Event Constants" in <module id="wolfpack.consts">wolfpack.consts</module> for details.
	\return True or false.
	\description This function checks if the given script can handle an event of the given type and returns true if it can.
*/
bool wpHasEvent( const QString& script, unsigned int event )
{
	cPythonScript* pythonscript = ScriptManager::instance()->find( script.toLatin1() );

	if ( !pythonscript )
	{
		PyErr_SetString( PyExc_ValueError, "You tried to access an unknown script." );
		throw_error_already_set();
	}

	return pythonscript->canHandleEvent( ( ePythonEvent ) event );
}

/*
	\function wolfpack.callnamedevent
	\param script The id of the script containing the event.
	\param event The name of the event handler function you want to call.
	\param args A tuple containing the arguments for the event handler.
	\return The return value from the event handler is passed trough.
	\description This function calls an event handler in the given script.
*/
static PyObject* wpCallNamedEvent( PyObject* /*self*/, PyObject* args )
{
	char* script;
	char* event;
	PyObject* eventargs;

	if ( !PyArg_ParseTuple( args, "ssO!:wolfpack.callevent(scriptname, event, args)", &script, &event, &PyTuple_Type, &eventargs ) )
	{
		return 0;
	}

	cPythonScript* pythonscript = ScriptManager::instance()->find( script );

	if ( !pythonscript )
	{
		PyErr_SetString( PyExc_ValueError, "You tried to access an unknown script." );
		return 0;
	}

	PyObject* result = pythonscript->callEvent( event, eventargs );

	if ( !result )
	{
		Py_RETURN_NONE;
	}

	return result;
}

/*
	\function wolfpack.hasnamedevent
	\param script The id of the script containing the event.
	\param event The name of the event handler you want to check for.
	\return True or false.
	\description This function checks if the given script can handle an event with the given name and returns true if it can.
*/
bool wpHasNamedEvent( const QString& script, const QString& event )
{
	cPythonScript* pythonscript = ScriptManager::instance()->find( script.toLatin1() );

	if ( !pythonscript )
	{
		PyErr_SetString( PyExc_ValueError, "You tried to access an unknown script." );
		throw_error_already_set();
	}

	return pythonscript->canHandleEvent( event );
}

/*
	\function wolfpack.getoption
	\param key The name of the option.
	\param default The default value that is returned if the option does not exist.
	\return A string with the value of the option.
	\description This function retrieves an option from the world database.
*/
QString wpGetOption( const QString& arg_key, const QString& arg_def )
{
	QString value;
	World::instance()->getOption( arg_key, value, arg_def );
	return value;
}

/*
	\function wolfpack.setoption
	\param key The name of the option.
	\param value A string containing the value of the option.
	\description This function sets a given option in the world database.
*/
void wpSetOption( const QString& arg_key, const QString& arg_val )
{
	World::instance()->setOption( arg_key, arg_val );
}

/*
	\function wolfpack.bodyinfo
	\param bodyid The id of the character body to get the information for.
	This is a numeric value.
	\return A dictionary with the following keys:<code>
	- basesound
	- flags
	- figurine
	- type
	- body
	- mountid
	</code>
	\description Retrieve information about a given character body.
*/
static PyObject* wpBodyInfo( PyObject* /*self*/, PyObject* args )
{
	unsigned short body;
	if ( !PyArg_ParseTuple( args, "H:wolfpack.bodyinfo(body)", &body ) )
	{
		return 0;
	}

	const stBodyInfo &info = CharBaseDefs::instance()->getBodyInfo( body );

	PyObject *dict = PyDict_New();
	PyDict_SetStolenItem( dict, "basesound", PyInt_FromLong( info.basesound ) );
	PyDict_SetStolenItem( dict, "body", PyInt_FromLong( info.body ) );
	PyDict_SetStolenItem( dict, "figurine", PyInt_FromLong( info.figurine ) );
	PyDict_SetStolenItem( dict, "mountid", PyInt_FromLong( info.mountid ) );
	PyDict_SetStolenItem( dict, "flags", PyInt_FromLong( info.flags ) );
	PyDict_SetStolenItem( dict, "type", PyInt_FromLong( info.type ) );
	return dict;
}

/*
	\function wolfpack.charbase
	\param id The definition id.
	\return A <object id="basedef">basedef</object> object.
	\description Get a <object id="basedef">basedef</object> object for a given npc definition id.
*/
static PyObject* wpCharBase( PyObject*, PyObject* args )
{
	char *id;
	if ( !PyArg_ParseTuple( args, "s:wolfpack.charbase(id)", &id ) )
	{
		return 0;
	}

	cCharBaseDef *basedef = CharBaseDefs::instance()->get( id );

	if ( !basedef )
	{
		Py_RETURN_NONE;
	}
	else
	{
		return basedef->getPyObject();
	}
}

/*
	\function wolfpack.itembase
	\param id The definition id.
	\return A <object id="basedef">basedef</object> object.
	\description Get a <object id="basedef">basedef</object> object for a given item definition id.
*/
static PyObject* wpItemBase( PyObject*, PyObject* args )
{
	char *id;
	if ( !PyArg_ParseTuple( args, "s:wolfpack.itembase(id)", &id ) )
	{
		return 0;
	}

	cItemBaseDef *basedef = ItemBaseDefs::instance()->get( id );

	if ( !basedef )
	{
		Py_RETURN_NONE;
	}
	else
	{
		return basedef->getPyObject();
	}
}

/*
	\function wolfpack.tr
	\param message The message to be translated.
	\return The translated message into the current server's language.
	\description Get a message in English and translates it into the current server's language.
*/
QString wpTr( const char* message )
{
	return QCoreApplication::translate( "@pythonscript", message );
}


static PyMethodDef wpGlobal[] =
{
{ "npccount",			wpNpcCount,						METH_VARARGS, 0 },
{ "charbase",			wpCharBase,						METH_VARARGS, 0 },
{ "itembase",			wpItemBase,						METH_VARARGS, 0 },
{ "playercount",		wpPlayerCount,					METH_VARARGS, 0 },
{ "bodyinfo",			wpBodyInfo,						METH_VARARGS, 0 },
{ "charbase",			wpCharBase,						METH_VARARGS, 0 },
{ "callevent",			wpCallEvent,					METH_VARARGS, "Call an event in a script and return the result." },
{ "callnamedevent",		wpCallNamedEvent,				METH_VARARGS, "Call an event in a script and return the result." },
{ "getdefinition",		wpGetDefinition,				METH_VARARGS, "Gets a certain definition by it's id." },
{ "getdefinitions",		wpGetDefinitions,				METH_VARARGS, "Gets all definitions by type." },
{ "packet",				wpPacket,						METH_VARARGS, NULL },
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
{ "canplace",			wpCanPlace,						METH_VARARGS, 0 },
{ "canplaceboat",		wpCanPlaceBoat,					METH_VARARGS, 0 },
{ "canboatmoveto",		wpCanBoatMoveTo,				METH_VARARGS, 0 },
{ "region",				wpRegion,						METH_VARARGS, "Gets the region at a specific position" },
{ "spawnregion",		wpSpawnregion,					METH_VARARGS, 0 },
{ "newguild",			wpNewguild,						METH_VARARGS, 0},
{ "statics",			wpStatics,						METH_VARARGS, "Returns a list of static-item at a given position" },
{ "map",				wpMap,							METH_VARARGS, "Returns a dictionary with information about a given map tile" },
{ "hasmap",				wpHasMap,						METH_VARARGS, "Returns true if the map specified is present"	},
{ "items",				wpItems,						METH_VARARGS, "Returns a list of items in a specific sector." },
{ "itemiterator",		wpAllItemsIterator,				METH_NOARGS,  "Returns an iterator for all items in the world."	},
{ "chariterator",		wpAllCharsIterator,				METH_NOARGS,  "Returns an iterator for all chars in the world."	},
{ "chars",				wpChars,						METH_VARARGS, "Returns a list of chars in a specific sector." },
{ "landdata",			wpLanddata,						METH_VARARGS, "Returns the landdata information for a given tile stored on the server." },
{ "tiledata",			wpTiledata,						METH_VARARGS, "Returns the tiledata information for a given tile stored on the server." },
{ "coord",				wpCoord,						METH_VARARGS, "Creates a coordinate object from the given parameters (x,y,z,map)." },
{ "addmulti",			wpAddMulti,						METH_VARARGS, "Creates a multi object by given type CUSTOMHOUSE, HOUSE, BOAT." },
{ "list",				wpList,							METH_VARARGS, "Returns a list defined in the definitions as a Python List" },
{ "registerglobal",		wpRegisterGlobal,				METH_VARARGS, "Registers a global script hook." },
{ "registerpackethook", wpRegisterPacketHook,			METH_VARARGS, "Registers a packet hook." },
{ "registercommand",	wpRegisterCommand,				METH_VARARGS, "Registers a global command hook." },
{ "serverversion",		wpServerVersion,				METH_NOARGS, "Returns the server version string." },
{ "isstarting",			wpIsStarting,					METH_NOARGS, "Returns if the server is in starting state" },
{ "isrunning",			wpIsRunning,					METH_NOARGS, "Returns if the server is in running state" },
{ "isreloading",		wpIsReloading,					METH_NOARGS, "Returns if the server is in reload state" },
{ "isclosing",			wpIsClosing,					METH_NOARGS, "Returns if the server is in closing state" },
{ "tickcount",			wpTickcount,					METH_NOARGS, "Returns the current Tickcount on Windows" },
{ "queuecode",			wpQueueCode,					METH_VARARGS, NULL },
{ "charcount",			wpCharCount,					METH_NOARGS,  "Returns the number of chars in the world" },
{ "itemcount",			wpItemCount,					METH_NOARGS,  "Returns the number of items in the world" },
{ "definitionsiterator",wpDefinitionsIterator,			METH_VARARGS, NULL },
{ NULL, NULL, 0, NULL } // Terminator

};

/*
	\function wolfpack.sockets.first
	\return A <object id="socket">socket</object> object or None.
	\description This function is used to iterate thru the ingame sockets. It resets the iterator to the first 
	available socket and returns it. 
	\sa wolfpack.sockets.next
*/
QList<cUOSocket*> sockets;
int socketsIndex;

static PyObject* wpSocketsFirst( PyObject* /*self*/, PyObject* /*args*/ )
{
	socketsIndex = 0;
	sockets = Network::instance()->sockets();
	foreach( cUOSocket* s, sockets )
	{
		if ( s->state() != cUOSocket::InGame )
			sockets.removeAll( s );
	}

	if ( socketsIndex < sockets.count() )
		return PyGetSocketObject( sockets[socketsIndex] );
	else
		Py_RETURN_NONE;
}

/*
	\function wolfpack.sockets.next
	\return A <object id="socket">socket</object> object or None.
	\description This function sets the iterator to the next ingame available socket
	and returns it. If there is no more sockets available, None is returned.
	\sa wolfpack.sockets.first
*/
static PyObject* wpSocketsNext( PyObject* /*self*/, PyObject* /*args*/ )
{
	while ( ++socketsIndex < sockets.count() && sockets[socketsIndex]->state() != cUOSocket::InGame );

	if ( socketsIndex < sockets.count() )
		return PyGetSocketObject( sockets[socketsIndex] );
	else
		Py_RETURN_NONE;
}

/*
	\function wolfpack.sockets.count
	\return An integer value.
	\description This function returns how many sockets are connected. There is no distinction 
	on their state, which could be logging in sockets or ingame sockets.
*/
static PyObject* wpSocketsCount( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	return PyInt_FromLong( Network::instance()->count() );
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

/*
	\function wolfpack.accounts.count
	\return An integer value.
	\description This function returns the number of accounts on the server.
*/
static PyObject* wpAccountsCount( PyObject* /*self*/, PyObject* /*args*/ )
{
	return PyInt_FromLong( Accounts::instance()->count() );
}

/*
	\function wolfpack.accounts.find
	\param name A string containing the account name.
	\return An <object id="account">account</object> object if an account was found.
	None otherwise.
	\description This function tries to find an account with the given name and returns it.
*/
static PyObject* wpAccountsFind( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	if ( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	cAccount* account = Accounts::instance()->getRecord( getArgStr( 0 ) );
	return PyGetAccountObject( account );
}

/*
	\function wolfpack.accounts.list
	\return A tuple of strings.
	\description This function generates a tuple of all account names and returns it.
*/
static PyObject* wpAccountsList( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	PyObject* tuple = PyTuple_New( Accounts::instance()->count() );

	cAccounts::const_iterator it = Accounts::instance()->begin();
	unsigned int i = 0;
	while ( it != Accounts::instance()->end() )
	{
		const QString &login = ( *it )->login();
		PyTuple_SetItem( tuple, i++, QString2Python( login ) );
		++it;
	}

	return tuple;
}

/*
	\function wolfpack.accounts.acls
	\return A list of strings.
	\description This function generates a list of the names of all available acls and returns it.
*/
static PyObject* wpAccountsAcls( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	PyObject* list = PyList_New( 0 );

	QMap<QString, cAcl*>::const_iterator it = Commands::instance()->aclbegin();
	for ( ; it != Commands::instance()->aclend(); ++it )
	{
		QString name = it.key();
		if ( !name.isEmpty() )
			PyList_AppendStolen( list, PyString_FromString( name.toLatin1().constData() ) );
	}

	return list;
}

/*
	\function wolfpack.accounts.acl
	\param acl The name of the acl.
	\return None if no acl with the given name could be found. Otherwise a dictionary
	with information about the acl is returned. For each group in the acl it contains
	a key-value pair where the value is another dictionary containing the actions within the
	group. If for instance you want to check if the gm acl has access to any command, the following
	code should give you an idea:
	<code>
	allowed = 0
	acl = wolfpack.accounts.acl('gm')
	if acl.has_key('command') and acl['command'].has_key('any'):
	&nbsp;&nbsp;allowed = acl['command']['any'].lower() == 'true'</code>
	\description Retrieve an access control list from the server.
*/
static PyObject* wpAccountsAcl( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	if ( !checkArgStr( 0 ) )
	{
		Py_RETURN_NONE;
	}

	cAcl* acl = Commands::instance()->getACL( getArgStr( 0 ) );
	if ( !acl )
	{
		Py_RETURN_NONE;
	}

	PyObject* dict = PyDict_New();

	QMap<QString, QMap<QString, bool> >::const_iterator git;
	for ( git = acl->groups.begin(); git != acl->groups.end(); ++git )
	{
		PyObject* dict2 = PyDict_New();

		for ( QMap<QString, bool>::const_iterator it = ( *git ).begin(); it != ( *git ).end(); ++it ) {
			PyObject *key = QString2Python(it.key());
			PyDict_SetItem( dict2, key, it.value() ? Py_True : Py_False );
			Py_DECREF(key);
		}

		PyObject *key = QString2Python(git.key());
		PyDict_SetItem( dict, key, dict2 );
		Py_DECREF(key);
	}

	return dict;
}

/*
	\function wolfpack.accounts.add
	\param username A string containing the username of the new account.
	\param password A string containing the password for the new account.
	If MD5 password hashing is enabled, this password will be automatically converted.
	\return None or an <object id="account">account</object> object.
	\description This function creates a new account and returns it. If the account couldn't be created,
	it returns None.
*/
static PyObject* wpAccountsAdd( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	if ( !checkArgStr( 0 ) && !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	QString login = getArgStr( 0 );
	QString password = getArgStr( 1 );

	if ( login.length() < 1 && password.length() < 1 )
	{
		Py_RETURN_NONE;
	}

	cAccount* account = Accounts::instance()->getRecord( login );

	if ( account )
	{
		Py_RETURN_NONE;
	}

	account = Accounts::instance()->createAccount( login, password );
	return PyGetAccountObject( account );
}

/*
	\function wolfpack.accounts.reload
	\description Reload the accounts in the next mainloop tick. This means
	that the accounts will not be reloaded instantly.
*/
static PyObject* wpAccountsReload( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	Server::instance()->queueAction( RELOAD_ACCOUNTS );
	Py_RETURN_TRUE;
}

/*
	\function wolfpack.accounts.save
	\description Save the account database during the next mainloop iteration.
	That means that the accounts will not be saved instantly.
*/
static PyObject* wpAccountsSave( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	Server::instance()->queueAction( SAVE_ACCOUNTS );
	Py_RETURN_TRUE;
}

/*!
	wolfpack.accounts
	account related functions
*/
static PyMethodDef wpAccounts[] =
{
{ "count",		wpAccountsCount,	METH_VARARGS, "" },
{ "find",		wpAccountsFind,		METH_VARARGS, "Finds an account object." },
{ "list",		wpAccountsList,		METH_NOARGS, "Gets a list of Account names." },
{ "acls",		wpAccountsAcls,		METH_NOARGS, "Gets a list of valid ACL names." },
{ "acl",		wpAccountsAcl,		METH_VARARGS, "Returns an acl as a double dictionary." },
{ "add",		wpAccountsAdd,		METH_VARARGS, "Creates an account." },
{ "save",		wpAccountsSave,		METH_NOARGS, "Save accounts." },
{ "reload",		wpAccountsReload,	METH_NOARGS, "Reload accounts." },
{ NULL, NULL, 0, NULL } // Terminator

};

/*
	\function wolfpack.settings.getbool
	\param group A string containing the name of the group the configuration option is in.
	\param key A string containing the name of the configuration option within the given group.
	\param default If the configuration option does not exist, this value is returned instead.
	\param create Defaults to false. If this is true and the option does not exist, the value given
	as the default value will be written to the configuration file.
	\return A boolean value.
	\description This function retrieves a value from the configuration file.
*/
bool wpSettingsGetBool( const QString& group, const QString& key, bool def, bool create = false )
{
	return Config::instance()->getBool( group, key, def, create );
}

/*
	\function wolfpack.settings.setbool
	\param group A string containing the name of the group the configuration option is in.
	\param key A string containing the name of the configuration option within the given group.
	\param value The new value of the configuration option.
	\description This function changes or creates a new configuration option.
*/
void wpSettingsSetBool( const QString& group, const QString& key, bool value )
{
	Config::instance()->setBool( group, key, value );
}

/*
	\function wolfpack.settings.getnumber
	\param group A string containing the name of the group the configuration option is in.
	\param key A string containing the name of the configuration option within the given group.
	\param default If the configuration option does not exist, this value is returned instead.
	\param create Defaults to false. If this is true and the option does not exist, the value given
	as the default value will be written to the configuration file.
	\return An integer value.
	\description This function retrieves a value from the configuration file.
*/
long wpSettingsGetNumber( const QString& group, const QString& key, long def, bool create = false )
{
	return Config::instance()->getNumber( group, key, def, create );
}

/*
	\function wolfpack.settings.setnumber
	\param group A string containing the name of the group the configuration option is in.
	\param key A string containing the name of the configuration option within the given group.
	\param value The new value of the configuration option.
	\description This function changes or creates a new configuration option.
*/
void wpSettingsSetNumber( const QString& group, const QString& key, long value )
{
	Config::instance()->setNumber( group, key, value );
}

/*
	\function wolfpack.settings.getstring
	\param group A string containing the name of the group the configuration option is in.
	\param key A string containing the name of the configuration option within the given group.
	\param default If the configuration option does not exist, this value is returned instead.
	\param create Defaults to false. If this is true and the option does not exist, the value given
	as the default value will be written to the configuration file.
	\return A string.
	\description This function retrieves a value from the configuration file.
*/
QString wpSettingsGetString( const QString& group, const QString& key, const QString& def, bool create = false )
{
	return Config::instance()->getString( group, key, def, create );
}

/*
	\function wolfpack.settings.setstring
	\param group A string containing the name of the group the configuration option is in.
	\param key A string containing the name of the configuration option within the given group.
	\param value The new value of the configuration option.
	\description This function changes or creates a new configuration option.
*/
void wpSettingsSetString( const QString& group, const QString& key, const QString& value )
{
	Config::instance()->setString( group, key, value );
}

/*
	\function wolfpack.settings.removekey
	\param group String containing the name of the group.
	\param key String containing the name of the configuration option within the given group.
	\description This function removes the specified group/key pair from the configuration file.
*/
void wpSettingsRemoveKey( const QString& group, const QString& key )
{
	Config::instance()->removeKey( group, key );
}

/*
	\function wolfpack.settings.reload
	\description This function reloads the settings file.
*/
void wpSettingsReload()
{
	Config::instance()->reload();
}

/*
	\function wolfpack.settings.save
	\description This function saves the settings file to disk.
*/
void wpSettingsSave()
{
	Config::instance()->flush();
}

/*
	\function wolfpack.database.query
	\param query A string containing the SQL query for the database.
	\return A <object id="dbresult">dbresult</object> object.
	\description This function executes the given SQL query in the currently connected database and returns the result.
*/
static PyObject* wpQuery( PyObject* /*self*/, PyObject* args )
{
	char* query;

	if ( !PyArg_ParseTuple( args, "es:wolfpack.database.query(query)", "utf-8", &query ) )
	{
		return 0;
	}

	try
	{
		QSqlQuery result = PersistentBroker::instance()->query( query );

		PyMem_Free( query );

		return getPyObjectFromQSqlQuery( result );
	}
	catch ( wpException e )
	{
		PyMem_Free( query );
		PyErr_SetString( PyExc_RuntimeError, e.error().toLatin1() );
		return 0;
	}
	catch ( ... )
	{
		PyMem_Free( query );
		PyErr_SetString( PyExc_RuntimeError, "An error occured while querying the database." );
		return 0;
	}

	// we should never get here
	return 0;
}

/*
	\function wolfpack.database.execute
	\param query A string containing the SQL statement.
	\description This function executes the given SQL query in the currently connected database and discards the result if there is any.
*/
static PyObject* wpExecute( PyObject* /*self*/, PyObject* args )
{
	PyObject *pquery;
	QString query;

	if ( !PyArg_ParseTuple( args, "O:wolfpack.database.execute(query)", &pquery ) )
	{
		return 0;
	}

	query = extract<QString>(pquery);

	try
	{
		PersistentBroker::instance()->executeQuery( query );
	}
	catch ( wpException & e )
	{
		PyErr_SetString( PyExc_RuntimeError, e.error().toLatin1() );
		return 0;
	}
	catch ( ... )
	{
		PyErr_SetString( PyExc_RuntimeError, "An error occured while querying the database." );
		return 0;
	}

	Py_RETURN_NONE;
}

/*
	\function wolfpack.database.driver
	\param database The id of the database you want to query. See the "Database Constants" in this module.
	\return A string.
	\description This function returns the name of the database driver in use for the given database.
*/
static PyObject* wpDriver( PyObject* self, PyObject* args )
{
	Q_UNUSED( self );
	Q_UNUSED( args );
	unsigned int database;

	if ( !PyArg_ParseTuple( args, "I:wolfpack.database.driver(database)", &database ) )
		return 0;

	QString driver( "unknown" );

	if ( database == 1 )
		driver = Config::instance()->accountsDriver();
	else if ( database == 2 )
		driver = Config::instance()->databaseDriver();

	return PyString_FromString( driver.toLatin1() );
}

/*
	\function wolfpack.database.close
	\param database The id of the database you want to close. See the "Database Constants" in this module.
	\description This function closes the connection to the given database.
*/
static PyObject* wpClose( PyObject* /*self*/, PyObject* /*args*/ )
{
	try
	{
		PersistentBroker::instance()->disconnect();
	}
	catch ( ... )
	{
		PyErr_SetString( PyExc_RuntimeError, "Error while disconnecting from the database." );
		return 0;
	}

	Py_RETURN_NONE;
}

/*
	\function wolfpack.database.open
	\param database The id of the database you want to open. See the "Database Constants" in this module.
	\description This function opens the connection to the given database.
*/
static PyObject* wpOpen( PyObject* /*self*/, PyObject* args )
{
	unsigned int database;

	if ( !PyArg_ParseTuple( args, "I:wolfpack.database.open(database)", &database ) )
	{
		return 0;
	}

	try
	{
		if ( database == 1 )
		{
			PersistentBroker::instance()->connect( Config::instance()->accountsHost(), Config::instance()->accountsName(), Config::instance()->accountsUsername(), Config::instance()->accountsPassword(), Config::instance()->accountsPort() );
		}
		else if ( database == 2 )
		{
			PersistentBroker::instance()->connect( Config::instance()->databaseHost(), Config::instance()->databaseName(), Config::instance()->databaseUsername(), Config::instance()->databasePassword(), Config::instance()->databasePort() );
		}
	}
	catch ( wpException& e )
	{
		PyErr_SetString( PyExc_RuntimeError, e.error().toLatin1() );
		return 0;
	}
	catch ( ... )
	{
		PyErr_SetString( PyExc_RuntimeError, "Error while connecting to the database." );
		return 0;
	}

	Py_RETURN_TRUE;
}

static PyMethodDef wpDatabase[] =
{
{ "query",				wpQuery,	METH_VARARGS, "Executes a sql query and returns the result." },
{ "execute",			wpExecute,	METH_VARARGS, "Executes a sql query and dont return a result." },
{ "driver",				wpDriver,	METH_VARARGS, "Returns the name of the database driver used." },
{ "close",				wpClose,	METH_VARARGS, "Closes the database." },
{ "open",				wpOpen,		METH_VARARGS, "Opens the database." },
{ 0, 0, 0, 0 }
};

BOOST_PYTHON_MODULE(wpconsole)
{
	def( "log", wpConsole_log );
	def( "send", wpConsole_send );
	def( "sendprogress", wpConsole_sendprogress );
	def( "senddone", wpConsole_senddone );
	def( "sendfail", wpConsole_sendfail );
	def( "sendskip", wpConsole_sendskip );
	def( "getbuffer", wpConsole_getbuffer );
	def( "shutdown", wpConsole_shutdown );
}

void init_wolfpack_console()
{
	boost::python::detail::init_module("_wolfpack.console", initwpconsole);
}

BOOST_PYTHON_MODULE(wptime)
{
	def( "minute", wpTime_minute );
	def( "hour", wpTime_hour );
	def( "days", wpTime_days );
	def( "minutes", wpTime_minutes );
	def( "currentlightlevel", wpTime_currentlightlevel );
	def( "currenttime", wpCurrenttime );
}

void init_wolfpack_time()
{
	boost::python::detail::init_module("_wolfpack.time", initwptime);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(wpSettingsGetBoolOverloads, wpSettingsGetBool, 3, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(wpSettingsGetNumberOverloads, wpSettingsGetNumber, 3, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(wpSettingsGetStringOverloads, wpSettingsGetString, 3, 4)

BOOST_PYTHON_MODULE(wpsettings)
{
	def( "getbool", wpSettingsGetBool, wpSettingsGetBoolOverloads() );
	def( "setbool", wpSettingsSetBool );
	def( "getnumber", wpSettingsGetNumber, wpSettingsGetNumberOverloads() );
	def( "setnumber", wpSettingsSetNumber );
	def( "getstring", wpSettingsGetString, wpSettingsGetStringOverloads() );
	def( "setstring", wpSettingsSetString );
	def( "removekey", wpSettingsRemoveKey );
	def( "reload", wpSettingsReload );
	def( "save", wpSettingsSave );
}

void init_wolfpack_settings()
{
	boost::python::detail::init_module("_wolfpack.settings", initwpsettings);
}

/*!
	This initializes the _wolfpack namespace and it's sub extensions
*/
BOOST_PYTHON_MODULE(_wolfpack)
{
	// Namespace level definitions
	def("queueaction", wpQueueAction );
	def("exportdefinitions", wpExportDefinitions );
	def("hasevent", wpHasEvent );
	def("hasnamedevent", wpHasNamedEvent );
	def("getoption", wpGetOption );
	def("setoption", wpSetOption );
	def("tr", wpTr );

	object wpNamespace( ( handle<>( borrowed( Py_InitModule( "_wolfpack", wpGlobal ) ) ) ) );

	init_wolfpack_console();
	object mConsole ( ( handle<>( borrowed(PyImport_AddModule("wpconsole") ) ) ) );
	scope().attr("console") = mConsole;

	init_wolfpack_time();
	object mTime ( ( handle<>( borrowed( PyImport_AddModule( "wptime" ) ) ) ) );
	scope().attr("time") = mTime;

	object mAccounts ( ( handle<>( borrowed( Py_InitModule( "_wolfpack.accounts", wpAccounts ) ) ) ) );
	scope().attr("accounts") = mAccounts;

	object mSockets ( ( handle<>( borrowed( Py_InitModule( "_wolfpack.sockets", wpSockets ) ) ) ) );
	scope().attr("sockets") = mSockets;

	init_wolfpack_settings();
	object mSettings ( ( handle<>( borrowed ( PyImport_AddModule( "wpsettings" ) ) ) ) );
	scope().attr("settings") = mSettings;

	object mDatabase ( ( handle<>( borrowed ( Py_InitModule( "_wolfpack.database", wpDatabase ) ) ) ) );
	scope().attr("database") = mDatabase;
}

int PyConvertObject( PyObject* object, cUObject** uoobject )
{
	if ( checkWpChar( object ) )
	{
		*uoobject = getWpChar( object );
	}
	else if ( checkWpItem( object ) )
	{
		*uoobject = getWpItem( object );
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Object expected." );
		return 0;
	}

	return 1;
}
