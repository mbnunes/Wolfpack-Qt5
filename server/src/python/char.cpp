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

#if defined (__unix__)
#include <limits.h>  //compatability issue. GCC 2.96 doesn't have limits include
#else
#include <limits> // Python tries to redefine some of this stuff, so include first
#endif

#include "utilities.h"
#include "skills.h"
#include "content.h"
#include "tempeffect.h"

#include "../chars.h"
#include "../territories.h"
#include "../prototypes.h"
#include "../junk.h"
#include "../classes.h"
#include "../skills.h"
#include "../combat.h"

/*!
	Struct for WP Python Chars
*/
typedef struct {
    PyObject_HEAD;
	P_CHAR pChar;
} wpChar;

// Forward Declarations
#define pGetInt( a, b ) if( !strcmp( name, a ) ) return PyInt_FromLong( self->pChar->b );
#define pGetStr( a, b ) if( !strcmp( name, a ) ) return PyString_FromString( self->pChar->b );

PyObject *wpChar_getAttr( wpChar *self, char *name );
int wpChar_setAttr( wpChar *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python chars
*/
static PyTypeObject wpCharType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "wpchar",
    sizeof(wpCharType),
    0,
    wpDealloc,
    0,
    (getattrfunc)wpChar_getAttr,
    (setattrfunc)wpChar_setAttr,
};

PyObject* PyGetCharObject( P_CHAR pChar )
{
	if( !pChar )
		return Py_None;

	wpChar *returnVal = PyObject_New( wpChar, &wpCharType );
	returnVal->pChar = pChar;
	return (PyObject*)returnVal;
}

// Methods

/*!
	Resends the character.
*/
PyObject* wpChar_update( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->resend( false );

	return PyTrue;
}

/*!
	Removes the character from view.
*/
PyObject* wpChar_removefromview( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) || getArgInt( 0 ) == 0 )
		self->pChar->removeFromView( false );
	else
		self->pChar->removeFromView( true );

	return PyTrue;
}

/*!
	Displays a message to the character if connected.
*/
PyObject* wpChar_message( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( ( PyTuple_Size( args ) < 1 ) || !PyString_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for char.message is 1" );
		return PyFalse;
	}

	QString message = PyString_AsString( PyTuple_GetItem( args, 0 ) );

	if( PyTuple_Size( args ) == 2 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
		self->pChar->message( message, PyInt_AsLong( PyTuple_GetItem( args, 1 ) ) );
	else
		self->pChar->message( message );

	return PyTrue;
}

/*!
	Moves the char to the specified location
*/
PyObject* wpChar_moveto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) == 1 && checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
	{
		self->pChar->moveTo( getWpCoord( PyTuple_GetItem( args, 0 ) ) );
		return PyTrue;
	}

	// Gather parameters
	Coord_cl pos = self->pChar->pos;

	if( PyTuple_Size( args ) <= 1 )
	{
		clConsole.send( "Minimum argument count for item.moveto is 2" );
		return PyFalse;
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

	self->pChar->moveTo( pos );

	return PyTrue;
}

/*!
	Plays a soundeffect originating from the char
*/
PyObject* wpChar_soundeffect( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for char.soundeffect is 1\n" );
		return PyFalse;
	}

	if( PyTuple_Size( args ) > 1 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) && !PyInt_AsLong( PyTuple_GetItem( args, 1 ) ) )
		self->pChar->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ), false );
	else
		self->pChar->soundEffect( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) );

	return PyTrue;
}

/*!
	Returns the distance towards a given object or position
*/
PyObject* wpChar_distanceto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyInt_FromLong( -1 );

	// Probably an object
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *pObj = PyTuple_GetItem( args, 0 );

		if( checkWpCoord( PyTuple_GetItem( args, 0 ) ) )
			return PyInt_FromLong( self->pChar->pos.distance( getWpCoord( pObj ) ) );

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( pItem->pos.distance( self->pChar->pos ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( pChar->pos.distance( self->pChar->pos ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pChar->pos;

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( self->pChar->pos.distance( pos ) );
	}

	clConsole.send( "Minimum argment count for char.distanceto is 1\n" );
	return PyInt_FromLong( -1 );
}

/*!
	Lets the character perform an action
*/
PyObject* wpChar_action( wpChar* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for char.action is 1" );
		return PyFalse;
	}

	self->pChar->action( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) );
	return PyTrue;
}

/*!
	Returns the direction of a character 
	toward some object or position
*/
PyObject* wpChar_directionto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyInt_FromLong( -1 );

	// Probably an object
	if( PyTuple_Size( args ) == 1 )
	{
		PyObject *pObj = PyTuple_GetItem( args, 0 );

		if( checkWpCoord( pObj ) )
		{
			Coord_cl pos = getWpCoord( pObj );
			return PyInt_FromLong( chardirxyz( self->pChar, pos.x, pos.y ) );
		}

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( chardirxyz( self->pChar, pItem->pos.x, pItem->pos.y ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( chardir( pChar, self->pChar ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pChar->pos;

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( chardirxyz( self->pChar, pos.x, pos.y ) );
	}

	clConsole.send( "Minimum argment count for char.directionto is 1\n" );
	return PyInt_FromLong( -1 );
}

/*!
	Performs a skillcheck using the given skill
	and minimum and maximum arguments.
*/
PyObject* wpChar_checkskill( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	// 3 Args: skill-id, min, max
	if( PyTuple_Size( args ) < 3 || 
		!PyInt_Check( PyTuple_GetItem( args, 0 ) ) ||
		!PyInt_Check( PyTuple_GetItem( args, 1 ) ) ||
		!PyInt_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		clConsole.send( "Minimum argument count for char.checkskill is 3\n" );
		return PyFalse;
	}

	UINT16 skillId = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	UINT16 min = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	UINT16 max = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );

	bool success = self->pChar->checkSkill( skillId, min, max );

	return success ? PyTrue : PyFalse;
}

/*!
	Returns an item object for an item equipped
	on the specified layer. If there is no item
	it returns py_none.
*/
PyObject* wpChar_itemonlayer( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Miniumum argument count for char.itemonlayer is 1\n" );
		return Py_None;
	}

	return PyGetItemObject( self->pChar->GetItemOnLayer( PyInt_AsLong( PyTuple_GetItem( args, 0 ) ) ) );
}

/*!
	Returns the combat skill currently used by the character
*/
PyObject* wpChar_combatskill( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_ITEM pi = self->pChar->rightHandItem();
	if( !pi )
		pi = self->pChar->leftHandItem();

	return PyInt_FromLong( Combat::weaponSkill( pi ) );
}

/*!
	Takes at least two arguments (amount,item-id)
	Optionally the color of the item we 
	want to consume too.
	It consumes the items and amount specified
	and returns how much have been really consumed.
*/
PyObject* wpChar_useresource( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyInt_FromLong( 0 );
	
	if( PyTuple_Size( args ) < 2 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
	{
		clConsole.send( "Minimum argument count for char.useresource is 2\n" );
		return PyInt_FromLong( 0 );
	}

    UINT16 amount = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
	UINT16 id = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
	UINT16 color = 0;

	if( PyTuple_Size( args ) > 2 && PyInt_Check( PyTuple_GetItem( args, 2 ) ) )
		color = PyInt_AsLong( PyTuple_GetItem( args, 2 ) );

	P_ITEM pPack = self->pChar->getBackpack();
	UINT16 deleted = 0;

	if( pPack )
		deleted = amount - pPack->DeleteAmount( amount, id, color );

	return PyInt_FromLong( deleted );
}

/*!
	Resurrects the current character
*/
PyObject* wpChar_resurrect( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->resurrect();

	return PyTrue;
}

/*!
	Resurrects the current character
*/
PyObject* wpChar_kill( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->kill();

	return PyTrue;
}

/*!
	Deals damage to the character
	This cannot be used for healing!
*/
PyObject* wpChar_damage( wpChar* self, PyObject* args )
{
	signed short tempshort; 

	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		clConsole.send( "Minimum argument count for char.damage is 1\n" );
		return PyFalse;
	}

	if( getArgInt( 0 ) == 0 )
		return PyTrue;

	// Play take-damage animation / soundeffect
	Combat::playGetHitSoundEffect( self->pChar );	
	Combat::playGetHitAnimation( self->pChar );

	self->pChar->setHp( self->pChar->hp() - getArgInt( 0 ) );
	self->pChar->updateHealth();

	if( self->pChar->hp() <= 0 )
		self->pChar->kill();

	return PyTrue;
}

/*!
	Shows an emote above the chars head
*/
PyObject* wpChar_emote( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !PyTuple_Size( args ) || !PyString_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		clConsole.send( "Minimum argument count for char.emote is 1\n" );
		return PyFalse;
	}

	QString message = QString( "*%1*" ).arg( PyString_AsString( PyTuple_GetItem( args, 0 ) ) );
	self->pChar->emote( message );
	return PyTrue;
}

/*!
	Takes at least one argument (item-id)
	Optionally the color
	It returns the amount of a resource
	available
*/
PyObject* wpChar_countresource( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;
	
	if( PyTuple_Size( args ) < 1 || !checkArgInt( 0 ) )
	{
		clConsole.send( "Minimum argument count for char.countresource is 1\n" );
		return PyInt_FromLong( 0 );
	}

	UINT16 id = getArgInt( 0 );
	INT16 color = -1;

	if( PyTuple_Size( args ) > 1 && checkArgInt( 1 ) )
		color = getArgInt( 1 );

	P_ITEM pPack = self->pChar->getBackpack();
	UINT16 avail = 0;

	if( pPack )
		avail = pPack->CountItems( id, color );

	return PyInt_FromLong( avail );
}

PyObject* wpChar_isitem( wpChar* self, PyObject* args )
{
	return PyFalse;
}

PyObject* wpChar_ischar( wpChar* self, PyObject* args )
{
	return PyTrue;
}

/*!
	Returns the custom tag passed
*/
PyObject* wpChar_gettag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return Py_None;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for gettag is 1\n" );
		return Py_None;
	}

	QString key = getArgStr( 0 );
	cVariant value = self->pChar->tags.get( key );

	if( value.type() == cVariant::String )
		return PyString_FromString( value.asString().latin1() );
	else if( value.type() == cVariant::Int )
		return PyInt_FromLong( value.asInt() );

	return Py_None;
}

/*!
	Sets a custom tag
*/
PyObject* wpChar_settag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) || ( !checkArgStr( 1 ) && !checkArgInt( 1 )  ) )
	{
		clConsole.send( "Minimum argument count for settag is 2\n" );
		return PyFalse;
	}

	QString key = getArgStr( 0 );

	self->pChar->tags.remove( key );

	if( checkArgStr( 1 ) )
		self->pChar->tags.set( key, cVariant( getArgStr( 1 ) ) );
	else if( checkArgInt( 1 ) )
		self->pChar->tags.set( key, cVariant( (int)getArgInt( 1 ) ) );

	return PyTrue;
}

/*!
	Checks if a certain tag exists
*/
PyObject* wpChar_hastag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for hastag is 1\n" );
		return PyFalse;
	}

	QString key = getArgStr( 0 );
	
	return self->pChar->tags.get( key ).isValid() ? PyTrue : PyFalse;
}

/*!
	Deletes a given tag
*/
PyObject* wpChar_deltag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for deltag is 1\n" );
		return PyFalse;
	}

	QString key = getArgStr( 0 );
	self->pChar->tags.remove( key );

	return PyTrue;
}

/*!
	Adds a follower
*/
PyObject* wpChar_addfollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for addfollower is 1" );
		return PyFalse;
	}

	P_CHAR pPet = getArgChar( 0 );
	
	if( pPet )
		self->pChar->addFollower( pPet );

	return PyTrue;
}

/*!
	Removes a follower
*/
PyObject* wpChar_removefollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for removefollower is 1" );
		return PyFalse;
	}
  
	P_CHAR pPet = getArgChar( 0 );
	
	if( pPet )
		self->pChar->removeFollower( pPet );

	return PyTrue;
}

/*!
	Checks if the Char has a follower
*/
PyObject* wpChar_hasfollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		clConsole.send( "Minimum argument count for hasfollower is 1" );
		return PyFalse;
	}

	P_CHAR pPet = getArgChar( 0 );
	
	if( pPet )
	{
		cChar::Followers::iterator iter = self->pChar->followers().begin();

		for( ; iter != self->pChar->followers().end(); ++iter )
			if( (*iter) == pPet )
				return PyTrue;
	}

	return PyTrue;
}

/*!
	Resends the healthbar to the environment.
*/
PyObject* wpChar_updatehealth( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->updateHealth();
	return PyTrue;
}

/*!
	Resends the mana to this character.
*/
PyObject* wpChar_updatemana( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !self->pChar->socket() )
		return PyFalse;
	
	self->pChar->socket()->updateMana();

	return PyTrue;
}

/*!
	Resends the Stamina to this character.
*/
PyObject* wpChar_updatestamina( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !self->pChar->socket() )
		return PyFalse;
	
	self->pChar->socket()->updateStamina();

	return PyTrue;
}

/*!
	Resends all stats to this character.
*/
PyObject* wpChar_updatestats( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !self->pChar->socket() )
		return PyFalse;
	
	self->pChar->socket()->sendStatWindow();

	return PyTrue;
}


/*!
	What weapon is the character currently wearing?
*/
PyObject* wpChar_getweapon( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetItemObject( self->pChar->getWeapon() ); 
}

/*!
	Turns towards a specific object.
*/
PyObject* wpChar_turnto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgObject( 0 ) )
	{
		clConsole.send( "Minimum argument count for turnto: 1\n" );
		return PyFalse;
	}

	cUObject *object = 0;

	if( checkArgChar( 0 ) )
		object = getArgChar( 0 );
	else if( checkArgItem( 0 ) )
		object = getArgItem( 1 );

	if( object )
		self->pChar->turnTo( object );

	return PyTrue;
}

/*!
	Mounts this character on a specific mount.
*/
PyObject* wpChar_mount( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		clConsole.send( "Minimum argument count for turnto: 1\n" );
		return PyFalse;
	}

	P_CHAR pChar = getArgChar( 0 );

	if( pChar )
		self->pChar->mount( pChar );

	return PyTrue;
}

/*!
	Unmounts this character and returns the old mount.
*/
PyObject* wpChar_unmount( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetCharObject( self->pChar->unmount() );
}

/*!
	Equips a given item on this character.
*/
PyObject* wpChar_equip( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgItem( 0 ) )
	{
		clConsole.send( "Minimum argument count for equip: 1\n" );
		return PyFalse;
	}

	P_ITEM pItem = getArgItem( 0 );
	
	if( pItem )
		self->pChar->wear( pItem );

	return PyTrue;
}

/*!
	Gets or Autocreates a bankbox for the character.
*/
PyObject* wpChar_getbankbox( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetItemObject( self->pChar->getBankBox() );
}

/*!
	Gets or Autocreates a backpack for the character.
*/
PyObject* wpChar_getbackpack( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetItemObject( self->pChar->getBackpack() );
}

/*!
	Shows a moving effect moving toward a given object or coordinate.
*/
PyObject* wpChar_movingeffect( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( ( !checkArgObject( 1 ) && !checkArgCoord( 1 ) ) || !checkArgInt( 0 ) )
	{
		clConsole.send( "Minimum argument count for movingeffect is 2\n" );
		return PyFalse;
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

//void effect( UINT16 id, cUObject *target, bool fixedDirection = true, bool explodes = false, UINT8 speed = 10, UINT16 hue = 0, UINT16 renderMode = 0 );

	if( object )
		self->pChar->effect( id, (cUObject*)object, fixedDirection, explodes, speed, hue, renderMode );
	else 
		self->pChar->effect( id, (Coord_cl)pos, fixedDirection, explodes, speed, hue, renderMode );

	return PyTrue;
}

/*!
	Shows an effect staying with this character.
*/
PyObject* wpChar_effect( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		clConsole.send( "Minimum argument count for effect is 1\n" );
		return PyFalse;
	}
	
	UINT16 id = getArgInt( 0 );

	// Optional Arguments
	UINT8 duration = 5;
	UINT8 speed = 10;
	UINT16 hue = 0;
	UINT16 renderMode = 0;
	
	if( checkArgInt( 1 ) )
		speed = getArgInt( 1 );

	if( checkArgInt( 2 ) )
		speed = getArgInt( 2 );

	if( checkArgInt( 3 ) )
		hue = getArgInt( 3 );

	if( checkArgInt( 4 ) )
		renderMode = getArgInt( 4 );

	self->pChar->effect( id, speed, duration, hue, renderMode );

	return PyTrue;
}

/*!
	Adds a temp effect to this character.
*/
PyObject* wpChar_addtimer( wpChar* self, PyObject* args )
{
	// Three arguments
	if( PyTuple_Size( args ) != 3 || !checkArgInt( 0 ) || !checkArgStr( 1 ) || !PyList_Check( PyTuple_GetItem( args, 2 ) ) )
	{
		clConsole.send( "Minimum argument count for char.addtimer is 3" );
		return PyFalse;
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
	
	effect->setDest( self->pChar->serial );
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );

	return PyFalse;
}

static PyMethodDef wpCharMethods[] = 
{
	{ "moveto",			(getattrofunc)wpChar_moveto, METH_VARARGS, "Moves the character to the specified location." },
	{ "resurrect",		(getattrofunc)wpChar_resurrect, METH_VARARGS, "Resurrects the character." },
	{ "kill",			(getattrofunc)wpChar_kill, METH_VARARGS, "This kills the character." },
	{ "damage",			(getattrofunc)wpChar_damage, METH_VARARGS, "This damages the current character." },
    { "update",			(getattrofunc)wpChar_update, METH_VARARGS, "Resends the char to all clients in range." },
	{ "removefromview", (getattrofunc)wpChar_removefromview, METH_VARARGS, "Removes the char from all surrounding clients." },
	{ "message",		(getattrofunc)wpChar_message, METH_VARARGS, "Displays a message above the characters head - only visible for the player." },
	{ "soundeffect",	(getattrofunc)wpChar_soundeffect, METH_VARARGS, "Plays a soundeffect for the character." },
	{ "distanceto",		(getattrofunc)wpChar_distanceto, METH_VARARGS, "Distance to another object or a given position." },
	{ "action",			(getattrofunc)wpChar_action, METH_VARARGS, "Lets the char perform an action." },
	{ "directionto",	(getattrofunc)wpChar_directionto, METH_VARARGS, "Distance to another object or a given position." },
	{ "checkskill",		(getattrofunc)wpChar_checkskill, METH_VARARGS, "Performs a skillcheck for the character." },
	{ "itemonlayer",	(getattrofunc)wpChar_itemonlayer, METH_VARARGS, "Returns the item currently weared on a specific layer, or returns none." },
	{ "combatskill",	(getattrofunc)wpChar_combatskill, METH_VARARGS, "Returns the combat skill the character would currently use." },
	{ "getweapon",		(getattrofunc)wpChar_getweapon, METH_VARARGS, "What weapon does the character currently wear." },
	{ "useresource",	(getattrofunc)wpChar_useresource, METH_VARARGS, "Consumes a resource posessed by the char." },
	{ "countresource",	(getattrofunc)wpChar_countresource, METH_VARARGS, "Counts the amount of a certain resource the user has." },
	{ "emote",			(getattrofunc)wpChar_emote, METH_VARARGS, "Shows an emote above the character." },
	{ "turnto",			(getattrofunc)wpChar_turnto, METH_VARARGS, "Turns towards a specific object and resends if neccesary." },
	{ "equip",			(getattrofunc)wpChar_equip, METH_VARARGS, "Equips a given item on this character." },
	{ "addtimer",		(getattrofunc)wpChar_addtimer, METH_VARARGS, "Adds a timer to this character." },

	// Update Stats
	{ "updatestats",	(getattrofunc)wpChar_updatemana, METH_VARARGS, "Resends other stats to this character." },
	{ "updatemana",		(getattrofunc)wpChar_updatemana, METH_VARARGS, "Resends the manabar to this character." },
	{ "updatestamina",	(getattrofunc)wpChar_updatestamina, METH_VARARGS, "Resends the stamina bar to this character." },
	{ "updatehealth",	(getattrofunc)wpChar_updatehealth, METH_VARARGS, "Resends the healthbar to the environment." },

	// Mount/Unmount
	{ "unmount",		(getattrofunc)wpChar_unmount, METH_VARARGS, "Unmounts this character and returns the character it was previously mounted." },
	{ "mount",			(getattrofunc)wpChar_mount, METH_VARARGS, "Mounts this on a specific mount." },

	// Effects
	{ "movingeffect",			(getattrofunc)wpChar_movingeffect, METH_VARARGS, "Shows a moving effect moving toward a given object or coordinate." },
	{ "effect",					(getattrofunc)wpChar_effect, METH_VARARGS, "Shows an effect staying with this character." },

	// Bank/Backpack
	{ "getbankbox",		(getattrofunc)wpChar_getbankbox, METH_VARARGS,	"Gets and autocreates a bankbox for the character." },
	{ "getbackpack",	(getattrofunc)wpChar_getbackpack, METH_VARARGS, "Gets and autocreates a backpack for the character." },

	// Follower System
	{ "addfollower",	(getattrofunc)wpChar_addfollower, METH_VARARGS, "Adds a follower to the user." },
	{ "removefollower",	(getattrofunc)wpChar_removefollower, METH_VARARGS, "Removes a follower from the user." },
	{ "hasfollower",	(getattrofunc)wpChar_hasfollower, METH_VARARGS, "Checks if a certain character is a follower of this." },

	// Tag System
	{ "gettag",				(getattrofunc)wpChar_gettag, METH_VARARGS, "Gets a tag assigned to a specific char." },
	{ "settag",				(getattrofunc)wpChar_settag, METH_VARARGS, "Sets a tag assigned to a specific char." },
	{ "hastag",				(getattrofunc)wpChar_hastag, METH_VARARGS, "Checks if a certain char has the specified tag." },
	{ "deltag",				(getattrofunc)wpChar_deltag, METH_VARARGS, "Deletes the specified tag." },

	// Is*? Functions
	{ "isitem",			(getattrofunc)wpChar_isitem, METH_VARARGS, "Is this an item." },
	{ "ischar",			(getattrofunc)wpChar_ischar, METH_VARARGS, "Is this a char." },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpChar_getAttr( wpChar *self, char *name )
{
	pGetStr( "name", name )
	else pGetStr( "orgname", orgname().latin1() )
	else pGetStr( "title", title().latin1() )

	else pGetInt( "id", id() )
	else pGetInt( "xid", xid() )
    
	else pGetInt( "dir", dir() )

	// Flags
	else pGetInt( "allmove", canMoveAll() ? 1 : 0 )
	else pGetInt( "frozen", isFrozen() ? 1 : 0 )
	else pGetInt( "iconmultis", viewHouseIcons() ? 1 : 0 )
	else pGetInt( "permhidden", isHiddenPermanently() ? 1 : 0 )
	else pGetInt( "hidden", hidden() )
	else pGetInt( "nomana", priv2() & 0x10 ? 1 : 0 )
	else pGetInt( "dispellable", priv2() & 0x20 ? 1 : 0 )
	else pGetInt( "permmagicreflect", priv2() & 0x40 ? 1 : 0 )
	else pGetInt( "noreags", priv2() & 0x80 ? 1 : 0 )

	else if( !strcmp( name, "fonttype" ) ) 
		return PyInt_FromLong( self->pChar->fonttype() );
	else if( !strcmp( name, "saycolor" ) ) 
		return PyInt_FromLong( self->pChar->saycolor() );
	else if( !strcmp( name, "emotecolor" ) ) 
		return PyInt_FromLong( self->pChar->emotecolor() );
	else if( !strcmp( name, "str" ) ) 
		return PyInt_FromLong( self->pChar->st() );
	else pGetInt( "dex", effDex() )
	else if( !strcmp( name, "int" ) )
		return PyInt_FromLong( self->pChar->in() );
	else if ( !strcmp( name, "str2" ) )
		return PyInt_FromLong( self->pChar->st2() );
	else pGetInt( "dex2", decDex() )
	else if( !strcmp( name, "int2" ) )
		return PyInt_FromLong( self->pChar->in2() );
	else if ( !strcmp( name, "health" ) )
		return PyInt_FromLong( self->pChar->hp() );
	else if ( !strcmp( name, "stamina" ) )
		return PyInt_FromLong( self->pChar->stm() );
	else if ( !strcmp( name, "mana" ) )
		return PyInt_FromLong( self->pChar->mn() );
	else if ( !strcmp( name, "hidamage" ) )
		return PyInt_FromLong(self->pChar->hidamage() );
	else if ( !strcmp( name, "lodamage" ) )
		return PyInt_FromLong( self->pChar->lodamage() );
	else if ( !strcmp( name, "npc" ) )
		return PyInt_FromLong( self->pChar->npc() );
	else if ( !strcmp( name, "shop" ) )
		return PyInt_FromLong( self->pChar->shop() );
	else if ( !strcmp( name, "cell" ) )
		return PyInt_FromLong( self->pChar->cell() );
	else if( !strcmp( name, "owner" ) )
		return PyGetCharObject( self->pChar->owner() );
	else if( !strcmp( name, "karma" ) )
		return PyInt_FromLong( self->pChar->karma() );
	else if( !strcmp( name, "fame" ) )
		return PyInt_FromLong( self->pChar->fame() );
	else if( !strcmp( name, "kills" ) )
		return PyInt_FromLong( self->pChar->kills() );
	else if( !strcmp( name, "deaths" ) )
		return PyInt_FromLong( self->pChar->deaths() );
	else if( !strcmp( name, "dead" ) )
		return PyInt_FromLong( self->pChar->dead() );
	else if( !strcmp( name, "backpack" ) )
		return PyGetItemObject( self->pChar->getBackpack() );

	else if( !strcmp( name, "def" ) )
		return PyInt_FromLong( self->pChar->def() );
	else if( !strcmp( name, "war" ) )
		return PyInt_FromLong( self->pChar->war() );
	
	// Target
	else if( !strcmp( "target", name ) )
	{
		if( isItemSerial( self->pChar->targ() ) )
			return PyGetItemObject( FindItemBySerial( self->pChar->targ() ) );
		else if( isCharSerial( self->pChar->targ() ) )
			return PyGetCharObject( FindCharBySerial( self->pChar->targ() ) );
	}

	else if( !strcmp( "npcwander", name ) )
		return PyInt_FromLong( self->pChar->npcWander() );
	else if( !strcmp( "oldnpcwander", name ) )
		return PyInt_FromLong( self->pChar->oldnpcWander() );
	
	// Region object
	else if( !strcmp( "region", name ) )
		return PyGetRegionObject( self->pChar->region() );

	else if( !strcmp( "skilldelay", name ) )
		return PyInt_FromLong( self->pChar->skilldelay() );
	else if( !strcmp( "objectdelay", name ) )
		return PyInt_FromLong( self->pChar->objectdelay() );
	else if( !strcmp( "taming", name ) )
		return PyInt_FromLong( self->pChar->taming() );
	else if( !strcmp( "summontimer", name ) )
		return PyInt_FromLong( self->pChar->summontimer() );
	else if( !strcmp( "visrange", name ) )
		return PyInt_FromLong( self->pChar->VisRange() );
	
	else if( !strcmp( "trackingtarget", name ) )
	{
		if( isItemSerial( self->pChar->trackingTarget() ) )
			return PyGetItemObject( FindItemBySerial( self->pChar->trackingTarget() ) );
		else if( isCharSerial( self->pChar->trackingTarget() ) )
			return PyGetCharObject( FindCharBySerial( self->pChar->trackingTarget() ) );
	}

	// Account object
	else if( !strcmp( "account", name ) )
	{
		return Py_None;
	}

	else pGetInt( "incognito", incognito() )
	else pGetInt( "polymorph", polymorph() )
	else pGetInt( "haircolor", haircolor() )
	else pGetInt( "hairstyle", hairstyle() )
	else pGetInt( "beardcolor", beardcolor() )
	else pGetInt( "beardstyle", beardstyle() )

	else if( !strcmp( "socket", name ) )
		return PyGetSocketObject( self->pChar->socket() );

	else if( !strcmp( "pos", name ) )
		return PyGetCoordObject( self->pChar->pos );

	else if( !strcmp( "baseskill", name ) )
	{
		wpSkills *skills = PyObject_New( wpSkills, &wpSkillsType );
		skills->base = true;
		skills->pChar = self->pChar;
		return (PyObject*)( skills );
	}

	else if( !strcmp( "skill", name ) )
	{
		wpSkills *skills = PyObject_New( wpSkills, &wpSkillsType );
		skills->base = false;
		skills->pChar = self->pChar;
		return (PyObject*)( skills );
	}

	else pGetInt( "direction", beardstyle() )
	else pGetInt( "serial", serial )

	else pGetStr( "profile", profile() )

	// If no property is found search for a method
	return Py_FindMethod( wpCharMethods, (PyObject*)self, name );
}

int wpChar_setAttr( wpChar *self, char *name, PyObject *value )
{
	setStrProperty( "name", pChar->name )
	else if ( !strcmp( "orgname", name) )
		self->pChar->setOrgname( PyString_AS_STRING( value ) );
	else if ( !strcmp( "title", name) )
		self->pChar->setTitle( PyString_AS_STRING( value ) );
	else setIntProperty( "serial", pChar->serial )
	else if( !strcmp( "body", name ) )
		self->pChar->setId( PyInt_AS_LONG( value ) );
	else if ( !strcmp("xbody", name ) )
		self->pChar->setXid(PyInt_AS_LONG( value ) );		
	else if ( !strcmp( "skin", name ) )
		self->pChar->setSkin( PyInt_AS_LONG(value ) );
	else if ( !strcmp( "xskin", name ) )
		self->pChar->setXSkin( PyInt_AS_LONG(value ) );
	else if ( !strcmp( "health", name ) )
		self->pChar->setHp( PyInt_AS_LONG( value ) );
	else if ( !strcmp( "stamina", name ) )
		self->pChar->setStm( PyInt_AS_LONG( value ) );
	else if ( !strcmp( "mana", name ) )
		self->pChar->setMn( PyInt_AS_LONG( value ) );
	else if ( !strcmp( "str", name ) )
		self->pChar->setSt( PyInt_AS_LONG( value ) );
	else if ( !strcmp( "dex", name ) )
		self->pChar->setDex( PyInt_AS_LONG( value ) );
	else if ( !strcmp( "int", name ) )
		self->pChar->setIn( PyInt_AS_LONG( value ) );
	else if( !strcmp("direction", name ) )
		self->pChar->setDir( PyInt_AS_LONG( value ) );
	else if( !strcmp("flags2", name ) )
		self->pChar->setPriv2( PyInt_AS_LONG( value ) );
	else if( !strcmp("hidamage", name ) )
		self->pChar->setHiDamage( PyInt_AS_LONG( value ) );
	else if( !strcmp("lodamage", name ) )
		self->pChar->setLoDamage( PyInt_AS_LONG( value ) );
	else if( !strcmp("objectdelay", name ) )
		self->pChar->setObjectDelay( PyInt_AS_LONG( value ) );
	else if( !strcmp( name, "pos" ) && checkWpCoord( value ) )
		self->pChar->moveTo( getWpCoord( value ) );

	return 0;
}

P_CHAR getWpChar( PyObject *pObj )
{
	if( pObj->ob_type != &wpCharType )
		return 0;

	wpChar *item = (wpChar*)( pObj );
	return item->pChar;
}

bool checkWpChar( PyObject *pObj )
{
	if( pObj->ob_type != &wpCharType )
		return false;
	else
		return true;
}
