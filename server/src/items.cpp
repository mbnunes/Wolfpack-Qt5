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
#include "sectors.h"
#include "scriptmanager.h"
#include "basechar.h"
#include "player.h"
#include "basics.h"

#include "inlines.h"
#include "console.h"

// System Includes
#include <math.h>
#include <algorithm>

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

cItem::cItem( const cItem& src ) : cUObject(src), totalweight_( 0 ), container_( 0 )
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
	moveTo( src.pos_, true );
}

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
long cItem::reduceAmount( const short amt )
{
	Q_UINT16 rest = 0;
	if ( amount_ > amt )
	{
		setAmount( amount_ - amt );
		update();
		changed( TOOLTIP );
		flagChanged();
	}
	else
	{
		this->remove();
		rest = amt - amount_;
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
	case 0x0A2C:	// chest of drawers
	case 0x0A30:	// chest of drawers
	case 0x0A34:	// chest of drawers
	case 0x0A38:	// chest of drawers
	case 0x0A4D:	// armoire
	case 0x0A4F:	// armoire
	case 0x0A51:	// armoire
	case 0x0A53:	// armoire
	case 0x0A97:	// bookshelf
	case 0x0A98:	// bookshelf
	case 0x0A99:	// bookshelf
	case 0x0A9A:	// bookshelf
	case 0x0A9B:	// bookshelf
	case 0x0A9C:	// bookshelf
	case 0x0A9D:	// bookshelf
	case 0x0A9E:	// bookshelf
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
	cItem::ContainerContent::const_iterator it( content_.begin() );
	cItem::ContainerContent::const_iterator end( content_.end() );
	for ( ; it != end; ++it )
	{
		if ( ( *it )->pileItem( pItem ) )
			return true;
	}
	return false;
}

void cItem::setRandPosInCont( cItem* pCont )
{
	int k = pCont->containerGumpType();
	Coord_cl position = pos();
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
	setPos( position );
}

/*!
	Recurses through the container given by serial and deletes items of
	the given id and color(if given) until the given amount is reached
*/
int cItem::deleteAmount( int amount, unsigned short _id, unsigned short _color )
{
	int rest = amount;
	P_ITEM pi;
	cItem::ContainerContent container( this->content() );
	cItem::ContainerContent::const_iterator it( container.begin() );
	cItem::ContainerContent::const_iterator end( container.end() );
	for ( ; it != end; ++it )
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

void cItem::postload( unsigned int /*version*/ )
{
}

void cItem::load( cBufferedReader& reader, unsigned int version )
{
	cUObject::load( reader, version );

	id_ = reader.readShort();
	color_ = reader.readShort();
	// Here we assume that containers are always before us in the save
	cUObject* container = World::instance()->findObject( reader.readInt() );
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
			iContainer->addItem( this, false, true, true );
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
}

void cItem::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "items" );

		addField( "serial", serial() );
		addField( "id", id() );
		addField( "color", color() );
		SERIAL contserial = INVALID_SERIAL;
		if ( container_ )
			contserial = container_->serial();
		addField( "cont", contserial );
		addField( "layer", layer_ );
		addField( "amount", amount_ );
		addField( "hp", hp_ );
		addField( "maxhp", maxhp_ );
		addField( "movable", movable_ );
		addField( "owner", ownserial_ );
		addField( "visible", visible_ );
		addField( "priv", priv_ );
		addStrField( "baseid", baseid() );

		addCondition( "serial", serial() );
		saveFields;
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
	if ( !name_.isEmpty() && !name_.startsWith("#") )
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
	if ( ser == INVALID_SERIAL )
		return;

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
	this->setPos( Coord_cl( 100, 100, 0 ) );
	this->color_ = 0x00; // Hue
	this->layer_ = 0; // Layer if equipped on paperdoll
	this->amount_ = 1; // Amount of items in pile
	this->hp_ = 0; //Number of hit points an item has.
	this->maxhp_ = 0; // Max number of hit points an item can have.
	this->movable_ = 0; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable.
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
	if ( free )
	{
		return;
	}

	if ( canHandleEvent( EVENT_DELETE ) )
	{
		PyObject* args = Py_BuildValue( "(N)", getPyObject() );
		callEventHandler( EVENT_DELETE, args );
		Py_DECREF( args );
	}

	clearScripts();
	removeFromView( false ); // Remove it from all clients in range
	free = true;

	SetOwnSerial( -1 );

	// Check if this item is registered as a guildstone and remove it from the guild
	for ( cGuilds::iterator it = Guilds::instance()->begin(); it != Guilds::instance()->end(); ++it )
	{
		cGuild* guild = it.data();
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
		SectorMaps::instance()->remove( this );

		// Remove us from a possilbe multi container too
		if ( multi_ )
		{
			multi_->removeObject( this );
			multi_ = 0;
		}
	}

	// Create a copy of the content so we don't accidently change our working copy
	ContainerContent container( content() );
	ContainerContent::const_iterator it2;
	for ( it2 = container.begin(); it2 != container.end(); ++it2 )
		( *it2 )->remove();

	cUObject::remove();
}

bool cItem::onSingleClick( P_PLAYER Viewer ) {
	bool result = false;
	if (canHandleEvent(EVENT_SINGLECLICK)) {
		PyObject* args = Py_BuildValue("O&O&", PyGetItemObject, this, PyGetCharObject, Viewer);
		result = callEventHandler(EVENT_SINGLECLICK, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onDropOnItem( P_ITEM pItem )
{
	bool result = false;
	if (canHandleEvent(EVENT_DROPONITEM)) {
		PyObject* args = Py_BuildValue( "O&O&", PyGetItemObject, layer_ == 0x1E ? pItem : this, PyGetItemObject, layer_ == 0x1E ? this : pItem );
		result = callEventHandler(EVENT_DROPONITEM, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onDropOnGround( const Coord_cl& pos )
{
	bool result = false;
	if (canHandleEvent(EVENT_DROPONGROUND)) {
		PyObject* args = Py_BuildValue( "O&N", PyGetItemObject, this, PyGetCoordObject( pos ) );
		result = callEventHandler(EVENT_DROPONGROUND, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onPickup( P_CHAR pChar )
{
	bool result = false;

	if (canHandleEvent(EVENT_PICKUP))
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler(EVENT_PICKUP, args);
		Py_DECREF(args);
	}

	return result;
}

bool cItem::onEquip( P_CHAR pChar, unsigned char layer )
{
	bool result = false;
	if (canHandleEvent(EVENT_EQUIP)) {
		PyObject* args = Py_BuildValue( "O&O&b", PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = callEventHandler(EVENT_EQUIP, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onUnequip( P_CHAR pChar, unsigned char layer )
{
	bool result = false;
	if (canHandleEvent(EVENT_UNEQUIP)) {
		PyObject* args = Py_BuildValue( "O&O&b", PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = callEventHandler(EVENT_UNEQUIP, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onWearItem( P_PLAYER pPlayer, P_CHAR pChar, unsigned char layer )
{
	bool result = false;
	if (canHandleEvent(EVENT_WEARITEM)) {
		PyObject* args = Py_BuildValue( "O&O&O&b", PyGetCharObject, pPlayer, PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = callEventHandler(EVENT_WEARITEM, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onUse(P_CHAR pChar) {
	bool result = false;
	if (canHandleEvent(EVENT_USE)) {
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler(EVENT_USE, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onCollide(P_CHAR pChar)
{
	bool result = false;
	if (canHandleEvent(EVENT_COLLIDE)) {
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler(EVENT_COLLIDE, args);
		Py_DECREF(args);
	}
	return result;
}

bool cItem::onDropOnChar( P_CHAR pChar )
{
	bool result = false;
	if (canHandleEvent(EVENT_DROPONCHAR)) {
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = callEventHandler(EVENT_DROPONCHAR, args);
		Py_DECREF(args);
	}
	return result;
}

void cItem::processNode( const cElement* Tag )
{
	flagChanged();
	// we do this as we're going to modify the element
	QString TagName = Tag->name();
	QString Value = Tag->value();

	const cElement* section = Definitions::instance()->getDefinition( WPDT_DEFINE, TagName );

	// <amount>10</amount>
	if ( TagName == "amount" )
		this->setAmount( Value.toUShort() );

	// <durability>10</durabilty>
	else if ( TagName == "durability" )
	{
		this->setMaxhp( Value.toLong() );
		this->setHp( this->maxhp() );
	}

	// <movable />
	// <ownermovable />
	// <immovable />
	else if ( TagName == "movable" )
		this->movable_ = 1;
	else if ( TagName == "immovable" )
		this->movable_ = 2;
	else if ( TagName == "ownermovable" )
		this->movable_ = 3;

	// <decay />
	// <nodecay />
	else if ( TagName == "decay" )
		setNoDecay( false );
	else if ( TagName == "nodecay" )
		setNoDecay( true );

	// <dispellable />
	// <notdispellable />
	else if ( TagName == "dispellable" )
		this->priv_ |= 0x04;
	else if ( TagName == "notdispellable" )
		this->priv_ &= 0xFB;

	// <newbie />
	// <notnewbie />
	else if ( TagName == "newbie" )
		this->setNewbie( true );
	else if ( TagName == "notnewbie" )
		this->setNewbie( false );

	// <twohanded />
	else if ( TagName == "twohanded" )
		this->setTwohanded( true );

	// <singlehanded />
	else if ( TagName == "singlehanded" )
		this->setTwohanded( false );

	// <visible />
	// <invisible />
	// <ownervisible />
	else if ( TagName == "invisible" )
		this->visible_ = 2;
	else if ( TagName == "visible" )
		this->visible_ = 0;
	else if ( TagName == "ownervisible" )
		this->visible_ = 1;

	// <dye />
	// <nodye />
	else if ( TagName == "dye" )
		this->setDye( true );
	else if ( TagName == "nodye" )
		this->setDye( false );

	// <id>0x12f9</id>
	else if ( TagName == "id" )
	{
		this->setId( Value.toUShort() );
	}

	// <content><item id="a" />...<item id="z" /></content> (sereg)
	else if ( TagName == "content" && Tag->childCount() > 0 )
		this->processContainerNode( Tag );

	// <inherit>f23</inherit>
	else if ( TagName == "inherit" )
	{
		const cElement* section;

		if ( Tag->hasAttribute( "id" ) )
			section = Definitions::instance()->getDefinition( WPDT_ITEM, Tag->getAttribute( "id" ) );
		else
			section = Definitions::instance()->getDefinition( WPDT_ITEM, Value );

		if ( section )
			applyDefinition( section );
	}
	else if ( section )
	{
		for ( unsigned int i = 0; i < section->childCount(); ++i )
			processModifierNode( section->getChild( i ) );
	}
	else
		cUObject::processNode( Tag );
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
			int offset = Value.find( "%1" );
			QString left = Value.left( offset );
			QString right = Value.right( Value.length() - ( offset + 2 ) );
			name_ = left + name_ + right;
			*/
			name_ = Value.arg( name_ );
		}
	}

	// <durability>-10</durabilty>
	else if ( TagName == "durability" )
	{
		if ( Value.contains( "." ) || Value.contains( "," ) )
			setMaxhp( ( Q_INT32 ) ceil( ( float ) maxhp() * Value.toFloat() ) );
		else
			setMaxhp( maxhp() + Value.toLong() );
		setHp( maxhp() );
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
		<item id="myItem1"><amount><random ... /></amount><color><colorlist><random...></colorlist></color></item>
		...
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
				if (nItem) {
					addItem( nItem );
					for ( unsigned int j = 0; j < element->childCount(); ++j )
						nItem->processNode( element->getChild( j ) );
					if ( this->layer() == cBaseChar::BuyRestockContainer )
						nItem->setRestock( nItem->amount() );
				}
			}
			else if ( element->hasAttribute( "list" ) )
			{
				qWarning( "cItem::processContainerNode <item list=\"myList\"/> not implemented!!!" );
			}
			else
			{
				Console::instance()->log( LOG_ERROR, QString( "Content element lacking id and list attribute in item definition '%1'." ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
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
			if ( name_.isEmpty() )
			{
				message = 1050039;
				params = QString( "%1\t#%2" ).arg( amount_ ).arg( 1020000 + id_ );
			}
			else
			{
				message = 1050039;
				params = QString( "%1\t%2" ).arg( amount_ ).arg( name_ );
			}
		}
		else
		{
			if ( name_.isEmpty() )
			{
				message = 1042971;
				params = QString( "#%2" ).arg( 1020000 + id_ );
			}
			else
			{
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
			for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
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
					if ( isAllMovable() )
					{
						flags |= 0x20;
					}
					else if ( player->account()->isAllMove() )
					{
						flags |= 0x20;
					}
					else if ( isOwnerMovable() && player->owns( this ) )
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
			if ( isAllMovable() )
			{
				flags |= 0x20;
			}
			else if ( player->account()->isAllMove() )
			{
				flags |= 0x20;
			}
			else if ( isOwnerMovable() && player->owns( this ) )
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
			singlesocket->send( &equipItem );
			sendTooltip( singlesocket );
		}
		else
		{
			for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
			{
				if ( socket->canSee( this ) )
				{
					socket->send( &equipItem );
					sendTooltip( socket );
				}
			}
		}

		// items in containers
	}
	else if ( container_ && container_->isItem() )
	{
		cUOTxAddContainerItem contItem;
		contItem.fromItem( this );

		if ( singlesocket )
		{
			singlesocket->send( &contItem );
			sendTooltip( singlesocket );
		}
		else
		{
			for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
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

P_ITEM cItem::dupe()
{
	P_ITEM nItem = new cItem( *this );
	nItem->setSerial( World::instance()->findItemSerial() );

	if ( container_ )
	{
		P_CHAR pchar = dynamic_cast<P_CHAR>( container_ );

		if ( pchar )
		{
			nItem->container_ = 0;
			nItem->moveTo( pchar->pos(), true );
		}
		else
		{
			P_ITEM item = dynamic_cast<P_ITEM>( container_ );

			if ( item )
				item->addItem( nItem, false, true, true );
		}
	}
	else
	{
		nItem->moveTo( pos_ );
	}

	return nItem;
}

void cItem::soundEffect( Q_UINT16 sound )
{
	for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		if ( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			mSock->soundEffect( sound, this );
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

void cItem::talk( const QString& message, UI16 color, Q_UINT8 type, bool autospam, cUOSocket* socket )
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

	cUOTxUnicodeSpeech* textSpeech = new cUOTxUnicodeSpeech();
	textSpeech->setSource( serial() );
	textSpeech->setModel( 0 );
	textSpeech->setFont( 3 ); // Default Font
	textSpeech->setType( speechType );
	textSpeech->setLanguage( "" );
	textSpeech->setName( getName(true) );
	textSpeech->setColor( color );
	textSpeech->setText( message );

	if ( socket )
	{
		socket->send( textSpeech );
	}
	else
	{
		// Send to all clients in range
		for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		{
			if ( mSock->canSee( this ) )
			{
				mSock->send( new cUOTxUnicodeSpeech( *textSpeech ) );
			}
		}
		delete textSpeech;
	}
}

void cItem::talk( const Q_UINT32 MsgID, const QString& params, const QString& affix, bool prepend, UI16 color, cUOSocket* socket )
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
		for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
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
			for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
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

QPtrList< cItem > cItem::getContainment() const
{
	ContainerContent containment = content();
	ContainerContent::iterator it = containment.begin();
	QPtrList<cItem> itemlist;

	while ( it != containment.end() )
	{
		P_ITEM pItem = *it;

		// we'v got a container
		if ( pItem->type() == 1 || pItem->type() == 63 )
		{
			QPtrList<cItem> sublist = pItem->getContainment();

			// Transfer the items
			QPtrListIterator<cItem> pit( sublist );
			P_ITEM pi;
			while ( ( pi = pit.current() ) )
			{
				itemlist.append( pi );
				++pit;
			}
		}
		// Or just put it into our list
		else
			itemlist.append( pItem );

		++it;
	}

	return itemlist;
}

unsigned char cItem::classid;

static FactoryRegistration<cItem> registration("cItem");

void cItem::load( char** result, Q_UINT16& offset )
{
	cUObject::load( result, offset ); // Load the items we inherit from first

	// Broken Serial?
	if ( !isItemSerial( serial() ) )
		throw QString( "Item has invalid character serial: 0x%1" ).arg( serial(), 0, 16 );

	id_ = atoi( result[offset++] );
	color_ = atoi( result[offset++] );

	SERIAL containerSerial = atoi( result[offset++] );

	layer_ = atoi( result[offset++] );
	amount_ = atoi( result[offset++] );
	hp_ = atoi( result[offset++] );
	maxhp_ = atoi( result[offset++] );
	movable_ = atoi( result[offset++] );
	ownserial_ = atoi( result[offset++] );
	visible_ = atoi( result[offset++] );
	priv_ = atoi( result[offset++] );
	basedef_ = ItemBaseDefs::instance()->get( result[offset++] );

	// Their own weight should already be set.
	totalweight_ = amount_ * weight();

	//  Warning, ugly optimization ahead, if you have a better idea, we want to hear it.
	//  For load speed and memory conservation, we will store the SERIAL of the container
	//  here and then right after load is done we replace that value with it's memory address
	//  as it should be.
	if ( containerSerial != INVALID_SERIAL )
	{
		container_ = reinterpret_cast<cUObject*>( containerSerial );
		setUnprocessed( true );
	}
	// ugly optimization ends here.
	World::instance()->registerObject( this );
}

void cItem::buildSqlString( const char *objectid, QStringList& fields, QStringList& tables, QStringList& conditions )
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

	if ( pItem == this )
	{
		Console::instance()->log( LOG_WARNING, QString( "Rejected putting an item into itself (%1)" ).arg( serial_, 0, 16 ) );
		return;
	}

	if ( pItem->multi() )
	{
		if ( !pItem->unprocessed() )
		{
			pItem->multi()->removeObject( this );
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

	content_.push_back( pItem );
	pItem->layer_ = 0;
	pItem->container_ = this;

	if ( handleWeight )
	{
		// Increase the totalweight upward recursively
		setTotalweight( totalweight() + pItem->totalweight() );
	}

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
	//ContainerContent::iterator it = std::find(content_.begin(), content_.end(), pItem);
	ContainerContent::iterator it = content_.begin();
	while ( it != content_.end() )
	{
		if ( ( *it ) == pItem )
		{
			content_.erase( it );
			if ( handleWeight )
			{
				setTotalweight( this->totalweight() - pItem->totalweight() );
			}
			break;
		}
		++it;
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

cItem::ContainerContent cItem::content() const
{
	return content_;
}

bool cItem::contains( const cItem* pItem ) const
{
	ContainerContent::const_iterator it = std::find( content_.begin(), content_.end(), pItem );
	return it != content_.end();
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

Q_UINT16 cItem::getWeaponSkill()
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
		setBaseid( value.toString().latin1() );
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
	else
		SET_INT_PROPERTY( "layer", layer_ )
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
			P_CHAR pChar = pCont->getOutmostChar();

			if ( pChar )
				setPos( pChar->pos() );
			else
				setPos( pCont->pos() );

			removeFromCont();
			MapObjects::instance()->add( this );
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

PyObject* cItem::getProperty( const QString& name )
{
	PY_PROPERTY( "id", id_ )
	/*
	\rproperty item.lightsource For lightsources this is the type of the light.

	This property is inherited from the definition specified in the baseid property.
	*/
	PY_PROPERTY( "lightsource", lightsource() )
	/*
	\rproperty item.decaydelay The decay delay for this item in miliseconds.

	This is 0 if the item won't decay.
	*/
	PY_PROPERTY( "decaydelay", ( int ) decayDelay() )

	PY_PROPERTY( "baseid", baseid() )
	PY_PROPERTY( "color", color_ )
	PY_PROPERTY( "amount", amount_ )
	PY_PROPERTY( "layer", layer_ )
	/*
	\rproperty item.type The type value of an object. Used to group weapons,
	armor and other equipables, as well as usable objects.

	This property is inherited from the definition specified in the baseid property.
	*/
	PY_PROPERTY( "type", type() )
	/*
	\rproperty item.weight The weight value of the object.

	This property is inherited from the definition specified in the baseid property.
	*/
	PY_PROPERTY( "weight", weight() )
	/*
	\rproperty item.sellprice The value at which this object can be sold to vendors.

	This property is inherited from the definition specified in the baseid property.
	*/
	PY_PROPERTY( "sellprice", ( int ) sellprice() )
	/*
	\rproperty item.buyprice The value at which this object is bought from vendors.

	This property is inherited from the definition specified in the baseid property.
	*/
	PY_PROPERTY( "buyprice", ( int ) buyprice() )

	PY_PROPERTY( "health", hp_ )
	PY_PROPERTY( "maxhealth", maxhp_ )
	PY_PROPERTY( "owner", owner() )
	PY_PROPERTY( "totalweight", totalweight_ )
	// container
	PY_PROPERTY( "container", container_ )
	// Visible
	PY_PROPERTY( "visible", visible_ == 0 ? 1 : 0 )
	PY_PROPERTY( "ownervisible", visible_ == 1 ? 1 : 0 )
	// Flags
	PY_PROPERTY( "dye", dye() ? 1 : 0 )
	PY_PROPERTY( "decay", priv_ & 0x01 ? 0 : 1 )
	PY_PROPERTY( "newbie", priv_ & 0x02 ? 1 : 0 )
	PY_PROPERTY( "dispellable", priv_ & 0x04 ? 1 : 0 )
	PY_PROPERTY( "secured", priv_ & 0x08 ? 1 : 0 )
	PY_PROPERTY( "allowmeditation", priv_ & 0x10 ? 1 : 0 )
	PY_PROPERTY( "twohanded", priv_ & 0x20 ? 1 : 0 )
	/*
	\rproperty item.corpse Specifies whether this item is a corpse or not.
	*/
	PY_PROPERTY( "corpse", corpse() )
	PY_PROPERTY( "visible", visible() )
	PY_PROPERTY( "visible", visible_ == 0 ? 1 : 0 )
	PY_PROPERTY( "ownervisible", visible_ == 1 ? 1 : 0 )
	PY_PROPERTY( "movable", movable_ )
	/*
	\rproperty item.watersource This property indicates that this type of item is a source of
	water. If there is a "quantity" tag for the item, it should be used, otherwise the source
	is indepletable.

	This property is inherited from the base id of this item.
	*/
	PY_PROPERTY( "watersource", isWaterSource() )
	/*
	\rproperty item.basescripts This is a comma separated list of scripts assigned to this item
	via the baseid. They are called after the scripts assigned dynamically to the item.
	*/
	PY_PROPERTY( "basescripts", basedef_ ? basedef_->baseScriptList() : "" );

	return cUObject::getProperty( name );
}

void cItem::sendTooltip( cUOSocket* mSock )
{
	// There is a list of statically overridden items in the client (@50A1C0 for 4.0.0o)
	unsigned short id = this->id();

	// Mostly Signs (not movable but still have tooltips shown)
	if ( ( id >= 0xba3 && id <= 0xc0e ) ||    // House Signs
		( id >= 0x1297 && id <= 0x129e ) ||    // Road Signs
		( id >= 0x3e4a && id <= 0x3e55 ) ||    // Tillermen
		( id >= 0xed4 && id <= 0xede ) ||    // Graves and Guildstones
		( id >= 0x1165 && id <= 0x1184 ) ||    // More Gravestones
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
		if ( tile.flag3 & 0x20 == 0 )
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
		nItem->setBaseid( id.latin1() );
		nItem->applyDefinition( section );
		cDelayedOnCreateCall* onCreateCall = new cDelayedOnCreateCall( nItem, id );
		Timers::instance()->insert( onCreateCall );
	}
	else
	{
		Console::instance()->log( LOG_ERROR, QString( "Unable to create unscripted item: %1\n" ).arg( id ) );
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
		pItem->setBaseid( QString::number( id, 16 ).latin1() );
	}

	return pItem;
}

void cItem::createTooltip( cUOTxTooltipList& tooltip, cPlayer* player )
{
	cUObject::createTooltip( tooltip, player );

	// Add the object name.
	if ( amount_ > 1 )
	{
		if ( name_.isEmpty() )
			tooltip.addLine( 1050039, QString( "%1\t#%2" ).arg( amount_ ).arg( 1020000 + id_ ) );
		else
			tooltip.addLine( 1050039, QString( "%1\t%2" ).arg( amount_ ).arg( name_ ) );
	}
	else
	{
		if ( name_.isEmpty() )
			tooltip.addLine( 1042971, QString( "#%2" ).arg( 1020000 + id_ ) );
		else
			tooltip.addLine( 1042971, name_ );
	}

	// For containers (hardcoded type), add count of items and total weight.
	if ( type() == 1 )
	{
		unsigned int count = content_.size();
		unsigned int weight = ( unsigned int ) floor( totalweight_ );
		tooltip.addLine( 1050044, QString( "%1\t%2" ).arg( count ).arg( weight ) );
	}

	// Newbie Items
	if ( newbie() )
		tooltip.addLine( 1038021, "" );

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
	if ( id() != pItem->id() || color() != pItem->color() || type() == 1 || type() != pItem->type() || bindmenu() != pItem->bindmenu() || scriptList() != pItem->scriptList() || baseid() != pItem->baseid() )
	{
		return false;
	}

	// Check Tags (rather expensive)
	if ( tags_ != pItem->tags_ )
	{
		return false;
	}

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

	ContainerContent::const_iterator it(content_.begin());
	ContainerContent::const_iterator end(content_.end());
	for ( ; it != end; ++it )
	{
		count += ( *it )->countItems( baseids );
	}
	return count;
}

unsigned int cItem::countItems( short id, short color ) const
{
	unsigned int total = 0;
	QPtrList<cItem> content = getContainment();

	for ( P_ITEM pi = content.first(); pi; pi = content.next() )
	{
		if ( !pi || pi->free )			// just to be sure ;-)
			continue;
		if ( pi->id() == id && ( color == -1 || pi->color() == color ) )
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

	if ( content().size() > 0 )
	{
		ContainerContent content( this->content() );
		ContainerContent::iterator it = content.begin();
		while ( amount > 0 && it != content.end() )
		{
			amount = ( *it )->removeItems( baseids, amount );
			++it;
		}
	}

	return amount;
}

void cItem::moveTo( const Coord_cl& newpos, bool noremove )
{
	// See if the map is valid
	if ( !Maps::instance()->hasMap( newpos.map ) )
	{
		return;
	}

	if ( container_ )
	{
		pos_ = newpos;
		changed_ = true;
	}
	else
	{
		cUObject::moveTo( newpos, noremove );
	}
}

bool cItem::isInLockedItem()
{
	if ( container_ && container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast<P_ITEM>( container_ );

		if ( pCont->hasScript( "lock" ) )
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
	if ( container_ || nodecay() || multi_ )
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
	cUObject::save(writer);

	// Save container content
	ContainerContent::iterator it = content_.begin();
	for (; it != content_.end(); ++it) {
		(*it)->save(writer);
	}
}

void cItem::load( cBufferedReader& reader )
{
	load( reader, reader.version() );

	World::instance()->registerObject( this );

	if ( !container_ )
	{
		SectorMaps::instance()->add( this );
	}
}

bool cItem::callEventHandler(ePythonEvent event, PyObject *args, bool ignoreErrors) {
	PyObject *result = callEvent(event, args, ignoreErrors);

	if (result) {
		if (PyObject_IsTrue(result)) {
			Py_DECREF(result);
			return true;
		} else {
			Py_DECREF(result);
		}
	}
	return false;
}

PyObject *cItem::callEvent(ePythonEvent event, PyObject *args, bool ignoreErrors) {
	PyObject *result = 0;

	if (scriptChain) {
		result = cPythonScript::callChainedEvent(event, scriptChain, args);
	
		// Break if there has been a result already
		if (result && PyObject_IsTrue(result)) {
			return result;
		}
	}

	// call the basescripts
	if (basedef_) {
		const QPtrList<cPythonScript> &list = basedef_->baseScripts();
		QPtrList<cPythonScript>::const_iterator it(list.begin());
		for (; it != list.end(); ++it) {
			result = (*it)->callEvent(event, args, ignoreErrors);

			if (result && PyObject_IsTrue(result)) {
				return result;
			}
		}
	}

	// check for a global handler
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook(event);

	if (globalHook) {
		result = globalHook->callEvent(event, args, ignoreErrors);
	}

	return result;
}

bool cItem::canHandleEvent(ePythonEvent event) {
	// Is there a global event?
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook(event);
	
	if (globalHook) {
		return true;
	}

	if (cPythonScript::canChainHandleEvent(event, scriptChain)) {
		return true;
	}

	if (basedef_) {
		const QPtrList<cPythonScript> &list = basedef_->baseScripts();
		QPtrList<cPythonScript>::const_iterator it(list.begin());
		for (; it != list.end(); ++it) {
			if ((*it)->canHandleEvent(event)) {
				return true;
			}
		}
	}

	return false;
}

bool cItem::hasScript( const QCString& name )
{
	if (basedef_) {
		const QPtrList<cPythonScript> &list = basedef_->baseScripts();
		QPtrList<cPythonScript>::const_iterator it(list.begin());
		for (; it != list.end(); ++it) {
			if ((*it)->name() == name) {
				return true;
			}
		}
	}

	return cUObject::hasScript(name);
}
