//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "accounts.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "items.h"
#include "tilecache.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "pythonscript.h"
#include "maps.h"
#include "network.h"
#include "log.h"
#include "multis.h"
#include "persistentbroker.h"
#include "guilds.h"
#include "dbdriver.h"
#include "world.h"
#include "sectors.h"
#include "scriptmanager.h"
#include "itemid.h"
#include "basechar.h"
#include "player.h"
#include "basics.h"
#include "srvparams.h"
#include "globals.h"
#include "inlines.h"
#include "console.h"

// System Includes
#include <math.h>
#include <algorithm>

using namespace std;

/*****************************************************************************
  cItem member functions
 *****************************************************************************/

// constructor
cItem::cItem(): container_(0), totalweight_(0), sellprice_( 0 ),
buyprice_( 0 ), restock_( 1 ), baseid_(QString::null)
{
	spawnregion_ = QString::null;
	Init( false );
};

cItem::cItem( const cItem &src ) {
	// Copy Events
	scriptChain = 0;
	eventList_ = src.eventList_;
	recreateEvents();
	this->multis_ = src.multis_;
	this->name_ = src.name_;
	this->pos_  = src.pos_;
	this->tags_ = src.tags_;
	//cItem properties setting
	this->serial_ = INVALID_SERIAL; // IMPORTANT
	this->amount_ = src.amount_;
	this->buyprice_ = src.buyprice_;
	this->changed( TOOLTIP );
	this->color_ = src.color_;
	this->decaytime_ = src.decaytime_;
	this->def_ = src.def_;
	this->flagChanged();
	this->free = false;
	this->hidamage_=src.hidamage_;
	this->hp_ = src.hp_;
	this->lodamage_=src.lodamage_;
	this->magic_ = src.magic_;
	this->maxhp_ = src.maxhp_;
	this->priv_=src.priv_;
	this->restock_ = src.restock_;
	this->sellprice_ = src.sellprice_;
	this->setId(src.id());
	this->setOwnSerialOnly(src.ownSerial());
	this->spawnregion_=src.spawnregion_;
	this->speed_=src.speed_;
	this->type2_ = src.type2_;
	this->type_ = src.type_;
	this->visible_=src.visible_;
	this->weight_ = src.weight_;
	this->baseid_ = src.baseid_;
	this->totalweight_ = ceilf( amount_ * weight_ * 100 ) / 100;
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
	return tile.flag2&0x08;
}

void cItem::toBackpack( P_CHAR pChar )
{
	flagChanged();
	P_ITEM pPack = pChar->getBackpack();

	// Pack it to the ground
	if( !pPack )
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

///////////////
// Name:	ReduceAmount
// history:	by Duke, 4.06.2000
//			added P_ITEM interface Duke, 3.10.2000
//			made it the first member of cItem Duke, 23.12.2000
// Purpose:	reduces the given item's amount by 'amt' and deletes it if
//			necessary and returns 0. If the request could not be fully satisfied,
//			the remainder is returned
//
long cItem::reduceAmount( const short amt )
{
	UINT16 rest = 0;
	if( amount_ > amt )
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

void cItem::setOwnSerialOnly(int ownser)
{
	flagChanged();
	ownserial_ = ownser;
}

void cItem::SetOwnSerial(int ownser)
{
	flagChanged();
	setOwnSerialOnly(ownser);
}

void cItem::SetMultiSerial(long mulser)
{
	flagChanged();
	this->setMultis(mulser);
}

void cItem::MoveTo(int newx, int newy, signed char newz)
{
	moveTo( Coord_cl(newx, newy, newz, pos().z, pos().map ) );
}

// author: LB purpose: returns the type of pack
// to handle its x,y coord system corretly.
// interpretation of the result:
// valid x,y ranges depending on type:
// type -1 : no pack
// type  1 : y-range 50  .. 100
// type  2 : y-range 30  .. 80
// type  3 : y-range 100 .. 150
// type  4 : y-range 40  .. 140
// x-range 18 .. 118 for 1,2,3
//         40 .. 140 for 4
//
short cItem::GetContGumpType()
{
	switch (id())
	{
	case 0x09a8: return 1;
	case 0x09a9: return 2;
	case 0x09aa: return 1;
	case 0x09ab: return 3;
	case 0x09b0: return 1;
	case 0x09b2: return 4;

	case 0x0A2C: return 1;	// chest of drawers
	case 0x0A30: return 1;	// chest of drawers
	case 0x0A34: return 1;	// chest of drawers
	case 0x0A38: return 1;	// chest of drawers

	case 0x0A4D: return 1;	// armoire
	case 0x0A4F: return 1;	// armoire
	case 0x0A51: return 1;	// armoire
	case 0x0A53: return 1;	// armoire

	case 0x0A97: return 1;	// bookshelf
	case 0x0A98: return 1;	// bookshelf
	case 0x0A99: return 1;	// bookshelf
	case 0x0A9A: return 1;	// bookshelf
	case 0x0A9B: return 1;	// bookshelf
	case 0x0A9C: return 1;	// bookshelf
	case 0x0A9D: return 1;	// bookshelf
	case 0x0A9E: return 1;	// bookshelf

	case 0x0e3c: return 2;
	case 0x0e3d: return 2;
	case 0x0e3e: return 2;
	case 0x0e3f: return 2;
	case 0x0e40: return 3;
	case 0x0e41: return 3;
	case 0x0e42: return 3;
	case 0x0e43: return 3;
	case 0x0e75: return 4;

	case 0x0e76: return 1;
	case 0x0e77: return 4;
	case 0x0e78: return 2;
	case 0x0e79: return 1;
	case 0x0e7a: return 1;

	case 0x0e7c: return 3;
	case 0x0e7d: return 1;
	case 0x0e7e: return 2;
	case 0x0e7f: return 4;
	case 0x0e80: return 1;
	case 0x0e83: return 4;

	case 0x2006: return 5;	// a corpse/coffin
	default: return -1;
	}
}

bool cItem::PileItem(cItem* pItem)	// pile two items
{
	if (!(isPileable() && pItem->isPileable() &&
		this->serial()!=pItem->serial() &&
		this->id()==pItem->id() &&
		this->color() == pItem->color() ))
		return false;	//cannot stack.

	if (this->amount() + pItem->amount() > 65535)
	{
		Coord_cl position = pItem->pos();
		position.x = this->pos().x;
		position.y = this->pos().y;
		position.z = 9;
		pItem->setPos( position );
		pItem->setAmount( (this->amount()+pItem->amount()) - 65535 );
		this->setAmount( 65535 );
		pItem->update();
	}
	else
	{
		this->setAmount( this->amount()  + pItem->amount() );
		pItem->remove();
	}

	changed( TOOLTIP );
	flagChanged();
	update();
	return true;
}

bool cItem::ContainerPileItem(cItem* pItem)	// try to find an item in the container to stack with
{
	cItem::ContainerContent::const_iterator it(content_.begin());
	cItem::ContainerContent::const_iterator end(content_.end());
	for( ; it != end; ++it )
	{
		P_ITEM pi = *it;
		if (pi->id() == pItem->id() && !pi->free && pi->color() == pItem->color())
			if( pi->PileItem( pItem ) )
				return true;
	}
	return false;
}

void cItem::SetRandPosInCont(cItem* pCont)
{
	int k = pCont->GetContGumpType();
	Coord_cl position = pos();
	position.x = RandomNum(18, 118);
	position.z=9;

	switch (k)
	{
	case 1: position.y = RandomNum(50, 100);	break;
	case 2: position.y = RandomNum(30, 80);		break;
	case 3: position.y = RandomNum(100, 140);	break;
	case 4: position.y = RandomNum(60, 140);
			position.x = RandomNum(60, 140);	break;
	case 5: position.y = RandomNum(85, 160);
			position.x = RandomNum(20, 70);		break;
	default: position.y = RandomNum(30, 80);
	}
	setPos( position );
}

/*!
  \internal
  This function will search the given container and subcontainers by serial
  and return the sum of items found by the given pair of ID (model number) and
  color.
*/
static int ContainerCountItems(const int serial, short id, short color)
{
	int total = 0;
	P_ITEM container = FindItemBySerial( serial );
	QPtrList<cItem> content = container->getContainment();

	for( P_ITEM pi = content.first(); pi; pi = content.next() )
	{
		if (!pi || pi->free)			// just to be sure ;-)
			continue;
		if (pi->id() == id && ( color == -1 || pi->color() == color ) )
			total += pi->amount();
	}

	return total;
}


int cItem::CountItems(short ID, short col) const
{
	return ContainerCountItems(serial(), ID, col);
}

///////////////////////
// Name:	DeleteAmount
// history:	DeleQuan() by Duke, 16.11.2000
//			moved to cItem (Duke,27.3.2001)
// Purpose:	recurses through the container given by serial and deletes items of
//			the given id and color(if given) until the given amount is reached
//
int cItem::DeleteAmount(int amount, unsigned short _id, unsigned short _color)
{
	int rest=amount;
	P_ITEM pi;
	cItem::ContainerContent container(this->content());
	cItem::ContainerContent::const_iterator it ( container.begin() );
	cItem::ContainerContent::const_iterator end( container.end()   );
	for ( ; it != end; ++it )
	{
		pi = *it;
		if (pi->type() == 1)
			rest = pi->DeleteAmount(rest, _id, _color);
		if (pi->id() == _id && ( _color == 0 || ( pi->color() == _color ) ) )
			rest=pi->reduceAmount(rest);
		if (rest<=0)
			break;
	}
	return rest;
}

void cItem::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "items" );

		addField("serial",			serial() );
		addField("id",				id() );
		addField("color",			color() );
		SERIAL contserial = INVALID_SERIAL;
		if ( container_ )
			contserial = container_->serial();
		addField("cont",			contserial);
		addField("layer",			layer_);
		addField("type",			type_);
		addField("type2",			type2_);
		addField("amount",			amount_);
		addField("decaytime",		(decaytime_ > uiCurrentTime) ? decaytime_ - uiCurrentTime : 0	);
		addField("def",				def_);
		addField("hidamage",		hidamage_);
		addField("lodamage",		lodamage_);
		addField("weight",			( ceilf( weight_ * 100 ) / 100 ) );
		addField("hp",				hp_ );
		addField("maxhp",			maxhp_ );
		addField("speed",			speed_ );
		addField("magic",			magic_ );
		addField("owner",			ownserial_ );
		addField("visible",			visible_ );
		addStrField("spawnregion",	spawnregion_ );
		addField("priv",			priv_ );
		addField("sellprice",		sellprice_ );
		addField("buyprice",		buyprice_ );
		addField("restock",			restock_ );
		addStrField("baseid",		baseid_ );

		addCondition( "serial", serial() );
		saveFields;
	}
	cUObject::save();
}

static void itemRegisterAfterLoading( P_ITEM pi );

bool cItem::del()
{
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "items", QString( "serial = '%1'" ).arg( serial() ) );
	flagChanged();
	return cUObject::del();
}

QString cItem::getName( bool shortName )
{
	if( !name_.isNull() )
		return name_;

	tile_st tile = TileCache::instance()->getTile( id_ );

	QString itemname = tile.name;

	// NEVER prepend a or at when shortName is true (You destroy your a shirt sounds stupid doesnt it)
	if( !shortName )
	{
		if( tile.flag2 & 0x80 )
			itemname.prepend( "an " );
		else
			itemname.prepend( "a " );
	}

	// Now "parse" the %/% information
	if( itemname.contains( "%" ) )
	{
		// Test if we have a %.../...% form or a simple %
		QRegExp simple( "%([^/]+)%" );
		simple.setMinimal( TRUE );

		if( itemname.contains( simple ) )
			itemname.replace( simple, simple.cap( 1 ) );
	}

	return itemname;
}

void cItem::setSerial( const SERIAL ser )
{
	if( ser == INVALID_SERIAL )
		return;

	if( serial() != INVALID_SERIAL )
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

	if( createSerial )
		this->setSerial( World::instance()->findItemSerial() );

	this->container_ = 0;

	this->setMultis( INVALID_SERIAL ); //Multi serial
	this->free = false;
	this->setId( 0x0001 ); // Item visuals as stored in the client
	this->setPos( Coord_cl(100, 100, 0) );
	this->color_ = 0x00; // Hue
	this->layer_ = 0; // Layer if equipped on paperdoll
	this->type_=0; // For things that do special things on doubleclicking
	this->type2_=0;
	this->weight_ = 0;
	this->amount_ = 1; // Amount of items in pile
	this->def_=0; // Item defense
	this->lodamage_=0; //Minimum Damage weapon inflicts
	this->hidamage_=0; //Maximum damage weapon inflicts
	this->hp_=0; //Number of hit points an item has.
	this->maxhp_=0; // Max number of hit points an item can have.
	this->speed_=0; //The speed of the weapon
	this->magic_ = 0; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable.
	this->decaytime_ = 0;
	this->setOwnSerialOnly(-1);
	this->visible_=0; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	this->priv_ = 0; // Bit 0, nodecay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
}

/*!
	\brief Removes this item from the game world.
*/
void cItem::remove()
{
	// Already Deleted
	if (free) {
		return;
	}
	
	removeFromView(false); // Remove it from all clients in range
	
	// Update Top Objects
	setSpawnRegion(QString::null);
	SetOwnSerial(-1);

	// Check if this item is registered as a guildstone and remove it
	// from the container if neccesary.
	for (cGuilds::iterator it = Guilds::instance()->begin(); it != Guilds::instance()->end(); ++it) {
		cGuild *guild = it.data();
		if (guild->guildstone() == this) {
			guild->setGuildstone(0);
		}
	}

	// Remove from the sector map if its a world item
	// Otherwise check if there is a top container
	if (container() && !container()->free) {
		removeFromCont();
	} else {
		SectorMaps::instance()->remove(this);
	}

	// Create a copy of the content so we don't accidently change our working copy
	ContainerContent container(content()); 
	ContainerContent::const_iterator it2;
	for (it2 = container.begin(); it2 != container.end(); ++it) {
		(*it2)->remove();
	}

	// Remove us from a multi container
	if (multis() != INVALID_SERIAL) {
		cMulti *pMulti = dynamic_cast<cMulti*>(World::instance()->findItem(multis()));

		if (pMulti) {
			pMulti->removeItem(this);
		}
	}

	// Queue up for deletion from worldfile
	World::instance()->deleteObject(this);
}

void cItem::startDecay()
{
	if( container_ || nodecay() )
		return;

//	flagChanged();

	decaytime_ = uiCurrentTime;

	// Player corpses take longer to decay
	if( !corpse() )
	{
		decaytime_ += SrvParams->itemDecayTime() * MY_CLOCKS_PER_SEC;
	}
	else
	{
		decaytime_ += SrvParams->corpseDecayTime() * MY_CLOCKS_PER_SEC;
	}
}

void cItem::decay( unsigned int currenttime )
{
	// Locked Down Items, NoDecay Items and Items in Containers can never decay
	// And ofcourse items in multis cannot
	// Static/Nevermovable items can't decay too
	if (container() || nodecay() || isLockedDown() || multis() != INVALID_SERIAL || magic_ >= 2) {
		return;
	}

	// Start decaying
	if( !decaytime() )
	{
		startDecay();
		return;
	}

	// The Item is about to decay
	if( decaytime() <= currenttime )
	{
		// If it's a corpse and the items should not decay along with
		// it, then place them at the corpse's position
		if( corpse() && !SrvParams->lootdecayswithcorpse() )
		{
			cItem::ContainerContent container( content() );
			cItem::ContainerContent::const_iterator it (container.begin() );
			cItem::ContainerContent::const_iterator end(container.end());
			for (; it != end; ++it )
			{
				P_ITEM pItem = *it;
				pItem->removeFromCont( true );
				pItem->moveTo( pos() );
				pItem->update();
			}
		}

		remove();
	}
}

void cItem::respawn( unsigned int currenttime )
{
/*void cAllItems::RespawnItem( UINT32 currenttime, P_ITEM pItem )
{
	if( !pItem || pItem->free )
		return;

	// Not ready to respawn yet
	if( pItem->gatetime() > currenttime )
		return;

	switch( pItem->type() )
	{
	// Item Spawner
	case 61:
		{
			// Check if it's worth respawning
			vector< SERIAL > spawned = spawnsp.getData( pItem->serial() );
			UINT32 amount = spawned.size();

			// Do a sanity check
			for( UINT32 i = 0; i < spawned.size(); ++i )
			{
				P_ITEM pSpawned = FindItemBySerial( spawned[i] );

				// Item has been deleted
				if( !pSpawned )
				{
					--amount;

				}
				// Item has been moved
				else if( ( pSpawned->free ) || ( pSpawned->pos() != pItem->pos() ) )
				{
					pSpawned->SetSpawnSerial( INVALID_SERIAL );
					--amount;
				}
			}

			// Respawn the item
			// morex = min Time in secs till next respawn
			// morey = max Time in secs till next respawn
			// carve = item-section
			if( amount < pItem->amount() )
			{
				// Create the spawned item @ our position
				P_ITEM pSpawned = cItem::createFromScript( pItem->carve() );

				if( !pSpawned )
				{
					Console::instance()->send( tr( "Unable to spawn unscripted item: %1" ).arg( pItem->carve() ) );
					break;
				}

				pSpawned->moveTo( pItem->pos() );
				pSpawned->SetSpawnSerial( pItem->serial() );
				pSpawned->update();
			}
		}
		break;
	// NPC Spawner
	case 62:
		{
			// Check if it's worth respawning
			vector< SERIAL > spawned = spawnsp.getData( pItem->serial() );
			UINT32 amount = spawned.size();

			// Do a sanity check
			for( UINT32 i = 0; i < spawned.size(); ++i )
			{
				P_CHAR pSpawned = FindCharBySerial( spawned[i] );

				// Char has been deleted
				if( !pSpawned )
				{
					spawnsp.remove( pItem->serial(), pSpawned->serial() );
					--amount;
				}
				// Char has been tamed/changed owner
				else if( pSpawned->tamed() )
				{
					--amount;
				}
			}

			// Is there anything to be spawned
			if( amount < pItem->amount() )
			{
				P_CHAR pSpawned = cCharStuff::createScriptNpc( pItem->carve(), pItem->pos() );
				if( pSpawned )
					pSpawned->SetSpawnSerial( pItem->serial() );
			}
		}
		break;
	};

	pItem->setGateTime(currenttime + ( RandomNum( pItem->morex(), pItem->morey() ) * MY_CLOCKS_PER_SEC ));

	*//*

	// Chest spawner
			else if ((pi->type()==63)||(pi->type()==64)||(pi->type()==65)||(pi->type()==66)||(pi->type()==8))
			{
				serial=pi->serial();
				unsigned int j;
				vector<SERIAL> vecContainer = contsp.getData(pi->serial());
				for (j=0;j<vecContainer.size();j++)
				{
					P_ITEM pi_ci = FindItemBySerial(vecContainer[j]);
					if (pi_ci != NULL)
					if (pi_ci->contserial == pi->serial() && !pi_ci->free)
					{
						m++;
					}
				}
				if(m<pi->amount())
				{
					if (pi->gatetime==0)
					{
						pi->gatetime=(rand()%((int)(1+((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +
							(pi->morey*MY_CLOCKS_PER_SEC*60)+uiCurrentTime;
					}
					if ((pi->gatetime<=currenttime ||(overflow)) && pi->morex!=0)
					{
						if(pi->type()==63)
							pi->setType( 64 ); //Lock the container
						//numtostr(pi->morex,m); //ilist); //LB, makes chest spawners using random Itemlist items instead of a single type, LB
						if(pi->morex)
							Items->AddRespawnItem(pi, QString("%1").arg(pi->morex), true);//If the item contains an item list then it will randomly choose one from the list, JM
						else
						{
							QString itemSect = DefManager->getRandomListEntry( "70" );
							Items->AddRespawnItem(pi, itemSect, true);
						}
						pi->gatetime=0;
					}
				}
			}
		}//If time
	}//for
}*/
/*
void cAllItems::AddRespawnItem(P_ITEM pItem, QString itemSect, bool spawnInItem )
{
	if (pItem == NULL)
		return;

	P_ITEM pi = cItem::createFromScript( itemSect ); // lb, bugfix
	if (pi == NULL) return;

	if( !spawnInItem )
	{
		pi->moveTo(pItem->pos()); //add spawned item to map cell if not in a container
	}
	else
	{
		pItem->addItem(pi);
	}
	pi->SetSpawnSerial(pItem->serial());

	if(spawnInItem)
	{
		P_ITEM pChest = NULL;
		if (pi->spawnserial!=-1)
			pChest=FindItemBySerial(pi->spawnserial);
		if (pChest)
		{
			pi->SetRandPosInCont(pChest);
		}
	}
	pi->update();//AntiChrist
}*/
}

bool cItem::onSingleClick( P_PLAYER Viewer )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetItemObject, this, PyGetCharObject, Viewer );
		result = cPythonScript::callChainedEventHandler( EVENT_SINGLECLICK, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onDropOnItem( P_ITEM pItem )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetItemObject, layer_ == 0x1E ? pItem : this, PyGetItemObject, layer_ == 0x1E ? this : pItem );
		result = cPythonScript::callChainedEventHandler( EVENT_DROPONITEM, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onDropOnGround( const Coord_cl &pos )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&N", PyGetItemObject, this, PyGetCoordObject( pos ) );
		result = cPythonScript::callChainedEventHandler( EVENT_DROPONGROUND, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onPickup( P_CHAR pChar )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = cPythonScript::callChainedEventHandler( EVENT_PICKUP, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onEquip( P_CHAR pChar, unsigned char layer )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&b", PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = cPythonScript::callChainedEventHandler( EVENT_EQUIP, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onBookUpdateInfo( P_CHAR pChar, const QString &author, const QString &title )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&uu", PyGetCharObject, pChar, PyGetItemObject, this, author.ucs2(), title.ucs2() );
		result = cPythonScript::callChainedEventHandler( EVENT_BOOKUPDATEINFO, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onBookRequestPage( P_CHAR pChar, unsigned short page )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&h", PyGetCharObject, pChar, PyGetItemObject, this, page );
		result = cPythonScript::callChainedEventHandler( EVENT_BOOKREQUESTPAGE, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onBookUpdatePage( P_CHAR pChar, unsigned short page, const QString &content )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&hu", PyGetCharObject, pChar, PyGetItemObject, this, page, content.ucs2() );
		result = cPythonScript::callChainedEventHandler( EVENT_BOOKUPDATEPAGE, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onUnequip( P_CHAR pChar, unsigned char layer )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&b", PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = cPythonScript::callChainedEventHandler( EVENT_UNEQUIP, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onWearItem( P_PLAYER pPlayer, P_CHAR pChar, unsigned char layer )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&O&b", PyGetCharObject, pPlayer, PyGetCharObject, pChar, PyGetItemObject, this, layer );
		result = cPythonScript::callChainedEventHandler( EVENT_WEARITEM, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onUse( P_CHAR pChar )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = cPythonScript::callChainedEventHandler( EVENT_USE, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}


bool cItem::onCollide( P_CHAR pChar )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = cPythonScript::callChainedEventHandler( EVENT_COLLIDE, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onDropOnChar( P_CHAR pChar )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, pChar, PyGetItemObject, this );
		result = cPythonScript::callChainedEventHandler( EVENT_DROPONCHAR, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cItem::onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_SHOWTOOLTIP );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&O&O&", PyGetCharObject, sender, PyGetItemObject, this, PyGetTooltipObject, tooltip );

		result = cPythonScript::callChainedEventHandler( EVENT_SHOWTOOLTIP, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_SHOWTOOLTIP, args );

		Py_DECREF( args );
	}

	return result;
}


void cItem::processNode( const cElement *Tag )
{
	flagChanged();
	// we do this as we're going to modify the element
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	const cElement *section = DefManager->getDefinition( WPDT_DEFINE, TagName );

	// <bindmenu>contextmenu</bindmenu>
	// <bindmenu id="contextmenu" />
	if( TagName == "bindmenu" )
	{
		if( Tag->hasAttribute( "id" ) )
			this->setBindmenu( Tag->getAttribute( "id" ));
		else
			setBindmenu(Value);
	}

	// <attack min="1" max="2"/>
	else if( TagName == "attack" )
	{
		if( Tag->hasAttribute( "min" ) )
			this->setLodamage( Tag->getAttribute( "min" ).toInt() );

		if( Tag->hasAttribute( "max" ) )
			this->setHidamage( Tag->getAttribute( "max" ).toInt() );

		// Better...
		if( this->lodamage() > this->hidamage() )
			this->setHidamage( this->lodamage() );
	}

	// for convenience
	// <food>1</food>
	else if( TagName == "food" )
	{
		setType( 14 );
		setType2( Value.toUInt() );
	}

	// <amount>10</amount>
	else if ( TagName == "amount" )
		this->setAmount( Value.toUShort() );

	// <weight>10</weight>
	else if( TagName == "weight" )
		this->setWeight( Value.toFloat() );

	// <durability>10</durabilty>
	else if( TagName == "durability" )
	{
		this->setMaxhp( Value.toLong() );
		this->setHp( this->maxhp() );
	}

	// <speed>10</speed>
	else if( TagName == "speed" )
		this->setSpeed( Value.toLong() );

	// <lightsource>10</lightsource>
	else if( TagName == "lightsource" )
		this->dir_ = Value.toUShort();

	// <movable />
	// <ownermovable />
	// <immovable />
	else if( TagName == "movable" )
		this->magic_ = 1;
	else if( TagName == "immovable" )
		this->magic_ = 2;
	else if( TagName == "ownermovable" )
		this->magic_ = 3;

	// <decay />
	// <nodecay />
	else if( TagName == "decay" )
		setNoDecay( false );
	else if( TagName == "nodecay" )
		setNoDecay( true );

	// <dispellable />
	// <notdispellable />
	else if( TagName == "dispellable" )
		this->priv_ |= 0x04;
	else if( TagName == "notdispellable" )
		this->priv_ &= 0xFB;

	// <newbie />
	// <notnewbie />
	else if( TagName == "newbie" )
		this->setNewbie( true );
	else if( TagName == "notnewbie" )
		this->setNewbie( false );

	// <twohanded />
	else if( TagName == "twohanded" )
		this->setTwohanded( true );

	// <singlehanded />
	else if( TagName == "singlehanded" )
		this->setTwohanded( false );

	// <visible />
	// <invisible />
	// <ownervisible />
	else if( TagName == "invisible" )
		this->visible_ = 2;
	else if( TagName == "visible" )
		this->visible_ = 0;
	else if( TagName == "ownervisible" )
		this->visible_ = 1;

	// <dye />
	// <nodye />
	else if( TagName == "dye" )
		this->setDye( true );
	else if( TagName == "nodye" )
		this->setDye( false );

	// <corpse />
	// <nocorpse />
	else if( TagName == "corpse" )
		this->setCorpse( true );
	else if( TagName == "nocorpse" )
		this->setCorpse( false );

	// <id>0x12f9</id>
	else if( TagName == "id" )
	{
		this->setId( Value.toUShort() );

		// In addition to the normal behaviour we retrieve the weight of the
		// item here.
		setWeight( TileCache::instance()->getTile( id_ ).weight );
	}

	// <content><item id="a" />...<item id="z" /></contains> (sereg)
	else if( TagName == "content" && Tag->childCount() > 0 )
		this->processContainerNode( Tag );

	// <inherit>f23</inherit>
	else if( TagName == "inherit" )
	{
		const cElement *section;

		if( Tag->hasAttribute( "id" ) )
			section = DefManager->getDefinition( WPDT_ITEM, Tag->getAttribute( "id" ) );
		else
			section = DefManager->getDefinition( WPDT_ITEM, Value );

		if( section )
			applyDefinition( section );
	}

	else if( section )
	{
		for( unsigned int i = 0; i < section->childCount(); ++i )
			processModifierNode( section->getChild( i ) );
	}
	else
		cUObject::processNode( Tag );

}

void cItem::processModifierNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	// <name>magic %1</name>
	if( TagName == "name" )
	{
		// Bad: # = iron #
		if( name_.isNull() )
		{
			name_ = getName( true );
		}

		// This prevents double naming issues (magic magic item)
		// magic %1 | magic item
		// This is rather tough i'd say, we have to check whether we already
		// have the prefix *OR* suffix
		if( !Value.contains( "%1" ) )
		{
			name_ = Value;
		}
		else
		{
			/*int offset = Value.find( "%1" );
			QString left = Value.left( offset );
			QString right = Value.right( Value.length() - ( offset + 2 ) );
			name_ = left + name_ + right;*/
			name_ = Value.arg( name_ );
		}
	}

	// <attack min="-1" max="+2"/>
	else if( TagName == "attack" )
	{
		if( Tag->hasAttribute( "min" ) )
		{
			Value = Tag->getAttribute("min");
			if( Value.contains(".") || Value.contains(",") )
				setLodamage( (UINT16)ceil((float)lodamage() * Value.toFloat()) );
			else
				setLodamage( lodamage() + Value.toInt() );
		}

		if( Tag->hasAttribute( "max" ) )
		{
			Value = Tag->getAttribute("max");
			if( Value.contains(".") || Value.contains(",") )
				setHidamage( (UINT16)ceil((float)hidamage() * Value.toFloat()) );
			else
				setHidamage( hidamage() + Value.toInt() );
		}

		// Better...
		if( lodamage() > hidamage() )
			setHidamage( lodamage() );
	}

	// <defense>+10</defense>
	else if( TagName == "defense" )
	{
		if( Value.contains(".") || Value.contains(",") )
			def_ = (UINT32)ceil((float)def_ * Value.toFloat());
		else
			def_ += Value.toUInt();
	}

	// <weight>-10</weight>
	else if( TagName == "weight" )
	{
		if( Value.contains(".") || Value.contains(",") )
			setWeight( ceilf( (float)weight() * Value.toFloat() * 100 ) / 100 );
		else
			setWeight( (float)weight() + Value.toFloat() );
	}

	// <sellprice>+20</sellprice>
	else if( TagName == "sellprice" )
	{
		if( Value.contains(".") || Value.contains(",") )
			sellprice_ = (INT32)ceil((float)sellprice_ * Value.toFloat());
		else
			sellprice_ += Value.toInt();
	}

	// <buyprice>+20</buyprice>
	else if( TagName == "buyprice" )
	{
		if( Value.contains(".") || Value.contains(",") )
			buyprice_ = (INT32)ceil((float)buyprice_ * Value.toFloat());
		else
			buyprice_ += Value.toInt();
	}

	// <durability>-10</durabilty>
	else if( TagName == "durability" )
	{
		if( Value.contains(".") || Value.contains(",") )
			setMaxhp( (INT32)ceil((float)maxhp() * Value.toFloat()) );
		else
			setMaxhp( maxhp() + Value.toLong() );
		setHp( maxhp() );
	}

	// <speed>-10</speed>
	else if( TagName == "speed" )
	{
		if( Value.contains(".") || Value.contains(",") )
			setSpeed( (INT32)ceil((float)speed() * Value.toFloat()) );
		else
			setSpeed( speed() + Value.toLong() );
	}

	else
		cUObject::processNode( Tag );
}

void cItem::processContainerNode( const cElement *tag )
{
	//item containers can be scripted like this:
	/*
	<contains>
		<item list="myList" />
		<item id="myItem1"><amount><random ... /></amount><color><colorlist><random...></colorlist></color></item>
		...
	</contains>
	*/
	for( unsigned int i = 0; i < tag->childCount(); ++i )
	{
		const cElement *element = tag->getChild( i );

		if( element->name() == "item" )
		{
			if( element->hasAttribute( "id" ) )
			{
				cItem* nItem = cItem::createFromScript( element->getAttribute("id") );
				addItem( nItem, true, false );
				for ( unsigned int j = 0; j < element->childCount(); ++j )
					nItem->processNode( element->getChild( j ) );
				if ( this->layer() == cBaseChar::BuyRestockContainer )
					nItem->setRestock( nItem->amount() );
			}
			else if( element->hasAttribute( "list" ) )
			{
				qWarning("cItem::processContainerNode <item list=\"myList\"/> not implemented!!!");
			}
			else
			{
				Console::instance()->log( LOG_ERROR, QString( "Content element lacking id and list attribute in item definition '%1'." ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
			}
		}
		else
		{
			Console::instance()->log( LOG_ERROR, QString( "Unknown content element '%1' in item definition '%2'." ).arg( element->name() ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
		}
	}
}

void cItem::showName( cUOSocket *socket )
{
	// End chars/npcs section

    if( onSingleClick( socket->player() ) )
        return;

	QString itemname( "" );

	if( !name_.isNull() )
		itemname = getName();

	// Amount information
	if( amount_ > 1 )
		itemname.append( tr( ": %1" ).arg( amount_ ) );

	// Show serials
	if( socket->player() && socket->player()->account() && socket->player()->account()->isShowSerials() )
		itemname.append( tr( " [%1]" ).arg( serial(), 8, 16 ) );

	// Try a localized Message
	if( name_.isNull() )
		socket->clilocMessageAffix( 0xF9060 + id_, "", itemname, 0x3B2, 3, this );
	else
		socket->showSpeech( this, itemname );

/*	// When we click on a player vendors item,
	// we show the price as well
	if( container_ && container_->isItem() )
	{
		P_CHAR pc_j = getOutmostChar();
		if( pc_j && pc_j->npcaitype() == 17 )
			socket->showSpeech( this, tr( "at %1gp" ).arg( price_ ) );
	}*/

	// Show RepSys Settings of Victim when killed
	if( corpse() && hasTag( "notoriety" ) )
	{
		int notoriety = getTag( "notoriety" ).toInt();

		if( notoriety == 1 )
			socket->showSpeech( this, tr( "[Innocent]" ), 0x005A );
		else if( notoriety == 2 )
			socket->showSpeech( this, tr( "[Criminal]" ), 0x03B2 );
		else if( notoriety == 3 )
			socket->showSpeech( this, tr( "[Murderer]" ), 0x0026 );
	}

	// Let's handle secure/locked down stuff.
	if ( isLockedDown() && type() != 12 && type() != 13 && type() != 203 )
	{
		if ( !secured() )
			socket->showSpeech( this, tr( "[locked down]" ), 0x481 );
		else
			socket->showSpeech( this, tr( "[locked down & secure]" ), 0x481 );
	}

	// Send the item/weight as the last line in case of containers
	if( type() == 1 || type() == 63 || type() == 65 || type() == 87 )
	{
		float tWeight = totalweight_;

		if( weight_ == 255 )
			tWeight -= 255;

		QString message = tr( "[%1 items, %2 stones]" ).arg( content_.size() ).arg( tWeight );

		socket->showSpeech( this, message, 0x3B2 );
	}
}

// This either sends a ground-item or a backpack item
void cItem::update(cUOSocket *singlesocket)
{
	if (free) {
		return;
	}

	// Items on Ground
	if (!container_) {
		// we change the packet during iteration, so we have to
		// recompress it
		cUOTxSendItem sendItem;
		sendItem.setSerial(serial_);
		sendItem.setId(id_);
		sendItem.setAmount(amount_);
		sendItem.setColor(color_);
		sendItem.setCoord(pos_);
		sendItem.setDirection(dir_);

		// Send to one person only
		if (!singlesocket) {
			for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
				if (socket->canSee(this)) {
					P_PLAYER player = socket->player();
					unsigned char flags = 0;
					cUOTxSendItem packetCopy(sendItem);

					// Always Movable Flag
					if (isAllMovable()) {
						flags |= 0x20;
					} else if (player->account()->isAllMove()) {
						flags |= 0x20;
					} else if (isOwnerMovable() && player->Owns(this)) {
						flags |= 0x20;
					}

					if (visible_ != 0) {
						flags |= 0x80;
					}

					packetCopy.setFlags(flags);

					socket->send(&packetCopy);
					sendTooltip(socket);
				}
			}
		} else if (singlesocket && singlesocket->canSee(this)) {
			P_PLAYER player = singlesocket->player();
			unsigned char flags = 0;

			// Always Movable Flag
			if (isAllMovable()) {
				flags |= 0x20;
			} else if (player->account()->isAllMove()) {
				flags |= 0x20;
			} else if (isOwnerMovable() && player->Owns(this)) {
				flags |= 0x20;
			}

			if (visible_ != 0) {
				flags |= 0x80;
			}

			sendItem.setFlags(flags);

			singlesocket->send(&sendItem);
			sendTooltip(singlesocket);
		}
	}
	// equipped items
	else if( container_ && container_->isChar() )
	{
		cUOTxCharEquipment equipItem;
		equipItem.fromItem(this);

		if (singlesocket) {
			singlesocket->send(&equipItem);
			sendTooltip(singlesocket);
		} else {
			for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
				if (socket->canSee(this)) {
					socket->send(&equipItem);
					sendTooltip(socket);
				}
			}
		}
	
	// items in containers
	} else if (container_ && container_->isItem()) {
		cUOTxAddContainerItem contItem;
		contItem.fromItem(this);

		if (singlesocket) {
			singlesocket->send(&contItem);
			sendTooltip(singlesocket);
		} else {
			for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
				if (socket->canSee(this)) {
					socket->send(&contItem);
					sendTooltip(socket);
				}
			}
		}
	}
}

P_ITEM cItem::dupe()
{
	P_ITEM nItem = new cItem(*this);
	nItem->setSerial(World::instance()->findItemSerial());

	if (container_) {
		P_CHAR pchar = dynamic_cast<P_CHAR>(container_);

		if (pchar) {
			nItem->container_ = 0;
			nItem->moveTo(pchar->pos(), true);
		} else {
			P_ITEM item = dynamic_cast<P_ITEM>(container_);
	
			if (item) {
				item->addItem(nItem, false, true, true);
			}
		}
	} else {
		nItem->moveTo(pos_);
	}

	return nItem;
}

void cItem::soundEffect( UINT16 sound )
{
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			mSock->soundEffect( sound, this );
}

// Our weight has changed
// Update the top-containers
void cItem::setWeight( float nValue )
{
	setTotalweight( ceilf( ( totalweight_ - ( amount_ * weight_ ) ) * 100 ) / 100 );
	changed( TOOLTIP );
	flagChanged();
	weight_ = ceilf( nValue * 100 ) / 100;
	setTotalweight( ceilf( ( totalweight_ + ( amount_ * weight_ ) ) * 100 ) / 100 );
}

// This subtracts the weight of the top-container
// And then readds the new weight
void cItem::setTotalweight( float data )
{
	//if( data < 0 )
		// FixWeight!

	// Completely ignore the container if the free flag is set
	// this flag is abused during the load phase of the server
	// to flag items with yet unprocessed container values
	if (!free && container_) {
		if(container_->isChar())
		{
			P_CHAR pChar = dynamic_cast<P_CHAR>( container_ );
			if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
				pChar->setWeight( pChar->weight() - totalweight_ );
		} else if(container_->isItem()) {
			P_ITEM pItem = dynamic_cast<P_ITEM>( container_ );
			if( pItem )
				pItem->setTotalweight( pItem->totalweight() - totalweight_ );
		}
	}

	changed( TOOLTIP );
	flagChanged();
	totalweight_ = ceilf( data * 100 ) / 100;

	// Completely ignore the container if the free flag is set
	// this flag is abused during the load phase of the server
	// to flag items with yet unprocessed container values
	if (!free && container_) {
		if (container_->isChar()) {
			P_CHAR pChar = dynamic_cast<P_CHAR>( container_ );
			if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
				pChar->setWeight( pChar->weight() + totalweight_ );
		} else if (container_->isItem()) {
			P_ITEM pItem = dynamic_cast<P_ITEM>( container_ );
			if(pItem) {
				pItem->setTotalweight( pItem->totalweight() + totalweight_);
			}
		}
	}
}

void cItem::talk( const QString &message, UI16 color, UINT8 type, bool autospam, cUOSocket* socket )
{
	QString lang;

	if( socket )
		lang = socket->lang();

	cUOTxUnicodeSpeech::eSpeechType speechType;

	switch( type )
	{
	case 0x01:
		speechType = cUOTxUnicodeSpeech::Broadcast;
	case 0x06:
		speechType = cUOTxUnicodeSpeech::System;
	case 0x09:
		speechType = cUOTxUnicodeSpeech::Yell;
	case 0x02:
		speechType = cUOTxUnicodeSpeech::Emote;
	case 0x08:
		speechType = cUOTxUnicodeSpeech::Whisper;
	default:
		speechType = cUOTxUnicodeSpeech::Regular;
	};

	cUOTxUnicodeSpeech textSpeech;
	textSpeech.setSource( serial() );
	textSpeech.setModel( id() );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( speechType );
	textSpeech.setLanguage( lang );
	textSpeech.setName( name_ );
	textSpeech.setColor( color );
	textSpeech.setText( message );
	textSpeech.setText( message );

	if( socket )
	{
		socket->send( &textSpeech );
	}
	else
	{
		// Send to all clients in range
		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
				if( mSock->player() && ( mSock->player()->dist( this ) < 18 ) )
				{
					mSock->send( &textSpeech );
				}
		}
	}
}

bool cItem::wearOut() {
	if (RandomNum(1, 4) == 4) {
		setHp(hp() - 1);
	}

	if (hp() <= 0) {
		// Get the owner of the item
		P_CHAR owner = getOutmostChar();
		P_PLAYER pOwner = dynamic_cast<P_PLAYER>(owner);

		if(pOwner && pOwner->socket()) {
			if (!name_.isEmpty()) {
				pOwner->socket()->clilocMessageAffix(1008129, QString::null, name_);
			} else {
				pOwner->socket()->clilocMessageAffix(1008129, QString::null, getName());
			}
		}

		// Show to all characters in range that the item has been destroyed and not just unequipped
		if (owner) {
			for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
				if (owner != socket->player() && socket->canSee(owner)) {
					socket->clilocMessageAffix(0xf9060 + id_, "", tr("You see %1 destroy his ").arg(owner->name()), 0x23, 3, owner, false, true);
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
	QPtrList< cItem > itemlist;

	while( it != containment.end() )
	{
		P_ITEM pItem = *it;

		// we'v got a container
		if( pItem->type() == 1 || pItem->type() == 63 )
		{
			QPtrList< cItem > sublist = pItem->getContainment();

			// Transfer the items
			QPtrListIterator< cItem > pit( sublist );
			P_ITEM pi;
			while( ( pi = pit.current() ) )
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

static void itemRegisterAfterLoading( P_ITEM pi )
{
	World::instance()->registerObject( pi );

	// Set the outside indices
	if( pi->maxhp() == 0 )
		pi->setMaxhp( pi->hp() );
}

static cUObject* productCreator()
{
	return new cItem;
}

void cItem::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT %1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cItem' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType("cItem", productCreator);
	UObjectFactory::instance()->registerSqlQuery( "cItem", sqlString );
}

void cItem::load( char **result, UINT16 &offset )
{
	cUObject::load( result, offset ); // Load the items we inherit from first

	// Broken Serial?
	if( !isItemSerial( serial() ) )
		throw QString( "Item has invalid character serial: 0x%1" ).arg( serial(), 0, 16 );

	id_ = atoi( result[offset++] );
	color_ = atoi( result[offset++] );

	//  Warning, ugly optimization ahead, if you have a better idea, we want to hear it.
	//  For load speed and memory conservation, we will store the SERIAL of the container
	//  here and then right after load is done we replace that value with it's memory address
	//  as it should be.
	SERIAL containerSerial = atoi( result[offset++] );

	if (containerSerial != INVALID_SERIAL) {
		container_ = reinterpret_cast<cUObject*>(containerSerial);
		free = true; // Abuse free for lingering items
	}

	// ugly optimization ends here.

	layer_ = atoi( result[offset++] );
	type_ = atoi( result[offset++] );
	type2_ = atoi( result[offset++] );
	amount_ = atoi( result[offset++] );
	decaytime_ = atoi( result[offset++] );
	if( decaytime_ > 0 )
		decaytime_ += uiCurrentTime;
	def_ = atoi( result[offset++] );
	hidamage_ = atoi( result[offset++] );
	lodamage_ = atoi( result[offset++] );
	weight_ = (float)atof( result[offset++] );
	hp_ = atoi( result[offset++] );
	maxhp_ = atoi( result[offset++] );
	speed_ = atoi( result[offset++] );
	magic_ = atoi( result[offset++] );
	ownserial_ = atoi( result[offset++] );
	visible_ = atoi( result[offset++] );

	if( strlen( result[offset] ) == 0 )
		spawnregion_ = QString::null;
	else
		spawnregion_ = QString::fromUtf8( result[offset] );
	offset++;

	priv_ = atoi( result[offset++] );
	sellprice_ = atoi( result[offset++] );
	buyprice_ = atoi( result[offset++] );
	restock_ = atoi( result[offset++] );
	baseid_ = result[offset++];

	// Their own weight should already be set.
	totalweight_ = ceilf( amount_ * weight_ * 100 ) / 100;

	itemRegisterAfterLoading( this );
}

void cItem::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cUObject::buildSqlString( fields, tables, conditions );
	fields.push_back( "items.id,items.color,items.cont,items.layer,items.type,items.type2,items.amount,items.decaytime,items.def,items.hidamage,items.lodamage,items.weight,items.hp,items.maxhp,items.speed,items.magic,items.owner,items.visible,items.spawnregion,items.priv,items.sellprice,items.buyprice,items.restock,items.baseid" );
	tables.push_back( "items" );
	conditions.push_back( "uobjectmap.serial = items.serial" );
}


void cItem::addItem( cItem* pItem, bool randomPos, bool handleWeight, bool noRemove )
{
	if( !pItem )
		return;

	if( pItem == this )
	{
		Console::instance()->log( LOG_WARNING, QString( "Rejected putting an item into itself (%1)" ).arg( serial_, 0, 16 ) );
		return;
	}

	if( !noRemove )
		pItem->removeFromCont( handleWeight );

	content_.push_back( pItem );
	pItem->layer_ = 0;
	pItem->container_ = this;

	if( randomPos && !this->ContainerPileItem( pItem ) ) // try to pile
	{
		if (randomPos)
			pItem->SetRandPosInCont( this ); // not piled, random pos
	}

	if ( handleWeight )
		setTotalweight( this->totalweight() + pItem->totalweight() );
}

void cItem::removeItem( cItem* pItem, bool handleWeight )
{
	//ContainerContent::iterator it = std::find(content_.begin(), content_.end(), pItem);
	ContainerContent::iterator it = content_.begin();
	while( it != content_.end() )
	{
		if( (*it) == pItem )
		{
			content_.erase(it);
			if (handleWeight)
				setTotalweight(	this->totalweight() - pItem->totalweight() );
			break;
		}

		++it;
	}

	pItem->container_ = 0;
	pItem->flagChanged();
	pItem->setLayer( 0 );
}

cItem::ContainerContent cItem::content() const
{
	return content_;
}

bool cItem::contains( const cItem* pItem ) const
{
	ContainerContent::const_iterator it = std::find(content_.begin(), content_.end(), pItem);
	return it != content_.end();

}


void cItem::removeFromCont( bool handleWeight )
{
	if( !container_ )
		return;

	if( container_->isChar() )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( container_ );
		if( pChar )
			pChar->removeItem( (cBaseChar::enLayer)layer_, handleWeight );
	}
	else if( container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast< P_ITEM >( container_ );
		if( pCont )
			pCont->removeItem( this, handleWeight );
	}

	container_ = 0;
	flagChanged();
}

P_ITEM cItem::getOutmostItem()
{
	if( container_ && container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast< P_ITEM >( container_ );
		if( pCont )
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

	if (container_) {
		result = dynamic_cast<P_CHAR>(container_);

		if (!result) {
			P_ITEM container = dynamic_cast<P_ITEM>(container_);

			if (container) {
				result = container->getOutmostChar();
			}
		}
	}

	return result;
}

// If we change the amount, the weight changes as well
void cItem::setAmount( UI16 nValue )
{
	setTotalweight( totalweight_ + ceilf( ( nValue - amount_ ) * weight_ * 100 ) / 100 );
	amount_ = nValue;	
	changed( TOOLTIP );
	flagChanged();
}

UINT16 cItem::getWeaponSkill()
{
	switch( type_ )
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
stError *cItem::setProperty( const QString &name, const cVariant &value )
{
	changed( TOOLTIP );
	flagChanged();

	SET_INT_PROPERTY( "id", id_ )
	else SET_INT_PROPERTY( "color", color_ )

	else if( name == "baseid" )
	{
		baseid_ = value.toString();
	}

	// Amount needs weight handling
	else if( name == "amount" )
	{
		int val = value.toInt();
		if( val <= 0 )
		{
			this->remove();
			return 0;
		}

		int diff = val - amount_;
		setTotalweight( totalweight_ + ceilf( diff * weight_ * 100 ) / 100 );
		amount_ = val;
		return 0;
	}

	else SET_INT_PROPERTY( "layer", layer_ )
	else SET_INT_PROPERTY( "type", type_ )
	else SET_INT_PROPERTY( "type2", type2_ )
	else SET_INT_PROPERTY( "speed", speed_ )
	else SET_INT_PROPERTY( "lodamage", lodamage_ )
	else SET_INT_PROPERTY( "hidamage", hidamage_ )
	else SET_FLOAT_PROPERTY( "weight", weight_ )
	else SET_INT_PROPERTY( "health", hp_ )
	else SET_INT_PROPERTY( "maxhealth", maxhp_ )
	else SET_INT_PROPERTY( "owner", ownserial_ )

	else if( name == "totalweight" )
	{
		setTotalweight( static_cast<QString>( value.toString() ).toFloat() );
		return 0;
	}

	else if( name == "container" )
	{
		// To int and Check for Serial type (makes it safer as well)
		SERIAL cser = value.toInt();
		P_CHAR pChar = FindCharBySerial( cser );
		P_ITEM pItem = FindItemBySerial( cser );

		if( pItem )
			pItem->addItem( this );
		else if( pChar )
		{
			// Get a valid layer
			tile_st tInfo = TileCache::instance()->getTile( id_ );
			if( tInfo.layer != 0 )
				pChar->addItem( (cBaseChar::enLayer)tInfo.layer, this );
		}
		else
		{
			// Remove from Cont and move to the old containers position
			P_ITEM pCont = getOutmostItem();
			P_CHAR pChar = pCont->getOutmostChar();

			if( pChar )
				setPos( pChar->pos() );
			else
				setPos( pCont->pos() );

			removeFromCont();
			MapObjects::instance()->add( this );
		}
	}

	else SET_INT_PROPERTY( "defense", def_ )
	else SET_INT_PROPERTY( "decaytime", decaytime_ )

	else if( name == "visible" )
	{
		if( value.toInt() )
			visible_ = 0;
		else
			visible_ = 2;

		return 0;
	}

	else if( name == "ownervisible" )
	{
		if( value.toInt() )
			visible_ = 1;
		else
			visible_ = 2;

		return 0;
	}

	else SET_STR_PROPERTY( "spawnregion", spawnregion_ )
	else SET_INT_PROPERTY( "sellprice", sellprice_ )
	else SET_INT_PROPERTY( "buyprice", buyprice_ )
	else SET_INT_PROPERTY( "restock", restock_ )
	else SET_INT_PROPERTY( "magic", magic_ )
	else SET_INT_PROPERTY( "visible", visible_ )

	// Flags
	else if( name == "decay" )
	{
		if( value.toInt() )
			setNoDecay( false );
		else
			setNoDecay( true );
		return 0;
	}
	else if( name == "newbie" )
	{
		if( value.toInt() )
			priv_ |= 0x02;
		else
			priv_ &= ~0x02;
		return 0;
	}
	else if( name == "dispellable" )
	{
		if( value.toInt() )
			priv_ |= 0x04;
		else
			priv_ &= ~0x04;
		return 0;
	}
	else if( name == "secured" )
	{
		if( value.toInt() )
			priv_ |= 0x08;
		else
			priv_ &= ~0x08;
		return 0;
	}
	else if( name == "wipeable" )
	{
		if( value.toInt() )
			priv_ |= 0x10;
		else
			priv_ &= ~0x10;
		return 0;
	}
	else if( name == "twohanded" )
	{
		if( value.toInt() )
			priv_ |= 0x20;
		else
			priv_ &= ~0x20;
		return 0;
	}
	else if( name == "dye" )
	{
		setDye( value.toInt() != 0 ? true : false );
	}
	else if( name == "corpse" )
	{
		if( value.toInt() )
			priv_ |= 0x40;
		else
			priv_ &= ~0x40;
		return 0;
	}

	return cUObject::setProperty( name, value );
}

stError *cItem::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "id", id_ )
	else GET_PROPERTY( "baseid", baseid_ )
	else GET_PROPERTY( "color", color_ )
	else GET_PROPERTY( "amount", amount_ )
	else GET_PROPERTY( "layer", layer_ )
	else GET_PROPERTY( "type", type_ )
	else GET_PROPERTY( "type2", type2_ )
	else GET_PROPERTY( "speed", speed_ )
	else GET_PROPERTY( "lodamage", lodamage_ )
	else GET_PROPERTY( "hidamage", hidamage_ )
	else GET_PROPERTY( "weight", weight_ )
	else GET_PROPERTY( "health", hp_ )
	else GET_PROPERTY( "maxhealth", maxhp_ )
	else GET_PROPERTY( "owner", owner() )
	else GET_PROPERTY( "totalweight", totalweight_ )

	// container
	else if( name == "container" )
	{
		if( container_ && container_->isItem() )
			value = cVariant( dynamic_cast< P_ITEM >( container_ ) );
		else if( container_ && container_->isChar() )
			value = cVariant( dynamic_cast< P_CHAR >( container_ ) );
		else
			value = cVariant( (P_ITEM)0 );

		return 0;
	}

	else GET_PROPERTY( "defense", (int)def_ )
	else GET_PROPERTY( "decaytime", (int)decaytime_ )

	// Visible
	else GET_PROPERTY( "visible", visible_ == 0 ? 1 : 0 )
	else GET_PROPERTY( "ownervisible", visible_ == 1 ? 1 : 0 )
	else GET_PROPERTY( "spawnregion", spawnregion_ )

	else GET_PROPERTY( "buyprice", buyprice_ )
	else GET_PROPERTY( "sellprice", sellprice_ )
	else GET_PROPERTY( "restock", restock_ )
	else GET_PROPERTY( "magic", magic_ )

	// Flags
	else GET_PROPERTY( "dye", dye() ? 1 : 0 )
	else GET_PROPERTY( "decay", priv_ & 0x01 ? 0 : 1 )
	else GET_PROPERTY( "newbie", priv_ & 0x02 ? 1 : 0 )
	else GET_PROPERTY( "dispellable", priv_ & 0x04 ? 1 : 0 )
	else GET_PROPERTY( "secured", priv_ & 0x08 ? 1 : 0 )
	else GET_PROPERTY( "wipeable", priv_ & 0x10 ? 1 : 0 )
	else GET_PROPERTY( "twohanded", priv_ & 0x20 ? 1 : 0 )
	else GET_PROPERTY( "corpse", priv_ & 0x40 ? 1 : 0 )
	else GET_PROPERTY( "visible", visible() )
	else return cUObject::getProperty( name, value );
}

void cItem::sendTooltip( cUOSocket* mSock )
{
	// There is a list of statically overridden items in the client (@50A1C0 for 4.0.0o)
	unsigned short id = this->id();

	// Mostly Signs (not movable but still have tooltips shown)
	if(	( id >= 0xba3 && id <= 0xc0e ) ||	// House Signs
		( id >= 0x1297 && id <= 0x129e ) ||	// Road Signs
		( id >= 0x3e4a && id <= 0x3e55 ) ||	// Tillermen
		( id >= 0xed4 && id <= 0xede ) ||	// Graves and Guildstones
		( id >= 0x1165 && id <= 0x1184 ) ||	// More Gravestones
		( id == 0x2006 ) ||
		!name_.isNull()						// Non Default Name
		)
	{
		cUObject::sendTooltip( mSock );
		return;
	}

	// Don't send the tooltip if we're a supposed-to-be-static item
	tile_st tile = TileCache::instance()->getTile( id_ );

	// If the item is not movable for the client, the item should not have a tooltip
	// Exceptions are noted above and containers
	if( tile.weight == 255 && !isAllMovable() )
	{
		if( tile.flag3 & 0x20 == 0 )
			return;
	}

	cUObject::sendTooltip( mSock );
}

/*!
	Selects an item id from a list and creates it.
*/
P_ITEM cItem::createFromList( const QString &id )
{
	QString entry = DefManager->getRandomListEntry( id );
	return createFromScript( entry);
}

/*!
	Creates a new item and applies the specified definition section on it.
*/
P_ITEM cItem::createFromScript( const QString& id )
{
	P_ITEM nItem = 0;

	// Get an Item and assign a serial to it
	const cElement* section = DefManager->getDefinition( WPDT_ITEM, id );

	if( section )
	{
		nItem = new cItem;
		nItem->Init( true );
		nItem->setBaseid(id);
		nItem->applyDefinition( section );
		nItem->onCreate( id );
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

	// Set the Weight based on the Tiledata
	tile_st tile = TileCache::instance()->getTile( id );

	if( tile.weight < 255 )
		pItem->setWeight( tile.weight );

	return pItem;
}

void cItem::createTooltip(cUOTxTooltipList &tooltip, cPlayer *player) {
	cUObject::createTooltip(tooltip, player);

	if (!onShowTooltip(player, &tooltip)) {
		if (name_.isNull() || name_.isEmpty()) {
			if (amount_ > 1) {
				//tooltip.addLine(0x1005bd, " \t#" + QString::number( 0xF9060 + id_ ) + "\t: " + QString::number(amount_));
				tooltip.addLine(1050039, QString::number(amount_) + "\t#" + QString::number(0xf9060 + id_));
			} else {
				//tooltip.addLine(0xF9060 + id_, "");
				tooltip.addLine(1050039, " \t#" + QString::number(0xf9060 + id_));
			}
		}
		else
			if (amount_ > 1) {
				//tooltip.addLine(0x1005bd, " \t#" + QString::number( 0xF9060 + id_ ) + "\t: " + QString::number(amount_));
				tooltip.addLine(1050039, QString::number(amount_) + "\t" + name_);
			} else {
				//tooltip.addLine(0xF9060 + id_, "");
				tooltip.addLine(1050039, " \t" + name_);
			}
	}
}

// Python implementation
PyObject *cItem::getPyObject() {
	return PyGetItemObject(this);
}

const char *cItem::className() const {
	return "item";
}
