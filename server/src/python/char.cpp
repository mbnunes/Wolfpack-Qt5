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

#include "utilities.h"
#include "skills.h"
#include "content.h"
#include "../chars.h"
#include "../territories.h"
#include "../prototypes.h"
#include "../junk.h"
#include "../classes.h"
#include "../skills.h"
#include "../combat.h"

extern cCharStuff *Npcs;

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
    PyObject_HEAD_INIT(NULL)
    0,
    "WPChar",
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

	self->pChar->removeFromView( false );

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

	bool success = Skills->CheckSkill( self->pChar, skillId, min, max );

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

//	self->pChar->hp -= getArgInt( 0 );
	tempshort = self->pChar->hp();
	self->pChar->setHp( tempshort - getArgInt( 0 ) );

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

static PyMethodDef wpCharMethods[] = 
{
	{ "moveto",			(getattrofunc)wpChar_moveto, METH_VARARGS, "Moves the character to the specified location." },
	{ "resurrect",		(getattrofunc)wpChar_resurrect, METH_VARARGS, "Resurrects the character." },
	{ "kill",			(getattrofunc)wpChar_kill, METH_VARARGS, "This kills the character." },
	{ "damage",			(getattrofunc)wpChar_damage, METH_VARARGS, "This damages the current character." },
    { "update",			(getattrofunc)wpChar_update, METH_VARARGS, "Sends the char to all clients in range." },
	{ "removefromview", (getattrofunc)wpChar_removefromview, METH_VARARGS, "Removes the char from all surrounding clients." },
	{ "message",		(getattrofunc)wpChar_message, METH_VARARGS, "Displays a message above the characters head - only visible for the player." },
	{ "soundeffect",	(getattrofunc)wpChar_soundeffect, METH_VARARGS, "Plays a soundeffect for the character." },
	{ "distanceto",		(getattrofunc)wpChar_distanceto, METH_VARARGS, "Distance to another object or a given position." },
	{ "action",			(getattrofunc)wpChar_action, METH_VARARGS, "Lets the char perform an action." },
	{ "directionto",	(getattrofunc)wpChar_directionto, METH_VARARGS, "Distance to another object or a given position." },
	{ "checkskill",		(getattrofunc)wpChar_checkskill, METH_VARARGS, "Performs a skillcheck for the character." },
	{ "itemonlayer",	(getattrofunc)wpChar_itemonlayer, METH_VARARGS, "Returns the item currently weared on a specific layer, or returns none." },
	{ "combatskill",	(getattrofunc)wpChar_combatskill, METH_VARARGS, "Returns the combat skill the character would currently use." },
	{ "useresource",	(getattrofunc)wpChar_useresource, METH_VARARGS, "Consumes a resource posessed by the char." },
	{ "countresource",	(getattrofunc)wpChar_countresource, METH_VARARGS, "Counts the amount of a certain resource the user has." },
	{ "emote",			(getattrofunc)wpChar_emote, METH_VARARGS, "Let's the user emote." },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpChar_getAttr( wpChar *self, char *name )
{
	pGetStr( "name", name.c_str() )
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
//	else pGetInt( "int", in )
	else if( !strcmp( name, "int" ) )
		return PyInt_FromLong( self->pChar->in() );

	else if ( !strcmp( name, "str2" ) )
		return PyInt_FromLong( self->pChar->st2() );
	else pGetInt( "dex2", decDex() )
//	else pGetInt( "int2", in2 )
	else if( !strcmp( name, "int2" ) )
		return PyInt_FromLong( self->pChar->in2() );

//	else pGetInt( "health", hp )
	else if ( !strcmp( name, "health" ) )
		return PyInt_FromLong( self->pChar->hp() );
//	else pGetInt( "stamina", stm )
	else if ( !strcmp( name, "stamina" ) )
		return PyInt_FromLong( self->pChar->stm() );
//	else pGetInt( "mana", mn )
	else if ( !strcmp( name, "mana" ) )
		return PyInt_FromLong( self->pChar->mn() );

//	else pGetInt( "hidamage", hidamage )
	else if ( !strcmp( name, "hidamage" ) )
		return PyInt_FromLong(self->pChar->hidamage() );
//	else pGetInt( "lodamage", lodamage )
	else if ( !strcmp( name, "lodamage" ) )
		return PyInt_FromLong( self->pChar->lodamage() );

//	else pGetInt( "npc", npc )
	else if ( !strcmp( name, "npc" ) )
		return PyInt_FromLong( self->pChar->npc() );
//	else pGetInt( "shop", shop )
	else if ( !strcmp( name, "shop" ) )
		return PyInt_FromLong( self->pChar->shop() );
//	else pGetInt( "cell", cell )
	else if ( !strcmp( name, "cell" ) )
		return PyInt_FromLong( self->pChar->cell() );
	
	// Owner
	else if( !strcmp( name, "owner" ) )
		return PyGetCharObject( FindCharBySerial( self->pChar->ownserial() ) );

//	else pGetInt( "karma", karma )
	else if( !strcmp( name, "karma" ) )
		return PyInt_FromLong( self->pChar->karma() );
//	else pGetInt( "fame", fame )
	else if( !strcmp( name, "fame" ) )
		return PyInt_FromLong( self->pChar->fame() );
//	else pGetInt( "kills", kills )
	else if( !strcmp( name, "kills" ) )
		return PyInt_FromLong( self->pChar->kills() );
//	else pGetInt( "deaths", deaths )
	else if( !strcmp( name, "deaths" ) )
		return PyInt_FromLong( self->pChar->deaths() );
//	else pGetInt( "dead", dead )
	else if( !strcmp( name, "dead" ) )
		return PyInt_FromLong( self->pChar->dead() );

	else if( !strcmp( name, "backpack" ) )
		return PyGetItemObject( self->pChar->getBackpack() );

//	else pGetInt( "def", def )
	else if( !strcmp( name, "def" ) )
		return PyInt_FromLong( self->pChar->def() );
//	else pGetInt( "war", war )
	else if( !strcmp( name, "war" ) )
		return PyInt_FromLong( self->pChar->war() );
	
	// Target
	else if( !strcmp( "target", name ) )
	{
		if( isItemSerial( self->pChar->targ ) )
			return PyGetItemObject( FindItemBySerial( self->pChar->targ ) );
		else if( isCharSerial( self->pChar->targ ) )
			return PyGetCharObject( FindCharBySerial( self->pChar->targ ) );
	}

	else pGetInt( "npcwander", npcWander )
	else pGetInt( "oldnpcwander", oldnpcWander )
	
	// Region object
	else if( !strcmp( "region", name ) )
		return PyGetRegionObject( self->pChar->region );

	else pGetInt( "skilldelay", skilldelay )
	else pGetInt( "objectdelay", objectdelay )
	else pGetInt( "taming", taming )
	else pGetInt( "summontimer", summontimer )
	else pGetInt( "visrange", VisRange )
	
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

	/*getStrProperty( "name", pChar->name.c_str() )
	else getStrProperty( "orgname", pChar->orgname().latin1() )
	else getStrProperty( "title", pChar->title().latin1() )
	else getIntProperty( "serial", pChar->serial )
	else getIntProperty( "body", pChar->id() )
	else getIntProperty( "xbody", pChar->xid )
	else getIntProperty( "skin", pChar->skin() )
	else getIntProperty( "xskin", pChar->xskin() )
	
	else getIntProperty( "health", pChar->hp )
	else getIntProperty( "stamina", pChar->stm )
	else getIntProperty( "mana", pChar->mn )

	else getIntProperty( "str", pChar->st )
	else getIntProperty( "dex", pChar->effDex() )
	else getIntProperty( "int", pChar->in )

	else if( !strcmp( "pos", name ) )
		return PyGetCoordObject( self->pChar->pos );

	else getIntProperty( "direction", pChar->dir )
	else getIntProperty( "flags2", pChar->priv2 )
	else getIntProperty( "hidamage", pChar->hidamage )
	else getIntProperty( "lodamage", pChar->lodamage )
	else getIntProperty( "objectdelay", pChar->objectdelay )

	else if( !strcmp( "equipment", name ) )
	{
		wpContent *content = PyObject_New( wpContent, &wpContentType );
		content->contserial = self->pChar->serial;
		return (PyObject*)content;
	}
	else if( !strcmp( "skill", name ) )
	{
		wpSkills *skills = PyObject_New( wpSkills, &wpSkillsType );
		skills->base = false;
		skills->pChar = self->pChar;
		return (PyObject*)( skills );
	}

	else if( !strcmp( "socket", name ) )
		return PyGetSocketObject( self->pChar->socket() );

	else if( !strcmp( "equipment", name ) )
	{
		Py_WPEquipment *returnVal = PyObject_New( Py_WPEquipment, &Py_WPEquipmentType );
		returnVal->pChar = self->pChar; // Never forget that
		return (PyObject*)returnVal;
		return Py_None;
	}

	// Base skill
	else for( UINT8 i = 0; i < ALLSKILLS; ++i )
		if( field.upper() == skillname[ i ] ) 
			return PyInt_FromLong( self->pChar->baseSkill( i ) );*/

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
	
//	else setIntProperty( "xbody", pChar->xid() )
	else if ( !strcmp("xbody", name ) )
		self->pChar->setXid(PyInt_AS_LONG( value ) );		
	else if ( !strcmp( "skin", name ) )
		self->pChar->setSkin( PyInt_AS_LONG(value ) );
	else if ( !strcmp( "xskin", name ) )
		self->pChar->setXSkin( PyInt_AS_LONG(value ) );
	
//	else setIntProperty( "health", pChar->hp )
	else if ( !strcmp( "health", name ) )
		self->pChar->setHp( PyInt_AS_LONG( value ) );
		
//	else setIntProperty( "stamina", pChar->stm() )
	else if ( !strcmp( "stamina", name ) )
		self->pChar->setStm( PyInt_AS_LONG( value ) );
//	else setIntProperty( "mana", pChar->mn() )
	else if ( !strcmp( "mana", name ) )
		self->pChar->setMn( PyInt_AS_LONG( value ) );

	else if ( !strcmp( "str", name ) )
		self->pChar->setSt( PyInt_AS_LONG( value ) );
	else if ( !strcmp( "dex", name ) )
		self->pChar->setDex( PyInt_AS_LONG( value ) );
	
//	else setIntProperty( "int", pChar->in() )
	else if ( !strcmp( "int", name ) )
		self->pChar->setIn( PyInt_AS_LONG( value ) );
//	else setIntProperty( "direction", pChar->dir() )
	else if( !strcmp("direction", name ) )
		self->pChar->setDir( PyInt_AS_LONG( value ) );
//	else setIntProperty( "flags2", pChar->priv2() )
	else if( !strcmp("flags2", name ) )
		self->pChar->setPriv2( PyInt_AS_LONG( value ) );
//	else setIntProperty( "hidamage", pChar->hidamage )
	else if( !strcmp("hidamage", name ) )
		self->pChar->setHiDamage( PyInt_AS_LONG( value ) );
//	else setIntProperty( "lodamage", pChar->lodamage )
	else if( !strcmp("lodamage", name ) )
		self->pChar->setLoDamage( PyInt_AS_LONG( value ) );
	else setIntProperty( "objectdelay", pChar->objectdelay )
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
