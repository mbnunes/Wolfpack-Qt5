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
#include "wolfpack.h"
#include "accounts.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "utilsys.h"
#include "iserialization.h"
#include "debug.h"
#include "items.h"
#include "books.h"
#include "mapobjects.h"
#include "tilecache.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "pythonscript.h"
#include "maps.h"
#include "network.h"
#include "multis.h"
#include "spellbook.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "world.h"
#include "scriptmanager.h"
#include "itemid.h"
#include "basechar.h"
#include "player.h"

// System Includes
#include <math.h>
#include <algorithm>

using namespace std;

#undef  DBGFILE
#define DBGFILE "items.cpp"

// constructor
cItem::cItem(): container_(0), totalweight_(0), incognito(false),
rndvaluerate_(0), dooropen_(0),gatetime_(0),gatenumber_(-1),
timeused_last(0), sellprice_( 0 ), buyprice_( 0 ), restock_( 1 ), antispamtimer_( 0 )
{
	Init( false );
};

cItem::cItem( const cItem &src )
{
	changed( SAVE|TOOLTIP );
	this->name_ = src.name_;
	this->name2_ = src.name2_;
	this->creator_ = src.creator_;
	this->incognito = src.incognito;
	this->madewith_ = src.madewith_;
	this->rank_ = src.rank_;
	this->good_ = src.good_;
	this->rndvaluerate_ = src.rndvaluerate_;

	this->setMultis( src.multis() );
	this->free = false;
	this->setId(src.id());
	this->setPos(src.pos());
	this->color_ = src.color_;
	this->layer_ = src.layer_;
	this->type_ = src.type_;
	this->type2_ = src.type2_;
	this->offspell_ = src.offspell_;
	this->weight_ = src.weight_;
	this->more1_ = src.more1_;
	this->more2_ = src.more2_;
	this->more3_ = src.more3_;
	this->more4_ = src.more4_;
	this->morex_ = src.morex_;
	this->morey_ = src.morey_;;
	this->morez_ = src.morez_;
	this->amount_ = src.amount_;
	this->doordir_ = src.doordir_;
	this->dooropen_ = src.dooropen_;
	this->dye_ = src.dye_;
	this->att_ = src.att_;
	this->def_ = src.def_;
	this->lodamage_=src.lodamage_;
	this->hidamage_=src.hidamage_;
	this->hp_ = src.hp_;
	this->maxhp_ = src.maxhp_;
	this->st_ = src.st_;
	this->st2_ = src.st2_;
	this->dx_ = src.dx_;
	this->dx2_ = src.dx2_;
	this->in_ = src.in_;
	this->in2_ = src.in2_;
	this->speed_=src.speed_;
	this->magic_ = src.magic_;
	this->gatetime_ = src.gatetime_;
	this->gatenumber_ = src.gatenumber_;
	this->decaytime_ = src.decaytime_;
	this->setOwnSerialOnly(src.ownSerial());
	this->visible_=src.visible_;
	this->spawnserial=src.spawnserial;
	this->dir_=src.dir_;
	this->priv_=src.priv_;
	this->buyprice_ = src.buyprice_;
	this->sellprice_ = src.sellprice_;
	this->restock_ = src.restock_;
	this->disabled_ = src.disabled_;
	this->poisoned_ = src.poisoned_;
	this->time_unused=src.time_unused;
	this->timeused_last=getNormalizedTime();
	this->setSpawnRegion( src.spawnregion() );
	this->desc_ = src.desc_;
	// We're *NOT* copying the contents over
	
	this->setTags( src.tags() );
	this->accuracy_ = 100;
	this->container_ = src.container_;
	this->totalweight_ = src.totalweight_;
	setTotalweight( amount_ * weight_ );
}

P_CHAR cItem::owner( void ) const
{
	return FindCharBySerial( ownserial_ ); 
}

void cItem::setOwner( P_CHAR nOwner )
{
	changed( SAVE );
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
	changed( SAVE );
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

void cItem::startDecay()			
{
	if( container_ )
		return;

	changed( SAVE );
	this->decaytime_ = SrvParams->decayTime()*MY_CLOCKS_PER_SEC+uiCurrentTime;
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
long cItem::reduceAmount(const short amt)
{
	UINT16 rest = 0;
	if( amount_ > amt )
	{
		setAmount( amount_ - amt );
		update();
		changed( SAVE|TOOLTIP );
	}
	else
	{
		Items->DeleItem(this);
		rest = amt - amount_;
	}
	return rest;
}

void cItem::setOwnSerialOnly(int ownser)
{
	changed( SAVE );
	ownserial_ = ownser;
}

void cItem::SetOwnSerial(int ownser)
{
	changed( SAVE );
	setOwnSerialOnly(ownser);
}

void cItem::SetSpawnSerial(long spawnser)
{
	spawnserial = spawnser;
	changed( SAVE );
}

void cItem::SetMultiSerial(long mulser)
{
	changed( SAVE );
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
	
	case 0x0EFA: return 4;	// spellbook. Position shouldn't matter, but as it can be opened like a backpack...(Duke)
		
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
		Items->DeleItem( pItem );
	}
	
	changed( SAVE|TOOLTIP );
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
	int k=pCont->GetContGumpType();	
	if (k==-1)
	{
		LogWarningVar("trying to put something INTO a non container, id=0x%X",pCont->id());
		k=1;
	}
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
	if( decaytime_ > 0 && !changed_ )
	{
		initSave;
		setTable( "items" );

		addField("decaytime",	(decaytime_ > uiCurrentTime) ? decaytime_ - uiCurrentTime : 0	);

		addCondition( "serial", serial() );
		saveFields;
	}

	if ( changed_ )
	{
		initSave;
		setTable( "items" );
		
		addField("serial",		serial());
		addField("id",			id());
		addStrField("name",			name_); // warning: items do not use cUObject name! (nuts, remove it)
		addStrField("name2",			name2_);
		addStrField("creator",		creator_);
		addField("sk_name",		madewith_);
		addField("color",			color());
		SERIAL contserial = INVALID_SERIAL;
		if ( container_ )
			contserial = container_->serial();
		addField("cont",			contserial);
		addField("layer",			layer_);
		addField("type",			type_);
		addField("type2",			type2_);
		addField("offspell",		offspell_);
		addField("more1",			more1_);
		addField("more2",			more2_);
		addField("more3",			more3_);
		addField("more4",			more4_);
		addField("morex",			morex_);
		addField("morey",			morey_);
		addField("morez",			morez_);
		addField("amount",		amount_);
		addField("doordir",		doordir_);
		addField("dye",			dye_);
		addField("decaytime",	(decaytime_ > uiCurrentTime) ? decaytime_ - uiCurrentTime : 0	);
		addField("att",			att_);
		addField("def",			def_);
		addField("hidamage",		hidamage_);
		addField("lodamage",		lodamage_);
		addField("st",			st_);
		addField("time_unused",	time_unused);
		addField("weight",		weight_);
		addField("hp",			hp_);
		addField("maxhp",			maxhp_);
		addField("rank",			rank_);
		addField("st2",			st2_);
		addField("dx",			dx_);
		addField("dx2",			dx2_);
		addField("intelligence",	in_);
		addField("intelligence2",	in2_);
		addField("speed",			speed_);
		addField("poisoned",		poisoned_);
		addField("magic",			magic_);
		addField("owner",			ownserial_);
		addField("visible",		visible_);
		addField("spawn",			spawnserial);
		addField("dir",			dir_);
		addField("priv",			priv_);
		addField("sellprice",			sellprice_);
		addField("buyprice",			buyprice_);
		addField("restock",		restock_);
		addField("disabled",		disabled_);
		addStrField("spawnregion",	spawnregion_);
		addField("good",			good_);
		addStrField("description",	desc_);
		addField("accuracy",		accuracy_);
		
		addCondition( "serial", serial() );
		saveFields;
	}
	cUObject::save();
	changed_ = false;
}

static void itemRegisterAfterLoading( P_ITEM pi );

bool cItem::del()
{
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "items", QString( "serial = '%1'" ).arg( serial() ) );
	changed( SAVE );
	return cUObject::del();
}

QString cItem::getName( bool shortName )
{
	// Book Titles
	if( type_ == 11 )
	{
		cBook *pBook = dynamic_cast< cBook* >( this );
		if( pBook && !pBook->title().isEmpty() )
		{
			QString bookname = pBook->title();

			// Append author
			if( !shortName && !pBook->author().isEmpty() )
				bookname.append( tr( " by %1" ).arg( pBook->author() ) );

			return bookname;
		}
	}

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
	changed( SAVE|TOOLTIP );

	cUObject::setSerial( INVALID_SERIAL );

	if( createSerial )
		this->setSerial( World::instance()->findItemSerial() );

	this->container_ = 0;
	this->name_ = "#";
	this->name2_ = this->name_;
	this->incognito=false;//AntiChrist - incognito
	this->madewith_=0; // Added by Magius(CHE)
	this->rank_ = 0; // Magius(CHE)
	this->good_=-1; // Magius(CHE)
	this->rndvaluerate_=0; // Magius(CHE) (2)

	this->setMultis( INVALID_SERIAL );//Multi serial
	this->free = false;
	this->setId(0x0001); // Item visuals as stored in the client
	this->setPos( Coord_cl(100, 100, 0) );
	this->color_ = 0x00; // Hue
	this->layer_ = 0; // Layer if equipped on paperdoll
	this->type_=0; // For things that do special things on doubleclicking
	this->type2_=0;
	this->offspell_ = 0;
	this->weight_ = 0;
	this->more1_=0; // For various stuff
	this->more2_=0;
	this->more3_=0;
	this->more4_=0;
	this->morex_=0;
	this->morey_=0;
	this->morez_=0;
	this->amount_ = 1; // Amount of items in pile
	this->doordir_=0; // Reserved for doors
	this->dooropen_=0;
	this->dye_=0; // Reserved: Can item be dyed by dye kit
	this->att_=0; // Item attack
	this->def_=0; // Item defense
	this->lodamage_=0; //Minimum Damage weapon inflicts
	this->hidamage_=0; //Maximum damage weapon inflicts
	this->hp_=0; //Number of hit points an item has.
	this->maxhp_=0; // Max number of hit points an item can have.
	this->st_=0; // The strength needed to equip the item
	this->st2_=0; // The strength the item gives
	this->dx_ = 0; // The dexterity needed to equip the item
	this->dx2_ = 0; // The dexterity the item gives
	this->in_ = 0; // The intelligence needed to equip the item
	this->in2_ = 0; // The intelligence the item gives
	this->speed_=0; //The speed of the weapon
	this->magic_ = 0; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable.
	this->gatetime_= 0;
	this->gatenumber_= -1;
	this->decaytime_ = 0;
	this->setOwnSerialOnly(-1);
	this->visible_=0; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	this->spawnserial=-1;
	this->dir_=0; // Direction, or light source type.
	// Everything decays by default.
	this->priv_=1; // Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
	this->disabled_ = 0; //Item is disabled, cant trigger.
	this->poisoned_ = 0; //AntiChrist -- for poisoning skill
	this->time_unused = 0;
	this->timeused_last=getNormalizedTime();
	this->spawnregion_ = QString::null;
	this->accuracy_ = 100;
}

/*!
	This Function is used for removing items
*/
void cAllItems::DeleItem(P_ITEM pi)
{
	// TODO: Insert handling of the unequip events here

	if( !pi )
		return;

	if (!pi->free)
	{
		// Remove it from view
		pi->removeFromView( true );

		pi->SetSpawnSerial(-1);
		pi->SetOwnSerial(-1);

		// - remove from cMapObjects::getInstance() if a world item
		if( pi->isInWorld() ) 
			MapObjects::instance()->remove(pi);
		else
			pi->removeFromCont();

        // if a new book gets deleted also delete the corresponding bok file

		// Also delete all items inside if it's a container.
		cItem::ContainerContent container(pi->content());
		cItem::ContainerContent::const_iterator it ( container.begin() );
		cItem::ContainerContent::const_iterator end( container.end()   );
		for ( ; it != end; ++it )
		{
			// This could turn out to be a problem. We're removing from top-cont as well
			P_ITEM pContent = *it;
			if( pContent )
				DeleItem( pContent );
		}

		// if it is within a multi, delete it from the multis vector
		if( pi->multis() != INVALID_SERIAL )
		{
			cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pi->multis() ) );
			if( pMulti )
			{
				pMulti->removeItem( pi );
			}
		}

		pi->del(); // Remove from database
		
		// Queue for later delete.
		World::instance()->deleteObject( pi );
	}
}

P_ITEM cAllItems::SpawnItemBank(P_PLAYER pc_ch, QString nItem)
{
	if (pc_ch == NULL) 
		return NULL;
	
	P_ITEM bankbox = pc_ch->getBankBox();

	P_ITEM pi = createScriptItem(nItem);
	if (pi == NULL)
		return NULL;
	bankbox->addItem(pi);
	return pi;
}

P_ITEM cAllItems::SpawnItem(P_CHAR pc_ch, int nAmount, const char* cName, bool pileable, short id, short color, bool bPack)
{
	if (pc_ch == NULL) 
		return NULL;

	P_ITEM pPack = pc_ch->getBackpack();
	bool pile = false;
	
	if (pileable)
	{
		// make sure it's REALLY pileable ! (Duke)
		tile_st tile = TileCache::instance()->getTile( id );
		if( tile.flag2 & 0x08 )
			pile = true;
		else
		{
			// some calls to this functions (eg. IDADD) *allways* try to spawn pileable :/
			// to get rid of this message, let's try to use the .mul default (Duke, 20.10.01)
			// LogErrorVar("Spawning an item that is not truly pileable, id=%x",id);
			// pile=pileable;
		}
	}
	//Auto-Stack code!
	// If we already have an item of the same kind in our backpack,
	// we can simply spawn by increasing the amount of that item
	if (bPack && pPack && pile==1)
	{
		cItem::ContainerContent container = pPack->content();
		cItem::ContainerContent::const_iterator it ( container.begin() );
		cItem::ContainerContent::const_iterator end( container.end()   );
		for ( ; it != end; ++it )
		{
			P_ITEM pSt = *it;
			if (pSt->id() == id && !pSt->free && pSt->color() == color)
			{
				if (pSt->amount() + nAmount > 65535)	// if it would create an overflow (amount is ushort!),
					continue;						// let's search for another pile to add to
				pSt->setAmount( pSt->amount() + nAmount );
				pSt->update();
				return pSt;
			}
		}
	}
	// no such item found, so let's create it
	P_ITEM pi = new cItem;
	if (pi == NULL) return NULL;

	pi->Init();
	if(cName!=NULL)
		pi->setName( cName );
	pi->setId(id);
	pi->setColor( color );
	pi->setAmount( nAmount );
	pi->setAtt(5);
	pi->setPriv( pi->priv() | 0x01 );
	if (IsCutCloth(pi->id())) pi->setDye(1);// -Fraz- fix for cut cloth not dying
	if (bPack)
	{
		if (pPack)
		{
			pPack->addItem(pi);
		}
		else
		{// LB place it at players feet if he hasnt got backpack
			pi->moveTo(pc_ch->pos());
		}
	}
	
	pi->update();
	return pi;
}

char cAllItems::isFieldSpellItem(P_ITEM pi) //LB
{
	int a=0;
	if (pi == NULL)
		return 0;
	short id = pi->id();
	if (id==0x3996 || id==0x398C) a=1; // fire field
	else if (id==0x3915 || id==0x3920) a=2; // poison field
	else if (id==0x3979 || id==0x3967) a=3; // paralyse field
	else if (id==0x3956 || id==0x3946) a=4; // energy field;
	else if (id==0x0080) a=5;                // wall of stone
	else if (id>=0x122a && id <=0x122e) a=6; // blood
	
	return a;
}

void cAllItems::DecayItem(unsigned int currenttime, P_ITEM pi) 
{
	int preservebody;
	if ( pi == NULL )
		return;
	cMulti* pi_multi = NULL;
	
	if(pi->isLockedDown()) {pi->setDecayTime(0); return;}
	if( pi->decaytime() <= currenttime )
	{
		if (pi->priv()&0x01 && pi->isInWorld() && !pi->free)
		{  // decaytime = 5 minutes, * 60 secs per min, * MY_CLOCKS_PER_SEC
			if (pi->decaytime()==0) 
			{
				pi->startDecay();
			}
			
			if (pi->decaytime()<=currenttime)
			{
                //Multis --Boats ->
				
				if (!Items->isFieldSpellItem(pi)) // Gives fieldspells a chance to decay in multis, LB
				{
					if (pi->multis()<1 && !pi->corpse())
					{
						// JustMichael -- Added a check to see if item is in a house
						pi_multi = cMulti::findMulti( pi->pos() );
						if ( pi_multi )
						{
							if( pi_multi->itemsdecay() ) //JustMichael -- set more to 1 and stuff can decay in the building
							{
								pi->startDecay();
								return;
							}
						}
					} 
					else if (pi->multis()>0 && !pi->corpse()) 
					{					
						pi->startDecay();
						return;
					}
				}
				//End Boats/Mutlis
				
				//Keep player's corpse as long as it has more than 1 item on it
				//up to playercorpsedecaymultiplier times the decay rate
				if (pi->corpse() && pi->ownSerial()!=-1)
				{
					preservebody = pi->content().size();
					if( preservebody > 1 && pi->more4() )
					{
						pi->setMore4( pi->more4() - 1 );
						pi->startDecay();
						return;
					}
				}
				if( (pi->type() == 1 && !pi->corpse() ) || (pi->ownSerial() != -1 && pi->corpse() ) || (!SrvParams->lootdecayswithcorpse() && pi->corpse() ))
				{
					cItem::ContainerContent container(pi->content());
					cItem::ContainerContent::const_iterator it (container.begin());
					cItem::ContainerContent::const_iterator end(container.end());
					for (; it != end; ++it )
					{
						P_ITEM pi_j = *it;
                        if (pi_j != NULL) //lb
						{
							if ( pi_j == pi->container() )
							{
								pi->removeItem(pi_j);
								pi_j->moveTo(pi->pos());
								
								pi_j->startDecay();
								pi_j->update();//AntiChrist
							}
						}
					}
					Items->DeleItem(pi);
				} 
				else 
				{
					if( pi->isInWorld() )
					{
						Items->DeleItem(pi);
					}
					else
					{
						pi->startDecay();
					}
				}
			}
		}
	}
}
//END FUNCTION

void cAllItems::RespawnItem( UINT32 currenttime, P_ITEM pItem )
{
/*	if( !pItem || pItem->free )
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
				P_ITEM pSpawned = Items->createScriptItem( pItem->carve() );

				if( !pSpawned )
				{
					clConsole.send( tr( "Unable to spawn unscripted item: %1" ).arg( pItem->carve() ) );
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

	/*

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
	}//for */
}

void cAllItems::AddRespawnItem(P_ITEM pItem, QString itemSect, bool spawnInItem )
{
	if (pItem == NULL)
		return;

	P_ITEM pi = createScriptItem(itemSect); // lb, bugfix
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


	//** Lb bugfix for spawning in wrong pack positions **//
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
}

/*!
	Retrieves the Item Information stored in Section and creates an item based on it
*/
P_ITEM cAllItems::createScriptItem( const QString& Section )
{
	if( Section.length() == 0 )
		return NULL;

	P_ITEM nItem = NULL;

	// Get an Item and assign a serial to it
	const cElement* DefSection = DefManager->getDefinition( WPDT_ITEM, Section );
	
	if( !DefSection ) // section not found 
	{
		clConsole.log( LOG_ERROR, QString( "Unable to create unscripted item: %1\n" ).arg( Section ) );
		return NULL;
	}

	// books etc.
	QString type = DefSection->getAttribute( "type" );

	if( type == "book" )
	{
		cBook* nBook = new cBook();
		nBook->setSerial( World::instance()->findItemSerial() );

		nBook->applyDefinition( DefSection );
		nBook->setSection( Section );

		nItem = nBook;
	}
	else if( type == "spellbook" )
	{
		cSpellBook *spellBook = new cSpellBook();
		spellBook->Init( true );
		spellBook->applyDefinition( DefSection );
		nItem = spellBook;
	}
	else
	{
		nItem = new cItem;
		nItem->Init( true );
		nItem->applyDefinition( DefSection );
	}

	nItem->onCreate( Section );

	return nItem;
}

// Added by DarkStorm
bool cItem::onSingleClick( P_PLAYER Viewer )
{

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSingleClick( this, Viewer ) )
			return true;

	return false;
}

bool cItem::onDropOnItem( P_ITEM pItem )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// we are the item being dragged
		if( layer_ == 0x1E )
		{
			if( scriptChain[ i ]->onDropOnItem( pItem, this ) )
				return true;
		}
		else
		{
			if( scriptChain[ i ]->onDropOnItem( this, pItem ) )
				return true;
		}
	}

	return false;
}

bool cItem::onDropOnGround( const Coord_cl &pos )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onDropOnGround( this, pos ) )
			return true;

	return false;
}

bool cItem::onPickup( P_CHAR pChar )
{

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onPickup( pChar, this ) )
			return true;

	return false;
}

bool cItem::onDropOnChar( P_CHAR pChar )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onDropOnChar( pChar, this ) )
			return true;

	return false;
}

bool cItem::onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip )
{

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowToolTip( sender, this, tooltip ) )
			return true;

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_ITEM, EVENT_SHOWTOOLTIP );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onShowToolTip( sender, this, tooltip ) ) 
			return true;

	return false;
}


void cItem::processNode( const cElement *Tag )
{
	changed( SAVE );
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

	// <name>my Item</name>
	if( TagName == "name" )
		this->setName( Value );
	 
	// <identified>my magic item</identified>
	else if( TagName == "identified" )
		this->setName2( Value.latin1() );

	// <amount>120</amount>
	else if( TagName == "amount" )
	{
		this->setAmount( Value.toULong() );
		this->restock_ = Value.toULong(); // Maximumm sell-amount from the beginning
	}

	// <color>480</color>
	else if( TagName == "color" )
		this->setColor( Value.toUShort() );

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

	// <defense>10</defense>
	else if( TagName == "defense" )
		this->def_ = Value.toInt();

	// for convenience
	// <food>1</food>
	else if( TagName == "food" )
	{
		setType( 14 );
		setType2( Value.toUInt() );
	}

	// <type>10</type>
	else if( TagName == "type" )
		this->setType( Value.toUInt() );

	// <type2>10</type2>
	else if( TagName == "type2" )
		this->setType2( Value.toUInt() );

	// <weight>10</weight>
	else if( TagName == "weight" )
		this->setWeight( (UINT32)( Value.toFloat() * 10 ) );

	// <buyprice>10</buyprice>
	else if( TagName == "buyprice" )
	{
		this->buyprice_ = Value.toInt();
	}

	// <sellprice>10</sellprice>
	else if( TagName == "sellprice" )
		this->sellprice_ = Value.toInt();

	// <price>10</price>
//	else if( TagName == "price" )
//		this->price_ = Value.toInt();   --> this was supposed to be player vendor price, the player defines it! :)

	// <carve></carve> For corpses and item spawners
//	else if( TagName == "carve" )
//		this->carve_ = Value;
		
	// <restock>10</restock>
	else if( TagName == "restock" )
		this->restock_ = Value.toInt();

	// <durability>10</durabilty>
	else if( TagName == "durability" )
	{
		this->setMaxhp( Value.toLong() );
		this->setHp( this->maxhp() );
	}

	// <speed>10</speed>
	else if( TagName == "speed" )
		this->setSpeed( Value.toLong() );

	// <good>10</good>
	else if( TagName == "good" )
		this->good_ = Value.toInt();

	// <lightsource>10</lightsource>
	else if( TagName == "lightsource" )
		this->dir_ = Value.toUShort();

	// <more1>10</more1>
	else if( TagName == "more1" )
		this->more1_ = Value.toInt();

	// <more>10</more> <<<<< alias for more1
	else if( TagName == "more" )
		this->more1_ = Value.toInt();

	// <more2>10</more2>
	else if( TagName == "more2" )
		this->more2_ = Value.toInt();

	// <morex>10</morex>
	else if( TagName == "morex" )
		this->morex_ = Value.toInt();

	// <morey>10</morey>
	else if( TagName == "morey" )
		this->morey_ = Value.toInt();

	// <morez>10</morez>
	else if( TagName == "morez" )
		this->morez_ = Value.toInt();

	// <morexyz>10</morexyz>
	else if( TagName == "morexyz" )
	{
		QStringList Elements = QStringList::split( ",", Value );
		if( Elements.count() == 3 )
		{
			this->morex_ = Elements[ 0 ].toInt();
			this->morey_ = Elements[ 1 ].toInt();
			this->morez_ = Elements[ 2 ].toInt();
		}
	}

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
		this->priv_ |= 0x01;
	else if( TagName == "nodecay" )
		this->priv_ &= 0xFE;

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

	// <requires type="xx">2</requires>
	else if( TagName == "requires" )
	{
		if( !Tag->hasAttribute( "type" ) )
			return;

		QString Type = Tag->getAttribute( "type" );
			
		if( Type == "str" )
			this->st_ = Value.toULong();
		else if( Type == "dex" )
			this->dx_ = Value.toULong();
		else if( Type == "int" )
			this->in_ = Value.toULong();
	}

	// <visible />
	// <invisible />
	// <ownervisible />
	else if( TagName == "invisible" )
		this->visible_ = 2;
	else if( TagName == "visible" )
		this->visible_ = 0;
	else if( TagName == "ownervisible" )
		this->visible_ = 1;

	// <modifier type="xx">2</modifier>
	else if( TagName == "modifier" )
	{
		if( !Tag->hasAttribute( "type" ) )
			return;

		QString Type = Tag->getAttribute( "type" );
			
		if( Type == "str" )
			this->st2_ = Value.toShort();
		else if( Type == "dex" )
			this->dx2_ = Value.toShort();
		else if( Type == "int" )
			this->in2_ = Value.toShort();
	}

	// <dye />
	// <nodye />
	else if( TagName == "dye" )
		this->setDye(1);
	else if( TagName == "nodye" )
		this->setDye(0);

	// <corpse />
	// <nocorpse />
	else if( TagName == "corpse" )
		this->setCorpse( true );
	else if( TagName == "nocorpse" )
		this->setCorpse( false );

	// <id>12f9</id>
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

	// <accuracy>20</accuracy> value between 0 and 100
	else if( TagName == "accuracy" )
	{
		setAccuracy( Value.toUShort() );
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
		if( name_ == "#" )
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

	// <identified>%1 of Hardening</identified>
	else if( TagName == "identified" )
		name2_ = Value.arg( name2_ );

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
			setWeight( (INT32)ceil((float)weight() * Value.toFloat()) );
		else
			setWeight( weight() + Value.toInt() );
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

	// <requires type="xx">2</requires>
	else if( TagName == "requires" )
	{
		if( !Tag->hasAttribute( "type" ) )
			return;

		QString Type = Tag->getAttribute( "type" );
			
		if( Type == "str" )
		{
			if( Value.contains(".") || Value.contains(",") )
				st_ = (INT32)ceil((float)st_ * Value.toFloat());
			else
				this->st_ += Value.toLong();
		}
		else if( Type == "dex" )
		{
			if( Value.contains(".") || Value.contains(",") )
				dx_ = (INT32)ceil((float)dx_ * Value.toFloat());
			else
				this->dx_ += Value.toLong();
		}
		else if( Type == "int" )
		{
			if( Value.contains(".") || Value.contains(",") )
				in_ = (INT32)ceil((float)in_ * Value.toFloat());
			else
				this->in_ += Value.toLong();
		}
	}

	// <modifier type="xx">2</modifier>
	else if( TagName == "modifier" )
	{
		if( !Tag->hasAttribute( "type" ) )
			return;

		QString Type = Tag->getAttribute( "type" );
			
		if( Type == "str" )
		{
			if( Value.contains(".") || Value.contains(",") )
				st2_ = (INT32)ceil((float)st2_ * Value.toFloat());
			else
				this->st2_ += Value.toLong();
		}
		else if( Type == "dex" )
		{
			if( Value.contains(".") || Value.contains(",") )
				dx2_ = (INT32)ceil((float)dx2_ * Value.toFloat());
			else
				this->dx2_ += Value.toLong();
		}
		else if( Type == "int" )
		{
			if( Value.contains(".") || Value.contains(",") )
				in2_ = (INT32)ceil((float)in2_ * Value.toFloat());
			else
				this->in2_ += Value.toLong();
		}
	}

	// <color>480</color>
	else if( TagName == "color" )
		this->setColor( color() + Value.toUShort() );

	// <id>12f9</id>
	else if( TagName == "id" )
	{
		this->setId( id() + Value.toUShort() );

		// In addition to the normal behaviour we retrieve the weight of the
		// item here.
		setWeight( TileCache::instance()->getTile( id_ ).weight );
	}
}

void cItem::processContainerNode( const cElement *Tag )
{
	//item containers can be scripted like this:
	/*
	<contains>
		<item><inherit list="myList" /></item>
		<item><inherit id="myItem1" /><amount><random ... /></amount><color><colorlist><random...></colorlist></color></item>
		...
	</contains>
	*/
	std::vector< const cElement* > content;
	unsigned int i;

	for( i = 0; i < Tag->childCount(); ++i )
	{
		const cElement *childNode = Tag->getChild( i );

		if( childNode->name() == "item" )
		{
			content.push_back( childNode );
		}
		else if( childNode->name() == "getlist" && childNode->hasAttribute( "id" ) )
		{
			QStringList list = DefManager->getList( childNode->getAttribute( "id" ) );
			
			for( QStringList::iterator it = list.begin(); it != list.end(); it++ )
			{
				const cElement *lItem = DefManager->getDefinition( WPDT_ITEM, *it );
				content.push_back( lItem );
			}
		}		
	}
		
	for( i = 0; i < content.size(); i++ )
	{
			P_ITEM nItem = new cItem;
			nItem->Init( true );
			nItem->applyDefinition( content[ i ] );
			addItem( nItem );
	}
}

void cItem::showName( cUOSocket *socket )
{
	// End chars/npcs section
	
    if( onSingleClick( socket->player() ) )
        return;       
		
	QString itemname( "" );
	
	if( name_ != "#" )
		itemname = getName();

	// Add creator's mark (if any)
	if( !creator_.isEmpty() && madewith() )
		itemname.append( tr( " crafted by %2" ).arg( creator_ ) );

	// Amount information
	if( amount_ > 1 )
		itemname.append( tr( ": %1" ).arg( amount_ ) );

	// Show serials
	if( socket->player() && socket->player()->account() && socket->player()->account()->isShowSerials() )
		itemname.append( tr( " [%1]" ).arg( serial(), 8, 16 ) );

	// Pages
	if( type_ == 11 )
	{
		cBook *pBook = dynamic_cast< cBook* >( this );

		// Append pages
		if( pBook )
			itemname.append( tr( " [pages: %1]" ).arg( pBook->pages() ) );
	}

	// Show charges for wands only if they are identified
	if( type() == 15 && name_ == name2_ )
			itemname.append( tr( " [%1 charge%2]" ).arg( morez_ ).arg( ( morez_ > 1 ) ? "s" : "" ) );
	else if( type() == 404 || type() == 181 )
		itemname.append( tr( " [%1 charge%2]" ).arg( morex_ ).arg( ( morex_ > 1 ) ? "s" : "" ) );

	// Try a localized Message
	if( name_ == "#" )
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
	if( corpse() )
	{
		if( more2_ == 1 )
			socket->showSpeech( this, tr( "[Innocent]" ), 0x005A );
		else if( more2_ == 2 )
			socket->showSpeech( this, tr( "[Criminal]" ), 0x03B2 );
		else if( more2_ == 3 )
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
		UINT16 tWeight = totalweight_;
		
		if( weight_ == 255 )
			tWeight -= 255;

		QString message = tr( "[%1 items, %2 stones]" ).arg( content_.size() ).arg( tWeight/10 );

		socket->showSpeech( this, message, 0x3B2 );
	}
}

// This either sends a ground-item or a backpack item
void cItem::update( cUOSocket *mSock )
{
	if( free )
	{
		removeFromView( false );
		return;
	}

	// Items on Ground
	if( isInWorld() )
	{
		// we change the packet during iteration, so we have to
		// recompress it
		cUOTxSendItem* sendItem = new cUOTxSendItem();
		sendItem->setSerial( serial() );
		sendItem->setId( id() );
		sendItem->setAmount( amount() );
		sendItem->setColor( color() );
		sendItem->setCoord( pos() );
		sendItem->setDirection( dir_ );

		if( mSock )
		{
			P_PLAYER pChar = mSock->player();

			// Only send to sockets in range
			if( !pChar || !pChar->account() || ( pChar->dist( this ) > pChar->visualRange() ) )
				return;

			// Completely invisible
			if( ( visible_ == 2 ) && !pChar->isGM() )
				return;

			// Visible to owners and GMs only
			else if( ( visible_ == 1 ) && !pChar->Owns( this ) && !pChar->isGM() )
				return;
	        
			if( isAllMovable() )
				sendItem->setFlags( 0x20 );

			else if( pChar->account()->isAllMove() )
				sendItem->setFlags( 0x20 );

			else if( isOwnerMovable() && pChar->Owns( this ) )
				sendItem->setFlags( 0x20 );

			if( ( visible() > 0 ) && !pChar->Owns( this ) )
				sendItem->setFlags( sendItem->flags() | 0x80 );

			// TODO: Insert code for view-multi-as-icon & view-lightsource-as-candle
		
			mSock->send( sendItem );
			sendTooltip( mSock );
		}
		else
		{
			for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
			{
				P_PLAYER pChar = mSock->player();
	
				// Only send to sockets in range
				if( !pChar || !pChar->account() || ( pChar->dist( this ) > pChar->visualRange() ) )
					continue;
	
				// Completely invisible
				if( ( visible() == 2 ) && !pChar->isGM() )
					continue;
	
				// Visible to owners and GMs only
				else if( ( visible() == 1 ) && !pChar->Owns( this ) && !pChar->isGM() )
					continue;

				cUOTxSendItem sockSendItem( *sendItem );
	            
				if( isAllMovable() )
					sockSendItem.setFlags( 0x20 );
				else if( pChar->account()->isAllMove() )
					sockSendItem.setFlags( 0x20 );
				else if( ( isOwnerMovable() || isLockedDown() ) && pChar->Owns( this ) )
					sockSendItem.setFlags( 0x20 );
	
				if( ( visible() > 0 ) && !pChar->Owns( this ) )
					sockSendItem.setFlags( sockSendItem.flags() | 0x80 );
	
				// TODO: Insert code for view-multi-as-icon & view-lightsource-as-candle
				
				mSock->send( &sockSendItem );
				sendTooltip( mSock );
			}
			delete sendItem;
		}
	}
	// equipped items
	else if( container_ && container_->isChar() )
	{
		cUOTxCharEquipment equipItem;
		equipItem.fromItem( this );
		P_CHAR pOwner = dynamic_cast<P_CHAR>( container_ );

		if( !pOwner )
			return;

		for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		{
			P_PLAYER pChar = socket->player();

			// Only send to sockets in range
			if( !pChar || !pChar->inRange( pOwner, pChar->visualRange() ) )
				continue;

			socket->send( &equipItem );
			sendTooltip( socket );
		}
	}
	// items in containers
	else if( container_ && container_->isItem() )
	{
		cUOTxAddContainerItem contItem;
		contItem.fromItem( this );	

		P_ITEM iCont = getOutmostItem();
		cUObject *oCont = iCont;

		if( iCont && iCont->container() && iCont->container()->isChar() )
			oCont = dynamic_cast<P_CHAR>( iCont->container() );

		if( !oCont )
			return;

		for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		{
			P_PLAYER pChar = socket->player();

			if( !pChar || ( pChar->dist( oCont ) > pChar->visualRange() ) )
				continue;

			socket->send( &contItem );
			sendTooltip( socket );
		}
	}
}

P_ITEM cItem::dupe()
{
	if( corpse() )
		return NULL;

	P_ITEM nItem = new cItem( (*this) );
	nItem->setSerial( World::instance()->findItemSerial() );
	
	// We wont dupe items on chars without proper handling
	P_CHAR pWearer = dynamic_cast<P_CHAR>( nItem->container() );
	if( pWearer )
	{
		nItem->setLayer( 0 );
		nItem->setContainer( 0 );
		nItem->moveTo( pWearer->pos() );
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
void cItem::setWeight( SI16 nValue )
{
	setTotalweight( totalweight_ - ( amount_ * weight_ ) );
	changed( SAVE|TOOLTIP );
	weight_ = nValue;
	setTotalweight( totalweight_ + ( amount_ * weight_ ) );
}

// This subtracts the weight of the top-container
// And then readds the new weight
void cItem::setTotalweight( INT32 data )
{
	//if( data < 0 )
		// FixWeight!

	if ( container_ )
	{
		if( container_->isChar() )
		{
			P_CHAR pChar = dynamic_cast<P_CHAR>( container_ );
			if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
				pChar->setWeight( pChar->weight() - totalweight_ );
		}
		else if( container_->isItem() )
		{
			P_ITEM pItem = dynamic_cast<P_ITEM>( container_ );
			if( pItem )
				pItem->setTotalweight( pItem->totalweight() - totalweight_ );
		}
	}

	changed( SAVE|TOOLTIP );
	totalweight_ = data;

	if ( container_ )
	{
		if( container_->isChar() )
		{
			P_CHAR pChar = dynamic_cast<P_CHAR>( container_ );
			if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
				pChar->setWeight( pChar->weight() + totalweight_ );
		}
		else if( container_->isItem() )
		{
			P_ITEM pItem = dynamic_cast<P_ITEM>( container_ );
			if( pItem )
				pItem->setTotalweight( pItem->totalweight() + totalweight_ );
		}
	}
}

void cItem::applyRank( UI08 rank )
{
	// Variables to change: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
	double minmod = (double)(rank-1) * 10.0f / 100.0f;
	double maxmod = (double)(rank+1) * 10.0f / 100.0f;
	UINT16 minhp_ = (UINT16)floor( minmod * (double)hp() );
	UINT16 maxhp_ = (UINT16)floor( maxmod * (double)hp() );
	INT16  minlodam = (UINT16)floor( minmod * (double)lodamage() );
	INT16  maxlodam = (UINT16)floor( maxmod * (double)lodamage() );
	INT16  minhidam = (UINT16)floor( minmod * (double)hidamage() );
	INT16  maxhidam = (UINT16)floor( maxmod * (double)hidamage() );
	UINT16 mindef = (UINT16)floor( minmod * (double)def() );
	UINT16 maxdef = (UINT16)floor( maxmod * (double)def() );
	
	changed( SAVE|TOOLTIP );
	setLodamage( RandomNum( minlodam, maxlodam ) );
	setHidamage( RandomNum( minhidam, maxhidam ) );
	def_ = RandomNum( mindef, maxdef );
	setMaxhp( RandomNum( minhp_, maxhp_ ) );
	setHp( maxhp() );
	this->rank_ = rank_;
}

void cItem::talk( const QString &message, UI16 color, UINT8 type, bool autospam, cUOSocket* socket )
{
	if( autospam )
	{
		if( antispamtimer() < uiCurrentTime )
			setAntispamtimer( uiCurrentTime + MY_CLOCKS_PER_SEC*10 );
		else 
			return;
	}

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

bool cItem::wearOut()
{
	if( RandomNum( 1, 4 ) == 4 )
		setHp( hp() - 1 );

	if( hp() <= 0 )
	{
		// Get the owner of the item
		P_PLAYER pOwner = dynamic_cast<P_PLAYER>(getOutmostChar());

		if( pOwner )
		{
			pOwner->message( tr( "*You destroy your %1*" ).arg( getName( true ) ), 0x23 );

			// Send it to the people in range
			cUOSocket *mSock = 0;
			for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
			{
				if( !mSock || mSock == pOwner->socket() )
					continue;

				if( mSock->player() && mSock->player()->inRange( pOwner, mSock->player()->visualRange() ) )
					mSock->showSpeech( pOwner, tr( "You see %1 destroying his %2" ).arg( pOwner->name() ).arg( getName( true ) ), 0x23, 3, cUOTxUnicodeSpeech::Emote );
			}
		}

		Items->DeleItem( this );
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
			while( pi = pit.current() )
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

	if( pi->objectID() == "cGuildStone" ) // register as guild as well
		guilds.push_back(pi->serial());

	// Set the outside indices
	pi->SetSpawnSerial( pi->spawnserial );
	pi->SetOwnSerial( pi->ownSerial() );

	if( pi->maxhp() == 0) 
		pi->setMaxhp( pi->hp() );
	if( pi->name() == "#" )
		pi->setName( QString::null );
}

void cItem::load( char **result, UINT16 &offset )
{
	cUObject::load( result, offset ); // Load the items we inherit from first

	// Broken Serial?
	if( !isItemSerial( serial() ) )
		throw QString( "Item has invalid character serial: 0x%1" ).arg( serial(), 0, 16 );

	id_ = atoi( result[offset++] );
	name_ = result[offset++];
	name2_ = result[offset++];
	creator_ = result[offset++];
	madewith_ = atoi( result[offset++] );
	color_ = atoi( result[offset++] );
	
	//  Warning, ugly optimization ahead, if you have a better idea, we want to hear it. 
	//  For load speed and memory conservation, we will store the SERIAL of the container
	//  here and then right after load is done we replace that value with it's memory address
	//  as it should be.
	SERIAL containerSerial = atoi( result[offset++] );

	if( containerSerial != INVALID_SERIAL ) // if it's invalid, we won't set.
		container_ = (cUObject*)containerSerial;
	
	// ugly optimization ends here.
	
	layer_ = atoi( result[offset++] );
	type_ = atoi( result[offset++] );
	type2_ = atoi( result[offset++] );
	offspell_ = atoi( result[offset++] );
	more1_ = atoi( result[offset++] );
	more2_ = atoi( result[offset++] );
	more3_ = atoi( result[offset++] );
	more4_ = atoi( result[offset++] );
	morex_ = atoi( result[offset++] );
	morey_ = atoi( result[offset++] );
	morez_ = atoi( result[offset++] );
	amount_ = atoi( result[offset++] );
	doordir_ = atoi( result[offset++] );
	dye_ = atoi( result[offset++] );
	decaytime_ = atoi( result[offset++] );
	if( decaytime_ > 0 ) 
		decaytime_ += uiCurrentTime;
	att_ = atoi( result[offset++] );
	def_ = atoi( result[offset++] );
	hidamage_ = atoi( result[offset++] );
	lodamage_ = atoi( result[offset++] );
	st_ = atoi( result[offset++] );
	time_unused = atoi( result[offset++] );
	weight_ = atoi( result[offset++] );
	hp_ = atoi( result[offset++] );
	maxhp_ = atoi( result[offset++] );
	rank_ = atoi( result[offset++] );
	st2_ = atoi( result[offset++] );
	dx_ = atoi( result[offset++] );
	dx2_ = atoi( result[offset++] );
	in_ = atoi( result[offset++] );
	in2_ = atoi( result[offset++] );
	speed_ = atoi( result[offset++] );
	poisoned_ = atoi( result[offset++] );
	magic_ = atoi( result[offset++] );
	ownserial_ = atoi( result[offset++] );
	visible_ = atoi( result[offset++] );
	spawnserial = atoi( result[offset++] );
	dir_ = atoi( result[offset++] );
	priv_ = atoi( result[offset++] );
	sellprice_ = atoi( result[offset++] );
	buyprice_ = atoi( result[offset++] );
	restock_ = atoi( result[offset++] );
	disabled_ = atoi( result[offset++] );
	spawnregion_ = result[offset++];
	good_ = atoi( result[offset++] );
	desc_ = result[offset++];
	accuracy_ = atoi( result[offset++] );

	// Their own weight should already be set.
	totalweight_ = amount_ * weight_;

	itemRegisterAfterLoading( this );
	changed_ = false;
}

void cItem::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cUObject::buildSqlString( fields, tables, conditions );
	fields.push_back( "items.id,items.name,items.name2,items.creator,items.sk_name,items.color,items.cont,items.layer,items.type,items.type2,items.offspell,items.more1,items.more2,items.more3,items.more4,items.morex,items.morey,items.morez,items.amount,items.doordir,items.dye,items.decaytime,items.att,items.def,items.hidamage,items.lodamage,items.st,items.time_unused,items.weight,items.hp,items.maxhp,items.rank,items.st2,items.dx,items.dx2,items.intelligence,items.intelligence2,items.speed,items.poisoned,items.magic,items.owner,items.visible,items.spawn,items.dir,items.priv,items.sellprice,items.buyprice,items.restock,items.disabled,items.spawnregion,items.good,items.description,items.accuracy" ); // for now! later on we should specify each field
	tables.push_back( "items" );
	conditions.push_back( "uobjectmap.serial = items.serial" );
}


void cItem::addItem( cItem* pItem, bool randomPos, bool handleWeight, bool noRemove )
{
	if( !pItem )
		return;

	if( pItem == this )
	{
		clConsole.log( LOG_WARNING, QString( "Rejected putting an item into itself (%1)" ).arg( serial_, 0, 16 ) );
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
	pItem->changed( SAVE );
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
	changed( SAVE );
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
	if( container_ && container_->isChar() )
	{		
		return dynamic_cast< P_CHAR >( container_ );
	}
	else if( container_ && container_->isItem() )
	{
		P_ITEM pCont = dynamic_cast< P_ITEM >( container_ );

		if( pCont )
			return pCont->getOutmostChar();
		else
			return 0;
	}
	else
		return 0;		
}

// If we change the amount, the weight changes as well
void cItem::setAmount( UI16 nValue )
{
	setTotalweight( totalweight_ + ( ( nValue - amount_ ) * weight_ ) );
	amount_ = nValue;	
	changed( SAVE|TOOLTIP );
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
	changed( SAVE|TOOLTIP );
	SET_INT_PROPERTY( "id", id_ )
	else SET_INT_PROPERTY( "color", color_ )
	
	// Amount needs weight handling
	else if( name == "amount" )
	{
		int val = value.toInt();
		if( val <= 0 )
		{
			Items->DeleItem( this );
			return 0;
		}

		int diff = val - amount_;
		setTotalweight( totalweight_ + diff * weight_ );
		amount_ = val;
		return 0;
	}

	else SET_STR_PROPERTY( "name2", name2_ )
	else SET_STR_PROPERTY( "name", name_ )
	else SET_INT_PROPERTY( "layer", layer_ )
	else SET_INT_PROPERTY( "type", type_ )
	else SET_INT_PROPERTY( "type2", type2_ )
	else SET_INT_PROPERTY( "offspell", offspell_ )
	else SET_INT_PROPERTY( "speed", speed_ )
	else SET_INT_PROPERTY( "lodamage", lodamage_ )
	else SET_INT_PROPERTY( "hidamage", hidamage_ )
	else if( name == "weight" )
	{
		setWeight( value.toInt() );
		return 0;
	}
	else if( name == "stones" )
	{
		setWeight( value.toInt() * 10 );
		return 0;
	}
	else SET_INT_PROPERTY( "health", hp_ )
	else SET_INT_PROPERTY( "maxhealth", maxhp_ )
	else SET_STR_PROPERTY( "spawnregion", spawnregion_ )
	else SET_INT_PROPERTY( "owner", ownserial_ )

	else if( name == "totalweight" )
	{
		setTotalweight( value.toInt() );
		return 0;
	}
	else SET_INT_PROPERTY( "antispamtimer", antispamtimer_ )
	else SET_INT_PROPERTY( "accuracy", accuracy_ )
	
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

	else SET_INT_PROPERTY( "more1", more1_ )
	else SET_INT_PROPERTY( "more2", more2_ )
	else SET_INT_PROPERTY( "more3", more3_ )
	else SET_INT_PROPERTY( "more4", more4_ )
	else SET_INT_PROPERTY( "morex", morex_ )
	else SET_INT_PROPERTY( "morey", morey_ )
	else SET_INT_PROPERTY( "morez", morez_ )
	else SET_INT_PROPERTY( "doordir", doordir_ )
	else SET_INT_PROPERTY( "dooropen", dooropen_ )
	else SET_INT_PROPERTY( "dye", dye_ )
	else SET_INT_PROPERTY( "attack", att_ )
	else SET_INT_PROPERTY( "defense", def_ )
	else SET_INT_PROPERTY( "strength", st_ )
	else SET_INT_PROPERTY( "dexterity", dx_ )
	else SET_INT_PROPERTY( "intelligence", in_ )
	else if( name == "strength2" )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( container_ );
		if( pChar )
			pChar->removeItemBonus( this );

		st2_ = value.toInt();

		if( pChar )
			pChar->giveItemBonus( this );
		
		return 0;
	}
	else if( name == "dexterity2" )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( container_ );
		if( pChar )
			pChar->removeItemBonus( this );

		dx2_ = value.toInt();

		if( pChar )
			pChar->giveItemBonus( this );

		return 0;
	}
	else if( name == "intelligence2" )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( container_ );
		if( pChar )
			pChar->removeItemBonus( this );

		in2_ = value.toInt();

		if( pChar )
			pChar->giveItemBonus( this );

		return 0;
	}
	else SET_INT_PROPERTY( "gatetime", gatetime_ )
	else SET_INT_PROPERTY( "gatenumber", gatenumber_ )
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

	else SET_INT_PROPERTY( "spawn", spawnserial )
	else SET_INT_PROPERTY( "direction", dir_ )
	else SET_INT_PROPERTY( "sellprice", sellprice_ )
	else SET_INT_PROPERTY( "buyprice", buyprice_ )
	else SET_INT_PROPERTY( "restock", restock_ )
	else SET_INT_PROPERTY( "disabled", disabled_ )
	else SET_INT_PROPERTY( "poisoned", poisoned_ )
	else SET_INT_PROPERTY( "rank", rank_ )
	else SET_STR_PROPERTY( "creator", creator_ )
	else SET_INT_PROPERTY( "good", good_ )
	else SET_INT_PROPERTY( "rndvaluerate", rndvaluerate_ )
	else SET_INT_PROPERTY( "madewith", madewith_ )
	else SET_STR_PROPERTY( "description", desc_ )
	else SET_INT_PROPERTY( "incognito", incognito )
	else SET_INT_PROPERTY( "timeunused", time_unused )
	else SET_INT_PROPERTY( "timeusedlast", timeused_last )
	else SET_INT_PROPERTY( "magic", magic_ )
	else SET_INT_PROPERTY( "visible", visible_ )

	// Flags
	else if( name == "decay" )
	{
		if( value.toInt() )
			priv_ |= 0x01;
		else
			priv_ &= ~0x01;
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
	else GET_PROPERTY( "color", color_ )
	else GET_PROPERTY( "amount", amount_ )
	else GET_PROPERTY( "name2", name2_ )
	else GET_PROPERTY( "name", name_ )
	else GET_PROPERTY( "layer", layer_ )
	// Flag properties are set elsewhere!!
	else GET_PROPERTY( "type", type_ )
	else GET_PROPERTY( "type2", type2_ )
	else GET_PROPERTY( "offspell", offspell_ )
	else GET_PROPERTY( "speed", speed_ )
	else GET_PROPERTY( "lodamage", lodamage_ )
	else GET_PROPERTY( "hidamage", hidamage_ )
	else GET_PROPERTY( "weight", weight_ )
	else GET_PROPERTY( "stones", weight_ / 10 )
	else GET_PROPERTY( "health", hp_ )
	else GET_PROPERTY( "maxhealth", maxhp_ )
	else GET_PROPERTY( "spawnregion", spawnregion_ )
	else GET_PROPERTY( "owner", owner() )
	else GET_PROPERTY( "totalweight", totalweight_ )
	else GET_PROPERTY( "antispamtimer", (int)antispamtimer_ )
	else GET_PROPERTY( "accuracy", accuracy_ )
	
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

	else GET_PROPERTY( "more1", more1_ )
	else GET_PROPERTY( "more2", more2_ )
	else GET_PROPERTY( "more3", more3_ )
	else GET_PROPERTY( "more4", more4_ )
	else GET_PROPERTY( "morex", (int)morex_ )
	else GET_PROPERTY( "morey", (int)morey_ )
	else GET_PROPERTY( "morez", (int)morez_ )
	else GET_PROPERTY( "doordir", doordir_ )
	else GET_PROPERTY( "dooropen", dooropen_ )
	else GET_PROPERTY( "dye", dye_ )
	else GET_PROPERTY( "attack", (int)att_ )
	else GET_PROPERTY( "defense", (int)def_ )
	else GET_PROPERTY( "strength", st_ )
	else GET_PROPERTY( "strength2", st2_ )
	else GET_PROPERTY( "dexterity", dx_ )
	else GET_PROPERTY( "dexterity2", dx2_ )
	else GET_PROPERTY( "intelligence", in_ )
	else GET_PROPERTY( "intelligence2", in2_ )
	else GET_PROPERTY( "gatetime", (int)gatetime_ )
	else GET_PROPERTY( "gatenumber", gatenumber_ )
	else GET_PROPERTY( "decaytime", (int)decaytime_ )

	// Visible
	else GET_PROPERTY( "visible", visible_ == 0 ? 1 : 0 )
	else GET_PROPERTY( "ownervisible", visible_ == 1 ? 1 : 0 )
	else GET_PROPERTY( "spawn", FindItemBySerial( spawnserial ) )

	else GET_PROPERTY( "direction", dir_ )
	else GET_PROPERTY( "buyprice", buyprice_ )
	else GET_PROPERTY( "sellprice", sellprice_ )
	else GET_PROPERTY( "restock", restock_ )
	else GET_PROPERTY( "disabled", (int)disabled_ )
	else GET_PROPERTY( "poisoned", (int)poisoned_ )
	else GET_PROPERTY( "rank", rank_ )
	else GET_PROPERTY( "creator", creator_ )
	else GET_PROPERTY( "good", good_ )
	else GET_PROPERTY( "rndvaluerate", rndvaluerate_ )
	else GET_PROPERTY( "madewith", madewith_ )
	else GET_PROPERTY( "description", desc_ )
	else GET_PROPERTY( "incognito", incognito ? 1 : 0 )
	else GET_PROPERTY( "timeunused", (int)time_unused )
	else GET_PROPERTY( "timeusedlast", (int)timeused_last )
	else GET_PROPERTY( "magic", magic_ )

	// Flags
	else GET_PROPERTY( "decay", priv_ & 0x01 ? 1 : 0 )
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
	// Don't send the tooltip if we're a supposed-to-be-static item
	tile_st tile = TileCache::instance()->getTile( id_ );

	if( tile.weight == 255 && !isAllMovable() )
		return;

	cUObject::sendTooltip( mSock );
}
