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

#include "Python.h"

#include "../items.h"
#include "../tilecache.h"
#include "../prototypes.h"
#include "../wolfpack.h"
#include "../wpscriptmanager.h"
#include "../itemid.h"
#include "../spellbook.h"
#include "../books.h"

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
int wpItem_compare( PyObject *a, PyObject *b );

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
	wpItem_compare
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
	Q_UNUSED(args);	
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
	Q_UNUSED(args);	
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
	Coord_cl pos = self->pItem->pos();

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
PyObject* wpItem_weaponskill( wpItem* self, PyObject* args )
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
	cVariant value = self->pItem->tags().get( key );

	if( value.type() == cVariant::String )
	{
		QString strValue = value.asString();

		if( !strValue.isNull() )
			return PyString_FromString( strValue.latin1() );
		else
			return PyString_FromString( "" );
	}
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

	self->pItem->tags().remove( key );

	if( checkArgStr( 1 ) )
		self->pItem->tags().set( key, cVariant( QString( getArgStr( 1 ) ) ) );
	else if( checkArgInt( 1 ) )
		self->pItem->tags().set( key, cVariant( (int)getArgInt( 1 ) ) );

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
	
	return self->pItem->tags().get( key ).isValid() ? PyTrue : PyFalse;
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
	self->pItem->tags().remove( key );

	return PyTrue;
}

PyObject* wpItem_ischar( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	Q_UNUSED(self);	
	return PyFalse;
}

PyObject* wpItem_isitem( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	Q_UNUSED(self);	
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
PyObject* wpItem_getoutmostitem( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	return PyGetItemObject( self->pItem->getOutmostItem() );
}

/*!
	Gets the outmost character this item is contained in.
*/
PyObject* wpItem_getoutmostchar( wpItem* self, PyObject* args )
{
	Q_UNUSED(args);	
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	return PyGetCharObject( self->pItem->getOutmostChar() );
}

PyObject* wpItem_getname( wpItem* self  )
{
	if( !self->pItem )
		return false;

	QString name = self->pItem->getName( true );
	return PyString_FromString( name.latin1() );
}

/*!
	Adds an item to this container.
*/
PyObject* wpItem_additem( wpItem* self, PyObject* args )
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

PyObject* wpItem_getadv( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( !checkArgInt( 0 ) || !checkArgInt( 1 ))
	{
		PyErr_BadArgument();
		return 0;
	}
	ushort list = getArgInt( 0 ); //Which list ? 
								  //REGEN | BONUS | DAMAGE | ENH | HIT | REQ | RESIST | REFLECT

	ushort id = getArgInt( 1 );   //Which property ?

								  //...
 	switch( list )
	{
	case REGEN:
		return PyInt_FromLong( self->pItem->regen( id ) ); break;
	case BONUS:
		return PyInt_FromLong( self->pItem->bonus( id ) ); break;
	case DAMAGE:
		return PyInt_FromLong( self->pItem->damage( id ) ); break;
	case ENH:
		return PyInt_FromLong( self->pItem->enh( id ) ); break;
	case HIT:
		return PyInt_FromLong( self->pItem->hit( id ) ); break;
	case REQ:
		return PyInt_FromLong( self->pItem->req( id ) ); break;
	case RESIST: 
		return PyInt_FromLong( self->pItem->resist( id ) ); break;
	case REFLECT:
		return PyInt_FromLong( self->pItem->reflect( id ) ); break;
	default:
		return PyFalse;
	}
}

PyObject* wpItem_setadv( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ))
	{
		PyErr_BadArgument();
		return 0;
	}
	ushort list = getArgInt( 0 ); //Which list ? 
								  //REGEN | BONUS | DAMAGE | ENH | HIT | REQ | RESIST | REFLECT

	ushort id = getArgInt( 1 );   //Which property ?
								  //...

	ushort data = getArgInt( 2 ); //Value to write

	switch( list )
	{
	case REGEN:
		self->pItem->setRegen( id, data ); break;
	case BONUS:
		self->pItem->setBonus( id, data ); break;
	case DAMAGE:
		self->pItem->setDamage( id, data ); break;
	case ENH:
		self->pItem->setEnh( id, data ); break;
	case HIT:
		self->pItem->setHit( id, data ); break;
	case REQ:
		self->pItem->setReq( id, data ); break;
	case RESIST: 
		self->pItem->setResist( id, data ); break;
	case REFLECT:
		self->pItem->setReflect( id, data ); break;
	default:
		return PyFalse;
	}

	return PyTrue;
}

PyObject* wpItem_spellscount( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
		return PyFalse;

	cSpellBook *pBook = dynamic_cast< cSpellBook* >( self->pItem );
	if( !pBook )
	{
		return PyFalse;
	}
	return PyInt_FromLong( pBook->spellCount() );

}

/*
 * Check if this spellbook has a spell
 */
PyObject* wpItem_hasspell( wpItem* self, PyObject* args )
{
	if( !self->pItem || self->pItem->free )
	{
		return PyFalse;
	}
	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	cSpellBook *pBook = dynamic_cast< cSpellBook* >( self->pItem );
	if( !pBook )
	{
		return PyFalse;
	}

	UINT8 spell_num = getArgInt( 0 );
	return PyInt_FromLong( pBook->hasSpell( spell_num ) );
}

static PyMethodDef wpItemMethods[] = 
{
	{ "additem",			(getattrofunc)wpItem_additem, METH_VARARGS, "Adds an item to this container." },
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
	{ "getadv",				(getattrofunc)wpItem_getadv, METH_VARARGS,"Get advanced modifiers." },
	{ "setadv",				(getattrofunc)wpItem_setadv, METH_VARARGS,"Set advanced modifiers." },
	{ "spellscount",		(getattrofunc)wpItem_spellscount, METH_VARARGS,"Get spells count in spellbook" },
	{ "hasspell", (getattrofunc)wpItem_hasspell, METH_VARARGS,"Check if this spellbook has the spell" },

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
	// Special Python things
	if( !strcmp( "content", name ) )
	{	
		cBook* pBook = dynamic_cast< cBook* >(self->pItem);
		if( pBook )
		{
			QStringList content = pBook->content();
			QStringList::iterator it = content.begin();
			PyObject *list = PyList_New( content.size() );
			UINT32 i = 0;
			while( it != content.end() )
			{
				PyList_SetItem( list, i, PyString_FromString( *it ) );
				++it;
				++i;
			}
			return list;
		}

		cItem::ContainerContent content = self->pItem->content();
		PyObject *list = PyList_New( content.size() );
		for( INT32 i = 0; i < content.size(); ++i )
			PyList_SetItem( list, i, PyGetItemObject( content[i] ) );		
		return list;
	}
	else if( !strcmp( "events", name ) )
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
			case cVariant::Char:
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
					obj = PyString_FromString( "" );
				else
					obj = PyString_FromString( result.toString().latin1() );
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

int wpItem_setAttr( wpItem *self, char *name, PyObject *value )
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

			WPDefaultScript *script = ScriptManager->find( PyString_AsString( PyList_GetItem( value, i ) ) );
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
					if( pCont->atLayer( (cChar::enLayer)tile.layer ) )
						pCont->atLayer( (cChar::enLayer)tile.layer )->toBackpack( pCont );
					pCont->addItem( (cChar::enLayer)tile.layer, self->pItem );
				}
			}
		}
		else
		{
			self->pItem->removeFromCont();
			self->pItem->moveTo( self->pItem->pos() );
		}
	}
	else if( !strcmp( "content", name ) )
	{
		cBook* pBook = dynamic_cast< cBook* >( self->pItem );
		if( pBook && PyList_Check( value ) )
		{
			QStringList content;
			UINT32 i;
			for( i = 0; i < PyList_Size( value ); ++i )
			{
				if( !PyString_Check( PyList_GetItem( value, i ) ) )
				{
					content.push_back("");
					continue;
				}

				content.push_back( PyString_AsString( PyList_GetItem( value, i ) ) );
			}
		}
		else
		{
			PyErr_BadArgument();
			return -1;
		}
	}
	else
	{
		cVariant val;
		if( PyString_Check( value ) )
			val = cVariant( PyString_AsString( value ) );
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
