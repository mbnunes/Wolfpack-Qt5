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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "wolfpack.h"
#include "network/uosocket.h"
#include "SndPkg.h"
#include "utilsys.h"
#include "iserialization.h"
#include "debug.h"
#include "items.h"
#include "books.h"
#include "regions.h"
#include "tilecache.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "wpdefaultscript.h"
#include "mapstuff.h"
#include "network.h"
#include "classes.h"

#undef  DBGFILE
#define DBGFILE "items.cpp"

P_CHAR cItem::owner( void )
{
	return FindCharBySerial( ownserial ); 
}

void cItem::setOwner( P_CHAR nOwner )
{
	ownserial = ( nOwner == NULL ) ? INVALID_SERIAL : nOwner->serial;
}

// Is the Item pileable?
bool cItem::isPileable()
{
	tile_st tile = cTileCache::instance()->getTile( id_ );
	return tile.flag2&0x08;
}

void cItem::toBackpack( P_CHAR pChar )
{
	P_ITEM pPack = pChar->getBackpack();
	
	// Pack it to the ground
	if( !pPack )
	{
		setContSerial( INVALID_SERIAL );
		moveTo( pChar->pos );
		update();
	}
	// Or to the backpack
	else
		pPack->AddItem( this );
}

// Gets the corpse an item is in
P_ITEM cItem::getCorpse( void )
{
	if( isCharSerial( contserial ) || isInWorld() )
		return NULL;

	P_ITEM Cont = GetOutmostCont( this );

	if( !Cont || !Cont->corpse() )
		return NULL;

	return Cont;
}

// constructor
cItem::cItem( cItem &src )
{
	this->name_ = src.name_;
	this->name2_ = src.name2_;
	this->creator = src.creator;
	this->incognito = src.incognito;
	this->madewith = src.madewith;
	this->rank = src.rank;
	this->good = src.good;
	this->rndvaluerate = src.rndvaluerate;

	this->multis = src.multis;
	this->free = false;
	this->setId(src.id());
	this->pos = src.pos;
	this->color_ = src.color_;
	this->setContSerial(src.contserial);
	this->layer_ = src.layer_;
	this->type_ = src.type_;
	this->type2_ = src.type2_;
	this->offspell_ = src.offspell_;
	this->weight_ = src.weight_;
	this->more1 = src.more1;
	this->more2 = src.more2;
	this->more3 = src.more3;
	this->more4 = src.more4;
	this->moreb1_ = src.moreb1_;
	this->moreb2_ = src.moreb2_;
	this->moreb3_ = src.moreb3_;
	this->moreb4_ = src.moreb4_;
	this->morex = src.morex;
	this->morey = src.morey;;
	this->morez = src.morez;
	this->amount_ = src.amount_;
	this->amount2_ = src.amount2_;
	this->doordir = src.doordir;
	this->dooropen = src.dooropen;
	this->dye = src.dye;
	this->carve_ = src.carve_;
	this->att = src.att;
	this->def = src.def;
	this->lodamage_=src.lodamage_;
	this->hidamage_=src.hidamage_;
	this->racehate_ = src.racehate_;
	this->smelt_=src.smelt_;
	this->hp_ = src.hp_;
	this->maxhp_=src.maxhp_;
	this->st=src.st;
	this->st2=src.st2;
	this->dx=src.dx;
	this->dx2=src.dx2;
	this->in=src.in;
	this->in2=src.in2;
	this->speed_=src.speed_;
	this->magic=src.magic;
	this->gatetime=src.gatetime;
	this->gatenumber=src.gatenumber;
	this->decaytime = src.decaytime;
	this->setOwnSerialOnly(src.ownserial);
	this->visible=src.visible;
	this->spawnserial=src.spawnserial;
	this->dir=src.dir;
	this->priv=src.priv;
	this->value=src.value;
	this->restock=src.restock;
	this->trigger=src.trigger;
	this->trigtype=src.trigtype;
	this->disabled=src.disabled;
	this->disabledmsg = src.disabledmsg;
	this->tuses=src.tuses;
	this->poisoned=src.poisoned;
	this->murderer_ = src.murderer_;
 	this->murdertime=src.murdertime;
    this->glow=src.glow;
    this->glow_effect=src.glow_effect;
    this->glow_color = src.glow_color;
	this->time_unused=src.time_unused;
	this->timeused_last=getNormalizedTime();
	this->setSpawnRegion( src.spawnregion() );
	this->desc = src.desc;
	setTotalweight( src.totalweight() );

	this->tags = src.tags;
}

inline QString cItem::objectID() const
{
	return "ITEM";
}

void cItem::startDecay()			
{
	if( contserial != INVALID_SERIAL )
		return;

	this->decaytime = SrvParams->decayTime()*MY_CLOCKS_PER_SEC+uiCurrentTime;
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
long cItem::ReduceAmount(const short amt)
{
	UINT16 rest = 0;
	if( amount_ > amt )
	{
		setAmount( amount_ - amt );
		update();
	}
	else
	{
		Items->DeleItem(this);
		rest = amt - amount_;
	}

	return rest;
}

void cItem::setContSerial( SERIAL nValue )
{
	// If the item is in the bank or any sell-container it's NOT counted as char-weight
	// bool inBank = ( outmostCont && 	( outmostCont->contserial == pChar->serial ) && ( outmostCont->layer() >= 0x1A ) );
	P_CHAR oldOwner = 0;
	P_CHAR newOwner = 0;

	if( contserial != INVALID_SERIAL )
	{
		if( isItemSerial( contserial ) )
		{
			P_ITEM pItem = FindItemBySerial( contserial );

			if( pItem )
			{
				pItem->setTotalweight( pItem->totalweight() - totalweight_ );
				oldOwner = GetPackOwner( pItem, 64 ); // Try to find it at least, otherwise its 0
			}
		}
		else if( isCharSerial( contserial ) )
		{
			P_CHAR pChar = FindCharBySerial( contserial );

			if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
			{
				pChar->setWeight( pChar->weight() - weight_ );
				oldOwner = pChar;
			}
		}

		contsp.remove( contserial, serial );
	}

	contserial = nValue;

	if( contserial != INVALID_SERIAL )
	{
		// Get the New owner only if we're taking an item along (no bank no sell conts. etc.)
		if( isItemSerial( contserial ) )
		{
			P_ITEM pItem = FindItemBySerial( contserial );

			if( pItem )
			{
				pItem->setTotalweight( pItem->totalweight() + totalweight_ );
				newOwner = GetPackOwner( pItem, 64 );
			}
		}
		else if( isCharSerial( contserial ) )
		{
			P_CHAR pChar = FindCharBySerial( contserial );

			if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
			{
				pChar->setWeight( pChar->weight() + totalweight_ );
				newOwner = pChar;
			}
		}

		contsp.insert( contserial, serial );		
	}

	// There was an owner change
	if( oldOwner != newOwner )
	{
		if( oldOwner && oldOwner->socket() )
			oldOwner->socket()->sendStatWindow();

		if( newOwner && newOwner->socket() )
			newOwner->socket()->sendStatWindow();
	}
}

void cItem::setOwnSerialOnly(long ownser)
{
	ownserial=ownser;
}

void cItem::SetOwnSerial(long ownser)
{
	if (ownserial != INVALID_SERIAL)	// if it was set, remove the old one
		ownsp.remove(ownserial, serial);
	
	setOwnSerialOnly(ownser);

	if (ownser != INVALID_SERIAL)		// if there is an owner, add it
		ownsp.insert(ownserial, serial);
}

void cItem::SetSpawnSerial(long spawnser)
{
	if (spawnserial != INVALID_SERIAL)	// if it was set, remove the old one
		spawnsp.remove(spawnserial, this->serial);

	spawnserial=spawnser;

	if (spawnser!=-1)		// if there is a spawner, add it
		spawnsp.insert(spawnserial, this->serial);
}

void cItem::SetMultiSerial(long mulser)
{
	if (this->multis != INVALID_SERIAL)	// if it was set, remove the old one
		imultisp.remove(this->multis, this->serial);

	this->multis = mulser;

	if (this->multis != INVALID_SERIAL)		// if there is multi, add it
		imultisp.insert(this->multis, this->serial);
}

void cItem::MoveTo(int newx, int newy, signed char newz)
{
	mapRegions->Remove(this);
	pos.x=newx;
	pos.y=newy;
	pos.z=newz;
	mapRegions->Add(this);
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

bool cItem::AddItem(cItem* pItem, short xx, short yy)	// Add Item to container
{
	if( !pItem ) 
		return false;

	pItem->setContSerial( serial );
	
	// use the given position
	if( xx != -1 ) 
	{
		pItem->pos.x = xx;
		pItem->pos.y = yy;
		pItem->pos.z = 9;
		pItem->update();
	}	// no pos given
	else		
	{
		if( !this->ContainerPileItem( pItem ) ) // try to pile
		{
			pItem->SetRandPosInCont( this ); // not piled, random pos
			pItem->update();
		}
	}	
	return true;
}

bool cItem::PileItem(cItem* pItem)	// pile two items
{
	if (!(isPileable() && pItem->isPileable() &&
		this->serial!=pItem->serial &&
		this->id()==pItem->id() &&
		this->color() == pItem->color() ))
		return false;	//cannot stack.

	if (this->amount() + pItem->amount() > 65535)
	{
		pItem->pos.x=this->pos.x;
		pItem->pos.y=this->pos.y;
		pItem->pos.z=9;
		pItem->setAmount( (this->amount()+pItem->amount()) - 65535 );
		this->setAmount( 65535 );
		pItem->update();
	}
	else
	{
		this->setAmount( this->amount()  + pItem->amount() );
		Items->DeleItem( pItem );
	}
	
	update();
	return true;
}

bool cItem::ContainerPileItem(cItem* pItem)	// try to find an item in the container to stack with
{
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(this->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if ( pi == NULL )
		{
			contsp.remove( this->serial, vecContainer[ci] ); // remove invalid entrie
			continue; // skip to next.
		}
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
	pos.x = RandomNum(18, 118);
	pos.z=9;

	switch (k) 
	{
	case 1: pos.y = RandomNum(50, 100);		break;
	case 2: pos.y = RandomNum(30, 80);		break;
	case 3: pos.y = RandomNum(100, 140);	break;
	case 4: pos.y = RandomNum(60, 140);
			pos.x = RandomNum(60, 140);		break;
	case 5: pos.y = RandomNum(85, 160);
			pos.x = RandomNum(20, 70);		break;
	default: pos.y = RandomNum(30, 80);
	}
}

/*!
  \internal
  This function will search the given container and subcontainers by serial
  and return the sum of items found by the given pair of ID (model number) and
  color.
*/
static int ContainerCountItems(const int serial, short id, short color)
{
	unsigned int ci=0; 
	int total=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (!pi || pi->free)			// just to be sure ;-)
			continue;
		if( pi->type() == 1 )		// a subcontainer ?
		{
			total += ContainerCountItems(pi->serial, id, color);
			continue;
		}
		if (pi->id()==id &&
			(color==-1 || pi->color() == color))
			total += pi->amount();
	}
	return total;
}


int cItem::CountItems(short ID, short col)
{
	return ContainerCountItems(serial, ID, col);
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
	unsigned int ci=0;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->type()==1)
			rest=pi->DeleteAmount(rest, _id, _color);
		if (pi->id() == _id && ( _color == 0 || ( pi->color() == _color ) ) )
			rest=pi->ReduceAmount(rest);
		if (rest<=0)
			break;
	}
	return rest;
}

void cItem::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		unsigned short temp;
		archive.read("id",			temp);			setId(temp);
		archive.read("name",		name_);
		archive.read("name2",		name2_);
		archive.read("creator",		creator);
		archive.read("sk_name",		madewith);
		archive.read("color",		color_);
		archive.read("cont",		contserial);
		archive.read("layer",		layer_);
		archive.read("type",		type_ );
		archive.read("type2",		type2_);
		archive.read("offspell",	offspell_);
		archive.read("more1",		more1);
		archive.read("more2",		more2);
		archive.read("more3",		more3);
		archive.read("more4",		more4);
		archive.read("moreb1",		moreb1_);
		archive.read("moreb2",		moreb2_);
		archive.read("moreb3",		moreb3_);
		archive.read("moreb4",		moreb4_);
		archive.read("morex",		morex);
		archive.read("morey",		morey);
		archive.read("morez",		morez);
		archive.read("amount",		amount_);
		archive.read("doordir",		doordir);
		archive.read("dye",			dye);
		archive.read("decaytime",	decaytime);
		if ( decaytime > 0 )
			decaytime += uiCurrentTime;
		archive.read("att",			att);
		archive.read("def",			def);
		archive.read("hidamage",	hidamage_);
		archive.read("lodamage",	lodamage_);
		archive.read("racehate",	racehate_);
		archive.read("st",			st);
		archive.read("time_unused",	time_unused);

		archive.read("weight",		weight_);
		archive.read("hp",			hp_);
		archive.read("maxhp",		maxhp_);
		archive.read("rank",		rank);
		archive.read("st2",			st2);
		archive.read("dx",			dx);
		archive.read("dx2",			dx2);
		archive.read("in",			in);
		archive.read("in2",			in2);
		archive.read("speed",		speed_);
		archive.read("poisoned",	poisoned);
		archive.read("magic",		magic);
		archive.read("owner",		ownserial);
		archive.read("visible",		visible);
		archive.read("spawn",		spawnserial);
		archive.read("dir",			dir);
		archive.read("priv",		priv);
		archive.read("value",		value);
		archive.read("restock",		restock);
		archive.read("trigger",		trigger);
		archive.read("trigtype",	trigtype);
		archive.read("disabled",	disabled);
		archive.read("spawnregion",	spawnregion_);
		archive.read("uses",		tuses);
		archive.read("good",		good);
		archive.read("smelt",		smelt_);
		archive.read("glow",		glow);
		archive.read("glow_color",	glow_color);
		archive.read("glowtype",	glow_effect);
		archive.read("desc",		desc);
		archive.read("carve",		carve_);
	}
	else if ( archive.isWritting())
	{
		archive.write("id",			id());
		archive.write("name",		name_); // warning: items do not use cUObject name!
		archive.write("name2",		name2_);
		archive.write("creator",	creator);
		archive.write("sk_name",	madewith);
		archive.write("color",		color());
		archive.write("cont",		contserial);
		archive.write("layer",		layer_);
		archive.write("type",		type_);
		archive.write("type2",		type2_);
		archive.write("offspell",	offspell_);
		archive.write("more1",		more1);
		archive.write("more2",		more2);
		archive.write("more3",		more3);
		archive.write("more4",		more4);
		archive.write("moreb1",		moreb1_);
		archive.write("moreb2",		moreb2_);
		archive.write("moreb3",		moreb3_);
		archive.write("moreb4",		moreb4_);
		archive.write("morex",		morex);
		archive.write("morey",		morey);
		archive.write("morez",		morez);
		archive.write("amount",		amount_);
		archive.write("doordir",	doordir);
		archive.write("dye",		dye);
		archive.write("decaytime",	decaytime > 0 ? decaytime - uiCurrentTime : 0);
		archive.write("att",		att);
		archive.write("def",		def);
		archive.write("hidamage",	hidamage_);
		archive.write("lodamage",	lodamage_);
		archive.write("racehate",	racehate_);
		archive.write("st",			st);
		archive.write("time_unused",time_unused);
		archive.write("weight",		weight_);
		archive.write("hp",			hp_);
		archive.write("maxhp",		maxhp_);
		archive.write("rank",		rank);
		archive.write("st2",		st2);
		archive.write("dx",			dx);
		archive.write("dx2",		dx2);
		archive.write("in",			in);
		archive.write("in2",		in2);
		archive.write("speed",		speed_);
		archive.write("poisoned",	poisoned);
		archive.write("magic",		magic);
		archive.write("owner",		ownserial);
		archive.write("visible",	visible);
		archive.write("spawn",		spawnserial);
		archive.write("dir",		dir);
		archive.write("priv",		priv);
		archive.write("value",		value);
		archive.write("restock",	restock);
		archive.write("trigger",	trigger);
		archive.write("trigtype",	trigtype);
		archive.write("disabled",	disabled);
		archive.write("spawnregion",spawnregion_);
		archive.write("uses",		tuses);
		archive.write("good",		good);
		archive.write("smelt",		smelt_);
		archive.write("glow",		glow);
		archive.write("glow_color",	glow_color);
		archive.write("glowtype",	glow_effect);
		archive.write("desc",		desc);
		archive.write("carve",		carve_);
	}
	cUObject::Serialize(archive);
}

static int getname(P_ITEM pi, char* itemname)
{
	int j, len, mode, used, ok, namLen;
	if (pi == NULL)
		return 1;
	if (pi->name() != "#")
	{
		strcpy((char*)itemname, pi->name().ascii());
		return strlen((char*)itemname)+1;
	}

	tile_st tile = cTileCache::instance()->getTile( pi->id() );

	if (tile.flag2&0x80) strcpy((char*)itemname, "an ");
	else if (tile.flag2&0x40) strcpy((char*)itemname, "a ");
	else itemname[0]=0;
	namLen = strlen( (char*)itemname );
	mode=0;
	used=0;
	len=strlen((char *) tile.name);
	for (j=0;j<len;j++)
	{
		ok=0;
		if ((tile.name[j]=='%')&&(mode==0)) mode=2;
		else if ((tile.name[j]=='%')&&(mode!=0)) mode=0;
		else if ((tile.name[j]=='/')&&(mode==2)) mode=1;
		else if (mode==0) ok=1;
		else if ( ( mode == 1 ) && ( pi->amount() == 1 ) ) ok = 1;
		else if ( ( mode == 2 ) && ( pi->amount() > 1 ) ) ok = 1;
		if (ok)
		{
			itemname[namLen++] = tile.name[j];
			itemname[namLen] = '\0';
			if (mode) used=1;
		}
	}
	return strlen((char*)itemname)+1;
}

int cItem::getName(char* itemname)
{
	return getname(this, itemname);
}

QString cItem::getName(void)
{
	if( name_ != "#" )
		return name_;

	char itemname[256] = {0,};
	cItem::getName(itemname);
	return QString(itemname);
}

P_ITEM cAllItems::MemItemFree()// -- Find a free item slot, checking freeitemmem[] first
{
	return new cItem;
}

void cItem::SetSerial(long ser)
{
	this->serial=ser;
	if (ser != INVALID_SERIAL)
		cItemsManager::getInstance()->registerItem( this );
}

// -- Initialize an Item in the items array
void cItem::Init( bool mkser )
{
	if (mkser)		// give it a NEW serial #
	{
		this->SetSerial(cItemsManager::getInstance()->getUnusedSerial());
	}
	else
	{
		this->SetSerial(INVALID_SERIAL);
	}

	this->name_ = "#";
	this->name2_ = "#";
	this->incognito=false;//AntiChrist - incognito
	this->madewith=0; // Added by Magius(CHE)
	this->rank=0; // Magius(CHE)
	this->good=-1; // Magius(CHE)
	this->rndvaluerate=0; // Magius(CHE) (2)

	this->multis=INVALID_SERIAL;//Multi serial
	this->free = false;
	this->setId(0x0001); // Item visuals as stored in the client
	// this->name2[0]=0x00; Removed by Magius(CHE)
	this->pos = Coord_cl(100, 100, 0);
	this->color_ = 0x00; // Hue
	this->contserial = INVALID_SERIAL; // Container that this item is found in
	this->layer_ = 0; // Layer if equipped on paperdoll
	this->type_=0; // For things that do special things on doubleclicking
	this->type2_=0;
	this->offspell_ = 0;
	this->weight_ = 0;
	this->more1=0; // For various stuff
	this->more2=0;
	this->more3=0;
	this->more4=0;
	this->moreb1_=0;
	this->moreb2_=0;
	this->moreb3_=0;
	this->moreb4_=0;
	this->morex=0;
	this->morey=0;
	this->morez=0;
	this->amount_ = 1; // Amount of items in pile
	this->amount2_ = 0; //Used to track things like number of yards left in a roll of cloth
	this->doordir=0; // Reserved for doors
	this->dooropen=0;
	this->dye=0; // Reserved: Can item be dyed by dye kit
	this->carve_=(char*)0;// carving system
	this->att=0; // Item attack
	this->def=0; // Item defense
	this->lodamage_=0; //Minimum Damage weapon inflicts
	this->hidamage_=0; //Maximum damage weapon inflicts
	this->racehate_=-1; //race hating weapon -Fraz-
	this->smelt_ = 0; // for smelting items
	this->hp_=0; //Number of hit points an item has.
	this->maxhp_=0; // Max number of hit points an item can have.
	this->st=0; // The strength needed to equip the item
	this->st2=0; // The strength the item gives
	this->dx=0; // The dexterity needed to equip the item
	this->dx2=0; // The dexterity the item gives
	this->in=0; // The intelligence needed to equip the item
	this->in2=0; // The intelligence the item gives
	this->speed_=0; //The speed of the weapon
	this->magic=0; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable.
	this->gatetime=0;
	this->gatenumber=-1;
	this->decaytime=0;
	this->setOwnSerialOnly(-1);
	this->visible=0; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	this->spawnserial=-1;
	this->dir=0; // Direction, or light source type.
	this->priv=0; // Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
	this->value=0; // Price shopkeeper sells item at.
	this->restock=0; // Number up to which shopkeeper should restock this item
	this->trigger=0; //Trigger number that item activates
	this->trigtype=0; //Type of trigger
	this->disabled = 0; //Item is disabled, cant trigger.
	this->disabledmsg = ""; //Item disabled message. -- by Magius(CHE) �
	this->tuses = 0;    //Number of uses for trigger
	this->poisoned = 0; //AntiChrist -- for poisoning skill
 	this->murdertime = 0; //AntiChrist -- for corpse -- when the people has been killed
    this->glow = INVALID_SERIAL;
    this->glow_effect = 0;
    this->glow_color = 0;
	this->time_unused = 0;
	this->timeused_last=getNormalizedTime();
	this->spawnregion_ = "";
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
		pi->removeFromView( false );

		if (pi->glow != INVALID_SERIAL) 
		{  
			P_ITEM pj = FindItemBySerial( pi->glow );
			if (pj != NULL) Items->DeleItem(pj); // lb glow stuff, deletes the glower of an glowing stuff automatically
		}

		pi->SetSpawnSerial(-1);
		pi->SetOwnSerial(-1);

		// - remove from mapRegions if a world item
		if( pi->isInWorld() ) 
		{
			mapRegions->Remove(pi);
		}
		else
		{
			pi->setContSerial( INVALID_SERIAL );
		}

        // if a new book gets deleted also delete the corresponding bok file

		// Also delete all items inside if it's a container.
		vector<SERIAL> vecContainer = contsp.getData(pi->serial);
		unsigned int i;
		for (i = 0; i < vecContainer.size(); i++)
		{
			P_ITEM pContent = FindItemBySerial(vecContainer[i]);
			if (pContent != NULL)
				DeleItem(pContent);
		}

		// Queue for later delete.
		cItemsManager::getInstance()->deleteItem(pi);
	}
}

////////////////
//o---------------------------------------------------------------------------o
//| Function:   SpawnItem (2 interfaces)
//| Programmer: unknown, revamped by Duke,24.04.2000
//o---------------------------------------------------------------------------o
//| Purpose:    Will create an item based on item ID1 and ID2. If nPack is 1 it will
//|             put the item in nSocket's pack (auto-stacking). If nSend is 1 it will
//|             senditem() to all online characters. If you want to make specific changes
//|             to the item before sending the information pass nSend as 0.
//| Remarks:    I took the idea (to overload) from the 'official' source. So thanks to fur!
//o---------------------------------------------------------------------------o
        /*
        ** OK, here's the scoop, in a few key places we were calling add item when there was
        ** flat out no way to get a socket, say if a region spawner made an item, there is no
        ** associated socket.  This was causing the socket to be -1, and it was indexing into
        ** curchar[-1] and blowing up.  So I made a new version that takes a socket and a
        ** a character (the following function) if you want to call that one with socket = -1
        ** thats fine, because you have to pass the character in as well.  If you call this
        ** function (which was the original) you are not allowed to pass in -1 for the socket
        ** because it will crash. I've put the assert() in its place, so at least when it does
        ** crash you'll know why. If this assert() ever happens to you, it means you need to
        ** fix the CALLER of this function to pass the character in and call the other function
        ** instead. Taking the assert() out won't help, its just liable to crash on currchar[-1]
        ** anyways.  - fur
        */
        // what fur said about the assert only partially applies to this version. Duke
       
P_ITEM cAllItems::SpawnItem(UOXSOCKET nSocket,
					int nAmount, const char* cName, int nStackable,
					unsigned char cItemId1, unsigned char cItemId2,
					unsigned short cColorId,
					int nPack, int nSend)
{
	if (nSocket < 0)
	{
		clConsole.send("ERROR: SpawnItem called with bad socket\n");
		return NULL;
	}
	else
	{
		return SpawnItem(nSocket, currchar[nSocket], nAmount, cName, nStackable, cItemId1, cItemId2, cColorId, nPack, nSend);
	}
}

P_ITEM cAllItems::SpawnItem(UOXSOCKET nSocket, P_CHAR ch,
					int nAmount, const char* cName, int nStackable,
					unsigned char cItemId1, unsigned char cItemId2,
					unsigned short cColorId,
					int nPack, int nSend)
{
	return SpawnItem(ch, nAmount, cName, nStackable,(short)((cItemId1<<8)+cItemId2), cColorId, nPack);
}

P_ITEM cAllItems::SpawnItemBank(P_CHAR pc_ch, QString nItem)
{
	if (pc_ch == NULL) 
		return NULL;
	
	P_ITEM bankbox = pc_ch->getBankBox();

	UOXSOCKET s = calcSocketFromChar(pc_ch);          // Don't check if s == -1, it's ok if it is.
	P_ITEM pi = createScriptItem(s, nItem, 1);
	if (pi == NULL)
		return NULL;
	GetScriptItemSetting(pi); 
	bankbox->AddItem(pi);
	return pi;
}

P_ITEM cAllItems::SpawnItem(P_CHAR pc_ch, int nAmount, const char* cName, bool pileable, short id, short color, bool bPack)
{
	if (pc_ch == NULL) 
		return NULL;

	P_ITEM pPack=Packitem(pc_ch);
	bool pile = false;
	
	if (pileable)
	{
		// make sure it's REALLY pileable ! (Duke)
		tile_st tile = cTileCache::instance()->getTile( id );
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
		
		unsigned int ci;
		vector<SERIAL> vecContainer = contsp.getData(pPack->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			P_ITEM pSt = FindItemBySerial(vecContainer[ci]);
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
	P_ITEM pi = Items->MemItemFree();
	if (pi == NULL) return NULL;

	pi->Init();
	if(cName!=NULL)
		pi->setName( cName );
	pi->setId(id);
	pi->setColor( color );
	pi->setAmount( nAmount );
	pi->att=5;
	pi->priv |= 0x01;
	if (IsCutCloth(pi->id())) pi->dye=1;// -Fraz- fix for cut cloth not dying
	if (bPack)
	{
		if (pPack)
		{
			pi->setContSerial(pPack->serial);
			pi->pos.x=(50+rand()%80);
			pi->pos.y=(50+rand()%80);
			pi->pos.z=9;
		}
		else
		{// LB place it at players feet if he hasnt got backpack
			pi->MoveTo(pc_ch->pos.x, pc_ch->pos.y, pc_ch->pos.z);
		}
	}
	
	//clConsole.send("Adding Harditems settings in items.cpp:spawnitem\n");
	GetScriptItemSetting(pi); // Added by Magius(CHE) (2)
	pi->update();
	return pi;
}

void cAllItems::GetScriptItemSetting(P_ITEM pi)
{// rewritten by sereg
	if(pi == NULL) 
		return;

	char buff[512] = {0};

	sprintf(buff,"x%x",pi->id());

	QDomElement* itemSect = DefManager->getSection( WPDT_ITEM, buff );
	if( !itemSect->isNull() )
		pi->applyDefinition( *itemSect );
}

P_ITEM cAllItems::SpawnItemBackpack2(UOXSOCKET s, QString nItem, int nDigging) // Added by Genesis 11-5-98
{
	P_CHAR pc_currchar = currchar[s];
	P_ITEM backpack = Packitem(pc_currchar);
	
	P_ITEM pi = createScriptItem(s, nItem, 1);
	if (pi == NULL || backpack == NULL)
		return NULL;

	if(nDigging) 
	{
		if( pi->value != 0 ) 
			pi->value = 1 + ( rand() % ( pi->value ) ); 
		
		if(pi->hp()!=0) 
			pi->setHp( 1 + ( rand() % pi->hp() ) );
		
		if(pi->maxhp() != 0)
		{
			pi->setMaxhp( 1 + ( rand() % pi->maxhp() ) ); 
			pi->setHp( 1 + ( rand() % pi->maxhp() ) );
		}
	}

	GetScriptItemSetting(pi);

	backpack->AddItem(pi);
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
	int serial, preservebody;
	if ( pi == NULL )
		return;
	P_ITEM pi_multi = NULL;

	if(pi->isLockedDown()) {pi->decaytime=0; return;}
	if( pi->decaytime <= currenttime || (overflow) )//fixed by JustMichael
	{
		if (pi->priv&0x01 && pi->isInWorld() && !pi->free)
		{  // decaytime = 5 minutes, * 60 secs per min, * MY_CLOCKS_PER_SEC
			if (pi->decaytime==0) 
			{
				pi->startDecay();
			}
			
			if (pi->decaytime<=currenttime)
			{
                //Multis --Boats ->

				if (!Items->isFieldSpellItem(pi)) // Gives fieldspells a chance to decay in multis, LB
				{
				  if (pi->multis<1 && !pi->corpse())
				  {
					// JustMichael -- Added a check to see if item is in a house
					pi_multi = findmulti(pi->pos);
					if (pi_multi != NULL)
					{
						if( pi_multi->more4==0) //JustMichael -- set more to 1 and stuff can decay in the building
						{
							pi->startDecay();
							pi->SetMultiSerial(pi_multi->serial);
							return;
						}
					}
				} 
				  else if (pi->multis>0 && !pi->corpse()) 
				{					
					  pi->startDecay();
					  return;
				}
				}
				//End Boats/Mutlis

				//JustMichael--Keep player's corpse as long as it has more than 1 item on it
				//up to playercorpsedecaymultiplier times the decay rate
				if (pi->corpse() && pi->GetOwnSerial()!=-1)
				{
					preservebody=0;
					serial=pi->serial;
					unsigned int ci;
					vector<SERIAL> vecContainer = contsp.getData(serial);
					for( ci=0; ci < vecContainer.size(); ci++ )
					{
						P_ITEM pj = FindItemBySerial(vecContainer[ci]);
						if( pj != NULL )
						{
							preservebody++;
						}
						if(preservebody) break; //lagfix - AntiChrist - not necessary to check ALL the items!!!
					}

					if( preservebody > 1 && pi->more4 )
					{
						pi->more4--;
						pi->startDecay();
						return;
					}
				}
				if( (pi->type() == 1 && !pi->corpse() ) || (pi->GetOwnSerial() != -1 && pi->corpse() ) || (!SrvParams->lootdecayswithcorpse() && pi->corpse() ))
				{
					serial=pi->serial;
					vector<SERIAL> vecContainer = contsp.getData(serial);
					unsigned int ci;
					for (ci=0;ci<vecContainer.size();ci++)
					{
						P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
                        if (pi_j != NULL) //lb
						{
						   if (pi_j->contserial==pi->serial)// && (items[j].layer!=0x0B)&&(items[j].layer!=0x10))
						   {
							pi_j->setContSerial(-1);
							pi_j->MoveTo(pi->pos.x,pi->pos.y,pi->pos.z);

							pi_j->startDecay();
							pi_j->update();//AntiChrist
						   }
						} // enof of if j!=-1
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
	if( !pItem || pItem->free )
		return;

	// Not ready to respawn yet
	if( pItem->gatetime > currenttime )
		return;

	switch( pItem->type() )
	{
	// Item Spawner
	case 61:
		{
			// Check if it's worth respawning
			vector< SERIAL > spawned = spawnsp.getData( pItem->serial );
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
				else if( ( pSpawned->free ) || ( pSpawned->pos != pItem->pos ) )
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
					clConsole.send( QString( "Unable to spawn unscripted item: %1" ).arg( pItem->carve() ) );
					break;
				}

				pSpawned->moveTo( pItem->pos );
				pSpawned->SetSpawnSerial( pItem->serial );
				pSpawned->update();
			}
		}
		break;
	// NPC Spawner
	case 62:
		{
			// Check if it's worth respawning
			vector< SERIAL > spawned = spawnsp.getData( pItem->serial );
			UINT32 amount = spawned.size();

			// Do a sanity check
			for( UINT32 i = 0; i < spawned.size(); ++i )
			{
				P_CHAR pSpawned = FindCharBySerial( spawned[i] );

				// Char has been deleted
				if( !pSpawned )
				{
					spawnsp.remove( pItem->serial, pSpawned->serial );
					--amount;
				}
				// Char has been tamed/changed owner
				else if( pSpawned->tamed() || pSpawned->spawnSerial() != pItem->serial )
				{
					spawnsp.remove( pItem->serial, pSpawned->serial );
					--amount;
				}
			}

			// Is there anything to be spawned
			if( amount < pItem->amount() )
			{
				P_CHAR pSpawned = Npcs->createScriptNpc( pItem->carve(), pItem->pos );
				if( pSpawned )
					pSpawned->SetSpawnSerial( pItem->serial );
			}
		}
		break;
	};

	pItem->gatetime = currenttime + ( RandomNum( pItem->morex, pItem->morey ) * MY_CLOCKS_PER_SEC );

	/*

	// Chest spawner
			else if ((pi->type()==63)||(pi->type()==64)||(pi->type()==65)||(pi->type()==66)||(pi->type()==8))
			{
				serial=pi->serial;
				unsigned int j;
				vector<SERIAL> vecContainer = contsp.getData(pi->serial);
				for (j=0;j<vecContainer.size();j++)
				{
					P_ITEM pi_ci = FindItemBySerial(vecContainer[j]);
					if (pi_ci != NULL)
					if (pi_ci->contserial == pi->serial && !pi_ci->free)
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

	P_ITEM pi = createScriptItem(-1, itemSect, 1); // lb, bugfix
	if (pi == NULL) return;
	
	if( !spawnInItem )
	{
		pi->MoveTo(pItem->pos.x, pItem->pos.y, pItem->pos.z); //add spawned item to map cell if not in a container
	}
	else
	{
		pi->setContSerial(pItem->serial); //set item in pointer array
	}
	pi->SetSpawnSerial(pItem->serial);


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

void cAllItems::CheckEquipment(P_CHAR pc_p) // check equipment of character p
{
	if (pc_p == NULL)
		return;

	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc_p->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if(pi->st>pc_p->st() )//if strength required > character's strength
		{
			if(pi->name() == "#")
				pi->getName(temp2);
			else
				strcpy((char*)temp2, pi->name().ascii() );
			
			sprintf((char*)temp, "You are not strong enough to keep %s equipped!", temp2);
			sysmessage(calcSocketFromChar(pc_p), (char*)temp);
			itemsfx(calcSocketFromChar(pc_p), pi->id());
			
			//Subtract stats bonus and poison
			pc_p->removeItemBonus(pi);
						
			pi->setContSerial(-1);
			pi->MoveTo(pc_p->pos.x,pc_p->pos.y,pc_p->pos.z);
			pi->update();
			
			for (int j=0;j<now;j++)
				if (inrange1p(pc_p, currchar[j])&&perm[j])
				{
					wornitems(j, pc_p);
					senditem(j, pi);
				}
		}
	}		
}

P_ITEM cAllItems::createScriptItem( UOXSOCKET s, QString Section, UI32 nSpawned )
{
	P_ITEM nItem = this->createScriptItem( Section );

	if( nItem == NULL )
		return NULL;

	if( s != -1 && !nSpawned )
	{
		if (triggerx)
		{
			nItem->MoveTo(triggerx,triggery,triggerz);
		}
		else
		{
			short xx,yy;
			signed char zz;
			xx=(buffer[s][11]<<8)+buffer[s][12];
			yy=(buffer[s][13]<<8)+buffer[s][14];
			zz=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
			nItem->MoveTo(xx,yy,zz);
		}
	}
	else
	{
		if( nItem->isInWorld() )
			mapRegions->Add( nItem );
		nItem->update();
	}

	return nItem;
}

// Retrieves the Item Information stored in Section
// And creates an item based on it
P_ITEM cAllItems::createScriptItem( QString Section )
{
	if( Section.length() == 0 )
		return NULL;

	P_ITEM nItem = NULL;

	// Get an Item and assign a serial to it
	QDomElement* DefSection = DefManager->getSection( WPDT_ITEM, Section );
	
	if( DefSection->isNull() ) // section not found 
	{
		clConsole.log( QString("Unable to create unscripted item: %1\n").arg(Section).latin1() );
		return NULL;
	}

	//books:
	if( DefSection->attributes().contains( "type" ) )
	{
		if( DefSection->attribute( "type" ) == "book" )
		{
			cBook* nBook = new cBook();
			nBook->Init();
			cItemsManager::getInstance()->registerItem( nBook );

			nBook->applyDefinition( *DefSection );
			nBook->setSection( Section );

			nItem = nBook;
		}
	}
	else
	{
		nItem = MemItemFree();
		nItem->Init( true );
		cItemsManager::getInstance()->registerItem( nItem );
		
		nItem->applyDefinition( *DefSection );
	}

	return nItem;
}

// Creates an Item from an item-list
// And applies additional sections to it (<amount><random min="" max="" /></amount>
P_ITEM cAllItems::createListItem( QString Section )
{
	return NULL;
}

// Added by DarkStorm
bool cItem::onShowItemName( P_CHAR Viewer )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowItemName( this, Viewer ) )
			return true;

	return false;

}

bool cItem::onTalkToItem( P_CHAR Talker, const QString &Text )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onTalkToItem( Talker, this, Text ) )
			return true;

	return false;
}

void cItem::processNode( const QDomElement& Tag )
{
	// we do this as we're going to modify the element
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );
	QStringList defineSections = DefManager->getSections( WPDT_DEFINE );

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
		this->restock = Value.toULong(); // Maximumm sell-amount from the beginning
	}

	// <color>480</color>
	else if( TagName == "color" )
		this->setColor( Value.toUShort() );

	// <events>a,b,c</events>
	else if( TagName == "events" )
	{
		eventList_ = QStringList::split( ",", Value );
		recreateEvents();
	}

	// <attack min="1" max="2"/>
	else if( TagName == "attack" )
	{
		if( Tag.attributes().contains( "min" ) )
			this->setLodamage( Tag.attribute( "min" ).toInt() );

		if( Tag.attributes().contains( "max" ) )
			this->setHidamage( Tag.attribute( "max" ).toInt() );

		// Better...
		if( this->lodamage() > this->hidamage() )
			this->setHidamage( this->lodamage() );
	}

	// <defense>10</defense>
	else if( TagName == "defense" )
		this->def = Value.toInt();

	// <type>10</type>
	else if( TagName == "type" )
		this->setType( Value.toUInt() );

	// <weight>10</weight>
	else if( TagName == "weight" )
		this->setWeight( (UINT32)( Value.toFloat() * 10 ) );

	// <value>10</value>
	else if( TagName == "value" )
		this->value = Value.toInt();

	// <carve></carve> For corpses and item spawners
	else if( TagName == "carve" )
		this->carve_ = Value;
		
	// <restock>10</restock>
	else if( TagName == "restock" )
		this->restock = Value.toInt();

	// <layer>10</layer>
	else if( TagName == "layer" )
		this->setLayer( Value.toShort() );

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
		this->good = Value.toInt();

	// <lightsource>10</lightsource>
	else if( TagName == "lightsource" )
		this->dir = Value.toUShort();

	// <more1>10</more1>
	else if( TagName == "more1" )
		this->more1 = Value.toInt();

	// <more>10</more> <<<<< alias for more1
	else if( TagName == "more" )
		this->more1 = Value.toInt();

	// <more2>10</more2>
	else if( TagName == "more2" )
		this->more2 = Value.toInt();

	// <morex>10</morex>
	else if( TagName == "morex" )
		this->morex = Value.toInt();

	// <morex>10</morex>
	else if( TagName == "morey" )
		this->morex = Value.toInt();

	// <morez>10</morez>
	else if( TagName == "morez" )
		this->morez = Value.toInt();

	// <morexyz>10</morexyz>
	else if( TagName == "morexyz" )
	{
		QStringList Elements = QStringList::split( ",", Value );
		if( Elements.count() == 3 )
		{
			this->morex = Elements[ 0 ].toInt();
			this->morey = Elements[ 1 ].toInt();
			this->morez = Elements[ 2 ].toInt();
		}
	}

	// <movable />
	// <ownermovable />
	// <immovable />
	else if( TagName == "movable" )
		this->magic = 1;
	else if( TagName == "immovable" )
		this->magic = 2;
	else if( TagName == "ownermovable" )
		this->magic = 3;

	// <decay />
	// <nodecay />
	else if( TagName == "decay" )
		this->priv |= 0x01;
	else if( TagName == "nodecay" )
		this->priv &= 0xFE;

	// <dispellable />
	// <notdispellable />
	else if( TagName == "dispellable" )
		this->priv |= 0x04;
	else if( TagName == "notdispellable" )
		this->priv &= 0xFB;

	// <newbie />
	// <notnewbie />
	else if( TagName == "newbie" )
		this->priv |= 0x04;
	else if( TagName == "notnewbie" )
		this->priv &= 0xFB;

	// <twohanded />
	else if( TagName == "twohanded" )
		this->setTwohanded( true );

	// <singlehanded />
	else if( TagName == "singlehanded" )
		this->setTwohanded( false );

	// <racehate>2</racehate>
	else if( TagName == "racehate" )
		this->setRacehate( Value.toInt() );

	// <trigger>2</trigger>
	else if( TagName == "trigger" )
		this->trigger = Value.toInt();

	// <triggertype>2</triggertype>
	else if( TagName == "triggertype" )
		this->trigtype = Value.toInt();

	// <smelt>2</smelt>
	else if( TagName == "smelt" )
		this->setSmelt( Value.toInt() );

	// <requires type="xx">2</requires>
	else if( TagName == "requires" )
	{
		if( !Tag.attributes().contains( "type" ) )
			return;

		QString Type = Tag.attribute( "type" );
			
		if( Type == "str" )
			this->st = Value.toULong();
		else if( Type == "dex" )
			this->dx = Value.toULong();
		else if( Type == "int" )
			this->in = Value.toULong();
	}

	// <visible />
	// <invisible />
	// <ownervisible />
	else if( TagName == "invisible" )
		this->visible = 2;
	else if( TagName == "visible" )
		this->visible = 0;
	else if( TagName == "ownervisible" )
		this->visible = 1;

	// <modifier type="xx">2</modifier>
	else if( TagName == "modifier" )
	{
		if( !Tag.attributes().contains( "type" ) )
			return;

		QString Type = Tag.attribute( "type" );
			
		if( Type == "str" )
			this->st2 = Value.toULong();
		else if( Type == "dex" )
			this->dx2 = Value.toULong();
		else if( Type == "int" )
			this->in2 = Value.toULong();
	}

	// <dye />
	// <nodye />
	else if( TagName == "dye" )
		this->dye = 1;
	else if( TagName == "nodye" )
		this->dye = 0;

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
		setWeight( cTileCache::instance()->getTile( id_ ).weight );
	}

	// <content><item id="a" />...<item id="z" /></contains> (sereg)
	else if( TagName == "content" && Tag.hasChildNodes() )
		this->processContainerNode( Tag ); 

	else if( TagName == "inherit" && Tag.attributes().contains( "id" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_ITEM, Tag.attribute( "id" ) );
		if( !DefSection->isNull() )
			applyDefinition( *DefSection );
	}

	else if( TagName == "inherit" )
	{
		QString nodeValue = getNodeValue( Tag );
		QDomElement* DefSection = DefManager->getSection( WPDT_ITEM, nodeValue );
		if( !DefSection->isNull() )
			applyDefinition( *DefSection );
	}

	else if( defineSections.contains( TagName ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_DEFINE, TagName );
		if( !DefSection->isNull() )
		{
			QDomNode chNode = Tag.firstChild();
			while( !chNode.isNull() )
			{
				if( chNode.isElement() )
					processModifierNode( chNode.toElement() );
				chNode = chNode.nextSibling();
			}
		}
	}

	else
		cUObject::processNode( Tag );
}

void cItem::processModifierNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	// <name>magic %1</name>
	if( TagName == "name" )
	{
		name_ = Value.arg( name_ );
	}

	// <identified>%1 of Hardening</identified>
	else if( TagName == "identified" )
		name2_ = Value.arg( name2_ );

	// <color>-10</color>
	else if( TagName == "color" )
	{
		if( Value.contains(".") || Value.contains(",") )
			color_ = (UINT16)ceil((float)color_ * Value.toFloat());
		else
			color_ += Value.toShort();
	}

	// <attack min="-1" max="+2"/>
	else if( TagName == "attack" )
	{
		if( Tag.attributes().contains( "min" ) )
		{
			Value = Tag.attribute("min");
			if( Value.contains(".") || Value.contains(",") )
				setLodamage( (UINT16)ceil((float)lodamage() * Value.toFloat()) );
			else
				setLodamage( lodamage() + Value.toInt() );
		}

		if( Tag.attributes().contains( "max" ) )
		{
			Value = Tag.attribute("max");
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
			def = (UINT32)ceil((float)def * Value.toFloat());
		else
			def += Value.toUInt();
	}

	// <weight>-10</weight>
	else if( TagName == "weight" )
	{
		if( Value.contains(".") || Value.contains(",") )
			setWeight( (INT32)ceil((float)weight() * Value.toFloat()) );
		else
			setWeight( weight() + Value.toInt() );
	}

	// <value>+20</value>
	else if( TagName == "value" )
	{
		if( Value.contains(".") || Value.contains(",") )
			value = (INT32)ceil((float)value * Value.toFloat());
		else
			value += Value.toInt();
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
		if( !Tag.attributes().contains( "type" ) )
			return;

		QString Type = Tag.attribute( "type" );
			
		if( Type == "str" )
		{
			if( Value.contains(".") || Value.contains(",") )
				st = (INT32)ceil((float)st * Value.toFloat());
			else
				this->st += Value.toLong();
		}
		else if( Type == "dex" )
		{
			if( Value.contains(".") || Value.contains(",") )
				dx = (INT32)ceil((float)dx * Value.toFloat());
			else
				this->dx += Value.toLong();
		}
		else if( Type == "int" )
		{
			if( Value.contains(".") || Value.contains(",") )
				in = (INT32)ceil((float)in * Value.toFloat());
			else
				this->in += Value.toLong();
		}
	}

	// <modifier type="xx">2</modifier>
	else if( TagName == "modifier" )
	{
		if( !Tag.attributes().contains( "type" ) )
			return;

		QString Type = Tag.attribute( "type" );
			
		if( Type == "str" )
		{
			if( Value.contains(".") || Value.contains(",") )
				st2 = (INT32)ceil((float)st2 * Value.toFloat());
			else
				this->st2 += Value.toLong();
		}
		else if( Type == "dex" )
		{
			if( Value.contains(".") || Value.contains(",") )
				dx2 = (INT32)ceil((float)dx2 * Value.toFloat());
			else
				this->dx2 += Value.toLong();
		}
		else if( Type == "int" )
		{
			if( Value.contains(".") || Value.contains(",") )
				in2 = (INT32)ceil((float)in2 * Value.toFloat());
			else
				this->in2 += Value.toLong();
		}
	}

	// <id>+3</id>
	else if( TagName == "id" )
	{
		this->setId( id() + Value.toShort() );
	}
}

void cItem::processContainerNode( const QDomElement &Tag )
{
	//item containers can be scripted like this:
	/*
	<contains>
		<item><inherit list="myList" /></item>
		<item><inherit id="myItem1" /><amount><random ... /></amount><color><colorlist><random...></colorlist></color></item>
		...
	</contains>
	*/
	QDomNode childNode = Tag.firstChild();
	vector< QDomElement > equipment;
		
	while( !childNode.isNull() )
	{		
		if( childNode.nodeName() == "item" )
			equipment.push_back( childNode.toElement() );
		else if( childNode.nodeName() == "getlist" && childNode.attributes().contains( "id" ) )
		{
			QStringList list = DefManager->getList( childNode.toElement().attribute( "id" ) );
			for( QStringList::iterator it = list.begin(); it != list.end(); it++ )
				if( DefManager->getSection( WPDT_ITEM, *it ) )
					equipment.push_back( *DefManager->getSection( WPDT_ITEM, *it ) );
		}

		childNode = childNode.nextSibling();
	}
		
	for( SI32 i = 0; i < equipment.size(); i++ )
	{
			P_ITEM nItem = Items->MemItemFree();

			if( nItem == NULL )
				continue;
	
			nItem->Init( true );
			cItemsManager::getInstance()->registerItem( nItem );

			nItem->applyDefinition( equipment[ i ] );

			nItem->setContSerial( this->serial );
	}
	childNode = childNode.nextSibling();
}

void cItem::showName( cUOSocket *socket )
{
	// End chars/npcs section
	
    if( onShowItemName( socket->player() ) )
        return;        
	
	name_ = getName();
	UOXSOCKET s = calcSocketFromChar( socket->player() ); // for Legacy code

	if (type() == 9)
	{
		int spellcount=Magic->SpellsInBook(this);
		sprintf((char*)temp, "%i spells", spellcount);
		itemmessage(s, (char*)temp, serial,0x0481);
	}

	if (type() == 1000) // Ripper...used for bank checks.
	{
		sprintf((char*)temp, "value : %i", value);
		itemmessage(s, (char*)temp, serial,0x0481);
	}

	if (type() == 187) // Ripper...used for slotmachine.
	{
		sprintf((char*)temp, "[%i gold Slot]", SrvParams->slotAmount());
		itemmessage(s, (char*)temp, serial,0x0481);
	}
	
	if (socket->player()->getPriv()&8)
	{
		if (amount() > 1)
			sprintf((char*)temp, "%s [%x]: %i", name_.latin1(), serial, amount());
		else
			sprintf((char*)temp, "%s [%x]", name_.latin1(), serial);
		itemmessage(s, (char*)temp, serial);
		return;
	}
	
	// Click in a Player Vendor item, show description, price and return
	if (!isInWorld() && isItemSerial(contserial))
	{
		P_CHAR pc_j = GetPackOwner(FindItemBySerial(contserial));
		if (pc_j != NULL)
		{
			if (pc_j->npcaitype() == 17)
			{
				if (creator.size() > 0 && madewith>0)
					sprintf((char*)temp2, "%s %s by %s", desc.c_str(), skill[madewith - 1].madeword.latin1(), creator.c_str()); 
				else
					strcpy((char*)temp2, desc.c_str()); // LB bugfix
				
				sprintf((char*)temp, "%s at %igp", temp2, value);
				itemmessage(s, (char*)temp, serial);
				return;
			}
		}
	}
	
	// From now on, we will build the message into temp, and let itemname with just the name info
	// Add amount info.
	if (!isPileable() || amount() == 1)
		strncpy((char*)temp, name_.latin1(), 100);
	else 
		if (name_.right(1) != "s") // avoid iron ingotss : x
			sprintf((char*)temp, "%ss : %i", name_.latin1(), amount());
		else
			sprintf((char*)temp, "%s : %i", name_.latin1(), amount());
		
	// Add creator's mark (if any)
	if( creator.size() > 0 && madewith > 0 )
		sprintf((char*)temp, "%s %s by %s", temp, skill[madewith - 1].madeword.latin1(), creator.c_str());
	
	if( type() == 15 )
	{
		if (name2() == name())
		{
			sprintf((char*)temp, "%s %i charge", temp, morez);
			if (morez != 1)
			strcat(temp, "s");
		}
	}
	else if( type() == 404 || type() == 181 )
	{
		if (name2() == name())
		{
			sprintf( (char*)temp, "%s %i charge", temp, morex);
			if( morex != 1 )
				strcat(temp, "s");
		}
	}

	// Corpse highlighting...Ripper
	if( corpse() )
	{
		if( more2 == 1 )
			socket->showSpeech( this, tr( "[Innocent]" ), 0x005A );
		else if( more2 == 2 )
			socket->showSpeech( this, tr( "[Criminal]" ), 0x03B2 );
		else if( more2 == 3 )
			socket->showSpeech( this, tr( "[Murderer]" ), 0x0026 );
	}

	// Let's handle secure/locked down stuff.
	if (isLockedDown() && type() != 12 && type() != 13 && type() != 203)
	{
		if ( !secured() )
			itemmessage(s, "[locked down]", serial, 0x0481);
		if ( secured() && isLockedDown())
			itemmessage(s, "[locked down & secure]", serial, 0x0481);				
	}
	
	itemmessage(s, (char*)temp, serial);
	
	// Send the item/weight as the last line in case of containers
	if( type() == 1 || type() == 63 || type() == 65 || type() == 87 )
	{
		vector< SERIAL > items = contsp.getData( serial );
		UINT16 tWeight = totalweight_;
		
		if( weight_ == 255 )
			tWeight -= 255;

		QString message = tr( "[%1 items, %2 stones]" ).arg( items.size() ).arg( tWeight );

		socket->showSpeech( this, message, 0x3B2 );

		/*int amt = 0;
		int wgt = (int) Weight->LockeddownWeight(this, &amt, 0); // get stones and item #
		if( amt > 0 )
		{
			sprintf((char*)temp2, "[%i items, %i stones]", amt, wgt);
			itemmessage(s, (char*)temp2, serial);
		}
		else
			itemmessage(s, "[0 items, 0 stones]", serial);*/
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
		cUOTxSendItem sendItem;
		sendItem.setSerial( serial );
		sendItem.setId( id() );
		sendItem.setAmount( amount() );
		sendItem.setColor( color() );
		sendItem.setCoord( pos );
		sendItem.setDirection( dir );

		if( mSock )
		{
			P_CHAR pChar = mSock->player();

			// Only send to sockets in range
			if( !pChar || ( pChar->pos.distance( pos ) > pChar->VisRange ) )
				return;

			// Completely invisible
			if( ( visible == 2 ) && !pChar->isGM() )
				return;

			// Visible to owners and GMs only
			else if( ( visible == 1 ) && !pChar->Owns( this ) && !pChar->isGM() )
				return;
	        
			if( isAllMovable() )
				sendItem.setFlags( 0x20 );
			else if( pChar->canMoveAll() )
				sendItem.setFlags( 0x20 );
			else if( ( isOwnerMovable() || isLockedDown() ) && pChar->Owns( this ) )
				sendItem.setFlags( 0x20 );

			if( ( visible > 0 ) && !pChar->Owns( this ) )
				sendItem.setFlags( sendItem.flags() | 0x80 );

			// TODO: Insert code for view-multi-as-icon & view-lightsource-as-candle

			mSock->send( &sendItem );
		}
		else
		{
			for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
			{
				P_CHAR pChar = mSock->player();
	
				// Only send to sockets in range
				if( !pChar || ( pChar->pos.distance( pos ) > pChar->VisRange ) )
					continue;
	
				// Completely invisible
				if( ( visible == 2 ) && !pChar->isGM() )
					continue;
	
				// Visible to owners and GMs only
				else if( ( visible == 1 ) && !pChar->Owns( this ) && !pChar->isGM() )
					continue;
	            
				if( isAllMovable() )
					sendItem.setFlags( 0x20 );
				else if( pChar->canMoveAll() )
					sendItem.setFlags( 0x20 );
				else if( ( isOwnerMovable() || isLockedDown() ) && pChar->Owns( this ) )
					sendItem.setFlags( 0x20 );
	
				if( ( visible > 0 ) && !pChar->Owns( this ) )
					sendItem.setFlags( sendItem.flags() | 0x80 );
	
				// TODO: Insert code for view-multi-as-icon & view-lightsource-as-candle
	
				mSock->send( &sendItem );
			}
		}
	}
	// equipped items
	else if( isCharSerial( contserial ) )
	{
		cUOTxCharEquipment equipItem;
		equipItem.fromItem( this );
		P_CHAR pOwner = FindCharBySerial( contserial );

		if( !pOwner )
			return;

		for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		{
			P_CHAR pChar = socket->player();

			// Only send to sockets in range
			if( !pChar || !pChar->inRange( pOwner, pChar->VisRange ) )
				continue;

			socket->send( &equipItem );
		}
	}
	// items in containers
	else if( isItemSerial( contserial ) )
	{
		cUOTxAddContainerItem contItem;
		contItem.fromItem( this );

		P_ITEM iCont = GetOutmostCont( this, 0xFF );
		cUObject *oCont = iCont;

		if( isCharSerial( iCont->contserial ) )
			oCont = FindCharBySerial( iCont->contserial );

		if( !oCont )
			return;

		for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		{
			P_CHAR pChar = socket->player();

			if( !pChar || ( pChar->pos.distance( oCont->pos ) > pChar->VisRange ) )
				continue;

			socket->send( &contItem );
		}
	}
}

P_ITEM cItem::dupe()
{
	if( corpse() )
		return NULL;

	P_ITEM nItem = new cItem( (*this) );
	nItem->SetSerial( cItemsManager::getInstance()->getUnusedSerial() );
	cItemsManager::getInstance()->registerItem( nItem );
	
	// We wont dupe items on chars without proper handling
	P_CHAR pWearer = FindCharBySerial( nItem->contserial );
	if( pWearer )
	{
		nItem->setLayer( 0 );
		nItem->setContSerial( INVALID_SERIAL );
		nItem->moveTo( pWearer->pos );
	}

	return nItem;
}

void cItem::soundEffect( UINT16 sound )
{
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange ) )
			mSock->soundEffect( sound, this );
}

// Our weight has changed
// Update the top-containers
void cItem::setWeight( SI16 nValue )
{
	setTotalweight( totalweight_ - weight_ );
	weight_ = nValue;
	setTotalweight( totalweight_ + weight_ );
}

// This subtracts the weight of the top-container
// And then readds the new weight
void cItem::setTotalweight( INT32 data )
{
	//if( data < 0 )
		// FixWeight!

	if( isCharSerial( contserial ) )
	{
		P_CHAR pChar = FindCharBySerial( contserial );
		if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
			pChar->setWeight( pChar->weight() - totalweight_ );
	}
	else if( isItemSerial( contserial ) )
	{
		P_ITEM pItem = FindItemBySerial( contserial );
		if( pItem )
			pItem->setTotalweight( pItem->totalweight() - totalweight_ );
	}

	totalweight_ = data;

	if( isCharSerial( contserial ) )
	{
		P_CHAR pChar = FindCharBySerial( contserial );
		if( pChar && ( ( layer_ < 0x1A ) || ( layer_ == 0x1E ) ) )
			pChar->setWeight( pChar->weight() + totalweight_ );
	}
	else if( isItemSerial( contserial ) )
	{
		P_ITEM pItem = FindItemBySerial( contserial );
		if( pItem )
			pItem->setTotalweight( pItem->totalweight() + totalweight_ );
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
	UINT16 mindef = (UINT16)floor( minmod * (double)def );
	UINT16 maxdef = (UINT16)floor( maxmod * (double)def );
	
	setLodamage( RandomNum( minlodam, maxlodam ) );
	setHidamage( RandomNum( minhidam, maxhidam ) );
	def = RandomNum( mindef, maxdef );
	setMaxhp( RandomNum( minhp_, maxhp_ ) );
	setHp( maxhp() );
}


