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

#include "utilities.h"
#include "skills.h"
#include "content.h"
#include "tempeffect.h"

#include "../chars.h"
#include "../territories.h"
#include "../prototypes.h"
#include "../wolfpack.h"
#include "../classes.h"
#include "../newmagic.h"
#include "../skills.h"
#include "../combat.h"
#include "../srvparams.h"
#include "../walking.h"
#include "../commands.h"
#include "../wpscriptmanager.h"

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
int wpChar_compare( PyObject*, PyObject* );

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
	wpChar_compare,
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
	Q_UNUSED(args);
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
		PyErr_BadArgument();
		return NULL;
	}

	QString message = PyString_AsString( PyTuple_GetItem( args, 0 ) );

	if( ( self->pChar->id() == 0x3DB ) && message.startsWith( SrvParams->commandPrefix() ) )
		cCommands::instance()->process( self->pChar->socket(), message.right( message.length()-1 ) );
	else if( message.startsWith( SrvParams->commandPrefix() ) )
		cCommands::instance()->process( self->pChar->socket(), message.right( message.length()-1 ) );
	else if( PyTuple_Size( args ) == 2 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
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
	Coord_cl pos = self->pChar->pos();

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

	self->pChar->moveTo( pos );

	return PyTrue;
}

/*!
	Plays a creature specific sound.
*/
PyObject* wpChar_sound( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	playmonstersound( self->pChar, self->pChar->id(), getArgInt( 0 ) );
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
		PyErr_BadArgument();
		return NULL;
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
			return PyInt_FromLong( self->pChar->pos().distance( getWpCoord( pObj ) ) );

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( pItem->dist( self->pChar ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( pChar->dist( self->pChar ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pChar->pos();

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( self->pChar->pos().distance( pos ) );
	}

	PyErr_BadArgument();
	return NULL;
}

/*!
	Lets the character perform an action
*/
PyObject* wpChar_action( wpChar* self, PyObject* args )
{
	if( PyTuple_Size( args ) < 1 || !PyInt_Check( PyTuple_GetItem( args, 0 ) ) )
	{
		PyErr_BadArgument();
		return NULL;
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
			return PyInt_FromLong( chardirxyz( self->pChar, pItem->pos().x, pItem->pos().y ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( chardir( pChar, self->pChar ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pChar->pos();

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( chardirxyz( self->pChar, pos.x, pos.y ) );
	}

	PyErr_BadArgument();
	return NULL;
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
		PyErr_BadArgument();
		return NULL;
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

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyGetItemObject( self->pChar->atLayer( (cChar::enLayer)getArgInt( 0 ) ) );
}

/*!
	Returns the combat skill currently used by the character
*/
PyObject* wpChar_combatskill( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
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
		PyErr_BadArgument();
		return NULL;
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
	Q_UNUSED(args);
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
	Q_UNUSED(args);
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
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
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
		PyErr_BadArgument();
		return NULL;
	}

	QString message = QString( "*%1*" ).arg( PyString_AsString( PyTuple_GetItem( args, 0 ) ) );
	self->pChar->emote( message );
	return PyTrue;
}

/*!
	The character says something.
*/
PyObject* wpChar_say( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	INT16 color = -1;
	
	if( checkArgInt( 1 ) )
		color = getArgInt( 1 );

	self->pChar->talk( getArgStr( 0 ), color );
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
		PyErr_BadArgument();
		return NULL;
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
	Q_UNUSED(args);
	Q_UNUSED(self);
	return PyFalse;
}

PyObject* wpChar_ischar( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
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
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	cVariant value = self->pChar->tags().get( key );

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
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );

	self->pChar->tags().remove( key );

	if( checkArgStr( 1 ) )
		self->pChar->tags().set( key, cVariant( getArgStr( 1 ) ) );
	else if( checkArgInt( 1 ) )
		self->pChar->tags().set( key, cVariant( (int)getArgInt( 1 ) ) );

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
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	
	return self->pChar->tags().get( key ).isValid() ? PyTrue : PyFalse;
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
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	self->pChar->tags().remove( key );

	return PyTrue;
}

/*!
	Adds a follower
*/
PyObject* wpChar_addfollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
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

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
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

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
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
	Q_UNUSED(args);
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
	Q_UNUSED(args);
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
	Q_UNUSED(args);
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
	Q_UNUSED(args);
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
	Q_UNUSED(args);
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
		PyErr_BadArgument();
		return NULL;
	}

	cUObject *object = 0;

	if( checkArgChar( 0 ) )
		object = getArgChar( 0 );
	else if( checkArgItem( 0 ) )
		object = getArgItem( 0 );

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
		PyErr_BadArgument();
		return NULL;
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
	Q_UNUSED(args);
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
		PyErr_BadArgument();
		return NULL;
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
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetItemObject( self->pChar->getBankBox() );
}

/*!
	Gets or Autocreates a backpack for the character.
*/
PyObject* wpChar_getbackpack( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
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
		PyErr_BadArgument();
		return NULL;
	}
	
	UINT16 id = getArgInt( 0 );

	// Optional Arguments
	UINT8 speed = 5;
	UINT8 duration = 10;
	UINT16 hue = 0;
	UINT16 renderMode = 0;
	
	if( checkArgInt( 1 ) )
		speed = getArgInt( 1 );

	if( checkArgInt( 2 ) )
		duration = getArgInt( 2 );

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
PyObject* wpChar_dispel( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	// We now have the list of effects applied to pChar
	// Now check if we force dispelling and only have two arguments
	if( PyTuple_Size( args ) == 0 )
	{
		TempEffects::instance()->dispel( self->pChar, 0, false );
	}
	else if( checkArgChar( 0 ) )
	{
		// Iterate trough the list of tempeffects
		cChar::Effects effects = self->pChar->effects();

		P_CHAR pSource = getArgChar( 0 );
		bool force = checkArgInt( 1 ) && ( getArgInt( 1 ) != 0 );
		QString dispelid;
		PyObject *dispelargs = 0;

		if( checkArgStr( 2 ) )
			dispelid = getArgStr( 2 );

		if( PyList_Check( PyTuple_GetItem( args, 3 ) ) )
			dispelargs = PyList_AsTuple( PyTuple_GetItem( args, 3 ) );
		
		if( !dispelargs )
			dispelargs = PyTuple_New( 0 );

		for( uint i = 0; i < effects.size(); ++i )
		{
			// No python effect, but we are forcing.
			if( ( force || effects[i]->dispellable ) && dispelid.isNull() && effects[i]->objectID() != "cPythonEffect" )
			{
				effects[i]->Dispel( pSource, false );
				self->pChar->removeEffect( effects[i] );
				TempEffects::instance()->teffects.erase( effects[i] );
			}
			// We are dispelling everything and this is a python effect
			else if( ( force || effects[i]->dispellable ) && dispelid.isNull() && effects[i]->objectID() == "cPythonEffect" )
			{
				// At least use the specific dispel function
				cPythonEffect *pEffect = dynamic_cast< cPythonEffect* >( effects[i] );
				if( pEffect )
				{
					pEffect->Dispel( pSource, dispelargs );
					self->pChar->removeEffect( effects[i] );
					TempEffects::instance()->teffects.erase( effects[i] );
				}
			}
			// We are dispelling specific python effects
			else if( ( force || effects[i]->dispellable ) && effects[i]->objectID() == "cPythonEffect" && !dispelid.isNull() )
			{
				cPythonEffect *pEffect = dynamic_cast< cPythonEffect* >( effects[i] );
				if( pEffect && pEffect->dispelId() == dispelid )
				{
					pEffect->Dispel( pSource, dispelargs );
					self->pChar->removeEffect( effects[i] );
					TempEffects::instance()->teffects.erase( effects[i] );
				}
			}
			
		}
	}

	return PyTrue;
}

/*!
	Adds a temp effect to this character.
*/
PyObject* wpChar_addtimer( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

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
	
	// dispellable
	if( checkArgInt( 4 ) && getArgInt( 4 ) != 0 )
		effect->dispellable = true;
	else
		effect->dispellable = false;

	// dispelname
	if( checkArgStr( 5 ) )
		effect->setDispelId( getArgStr( 5 ) );

	// dispelfunc
	if( checkArgStr( 6 ) )
		effect->setDispelFunc( getArgStr( 6 ) );

	effect->setDest( self->pChar->serial() );
	effect->setExpiretime_ms( expiretime );
	TempEffects::instance()->insert( effect );

	return PyFalse;
}

/*!
	Checks if we can stand at a certain point.
*/
PyObject* wpChar_maywalk( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	// Four Arguments: x,y,z,map
	if( !checkArgInt( 0 ) || !checkArgInt( 1 ) || !checkArgInt( 2 ) || !checkArgInt( 3 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	//if( !mayWalk( self->pChar, Coord_cl( getArgInt( 0 ), getArgInt( 1 ), getArgInt( 2 ), getArgInt( 3 ) ) ) )
	int argx = getArgInt( 0 );
	int argy = getArgInt( 1 );
	int argz = getArgInt( 2 );
	int argmap = getArgInt( 3 );
	Coord_cl argcoord( argx, argy, argz, argmap );
	if( !mayWalk( self->pChar, argcoord ) )
		return PyFalse;
	else
		return PyTrue;
}

/*!
	Are we criminal.
*/
PyObject* wpChar_iscriminal( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return self->pChar->crimflag() ? PyTrue : PyFalse;
}

/*!
	Are we a murderer.
*/
PyObject* wpChar_ismurderer( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return self->pChar->isMurderer() ? PyTrue : PyFalse;
}

/*!
	Make this character criminal.
*/
PyObject* wpChar_criminal( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->criminal();

	return PyTrue;
}

/*!
	Let's this character attack someone else.
*/
PyObject* wpChar_attack( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_CHAR pChar = getArgChar( 0 );

	if( !pChar || self->pChar == pChar )
		return PyFalse;

	self->pChar->attackTarget( pChar );

	return PyTrue;
}

/*!
	The character should follow someone else.
*/
PyObject* wpChar_follow( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_CHAR pChar = getArgChar( 0 );

	if( !pChar || pChar == self->pChar )
		return PyFalse;

	self->pChar->setFtarg( pChar->serial() );
	self->pChar->setNpcWander( 1 );
	self->pChar->setNextMoveTime();

	return PyTrue;
}

/*!
	Disturbs whatever this character is doing right now.
*/
PyObject* wpChar_disturb( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->disturbMed();
	NewMagic->disturb( self->pChar );

	return PyTrue;
}

/*!
	The character should follow someone else.
*/
PyObject* wpChar_goto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgCoord( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	Coord_cl pos = getArgCoord( 0 );

	if( pos.map != self->pChar->pos().map )
	{
		PyErr_Warn( PyExc_Warning, "Cannot move to a different map using goto." );
		return PyFalse;
	}

	self->pChar->setPtarg( pos );
	self->pChar->setNpcWander( 6 );
	self->pChar->setNextMoveTime();

	return PyTrue;
}

/*!
	This resends the flags of a certain character.
	Please note that you have to do a socket.resendplayer in addition
	to this if you want to update the socket itself.
*/
PyObject* wpChar_updateflags( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->update( true );
	
	return PyTrue;
}

/*!
	Checks if a certain character is able to reach another object. 
	The second parameter specifies the range the character needs to be in.
*/
PyObject* wpChar_canreach( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free || !checkArgObject( 0 ) || !checkArgInt( 1 ) )
		return PyFalse;

	cUObject *obj = getArgChar( 0 );
	if( !obj )
		obj = getArgItem( 1 );

	if( !obj )
		return PyFalse;

	UINT32 range = getArgInt( 1 );

	if( self->pChar->pos().map != obj->pos().map )
		return PyFalse;

	if( self->pChar->pos().distance( obj->pos() ) > range )
		return PyFalse;

	if( !lineOfSight( self->pChar->pos(), obj->pos(), TREES_BUSHES|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
		return PyFalse;

	return PyTrue;
}

static PyMethodDef wpCharMethods[] = 
{
	{ "moveto",			(getattrofunc)wpChar_moveto, METH_VARARGS, "Moves the character to the specified location." },
	{ "resurrect",		(getattrofunc)wpChar_resurrect, METH_VARARGS, "Resurrects the character." },
	{ "kill",			(getattrofunc)wpChar_kill, METH_VARARGS, "This kills the character." },
	{ "damage",			(getattrofunc)wpChar_damage, METH_VARARGS, "This damages the current character." },
    { "update",			(getattrofunc)wpChar_update, METH_VARARGS, "Resends the char to all clients in range." },
	{ "updateflags",	(getattrofunc)wpChar_updateflags, METH_VARARGS, "Resends the character if flags have changed (take care, this might look like a move)." },
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
	{ "say",			(getattrofunc)wpChar_say, METH_VARARGS, "The character begins to talk." },
	{ "turnto",			(getattrofunc)wpChar_turnto, METH_VARARGS, "Turns towards a specific object and resends if neccesary." },
	{ "equip",			(getattrofunc)wpChar_equip, METH_VARARGS, "Equips a given item on this character." },
	{ "maywalk",		(getattrofunc)wpChar_maywalk, METH_VARARGS, "Checks if this character may walk to a specific cell." },
	{ "sound",			(getattrofunc)wpChar_sound, METH_VARARGS, "Play a creature specific sound." },
	{ "disturb",		(getattrofunc)wpChar_disturb, METH_VARARGS, "Disturbs whatever this character is doing right now." },
	{ "canreach",		(getattrofunc)wpChar_canreach, METH_VARARGS, "Checks if this character can reach a certain object." },
	
	// Mostly NPC functions
	{ "attack",			(getattrofunc)wpChar_attack, METH_VARARGS, "Let's the character attack someone else." },
	{ "goto",			(getattrofunc)wpChar_goto, METH_VARARGS, "The character should go to a coordinate." },
	{ "follow",			(getattrofunc)wpChar_follow, METH_VARARGS, "The character should follow someone else." },

	{ "addtimer",		(getattrofunc)wpChar_addtimer, METH_VARARGS, "Adds a timer to this character." },
	{ "dispel",			(getattrofunc)wpChar_dispel, METH_VARARGS, "Dispels this character (with special options)." },

	// Update Stats
	{ "updatestats",	(getattrofunc)wpChar_updatestats, METH_VARARGS, "Resends other stats to this character." },
	{ "updatemana",		(getattrofunc)wpChar_updatemana, METH_VARARGS, "Resends the manabar to this character." },
	{ "updatestamina",	(getattrofunc)wpChar_updatestamina, METH_VARARGS, "Resends the stamina bar to this character." },
	{ "updatehealth",	(getattrofunc)wpChar_updatehealth, METH_VARARGS, "Resends the healthbar to the environment." },

	// Mount/Unmount
	{ "unmount",		(getattrofunc)wpChar_unmount, METH_VARARGS, "Unmounts this character and returns the character it was previously mounted." },
	{ "mount",			(getattrofunc)wpChar_mount, METH_VARARGS, "Mounts this on a specific mount." },

	// Effects
	{ "movingeffect",	(getattrofunc)wpChar_movingeffect, METH_VARARGS, "Shows a moving effect moving toward a given object or coordinate." },
	{ "effect",			(getattrofunc)wpChar_effect, METH_VARARGS, "Shows an effect staying with this character." },

	// Bank/Backpack
	{ "getbankbox",		(getattrofunc)wpChar_getbankbox, METH_VARARGS,	"Gets and autocreates a bankbox for the character." },
	{ "getbackpack",	(getattrofunc)wpChar_getbackpack, METH_VARARGS, "Gets and autocreates a backpack for the character." },

	// Follower System
	{ "addfollower",	(getattrofunc)wpChar_addfollower, METH_VARARGS, "Adds a follower to the user." },
	{ "removefollower",	(getattrofunc)wpChar_removefollower, METH_VARARGS, "Removes a follower from the user." },
	{ "hasfollower",	(getattrofunc)wpChar_hasfollower, METH_VARARGS, "Checks if a certain character is a follower of this." },

	// Tag System
	{ "gettag",			(getattrofunc)wpChar_gettag, METH_VARARGS, "Gets a tag assigned to a specific char." },
	{ "settag",			(getattrofunc)wpChar_settag, METH_VARARGS, "Sets a tag assigned to a specific char." },
	{ "hastag",			(getattrofunc)wpChar_hastag, METH_VARARGS, "Checks if a certain char has the specified tag." },
	{ "deltag",			(getattrofunc)wpChar_deltag, METH_VARARGS, "Deletes the specified tag." },

	// Reputation System
	{ "iscriminal",		(getattrofunc)wpChar_iscriminal, METH_VARARGS, "Is this character criminal.." },
	{ "ismurderer",		(getattrofunc)wpChar_ismurderer, METH_VARARGS, "Is this character a murderer." },
	{ "criminal",		(getattrofunc)wpChar_criminal, METH_VARARGS, "Make this character criminal." },

	// Is*? Functions
	{ "isitem",			(getattrofunc)wpChar_isitem, METH_VARARGS, "Is this an item." },
	{ "ischar",			(getattrofunc)wpChar_ischar, METH_VARARGS, "Is this a char." },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpChar_getAttr( wpChar *self, char *name )
{
	// Python specific stuff
	pGetInt( "gm", isGM() ? 1 : 0 )
	
	else if( !strcmp( "region", name ) )
		return PyGetRegionObject( self->pChar->region() );

/*	else if( !strcmp( "account", name ) )
	{
		return Py_None;
	}*/

	else if( !strcmp( "socket", name ) )
		return PyGetSocketObject( self->pChar->socket() );

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

	else if( !strcmp( "followers", name ) )
	{
		cChar::Followers followers = self->pChar->followers();
		PyObject *rVal = PyTuple_New( followers.size() );

		for( uint i = 0; i < followers.size(); ++i )
			PyTuple_SetItem( rVal, i, PyGetCharObject( followers[i] ) );

		return rVal;
	}
	else if( !strcmp( "guards", name ) )
	{
		cChar::Followers guards = self->pChar->guardedby();
		PyObject *rVal = PyTuple_New( guards.size() );

		for( uint i = 0; i < guards.size(); ++i )
			PyTuple_SetItem( rVal, i, PyGetCharObject( guards[i] ) );

		return rVal;
	}
	else if( !strcmp( "events", name ) )
	{
		QStringList events = QStringList::split( ",", self->pChar->eventList() );
		PyObject *list = PyList_New( events.count() );
		for( uint i = 0; i < events.count(); ++i )
			PyList_SetItem( list, i, PyString_FromString( events[i].latin1() ) );
		return list;
	}
	else
	{
		cVariant result;
		stError *error = self->pChar->getProperty( name, result );

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

	// If no property is found search for a method
	return Py_FindMethod( wpCharMethods, (PyObject*)self, name );
}

int wpChar_setAttr( wpChar *self, char *name, PyObject *value )
{
	// Special Python things.
	if( !strcmp( "events", name ) )
	{
		if( !PyList_Check( value ) )
		{
			PyErr_BadArgument();
			return -1;
		}

		self->pChar->clearEvents();
		int i;
		for( i = 0; i < PyList_Size( value ); ++i )
		{
			if( !PyString_Check( PyList_GetItem( value, i ) ) )
				continue;

			WPDefaultScript *script = ScriptManager->find( PyString_AsString( PyList_GetItem( value, i ) ) );
			if( script )
				self->pChar->addEvent( script );
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

		stError *error = self->pChar->setProperty( name, val );

		if( error )
		{
			PyErr_Format( PyExc_TypeError, "Error while setting attribute '%s': %s", name, error->text.latin1() );
			delete error;
			return 0;
		}
	}

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

int wpChar_compare( PyObject *a, PyObject *b )
{
	// Both have to be characters
	if( a->ob_type != &wpCharType || b->ob_type != &wpCharType ) 
		return -1;

	P_CHAR pA = getWpChar( a );
	P_CHAR pB = getWpChar( b );

	return !( pA == pB );
}
