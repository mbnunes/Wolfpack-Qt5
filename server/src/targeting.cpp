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
#include "guildstones.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "tilecache.h"
#include "wpscriptmanager.h"
#include "targetrequests.h"
#include "maps.h"
#include "classes.h"
#include "network/uosocket.h"
#include "gumps.h"
#include "network.h"
#include "skills.h"
#include "multis.h"

#undef DBGFILE
#define DBGFILE "targeting.cpp"
//#include "debug.h"


void cTargets::PlVBuy(int s)//PlayerVendors
{
	if (s == -1) 
		return;
	int v = addx[s];
	P_CHAR pc = FindCharBySerial(v);
	if (pc->free) return;
	P_CHAR pc_currchar = currchar[s];

	P_ITEM pBackpack = pc_currchar->getBackpack();
	if (!pBackpack) {sysmessage(s,"Time to buy a backpack"); return; } //LB

	int serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi=FindItemBySerial(serial);		// the item
	if (pi==NULL) return;
	if (pi->isInWorld()) return;
	int price=pi->value;

	P_ITEM np = dynamic_cast<P_ITEM>(pi->container());		// the pack
	P_CHAR npc = GetPackOwner(np);				// the vendor
	if(npc != pc || pc->npcaitype() != 17) return;

	if (pc_currchar->Owns(pc))
	{
		pc->talk( tr("I work for you, you need not buy things from me!"), -1, 0 );
		return;
	}

	int gleft=pc_currchar->CountGold();
	if (gleft<pi->value)
	{
		pc->talk( tr("You cannot afford that."), -1, 0 );
		return;
	}
	pBackpack->DeleteAmount(price,0x0EED);	// take gold from player

	pc->talk( tr("Thank you."), -1, 0 );
	pc->setHoldg(pc->holdg() + pi->value); // putting the gold to the vendor's "pocket"

	// sends item to the proud new owner's pack
	pBackpack->addItem(pi);
	pi->update();

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
			QDomElement* DefSection = DefManager->getSection( WPDT_MULTI, QString("%1").arg(addid3[s]) );
			if( !DefSection->isNull() )
			{
				UI32 houseid = 0;
				QDomNode childNode = DefSection->firstChild();
				while( !childNode.isNull() && houseid == 0 )
				{
					if( childNode.isElement() && childNode.nodeName() == "id" )
						houseid = childNode.toElement().text().toUInt();
					childNode = childNode.nextSibling();
				}
//				if( houseid != 0 )
//					attachPlaceRequest( s, new cBuildMultiTarget( QString("%1").arg(addid3[s]), currchar[s]->serial, INVALID_SERIAL ), houseid );
			}
			return; // Morrolan, here we WANT fall-thru, don't mess with this switch
		}
	}
	bool pileable = false;
	short id=(addid1[s]<<8)+addid2[s];
	tile_st tile = cTileCache::instance()->getTile( id );
	if (tile.flag2&0x08) pileable=true;

	P_ITEM pi = Items->SpawnItem(currchar[s], 1, "#", pileable, id, 0,0);
	if(!pi) return;
	pi->priv=0;	//Make them not decay
	pi->MoveTo(pp->TxLoc,pp->TyLoc,pp->TzLoc+cTileCache::instance()->tileHeight(pp->model));

	pi->update();
	addid1[s]=0;
	addid2[s]=0;
}



static void KeyTarget(int s, P_ITEM pi) // new keytarget by Morollan
{
	/*if (pi)
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
			else if( pi->type() == 117 && ( pi->type2() == 3 || pi->type2() == 2 ) )
			{
				cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial(pi->tags.get("boatserial").toUInt()));
				if( pBoat != NULL )
				{
					pBoat->switchPlankState( pi );
					pi->update();
				}
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
	}//if*/
}

void cTargets::IstatsTarget(int s)
{
	if ((buffer[s][7]==0)&&(buffer[s][8]==0)&&(buffer[s][9]==0)&&(buffer[s][10]==0))
	{
		/*Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
		sprintf((char*)temp, "Item [Static] ID [%x %x]",buffer[s][0x11], buffer[s][0x12]);
		sysmessage(s, (char*)temp);
		sprintf((char*)temp, "ID2 [%i], Height [%i]",((buffer[s][0x11]<<8)+buffer[s][0x12]), tile.height);
		sysmessage(s, (char*)temp);*/
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
				pi->more1(),pi->more2(),pi->more3(),pi->more4(),
				pi->pos.x,pi->pos.y,pi->pos.z,pi->amount(), pi->priv);
			sysmessage(s, (char*)temp);
			sprintf((char*)temp,"STR [%d] HP/MAX [%d/%d] Damage [%d-%d] Defence [%d] Rank [%d] SecureIt [%d] MoreXYZ [%i %i %i] Poisoned [%i] Weight [%d] Owner [%x] Creator [%s] MadeValue [%i] Value [%i] Decaytime[%i] Decay [%i] GoodType[%i] RandomValueRate[%i]",
				pi->st, pi->hp(),pi->maxhp(), pi->lodamage(), pi->hidamage(),pi->def,pi->rank,( pi->secured() ) ? 1 : 0,
				pi->morex, pi->morey, pi->morez,pi->poisoned,
				pi->weight(), pi->ownserial, // Ison 2-20-99
				pi->creator.latin1(),pi->madewith,pi->value,int(double(int(pi->decaytime-uiCurrentTime)/MY_CLOCKS_PER_SEC)),(pi->priv)&0x01,pi->good,pi->rndvaluerate); // Magius(CHE) (2)
				sysmessage(s,(char*)temp); // Ison 2-20-99
			// End Modified lines
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
		pi->update();
	return pi;
}

// public !!!
P_CHAR cTargets::NpcMenuTarget(int s)
{
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return NULL;
	return cCharStuff::createScriptNpc(s, NULL, QString("%1").arg(addmitem[s]));
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
			pi->update();
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

static void AddNpcTarget(int s, PKGx6C *pp)
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	P_CHAR pc = new cChar;
	if ( pc == NULL )
		return;
	pc->Init();
	pc->name = "Dummy";
	pc->setId(static_cast<ushort>(addid1[s] << 8)+addid2[s]);
	pc->setXid(pc->id());
	pc->setSkin(0);
	pc->setXSkin(0);
	pc->setPriv(0x10);
	pc->pos.x=pp->TxLoc;
	pc->pos.y=pp->TyLoc;
	pc->pos.z=pp->TzLoc+cTileCache::instance()->tileHeight(pp->model);
	cMapObjects::getInstance()->add(pc); // add it to da regions ...
	pc->isNpc();
	updatechar(pc);
}

static void Tiling(int s, PKGx6C *pp) // Clicking the corners of tiling calls this function - Crwth 01/11/1999
{
	/*if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
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
			pi->update();
		}

	addid1[s]=0;
	addid2[s]=0;*/
}

//public !!
void cTargets::Wiping(int s) // Clicking the corners of wiping calls this function - Crwth 01/11/1999
{
/*	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return;

	if (clickx[s]==-1 && clicky[s]==-1) {
		clickx[s]=(buffer[s][11]<<8)+buffer[s][12];
		clicky[s]=(buffer[s][13]<<8)+buffer[s][14];
//		if (addid1[s]) target(s,0,1,0,199,"Select second corner of inverse wiping box.");
//		else target(s,0,1,0,199,"Select second corner of wiping box.");
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
	}*/
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
	if(lineOfSight( pc_currchar->pos, clTemp4, WALLS_CHIMNEYS + DOORS + ROOFING_SLANTED))
	{
		P_ITEM pi = FindItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
		if (pi != NULL) // crashfix LB
		{
			pi->moveTo( clTemp4 );
			//pi->setContSerial(INVALID_SERIAL);
			pi->setGMMovable(); //make item unmovable once thrown
			movingeffect2(pc_currchar, pi, 0x0F, 0x0D, 0x11, 0x00, 0x00);
			pi->update();
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
			i->update();
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
	pi1->update();

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
/*
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
	pi1->update();


	if(pi3->morey)	//if it's a human corpse
	{
//		pc_currchar->karma-=100; // Ripper..lose karma and criminal.
		pc_currchar->setKarma( pc_currchar->karma() - 100 );
		sysmessage(s,"You have lost some karma!");
		criminal( pc_currchar );
		//create the Head
		sprintf((char*)temp,"the head of %s",pi3->name2().ascii());
		P_ITEM pi = Items->SpawnItem(s, pc_currchar,1,(char*)temp,0,0x1D,0xA0,0,0,0);
		if(pi == NULL) return;
		pi3->addItem(pi);
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
		QStringList carveList = DefManager->getList( pi3->carve() );

		QStringList::const_iterator it = carveList.begin();
		while( it != carveList.end() )
		{
			P_ITEM nItem = Items->createScriptItem( (*it) );
			if( nItem )
			{
				nItem->setContSerial( pi3->serial );
				nItem->pos.x=20+(rand()%50);
				nItem->pos.y=85+(rand()%75);
				nItem->pos.z=9;
				nItem->update();
			}
			it++;
		}
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
			pj->update();
		}
		Items->DeleItem(pi3);	//and then delete the corpse
	}
*/
}


void cTargets::AttackTarget(int s)
{
	P_CHAR target  = FindCharBySerial(addx[s]);
	P_CHAR target2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));
	if ( !target2 || !target ) 
		return;

    if (target->inGuardedArea()) // Ripper..No pet attacking in town.
	{
        sysmessage(s,"You cant have pets attack in town!");
        return;
	}
	target2->attackTarget( target );
}

void cTargets::FollowTarget(int s)
{
	// IS NOW STORED IN socket->tempInt(); !!!
    // LEGACY
	P_CHAR char1 = FindCharBySerial(addx[s]);
	P_CHAR char2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));

	if ( !(char1 && char2) ) // They were not found, could be bad formed packet.
		return;

	char1->setFtarg( char2->serial );
	char1->setNpcWander(1);
}

void cTargets::TransferTarget(int s)
{
	P_CHAR pc1 = FindCharBySerial(addx[s]);
	P_CHAR pc2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));
	if ( !( pc1 && pc2 ) ) // They were not found, could be bad formed packet.
		return;

	pc1->emote( tr("%1 will now take %2 as his master.").arg(pc1->name.latin1()).arg(pc2->name.latin1()), -1 );

	if (pc1->ownserial() != -1) 
		pc1->SetOwnSerial(-1);
	pc1->SetOwnSerial(pc2->serial);
	pc1->setNpcWander(1);

	pc1->setFtarg(INVALID_SERIAL);
	pc1->setNpcWander(0);
}

void cTargets::BuyShopTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if ((pc->serial==serial))
		{
			Targ->BuyShop(s, pc);
			return;
		}
		sysmessage(s, "Target shopkeeper not found...");
	}
}

int cTargets::BuyShop( UOXSOCKET s, P_CHAR pc )
{
	/*P_ITEM pCont1=NULL, pCont2=NULL;

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
	return 1;*/
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
//		pc->priv2 |= 8; 
		pc->setPriv2(pc->priv2() | 8);
		// staticeffect(i, 0x37, 0x09, 0x09, 0x19); 
		staticeffect3(pc->pos.x + 1, pc->pos.y + 1, pc->pos.z + 10, 0x37, 0x09, 0x09, 0x19, 0); 
		pc->soundEffect( 0x0208 ); 
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
//		pc->priv2 = pc->priv2&0xf7; // unhide - AntiChrist 
		pc->setPriv2(pc->priv2()&0xf7);
		// we cant use staticeffect cause the char is invis and 
		// regular chars could see it. Instead we will use the staticeffect3 
		// which takes the char coords. 
		// staticeffect(i, 0x37, 0x09, 0x09, 0x19); 
		staticeffect3(pc->pos.x + 1, pc->pos.y + 1, pc->pos.z + 10, 0x37, 0x09, 0x09, 0x19, 0); 
		pc->soundEffect( 0x0208 ); 
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
		pc->setSpeech( addx[s] );
	}
}

static void SetSpAttackTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setSpAttack(tempint[s]);
	}
}

void cTargets::SetSpaDelayTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setSpaDelay( tempint[s] );
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
		pc->soundEffect( 0x01F2 );
		staticeffect(pc, 0x37, 0x6A, 0x09, 0x06);
		pc->setMn( pc->in() );
		pc->setHp( pc->st() );
		pc->setStm(pc->effDex() );
		updatestats(pc, 0);
		updatestats(pc, 1);
		updatestats(pc, 2);
		return;
	}
	sysmessage(s,"That is not a person.");
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
			pi->update();
			return;
		}
	}
	else
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc != NULL)
		{
			pc->setDir(addx[s]);
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
	pc->resurrect();
	return true;
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
		pi->update();
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
		pi->update();
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
		pi->update();
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
		pi->update();
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

	serial = calcserial(pSign->more1(), pSign->more2(), pSign->more3(), pSign->more4());
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
	
	dynamic_cast< cHouse* >(pHouse)->removeKeys();
	
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
		pi3->update();
	}
	pi3->setMore1( static_cast<unsigned char>((pHouse->serial&0xFF000000)>>24) );
	pi3->setMore2( static_cast<unsigned char>((pHouse->serial&0x00FF0000)>>16) );
	pi3->setMore3( static_cast<unsigned char>((pHouse->serial&0x0000FF00)>>8) );
	pi3->setMore4( static_cast<unsigned char>((pHouse->serial&0x000000FF)) );
	pi3->setType( 7 );
	
	sysmessage(s, "You have transferred your house to %s.", pc->name.latin1());
	sprintf((char*)temp, "%s has transferred a house to %s.", currchar[s]->name.latin1(), pc->name.latin1());

	int k;
	for(k=0;k<now;k++)
		if(k!=s && ( (perm[k] && inrange1p(currchar[k], currchar[s]) )||
			(currchar[k]->serial==o_serial)))
			sysmessage(k, (char*)temp);
}

void cTargets::HouseEjectTarget(int s) // crackerjack 8/11/99 - kick someone out of house
{
/*	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
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
*/
}

void cTargets::HouseBanTarget(int s) 
{
	Targ->HouseEjectTarget(s);	// first, eject the player

	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];
	if (!pc)
		return;
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_home->pos ) );
	if( pHouse )
	{
		if (pc->serial == pc_home->serial) return;
		pHouse->addBan(pc);
		sysmessage(s, "%s has been banned from this house.", pc->name.latin1());
	}
}

void cTargets::HouseFriendTarget(int s) // crackerjack 8/12/99 - add somebody to friends list
{
	P_CHAR Friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];

	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_home->pos ) );

	if(Friend && pHouse)
	{
		if(Friend->serial == pc_home->serial)
		{
			sysmessage(s,"You are already the owner!");
			return;
		}
		pHouse->addFriend(Friend);
		sysmessage(s, "%s has been made a Friend of the house.", Friend->name.latin1());
	}
}

void cTargets::HouseUnBanTarget(int s)
{
	P_CHAR pc_banned = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_owner->pos ) );

	if(pc_banned && pHouse)
	{
		if(pc_banned->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeBan(pc_banned);
		sysmessage(s,"%s has been UnBanned!",pc_banned->name.latin1());
	}
	return;
}

void cTargets::HouseUnFriendTarget(int s)
{
	P_CHAR pc_friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_owner->pos ) );

	if(pc_friend && pHouse)
	{
		if(pc_friend->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeFriend(pc_friend);
		sysmessage(s,"%s is no longer a Friend of this home!", pc_friend->name.latin1());
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

		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi )
		{
			if(pi->isLockedDown())
			{
				sysmessage(s,"That item is already locked down, release it first!");
				return;
			}
			pi->setLockedDown();	// LOCKED DOWN!
			pi->setOwnSerialOnly(currchar[s]->serial);
			pi->update();
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

		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi && pi->type() == 1 )
		{
		    pi->setLockedDown();	// LOCKED DOWN!
			pi->setSecured( true );
			pi->setOwnSerialOnly(currchar[s]->serial);
			pi->update();
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
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		if (pi->secured() && !pc_currchar->Owns(pi))
		{
			sysmessage(s,"You cannot do that!");
			return;
		}
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
		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi && pi->isLockedDown() || pi->type() == 1 )
		{
			pi->setAllMovable();	// Default as stored by the client, perhaps we should keep a backup?
			pi->setSecured( false );
			pi->update();
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
		pc->setKills( addmitem[s] );
		setcharflag(pc);
	}
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
	pPet->setFtarg(currchar[s]->serial);
	pPet->setNpcWander(1);
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
		if (pc->dead())
		{
			Targ->NpcResurrectTarget(pc);
			return;
		}
	}
}


void cTargets::LoadCannon(int s)
{
	/*int serial=LongFromCharPtr(buffer[s]+7);
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
	}*/
}

void cTargets::MultiTarget(cUOSocket* socket) // If player clicks on something with the targetting cursor
{
	UOXSOCKET s = toOldSocket( socket );
	targetok[ s ] = 0;

	PKGx6C tbuf, *pt = &tbuf;
	UI32 tSerial = LongFromCharPtr( buffer[s]+2 );
	pt->Tnum = buffer[s][5];
	pt->Tserial = LongFromCharPtr(buffer[s]+7);
	pt->TxLoc = ShortFromCharPtr(buffer[s]+11);
	pt->TyLoc = ShortFromCharPtr(buffer[s]+13);
	pt->TzLoc = buffer[s][16];
	pt->model = ShortFromCharPtr(buffer[s]+17);

	// targetRequest
/*	if( buffer[s][2] == 0xFE )
	{
		// Unrequested Target info
		if( targetRequests.find( s ) == targetRequests.end() )
		{
			// Should not happen too often
			sysmessage( s, "Unrequested target info" );
			return;
		}

		if( targetRequests[ s ]->targetId() != tSerial )
		{
			sysmessage( s, "Wrong target info" );
			return;
		}

		// If the user cancels call timedout
		/*if( pt->TxLoc == -1 && pt->TyLoc == -1 && pt->Tserial == 0 && pt->model == 0 )
			targetRequests[ s ]->timedout( s );
		else
			targetRequests[ s ]->responsed( s, *pt );* //

		delete targetRequests[ s ];
		targetRequests.erase( targetRequests.find( s ) );

		return; // No further processing
	}
*/
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
//		case 1: { cRenameTarget		T(ps);		T.process();} break;
//		case 4: DyeTarget(s); break;
//		case 5: { cNewzTarget		T(ps);		T.process();} break;
		case 6: if (Iready) pi->setType( addid1[s] ); break; //Typetarget
//		case 7: Targ->IDtarget(s); break;
//		case 8:	XgoTarget(s); break;
//		case 10: ItemTarget(ps,pt); break;//MoreTarget
		case 11: if (Iready) KeyTarget(s,pi); break;
		case 12: Targ->IstatsTarget(s); break;
//		case 13: if (Cready) CstatsTarget(ps,pc); break;
//		case 14: if (Cready) GMTarget(ps,pc); break;
//		case 15: if (Cready) CnsTarget(ps,pc); break;
//		case 16: if (Cready) KillTarget(pc, 0x0b); break;
//		case 17: if (Cready) KillTarget(pc, 0x10); break;
//		case 18: if (Cready) KillTarget(pc, 0x15); break;
		case 19: if (Cready) pc->setFontType( addid1[s] ); break;
//		case 20: Targ->GhostTarget(s); break;
		case 21: Targ->ResurrectionTarget(s); break; // needed for /resurrect command
		case 26: Targ->AddItem( s ); break;
		case 27: Targ->NpcMenuTarget(s); break;
//		case 28: ItemTarget(ps,pt); break;//MovableTarget
//		case 30: if (Cready) OwnerTarget(ps,pc); else if (Iready) OwnerTarget(ps,pi); break;
//		case 31: ItemTarget(ps,pt); break;//ColorsTarget
//		case 32: Targ->DvatTarget(s); break;
		case 33: AddNpcTarget(s,pt); break;
//		case 34: if (Cready) pc->priv2|=2; break;
		case 34: if (Cready) pc->setPriv2(pc->priv2() | 2); break;
//		case 35: if (Cready) pc->priv2&=0xfd; break; // unfreeze, AntiChris used LB bugfix
		case 35: if (Cready) pc->setPriv2(pc->priv2() & 0xfd);
		case 38: Magic->Recall(s); break;
		case 39: Magic->Mark(s); break;
//		case 40: Skills->ItemIdTarget(s); break;
//		case 41: Skills->Evaluate_int_Target(s); break;
		//case 42: Skills->TameTarget(s); break;
		case 43: Magic->Gate(s); break;
		case 44: Magic->Heal(s); break; // we need this for /heal command
//		case 45: Fishing->FishTarget(ps); break;
		case 47: if (Cready) pc->setTitle( xtext[s] ); break;//TitleTarget
//		case 48: Targ->ShowAccountCommentTarget(s); break;
//		case 53: npcact(s); break;
		case 58: Targ->NpcResurrectTarget(currchar[s]); break;
		case 61: Targ->VisibleTarget(s); break;
		case 63: //MoreXTarget
		case 64: //MoreYTarget
		case 65: //MoreZTarget
		case 87: Magic->SbOpenContainer(s); break;
		case 88: Targ->SetDirTarget(s); break;

		case 100: Magic->NewCastSpell( s ); break;	// we now have this as our new spell targeting location
		case 109: Skills->BottleTarget(s); break;
		case 118: Targ->AttackTarget(s); break;
		case 119: Targ->TransferTarget(s); break;
		case 120: Targ->GuardTarget( s ); break;
		case 121: Targ->BuyShopTarget(s); break;
		case 124: Targ->FetchTarget(s); break;

		case 128: Skills->CreateBandageTarget(s); break;
		case 131: if (currchar[s]->isGM()) Targ->permHideTarget(s); break; /* not used */
		case 132: if (currchar[s]->isGM()) Targ->unHideTarget(s); break; /* not used */
		case 134: Skills->Carpentry( socket ); break;
		case 135: Targ->SetSpeechTarget(s); break;

		case 150: SetSpAttackTarget(s); break;
		case 151: Targ->FullStatsTarget(s); break;


		case 170: Targ->LoadCannon(s); break;
		case 175: Targ->SetPoisonTarget(s); break;
		case 176: Targ->SetPoisonedTarget(s); break;
		case 177: Targ->SetSpaDelayTarget(s); break;

		case 183: Skills->TinkerAxel(s); break;
		case 184: Skills->TinkerAwg(s); break;
		case 185: Skills->TinkerClock(s); break;
		case 186: vialtarget(s); break;

		case 198: Tiling(s,pt); break;
		case 199: Targ->Wiping(s); break;
		case 206: Targ->CanTrainTarget(s); break;
		case 207: ExpPotionTarget(s,pt); break;
		case 209: Targ->SetSplitTarget(s); break;
		case 210: Targ->SetSplitChanceTarget(s); break;

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
		case 240: Targ->SetMurderCount( s ); break; // Abaddon 13 Sept 1999

		case 245: 
		{
			QDomElement* DefSection = DefManager->getSection( WPDT_MULTI, QString("%1").arg(addid3[s]) );
			if( !DefSection->isNull() )
			{
				UI32 houseid = 0;
				QDomNode childNode = DefSection->firstChild();
				while( !childNode.isNull() && houseid == 0 )
				{
					if( childNode.isElement() && childNode.nodeName() == "id" )
						houseid = childNode.toElement().text().toUInt();
					childNode = childNode.nextSibling();
				}
//				if( houseid != 0 )
//					attachPlaceRequest( s, new cBuildMultiTarget( QString("%1").arg(addid3[s]), currchar[s]->serial, INVALID_SERIAL ), houseid );
			}
		}
			break;
//		case 247: Targ->ShowSkillTarget(s);break; //showskill target
//		case 248: Targ->MenuPrivTarg(s);break; // menupriv target
		case 251: Targ->NewXTarget(s); break; // NEWX
		case 252: Targ->NewYTarget(s); break; // NEWY
		case 253: Targ->IncXTarget(s); break; // INCX
		case 254: Targ->IncYTarget(s); break; // INCY
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
	TargetZ = buffer[s][16] + cTileCache::instance()->tileHeight( ( buffer[s][17] << 8) + buffer[s][18] );

	P_ITEM Item = Items->createScriptItem( ItemID );

	// No item created = fail
	if( Item == NULL )
	{
		sysmessage( s, "Unable to create item '%s'", xtext[ s ] );
		return;
	}

//	Item->setContSerial( -1 );
	Item->MoveTo( TargetX, TargetY, TargetZ );
	Item->update();
}
