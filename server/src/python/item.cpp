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

#include "../items.h"
#include "../tilecache.h"
#include "../scriptmanager.h"
#include "../itemid.h"
#include "../multis.h"
#include "../basechar.h"
#include "../singleton.h"

#include "utilities.h"
#include "content.h"
#include "tempeffect.h"
#include "objectcache.h"

/*!
	The object for Wolfpack Python items
*/
struct wpItem
{
    PyObject_HEAD;
	P_ITEM pItem;
};

// Note: Must be of a different type to cause more then 1 template instanciation
class cItemObjectCache : public cObjectCache< wpItem, 50>
{
};

typedef SingletonHolder< cItemObjectCache > ItemCache;

static void FreeItemObject( PyObject *obj )
{
	ItemCache::instance()->freeObj( obj );
}

// Forward Declarations
static PyObject *wpItem_getAttr( wpItem *self, char *name );
static int wpItem_setAttr( wpItem *self, char *name, PyObject *value );
int wpItem_compare( PyObject *a, PyObject *b );
long wpItem_hash(wpItem *self) {
	return self->pItem->serial();
}

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpItemType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "wpitem",
    sizeof(wpItemType),
    0,
    //FreeItemObject,				
	wpDealloc,
    0,								
    (getattrfunc)wpItem_getAttr,
    (setattrfunc)wpItem_setAttr,
	wpItem_compare,
	0,
	0,
	0,
	0,
	(hashfunc)wpItem_hash
};

PyObject* PyGetItemObject( P_ITEM item )
{
	if( item == NULL )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

//	wpItem *returnVal = ItemCache::instance()->allocObj( &wpItemType );
	wpItem *returnVal = PyObject_New( wpItem, &wpItemType );
	returnVal->pItem = item;
	return (PyObject*)returnVal;
}

// Method declarations

/*!
	Resends the item to all clients in range
*/
static PyObject* wpItem_update( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	self->pItem->update();

	return PyTrue;
}

/*!
	Removes the item
*/
static PyObject* wpItem_delete( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	self->pItem->remove();

	return PyTrue;
}

/*!
	Moves the item to the specified location
*/
static PyObject* wpItem_moveto( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	// Gather parameters
	Coord_cl pos = self->pItem->pos();
	char noRemove = 0;

	if( PyTuple_Size( args ) == 1 )
	{
		if( !PyArg_ParseTuple( args, "O&|b:item.moveto( coord )", &PyConvertCoord, &pos, &noRemove ) )
			return 0;

		self->pItem->moveTo( pos, noRemove ? true : false );
	}
	else
	{
		int x,y;
		int z = self->pItem->pos().z;
		int map = self->pItem->pos().map;

		if( !PyArg_ParseTuple( args, "ii|iib:item.moveto( x, y, [z], [map] )", &x, &y, &z, &map,  &noRemove ) )
			return 0;

		self->pItem->moveTo( Coord_cl( (unsigned short)x, (unsigned short)y, (char)z, (unsigned char)map ), noRemove ? true : false );
	}	

	return PyTrue;
}


/*!
	Removes the item from all clients in range
*/
static PyObject* wpItem_removefromview( wpItem* self, PyObject* args )
{
	int k = 1;
	if ( !PyArg_ParseTuple(args, "|i:item.removefromview( clean )", &k) )
		return 0;
	self->pItem->removeFromView( k != 0 ? true : false );
	Py_INCREF(Py_None);
	return Py_None;
}

/*!
	Plays a soundeffect originating from the item
*/
static PyObject* wpItem_soundeffect( wpItem* self, PyObject* args )
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
static PyObject* wpItem_distanceto( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyInt_FromLong( -1 );

	// Probably an object
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *pObj = PyTuple_GetItem( args, 0 );

		if( checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
			return PyInt_FromLong( self->pItem->pos().distance( getWpCoord( pObj ) ) );

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( pItem->dist( self->pItem ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( pChar->dist( self->pItem ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pItem->pos();

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( self->pItem->pos().distance( pos ) );
	}

	PyErr_BadArgument();
	return NULL;
}

/*!
	Returns the weaponskill needed to use this 
	weapon. It returns -1 if it is unable to
	determine the weapon skill.
*/
static PyObject* wpItem_weaponskill( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
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
static PyObject* wpItem_useresource( wpItem* self, PyObject* args )
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
static PyObject* wpItem_countresource( wpItem* self, PyObject* args )
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

	return PyLong_FromLong( avail );
}

/*!
	Returns the custom tag passed
*/
static PyObject* wpItem_gettag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	cVariant value = self->pItem->getTag( key );

	if( value.type() == cVariant::String )
		return PyUnicode_FromUnicode((Py_UNICODE*)value.toString().ucs2(), value.toString().length());
	else if( value.type() == cVariant::Int )
		return PyInt_FromLong( value.asInt() );
	else if( value.type() == cVariant::Double )
		return PyFloat_FromDouble( value.asDouble() );

	Py_INCREF( Py_None );
	return Py_None;
}

/*!
	Sets a custom tag
*/
static PyObject* wpItem_settag( wpItem* self, PyObject* args )
{
	if (self->pItem->free)
		return PyFalse;

	char *key;
	PyObject *object;

	if (!PyArg_ParseTuple( args, "sO:char.settag( name, value )", &key, &object ))
		return 0;

	if (PyString_Check(object)) {
		self->pItem->setTag(key, cVariant(PyString_AsString(object)));
	} else if (PyUnicode_Check(object)) {
		self->pItem->setTag(key, cVariant(QString::fromUcs2((ushort*)PyUnicode_AsUnicode(object))));
	} else if (PyInt_Check(object)) {
		self->pItem->setTag(key, cVariant((int)PyInt_AsLong(object)));
	} else if (PyFloat_Check(object)) {
		self->pItem->setTag(key, cVariant((double)PyFloat_AsDouble(object)));
	}

	return PyTrue;
}

/*!
	Checks if a certain tag exists
*/
static PyObject* wpItem_hastag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	char* pKey = 0;
	if ( !PyArg_ParseTuple( args, "s:item.hastag( key )", &pKey ) )
		return 0;

	QString key = pKey;
	
	return self->pItem->getTag( key ).isValid() ? PyTrue : PyFalse;
}

/*!
	Deletes a given tag
*/
static PyObject* wpItem_deltag( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = PyString_AsString( PyTuple_GetItem( args, 0 ) );
	self->pItem->removeTag( key );

	return PyTrue;
}

static PyObject* wpItem_ischar( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	Q_UNUSED(self);	
	return PyFalse;
}

static PyObject* wpItem_isitem( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	Q_UNUSED(self);	
	return PyTrue;
}

/*!
	Shows a moving effect moving toward a given object or coordinate.
*/
static PyObject* wpItem_movingeffect( wpItem* self, PyObject* args )
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
static PyObject* wpItem_addtimer( wpItem* self, PyObject* args )
{
	// Three arguments
	if( (PyTuple_Size( args ) < 3 && PyTuple_Size( args ) > 4) || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
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
	
	effect->setDest( self->pItem->serial() );
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );

	return PyTrue;
}

/*!
	Gets the outmost item this item is contained in.
*/
static PyObject* wpItem_getoutmostitem( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	return PyGetItemObject( self->pItem->getOutmostItem() );
}

/*!
	Gets the outmost character this item is contained in.
*/
static PyObject* wpItem_getoutmostchar( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	return PyGetCharObject( self->pItem->getOutmostChar() );
}

/*!
	Returns the item's name
*/
static PyObject* wpItem_getname( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pItem )
		return false;

	QString name = self->pItem->getName( true );
	return PyString_FromString( name.latin1() );
}

/*!
	Adds an item to this container.
*/
static PyObject* wpItem_additem( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( !checkArgItem( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_ITEM pItem = getArgItem( 0 );

	// Secondary Parameters
	bool randomPos = true;
	bool handleWeight = true;
	bool autoStack = true;

	if( checkArgInt( 1 ) )
		randomPos = getArgInt( 1 ) != 0;

	if( checkArgInt( 2 ) )
		handleWeight = getArgInt( 2 ) != 0;

	if( checkArgInt( 3 ) )
		autoStack = getArgInt( 3 ) != 0;

	// Special rules:
	// If randomPos == false but autoStack == true then manually set a random position as well
	// If randomPos == true but autoStack = false then manually set the random position

	if( randomPos && !autoStack )
	{
		self->pItem->addItem( pItem, false, handleWeight );
		pItem->SetRandPosInCont( self->pItem );
	}
	else if( !randomPos && autoStack )
	{
		Coord_cl pos = pItem->pos();
		self->pItem->addItem( pItem, true, handleWeight );
		if( !pItem->free )
			pItem->moveTo( pos );
	}
	else
	{
		self->pItem->addItem( pItem, randomPos, handleWeight );
	}

	return PyTrue;
}

/*!
	Amount of items inside this container
*/
static PyObject* wpItem_countItem( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyInt_FromLong( self->pItem->content().size() );

}

// If we are in a multi, return the multi object for it
// otherwise pynone
static PyObject* wpItem_multi( wpItem* self, PyObject* args )
{
	if( self->pItem->free )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	return PyGetMultiObject( dynamic_cast< cMulti* >( FindItemBySerial( self->pItem->multis() ) ) );  
}

static PyObject* wpItem_lightning( wpItem *self, PyObject *args )
{
	unsigned short hue = 0;
	
	if( !PyArg_ParseTuple( args, "|h:item.lightning( [hue] )", &hue ) )
		return 0;

	self->pItem->lightning( hue );

	return PyTrue;
}

static PyObject* wpItem_resendtooltip( wpItem *self, PyObject *args )
{
	if (!self->pItem->free) {
		self->pItem->resendTooltip();
	}

	return PyTrue;
}

static PyObject* wpItem_dupe( wpItem *self, PyObject *args )
{
	if (!self->pItem->free) {
		P_ITEM item = self->pItem->dupe();
		return item->getPyObject();
	}

	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject* wpItem_isblessed( wpItem *self, PyObject *args )
{
	if (self->pItem->free) {
		return 0;
	}
	return self->pItem->newbie() ? PyTrue : PyFalse;
}

static PyMethodDef wpItemMethods[] = 
{
	{ "additem",			(getattrofunc)wpItem_additem, METH_VARARGS, "Adds an item to this container." },
	{ "countitem",			(getattrofunc)wpItem_countItem, METH_VARARGS, "Counts how many items are inside this container." },
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
	{ "getname",			(getattrofunc)wpItem_getname, METH_VARARGS, "Get item name." },
	{ "multi",				(getattrofunc)wpItem_multi,	METH_VARARGS, 0 },
	{ "lightning",			(getattrofunc)wpItem_lightning, METH_VARARGS, 0 },
	{ "resendtooltip",		(getattrofunc)wpItem_resendtooltip, METH_VARARGS, 0 },
	{ "dupe",				(getattrofunc)wpItem_dupe, METH_VARARGS, 0 },

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
	{ "isblessed",			(getattrofunc)wpItem_isblessed, METH_VARARGS, "Is this item blessed(newbie) "},
    { NULL, NULL, 0, NULL }
};

// Getters + Setters

static PyObject *wpItem_getAttr( wpItem *self, char *name )
{
	// Special Python things
	if( !strcmp( "content", name ) )
	{	
		cItem::ContainerContent content = self->pItem->content();
		PyObject *list = PyList_New( content.size() );
		for( INT32 i = 0; i < content.size(); ++i )
			PyList_SetItem( list, i, PyGetItemObject( content[i] ) );		
		return list;
	} else if (!strcmp("tags", name)) {
		// Return a list with the keynames
		PyObject *list = PyList_New(0);

		QStringList tags = self->pItem->getTags();
		for (QStringList::iterator it = tags.begin(); it != tags.end(); ++it) {
			QString name = *it;
			if (!name.isEmpty()) {
				PyList_Append(list, PyString_FromString(name.latin1()));
			}
		}

		return list;
	} else if( !strcmp( "events", name ) )
	{
		QStringList events = QStringList::split( ",", self->pItem->eventList() );
		PyObject *list = PyList_New( events.count() );
		for( INT32 i = 0; i < events.count(); ++i )
			PyList_SetItem( list, i, PyString_FromString( events[i].latin1() ) );
		return list;
	}
	else
	{
		cVariant result;
		stError *error = self->pItem->getProperty( name, result );

		if( !error )
		{
			PyObject *obj = 0;

			switch( result.type() )
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
				if( result.toString().isNull() )
					obj = PyUnicode_FromWideChar(L"", 0);
				else
					obj = PyUnicode_FromUnicode((Py_UNICODE*)result.toString().ucs2(), result.toString().length() );
				break;
			case cVariant::Double:
				obj = PyFloat_FromDouble( result.toDouble() );
				break;
			case cVariant::Coord:
				obj = PyGetCoordObject( result.toCoord() );
				break;
			}

			if( !obj )
			{
				PyErr_Format( PyExc_ValueError, "Unsupported Property Type: %s", result.typeName() );
				return 0;
			}

			return obj;
		}
		else
			delete error;
	}	
	
	return Py_FindMethod( wpItemMethods, (PyObject*)self, name );
}

static int wpItem_setAttr( wpItem *self, char *name, PyObject *value )
{
	// Special Python things.
	if( !strcmp( "events", name ) )
	{
		if( !PyList_Check( value ) )
		{
			PyErr_BadArgument();
			return -1;
		}

		self->pItem->clearEvents();
		int i;
		for( i = 0; i < PyList_Size( value ); ++i )
		{
			if( !PyString_Check( PyList_GetItem( value, i ) ) )
				continue;

			cPythonScript *script = ScriptManager::instance()->find( PyString_AsString( PyList_GetItem( value, i ) ) );
			if( script )
				self->pItem->addEvent( script );
		}
	}
	else if( !strcmp( "container", name ) )
	{
		if( checkWpItem( value ) )
		{
			P_ITEM pCont = getWpItem( value );
			if( pCont )
				pCont->addItem( self->pItem );
		}
		else if( checkWpChar( value ) )
		{
			P_CHAR pCont = getWpChar( value );
			if( pCont )
			{
				tile_st tile = TileCache::instance()->getTile( self->pItem->id() );
				if( tile.layer )
				{
					if( pCont->atLayer( (cBaseChar::enLayer)tile.layer ) )
						pCont->atLayer( (cBaseChar::enLayer)tile.layer )->toBackpack( pCont );
					pCont->addItem( (cBaseChar::enLayer)tile.layer, self->pItem );
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
		if( PyString_Check( value ) )
			val = cVariant( PyString_AsString( value ) );
		else if( PyUnicode_Check( value ) )
			val = cVariant(QString::fromUcs2((ushort*)PyUnicode_AsUnicode(value)));
		else if( PyInt_Check( value ) )
			val = cVariant( PyInt_AsLong( value ) );
		else if( checkWpItem( value ) )
			val = cVariant( getWpItem( value ) );
		else if( checkWpChar( value ) )
			val = cVariant( getWpChar( value ) );
		else if( checkWpCoord( value ) )
			val = cVariant( getWpCoord( value ) );
		else if( PyFloat_Check( value ) )
			val = cVariant( PyFloat_AsDouble( value ) );

		if( !val.isValid() )
		{
			if( value->ob_type )
				PyErr_Format( PyExc_TypeError, "Unsupported object type: %s", value->ob_type->tp_name );
			else
				PyErr_Format( PyExc_TypeError, "Unknown object type" );
			return 0;
		}

		stError *error = self->pItem->setProperty( name, val );

		if( error )
		{
			PyErr_Format( PyExc_TypeError, "Error while setting attribute '%s': %s", name, error->text.latin1() );
			delete error;
			return 0;
		}
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

int wpItem_compare( PyObject *a, PyObject *b )
{
	// Both have to be characters
	if( a->ob_type != &wpItemType || b->ob_type != &wpItemType ) 
		return -1;

	P_ITEM pA = getWpItem( a );
	P_ITEM pB = getWpItem( b );

	return !( pA == pB );
}
