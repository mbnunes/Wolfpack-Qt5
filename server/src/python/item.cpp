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

#include "../items.h"
#include "../tilecache.h"
#include "../prototypes.h"
#include "../junk.h"

extern cAllItems *Items;

#include "utilities.h"
#include "content.h"
#include "tempeffect.h"

/*!
	The object for Wolfpack Python items
*/
typedef struct {
    PyObject_HEAD;
	P_ITEM pItem;
} wpItem;

// Forward Declarations
PyObject *wpItem_getAttr( wpItem *self, char *name );
int wpItem_setAttr( wpItem *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpItemType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "wpitem",
    sizeof(wpItemType),
    0,
    wpDealloc,				
    0,								
    (getattrfunc)wpItem_getAttr,
    (setattrfunc)wpItem_setAttr,
};

inline PyObject* PyGetItemObject( P_ITEM item )
{
	if( item == NULL )
		return Py_None;

	wpItem *returnVal = PyObject_New( wpItem, &wpItemType );
	returnVal->pItem = item;
	return (PyObject*)returnVal;
}

// Method declarations

/*!
	Resends the item to all clients in range
*/
PyObject* wpItem_update( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	self->pItem->update();

	return PyTrue;
}

/*!
	Removes the item
*/
PyObject* wpItem_delete( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	Items->DeleItem( self->pItem );

	return PyTrue;
}

/*!
	Moves the item to the specified location
*/
PyObject* wpItem_moveto( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( PyTuple_Size( args ) == 1 && checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
	{
		self->pItem->moveTo( getWpCoord( PyTuple_GetItem( args, 0 ) ) );
		return PyTrue;
	}

	// Gather parameters
	Coord_cl pos = self->pItem->pos;

	if( PyTuple_Size( args ) <= 1 )
	{
		PyErr_BadArgument();
		return NULL;
	}

	// X,Y
	if( PyTuple_Size( args ) >= 2 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyFalse;

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	}

	// Z
	if( PyTuple_Size( args ) >= 3 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 2 ) ) )
			return PyFalse;

		pos.z = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );
	}

	// MAP
	if( PyTuple_Size( args ) >= 4 )
	{
		if( !PyInt_Check( PyTuple_GetItem( args, 3 ) ) )
			return PyFalse;

		pos.map = PyInt_AsLong( PyTuple_GetItem( args, 3 ) );
	}

	self->pItem->moveTo( pos );

	return PyTrue;
}


/*!
	Removes the item from all clients in range
*/
PyObject* wpItem_removefromview( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( !checkArgInt( 0 ) || getArgInt( 0 ) == 0 )
		self->pItem->removeFromView( false );
	else
		self->pItem->removeFromView( true );

	return PyTrue;
}

/*!
	Plays a soundeffect originating from the item
*/
PyObject* wpItem_soundeffect( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	self->pItem->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) );

	return PyTrue;
}

/*!
	Returns the distance towards a given object or position
*/
PyObject* wpItem_distanceto( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyInt_FromLong( -1 );

	// Probably an object
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *pObj = PyTuple_GetItem( args, 0 );

		if( checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
			return PyInt_FromLong( self->pItem->pos.distance( getWpCoord( pObj ) ) );

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( pItem->pos.distance( self->pItem->pos ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( pChar->pos.distance( self->pItem->pos ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pItem->pos;

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( self->pItem->pos.distance( pos ) );
	}

		PyErr_BadArgument();
		return NULL;
}

/*!
	Returns the weaponskill needed to use this 
	weapon. It returns -1 if it is unable to
	determine the weapon skill.
*/
PyObject* wpItem_weaponskill( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyInt_FromLong( -1 );

	UINT16 id = self->pItem->id();

	if( IsSwordType( id ) )
		return PyInt_FromLong( SWORDSMANSHIP );
	else if( IsMaceType( id ) )
		return PyInt_FromLong( MACEFIGHTING );
	else if( IsFencingType( id ) )
		return PyInt_FromLong( FENCING );
	else if( IsBowType( id ) )
		return PyInt_FromLong( ARCHERY );

	return PyInt_FromLong( -1 );
}

/*!
	Takes at least two arguments (amount,item-id)
	Optionally the color of the item we 
	want to consume too.
	It consumes the items and amount specified
	and returns how much have been really consumed.
*/
PyObject* wpItem_useresource( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;
	
	if( PyTuple_Size( args ) < 2 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

    UINT16 amount = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	UINT16 id = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	UINT16 color = 0;

	if( PyTuple_Size( args ) > 2 && PyInt_Check( PyTuple_GetItem( args, 2 ) ) )
		color = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );

	UINT16 deleted = 0;
	deleted = self->pItem->DeleteAmount( amount, id, color );

	return PyInt_FromLong( deleted );
}

/*!
	Takes at least one argument (item-id)
	Optionally the color
	It returns the amount of a resource
	available
*/
PyObject* wpItem_countresource( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;
	
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT16 id = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	INT16 color = -1;

	if( PyTuple_Size( args ) > 1 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
		color = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );

	UINT16 avail = 0;
	avail = self->pItem->CountItems( id, color );

	return PyInt_FromLong( avail );
}

/*!
	Returns the custom tag passed
*/
PyObject* wpItem_gettag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return Py_None;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	cVariant value = self->pItem->tags.get( key );

	if( value.type() == cVariant::String )
		return PyString_FromString( value.asString().latin1() );
	else if( value.type() == cVariant::Int )
		return PyInt_FromLong( value.asInt() );

	return Py_None;
}

/*!
	Sets a custom tag
*/
PyObject* wpItem_settag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) || ( !checkArgStr( 1 ) && !checkArgInt( 1 )  ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );

	self->pItem->tags.remove( key );

	if( checkArgStr( 1 ) )
		self->pItem->tags.set( key, cVariant( QString( getArgStr( 1 ) ) ) );
	else if( checkArgInt( 1 ) )
		self->pItem->tags.set( key, cVariant( (int)getArgInt( 1 ) ) );

	return PyTrue;
}

/*!
	Checks if a certain tag exists
*/
PyObject* wpItem_hastag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	
	return self->pItem->tags.get( key ).isValid() ? PyTrue : PyFalse;
}

/*!
	Deletes a given tag
*/
PyObject* wpItem_deltag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	self->pItem->tags.remove( key );

	return PyTrue;
}

PyObject* wpItem_ischar( wpItem* self, PyObject* args )
{
	return PyFalse;
}

PyObject* wpItem_isitem( wpItem* self, PyObject* args )
{
	return PyTrue;
}

/*!
	Shows a moving effect moving toward a given object or coordinate.
*/
PyObject* wpItem_movingeffect( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( ( !checkArgObject( 1 ) && !checkArgCoord( 1 ) ) || !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
	
	UINT16 id = getArgInt( 0 );

	cUObject *object = getArgChar( 1 );
	if( !object )
		object = getArgItem( 1 );

	Coord_cl pos;
	
	if( checkArgCoord( 1 ) )
		pos = getArgCoord( 1 );

	// Optional Arguments
	bool fixedDirection = true;
	bool explodes = false;
	UINT8 speed = 10;
	UINT16 hue = 0;
	UINT16 renderMode = 0;

	if( checkArgInt( 2 ) )
		fixedDirection = getArgInt( 2 ) != 0;

	if( checkArgInt( 3 ) )
		explodes = getArgInt( 3 ) != 0;

	if( checkArgInt( 4 ) )
		speed = getArgInt( 4 );

	if( checkArgInt( 5 ) )
		hue = getArgInt( 5 );

	if( checkArgInt( 6 ) )
		renderMode = getArgInt( 6 );

	if( object )
		self->pItem->effect( id, object, fixedDirection, explodes, speed, hue, renderMode );
	else 
		self->pItem->effect( id, pos, fixedDirection, explodes, speed, hue, renderMode );

	return PyTrue;
}

/*!
	Adds a temp effect to this item.
*/
PyObject* wpItem_addtimer( wpItem* self, PyObject* args )
{
	// Three arguments
	if( PyTuple_Size( args ) != 3 || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
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
	
	effect->setDest( self->pItem->serial );
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );

	return PyTrue;
}

/*!
	Gets the outmost item this item is contained in.
*/
PyObject* wpItem_getoutmostitem( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	return PyGetItemObject( self->pItem->getOutmostItem() );
}

/*!
	Gets the outmost character this item is contained in.
*/
PyObject* wpItem_getoutmostchar( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	return PyGetCharObject( self->pItem->getOutmostChar() );
}

static PyMethodDef wpItemMethods[] = 
{
    { "update",				(getattrofunc)wpItem_update, METH_VARARGS, "Sends the item to all clients in range." },
	{ "removefromview",		(getattrofunc)wpItem_removefromview, METH_VARARGS, "Removes the item from the view of all in-range clients." },
	{ "delete",				(getattrofunc)wpItem_delete, METH_VARARGS, "Deletes the item and the underlying reference." },
	{ "moveto",				(getattrofunc)wpItem_moveto, METH_VARARGS, "Moves the item to the specified location." },
	{ "soundeffect",		(getattrofunc)wpItem_soundeffect, METH_VARARGS, "Sends a soundeffect to the surrounding sockets." },
	{ "distanceto",			(getattrofunc)wpItem_distanceto, METH_VARARGS, "Distance to another object or a given position." },
	{ "weaponskill",		(getattrofunc)wpItem_weaponskill, METH_VARARGS, "Returns the skill used with this weapon. -1 if it isn't a weapon." },
	{ "useresource",		(getattrofunc)wpItem_useresource, METH_VARARGS, "Consumes a given resource from within the current item." },
	{ "countresource",		(getattrofunc)wpItem_countresource, METH_VARARGS, "Returns the amount of a given resource available in this container." },
	{ "addtimer",			(getattrofunc)wpItem_addtimer, METH_VARARGS, "Attaches a timer to this object." },
	{ "getoutmostchar",		(getattrofunc)wpItem_getoutmostchar, METH_VARARGS, "Get the outmost character." },
	{ "getoutmostitem",		(getattrofunc)wpItem_getoutmostitem, METH_VARARGS, "Get the outmost item." },
	
	// Effects
	{ "movingeffect",		(getattrofunc)wpItem_movingeffect, METH_VARARGS, "Shows a moving effect moving toward a given object or coordinate." },

	// Tag System
	{ "gettag",				(getattrofunc)wpItem_gettag, METH_VARARGS, "Gets a tag assigned to a specific item." },
	{ "settag",				(getattrofunc)wpItem_settag, METH_VARARGS, "Sets a tag assigned to a specific item." },
	{ "hastag",				(getattrofunc)wpItem_hastag, METH_VARARGS, "Checks if a certain item has the specified tag." },
	{ "deltag",				(getattrofunc)wpItem_deltag, METH_VARARGS, "Deletes the specified tag." },

	// Is*? Functions
	{ "isitem",				(getattrofunc)wpItem_isitem, METH_VARARGS, "Is this an item." },
	{ "ischar",				(getattrofunc)wpItem_ischar, METH_VARARGS, "Is this a char." },
    { NULL, NULL, 0, NULL }
};

// Getters + Setters

PyObject *wpItem_getAttr( wpItem *self, char *name )
{
	getStrProperty( "name", pItem->getName().latin1() )
	else getIntProperty( "id", pItem->id() )
	else getStrProperty( "name2", pItem->name2().ascii() )
	else getIntProperty( "color", pItem->color() )
	else getIntProperty( "amount", pItem->amount() )
	else getIntProperty( "amount2", pItem->amount2() )
	else getIntProperty( "serial", pItem->serial )
	else getIntProperty( "layer", pItem->layer() )
	else getIntProperty( "twohanded", pItem->twohanded() ? 1 : 0 )
	else getIntProperty( "type", pItem->type() )
	else getIntProperty( "type2", pItem->type2() )

	else if( !strcmp( "pos", name ) )
		return PyGetCoordObject( self->pItem->pos );

	// What we contain
	else if( !strcmp( "content", name ) )
	{		
		cItem::ContainerContent content = self->pItem->content();
		PyObject *list = PyList_New( content.size() );
		for( INT32 i = 0; i < content.size(); ++i )
			PyList_SetItem( list, i, PyGetItemObject( content[i] ) );		
		return list;
	}

	// What we're contained in
	else if( !strcmp( "container", name ) )
	{
		if( isItemSerial( self->pItem->contserial ) )
			return PyGetItemObject( FindItemBySerial( self->pItem->contserial ) );
		else if( isCharSerial( self->pItem->contserial ) )
			return PyGetCharObject( FindCharBySerial( self->pItem->contserial ) );
		else
			return Py_None;
	}

	// What we contain
	else if( !strcmp( "content", name ) )
	{
		//Py_WPContent *returnVal = PyObject_New( Py_WPContent, &Py_WPContentType );
		//returnVal->pItem = self->Item; // Never forget that
		//return (PyObject*)returnVal;
		return Py_None;
	}

	else getIntProperty( "weight", pItem->weight() )
	else getIntProperty( "totalweight", pItem->totalweight() )
	else getIntProperty( "more1", pItem->more1() )
	else getIntProperty( "more2", pItem->more2() )
	else getIntProperty( "more3", pItem->more3() )
	else getIntProperty( "more4", pItem->more4() )
	else getIntProperty( "moreb1", pItem->moreb1() )
	else getIntProperty( "moreb2", pItem->moreb2() )
	else getIntProperty( "moreb3", pItem->moreb3() )
	else getIntProperty( "moreb4", pItem->moreb4() )
	else getIntProperty( "morex", pItem->morex() )
	else getIntProperty( "morey", pItem->morey() )
	else getIntProperty( "morez", pItem->morez() )
	else getIntProperty( "doordir", pItem->doordir() )
	else getIntProperty( "dooropen", pItem->dooropen() )
	else getIntProperty( "dye", pItem->dye() )
	else getIntProperty( "corpse", pItem->corpse() )
	else getIntProperty( "defense", pItem->def() )
	else getIntProperty( "lodamage", pItem->lodamage() )
	else getIntProperty( "hidamage", pItem->hidamage() )
	else getIntProperty( "hp", pItem->hp() )
	else getIntProperty( "maxhp", pItem->maxhp() )
	else getIntProperty( "str", pItem->st() )
	else getIntProperty( "dex", pItem->dx() )
	else getIntProperty( "int", pItem->in() )
	else getIntProperty( "str2", pItem->st2() )
	else getIntProperty( "dex2", pItem->dx2() )
	else getIntProperty( "int2", pItem->in2() )
	else getIntProperty( "speed", pItem->speed() )
	else getIntProperty( "secured", pItem->secured() ? 1 : 0 )
	else getIntProperty( "moveable", pItem->magic() )
	else getIntProperty( "gatetime", pItem->gatetime() )
	else getIntProperty( "gatenumber", pItem->gatenumber() )
	else getIntProperty( "decaytime", pItem->decaytime() )
	
	else if( !strcmp( name, "decay" ) )
		return PyInt_FromLong( (self->pItem->priv&0x01) ? 1 : 0 );

	// ownserial
	else getIntProperty( "visible", pItem->visible )
	// spanserial
	else getIntProperty( "dir", pItem->dir ) // lightsource type
	else getIntProperty( "priv", pItem->priv ) 
	else getIntProperty( "disabled", pItem->disabled() )
	else getStrProperty( "disabledmsg", pItem->disabledmsg().latin1() ) 
	else getIntProperty( "poisoned", pItem->poisoned() ) 
	else getStrProperty( "murderer", pItem->murderer().latin1() ) 
	else getIntProperty( "murdertime", pItem->murdertime() ) 
	else getIntProperty( "rank", pItem->rank() ) 
	else getStrProperty( "creator", pItem->creator.latin1() ) 
	else getIntProperty( "good", pItem->good ) 
	else getIntProperty( "madewith", pItem->madewith ) 
	else getStrProperty( "desc", pItem->desc.latin1() ) 
	else getStrProperty( "spawnregion", pItem->spawnregion().latin1() )
	
	else if( !strcmp( "events", name ) )
	{
		QStringList events = QStringList::split( ",", self->pItem->eventList() );
		PyObject *list = PyList_New( events.count() );
		for( INT32 i = 0; i < events.count(); ++i )
			PyList_SetItem( list, i, PyString_FromString( events[i].latin1() ) );
		return list;
	}

	// If no property is found search for a method
	return Py_FindMethod( wpItemMethods, (PyObject*)self, name );
}

int wpItem_setAttr( wpItem *self, char *name, PyObject *value )
{
	if( !strcmp( name, "id" ) )
		self->pItem->setId( PyInt_AS_LONG( value ) );

	//else setStrProperty( "name", pItem->name )
	else if( !strcmp( "name", name ) )
		self->pItem->setName( PyString_AS_STRING( value ) ); 

	else if( !strcmp( "name2", name ) )
		self->pItem->setName2( PyString_AS_STRING( value ) );
	
	else if( !strcmp( "color", name ) )
		self->pItem->setColor( PyInt_AS_LONG( value ) );

	else if( !strcmp( "amount", name ) )
		self->pItem->setAmount( PyInt_AS_LONG( value ) );

	else if( !strcmp( "amount2", name ) )
		self->pItem->setAmount2( PyInt_AS_LONG( value ) );

	else setIntProperty( "serial", pItem->serial )

	else if( !strcmp( name, "twohanded" ) )
		self->pItem->setTwohanded( ( PyInt_AS_LONG( value ) != 0 ) ? true : false );

	else if( !strcmp( name, "type" ) )
		self->pItem->setType( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "hp" ) )
		self->pItem->setHp( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "maxhp" ) )
		self->pItem->setMaxhp( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "type2" ) )
		self->pItem->setType2( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "weight" ) )
		self->pItem->setWeight( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "moreb1" ) )
		self->pItem->setMoreb1( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "moreb2" ) )
		self->pItem->setMoreb1( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "moreb3" ) )
		self->pItem->setMoreb1( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "moreb4" ) )
		self->pItem->setMoreb1( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "decay" ) )
		if( PyObject_IsTrue( value ) )
		{
			self->pItem->priv |= 0x01;
			self->pItem->startDecay();
		}
		else
		{
			self->pItem->priv &= 0xFE;
			self->pItem->setDecayTime(0);
		}

	// CONTAINER!!
//	else setIntProperty( "more1", pItem->more1() )
	else if( !strcmp( name, "more1" ) )
		self->pItem->setMore1( PyInt_AS_LONG( value ) );
//	else setIntProperty( "more2", pItem->more2() )
	else if( !strcmp( name, "more2" ) )
		self->pItem->setMore2( PyInt_AS_LONG( value ) );
//	else setIntProperty( "more3", pItem->more3() )
	else if( !strcmp( name, "more3" ) )
		self->pItem->setMore3( PyInt_AS_LONG( value ) );
//	else setIntProperty( "more4", pItem->more4() )
	else if( !strcmp( name, "more4" ) )
		self->pItem->setMore4( PyInt_AS_LONG( value ) );
//	else setIntProperty( "morex", pItem->morex() )
	else if( !strcmp( name, "morex" ) )
		self->pItem->setMoreX( PyInt_AS_LONG( value ) );
//	else setIntProperty( "morey", pItem->morey() )
	else if( !strcmp( name, "morey" ) )
		self->pItem->setMoreY( PyInt_AS_LONG( value ) );
//	else setIntProperty( "morez", pItem->morez() )
	else if( !strcmp( name, "morez" ) )
		self->pItem->setMoreZ( PyInt_AS_LONG( value ) );
//	else setIntProperty( "doordir", pItem->doordir )
	else if( !strcmp( name, "doordir" ) )
		self->pItem->setDoorDir( PyInt_AS_LONG( value ) );
//	else setIntProperty( "dooropen", pItem->dooropen )
	else if( !strcmp( name, "dooropen" ) )
		self->pItem->setDoorOpen( PyInt_AS_LONG( value ) );
	// PILEABLE
//	else setIntProperty( "dye", pItem->dye )
	else if( !strcmp( name, "dye" ) )
		self->pItem->setDye( PyInt_AS_LONG( value ) );
//	else setIntProperty( "defense", pItem->def )
	else if( !strcmp( name, "defense" ) )
		self->pItem->setDef( PyInt_AS_LONG( value ) );
//	else setIntProperty( "str", pItem->st )
	else if( !strcmp( name, "str" ) )
		self->pItem->setSt( PyInt_AS_LONG( value ) );
//	else setIntProperty( "dex", pItem->dx )
	else if( !strcmp( name, "dex" ) )
		self->pItem->setDx( PyInt_AS_LONG( value ) );
//	else setIntProperty( "int", pItem->in )
	else if( !strcmp( name, "int" ) )
		self->pItem->setIn( PyInt_AS_LONG( value ) );
//	else setIntProperty( "str2", pItem->st2 )
	else if( !strcmp( name, "str2" ) )
		self->pItem->setSt2( PyInt_AS_LONG( value ) );
//	else setIntProperty( "dex2", pItem->dx2 )
	else if( !strcmp( name, "dex2" ) )
		self->pItem->setDx2( PyInt_AS_LONG( value ) );
//	else setIntProperty( "int2", pItem->in2 )
	else if( !strcmp( name, "int2" ) )
		self->pItem->setIn2( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "speed" ) )
		self->pItem->setSpeed( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "corpse" ) )
		self->pItem->setCorpse( ( PyInt_AS_LONG( value ) == 0 ) ? false : true );

	else if( !strcmp( name, "lodamage" ) )
		self->pItem->setLodamage( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "hidamage" ) )
		self->pItem->setHidamage( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "secured" ) )
		self->pItem->setSecured( ( PyInt_AS_LONG( value ) == 1 ) ? true : false );

//	else setIntProperty( "moveable", pItem->magic )
	else if( !strcmp( name, "moveable" ) )
		self->pItem->setMagic( PyInt_AS_LONG( value ) );
//	else if( !strcmp( name, "moveable ) )
//	else setIntProperty( "gatetime", pItem->gatetime )
	else if( !strcmp( name, "gatetime" ) )
		self->pItem->setGateTime( PyInt_AS_LONG( value ) );
//	else setIntProperty( "gatenumber", pItem->gatenumber )
	else if( !strcmp( name, "gatenumber" ) )
		self->pItem->setGateNumber( PyInt_AS_LONG( value ) );
//	else setIntProperty( "decaytime", pItem->decaytime )
	else if( !strcmp( name, "decaytime" ) )
		self->pItem->setDecayTime( PyInt_AS_LONG( value ) );
	// ownserial
	else setIntProperty( "visible", pItem->visible )
	// spanserial
	else setIntProperty( "dir", pItem->dir ) // lightsource type
	else setIntProperty( "priv", pItem->priv ) 
//	else setIntProperty( "disabled", pItem->disabled() )
	else if( !strcmp( name, "disabled" ) )
		self->pItem->setDisabled( PyInt_AS_LONG( value ) );
	else setStrProperty( "disabledmsg", pItem->disabledmsg() ) 
//	else setIntProperty( "poisoned", pItem->poisoned() ) 
	else if( !strcmp( name, "poisoned" ) )
		self->pItem->setPoisoned( PyInt_AS_LONG( value ) );

	else if( !strcmp( name, "murderer" ) )
		self->pItem->setMurderer( PyString_AS_STRING( value ) );

//	else setIntProperty( "murdertime", pItem->murdertime() ) 
	else if( !strcmp( name, "murdertime" ) )
		self->pItem->setMurderTime( PyInt_AS_LONG( value ) );
//	else setIntProperty( "rank", pItem->rank() ) 
	else if( !strcmp( name, "rank" ) )
		self->pItem->setRank( PyInt_AS_LONG( value ) );
	else setStrProperty( "creator", pItem->creator ) 
	else setIntProperty( "good", pItem->good ) 
	else setIntProperty( "madewith", pItem->madewith ) 
	else setStrProperty( "desc", pItem->desc ) 

	else if( !strcmp( name, "spawnregion" ) )
		self->pItem->setSpawnRegion( PyString_AS_STRING( value ) );
	
	// Moving the item into a container
	else if( !strcmp( name, "container" ) )
	{
		P_CHAR pChar = getWpChar( value );
		P_ITEM pItem = getWpItem( value );
		
		if( pItem )
			pItem->addItem( self->pItem );
		else if( pChar )
		{
			// Get a valid layer
			tile_st tInfo = TileCache::instance()->getTile( self->pItem->id() );
			if( tInfo.layer != 0 )
				pChar->addItem( (cChar::enLayer)tInfo.layer, self->pItem );
		}
		// Are we intentionally moving us into an invalid container?
		else if( !PyObject_IsTrue( value ) )
			self->pItem->removeFromCont();
	}

	return 0;
}

P_ITEM getWpItem( PyObject *pObj )
{
	if( pObj->ob_type != &wpItemType )
		return 0;

	wpItem *item = (wpItem*)( pObj );
	return item->pItem;
}

bool checkWpItem( PyObject *pObj )
{
	if( pObj->ob_type != &wpItemType )
		return false;
	else
		return true;
}
