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

#include "utilities.h"
#include "skills.h"
#include "content.h"
#include "tempeffect.h"

#include "../territories.h"
#include "../skills.h"
#include "../combat.h"
#include "../srvparams.h"
#include "../walking.h"
#include "../commands.h"
#include "../scriptmanager.h"
#include "../makemenus.h"
#include "../npc.h"
#include "../basechar.h"
#include "../player.h"
#include "../singleton.h"
#include "objectcache.h"

/*!
	Struct for WP Python Chars
*/
typedef struct {
    PyObject_HEAD;
	P_CHAR pChar;
} wpChar;

// Note: Must be of a different type to cause more then 1 template instanciation
class cCharObjectCache : public cObjectCache< wpChar, 50 >
{
};

typedef SingletonHolder< cCharObjectCache > CharCache;

static void FreeCharObject( PyObject *obj )
{
	CharCache::instance()->freeObj( obj );
}

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
//	FreeCharObject,
	wpDealloc,    
	0,
    (getattrfunc)wpChar_getAttr,
    (setattrfunc)wpChar_setAttr,
	wpChar_compare,
};

PyObject* PyGetCharObject( P_CHAR pChar )
{
	if( !pChar )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

//	wpChar *returnVal = CharCache::instance()->allocObj( &wpCharType );
	wpChar *returnVal = PyObject_New( wpChar, &wpCharType );
	returnVal->pChar = pChar;
	return (PyObject*)returnVal;
}

// Methods

/*!
	Resends the character.
*/
static PyObject* wpChar_update( wpChar* self, PyObject* args )
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
static PyObject* wpChar_removefromview( wpChar* self, PyObject* args )
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
static PyObject* wpChar_message( wpChar* self, PyObject* args )
{
	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
	
	if( !player || !player->socket() )
		return PyFalse;

	if( checkArgStr( 0 ) )
	{
		QString message = getArgStr( 0 );

		if( ( player->bodyID() == 0x3DB ) && message.startsWith( SrvParams->commandPrefix() ) )
			Commands::instance()->process( player->socket(), message.right( message.length()-1 ) );
		else if( message.startsWith( SrvParams->commandPrefix() ) )
			Commands::instance()->process( player->socket(), message.right( message.length()-1 ) );
		else if( PyTuple_Size( args ) == 2 && PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			player->message( message, PyInt_AsLong( PyTuple_GetItem( args, 1 ) ) );
		else
			player->message( message );
	}
	else if( checkArgInt( 0 ) )
	{
		// Affix?
		unsigned int id;
		char *clilocargs = 0;
		char *affix = 0;

		if( !PyArg_ParseTuple( args, "i|ss:char.message( clilocid, [args], [affix] )", &id, &clilocargs, &affix ) )
			return 0;		

		// Cliloc Message
		if( affix )
			player->socket()->clilocMessageAffix( id, clilocargs, affix, 0x3b2, 3, player, false, false );
		else
			player->socket()->clilocMessage( id, clilocargs, 0x3b2, 3, player );
	}
	else
	{
		PyErr_BadArgument();
		return NULL;
	}

	return PyTrue;
}

/*!
	Moves the char to the specified location
*/
static PyObject* wpChar_moveto( wpChar* self, PyObject* args )
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
static PyObject* wpChar_sound( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	unsigned char arg = getArgInt( 0 );

	if( arg > cBaseChar::Bark_Death )
		return PyFalse;

	self->pChar->bark( (cBaseChar::enBark)arg );
	return PyTrue;
}

/*!
	Plays a soundeffect originating from the char
*/
static PyObject* wpChar_soundeffect( wpChar* self, PyObject* args )
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
static PyObject* wpChar_distanceto( wpChar* self, PyObject* args )
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
static PyObject* wpChar_action( wpChar* self, PyObject* args )
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
static PyObject* wpChar_directionto( wpChar* self, PyObject* args )
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
			return PyInt_FromLong( self->pChar->pos().direction( Coord_cl( pos.x, pos.y ) ) );
		}

		// Item
		P_ITEM pItem = getWpItem( pObj );
		if( pItem )
			return PyInt_FromLong( self->pChar->pos().direction( pItem->pos() ) );

		P_CHAR pChar = getWpChar( pObj );
        if( pChar )
			return PyInt_FromLong( pChar->direction( self->pChar ) );
	}
	else if( PyTuple_Size( args ) >= 2 ) // Min 2 
	{
		Coord_cl pos = self->pChar->pos();

		if( !PyInt_Check( PyTuple_GetItem( args, 0 ) ) || !PyInt_Check( PyTuple_GetItem( args, 1 ) ) )
			return PyInt_FromLong( -1 );

		pos.x = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		pos.y = PyInt_AsLong( PyTuple_GetItem( args, 1 ) );  

		return PyInt_FromLong( self->pChar->pos().direction( pos ) );
	}

	PyErr_BadArgument();
	return NULL;
}

/*!
	Performs a skillcheck using the given skill
	and minimum and maximum arguments.
*/
static PyObject* wpChar_checkskill( wpChar* self, PyObject* args )
{
	if( self->pChar->free )
		return PyFalse;

	unsigned short skill;
	unsigned short min, max;

	if( !PyArg_ParseTuple( args, "hhh|char.checkskill( skill, min, max )", &skill, &min, &max ) )
		return 0;
	
	bool success = self->pChar->checkSkill( skill, min, max );

	return success ? PyTrue : PyFalse;
}

/*!
	Returns an item object for an item equipped
	on the specified layer. If there is no item
	it returns Py_None.
*/
static PyObject* wpChar_itemonlayer( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	return PyGetItemObject( self->pChar->atLayer( (cBaseChar::enLayer)getArgInt( 0 ) ) );
}

/*!
	Returns the combat skill currently used by the character
*/
static PyObject* wpChar_combatskill( wpChar* self, PyObject* args )
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
static PyObject* wpChar_useresource( wpChar* self, PyObject* args )
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
static PyObject* wpChar_resurrect( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->resurrect();

	return PyTrue;
}

/*!
	Kills the current character
*/
static PyObject* wpChar_kill( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	self->pChar->damage( DAMAGE_GODLY, self->pChar->hitpoints() );

	return PyTrue;
}

/*!
	Deals damage to the character
	This cannot be used for healing!
*/
static PyObject* wpChar_damage( wpChar* self, PyObject* args )
{
	if( self->pChar->free )
		return PyFalse;

	int type, amount;
	PyObject *source = 0;
	
	if( !PyArg_ParseTuple( args, "ii|O:char.damage( type, amount, source )", &type, &amount, &source ) )
		return 0;

	cUObject *pSource = 0;
	if( checkWpItem( source ) )
		pSource = getWpItem( source );
	else if( checkWpChar( source ) )
		pSource = getWpChar( source );

	return PyInt_FromLong( self->pChar->damage( (eDamageType)type, amount, pSource ) );
}

/*!
	Shows an emote above the chars head
*/
static PyObject* wpChar_emote( wpChar* self, PyObject* args )
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
static PyObject* wpChar_say( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgStr( 0 ) )
	{
		P_NPC npc = dynamic_cast<P_NPC>( self->pChar);

		if ( !npc )
			return PyFalse;

		if( !checkArgInt( 0 ) )
		{
			PyErr_BadArgument();
			return 0;
		}

		ushort color = -1;
		uint cliloc = getArgInt( 0 );
	
		if( checkArgInt( 1 ) )
			color = getArgInt( 1 );
	
		npc->talk( cliloc, 0, 1 );
		return PyTrue;
	}
	else
	{
		INT16 color = -1;
		
		if( checkArgInt( 1 ) )
			color = getArgInt( 1 );

		self->pChar->talk( getArgStr( 0 ), color );
		return PyTrue;
	}
	return PyFalse;
}

/*!
	Takes at least one argument (item-id)
	Optionally the color
	It returns the amount of a resource
	available
*/
static PyObject* wpChar_countresource( wpChar* self, PyObject* args )
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

static PyObject* wpChar_isitem( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
	return PyFalse;
}

static PyObject* wpChar_ischar( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	Q_UNUSED(self);
	return PyTrue;
}

/*!
	Returns the custom tag passed
*/
static PyObject* wpChar_gettag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	cVariant value = self->pChar->getTag( key );

	if( value.type() == cVariant::String )
		return PyString_FromString( value.asString().latin1() );
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
static PyObject* wpChar_settag( wpChar* self, PyObject* args )
{
	if( self->pChar->free )
		return PyFalse;

	char *key;
	PyObject *object;

	if (!PyArg_ParseTuple( args, "sO:char.settag( name, value )", &key, &object ))
		return 0;

	if (PyString_Check(object)) {
		self->pChar->setTag(key, cVariant(PyString_AsString(object)));
	} else if (PyUnicode_Check(object)) {
		self->pChar->setTag(key, cVariant(QString::fromUcs2((ushort*)PyUnicode_AsUnicode(object))));
	} else if (PyInt_Check(object)) {
		self->pChar->setTag(key, cVariant((int)PyInt_AsLong(object)));
	} else if (PyFloat_Check(object)) {
		self->pChar->setTag(key, cVariant((double)PyFloat_AsDouble(object)));
	}

	return PyTrue;
}

/*!
	Checks if a certain tag exists
*/
static PyObject* wpChar_hastag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	
	return self->pChar->getTag( key ).isValid() ? PyTrue : PyFalse;
}

/*!
	Deletes a given tag
*/
static PyObject* wpChar_deltag( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString key = getArgStr( 0 );
	self->pChar->removeTag( key );

	return PyTrue;
}

/*!
 * Sends MakeMenu defined as xml file to this character
 */
static PyObject* wpChar_sendmakemenu( wpChar* self, PyObject* args )
{
	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
	if( !self->pChar || self->pChar->free || !player )
		return PyFalse;
	if( !player->socket() )
		return PyFalse;
	if( PyTuple_Size( args ) < 1 || !checkArgStr( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	QString menuName = getArgStr( 0 );
	MakeMenus::instance()->callMakeMenu( player->socket(), menuName );
	
	return PyTrue;
}

/*!
	Adds a follower
*/
static PyObject* wpChar_addfollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
	if ( !player )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	P_NPC pPet = dynamic_cast<P_NPC>( getArgChar( 0 ) );
	
	if( pPet )
	{
		player->addPet( pPet );
		return PyTrue;
	}
	return PyFalse;
}

/*!
	Removes a follower
*/
static PyObject* wpChar_removefollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}
  
	P_NPC pPet = dynamic_cast<P_NPC>( getArgChar( 0 ) );
	
	if( pPet )
	{
		player->removePet( pPet );
		return PyTrue;
	}
	return PyFalse;
}

/*!
	Checks if the Char has a follower
*/
static PyObject* wpChar_hasfollower( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
	if ( !player )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	P_NPC pPet = dynamic_cast<P_NPC>( getArgChar( 0 ) );
	
	if( pPet )
	{
		cBaseChar::CharContainer::const_iterator iter = player->pets().begin();
		cBaseChar::CharContainer::const_iterator end  = player->pets().end();
		return std::binary_search( iter, end, (cBaseChar*)pPet ) ? PyTrue : PyFalse;
	}

	return PyFalse;
}

/*!
	Resends the healthbar to the environment.
*/
static PyObject* wpChar_updatehealth( wpChar* self, PyObject* args )
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
static PyObject* wpChar_updatemana( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	if( !player->socket() )
		return PyFalse;
	
	player->socket()->updateMana();

	return PyTrue;
}

/*!
	Resends the Stamina to this character.
*/
static PyObject* wpChar_updatestamina( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	if( !player->socket() )
		return PyFalse;
	
	player->socket()->updateStamina();

	return PyTrue;
}

/*!
	Resends all stats to this character.
*/
static PyObject* wpChar_updatestats( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	if( !player->socket() )
		return PyFalse;
	
	player->socket()->sendStatWindow();

	return PyTrue;
}


/*!
	What weapon is the character currently wearing?
*/
static PyObject* wpChar_getweapon( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetItemObject( self->pChar->getWeapon() ); 
}

/*!
	Turns towards a specific object.
*/
static PyObject* wpChar_turnto( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( checkArgCoord( 0 ) )
	{
		Coord_cl pos = getArgCoord( 0 );
		self->pChar->turnTo( pos );
		return PyTrue;
	}

	if( !checkArgObject( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	cUObject *object = 0;

	if( checkArgChar( 0 ) )
		object = getArgChar( 0 );
	else if( checkArgItem( 0 ) )
	{
		P_ITEM pItem = getArgItem( 0 );
		
		pItem = pItem->getOutmostItem();

		if( pItem->container() && pItem->container()->isChar() )
			object = pItem->container();
		else
			object = pItem;
	}

	if( object && object != self->pChar )
		self->pChar->turnTo( object );

	return PyTrue;
}

/*!
	Mounts this character on a specific mount.
*/
static PyObject* wpChar_mount( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	P_NPC pChar = dynamic_cast<P_NPC>( getArgChar( 0 ) );

	if( pChar )
	{
		player->mount( pChar );
		return PyTrue;
	}
	return PyFalse;
}

/*!
	Unmounts this character and returns the old mount.
*/
static PyObject* wpChar_unmount( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;
	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	return PyGetCharObject( player->unmount() );
}

/*!
	Equips a given item on this character.
*/
static PyObject* wpChar_equip( wpChar* self, PyObject* args )
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
static PyObject* wpChar_getbankbox( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	return PyGetItemObject( player->getBankBox() );
}

/*!
	Gets or Autocreates a backpack for the character.
*/
static PyObject* wpChar_getbackpack( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return PyGetItemObject( self->pChar->getBackpack() );
}

/*!
	Shows a moving effect moving toward a given object or coordinate.
*/
static PyObject* wpChar_movingeffect( wpChar* self, PyObject* args )
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
static PyObject* wpChar_effect( wpChar* self, PyObject* args )
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
static PyObject* wpChar_dispel( wpChar* self, PyObject* args )
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
		cBaseChar::EffectContainer effects = self->pChar->effects();

		P_CHAR pSource = getArgChar( 0 );
		bool force = checkArgInt( 1 ) && ( getArgInt( 1 ) != 0 );
		QString dispelid;
		PyObject *dispelargs = 0;

		if( checkArgStr( 2 ) )
			dispelid = getArgStr( 2 );

		if( PyTuple_Size( args ) > 3 && PyList_Check( PyTuple_GetItem( args, 3 ) ) )
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
				TempEffects::instance()->erase( effects[i] );
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
					TempEffects::instance()->erase( effects[i] );
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
					TempEffects::instance()->erase( effects[i] );
				}
			}
			
		}
	}

	return PyTrue;
}

/*!
	Adds a temp effect to this character.
*/
static PyObject* wpChar_addtimer( wpChar* self, PyObject* args )
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
static PyObject* wpChar_maywalk( wpChar* self, PyObject* args )
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
static PyObject* wpChar_iscriminal( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return self->pChar->isCriminal() ? PyTrue : PyFalse;
}

/*!
	Are we a murderer.
*/
static PyObject* wpChar_ismurderer( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	return self->pChar->isMurderer() ? PyTrue : PyFalse;
}

/*!
	Make this character criminal.
*/
static PyObject* wpChar_criminal( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	player->makeCriminal();

	return PyTrue;
}

/*!
	Are we dead.
*/
static PyObject* wpChar_isdead( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyTrue;

	return self->pChar->isDead() ? PyTrue : PyFalse;
}

/*!
	Let's this character attack someone else.
*/
static PyObject* wpChar_attack( wpChar* self, PyObject* args )
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

	self->pChar->fight( pChar );

	return PyTrue;
}

/*!
	The character should follow someone else.
*/
static PyObject* wpChar_follow( wpChar* self, PyObject* args )
{
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	if( !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_PLAYER pChar = dynamic_cast<P_PLAYER>( getArgChar( 0 ) );

	if( !pChar || pChar == self->pChar )
		return PyFalse;

	P_NPC npc = dynamic_cast<P_NPC>( self->pChar );
	if ( !npc )
		return PyFalse;

	pChar->addPet( npc );
	npc->setWanderType( enFollowTarget );
	npc->setWanderFollowTarget( pChar );

	return PyTrue;
}

/*!
	Disturbs whatever this character is doing right now.
*/
static PyObject* wpChar_disturb( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( !self->pChar || self->pChar->free )
		return PyFalse;

	P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

	if ( !player )
		return PyFalse;

	player->disturbMed();

	return PyTrue;
}

/*!
	The character should follow someone else.
*/
static PyObject* wpChar_goto( wpChar* self, PyObject* args )
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

	P_NPC npc = dynamic_cast<P_NPC>( self->pChar );

	if ( !npc )
		return PyFalse;

	npc->setWanderType( enDestination );
	npc->setWanderDestination( pos );

	return PyTrue;
}

/*!
	This resends the flags of a certain character.
	Please note that you have to do a socket.resendplayer in addition
	to this if you want to update the socket itself.
*/
static PyObject* wpChar_updateflags( wpChar* self, PyObject* args )
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
static PyObject* wpChar_canreach( wpChar* self, PyObject* args )
{
	Q_UNUSED(args);
	if( self->pChar->free || ( !checkArgObject( 0 ) && !checkArgCoord( 0 ) ) || !checkArgInt( 1 ) )
		return PyFalse;

	P_PLAYER pPlayer = dynamic_cast< P_PLAYER >( self->pChar );

	if( pPlayer && pPlayer->isGM() )
		return PyTrue;

	Coord_cl pos;

	// Parameter 1: Coordinate
	if( checkArgCoord( 0 ) )
	{
		pos = getArgCoord( 0 );
	}
	// Parameter1: Item/Char
	else
	{
		cUObject *obj = getArgChar( 0 );

		if( !obj )
		{
			P_ITEM pItem = getArgItem( 0 );

			if( pItem && pItem->getOutmostChar() == self->pChar )
				return PyTrue;

			obj = pItem;
		}
	
		if( !obj )
			return PyFalse;

		pos = obj->pos();
	}

	UINT32 range = getArgInt( 1 );

	if( self->pChar->pos().map != pos.map )
		return PyFalse;

	if( self->pChar->pos().distance( pos ) > range )
		return PyFalse;

	if( !self->pChar->pos().lineOfSight( pos ) )
		return PyFalse;

	return PyTrue;
}

static PyObject* wpChar_canpickup( wpChar* self, PyObject* args )
{
	if( self->pChar->free )
		return PyFalse;

	if( PyTuple_Size( args ) != 1 )
	{
		PyErr_BadArgument();
		return 0;
	}

	P_ITEM pItem = getArgItem( 0 );

	P_PLAYER pPlayer = dynamic_cast< P_PLAYER >( self->pChar );

	if( !pPlayer )
		return PyFalse;

	return pPlayer->canPickUp( pItem ) ? PyTrue : PyFalse;
}

static PyObject* wpChar_cansee( wpChar *self, PyObject *args )
{
	if( self->pChar->free )
		return PyFalse;

	PyObject *object = 0;
	unsigned int touch = 1;

	if( !PyArg_ParseTuple( args, "O|i:char.cansee( [char,item,pos], [touch] )", &object, &touch ) )
		return 0;

	Coord_cl &pos = Coord_cl::null;

	// Item
	if( checkWpItem( object ) )
	{
		P_ITEM pItem = getWpItem( object );

		// Invisibility Check
		if( ( ( pItem->visible() == 1 && pItem->owner() != self->pChar ) || pItem->visible() == 2 ) )
		{
			P_PLAYER pPlayer = dynamic_cast< P_PLAYER >( self->pChar );
			if( !pPlayer || !pPlayer->isGM() )
				return PyFalse;
		}

		pos = pItem->pos();
	}

	// Char
	else if( checkWpChar( object ) )
	{
		P_CHAR pChar = getWpChar( object );

		if( pChar->isHidden() || pChar->isInvisible() )
		{
			P_PLAYER pPlayer = dynamic_cast< P_PLAYER >( self->pChar );
			if( !pPlayer || !pPlayer->isGM() )
				return PyFalse;
		}

		pos = pChar->pos();
	}

	// Position
	else if( checkWpCoord( object ) )
	{
		pos = getWpCoord( object );
	}
	else
	{
		PyErr_SetString( PyExc_RuntimeError, "Incompatible Object for char.cansee()." );
		return 0;
	}

	if( pos == Coord_cl::null )
		return PyFalse;

	bool result = self->pChar->pos().lineOfSight( pos, touch != 0 );

	return result ? PyTrue : PyFalse;
}

static PyObject* wpChar_lightning( wpChar *self, PyObject *args )
{
	unsigned short hue = 0;
	
	if( !PyArg_ParseTuple( args, "|h:char.lightning( [hue] )", &hue ) )
		return 0;

	self->pChar->lightning( hue );

	return PyTrue;
}

static PyObject* wpChar_resendtooltip( wpChar *self, PyObject *args )
{
	if( self->pChar->free )
		return PyFalse;

	self->pChar->resendTooltip();
	return PyTrue;
}

static PyObject* wpChar_additem( wpChar *self, PyObject *args )
{
	if( self->pChar->free )
		return PyFalse;

	if( !checkArgInt( 0 ) && !checkArgItem( 1 ) )
	{
		PyErr_BadArgument();
		return 0;
	}
	bool handleWeight = true;
	bool noRemove = false;

	if( PyTuple_Size( args ) > 2 && checkArgInt( 2 ) )
		handleWeight = getArgInt( 2 ) > 0?true:false;

	if( PyTuple_Size( args ) > 2 && checkArgInt( 3 ) )
		noRemove = getArgInt( 3 ) > 0?true:false;

	int layer = getArgInt( 0 );
	P_ITEM pItem = getArgItem( 1 );
	
	if( !pItem )
		return PyFalse;

	self->pChar->addItem( (cBaseChar::enLayer)layer, pItem, handleWeight, noRemove );

	return PyTrue;
}


static PyMethodDef wpCharMethods[] = 
{
	{ "moveto",			(getattrofunc)wpChar_moveto,			METH_VARARGS, "Moves the character to the specified location." },
	{ "resurrect",		(getattrofunc)wpChar_resurrect,			METH_VARARGS, "Resurrects the character." },
	{ "kill",			(getattrofunc)wpChar_kill,				METH_VARARGS, "This kills the character." },
	{ "damage",			(getattrofunc)wpChar_damage,			METH_VARARGS, "This damages the current character." },
    { "update",			(getattrofunc)wpChar_update,			METH_VARARGS, "Resends the char to all clients in range." },
	{ "resendtooltip",	(getattrofunc)wpChar_resendtooltip,		METH_VARARGS, "Resends the tooltip for this character." },
	{ "updateflags",	(getattrofunc)wpChar_updateflags,		METH_VARARGS, "Resends the character if flags have changed (take care, this might look like a move)." },
	{ "removefromview", (getattrofunc)wpChar_removefromview,	METH_VARARGS, "Removes the char from all surrounding clients." },
	{ "message",		(getattrofunc)wpChar_message,			METH_VARARGS, "Displays a message above the characters head - only visible for the player." },
	{ "soundeffect",	(getattrofunc)wpChar_soundeffect,		METH_VARARGS, "Plays a soundeffect for the character." },
	{ "distanceto",		(getattrofunc)wpChar_distanceto,		METH_VARARGS, "Distance to another object or a given position." },
	{ "action",			(getattrofunc)wpChar_action,			METH_VARARGS, "Lets the char perform an action." },
	{ "directionto",	(getattrofunc)wpChar_directionto,		METH_VARARGS, "Distance to another object or a given position." },
	{ "checkskill",		(getattrofunc)wpChar_checkskill,		METH_VARARGS, "Performs a skillcheck for the character." },
	{ "itemonlayer",	(getattrofunc)wpChar_itemonlayer,		METH_VARARGS, "Returns the item currently weared on a specific layer, or returns none." },
	{ "combatskill",	(getattrofunc)wpChar_combatskill,		METH_VARARGS, "Returns the combat skill the character would currently use." },
	{ "getweapon",		(getattrofunc)wpChar_getweapon,			METH_VARARGS, "What weapon does the character currently wear." },
	{ "useresource",	(getattrofunc)wpChar_useresource,		METH_VARARGS, "Consumes a resource posessed by the char." },
	{ "countresource",	(getattrofunc)wpChar_countresource,		METH_VARARGS, "Counts the amount of a certain resource the user has." },
	{ "emote",			(getattrofunc)wpChar_emote,				METH_VARARGS, "Shows an emote above the character." },
	{ "say",			(getattrofunc)wpChar_say,				METH_VARARGS, "The character begins to talk." },
	{ "turnto",			(getattrofunc)wpChar_turnto,			METH_VARARGS, "Turns towards a specific object and resends if neccesary." },
	{ "equip",			(getattrofunc)wpChar_equip,				METH_VARARGS, "Equips a given item on this character." },
	{ "maywalk",		(getattrofunc)wpChar_maywalk,			METH_VARARGS, "Checks if this character may walk to a specific cell." },
	{ "sound",			(getattrofunc)wpChar_sound,				METH_VARARGS, "Play a creature specific sound." },
	{ "disturb",		(getattrofunc)wpChar_disturb,			METH_VARARGS, "Disturbs whatever this character is doing right now." },
	{ "canreach",		(getattrofunc)wpChar_canreach,			METH_VARARGS, "Checks if this character can reach a certain object." },
	{ "canpickup",		(getattrofunc)wpChar_canpickup,			METH_VARARGS, NULL },
	{ "cansee",			(getattrofunc)wpChar_cansee,			METH_VARARGS, NULL },
	{ "lightning",		(getattrofunc)wpChar_lightning,			METH_VARARGS, NULL },
	{ "additem",		(getattrofunc)wpChar_additem,			METH_VARARGS, "Creating item on specified layer."},
	{ "isdead",			(getattrofunc)wpChar_isdead,			METH_VARARGS, "Checks if the character is alive or not."},
	
	// Mostly NPC functions
	{ "attack",			(getattrofunc)wpChar_attack,			METH_VARARGS, "Let's the character attack someone else." },
	{ "goto",			(getattrofunc)wpChar_goto,				METH_VARARGS, "The character should go to a coordinate." },
	{ "follow",			(getattrofunc)wpChar_follow,			METH_VARARGS, "The character should follow someone else." },

	{ "addtimer",		(getattrofunc)wpChar_addtimer,			METH_VARARGS, "Adds a timer to this character." },
	{ "dispel",			(getattrofunc)wpChar_dispel,			METH_VARARGS, "Dispels this character (with special options)." },

	// Update Stats
	{ "updatestats",	(getattrofunc)wpChar_updatestats,		METH_VARARGS, "Resends other stats to this character." },
	{ "updatemana",		(getattrofunc)wpChar_updatemana,		METH_VARARGS, "Resends the manabar to this character." },
	{ "updatestamina",	(getattrofunc)wpChar_updatestamina,		METH_VARARGS, "Resends the stamina bar to this character." },
	{ "updatehealth",	(getattrofunc)wpChar_updatehealth,		METH_VARARGS, "Resends the healthbar to the environment." },

	// Mount/Unmount
	{ "unmount",		(getattrofunc)wpChar_unmount,			METH_VARARGS, "Unmounts this character and returns the character it was previously mounted." },
	{ "mount",			(getattrofunc)wpChar_mount,				METH_VARARGS, "Mounts this on a specific mount." },

	// Effects
	{ "movingeffect",	(getattrofunc)wpChar_movingeffect,		METH_VARARGS, "Shows a moving effect moving toward a given object or coordinate." },
	{ "effect",			(getattrofunc)wpChar_effect,			METH_VARARGS, "Shows an effect staying with this character." },

	// Bank/Backpack
	{ "getbankbox",		(getattrofunc)wpChar_getbankbox,		METH_VARARGS,	"Gets and autocreates a bankbox for the character." },
	{ "getbackpack",	(getattrofunc)wpChar_getbackpack,		METH_VARARGS, "Gets and autocreates a backpack for the character." },

	// Follower System
	{ "addfollower",	(getattrofunc)wpChar_addfollower,		METH_VARARGS, "Adds a follower to the user." },
	{ "removefollower",	(getattrofunc)wpChar_removefollower,	METH_VARARGS, "Removes a follower from the user." },
	{ "hasfollower",	(getattrofunc)wpChar_hasfollower,		METH_VARARGS, "Checks if a certain character is a follower of this." },

	// Tag System
	{ "gettag",			(getattrofunc)wpChar_gettag,			METH_VARARGS, "Gets a tag assigned to a specific char." },
	{ "settag",			(getattrofunc)wpChar_settag,			METH_VARARGS, "Sets a tag assigned to a specific char." },
	{ "hastag",			(getattrofunc)wpChar_hastag,			METH_VARARGS, "Checks if a certain char has the specified tag." },
	{ "deltag",			(getattrofunc)wpChar_deltag,			METH_VARARGS, "Deletes the specified tag." },

	// Crafting Menu
	{ "sendmakemenu",	(getattrofunc)wpChar_sendmakemenu,		METH_VARARGS, "Sends MakeMenu to this character." },

	// Reputation System
	{ "iscriminal",		(getattrofunc)wpChar_iscriminal,		METH_VARARGS, "Is this character criminal.." },
	{ "ismurderer",		(getattrofunc)wpChar_ismurderer,		METH_VARARGS, "Is this character a murderer." },
	{ "criminal",		(getattrofunc)wpChar_criminal,			METH_VARARGS, "Make this character criminal." },

	// Is*? Functions
	{ "isitem",			(getattrofunc)wpChar_isitem,			METH_VARARGS, "Is this an item." },
	{ "ischar",			(getattrofunc)wpChar_ischar,			METH_VARARGS, "Is this a char." },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpChar_getAttr( wpChar *self, char *name )
{
	// Python specific stuff
	if ( !strcmp( "gm", name ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
	
		if (!player)
			return PyFalse;

		return player->isGM() ? PyTrue : PyFalse;
	} else if( !strcmp("rank", name)) {
		P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
	
		if (!player)
			return PyInt_FromLong(1);

		cAccount *account = player->account();

		if (account) {
			return PyInt_FromLong(account->rank());
		} else {
			return PyInt_FromLong(1);
		}
	} else if( !strcmp( "region", name ) )
		return PyGetRegionObject( self->pChar->region() );

	else if( !strcmp( "account", name ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );
		if ( !player )
		{
			Py_INCREF( Py_None );
			return Py_None;
		}
		return PyGetAccountObject( player->account() );
	}
	else if( !strcmp( "socket", name ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

		if ( !player )
		{
			Py_INCREF( Py_None );
			return Py_None;
		}
		return PyGetSocketObject( player->socket() );
	}
	else if( !strcmp( "skill", name ) )
	{
		wpSkills *skills = PyObject_New( wpSkills, &wpSkillsType );
		skills->pChar = self->pChar;
		skills->type = 0;
		return (PyObject*)( skills );
	}

	else if( !strcmp( "skillcap", name ) )
	{
		wpSkills *skills = PyObject_New( wpSkills, &wpSkillsType );
		skills->pChar = self->pChar;
		skills->type = 1;
		return (PyObject*)( skills );
	}

	else if( !strcmp( "skilllock", name ) )
	{
		wpSkills *skills = PyObject_New( wpSkills, &wpSkillsType );
		skills->pChar = self->pChar;
		skills->type = 2;
		return (PyObject*)( skills );
	}

	else if( !strcmp( "followers", name ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( self->pChar );

		if ( !player )
		{
			Py_INCREF( Py_None );
			return Py_None;
		}

		cBaseChar::CharContainer followers = player->pets();
		PyObject *rVal = PyTuple_New( followers.size() );

		for( uint i = 0; i < followers.size(); ++i )
			PyTuple_SetItem( rVal, i, PyGetCharObject( followers[i] ) );

		return rVal;
	}
	else if( !strcmp( "guards", name ) )
	{
		cBaseChar::CharContainer guards = self->pChar->guardedby();
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

			cPythonScript *script = ScriptManager::instance()->find( PyString_AsString( PyList_GetItem( value, i ) ) );
			if( script )
				self->pChar->addEvent( script );
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
