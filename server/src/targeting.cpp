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
//#include "cmdtable.h" // who took this out and why?? cj 8/11/99
#include "targeting.h"
#include "trigger.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "wpscriptmanager.h"
#include "wptargetrequests.h"
#include "mapstuff.h"
#include "classes.h"
#include "gumps.h"
#include "network.h"

#undef DBGFILE
#define DBGFILE "targeting.cpp"
//#include "debug.h"

///////////////////
// class:	cTarget
// purpose:	base class of all Target classes
//			encapsulates the basic functions of target processing
// remarks:	intended to become a member of the 'transaction' class in the future
//			the cTargets::multitarget shall become the ctarget::factory
//			Duke, 07/16/00
//			Changed my mind. These classes are now considered 'experimental'
//			Duke, 7.11.2000
//
class cTarget
{
protected:
	UOXSOCKET s;
	SERIAL serial;
	void makeSerial()		{serial=LongFromCharPtr(buffer[s]+7);}
public:
	cTarget(P_CLIENT pCli)	{s=pCli->GetSocket();}
	virtual void process() = 0;
};

class cCharTarget : public virtual cTarget
{
protected:
	P_CHAR pc;

public:
	cCharTarget(P_CLIENT pCli) : cTarget(pCli) {}
	virtual void CharSpecific() = 0;
	virtual void process()
	{
		makeSerial();
		pc = FindCharBySerial(serial);
		if (pc != NULL)
			CharSpecific();
		else
			sysmessage(s,"That is not a character.");
	}
};

class cItemTarget : public virtual cTarget
{
protected:
	P_ITEM pi;
public:
	cItemTarget(P_CLIENT pCli) : cTarget(pCli) {}
	virtual void ItemSpecific() = 0;
	virtual void process()
	{
		makeSerial();
		pi = FindItemBySerial(serial);
		if (pi != NULL)
			ItemSpecific();
		else
			sysmessage(s, "That is not an item.");
	}
};

class cWpObjTarget : public virtual cItemTarget, public virtual cCharTarget
{
public:
	cWpObjTarget(P_CLIENT pCli) : cItemTarget(pCli), cCharTarget(pCli), cTarget(pCli) {}
//	virtual void CharSpecific() = 0;
//	virtual void ItemSpecific() = 0;
	virtual void process()
	{
		makeSerial();
		if(isItemSerial(serial)) // an item's serial ?
		{
			pi = FindItemBySerial(serial);
			if (pi != NULL)
				ItemSpecific();
			else
				sysmessage(s,"That is not a valid item.");
		}
		else
		{
			pc = FindCharBySerial(serial);
			if (pc != NULL)
				CharSpecific();
			else
				sysmessage(s,"That is not a valid character.");
		}
	}
};

void cTargets::PlVBuy(int s)//PlayerVendors
{
	if (s == -1) 
		return;
	int v = addx[s];
	P_CHAR pc = FindCharBySerial(v);
	if (pc->free) return;
	P_CHAR pc_currchar = currchar[s];

	P_ITEM pBackpack = Packitem(pc_currchar);
	if (!pBackpack) {sysmessage(s,"Time to buy a backpack"); return; } //LB

	int serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi=FindItemBySerial(serial);		// the item
	if (pi==NULL) return;
	if (pi->isInWorld()) return;
	int price=pi->value;

	P_ITEM np = FindItemBySerial(pi->contserial);		// the pack
	P_CHAR npc = GetPackOwner(np);				// the vendor
	if(npc != pc || pc->npcaitype() != 17) return;

	if (pc_currchar->Owns(pc))
	{
		npctalk(s, pc, "I work for you, you need not buy things from me!",0);
		return;
	}

	int gleft=pc_currchar->CountGold();
	if (gleft<pi->value)
	{
		npctalk(s, pc, "You cannot afford that.",0);
		return;
	}
	pBackpack->DeleteAmount(price,0x0EED);	// take gold from player

	npctalk(s, pc, "Thank you.",0);
	pc->setHoldg(pc->holdg() + pi->value); // putting the gold to the vendor's "pocket"

	// sends item to the proud new owner's pack
	pi->setContSerial(pBackpack->serial);
	RefreshItem(pi);

}

////////////////
// name:		triggertarget
// history:		by Magius(CHE),24 August 1999
// Purpose:		Select an item or an npc to set with new trigger.
//
void cTargets::triggertarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)//Char
	{
			//triggerwitem(i,-1,1); is this used also for npcs?!?!
	} 
	else
	{//item
		P_ITEM pi = FindItemBySerial(serial);
		if(pi != NULL)
		{
			Trig->triggerwitem(s, pi,1);
		}
	}
}

void cTargets::BanTarg(int s)
{

}

////////////////
// name:		addtarget
// history:		UnKnown (Touched tabstops by Tauriel Dec 29, 1998)
// Purpose:		Adds an item when using /add # # .
//
static void AddTarget(int s, PKGx6C *pp)
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;

	if (addid1[s]>=0x40) // LB 25-dec-199, changed == to >= for chest multis, probably not really necassairy
	{
		switch (addid2[s])
		{
		case 100:
		case 102:
		case 104:
		case 106:
		case 108:
		case 110:
		case 112:
		case 114:
		case 116:
		case 118:
		case 120:
		case 122:
		case 124:
		case 126:
		case 140:
			BuildHouse(s,addid3[s]);//If its a valid house, send it to buildhouse!
			return; // Morrolan, here we WANT fall-thru, don't mess with this switch
		}
	}
	bool pileable = false;
	short id=(addid1[s]<<8)+addid2[s];
	tile_st tile;
	Map->SeekTile(id, &tile);
	if (tile.flag2&0x08) pileable=true;

	P_ITEM pi = Items->SpawnItem(currchar[s], 1, "#", pileable, id, 0,0);
	if(!pi) return;
	pi->priv=0;	//Make them not decay
	pi->MoveTo(pp->TxLoc,pp->TyLoc,pp->TzLoc+Map->TileHeight(pp->model));

	RefreshItem(pi);
	addid1[s]=0;
	addid2[s]=0;
}

class cRenameTarget : public cWpObjTarget
{
public:
	cRenameTarget(P_CLIENT pCli) : cWpObjTarget(pCli), cItemTarget(pCli), cCharTarget(pCli), cTarget(pCli) {}
	void CharSpecific()
	{
		pc->name = xtext[s];
	}
	void ItemSpecific()
	{
		if(addx[s]==1) //rename2 //New -- Zippy
			pi->setName2( xtext[s] );
		else
			pi->setName( xtext[s] );
	}
};

static void TeleTarget(int s, PKGx6C *pp) 
{ 
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return; 
	P_CHAR pc_currchar = currchar[s]; 
	int x=pp->TxLoc; 
	int y=pp->TyLoc; 
	signed char z=pp->TzLoc; 
	Coord_cl clTemp3(x,y,z) ;
	if ((pc_currchar->isGM()) || (line_of_sight( s, pc_currchar->pos, clTemp3,WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING))) 
	{ 
		doGmMoveEff(s); 
		
		pc_currchar->MoveTo(x,y,z+Map->TileHeight(pp->model)); 
		teleport(pc_currchar); 
		
		doGmMoveEff(s); 
	} 
}

class cRemoveTarget : public cWpObjTarget
{
public:
	cRemoveTarget(P_CLIENT pCli) : cWpObjTarget(pCli), cItemTarget(pCli), cCharTarget(pCli), cTarget(pCli) {}
	void CharSpecific()
	{
		if (pc->account() > -1 && pc->isPlayer()) // player check added by LB
		{
			sysmessage(s,"You cant delete players");
			return;
		}
		sysmessage(s, "Removing character.");
		Npcs->DeleteChar( pc );
	}
	void ItemSpecific()
	{
		sysmessage(s, "Removing item.");
		Items->DeleItem(pi);
	}
};

void DyeTarget(int s)
{
	int body,c1,c2,b,k;

	int serial=LongFromCharPtr(buffer[s]+7);
	if ((addid1[s]==255)&&(addid2[s]==255))
	{
		P_ITEM pi=FindItemBySerial(serial);
		if (pi!=NULL)
		{
			SndDyevat(s,pi->serial, pi->id());
			RefreshItem(pi);
		}
		P_CHAR pc = FindCharBySerial(serial);
		if (pc != NULL)
		{
			SndDyevat(s,pc->serial,0x2106);
		}
	}
	else
	{
		P_ITEM pi=FindItemBySerial(serial);
		if (pi!=NULL)
		{
			c1=addid1[s]; // lord binary, dying crash bugfix
			c2=addid2[s];

			if (!dyeall[s])
			{
				if ((((c1<<8)+c2)<0x0002) ||
					(((c1<<8)+c2)>0x03E9) )
				{
					c1=0x03;
					c2=0xE9;
				}
			}

			b=((((c1<<8)+c2)&0x4000)>>14)+((((c1<<8)+c2)&0x8000)>>15);
			if (!b)
			{
				pi->setColor( static_cast<unsigned short>(c1 << 8) + c2 );
			}
			RefreshItem(pi);
		}

		P_CHAR pc = FindCharBySerial(serial);
		if (pc != NULL)
		{
			body=(pc->id1<<8)+pc->id2;
			k=(addid1[s]<<8)+addid2[s];
			if( ( (k>>8) < 0x80 ) && body >= 0x0190 && body <= 0x0193 )
				k += 0x8000;

			b=k&0x4000;
			if (b==16384 && (body >=0x0190 && body<=0x03e1))
				k=0xf000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

			if (k!=0x8000) // 0x8000 also crashes client ...
			{
				pc->setSkin(k);
				pc->setXSkin(k);
				updatechar(pc);
			}
		}
	}
}

class cNewzTarget : public cWpObjTarget
{
public:
	cNewzTarget(P_CLIENT pCli) : cWpObjTarget(pCli), cItemTarget(pCli), cCharTarget(pCli), cTarget(pCli) {}
	void CharSpecific()
	{
		pc->dispz=pc->pos.z=addx[s];
		teleport(pc);
	}
	void ItemSpecific()
	{
		pi->pos.z=addx[s];
		RefreshItem(pi);
	}
};

//public !!
void cTargets::IDtarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	if (isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;

		pi->setId( ( addx[s] << 8 ) + addy[s]  );
		RefreshItem(pi);
		return;
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		pc->id1=addx[s];
		pc->id2=addy[s];
		pc->xid = pc->id();
		updatechar(pc);
	}
}

//public !!
void cTargets::XTeleport(int s, int x)
{
	SERIAL serial = INVALID_SERIAL;
	P_CHAR pc_currchar = currchar[s];
	P_CHAR pc = NULL;
	switch (x)
	{
		case 0:
			serial = LongFromCharPtr(buffer[s] + 7);
			pc = FindCharBySerial(serial);
			break;
		case 2:
			if (perm[makenumber(1)])
			{
				pc = currchar[makenumber(1)];
			}
			else 
				return;
			break;
		case 3:
			{
				UOXSOCKET s = pc_currchar->making;
				currchar[s]->moveTo(pc_currchar->pos);
				teleport(currchar[s]);
			}
			return;
		case 5:
			serial = calcserial(hexnumber(1), hexnumber(2), hexnumber(3), hexnumber(4));
			pc = FindCharBySerial(serial);
			break;
	}
	
	if (pc != NULL)
	{
		pc->moveTo(pc_currchar->pos);
		updatechar(pc);
		return;// Zippy
	}
	P_ITEM pi = FindItemBySerial(serial);
	if (pi != NULL)
	{
		pi->moveTo(pc_currchar->pos);
		RefreshItem(pi);
	}
}

void XgoTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		Coord_cl pos(pc->pos);
		pos.x = addx[s];
		pos.y = addy[s];
		pos.z = addz[s];
		pc->moveTo(pos);
		updatechar(pc);
	}
}

static void PrivTarget(int s, P_CHAR pc)
{
	if (SrvParams->gmLog())	//Logging
	{
		sprintf((char*)temp, "%s.gm_log", currchar[s]->name.c_str());
		sprintf((char*)temp2, "%s as given %s Priv [%x][%x]\n", currchar[s]->name.c_str(), pc->name.c_str(), addid1[s],addid2[s]);
		savelog((char*)temp2, (char*)temp);
	}
	pc->setPriv(addid1[s]);
	pc->priv2=addid2[s];
}

static void KeyTarget(int s, P_ITEM pi) // new keytarget by Morollan
{
	if (pi)
	{
		if ((pi->more1==0)&&(pi->more2==0)&&
			(pi->more3==0)&&(pi->more4==0))
		{
			if ( pi->type() == 7 && (iteminrange(s,pi,2) || (!pi->isInWorld()) ) )
			{
				if (!Skills->CheckSkill(currchar[s], TINKERING, 400, 1000))
				{
					sysmessage(s,"You fail and destroy the key blank.");
					Items->DeleItem(pi);
				}
				else
				{
					pi->more1=addid1[s];
					pi->more2=addid2[s];
					pi->more3=addid3[s];
					pi->more4=addid4[s];
					sysmessage(s, "You copy the key."); //Morrolan can copy keys
				}
			}
			return;
		}//if
		else if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
			(pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
			(addid1[s]==(unsigned char)'\xFF'))
		{
			if (((pi->type()==1)||(pi->type()==63))&&(iteminrange(s,pi,2)))
			{
				if(pi->type()==1) pi->setType( 8 );
				if(pi->type()==63) pi->setType( 64 );
				sysmessage(s, "You lock the container.");
				return;
			}
			else if ((pi->type()==7)&&(iteminrange(s,pi,2)))
			{
				currchar[s]->inputitem = pi->serial;
				currchar[s]->inputmode = cChar::enDescription;
				sysmessage(s,"Enter new name for key.");//morrolan rename keys
				return;
			}
			else if ((pi->type()==8)||(pi->type()==64)&&(iteminrange(s,pi,2)))
			{
				if(pi->type()==8) pi->setType( 1 );
				if(pi->type()==64) pi->setType( 63 );
				sysmessage(s, "You unlock the container.");
				return;
			}
			else if ((pi->type()==12)&&(iteminrange(s,pi,2)))
			{
				pi->setType( 13 );
				sysmessage(s, "You lock the door.");
				return;
			}
			else if ((pi->type()==13)&&(iteminrange(s,pi,2)))
			{
				pi->setType( 12 );
				sysmessage(s, "You unlock the door.");
				return;
			}
			else if (pi->id()==0x0BD2)
			{
				sysmessage(s, "What do you wish the sign to say?");
				currchar[s]->inputitem = pi->serial; //Morrolan sign kludge
				currchar[s]->inputmode=cChar::enHouseSign;
				return;
			}

			//Boats ->
			else if( pi->type() == 117 && pi->type2() == 3 )
			{
				Boats->OpenPlank(pi);
				RefreshItem(pi);
			}
			//End Boats --^
		}//else if
		else
		{
			if (pi->type()==7) sysmessage (s, "That key is not blank!");
			else if (pi->more1=='\x00') sysmessage(s, "That does not have a lock.");
			else sysmessage(s, "The key does not fit into that lock.");
			return;
		}//else
		return;
	}//if
}//keytarget()

void cTargets::IstatsTarget(int s)
{
	if ((buffer[s][7]==0)&&(buffer[s][8]==0)&&(buffer[s][9]==0)&&(buffer[s][10]==0))
	{
		tile_st tile;
		Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
		sprintf((char*)temp, "Item [Static] ID [%x %x]",buffer[s][0x11], buffer[s][0x12]);
		sysmessage(s, (char*)temp);
		sprintf((char*)temp, "ID2 [%i], Height [%i]",((buffer[s][0x11]<<8)+buffer[s][0x12]), tile.height);
		sysmessage(s, (char*)temp);
	}
	else
	{
		int serial=LongFromCharPtr(buffer[s]+7);
		PC_ITEM pi=FindItemBySerial(serial);
		if (pi!=NULL)
		{
			// Modified by Magius(CHE)
			char contstr[20];
			int co=pi->contserial;
			sprintf(contstr,"[%x %x %x %x]",
				(co&0xFF000000)>>24,
				(co&0x00FF0000)>>16,
				(co&0x0000FF00)>>8,
				(co&0x000000FF) );
			sprintf((char*)temp, "Item [Dynamic] Ser [%8x] ID [%4x] Name [%s] Name2 [%s] Color [%4x] Cont %s Layer [%x] Type [%d] Magic [%x] More [%x %x %x %x] Position [%i %i %i] Amount [%i] Priv [%x]",
				pi->serial, pi->id(),
				pi->name().ascii(),pi->name2().ascii(),pi->color(),
				contstr,
				pi->layer(),pi->type(),pi->magic,
				pi->more1,pi->more2,pi->more3,pi->more4,
				pi->pos.x,pi->pos.y,pi->pos.z,pi->amount(), pi->priv);
			sysmessage(s, (char*)temp);
			sprintf((char*)temp,"STR [%d] HP/MAX [%d/%d] Damage [%d-%d] Defence [%d] Rank [%d] Smelt [%d] SecureIt [%d] MoreXYZ [%i %i %i] Poisoned [%i] RaceHate [%i] Weight [%d] Owner [%x] Creator [%s] MadeValue [%i] Value [%i] Decaytime[%i] Decay [%i] GoodType[%i] RandomValueRate[%i]",
				pi->st, pi->hp(),pi->maxhp(), pi->lodamage(), pi->hidamage(),pi->def,pi->rank,pi->smelt(),( pi->secured() ) ? 1 : 0,
				pi->morex, pi->morey, pi->morez,pi->poisoned,pi->racehate(),
				pi->weight(), pi->ownserial, // Ison 2-20-99
				pi->creator.c_str(),pi->madewith,pi->value,int(double(int(pi->decaytime-uiCurrentTime)/MY_CLOCKS_PER_SEC)),(pi->priv)&0x01,pi->good,pi->rndvaluerate); // Magius(CHE) (2)
				sysmessage(s,(char*)temp); // Ison 2-20-99
			// End Modified lines
		}
	}
}
void cTargets::TargIdTarget(int s) // Fraz
{
	const P_ITEM pi = FindItemBySerPtr(buffer[s] + 7);
	if (pi)
	{
		if (buffer[s][7] >= 0x40)
		{
			if (pi && !pi->isLockedDown())
			{
				if (pi->name2() != "#")
					pi->setName( pi->name2() );
				if (pi->name() == "#")
					pi->getName(temp2);
				else 
					strcpy((char*)temp2, pi->name().ascii() );
				sprintf((char*)temp, "You found that this item appears to be called: %s", temp2);
				sysmessage(s, (char*) temp);
			}
		}			
		if (pi->type() != 15)
		{
			if (pi->type() != 404)
			{
				sysmessage(s, "This item has no hidden magical properties.");
			}
			else
			{
				sprintf((char*)temp, "It is enchanted with item identification, and has %d charges remaining.", pi->morex);
				sysmessage(s, (char*)temp);
			}
		}
		else
		{
			sprintf((char*)temp, "It is enchanted with the spell %s, and has %d charges remaining.", spellname[(8*(pi->morex - 1)) + pi->morey - 1], pi->morez);
			sysmessage(s, (char*)temp);
		}
	}
}
static void CstatsTarget(P_CLIENT ps, P_CHAR pc)
{
	UOXSOCKET s = ps->GetSocket();

	sprintf((char*)temp, "Ser [%8x] ID [%2x] Name [%s] Skin [%x] Account [%x] Priv [%x %x] Position [%i %i %i] CTimeout [%i] Fame [%i] Karma [%i] Deaths [%i] Kills [%i] NPCAI [%x] NPCWANDER [%d] WEIGHT [%.2f]",
		pc->serial,pc->id(),
		pc->name.c_str(),pc->skin(),
		pc->account(),pc->getPriv(),pc->priv2,
		pc->pos.x,pc->pos.y,pc->pos.z, pc->timeout,
		pc->fame,pc->karma,pc->deaths,pc->kills,
		pc->npcaitype(), pc->npcWander, (float)pc->weight);
	sysmessage(s, (char*)temp);
	sprintf((char*)temp, "Other Info: Poisoned [%i] Poison [%i] Hunger [%i] Attacker Serial [%x] Target Serial [%x] Carve[%i]", //Changed by Magius(CHE)
		pc->poisoned(),pc->poison(),pc->hunger(),pc->attacker,pc->targ,pc->carve()); //Changed by Magius(CHE)
	sysmessage(s, (char*)temp);
	Gumps->Open(s, pc, 0, 8);
	statwindow(s, pc);
}

static void MoveBelongingsToBp(P_CHAR pc, P_CHAR pc_c)
{
	P_ITEM pPack = Packitem(pc);
	if (pPack == NULL)
	{
		pPack = Items->SpawnItem(calcSocketFromChar(pc_c),pc_c,1,"#",0,0x0E,0x75,0,0,0);
		if (pPack == NULL)
			return;
		pc->packitem = pPack->serial; 
		pPack->setContSerial(pc_c->serial);
		pPack->setLayer( 0x15 );
		pPack->setType( 1 );
		pPack->dye=1;
	}

	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer() != 0x15 && pi->layer() != 0x1D &&
			pi->layer() != 0x10 && pi->layer() != 0x0B && (!pi->free))
		{
			if ((pi->trigon==1) && (pi->trigtype==2) && (pi->layer()<19))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
			{
				Trig->triggerwitem(calcSocketFromChar(pc_c), pi, 1); // trigger is fired
			}
			pi->pos.x=(rand()%80)+50;
			pi->pos.y=(rand()%80)+50;
			pi->pos.z=9;
			pi->setContSerial(pPack->serial);
			pi->setLayer( 0x00 );
			SndRemoveitem(pi->serial);
			RefreshItem(pi);
		}
		else if (pc->Wears(pi) &&
			(pi->layer()==0x0B || pi->layer()==0x10))	// hair & beard (Duke)
		{
			Items->DeleItem(pi);
		}
	}
	updatechar(pc_c);
}

static void GMTarget(P_CLIENT ps, P_CHAR pc)
{
	UOXSOCKET s = ps->GetSocket();

	int i;	
	if (SrvParams->gmLog())
	{
		sprintf((char*)temp, "%s.gm_log",currchar[s]->name.c_str());
		sprintf((char*)temp2, "%s has made %s a GM.\n",currchar[s]->name.c_str(),pc->name.c_str());
		savelog((char*)temp2, (char*)temp);
	}
	UOXSOCKET targSocket = calcSocketFromChar(pc);
	if ( targSocket == -1 )
		return;
	unmounthorse(targSocket);	//AntiChrist bugfix
	
	pc->setId(0x03DB);
	pc->setSkin(0x8021);
	pc->xid = 0x03DB;
	pc->setXSkin(0x8021);
	pc->setPriv(0xF7);
	pc->priv2 = (unsigned char) (0xD9);
	pc->setGmRestrict(0); // By default, let's not restrict them.
	
	if (pc->account() == 0) 
		pc->setPrivLvl("admin");
	else
		pc->setPrivLvl("gm"); // gm default
	pc->setMenupriv(-1); // LB, disabling menupriv stuff for gms per default
	
	for (i = 0; i < TRUESKILLS; i++)
	{
		pc->setBaseSkill(i,1000);
		pc->setSkill(i, 1000);
	}
	
	// All stats to 100
	pc->st  = 100;
	pc->st2 = 100;
	pc->hp  = 100;
	pc->stm = 100;
	pc->in  = 100;
	pc->in2 = 100;
	pc->mn  = 100;
	pc->mn2 = 100;
	pc->setDex(100);
	
	if (strncmp(pc->name.c_str(), "GM", 2))
	{
		sprintf((char*)temp, "GM %s", pc->name.c_str());
		pc->name = (char*)temp;
	}
	MoveBelongingsToBp(pc, pc);
}

static void CnsTarget(P_CLIENT ps, P_CHAR pc)
{
	UOXSOCKET s = ps->GetSocket();

	if (SrvParams->gmLog())
	{
		// logging
		sprintf((char*)temp, "%s.gm_log",currchar[s]->name.c_str());
		sprintf((char*)temp2, "%s has made %s a Counselor.\n",currchar[s]->name.c_str(),pc->name.c_str());
		savelog((char*)temp2, (char*)temp);
	}
	pc->setId(0x03DB);
	pc->setSkin(0x8003);
	pc->xid=0x03DB;
	pc->setXSkin(0x8002);
	pc->setPriv(0xB6);
	pc->priv2='\x8D';
	if (strncmp(pc->name.c_str(), "Counselor", 9))
	{
		sprintf((char*)temp, "Counselor %s", pc->name.c_str());
		pc->name  = (char*)temp;
	}
	pc->setPrivLvl("counselor"); // counselor defaults
	if (pc->account()==0) pc->setPrivLvl("admin");
	MoveBelongingsToBp(pc, pc);
}

static void KillTarget(P_CHAR pc, int ly)
{
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer()==ly)
		{
			Items->DeleItem(pi);
		}
	}
}

void cTargets::GhostTarget(int s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if(pc != NULL)
	{
		if(!pc->dead)
		{
			P_CHAR pc_currchar = currchar[s];
			pc->attacker=pc_currchar->serial; //AntiChrist -- for forensics ev
			bolteffect(pc, true);
			soundeffect2(pc, 0x0029);
			deathstuff(pc);
		}
		else
			sysmessage(s,"That player is already dead.");
	}
}

class cBoltTarget : public cCharTarget
{
public:
	cBoltTarget(P_CLIENT pCli) : cCharTarget(pCli), cTarget(pCli) {}
	void CharSpecific()
	{
		if (w_anim[0]==0 && w_anim[1]==0)
		{
			bolteffect(pc, true);
			soundeffect2(pc, 0x0029);
		}
		else
		{
			for (int j=0;j<=333;j++) 
				bolteffect2(pc, w_anim[0],w_anim[1]);
		}
	}
};

class cSetAmountTarget : public cItemTarget
{
public:
	cSetAmountTarget(P_CLIENT pCli) : cItemTarget(pCli), cTarget(pCli) {}
	void ItemSpecific()
	{
		if (addx[s] > 64000) //Ripper..to fix a client bug for over 64k.
		{
			sysmessage(s, "No amounts over 64k in a pile!");
			return;
		}
		this->pi->setAmount( addx[s] );
		RefreshItem(pi);
	}
};

void cTargets::CloseTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if(pc != NULL)
	{
		UOXSOCKET j = calcSocketFromChar(pc);
		if(j>-1)
		{
			sysmessage(s, "Kicking player");
			sysmessage(j, "You have been kicked!"); //New -- Zippy
			Network->Disconnect(j);
		}
	}
}

// public !!!
P_ITEM cTargets::AddMenuTarget(int s, int x, int addmitem) //Tauriel 11-22-98 updated for new items
{
	if (s>=0)
		if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return NULL;

	P_ITEM pi = Items->createScriptItem(s, QString("%1").arg(addmitem), 0);
	if (pi == NULL) return NULL;
	if (x)
		RefreshItem(pi);
	return pi;
}

// public !!!
P_CHAR cTargets::NpcMenuTarget(int s)
{
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return NULL;
	return Npcs->createScriptNpc(s, NULL, QString("%1").arg(addmitem[s]));
}

void cTargets::VisibleTarget (int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))//item
	{
		P_ITEM pi = FindItemBySerial(serial);
		if(pi != NULL)
		{
			pi->visible=addx[s];
			RefreshItem(pi);
		}
	}
	else
	{//char
		P_CHAR pc = FindCharBySerial(serial);
		if(pc != NULL)
		{
			pc->setHidden( addx[s] );
			updatechar(pc);
		}
	}
}

///////////////
// Name:	ContainerEmptyTarget1&2
// Purpose:	used by the '#empty' user-command
// history: by Duke, 29.12.2000
//
static void ContainerEmptyTarget1(P_CLIENT ps, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	if (isItemSerial(pi->serial) && pi->type() == 1)
	{
		addx[s]=pi->serial;
		target(s, 0, 1, 0, 72, "Select container to fill:");
	}
	else
		sysmessage(s,"That is not a valid container!");
}
static void ContainerEmptyTarget2(P_CLIENT ps, P_ITEM pNewCont)
{
	UOXSOCKET s = ps->GetSocket();
	if( pNewCont->type() == 1 )
	{
		P_ITEM pi;	// item to move from old container
		unsigned int ci = 0;
		vector<SERIAL> vecContainer = contsp.getData(addx[s]);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pi = FindItemBySerial(vecContainer[ci]);
			pNewCont->AddItem(pi);
		}
	}
	else
		sysmessage(s,"That is not a valid container!");
}

static void OwnerTarget(P_CLIENT ps, P_CHAR pc)
{
	UOXSOCKET s = ps->GetSocket();
	
	int addser=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	pc->SetOwnSerial(addser);
	if (addser==-1)
	{
		pc->setTamed(false);
	}
	else
	{
		pc->setTamed(true);
	}
}

static void OwnerTarget(P_CLIENT ps, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	
	int os=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	pi->SetOwnSerial(os);

	if (pi->visible==1)
	{
		for (int j=0;j<now;j++) if (perm[j]) senditem(j,pi); // necassairy for items with visible value of 1
	}
}

void cTargets::DvatTarget(int s)
{
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && pi->dye==1)//if dyeable
	{
		P_CHAR pc = GetPackOwner(pi);
		if(pc == currchar[s] || pi->isInWorld())
		{//if on ground or currchar is owning the item - AntiChrist
			pi->setColor( static_cast<unsigned short>(addid1[s]<<8) + addid2[s] );
			RefreshItem(pi);
			soundeffect(s,0x02,0x3e); // plays the dye sound, LB
		} else
		{
			sysmessage(s,"That is not yours!!");
		}
	}
	else
	{
		sysmessage(s, "You can only dye cloth with this.");
	}
}

static void AddNpcTarget(int s, PKGx6C *pp)
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	P_CHAR pc = Npcs->MemCharFree ();
	if ( pc == NULL )
		return;
	pc->Init();
	pc->name = "Dummy";
	pc->id1=addid1[s];
	pc->id2=addid2[s];
	pc->xid = pc->id();
	pc->setSkin(0);
	pc->setXSkin(0);
	pc->setPriv(0x10);
	pc->pos.x=pp->TxLoc;
	pc->pos.y=pp->TyLoc;
	pc->dispz=pc->pos.z=pp->TzLoc+Map->TileHeight(pp->model);
	mapRegions->Add(pc); // add it to da regions ...
	pc->isNpc();
	updatechar(pc);
}

void cTargets::AllSetTarget(int s)
{
	SERIAL serial = LongFromCharPtr( buffer[ s ] + 7 );
	QString commandLine( SocketStrings[ s ].c_str() );
	
	QString commandStr;
	QString parameterStr;

	if( commandLine.find( " " ) == -1 )
		commandStr = commandLine;
	else
	{
		commandStr = commandLine.left( commandLine.find( " " ) );
		parameterStr = commandLine.right( commandLine.length() - ( commandLine.find( " " ) + 1 ) );
	}

	commandStr = commandStr.lower();

	// Events
	if( commandStr == "events" )
	{
		QStringList events = QStringList::split( ",", parameterStr );

		cUObject *Object = NULL;

		if( serial > 0x40000000 )
			Object = FindItemBySerial( serial );
		else
			Object = FindCharBySerial( serial );

		if( Object == NULL )
			return;

		for( UI08 i = 0; i < events.count(); i++ )
		{
			QString eventName = events[ i ];

			WPDefaultScript *scriptObj = ScriptManager->find( eventName );

			// No event with that name found
			if( scriptObj == NULL )
			{
				sysmessage( s, "Could not find event: " + eventName );
				continue;
			}

			Object->addEvent( scriptObj );
		}
	}

	/*int j;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if(pc != NULL)
	{
		UOXSOCKET k = calcSocketFromChar(pc);
		if (addx[s]<TRUESKILLS)
		{
			pc->baseskill[addx[s]]=addy[s];
			Skills->updateSkillLevel(pc, addx[s]);
			if (k!=-1) updateskill(k, addx[s]);
		}
		else if (addx[s]==ALLSKILLS)
		{
			for (j=0;j<TRUESKILLS;j++)
			{
				pc->baseskill[j]=addy[s];
				Skills->updateSkillLevel(pc, j);
				if (k!=-1) updateskill(k,j);
			}
		}
		else if (addx[s]==STR)
		{
			pc->st=addy[s];
			for (j=0;j<TRUESKILLS;j++)
			{
				Skills->updateSkillLevel(pc,j);
				if (k!=-1) updateskill(k,j);
			}
			if (k!=-1) 
				statwindow(k, pc);
		}
		else if (addx[s]==DEX)
		{
			pc->setDex(addy[s]);
			for (j=0;j<TRUESKILLS;j++)
			{
				Skills->updateSkillLevel(pc,j);
				if (k!=-1) updateskill(k,j);
			}
			if (k!=-1) 
				statwindow(k, pc);
		}
		else if (addx[s]==INT)
		{
			pc->in=addy[s];
			for (j=0;j<TRUESKILLS;j++)
			{
				Skills->updateSkillLevel(pc,j);
				if (k!=-1) updateskill(k,j);
			}
			if (k!=-1) 
				statwindow(k, pc);
		}
		else if (addx[s]==FAME)
		{
			pc->fame=addy[s];
		}
		else if (addx[s]==KARMA)
		{
			pc->karma=addy[s];
		}
	}*/
}

static void InfoTarget(int s, PKGx6C *pp) // rewritten to work also with map-tiles, not only static ones by LB
{
	int tilenum,x1,y1,x2,y2,x,y;
	signed char z;
	unsigned long int pos;
	tile_st tile;
	map_st map1;
	land_st land;

	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	x=pp->TxLoc;
	y=pp->TyLoc;
	z=pp->TzLoc;

	if (pp->model==0)	// damn osi not me why the tilenum is only send for static tiles, LB
	{	// manually calculating the ID's if it's a maptype
		x1=x/8;
		y1=y/8;
		x2=(x-(x1*8));
		y2=(y-(y1*8));
		pos=(x1*512*196)+(y1*196)+(y2*24)+(x2*3)+4;
		fseek(mapfile, pos, SEEK_SET);
		fread(&map1, 3, 1, mapfile);
		Map->SeekLand(map1.id, &land);
		clConsole.send("type: map-tile\n");
		clConsole.send("tilenum: %i\n",map1.id);
		clConsole.send("Flag1:%x\n", land.flag1);
		clConsole.send("Flag2:%x\n", land.flag2);
		clConsole.send("Flag3:%x\n", land.flag3);
		clConsole.send("Flag4:%x\n", land.flag4);
		clConsole.send("Unknown1:%lx\n", land.unknown1);
		clConsole.send("Unknown2:%x\n", land.unknown2);
		clConsole.send("Name:%s\n", land.name);
	}
	else
	{
		tilenum=pp->model; // lb, bugfix
		Map->SeekTile(tilenum, &tile);
		clConsole.send("type: static-tile\n");
		clConsole.send("tilenum: %i\n",tilenum);
		clConsole.send("Flag1:%x\n", tile.flag1);
		clConsole.send("Flag2:%x\n", tile.flag2);
		clConsole.send("Flag3:%x\n", tile.flag3);
		clConsole.send("Flag4:%x\n", tile.flag4);
		clConsole.send("Weight:%x\n", tile.weight);
		clConsole.send("Layer:%x\n", tile.layer);
		clConsole.send("Anim:%lx\n", tile.animation);
		clConsole.send("Unknown1:%lx\n", tile.unknown1);
		clConsole.send("Unknown2:%x\n", tile.unknown2);
		clConsole.send("Unknown3:%x\n", tile.unknown3);
		clConsole.send("Height:%x\n", tile.height);
		clConsole.send("Name:%s\n", tile.name);
	}
	sysmessage(s, "Item info has been dumped to the console.");
	clConsole.send("\n");
}

void cTargets::TweakTarget(int s)//Lag fix -- Zippy
{
	int serial=LongFromCharPtr(buffer[s]+7);
	if (serial == 0) //Client sends zero if invalid!
		return;
	if (isCharSerial(serial))//Char
	{
		if (FindCharBySerial(serial) == NULL) // but let's make sure
			return;
		tweakmenu(s, serial);
	} 
	else
	{//item
		if (FindItemBySerial(serial) == NULL)
			return;
		tweakmenu(s, serial);
	}
}

static void SetInvulFlag(P_CLIENT ps, P_CHAR pc)
{
	if (addx[ps->GetSocket()]==1)
		pc->makeInvulnerable();
	else
		pc->makeVulnerable();
}

static void Tiling(int s, PKGx6C *pp) // Clicking the corners of tiling calls this function - Crwth 01/11/1999
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	if (clickx[s]==-1 && clicky[s]==-1)
	{
		clickx[s]=pp->TxLoc;
		clicky[s]=pp->TyLoc;
		target(s,0,1,0,198, "Select second corner of bounding box.");
		return;
	}

	int c;
	bool pileable = false;
	tile_st tile;
	int x1=clickx[s],x2=pp->TxLoc;
	int y1=clicky[s],y2=pp->TyLoc;

	clickx[s]=-1;clicky[s]=-1;

	if (x1>x2) {c=x1;x1=x2;x2=c;}
	if (y1>y2) {c=y1;y1=y2;y2=c;}

	if (addid1[s]==0x40)
	{
		switch (addid2[s])
		{
		case 100:
		case 102:
		case 104:
		case 106:
		case 108:
		case 110:
		case 112:
		case 114:
		case 116:
		case 118:
		case 120:
		case 122:
		case 124:
		case 126:
		case 140:
			AddTarget(s,pp);
			return;
		}
	}

	int x,y;
	short id=(addid1[s]<<8)+addid2[s];
	Map->SeekTile(id, &tile);
	if (tile.flag2&0x08) pileable = true;
	for (x=x1;x<=x2;x++)
		for (y=y1;y<=y2;y++)
		{
			P_ITEM pi = Items->SpawnItem(currchar[s], 1, "#", pileable, id, 0, 0);
			if(!pi) return;
			pi->priv=0;	//Make them not decay
			pi->MoveTo(x,y,pp->TzLoc+Map->TileHeight(pp->model));
			RefreshItem(pi);
		}

	addid1[s]=0;
	addid2[s]=0;
}

//public !!
void cTargets::Wiping(int s) // Clicking the corners of wiping calls this function - Crwth 01/11/1999
{
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return;

	if (clickx[s]==-1 && clicky[s]==-1) {
		clickx[s]=(buffer[s][11]<<8)+buffer[s][12];
		clicky[s]=(buffer[s][13]<<8)+buffer[s][14];
		if (addid1[s]) target(s,0,1,0,199,"Select second corner of inverse wiping box.");
		else target(s,0,1,0,199,"Select second corner of wiping box.");
		return;
	}

	int x1=clickx[s],x2=(buffer[s][11]<<8)+buffer[s][12];
	int y1=clicky[s],y2=(buffer[s][13]<<8)+buffer[s][14];

	clickx[s]=-1;clicky[s]=-1;

	int c;
	if (x1>x2) {c=x1;x1=x2;x2=c;}
	if (y1>y2) {c=y1;y1=y2;y2=c;}
	if (addid1[s]==1)
	{ // addid1[s]==1 means to inverse wipe
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{
			P_ITEM pi=iterItems.GetData();
			if (!(pi->pos.x>=x1 && pi->pos.x<=x2 && pi->pos.y>=y1 && pi->pos.y<=y2) && pi->isInWorld() && !pi->wipe())
			{
				iterItems--;
				Items->DeleItem(pi);
			}
		}
	}
	else
	{
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{
			P_ITEM pi=iterItems.GetData();
			if (pi->pos.x>=x1 && pi->pos.x<=x2 && pi->pos.y>=y1 && pi->pos.y<=y2 && pi->isInWorld() && !pi->wipe())
			{
				iterItems--;
				Items->DeleItem(pi);
			}
		}
	}
}

static void ExpPotionTarget(int s, PKGx6C *pp) //Throws the potion and places it (unmovable) at that spot
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	P_CHAR pc_currchar = currchar[s];

	// ANTICHRIST -- CHECKS LINE OF SIGHT!
	Coord_cl clTemp4(pc_currchar->pos);
	clTemp4.x = pp->TxLoc;
	clTemp4.y = pp->TyLoc;
	clTemp4.z = pp->TzLoc;
	if(line_of_sight(s, pc_currchar->pos, clTemp4, WALLS_CHIMNEYS + DOORS + ROOFING_SLANTED))
	{
		P_ITEM pi = FindItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
		if (pi != NULL) // crashfix LB
		{
			pi->moveTo( clTemp4 );
			pi->setContSerial(INVALID_SERIAL);
			pi->setGMMovable(); //make item unmovable once thrown
			movingeffect2(pc_currchar, pi, 0x0F, 0x0D, 0x11, 0x00, 0x00);
			RefreshItem(pi);
		}
	}
	else 
		sysmessage(s,"You cannot throw the potion there!");
}

void cTargets::SquelchTarg(int s)//Squelch
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if(pc->isGM())
		{
			sysmessage(s, "You cannot squelch GMs.");
			return;
		}
		if (pc->squelched())
		{
			pc->setSquelched(0);
			sysmessage(s, "Un-squelching...");
			sysmessage(calcSocketFromChar(pc), "You have been unsquelched!");
			pc->setMutetime(0);
		}
		else
		{
			pc->setMutetime(-1);
			pc->setSquelched(1);
			sysmessage(s, "Squelching...");
			sysmessage(calcSocketFromChar(pc), "You have been squelched!");
			
			if (addid1[s]!=255 || addid1[s]!=0)
		
			{
				pc->setMutetime((unsigned int) (uiCurrentTime+(addid1[s]*MY_CLOCKS_PER_SEC)));
				
				addid1[s]=255;
			
				pc->setSquelched(2);
			}
		}
	}
}


static void TeleStuff(int s, PKGx6C *pp)
{
	sysmessage(s, "Command temporary disabled for code restructure");
/*
	static int targ=-1;//What/who to tele
	int x, y, z;
	int serial, i;
	if (targ==-1)
	{
		serial=LongFromCharPtr(buffer[s]+7);
		targ=calcCharFromSer(serial);

		if(targ!=-1)
		{
			targ+=1000000;
			target(s,0,1,0,222,"Select location to put this character.");
		} else {
			targ=calcItemFromSer(serial);
			if(targ!=-1)
				target(s,0,1,0,222,"Select location to put this item.");
		}
		return;
	} else
	{
		if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
		x=pp->TxLoc;
		y=pp->TyLoc;
		z=pp->TzLoc+Map->TileHeight(pp->model);

		if (targ>999999)//character
		{
			sysmessage(s, "Moving character...");
			i=targ-1000000;
			chars[i].MoveTo(x,y,z);
			teleport(i);
		} else {
			i=targ;
			items[i].MoveTo(x,y,z);
			sysmessage(s, "Moving item...");
			RefreshItem(i);
		}
		targ=-1;
		return;
	}
	*/

}

void CarveTarget(int s, int feat, int ribs, int hides, int fur, int wool, int bird)
{
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pi1 = Items->SpawnItem(pc_currchar, 1, "#", 0, 0x122A, 0, 0);	//add the blood puddle
	P_ITEM pi2=FindItemBySerial(npcshape[0]);
	if(!pi1) return;
	pi1->pos.x=pi2->pos.x;
	pi1->pos.y=pi2->pos.y;
	pi1->pos.z=pi2->pos.z;
	pi1->setGMMovable();//AntiChrist - makes the item unmovable
	pi1->startDecay();
	RefreshItem(pi1);

//	int c;
	if (feat)
	{
		P_ITEM pi = Items->SpawnItem(s, pc_currchar,feat,"feather",1,0x1B,0xD1,0,1,1);
		if(pi == NULL) return;
		sysmessage(s,"You pluck the bird and get some feathers.");
	}
	if (ribs)
	{
		P_ITEM pi = Items->SpawnItem(s,pc_currchar,ribs,"raw rib",1,0x09,0xF1,0,1,1);
		if(pi == NULL) return;
		sysmessage(s,"You carve away some meat.");
	}

	if (hides)
	{
		P_ITEM pi = Items->SpawnItem(s,pc_currchar,hides,"hide",1,0x10,0x78,0,1,1);
		if(pi == NULL) return;
		sysmessage(s,"You skin the corpse and get the hides.");
	}

	if (fur)
	{	// animals with fur now yield hides (OSI). Duke, 7/17/00
		P_ITEM pi = Items->SpawnItem(s, pc_currchar,fur,"hide",1,0x10,0x78,0,1,1);
		if(pi == NULL) return;
		sysmessage(s,"You skin the corpse and get the hides.");
/*		c=Items->SpawnItem(s,fur,"fur",1,0x11,0xFA,0,0,1,1);
		if(c==-1) return;
		sysmessage(s, "You skin the corpse and get some fur.");*/
	}

	if (wool)
	{
		P_ITEM pi = Items->SpawnItem(s, pc_currchar,wool,"unspun wool",1,0x0D,0xF8,0,1,1);
		if(pi == NULL) return;
		sysmessage(s, "You skin the corpse and get some unspun wool.");
	}
	if (bird)
	{
		P_ITEM pi = Items->SpawnItem(s, pc_currchar,bird,"raw bird",1,0x09,0xB9,0,1,1);
		if(pi == NULL) return;
		sysmessage(s, "You carve away some raw bird.");
	}
	Weight->NewCalc(pc_currchar);
}

//AntiChrist - new carving system - 3/11/99
//Human-corpse carving code added
//Scriptable carving product added
static void newCarveTarget(UOXSOCKET s, P_ITEM pi3)
{
	P_CHAR pc_currchar = currchar[s];
	bool deletecorpse=false;
	int storeval;
	char sect[512];
	long int pos;

	P_ITEM pi1 = Items->SpawnItem(pc_currchar, 1, "#", 0, 0x122A, 0, 0);	//add the blood puddle
	P_ITEM pi2 = FindItemBySerial(npcshape[0]);
	if (pi3 == NULL)
		return;
	if(!pi1) return;
	pi1->moveTo(pi2->pos);
	pi1->setGMMovable();//AntiChrist - makes the item unmovable
	pi1->startDecay();
	RefreshItem(pi1);


	if(pi3->morey)	//if it's a human corpse
	{
		pc_currchar->karma-=100; // Ripper..lose karma and criminal.
		sysmessage(s,"You have lost some karma!");
		criminal( pc_currchar );
		//create the Head
		sprintf((char*)temp,"the head of %s",pi3->name2().ascii());
		P_ITEM pi = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xA0,0,0,0);
		if(pi == NULL) return;
		pi->setContSerial(pi3->serial);
		pi->setLayer( 0x01 );
		pi->att=5;

		//AntiChrist & Magius(CHE) - store item's owner, so that lately
		//if we want to know the owner we can do it
		pi->setOwnSerialOnly(pi3->ownserial);

		//create the Body
		sprintf((char*)temp,"the heart of %s",pi3->name2().ascii());
		P_ITEM pi4 = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1C,0xED,0,0,0);
		if(pi4 == NULL) return;
		pi4->setContSerial(pi3->serial);
		pi4->setLayer( 0x01 );
		pi4->att=5;
		pi4->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Heart
		sprintf((char*)temp,"the body of %s",pi3->name2().ascii());
		P_ITEM pi5 = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xAD,0,0,0);
		if(pi5 == NULL) return;
		pi5->setContSerial(pi3->serial);
		pi5->setLayer( 0x01 );
		pi5->att=5;
		pi5->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Left Arm
		sprintf((char*)temp,"the left arm of %s",pi3->name2().ascii());
		P_ITEM pi6 = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xA1,0,0,0);
		if(pi6==NULL) return;
		pi6->setContSerial(pi3->serial);
		pi6->setLayer( 0x01 );
		pi6->att=5;
		pi6->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Right Arm
		sprintf((char*)temp,"the right arm of %s",pi3->name2().ascii());
		P_ITEM pi7 = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xA2,0,0,0);
		if(pi7==NULL) return;//AntiChrist to preview crashes
		pi7->setContSerial(pi3->serial);
		pi7->setLayer( 0x01 );
		pi7->att=5;
		pi7->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Left Leg
		sprintf((char*)temp,"the left leg of %s",pi3->name2().ascii());
		P_ITEM pi8 = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xA3,0,0,0);
		if(pi8 == NULL) return;//AntiChrist to preview crashes
		pi8->setContSerial(pi3->serial);
		pi8->setLayer( 0x01 );
		pi8->att=5;
		pi8->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Rigth Leg
		sprintf((char*)temp,"the right leg of %s",pi3->name2().ascii());
		P_ITEM pi9=Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xA4,0,0,0);
		if(pi9==NULL) return;
		
		pi9->setContSerial(pi3->serial);
		pi9->setLayer( 0x01 );
		pi9->att=5;
		pi9->setOwnSerialOnly(pi3->ownserial);	// see above

		//human: always delete corpse!
		deletecorpse=true;
	} else
	{
		openscript("carve.scp");
		sprintf(sect,"CARVE %i",pi3->carve);
		if (!i_scripts[carve_script]->find(sect))
		{
			closescript();
			return;
		}

		unsigned long loopexit=0;
		do
		{
			read2();
			if (script1[0]!='}')
			{
				if (!(strcmp("ADDITEM",(char*)script1)))
				{
					storeval=str2num(script2);
					pos=ftell(scpfile);
					closescript();
					P_ITEM pi10 = Items->createScriptItem(s,script2,0);
					if (pi10 == NULL)
						return;
					pi10->setLayer( 0x00 );
					pi10->setContSerial(pi3->serial);
					pi10->pos.x=20+(rand()%50);
					pi10->pos.y=85+(rand()%75);
					pi10->pos.z=9;
					RefreshItem(pi10);//let's finally refresh the item
					strcpy((char*)script1, "DUMMY");
					openscript("carve.scp");
					fseek(scpfile, pos, SEEK_SET);
				}
			}
		}
		while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

		closescript();
	}


	if(deletecorpse)//if corpse has to be deleted
	{
		//let's empty it
		unsigned int ci;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pi3->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);
			pj->setContSerial(-1);
			pj->moveTo(pi3->pos);
			pj->startDecay();
			RefreshItem(pj);
		}
		Items->DeleItem(pi3);	//and then delete the corpse
	}
}

static void CorpseTarget(const P_CLIENT pC)
{
	int n = 0;
	UOXSOCKET s = pC->GetSocket();
	
	int serial = LongFromCharPtr(buffer[s] + 7);
	P_ITEM pi = FindItemBySerial(serial);
	if (pi != NULL)
	{
		if (iteminrange(s, pi, 1))
		{
			npcshape[0] = pi->serial;
			action(s, 0x20);
			n = 1;
			if (pi->more1 == 0)
			{
				pi->more1 = 1;// corpse being carved...can't carve it anymore
				
				if (pi->morey || pi->carve>-1)
				{// if specified, use enhanced carving system!
					newCarveTarget(s, pi);// AntiChrist
				}
				else
				{// else use standard carving
					switch (pi->amount())
					{
						case 0x01: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Ogre
						case 0x02: 
							CarveTarget(s, 0, 5, 0, 0, 0, 0);
							break; // Ettin
						case 0x03: 
							break;	// Zombie
						case 0x04: 
							break;	// Gargoyle
						case 0x05: 
							CarveTarget(s, 36, 0, 0, 0, 0, 1);
							break; // Eagle
						case 0x06: 
							CarveTarget(s, 25, 0, 0, 0, 0, 1);
							break; // Bird
						case 0x07: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc (with an axe in 2d)
						case 0x08: 
							break;	// Corpser
						case 0x09: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Daemon
						case 0x0A: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Daemon (with a sword in 2d)
						case 0x0B: 
							break;	// Dread Spider
						case 0x0C: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Dragon (green in t2a/grey in uo:td)
						case 0x0D: 
							break;	// Air Elemental
						case 0x0E: 
							break;	// Earth Elemental
						case 0x0F: 
							break;	// Fire Elemental
						case 0x10: 
							break;	// Water Elemental
						case 0x11: 
							CarveTarget(s, 0, 3, 0, 0, 0, 0);
							break; // Orc
						case 0x12: 
							CarveTarget(s, 0, 5, 0, 0, 0, 0);
							break; // Ettin (with club in 2d)
						case 0x13: 
							break;       // Frost Spider
						case 0x14: 
							break;       // Giant Spider
						case 0x15: 
							CarveTarget(s, 0, 4, 20, 0, 0, 0);
							break; // Giant Serpent
						case 0x16: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Gazer
						case 0x17: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Dire Wolf
						case 0x18: 
							break;	// Liche
						case 0x19: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Grey Wolf
						case 0x1A: 
							break;	// Ghoul
						case 0x1B: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Grey Wolf (another one)
						case 0x1C: 
							break;	// Spider
						case 0x1D: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Gorilla
						case 0x1E: 
							CarveTarget(s, 50, 0, 0, 0, 0, 1);
							break; // Harpy
						case 0x1F: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Headless
							// case 0x20: break;	//-NULL-
						case 0x21: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Lizardman
						case 0x22: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // White Wolf
						case 0x23: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Lizardman (with spear in 2d)
						case 0x24: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Lizardman (with mace in 2d)
						case 0x25: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // White Wolf (another one)
						case 0x26: 
							break;	// Black Gate Daemon
						case 0x27: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Mongbat
						case 0x28: 
							break;	// Elder Daemon
						case 0x29: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc (with club in 2d)
						case 0x2A: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Ratman
						case 0x2B: 
							break;	// Ice Fiend
						case 0x2C: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Ratman (with axe in 2d)
						case 0x2D: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Ratman (with dagger in 2d)
						case 0x2E: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Ancient Wyrm
						case 0x2F: 
							break;	// Reaper
						case 0x30: 
							break;	// Giant Scorpion
						case 0x31: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // White Wyrm
						case 0x32: 
							break;	// Skeleton
						case 0x33: 
							break;	// Slime
						case 0x34: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Snake
						case 0x35: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Troll (with axe in 2d)
						case 0x36: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Troll
						case 0x37: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Troll (with club in 2d)
						case 0x38: 
							break;	// Skeleton (with axe in 2d)
						case 0x39: 
							break;	// Skeleton (with sword in 2d)
						case 0x3A: 
							break;	// Wisp
						case 0x3B: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Red Dragon
						case 0x3C: 
							CarveTarget(s, 0, 10, 20, 0, 0, 0);
							break; // Drake (green in t2a/grey in uo:td)
						case 0x3D: 
							CarveTarget(s, 0, 10, 20, 0, 0, 0);
							break; // Red Drake
						case 0x3E: 
							CarveTarget(s, 0, 10, 20, 0, 0, 0);
							break; // Wyvern
						case 0x3F: 
							CarveTarget(s, 0, 1, 10, 0, 0, 0);
							break; // Cougar
						case 0x40: 
							CarveTarget(s, 0, 1, 0, 2, 0, 0);
							break; // Snow Leopard
						case 0x41: 
							CarveTarget(s, 0, 1, 0, 2, 0, 0);
							break; // Snow Leopard (another one)
						case 0x42: 
							break;       // Swamp Tentacles
						case 0x43: 
							break;       // Stone Gargoyle
						case 0x44: 
							break;       // Elder Gazer
						case 0x45: 
							break;       // Elder Gazer (another one)
						case 0x46: 
							CarveTarget(s, 0, 0, 0, 0, 0, 0);
							break; // Terathan Warrior
						case 0x47: 
							CarveTarget(s, 0, 0, 0, 0, 0, 0);
							break; // Terathan Drone
						case 0x48: 
							CarveTarget(s, 0, 0, 0, 0, 0, 0);
							break; // Terathan Queen
						case 0x49: 
							break;       // Stone Harpy
						case 0x4A: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Imp
						case 0x4B: 
							CarveTarget(s, 0, 4, 0, 0, 0, 0);
							break; // Titan
						case 0x4C: 
							CarveTarget(s, 0, 4, 0, 0, 0, 0);
							break; // Cyclopean Warrior
						case 0x4D: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Kraken
						case 0x4E: 
							break;       // Lich Lord
						case 0x50: 
							CarveTarget(s, 0, 10, 2, 0, 0, 0);
							break; // Giant Toad
						case 0x51: 
							CarveTarget(s, 0, 4, 1, 0, 0, 0);
							break; // Bullfrog
						case 0x52: 
							break;       // Lich Lord (another one)
						case 0x53: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Ogre Lord
						case 0x54: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Ogre Lord (another one)
						case 0x55: 
							CarveTarget(s, 0, 5, 7, 0, 0, 0);
							break; // Ophidian Mage
						case 0x56: 
							CarveTarget(s, 0, 5, 7, 0, 0, 0);
							break; // Ophidian Warrior
						case 0x57: 
							CarveTarget(s, 0, 5, 7, 0, 0, 0);
							break; // Ophidian Queen
						case 0x58: 
							CarveTarget(s, 0, 2, 8, 0, 0, 0);
							break; // Mountain Goat
						case 0x59: 
							CarveTarget(s, 0, 4, 20, 0, 0, 0);
							break; // Giant Ice Snake
						case 0x5A: 
							CarveTarget(s, 0, 4, 20, 0, 0, 0);
							break; // Giant Lava Snake
						case 0x5B: 
							CarveTarget(s, 0, 4, 20, 0, 0, 0);
							break; // Giant Silver Serpent
						case 0x5C: 
							CarveTarget(s, 0, 4, 20, 0, 0, 0);
							break; // Giant Silver Serpent (another one)
						case 0x5D: 
							CarveTarget(s, 0, 4, 20, 0, 0, 0);
							break; // Giant Silver Serpent (another one)
						case 0x5E: 
							break;       // Frost Slime
						case 0x5F: 
							break;       // Leviathan (only works in 2D client)
						case 0x60: 
							break;       // Frost Slime (another one)
						case 0x61: 
							break;       // Hell Hound
						case 0x62: 
							break;       // Hell Hound (another one)
						case 0x63: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Dark Wolf
						case 0x64: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Silver Wolf
						case 0x65: 
							CarveTarget(s, 0, 4, 10, 0, 0, 0);
							break; // Centaur
						case 0x66: 
							break;       // Exodus
						case 0x67: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Asian Dragon
						case 0x68: 
							break;       // Skeletal Dragon
							// case 0x69: break;       //-NULL-
						case 0x6A: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Shadow Wyrm
						case 0x6B: 
							break;       // Agapite Elemental
						case 0x6C: 
							break;       // Bronze Elemental
						case 0x6D: 
							break;       // Copper Elemental
						case 0x6E: 
							break;       // Dull Copper Elemental
						case 0x6F: 
							break;       // Iron Elemental
						case 0x70: 
							break;       // Valorite Elemental
						case 0x71: 
							break;       // Verite Elemental
						case 0x72: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Dark Steed
						case 0x73: 
							break;       // Ethereal Horse
						case 0x74: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Nightmare
						case 0x75: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Silver Steed
						case 0x76: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // War Horse (Britannians Faction)
						case 0x77: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // War Horse (Mage Council Faction)
						case 0x78: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // War Horse (Minax Faction)
						case 0x79: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // War Horse (Shadowlords Faction)
						case 0x7A: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Unicorn
						case 0x7B: 
							break;       // Ethereal Warrior (smallest)
						case 0x7C: 
							break;       // Evil Mage
						case 0x7D: 
							break;       // Evil Mage Master
						case 0x7E: 
							break;       // Evil Mage Master (another one)
						case 0x7F: 
							break;       // Predator Hell Cat
						case 0x80: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Pixie (smallest)
						case 0x81: 
							break;       // Swamp Tentacles (another one)
						case 0x82: 
							break;       // Blistering Gargoyle
						case 0x83: 
							break;       // Efreet
						case 0x84: 
							break;       // Kirin
						case 0x85: 
							break;       // Small Alligator
						case 0x86: 
							break;       // Komodo Dragon
						case 0x87: 
							CarveTarget(s, 0, 2, 0, 0, 0, 0);
							break; // Artic Ogre Lord
						case 0x88: 
							break;       // Ophidian Archmage
						case 0x89: 
							break;       // Ophidian Knight
						case 0x8A: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc Lord
						case 0x8B: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc Lord (another one)
						case 0x8C: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc Shaman
						case 0x8D: 
							break;       // Paladin
						case 0x8E: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Ratman (another one)
						case 0x8F: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Ratman Shaman
						case 0x90: 
							break;       // Sea Horse
						case 0x91: 
							CarveTarget(s, 0, 12, 0, 0, 0, 0);
							break; // Sea Serpent
						case 0x92: 
							break;       // Shadowlord
						case 0x93: 
							break;       // Skeleton Knight
						case 0x94: 
							break;       // Skeleton Mage
						case 0x95: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Succubus (smallest)
						case 0x96: 
							CarveTarget(s, 0, 10, 0, 0, 0, 0);
							break; // Sea Serpent (smallest)
						case 0x97: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Dolphin
						case 0x98: 
							break;       // Terathan Avenger
						case 0x99: 
							break;       // Ghoul
						case 0x9A: 
							break;       // Mummy
						case 0x9B: 
							break;       // Rotting Corpse
							// case 0x9C: break;       //-NULL-
						case 0x9D: 
							break;       // Black Widow Spider (smallest)
						case 0x9E: 
							break;       // Acid Elemental
						case 0x9F: 
							break;       // Blood Elemental
						case 0xA0: 
							break;       // Blood Elemental (another one)
						case 0xA1: 
							break;       // Ice Elemental
						case 0xA2: 
							break;       // Poison Elemental
						case 0xA3: 
							break;       // Snow Elemental
						case 0xA4: 
							break;       // Energy Vortex
						case 0xA5: 
							break;       // Black Wisp
						case 0xA6: 
							break;       // Gold Elemental
						case 0xA7: 
							CarveTarget(s, 0, 1, 0, 2, 0, 0);
							break; // Brown Bear
						case 0xA8: 
							break;       // Shadow Fiend
							// case 0xA9: break;       //-NULL-
						case 0xAA: 
							break;       // Ethereal Llama
						case 0xAB: 
							break;       // Ethereal Ostard
						case 0xAC: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // Red Dragon (biggest)
						case 0xAD: 
							break;       // Black Widow Spider (biggest)
						case 0xAE: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Succubus (biggest)
						case 0xAF: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Ethereal Warrior (biggest)
						case 0xB0: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Pixie (biggest)
						case 0xB1: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Nightmare (another one)
						case 0xB2: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Nightmare (another one)
						case 0xB3: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Nightmare (another one)
						case 0xB4: 
							CarveTarget(s, 0, 19, 20, 0, 0, 0);
							break; // White Wyrm (Retro)
						case 0xB5: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc Scout
						case 0xB6: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Orc Bomber
							// case 0xB7-case 0xBA: break; //-NULL-
						case 0xBB: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Ridgeback
						case 0xBC: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Savage Ridgeback
						case 0xBD: 
							CarveTarget(s, 0, 4, 0, 0, 0, 0);
							break; // Orc Brute
							// case 0xBE-case 0xC7: break; //-NULL-
						case 0xC8: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Horse (Dappled Brown)
						case 0xC9: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Cat
						case 0xCA: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Alligator
						case 0xCB: 
							CarveTarget(s, 0, 6, 0, 0, 0, 0);
							break; // Pig
						case 0xCC: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Horse (dark)
						case 0xCD: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Rabbit
						case 0xCE: 
							CarveTarget(s, 0, 1, 12, 0 , 0, 0);
							break; // Lava Lizard
						case 0xCF: 
							CarveTarget(s, 0, 3, 0, 0, 1, 0);
							break; // Sheep
						case 0xD0: 
							CarveTarget(s, 25, 0, 0, 0, 0, 1);
							break; // Chicken
						case 0xD1: 
							CarveTarget(s, 0, 2, 8, 0, 0, 0);
							break; // Goat Billy
						case 0xD2: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Desert Ostard
						case 0xD3: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Bear
						case 0xD4: 
							CarveTarget(s, 0, 1, 0, 2, 0, 0);
							break; // Grizzly Bear
						case 0xD5: 
							CarveTarget(s, 0, 2, 0, 3, 0, 0);
							break; // Polar Bear
						case 0xD6: 
							CarveTarget(s, 0, 1, 10, 0, 0, 0);
							break; // Panther
						case 0xD7: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Giant Rat
						case 0xD8: 
							CarveTarget(s, 0, 8, 12, 0, 0, 0);
							break; // Cow (black)
						case 0xD9: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Hound
						case 0xDA: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Frenzied Ostard
						case 0xDB: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Forest Ostard
						case 0xDC: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Llama
						case 0xDD: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Walrus
							// case 0xDE: break;	//-NULL-
						case 0xDF: 
							CarveTarget(s, 0, 3, 0, 0, 0, 0);
							break; // Sheep (Shorn)
						case 0xE1: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Timber Wolf
						case 0xE2: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Horse (Dappled Grey)
							// case 0xE3: break;	//-NULL-
						case 0xE4: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Horse (tan)
							// case 0xE5: break;	//-NULL-
							// case 0xE6: break;	//-NULL-
						case 0xE7: 
							CarveTarget(s, 0, 8, 12, 0, 0, 0);
							break; // Spotted Cow
						case 0xE8: 
							CarveTarget(s, 0, 10, 15, 0, 0, 0);
							break; // Brown Bull
						case 0xE9: 
							CarveTarget(s, 0, 10, 15, 0, 0, 0);
							break; // Spotted Bull
						case 0xEA: 
							CarveTarget(s, 0, 6, 15, 0, 0, 0);
							break; // Great Heart
							// case 0xEB: break;	//-NULL-
							// case 0xEC: break;	//-NULL-
						case 0xED: 
							CarveTarget(s, 0, 5, 8, 0, 0, 0);
							break; // Hind
						case 0xEE: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Sewer Rat
							// case 0xEF-case 0xFF: break; //-NULL-
						case 0x0122: 
							CarveTarget(s, 0, 10, 0, 0, 0, 0);
							break; // Boar
							// case 0x0123: break;   //-NULL-
						case 0x0124: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Pack Llama
							// case 0x0125-case 0x01A5: break; //-NULL
						case 0x01A4: 
							break;     // Blonde Boy (only in pre-t2a relases)
						case 0x01A5: 
							break;     // Brunette Boy (only in pre-t2a relases)
						case 0x01A6: 
							break;     // Toddler Girl (only in pre-t2a relases)
						case 0x01A7: 
							break;     // Brunette Girl (only in pre-t2a relases)
							// case 0x01A8-case 0x02EF: break; //-NULL-
						case 0x02F0: 
							break;     // Iron Golem
						case 0x02F1: 
							break;     // Gargoyle Slave
						case 0x02F2: 
							break;     // Gargoyle Enforcer
						case 0x02F3: 
							break;     // Gargoyle Guard
						case 0x02F4: 
							break;     // Exodus Clockwork Overseer
						case 0x02F5: 
							break;     // Exodus Clockwork Minion
						case 0x02F6: 
							break;     // Gargoyle Shopkeeper
							// case 0x02F7-case 0xFA: break; //-NULL-
						case 0x02FB: 
							break;     // Exodus Clockwork Minion (biggest)
						case 0x02FC: 
							break;     // Juka Warrior
						case 0x02FD: 
							break;     // Juka Mage
						case 0x02FE: 
							break;     // Kabur (NPC)
						case 0x02FF: 
							break;     // Blackthorn Cohort
						case 0x0300: 
							break;     // Juggernaut
						case 0x0301: 
							break;     // Future Blackthorn (NPC)
						case 0x0302: 
							break;     // Meer Mage
						case 0x0303: 
							break;     // Meer Warrior
						case 0x0304: 
							break;     // Adranath (NPC)
						case 0x0305: 
							break;     // Capitain Dasha (NPC)
						case 0x0306: 
							break;     // Dawn Girl (NPC)
							// case 0x0307: break;     //Plague Beast (not working yet)
						case 0x0308: 
							break;     //Horde Demon (smallest)
							// case 0x0309: break;     //Doppleganger (not working yet)
							// case 0x030A: break;     //Swarm (not working yet)
							// case 0x030B: break;     //Bogling (not working yet)
							// case 0x030C: break;     //Bog Thing (not working yet)
							// case 0x030D: break;     //Fire Ant Worker (not working yet)
							// case 0x030E: break;     //Fire Ant Warrior (not working yet)
							// case 0x030F: break;     //Fire Ant Queen (not working yet)
							// case 0x0310: break;     //Arcane Demon (not working yet)
							// case 0x0311: break;     //Four Armed Demon (not working yet)
							// case 0x0312: break;     //Chariot (not working yet)
							// case 0x0313: break;     //-NULL-
							// case 0x0314: break;     //Sphinx (not working yet)
							// case 0x0315: break;     //Quagmire (not working yet)
							// case 0x0316: break;     //Sand Vortex (not working yet)
							// case 0x0317: break;     //Giant Beetle (not working yet)
							// case 0x0318: break;     //Chaos Demon (not working yet)
							// case 0x0319: break;     //Skeletal Mount (not working yet)
							// case 0x031A: break;     //Swamp Dragon (not working yet)
						case 0x031B: 
							break;     //Horde Demon
						case 0x031C: 
							break;     //Horde Demon (biggest)
						case 0x031D: 
							break;     // Fire Dragon (buggy model)
						case 0x031E: 
							break;     // Rust Dragon (buggy model)
							// case 0x031F: break;     //Armored Swamp Dragon (not working yet)
							// case 0x0320-case 0x033: break; //-NULL-
						case 0x0334: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Dappled Brown Horse (another one)
						case 0x0338: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Dark Brown Horse (another one)
						case 0x0339: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Desert Ostard (another one)
						case 0x033A: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Forest Ostard (another one)
						case 0x033B: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Frenzied Ostard (another one)
						case 0x033C: 
							CarveTarget(s, 0, 1, 12, 0, 0, 0);
							break; // Llama (another one)
							// case 0x033D: break; //-NULL-
							// case 0x033E: break; //-NULL-
						case 0x033F: 
							CarveTarget(s, 25, 0, 0, 0, 0, 1);
							break; // Parrot
							// case 0x0340: CarveTarget(s,36, 0, 0, 0, 0, 1); break; //Phoenix (Not working yet)
							// case 0x0341: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Turkey (Not working yet)
						case 0x0342: 
							break;     // Hell Cat
						case 0x0343: 
							CarveTarget(s, 0, 1, 0, 1, 0, 0);
							break; // Jackrabbit
							// case 0x0344: break;     //-NULL-
						case 0x0345: 
							break;     // Ice Snake
						case 0x0346: 
							break;     // Lava Snake
						case 0x0347: 
							CarveTarget(s, 0, 15, 0, 0, 0, 0);
							break; // Ridgeback (another one)
							// case 0x0348: break;     //Giant Ridgeback (not working yet)
							// case 0x0349: break;     //Flame Ridgeback (not working yet)
							// case 0x034A: break;     //Hatchling Ridgeback (not working yet)
						case 0x034B: 
							CarveTarget(s, 0, 1, 0, 0, 0, 0);
							break; // Town Rat
							// case 0x034C-case 0x034F: break; //-NULL-
						case 0x0350: 
							CarveTarget(s, 0, 3, 10, 0, 0, 0);
							break; // Tan Horse (another one)
							// case 0x0351-case 0x03F5: break; //-NULL-
							// case 0x03E6: break; //Kirin (another one) (not working! why??)
						default:
							LogErrorVar("Fallout of switch statement, value <%i>", pi->amount());
					}// switch
				}// if morey || carve>-1
			}
			else 
			{
				sysmessage(s, "You carve the corpse but find nothing usefull.");
			}// if more1==0
			// break;
		}
	}// if i!=-1
	if (!n)
		sysmessage(s, "That is too far away.");
}

static void BladeTarget(P_CLIENT pC, PKGx6C *pp)
{
	UOXSOCKET s = pC->GetSocket();
	short id=pp->model;

	if (isCharSerial(pp->Tserial))
	{
		;	// a sheep or goat ?
	}
	else if (IsCorpse(id))
	{
		CorpseTarget(pC);
	}
	else if(IsFish(id))
	{
		const P_ITEM rpi=FindItemBySerial(pp->Tserial);
		short int amt=0;
		if (!rpi) return;
		if (rpi->amount()>1)
			amt=(rpi->amount()*4);
		else
			amt=4; 
		soundeffect(s,0x00,0x50);
		P_ITEM pi = Items->SpawnItem(pC->getPlayer(), amt, "#", 1, 0x097A, 0, 1);
		if(!pi)
			return;
		RefreshItem(pi);
		Items->DeleItem(rpi);
	}
	else
		sysmessage(s,"You can't think of a way to use your blade on that.");
}

void cTargets::SwordTarget(const P_CLIENT pC, PKGx6C *pp)
{
	UOXSOCKET s = pC->GetSocket();

	if (IsTree(pp->model))
	{
		P_CHAR pc = pC->getPlayer();
		if (!pc->onHorse()) action(s,0x0D);
		else action(s,0x1d);
		soundeffect(s,0x01,0x3E);
		P_ITEM pi = Items->SpawnItem(pc,1,"#",1,0x0DE1,0,0); //Kindling
		if(!pi)
			return;
		pi->moveTo(pc->pos);
		RefreshItem(pi);
		sysmessage(s, "You hack at the tree and produce some kindling.");
	}
	else if( IsLog(pp->model) || IsBoard(pp->model) )
		Skills->BowCraft(s);
	else
		BladeTarget(pC,pp);
}

static void AxeTarget(P_CLIENT pC, PKGx6C *pp)
{
	if (IsTree(pp->model))
		Skills->TreeTarget(pC->GetSocket());
	else
		BladeTarget(pC,pp);
}

void cTargets::NpcTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		addid1[s] = static_cast<unsigned char>(pc->serial>>24);
		addid2[s] = static_cast<unsigned char>(pc->serial>>16);
		addid3[s] = static_cast<unsigned char>(pc->serial>>8);
		addid4[s] = static_cast<unsigned char>(pc->serial%256);
		target(s, 0, 1, 0, 57, "Select NPC to follow this player.");
	}
}

void cTargets::NpcTarget2(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->isNpc())
		{
			pc->ftarg = calcserial(addid1[s], addid2[s], addid3[s], addid4[s]);
			pc->npcWander=1;
		}
	}
}

void cTargets::NpcRectTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if ((pc->isNpc()))
		{
			pc->fx1=addx[s];
			pc->fy1=addy[s];
		    pc->fz1=-1;
			pc->fx2=addx2[s];
			pc->fy2=addy2[s];
			pc->npcWander=3;
		}
	}
}

void cTargets::NpcCircleTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if ((pc->isNpc()))
		{
			pc->fx1=addx[s];
			pc->fy1=addy[s];
			pc->fz1=-1;
			pc->fx2=addx2[s];
			pc->npcWander=2; // bugfix, LB
		}
	}
}

void cTargets::NpcWanderTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if ((pc->isNpc())) pc->npcWander=npcshape[0];
	}
}

//taken from 6904t2(5/10/99) - AntiChrist
void cTargets::NpcAITarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setNpcAIType(addx[s]);
		sysmessage(s, "Npc AI changed.");//AntiChrist
	}
}

void cTargets::xBankTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		openbank(s, pc);
	}
}

void cTargets::xSpecialBankTarget(int s)//AntiChrist
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		openspecialbank(s, pc);
	}
}

void cTargets::SellStuffTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		sellstuff(s, pc);
	}
}

void cTargets::ReleaseTarget(int s, int c) 
{ 

	P_CHAR pc_i = NULL;
	if (c == INVALID_SERIAL) 
	{ 
		SERIAL serial = LongFromCharPtr(buffer[s] + 7); 
		pc_i = FindCharBySerial(serial); 
	} 
	else 
	{ 
		pc_i = FindCharBySerial(c); 
	} 
	if (pc_i != NULL) 
	{ 
		P_CHAR pc = pc_i;
		if (pc->cell == 0) 
		{ 
			sysmessage(s, "That player is not in jail!"); 
		} 
		else 
		{ 
			jails[pc->cell].occupied = false; 
			pc->moveTo(jails[pc->cell].oldpos);
			pc->cell = 0; 
			pc->priv2 = 0; 
			pc->jailsecs = 0; 
			pc->jailtimer = 0; 
			teleport(pc); 
			soundeffect(c, 1, 0xfd); // Play sound effect for player 
			sysmessage(c, "You are released.."); 
			sysmessage(s, "Player %s released.", pc->name.c_str()); 
		} 
	} 
}

void cTargets::GmOpenTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if( pi->layer() == addmitem[s] )
		{
			backpack(s, pi->serial);
			return;
		}
	}
	sysmessage(s,"No object was found at that layer on that character");
}

void cTargets::StaminaTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		soundeffect2(pc, 0x01F2);
		staticeffect(pc, 0x37, 0x6A, 0x09, 0x06);
		pc->stm = pc->effDex();
		updatestats(pc, 2);
		return;
	}
	sysmessage(s,"That is not a person.");
}

void cTargets::ManaTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		soundeffect2(pc, 0x01F2);
		staticeffect(pc, 0x37, 0x6A, 0x09, 0x06);
		pc->mn = pc->in;
		updatestats(pc, 1);
		return;
	}
	sysmessage(s,"That is not a person.");
}

void cTargets::MakeShopTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		Commands->MakeShop(pc);
		teleport(pc);
		sysmessage(s, "The buy containers have been added.");
		return;
	}
	sysmessage(s, "Target character not found...");
}

void cTargets::JailTarget(int s, int c) 
{ 
	SERIAL serial; 
	P_CHAR tmpnum = NULL;
	
	int x = 0; 
	if (c == INVALID_SERIAL) 
	{ 
		serial = LongFromCharPtr(buffer[s] + 7); 
		tmpnum = FindCharBySerial(serial); 
	} 
	else 
	{ 
		tmpnum = FindCharBySerial(c); 
	} 
	if (tmpnum == NULL)
		return; // lb 
	P_CHAR pc = tmpnum; 
	
	if (pc->cell>0) 
	{ 
		sysmessage(s, "That player is already in jail!"); 
		return; 
	} 
	int i;
	for (i = 1; i < 11; i++) 
	{ 
		if (!jails[i].occupied) 
		{ 
			jails[i].oldpos = pc->pos;
			pc->moveTo(jails[i].pos); 
			pc->cell = i; 
			pc->priv2 = 2; // freeze them  Ripper 
			
			
			// blackwinds jail
			pc->jailsecs = addmitem[s]; // Additem array used for jail time here.. 
			addmitem[s] = 0; // clear it 
			pc->jailtimer = uiCurrentTime +(MY_CLOCKS_PER_SEC*pc->jailsecs); 
			teleport(tmpnum); 
			UOXSOCKET prisoner = calcSocketFromChar(tmpnum); 
			jails[i].occupied = 1; 
			sysmessage(prisoner, "You are jailed !"); 
			sysmessage(prisoner, "You notice you just got something new at your backpack.."); 
			sysmessage(s, "Player %s has been jailed in cell %i.", pc->name.c_str(), i); 
			Items->SpawnItemBackpack2(prisoner, "50040", 0); // spawn crystall ball of justice to prisoner. 
			// end blackwinds jail 
			
			x++; 
			break; 
		} 
	} 
	if (x == 0) 
		sysmessage(s, "All jails are currently full!"); 
}

void cTargets::AttackTarget(int s)
{
	P_CHAR target = FindCharBySerial(addx[s]);
	P_CHAR target2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));
    if (target->inGuardedArea()) // Ripper..No pet attacking in town.
	{
        sysmessage(s,"You cant have pets attack in town!");
        return;
	}
	if (target2 == NULL || target == NULL) return;
	npcattacktarget(target2, target);
}

void cTargets::FollowTarget(int s)
{

	P_CHAR char1 = FindCharBySerial(addx[s]);
	P_CHAR char2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));

	char1->ftarg = char2->serial;
	char1->npcWander = 1;
}

void cTargets::TransferTarget(int s)
{
	char t[120];

	P_CHAR pc1 = FindCharBySerial(addx[s]);
	P_CHAR pc2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));
	if ( pc1 == NULL || pc2 == NULL)
		return;

	sprintf(t,"* %s will now take %s as his master *",pc1->name.c_str(), pc2->name.c_str());
	npctalkall(pc1, t, 0);

	if (pc1->ownserial != -1) 
		pc1->SetOwnSerial(-1);
	pc1->SetOwnSerial(pc2->serial);
	pc1->npcWander=1;

	pc1->ftarg = INVALID_SERIAL;
	pc1->npcWander=0;
}

void cTargets::BuyShopTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
		if ((pc->serial==serial))
		{
			Targ->BuyShop(s, pc);
			return;
		}
		sysmessage(s, "Target shopkeeper not found...");
}

int cTargets::BuyShop(UOXSOCKET s, P_CHAR pc)
{
	P_ITEM pCont1=NULL, pCont2=NULL;

	if ( pc == NULL )
		return 0;

	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if( pi->layer() == 0x1A )
		{
			pCont1=pi;
		}
		else if( pi->layer() == 0x1B )
		{
			pCont2=pi;
		}
	}

	if (!pCont1 || !pCont2)
	{
		return 0;
	}

	impowncreate(s, pc, 0); // Send the NPC again to make sure info is current. (OSI does this we might not have to)
	sendshopinfo(s, pc, pCont1); // Send normal shop items
	sendshopinfo(s, pc, pCont2); // Send items sold to shop by players
	SndShopgumpopen(s,pc->serial);
	statwindow(s, currchar[s]); // Make sure the gold total has been sent.
	return 1;
}

////////////////////////////////// 
// Changed hideing to make flamestrike and hide work better 
// 
//
void cTargets::permHideTarget(int s) 
{ 
	SERIAL serial = LongFromCharPtr(buffer[s] + 7); 
	P_CHAR pc = FindCharBySerial(serial); 
	if (pc != NULL) 
	{ 
		if (pc->hidden() == 1) 
		{ 
			if (pc == currchar[s])
				sysmessage(s, "You are already hiding."); 
			else 
				sysmessage(s, "He is already hiding."); 
			return; 
		} 
		pc->priv2 |= 8; 
		// staticeffect(i, 0x37, 0x09, 0x09, 0x19); 
		staticeffect3(pc->pos.x + 1, pc->pos.y + 1, pc->pos.z + 10, 0x37, 0x09, 0x09, 0x19, 0); 
		soundeffect2(pc, 0x0208); 
		tempeffect(pc, pc, 33, 1, 0, 0); 
		return; 
	} 
} 
// 
// 
// Aldur 
////////////////////////////////// 

////////////////////////////////// 
// Changed unhideing to make flamestrike and unhide work better 
// 
//
void cTargets::unHideTarget(int s) 
{ 
	SERIAL serial = LongFromCharPtr(buffer[s] + 7); 
	P_CHAR pc = FindCharBySerial(serial); 
	if (pc != NULL) 
	{ 
		if (pc->hidden() == 0) 
		{ 
			if (pc == currchar[s])
				sysmessage(s, "You are not hiding."); 
			else 
				sysmessage(s, "He is not hiding."); 
			return; 
		} 
		pc->priv2 = pc->priv2&0xf7; // unhide - AntiChrist 
		// we cant use staticeffect cause the char is invis and 
		// regular chars could see it. Instead we will use the staticeffect3 
		// which takes the char coords. 
		// staticeffect(i, 0x37, 0x09, 0x09, 0x19); 
		staticeffect3(pc->pos.x + 1, pc->pos.y + 1, pc->pos.z + 10, 0x37, 0x09, 0x09, 0x19, 0); 
		soundeffect2(pc, 0x0208); 
		tempeffect(pc, pc, 34, 1, 0, 0); 
		return; 
	} 
} 
// 
// 
// Aldur 
//////////////////////////////////

void cTargets::SetSpeechTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->isPlayer())
		{
			sysmessage(s,"You can only change speech for npcs.");
			return;
		}
		pc->speech=addx[s];
	}
}

static void SetSpAttackTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->spattack=tempint[s];
	}
}

void cTargets::SetSpaDelayTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->spadelay=tempint[s];
	}
}

void cTargets::SetPoisonTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setPoison(tempint[s]);
	}
}

void cTargets::SetPoisonedTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setPoisoned(tempint[s]);
		pc->setPoisonwearofftime(uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()));
		impowncreate(calcSocketFromChar(pc), pc, 1); //Lb, sends the green bar !
	}
}

void cTargets::FullStatsTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		soundeffect2(pc, 0x01F2);
		staticeffect(pc, 0x37, 0x6A, 0x09, 0x06);
		pc->mn=pc->in;
		pc->hp=pc->st;
		pc->stm=pc->effDex();
		updatestats(pc, 0);
		updatestats(pc, 1);
		updatestats(pc, 2);
		return;
	}
	sysmessage(s,"That is not a person.");
}

void cTargets::SetAdvObjTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setAdvobj(tempint[s]);
	}
}

////////////////
// name:		CanTrainTarget
// history:		by Antrhacks 1-3-99
// Purpose:		Used for training by NPC's
//
void cTargets::CanTrainTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->isPlayer())
		{
			sysmessage(s, "Only NPC's may train.");
			return;
		}
		pc->setCantrain(!pc->cantrain());	//turn on if off, off if on
	}
}

void cTargets::SetSplitTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setSplit(tempint[s]);
	}
}

void cTargets::SetSplitChanceTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setSplitchnc(tempint[s]);
	}
}

void cTargets::SetDirTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);

	if (isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi != NULL)
		{
			pi->dir=addx[s];
			RefreshItem(pi);
			return;
		}
	}
	else
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc != NULL)
		{
			pc->dir=addx[s];
			updatechar(pc);
			return;
		}
	}
}

////////////////
// name:		NpcResurrectTarget
// history:		by UnKnown (Touched tabstops by Tauriel Dec 28, 1998)
// Purpose:		Resurrects a character
//
bool cTargets::NpcResurrectTarget(P_CHAR pc)
{
	if ( pc == NULL)
		return true;

	if (pc->dead)
	{//Shouldn' be a validNPCMove inside a door, might fix house break in. -- from zippy code
		Fame(pc,0);
		soundeffect2(pc, 0x0214);
		pc->setId(pc->xid);
		pc->setSkin(pc->xskin());
		pc->dead=false;
		pc->hp=pc->st;// /10;
		pc->stm=pc->effDex();// /10;
		pc->mn=pc->in; ///10;
		pc->attacker = INVALID_SERIAL;
		pc->resetAttackFirst();
		pc->war=false;

		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{
			P_ITEM pj = iterItems.GetData();
			if (pc->Wears(pj) && pj->layer()==0x1A)
			{
				pj->setLayer( 0x15 );
				pc->packitem = pj->serial;	//Tauriel packitem speedup
				//break;
			}
		}
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{
			P_ITEM pj = iterItems.GetData();
			if (pj->serial == pc->robe)
			{
				Items->DeleItem(pj);

				P_ITEM pi = Items->SpawnItem(pc, 1, "a robe", 0, 0x1F03, 0, 0);
				if(!pi) return false;
				pi->setContSerial(pc->serial);
				pi->setLayer( 0x16 );
				pi->dye=1;
				break;
			}
		}
		teleport(pc);
		return true;
	}

	UOXSOCKET k = calcSocketFromChar(pc);
	if (k <= -1) return false;

	sysmessage(k,"That person isn't dead");
	return false;
}

void cTargets::NewXTarget(int s) // Notice a high similarity to th function above? Wonder why. - Gandalf
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	if (isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.x = addx[s];
		pi->moveTo(pos);
		RefreshItem(pi);
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.x = addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::NewYTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	if (isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.y = addx[s];
		pi->moveTo(pos);
		RefreshItem(pi);
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.y = addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::IncXTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.x += addx[s];
		pi->moveTo(pos);
		RefreshItem(pi);
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.x += addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::IncYTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.y += addx[s];
		pi->moveTo(pos);
		RefreshItem(pi);
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.y += addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::HouseOwnerTarget(int s) // crackerjack 8/10/99 - change house owner
{
	int os, i;
	int o_serial = LongFromCharPtr(buffer[s]+7);
	if(o_serial==-1) return;
	P_CHAR pc = FindCharBySerial(o_serial);
	if ( pc == NULL ) return;

	SERIAL serial = calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	P_ITEM pSign = FindItemBySerial(serial);
	if ( pSign == NULL )
		return;

	serial = calcserial(pSign->more1, pSign->more2, pSign->more3, pSign->more4);
	P_ITEM pHouse = FindItemBySerial(serial);
	if ( pHouse == NULL )
		return;
	
	if(pc->serial == currchar[s]->serial)
	{
		sysmessage(s, "you already own this house!");
		return;
	}
	
	pSign->SetOwnSerial(o_serial);
	
	pHouse->SetOwnSerial(o_serial);
	
	RemoveKeys(pHouse->serial);
	
	os=-1;
	for(i=0;i<now && os==-1;i++)
	{
		if( currchar[i]->serial == pc->serial && perm[i]) os=i;
	}
	
	P_ITEM pi3=NULL;
	if(os!=-1)
	{
		pi3 = Items->SpawnItem(os, pc, 1, "a house key", 0, 0x10, 0x0F, 0, 1,1);//gold key for everything else
		if(pi3 == NULL) return;
	}
	else
	{
		pi3 = Items->SpawnItem(pc, 1, "a house key", 0, 0x100F,0,0);//gold key for everything else
		if(!pi3) return;
		pi3->moveTo(pc->pos);
		RefreshItem(pi3);
	}
	pi3->more1 = static_cast<unsigned char>((pHouse->serial&0xFF000000)>>24);
	pi3->more2 = static_cast<unsigned char>((pHouse->serial&0x00FF0000)>>16);
	pi3->more3 = static_cast<unsigned char>((pHouse->serial&0x0000FF00)>>8);
	pi3->more4 = static_cast<unsigned char>((pHouse->serial&0x000000FF));
	pi3->setType( 7 );
	
	sysmessage(s, "You have transferred your house to %s.", pc->name.c_str());
	sprintf((char*)temp, "%s has transferred a house to %s.", currchar[s]->name.c_str(), pc->name.c_str());

	int k;
	for(k=0;k<now;k++)
		if(k!=s && ( (perm[k] && inrange1p(currchar[k], currchar[s]) )||
			(currchar[k]->serial==o_serial)))
			sysmessage(k, (char*)temp);
}

void cTargets::HouseEjectTarget(int s) // crackerjack 8/11/99 - kick someone out of house
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if (!pc)
		return;
	int serial=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	if(serial == INVALID_SERIAL) return;
	P_ITEM pHouse = FindItemBySerial(serial);
	if(pHouse != NULL)
	{
		int sx, sy, ex, ey;
		Map->MultiArea(pHouse, &sx,&sy,&ex,&ey);
		if(pc->serial == currchar[s]->serial)
		{
			sysmessage(s,"Do you really want to do that?!");
			return;
		}
		if(pc->pos.x>=sx&&pc->pos.y>=sy&&pc->pos.x<=ex&&pc->pos.y<=ey)
		{
			Coord_cl pos(pc->pos);
			pos.x = ex;
			pos.y = ey;
			pc->moveTo( pos );
			teleport(pc);
			sysmessage(s, "Player ejected.");
		}
		else
			sysmessage(s, "That is not inside the house.");
	}
}

void cTargets::HouseBanTarget(int s) 
{
	Targ->HouseEjectTarget(s);	// first, eject the player

	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];
	if (!pc)
		return;
	cHouse* pHouse = dynamic_cast<cHouse*>(findmulti(pc_home->pos));
	if(pHouse != NULL)
	{
		if (pc->serial == pc_home->serial) return;
		pHouse->addBan(pc);
		sysmessage(s, "%s has been banned from this house.", pc->name.c_str());
	}
}

void cTargets::HouseFriendTarget(int s) // crackerjack 8/12/99 - add somebody to friends list
{
	P_CHAR Friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];

	cHouse* pHouse = dynamic_cast<cHouse*>(findmulti(pc_home->pos));

	if(Friend && pHouse)
	{
		if(Friend->serial == pc_home->serial)
		{
			sysmessage(s,"You are already the owner!");
			return;
		}
		pHouse->addFriend(Friend);
		sysmessage(s, "%s has been made a Friend of the house.", Friend->name.c_str());
	}
}

void cTargets::HouseUnBanTarget(int s)
{
	P_CHAR pc_banned = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast<cHouse*>(findmulti(pc_owner->pos));

	if(pc_banned && pHouse)
	{
		if(pc_banned->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeBan(pc_banned);
		sysmessage(s,"%s has been UnBanned!",pc_banned->name.c_str());
	}
	return;
}

void cTargets::HouseUnFriendTarget(int s)
{
	P_CHAR pc_friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast<cHouse*>(findmulti(pc_owner->pos));

	if(pc_friend && pHouse)
	{
		if(pc_friend->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeFriend(pc_friend);
		sysmessage(s,"%s is no longer a Friend of this home!", pc_friend->name.c_str());
	}
	return;
}

void cTargets::HouseLockdown( UOXSOCKET s ) // Abaddon
// PRE:		S is the socket of a valid owner/coowner and is in a valid house
// POST:	either locks down the item, or puts a message to the owner saying he's a moron
// CODER:	Abaddon
// DATE:	17th December, 1999
{
	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);

	if( pi != NULL )
	{
		short id = pi->id();

		// not needed anymore, cause called from house_sped that already checks that ...

		// time to lock it down!

		if (Items->isFieldSpellItem ( pi ))
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if( IsAnvil( id ) )
		{
			sysmessage(s, "You cant lockdown anvils!");
			return;
		}
		if( IsForge( id ) )
		{
			sysmessage(s, "You cant lockdown forges!");
			return;
		}

		P_ITEM pi_multi = findmulti( pi->pos );
		if( pi_multi != NULL )
		{
			if(pi->isLockedDown())
			{
				sysmessage(s,"That item is already locked down, release it first!");
				return;
			}
			pi->setLockedDown();	// LOCKED DOWN!
			DRAGGED[s]=0;
			pi->setOwnSerialOnly(currchar[s]->serial);
			RefreshItem(pi);
			return;
		}
		else
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}
}

void cTargets::HouseSecureDown( UOXSOCKET s ) // Ripper
// For locked down and secure chests
{
	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		// time to lock it down!
		if (Items->isFieldSpellItem ( pi ) )
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if( pi->isLockedDown() )
		{
			sysmessage(s,"That item is already locked down, release it first!");
			return;
		}

		P_ITEM pi_multi = findmulti( pi->pos );
		if( pi_multi != NULL && pi->type() == 1 )
		{
		    pi->setLockedDown();	// LOCKED DOWN!
			pi->setSecured( true );
			DRAGGED[s]=0;
			pi->setOwnSerialOnly(currchar[s]->serial);
			RefreshItem(pi);
			return;
		}
		if( pi->type() != 1 )
		{
			sysmessage(s,"You can only secure chests!");
			return;
		}
		else
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}
}

void cTargets::HouseRelease( UOXSOCKET s ) // Abaddon & Ripper
// PRE:		S is the socket of a valid owner/coowner and is in a valid house, the item is locked down
// POST:	either releases the item from lockdown, or puts a message to the owner saying he's a moron
// CODER:	Abaddon
// DATE:	17th December, 1999
// update: 5-8-00
{
	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		if (Items->isFieldSpellItem(pi))
		{
			sysmessage(s,"you cannot release this!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant release doors or signs!");
			return;
		}

		// time to unlock it!
		P_ITEM pi_multi = findmulti( pi->pos );
		if( pi_multi != NULL && pi->isLockedDown() || pi->type() == 1 )
		{
			pi->setAllMovable();	// Default as stored by the client, perhaps we should keep a backup?
			pi->setSecured( false );
			RefreshItem( pi );
			return;
		}
		else if( pi_multi == NULL )
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}
}

void cTargets::SetMurderCount( int s )
{
	int serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if( pc != NULL )
	{
		pc->kills = addmitem[s];
		setcharflag(pc);
	}
}

void cTargets::GlowTarget(int s) // LB 4/9/99, makes items glow
{
	int c;

	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL) return;
	P_ITEM pi1 = FindItemBySerial(serial);
	if (pi1 == NULL)
	{
		sysmessage(s,"Item not found.");
		return;
	}

	P_CHAR pc_currchar = currchar[s];
	if (!pi1->isInWorld())
	{
		P_ITEM pj = FindItemBySerial(pi1->contserial); // in bp ?
		P_CHAR pc_l = FindCharBySerial(pi1->contserial); // equipped ?
		P_CHAR pc_k = NULL;
		if (pc_l == NULL) 
			pc_k = GetPackOwner(pj); 
		else 
			pc_k = pc_l;
		if (pc_k != currchar[s])	// creation only allowed in the creators pack/char otherwise things could go wrong
		{
			sysmessage(s,"you can't unglow items in other persons packs or hands");
			return;
		}
	}

	if (pi1->glow != INVALID_SERIAL)
	{
		sysmessage(s,"that object already glows!\n");
		return;
	}

	c=0x99;
	pi1->glow_color = pi1->color(); // backup old colors

	pi1->setColor( c );

	P_ITEM pi2 = Items->SpawnItem(s, pc_currchar,1,"glower",0,0x16,0x47,0,0,1); // new client 1.26.2 glower object

	if(pi2 == NULL) return;
	pi2->dir=29; // set light radius maximal
	pi2->visible=0;

	pi2->setOwnerMovable();

	mapRegions->Remove(pi2); // remove if add in spawnitem
	pi2->setLayer( pi1->layer() );
	if( pi2->layer() == 0 ) // if not equipped -> coords of the light-object = coords of the
	{
		pi2->pos.x=pi1->pos.x;
		pi2->pos.y=pi1->pos.y;
		pi2->pos.z=pi1->pos.z;
	} 
	else // if equipped -> place lightsource at player ( height= approx hand level )
	{
		pi2->pos.x=pc_currchar->pos.x;
		pi2->pos.y=pc_currchar->pos.y;
		pi2->pos.z=pc_currchar->pos.z+4;
	}

	pi2->priv=0; // doesnt decay

	pi1->glow=pi2->serial; // set glow-identifier


	RefreshItem(pi1);
	RefreshItem(pi2);

	impowncreate(s, pc_currchar, 0); // if equipped send new color too
}

void cTargets::UnglowTaget(int s) // LB 4/9/99, removes the glow-effect from items
{
	int c;
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi)
	{
		sysmessage(s,"no item found");
		return;
	}

	if (!pi->isInWorld())
	{
		P_ITEM pj = FindItemBySerial(pi->contserial); // in bp ?
		P_CHAR pc_l = FindCharBySerial(pi->contserial); // equipped ?
		P_CHAR pc_k = NULL;
		if (pc_l == NULL) 
			pc_k = GetPackOwner(pj); 
		else 
			pc_k = pc_l;
		if (pc_k != currchar[s])	// creation only allowed in the creators pack/char otherwise things could go wrong
		{
			sysmessage(s,"you can't unglow items in other persons packs or hands");
			return;
		}
	}

	c = pi->glow;
	if(c == INVALID_SERIAL) return;
	P_ITEM pj = FindItemBySerial(c);

	if (pi->glow==0 || pj == NULL )
	{
		sysmessage(s,"that object doesnt glow!\n");
		return;
	}

	pi->setColor( pi->glow_color );

	Items->DeleItem(pj); // delete glowing object

	pi->glow = INVALID_SERIAL; // remove glow-identifier
	RefreshItem(pi);

	impowncreate(s, currchar[s], 0); // if equipped send new old color too

	currchar[s]->removeHalo(pi);
}

void cTargets::MenuPrivTarg(int s)//LB's menu privs
{
	int i;
	char temp[512];

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	P_CHAR pc = FindCharBySerial(serial);
	if (pc!=NULL)
	{
		i=addid1[s];
		sprintf(temp,"Setting Menupriv number %i",i);
		sysmessage(s,temp);
		sprintf(temp,"Menupriv %i set by %s",i,currchar[s]->name.c_str());
		sysmessage(calcSocketFromChar(pc),temp);
		pc->setMenupriv(i);
	}
}

void cTargets::ShowSkillTarget(int s) // LB's showskills
{
	int a,j,k,b=0,c,z,zz,ges=0;
	char skill_info[(ALLSKILLS+1)*40];
	char sk[25];

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		z=addx[s];
		if (z<0 || z>3) z=0;
		if (z==2 || z==3)
			sprintf(skill_info, "%s's skills:", pc->name.c_str());
		else
			sprintf(skill_info, "%s's baseskills:", pc->name.c_str());

		b=pc->name.size()+11;
		if (b>23) b=23;

		for (c=b;c<=26;c++)
			strcpy(&skill_info[strlen(skill_info)], " ");

		numtostr(ges,sk);
		sprintf((char*)temp,"sum: %s",sk);
		strcpy(&skill_info[strlen(skill_info)],(char*)temp);

		for (a=0;a<ALLSKILLS;a++)
		{
			if (z==0 || z==1) k=pc->baseSkill(a); else k=pc->skill(a);
			if (z==0 || z==2) zz=9; else zz=-1;

			if (k>zz) // show only if skills >=1
			{
				if (z==2 || z==3) j=pc->skill(a)/10; else j=pc->baseSkill(a)/10;	// get skill value
				numtostr(j,sk);		// skill-value string in sk
				ges+=j;
				sprintf((char*)temp, "%s %s", skillname[a],sk);
				strcpy(&skill_info[strlen(skill_info)],(char*)temp);

				b=strlen(skillname[a])+strlen(sk)+1; // it doesnt like \n's, so insert spaces till end of line
				if (b>23) b=23;
				for (c=b;c<=26;c++)
					 strcpy(&skill_info[strlen(skill_info)], " ");
			}
		}
		numtostr(ges,sk);
		sprintf((char*)temp,"sum: %s  ",sk);
		strcpy(&skill_info[strlen(skill_info)],(char*)temp);

		SndUpdscroll(s, strlen(skill_info), skill_info);
	}
	else
		sysmessage(s,"no valid target");
}

void cTargets::FetchTarget(UOXSOCKET s) // Ripper
{
	sysmessage(s,"Fetch is not available at this time.");
}

void cTargets::GuardTarget( UOXSOCKET s )
{
	P_CHAR pPet = FindCharBySerial(addx[s]);
	if (pPet == NULL)
	{
		sysmessage(s, "Currently can't guard anyone but yourself.."); 
		LogError("Lost pet serial");
		return;
	}

	P_CHAR pToGuard = FindCharBySerPtr(buffer[s]+7);
	if( !pToGuard || !pToGuard->Owns(pPet) )
	{
		sysmessage( s, "Currently can't guard anyone but yourself!" );
		return;
	}
	pPet->setNpcAIType( 32 ); // 32 is guard mode
	pPet->ftarg = currchar[s]->serial;
	pPet->npcWander=1;
	sysmessage(s, "Your pet is now guarding you.");
	currchar[s]->setGuarded(true);
}

void cTargets::ResurrectionTarget( UOXSOCKET s )
{
	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->dead)
		{
			Targ->NpcResurrectTarget(pc);
			return;
		}
	}
}

//AntiChrist - shows the COMMENT line in the account section of player current acct.
void cTargets::ShowAccountCommentTarget(int s)
{
	int j,accountfound=0,commentfound=0;
	temp2[0] = '\0';

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	P_CHAR pc = FindCharBySerial(serial);
	if(pc != NULL)
	{
		if(pc->account()==-1)
		{
			sysmessage(s,"No account available for that character.");
			return; //only if char has an account
		}

		openscript("accounts.adm");
		unsigned long loopexit=0;
		do
		{
			read2();
			if (!(strcmp((char*)script1, "SECTION")))
			{
				j=str2num(script2);
				if(j==pc->account()) accountfound=1;//we are in the right section
			}

			if (!(strcmp((char*)script1, "COMMENT")))
			{
				if(accountfound)
				{
					strcpy((char*)temp2,(char*)script2);
					commentfound=1;
					break;
				}
			}
		}
		while ((strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS));

		if(accountfound==0)
		{
			sysmessage(s,"Can't find character's account in ACCOUNTS.ADM file!!!");
			closescript();
			return;
		}

		if(commentfound==0 && strlen((char*)temp2)>0)
		{
			sysmessage(s,"No comment available for that character's account.");
			closescript();
			return;
		}

		sysmessage(s,"Account Comment: %s",temp2);
		closescript();
	}
}

void cTargets::SetHome(int s)
{
}

void cTargets::SetWork(int s)
{
}

void cTargets::SetFood(int s)
{
}

static void ItemTarget(P_CLIENT ps, PKGx6C *pt)
{
	UOXSOCKET s=ps->GetSocket();
	P_ITEM pi=FindItemBySerial(pt->Tserial);
	if (pi==NULL) return;
	switch(pt->Tnum)
	{
	case 10://MoreTarget
		pi->more1=addid1[s];
		pi->more2=addid2[s];
		pi->more3=addid3[s];
		pi->more4=addid4[s];
		RefreshItem(pi);
		break;
	case  28://MovableTarget
	case 111://yes, it's duplicate
		pi->magic=addx[s];
		RefreshItem(pi);
		break;
	case 31://ColorsTarget
		if (pi->id()==0x0FAB ||						//dye vat
			pi->id()==0x0EFF || pi->id()==0x0E27 )	//hair dye
			SndDyevat(s,pi->serial,pi->id());
		else
			sysmessage(s, "You can only use this item on a dye vat.");
		break;
	case 63://MoreXTarget
		pi->morex=addx[s];
		break;
	case 64://MoreYTarget
		pi->morey=addx[s];
		break;
	case 65://MoreZTarget
		pi->morez=addx[s];
		break;
	case 66://MoreXYZTarget
		pi->morex=addx[s];
		pi->morey=addy[s];
		pi->morez=addz[s];
		break;
	case 89://ObjPrivTarget
		if (addid1[s]==0) pi->priv=pi->priv&0xFE; // lb ...
		if (addid1[s]==1) pi->priv=pi->priv|0x01;
		if (addid1[s]==3) pi->priv=addid2[s];
		break;
	case 122://SetValueTarget
		pi->value=addx[s];
		break;
	case 123://SetRestockTarget
		pi->restock=addx[s];
		break;
	case 129://SetAmount2Target
		if (addx[s] > 64000) //Ripper..to fix a client bug for over 64k.
		{
			sysmessage(s, "No amounts over 64k in a pile!");
			return;
		}
		pi->setAmount2( addx[s] );
		RefreshItem(pi);
		break;
	case 133://SetWipeTarget
		pi->setWipe( addid1[s] != 0 ? true : false );
		RefreshItem(pi);
		break;
	}
}

void cTargets::LoadCannon(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(serial);
	if (pi != NULL)
	{
		//if((pi->id1==0x0E && pi->id2==0x91) && pi->morez==0)
		if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
			(pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
			(addid1[s]==(unsigned char)'\xFF'))
		{
			if ((pi->morez==0)&&(iteminrange(s, pi, 2)))
			{
				if(pi->morez==0)
				pi->setType( 15 );
				pi->morex=8;
				pi->morey=10;
				pi->morez=1;
				sysmessage(s, "You load the cannon.");
			}
			else
			{
				if (pi->more1=='\x00') sysmessage(s, "That doesn't work in cannon.");
				else sysmessage(s, "That object doesn't fit into cannon.");
			}
		}
	}
}

void cTargets::DupeTarget(int s)
{
	if (addid1[s]>=1)
	{
		int serial=LongFromCharPtr(buffer[s]+7);
		P_ITEM pi = FindItemBySerial(serial);
		if (pi != NULL)
		{
			for (int j=0;j<addid1[s];j++)
			{
				Commands->DupeItem(s, pi, 1); // lb bugfix
				sysmessage(s,"DupeItem done.");//AntiChrist
			}
		}
	}
}

void cTargets::MoveToBagTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(serial);
	if (pi == NULL) return;
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pBackpack = Packitem(pc_currchar);
	if(pBackpack == NULL) return;
	
	pi->setContSerial(pBackpack->serial);
	pi->pos.x=50+rand()%80;
	pi->pos.y=50+rand()%80;
	pi->pos.z=9;
	pi->setLayer( 0x00 );
	pi->decaytime=0;//reset decaytimer
	
	SndRemoveitem(pi->serial);
	RefreshItem(pi);
}

void cTargets::MultiTarget(P_CLIENT ps) // If player clicks on something with the targetting cursor
{
//	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF)
//		return; // do nothing if user cancels, avoids CRASH! - Morrolan
// Duke: Nonsens !! this also happens when you target the backpack on the paperdoll !

	UOXSOCKET s=ps->GetSocket();
	targetok[s]=0;

	PKGx6C tbuf, *pt=&tbuf;
	pt->Tnum=buffer[s][5];
	pt->Tserial=LongFromCharPtr(buffer[s]+7);
	pt->TxLoc=ShortFromCharPtr(buffer[s]+11);
	pt->TyLoc=ShortFromCharPtr(buffer[s]+13);
	pt->TzLoc=buffer[s][16];
	pt->model=ShortFromCharPtr(buffer[s]+17);

	// targetRequest
	if( ( buffer[s][2] == 0 ) && ( buffer[s][3] == 0 ) && ( buffer[s][4] == 0 ) && ( buffer[s][5] == 1 ) )
	{
		// Unrequested Target info
		if( targetRequests.find( s ) == targetRequests.end() )
		{
			// Should not happen too often
			sysmessage( s, "Unrequested target info" );
			return;
		}

		// If the user cancels call timedout
		if( pt->TxLoc == -1 && pt->TyLoc == -1 && pt->Tserial == 0 && pt->model == 0 )
			targetRequests[ s ]->timedout( s );
		else
			targetRequests[ s ]->responsed( s, *pt );

		delete targetRequests[ s ];
		targetRequests.erase( targetRequests.find( s ) );
	}

	if (pt->TxLoc==-1 && pt->TyLoc==-1) // do nothing if user cancelled
		if (pt->Tserial==0 && pt->model==0) // this seems to be the complete 'cancel'-criteria (Duke)
			return;

	if ((buffer[s][2]==0)&&(buffer[s][3]==1)&&(buffer[s][4]==0))
	{
		bool Iready=false, Cready=false;
		P_ITEM pi = NULL;
		P_CHAR pc = NULL;
		if(buffer[s][7]>=0x40) // an item's serial ?
		{
			pi=FindItemBySerial(pt->Tserial);
			if (pi!=NULL)
				Iready=true;
		}
		else
		{
			pc=FindCharBySerial(pt->Tserial);
			if (pc!=NULL)
			{
				Cready=true;
			}
		}

		switch(pt->Tnum)
		{
		case 0: AddTarget(s,pt); break;
		case 1: { cRenameTarget		T(ps);		T.process();} break;
		case 2: TeleTarget(s,pt); break; // LB, bugfix, we need it for the /tele command
		case 3: { cRemoveTarget		T(ps);		T.process();} break;
		case 4: DyeTarget(s); break;
		case 5: { cNewzTarget		T(ps);		T.process();} break;
		case 6: if (Iready) pi->setType( addid1[s] ); break; //Typetarget
		case 7: Targ->IDtarget(s); break;
		case 8:	XgoTarget(s); break;
		case 9: if (Cready) PrivTarget(s,pc); break;
		case 10: ItemTarget(ps,pt); break;//MoreTarget
		case 11: if (Iready) KeyTarget(s,pi); break;
		case 12: Targ->IstatsTarget(s); break;
		case 13: if (Cready) CstatsTarget(ps,pc); break;
		case 14: if (Cready) GMTarget(ps,pc); break;
		case 15: if (Cready) CnsTarget(ps,pc); break;
		case 16: if (Cready) KillTarget(pc, 0x0b); break;
		case 17: if (Cready) KillTarget(pc, 0x10); break;
		case 18: if (Cready) KillTarget(pc, 0x15); break;
		case 19: if (Cready) pc->fonttype=addid1[s]; break;
		case 20: Targ->GhostTarget(s); break;
		case 21: Targ->ResurrectionTarget(s); break; // needed for /resurrect command
		case 22: { cBoltTarget		T(ps);	T.process();} break;
		case 23: { cSetAmountTarget	T(ps);	T.process();} break;
		case 24:
			{
				SERIAL serial=LongFromCharPtr(buffer[s]+7);
				P_ITEM pi = FindItemBySerial(serial);
				P_CHAR pc_currchar = currchar[s];
				if ( pi != NULL )
				{
					Trig->triggerwitem(s, pi, 0);
					pc_currchar->setEnvokeid(0x00);
					return;
				}
				// Checking if target is an NPC	--- By Magius(CHE) §
				P_CHAR pc_i = FindCharBySerial(serial);
				if(pc_i != NULL)
				{
					Trig->triggernpc(s, pc_i, 0);
					pc_currchar->setEnvokeid(0x00);
					return;
				}
				// End Addons by Magius(CHE) §
				Trig->triggerwitem(s, NULL, 0);
				pc_currchar->setEnvokeid(0x00);
				return;
			}
		case 25: Targ->CloseTarget(s); break;
		case 26: Targ->AddItem( s ); break;
		case 27: Targ->NpcMenuTarget(s); break;
		case 28: ItemTarget(ps,pt); break;//MovableTarget
		case 29: Skills->ArmsLoreTarget(s); break;
		case 30: if (Cready) OwnerTarget(ps,pc); else if (Iready) OwnerTarget(ps,pi); break;
		case 31: ItemTarget(ps,pt); break;//ColorsTarget
		case 32: Targ->DvatTarget(s); break;
		case 33: AddNpcTarget(s,pt); break;
		case 34: if (Cready) pc->priv2|=2; break;
		case 35: if (Cready) pc->priv2&=0xfd; break; // unfreeze, AntiChris used LB bugfix
		case 36: Targ->AllSetTarget(s); break;
		case 37: Skills->AnatomyTarget(s); break;
		case 38: Magic->Recall(s); break;
		case 39: Magic->Mark(s); break;
		case 40: Skills->ItemIdTarget(s); break;
		case 41: Skills->Evaluate_int_Target(s); break;
		case 42: Skills->TameTarget(s); break;
		case 43: Magic->Gate(s); break;
		case 44: Magic->Heal(s); break; // we need this for /heal command
		case 45: Fishing->FishTarget(ps); break;
		case 46: InfoTarget(s,pt); break;
		case 47: if (Cready) pc->setTitle( xtext[s] ); break;//TitleTarget
		case 48: Targ->ShowAccountCommentTarget(s); break;
		case 49: Skills->CookOnFire(s,0x09,0x7B,"fish steaks"); break;
		case 50: Skills->Smith(s); break;
		case 51: Skills->Mine(s); break;
		case 52: Skills->SmeltOre(s); break;
		case 53: npcact(s); break;
		case 54: Skills->CookOnFire(s,0x09,0xB7,"bird"); break;
		case 55: Skills->CookOnFire(s,0x16,0x0A,"lamb"); break;
		case 56: Targ->NpcTarget(s); break;
		case 57: Targ->NpcTarget2(s); break;
		case 58: Targ->NpcResurrectTarget(currchar[s]); break;
		case 59: Targ->NpcCircleTarget(s); break;
		case 60: Targ->NpcWanderTarget(s); break;
		case 61: Targ->VisibleTarget(s); break;
		case 62: Targ->TweakTarget(s); break;
		case 63: //MoreXTarget
		case 64: //MoreYTarget
		case 65: //MoreZTarget
		case 66: ItemTarget(ps,pt); break;//MoreXYZTarget
		case 67: Targ->NpcRectTarget(s); break;
		case 68: Skills->CookOnFire(s,0x09,0xF2,"ribs"); break;
		case 69: Skills->CookOnFire(s,0x16,0x08,"chicken legs"); break;
		case 70: Skills->TasteIDTarget(s); break;
		case 71: if (Iready) ContainerEmptyTarget1(ps,pi); break;
		case 72: if (Iready) ContainerEmptyTarget2(ps,pi); break;
		case 75: Targ->TargIdTarget(s); break;
		case 76: AxeTarget(ps,pt); break;
		case 77: Skills->DetectHidden(s); break;

		case 79: Skills->ProvocationTarget1(s); break;
		case 80: Skills->ProvocationTarget2(s); break;
		case 81: Skills->EnticementTarget1(s); break;
		case 82: Skills->EnticementTarget2(s); break;

		case 86: Targ->SwordTarget(ps,pt); break;
		case 87: Magic->SbOpenContainer(s); break;
		case 88: Targ->SetDirTarget(s); break;
		case 89: ItemTarget(ps,pt); break;//ObjPrivTarget

		case 100: Magic->NewCastSpell( s ); break;	// we now have this as our new spell targeting location

		case 105: Targ->xSpecialBankTarget(s); break;//AntiChrist
		case 106: Targ->NpcAITarget(s); break;
		case 107: Targ->xBankTarget(s); break;
		case 108: Skills->AlchemyTarget(s); break;
		case 109: Skills->BottleTarget(s); break;
		case 110: Targ->DupeTarget(s); break;
		case 111: ItemTarget(ps,pt); break;//MovableTarget
		case 112: Targ->SellStuffTarget(s); break;
		case 113: Targ->ManaTarget(s); break;
		case 114: Targ->StaminaTarget(s); break;
		case 115: Targ->GmOpenTarget(s); break;
		case 116: Targ->MakeShopTarget(s); break;
		case 117: Targ->FollowTarget(s); break;
		case 118: Targ->AttackTarget(s); break;
		case 119: Targ->TransferTarget(s); break;
		case 120: Targ->GuardTarget( s ); break;
		case 121: Targ->BuyShopTarget(s); break;
		case 122: ItemTarget(ps,pt); break;//SetValueTarget
		case 123: ItemTarget(ps,pt); break;//SetRestockTarget
		case 124: Targ->FetchTarget(s); break;

		case 126: Targ->JailTarget(s,-1); break;
		case 127: Targ->ReleaseTarget(s,-1); break;
		case 128: Skills->CreateBandageTarget(s); break;
		case 129: ItemTarget(ps,pt); break;//SetAmount2Target
		case 130: Skills->HealingSkillTarget(s); break;
		case 131: if (currchar[s]->isGM()) Targ->permHideTarget(s); break; /* not used */
		case 132: if (currchar[s]->isGM()) Targ->unHideTarget(s); break; /* not used */
		case 133: ItemTarget(ps,pt); break;//SetWipeTarget
		case 134: Skills->Carpentry(s); break;
		case 135: Targ->SetSpeechTarget(s); break;
		case 136: Targ->XTeleport(s,0); break;

		case 150: SetSpAttackTarget(s); break;
		case 151: Targ->FullStatsTarget(s); break;
		case 152: Skills->BeggingTarget(s); break;
		case 153: Skills->AnimalLoreTarget(s); break;
		case 154: Skills->ForensicsTarget(s); break;
		case 155:
			{
				currchar[s]->setPoisonserial(LongFromCharPtr(buffer[s]+7));
				target(s, 0, 1, 0, 156, "What item do you want to poison?");
				return;
			}
		case 156: Skills->PoisoningTarget(s); break;

		case 160: Skills->Inscribe(s,0); break;

		case 162: Skills->LockPick(s); break;

		case 164: Skills->Wheel(s, YARN); break;
		case 165: Skills->Loom(s); break;
		case 166: Skills->Wheel(s, THREAD); break;
		case 167: Skills->Tailoring(s); break;

		case 170: Targ->LoadCannon(s); break;
		case 172: Skills->Fletching(s); break;
		case 173: Skills->MakeDough(s); break;
		case 174: Skills->MakePizza(s); break;
		case 175: Targ->SetPoisonTarget(s); break;
		case 176: Targ->SetPoisonedTarget(s); break;
		case 177: Targ->SetSpaDelayTarget(s); break;
		case 178: Targ->SetAdvObjTarget(s); break;
		case 179: if (Cready) SetInvulFlag(ps,pc); break;
		case 180: Skills->Tinkering(s); break;
		case 181: Skills->PoisoningTarget(s); break;

		case 183: Skills->TinkerAxel(s); break;
		case 184: Skills->TinkerAwg(s); break;
		case 185: Skills->TinkerClock(s); break;
		case 186: vialtarget(s); break;

		case 198: Tiling(s,pt); break;
		case 199: Targ->Wiping(s); break;
		case 200: Commands->SetItemTrigger(s); break;
		case 201: Commands->SetNPCTrigger(s); break;
		case 202: Commands->SetTriggerType(s); break;
		case 203: Commands->SetTriggerWord(s); break;
		case 204: triggertarget(s); break; // Fixed by Magius(CHE)
		case 205: Skills->StealingTarget(s); break;
		case 206: Targ->CanTrainTarget(s); break;
		case 207: ExpPotionTarget(s,pt); break;
		case 209: Targ->SetSplitTarget(s); break;
		case 210: Targ->SetSplitChanceTarget(s); break;
		case 212: Commands->Possess(s); break;
		case 213: Skills->PickPocketTarget(s); break;

		case 220: 
			{
				cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(currchar[s]->guildstone()));
				if ( pStone != NULL )
					pStone->Recruit(s);			
			}
			break;
		case 221:
			{
				cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(currchar[s]->guildstone()));
				if ( pStone != NULL )
			//		pStone->TargetWar(s);
				sysmessage(s, "Sorry, currently disabled");
			}
			break;
		case 222: TeleStuff(s,pt); break;
		case 223: Targ->SquelchTarg(s); break;//Squelch
		case 224: Targ->PlVBuy(s); break;//PlayerVendors
		case 227: Targ->HouseOwnerTarget(s); break; // cj aug11/99
		case 228: Targ->HouseEjectTarget(s); break; // cj aug11/99
		case 229: Targ->HouseBanTarget(s); break; // cj aug12/99
		case 230: Targ->HouseFriendTarget(s); break; // cj aug 12/99
		case 231: Targ->HouseUnBanTarget(s); break; // cj aug 12/99
		case 232: Targ->HouseLockdown( s ); break; // Abaddon 17th December 1999
		case 233: Targ->HouseRelease( s ); break; // Abaddon 17th December 1999
		case 234: Targ->HouseSecureDown( s ); break; // Ripper
		case 235: Targ->BanTarg(s); break;
		case 236: Skills->RepairTarget(s); break; //Ripper..Repairing item
		case 237: Skills->SmeltItemTarget(s); break; //Ripper..Smelting item
		//taken from 6904t2(5/10/99) - AntiChrist
		case 240: Targ->SetMurderCount( s ); break; // Abaddon 13 Sept 1999

		case 245: BuildHouse(s,addid3[s]);	 break;

		case 247: Targ->ShowSkillTarget(s);break; //showskill target
		case 248: Targ->MenuPrivTarg(s);break; // menupriv target
		case 249: Targ->UnglowTaget(s);break; // unglow
		case 251: Targ->NewXTarget(s); break; // NEWX
		case 252: Targ->NewYTarget(s); break; // NEWY
		case 253: Targ->IncXTarget(s); break; // INCX
		case 254: Targ->IncYTarget(s); break; // INCY
		case 255: Targ->GlowTarget(s); break; // glow
		case 256: Targ->SetHome(s); break;
		case 257: Targ->SetWork(s); break;
		case 258: Targ->SetFood(s); break;
		case 259: Targ->HouseUnFriendTarget(s); break;

		default:
			LogErrorVar("Fallout of switch statement, multitarget(), value=(%i)",pt->Tnum);
		}
	}
}

void cTargets::AddItem( UOXSOCKET s )
{
	if( s == -1 )
		return;

	QString ItemID = xtext[ s ];
	
	QDomElement *ItemNode = DefManager->getSection( WPDT_ITEM, ItemID );
	
	// No Item found...
	if( ItemNode->isNull() )
	{
		sysmessage( s, "There is no such item '%s'", xtext[ s ] );
		return;
	}

	// ...Otherwise get x+y+z coordinates first
	UI16 TargetX, TargetY;
	SI08 TargetZ;

	TargetX = ( buffer[s][11] << 8 ) + buffer[s][12];
	TargetY = ( buffer[s][13] << 8 ) + buffer[s][14];
	TargetZ = buffer[s][16] + Map->TileHeight( ( buffer[s][17] << 8) + buffer[s][18] );

	P_ITEM Item = Items->createScriptItem( ItemID );

	// No item created = fail
	if( Item == NULL )
	{
		sysmessage( s, "Unable to create item '%s'", xtext[ s ] );
		return;
	}

	Item->setContSerial( -1 );
	Item->MoveTo( TargetX, TargetY, TargetZ );
	RefreshItem( Item );
}
