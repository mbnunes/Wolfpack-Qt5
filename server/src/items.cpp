/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "timing.h"
#include "accounts.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "items.h"
#include "muls/tilecache.h"
#include "serverconfig.h"
#include "definitions.h"
#include "pythonscript.h"
#include "muls/maps.h"
#include "network/network.h"
#include "log.h"
#include "persistentbroker.h"
#include "multi.h"
#include "guilds.h"
#include "dbdriver.h"
#include "world.h"
#include "mapobjects.h"
#include "scriptmanager.h"
#include "basechar.h"
#include "player.h"
#include "basics.h"

#include "inlines.h"
#include "console.h"

// System Includes
#include <math.h>
#include <algorithm>
#include <QByteArray>
#include <QList>
#include <QSqlQuery>
#include <QVariant>

using namespace std;

/*****************************************************************************
 * cItem member functions
 *****************************************************************************/

// constructor
cItem::cItem() : totalweight_( 0 ), container_( 0 )
{
	basedef_ = 0;
	Init( false );
};

cItem::cItem( const cItem& src ) : cUObject( src ), totalweight_( 0 ), container_( 0 )
{
	Init( false );
	// Copy Events
	scriptChain = 0;
	setScriptList( src.scriptList() );
	this->name_ = src.name_;
	this->tags_ = src.tags_;
	//cItem properties setting
	this->serial_ = INVALID_SERIAL; // IMPORTANT
	this->amount_ = src.amount_;
	this->changed( TOOLTIP );
	this->color_ = src.color_;
	this->flagChanged();
	this->free = false;
	this->hp_ = src.hp_;
	this->movable_ = src.movable_;
	this->maxhp_ = src.maxhp_;
	this->priv_ = src.priv_;
	this->setId( src.id() );
	this->setOwnSerialOnly( src.ownSerial() );
	this->visible_ = src.visible_;
	this->basedef_ = src.basedef_;
	this->totalweight_ = amount_ * weight();
	this->multi_ = 0;
	this->isPersistent = false;
	pos_ = src.pos_; // Copy position
	pos_.setInternalMap(); // Make absolutly sure that we're not flagged as being in the sector maps yet
}

// static definitions
QSqlQuery * cItem::insertQuery_ = NULL;
QSqlQuery * cItem::updateQuery_ = NULL;

P_CHAR cItem::owner( void ) const
{
	return FindCharBySerial( ownserial_ );
}

void cItem::setOwner( P_CHAR nOwner )
{
	flagChanged();
	ownserial_ = ( nOwner == NULL ) ? INVALID_SERIAL : nOwner->serial();
}

// Is the Item pileable?
bool cItem::isPileable()
{
	tile_st tile = TileCache::instance()->getTile( id_ );
	return tile.flag2 & 0x08;
}

void cItem::toBackpack( P_CHAR pChar )
{
	flagChanged();
	P_ITEM pPack = pChar->getBackpack();

	// Pack it to the ground
	if ( !pPack )
	{
		removeFromCont();
		moveTo( pChar->pos() );
	}
	// Or to the backpack
	else
	{
		pPack->addItem( this );
	}

	update();
}

/*
 * Name: ReduceAmount
 * History: by Duke, 4.06.2000
 * 		added P_ITEM interface Duke, 3.10.2000
 * 		made it the first member of cItem Duke, 23.12.2000
 * Purpose: reduces the given item's amount by 'amt' and deletes it if
 * 		necessary and returns 0. If the request could not be fully satisfied,
 * 		the remainder is returned
 */
long cItem::reduceAmount( unsigned int amt )
{
	quint32 rest = 0;
	if ( amount_ > amt )
	{
		setAmount( amount_ - amt );
		update();
		changed( TOOLTIP );
		flagChanged();
	}
	else
	{
		rest = amt - amount_;
		this->remove();
	}
	return rest;
}

void cItem::setOwnSerialOnly( int ownser )
{
	flagChanged();
	ownserial_ = ownser;
}

void cItem::SetOwnSerial( int ownser )
{
	flagChanged();
	setOwnSerialOnly( ownser );
}

/*
 * Author: LB purpose: returns the type of pack
 * to handle its x,y coord system corretly.
 * interpretation of the result:
 * valid x,y ranges depending on type:
 * type -1 : no pack
 * type  1 : y-range 50  .. 100
 * type  2 : y-range 30  .. 80
 * type  3 : y-range 100 .. 150
 * type  4 : y-range 40  .. 140
 * x-range 18 .. 118 for 1,2,3
 *  	   40 .. 140 for 4
 */
short cItem::containerGumpType() const
{
	switch ( id() )
	{
	case 0x09a8:
	case 0x09aa:
	case 0x09b0:
	case 0x0A2C:
		// chest of drawers
	case 0x0A30:
		// chest of drawers
	case 0x0A34:
		// chest of drawers
	case 0x0A38:
		// chest of drawers
	case 0x0A4D:
		// armoire
	case 0x0A4F:
		// armoire
	case 0x0A51:
		// armoire
	case 0x0A53:
		// armoire
	case 0x0A97:
		// bookshelf
	case 0x0A98:
		// bookshelf
	case 0x0A99:
		// bookshelf
	case 0x0A9A:
		// bookshelf
	case 0x0A9B:
		// bookshelf
	case 0x0A9C:
		// bookshelf
	case 0x0A9D:
		// bookshelf
	case 0x0A9E:
		// bookshelf
	case 0x0e76:
	case 0x0e79:
	case 0x0e7a:
	case 0x0e7d:
	case 0x0e80:
		return 1;

	case 0x09a9:
	case 0x0e3c:
	case 0x0e3d:
	case 0x0e3e:
	case 0x0e3f:
	case 0x0e78:
	case 0x0e7e:
		return 2;

	case 0x09ab:
	case 0x0e40:
	case 0x0e41:
	case 0x0e42:
	case 0x0e43:
	case 0x0e7c:
		return 3;

	case 0x09b2:
	case 0x0e75:
	case 0x0e77:
	case 0x0e7f:
	case 0x0e83:
		return 4;

	case 0x2006:
		return 5; // a corpse/coffin

	default:
		return -1;
	}
}

bool cItem::pileItem( cItem* pItem )
{
	if ( !canStack( pItem ) )
		return false;

	if ( amount() + pItem->amount() > 65535 )
	{
		pItem->setAmount( ( amount() + pItem->amount() ) - 65535 );
		setAmount( 65535 );
		update();
		return false; // Old item still there
	}
	else
	{
		setAmount( this->amount() + pItem->amount() );
		pItem->remove();
		update();
		return true; // Stacked
	}
}

/*!
	Tries to find an item in the container to stack with
*/
bool cItem::containerPileItem( cItem* pItem )
{
	for ( ContainerIterator it( this ); !it.atEnd(); ++it )
	{
		if ( ( *it )->pileItem( pItem ) )
			return true;
	}
	return false;
}

void cItem::setRandPosInCont( cItem* pCont )
{
	int k = pCont->containerGumpType();
	Coord position = pos();
	position.x = RandomNum( 18, 118 );
	position.z = 9;

	switch ( k )
	{
	case 1:
		position.y = RandomNum( 50, 100 );
		break;
	case 2:
		position.y = RandomNum( 30, 80 );
		break;
	case 3:
		position.y = RandomNum( 100, 140 );
		break;
	case 4:
		position.y = RandomNum( 60, 140 );
		position.x = RandomNum( 60, 140 );
		break;
	case 5:
		position.y = RandomNum( 85, 160 );
		position.x = RandomNum( 20, 70 );
		break;
	default:
		position.y = RandomNum( 30, 80 );
	}
	moveTo( position );
}

/*!
	Recurses through the container given by serial and deletes items of
	the given id and color(if given) until the given amount is reached
*/
int cItem::deleteAmount( int amount, unsigned short _id, unsigned short _color )
{
	unsigned int rest = amount;
	P_ITEM pi;
	for ( ContainerCopyIterator it( this ); !it.atEnd(); ++it )
	{
		pi = *it;
		if ( pi->type() == 1 )
			rest = pi->deleteAmount( rest, _id, _color );
		if ( pi->id() == _id && ( _color == 0 || ( pi->color() == _color ) ) )
			rest = pi->reduceAmount( rest );
		if ( rest <= 0 )
			break;
	}
	return rest;
}

void cItem::save( cBufferedWriter& writer, unsigned int version )
{
	if ( free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Skipping item 0x%1 during save process because it's already freed.\n" ).arg( serial_, 0, 16 ) );
	}
	else if ( container_ && container_->free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Skipping item 0x%1 during save process because it's in a freed container.\n" ).arg( serial_, 0, 16 ) );
	}
	else
	{

		cUObject::save( writer, version );

		writer.writeShort( id_ );
		writer.writeShort( color_ );
		writer.writeInt( container_ ? container_->serial() : INVALID_SERIAL );
		writer.writeByte( layer_ );
		writer.writeShort( amount_ );
		writer.writeShort( hp_ );
		writer.writeShort( maxhp_ );
		writer.writeByte( movable_ );
		writer.writeInt( ownserial_ );
		writer.writeByte( visible_ );
		writer.writeByte( priv_ );
		writer.writeAscii( baseid() );
	}
}

void cItem::postload( unsigned int /*version*/ )
{
	if ( container_ )
	{
		pos_.setInternalMap();
	}

	if ( !container_ && !pos_.isInternalMap() )
	{
		MapObjects::instance()->add( this );
	}
}

void cItem::load( cBufferedReader& reader, unsigned int version )
{
	cUObject::load( reader, version );

	id_ = reader.readShort();
	color_ = reader.readShort();
	// Here we assume that containers are always before us in the save
	SERIAL containerSerial = reader.readInt();
	cUObject* container = World::instance()->findObject( containerSerial );
	layer_ = reader.readByte();
	amount_ = reader.readShort();
	hp_ = reader.readShort();
	maxhp_ = reader.readShort();
	movable_ = reader.readByte();
	ownserial_ = reader.readInt();
	visible_ = reader.readByte();
	priv_ = reader.readByte();
	basedef_ = ItemBaseDefs::instance()->get( reader.readAscii() );
	totalweight_ = amount_ * weight();

	// Add to container and handle weight
	if ( container )
	{
		P_ITEM iContainer = dynamic_cast<P_ITEM>( container );
		if ( iContainer )
		{
			iContainer->addItem( this, false, true, false );
		}
		else
		{
			P_CHAR cContainer = dynamic_cast<P_CHAR>( container );
			if ( cContainer )
			{
				cContainer->addItem( ( cBaseChar::enLayer ) layer(), this, true, true );
			}
		}
	}
	else
	{
		if ( containerSerial != INVALID_SERIAL )
		{
			// Indicate an error
			reader.setError( tr( "Deleting item 0x%1 because of invalid container 0x%2.\n" ).arg( serial_, 0, 16 ).arg( containerSerial, 0, 16 ) );
		}
	}
}

void cItem::save()
{
	if ( changed_ )
	{
		QSqlQuery * q;
		if ( isPersistent )
			q = cItem::getUpdateQuery();
		else
			q = cItem::getInsertQuery();

		q->addBindValue( serial() );
		q->addBindValue( id() );
		q->addBindValue( color() );
		if ( container_ )
			q->addBindValue( container_->serial() );
		else
			q->addBindValue( INVALID_SERIAL );
		q->addBindValue( layer_ );
		q->addBindValue( amount_ );
		q->addBindValue( hp_ );
		q->addBindValue( maxhp_ );
		q->addBindValue( movable_ );
		q->addBindValue( ownserial_ );
		q->addBindValue( visible_ );
		q->addBindValue( priv_ );
		q->addBindValue( QString( baseid() ) );

		if ( isPersistent )
			q->addBindValue( serial() );

		q->exec();
	}
	cUObject::save();
}

bool cItem::del()
{
	if ( !isPersistent )
		return false; // We didn't need to delete the object

	PersistentBroker::instance()->addToDeleteQueue( "items", QString( "serial = '%1'" ).arg( serial() ) );
	flagChanged();
	return cUObject::del();
}

QString cItem::getName( bool shortName )
{
	if ( !name_.isEmpty() && !name_.startsWith( "#" ) )
		return name_;

	tile_st tile = TileCache::instance()->getTile( id_ );

	QString itemname = tile.name;

	// NEVER prepend a or at when shortName is true (You destroy your a shirt sounds stupid doesnt it)
	if ( !shortName )
	{
		if ( tile.flag2 & 0x80 )
			itemname.prepend( "an " );
		else
			itemname.prepend( "a " );
	}

	// Now "parse" the %/% information
	if ( itemname.contains( "%" ) )
	{
		// Test if we have a %.../...% form or a simple %
		QRegExp simple( "%([^/]+)%" );
		simple.setMinimal( TRUE );

		if ( itemname.contains( simple ) )
			itemname.replace( simple, simple.cap( 1 ) );
	}

	return itemname;
}

void cItem::setSerial( const SERIAL ser )
{
	if ( ser == INVALID_SERIAL || ser == serial_ )
		return;

	// is the new serial already occupied?
	P_ITEM other = World::instance()->findItem( ser );
	if ( other && other != this )
	{
		Console::instance()->log( LOG_ERROR, tr( "Trying to change the serial of item 0x%1 to the already occupied serial 0x%2.\n" ).arg( serial_, 0, 16 ).arg( ser, 0, 16 ) );
		return;
	}

	if ( serial() != INVALID_SERIAL )
		World::instance()->unregisterObject( this );

	cUObject::setSerial( ser );

	World::instance()->registerObject( this );
}

// -- Initialize an Item in the items array
void cItem::Init( bool createSerial )
{
	changed( TOOLTIP );
	flagChanged();

	cUObject::setSerial( INVALID_SERIAL );

	if ( createSerial )
		this->setSerial( World::instance()->findItemSerial() );

	this->container_ = 0;
	this->free = false;
	this->setId( 0x0001 ); // Item visuals as stored in the client
	this->pos_.setInternalMap();
	this->color_ = 0x00; // Hue
	this->layer_ = 0; // Layer if equipped on paperdoll
	this->amount_ = 1; // Amount of items in pile
	this->hp_ = 0; //Number of hit points an item has.
	this->maxhp_ = 0; // Max number of hit points an item can have.
	this->movable_ = 0; // 0=Default as stored in client, 1=Always movable, 2=Owner movable, 3=Never movable.
	this->setOwnSerialOnly( -1 );
	this->visible_ = 0; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	this->priv_ = 0; // Bit 0, nodecay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
}

/*!
	Removes this item from the game world.
*/
void cItem::remove()
{
	// Already Deleted
	if ( free ) {
		return;
	}

	if ( canHandleEvent( EVENT_DELETE ) )
	{
		PyObject* args = Py_BuildValue( "(N)", getPyObject() );
		callEventHandler( EVENT_DELETE, args );
		Py_DECREF( args );
	}

	removeFromView( false ); // Remove it from all clients in range
	free = true;

	clearScripts();
	SetOwnSerial( -1 );

	// Check if this item is registered as a guildstone and remove it from the guild
	for ( cGuilds::iterator it = Guilds::instance()->begin(); it != Guilds::instance()->end(); ++it )
	{
		cGuild* guild = it.value();
		if ( guild->guildstone() == this )
			guild->setGuildstone( 0 );
	}

	// Remove from the sector map if its a world item
	// Otherwise check if there is a top container
	if ( container() && !container()->free )
	{
		removeFromCont();
	}
	else
	{
		// Remove us from a possilbe multi container too
		if ( multi_ )
		{
			multi_->removeObject( this );
			multi_ = 0;
		}
	}

	// Create a copy of the content so we don't accidently change our working copy
	for ( ContainerCopyIterator it2( this ); !it2.atEnd(); ++it2 )
		( *it2 )->remove();

	cUObject::remove();
}

bool cItem::onSingleClick( P_PLAYER Viewer )
{
	bool result = false;
	if ( canHandleEvent( EVENT_SINGLECLICK ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetItemObject, this, PyGetCharObject, Viewer );
		result = callEventHandler( EVENT_SINGLECLICK, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onDropOnItem( P_ITEM pItem )
{
	bool result = false;
	if ( canHandleEvent( EVENT_DROPONITEM ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetItemObject, layer_ == 0x1E ? pItem : this, PyGetItemObject, layer_ == 0x1E ? this : pItem );
		result = callEventHandler( EVENT_DROPONITEM, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onDropOnGround( const Coord& pos )
{
	bool result = false;
	if ( canHandleEvent( EVENT_DROPONGROUND ) )
	{
		PyObject* args = Py_BuildValue( "O&N", PyGetItemObject, this, PyGetCoordObject( pos ) );
		result = callEventHandler( EVENT_DROPONGROUND, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onPickup( P_CHAR pChar )
{
	bool result = false;

	if ( canHandleEvent( EVENT_PICKUP ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler( EVENT_PICKUP, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onPickupFromContainer( P_CHAR pChar, P_ITEM pItem )
{
	bool result = false;

	if ( canHandleEvent( EVENT_PICKUPFROMCONTAINER ) )
	{
		PyObject* args = Py_BuildValue( "O&O&O&", PyGetCharObject, pChar, PyGetItemObject, pItem, PyGetItemObject, this );
		result = callEventHandler( EVENT_PICKUPFROMCONTAINER, args );
		Py_DECREF( args );
	}

	return result;
}

int cItem::onBuy( P_CHAR pVendor, P_CHAR pChar, int amount )
{
	PyObject* args = Py_BuildValue( "O&O&O&i", PyGetItemObject, this, PyGetCharObject, pVendor, PyGetCharObject, pChar, amount );
	if ( canHandleEvent( EVENT_BUY ) )
	{
		PyObject *result = callEvent( EVENT_BUY, args );
		if ( result )
		{
			if ( PyInt_CheckExact( result ) )
			{
				amount = PyInt_AsLong( result );
			}
			else if ( PyLong_CheckExact( result ) )
			{
				amount = PyLong_AsLong( result );
			}
		}
		else
		{
			amount = 0;
		}
		Py_XDECREF( result );
	}

	Py_XDECREF( args );

	return amount;
}

bool cItem::onEquip( P_CHAR pChar, unsigned char layer )
{
	bool result = false;
	if ( canHandleEvent( EVENT_EQUIP ) )
	{
		PyObject* args = Py_BuildValue( "O&O&b", PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = callEventHandler( EVENT_EQUIP, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onUnequip( P_CHAR pChar, unsigned char layer )
{
	bool result = false;
	if ( canHandleEvent( EVENT_UNEQUIP ) )
	{
		PyObject* args = Py_BuildValue( "O&O&b", PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = callEventHandler( EVENT_UNEQUIP, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onWearItem( P_PLAYER pPlayer, P_CHAR pChar, unsigned char layer )
{
	bool result = false;
	if ( canHandleEvent( EVENT_WEARITEM ) )
	{
		PyObject* args = Py_BuildValue( "O&O&O&b", PyGetCharObject, pPlayer, PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = callEventHandler( EVENT_WEARITEM, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onUse( P_CHAR pChar )
{
	bool result = false;
	if ( canHandleEvent( EVENT_USE ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler( EVENT_USE, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onCollide( P_CHAR pChar )
{
	bool result = false;
	if ( canHandleEvent( EVENT_COLLIDE ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler( EVENT_COLLIDE, args );
		Py_DECREF( args );
	}
	return result;
}

bool cItem::onDropOnChar( P_CHAR pChar )
{
	bool result = false;
	if ( canHandleEvent( EVENT_DROPONCHAR ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler( EVENT_DROPONCHAR, args );
		Py_DECREF( args );
	}
	return result;
}

void cItem::processNode( const cElement* Tag, uint hash )
{
/*
#define OUTPUT_HASH(x) QString("%1 = %2\n").arg(x).arg( elfHash( x ), 0, 16)
	Console::instance()->send(
		OUTPUT_HASH("amount") + 
		OUTPUT_HASH("durability") + 
		OUTPUT_HASH("movable") + 
		OUTPUT_HASH("ownermovable") + 
		OUTPUT_HASH("immovable") + 
		OUTPUT_HASH("decay") + 
		OUTPUT_HASH("nodecay") + 
		OUTPUT_HASH("dispellable") + 
		OUTPUT_HASH("notdispellable") + 
		OUTPUT_HASH("newbie") + 
		OUTPUT_HASH("notnewbie") + 
		OUTPUT_HASH("twohanded") + 
		OUTPUT_HASH("singlehanded") + 
		OUTPUT_HASH("invisible") + 
		OUTPUT_HASH("visible") + 
		OUTPUT_HASH("ownervisible") + 
		OUTPUT_HASH("dye") + 
		OUTPUT_HASH("nodye") + 
		OUTPUT_HASH("content") + 
		OUTPUT_HASH("inherit")
		);
#undef OUTPUT_HASH
*/
	flagChanged();
	// we do this as we're going to modify the element
	QString Value = Tag->value();

	if ( !hash )
		hash = Tag->nameHash();

	switch ( hash )
	{
	case 0x6846c54: // amount
		// <amount>10</amount>
		this->setAmount( Value.toUShort() );
		break;

	case 0x78aac39: // durability
		// <durability>10</durabilty>
		this->setMaxhp( Value.toLong() );
		this->setHp( this->maxhp() );
		break;

	case 0x46c7955: // movable
		this->movable_ = 1;
		break;

	case 0x50235b5: // ownermovable
		this->movable_ = 2;
		break;

	case 0x46c0965: // immovable
		this->movable_ = 3;
		break;

	case 0x6ab989: // decay
		setNoDecay( false );
		break;

	case 0x55ab9f9: // nodecay
		setNoDecay( true );
		break;

	case 0xc597345: //dispellable
		this->priv_ |= 0x04;
		break;

	case 0xe997615: // notdispellable
		this->priv_ &= 0xFB;
		break;

	case 0x74cd8f5: // newbie
		this->setNewbie( true );
		break;

	case 0xb4cad95: // notnewbie
		this->setNewbie( false );
		break;

	case 0x5e83154: // twohanded
		this->setTwohanded( true );
		break;
		
	case 0xc480494: // singlehanded
		this->setTwohanded( false );
		break;

	case 0xd098975: // invisible
		this->visible_ = 2;
		break;

	case 0xd09f955: // visible
		this->visible_ = 0;
		break;

	case 0xc67b5b5: // ownervisible
		this->visible_ = 1;
		break;

	case 0x6bf5: // dye
		this->setDye( true );
		break;

	case 0x755bf5: // nodye
		this->setDye( false );
		break;

	case 0xa65ac34: // content
		// <content><item id="a" />...<item id="z" /></content> (sereg)
		this->processContainerNode( Tag );
		break;

	case 0x4ec974:
		{
			const cElement* section;

			if ( Tag->hasAttribute( "id" ) )
			{
				section = Definitions::instance()->getDefinition( WPDT_ITEM, Tag->getAttribute( "id" ) );
			}
			else
			{
				section = Definitions::instance()->getDefinition( WPDT_ITEM, Value );
			}

			if ( section )
				applyDefinition( section );
		}
		break;

	default:
		{
			const cElement* section = Definitions::instance()->getDefinition( WPDT_DEFINE, Tag->name() );
			if ( section )
				for ( unsigned int i = 0; i < section->childCount(); ++i )
					processModifierNode( section->getChild( i ) );
			else
				cUObject::processNode( Tag );
		}
		break;
	}
}

void cItem::processModifierNode( const cElement* Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->value();

	// <name>magic %1</name>
	if ( TagName == "name" )
	{
		// Bad: # = iron #
		if ( name_.isNull() )
		{
			name_ = getName( true );
		}

		// This prevents double naming issues (magic magic item)
		// magic %1 | magic item
		// This is rather tough i'd say, we have to check whether we already
		// have the prefix *OR* suffix
		if ( !Value.contains( "%1" ) )
		{
			name_ = Value;
		}
		else
		{
			/*
			** int offset = Value.find( "%1" );
			** QString left = Value.left( offset );
			** QString right = Value.right( Value.length() - ( offset + 2 ) );
			** name_ = left + name_ + right;
			*/
			name_ = Value.arg( name_ );
		}
	}
	else
		cUObject::processNode( Tag );
}

void cItem::processContainerNode( const cElement* tag )
{
	//item containers can be scripted like this:
	/*
	<contains>
		<item list="myList" />
		<item randomlist="myList1,myList2" />
		<item id="myItem1" />
		<item list="myList"><amount></amount></item>
		<item randomlist="myList1,myList2"><amount></amount></item>
		<item id="myItem1"><amount></amount></item></item>
	</contains>
	*/
	for ( unsigned int i = 0; i < tag->childCount(); ++i )
	{
		const cElement* element = tag->getChild( i );

		if ( element->name() == "item" )
		{
			if ( element->hasAttribute( "id" ) )
			{
				cItem* nItem = cItem::createFromScript( element->getAttribute( "id" ) );
				if ( nItem )
				{
					addItem( nItem );
					for ( unsigned int j = 0; j < element->childCount(); ++j )
						nItem->processNode( element->getChild( j ) );
					if ( this->layer() == cBaseChar::BuyRestockContainer )
						nItem->setRestock( nItem->amount() );
				}
			}
			else if ( element->hasAttribute( "list" ) )
			{
				cItem* nItem = cItem::createFromList( element->getAttribute( "list" ) );
				if ( nItem )
				{
					addItem( nItem );
					for ( unsigned int j = 0; j < element->childCount(); ++j )
						nItem->processNode( element->getChild( j ) );
					if ( this->layer() == cBaseChar::BuyRestockContainer )
						nItem->setRestock( nItem->amount() );
				}
			}
			else if ( element->hasAttribute( "randomlist" ) )
			{
				QStringList RandValues = element->getAttribute( "randomlist" ).split( "," );
				cItem* nItem = cItem::createFromList( RandValues[RandomNum( 0, RandValues.size() - 1 )] );
				if ( nItem )
				{
					addItem( nItem );
					for ( unsigned int j = 0; j < element->childCount(); ++j )
						nItem->processNode( element->getChild( j ) );
					if ( this->layer() == cBaseChar::BuyRestockContainer )
						nItem->setRestock( nItem->amount() );
				}
			}
			else
			{
				Console::instance()->log( LOG_ERROR, tr( "Content element lacking id, list, or randomlist attribute in item definition '%1'." ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
			}
		}
	}
}

void cItem::showName( cUOSocket* socket )
{
	if ( !onSingleClick( socket->player() ) )
	{
		unsigned int message;
		QString params = QString::null;

		if ( amount_ > 1 )
		{
			message = 1050039;
			if ( name_.isEmpty() ) {
				unsigned int clilocName = this->clilocName();
				if (clilocName != 0) {
					params = QString( "%1\t#%2" ).arg( amount_ ).arg( clilocName );
				} else {
					params = QString( "%1\t#%2" ).arg( amount_ ).arg( 1020000 + id_ );
				}
			} else {
				params = QString( "%1\t%2" ).arg( amount_ ).arg( name_ );
			}
		}
		else
		{
			if ( name_.isEmpty() ) {
				unsigned int clilocName = this->clilocName();
				if (clilocName != 0) {
					message = clilocName;
				} else {
					message = 1020000 + id_;
				}
			} else {
				message = 1042971;
				params = name_;
			}
		}

		socket->clilocMessage( message, params, 0x3b2, 3, this, true );
	}
}

// This either sends a ground-item or a backpack item
void cItem::update( cUOSocket* singlesocket )
{
	if ( free )
		return;

	if ( !singlesocket && Network::instance()->count() == 0 )
		return; // no one to receive updates.

	// Items on Ground
	if ( !container_ )
	{
		// we change the packet during iteration, so we have to
		// recompress it
		cUOTxSendItem sendItem;
		sendItem.setSerial( serial_ );
		sendItem.setId( id_ );
		sendItem.setAmount( amount_ );
		sendItem.setColor( color_ );
		sendItem.setCoord( pos_ );
		sendItem.setDirection( lightsource() );

		// Send to one person only
		if ( !singlesocket )
		{
			QList<cUOSocket*> sockets = Network::instance()->sockets();
			foreach ( cUOSocket* socket, sockets )
			{
				if ( socket->canSee( this ) )
				{
					P_PLAYER player = socket->player();
					unsigned char flags = 0;
					cUOTxSendItem packetCopy( sendItem );

					if ( socket->account()->isMultiGems() && isMulti() )
					{
						packetCopy.setId( 0x1ea7 );
					}

					// Always Movable Flag
					if ( !isLockedDown() && isAllMovable() )
					{
						flags |= 0x20;
					}
					else if ( player->account()->isAllMove() )
					{
						flags |= 0x20;
					}
					else if ( !isLockedDown() && isOwnerMovable() && player->owns( this ) )
					{
						flags |= 0x20;
					}

					if ( visible_ != 0 )
					{
						flags |= 0x80;
					}

					packetCopy.setFlags( flags );

					socket->send( &packetCopy );
					sendTooltip( socket );
				}
			}
		}
		else if ( singlesocket && singlesocket->canSee( this ) )
		{
			P_PLAYER player = singlesocket->player();
			unsigned char flags = 0;

			// Always Movable Flag
			if ( !isLockedDown() && isAllMovable() )
			{
				flags |= 0x20;
			}
			else if ( player->account()->isAllMove() )
			{
				flags |= 0x20;
			}
			else if ( !isLockedDown() && isOwnerMovable() && player->owns( this ) )
			{
				flags |= 0x20;
			}

			if ( visible_ != 0 )
			{
				flags |= 0x80;
			}

			if ( singlesocket->account()->isMultiGems() && isMulti() )
			{
				sendItem.setId( 0x1ea7 );
			}

			sendItem.setFlags( flags );

			singlesocket->send( &sendItem );
			sendTooltip( singlesocket );
		}
	}
	// equipped items
	else if ( container_ && container_->isChar() )
	{
		cUOTxCharEquipment equipItem;
		equipItem.fromItem( this );

		if ( singlesocket )
		{
			// don't remove backpack, because this would close all bags
			if ( this->layer() != cBaseChar::Backpack )
				singlesocket->removeObject( this );
			singlesocket->send( &equipItem );
			sendTooltip( singlesocket );
		}
		else
		{
			QList<cUOSocket*> sockets = Network::instance()->sockets();
			foreach ( cUOSocket* socket, sockets )
			{
				if ( socket->canSee( this ) )
				{
					if ( this->layer() != cBaseChar::Backpack )
						socket->removeObject( this );
					socket->send( &equipItem );
					sendTooltip( socket );
				}
			}
		}

		// items in containers
	}
	else if ( container_ && container_->isItem() )
	{
		//Old Client Version
		//cUOTxAddContainerItem contItem;
		cUOTxAddNewContainerItem contItem;
		contItem.fromItem( this );

		if ( singlesocket )
		{
			singlesocket->send( &contItem );
			sendTooltip( singlesocket );
		}
		else
		{
			QList<cUOSocket*> sockets = Network::instance()->sockets();
			foreach ( cUOSocket* socket, sockets )
			{
				if ( socket->canSee( this ) )
				{
					socket->send( &contItem );
					sendTooltip( socket );
				}
			}
		}
	}
}

P_ITEM cItem::dupe( bool dupeContent )
{
	P_ITEM nItem = new cItem( *this );
	nItem->setSerial( World::instance()->findItemSerial() );
	nItem->container_ = 0;

	if ( container_ )
	{
		P_CHAR pchar = dynamic_cast<P_CHAR>( container_ );

		if ( pchar )
		{
			nItem->moveTo( pchar->pos() );
		}
		else
		{
			P_ITEM item = dynamic_cast<P_ITEM>( container_ );

			if ( item )
			{
				item->addItem( nItem, false, true, false, false );
			}
		}
	}
	else
	{
		nItem->moveTo( pos_ );
	}

	if ( dupeContent )
	{
		this->dupeContent( nItem );
	}

	return nItem;
}

void cItem::dupeContent( P_ITEM container )
{

	for ( ContainerIterator it( this ); !it.atEnd(); ++it )
	{
		P_ITEM nItem = new cItem( **it );
		nItem->setSerial( World::instance()->findItemSerial() );
		nItem->container_ = 0;

		container->addItem( nItem, false, true, false, false );

		(*it)->dupeContent( nItem );
	}
}

void cItem::soundEffect( quint16 sound )
{
	QList<cUOSocket*> sockets = Network::instance()->sockets();
	foreach ( cUOSocket* mSock, sockets )
	{
		if ( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			mSock->soundEffect( sound, this );
	}
}

// This subtracts the weight of the top-container
// And then readds the new weight
void cItem::setTotalweight( float data )
{
	float difference = data - totalweight_;

	if ( difference != 0 )
	{
		totalweight_ += difference;

		if ( !unprocessed() && container_ )
		{
			P_CHAR pChar = dynamic_cast<P_CHAR>( container_ );

			if ( pChar && ( layer_ < 0x1A || layer_ == 0x1E ) )
			{
				pChar->setWeight( pChar->weight() + difference );
			}
			else
			{
				P_ITEM pItem = dynamic_cast<P_ITEM>( container_ );
				if ( pItem )
				{
					pItem->setTotalweight( pItem->totalweight() + difference );
				}
			}
		}
	}
}

void cItem::talk( const QString& message, UI16 color, quint8 type, bool autospam, cUOSocket* socket )
{
	Q_UNUSED( autospam );
	if ( color == 0xFFFF )
	{
		color = 0x3b2;
	}

	cUOTxUnicodeSpeech::eSpeechType speechType;

	switch ( type )
	{
	case 0x01:
		speechType = cUOTxUnicodeSpeech::Broadcast; break;
	case 0x06:
		speechType = cUOTxUnicodeSpeech::System; break;
	case 0x09:
		speechType = cUOTxUnicodeSpeech::Yell; break;
	case 0x02:
		speechType = cUOTxUnicodeSpeech::Emote; break;
	case 0x08:
		speechType = cUOTxUnicodeSpeech::Whisper; break;
	case 0x0A:
		speechType = cUOTxUnicodeSpeech::Spell; break;
	default:
		speechType = cUOTxUnicodeSpeech::Regular; break;
	};

	cUOTxUnicodeSpeech textSpeech;
	textSpeech.setSource( serial() );
	textSpeech.setModel( 0 );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( speechType );
	textSpeech.setLanguage( "" );
	textSpeech.setName( getName( true ) );
	textSpeech.setColor( color );
	textSpeech.setText( message );

	if ( socket )
	{
		socket->send( &textSpeech );
	}
	else
	{
		// Send to all clients in range
		QList<cUOSocket*> sockets = Network::instance()->sockets();
		foreach ( cUOSocket* mSock, sockets )
		{
			if ( mSock->canSee( this ) )
			{
				mSock->send( &textSpeech );
			}
		}
	}
}

void cItem::talk( const quint32 MsgID, const QString& params, const QString& affix, bool prepend, UI16 color, cUOSocket* socket )
{
	if ( color == 0xFFFF )
		color = 0x3b2;

	if ( socket )
	{
		if ( affix.isEmpty() )
			socket->clilocMessage( MsgID, params, color, 3, this );
		else
			socket->clilocMessageAffix( MsgID, params, affix, color, 3, this, false, prepend );
	}
	else
	{
		// Send to all clients in range
		QList<cUOSocket*> sockets = Network::instance()->sockets();
		foreach ( cUOSocket* mSock, sockets )
		{
			if ( mSock->canSee( this ) )
			{
				if ( affix.isEmpty() )
					mSock->clilocMessage( MsgID, params, color, 3, this );
				else
					mSock->clilocMessageAffix( MsgID, params, affix, color, 3, this, false, prepend );
			}
		}
	}
}

bool cItem::wearOut()
{
	if ( RandomNum( 1, 4 ) == 4 )
		setHp( hp() - 1 );

	if ( hp() <= 0 )
	{
		// Get the owner of the item
		P_CHAR owner = getOutmostChar();
		P_PLAYER pOwner = dynamic_cast<P_PLAYER>( owner );

		if ( pOwner && pOwner->socket() )
		{
			if ( !name_.isEmpty() )
			{
				pOwner->socket()->clilocMessageAffix( 1008129, QString::null, name_ );
			}
			else
			{
				pOwner->socket()->clilocMessageAffix( 1008129, QString::null, getName() );
			}
		}

		// Show to all characters in range that the item has been destroyed and not just unequipped
		if ( owner )
		{
			QList<cUOSocket*> sockets = Network::instance()->sockets();
			foreach ( cUOSocket* socket, sockets )
			{
				if ( owner != socket->player() && socket->canSee( owner ) )
				{
					socket->clilocMessageAffix( 0xf9060 + id_, "", tr( "You see %1 destroy his " ).arg( owner->name() ), 0x23, 3, owner, false, true );
				}
			}
		}

		remove();
		return true;
	}

	return false;
}

QList< cItem* > cItem::getContainment() const
{
	QList<cItem*> itemlist;

	for ( ContainerIterator it( content_ ); !it.atEnd(); ++it )
	{
		P_ITEM pItem = *it;

		// we'v got a container
		if ( pItem->type() == 1 || pItem->type() == 63 )
		{
			QList<cItem*> sublist = pItem->getContainment();

			// Transfer the items
			foreach( cItem* pi, sublist )
			{
				itemlist.append( pi );
			}
		}
		// Or just put it into our list
		else
			itemlist.append( pItem );
	}

	return itemlist;
}

unsigned char cItem::classid;

static FactoryRegistration<cItem> registration( "cItem" );

void cItem::load( QSqlQuery& result, ushort& offset )
{
	cUObject::load( result, offset ); // Load the items we inherit from first

	// Broken Serial?
	if ( !isItemSerial( serial() ) )
		throw wpException( QString( "Item has invalid character serial: 0x%1" ).arg( serial(), 0, 16 ) );

	id_ = result.value( offset++ ).toInt();
	color_ = result.value( offset++ ).toInt();

	SERIAL containerSerial = result.value( offset++ ).toInt();

	layer_ = result.value( offset++ ).toInt();
	amount_ = result.value( offset++ ).toInt();
	hp_ = result.value( offset++ ).toInt();
	maxhp_ = result.value( offset++ ).toInt();
	movable_ = result.value( offset++ ).toInt();
	ownserial_ = result.value( offset++ ).toInt();
	visible_ = result.value( offset++ ).toInt();
	priv_ = result.value( offset++ ).toInt();
	basedef_ = ItemBaseDefs::instance()->get( result.value( offset++ ).toByteArray() );

	// Their own weight should already be set.
	totalweight_ = amount_ * weight();

	//  Warning, ugly optimization ahead, if you have a better idea, we want to hear it.
	//  For load speed and memory conservation, we will store the SERIAL of the container
	//  here and then right after load is done we replace that value with it's memory address
	//  as it should be.
	if ( containerSerial != INVALID_SERIAL )
	{
		container_ = reinterpret_cast<cUObject*>( static_cast<size_t>( containerSerial ) );
		setUnprocessed( true );
	}
	// ugly optimization ends here.
	World::instance()->registerObject( this );
}

void cItem::buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions )
{
	cUObject::buildSqlString( objectid, fields, tables, conditions );
	fields.push_back( "items.id,items.color,items.cont,items.layer,items.amount,items.hp,items.maxhp,items.movable,items.owner,items.visible,items.priv,items.baseid" );
	tables.push_back( "items" );
	conditions.push_back( "uobjectmap.serial = items.serial" );
}

void cItem::addItem( cItem* pItem, bool randomPos, bool handleWeight, bool noRemove, bool autoStack )
{
	if ( !pItem )
		return;

	if ( free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Rejected putting an item (%1) into a freed container (%2)" ).arg( pItem->serial(), 0, 16 ).arg( serial_, 0, 16 ) );
		return;
	}

	if ( pItem == this )
	{
		Console::instance()->log( LOG_WARNING, tr( "Rejected putting an item into itself (%1)" ).arg( serial_, 0, 16 ) );
		return;
	}

	if ( pItem->multi() )
	{
		if ( !pItem->unprocessed() )
		{
			// pItem has to be removed from Multi!
			pItem->multi()->removeObject( pItem );
		}
		pItem->setMulti( 0 );
	}

	if ( !noRemove )
	{
		pItem->removeFromCont( handleWeight );
	}

	if ( randomPos )
	{
		if ( autoStack && containerPileItem( pItem ) )
		{
			// If the Server is running and this happens, resend the tooltip of us and
			// all our parent containers.
			if ( Server::instance()->getState() == RUNNING )
			{
				P_ITEM cont = this;

				while ( cont )
				{
					cont->resendTooltip();
					cont = dynamic_cast<P_ITEM>( cont->container() );
				}
			}

			return; // The item in question was removed.
		}
		else
		{
			pItem->setRandPosInCont( this );
		}
	}

	if ( !content_.contains( pItem ) && handleWeight )
	{
		// Increase the totalweight upward recursively
		setTotalweight( totalweight() + pItem->totalweight() );
	}

	content_.add( pItem );
	pItem->layer_ = 0;
	pItem->setContainer( this );

	// If the Server is running and this happens, resend the tooltip of us and
	// all our parent containers.
	if ( Server::instance()->getState() == RUNNING )
	{
		P_ITEM cont = this;
		while ( cont )
		{
			cont->resendTooltip();
			cont = dynamic_cast<P_ITEM>( cont->container() );
		}
	}
}

void cItem::removeItem( cItem* pItem, bool handleWeight )
{
	// only do this if it's really in the container
	if ( content_.contains( pItem ) )
	{
		content_.remove( pItem );
		if ( handleWeight )
		{
			setTotalweight( this->totalweight() - pItem->totalweight() );
		}
	}

	pItem->container_ = 0;
	pItem->flagChanged();
	pItem->setLayer( 0 );

	// If the Server is running and this happens, resend the tooltip of us and
	// all our parent containers.
	if ( Server::instance()->getState() == RUNNING )
	{
		P_ITEM cont = this;

		while ( cont )
		{
			cont->resendTooltip();
			cont = dynamic_cast<P_ITEM>( cont->container() );
		}
	}
}

const ContainerContent& cItem::content() const
{
	return content_;
}

bool cItem::contains( const cItem* pItem ) const
{
	return content_.contains( pItem );
}


void cItem::removeFromCont( bool handleWeight )
{
	if ( !container_ )
		return;

	if ( container_->isChar() )
	{
		P_CHAR pChar = dynamic_cast<P_CHAR>( container_ );
		if ( pChar )
			pChar->removeItem( ( cBaseChar::enLayer ) layer_, handleWeight );
	}
	else if ( container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast<P_ITEM>( container_ );
		if ( pCont )
			pCont->removeItem( this, handleWeight );
	}

	container_ = 0;
	flagChanged();
}

P_ITEM cItem::getOutmostItem()
{
	if ( container_ && container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast<P_ITEM>( container_ );
		if ( pCont )
			return pCont->getOutmostItem();
		else
			return this;
	}
	else
		return this;
}

P_CHAR cItem::getOutmostChar()
{
	P_CHAR result = 0;

	if ( container_ )
	{
		result = dynamic_cast<P_CHAR>( container_ );

		if ( !result )
		{
			P_ITEM container = dynamic_cast<P_ITEM>( container_ );

			if ( container )
			{
				result = container->getOutmostChar();
			}
		}
	}

	return result;
}

// If we change the amount, the weight changes as well
void cItem::setAmount( UI16 nValue )
{
	setTotalweight( totalweight_ + ( nValue - amount_ ) * weight() );
	amount_ = nValue;
	changed( TOOLTIP );
	flagChanged();
}

quint16 cItem::getWeaponSkill()
{
	switch ( type() )
	{
		// 1001: Sword Weapons (Swordsmanship)
		// 1002: Axe Weapons (Swordsmanship + Lumberjacking)
	case 1001:
	case 1002:
		return SWORDSMANSHIP;
		break;

		// 1003: Macefighting (Staffs)
		// 1004: Macefighting (Maces/WarHammer)
	case 1003:
	case 1004:
		return MACEFIGHTING;
		break;

		// 1005: Fencing
	case 1005:
		return FENCING;
		break;

		// 1006: Bows
		// 1007: Crossbows
	case 1006:
	case 1007:
		return ARCHERY;
		break;

	default:
		return WRESTLING;
	};
}

// Simple setting and getting of properties for scripts and the set command.
stError* cItem::setProperty( const QString& name, const cVariant& value )
{
	changed( TOOLTIP );
	flagChanged();
	/*
		\property item.id The artwork id of the object.
	*/
	SET_INT_PROPERTY( "id", id_ )
	/*
	\property item.color The hue id of the artwork.
	*/
	else
		SET_INT_PROPERTY( "color", color_ )
		/*
		\property item.baseid The base-id refering to the definition from which the object was created.
		*/
	else if ( name == "baseid" )
	{
		setBaseid( value.toString().toLatin1() );
		return 0;
	}
	else if ( name == "lockeddown" )
	{
		setLockedDown( value.toInt() != 0 );
		return 0;
	}
	/*
		\property item.amount The amount of objects in a stack.
	*/
	// Amount needs weight handling
	else if ( name == "amount" )
	{
		int val = value.toInt();
		if ( val <= 0 )
		{
			this->remove();
			return 0;
		}

		int diff = val - amount_;
		setTotalweight( totalweight_ + diff * weight() );
		amount_ = val;
		return 0;
	}
	/*
		\property item.layer The layer of the object, used with equipable objects.
	*/
	else if (name == "layer")
	{
		if (container_ && container_->isChar()) 
		{
			PROPERTY_ERROR(-3, "You may not change the layer of items that are currently equipped.");
		} 
		else 
		{
			SET_INT_PROPERTY("layer", layer_);
		}
	}
		/*
		\property item.health The current health or durability of the object.
		*/
	else
		SET_INT_PROPERTY( "health", hp_ )
		/*
		\property item.maxhealth The maximum health or durability value of the object.
		*/
	else
		SET_INT_PROPERTY( "maxhealth", maxhp_ )
		/*
		\property item.owner The character serial to which the object belongs to.
		*/
	else
		SET_INT_PROPERTY( "owner", ownserial_ )
		/*
		\property item.totalweight The total weight of stacked objcets.
		*/
	else if ( name == "totalweight" )
	{
		setTotalweight( static_cast<QString>( value.toString() ).toFloat() );
		return 0;
	}
	/*
		\property container The <object id="CHAR">CHAR</object> or
			<object id="ITEM">ITEM</object> this item is contained in. See the layer property for
			the layer this item is equipped on if this is a character. This property may also be None.
	*/
	else if ( name == "container" )
	{
		// To int and Check for Serial type (makes it safer as well)
		SERIAL cser = value.toInt();
		P_CHAR pChar = FindCharBySerial( cser );
		P_ITEM pItem = FindItemBySerial( cser );

		if ( pItem )
			pItem->addItem( this );
		else if ( pChar )
		{
			// Get a valid layer
			tile_st tInfo = TileCache::instance()->getTile( id_ );
			if ( tInfo.layer != 0 )
				pChar->addItem( ( cBaseChar::enLayer ) tInfo.layer, this );
		}
		else
		{
			// Remove from Cont and move to the old containers position
			P_ITEM pCont = getOutmostItem();
			pChar = pCont->getOutmostChar();

			removeFromCont();

			if ( pChar )
				moveTo( pChar->pos() );
			else
				moveTo( pCont->pos() );
		}
	}
	/*
		\property item.visible The visibile level of the object.
		Values:
		<code>0 - Invisible
		1 - Visible
		</code>
		*/
	else if ( name == "visible" )
	{
		if ( value.toInt() )
			visible_ = 0;
		else
			visible_ = 2;

		return 0;
	}
	/*
		\property item.ownervisible Owner visibility level
		Values:
		<code>1 - Owner can see
		0 - Owner can not see.
		</code>
	*/
	else if ( name == "ownervisible" )
	{
		if ( value.toInt() )
			visible_ = 1;
		else
			visible_ = 2;

		return 0;
	}

	/*
		\property item.movable The movable permission for the object.
		Values:
		<code>0 - Tiledata default
		1 - Movable by anyone
		2 - Owner
		3 - Nobody
		</code>
		*/
	else
		SET_INT_PROPERTY( "movable", movable_ )

		// Flags
		/*
		\property item.decay Enable or disable the decay of the object.
		Values:
		<code>0 - No Decay
		1 - Decay
		</code>
		*/
	else if ( name == "decay" )
	{
		if ( value.toInt() )
			setNoDecay( false );
		else
			setNoDecay( true );
		return 0;
	}
	/*
		\property item.newbie Enable or disable the object's newbie flag.
		Values:
		<code>0 - False
		1 - True
		</code>
	*/
	else if ( name == "newbie" )
	{
		if ( value.toInt() )
			setNewbie( true );
		else
			setNewbie( false );
		return 0;
	}
	/*
		\property item.dispellable Enable or disable dispelling of this object.
		Values:
		<code>0 - False
		1 - True
		</code>
	*/
	else if ( name == "dispellable" )
	{
		if ( value.toInt() )
			priv_ |= 0x04;
		else
			priv_ &= ~0x04;
		return 0;
	}
	/*
		\property item.secured Enable or disable as a secured object.
		Values:
		<code>0 - False
		1 - True
		</code>
	*/
	else if ( name == "secured" )
	{
		if ( value.toInt() )
			priv_ |= 0x08;
		else
			priv_ &= ~0x08;
		return 0;
	}
	/*
		\property item.allowmeditation Enable or disable the use of the meditation skill with this object.
		Values:
		<code>0 - Disallow Meditation
		1 - Allow Meditation
		</code>
	*/
	else if ( name == "allowmeditation" )
	{
		if ( value.toInt() )
			priv_ |= 0x10;
		else
			priv_ &= ~0x10;
		return 0;
	}
	/*
		\property item.twohanded Defines if the object a two handed weapon.
		Values:
		<code>0 - One Handed
		1 - Two Handed
		</code>
	*/
	else if ( name == "twohanded" )
	{
		if ( value.toInt() )
			priv_ |= 0x20;
		else
			priv_ &= ~0x20;
		return 0;
	}
	/*
		\property item.dye Defines if the object can be dyed or not.
		Values:
		<code>0 - No
		1 - Yes
		</code>
	*/
	else if ( name == "dye" )
	{
		setDye( value.toInt() != 0 ? true : false );
		return 0;
	}

	return cUObject::setProperty( name, value );
}

PyObject* cItem::getProperty( const QString& name, uint hash )
{
	/*
	#define OUTPUT_HASH(x) QString("case 0x%2: // %1\n").arg(x).arg( elfHash( x ), 0, 16)
	Console::instance()->send(
	OUTPUT_HASH("id") + 
	OUTPUT_HASH("lightsource") + 
	OUTPUT_HASH("decaydelay") + 
	OUTPUT_HASH("baseid") + 
	OUTPUT_HASH("color") + 
	OUTPUT_HASH("amount") + 
	OUTPUT_HASH("layer") + 
	OUTPUT_HASH("type") + 
	OUTPUT_HASH("weight") + 
	OUTPUT_HASH("sellprice") + 
	OUTPUT_HASH("buyprice") + 
	OUTPUT_HASH("health") + 
	OUTPUT_HASH("maxhealth") + 
	OUTPUT_HASH("owner") + 
	OUTPUT_HASH("totalweight") + 
	OUTPUT_HASH("container") + 
	OUTPUT_HASH("visible") + 
	OUTPUT_HASH("ownervisible") + 
	OUTPUT_HASH("dye") + 
	OUTPUT_HASH("decay") +
	OUTPUT_HASH("newbie") +
	OUTPUT_HASH("dispellable") +
	OUTPUT_HASH("secured") +
	OUTPUT_HASH("allowmeditation") +
	OUTPUT_HASH("twohanded") +
	OUTPUT_HASH("corpse") +
	OUTPUT_HASH("movable") +
	OUTPUT_HASH("lockeddown") +
	OUTPUT_HASH("watersource") +
	OUTPUT_HASH("basescripts")
	);
	#undef OUTPUT_HASH
	*/
	
	if ( !hash )
		hash = elfHash( name.toLatin1() );

	switch ( hash )
	{
	case 0x6f4: // id
		return createPyObject(id_);
	case 0xbf43565: // lightsource
		/*
		\rproperty item.lightsource For lightsources this is the type of the light.

		This property is inherited from the definition specified in the baseid property.
		*/
		return createPyObject(lightsource());
	case 0x8fc2919: // decaydelay
		/*
		\rproperty item.decaydelay The decay delay for this item in miliseconds.

		This is 0 if the item won't decay.
		*/
		return createPyObject(( int ) decayDelay());
	case 0x6889bf4: // baseid
		return createPyObject(baseid());
	case 0x6a6362: // color
		return createPyObject(color_);
	case 0x6846c54: // amount
		return createPyObject(amount_);
	case 0x728fc2: // layer
		return createPyObject(layer_);
	case 0x7c065: // type
		/*
		\rproperty item.type The type value of an object. Used to group weapons,
		armor and other equipables, as well as usable objects.

		This property is inherited from the definition specified in the baseid property.
		*/
		return createPyObject(type());
	case 0x7dbfdf4: // weight
		/*
		\rproperty item.weight The weight value of the object.

		This property is inherited from the definition specified in the baseid property.
		*/
		return createPyObject( weight() );
	case 0x337f655: // sellprice
		/*
		\rproperty item.sellprice The value at which this object can be sold to vendors.

		This property is inherited from the definition specified in the baseid property.
		*/
		return createPyObject( ( int ) sellprice() );
	case 0xd078905: // buyprice
		/*
		\rproperty item.buyprice The value at which this object is bought from vendors.

		This property is inherited from the definition specified in the baseid property.
		*/
		return createPyObject( ( int ) buyprice() );
	case 0x6eb83a8: // health
		return createPyObject( hp_ );
	case 0xeeb1028: // maxhealth
		return createPyObject( maxhp_ );
	case 0x76e4c2: // owner
		return createPyObject( owner() );
	case 0x3a09774: // totalweight
		return createPyObject( totalweight_ );
	case 0x5a7aea2: // container
		return createPyObject( container_ );
	case 0xd09f955: // visible
		return createPyObject( visible_ == 0 ? 1 : 0 );
	case 0xc67b5b5: // ownervisible
		return createPyObject( visible_ == 1 ? 1 : 0 );
	case 0x6bf5: // dye
		return createPyObject( dye() ? 1 : 0 );
	case 0x6ab989: // decay
		return createPyObject( priv_ & 0x01 ? 0 : 1 );
	case 0x74cd8f5: // newbie
		return createPyObject( priv_ & 0x02 ? 1 : 0 );
	case 0xc597345: // dispellable
		return createPyObject( priv_ & 0x04 ? 1 : 0 );
	case 0x9bac8c4: // secured
		return createPyObject( priv_ & 0x08 ? 1 : 0 );
	case 0x3c6fdfe: // allowmeditation
		return createPyObject( priv_ & 0x10 ? 1 : 0 );
	case 0x5e83154: // twohanded
		return createPyObject( priv_ & 0x20 ? 1 : 0 );
	case 0x6a69795: // corpse
		/*
		\rproperty item.corpse Specifies whether this item is a corpse or not.
		*/
		return createPyObject( corpse() );
	case 0x46c7955: // movable
		return createPyObject( movable_ );
	case 0x1bda37e: // lockeddown
		/*
		\property item.lockeddown This property indicates that the item has been locked down by a
		player.
		*/
		return createPyObject( isLockedDown() );
	case 0x9fb4255: // watersource
		/*
		\rproperty item.watersource This property indicates that this type of item is a source of
		water. If there is a "quantity" tag for the item, it should be used, otherwise the source
		is indepletable.
		This property is inherited from the base id of this item.
		*/
		return createPyObject( isWaterSource() );
	case 0x9c18e73: // basescripts
		/*
		\rproperty item.basescripts This is a comma separated list of scripts assigned to this item
		via the baseid. They are called after the scripts assigned dynamically to the item.
		*/
		return createPyObject( basedef_ ? basedef_->baseScriptList() : "" );

	default:
		return cUObject::getProperty( name, hash );
	}
}

void cItem::sendTooltip( cUOSocket* mSock )
{
	// There is a list of statically overridden items in the client (@50A1C0 for 4.0.0o)
	unsigned short id = this->id();

	// Mostly Signs (not movable but still have tooltips shown)
	if ( ( id >= 0xba3 && id <= 0xc0e ) ||     // House Signs
		( id >= 0x1297 && id <= 0x129e ) || 	// Road Signs
		( id >= 0x3e4a && id <= 0x3e55 ) || 	// Tillermen
		( id >= 0xed4 && id <= 0xede ) ||     // Graves and Guildstones
		( id >= 0x1165 && id <= 0x1184 ) || 	// More Gravestones
		( id == 0x2006 ) || !name_.isEmpty() // Non Default Name
	   )
	{
		cUObject::sendTooltip( mSock );
		return;
	}

	// Don't send the tooltip if we're a supposed-to-be-static item
	tile_st tile = TileCache::instance()->getTile( id_ );

	// If the item is not movable for the client, the item should not have a tooltip
	// Exceptions are noted above and containers
	if ( tile.weight == 255 && !isAllMovable() )
	{
		if ( ( tile.flag3 & 0x20 ) == 0 )
			return;
	}

	cUObject::sendTooltip( mSock );
}

/*!
	Selects an item id from a list and creates it.
*/
P_ITEM cItem::createFromList( const QString& id )
{
	QString entry = Definitions::instance()->getRandomListEntry( id );
	return createFromScript( entry );
}

/*!
	Creates a new item and applies the specified definition section on it.
*/
P_ITEM cItem::createFromScript( const QString& id )
{
	P_ITEM nItem = 0;

	// Get an Item and assign a serial to it
	const cElement* section = Definitions::instance()->getDefinition( WPDT_ITEM, id );

	if ( section )
	{
		nItem = new cItem;
		nItem->Init( true );
		nItem->setBaseid( id.toLatin1() );
		nItem->applyDefinition( section );
		cDelayedOnCreateCall* onCreateCall = new cDelayedOnCreateCall( nItem, id );
		Timers::instance()->insert( onCreateCall );
	}
	else
	{
		Console::instance()->log( LOG_ERROR, tr( "Unable to create unscripted item: %1\n" ).arg( id ) );
	}

	return nItem;
}

P_ITEM cItem::createFromId( unsigned short id )
{
	P_ITEM pItem = new cItem;
	pItem->Init( true );
	pItem->setId( id );

	// Try to set a baseid
	const cElement* element = Definitions::instance()->getDefinition( WPDT_ITEM, QString::number( id, 16 ) );

	if ( element )
	{
		pItem->setBaseid( QString::number( id, 16 ).toLatin1() );
	}

	return pItem;
}

void cItem::createTooltip( cUOTxTooltipList& tooltip, cPlayer* player )
{
	cUObject::createTooltip( tooltip, player );

	// Add the object name.
	if ( amount_ > 1 )
	{
		if ( name_.isEmpty() ) {
			unsigned int clilocName = this->clilocName();
			if (clilocName != 0) {
				tooltip.addLine( 1050039, QString( "%1\t#%2" ).arg( amount_ ).arg( clilocName ) );
			} else {
				tooltip.addLine( 1050039, QString( "%1\t#%2" ).arg( amount_ ).arg( 1020000 + id_ ) );
			}
		} else {
			tooltip.addLine( 1050039, QString( "%1\t%2" ).arg( amount_ ).arg( name_ ) );
		}
	}
	else
	{
		if ( name_.isEmpty() ) {
			unsigned int clilocName = this->clilocName();
			if (clilocName != 0) {
				tooltip.addLine( clilocName, "" );
			} else {
				tooltip.addLine( 1020000 + id_, "" );
			}
		} else {
			tooltip.addLine( 1042971, name_ );
		}
	}

	// Add tooltip for locked down items
	if (isLockedDown()) {
		tooltip.addLine(501643, "");
	}

	// For containers (hardcoded type), add count of items and total weight.
	if ( type() == 1 )
	{
		unsigned int count = content_.count();
		unsigned int weight = ( unsigned int ) floor( totalweight_ );
		tooltip.addLine( 1050044, QString( "%1\t%2" ).arg( count ).arg( weight ) );
	}

	// Newbie/Blessed Items
	if ( newbie() )
	{
		tooltip.addLine( 1038021, "" );
	}
	// Cursed Items
	else if ( hasTag( "cursed" ) )
	{
		tooltip.addLine( 1049643, "" );
	}

	// Invisible to others
	if ( player->isGM() && visible() > 0 )
		tooltip.addLine( 3000507, "" );

	onShowTooltip( player, &tooltip );
}

// Python implementation
PyObject* cItem::getPyObject()
{
	return PyGetItemObject( this );
}

const char* cItem::className() const
{
	return "item";
}

bool cItem::canStack( cItem* pItem )
{
	tile_st tile = TileCache::instance()->getTile( id_ );
	if ( !( tile.flag2 & 0x08 ) )
	{
		return false;
	}

	// Do some basic checks and see if the item is a
	// container (they never stack).
	if ( name() != pItem->name() || id() != pItem->id() || color() != pItem->color() || type() == 1 || scriptList() != pItem->scriptList() || baseid() != pItem->baseid() )
	{
		return false;
	}

	// Check Tags (rather expensive)
	/*if ( tags_ != pItem->tags_ )
	{
		return false;
	}*/

	return true;
}

/*!
	Counts the items in this container which match a list of
	specific \p baseids.
*/
unsigned int cItem::countItems( const QStringList& baseids ) const
{
	unsigned int count = 0;
	if ( baseids.contains( baseid() ) )
	{
		count += amount();
	}

	for ( ContainerIterator it( this ); !it.atEnd(); ++it )
	{
		count += ( *it )->countItems( baseids );
	}
	return count;
}

void cItem::setGridLocation(unsigned char value)
{
	if (value >= 0 && value < 126)
	{
		gridLocation = value;
	}
}

unsigned char cItem::getGridLocation()
{
	return gridLocation;
}

unsigned int cItem::countItems( short id, short color ) const
{
	unsigned int total = 0;
	QList<cItem*> content = getContainment();

	foreach ( P_ITEM pi, content )
	{
		if ( !pi || pi->free ) // just to be sure ;-)
			continue;
		if ( pi->id() == id && ( color == -1 || pi->color() == color ) )
			total += pi->amount();
	}
	return total;
}

unsigned int cItem::countBaseItems( QString baseid, short color ) const
{
	unsigned int total = 0;
	QList<cItem*> content = getContainment();

	foreach ( P_ITEM pi, content )
	{
		if ( !pi || pi->free ) // just to be sure ;-)
			continue;
		if ( pi->baseid() == baseid && ( color == -1 || pi->color() == color ) )
			total += pi->amount();
	}
	return total;
}

/*!
	\brief Removes a certain amount of items from this container
	recursively.
	\param baseids The list of baseids that a item can have.
	\param amount The amount of items to remove.
	\returns The remaining amount of items to be removed.
*/
unsigned int cItem::removeItems( const QStringList& baseids, unsigned int amount )
{
	// We can statisfy the need by removing from ourself
	if ( baseids.contains( baseid() ) )
	{
		if ( this->amount() > amount )
		{
			setAmount( this->amount() - amount );
			update();
			return 0;
		}
		else
		{
			amount -= this->amount();
			remove();
			return amount;
		}
	}

	for ( ContainerCopyIterator it( this ); !it.atEnd(); ++it )
	{
		amount = ( *it )->removeItems( baseids, amount );
	}

	return amount;
}

/*!
\brief Removes a certain amount of items from this container
recursively.
\param baseids The list of baseids that a item can have.
\param amount The amount of items to remove.
\returns The remaining amount of items to be removed.
*/
unsigned int cItem::removeItem( const QString& id, unsigned int amount )
{
	// We can statisfy the need by removing from ourself
	if ( id == QString(this->baseid()) )
	{
		if ( this->amount() > amount )
		{
			setAmount( this->amount() - amount );
			update();
			return 0;
		}
		else
		{
			amount -= this->amount();
			remove();
			return amount;
		}
	}

	for ( ContainerCopyIterator it( this ); !it.atEnd(); ++it )
	{
		amount = ( *it )->removeItem( id, amount );
	}

	return amount;
}

void cItem::moveTo( const Coord& newpos )
{
	if ( container_ )
	{
		pos_ = newpos;
		pos_.setInternalMap(); // We're not in the sector maps
		changed_ = true;
	}
	else
	{
		// See if the map is valid
		if ( !newpos.isInternalMap() && !Maps::instance()->hasMap( newpos.map ) )
		{
			return;
		}

		cUObject::moveTo( newpos );
	}
}

bool cItem::isInLockedItem()
{
	if ( container_ && container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast<P_ITEM>( container_ );

		if ( pCont->hasScript( "lock" ) && pCont->hasTag( "locked" ) && pCont->getTag( "locked" ).toInt() != 0 )
		{
			return true;
		}

		return pCont->isInLockedItem();
	}
	else
		return false;
}

void cItem::setRestock( unsigned short value )
{
	if ( value == 0 )
	{
		removeTag( "restock" );
	}
	else
	{
		setTag( "restock", cVariant( ( int ) value ) );
	}
}

unsigned short cItem::restock()
{
	if ( !hasTag( "restock" ) )
	{
		return 0;
	}
	else
	{
		return getTag( "restock" ).toInt();
	}
}

unsigned int cItem::decayDelay()
{
	if ( container_ || nodecay() || ( !corpse() && multi_ ) || isLockedDown() )
	{
		return 0;
	}

	if ( basedef_ && basedef_->decaydelay() != 0 )
	{
		return basedef_->decaydelay() * MY_CLOCKS_PER_SEC;
	}

	return Config::instance()->itemDecayTime() * MY_CLOCKS_PER_SEC;
}

void cItem::save( cBufferedWriter& writer )
{
	if ( free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Skipping item 0x%1 during save process because it's already freed.\n" ).arg( serial_, 0, 16 ) );
	}
	else if ( container_ && container_->free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Skipping item 0x%1 during save process because it's in a freed container.\n" ).arg( serial_, 0, 16 ) );
	}
	else
	{
		cUObject::save( writer );

		// Save container content
		for ( ContainerIterator it( this ); !it.atEnd(); ++it )
		{
			( *it )->save( writer );
		}
	}
}

void cItem::load( cBufferedReader& reader )
{
	load( reader, reader.version() );

	World::instance()->registerObject( this );
}

bool cItem::callEventHandler( ePythonEvent event, PyObject* args, bool ignoreErrors )
{
	PyObject *result = callEvent( event, args, ignoreErrors );

	if ( result )
	{
		if ( PyObject_IsTrue( result ) )
		{
			Py_DECREF( result );
			return true;
		}
		else
		{
			Py_DECREF( result );
		}
	}
	return false;
}

PyObject* cItem::callEvent( ePythonEvent event, PyObject* args, bool ignoreErrors )
{
	PyObject *result = 0;

	if ( scriptChain )
	{
		result = cPythonScript::callChainedEvent( event, scriptChain, args );

		// Break if there has been a result already
		if ( result && PyObject_IsTrue( result ) )
		{
			return result;
		}
	}

	// call the basescripts
	if ( basedef_ )
	{
		const QList<cPythonScript*> &list = basedef_->baseScripts();
		QList<cPythonScript*>::const_iterator it( list.begin() );
		for ( ; it != list.end(); ++it )
		{
			result = ( *it )->callEvent( event, args, ignoreErrors );

			if ( result && PyObject_IsTrue( result ) )
			{
				return result;
			}
		}
	}

	// check for a global handler
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook( event );

	if ( globalHook )
	{
		result = globalHook->callEvent( event, args, ignoreErrors );
	}

	return result;
}

bool cItem::canHandleEvent( ePythonEvent event )
{
	// Is there a global event?
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook( event );

	if ( globalHook )
	{
		return true;
	}

	if ( cPythonScript::canChainHandleEvent( event, scriptChain ) )
	{
		return true;
	}

	if ( basedef_ )
	{
		const QList<cPythonScript*> &list = basedef_->baseScripts();
		QList<cPythonScript*>::const_iterator it( list.begin() );
		for ( ; it != list.end(); ++it )
		{
			if ( ( *it )->canHandleEvent( event ) )
			{
				return true;
			}
		}
	}

	return false;
}

bool cItem::hasScript( const QByteArray& name )
{
	if ( basedef_ )
	{
		const QList<cPythonScript*> &list = basedef_->baseScripts();
		QList<cPythonScript*>::const_iterator it( list.begin() );
		for ( ; it != list.end(); ++it )
		{
			if ( ( *it )->name() == name )
			{
				return true;
			}
		}
	}

	return cUObject::hasScript( name );
}

Coord cItem::getOutmostPos()
{
	if ( container_ )
	{
		if ( container_->isChar() )
		{
			return container_->pos();
		}
		P_ITEM container = dynamic_cast<P_ITEM>( container_ );
		if ( container )
		{
			return container->getOutmostPos();
		}
	}
	return pos_;
}

unsigned int cItem::getSellPrice( P_CHAR pVendor, P_CHAR player )
{
	unsigned int sellprice = this->sellprice();
	bool fromItem = false;
	bool itemCanHandle = canHandleEvent( EVENT_GETSELLPRICE );
	bool npcCanHandle = pVendor->canHandleEvent( EVENT_GETSELLPRICE );
	PyObject *args = 0;

	if ( itemCanHandle || npcCanHandle )
	{
		args = Py_BuildValue( "(NNN)", getPyObject(), pVendor->getPyObject(), player->getPyObject() );
	}

	if ( itemCanHandle )
	{
		PyObject *result = callEvent( EVENT_GETSELLPRICE, args );
		if ( result )
		{
			if ( PyInt_CheckExact( result ) )
			{
				sellprice = PyInt_AsLong( result );
				fromItem = true;
			}
			else if ( PyLong_CheckExact( result ) )
			{
				sellprice = PyLong_AsLong( result );
				fromItem = true;
			}
		}
		Py_XDECREF( result );
	}

	if ( npcCanHandle && !fromItem )
	{
		PyObject *result = pVendor->callEvent( EVENT_GETSELLPRICE, args );
		if ( result )
		{
			if ( PyInt_CheckExact( result ) )
			{
				sellprice = PyInt_AsLong( result );
				fromItem = true;
			}
			else if ( PyLong_CheckExact( result ) )
			{
				sellprice = PyLong_AsLong( result );
				fromItem = true;
			}
		}
		Py_XDECREF( result );
	}

	Py_XDECREF( args );

	return sellprice;
}

unsigned int cItem::getBuyPrice( P_CHAR pVendor, P_CHAR player )
{
	unsigned int buyprice = this->buyprice();
	bool fromItem = false;
	bool itemCanHandle = canHandleEvent( EVENT_GETBUYPRICE );
	bool npcCanHandle = pVendor->canHandleEvent( EVENT_GETBUYPRICE );
	PyObject *args = 0;

	if ( itemCanHandle || npcCanHandle )
	{
		args = Py_BuildValue( "(NNN)", getPyObject(), pVendor->getPyObject(), player->getPyObject() );
	}

	if ( itemCanHandle )
	{
		PyObject *result = callEvent( EVENT_GETBUYPRICE, args );
		if ( result )
		{
			if ( PyInt_CheckExact( result ) )
			{
				buyprice = PyInt_AsLong( result );
				fromItem = true;
			}
			else if ( PyLong_CheckExact( result ) )
			{
				buyprice = PyLong_AsLong( result );
				fromItem = true;
			}
		}
		Py_XDECREF( result );
	}

	if ( npcCanHandle && !fromItem )
	{
		PyObject *result = pVendor->callEvent( EVENT_GETBUYPRICE, args );
		if ( result )
		{
			if ( PyInt_CheckExact( result ) )
			{
				buyprice = PyInt_AsLong( result );
				fromItem = true;
			}
			else if ( PyLong_CheckExact( result ) )
			{
				buyprice = PyLong_AsLong( result );
				fromItem = true;
			}
		}
		Py_XDECREF( result );
	}

	Py_XDECREF( args );

	return buyprice;
}

void cItem::setContainer( cUObject* d )
{
	if ( d && !pos_.isInternalMap() )
	{
		MapObjects::instance()->remove( this );
		pos_.setInternalMap();

		if ( multi_ )
		{
			multi_->removeObject( this );
			multi_ = 0;
		}
	}

	container_ = d; flagChanged();
}
