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

//#include "wolfpack.h"
//#include "basics.h"

//#include "cmdtable.h" // who took this out and why?? cj 8/11/99
#include "targeting.h"
//#include "itemid.h"
//#include "SndPkg.h"

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
	int s,serial,inx;
	P_ITEM pi;
	P_CHAR pc;
	P_CLIENT ps;
	void makeSerial()		{serial=LongFromCharPtr(buffer[s]+7);}
	void makeCharIndex()	{inx=calcCharFromSer(serial);}
	void makeItemIndex()	{inx=calcItemFromSer(serial);}
public:
	cTarget(P_CLIENT pCli)	{s=pCli->GetSocket();}
	virtual void process() = 0;
};

class cCharTarget : public cTarget
{
public:
	cCharTarget(P_CLIENT pCli) : cTarget(pCli) {}
	virtual void CharSpecific() = 0;
	virtual void process()
	{
		makeSerial();
		makeCharIndex();
		if(inx > -1)
		{
			int err;
			pc = MAKE_CHARREF_LOGGED(inx,err);
			if (err) return;
			CharSpecific();
		}
		else
			sysmessage(s,"That is not a character.");
	}
};

class cItemTarget : public cTarget
{
public:
	cItemTarget(P_CLIENT pCli) : cTarget(pCli) {}
	virtual void ItemSpecific() = 0;
	virtual void process()
	{
		makeSerial();
		makeItemIndex();
		if(inx > -1)
		{
			pi = MAKE_ITEMREF_LR(inx);
			ItemSpecific();
		}
		else
			sysmessage(s,"That is not an item.");
	}
};

class cWpObjTarget : public cTarget
{
public:
	cWpObjTarget(P_CLIENT pCli) : cTarget(pCli) {}
	virtual void CharSpecific() = 0;
	virtual void ItemSpecific() = 0;
	virtual void process()
	{
		makeSerial();
		if(buffer[s][7]>=0x40) // an item's serial ?
		{
			makeItemIndex();
			if(inx > -1)
			{
				pi = MAKE_ITEMREF_LR(inx);
				ItemSpecific();
			}
			else
				sysmessage(s,"That is not a valid item.");
		}
		else
		{
			makeCharIndex();
			if(inx > -1)
			{
				int err;
				pc = MAKE_CHARREF_LOGGED(inx,err);
				if (err) return;
				CharSpecific();
			}
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
	P_CHAR pc = MAKE_CHARREF_LR(v);
	if (pc->free) return;
	int cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	P_ITEM pBackpack = Packitem(pc_currchar);
	if (!pBackpack) {sysmessage(s,"Time to buy a backpack"); return; } //LB

	int serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi=FindItemBySerial(serial);		// the item
	if (pi==NULL) return;
	if (pi->isInWorld()) return;
	int price=pi->value;

	int np=calcItemFromSer(pi->contserial);	// the pack
	int npc=GetPackOwner(np);				// the vendor
	if(npc!=v || pc->npcaitype!=17) return;

	if (pc_currchar->Owns(pc))
	{
		npctalk(s, v, "I work for you, you need not buy things from me!",0);
		return;
	}

	int gleft=pc_currchar->CountGold();
	if (gleft<pi->value)
	{
		npctalk(s, v, "You cannot afford that.",0);
		return;
	}
	pBackpack->DeleteAmount(price,0x0EED);	// take gold from player

	npctalk(s, v, "Thank you.",0);
	pc->holdg+=pi->value; // putting the gold to the vendor's "pocket"

	// sends item to the proud new owner's pack
	pi->SetContSerial(pBackpack->serial);
	RefreshItem(pi);

}

////////////////
// name:		triggertarget
// history:		by Magius(CHE),24 August 1999
// Purpose:		Select an item or an npc to set with new trigger.
//
void cTargets::triggertarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)//Char
	{
			//triggerwitem(i,-1,1); is this used also for npcs?!?!
	} else
	{//item
		i=calcItemFromSer(serial);
		if(i!=-1)
		{
			triggerwitem(s,i,1);
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
			HouseManager->AddHome(s,addid3[s]);//If its a valid house, send it to buildhouse!
			return; // Morrolan, here we WANT fall-thru, don't mess with this switch
		}
	}
	int pileable=0;
	short id=(addid1[s]<<8)+addid2[s];
	tile_st tile;
	Map->SeekTile(id, &tile);
	if (tile.flag2&0x08) pileable=1;

	P_ITEM pi=Items->SpawnItem(currchar[s], 1, "#", pileable, id, 0,0);
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
	cRenameTarget(P_CLIENT pCli) : cWpObjTarget(pCli) {}
	void CharSpecific()
	{
		strcpy(pc->name,xtext[s]);
	}
	void ItemSpecific()
	{
		if(addx[s]==1) //rename2 //New -- Zippy
			strcpy(pi->name2,xtext[s]);
		else
			strcpy(pi->name,xtext[s]);
	}
};

//we don't need this anymore - AntiChrist (9/99)
// hehe, yes we do
// what about the /tele command ???
// modified for command only purpose and uncommented by LB...

// Changed to have a gmmove effect at origin and at target point 
// Aldur
// 
static void TeleTarget(int s, PKGx6C *pp) 
{ 
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return; 
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]); 
	int x=pp->TxLoc; 
	int y=pp->TyLoc; 
	signed char z=pp->TzLoc; 
	Coord_cl clTemp3(x,y,z) ;
	if ((pc_currchar->isGM()) || (line_of_sight( s, pc_currchar->pos, clTemp3,WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING))) 
	{ 
		doGmMoveEff(s); 
		
		pc_currchar->MoveTo(x,y,z+Map->TileHeight(pp->model)); 
		teleport(DEREF_P_CHAR(pc_currchar)); 
		
		doGmMoveEff(s); 
	} 
}

class cRemoveTarget : public cWpObjTarget
{
public:
	cRemoveTarget(P_CLIENT pCli) : cWpObjTarget(pCli) {}
	void CharSpecific()
	{
		if (pc->account>-1 && pc->isPlayer()) // player check added by LB
		{
			sysmessage(s,"You cant delete players");
			return;
		}
		sysmessage(s, "Removing character.");
		Npcs->DeleteChar(inx);
	}
	void ItemSpecific()
	{
		sysmessage(s, "Removing item.");
		Items->DeleItem(inx);
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
		int i=calcCharFromSer(serial);
		if (i!=-1)
		{
			P_CHAR pc = MAKE_CHARREF_LR(i);
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
				pi->color1=c1;
				pi->color2=c2;
			}
			RefreshItem(pi);
		}

		int i=calcCharFromSer(serial);
		if (i!=-1)
		{
			P_CHAR pc = MAKE_CHARREF_LR(i);
			body=(pc->id1<<8)+pc->id2;
			k=(addid1[s]<<8)+addid2[s];
			if( ( (k>>8) < 0x80 ) && body >= 0x0190 && body <= 0x0193 )
				k += 0x8000;

			b=k&0x4000;
			if (b==16384 && (body >=0x0190 && body<=0x03e1))
				k=0xf000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

			if (k!=0x8000) // 0x8000 also crashes client ...
			{
				pc->skin = pc->xskin = k;
				updatechar(i);
			}
		}
	}
}

class cNewzTarget : public cWpObjTarget
{
public:
	cNewzTarget(P_CLIENT pCli) : cWpObjTarget(pCli) {}
	void CharSpecific()
	{
		pc->dispz=pc->pos.z=addx[s];
		teleport(inx);
	}
	void ItemSpecific()
	{
		pi->pos.z=addx[s];
		RefreshItem(inx);
	}
};

//public !!
void cTargets::IDtarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	if (i!=-1)
	{
		P_ITEM pi=MAKE_ITEMREF_LR(i);
		pi->id1=addx[s];
		pi->id2=addy[s];
		RefreshItem(i);
		return;
	}
	i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->id1=addx[s];
		pc->id2=addy[s];
		pc->xid1=addx[s];
		pc->xid2=addy[s];
		updatechar(i);
	}
}

//public !!
void cTargets::XTeleport(int s, int x)
{
	int i, serial = INVALID_SERIAL;
	int cc = currchar[s];
	switch (x)
	{
		case 0:
			serial = LongFromCharPtr(buffer[s] + 7);
			i = calcCharFromSer(serial);
			break;
		case 2:
			if (perm[makenumber(1)])
			{
				i = currchar[makenumber(1)];
			}
			else 
				return;
			break;
		case 3:
			i = chars[cc].making;
			chars[currchar[i]].MoveTo(chars[cc].pos.x, chars[cc].pos.y, chars[cc].pos.z);
			teleport(currchar[i]);
			return;
		case 5:
			serial = calcserial(hexnumber(1), hexnumber(2), hexnumber(3), hexnumber(4));
			i = calcCharFromSer(serial);
			break;
	}
	
	if (i!=-1)
	{
		chars[i].MoveTo(chars[cc].pos.x, chars[cc].pos.y, chars[cc].pos.z);
		updatechar(i);
		return;// Zippy
	}
	i = calcItemFromSer(serial);
	if (i!=-1)
	{
		items[i].MoveTo(chars[cc].pos.x, chars[cc].pos.y, chars[cc].pos.z);
		RefreshItem(i);
	}
}

void XgoTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		chars[i].MoveTo(addx[s],addy[s],addz[s]);
		updatechar(i);
	}
}

static void PrivTarget(int s, P_CHAR pc)
{
	if (SrvParms->gm_log)	//Logging
	{
		sprintf((char*)temp, "%s.gm_log",chars[currchar[s]].name);
		sprintf((char*)temp2, "%s as given %s Priv [%x][%x]\n",chars[currchar[s]].name,pc->name,addid1[s],addid2[s]);
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
			if ( pi->type==7 && (iteminrange(s,pi,2) || (!pi->isInWorld()) ) )
			{
				if (!Skills->CheckSkill(currchar[s],TINKERING, 400, 1000))
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
			if (((pi->type==1)||(pi->type==63))&&(iteminrange(s,pi,2)))
			{
				if(pi->type==1) pi->type=8;
				if(pi->type==63) pi->type=64;
				sysmessage(s, "You lock the container.");
				return;
			}
			else if ((pi->type==7)&&(iteminrange(s,pi,2)))
			{
				chars[currchar[s]].inputitem=DEREF_P_ITEM(pi);
				chars[currchar[s]].inputmode=6;
				sysmessage(s,"Enter new name for key.");//morrolan rename keys
				return;
			}
			else if ((pi->type==8)||(pi->type==64)&&(iteminrange(s,pi,2)))
			{
				if(pi->type==8) pi->type=1;
				if(pi->type==64) pi->type=63;
				sysmessage(s, "You unlock the container.");
				return;
			}
			else if ((pi->type==12)&&(iteminrange(s,pi,2)))
			{
				pi->type=13;
				sysmessage(s, "You lock the door.");
				return;
			}
			else if ((pi->type==13)&&(iteminrange(s,pi,2)))
			{
				pi->type=12;
				sysmessage(s, "You unlock the door.");
				return;
			}
			else if (pi->id()==0x0BD2)
			{
				sysmessage(s, "What do you wish the sign to say?");
				chars[currchar[s]].inputitem=DEREF_P_ITEM(pi); //Morrolan sign kludge
				chars[currchar[s]].inputmode=6;
				return;
			}

			//Boats ->
			else if(pi->type==117 && pi->type2==3)
			{
				Boats->OpenPlank(DEREF_P_ITEM(pi));
				RefreshItem(pi);
			}
			//End Boats --^
		}//else if
		else
		{
			if (pi->type==7) sysmessage (s, "That key is not blank!");
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
			sprintf((char*)temp, "Item [Dynamic] Ser [%x %x %x %x] ID [%x %x] Name [%s] Name2 [%s] Color [%x %x] Cont %s Layer [%x] Type [%d] Magic [%x] More [%x %x %x %x] Position [%i %i %i] Amount [%i] Priv [%x]",
				pi->ser1,pi->ser2,pi->ser3,pi->ser4,pi->id1,pi->id2,
				pi->name,pi->name2,pi->color1,pi->color2,
				contstr,
				pi->layer,pi->type,pi->magic,
				pi->more1,pi->more2,pi->more3,pi->more4,
				pi->pos.x,pi->pos.y,pi->pos.z,pi->amount, pi->priv);
			sysmessage(s, (char*)temp);
			sprintf((char*)temp,"STR [%d] HP/MAX [%d/%d] Damage [%d-%d] Defence [%d] Rank [%d] Smelt [%d] SecureIt [%d] MoreXYZ [%i %i %i] Poisoned [%i] RaceHate [%i] Weight [%d] Owner [%x] Creator [%s] MadeValue [%i] Value [%i] Decaytime[%i] Decay [%i] GoodType[%i] RandomValueRate[%i]",
				pi->st, pi->hp,pi->maxhp, pi->lodamage, pi->hidamage,pi->def,pi->rank,pi->smelt,pi->secureIt,
				pi->morex, pi->morey, pi->morez,pi->poisoned,pi->racehate,
				pi->weight, pi->ownserial, // Ison 2-20-99
				pi->creator,pi->madewith,pi->value,int(double(int(pi->decaytime-uiCurrentTime)/MY_CLOCKS_PER_SEC)),(pi->priv)&0x01,pi->good,pi->rndvaluerate); // Magius(CHE) (2)
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
			if (pi && pi->magic != 4)
			{
				if (pi->name2 &&(strcmp(pi->name2, "#")))
					strcpy(pi->name, pi->name2);
				if (pi->name[0] == '#')
					pi->getName(temp2);
				else 
					strcpy((char*)temp2, pi->name);
				sprintf((char*)temp, "You found that this item appears to be called: %s", temp2);
				sysmessage(s, (char*) temp);
			}
		}			
		if (pi->type != 15)
		{
			if (pi->type != 404)
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
	CHARACTER c = DEREF_P_CHAR(pc);

	sprintf((char*)temp, "Ser [%x %x %x %x] ID [%x %x] Name [%s] Skin [%x] Account [%x] Priv [%x %x] Position [%i %i %i] CTimeout [%i] Fame [%i] Karma [%i] Deaths [%i] Kills [%i] NPCAI [%x] NPCWANDER [%d] WEIGHT [%.2f]",
		pc->ser1,pc->ser2,pc->ser3,pc->ser4,pc->id1,pc->id2,
		pc->name,pc->skin,
		pc->account,pc->getPriv(),pc->priv2,
		pc->pos.x,pc->pos.y,pc->pos.z, pc->timeout,
		pc->fame,pc->karma,pc->deaths,pc->kills,
		pc->npcaitype, pc->npcWander, pc->weight);
	sysmessage(s, (char*)temp);
	sprintf((char*)temp, "Other Info: Poisoned [%i] Poison [%i] Hunger [%i] Attacker Serial [%x] Target [%i] Carve[%i]", //Changed by Magius(CHE)
		pc->poisoned,pc->poison,pc->hunger,pc->attacker,pc->targ,pc->carve); //Changed by Magius(CHE)
	sysmessage(s, (char*)temp);
	Gumps->Open(s, c, 0, 8);
	statwindow(s, c);
}

static void MoveBelongingsToBp(P_CHAR pc, CHARACTER c)
{
	int n;
	P_ITEM pPack = Packitem(pc);
	if (pPack == NULL)
	{
		pc->packitem=n=Items->SpawnItem(calcSocketFromChar(c),c,1,"#",0,0x0E,0x75,0,0,0,0);
		pPack=MAKE_ITEMREF_LR(n);
		if(n==-1) return;
		pPack->SetContSerial(chars[c].serial);
		pPack->layer=0x15;
		pPack->type=1;
		pPack->dye=1;
	}

	int ci=0, loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer!=0x15 && pi->layer!=0x1D &&
			pi->layer!=0x10 && pi->layer!=0x0B && (pi->free==0))
		{
			if ((pi->trigon==1) && (pi->trigtype==2) && (pi->layer<19))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
			{
			triggerwitem(c, DEREF_P_ITEM(pi), 1); // trigger is fired
			}
			pi->pos.x=(rand()%80)+50;
			pi->pos.y=(rand()%80)+50;
			pi->pos.z=9;
			pi->SetContSerial(pPack->serial);
			pi->layer=0x00;
			SndRemoveitem(pi->serial);
			RefreshItem(pi);
		}
		else if (pc->Wears(pi) &&
			(pi->layer==0x0B || pi->layer==0x10))	// hair & beard (Duke)
		{
			Items->DeleItem(pi);
		}
	}
	updatechar(c);
}

static void GMTarget(P_CLIENT ps, P_CHAR pc)
{
	UOXSOCKET s = ps->GetSocket();
	CHARACTER c = DEREF_P_CHAR(pc);
	int i;	
	if (SrvParms->gm_log)
	{
		sprintf((char*)temp, "%s.gm_log",chars[currchar[s]].name);
		sprintf((char*)temp2, "%s has made %s a GM.\n",chars[currchar[s]].name,pc->name);
		savelog((char*)temp2, (char*)temp);
	}
	unmounthorse(calcSocketFromChar(c));	//AntiChrist bugfix
	
	pc->id1=0x03;
	pc->id2='\xDB';
	pc->skin = 0x8021;
	pc->xid1=0x03;
	pc->xid2='\xDB';
	pc->xskin = 0x8021;
	pc->setPriv(0xF7);
	pc->priv2 = (unsigned char) (0xD9);
	pc->gmrestrict = 0; // By default, let's not restrict them.
	
	for (i = 0; i < 7; i++) // this overwrites all previous settings !
	{
		pc->priv3[i]=metagm[3][i]; // gm defaults
		//pc->menupriv=1;
		if (pc->account==0) pc->priv3[i]=0xffffffff;
		pc->menupriv=-1; // LB, disabling menupriv stuff for gms per default
	}
	
	for (i = 0; i < TRUESKILLS; i++)
	{
		pc->baseskill[i]=1000;
		pc->skill[i]=1000;
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
	
	if (strncmp(pc->name, "GM", 2))
	{
		sprintf((char*)temp, "GM %s", pc->name);
		strcpy(pc->name,(char*)temp);
	}
	MoveBelongingsToBp(pc,c);
}

static void CnsTarget(P_CLIENT ps, P_CHAR pc)
{
	UOXSOCKET s = ps->GetSocket();
	CHARACTER c = DEREF_P_CHAR(pc);

	if (SrvParms->gm_log)
	{
		// logging
		sprintf((char*)temp, "%s.gm_log",chars[currchar[s]].name);
		sprintf((char*)temp2, "%s has made %s a Counselor.\n",chars[currchar[s]].name,pc->name);
		savelog((char*)temp2, (char*)temp);
	}
	pc->id1=0x03;
	pc->id2='\xDB';
	pc->skin=0x8003;
	pc->xid1=0x03;
	pc->xid2='\xDB';
	pc->xskin=0x8002;
	pc->setPriv(0xB6);
	pc->priv2='\x8D';
	if (strncmp(pc->name, "Counselor", 9))
	{
		sprintf((char*)temp, "Counselor %s", pc->name);
		strcpy(pc->name,(char*)temp);
	}
	for (int u=0;u<7;u++) // this overwrites all previous settigns !!!
	{
		pc->priv3[u]=metagm[1][u]; // counselor defaults
		//pc->menupriv=4;
		if (pc->account==0) pc->priv3[u]=0xffffffff;
	}
	MoveBelongingsToBp(pc,c);
}

static void KillTarget(P_CHAR pc, int ly)
{
	int ci=0, loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==ly)
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
			P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
			pc->attacker=pc_currchar->serial; //AntiChrist -- for forensics ev
			bolteffect(DEREF_P_CHAR(pc), true);
			soundeffect2(DEREF_P_CHAR(pc), 0x00, 0x29);
			deathstuff(DEREF_P_CHAR(pc));
		}
		else
			sysmessage(s,"That player is already dead.");
	}
}

class cBoltTarget : public cCharTarget
{
public:
	cBoltTarget(P_CLIENT pCli) : cCharTarget(pCli) {}
	void CharSpecific()
	{
		if (w_anim[0]==0 && w_anim[1]==0)
		{
			bolteffect(inx, true);
			soundeffect2(inx, 0x00, 0x29);
		}
		else
		{
			for (int j=0;j<=333;j++) bolteffect2(inx,w_anim[0],w_anim[1]);
		}
	}
};

class cSetAmountTarget : public cItemTarget
{
public:
	cSetAmountTarget(P_CLIENT pCli) : cItemTarget(pCli) {}
	void ItemSpecific()
	{
		if (addx[s] > 64000) //Ripper..to fix a client bug for over 64k.
		{
			sysmessage(s, "No amounts over 64k in a pile!");
			return;
		}
		this->pi->amount=addx[s];
		RefreshItem(inx);
	}
};

void cTargets::CloseTarget(int s)
{
	int j;
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if(i!=-1)
	{
		j=calcSocketFromChar(i);
		if(j>-1)
		{
			sysmessage(s, "Kicking player");
			sysmessage(j, "You have been kicked!"); //New -- Zippy
			Network->Disconnect(j);
		}
	}
}

// public !!!
int cTargets::AddMenuTarget(int s, int x, int addmitem) //Tauriel 11-22-98 updated for new items
{
	int c;
	if (s>=0)
		if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return -1;

	c=Items->CreateScriptItem(s, addmitem, 0);
	if (c==-1) return -1;
	if (x)
		RefreshItem(c);
	return c;
}

// public !!!
int cTargets::NpcMenuTarget(int s)
{
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return -1;
	return Npcs->AddRespawnNPC(s,addmitem[s],0);
}

void cTargets::VisibleTarget (int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))//item
	{
		P_ITEM pi = MAKE_ITEMREF_LR(calcItemFromSer(serial));
		if(pi != NULL)
		{
			pi->visible=addx[s];
			RefreshItem(DEREF_P_ITEM(pi));
		}
	} else
	{//char
		P_CHAR pc = MAKE_CHARREF_LR(calcCharFromSer(serial));
		if(pc != NULL)
		{
			pc->hidden=addx[s];
			updatechar(DEREF_P_CHAR(pc));
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
	if (isItemSerial(pi->serial) && pi->type == 1)
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
	if (pNewCont->type==1)
	{
		P_ITEM pi;	// item to move from old container
		int ci=0,loopexit=0;
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
		pc->tamed=false;
	}
	else
	{
		pc->tamed=true;
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
		P_CHAR pc=GetPackOwner(pi);
		if(DEREF_P_CHAR(pc)==currchar[s] || pi->isInWorld())
		{//if on ground or currchar is owning the item - AntiChrist
			pi->color1=addid1[s];
			pi->color2=addid2[s];
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
	int c=Npcs->MemCharFree ();
	P_CHAR pc = MAKE_CHARREF_LR(c);
	pc->Init();
	strcpy(pc->name, "Dummy");
	pc->id1=addid1[s];
	pc->id2=addid2[s];
	pc->xid1=addid1[s];
	pc->xid2=addid2[s];
	pc->skin = 0;
	pc->xskin = 0;
	pc->setPriv(0x10);
	pc->pos.x=pp->TxLoc;
	pc->pos.y=pp->TyLoc;
	pc->dispz=pc->pos.z=pp->TzLoc+Map->TileHeight(pp->model);
	mapRegions->Add(pc); // add it to da regions ...
	pc->npc=1;
	updatechar(c);
}

void cTargets::AllSetTarget(int s)
{
	int j, k;

	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHARREF_LR(i);
	if(pc != NULL)
	{
		k=calcSocketFromChar(DEREF_P_CHAR(pc));
		if (addx[s]<TRUESKILLS)
		{
			pc->baseskill[addx[s]]=addy[s];
			Skills->updateSkillLevel(DEREF_P_CHAR(pc), addx[s]);
			if (k!=-1) updateskill(k, addx[s]);
		}
		else if (addx[s]==ALLSKILLS)
		{
			for (j=0;j<TRUESKILLS;j++)
			{
				pc->baseskill[j]=addy[s];
				Skills->updateSkillLevel(DEREF_P_CHAR(pc), j);
				if (k!=-1) updateskill(k,j);
			}
		}
		else if (addx[s]==STR)
		{
			pc->st=addy[s];
			for (j=0;j<TRUESKILLS;j++)
			{
				Skills->updateSkillLevel(DEREF_P_CHAR(pc),j);
				if (k!=-1) updateskill(k,j);
			}
			if (k!=-1) statwindow(k,DEREF_P_CHAR(pc));
		}
		else if (addx[s]==DEX)
		{
			pc->setDex(addy[s]);
			for (j=0;j<TRUESKILLS;j++)
			{
				Skills->updateSkillLevel(DEREF_P_CHAR(pc),j);
				if (k!=-1) updateskill(k,j);
			}
			if (k!=-1) statwindow(k,DEREF_P_CHAR(pc));
		}
		else if (addx[s]==INT)
		{
			pc->in=addy[s];
			for (j=0;j<TRUESKILLS;j++)
			{
				Skills->updateSkillLevel(DEREF_P_CHAR(pc),j);
				if (k!=-1) updateskill(k,j);
			}
			if (k!=-1) statwindow(k,DEREF_P_CHAR(pc));
		}
		else if (addx[s]==FAME)
		{
			pc->fame=addy[s];
		}
		else if (addx[s]==KARMA)
		{
			pc->karma=addy[s];
		}
	}
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

	int c,pileable=0;
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
	if (tile.flag2&0x08) pileable=1;
	for (x=x1;x<=x2;x++)
		for (y=y1;y<=y2;y++)
		{
			P_ITEM pi=Items->SpawnItem(currchar[s], 1, "#", pileable, id, 0, 0);
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
	unsigned int i ;
	if (addid1[s]==1)
	{ // addid1[s]==1 means to inverse wipe
		for ( i=0;i<itemcount;i++)
		{
			P_ITEM pi=MAKE_ITEMREF_LR(i);
			if (!(pi->pos.x>=x1 && pi->pos.x<=x2 && pi->pos.y>=y1 && pi->pos.y<=y2) && pi->isInWorld() && pi->wipe==0)
				Items->DeleItem(i);
		}
	}
	else
	{
		for (i=0;i<itemcount;i++)
		{
			P_ITEM pi=MAKE_ITEMREF_LR(i);
			if (pi->pos.x>=x1 && pi->pos.x<=x2 && pi->pos.y>=y1 && pi->pos.y<=y2 && pi->isInWorld() && pi->wipe==0)
				Items->DeleItem(i);
		}
	}
}

static void ExpPotionTarget(int s, PKGx6C *pp) //Throws the potion and places it (unmovable) at that spot
{
	int x, y, z;
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	x=pp->TxLoc;
	y=pp->TyLoc;
	z=pp->TzLoc;
	int cc=currchar[s];

	// ANTICHRIST -- CHECKS LINE OF SIGHT!
	Coord_cl clTemp4(x,y,z);
	if(line_of_sight(s, chars[cc].pos, clTemp4, WALLS_CHIMNEYS + DOORS + ROOFING_SLANTED))
	{
		P_ITEM pi = FindItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
		if (pi != NULL) // crashfix LB
		{
			pi->MoveTo(x, y, z);
			pi->SetContSerial(-1);
			pi->magic=2; //make item unmovable once thrown
			movingeffect2(cc, DEREF_P_ITEM(pi), 0x0F, 0x0D, 0x11, 0x00, 0x00);
			RefreshItem(pi);
		}
	}
	else sysmessage(s,"You cannot throw the potion there!");
}

static void Priv3Target(UOXSOCKET s, P_CHAR pc)
{
	clConsole.send("setpriv3target: %s\n", pc->name);
	pc->priv3[0]=priv3a[s];
	pc->priv3[1]=priv3b[s];
	pc->priv3[2]=priv3c[s];
	pc->priv3[3]=priv3d[s];
	pc->priv3[4]=priv3e[s];
	pc->priv3[5]=priv3f[s];
	pc->priv3[6]=priv3g[s];
}

void cTargets::SquelchTarg(int s)//Squelch
{
	int p;
//	int time=uiCurrentTime;

	int serial=LongFromCharPtr(buffer[s]+7);
	p=calcCharFromSer(serial);
	if (p!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(p);
		if(pc->isGM())
		{
			sysmessage(s, "You cannot squelch GMs.");
			return;
		}
		if (pc->squelched)
		{
			pc->squelched=0;
			sysmessage(s, "Un-squelching...");
			sysmessage(calcSocketFromChar(p), "You have been unsquelched!");
			pc->mutetime=-1;
		}
		else
		{
			pc->mutetime=-1;
			pc->squelched=1;
			sysmessage(s, "Squelching...");
			sysmessage(calcSocketFromChar(p), "You have been squelched!");
			
			if (addid1[s]!=255 || addid1[s]!=0)
		
			{
				pc->mutetime=(unsigned int) (uiCurrentTime+(addid1[s]*MY_CLOCKS_PER_SEC));
				
				addid1[s]=255;
			
				pc->squelched=2;
			}
		}
	}
}


static void TeleStuff(int s, PKGx6C *pp)
{
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
}

void CarveTarget(int s, int feat, int ribs, int hides, int fur, int wool, int bird)
{
	int cc = currchar[s];
	P_ITEM pi1=Items->SpawnItem(cc,1,"#",0,0x122A,0,0);	//add the blood puddle
	P_ITEM pi2=MAKE_ITEMREF_LR(npcshape[0]);
	if(!pi1) return;
	pi1->pos.x=pi2->pos.x;
	pi1->pos.y=pi2->pos.y;
	pi1->pos.z=pi2->pos.z;
	pi1->magic=2;//AntiChrist - makes the item unmovable
	pi1->startDecay();
	RefreshItem(pi1);

	int c;
	if (feat)
	{
		c=Items->SpawnItem(s,cc,feat,"feather",1,0x1B,0xD1,0,0,1,1);
		if(c==-1) return;
		sysmessage(s,"You pluck the bird and get some feathers.");
	}
	if (ribs)
	{
		c=Items->SpawnItem(s,cc,ribs,"raw rib",1,0x09,0xF1,0,0,1,1);
		if(c==-1) return;
		sysmessage(s,"You carve away some meat.");
	}

	if (hides)
	{
		c=Items->SpawnItem(s,cc,hides,"hide",1,0x10,0x78,0,0,1,1);
		if(c==-1) return;
		sysmessage(s,"You skin the corpse and get the hides.");
	}

	if (fur)
	{	// animals with fur now yield hides (OSI). Duke, 7/17/00
		c=Items->SpawnItem(s,cc,fur,"hide",1,0x10,0x78,0,0,1,1);
		if(c==-1) return;
		sysmessage(s,"You skin the corpse and get the hides.");
/*		c=Items->SpawnItem(s,fur,"fur",1,0x11,0xFA,0,0,1,1);
		if(c==-1) return;
		sysmessage(s, "You skin the corpse and get some fur.");*/
	}

	if (wool)
	{
		c=Items->SpawnItem(s,cc,wool,"unspun wool",1,0x0D,0xF8,0,0,1,1);
		if(c==-1) return;
		sysmessage(s, "You skin the corpse and get some unspun wool.");
	}
	if (bird)
	{
		c=Items->SpawnItem(s,cc,bird,"raw bird",1,0x09,0xB9,0,0,1,1);
		if(c==-1) return;
		sysmessage(s, "You carve away some raw bird.");
	}
	Weight->NewCalc(cc);
}

//AntiChrist - new carving system - 3/11/99
//Human-corpse carving code added
//Scriptable carving product added
static void newCarveTarget(UOXSOCKET s, ITEM i)
{
	int cc = currchar[s];
	bool deletecorpse=false;
	int c, storeval, n;
	char sect[512];
	long int pos;

	P_ITEM pi1=Items->SpawnItem(cc,1,"#",0,0x122A,0,0);	//add the blood puddle
	P_ITEM pi2=MAKE_ITEMREF_LR(npcshape[0]);
	P_ITEM pi3=MAKE_ITEMREF_LR(i);
	if(!pi1) return;
	pi1->MoveTo(pi2->pos.x,pi2->pos.y,pi2->pos.z);
	pi1->magic=2;//AntiChrist - makes the item unmovable
	pi1->startDecay();
	RefreshItem(pi1);


	if(pi3->morey)	//if it's a human corpse
	{
		chars[cc].fame-=100; // Ripper..lose fame and karma and criminal.
		chars[cc].karma-=100;
		sysmessage(s,"You lost some fame and karma!");
		criminal(cc);
		//create the Head
		sprintf((char*)temp,"the head of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1D,0xA0,0,0,0,0);
		P_ITEM pi=MAKE_ITEMREF_LR(c);
		if(c==-1) return;
		pi->SetContSerial(items[i].serial);
		pi->layer=0x01;
		pi->att=5;

		//AntiChrist & Magius(CHE) - store item's owner, so that lately
		//if we want to know the owner we can do it
		pi->setOwnSerialOnly(pi3->ownserial);

		//create the Body
		sprintf((char*)temp,"the heart of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1C,0xED,0,0,0,0);
		P_ITEM pi4=MAKE_ITEMREF_LR(c);
		if(c==-1) return;
		pi4->SetContSerial(items[i].serial);
		pi4->layer=0x01;
		pi4->att=5;
		pi4->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Heart
		sprintf((char*)temp,"the body of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1D,0xAD,0,0,0,0);
		P_ITEM pi5=MAKE_ITEMREF_LR(c);
		if(c==-1) return;
		pi5->SetContSerial(items[i].serial);
		pi5->layer=0x01;
		pi5->att=5;
		pi5->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Left Arm
		sprintf((char*)temp,"the left arm of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1D,0xA1,0,0,0,0);
		P_ITEM pi6=MAKE_ITEMREF_LR(c);
		if(c==-1) return;
		pi6->SetContSerial(items[i].serial);
		pi6->layer=0x01;
		pi6->att=5;
		pi6->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Right Arm
		sprintf((char*)temp,"the right arm of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1D,0xA2,0,0,0,0);
		P_ITEM pi7=MAKE_ITEMREF_LR(c);
		if(c==-1) return;//AntiChrist to preview crashes
		pi7->SetContSerial(items[i].serial);
		pi7->layer=0x01;
		pi7->att=5;
		pi7->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Left Leg
		sprintf((char*)temp,"the left leg of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1D,0xA3,0,0,0,0);
		P_ITEM pi8=MAKE_ITEMREF_LR(c);
		if(c==-1) return;//AntiChrist to preview crashes
		pi8->SetContSerial(items[i].serial);
		pi8->layer=0x01;
		pi8->att=5;
		pi8->setOwnSerialOnly(pi3->ownserial);	// see above

		//create the Rigth Leg
		sprintf((char*)temp,"the right leg of %s",pi3->name2);
		c=Items->SpawnItem(s,cc,1,(char*)temp,0,0x1D,0xA4,0,0,0,0);
		P_ITEM pi9=MAKE_ITEMREF_LR(c);
		if(c==-1) return;
		
		pi9->SetContSerial(items[i].serial);
		pi9->layer=0x01;
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
					n=Items->CreateScriptItem(s,storeval,0);
					P_ITEM pi10=MAKE_ITEMREF_LR(n);
					pi10->layer=0;
					pi10->SetContSerial(items[i].serial);
					pi10->pos.x=20+(rand()%50);
					pi10->pos.y=85+(rand()%75);
					pi10->pos.z=9;
					RefreshItem(n);//let's finally refresh the item
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
		int ci=0, loopexit=0;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pi3->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);
			pj->SetContSerial(-1);
			pj->MoveTo(pi3->pos.x,pi3->pos.y,pi3->pos.z);
			pj->startDecay();
			RefreshItem(pj);
		}
		Items->DeleItem(i);	//and then delete the corpse
	}
}

static void CorpseTarget(const P_CLIENT pC)
{
	int n=0;
	UOXSOCKET s = pC->GetSocket();

	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEMREF_LR(i);
	if(i!=-1)
	{
		if(iteminrange(s,i,1))
		{
			npcshape[0]=i;
			action(s,0x20);
			n=1;
			if(pi->more1==0)
			{
				pi->more1=1;//corpse being carved...can't carve it anymore

				if(pi->morey || pi->carve>-1)
				{//if specified, use enhanced carving system!
					newCarveTarget(s, i);//AntiChrist
				} else
				{//else use standard carving
					switch(pi->amount) {
					case 0x01: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Ogre
					case 0x02: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin
					case 0x03: break;	//Zombie
					case 0x04: break;	//Gargoyle
					case 0x05: CarveTarget(s,36, 0, 0, 0, 0, 1); break; //Eagle
					case 0x06: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Bird
					case 0x07: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/axe
					case 0x08: break;	//Corpser
					case 0x09: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon
					case 0x0A: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon w/sword
					case 0x0B: break;	//-NULL-
					case 0x0C: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (green)
					case 0x0D: break;	//Air Elemental
					case 0x0E: break;	//Earth Elemental
					case 0x0F: break;	//Fire Elemental
					case 0x10: break;	//Water Elemental
					case 0x11: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Orc
					case 0x12: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin w/club
					case 0x13: break; //-NULL-
					case 0x14: break; //-NULL-
					case 0x15: CarveTarget(s, 0, 4,20, 0, 0, 0); break; //Giant Serpent
					case 0x16: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Gazer
					case 0x17: break;	//-NULL-
					case 0x18: break;	//Liche
					case 0x19: break;	//-NULL-
					case 0x1A: break;	//Ghoul
					case 0x1B: break;	//-NULL-
					case 0x1C: break;	//Spider
					case 0x1D: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Gorilla
					case 0x1E: CarveTarget(s,50, 0, 0, 0, 0, 1); break; //Harpy
					case 0x1F: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Headless
					case 0x20: break;	//-NULL-
					case 0x21: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman
					case 0x0122: CarveTarget(s, 0,10, 0, 0, 0, 0); break; // Boar
					case 0x23: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/spear
					case 0x24: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/mace
					case 0x0124: CarveTarget(s, 0, 1, 12,0, 0, 0); break; //pack Llama
					case 0x25: break;	//-NULL-
					case 0x26: break;	//-NULL-
					case 0x27: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Mongbat
					case 0x28: break;	//-NULL-
					case 0x29: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/club
					case 0x2A: break;	//Ratman
					case 0x2B: break;	//-NULL-
					case 0x2C: break;	//Ratman w/axe
					case 0x2D: break;	//Ratman w/dagger
					case 0x2E: break;	//-NULL-
					case 0x2F: break;	//Reaper
					case 0x30: break;	//Scorpion
					case 0x31: break;	//-NULL-
					case 0x32: break;	//Skeleton
					case 0x33: break;	//Slime
					case 0x34: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Snake
					case 0x35: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/axe
					case 0x36: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll
					case 0x37: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/club
					case 0x38: break;	//Skeleton w/axe
					case 0x39: break;	//Skeleton w/sword
					case 0x3A: break;	//Wisp
					case 0x3B: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (red)
					case 0x3C: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (green)
					case 0x3D: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (red)
					case 0x46: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen Matriarche - t2a
					case 0x47: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen drone - t2a
					case 0x48: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen warrior, Terathen Avenger - t2a
					case 0x4B: CarveTarget(s, 0,4, 0, 0, 0, 0); break; //Titan - t2a
					case 0x4C: CarveTarget(s, 0, 4, 0, 0, 0, 0); break; //Cyclopedian Warrior - t2a
					case 0x50: CarveTarget(s, 0,10, 2, 0, 0, 0); break; //Giant Toad - t2a
					case 0x51: CarveTarget(s, 0, 4, 1, 0, 0, 0); break; //Bullfrog - t2a
					case 0x55: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian apprentice, Ophidian Shaman - t2a
					case 0x56: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian warrior, Ophidian Enforcer, Ophidian Avenger - t2a
					case 0x57: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian Matriarche - t2a
					case 0x5F: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Kraken - t2a
						//case 0x3E-case 0x95: break; //-NULL-
					case 0x96: CarveTarget(s, 0,10, 0, 0, 0, 0); break; //Sea Monster
					case 0x97: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dolphin
						//case 0x98-case 0xC7: break; //-NULL-
					case 0xC8: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
					case 0xC9: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Cat
					case 0xCA: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Alligator
					case 0xCB: CarveTarget(s, 0, 6, 0, 0, 0, 0); break; //Pig
					case 0xCC: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (dark)
					case 0xCD: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Rabbit
					case 0xCE: CarveTarget(s, 0, 1,12, 0 ,0, 0); break; //Lava Lizard - t2a
					case 0xCF: CarveTarget(s, 0, 3, 0, 0, 1, 0); break; //Sheep
					case 0xD0: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Chicken
					case 0xD1: CarveTarget(s, 0, 2, 8, 0, 0, 0); break; //Goat
					case 0xD2: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Desert Ostarge - t2a
					case 0xD3: CarveTarget(s, 0, 1, 12, 0, 0, 0); break; //Bear
					case 0xD4: CarveTarget(s, 0, 1, 0, 2, 0, 0); break; //Grizzly Bear
					case 0xD5: CarveTarget(s, 0, 2, 0, 3, 0, 0); break; //Polar Bear
					case 0xD6: CarveTarget(s, 0, 1, 10, 0, 0, 0); break; //Cougar
					case 0xD7: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Giant Rat
					case 0xD8: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (black)
					case 0xD9: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dog
					case 0xDA: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Frenzied Ostard - t2a
					case 0xDB: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Forest Ostard - t2a
					case 0xDC: CarveTarget(s, 0, 1, 12,0, 0, 0); break; //Llama
					case 0xDD: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Walrus
					case 0xDE: break;	//-NULL-
					case 0xDF: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Sheep (BALD)
					case 0xE1: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Timber Wolf
					case 0xE2: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (Silver)
					case 0xE3: break;	//-NULL-
					case 0xE4: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
					case 0xE5: break;	//-NULL-
					case 0xE6: break;	//-NULL-
					case 0xE7: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (brown)
					case 0xE8: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (brown)
					case 0xE9: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (d-brown)
					case 0xEA: CarveTarget(s, 0, 6,15, 0, 0, 0); break; //Great Heart
					case 0xEB: break;	//-NULL-
					case 0xEC: break;	//-NULL-
					case 0xED: CarveTarget(s, 0, 5, 8, 0, 0, 0); break; //Hind
					case 0xEE: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Rat
						//case 0xEF-case 0xFF: break; //-NULL-
					default:
						LogErrorVar("Fallout of switch statement, value <%i>", pi->amount);
					}// switch
				}//if morey || carve>-1
			} else {
				 sysmessage(s, "You carve the corpse but find nothing usefull.");
			}// if more1==0
		//break;
		}
	}// if i!=-1
	if (!n) sysmessage(s, "That is too far away.");
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
		if (rpi->amount>1)
			amt=(rpi->amount*4);
		else
			amt=4; 
		soundeffect(s,0x00,0x50);
		P_ITEM pi=Items->SpawnItem(pC->GetCurrChar(),amt,"#",1,0x097A,0,1);
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
		CHARACTER c = pC->GetCurrChar();
		P_CHAR pc = pC->getPlayer();
		if (!pc->onhorse) action(s,0x0D);
		else action(s,0x1d);
		soundeffect(s,0x01,0x3E);
		P_ITEM pi=Items->SpawnItem(c,1,"#",1,0x0DE1,0,0); //Kindling
		if(!pi)
			return;
		pi->MoveTo(pc->pos.x,pc->pos.y,pc->pos.z);
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
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		addid1[s]=pc->ser1;
		addid2[s]=pc->ser2;
		addid3[s]=pc->ser3;
		addid4[s]=pc->ser4;
		target(s, 0, 1, 0, 57, "Select NPC to follow this player.");
	}
}

void cTargets::NpcTarget2(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		if (pc->isNpc())
		{
			pc->ftarg=calcCharFromSer(addid1[s], addid2[s], addid3[s], addid4[s]);
			pc->npcWander=1;
		}
	}
}

void cTargets::NpcRectTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
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
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
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
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		if ((pc->isNpc())) pc->npcWander=npcshape[0];
	}
}

//taken from 6904t2(5/10/99) - AntiChrist
void cTargets::NpcAITarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->npcaitype=addx[s];
		sysmessage(s, "Npc AI changed.");//AntiChrist
	}
}

void cTargets::xBankTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		openbank(s, i);
	}
}

void cTargets::xSpecialBankTarget(int s)//AntiChrist
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		openspecialbank(s, i);
	}
}

void cTargets::SellStuffTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		sellstuff(s, i);
	}
}

void cTargets::ReleaseTarget(int s, int c) 
{ 
	int i, serial; 
	
	if (c==-1) 
	{ 
		serial = LongFromCharPtr(buffer[s] + 7); 
		i = calcCharFromSer(serial); 
	} 
	else 
	{ 
		i = calcCharFromSer(c); 
	} 
	if (i!=-1) 
	{ 
		P_CHAR pc = MAKE_CHARREF_LR(i); 
		if (pc->cell == 0) 
		{ 
			sysmessage(s, "That player is not in jail!"); 
		} 
		else 
		{ 
			jails[pc->cell].occupied = false; 
			pc->MoveTo(jails[pc->cell].oldpos.x, jails[pc->cell].oldpos.y, jails[pc->cell].oldpos.z);
			pc->cell = 0; 
			pc->priv2 = 0; 
			pc->jailsecs = 0; 
			pc->jailtimer = 0; 
			teleport(i); 
			soundeffect(c, 1, 0xfd); // Play sound effect for player 
			sysmessage(c, "You are released.."); 
			sysmessage(s, "Player %s released.", pc->name); 
		} 
	} 
}

void cTargets::GmOpenTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int ci=0, loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==addmitem[s])
		{
			backpack(s, pi->serial);
			return;
		}
	}
	sysmessage(s,"No object was found at that layer on that character");
}

void cTargets::StaminaTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		soundeffect2(i, 0x01, 0xF2);
		staticeffect(i, 0x37, 0x6A, 0x09, 0x06);
		pc->stm=pc->effDex();
		updatestats(i, 2);
		return;
	}
	sysmessage(s,"That is not a person.");
}

void cTargets::ManaTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		soundeffect2(i, 0x01, 0xF2);
		staticeffect(i, 0x37, 0x6A, 0x09, 0x06);
		pc->mn=pc->in;
		updatestats(i, 1);
		return;
	}
	sysmessage(s,"That is not a person.");
}

void cTargets::MakeShopTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		Commands->MakeShop(i);
		teleport(i);
		sysmessage(s, "The buy containers have been added.");
		return;
	}
	sysmessage(s, "Target character not found...");
}

void cTargets::JailTarget(int s, int c) 
{ 
	int i, tmpnum = 0, serial; 
	
	int x = 0; 
	if (c==-1) 
	{ 
		serial = LongFromCharPtr(buffer[s] + 7); 
		i = calcCharFromSer(serial); 
		tmpnum = i; 
	} 
	else 
	{ 
		i = calcCharFromSer(c); 
		tmpnum = i; 
	} 
	if (tmpnum==-1)
		return; // lb 
	P_CHAR pc = MAKE_CHARREF_LR(tmpnum); 
	
	if (pc->cell>0) 
	{ 
		sysmessage(s, "That player is already in jail!"); 
		return; 
	} 
	
	for (i = 1; i < 11; i++) 
	{ 
		if (!jails[i].occupied) 
		{ 
			jails[i].oldpos = pc->pos;
			pc->MoveTo(jails[i].pos.x, jails[i].pos.y, jails[i].pos.z); 
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
			sysmessage(s, "Player %s has been jailed in cell %i.", pc->name, i); 
			Items->SpawnItemBackpack2(prisoner, 50040, 0); // spawn crystall ball of justice to prisoner. 
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
	int target, target2;

	target=calcCharFromSer(addx[s]);
	target2=calcCharFromSer(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]);
	P_CHAR pc = MAKE_CHARREF_LR(target);
    if (pc->inGuardedArea()) // Ripper..No pet attacking in town.
	{
        sysmessage(s,"You cant have pets attack in town!");
        return;
	}
	if (target2==-1 || target==-1) return;
	npcattacktarget(target2, target);
}

void cTargets::FollowTarget(int s)
{
	int char1, char2;

	char1=calcCharFromSer(addx[s]);
	char2=calcCharFromSer(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]);
	P_CHAR pc = MAKE_CHARREF_LR(char1);

	pc->ftarg=char2;
	pc->npcWander=1;
}

void cTargets::TransferTarget(int s)
{
	int char1, char2;
	char t[120];

	char1=calcCharFromSer(addx[s]);
	char2=calcCharFromSer(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]);
	P_CHAR pc1 = MAKE_CHARREF_LR(char1);
	P_CHAR pc2 = MAKE_CHARREF_LR(char2);

	sprintf(t,"* %s will now take %s as his master *",pc1->name,pc2->name);
	npctalkall(char1,t,0);

	if (pc1->ownserial != -1) 
		pc1->SetOwnSerial(-1);
	pc1->SetOwnSerial(pc2->serial);
	pc1->npcWander=1;

	pc1->ftarg=-1;
	pc1->npcWander=0;
}

void cTargets::BuyShopTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHARREF_LR(i);
	if (pc != NULL)
		if ((pc->serial==serial))
		{
			Targ->BuyShop(s, i);
			return;
		}
		sysmessage(s, "Target shopkeeper not found...");
}

int cTargets::BuyShop(int s, int c)
{
	P_ITEM pCont1=NULL, pCont2=NULL;

	P_CHAR pc = MAKE_CHARREF_LRV(c, 0);

	int ci=0, loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==0x1A)
		{
			pCont1=pi;
		}
		else if (pi->layer==0x1B)
		{
			pCont2=pi;
		}
	}

	if (!pCont1 || !pCont2)
	{
		return 0;
	}

	impowncreate(s, c, 0); // Send the NPC again to make sure info is current. (OSI does this we might not have to)
	sendshopinfo(s, c, pCont1); // Send normal shop items
	sendshopinfo(s, c, pCont2); // Send items sold to shop by players
	SndShopgumpopen(s,pc->serial);
	statwindow(s,currchar[s]); // Make sure the gold total has been sent.
	return 1;
}

////////////////////////////////// 
// Changed hideing to make flamestrike and hide work better 
// 
// 
void cTargets::permHideTarget(int s) 
{ 
	int serial = LongFromCharPtr(buffer[s] + 7); 
	int i = calcCharFromSer(serial); 
	if (i!=-1) 
	{ 
		P_CHAR pc = MAKE_CHARREF_LR(i); 
		if (pc->hidden == 1) 
		{ 
			if (i == currchar[s])
				sysmessage(s, "You are already hiding."); 
			else 
				sysmessage(s, "He is already hiding."); 
			return; 
		} 
		pc->priv2 = pc->priv2 | 8; 
		// staticeffect(i, 0x37, 0x09, 0x09, 0x19); 
		staticeffect3(pc->pos.x + 1, pc->pos.y + 1, pc->pos.z + 10, 0x37, 0x09, 0x09, 0x19, 0); 
		soundeffect2(i, 0x02, 0x08); 
		tempeffect(i, i, 33, 1, 0, 0); 
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
	int serial = LongFromCharPtr(buffer[s] + 7); 
	int i = calcCharFromSer(serial); 
	if (i!=-1) 
	{ 
		P_CHAR pc = MAKE_CHARREF_LR(i); 
		if (pc->hidden == 0) 
		{ 
			if (i == currchar[s])
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
		soundeffect2(i, 0x02, 0x08); 
		tempeffect(i, i, 34, 1, 0, 0); 
		return; 
	} 
} 
// 
// 
// Aldur 
//////////////////////////////////

void cTargets::SetSpeechTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
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
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->spattack=tempint[s];
	}
}

void cTargets::SetSpaDelayTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->spadelay=tempint[s];
	}
}

void cTargets::SetPoisonTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->poison=tempint[s];
	}
}

void cTargets::SetPoisonedTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->poisoned=tempint[s];
		pc->poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // lb, poison wear off timer setting
		impowncreate(calcSocketFromChar(i),i,1); //Lb, sends the green bar !
	}
}

void cTargets::FullStatsTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		soundeffect2(i, 0x01, 0xF2);
		staticeffect(i, 0x37, 0x6A, 0x09, 0x06);
		pc->mn=pc->in;
		pc->hp=pc->st;
		pc->stm=pc->effDex();
		updatestats(i, 0);
		updatestats(i, 1);
		updatestats(i, 2);
		return;
	}
	sysmessage(s,"That is not a person.");
}

void cTargets::SetAdvObjTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->advobj=tempint[s];
	}
}

////////////////
// name:		CanTrainTarget
// history:		by Antrhacks 1-3-99
// Purpose:		Used for training by NPC's
//
void cTargets::CanTrainTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		if (pc->isPlayer())
		{
			sysmessage(s, "Only NPC's may train.");
			return;
		}
		pc->cantrain = !pc->cantrain;	//turn on if off, off if on
	}
}

void cTargets::SetSplitTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->split=tempint[s];
	}
}

void cTargets::SetSplitChanceTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->splitchnc=tempint[s];
	}
}

void cTargets::SetDirTarget(int s)
{
	int i;
	int serial=LongFromCharPtr(buffer[s]+7);

	if (isItemSerial(serial))
	{
		i=calcItemFromSer(serial);
		P_ITEM pi=MAKE_ITEMREF_LR(i);
		if (pi != NULL)
		{
			pi->dir=addx[s];
			RefreshItem(DEREF_P_ITEM(pi));
			return;
		}
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHARREF_LR(i);
		if (pc != NULL)
		{
			pc->dir=addx[s];
			updatechar(DEREF_P_CHAR(pc));
			return;
		}
	}
}

////////////////
// name:		NpcResurrectTarget
// history:		by UnKnown (Touched tabstops by Tauriel Dec 28, 1998)
// Purpose:		Resurrects a character
//
bool cTargets::NpcResurrectTarget(CHARACTER i)
{
	unsigned int j ;
	int k=calcSocketFromChar(i);
	if (k<=-1) return false;
	P_CHAR pc = MAKE_CHARREF_LRV(i,true);

	if (pc->dead)
	{//Shouldn' be a validNPCMove inside a door, might fix house break in. -- from zippy code
		Fame(i,0);
		soundeffect2(i, 0x02, 0x14);
		pc->id1=pc->xid1;
		pc->id2=pc->xid2;
		pc->skin = pc->xskin;
		pc->dead=false;
		pc->hp=pc->st;// /10;
		pc->stm=pc->effDex();// /10;
		pc->mn=pc->in; ///10;
		pc->attacker = INVALID_SERIAL;
		pc->resetAttackFirst();
		pc->war=0;

		for (j=0;j<itemcount;j++)
		{
			P_ITEM pj=MAKE_ITEMREF_LRV(j,true);
			if (pc->Wears(pj) && pj->layer==0x1A)
			{
				pj->layer=0x15;
				pc->packitem=j;	//Tauriel packitem speedup
				//break;
			}
		}
		for (j=0;j<itemcount;j++)
		{
			P_ITEM pj = MAKE_ITEMREF_LRV(j,true);
			if (pj->serial == pc->robe)
			{
				Items->DeleItem(j);

				P_ITEM pi=Items->SpawnItem(i,1,"a robe",0,0x1F03,0,0);
				if(!pi) return false;
				pi->SetContSerial(pc->serial);
				pi->layer=0x16;
				pi->dye=1;
				break;
			}
		}
		teleport(i);
		return true;
	}

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
		pi->MoveTo(addx[s],pi->pos.y,pi->pos.z);
		RefreshItem(DEREF_P_ITEM(pi));
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		pc->MoveTo(addx[s],pc->pos.y,pc->pos.z);
		teleport(DEREF_P_CHAR(pc));
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
		pi->MoveTo(pi->pos.x,addx[s],pi->pos.z);
		RefreshItem(pi);
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		pc->MoveTo(pc->pos.x,addx[s],pc->pos.z);
		teleport(DEREF_P_CHAR(pc));
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
		pi->MoveTo(pi->pos.x + addx[s], pi->pos.y, pi->pos.z);
		RefreshItem(DEREF_P_ITEM(pi));
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		pc->MoveTo(pc->pos.x + addx[s],pc->pos.y,pc->pos.z);
		teleport(DEREF_P_CHAR(pc));
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
		pi->MoveTo(pi->pos.x, pi->pos.y + addx[s], pi->pos.z);
		RefreshItem(DEREF_P_ITEM(pi));
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		pc->MoveTo(pc->pos.x, pc->pos.y + addx[s], pc->pos.z);
		teleport(DEREF_P_CHAR(pc));
	}
}

void cTargets::Priv3XTarget(int s) // crackerjack's addition, jul.24/99
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcCharFromSer(serial);
	if (i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		clConsole.send("setpriv3target: %s\n", pc->name);
		struct cmdtable_s *pct=&command_table[addx[s]];
		if(addy[s])
		{
			pc->priv3[pct->cmd_priv_m] |= (0-0xFFFFFFFF<<pct->cmd_priv_b);
			sysmessage(s, "%s has been granted access to the %s command.",pc->name, pct->cmd_name);
		}
		else
		{
			pc->priv3[pct->cmd_priv_m] -= (0-0xFFFFFFFF<<pct->cmd_priv_b);
			sysmessage(s, "%s has been revoked access to the %s command.",pc->name, pct->cmd_name);
		}
	}
}

void cTargets::ShowPriv3Target(int s) // crackerjack, jul 25/99
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int p=calcCharFromSer(serial);
	if (p!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(p);
		char priv_info[10248];
		int i;
		sprintf(priv_info, "%s can execute the following commands:\n", pc->name);
		i=0; unsigned long loopexit=0;
		while(command_table[i].cmd_name && (++loopexit < MAXLOOPS) )
		{
			if(command_table[i].cmd_priv_m==255||
				(pc->priv3[command_table[i].cmd_priv_m]&
				(0-0xFFFFFFFF<<command_table[i].cmd_priv_b)))
			{
				sprintf((char*)temp, " %s", command_table[i].cmd_name);
				strcpy(&priv_info[strlen(priv_info)], (char*)temp);
			}
			i++;
		}
		SndUpdscroll(s, strlen(priv_info), priv_info);
	}
	else
		sysmessage(s, "You cannot retrieve privilige information on that.");
}

void cTargets::HouseOwnerTarget(int s) // crackerjack 8/10/99 - change house owner
{
	int sign, house, os, i;
	int o_serial=LongFromCharPtr(buffer[s]+7);
	if(o_serial==-1) return;
	int own=calcCharFromSer(o_serial);
	if(own==-1) return;
	P_CHAR pc = MAKE_CHARREF_LR(own);

	int key;
	int serial=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	sign=calcItemFromSer(serial);
	P_ITEM pSign=MAKE_ITEMREF_LR(sign);

	serial=calcserial(pSign->more1, pSign->more2, pSign->more3, pSign->more4);
	house=calcItemFromSer(serial);
	P_ITEM pHouse=MAKE_ITEMREF_LR(house);
	
	if (sign ==-1 || house ==-1) return; //lb
	if(pc->serial == chars[currchar[s]].serial)
	{
		sysmessage(s, "you already own this house!");
		return;
	}
	
	pSign->SetOwnSerial(o_serial);
	
	pHouse->SetOwnSerial(o_serial);
	
	HouseManager->RemoveKeys(pHouse->serial);
	
	os=-1;
	for(i=0;i<now && os==-1;i++)
	{
		if(chars[currchar[i]].serial==pc->serial && perm[i]) os=i;
	}
	
	P_ITEM pi3=NULL;
	if(os!=-1)
	{
		key=Items->SpawnItem(os, own, 1, "a house key", 0, 0x10, 0x0F, 0, 0,1,1);//gold key for everything else
		if(key==-1) return;
		pi3=MAKE_ITEMREF_LR(key);
	}
	else
	{
		pi3=Items->SpawnItem(own, 1, "a house key", 0, 0x100F,0,0);//gold key for everything else
		if(!pi3) return;
		pi3->MoveTo(pc->pos.x,pc->pos.y,pc->pos.z);
		RefreshItem(pi3);
	}
	pi3->more1=pHouse->ser1;
	pi3->more2=pHouse->ser2;
	pi3->more3=pHouse->ser3;
	pi3->more4=pHouse->ser4;
	pi3->type=7;
	
	sysmessage(s, "You have transferred your house to %s.", pc->name);
	sprintf((char*)temp, "%s has transferred a house to %s.", chars[currchar[s]].name, pc->name);

	int k;
	for(k=0;k<now;k++)
		if(k!=s && ( (perm[k] && inrange1p(currchar[k],currchar[s]) )||
			(chars[currchar[k]].serial==o_serial)))
			sysmessage(k, (char*)temp);
}

void cTargets::HouseEjectTarget(int s) // crackerjack 8/11/99 - kick someone out of house
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if (!pc)
		return;
	int serial=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	if(serial == INVALID_SERIAL) return;
	int h=calcItemFromSer(serial);
	if(h!=-1)
	{
		int sx, sy, ex, ey;
		Map->MultiArea(h, &sx,&sy,&ex,&ey);
		if(pc->serial == chars[currchar[s]].serial)
		{
			sysmessage(s,"Do you really want to do that?!");
			return;
		}
		if(pc->pos.x>=sx&&pc->pos.y>=sy&&pc->pos.x<=ex&&pc->pos.y<=ey)
		{
			pc->MoveTo(ex,ey,pc->pos.z);
			teleport(DEREF_P_CHAR(pc));
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
	P_CHAR pc_home = MAKE_CHAR_REF(currchar[s]);
	if (!pc)
		return;
	int h=pc_home->MyHome();
	if(h!=-1)
	{
		if(pc->serial == pc_home->serial) return;
		int r=House[h]->AddBan(pc);
		if(r==1)
		{
			sysmessage(s, "%s has been banned from this house.", pc->name);
		} 
		else if(r==2)
		{
			sysmessage(s, "%s is already banned!.", pc->name);
		} 
		else
			sysmessage(s, "House Error!");
	}
}

void cTargets::HouseFriendTarget(int s) // crackerjack 8/12/99 - add somebody to friends list
{
	P_CHAR Friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = MAKE_CHAR_REF(currchar[s]);

	int h=pc_home->MyHome();

	if(Friend && h!=-1)
	{
		if(Friend->serial == pc_home->serial)
		{
			sysmessage(s,"You are already the owner!");
			return;
		}
		int r=House[h]->AddFriend(Friend);
		if(r==1)
		{
			sysmessage(s, "%s has been made a Friend of the house.", Friend->name);
		} 
		else if(r==2)
		{
			sysmessage(s, "%s is already a Friend of the house!", Friend->name);
		} 
		else 
		{
			sysmessage(s, "House Error!");
		}
	}
}

void cTargets::HouseUnBanTarget(int s)
{
	P_CHAR pc_banned = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = MAKE_CHAR_REF(currchar[s]);
	
	int h=pc_owner->MyHome();

	if(pc_banned && h!=-1)
	{
		if(pc_banned->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		bool r=House[h]->RemoveBan(pc_banned);
		if(true==r)
		{
			sysmessage(s,"%s has been UnBanned!",pc_banned->name);
		}
		else if(false==r)
		{
			sysmessage(s,"%s was never banned!",pc_banned->name);
		}
		else
		{
			sysmessage(s,"House Error!");
		}
	}
	return;
}

void cTargets::HouseUnFriendTarget(int s)
{
	P_CHAR pc_friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = MAKE_CHAR_REF(currchar[s]);
	
	int h=pc_owner->MyHome();

	if(pc_friend && h!=-1)
	{
		if(pc_friend->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		bool r=House[h]->RemoveFriend(pc_friend);
		if(true==r)
		{
			sysmessage(s,"%s is no longer a Friend of this home!",pc_friend->name);
		}
		else if(false==r)
		{
			sysmessage(s,"%s was never a Friend of this home!",pc_friend->name);
		}
		else
		{
			sysmessage(s,"House Error!");
		}
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
	int itemToLock = calcItemFromSer(ser);

	if( itemToLock != -1 )
	{
		P_ITEM pi=MAKE_ITEMREF_LR(itemToLock);
		short id = pi->id();
		/*houseSer = calcserial( addid1[s], addid2[s], addid3[s], addid4[s] );	// let's find our house
		house = calcItemFromSer(houseSer);*/

		// not needed anymore, cause called from house_sped that already checks that ...

		// time to lock it down!

		if (Items->isFieldSpellItem ( itemToLock ))
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if (pi->type==12 || pi->type==13 || pi->type==203)
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if ( IsAnvil(id) )
		{
			sysmessage(s, "You cant lockdown anvils!");
			return;
		}
		if ( IsForge(id) )
		{
			sysmessage(s, "You cant lockdown forges!");
			return;
		}

		P_ITEM pi_multi = findmulti( pi->pos );
		if( pi_multi != NULL )
		{
			if(pi->magic==4)
			{
				sysmessage(s,"That item is already locked down, release it first!");
				return;
			}
			pi->magic = 4;	// LOCKED DOWN!
			DRAGGED[s]=0;
			pi->setOwnSerialOnly(chars[currchar[s]].serial);
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
	int itemToLock = calcItemFromSer(ser);
	if( itemToLock != -1 )
	{
		// time to lock it down!
		P_ITEM pi=MAKE_ITEMREF_LR(itemToLock);

		if (Items->isFieldSpellItem ( itemToLock ))
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if (pi->type==12 || pi->type==13 || pi->type==203)
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if(pi->magic==4)
		{
			sysmessage(s,"That item is already locked down, release it first!");
			return;
		}

		P_ITEM pi_multi = findmulti( pi->pos );
		if( pi_multi != NULL && pi->type==1)
		{
		    pi->magic = 4;	// LOCKED DOWN!
			pi->secureIt = 1;
			DRAGGED[s]=0;
			pi->setOwnSerialOnly(chars[currchar[s]].serial);
			RefreshItem(pi);
			return;
		}
		if(pi->type!=1)
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
	int itemToLock = calcItemFromSer(ser);
	if( itemToLock != -1 )
	{
		P_ITEM pi=MAKE_ITEMREF_LR(itemToLock);
		if (Items->isFieldSpellItem(itemToLock))
		{
			sysmessage(s,"you cannot release this!");
			return;
		}
		if (pi->type==12 || pi->type==13 || pi->type==203)
		{
			sysmessage(s, "You cant release doors or signs!");
			return;
		}
		/*houseSer = calcserial( addid1[s], addid2[s], addid3[s], addid4[s] );	// let's find our house
		house = calcItemFromSer(houseSer);*/
		// time to lock it down!
		P_ITEM pi_multi = findmulti( pi->pos );
		if( pi_multi != NULL && pi->magic==4 || pi->type==1)
		{
			pi->magic = 1;	// Default as stored by the client, perhaps we should keep a backup?
			pi->secureIt = 0;
			RefreshItem( itemToLock );
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
	int i = calcCharFromSer(serial);
	if( i != -1 )
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->kills = addmitem[s];
		setcharflag(pc);
	}
}

void cTargets::GlowTarget(int s) // LB 4/9/99, makes items glow
{
	int c,i,k,l,j;

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL) return;
	i=calcItemFromSer(serial);
	if (i==-1)
	{
		sysmessage(s,"Item not found.");
		return;
	}

	P_ITEM pi1=MAKE_ITEMREF_LR(i);
	int cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	if (!pi1->isInWorld())
	{
		j=calcItemFromSer(pi1->contserial); // in bp ?
		l=calcCharFromSer(pi1->contserial); // equipped ?
		if (l==-1) k=GetPackOwner(j); else k=l;

		if (k!=cc)	// creation only allowed in the creators pack/char otherwise things could go wrong
		{
			sysmessage(s,"you can't create glowing items in other perons packs or hands");
			return;
		}
	}

	if (pi1->glow!=0)
	{
		sysmessage(s,"that object already glows!\n");
		return;
	}

	c=0x99;
	pi1->glow_c1=pi1->color1; // backup old colors
	pi1->glow_c2=pi1->color2;

	pi1->color1=c<<8; // set new color to yellow
	pi1->color2=c%256;

	c=Items->SpawnItem(s,cc,1,"glower",0,0x16,0x47,0,0,0,1); // new client 1.26.2 glower object
	P_ITEM pi2=MAKE_ITEM_REF(c);

	if(pi2 == NULL) return;
	pi2->dir=29; // set light radius maximal
	pi2->visible=0;

	pi2->magic=3;

	mapRegions->Remove(pi2); // remove if add in spawnitem
	pi2->layer=pi1->layer;
	if (pi2->layer==0) // if not equipped -> coords of the light-object = coords of the
	{
		pi2->pos.x=pi1->pos.x;
		pi2->pos.y=pi1->pos.y;
		pi2->pos.z=pi1->pos.z;
	} else // if equipped -> place lightsource at player ( height= approx hand level )
	{
		pi2->pos.x=pc_currchar->pos.x;
		pi2->pos.y=pc_currchar->pos.y;
		pi2->pos.z=pc_currchar->pos.z+4;
	}

	//mapRegions->AddItem(c);
	pi2->priv=0; // doesnt decay

	pi1->glow=pi2->serial; // set glow-identifier


	RefreshItem(i);
	RefreshItem(c);
	//setptr(&glowsp[pc_currchar->serial%HASHMAX],i);

	impowncreate(s,cc,0); // if equipped send new color too
}

void cTargets::UnglowTaget(int s) // LB 4/9/99, removes the glow-effect from items
{
	int c,i,j,l;
	int k;

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL) return;
	i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEMREF_LR(i);
	if (i==-1)
	{
		sysmessage(s,"no item found");
		return;
	}

	if (!pi->isInWorld())
	{
		j=calcItemFromSer(pi->contserial); // in bp ?
		l=calcCharFromSer(pi->contserial); // equipped ?
		if (l==-1) k=GetPackOwner(j); else k=l;
		if (k!=currchar[s])	// creation only allowed in the creators pack/char otherwise things could go wrong
		{
			sysmessage(s,"you can't unglow items in other perons packs or hands");
			return;
		}
	}

	c=pi->glow;
	if(c==-1) return;
	j=calcItemFromSer(c);

	if (pi->glow==0 || j==-1 )
	{
		sysmessage(s,"that object doesnt glow!\n");
		return;
	}

	pi->color1=pi->glow_c1;
	pi->color2=pi->glow_c2; // restore old color

	Items->DeleItem(j); // delete glowing object

	pi->glow=0; // remove glow-identifier
	RefreshItem(pi);

	impowncreate(s,currchar[s],0); // if equipped send new old color too

	chars[currchar[s]].removeHalo(pi);
//	removefromptr(&glowsp[chars[currchar[s]].serial%HASHMAX],i);

	//sysmessage(s,"unglow under cosntruction");
}

void cTargets::MenuPrivTarg(int s)//LB's menu privs
{
	int p,i;
	char temp[512];

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	p=calcCharFromSer(serial);
	if (p!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(p);
		i=addid1[s];
		sprintf(temp,"Setting Menupriv number %i",i);
		sysmessage(s,temp);
		sprintf(temp,"Menupriv %i set by %s",i,chars[currchar[s]].name);
		sysmessage(calcSocketFromChar(p),temp);
		pc->menupriv=i;
	}
}

void cTargets::ShowSkillTarget(int s) // LB's showskills
{
	int p,a,j,k,b=0,c,z,zz,ges=0;
	char skill_info[(ALLSKILLS+1)*40];
	char sk[25];

	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	p=calcCharFromSer(serial);
	if (p!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(p);
		z=addx[s];
		if (z<0 || z>3) z=0;
		if (z==2 || z==3)
			sprintf(skill_info, "%s's skills:", pc->name);
		else
			sprintf(skill_info, "%s's baseskills:", pc->name);

		b=strlen(pc->name)+11;
		if (b>23) b=23;

		for (c=b;c<=26;c++)
			strcpy(&skill_info[strlen(skill_info)], " ");

		numtostr(ges,sk);
		sprintf((char*)temp,"sum: %s",sk);
		strcpy(&skill_info[strlen(skill_info)],(char*)temp);

		for (a=0;a<ALLSKILLS;a++)
		{
			if (z==0 || z==1) k=pc->baseskill[a]; else k=pc->skill[a];
			if (z==0 || z==2) zz=9; else zz=-1;

			if (k>zz) // show only if skills >=1
			{
				if (z==2 || z==3) j=pc->skill[a]/10; else j=pc->baseskill[a]/10;	// get skill value
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
	pPet->npcaitype = 32; // 32 is guard mode
	pPet->ftarg=currchar[s];
	pPet->npcWander=1;
	sysmessage(s, "Your pet is now guarding you.");
	chars[currchar[s]].guarded = true;
}

void cTargets::ResurrectionTarget( UOXSOCKET s )
{
	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	int i=calcCharFromSer(serial);
	if (i>-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		if (pc->dead)
		{
			Targ->NpcResurrectTarget(i);
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
	int i=calcCharFromSer(serial);
	if(i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		if(pc->account==-1)
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
				if(j==pc->account) accountfound=1;//we are in the right section
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
	int serial=LongFromCharPtr(buffer[s]+7);
	if(serial == INVALID_SERIAL ) return;
	int i=calcCharFromSer(serial);
	if(i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->homelocx=addx[s];
		pc->homelocy=addy[s];
		pc->homelocz=addz[s];
	}
}

void cTargets::SetWork(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	int i=calcCharFromSer(serial);
	if(i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->worklocx=addx[s];
		pc->worklocy=addy[s];
		pc->worklocz=addz[s];
	}
}

void cTargets::SetFood(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	int i=calcCharFromSer(serial);
	if(i!=-1)
	{
		P_CHAR pc = MAKE_CHARREF_LR(i);
		pc->foodlocx=addx[s];
		pc->foodlocy=addy[s];
		pc->foodlocz=addz[s];
	}
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
		pi->amount2=addx[s];
		RefreshItem(pi);
		break;
	case 133://SetWipeTarget
		pi->wipe=addid1[s];
		RefreshItem(pi);
		break;
	}
}

void cTargets::LoadCannon(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEMREF_LR(i);
	if (i!=-1)
	{
		//if((pi->id1==0x0E && pi->id2==0x91) && pi->morez==0)
		if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
			(pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
			(addid1[s]==(unsigned char)'\xFF'))
		{
			if ((pi->morez==0)&&(iteminrange(s,i,2)))
			{
				if(pi->morez==0)
				pi->type=15;
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
		int i=calcItemFromSer(serial);
		if (i!=-1)
		{
			for (int j=0;j<addid1[s];j++)
			{
				Commands->DupeItem(s, i, 1); // lb bugfix
				sysmessage(s,"DupeItem done.");//AntiChrist
			}
		}
	}
}

void cTargets::MoveToBagTarget(int s)
{
	int serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEMREF_LR(i);
	if (i==-1) return;
	
	int p=packitem(currchar[s]);
	if(p==-1) return;
	
	pi->SetContSerial(items[p].serial);
	pi->pos.x=50+rand()%80;
	pi->pos.y=50+rand()%80;
	pi->pos.z=9;
	pi->layer=0x00;
	pi->decaytime=0;//reset decaytimer
	
	SndRemoveitem(pi->serial);
	RefreshItem(i);
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

	if (pt->TxLoc==-1 && pt->TyLoc==-1) // do nothing if user cancelled
		if (pt->Tserial==0 && pt->model==0) // this seems to be the complete 'cancel'-criteria (Duke)
			return;

//	cClient cli(s), *ps = &cli;

	if ((buffer[s][2]==0)&&(buffer[s][3]==1)&&(buffer[s][4]==0))
	{
		bool Iready=false, Cready=false;
		P_ITEM pi;
		P_CHAR pc;
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
				Cready=true;
		}

		switch(pt->Tnum)
		{
		case 0: AddTarget(s,pt); break;
		case 1: { cRenameTarget		T(ps);		T.process();} break;
		case 2: TeleTarget(s,pt); break; // LB, bugfix, we need it for the /tele command
		case 3: { cRemoveTarget		T(ps);		T.process();} break;
		case 4: DyeTarget(s); break;
		case 5: { cNewzTarget		T(ps);		T.process();} break;
		case 6: if (Iready) pi->type=addid1[s]; break; //Typetarget
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
				int serial=LongFromCharPtr(buffer[s]+7);
				int i=calcItemFromSer(serial);
				P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
				if(i!=-1)
				{
					triggerwitem(s,i,0);
					pc_currchar->envokeid1=0x00;
					pc_currchar->envokeid2=0x00;
					return;
				}
				// Checking if target is an NPC	--- By Magius(CHE) §
				i = calcCharFromSer(serial);
				if(i!=-1)
				{
					triggernpc(s,i,0);
					pc_currchar->envokeid1=0x00;
					pc_currchar->envokeid2=0x00;
					return;
				}
				// End Addons by Magius(CHE) §
				triggerwitem(s,-1,0);
				pc_currchar->envokeid1=0x00;
				pc_currchar->envokeid2=0x00;
				return;
			}
		case 25: Targ->CloseTarget(s); break;
		case 26: Targ->AddMenuTarget(s, 1, addmitem[s]); break;
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
		case 47: if (Cready) strcpy(pc->title,xtext[s]); break;//TitleTarget
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
		case 131: if (chars[currchar[s]].isGM()) Targ->permHideTarget(s); break; /* not used */
		case 132: if (chars[currchar[s]].isGM()) Targ->unHideTarget(s); break; /* not used */
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
				chars[currchar[s]].poisonserial=LongFromCharPtr(buffer[s]+7);
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
		case 171: Magic->BuildCannon(s); break;
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

		case 220: Guilds->Recruit(s); break;
		case 221: Guilds->TargetWar(s); break;
		case 222: TeleStuff(s,pt); break;
		case 223: Targ->SquelchTarg(s); break;//Squelch
		case 224: Targ->PlVBuy(s); break;//PlayerVendors
		case 225: Targ->Priv3XTarget(s); break; // SETPRIV3 +/- target
		case 226: Targ->ShowPriv3Target(s); break; // SHOWPRIV3
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

		case 245: HouseManager->AddHome(s,addid3[s]);	 break;

		case 247: Targ->ShowSkillTarget(s);break; //showskill target
		case 248: Targ->MenuPrivTarg(s);break; // menupriv target
		case 249: Targ->UnglowTaget(s);break; // unglow
		case 250: if (Cready) Priv3Target(s,pc); break; // meta gm target
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

