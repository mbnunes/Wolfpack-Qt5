/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#include "engine.h"

#include "../items.h"
#include "../muls/tilecache.h"
#include "../scriptmanager.h"
#include "../multi.h"
#include "../basechar.h"
#include "../singleton.h"

#include "utilities.h"
#include "content.h"
#include "tempeffect.h"
#include "objectcache.h"

/*
	\object item
	\inherit object
	\description This object represents an item in the wolfpack world.
*/
struct wpItem
{
	PyObject_HEAD;
	P_ITEM pItem;
};

// Note: Must be of a different type to cause more then 1 template instanciation
class cItemObjectCache : public cObjectCache<wpItem, 50>
{
};

typedef SingletonHolder<cItemObjectCache> ItemCache;

// Forward Declarations
static PyObject* wpItem_getAttr( wpItem* self, char* name );
static int wpItem_setAttr( wpItem* self, char* name, PyObject* value );
int wpItem_compare( PyObject* a, PyObject* b );
long wpItem_hash( wpItem* self )
{
	return self->pItem->serial();
}

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpItemType =
{
	PyObject_HEAD_INIT( &PyType_Type )
	0,
	"wpitem",
	sizeof( wpItemType ),
	0,
	wpDealloc,
	0,
	( getattrfunc ) wpItem_getAttr,
	( setattrfunc ) wpItem_setAttr,
	wpItem_compare,
	0,
	0,
	0,
	0,
	( hashfunc ) wpItem_hash
};

PyObject* PyGetItemObject( P_ITEM item )
{
	if ( item == NULL )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	//	wpItem *returnVal = ItemCache::instance()->allocObj( &wpItemType );
	wpItem* returnVal = PyObject_New( wpItem, &wpItemType );
	returnVal->pItem = item;
	return ( PyObject * ) returnVal;
}

// Method declarations

/*!
	Resends the item to all clients in range
*/
/*
	\method item.update
	\description Resend the item.
*/
static PyObject* wpItem_update( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	self->pItem->update();

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Removes the item
*/
/*
	\method item.delete
	\description Deletes the item.
*/
static PyObject* wpItem_delete( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	self->pItem->remove();

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Moves the item to the specified location
*/
/*
	\method item.moveto
	\description Change the position of this item.
	\param pos The coord object representing the new position.
*/
/*
	\method item.moveto
	\description Change the position of this item.
	\param x The new x coordinate of this item.
	\param y The new y coordinate of this item.
	\param z Defaults to the current z position of the item.
	The new z coordinate of this item.
	\param map Defaults to the current map the item is on.
	The new map coordinate of this item.
*/
static PyObject* wpItem_moveto( wpItem* self, PyObject* args )
{
	// Gather parameters
	Coord_cl pos = self->pItem->pos();
	char noRemove = 0;

	if ( PyTuple_Size( args ) == 1 )
	{
		if ( !PyArg_ParseTuple( args, "O&|b:item.moveto(coord, [noremove=0])", &PyConvertCoord, &pos, &noRemove ) )
		{
			return 0;
		}

		self->pItem->moveTo( pos, noRemove ? true : false );
	}
	else
	{
		if ( !PyArg_ParseTuple( args, "HH|bBB:item.moveto(x, y, [z], [map])", &pos.x, &pos.y, &pos.z, &pos.map, &noRemove ) )
		{
			return 0;
		}
		self->pItem->moveTo( pos, noRemove ? true : false );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.removefromview
	\description Remove the item from all clients who can currently see it.
*/
static PyObject* wpItem_removefromview( wpItem* self, PyObject* args )
{
	int k = 1;
	if ( !PyArg_ParseTuple( args, "|i:item.removefromview( clean )", &k ) )
		return 0;
	self->pItem->removeFromView( k != 0 ? true : false );
	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Plays a soundeffect originating from the item
*/
/*
	\method item.soundeffect
	\description Play a soundeffect originating from the item.
	\param sound The id of the soundeffect.
*/
static PyObject* wpItem_soundeffect( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	if ( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	self->pItem->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) );

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Returns the distance towards a given object or position
*/
/*
	\method item.distanceto
	\description Measure the distance between the item and another object.
	\param object The target object. May be another character, item or a coord
	object.
	\return The distance in tiles towards the given target.
*/
/*
	\method item.distanceto
	\description Measure the distance between the item and a coordinate.
	\param x The x component of the target coordinate.
	\param y The y component of the target coordinate.
	\return The distance in tiles towards the given coordinate.
*/
static PyObject* wpItem_distanceto( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyInt_FromLong( -1 );

	// Probably an object
	if ( PyTuple_Size( args ) == 1 )
	{
		PyObject* pObj = PyTuple_GetItem( args, 0 );

		if ( checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
			return PyInt_FromLong( self->pItem->pos().distance( getWpCoord( pObj ) ) );

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if ( pItem )
			return PyInt_FromLong( pItem->dist( self->pItem ) );

		P_CHAR pChar = getWpChar( pObj );
		if ( pChar )
			return PyInt_FromLong( pChar->dist( self->pItem ) );
	}
	else if ( PyTuple_Size( args ) >= 2 ) // Min 2
	{
		Coord_cl pos = self->pItem->pos();

		if ( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );

		return PyInt_FromLong( self->pItem->pos().distance( pos ) );
	}

	PyErr_BadArgument();
	return NULL;
}

/*
	\method item.useresource
	\description Consumes a given amount of a resource.
	\param amount The amount to consume.
	\param itemid The item id of the object to consume.
	\param color Defaults to 0.
	The color of the object to consume
*/
static PyObject* wpItem_useresource( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	if ( PyTuple_Size( args ) < 2 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT16 amount = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	UINT16 id = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	UINT16 color = 0;

	if ( PyTuple_Size( args ) > 2 && PyInt_Check( PyTuple_GetItem( args, 2 ) ) )
		color = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );

	UINT16 deleted = 0;
	deleted = self->pItem->DeleteAmount( amount, id, color );

	return PyInt_FromLong( deleted );
}

/*
	\method item.countresource
	\description Returns the amount of a given resource.
	\param itemid The item id of the resource to count.
	\param color Defaults to 0.
	The color of the items to count.
	\return The amount of item-id
*/
static PyObject* wpItem_countresource( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	if ( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT16 id = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	INT16 color = -1;

	if ( PyTuple_Size( args ) > 1 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
		color = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );

	UINT16 avail = 0;
	avail = self->pItem->CountItems( id, color );

	return PyLong_FromLong( avail );
}

/*
	\method item.gettag
	\description Returns the value of a custom tag. Three types of tag types: String, Int and Float.
	\param name The name of the tag.
	\return Returns the value of the given tag name.
*/
static PyObject* wpItem_gettag( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	if ( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	cVariant value = self->pItem->getTag( key );

	if ( value.type() == cVariant::String )
		return PyUnicode_FromUnicode( ( Py_UNICODE * ) value.toString().ucs2(), value.toString().length() );
	else if ( value.type() == cVariant::Int )
		return PyInt_FromLong( value.asInt() );
	else if ( value.type() == cVariant::Double )
		return PyFloat_FromDouble( value.asDouble() );

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Sets a custom tag
*/
/*
	\method item.settag
	\description Set a value for the given tag name. Three value types: String, Int and Float.
	\param name The name of the tag
	\param value The value of the tag.
*/
static PyObject* wpItem_settag( wpItem* self, PyObject* args )
{
	if ( self->pItem->free )
		return PyFalse();

	char* key;
	PyObject* object;

	if ( !PyArg_ParseTuple( args, "sO:item.settag( name, value )", &key, &object ) )
		return 0;

	if ( PyString_Check( object ) )
	{
		self->pItem->setTag( key, cVariant( PyString_AsString( object ) ) );
	}
	else if ( PyUnicode_Check( object ) )
	{
		self->pItem->setTag( key, cVariant( QString::fromUcs2( ( ushort * ) PyUnicode_AsUnicode( object ) ) ) );
	}
	else if ( PyInt_Check( object ) )
	{
		self->pItem->setTag( key, cVariant( ( int ) PyInt_AsLong( object ) ) );
	}
	else if ( PyFloat_Check( object ) )
	{
		self->pItem->setTag( key, cVariant( ( double ) PyFloat_AsDouble( object ) ) );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Checks if a certain tag exists
*/
/*
	\method item.hastag
	\description Returns the given tag name exists.
	\param name
	\return Returns true or false if the tag exists.
*/
static PyObject* wpItem_hastag( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	char* pKey = 0;
	if ( !PyArg_ParseTuple( args, "s:item.hastag( key )", &pKey ) )
		return 0;

	QString key = pKey;

	return self->pItem->getTag( key ).isValid() ? PyTrue() : PyFalse();
}

/*!
	Deletes a given tag
*/
/*
	\method item.deltag
	\description Deletes the tag under a given name.
	\param name The name of the tag that should be deleted.
*/
static PyObject* wpItem_deltag( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	if ( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	self->pItem->removeTag( key );

	Py_INCREF( Py_None );
	return Py_None;
}
/*
	\method item.ischar
	\description Returns whether the item is a character.
	\return Returns true or false.
*/
static PyObject* wpItem_ischar( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	Q_UNUSED( self );
	return PyFalse();
}
/*
	\method item.isitem
	\description Returns whether the item is an item.
	\return Returns true or false.
*/
static PyObject* wpItem_isitem( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	Q_UNUSED( self );
	return PyTrue();
}

/*!
	Shows a moving effect moving toward a given object or coordinate.
*/
/*
	\method item.movingeffect
	\description Shows a moving effect moving toward a given object or coordinate.
	\param id The id of the moving object.
	\param target Can be an item, character or pos.
	\param fixedDirection Set a fixed direction of the moving id.
	\param explodes True or false for exploding at the end, no damage.
	\param speed Speed at which the id moves. Default is 10.
	\param hue Hue of the moving id
	\param renderMode Unknown
*/
static PyObject* wpItem_movingeffect( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	if ( ( !checkArgObject( 1 ) && !checkArgCoord( 1 ) ) || !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT16 id = getArgInt( 0 );

	cUObject* object = getArgChar( 1 );
	if ( !object )
		object = getArgItem( 1 );

	Coord_cl pos;

	if ( checkArgCoord( 1 ) )
		pos = getArgCoord( 1 );

	// Optional Arguments
	bool fixedDirection = true;
	bool explodes = false;
	UINT8 speed = 10;
	UINT16 hue = 0;
	UINT16 renderMode = 0;

	if ( checkArgInt( 2 ) )
		fixedDirection = getArgInt( 2 ) != 0;

	if ( checkArgInt( 3 ) )
		explodes = getArgInt( 3 ) != 0;

	if ( checkArgInt( 4 ) )
		speed = getArgInt( 4 );

	if ( checkArgInt( 5 ) )
		hue = getArgInt( 5 );

	if ( checkArgInt( 6 ) )
		renderMode = getArgInt( 6 );

	if ( object )
		self->pItem->effect( id, object, fixedDirection, explodes, speed, hue, renderMode );
	else
		self->pItem->effect( id, pos, fixedDirection, explodes, speed, hue, renderMode );

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Adds a temp effect to this item.
*/
/*
	\method item.addtimer
	\description Set a delayed timer for a script function to execute.
	\param expiretime The INT value of the time.
	\param function The function to be executed. String.
	\param serialize Saves the timer. Useful if you crash.
	\return Returns true or false if the tag exists.
*/
static PyObject* wpItem_addtimer( wpItem* self, PyObject* args )
{
	// Three arguments
	if ( ( PyTuple_Size( args ) < 3 && PyTuple_Size( args ) > 4 ) || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	UINT32 expiretime = getArgInt( 0 );
	QString function = getArgStr( 1 );
	PyObject* py_args = PyList_AsTuple( PyTuple_GetItem( args, 2 ) );

	cPythonEffect* effect = new cPythonEffect( function, py_args );

	// Should we save this effect?
	if ( checkArgInt( 3 ) && getArgInt( 3 ) != 0 )
		effect->setSerializable( true );
	else
		effect->setSerializable( false );

	effect->setDest( self->pItem->serial() );
	effect->setExpiretime_ms( expiretime );
	Timers::instance()->insert( effect );

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Gets the outmost item this item is contained in.
*/
static PyObject* wpItem_getoutmostitem( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	return PyGetItemObject( self->pItem->getOutmostItem() );
}

/*!
	Gets the outmost character this item is contained in.
*/
static PyObject* wpItem_getoutmostchar( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	return PyGetCharObject( self->pItem->getOutmostChar() );
}

/*!
	Returns the item's name
*/
/*
	\method item.getname
	\description Returns the name of the object.
	\return Returns the object's name.
*/
static PyObject* wpItem_getname( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem )
		return false;

	QString name = self->pItem->getName( true );
	return PyString_FromString( name.latin1() );
}

/*!
	Adds an item to this container.
*/
/*
	\method item.additem
	\description Adds an item to the container.
	\param item Item to add.
	\param randomPos Gives the item a random position in the pack
	\param autostack Autostacks the item
*/
static PyObject* wpItem_additem( wpItem* self, PyObject* args )
{
	if ( !self->pItem || self->pItem->free )
		return PyFalse();

	if ( !checkArgItem( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_ITEM pItem = getArgItem( 0 );

	// Secondary Parameters
	bool randomPos = true;
	bool handleWeight = true;
	bool autoStack = true;

	if ( checkArgInt( 1 ) )
		randomPos = getArgInt( 1 ) != 0;

	if ( checkArgInt( 2 ) )
		handleWeight = getArgInt( 2 ) != 0;

	if ( checkArgInt( 3 ) )
		autoStack = getArgInt( 3 ) != 0;

	// Special rules:
	// If randomPos == false but autoStack == true then manually set a random position as well
	// If randomPos == true but autoStack = false then manually set the random position

	if ( randomPos && !autoStack )
	{
		self->pItem->addItem( pItem, false, handleWeight );
		pItem->SetRandPosInCont( self->pItem );
	}
	else if ( !randomPos && autoStack )
	{
		Coord_cl pos = pItem->pos();
		self->pItem->addItem( pItem, true, handleWeight );
		if ( !pItem->free )
			pItem->moveTo( pos );
	}
	else
	{
		self->pItem->addItem( pItem, randomPos, handleWeight );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Amount of items inside this container
*/
/*
	\method item.countitem
	\return Returns the amount of items in a container.
*/
static PyObject* wpItem_countItem( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem || self->pItem->free )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyInt_FromLong( self->pItem->content().size() );
}

// If we are in a multi, return the multi object for it
// otherwise pynone
/*
	\method item.multi
	\description Checks if the item is a multi object,
	\return Returns true or false if a multi.
*/
static PyObject* wpItem_multi( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);
	/*	
	if( self->pItem->free )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}
	return PyGetMultiObject( dynamic_cast< cMulti* >( FindItemBySerial( self->pItem->multis() ) ) 
	*/
	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.lightning
	\description Zaps the object with a lightning bolt!
	\param hue The hue value of the lightning.
*/
static PyObject* wpItem_lightning( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	unsigned short hue = 0;

	if ( !PyArg_ParseTuple( args, "|h:item.lightning( [hue] )", &hue ) )
		return 0;

	self->pItem->lightning( hue );

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.resendtooltip
	\description Resends the tooltip of the object.
*/
static PyObject* wpItem_resendtooltip( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem->free )
		self->pItem->resendTooltip();

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.dupe
	\description Creatues a dupe of the item
*/
static PyObject* wpItem_dupe( wpItem* self, PyObject* args )
{
	Q_UNUSED( args );
	if ( !self->pItem->free )
	{
		P_ITEM item = self->pItem->dupe();
		return item->getPyObject();
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.isblessed
	\description Determines if an item is blessed or not.
	\return True, False
*/
static PyObject* wpItem_isblessed( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);
	if ( self->pItem->free )
	{
		return 0;
	}
	return self->pItem->newbie() ? PyTrue() : PyFalse();
}

/*
	\method item.canstack
	\description Determines if the item is stackable.
	\return True, False
*/
static PyObject* wpItem_canstack( wpItem* self, PyObject* args )
{
	P_ITEM other;
	if ( !PyArg_ParseTuple( args, "O&:item.canstack(other)", &PyConvertItem, &other ) )
	{
		return 0;
	}

	return self->pItem->canStack( other ) ? PyTrue() : PyFalse();
}

/*
	\method item.countitems
	\description Counts items recursively in a container by matching their baseids
	against a given list of strings.
	\param baseids A list of strings the baseids of all items will be matched against.
	\return Amount of matching items.
*/
static PyObject* wpItem_countitems( wpItem* self, PyObject* args )
{
	PyObject* list;
	if ( !PyArg_ParseTuple( args, "O!:item.countitems(baseids)", &PyList_Type, &list ) )
	{
		return 0;
	}

	QStringList baseids;

	for ( int i = 0; i < PyList_Size( list ); ++i )
	{
		PyObject* item = PyList_GetItem( list, i );
		if ( PyString_Check( item ) )
		{
			baseids.append( PyString_AsString( item ) );
		}
		else if ( PyUnicode_Check( item ) )
		{
			baseids.append( QString::fromUcs2( ( ushort * ) PyUnicode_AsUnicode( item ) ) );
		}
	}

	return PyInt_FromLong( self->pItem->countItems( baseids ) );
}

/*
	\method item.removeitems
	\description Removes items recursively from a container by matching their baseids
	against a given list of strings.
	\param baseids A list of strings the baseids of the found items will be matched against.
	\param amount Amount of items to remove.
	\return The amount of items that still would need to be removed. If the requested amount could be
	statisfied, the return value is 0.
*/
static PyObject* wpItem_removeitems( wpItem* self, PyObject* args )
{
	PyObject* list;
	unsigned int amount;
	if ( !PyArg_ParseTuple( args, "O!I:item.removeitems(baseids, amount)", &PyList_Type, &list, &amount ) )
	{
		return 0;
	}

	QStringList baseids;

	for ( int i = 0; i < PyList_Size( list ); ++i )
	{
		PyObject* item = PyList_GetItem( list, i );
		if ( PyString_Check( item ) )
		{
			baseids.append( PyString_AsString( item ) );
		}
		else if ( PyUnicode_Check( item ) )
		{
			baseids.append( QString::fromUcs2( ( ushort * ) PyUnicode_AsUnicode( item ) ) );
		}
	}

	return PyInt_FromLong( self->pItem->removeItems( baseids, amount ) );
}

/*
	\method item.removeevent
	\description Remove a python script from the event chain for this object.
	\param event The id of the python script you want to remove from the event chain.
*/
static PyObject* wpItem_removeevent( wpItem* self, PyObject* args )
{
	char* event;
	if ( !PyArg_ParseTuple( args, "s:item.removeevent(name)", &event ) )
	{
		return 0;
	}
	self->pItem->removeEvent( event );
	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.addevent
	\description Add a pythonscript to the event chain of this object.
	Does nothing if the object already has that event.
	\param event The id of the python script you want to add to the event chain.
*/
static PyObject* wpItem_addevent( wpItem* self, PyObject* args )
{
	char* event;
	if ( !PyArg_ParseTuple( args, "s:item.addevent(name)", &event ) )
	{
		return 0;
	}

	cPythonScript* script = ScriptManager::instance()->find( event );

	if ( !script )
	{
		PyErr_Format( PyExc_RuntimeError, "No such script: %s", event );
		return 0;
	}

	self->pItem->addEvent( script );
	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.hasevent
	\description Check if this object has a python script in its event chain.
	\param event The id of the python script you are looking for.
	\return True of the script is in the chain. False otherwise.
*/
static PyObject* wpItem_hasevent( wpItem* self, PyObject* args )
{
	char* event;
	if ( !PyArg_ParseTuple( args, "s:item.hasevent(name)", &event ) )
	{
		return 0;
	}

	if ( self->pItem->hasEvent( event ) )
	{
		Py_INCREF( Py_True );
		return Py_True;
	}
	else
	{
		Py_INCREF( Py_False );
		return Py_False;
	}
}

/*
	\method item.say
	\description Let the item say a text visible for everyone in range.
	\param text The text the item should say.
	\param color Defaults to 0x3b2.
	The color for the text.
*/
/*
	\method item.say
	\description Let the item say a localized text message.
	\param clilocid The id of the localizd message the item should say.
	\param params Defaults to an empty string.
	The parameters that should be parsed into the localized message.
	\param affix Defaults to an empty string.
	Text that should be appended or prepended (see the prepend parameter) to the
	localized message.
	\param prepend Defaults to false.
	If this boolean parameter is set to true, the affix is prepended rather than
	appended.
	\param color Defaults to 0x3b2.
	The color of the message.
	\param socket Defaults to None.
	If a socket object is given here, the message will only be seen by the given socket.
*/
static PyObject* wpItem_say( wpItem* self, PyObject* args, PyObject* keywds )
{
	if ( !checkArgStr( 0 ) )
	{
		uint id;
		char* clilocargs = 0;
		char* affix = 0;
		char prepend;
		uint color = 0x3b2;
		cUOSocket* socket = 0;

		static char* kwlist[] =
		{
			"clilocid",
			"args",
			"affix",
			"prepend",
			"color",
			"socket",
			NULL
		};

		if ( !PyArg_ParseTupleAndKeywords( args, keywds, "i|ssbiO&:char.say( clilocid, [args], [affix], [prepend], [color], [socket] )", kwlist, &id, &clilocargs, &affix, &prepend, &color, &PyConvertSocket, &socket ) )
			return 0;

		self->pItem->talk( id, clilocargs, affix, prepend, color, socket );
	}
	else
	{
		ushort color = 0x3b2;

		if ( checkArgInt( 1 ) )
			color = getArgInt( 1 );

		self->pItem->talk( getArgStr( 0 ), color );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*
	\method item.callevent
	\description Call a python event chain for this object. Ignore global hooks.
	\param event The id of the event you want to call. See <library id="wolfpack.consts">wolfpack.consts</library> for constants.
	\param args A tuple of arguments you want to pass to this event handler.
	\return The result of the first handling event.
*/
static PyObject* wpItem_callevent( wpItem* self, PyObject* args )
{
	unsigned int event;
	PyObject* eventargs;

	if ( !PyArg_ParseTuple( args, "IO!:item.callevent(event, args)", &event, &PyTuple_Type, &eventargs ) )
	{
		return 0;
	}

	if ( cPythonScript::canChainHandleEvent( ( ePythonEvent ) event, self->pItem->getEvents() ) )
	{
		bool result = cPythonScript::callChainedEventHandler( ( ePythonEvent ) event, self->pItem->getEvents(), eventargs );

		if ( result )
		{
			Py_INCREF( Py_True );
			return Py_True;
		}
	}

	Py_INCREF( Py_False );
	return Py_False;
}

/*
	\method item.effect
	\description Show an effect that moves along with the item.
	\param id The effect item id.
	\param speed Defaults to 5.
	This is the animation speed of the effect.
	\param duration Defaults to 10.
	This is how long the effect should be visible.
	\param hue Defaults to 0.
	This is the color for the effect.
	\param rendermode Defaults to 0.
	This is a special rendermode for the effect.
	Valid values are unknown.
*/
static PyObject* wpItem_effect( wpItem* self, PyObject* args )
{
	UINT16 id;
	// Optional Arguments
	UINT8 speed = 5;
	UINT8 duration = 10;
	UINT16 hue = 0;
	UINT16 renderMode = 0;

	if ( !PyArg_ParseTuple( args, "H|BBHH:char.effect(id, [speed], [duration], [hue], [rendermode])", &id, &speed, &duration, &hue, &renderMode ) )
	{
		return 0;
	}

	self->pItem->pos().effect( id, speed, duration, hue, renderMode );

	Py_INCREF( Py_None );
	return Py_None;
}

static PyMethodDef wpItemMethods[] =
{
	{ "additem",			( getattrofunc ) wpItem_additem, METH_VARARGS, "Adds an item to this container." },
	{ "countitem",			( getattrofunc ) wpItem_countItem, METH_VARARGS, "Counts how many items are inside this container." },
	{ "countitems",			( getattrofunc ) wpItem_countitems, METH_VARARGS, "Counts the items inside of this container based on a list of baseids." },
	{ "removeitems",		( getattrofunc ) wpItem_removeitems, METH_VARARGS, "Removes items inside of this container based on a list of baseids." },
	{ "update",				( getattrofunc ) wpItem_update, METH_VARARGS, "Sends the item to all clients in range." },
	{ "removefromview",		( getattrofunc ) wpItem_removefromview, METH_VARARGS, "Removes the item from the view of all in-range clients." },
	{ "delete",				( getattrofunc ) wpItem_delete, METH_VARARGS, "Deletes the item and the underlying reference." },
	{ "moveto",				( getattrofunc ) wpItem_moveto, METH_VARARGS, "Moves the item to the specified location." },
	{ "soundeffect",		( getattrofunc ) wpItem_soundeffect, METH_VARARGS, "Sends a soundeffect to the surrounding sockets." },
	{ "distanceto",			( getattrofunc ) wpItem_distanceto, METH_VARARGS, "Distance to another object or a given position." },
	{ "canstack",			( getattrofunc ) wpItem_canstack, METH_VARARGS, "Sees if the item can be stacked on another item." },
	{ "useresource",		( getattrofunc ) wpItem_useresource, METH_VARARGS, "Consumes a given resource from within the current item." },
	{ "countresource",		( getattrofunc ) wpItem_countresource, METH_VARARGS, "Returns the amount of a given resource available in this container." },
	{ "addtimer",			( getattrofunc ) wpItem_addtimer, METH_VARARGS, "Attaches a timer to this object." },
	{ "getoutmostchar",		( getattrofunc ) wpItem_getoutmostchar, METH_VARARGS, "Get the outmost character." },
	{ "getoutmostitem",		( getattrofunc ) wpItem_getoutmostitem, METH_VARARGS, "Get the outmost item." },
	{ "getname",			( getattrofunc ) wpItem_getname, METH_VARARGS, "Get item name." },
	{ "multi",				( getattrofunc ) wpItem_multi,	METH_VARARGS, 0 },
	{ "lightning",			( getattrofunc ) wpItem_lightning, METH_VARARGS, 0 },
	{ "resendtooltip",		( getattrofunc ) wpItem_resendtooltip, METH_VARARGS, 0 },
	{ "dupe",				( getattrofunc ) wpItem_dupe, METH_VARARGS, 0 },
	{ "say",				( getattrofunc ) wpItem_say, METH_VARARGS | METH_KEYWORDS, 0 },
	{ "effect",				( getattrofunc ) wpItem_effect, METH_VARARGS, 0 },

	// Event handling
	{ "callevent",			( getattrofunc ) wpItem_callevent, METH_VARARGS, 0 },
	{ "addevent",			( getattrofunc ) wpItem_addevent,			METH_VARARGS, 0},
	{ "removeevent",		( getattrofunc ) wpItem_removeevent,		METH_VARARGS, 0},
	{ "hasevent",			( getattrofunc ) wpItem_hasevent,			METH_VARARGS, 0},

	// Effects
	{ "movingeffect",		( getattrofunc ) wpItem_movingeffect, METH_VARARGS, "Shows a moving effect moving toward a given object or coordinate." },

	// Tag System
	{ "gettag",				( getattrofunc ) wpItem_gettag, METH_VARARGS, "Gets a tag assigned to a specific item." },
	{ "settag",				( getattrofunc ) wpItem_settag, METH_VARARGS, "Sets a tag assigned to a specific item." },
	{ "hastag",				( getattrofunc ) wpItem_hastag, METH_VARARGS, "Checks if a certain item has the specified tag." },
	{ "deltag",				( getattrofunc ) wpItem_deltag, METH_VARARGS, "Deletes the specified tag." },

	// Is*? Functions
	{ "isitem",				( getattrofunc ) wpItem_isitem, METH_VARARGS, "Is this an item." },
	{ "ischar",				( getattrofunc ) wpItem_ischar, METH_VARARGS, "Is this a char." },
	{ "isblessed",			( getattrofunc ) wpItem_isblessed, METH_VARARGS, "Is this item blessed(newbie) "},
	{ NULL, NULL, 0, NULL }
};

// Getters + Setters

static PyObject* wpItem_getAttr( wpItem* self, char* name )
{
	// Special Python things
	if ( !strcmp( "content", name ) )
	{
		cItem::ContainerContent content = self->pItem->content();
		PyObject* list = PyList_New( content.size() );
		for ( uint i = 0; i < content.size(); ++i )
			PyList_SetItem( list, i, PyGetItemObject( content[i] ) );
		return list;
	}
	else if ( !strcmp( "tags", name ) )
	{
		// Return a list with the keynames
		PyObject* list = PyList_New( 0 );

		QStringList tags = self->pItem->getTags();
		for ( QStringList::iterator it = tags.begin(); it != tags.end(); ++it )
		{
			QString name = *it;
			if ( !name.isEmpty() )
			{
				PyList_Append( list, PyString_FromString( name.latin1() ) );
			}
		}

		return list;
		/*
				\rproperty item.objects If the item is a multi object, this is a list of objects that are within
				the multi. If it's not a multi, this property is None.
			*/
	}
	else if ( !strcmp( "objects", name ) )
	{
		cMulti* multi = dynamic_cast<cMulti*>( self->pItem );

		if ( !multi )
		{
			Py_INCREF( Py_None );
			return Py_None;
		}

		const QPtrList<cUObject>& objects = multi->getObjects();
		PyObject* tuple = PyTuple_New( objects.count() );
		QPtrList<cUObject>::const_iterator it( objects.begin() );
		unsigned int i = 0;
		for ( ; it != objects.end(); ++it )
		{
			PyTuple_SetItem( tuple, i++, ( *it )->getPyObject() );
		}
		return tuple;
	}
	else if ( !strcmp( "events", name ) )
	{
		QStringList events = QStringList::split( ",", self->pItem->eventList() );
		PyObject* list = PyList_New( events.count() );
		for ( uint i = 0; i < events.count(); ++i )
			PyList_SetItem( list, i, PyString_FromString( events[i].latin1() ) );
		return list;
	}
	else
	{
		cVariant result;
		stError* error = self->pItem->getProperty( name, result );

		if ( !error )
		{
			PyObject* obj = 0;

			switch ( result.type() )
			{
			case cVariant::BaseChar:
				obj = PyGetCharObject( result.toChar() );
				break;
			case cVariant::Item:
				obj = PyGetItemObject( result.toItem() );
				break;
			case cVariant::Long:
			case cVariant::Int:
				obj = PyInt_FromLong( result.toInt() );
				break;
			case cVariant::String:
				if ( result.toString().isNull() )
					obj = PyUnicode_FromWideChar( L"", 0 );
				else
					obj = PyUnicode_FromUnicode( ( Py_UNICODE * ) result.toString().ucs2(), result.toString().length() );
				break;
			case cVariant::Double:
				obj = PyFloat_FromDouble( result.toDouble() );
				break;
			case cVariant::Coord:
				obj = PyGetCoordObject( result.toCoord() );
				break;
			}

			if ( !obj )
			{
				PyErr_Format( PyExc_ValueError, "Unsupported Property Type: %s", result.typeName() );
				return 0;
			}

			return obj;
		}
		else
			delete error;
	}

	return Py_FindMethod( wpItemMethods, ( PyObject * ) self, name );
}

static int wpItem_setAttr( wpItem* self, char* name, PyObject* value )
{
	// Special Python things.
	if ( !strcmp( "events", name ) )
	{
		if ( !PyList_Check( value ) )
		{
			PyErr_BadArgument();
			return -1;
		}

		self->pItem->clearEvents();
		int i;
		for ( i = 0; i < PyList_Size( value ); ++i )
		{
			if ( !PyString_Check( PyList_GetItem( value, i ) ) )
				continue;

			cPythonScript* script = ScriptManager::instance()->find( PyString_AsString( PyList_GetItem( value, i ) ) );
			if ( script )
				self->pItem->addEvent( script );
		}
	}
	else if ( !strcmp( "container", name ) )
	{
		if ( checkWpItem( value ) )
		{
			P_ITEM pCont = getWpItem( value );
			if ( pCont )
				pCont->addItem( self->pItem );
		}
		else if ( checkWpChar( value ) )
		{
			P_CHAR pCont = getWpChar( value );
			if ( pCont )
			{
				tile_st tile = TileCache::instance()->getTile( self->pItem->id() );
				if ( tile.layer )
				{
					if ( pCont->atLayer( ( cBaseChar::enLayer ) tile.layer ) )
						pCont->atLayer( ( cBaseChar::enLayer ) tile.layer )->toBackpack( pCont );
					pCont->addItem( ( cBaseChar::enLayer ) tile.layer, self->pItem );
				}
			}
		}
		else
		{
			self->pItem->removeFromCont();
			self->pItem->moveTo( self->pItem->pos() );
		}
	}
	else
	{
		cVariant val;
		if ( PyString_Check( value ) )
			val = cVariant( PyString_AsString( value ) );
		else if ( PyUnicode_Check( value ) )
			val = cVariant( QString::fromUcs2( ( ushort * ) PyUnicode_AsUnicode( value ) ) );
		else if ( PyInt_Check( value ) )
			val = cVariant( PyInt_AsLong( value ) );
		else if ( checkWpItem( value ) )
			val = cVariant( getWpItem( value ) );
		else if ( checkWpChar( value ) )
			val = cVariant( getWpChar( value ) );
		else if ( checkWpCoord( value ) )
			val = cVariant( getWpCoord( value ) );
		else if ( PyFloat_Check( value ) )
		{
			val = cVariant( PyFloat_AsDouble( value ) );
		}

		//if( !val.isValid() )
		//{
		//	if( value->ob_type )
		//		PyErr_Format( PyExc_TypeError, "Unsupported object type: %s", value->ob_type->tp_name );
		//	else
		//		PyErr_Format( PyExc_TypeError, "Unknown object type" );
		//	return 0;
		//}

		stError * error = self->pItem->setProperty( name, val );

		if ( error )
		{
			PyErr_Format( PyExc_TypeError, "Error while setting attribute '%s': %s", name, error->text.latin1() );
			delete error;
			return 0;
		}
	}

	return 0;
}

P_ITEM getWpItem( PyObject* pObj )
{
	if ( pObj->ob_type != &wpItemType )
		return 0;

	wpItem* item = ( wpItem* ) ( pObj );
	return item->pItem;
}

bool checkWpItem( PyObject* pObj )
{
	if ( pObj->ob_type != &wpItemType )
		return false;
	else
		return true;
}

int wpItem_compare( PyObject* a, PyObject* b )
{
	// Both have to be characters
	if ( a->ob_type != &wpItemType || b->ob_type != &wpItemType )
		return -1;

	P_ITEM pA = getWpItem( a );
	P_ITEM pB = getWpItem( b );

	return !( pA == pB );
}

int PyConvertItem( PyObject* object, P_ITEM* item )
{
	if ( object->ob_type != &wpItemType )
	{
		PyErr_BadArgument();
		return 0;
	}

	*item = ( ( wpItem * ) object )->pItem;
	return 1;
}
