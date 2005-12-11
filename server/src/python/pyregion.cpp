/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
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

#include "engine.h"
#include "utilities.h"
#include "../territories.h"
#include "../uotime.h"

#include <vector>

/*
	\object region
	\description This object type represents a region within the Ultima Online world.
*/
typedef struct
{
	PyObject_HEAD;
	cTerritory* pRegion;
	bool frozen; // unused yet
	Coord pos; // unused yet
} wpRegion;

// Forward Declarations
static PyObject* wpRegion_getAttr( wpRegion* self, char* name );
static int wpRegion_setAttr( wpRegion* self, char* name, PyObject* value );

static PyObject* wpRegion_str( wpRegion* self )
{
	return QString2Python( self->pRegion->name() );
}

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpRegionType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpregion",
sizeof( wpRegionType ),
0,
wpDealloc,
0,
( getattrfunc ) wpRegion_getAttr,
( setattrfunc ) wpRegion_setAttr,
0,
0,
0,
0,
0,
0,
0, // Call
( reprfunc ) wpRegion_str,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/*
	\method region.startrain
	\description This method make Region begins to rain.
*/
static PyObject* wpRegion_startrain( wpRegion* self, PyObject* args )
{

	self->pRegion->setIsRaining( true );

	Py_RETURN_NONE;
}

/*
	\method region.stoprain
	\description This method make Region stops to rain.
*/
static PyObject* wpRegion_stoprain( wpRegion* self, PyObject* args )
{

	self->pRegion->setIsRaining( false );

	Py_RETURN_NONE;
}

/*
	\method region.startsnow
	\description This method make Region begins to snow.
*/
static PyObject* wpRegion_startsnow( wpRegion* self, PyObject* args )
{

	self->pRegion->setIsSnowing( true );

	Py_RETURN_NONE;
}

/*
	\method region.stopsnow
	\description This method make Region stops to snow.
*/
static PyObject* wpRegion_stopsnow( wpRegion* self, PyObject* args )
{

	self->pRegion->setIsSnowing( false );

	Py_RETURN_NONE;
}

/*
	\method region.setweatherday
	\param day The Day to next Weather Change
	\description It will set the day for next Weather Update.
*/
static PyObject* wpRegion_setweatherday( wpRegion* self, PyObject* args )
{
	if ( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	unsigned char arg = getArgInt( 0 );

	int actualday = UoTime::instance()->days();

	if ( arg < actualday )
		Py_RETURN_FALSE;

	self->pRegion->setWeatherDay( arg );

	Py_RETURN_NONE;
}

/*
	\method region.setweatherhour
	\param hour The Hour to next Weather Change
	\description It will set the hour for next Weather update.
*/
static PyObject* wpRegion_setweatherhour( wpRegion* self, PyObject* args )
{
	if ( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	unsigned char arg = getArgInt( 0 );

	int actualhour = UoTime::instance()->hour();

	self->pRegion->setWeatherHour( arg );

	Py_RETURN_NONE;
}

static PyMethodDef wpRegionMethods[] =
{
{ "startrain", ( getattrofunc ) wpRegion_startrain,	METH_VARARGS, "The Region begins to rain" },
{ "stoprain", ( getattrofunc ) wpRegion_stoprain,	METH_VARARGS, "The Region stops to rain" },
{ "startsnow", ( getattrofunc ) wpRegion_startsnow,	METH_VARARGS, "The Region begins to snow" },
{ "stopsnow", ( getattrofunc ) wpRegion_stopsnow,	METH_VARARGS, "The Region stops to snow" },
{ "setweatherday", ( getattrofunc ) wpRegion_setweatherday,	METH_VARARGS, "Set the day of next Weather update" },
{ "setweatherhour", ( getattrofunc ) wpRegion_setweatherhour,	METH_VARARGS, "Set the hour of next Weather update" },
{ NULL, NULL, 0, NULL }
};

static PyObject* wpRegion_getAttr( wpRegion* self, char* name )
{
	/*
		\rproperty region.parent This property represents the parent region of this region. If there is no parent region this
		property contains None, otherwise another region object for the parent region.
	*/
	if ( !strcmp( name, "parent" ) )
	{
		// Check if valid region
		cTerritory* pRegion = dynamic_cast<cTerritory*>( self->pRegion->parent() );
		return PyGetRegionObject( pRegion );
	}
	/*
		\rproperty region.children This property contains a tuple of region objects for the subregions within this region.
	*/
	else if ( !strcmp( name, "children" ) )
	{
		QList<cBaseRegion*> children = self->pRegion->children();
		PyObject* tuple = PyTuple_New( children.size() );
		for ( uint i = 0; i < children.size(); ++i )
		{
			cTerritory* pRegion = dynamic_cast<cTerritory*>( children[i] );
			PyTuple_SetItem( tuple, i, PyGetRegionObject( pRegion ) );
		}
		return tuple;
	}
	/*
		\rproperty region.rectangles This property is a tuple of tuples containing x1, y1, x2 and y2 for the rectangles that define
		the area of this region.
	*/
	else if ( !strcmp( name, "rectangles" ) )
	{
		QList<cBaseRegion::rect_st> rectangles = self->pRegion->rectangles();
		PyObject* tuple = PyTuple_New( rectangles.size() );
		for ( uint i = 0; i < rectangles.size(); ++i )
		{
			PyObject* subtuple = PyTuple_New( 4 );
			PyTuple_SetItem( subtuple, 0, PyInt_FromLong( rectangles[i].x1 ) );
			PyTuple_SetItem( subtuple, 1, PyInt_FromLong( rectangles[i].y1 ) );
			PyTuple_SetItem( subtuple, 2, PyInt_FromLong( rectangles[i].x2 ) );
			PyTuple_SetItem( subtuple, 3, PyInt_FromLong( rectangles[i].y2 ) );

			PyTuple_SetItem( tuple, i, subtuple );
		}
		return tuple;
	}
	/*
		\rproperty region.name The name of this region.
	*/
	else if ( !strcmp( name, "name" ) )
		return QString2Python( self->pRegion->name() );
	/*
		\rproperty region.resores The Definition of Ores of this Region.
	*/
	else if ( !strcmp( name, "resores" ) )
		return QString2Python( self->pRegion->resores() );
	/*
		\rproperty region.firstcoin The Definition of the First Coin for this Region (From New Monetary).
	*/
	else if ( !strcmp( name, "firstcoin" ) )
		return QString2Python( self->pRegion->firstcoin() );
	/*
		\rproperty region.secondcoin The Definition of the Second Coin for this Region (From New Monetary).
	*/
	else if ( !strcmp( name, "secondcoin" ) )
		return QString2Python( self->pRegion->secondcoin() );
	/*
		\rproperty region.thirdcoin The Definition of the Third Coin for this Region (From New Monetary).
	*/
	else if ( !strcmp( name, "thirdcoin" ) )
		return QString2Python( self->pRegion->thirdcoin() );
	/*
		\rproperty region.midilist A list of midi sounds to be played for this region.
	*/
	else if ( !strcmp( name, "midilist" ) )
		return QString2Python( self->pRegion->midilist() );
	/*
		\rproperty region.guardowner The name of the guardowner for this region.
	*/
	else if ( !strcmp( name, "guardowner" ) )
		return QString2Python( self->pRegion->guardOwner() );
	/*
		\rproperty region.fixedlight The Fixed LightLevel for this Region (Will return -1 to no fixed light level)
	*/
	else if ( !strcmp( name, "fixedlight" ) )
		return PyInt_FromLong( self->pRegion->fixedlight() );

	// Flags
	/*
		\rproperty region.guarded This boolean flag indicates whether the region is guarded or not.
	*/
	else if ( !strcmp( name, "guarded" ) )
		return PyInt_FromLong( self->pRegion->isGuarded() ? 1 : 0 );
	/*
		\rproperty region.nomark This boolean flag indicates whether runes aren't markable in this region or not.
	*/
	else if ( !strcmp( name, "nomark" ) )
		return PyInt_FromLong( self->pRegion->isNoMark() ? 1 : 0 );
	/*
		\rproperty region.nogate This boolean flag indicates whether gates in or out of this region are allowed.
	*/
	else if ( !strcmp( name, "nogate" ) )
		return PyInt_FromLong( self->pRegion->isNoGate() ? 1 : 0 );
	/*
		\rproperty region.norecallout This boolean flag indicates whether recalling out of this region is allowed.
	*/
	else if ( !strcmp( name, "norecallout" ) )
		return PyInt_FromLong( self->pRegion->isNoRecallOut() ? 1 : 0 );
	/*
		\rproperty region.norecallin This boolean flag indicates whether recalling into this region is allowed.
	*/
	else if ( !strcmp( name, "norecallin" ) )
		return PyInt_FromLong( self->pRegion->isNoRecallIn() ? 1 : 0 );
	/*
		\rproperty region.noagressivemagic This boolean flag indicates whether agressive magic is forbidden in this region or not.
	*/
	else if ( !strcmp( name, "noagressivemagic" ) )
		return PyInt_FromLong( self->pRegion->isNoAgressiveMagic() ? 1 : 0 );
	/*
		\rproperty region.antimagic This boolean flag indicates whether magic is forbidden in this region or not.
	*/
	else if ( !strcmp( name, "antimagic" ) )
		return PyInt_FromLong( self->pRegion->isAntiMagic() ? 1 : 0 );
	/*
		\rproperty region.cave This boolean flag indicates that this region is underground.
	*/
	else if ( !strcmp( name, "cave" ) )
		return PyInt_FromLong( self->pRegion->isCave() ? 1 : 0 );
	/*
		\rproperty region.nodecay This boolean flag indicates that items do not decay in this region.
	*/
	else if ( !strcmp( name, "nodecay" ) )
		return PyInt_FromLong( self->pRegion->isNoDecay() ? 1 : 0 );
	/*
		\rproperty region.nomusic This boolean flag indicates that no music should be played in this region.
	*/
	else if ( !strcmp( name, "nomusic" ) )
		return PyInt_FromLong( self->pRegion->isNoMusic() ? 1 : 0 );
	/*
		\rproperty region.noguardmessage This boolean flag indicates that no guard message should show when entering this region..
	*/
	else if ( !strcmp( name, "noguardmessage" ) )
		return PyInt_FromLong( self->pRegion->isNoGuardMessage() ? 1 : 0 );
	/*
		\rproperty region.noentermessage This boolean flag indicates that no entrance message should show when entering this region.
	*/
	else if ( !strcmp( name, "noentermessage" ) )
		return PyInt_FromLong( self->pRegion->isNoEnterMessage() ? 1 : 0 );
	/*
		\rproperty region.instalogout This boolean flag indicates if Region is a InstaLogout Region or not.
	*/
	else if ( !strcmp( name, "instalogout" ) )
		return PyInt_FromLong( self->pRegion->isInstaLogout() ? 1 : 0 );
	/*
		\rproperty region.noteleport This boolean flag indicates that no Teleport Magic is allowed in this place.
	*/
	else if ( !strcmp( name, "noteleport" ) )
		return PyInt_FromLong( self->pRegion->isNoTeleport() ? 1 : 0 );
	/*
		\rproperty region.safe This boolean flag indicates the Region is a Safe Region (no one can be harmed here).
	*/
	else if ( !strcmp( name, "safe" ) )
		return PyInt_FromLong( self->pRegion->isSafe() ? 1 : 0 );
	/*
		\rproperty region.nocriminalcombat This boolean flag indicates that attacks here against innocent targets, cant make attacker criminal.
	*/
	else if ( !strcmp( name, "nocriminalcombat" ) )
		return PyInt_FromLong( self->pRegion->isNoCriminalCombat() ? 1 : 0 );
	/*
		\rproperty region.nokillcount This boolean flag indicates that Kills are not counted on this area.
	*/
	else if ( !strcmp( name, "nokillcount" ) )
		return PyInt_FromLong( self->pRegion->isNoKillCount() ? 1 : 0 );
	/*
		\rproperty region.israining This boolean flag indicates that this Region is Raining or not.
	*/
	else if ( !strcmp( name, "israining" ) )
		return PyInt_FromLong( self->pRegion->isRaining() ? 1 : 0);
	/*
		\rproperty region.issnowing This boolean flag indicates that this Region is Raining or not.
	*/
	else if ( !strcmp( name, "issnowing" ) )
		return PyInt_FromLong( self->pRegion->isSnowing() ? 1 : 0);
	/*
		\rproperty region.rainchance The Rain Chance for that Region
	*/
	else if ( !strcmp( name, "rainchance" ) )
		return PyInt_FromLong( self->pRegion->rainChance() );
	/*
		\rproperty region.snowchance The Snow Chance for that Region
	*/
	else if ( !strcmp( name, "snowchance" ) )
		return PyInt_FromLong( self->pRegion->snowChance() );
	/*
		\rproperty region.weatherday The day for Next Weather Change
	*/
	else if ( !strcmp( name, "weatherday" ) )
		return PyInt_FromLong( self->pRegion->weatherday() );
	/*
		\rproperty region.weatherhour The hour for Next Weather Change
	*/
	else if ( !strcmp( name, "weatherhour" ) )
		return PyInt_FromLong( self->pRegion->weatherhour() );
	/*
		\rproperty region.rainduration The Default duration for Rain in this Region
	*/
	else if ( !strcmp( name, "rainduration" ) )
		return PyInt_FromLong( self->pRegion->rainduration() );
	/*
		\rproperty region.snowduration The Default duration for Snow in this Region
	*/
	else if ( !strcmp( name, "snowduration" ) )
		return PyInt_FromLong( self->pRegion->snowduration() );
	/*
		\rproperty region.dryduration The Default duration for Dry in this Region
	*/
	else if ( !strcmp( name, "dryduration" ) )
		return PyInt_FromLong( self->pRegion->dryduration() );
	/*
		\rproperty region.rainrangeduration The Default range for Rain duration in this Region
	*/
	else if ( !strcmp( name, "rainrangeduration" ) )
		return PyInt_FromLong( self->pRegion->rainrangeduration() );
	/*
		\rproperty region.snowrangeduration The Default range for Snow duration in this Region
	*/
	else if ( !strcmp( name, "snowrangeduration" ) )
		return PyInt_FromLong( self->pRegion->snowrangeduration() );
	/*
		\rproperty region.dryrangeduration The Default range for Dry duration in this Region
	*/
	else if ( !strcmp( name, "dryrangeduration" ) )
		return PyInt_FromLong( self->pRegion->dryrangeduration() );

	return Py_FindMethod( wpRegionMethods, ( PyObject * ) self, name );
}

static int wpRegion_setAttr( wpRegion* self, char* name, PyObject* value )
{
	Q_UNUSED( self );
	Q_UNUSED( name );
	Q_UNUSED( value );
	// Regions have no changeable attributes yet
	return 0;
}

bool checkWpRegion( PyObject* object )
{
	return ( object->ob_type == &wpRegionType );
}

PyObject* PyGetRegionObject( cTerritory* pRegion )
{
	if ( !pRegion )
	{
		Py_RETURN_NONE;
	}

	wpRegion* returnVal = PyObject_New( wpRegion, &wpRegionType );
	returnVal->pRegion = pRegion;
	return ( PyObject * ) returnVal;
}

cTerritory* getWpRegion( PyObject* pObj )
{
	if ( pObj->ob_type != &wpRegionType )
		return NULL;

	return ( ( wpRegion * ) pObj )->pRegion;
}
