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

//
// -- Items.cpp  Item specific routines (add, delete change) in preperation for
//               going to a pointer based system
//

#include "wolfpack.h"
#include "SndPkg.h"
#include "utilsys.h"
#undef  DBGFILE
#define DBGFILE "items.cpp"
#include "debug.h"

#define ITEM_RESERVE 300	// minimum of free slots that should be left in the array.
							// otherwise, more memory will be allocated in the mainloop (Duke)

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
	long rest=0;
	if( amount > amt )
	{
		amount-=amt;
		RefreshItem(this);
	}
	else
	{
		Items->DeleItem(this);
		rest=amt-amount;
	}
	return rest;
}

// This method does not change the pointer arrays !!
// should only be used in VERY specific situations like initItem... Duke, 6.4.2001
void cItem::setContSerialOnly(long contser)
{
	contserial=contser;
}

void cItem::SetContSerial(long contser)
{
	if (contserial != INVALID_SERIAL)
		contsp.remove(contserial, this->serial);

	setContSerialOnly(contser);

	if (contser!=-1)
		contsp.insert(contserial, this->serial);
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
	if (multis!=-1)	// if it was set, remove the old one
		imultisp.remove(multis, serial);

	multis=mulser;

	if (mulser!=-1)		// if there is multi, add it
		imultisp.insert(mulser, serial);
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
	pItem->SetContSerial(this->serial);
	if (xx!=-1)	// use the given position
	{
		pItem->pos.x=xx;
		pItem->pos.y=yy;
		pItem->pos.z=9;
	}
	else		// no pos given
	{
		if (!this->ContainerPileItem(pItem))	// try to pile
			pItem->SetRandPosInCont(this);		// not piled, random pos
	}
	SndRemoveitem(pItem->serial);
	RefreshItem(pItem);
	return true;
}

bool cItem::PileItem(cItem* pItem)	// pile two items
{
	if (!(this->pileable && pItem->pileable &&
		this->serial!=pItem->serial &&
		this->id()==pItem->id() &&
		this->color()==pItem->color() ))
		return false;	//cannot stack.

	if (this->amount+pItem->amount>65535)
	{
		pItem->pos.x=this->pos.x;
		pItem->pos.y=this->pos.y;
		pItem->pos.z=9;
		pItem->amount=(this->amount+pItem->amount)-65535;
		this->amount=65535;
		RefreshItem(pItem);
	}
	else
	{
		this->amount+=pItem->amount;
		Items->DeleItem(pItem);
	}
	RefreshItem(this);
	return true;
}

bool cItem::ContainerPileItem(cItem* pItem)	// try to find an item in the container to stack with
{
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(this->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi->id() == pItem->id() && !pi->free && pi->color() == pItem->color())
			if (pi->PileItem(pItem))
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
int cItem::DeleteAmount(int amount, short id, short color)
{
	int rest=amount,loopexit=0;
	int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->type==1)
			rest=pi->DeleteAmount(rest, id, color);
		if (pi->id()==id
			&& (color==-1 || (pi->color()==color)))
			rest=pi->ReduceAmount(rest);
		if (rest<=0)
			break;
	}
	return rest;
}

void cItem::setId(short id)
{
	id1=id>>8;
	id2=id&0x00FF;
}

void cItem::setColor(short color)
{
	color1=color>>8;
	color2=color&0x00FF;
}

static int getname(int i, char* itemname)
{
	tile_st tile;
	int j, len, mode, used, ok, namLen;
	P_ITEM pi=MAKE_ITEMREF_LRV(i,1);
	if (pi->name[0]!='#')
	{
		strcpy((char*)itemname, pi->name);
		return strlen((char*)itemname)+1;
	}
	Map->SeekTile(pi->id(), &tile);
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
		else if ((mode==1)&&(pi->amount==1)) ok=1;
		else if ((mode==2)&&(pi->amount>1)) ok=1;
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
	return getname(DEREF_P_ITEM(this),itemname);
}

string cItem::getName(void)
{
	char itemname[256] = {0,};
	cItem::getName(itemname);
	string sItemName(itemname);
	return sItemName;
}

// return the weight of an item. May have to seek it from MUL files
int cItem::getWeight()
{
	unsigned int itemweight=0;

	if (this->weight>0) //weight is defined in scripts for this item
		itemweight=this->weight;
	else
	{
		tile_st tile;
		Map->SeekTile(this->id(), &tile);
		if (tile.weight==0) // can't find weight
		{
			if(this->type!=14)
				itemweight = 2;		// not food weighs .02 stone
			else
				itemweight = 100;	//food weighs 1 stone
		}
		else //found the weight from the tile, set it for next time
		{			
			itemweight = tile.weight*100;
			this->weight=itemweight; // set weight so next time don't have to search
		}
	}
	return itemweight;
}





void cAllItems::SetOwnSerial(ITEM i, long ownser)
{
	const P_ITEM pi = MAKE_ITEMREF_LR(i);
	pi->setOwnSerialOnly(ownser);
}

void cAllItems::SetPos(ITEM nItem, short x,short y, signed char z)
{
	const P_ITEM pi = MAKE_ITEMREF_LR(nItem);
	pi->pos.x=x;
	pi->pos.y=y;
	pi->pos.z=z;
}

void cAllItems::CheckMemoryRequest()
{
	if (moreItemMemoryRequested)
		ResizeMemory();
}

bool cAllItems::AllocateMemory(int NumberOfItems)
{
	bool memerr = false;
	imem=NumberOfItems;
	if (imem<100) imem=100;
	imem+=(2*ITEM_RESERVE);	// some reserve so realloc doesn't occur immediately after serverstart(Duke)

	clConsole.send(" Allocating initial dynamic Item memory of %i... ",imem);
	if ((realitems= (cItem *) malloc(imem*sizeof(cItem)))==NULL)
		memerr=true;
	else if ((itemids = (int *) malloc(imem*sizeof(int)))==NULL)
		memerr=true;

	if (memerr)
	{
		clConsole.send("\nERROR: Could not Allocate item memory!\n");
		error=1;
		return false;
	}
	unsigned int ilimit = 0;
	if ((imem-200) > 0)
		ilimit = imem-200 ;	
	for (unsigned int i = ilimit;i<imem;i++)
		realitems[i].free=1;

	clConsole.send("Done\n");
	return true;
}

bool cAllItems::ResizeMemory()
{
	char memerr=0;
	const int slots=4000;   // bugfix for crashes under w95/98, LB never ever touch this number ...
	// it has major influence on performance !! if it too low -> slow and can cause crashes under w95/98.
	// this is because w95/98 can only handle 8196 subsequent realloc calls ( crappy thing...)
	// free() calls DONT help !!!, btw so we have to make that number real big
	// under nt and unix this limit doesnt exist

	if ((realitems = (cItem *)realloc(realitems, (imem + slots)*sizeof(cItem)))==NULL)
		memerr=1;
	else if ((itemids = (int *)realloc(itemids, (imem + slots)*sizeof(int)))==NULL)
		memerr=1;
	
	if (memerr)
	{
		LogCriticalVar("Could not reallocate item memory after %i. No more items will be created.\nWOLFPACK may become unstable.",imem);
		cwmWorldState->savenewworld(1);
		return false;
	}
	else
	{
		for (unsigned int i=imem;i<imem+slots;i++)
			realitems[i].free=1;
		imem+=slots;
		moreItemMemoryRequested=false;
		return true;
	}
}
void cAllItems::CollectReusableSlots()
{
	if (imemover!=1)
	{
		imemover=0;
		imemcheck++;
		for (unsigned int i=0;i<itemcount;i++)
		{
			if (imemcheck==300)
			{
				imemover=1; 
				break;
			}
			if (items[i].free==1)
			{
				freeitemmem[imemcheck]=i;
				imemcheck++;
			}
		}
	}
}

///////////////////////
// Name:	GetReusableSlot
// history:	by Duke, 17.05.2001
// Purpose:	searches for an empty slot in the items array
//			It does this by simply searching from the *last* position where it found a slot.
//			If it reaches the end, it will search from the start. A timer prevents too many wrap-arounds.
//
int cAllItems::GetReusableSlot()
{
	static unsigned int NextMemCheck=0;
	static unsigned int LastFree=0;	// remember this point in the array to continue search from here next time

	unsigned int ctime=uiCurrentTime;
	if (NextMemCheck<=ctime || (overflow))	// didn't wrap around in the past minute
	{
		unsigned int started=LastFree;		// remember where we started the search from here next time
		for (  ;LastFree<itemcount;LastFree++)
		{
			if (items[LastFree].free)
				return LastFree;	
		}
		// we have reached the end so start again from the beginning of the array
		for (LastFree=0;LastFree<started;LastFree++)
		{
			if (items[LastFree].free)
				return LastFree;	
		}
		NextMemCheck=ctime+speed.checkmem*60*MY_CLOCKS_PER_SEC;	// nothing found, set time for next try
	}
	return -1;
}

P_ITEM cAllItems::MemItemFree()// -- Find a free item slot, checking freeitemmem[] first
{
	if (itemcount+ITEM_RESERVE >= imem-I_W_O_1)	//less than 'Reserve' free slots left, so get more memory
		moreItemMemoryRequested=true;

	int nItem=GetReusableSlot();		// try to use the space of a deleted item
	if (nItem==-1)
	{
		if (itemcount+1>=imem-I_W_O_1)	//theres no more free sluts.. er slots
		{
			LogCritical("couldn't resize item memory in time");
			return NULL;
		}
		nItem=itemcount;
		itemcount++;
	}
	return &items[nItem];
}

void cItem::SetSerial(long ser)
{
	this->ser1=(unsigned char) ((ser&0xFF000000)>>24); // Item serial number
	this->ser2=(unsigned char) ((ser&0x00FF0000)>>16);
	this->ser3=(unsigned char) ((ser&0x0000FF00)>>8);
	this->ser4=(unsigned char) ((ser&0x000000FF));
	this->serial=ser;
	if (ser > -1)
		setptr(&itemsp[ser%HASHMAX], DEREF_P_ITEM(this));
}

// -- Initialize an Item in the items array
void cItem::Init(char mkser)
{
//	if (nItem==itemcount) itemcount++;

	if (mkser)		// give it a NEW serial #
	{
		this->SetSerial(itemcount2);
		itemcount2++;
	}
	else
	{
		this->SetSerial(-1);
	}

	strcpy(this->name,"#");
	strcpy(this->name2,"#"); // Magius(CHE)
	this->creator[0] = '\0';
	this->incognito=false;//AntiChrist - incognito
	this->madewith=0; // Added by Magius(CHE)
	this->rank=0; // Magius(CHE)
	this->good=-1; // Magius(CHE)
	this->rndvaluerate=0; // Magius(CHE) (2)

	this->multis=-1;//Multi serial
	this->free=0;
	this->flags.isBeeingDragged=0;
	this->setId(0x0001); // Item visuals as stored in the client
	// this->name2[0]=0x00; Removed by Magius(CHE)
	this->pos.x=this->oldx=100;
	this->pos.y=this->oldy=100;
	this->pos.z=this->oldz=0;
	this->color1=0x00; // Hue
	this->color2=0x00;
	this->setContSerialOnly(-1); // Container that this item is found in
	this->oldcontserial=-1;
	this->layer=this->oldlayer=0; // Layer if equipped on paperdoll
	this->itmhand=0; // Layer if equipped on paperdoll
	this->type=0; // For things that do special things on doubleclicking
	this->type2=0;
	this->offspell=0;
	this->weight=0;
	this->more1=0; // For various stuff
	this->more2=0;
	this->more3=0;
	this->more4=0;
	this->moreb1=0;
	this->moreb2=0;
	this->moreb3=0;
	this->moreb4=0;
	this->morex=0;
	this->morey=0;
	this->morez=0;
	this->amount=1; // Amount of items in pile
	this->amount2=0; //Used to track things like number of yards left in a roll of cloth
	this->doordir=0; // Reserved for doors
	this->dooropen=0;
	this->pileable=0; // Can item be piled
	this->dye=0; // Reserved: Can item be dyed by dye kit
	this->corpse=0; // Is item a corpse
	this->carve=-1;//AntiChrist-for new carving system
	this->att=0; // Item attack
	this->def=0; // Item defense
	this->lodamage=0; //Minimum Damage weapon inflicts
	this->hidamage=0; //Maximum damage weapon inflicts
	this->racehate=-1; //race hating weapon -Fraz-
	this->smelt=0; // for smelting items
	this->secureIt=0; // secured chests
	this->wpsk=0; //The skill needed to use the item
	this->hp=0; //Number of hit points an item has.
	this->maxhp=0; // Max number of hit points an item can have.
	this->st=0; // The strength needed to equip the item
	this->st2=0; // The strength the item gives
	this->dx=0; // The dexterity needed to equip the item
	this->dx2=0; // The dexterity the item gives
	this->in=0; // The intelligence needed to equip the item
	this->in2=0; // The intelligence the item gives
	this->spd=0; //The speed of the weapon
	this->wipe=0; //Should this item be wiped with the /wipe command
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
	this->trigon=0; // equipped item trigger -Frazurbluu-
	this->disabled=0; //Item is disabled, cant trigger.
	this->disabledmsg[0] = 0; //Item disabled message. -- by Magius(CHE) §
	this->tuses=0;    //Number of uses for trigger
	this->poisoned=0; //AntiChrist -- for poisoning skill
	this->murderer[0]=0x00; //AntiChrist -- for corpse -- char's name who kille the char (forensic ev.)
 	this->murdertime=0; //AntiChrist -- for corpse -- when the people has been killed
    this->glow=0;
    this->glow_effect=0;
    this->glow_c1=0;
    this->glow_c2=0;
	this->time_unused=0;
	this->timeused_last=getNormalizedTime();
	this->spawnregion=0;
	
	this->desc[0]=0x00;
}

// -- delete an item (Actually just mark it is free)
void cAllItems::DeleItem(P_ITEM pi)
{		
	int j;

	if ( pi == NULL )
		return;

	if (pi->free==0)
	{
		
		removeitem[1]=pi->ser1;
		removeitem[2]=pi->ser2;
		removeitem[3]=pi->ser3;
		removeitem[4]=pi->ser4;

		if (pi->spawnregion>0 && pi->spawnregion<255)
		{
			spawnregion[pi->spawnregion].current--;
		}

		for (j=0;j<now;j++)
		{
			if (perm[j] && iteminrange(j, pi, VISRANGE)) 
				Xsend(j, removeitem, 5);
		}

		if (pi->glow != INVALID_SERIAL) 
		{  
			P_ITEM pj = FindItemBySerial( pi->glow );
			if (pj != NULL) Items->DeleItem(pj); // lb glow stuff, deletes the glower of an glowing stuff automatically
		}

		pi->SetSpawnSerial(-1);
		pi->SetOwnSerial(-1);
		// - remove from mapRegions if a world item
		if (pi->isInWorld()) 
		{
			mapRegions->Remove(pi);
		}
		else
			pi->SetContSerial(-1);


		if (pi->type==11 && (pi->morex==666 || pi->morey==999)) Books->delete_bokfile(pi); 
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
		removefromptr(&itemsp[pi->serial%HASHMAX], DEREF_P_ITEM(pi));
		pi->free=1;
		pi->pos.x=20+(xcounter++);
		pi->pos.y=50+(ycounter);
		pi->pos.z=9;

		if (xcounter==40)
		{
			ycounter++;
			xcounter=0;
		}
		if (ycounter==80)
		{
			ycounter=0;
			xcounter=0;
		}
		if (imemcheck<500)
		{
			imemcheck++;
			freeitemmem[imemcheck]=DEREF_P_ITEM(pi);
		}
		else imemover=1;
	}
	
}

// sockets
P_ITEM cAllItems::CreateFromScript(UOXSOCKET so, int itemnum)
{
	char sect[512];
	int tmp, loopexit = 0;
	long int pos;
	tile_st tile;
	
	openscript("items.scp");
	sprintf(sect, "ITEM %i", itemnum);
	if (!i_scripts[items_script]->find(sect))
	{
		closescript();
		if (n_scripts[custom_item_script][0] != 0)
		{
			openscript(n_scripts[custom_item_script]);
			if (!i_scripts[custom_item_script]->find(sect))
			{
				closescript(); // AntiChrist
				return NULL;
			}
			else 
				strcpy(sect, n_scripts[custom_item_script]);
		} else 
			return NULL;
	} else 
		strcpy(sect, "items.scp");
	
	P_ITEM pi = Items->MemItemFree();
	if (pi == NULL)
	{
		closescript();	// make sure script is closed before make_itemref might exit (Duke, 27.4.01)
		return NULL;
	}
	pi->Init();
	
	pi->setId(0x0915);
	
	do
	{
		read2();
		if (script1[0] != '}')
		{
			switch (script1[0])
			{
				case 'A':
				case 'a':
					if (!strcmp("AMOUNT", (char*)script1))		pi->amount = str2num(script2);
					else if (!strcmp("ATT", (char*)script1)) 	pi->att = getstatskillvalue((char*)script2);
					break;
					
				case 'C':
				case 'c':
					if (!strcmp("COLOR", (char*)script1))
					{
						tmp = hex2num(script2);
						pi->color1 = tmp >> 8;
						pi->color2 = tmp%256;
					}
					else if (!strcmp("CREATOR", (char*)script1))		strcpy(pi->creator, (char*)script2); // by Magius(CHE)
					else if (!strcmp("COLORLIST", (char*)script1))
					{
						pos = ftell(scpfile);
						closescript();
						tmp = addrandomcolor(so, (char*)script2);
						{
							pi->color1 = tmp >> 8;
							pi->color2 = tmp%256;
						}
						openscript(sect);
						fseek(scpfile, pos, SEEK_SET);
						strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
					}
					break;
					
				case 'D':
				case 'd':
					if (!strcmp("DAMAGE", (char*)script1))		pi->att = getstatskillvalue((char*)script2);
					else if (!strcmp("DEX", (char*)script1))
						pi->dx = str2num(script2);
					else if (!strcmp("DISABLED", (char*)script1))
						pi->disabled = uiCurrentTime + (str2num(script2)*MY_CLOCKS_PER_SEC);// AntiChrist
					else if (!strcmp("DISABLEMSG", (char*)script1))
						strcpy(pi->disabledmsg, (char*)script2);  
					else if (!strcmp("DISPELLABLE", (char*)script1))
						pi->priv |= 0x04;
					else if (!strcmp("DECAY", (char*)script1))
						pi->priv |= 0x01;
					else if (!strcmp("DIR", (char*)script1))
						pi->dir = str2num(script2);
					else if (!strcmp("DYE", (char*)script1))
						pi->dye = str2num(script2);
					else if (!strcmp("DEXADD", (char*)script1))
						pi->dx2 = str2num(script2);
					else if (!strcmp("DEF", (char*)script1))
						pi->def = str2num(script2);
					break;
					
				case 'G':
				case 'g':
					if (!(strcmp("GOOD", (char*)script1)))
						pi->good = str2num(script2); // Added by Magius(CHE)
					break;
					
				case 'H':
				case 'h':
					if (!(strcmp("HIDAMAGE", (char*)script1)))
						pi->hidamage = str2num(script2);
					else if (!(strcmp("HP", (char*)script1)))
						pi->hp = str2num(script2);
					break;
					
				case 'I':
				case 'i':
					if (!(strcmp("ID", (char*)script1)))
					{
						tmp = hex2num(script2);
						pi->setId(tmp);
					}
					else if (!strcmp("ITEMLIST", (char*)script1))
					{
						pos = ftell(scpfile);
						closescript();
						Items->DeleItem(pi);
						pi=CreateScriptRandomItem(so, (char*)script2);
						openscript(sect);
						fseek(scpfile, pos, SEEK_SET);
						strcpy((char*)script1, "DUMMY");
					}
					else if (!strcmp("INT", (char*)script1))
						pi->in = str2num(script2);
					else if (!strcmp("INTADD", (char*)script1))
						pi->in2 = str2num(script2);
					else if (!strcmp("ITEMHAND", (char*)script1))
						pi->itmhand = str2num(script2);
					break;
					
				case 'L':
				case 'l':
					if (!strcmp("LAYER", (char*)script1) && (so==-1))
						pi->layer = str2num(script2);
					else if (!strcmp("LODAMAGE", (char*)script1))
						pi->lodamage = str2num(script2);
					break;
					
				case 'M':
				case 'm':
					if (!strcmp("MORE", (char*)script1))
					{
						tmp = str2num(script2);
						pi->more1 = tmp >> 24;
						pi->more2 = tmp >> 16;
						pi->more3 = tmp >> 8;
						pi->more4 = tmp%256;
					}
					// MORE2 may not be useful ?
					else if (!strcmp("MORE2", (char*)script1))
					{
						tmp = str2num(script2);
						pi->moreb1 = tmp >> 24;
						pi->moreb2 = tmp >> 16;
						pi->moreb3 = tmp >> 8;
						pi->moreb4 = tmp%256;
					}
					else if (!strcmp("MOVABLE", (char*)script1))
						pi->magic = str2num(script2);
					else if (!strcmp("MAXHP", (char*)script1))
						pi->maxhp = str2num(script2); // by Magius(CHE)
					else if (!strcmp("MOREX", (char*)script1))
						pi->morex = str2num(script2);
					else if (!strcmp("MOREY", (char*)script1))
						pi->morey = str2num(script2);
					else if (!strcmp("MOREZ", (char*)script1))
						pi->morez = str2num(script2);
					break;
					
				case 'N':
				case 'n':
					if (!strcmp("NEWBIE", (char*)script1))
						pi->priv = pi->priv | 0x02;
					else if (!strcmp("NAME", (char*)script1))
						strcpy(pi->name, (char*)script2);
					else if (!strcmp("NAME2", (char*)script1))
						strcpy(pi->name2, (char*)script2);
					break;
					
				case 'O':
				case 'o':
					if (!strcmp("OFFSPELL", (char*)script1))
						pi->offspell = str2num(script2);
					break;
					
				case 'P':
				case 'p':
					if (!strcmp("POISONED", (char*)script1))
						pi->poisoned = str2num(script2);
					break;
					
				case 'R':
				case 'r':
					if (!strcmp("RACEHATE", (char*)script1))
						pi->racehate = str2num(script2);				
					else if (!strcmp("RANK", (char*)script1))
					{
						pi->rank = str2num(script2); // By Magius(CHE)
						if (pi->rank <= 0)
							pi->rank = 10;
					}
					else if (!(strcmp("RESTOCK", (char*)script1)))
						pi->restock = str2num(script2);
					break;
					
					
				case 'S':
				case 's':
					if (!strcmp("SK_MADE", (char*)script1))
						pi->madewith = str2num(script2); // by Magius(CHE)
					else if (!strcmp("SMELT", (char*)script1))
						pi->smelt = str2num(script2);
					else if (!strcmp("STR", (char*)script1))
						pi->st = str2num(script2);
					else if (!strcmp("SPD", (char*)script1))
						pi->spd = str2num(script2);
					else if (!strcmp("STRADD", (char*)script1))
						pi->st2 = str2num(script2);
					break;
					
				case 'T':
				case 't':
					if (!strcmp("TYPE", (char*)script1))
						pi->type = str2num(script2);
					else if (!strcmp("TRIGGER", (char*)script1))
						pi->trigger = str2num(script2);
					else if (!strcmp("TRIGTYPE", (char*)script1))
						pi->trigtype = str2num(script2);
					else if (!strcmp("TRIGON", (char*)script1))
						pi->trigon = str2num(script2);// New trigger type for items -Frazurbluu-
					break;
					
				case 'U':
				case 'u':
					if (!strcmp("USES", (char*)script1))
						pi->tuses = str2num(script2);
					break;
					
				case 'V':
				case 'v':
					if (!strcmp("VISIBLE", (char*)script1))
						pi->visible = str2num(script2);
					else if (!strcmp("VALUE", (char*)script1))
						pi->value = str2num(script2);
					break;
					
				case 'W':
				case 'w':
					if (!strcmp("WEIGHT", (char*)script1))
					{
						//int anum = 3;
						// anum=4;
						int anum = str2num(script2); // Ison 2-20-99
						pi->weight = anum;
					}
					break;
			}
		}
	}
	while ((script1[0] != '}') &&(++loopexit < MAXLOOPS));

	closescript();

	if (pi)	// checking pi is needed here because ITEMLIST can set it to NULL (Duke, 4.9.01)
	{
		Map->SeekTile(pi->id(), &tile);
		if (tile.flag2&0x08)
			pi->pileable = 1;
		
		if (!pi->maxhp && pi->hp)
			pi->maxhp = pi->hp; // Magius(CHE)
	}
	
	return pi;	
}

P_ITEM cAllItems::CreateScriptItem(int s, int itemnum, int nSpawned)
{
	P_ITEM pi = Items->CreateFromScript(s,itemnum);
	if (pi == NULL)
	{
		LogWarningVar("ITEM <%i> not found in the scripts",itemnum);
		return NULL;
	}

	if ((s!=-1) && (!nSpawned))
	{
		if (triggerx)
		{
			pi->MoveTo(triggerx,triggery,triggerz);
		}
		else
		{
			short xx,yy;
			signed char zz;
			xx=(buffer[s][11]<<8)+buffer[s][12];
			yy=(buffer[s][13]<<8)+buffer[s][14];
			zz=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
			pi->MoveTo(xx,yy,zz);
		}
	}
	else
	{
		if (pi->isInWorld())
			mapRegions->Add(pi);
	}

	return pi;
}

int cAllItems::CreateRandomItem(char * sItemList)//NEW FUNCTION -- 24/6/99 -- AntiChrist merging codes
{
	int i=0, loopexit=0, iList[256];  //-- no more than 256 items in a single item list
	char sect[512];
	openscript("items.scp");
	sprintf(sect, "ITEMLIST %s", sItemList);
	if (!i_scripts[items_script]->find(sect)) // -- Valid itemlist?
	{
		closescript();
		if (n_scripts[custom_item_script][0]!=0)
		{
			openscript(n_scripts[custom_item_script]);
			if (!i_scripts[custom_item_script]->find(sect))
			{
				closescript(); //AntiChrist
				return -1;
			}
		} else return -1;
	}

	do  // -- count items storing item #'s in iList[]
	{
		read1();
		if (script1[0]!='}')
		{
			iList[i]=str2num(script1);
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();

	if (i==0) return iList[0]; else return(iList[rand()%i]);
}

cItem* cAllItems::CreateScriptRandomItem(int s, char * sItemList)
{
	int i=0, loopexit=0, iList[512], k;  //-- no more than 512 items in a single item list (changed by Magius(CHE))
	char sect[512];
	openscript("items.scp");
	sprintf(sect, "ITEMLIST %s", sItemList);
	if (!i_scripts[items_script]->find(sect)) // -- Valid itemlist?
	{
		closescript();
		if (n_scripts[custom_item_script][0]!=0)
		{
			openscript(n_scripts[custom_item_script]);
			if (!i_scripts[custom_item_script]->find(sect))
			{
				closescript(); //AntiChrist
				return NULL;
			}
		} else return NULL;
	}

	do  // -- count items storing item #'s in iList[]
	{
		read1();
		if (script1[0]!='}')
		{
			iList[i]=str2num(script1);
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();


	if (i!=0) i=rand()%(i);
		k=iList[i];   // -- Get random Item #

	if (k!=0)
	{
		return CreateScriptItem(s, k, 1);  // -- Create Item
	}
	return NULL;
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
					int nAmount, char* cName, int nStackable,
					unsigned char cItemId1, unsigned char cItemId2,
					unsigned char cColorId1, unsigned char cColorId2,
					int nPack, int nSend)
{
	if (nSocket < 0)
	{
		clConsole.send("ERROR: SpawnItem called with bad socket\n");
		return NULL;
	}
	else
	{
		CHARACTER ch = currchar[nSocket];
		return SpawnItem(nSocket, ch, nAmount, cName, nStackable, cItemId1, cItemId2, cColorId1, cColorId2, nPack, nSend);
	}
}

P_ITEM cAllItems::SpawnItem(UOXSOCKET nSocket, CHARACTER ch,
					int nAmount, char* cName, int nStackable,
					unsigned char cItemId1, unsigned char cItemId2,
					unsigned char cColorId1, unsigned char cColorId2,
					int nPack, int nSend)
{
	P_ITEM pi=SpawnItem(ch, nAmount, cName, nStackable,(short)((cItemId1<<8)+cItemId2), (short)((cColorId1<<8)+cColorId2), nPack);
	if (pi==NULL) return NULL;
	if (nSend && nSocket>=0)
		statwindow(nSocket,ch);
	return pi;
}

P_ITEM cAllItems::SpawnItemBank(CHARACTER ch, int nItem)
{
	P_CHAR pc_ch = MAKE_CHAR_REF(ch);
	if (pc_ch == NULL) 
		return NULL;
	
	P_ITEM bankbox = pc_ch->GetBankBox();
	
	if (bankbox == NULL)
	{
		LogWarning("Bank box not found in SpawnItemBank()");
		return NULL;
	}

	UOXSOCKET s = calcSocketFromChar(DEREF_P_CHAR(pc_ch));          // Don't check if s == -1, it's ok if it is.
	P_ITEM pi = CreateScriptItem(s, nItem, 1);
	if (pi == NULL)
		return NULL;
	GetScriptItemSetting(pi); 
	bankbox->AddItem(pi);
	statwindow(s, DEREF_P_CHAR(pc_ch));
	return pi;
}

P_ITEM cAllItems::SpawnItem(CHARACTER ch,int nAmount, char* cName, char pileable, short id, short color, bool bPack)
{
	if (ch < 0) 
		return NULL;

	P_CHAR pc_ch = MAKE_CHAR_REF(ch);

	P_ITEM pPack=Packitem(pc_ch);
	char pile = 0;
	
	if (pileable==1)
	{					// make sure it's REALLY pileable ! (Duke)
		tile_st tile;
		Map->SeekTile(id, &tile);
		if (tile.flag2&0x08)
			pile=1;
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
			if (pSt->id()==id && !pSt->free && pSt->color()==color)
			{
				if (pSt->amount+nAmount > 65535)	// if it would create an overflow (amount is ushort!),
					continue;						// let's search for another pile to add to
				pSt->amount += nAmount;
				RefreshItem(pSt);
				return pSt;
			}
		}
	}
	// no such item found, so let's create it
	P_ITEM pi = Items->MemItemFree();
	if (pi == NULL) return NULL;

	pi->Init();
	if(cName!=NULL)
		strcpy(pi->name,cName);
	pi->setId(id);
	pi->color1=color>>8;
	pi->color2=color&0x00FF;
	pi->amount=nAmount;
	pi->pileable=pile;
	pi->att=5;
	pi->priv |= 0x01;
	if (IsCutCloth(pi->id())) pi->dye=1;// -Fraz- fix for cut cloth not dying
	if (bPack)
	{
		if (pPack)
		{
			pi->SetContSerial(pPack->serial);
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
	RefreshItem(pi);
	return pi;
}

void cAllItems::GetScriptItemSetting(P_ITEM pi)
{// by Magius(CHE) //bugfixed by AntiChrist
	//clConsole.send("cAllItems::GetScriptItemSetting....");//debug

	unsigned char tscript1[512];//AntiChrist - important bug/crash fix!!!
	unsigned char tscript2[512];//these have to be backed up before using them!!!
	unsigned long loopexit=0;

	strcpy((char*)tscript1,(char*)script1);//AntiChrist
	strcpy((char*)tscript2,(char*)script2);

	if(pi == NULL) 
		return;

	int tmp;
	bool sectfound=false;
	char buff[512];
    openscript("harditems.scp");
	buff[0] = '\0'; // Fix by Magius(CHE)

	sprintf(buff,"x%x",pi->id());

	//clConsole.send("Checking...%s\n",buff);
	//if (!i_scripts[hard_items]->find("SECTION"))//AntiChrist bugfix
	//{
	//	closescript();
	//	return;
	//}
	//clConsole.send("Rewrite item setting for item %x%x\n",pi->id1,pi->id2);
	do
	{
		read2();
		//clConsole.send("%s %s\n",script1,script2);
		if (script1[0]!='}')
		{
			if (script1[0] != 'S')
				continue;
			if (!(strcmp("SECTION",(char*)script1))) 
				if(strstr((char*)script2,buff)) sectfound = true;//AntiChrist bugfix

			if(sectfound)//AntiChrist bugfix
			{
				switch (script1[0])
				{
				case 'A':
				case 'a':
					if (!(strcmp("AMOUNT",(char*)script1))) pi->amount = str2num(script2); // -Fraz- moved from Case C
				case 'C':
				case 'c':
 					if (!(strcmp("CREATOR", (char*)script1))) strcpy(pi->creator, script2); // by Magius(CHE)
					else if (!(strcmp("COLOR",(char*)script1)))
					{
						tmp=hex2num(script2);
						pi->color1=tmp>>8;
						pi->color2=tmp%256;
					}
				break;
				
				case 'D':
				case 'd':
					if (!(strcmp("DEX", (char*)script1))) pi->dx=str2num(script2);
					else if (!(strcmp("DEXADD", (char*)script1))) pi->dx2=str2num(script2);
					else if ((!(strcmp("DAMAGE",(char*)script1)))||(!(strcmp("ATT",(char*)script1)))) 
						pi->att = getstatskillvalue((char*)script2);
					else if (!(strcmp("DEF",(char*)script1))) pi->def=str2num(script2);
					else if (!(strcmp("DYE",(char*)script1))) pi->dye=str2num(script2);
					else if (!(strcmp("DIR",(char*)script1))) pi->dir=str2num(script2);
					else if (!(strcmp("DECAY",(char*)script1))) pi->priv=pi->priv|0x01;
					else if (!(strcmp("DISPELLABLE",(char*)script1))) pi->priv=pi->priv|0x04;
					else if (!(strcmp("DISABLEMSG",(char*)script1))) strcpy(pi->disabledmsg,script2);  // Added by Magius(CHE) §
					else if (!(strcmp("DISABLED",(char*)script1))) pi->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC);//AntiChrist
				break;

				case 'G':
				case 'g':
				case 'H':
				case 'h':
					if (!(strcmp("GOOD",(char*)script1))) pi->good=str2num(script2); // Added by Magius(CHE)
					else if (!(strcmp("HP", (char*)script1))) pi->hp=str2num(script2);
					else if (!(strcmp("HIDAMAGE", (char*)script1))) pi->hidamage=str2num(script2);
				break;
				
				case 'I':
				case 'i':
				case 'L':
				case 'l':
					if (!(strcmp("INT", (char*)script1))) pi->in=str2num(script2);
					else if (!(strcmp("INTADD", (char*)script1))) pi->in2=str2num(script2);
					else if (!(strcmp("ITEMHAND",(char*)script1))) pi->itmhand=str2num(script2);
					else if (!(strcmp("LAYER",(char*)script1))) pi->layer=str2num(script2);
					else if (!(strcmp("LODAMAGE", (char*)script1))) pi->lodamage=str2num(script2);
				break;

				case 'M':
				case 'm':
					if (!(strcmp("MAXHP", (char*)script1))) pi->maxhp=str2num(script2); // by Magius(CHE)
					else if (!(strcmp("MOVABLE",(char*)script1))) pi->magic=str2num(script2);
					else if (!(strcmp("MORE", (char*)script1)))
					{
						tmp=str2num(script2);
						pi->more1 = (unsigned char) (tmp>>24);
						pi->more2 = (unsigned char) (tmp>>16);
						pi->more3 = (unsigned char) (tmp>>8);
						pi->more4 = (unsigned char) (tmp%256);
					}
					//MORE2 may not be useful ?
					else if (!(strcmp("MORE2", (char*)script1)))
					{
						tmp=str2num(script2);
						pi->moreb1=tmp>>24;
						pi->moreb2=tmp>>16;
						pi->moreb3=tmp>>8;
						pi->moreb4=tmp%256;
					}
					else if (!(strcmp("MOREX",(char*)script1))) pi->morex=str2num(script2);
					else if (!(strcmp("MOREY",(char*)script1))) pi->morey=str2num(script2);
					else if (!(strcmp("MOREZ",(char*)script1))) pi->morez=str2num(script2);
				break;

				case 'N':
				case 'n':
				case 'O':
				case 'o':
					if (!(strcmp("NAME",(char*)script1))) strcpy(pi->name, script2);
					else if (!(strcmp("NAME2",(char*)script1))) strcpy(pi->name2, script2);
					else if (!(strcmp("NEWBIE",(char*)script1))) pi->priv=pi->priv|0x02;
					else if (!(strcmp("OFFSPELL",(char*)script1))) pi->offspell=str2num(script2);

				break;

				case 'P':
				case 'p':
				case 'R':
				case 'r':
					if (!strcmp("POISONED",(char*)script1)) pi->poisoned=str2num(script2);
					else if (!strcmp("RACEHATE",(char*)script1)) pi->racehate=str2num(script2);
					else if (!strcmp("RESTOCK",(char*)script1)) pi->restock=str2num(script2);
					else if (!strcmp("RANK",(char*)script1))
					{
						pi->rank=str2num(script2); // By Magius(CHE)
						if (pi->rank==0) pi->rank=10;
					}
				break;
				
				case 'S':
				case 's':
					if (!(strcmp("SPD",(char*)script1))) pi->spd=str2num(script2);
					else if (!(strcmp("SK_MADE", (char*)script1))) pi->madewith=str2num(script2); // by Magius(CHE)
					else if (!(strcmp("STR", (char*)script1))) pi->st=str2num(script2);
					else if (!(strcmp("STRADD", (char*)script1))) pi->st2=str2num(script2);
					else if (!(strcmp("SMELT", (char*)script1))) pi->smelt=str2num(script2);
				break;
				
				case 'T':
				case 't':
					if (!(strcmp("TRIGGER",(char*)script1))) pi->trigger=str2num(script2);
					else if (!(strcmp("TRIGTYPE",(char*)script1))) pi->trigtype=str2num(script2);
					else if (!(strcmp("TRIGON",(char*)script1))) pi->trigon=str2num(script2);					
					else if (!(strcmp("TYPE",(char*)script1))) pi->type=str2num(script2);				
				case 'U':
				case 'u':
					if (!(strcmp("USES",(char*)script1))) pi->tuses=str2num(script2);
				break;

				default:
					if (!(strcmp("VISIBLE",(char*)script1))) pi->visible=str2num(script2);
					else if (!(strcmp("VALUE",(char*)script1))) pi->value=str2num(script2);

					else if (!(strcmp("WEIGHT",(char*)script1)))
					{
						//int anum=3;
						//anum=4;
						int anum=str2num(script2); // Ison 2-20-99
						pi->weight=anum;
						//clConsole.send("SETTING WEIGTH TO %i\n",pi->weight);
					}
					//Added by Krozy on 7-Sep-98
				break;
				}
			}
		}
		if(script1[0]=='}' && sectfound) break;
	}
	while( (strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS) );
    //while ((script1[0]!='}' && !sectfound) && strcmp(script1, "EOF"));

    closescript();
	//clConsole.send("Done.\n");//debug

	strcpy((char*)script1,(char*)tscript1);//AntiChrist
	strcpy((char*)script2,(char*)tscript2);

}

P_ITEM cAllItems::SpawnItemBackpack2(UOXSOCKET s, int nItem, int nDigging) // Added by Genesis 11-5-98
{
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	P_ITEM backpack = Packitem(pc_currchar);
	
	P_ITEM pi = CreateScriptItem(s, nItem, 1);
	if (pi == NULL)
		return NULL;

	if(nDigging) 
	{
		if (pi->value!=0) pi->value=1+(rand()%(pi->value)); 
		if (pi->hp!=0) pi->hp=1+(rand()%(pi->hp));
		// blackwinds fix 
		if (pi->maxhp != 0)
		{
			pi->maxhp=1+(rand()%pi->maxhp); 
			pi->hp=1+(rand()%pi->maxhp);
		}
	}

	GetScriptItemSetting(pi);

	backpack->AddItem(pi);
	RefreshItem(pi);
	
	statwindow(s, currchar[s]);
	return pi;
}

char cAllItems::isFieldSpellItem(int i) //LB
{
	int a=0;
	P_ITEM pi=MAKE_ITEMREF_LRV(i,0);
	short id=pi->id();
	if (id==0x3996 || id==0x398C) a=1; // fire field
	if (id==0x3915 || id==0x3920) a=2; // poison field
	if (id==0x3979 || id==0x3967) a=3; // paralyse field
	if (id==0x3956 || id==0x3946) a=4; // energy field;
	if (id==0x0080) a=5;                // wall of stone
	if (id>=0x122a && id <=0x122e) a=6; // blood
	
	return a;
}
//taken from 6904t2(5/10/99) - AntiChrist
void cAllItems::DecayItem(unsigned int currenttime, P_ITEM pi) 
{
	int j, serial, serhash, ci, preservebody;
	if ( pi == NULL )
		return;
	P_ITEM pi_multi = NULL;

	if(pi->magic==4) {pi->decaytime=0; return;}
	if( pi->decaytime <= currenttime || (overflow) )//fixed by JustMichael
	{
		if (pi->priv&0x01 && pi->isInWorld() && pi->free==0)
		{  // decaytime = 5 minutes, * 60 secs per min, * MY_CLOCKS_PER_SEC
			if (pi->decaytime==0) 
			{
				pi->startDecay();
			}
			
			if (pi->decaytime<=currenttime)
			{
                //Multis --Boats ->

				if (!Items->isFieldSpellItem(DEREF_P_ITEM(pi))) // Gives fieldspells a chance to decay in multis, LB
				{
				  if (pi->multis<1 && !pi->corpse)
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
				  else if (pi->multis>0 && !pi->corpse) 
				{					
					  pi->startDecay();
					  return;
				}
				}
				//End Boats/Mutlis

				//JustMichael--Keep player's corpse as long as it has more than 1 item on it
				//up to playercorpsedecaymultiplier times the decay rate
				if (pi->corpse == 1 && pi->GetOwnSerial()!=-1)
				{
					preservebody=0;
					serial=pi->serial;
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
				if( (pi->type == 1 && pi->corpse != 1 ) || (pi->GetOwnSerial() != -1 && pi->corpse) || (!SrvParms->lootdecayswithcorpse && pi->corpse ))
				{
					serial=pi->serial;
					serhash=serial%HASHMAX;
					vector<SERIAL> vecContainer = contsp.getData(serial);
					for (ci=0;ci<vecContainer.size();ci++)
					{
						P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
                        if (pi_j != NULL) //lb
						{
						   if (pi_j->contserial==pi->serial)// && (items[j].layer!=0x0B)&&(items[j].layer!=0x10))
						   {
							pi_j->SetContSerial(-1);
							pi_j->MoveTo(pi->pos.x,pi->pos.y,pi->pos.z);

							pi_j->startDecay();

							RefreshItem(pi_j);//AntiChrist
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

//NEW RESPAWNITEM FUNCTION STARTS HERE -- AntiChrist merging codes -- (24/6/99)

void cAllItems::RespawnItem(unsigned int currenttime, P_ITEM pi)
{
	int  j, k,m,serial,ci, c;
	//char ilist[66]="101010100010100101010100001101010000110101010101011010";
	if (pi == NULL)
		return;
	if (pi->free) return;

	for(c=0;c<pi->amount;c++)
	{
		if(pi->gatetime+(c*pi->morez*MY_CLOCKS_PER_SEC)<=currenttime)// && chars[i].hp<=chars[i].st)
		{
			if ((pi->disabled>0)&&((pi->disabled<=currenttime)||(overflow)))
			{
				pi->disabled=0;
			}
			m=0;
			if (pi->type==61)
			{
				k=0;
				serial=pi->serial;
				vector<SERIAL> vecSpawn = spawnsp.getData(pi->serial);
				for (j = 0; j < vecSpawn.size(); j++)
				{
					P_ITEM pi_ci = FindItemBySerial(vecSpawn[j]);
					if (pi_ci != NULL) 
						if((pi_ci->free==0))
						{
							if (pi != pi_ci && pi_ci->pos.x == pi->pos.x && pi_ci->pos.y == pi->pos.y && pi_ci->pos.z == pi->pos.z)
							{
								k = 1;
								break;
							}
						}
				}

				if (k==0)
				{
					if (pi->gatetime==0)
					{
						pi->gatetime=(rand()%((int)(1+((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +
							(pi->morey*MY_CLOCKS_PER_SEC*60)+currenttime;
					}
					if ((pi->gatetime<=currenttime ||(overflow)) && pi->morex!=0)
					{
						Items->AddRespawnItem(pi, pi->morex, 0);
						pi->gatetime=0;
					}
				}
			}
			else if (pi->type==62 || pi->type==69 || pi->type==125)
			{
				k=0;
				serial=pi->serial;
				vector<SERIAL> vecSpawned = cspawnsp.getData(pi->serial);
				for (j = 0; j < vecSpawned.size(); j++)
				{
					P_CHAR pc_ci = FindCharBySerial(vecSpawned[j]);
					if (pc_ci != NULL)
						if (pc_ci->spawnserial==pi->serial && !pc_ci->free)
						{
							k++;
						}
				}

				if (k<pi->amount)
				{
					if (pi->gatetime==0)
					{
						pi->gatetime=(rand()%((int)(1+
							((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +
							
							(pi->morey*MY_CLOCKS_PER_SEC*60)+currenttime;
					}
					if ((pi->gatetime<=currenttime || (overflow)) && pi->morex!=0)
					{
						Npcs->AddRespawnNPC(DEREF_P_ITEM(pi),pi->morex,1);//If you are trying to spawn NPCs you must call an
						pi->gatetime=0;					//NPC spawn not an item spawn. Fixed 9-3-99 Just Michael
					}
				}
			}
			else if ((pi->type==63)||(pi->type==64)||(pi->type==65)||(pi->type==66)||(pi->type==8))
			{
				serial=pi->serial;
				vector<SERIAL> vecContainer = contsp.getData(pi->serial);
				for (j=0;j<vecContainer.size();j++)
				{
					P_ITEM pi_ci = FindItemBySerial(vecContainer[j]);
					if (pi_ci != NULL)
					if (pi_ci->contserial == pi->serial && pi_ci->free==0)
					{
						m++;
					}
				}
				if(m<pi->amount)
				{
					if (pi->gatetime==0)
					{
						pi->gatetime=(rand()%((int)(1+((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +
							(pi->morey*MY_CLOCKS_PER_SEC*60)+uiCurrentTime;
					}
					if ((pi->gatetime<=currenttime ||(overflow)) && pi->morex!=0)
					{
						if(pi->type==63) pi->type=64; //Lock the container 
						//numtostr(pi->morex,m); //ilist); //LB, makes chest spawners using random Itemlist items instead of a single type, LB							
						if(pi->morex)
							Items->AddRespawnItem(pi, pi->morex, 1);//If the item contains an item list then it will randomly choose one from the list, JM
						else
						{
							ci=Items->CreateRandomItem("70"); //default itemlist);
							Items->AddRespawnItem(pi,ci, 1);
						}
						pi->gatetime=0;	
					}
				}
			}
		}//If time
	}//for 
}

void cAllItems::AddRespawnItem(P_ITEM pItem, int x, int y)
{
	if (pItem == NULL)
		return;

	P_ITEM pi = CreateScriptItem(-1, x, 1); // lb, bugfix
	if (pi == NULL) return;
	
	if(y<=0)
	{
		pi->MoveTo(pItem->pos.x, pItem->pos.y, pItem->pos.z); //add spawned item to map cell if not in a container
	}
	else
	{
		pi->SetContSerial(pItem->serial); //set item in pointer array
	}
	pi->SetSpawnSerial(pItem->serial);


	//** Lb bugfix for spawning in wrong pack positions **//
	if (y>0)
	{
		P_ITEM pChest = NULL;
		if (pi->spawnserial!=-1)
			pChest=FindItemBySerial(pi->spawnserial);
		if (pChest)
		{
			pi->SetRandPosInCont(pChest);
		}
	}
	RefreshItem(pi);//AntiChrist
}

void cAllItems::CheckEquipment(P_CHAR pc_p) // check equipment of character p
{
	if (pc_p == NULL)
		return;

	int ci=0, loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc_p->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if(pi->st>pc_p->st)//if strength required > character's strength
		{
			if(pi->name[0]=='#')
				pi->getName(temp2);
			else
				strcpy((char*)temp2,pi->name);
			
			sprintf((char*)temp, "You are not strong enough to keep %s equipped!", temp2);
			sysmessage(calcSocketFromChar(DEREF_P_CHAR(pc_p)), (char*)temp);
			itemsfx(calcSocketFromChar(DEREF_P_CHAR(pc_p)), pi->id());
			
			//Subtract stats bonus and poison
			pc_p->removeItemBonus(pi);
			if ((pi->trigon==1) && (pi->layer >0))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
			{
				triggerwitem(DEREF_P_CHAR(pc_p), pi, 1); // trigger is fired when unequipped? sorry this needs checked
			}
						
			pi->SetContSerial(-1);
			pi->MoveTo(pc_p->pos.x,pc_p->pos.y,pc_p->pos.z);
			
			for (int j=0;j<now;j++)
				if (inrange1p(DEREF_P_CHAR(pc_p), currchar[j])&&perm[j])
				{
					wornitems(j, DEREF_P_CHAR(pc_p));
					senditem(j, pi);
				}
		}
	}		
}

/////////////
// name:	AllItemsIterator
// purpose:	intended to replace the itemcount loops
// history:	by Duke, 27.6.01
//
P_ITEM AllItemsIterator::First()
{
	return pos < itemcount ? &items[pos] : NULL;
}
P_ITEM AllItemsIterator::Next()
{
	return ++pos < itemcount ? &items[pos] : NULL;
}
P_ITEM AllItemsIterator::GetData(void)
{ 
	return pos < itemcount ? &items[pos] : NULL; 
}


