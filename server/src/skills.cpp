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

/* 
*  WOLFPACK Skills
*/

#include "wolfpack.h"
#include "basics.h"
#include "itemid.h"
#include "im.h"
#include "SndPkg.h"

#undef DBGFILE
#define DBGFILE "skills.cpp"
#include "debug.h"

//int goldsmithing;
//1=iron, 2=golden, 3=agapite, 4=shadow, 5=mythril, 6=bronze, 7=verite, 8=merkite, 9=copper, 10=silver
int ingottype=0;//will hold number of ingot type to be deleted

inline void SetSkillDelay(CHARACTER cc) { 	P_CHAR pc = MAKE_CHARREF_LR(cc); SetTimerSec(&pc->skilldelay,SrvParms->skilldelay);}

//////////////////////////
// Function:	CalcRank
// History:		24 Agoust 1999 created by Magius(CHE)
//				16.9.2000 revamped a bit (Duke)
//
// Purpose:		calculate item rank based on player' skill.
//
int cSkills::CalcRank(int s,int skill)
{
	int rk_range,rank;
	float sk_range,randnum,randnum1;

	rk_range=itemmake[s].maxrank-itemmake[s].minrank;
	sk_range=(float) 50.00+chars[currchar[s]].skill[skill]-itemmake[s].minskill;
	if (sk_range<=0)
		rank=itemmake[s].minrank;
	else if (sk_range>=1000)
		rank=itemmake[s].maxrank;

	randnum = static_cast<float>(rand()%1000);
	if (randnum <= sk_range)
		rank = itemmake[s].maxrank;
	else
	{
		if (SrvParms->skilllevel==11)
			randnum1=(float) (rand()%1000);
		else
			randnum1=(float) (rand()%1000)-((randnum-sk_range)/(11-SrvParms->skilllevel));

		rank=(int) (randnum1*rk_range)/1000;
		rank+=itemmake[s].minrank-1;
		if (rank>itemmake[s].maxrank) rank=itemmake[s].maxrank;
		if (rank<itemmake[s].minrank) rank=itemmake[s].minrank;
	}
	return rank;
}

//////////////////////////
// Function:	ApplyRank
// History:		24 Agoust 1999 created by Magius(CHE)
//				16.9.2000 removed array access and revamped a bit (Duke)
//
// Purpose:		modify variables base on item's rank.
//
void cSkills::ApplyRank(int s,int i,int rank)
{
	char tmpmsg[512];
	*tmpmsg='\0';
	if(SrvParms->rank_system==0) return;
	const P_ITEM pi=MAKE_ITEMREF_LR(i);	// on error return

	if (SrvParms->rank_system==1)
	{
		pi->rank=rank;
		// Variables to change: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
		if (pi->lodamage>0) pi->lodamage=(rank*pi->lodamage)/10;
		if (pi->hidamage>0) pi->hidamage=(rank*pi->hidamage)/10;
		if (pi->att>0) pi->att=(rank*pi->att)/10;
		if (pi->def>0) pi->def=(rank*pi->def)/10;
		if (pi->hp>0) pi->hp=(rank*pi->hp)/10;
		if (pi->maxhp>0) pi->maxhp=(rank*pi->maxhp)/10;
		switch(rank)
		{
			case 1: sysmessage(s,"You made an item with no quality!"); break;
			case 2: sysmessage(s,"You made an item very below standard quality!");break;
			case 3: sysmessage(s,"You made an item below standard quality!");break;
			case 4: sysmessage(s,"You made a weak quality item!");break;
			case 5: sysmessage(s,"You made a standard quality item!");break;
			case 6: sysmessage(s,"You made a nice quality item!");break;
			case 7: sysmessage(s,"You made a good quality item!");break;
			case 8: sysmessage(s,"You made a great quality item!");break;
			case 9: sysmessage(s,"You made a beautiful quality item!");break;
			case 10: sysmessage(s,"You made a perfect quality item!");break;
		}
		sysmessage(s,tmpmsg);
	}
	else
		pi->rank=rank;
}

////////////////
// name:	cSkills::Zero_Itemmake(int s)
// history:	by Magius(CHE),24 Agoust 1999
// Purpose:	Resets all values into itemmake[s].
//
void cSkills::Zero_Itemmake(int s)
{
	itemmake[s].has=0;
	itemmake[s].has2=0;
	itemmake[s].needs=0;
	itemmake[s].minskill=0;
	itemmake[s].maxskill=0;
	itemmake[s].Mat1id=0;		// id of material used to make item
	itemmake[s].Mat1color=-1;	// color of material doesn't matter by default (Duke)
	itemmake[s].Mat2id=0;		// id of second material used to make item		(vagrant)
	itemmake[s].Mat2color=-1;	// color of material doesn't matter by default
	itemmake[s].minrank=10; // value of minum rank level of the item to create! - Magius(CHE)
	itemmake[s].maxrank=10; // value of maximum rank level of the item to create! - Magius(CHE)
	itemmake[s].number=0; // scp number of the item - used to regognize the item selected and apply Magius(CHE) rank!
}

////////////////
// name:	Skills->MakeMenutarget(int s,int x,int skill)
// history:	by Unknown
// Purpose:	create a specified menuitem target.
//
typedef int SOCK;

class cMMTsmith;		// forward declaration for cMMT factory

class cMMT				// MakeMenuTarget
{
protected:
	char* failtext;
	short badsnd1;
	short badsnd2;
public:
	cMMT(short badsnd=0, char *failmsg="You fail to create the item.")
	{
		badsnd1=badsnd>>8;
		badsnd2=badsnd&0x00FF;
		failtext=failmsg;
	}
	virtual void deletematerial(SOCK s, int amount)
	{
		P_ITEM pPack=Packitem(&chars[currchar[s]]);
		if (!pPack) return;
		int amt = max(amount, 1);

		pPack->DeleteAmount(amt,itemmake[s].Mat1id,itemmake[s].Mat1color);
		if (itemmake[s].Mat2id)						// if a 2nd material is used, delete that too
			pPack->DeleteAmount(amt,itemmake[s].Mat2id,itemmake[s].Mat2color);

/*		delequan(currchar[s],itemmake[s].Mat1id,amount>0 ? amount : 1);
		if (itemmake[s].Mat2id)												// if a 2nd material is used,
			delequan(currchar[s],itemmake[s].Mat2id,amount>0 ? amount : 1);	// delete that too */
	}
	virtual void delonfail(SOCK s)		{deletematerial(s, itemmake[s].needs/2);}
	virtual void delonsuccess(SOCK s)	{deletematerial(s, itemmake[s].needs);}
	virtual void playbad(SOCK s)		{soundeffect(s,badsnd1,badsnd2);}
	virtual void failmsg(SOCK s)		{sysmessage(s,failtext);}
	virtual void failure(SOCK s)		{delonfail(s);playbad(s);failmsg(s);}
	static cMMT* factory(short skill);
};

class cMMTsmith : public cMMT		// MakeMenuTarget for Smithing
{
public:
	cMMTsmith(short badsnd=0x002A) : cMMT(badsnd) {}
	virtual void deletematerial(SOCK s, int amount)
	{
		int p=packitem(currchar[s]);
		if (p==-1) return;
//		int ser=items[p].serial;
		P_ITEM pPack=&items[p];
		amount=(amount>0 ? amount : 1);
		switch(ingottype)
		{
			case 1: pPack->DeleteAmount(amount, 0x1BF2, 0x0961); break;//delete iron
			case 2: pPack->DeleteAmount(amount, 0x1BF2, 0x0466); break;//delete golden
			case 3: pPack->DeleteAmount(amount, 0x1BF2, 0x0150); break;//delete agapite
			case 4: pPack->DeleteAmount(amount, 0x1BF2, 0x0386); break;//delete shadow
			case 5: pPack->DeleteAmount(amount, 0x1BF2, 0x0191); break;//delete mythril
			case 6: pPack->DeleteAmount(amount, 0x1BF2, 0x02E7); break;//delete bronze
			case 7: pPack->DeleteAmount(amount, 0x1BF2, 0x022F); break;//delete verite
			case 8: pPack->DeleteAmount(amount, 0x1BF2, 0x02C3); break;//delete merkite
			case 9: pPack->DeleteAmount(amount, 0x1BF2, 0x046E); break;//delete copper
			case 10:pPack->DeleteAmount(amount, 0x1BF2, 0x0000); break;//delete silver
		}
		ingottype=0;
	}
};

cMMT* cMMT::factory(short skill)
{
	switch (skill)
	{
		case BLACKSMITHING:	return new cMMTsmith(0x002A);break; 
		case CARPENTRY:		return new cMMT(0x023D);break; 
		case INSCRIPTION:	return new cMMT(0,"You fail to inscribe the spell");break; 
		case TAILORING:		return new cMMT(0x0248);break;
		case BOWCRAFT:		return new cMMT(0x0051);break;
		case TINKERING:		return new cMMT(0x002A);break;
		default:			return new cMMT(0x0000);break;	// a generic handler
	}
}

void cSkills::MakeMenuTarget(int s, int x, int skill)
{
	int c;
	int cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	int rank=10; // For Rank-System --- Magius§(çhe)
	int tmpneed=0; // For Fixed Delquant -- Magius(CHE) §
	cMMT *targ = cMMT::factory(skill);
	
	// exploit fix - detects if they took the material out of their
	// backpack while the makemenu was active (Duke, 25.11.2000)
	int amt=getamount(cc, itemmake[s].Mat1id);
	if ( amt < itemmake[s].needs && !pc_currchar->isGM() )
	{
		sysmessage(s,"You do not have enough resources anymore!!");
		return;
	}
	
/*
	By Polygon:
	When doing cartography, check if empty map is still there
*/
	if (skill == CARTOGRAPHY)	// Is it carto?
	{
		if (!HasEmptyMap(cc))	// Did the map disappear?
		{
			sysmessage(s,"You don't have your blank map anymore!!");
			return;
		}
	}
//	END OF: By Polygon

	if(pc_currchar->making==999) {}
	else
	if(!Skills->CheckSkill(cc,skill, itemmake[s].minskill, itemmake[s].maxskill) && !pc_currchar->isGM()) //GM cannot fail! - AntiChrist
	{
		// Magius(CHE) §
		// With these 2 lines if you have a resouce item with
		// Amount=1 and fail to work on it, this resouce will be
		// Removed anyway. So noone can increase his skill using
		// a single resource item.
		tmpneed=itemmake[s].needs/2; // Magius(CHE) §
		if (tmpneed==0) itemmake[s].needs++; // Magius(CHE) §
		switch(skill) 
		{
		case BLACKSMITHING: targ->failure(s);break;
		case CARPENTRY:		targ->failure(s);break;
		case INSCRIPTION:
		case TAILORING:
		case BOWCRAFT:
		case TINKERING:		targ->failure(s);break;
		//Polygon: Do sounds, message and deletion if carto fails
		case CARTOGRAPHY:	DelEmptyMap(cc);soundeffect(s, 0x02, 0x49);sysmessage(s, "You scratch on the map but the result is unusable");break;
		default:		targ->failure(s);break;
		}
		Zero_Itemmake(s);
		return;
	}
	switch(skill)
	{	
		case BLACKSMITHING:	targ->delonsuccess(s);	break;
		case CARPENTRY:		targ->delonsuccess(s);break;
		case INSCRIPTION: delequan(cc, itemmake[s].Mat1id, 1); break;//don't use default, cauz we delete 1 scroll //use materialid
		//case TAILORING: delequan(cc, itemmake[s].materialid1,itemmake[s].materialid2, itemmake[s].needs);break;	//same as default skill
		case TINKERING:
		case BOWCRAFT:	targ->delonsuccess(s);break;
		// Polygon: Delete empty map for carto
		case CARTOGRAPHY:	if (!DelEmptyMap(cc)) return;break;
		default:
			delequan(cc, itemmake[s].Mat1id, itemmake[s].needs);
		}
		itemmake[s].Mat1id=0;
		c=Items->SpawnItemBackpack2(s, x, 0);
		if (c==-1)
		{
			LogWarningVar("bad script item # %d(Item Not found).", x);
			return;	//invalid script item
		} 
		// Starting Rank System Addon, Identify Item and Store the Creator Name- by Magius(CHE)
		const P_ITEM pi=MAKE_ITEMREF_LR(c);	// on error return
		if (pi->name2 && (strcmp(pi->name2,"#"))) 
			strcpy(pi->name,pi->name2); // Item identified! - }
		if (SrvParms->rank_system==1) rank=CalcRank(s,skill);
		else if (SrvParms->rank_system==0) rank=10;
		ApplyRank(s,c,rank);

		if(!pc_currchar->isGM())		//AntiChrist - do this only if not a GM! bugfix - to avoid "a door mixed by GM..."
		{
			strcpy(pi->creator,pc_currchar->name); // Memorize Name of the creator
			if (pc_currchar->skill[skill]>950)
				pi->madewith=skill+1; // Memorize Skill used
			else
				pi->madewith=0-skill-1; // Memorize Skill used
		}
		else
		{
			pi->creator[0]='\0';
			pi->madewith=0;
		}
		// End Rank System Addon

		pi->magic=1; // JM's bugfix

		if (skill == BLACKSMITHING			// let's see if we can make exceptional quality
			&& SrvParms->rank_system==0)	// this would interfer with the rank system
		{
			pi->rank=30; // Ripper..used for item smelting
			if(pi->color1==0x00 && pi->color2==0x00 && pi->smelt==2) pi->smelt=2; //Silver
			else if(pi->color1==0x04 && pi->color2==0x66) pi->smelt=3; //Golden
			else if(pi->color1==0x01 && pi->color2==0x50) pi->smelt=4; //Agapite
			else if(pi->color1==0x03 && pi->color2==0x86) pi->smelt=5; //Shadow
			else if(pi->color1==0x01 && pi->color2==0x91) pi->smelt=6; //Mythril
			else if(pi->color1==0x02 && pi->color2==0xE7) pi->smelt=7; //Bronze
			else if(pi->color1==0x02 && pi->color2==0x2F) pi->smelt=8; //Verite
			else if(pi->color1==0x02 && pi->color2==0xC3) pi->smelt=9; //Merkite
			else if(pi->color1==0x04 && pi->color2==0x6E) pi->smelt=10; //Copper
			else
			pi->smelt=1; //Iron

			int modifier = 1;
			int skmin = itemmake[s].minskill;
			int skcha = pc_currchar->skill[skill];
			int skdif = skcha-skmin;
			if (   skdif > 0 && skmin!=1000			// just to be sure ...
				&& skmin >= 500						// only for higher level items
				&& skdif > (rand()%(1000-skmin)))	// 1st test depends on how far the players
			{										// skill is above the requirements

				if (!(rand()%10)) modifier=10;		// 10% chance for 10% more
				if (!(rand()%100)) modifier=5;		// 1% chance for 20% more
				if (!(rand()%1000)) modifier=2;		// only one in a thousand for 50% more
				if (modifier > 1)
				{
					char p1[50], p2[50], p3[50], p4[50];
					unsigned int spaceleft=0;				// for space left in name field
					if (pi->name[0]=='#')
					{
						char tmp[100];
						pi->getName(tmp);				// get the name from tile data
						strcpy(pi->name,(char*)tmp);
					}
					spaceleft = sizeof(pi->name)-strlen(pi->name);
					switch (modifier)
					{
					case 10: strcpy(p1, " of high quality"); strcpy(p2," of h.q."); strcpy(p3,"(hQ)"); break;
					case  5: strcpy(p1 ," of very high quality"); strcpy(p2, " of v.h.q."); strcpy(p3, "(vhQ)"); break;
					case  2: strcpy(p1, " of exceptional quality"); strcpy(p2, " of e.q."); strcpy(p3, "(eQ)"); break;
					}
					if (strlen(p1) < spaceleft) strcpy(p4, p1);
					else if (strlen(p2) < spaceleft) strcpy(p4, p2);
					else if (strlen(p3) < spaceleft) strcpy(p4, p3);
					strcat(pi->name,p4);	// append name extension

					pi->hp += pi->hp / modifier;
					pi->maxhp = pi->hp;
					pi->def += pi->def / modifier;
					pi->att += pi->att / modifier;
					pi->lodamage += pi->lodamage / modifier;
					pi->hidamage += pi->hidamage / modifier;
				}
			}
		}

		if(pc_currchar->making==999)
			pc_currchar->making=c; // store item #
		else
			pc_currchar->making=0;
		if (skill==MINING) soundeffect(s,0x00,0x54); // Added by Magius(CHE)
		if (skill==BLACKSMITHING) soundeffect(s,0x00,0x2a);
		if (skill==CARPENTRY) soundeffect(s,0x02,0x3d);
		if (skill==INSCRIPTION) soundeffect(s,0x02,0x49);
		if (skill==TAILORING) soundeffect(s,0x02,0x48);
		if (skill==TINKERING) soundeffect(s,0x00,0x2A);
		// Polygon: Do the cartography sound
		if (skill==CARTOGRAPHY) soundeffect(s, 0x02, 0x49);
		
		if (skill == TAILORING) // -Fraz- Implementing color remembrance for tailored items
		{
			pi->color1=itemmake[s].newcolor1;
			pi->color2=itemmake[s].newcolor2;
			RefreshItem(pi);
		}
		if(!pc_currchar->making) sysmessage(s,"You create the item and place it in your backpack.");
//		itemmake[s].has=0;
//		itemmake[s].has2=0;
		statwindow(s,cc);
		Zero_Itemmake(s);
		/*		
		Code added by Polygon
		Creates a key for every chest-container
		and puts it in the chest
*/
		//if ((x == 2503) || (x == 726))	// Is it a wooden box?
		short id = pi->id();
		if( IsChest(id))
		{
			pi->more1 = pi->ser1;		// Store the serial number in the more-value
			pi->more2 = pi->ser2;		// Needed as unique lock-ID
			pi->more3 = pi->ser3;
			pi->more4 = pi->ser4;
			c=Items->CreateScriptItem(-1, 339, 1);
			if (c < 0) return;
			P_ITEM pik=MAKE_ITEMREF_LR(c);
			pik->type = 7;				// Item is a key
			pik->more1 = pi->more1;		// Copy the lock-number to the keys more-variable
			pik->more2 = pi->more2;		// to make it fit the lock
			pik->more3 = pi->more3;
			pik->more4 = pi->more4;
			strcpy(pik->creator,pc_currchar->name);	// Store the creator
			pik->SetContSerial(pi->serial);			// Set the container
			pik->SetRandPosInCont(pi);				// Put the damn thing in the container
			RefreshItem(c);							// Refresh it
			sysmessage(s, "You create a corresponding key and put it in the chest");
		}
//		End of: By Polygon
/*
		By Polygon:
		Give the map the correct attributes so that it
		shows the correct part of the map and has correct size
*/
		if ((x>70000) && (x < 70013)) // Is it a map?
		{
			int tlx, tly, lrx, lry;	// Tempoarly storing of topleft and lowright x,y
			if (x<70004)	// Is it a detail map? do +-250 from pos
			{
				tlx = pc_currchar->pos.x - 250;
				tly = pc_currchar->pos.y - 250;
				lrx = pc_currchar->pos.x + 250;
				lry = pc_currchar->pos.y + 250;
			}
			else if (x<70007)	// Is it a regional map? do +-500 from pos
			{
				tlx = pc_currchar->pos.x - 500;
				tly = pc_currchar->pos.y - 500;
				lrx = pc_currchar->pos.x + 500;
				lry = pc_currchar->pos.y + 500;
			}
			else if (x<70010)	// Is it a land map? do +-1000 from pos
			{
				tlx = pc_currchar->pos.x - 1000;
				tly = pc_currchar->pos.y - 1000;
				lrx = pc_currchar->pos.x + 1000;
				lry = pc_currchar->pos.y + 1000;
			}
			else				// So it's a worldmap eh?
			{
				tlx = 0;
				tly = 0;
				lrx = 0x13FF;
				lry = 0x0FFF;
			}
			// Check if we are over the borders and correct errors
			if (tlx < 0)	// Too far left?
			{
				lrx -= tlx;	// Add the stuff too far left to the right border (tlx is neg. so - and - gets + ;)
				tlx = 0;	// Set tlx to correct value
			}
			else if (lrx > 0x13FF) // Too far right?
			{
				tlx -= lrx - 0x13FF;	// Subtract what is to much from the left border
				lrx = 0x13FF;	// Set lrx to correct value
			}
			if (tly < 0)	// Too far top?
			{
				lry -= tly;	// Add the stuff too far top to the bottom border (tly is neg. so - and - gets + ;)
				tly = 0;	// Set tly to correct value
			}
			else if (lry > 0x0FFF) // Too far bottom?
			{
				tly -= lry - 0x0FFF;	// Subtract what is to much from the top border
				lry = 0x0FFF;	// Set lry to correct value
			}
			// Set the map data
			pi->more1=(tlx)>>8;		// Set topleft x
			pi->more2=(tlx)%256;	//	--- " ---
			pi->more3=(tly)>>8;		// Set topleft y
			pi->more4=(tly)%256;	//  --- " ---
			pi->moreb1=(lrx)>>8;	// Set lowright x
			pi->moreb2=(lrx)%256;	//  --- " ---
			pi->moreb3=(lry)>>8;	// Set lowright y
			pi->moreb4=(lry)%256;	//  --- " ---
		}
//		END OF: By Polygon
}

void cSkills::MakeMenu(int s, int m, int skill) // Menus for playermade objects
{ // s - character online #, m - menu to use, skill - skill being used
	int total, i;
	char lentext;
	char sect[512];
	char gmtext[30][257];
	int gmid[30];
	int gmnumber=0;
	int gmindex;
	int minres=0; // To calculate minimum resources required! By Magius(CHE) for Rank System
	int minskl=0; // To calculate minimum skill required! By Magius(CHE) for Rank System
	int tmpgmnumber=0; // By Magius(CHE) for Rank System
	make_st *imk=&itemmake[s];
	
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);

	pc_currchar->making=skill;
	
	Script *pScp=i_scripts[create_script];
	if (!pScp->Open()) return;
	sprintf(sect, "MAKEMENU %i", m);
	if (!pScp->find(sect)) 
	{
		pScp->Close();
		return;
	}
	gmindex=m;
	pScp->NextLine();
	strcpy(gmtext[0],(char*)script1);

	pScp->NextLineSplitted();
	unsigned long loopexit=0;
	do
	{
		if (script1[0]!='}')
		{
			char tmp[99];
			gmnumber++;
			tmpgmnumber++; // Magius(CHE)
			gmid[gmnumber]=hex2num(script1);
			strcpy(gmtext[gmnumber], script2);

			pScp->NextLineSplitted();
//			if (strcmp(script1,"RESOURCE"))
			if (pScp->CmpTok1("RESOURCE"))
			{
				sprintf(tmp,"create.scp, MAKEMENU %i: Expected 'RESOURCE <num>' after '%s'!", m,gmtext[gmnumber]);
				LogWarning(tmp);
				pScp->Close();
				return;
			}
			imk->needs=str2num(script2);

			pScp->NextLineSplitted();
			imk->minskill=str2num(script2);
			imk->maxskill=imk->minskill*SrvParms->skilllevel; // by Magius(CHE)
			if (strcmp((char*)script1,"SKILL"))
			{
				sprintf(tmp,"create.scp, MAKEMENU %i: Expected 'SKILL <num>' after 'RESOURCE %i'!", m,imk->needs);
				LogWarning(tmp);
				pScp->Close();
				return;
			}
			{
				char append[20];
				sprintf(append, " - [%d/%d.%d]", imk->needs,imk->minskill/10,imk->minskill%10);
				strcat(gmtext[gmnumber],append);
			}
			if (imk->maxskill<200) imk->maxskill=200;
			if ((imk->has<imk->needs) || ((imk->has2) &&
				(imk->has2<imk->needs)) || (pc_currchar->skill[skill]<imk->minskill))
				gmnumber--;

			pScp->NextLineSplitted();
			imk->number=str2num(script2);
			if (minres>imk->needs || !minres) minres=imk->needs;
			if (minskl>imk->minskill || !minskl) minskl=imk->minskill;
			if (strcmp((char*)script1,"ADDITEM") && strcmp((char*)script1,"MAKEMENU") && strcmp((char*)script1,"GOPLACE"))
			{
				sprintf(tmp,"create.scp, MAKEMENU %i: Expected 'ADDITEM/MAKEMENU <num>' after 'SKILL %i'!", m,imk->minskill);
				LogWarning(tmp);
				pScp->Close();
				return;
			}
		
			pScp->NextLineSplitted();
			if (!strcmp((char*)script1,"RANK"))	// Item Rank System - by Magius(CHE)
			{
				gettokennum((char*)script2, 0);
				imk->minrank=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				imk->maxrank=str2num(gettokenstr);
				pScp->NextLineSplitted();
			} else 
			{ // Set maximum rank if the item is not ranked!
				imk->minrank=10;
				imk->maxrank=10;
			}
			if (SrvParms->rank_system==0)
			{
				imk->minrank=10;
				imk->maxrank=10;
			}
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();
	if (!gmnumber) 
	{
		sysmessage(s,"You aren't skilled enough to make anything with what you have.");
		return; 
	}
	sprintf((char*)temp, "%i: %s", m, gmtext[0]);
	lentext=sprintf(gmtext[0], "%s", temp);
	total=9+1+lentext+1;
	for (i=1;i<=gmnumber;i++)
	{
		total+=4+1+strlen(gmtext[i]);
	}
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;
	gmprefix[3]=pc_currchar->ser1;
	gmprefix[4]=pc_currchar->ser2;
	gmprefix[5]=pc_currchar->ser3;
	gmprefix[6]=pc_currchar->ser4;
	gmprefix[7]=(gmindex+MAKEMENUOFFSET)>>8;
	gmprefix[8]=(gmindex+MAKEMENUOFFSET)%256;
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, gmtext[0], lentext);
	lentext=gmnumber;
	Xsend(s, &lentext, 1);
	for (i=1;i<=gmnumber;i++)
	{
		gmmiddle[0]=gmid[i]>>8;
		gmmiddle[1]=gmid[i]%256;
		Xsend(s, gmmiddle, 4);
		lentext=strlen(gmtext[i]);
		Xsend(s, &lentext, 1);
		Xsend(s, gmtext[i], lentext);
	}
	targetok[s]=1; 
}

void cSkills::Hide(int s) 
{ 
	int c = currchar[s]; 
	if (chars[c].attacker != INVALID_SERIAL)
	{ 
		P_CHAR pc_attacker = FindCharBySerial(chars[c].attacker);
		if (inrange1p(c, DEREF_P_CHAR(pc_attacker)))
		{
			sysmessage(s, "You cannot hide while fighting."); 
			return; 
		}
	} 
	
	if (chars[c].hidden == 1) 
	{ 
		sysmessage(s, "You are already hidden"); 
		return; 
	} 
	
	if (!Skills->CheckSkill(c, HIDING, 0, 1000)) 
	{ 
		sysmessage(s, "You are unable to hide here."); 
		return; 
	} 
	
	if (chars[c].isGM()) // add flamestrike effect for gms, LB 
	{ 
		////////////////////////////////// 
		// Change FS delay 
		// 
		// 
		staticeffect(c, 0x37, 0x09, 0x09, 0x19); 
		soundeffect2(c, 0x02, 0x08); 
		tempeffect(c, c, 33, 1, 0, 0); 
		// immediate hiding overwrites the effect. 
		// so lets hide after 4 secs. 
		// 1 sec works fine now so changed to this. 
		return; 
		// 
		// 
		// Aldur 
		////////////////////////////////// 
	} 
	sysmessage(s, "You have hidden yourself well."); 
	chars[c].hidden = 1; 
	updatechar(c); 
}

void cSkills::Stealth(int s)//AntiChrist
{
	int c=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	if (pc_currchar->hidden==0)
	{
		sysmessage(s,"You must hide first.");
		return;
	}
	if (pc_currchar->skill[HIDING]<700)
	{
		sysmessage(s,"You are not hidden well enough. Become better at hiding.");
		pc_currchar->unhide();
		return;
	}
	if (!Skills->CheckSkill(c,STEALTH, 0, 1000)) 
	{
		pc_currchar->unhide();
		return;
	}
	sprintf((char*)temp,"You can move %i steps unseen.", SrvParms->maxstealthsteps);
	sysmessage(s,(char*)temp);
	pc_currchar->hidden=1;
	pc_currchar->stealth=0; //AntiChrist -- init. steps already done
	updatechar(c);
}

void cSkills::PeaceMaking(int s)
{
	int inst, res1, res2, j;
	inst=Skills->GetInstrument(s);
	if (inst==-1) 
	{
		sysmessage(s, "You do not have an instrument to play on!");
		return;
	}
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	res1=Skills->CheckSkill(DEREF_P_CHAR(pc_currchar), PEACEMAKING, 0, 1000);
	res2=Skills->CheckSkill(DEREF_P_CHAR(pc_currchar), MUSICIANSHIP, 0, 1000);
	if (res1 && res2)
	{
		Skills->PlayInstrumentWell(s, inst);
		sysmessage(s, "You play your hypnotic music, stopping the battle.");
		
		//Char mapRegions
		cRegion::RegionIterator4Chars ri(pc_currchar->pos);
		for (ri.Begin(); ri.GetData() != ri.End(); ri++)
		{
			P_CHAR mapchar = ri.GetData();
			if (mapchar !=NULL)
			{
				if (inrange1p(DEREF_P_CHAR(mapchar), DEREF_P_CHAR(pc_currchar)) && mapchar->war)
				{
					j = calcSocketFromChar(DEREF_P_CHAR(mapchar));
					if ( j != INVALID_UOXSOCKET )
						if (perm[j]) 
							sysmessage(j, "You hear some lovely music, and forget about fighting.");
					if (mapchar->war) 
						npcToggleCombat(DEREF_P_CHAR(mapchar));
					mapchar->targ = -1;
					mapchar->attacker = INVALID_SERIAL;
					mapchar->resetAttackFirst();
				}
			}//mapitem
		}
	} 
	else 
	{
		Skills->PlayInstrumentPoor(s, inst);
		sysmessage(s, "You attempt to calm everyone, but fail.");
	}
}

void cSkills::PlayInstrumentWell(int s, int i)
{
	switch(items[i].id())
	{
	case 0x0E9C:	soundeffect2(currchar[s], 0x00, 0x38);	break;
	case 0x0E9D:
	case 0x0E9E:	soundeffect2(currchar[s], 0x00, 0x52);	break;
	case 0x0EB1:
	case 0x0EB2:	soundeffect2(currchar[s], 0x00, 0x45);	break;
	case 0x0EB3:
	case 0x0EB4:	soundeffect2(currchar[s], 0x00, 0x4C);	break;
	default:
		LogError("switch reached default");
	}
}

void cSkills::PlayInstrumentPoor(int s, int i)
{
	switch(items[i].id())
	{
	case 0x0E9C:	soundeffect2(currchar[s], 0x00, 0x39);	break;
	case 0x0E9D:
	case 0x0E9E:	soundeffect2(currchar[s], 0x00, 0x53);	break;
	case 0x0EB1:
	case 0x0EB2:	soundeffect2(currchar[s], 0x00, 0x46);	break;
	case 0x0EB3:
	case 0x0EB4:	soundeffect2(currchar[s], 0x00, 0x4D);	break;
	default:
		LogError("switch reached default");
	}
}

int cSkills::GetInstrument(int s)
{
	int x=packitem(currchar[s]);
	if (x==-1) return -1; //LB

	int ci=0,loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(items[x].serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if ( IsInstrument(pi->id()) )
		{
			return DEREF_P_ITEM(pi);
		}
	}
	return -1;
}

//////////////////////////////
// name:	DoOnePotion
// history: Duke,20.04.2000
// Purpose:	helper function for DoPotion
//			checks if player has enough regs for selected potion and deletes them
//
static bool DoOnePotion(int s,short regid, int regamount, char* regname)
{
	bool success=false;

	if (getamount(currchar[s],regid) >= regamount)
	{
		success=true;
		sprintf((char*)temp, "*%s starts grinding some %s in the mortar.*", chars[currchar[s]].name, regname);
		npcemoteall(currchar[s], (char*)temp,1); // LB, the 1 stops stupid alchemy spam
		delequan(currchar[s],regid,regamount);
	}
	else
		sysmessage(s, "You do not have enough reagents for that potion.");
	
	return success;
}

///////////////////////////
// name:	DoPotion
// history: unknown, revamped by Duke,21.04.2000
// Purpose:	determines regs and quantity, creates working sound and
//			indirectly calls CreatePotion on success
//
void cSkills::DoPotion(int s, int type, int sub, int mortar)
{
	if (sub == 0)	// user cancelled second alchemy menu 
		return;
	bool success=false;
	
	switch((type*10)+sub)
	{
	case 11: success=DoOnePotion(s,0x0F7B, 1,"blood moss");		break;//agility
	case 12: success=DoOnePotion(s,0x0F7B, 3,"blood moss");		break;//greater agility
	case 21: success=DoOnePotion(s,0x0F84, 1,"garlic");			break;//lesser cure
	case 22: success=DoOnePotion(s,0x0F84, 3,"garlic");			break;//cure
	case 23: success=DoOnePotion(s,0x0F84, 6,"garlic");			break;//greater cure
	case 31: success=DoOnePotion(s,0x0F8C, 3,"sulfurous ash");	break;//lesser explosion
	case 32: success=DoOnePotion(s,0x0F8C, 5,"sulfurous ash");	break;//explosion
	case 33: success=DoOnePotion(s,0x0F8C,10,"sulfurous ash");	break;//greater explosion
	case 41: success=DoOnePotion(s,0x0F85, 1,"ginseng");		break;//lesser heal
	case 42: success=DoOnePotion(s,0x0F85, 3,"ginseng");		break;//heal
	case 43: success=DoOnePotion(s,0x0F85, 7,"ginseng");		break;//greater heal
	case 51: success=DoOnePotion(s,0x0F8D, 1,"spider's silk");	break;//night sight
	case 61: success=DoOnePotion(s,0x0F88, 1,"nightshade");		break;//lesser poison
	case 62: success=DoOnePotion(s,0x0F88, 2,"nightshade");		break;//poison
	case 63: success=DoOnePotion(s,0x0F88, 4,"nightshade");		break;//greater poison
	case 64: success=DoOnePotion(s,0x0F88, 8,"nightshade");		break;//deadly poison
	case 71: success=DoOnePotion(s,0x0F7A, 1,"black pearl");	break;//refresh
	case 72: success=DoOnePotion(s,0x0F7A, 5,"black pearl");	break;//total refreshment
	case 81: success=DoOnePotion(s,0x0F86, 2,"mandrake");		break;//strength
	case 82: success=DoOnePotion(s,0x0F86, 5,"mandrake");		break;//greater strength
	default:
		LogErrorVar("switch reached default for <%i>",(type*10)+sub);
		return;
	}
	if (success)
	{
		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 0, 0);	// make grinding sound for a while
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 3, 0);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 6, 0);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 9, 0);
		tempeffect2(DEREF_P_CHAR(pc_currchar), &items[mortar], 10, type, sub, 0);	// this will indirectly call CreatePotion()
	}
}

///////////////////////////
// name:	CreatePotion
// history: unknown, revamped by Duke,21.04.2000
// Purpose:	does the appropriate skillcheck for the potion, creates it
//			in the mortar on success and tries to put it into a bottle
//
void cSkills::CreatePotion(int s, char type, char sub, int mortar)
{
	int success=0;

	P_CHAR pc = MAKE_CHARREF_LR(s)
	P_ITEM pi_mortar = MAKE_ITEM_REF(mortar);

	switch((10*type)+sub)
	{
	case 11:success=Skills->CheckSkill(s, ALCHEMY,151, 651);break;//agility
	case 12:success=Skills->CheckSkill(s, ALCHEMY,351, 851);break;//greater agility
	case 21:success=Skills->CheckSkill(s, ALCHEMY,  0, 500);break;//lesser cure
	case 22:success=Skills->CheckSkill(s, ALCHEMY,251, 751);break;//cure
	case 23:success=Skills->CheckSkill(s, ALCHEMY,651,1151);break;//greater cure
	case 31:success=Skills->CheckSkill(s, ALCHEMY, 51, 551);break;//lesser explosion
	case 32:success=Skills->CheckSkill(s, ALCHEMY,351, 851);break;//explosion
	case 33:success=Skills->CheckSkill(s, ALCHEMY,651,1151);break;//greater explosion
	case 41:success=Skills->CheckSkill(s, ALCHEMY,  0, 500);break;//lesser heal
	case 42:success=Skills->CheckSkill(s, ALCHEMY,151, 651);break;//heal
	case 43:success=Skills->CheckSkill(s, ALCHEMY,551,1051);break;//greater heal
	case 51:success=Skills->CheckSkill(s, ALCHEMY,  0, 500);break;//night sight
	case 61:success=Skills->CheckSkill(s, ALCHEMY,  0, 500);break;//lesser poison
	case 62:success=Skills->CheckSkill(s, ALCHEMY,151, 651);break;//poison
	case 63:success=Skills->CheckSkill(s, ALCHEMY,551,1051);break;//greater poison
	case 64:success=Skills->CheckSkill(s, ALCHEMY,901,1401);break;//deadly poison
	case 71:success=Skills->CheckSkill(s, ALCHEMY,  0, 500);break;//refresh
	case 72:success=Skills->CheckSkill(s, ALCHEMY,251, 751);break;//total refreshment
	case 81:success=Skills->CheckSkill(s, ALCHEMY,251, 751);break;//strength
	case 82:success=Skills->CheckSkill(s, ALCHEMY,451, 951);break;//greater strength
	default:
		LogError("switch reached default");
		return;
	}

	if (success==0 && !pc->isGM()) // AC bugfix
	{
		sprintf((char*)temp, "*%s tosses the failed mixture from the mortar, unable to create a potion from it.*", pc->name);
		npcemoteall(s, (char*)temp,0);
		return;
	}
	pi_mortar->type=17;
	pi_mortar->more1=type;
	pi_mortar->more2=sub;
	pi_mortar->morex=pc->skill[ALCHEMY];
	
	if (!(getamount(s, 0x0F0E)>=1))
	{
		target(calcSocketFromChar(s), 0, 1, 0, 109, "Where is an empty bottle for your potion?");
	}
	else
	{
		// Dupois - Added pouring potion sfx Oct 09, 1998
		soundeffect(s, 0x02, 0x40);	// Liquid sfx
		sprintf((char*)temp, "*%s pours the completed potion into a bottle.*", pc->name);
		npcemoteall(s, (char*)temp,0);
		delequan(s, 0x0F0E, 1);
		Skills->PotionToBottle(s, DEREF_P_ITEM(pi_mortar));
	} 
}

/////////////////////////
// name:	BottleTarget
// history: unknown, revamped by Duke,23.04.2000
// Purpose: Uses the targeted potion bottle *outside* the backpack to
//			pour in the potion from the mortar
//
void cSkills::BottleTarget(int s)
{
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	if (!pi || pi->magic==4) return;	// Ripper

	if (pi->id()==0x0F0E)	// an empty potion bottle ?
	{
		pi->ReduceAmount(1);

		int mortar=calcItemFromSer(addid1[s], addid2[s], addid3[s], addid4[s]);
		if(mortar <= -1) return;
		if (items[mortar].type==17) 
		{
			sprintf((char*)temp, "*%s pours the completed potion into a bottle.*", pc_currchar->name);
			npcemoteall(DEREF_P_CHAR(pc_currchar), (char*)temp,0);
			Skills->PotionToBottle(DEREF_P_CHAR(pc_currchar), mortar);
		}
	}
	else
		sysmessage(s,"This is not an appropriate container for a potion.");
}

/////////////////////////
// name:	PotionToBottle
// history: unknown, revamped by Duke,23.04.2000
// Purpose: this really creates the potion
//
void cSkills::PotionToBottle(CHARACTER s, int mortar)
{
	unsigned char id1,id2;
	char pn[50];

	P_CHAR pc = MAKE_CHARREF_LR(s);
	P_ITEM pi_mortar = MAKE_ITEM_REF(mortar);

	switch((10*pi_mortar->more1)+pi_mortar->more2)
	{
	case 11: id1=0x0F;id2=0x08;strcpy(pn, "an agility");				break;
	case 12: id1=0x0F;id2=0x08;strcpy(pn, "a greater agility");			break;
	case 21: id1=0x0F;id2=0x07;strcpy(pn, "a lesser cure");				break;
	case 22: id1=0x0F;id2=0x07;strcpy(pn, "a cure");					break;
	case 23: id1=0x0F;id2=0x07;strcpy(pn, "a greater cure");			break;
	case 31: id1=0x0F;id2=0x0D;strcpy(pn, "a lesser explosion");		break;
	case 32: id1=0x0F;id2=0x0D;strcpy(pn, "an explosion");				break;
	case 33: id1=0x0F;id2=0x0D;strcpy(pn, "a greater explosion");		break;
	case 41: id1=0x0F;id2=0x0C;strcpy(pn, "a lesser heal");				break;
	case 42: id1=0x0F;id2=0x0C;strcpy(pn, "a heal");					break;
	case 43: id1=0x0F;id2=0x0C;strcpy(pn, "a greater heal");			break;
	case 51: id1=0x0F;id2=0x06;strcpy(pn, "a night sight");				break;
	case 61: id1=0x0F;id2=0x0A;strcpy(pn, "a lesser poison");			break;
	case 62: id1=0x0F;id2=0x0A;strcpy(pn, "a poison");					break;
	case 63: id1=0x0F;id2=0x0A;strcpy(pn, "a greater poison");			break;
	case 64: id1=0x0F;id2=0x0A;strcpy(pn, "a deadly poison");			break;
	case 71: id1=0x0F;id2=0x0B;strcpy(pn, "a refresh");					break;
	case 72: id1=0x0F;id2=0x0B;strcpy(pn, "a total refreshment");		break;
	case 81: id1=0x0F;id2=0x09;strcpy(pn, "a strength");				break;
	case 82: id1=0x0F;id2=0x09;strcpy(pn, "a greater strength");		break;
	default:
		LogError("switch reached default");
		return;
	}
	
	int i = Items->SpawnItem(calcSocketFromChar(s),s,1,"#",0, id1, id2,0,0,1,0);
	P_ITEM pi_potion = MAKE_ITEM_REF(i);
	if (pi_potion == NULL) 
		return;
	
	sprintf(pi_potion->name,"%s potion",pn);
	pi_potion->type=19;
	pi_potion->morex = pi_mortar->morex;
	pi_potion->morey = pi_mortar->more1;
	pi_potion->morez = pi_mortar->more2;
	
	// the remainder of this function NOT (yet) revamped by Duke !
	
	// Addon for Storing creator NAME and SKILLUSED by Magius(CHE) §
	if(!pc->isGM())
	{
		strcpy(pi_potion->creator,pc->name); // Magius(CHE) - Memorize Name of the creator
		if (pc->skill[ALCHEMY]>950) pi_potion->madewith=ALCHEMY+1; // Memorize Skill used - Magius(CHE)
		else pi_potion->madewith=0-ALCHEMY-1; // Memorize Skill used - Magius(CHE)
	} else {
		pi_potion->creator[0]='\0';
		pi_potion->madewith=0;
	}
	
	RefreshItem(DEREF_P_ITEM(pi_potion));
	pi_mortar->type=0;
	// items[i].weight=100; // Ripper 11-25-99
	// AntiChrist NOTE: please! use the HARDITEMS.SCP...
	// the settings used in that script are used EVERY TIME we have an item created via
	// code ( and not via script )...so we simply can add the string:
	// WEIGHT 100
	// in the "potion section" of that file, and EVERY TIME the potions are created via scripts
	// they have those settings! :) that's easy isn't it? =P
	
	return;
}

char cSkills::CheckSkill(int c, unsigned short int sk, int low, int high)
{
	char skillused=0;
	P_CHAR pc = MAKE_CHARREF_LRV(c,0);
    UOXSOCKET s=-1;
    if(pc->isPlayer()) s=calcSocketFromChar(DEREF_P_CHAR(pc));
	
	if( pc->dead ) // fix for magic resistance exploit and probably others too, LB
	{
		sprintf((char*)temp,"Ghosts can not train %s",skillname[sk]);
		sysmessage(s, (char*)temp );
		return 0;
	}
	if (pc->isGM())
		return 1;
	if(high>1200) high=1200;

	int charrange=pc->skill[sk]-low;	// how far is the player's skill above the required minimum ?
	if(charrange<0) charrange=0;

	if (!(high-low))
	{
		LogCritical("minskill equals maxskill");
		return 0;
	}
	float chance = ((charrange*890)/(high-low))+100.0f;	// +100 means: *allways* a minimum of 10% for success
	if (chance>990) chance=990;	// *allways* a 1% chance of failure
	
	if( chance >= rand()%1000 ) skillused = 1;
	
	if(pc->baseskill[sk]<high)
	{
		if (sk!=MAGERY || (sk==MAGERY && pc->isPlayer() && currentSpellType[s]==0))
		{
			if(Skills->AdvanceSkill(DEREF_P_CHAR(pc), sk, skillused))
			{
				Skills->updateSkillLevel(DEREF_P_CHAR(pc), sk); 
				if(pc->isPlayer() && online(DEREF_P_CHAR(pc))) updateskill(s, sk);
			}
		}
	}
	return skillused;
}

char cSkills::AdvanceSkill(CHARACTER s, int sk, char skillused)
{
	int i=0, retval, incval,a,d=0;
	unsigned int ges = 0;
	unsigned char lockstate;

	int atrophy_candidates[ALLSKILLS+1];

	P_CHAR pc = MAKE_CHARREF_LRV(s, 0)


	lockstate=pc->lockSkill[sk];
	if (pc->isGM()) lockstate=0;
	// for gms no skill cap exists, also ALL skill will be interperted as up, no matter how they are set

	if (lockstate==2 || lockstate==1) return 0;// skill is locked -> forget it

	// also NOthing happens if you train a skill marked for atrophy !!!
	// skills only fall if others raise, ( osi quote ), so my interpretion
	// is that those marked as falling cant fall if you use them directly
	// exception: if you are gm its ignored! 
	
	int c=0;

	for (int b=0;b<(ALLSKILLS+1);b++) 
	{
		if (pc->lockSkill[b]==1 && pc->baseskill[b]!=0) // only count atrophy candidtes if they are above 0 !!!
		{
			atrophy_candidates[c]=b;
			c++;
		}
	}

	if (!pc->isGM())
	{
		for (a=0;a<ALLSKILLS;a++)
		{
			ges+=pc->baseskill[a];
		}
		ges=ges/10;

		if (ges>SrvParms->skillcap && c==0) // skill capped and no skill is marked as fall down.
		{
			sprintf((char*)temp,"You have reached the skill-cap of %i and no skill can fall!", SrvParms->skillcap);
			sysmessage(calcSocketFromChar(DEREF_P_CHAR(pc)),(char*)temp);
			return 0;
		}

	} else ges=0;
	
	unsigned long loopexit=0;
	while ( (wpadvance[1+i+skill[sk].advance_index].skill==sk && 
		wpadvance[1+i+skill[sk].advance_index].base<=pc->baseskill[sk]) && (++loopexit < MAXLOOPS) )
	{
		i++;
	}

	if(skillused)
		incval=(wpadvance[i+skill[sk].advance_index].success)*10;
	else
		incval=(wpadvance[i+skill[sk].advance_index].failure)*10;

	retval=0;
	//if (incval>rand()%1000)
	if (incval>rand()%SrvParms->skilladvancemodifier)
	{
		retval=1;
		pc->baseskill[sk]++;			
	}

	if (retval)
	{
		// no atrophy for gm's !! 
		if (ges>SrvParms->skillcap) // atrophy only if cap is reached !!!
		// if we are above the skill cap -> we have to let the atrophy candidates fall
		// important: we have to let 2 skills fall, or we'll never go down to cap
		// (especially if we are far above the cap from previous verisons)
		{
			int dsk = 0;	// the skill to be decreased
			if (c==1) 
			{
				dsk = atrophy_candidates[0];
				if (pc->baseskill[dsk]>=2) d=2; else d=1; // avoid value below 0 (=65535 cause unsigned)
				{ 
					if (d==1 && pc->baseskill[dsk]==0) d=0; // should never happen ...
						pc->baseskill[dsk]-=d;
					Skills->updateSkillLevel(DEREF_P_CHAR(pc), dsk); 		// we HAVE to correct the skill-value
					updateskill(calcSocketFromChar(DEREF_P_CHAR(pc)), dsk); // and send changed skill values packet so that client can re-draw correctly			
				}
			// this is very important cauz this is ONLY done for the calling skill value automatically .
			} 
			else
			{
				if (c!=0) d=rand()%c; else d=0;
				dsk = atrophy_candidates[d];
				if (pc->baseskill[dsk]>=1) 
				{
					pc->baseskill[dsk]--;
					Skills->updateSkillLevel(DEREF_P_CHAR(pc), dsk); 	
					updateskill(calcSocketFromChar(DEREF_P_CHAR(pc)), dsk); 				
				}

				if (c!=0) d=rand()%c; else d=0;
				dsk = atrophy_candidates[d];
				if (pc->baseskill[dsk]>=1) 
				{
					pc->baseskill[dsk]--;
					Skills->updateSkillLevel(DEREF_P_CHAR(pc), dsk); 	
					updateskill(calcSocketFromChar(DEREF_P_CHAR(pc)), dsk); 			
				}
			}
		}
		Skills->AdvanceStats(DEREF_P_CHAR(pc), sk);
	}
	return retval;
}

//////////////////////////////
// name:	AdvanceOneStat
// history:	by Duke, 21 March 2000
// Purpose: little helper functions for cSkills::AdvanceStats
//			finds the appropriate line for the used skill in advance table
//			and uses the value of that skill (!) to increase the stat
//			and cuts it down to 100 if necessary
//
static int calcStatIncrement(int sk, int i, int stat)
{
	unsigned long loopexit=0;
	while ((wpadvance[i+1].skill==sk &&		// if NEXT line is for same skill
			wpadvance[i+1].base <= stat*10) && (++loopexit < MAXLOOPS) )	// and is not higher than our stat
	{
		i++;								// then proceed to it !
	}
	return wpadvance[i].success;			// gather small increases
}

static int AdvanceOneStat(int sk, int i, int *stat, int *stat2, bool *update, bool aGM)
{
	*stat2 += calcStatIncrement(sk,i,*stat);// gather small increases
	if (*stat2>1000)						// until they reach 1000
	{
		*stat2 -= 1000;						// then change it
		*stat += 1;							// into one stat point
		*update=true;
	}
	if(*stat>100 && !aGM)					// cutting, but not for GMs
	{
		*stat=100;
		*update=true;
	}
	return *update;
}

///////////////////////////////
// name:	AdvanceStats
// history: revamped by Duke, 21 March 2000
// Purpose: Advance STR, DEX and INT after use of a skill
//			checks if STR+DEX+INT are higher than statcap from server.scp
//			gives all three stats the chance (from skills.scp & server.scp) to rise
//			and reduces the two other stats if necessary
//
void cSkills::AdvanceStats(CHARACTER s, int sk)
{
	int i,so;
	bool update=false;
	bool atCap=false;
	bool isGM=false;

	P_CHAR pc = MAKE_CHARREF_LR(s)

	if(pc->isGM())		// a GM ?
		isGM=true;
	if(Races[pc->race]->StrCap<=pc->st)
		atCap = true;
	if(Races[pc->race]->IntCap<=pc->in)
		atCap = true;
	if(Races[pc->race]->DexCap<=pc->realDex())
		atCap = true;
	
	i=skill[sk].advance_index;
	int mod=SrvParms->statsadvancemodifier;
	
	if (skill[sk].st>rand()%mod)
		if (AdvanceOneStat(sk, i, &(pc->st), &(pc->st2), &update, isGM) && atCap && !isGM)
			if (rand()%2) pc->chgRealDex(-1); else pc->in-=1;
	
	if (skill[sk].dx>rand()%mod)
		if (pc->incDecDex(calcStatIncrement(sk,i,pc->realDex())))
		{
			update = true;
			if (atCap)
				if (rand()%2) pc->st-=1; else pc->in-=1;
		}
	
	if (skill[sk].in>rand()%mod)
		if (AdvanceOneStat(sk, i, &(pc->in), &(pc->in2), &update, isGM) && atCap && !isGM)
			if (rand()%2) pc->chgRealDex(-1); else pc->st-=1;
	
	so=calcSocketFromChar(DEREF_P_CHAR(pc));
	if (update && (so!=-1))
	{
		statwindow(so, DEREF_P_CHAR(pc));				// update client's status window
		for (i=0; i<ALLSKILLS; i++)
		{
			updateSkillLevel(DEREF_P_CHAR(pc), i);		// update client's skill window
		}
		if (atCap && !isGM)
		{
			sprintf((char*)temp,"You have reached the stat-cap of %i!" ,SrvParms->statcap);
			sysmessage(so,(char*)temp);
		}
	}
}

void cSkills::SpiritSpeak(int s) // spirit speak time, on a base of 30 seconds + skill[SPIRITSPEAK]/50 + INT
{
	//	Unsure if spirit speaking should they attempt again?
	//	Suggestion: If they attempt the skill and the timer is !0 do not have it raise the skill
	
	if(!Skills->CheckSkill(currchar[s],SPIRITSPEAK, 0, 1000))
	{
		sysmessage(s,"You fail your attempt at contacting the netherworld.");
		return;
	}
	
	impaction(s,0x11);			// I heard there is no action...but I decided to add one
	soundeffect(s,0x02,0x4A);	// only get the sound if you are successful
	sysmessage(s,"You establish a connection to the netherworld.");
	SetTimerSec(&chars[currchar[s]].spiritspeaktimer,spiritspeak_data.spiritspeaktimer+chars[currchar[s]].in);
}

int cSkills::GetCombatSkill(int c)
{
	int skillused = WRESTLING;
	
	int ci=0,loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(chars[c].serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==1 || pi->layer==2)
		{
			if (IsSwordType(pi->id()) )
			{
				skillused = SWORDSMANSHIP;
				break;
			}
			if (IsMaceType(pi->id()) )
			{
				skillused = MACEFIGHTING;
				break;
			}
			if (IsFencingType(pi->id()) )
			{
				skillused = FENCING;
				break;
			}
			if (IsBowType(pi->id()) )
			{
				skillused = ARCHERY;
				break;
			}
		}
	}
	return(skillused);
}

void cSkills::SkillUse(int s, int x) // Skill is clicked on the skill list
{
	int cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	if (pc_currchar->cell>0)
	{
		sysmessage(s,"you are in jail and cant gain skills here!");
		return;
	}
	if (pc_currchar->dead)
	{
		sysmessage(s,"You cannot do that as a ghost.");
		return;
	}
	if (x!=STEALTH)
		pc_currchar->unhide();

	pc_currchar->disturbMed(); // Meditation

	if( pc_currchar->casting )
	{
		sysmessage( s, "You can't do that while you are casting" );
		return;
	}
	if(pc_currchar->skilldelay<=uiCurrentTime || pc_currchar->isGM())
		switch(x)
	{
	case ARMSLORE:
		target(s, 0, 1, 0, 29, "What item do you wish to get information about?");
		SetSkillDelay(cc);
		return;
	case ANATOMY:
		target(s, 0, 1, 0, 37, "Whom shall I examine?");
		SetSkillDelay(cc);
		return;
	case ITEMID:
		target(s, 0, 1, 0, 40, "What do you wish to appraise and identify?");
		SetSkillDelay(cc);
		return;
	case EVALUATINGINTEL:
		target(s, 0, 1, 0, 41, "What would you like to evaluate?");
		SetSkillDelay(cc);
		return;
	case TAMING:
		target(s, 0, 1, 0, 42, "Tame which animal?");
		SetSkillDelay(cc);
		return;
	case HIDING:
		Skills->Hide(s);
		SetSkillDelay(cc);
		return;
	case STEALTH:
		Skills->Stealth(s);
		SetSkillDelay(cc);
		return;
	case DETECTINGHIDDEN:
		target(s, 0, 1, 0, 77, "Where do you wish to search for hidden characters?");
		SetSkillDelay(cc);
		return;
	case PEACEMAKING:
		Skills->PeaceMaking(s);
		SetSkillDelay(cc);
		return;
	case PROVOCATION:
		target(s, 0, 1, 0, 79, "Whom do you wish to incite?");
		SetSkillDelay(cc);
		return;
	case ENTICEMENT:
		target(s, 0, 1, 0, 81, "Whom do you wish to entice?");
		SetSkillDelay(cc);
		return;
	case SPIRITSPEAK:
		Skills->SpiritSpeak(s);
		SetSkillDelay(cc);
		return;
	case STEALING:
		if (SrvParms->rogue)
		{
			target(s,0,1,0,205, "What do you wish to steal?");
			SetSkillDelay(cc);
			return;
		} else {
			sysmessage(s, "That skill has been disabled.");
			return;
		}
	case INSCRIPTION:
		target(s, 0, 1, 0, 160, "What do you wish to place a spell on?");
		SetSkillDelay(cc);
		return;
	case TRACKING:
		Skills->TrackingMenu(s,TRACKINGMENUOFFSET);
		SetSkillDelay(cc);
		return;
	case BEGGING:
		target(s, 0, 1, 0, 152, "Whom do you wish to annoy?");
		SetSkillDelay(cc);
		return;
	case ANIMALLORE:
		target(s, 0, 1, 0, 153, "What animal do you wish to get information about?");
		SetSkillDelay(cc);
		return;
	case FORENSICS:
		target(s, 0, 1, 0, 154, "What corpse do you want to examine?");
		SetSkillDelay(cc);
		return;
	case POISONING:
		target(s, 0, 1, 0, 155, "What poison do you want to apply?");
		SetSkillDelay(cc);
		return;

	case TASTEID:
         target(s, 0, 1, 0, 70, "What do you want to taste?");
         SetSkillDelay(cc);
         return;

	case MEDITATION:  //Morrolan - Meditation
		if(SrvParms->armoraffectmana)
		{
			Skills->Meditation(s);
			SetSkillDelay(cc);
		}
		else sysmessage(s, "Meditation is turned off.  Tell your GM to enable ARMOR_AFFECT_MANA_REGEN in server.scp to enable it.");
		return;
/*
	By Polygon:
	Added support for cartography skill
*/
	case CARTOGRAPHY:
		Skills->Cartography(s);
		SetSkillDelay(cc);
		return;
//	END OF: By Polygon
	default:
		sysmessage(s, "That skill has not been implemented yet.");
		return;
	}
	else
		sysmessage(s, "You must wait a few moments before using another skill.");
}

void cSkills::RandomSteal(int s)
{
	int p, i, skill, item;
	char temp2[512];
	tile_st tile;
	int cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);	
	int cansteal = max(1,pc_currchar->baseskill[STEALING]/10);
	cansteal = cansteal * 10;
	
	P_CHAR pc_npc = FindCharBySerPtr(buffer[s]+7);
	if (pc_npc == NULL) 
		return;

	p = packitem(DEREF_P_CHAR(pc_npc));
	if (p==-1) {sysmessage(s,"bad luck, your victim doesnt have a backpack"); return; } //LB
	
	i=0;
	vector<SERIAL> vecContainer = contsp.getData(items[p].serial);
	if (vecContainer.size() != 0)
		item = calcItemFromSer(vecContainer[rand()%vecContainer.size()]);
	else 
		item = -1;

	if (pc_npc == pc_currchar) {
		sysmessage(s,"You catch yourself red handed.");
		return;
	}
	
	if (pc_npc->npcaitype==17)
	{
		sysmessage(s, "You cannot steal that.");
		return;
	}

	// Lb, crashfix, happens if pack=empty i guess
	if (item<0 || item>=imem) 
	{ 
		sysmessage(s,"your victim doesnt have posessions");
		return;
	}

	sprintf((char*)temp, "You reach into %s's pack and try to take something...%s",pc_npc->name, items[item].name);
	sysmessage(s, (char*)temp);
	if (npcinrange(s,DEREF_P_CHAR(pc_npc),1))
	{
		if ((items[item].weight>cansteal) && (items[item].type!=1 && items[item].type!=63 &&
			items[item].type!=65 && items[item].type!=87))//Containers
		{
			sysmessage(s,"That is too heavy.");
			return;
		} else if((items[item].type==1 || items[item].type==63 || // lb bugfix, was &&
			items[item].type==65 || items[item].type==87) && (Weight->RecursePacks(item)>cansteal))
		{
			sysmessage(s,"That is too heavy.");
			return;
		}
		if (pc_npc->isGMorCounselor())
		{
			sysmessage(s, "You can't steal from gods.");
			return;
		}
		if(items[item].priv & 0x02)//newbie
		{
			sysmessage(s,"That item has no value to you.");
			return;
		}
		
		skill=Skills->CheckSkill(DEREF_P_CHAR(pc_currchar),STEALING,0,999);
		if (skill)
		{
			//pack=packitem(DEREF_P_CHAR(pc_currchar));
			items[item].SetContSerial(items[packitem(DEREF_P_CHAR(pc_currchar))].serial);
			sysmessage(s,"You successfully steal that item.");
			all_items(s);
		} else sysmessage(s, "You failed to steal that item.");
		
		if ((!skill && rand()%5+15==17) || (pc_currchar->skill[STEALING]<rand()%1001))
		{//Did they get cought? (If they fail 1 in 5 chance, other wise their skill away from 1000 out of 1000 chance)
			sysmessage(s,"You have been cought!");
			
			if (pc_npc->isNpc()) npctalkall(DEREF_P_CHAR(pc_npc), "Guards!! A thief is amoung us!",0);
			
			if (pc_npc->isInnocent() && pc_currchar->attacker != pc_npc->serial && Guilds->Compare(DEREF_P_CHAR(pc_currchar),DEREF_P_CHAR(pc_npc))==0)//AntiChrist
				criminal(DEREF_P_CHAR(pc_currchar));//Blue and not attacker and not guild
			
			if (items[item].name[0] != '#')
			{
				sprintf((char*)temp,"You notice %s trying to steal %s from you!",pc_currchar->name,items[item].name);
				sprintf(temp2,"You notice %s trying to steal %s from %s!",pc_currchar->name,items[item].name,pc_npc->name);
			} else {
				Map->SeekTile(items[item].id(),&tile);
				sprintf((char*)temp,"You notice %s trying to steal %s from you!",pc_currchar->name, tile.name);
				sprintf(temp2,"You notice %s trying to steal %s from %s!",pc_currchar->name,tile.name,pc_npc->name);
			}
			sysmessage(s,(char*)temp); // bugfix, LB
			
			for(i=0;i<now;i++)
			{
				if (perm[i])
				{
				    if((i!=s)&&(inrange1p(DEREF_P_CHAR(pc_currchar),currchar[i]))&&(rand()%10+10==17||(rand()%2==1 && chars[currchar[i]].in>=pc_currchar->in))) sysmessage(s,temp2);
				}
			}
		}
	} else sysmessage(s, "You are too far away to steal that item.");
}

void cSkills::Tracking(int s,int selection)
{
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	pc_currchar->trackingtarget = pc_currchar->trackingtargets[selection]; // sets trackingtarget that was selected in the gump
	SetTimerSec(&pc_currchar->trackingtimer,(((tracking_data.basetimer*pc_currchar->skill[TRACKING])/1000)+1)); // tracking time in seconds ... gm tracker -> basetimer+1 seconds, 0 tracking -> 1 sec, new calc by LB
	SetTimerSec(&pc_currchar->trackingdisplaytimer,tracking_data.redisplaytime);
	sprintf((char*)temp,"You are now tracking %s.",chars[pc_currchar->trackingtarget].name);
	sysmessage(s,(char*)temp);
	Skills->Track(DEREF_P_CHAR(pc_currchar));
}

void cSkills::CreateTrackingMenu(int s,int m)
{
	unsigned int i;
	int total;
	char lentext;
	char sect[512];
	char gmtext[MAXTRACKINGTARGETS][257]; // crashfix, LB
	int gmid[MAXTRACKINGTARGETS]; // crashfix, LB
	//int gmnumber;
	int id;
	int d;
	int id1=62; // default tracking animals
	int id2=399;
	int icon=8404; 
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	char type[40]="You see no signs of any animals.";
	unsigned int MaxTrackingTargets=0;
	unsigned int distance=tracking_data.baserange + pc_currchar->skill[TRACKING]/50;

	if(m==(2+TRACKINGMENUOFFSET))
	{
		id1=1;
		id2=61;
		icon=0x20d1;
		strcpy(type,"You see no signs of any creatures.");
	}
	if(m==(3+TRACKINGMENUOFFSET))
	{
		id1=400;
		id2=402;
		icon=8454;
		strcpy(type,"You see no signs of anyone.");
	}
	
	openscript("tracking.scp");
	sprintf(sect, "TRACKINGMENU %i", m);
	if(!i_scripts[tracking_script]->find(sect)) 
	{
		closescript();
		return;
	}
	
	read1();
	lentext=sprintf(gmtext[0], "%s", script1);
	
	//for (i=0;i<charcount;i++)
	//{
	
	//Char mapRegions
	int	StartGrid=mapRegions->StartGrid(pc_currchar->pos.x,pc_currchar->pos.y);
	
	cRegion::RegionIterator4Chars ri(pc_currchar->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR mapchar = ri.GetData();
		if (mapchar != NULL)
		{
			d = chardist(DEREF_P_CHAR(mapchar), currchar[s]);
			
			id = mapchar->id();
			if((d<=distance)&&(!mapchar->dead)&&(id>=id1&&id<=id2)&&calcSocketFromChar(DEREF_P_CHAR(mapchar))!=s&&(online(DEREF_P_CHAR(mapchar))||mapchar->isNpc()))
			{
				pc_currchar->trackingtargets[MaxTrackingTargets] = DEREF_P_CHAR(mapchar);
				MaxTrackingTargets++;
				if (MaxTrackingTargets>=MAXTRACKINGTARGETS) break; // lb crashfix
				switch(Skills->TrackingDirection(s,DEREF_P_CHAR(mapchar)))
				{
				case NORTH:
					strcpy((char*)temp,"to the North");
					break;
				case NORTHWEST:
					strcpy((char*)temp,"to the Northwest");
					break;
				case NORTHEAST:
					strcpy((char*)temp,"to the Northeast");
					break;
				case SOUTH:
					strcpy((char*)temp,"to the South");
					break;
				case SOUTHWEST:
					strcpy((char*)temp,"to the Southwest");
					break;
				case SOUTHEAST:
					strcpy((char*)temp,"to the Southeast");
					break;
				case WEST:
					strcpy((char*)temp,"to the West");
					break;
				case EAST:
					strcpy((char*)temp,"to the East");
					break;
				default:
					strcpy((char*)temp,"right next to you");
					break;
				}//switch
				sprintf(gmtext[MaxTrackingTargets], "%s %s",mapchar->name,temp);						
				gmid[MaxTrackingTargets]=creatures[mapchar->id()].icon; // placing correct icon, LB
			}
		}//if mapitem
	}
	
	if(MaxTrackingTargets==0)
	{
		sysmessage(s,type);
		closescript();//AntiChrist
		return;
	}
	
	total=9+1+lentext+1;
	for (i=1;i<=MaxTrackingTargets;i++) total+=4+1+strlen(gmtext[i]);
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;
	gmprefix[3]=pc_currchar->ser1;
	gmprefix[4]=pc_currchar->ser2;
	gmprefix[5]=pc_currchar->ser3;
	gmprefix[6]=pc_currchar->ser4;
	gmprefix[7]=(m+TRACKINGMENUOFFSET)>>8;
	gmprefix[8]=(m+TRACKINGMENUOFFSET)%256;
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, gmtext[0], lentext);
	Xsend(s, &MaxTrackingTargets, 1);
	for (i=1;i<=MaxTrackingTargets;i++)
	{
		gmmiddle[0]=gmid[i]>>8;
		gmmiddle[1]=gmid[i]%256;
		Xsend(s, gmmiddle, 4);
		lentext=strlen(gmtext[i]);
		Xsend(s, &lentext, 1);
		Xsend(s, gmtext[i], lentext);
	}
	closescript();
}

void cSkills::TrackingMenu(int s,int gmindex)
{
	int total, i;
	char lentext;
	char sect[512];
	char gmtext[MAXTRACKINGTARGETS][257];
	int gmid[MAXTRACKINGTARGETS];
	int gmnumber=0;
	if (gmindex<1000) 
	{
		Skills->Tracking(s,gmindex);
		return;
	}

	openscript("tracking.scp");
	sprintf(sect, "TRACKINGMENU %i",gmindex);
	if (!i_scripts[tracking_script]->find(sect)) 
	{
		closescript();
		return;
	}
	read1();
	lentext=sprintf(gmtext[0], "%s", script1);
	unsigned long loopexit=0;
	do
	{
		read2();
		if (script1[0]!='}')
		{
			gmnumber++;
			gmid[gmnumber]=hex2num(script1);
			strcpy(gmtext[gmnumber], script2);
			read1();
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	total=9+1+lentext+1;
	for (i=1;i<=gmnumber;i++) total+=4+1+strlen(gmtext[i]);
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;
	gmprefix[3]=chars[currchar[s]].ser1;
	gmprefix[4]=chars[currchar[s]].ser2;
	gmprefix[5]=chars[currchar[s]].ser3;
	gmprefix[6]=chars[currchar[s]].ser4;
	gmprefix[7]=(gmindex+TRACKINGMENUOFFSET)>>8;
	gmprefix[8]=(gmindex+TRACKINGMENUOFFSET)%256;
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, gmtext[0], lentext);
	lentext=gmnumber;
	Xsend(s, &lentext, 1);
	for (i=1;i<=gmnumber;i++)
	{
		gmmiddle[0]=gmid[i]>>8;
		gmmiddle[1]=gmid[i]%256;
		Xsend(s, gmmiddle, 4);
		lentext=strlen(gmtext[i]);
		Xsend(s, &lentext, 1);
		Xsend(s, gmtext[i], lentext);
	}
}

void cSkills::Track(int i)
{
	int s=calcSocketFromChar(i);
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	int direction=5;
	if((pc_i->pos.y-direction)>=chars[pc_i->trackingtarget].pos.y)  // North
	{
		sprintf((char*)temp,"%s is to the North",chars[pc_i->trackingtarget].name);
		if((pc_i->pos.x-direction)>chars[pc_i->trackingtarget].pos.x)
			sprintf((char*)temp,"%s is to the Northwest",chars[pc_i->trackingtarget].name);
		if((pc_i->pos.x+direction)<chars[pc_i->trackingtarget].pos.x)
			sprintf((char*)temp,"%s is to the Northeast",chars[pc_i->trackingtarget].name);
	}
	else if((pc_i->pos.y+direction)<=chars[pc_i->trackingtarget].pos.y)  // South
	{
		sprintf((char*)temp,"%s is to the South",chars[pc_i->trackingtarget].name);
		if((pc_i->pos.x-direction)>chars[pc_i->trackingtarget].pos.x)
			sprintf((char*)temp,"%s is to the Southwest",chars[pc_i->trackingtarget].name);
		if((pc_i->pos.x+direction)<chars[pc_i->trackingtarget].pos.x)
			sprintf((char*)temp,"%s is to the Southeast",chars[pc_i->trackingtarget].name);
	}
	else if((pc_i->pos.x-direction)>=chars[pc_i->trackingtarget].pos.x)  // West
	{
		sprintf((char*)temp,"%s is to the West",chars[pc_i->trackingtarget].name);
	}
	else if((pc_i->pos.x+direction)<=chars[pc_i->trackingtarget].pos.x)  // East
	{
		sprintf((char*)temp,"%s is to the East",chars[pc_i->trackingtarget].name);
	}
	else sprintf((char*)temp,"%s is right next to you",chars[pc_i->trackingtarget].name);
	
	char arrow[7];
	arrow[0]='\xBA';
	arrow[1]=1;
	arrow[2]=(chars[pc_i->trackingtarget].pos.x-1)>>8;
	arrow[3]=(chars[pc_i->trackingtarget].pos.x-1)%256;
	arrow[4]=chars[pc_i->trackingtarget].pos.y>>8;
	arrow[5]=chars[pc_i->trackingtarget].pos.y%256;
	Xsend(s,arrow,6);
}

int cSkills::TrackingDirection(int s,int i)
{
	int direction=5;
	P_CHAR pc_i = MAKE_CHARREF_LRV(i,0);
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(currchar[s],0);
	if((pc_currchar->pos.y-direction)>=pc_i->pos.y)  // North
	{
		if((pc_currchar->pos.x-direction)>pc_i->pos.x)
			return NORTHWEST;
		if((pc_currchar->pos.x+direction)<pc_i->pos.x)
			return NORTHEAST;
		return NORTH;
	}
	else if((pc_currchar->pos.y+direction)<=pc_i->pos.y)  // South
	{
		if((pc_currchar->pos.x-direction)>pc_i->pos.x)
			return SOUTHWEST;
		if((pc_currchar->pos.x+direction)<pc_i->pos.x)
			return SOUTHEAST;
		return SOUTH;
	}
	else if((pc_currchar->pos.x-direction)>=pc_i->pos.x)  // West
		return WEST;
	else if((pc_currchar->pos.x+direction)<=pc_i->pos.x)  // East
		return EAST;
	else return 0;
}
// Redone by LB on dec 28'th 1999
// Thx goes to Ripper for the design 
///////////////////////
// name:	CheckThreeSkills
// history:	by Duke, 8 April 2000
// Purpose:	little helper function for TellScroll()
//			checks the 3 skills required for engraving and cumulates
//			the returnvalues i.e. only a part==3 means success!
//
static int CheckThreeSkills(int s, int low, int high)
{
	int part=0;
	currentSpellType[s]=0;		// needed for MAGERY check
	part += Skills->CheckSkill(currchar[s], INSCRIPTION,low, high);
	part += Skills->CheckSkill(currchar[s], MAGERY,		low, high);
	part += Skills->CheckSkill(currchar[s], TINKERING,	low, high);
	return part;
}

///////////////////////
// name:	TellScroll
// history:	unknown, modified by LB & AntiChrist, 
//			partially revamped by Duke, 8 April 2000
// Purpose:	callback function for inscription menue
//			This is the point where control is returned after the user
//			selected a spell to inscribe/engrave from the gump menu.
//
void TellScroll( char *menu_name, int s, long snum )
{
	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	unsigned x,cir,spl;
	int i,k,part;

	if(snum<=0) return;				// bad spell selction

	i=pc_currchar->making;	// lets re-grab the item they clicked on
	pc_currchar->making=0;	// clear it out now that we are done with it.
	
	P_ITEM pi = MAKE_ITEM_REF(i);

	cir=(int)((snum-800)/10);		// snum holds the circle/spell as used in inscribe.gmp
	spl=(((snum-800)-(cir*10))+1);	// i.e. 800 + 1-based circle*10 + zero-based spell
									// snum is also equals the item # in items.scp of the scrool to be created !
	k=packitem(DEREF_P_CHAR(pc_currchar));
	if (k<0) return;
	
	for (x = 0; x < itemcount; x++)		// find the spellbook
	{
		P_ITEM pb = &items[x];
		if (pb->type==9 && (pb->contserial==items[k].serial ||
			(pb->layer==1 && pc_currchar->Wears(pb))))
		{
			if (!Magic->CheckBook( cir, spl-1, x))
			{
				sysmessage(s,"You don't have this spell in your spell book!");
				return;
			}
		}
	}
	
	int num=(8*(cir-1))+spl;	// circle & spell combined to a zero-based index
	
	if (spells[num].action)
		impaction(s, spells[num].action);
	npctalkall(DEREF_P_CHAR(pc_currchar), spells[num].mantra,0);
	
	if(!Magic->CheckReagents(DEREF_P_CHAR(pc_currchar), spells[num].reagents)
		|| !Magic->CheckMana(DEREF_P_CHAR(pc_currchar), num))
	{
		Magic->SpellFail(s);
		return;
	}
	Magic->SubtractMana(pc_currchar, spells[num].mana);
	
	if (pi->id()==0x0E34)  //is it a scroll?
	{
		itemmake[s].Mat1id=0x0E34; 
		itemmake[s].needs=1; 
		itemmake[s].has=getamount(DEREF_P_CHAR(pc_currchar), 0x0E34); 
		itemmake[s].minskill=(cir-1)*100;	//set range values based on scroll level
		itemmake[s].maxskill=(cir+2)*100;

		Magic->DelReagents(DEREF_P_CHAR(pc_currchar), spells[num].reagents);
		
		Skills->MakeMenuTarget(s,snum,INSCRIPTION); //put it in your pack
	}
	else if ((pi->att>0)||(pi->def>0)||(pi->hidamage)) //or is it an item?
	{
		part=0;
		switch(cir)
		{
		case 1:	part = CheckThreeSkills(s,  11, 401);break;
		case 2:	part = CheckThreeSkills(s,  61, 501);break;
		case 3:	part = CheckThreeSkills(s, 161, 601);break;
		case 4:	part = CheckThreeSkills(s, 261, 701);break;
		case 5:	part = CheckThreeSkills(s, 361, 801);break;
		case 6:	part = CheckThreeSkills(s, 461, 901);break;
		case 7:	part = CheckThreeSkills(s, 661,1101);break;
		case 8:	part = CheckThreeSkills(s, 761,1201);break;
		default:
			LogError("switch reached default");
			return;
		}
		if (part < 3)		// failure !
		{
			switch(part)
			{
			case 0:
				sysmessage(s,"Your hand jerks and you punch a hole in the item");
				pi->hp -= 3;
				break;
			case 1:
				sysmessage(s,"Your hand slips and you dent the item");
				pi->hp -= 2;
				break;
			case 2:
				sysmessage(s,"Your hand cramps and you scratch the item");
				pi->hp--;
				break;
			}
			if(pi->hp<1)
				pi->hp=1;
		}
		else				// success !
		{
			if (!( pi->morez == 0 ||(pi->morex == cir && pi->morey == spl))  )
				sysmessage(s,"This item already has a spell!");

			else if ( pi->morez >= (9 - pi->morex)*2)
				sysmessage(s,"Item at max charges!");

			else
			{
				Skills->EngraveAction(s, i, cir, spl);	// check mana & set name

				if (!(pi->morex == cir && pi->morey == spl))	// not THIS spell
				{
					pi->type2=pi->type; //kept type of item for returning to this type when item remain no charge 
					pi->type=15;  //make it magical
					pi->morex=cir;//spell circle
					pi->morey=spl;//spell number
					sysmessage(s,"Item successfully Engraved");
				}
				else
					sysmessage(s,"You added one more charge to the Item");

				pi->morez++;  // charges
				sysmessage(s,"Item successfully Engraved");
			}
		}
	}//else if
}

/////////////////////////////
// name:	Inscribe
// history:	unknown, modified by AntiChrist
//			partially revamped by Duke,8 April 2000
// Purpose:	bring up inscription menue
// Remarks:	snum is not needed here. Should change classes.h and targeting.cpp
//
int cSkills::Inscribe(int s,long snum) 
{
	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) 
	{
		LogError("Morrolan - Inscribing click on invalid object?");
		return 0;	// don't do this if the buffer got flushed - Morrolan prevents CRASH!
					// are you sure ??? that is click xLoc & click yLoc ! - Duke
	}

	int i=calcItemFromPtr(buffer[s]+7);		// Find what they clicked on
	if (i<=-1
		||!((items[i].id()==0x0E34)||  //its a scroll
			(items[i].att>0 || items[i].def>0 || items[i].hidamage>0)))	// its something else
	{
		sysmessage(s,"You could not possibly engrave on that!");
		chars[currchar[s]].making=0;
	}
	else
	{
		chars[currchar[s]].making=i;		//we gotta remember what they clicked on!

		/* select spell gump menu system here, must return control to WOLFPACK so we dont
		freeze the game. when returning to this routine, use snum to determine where to go
		and snum is also the value of what they selected. */
		im_sendmenu( "InscribeMenu", s );  //this is in im.cpp file using inscribe.gmp
	}
	return 0;
}

////////////////////////
// name:	TellScroll
// history:	unknown, modified by AntiChrist, totally revamped by Duke,8 April 2000
// Purpose:	check mana&regs and set the name of selected spell
//
int cSkills::EngraveAction(int s, int i, int cir, int spl)
{
	char *spn;					// spellname
	int num=(8*(cir-1))+spl;
	P_ITEM pi=MAKE_ITEMREF_LRV(i,0);
	Magic->DelReagents(currchar[s], spells[num].reagents);
		
	switch(cir*10 + spl)
	{
	// first circle
	case 11: spn="Clumsy";
			pi->offspell=1;
			break;
	case 12: spn="Create Food";break;   
	case 13: spn="Feeblemind";
			pi->offspell=2;
			break;
	case 14: spn="Heal";break;
	case 15: spn="Magic Arrow";
			pi->offspell=3;
			break;
	case 16: spn="Night Sight";break;
	case 17: // Reactive Armor
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 18: spn="Weaken";
			pi->offspell=4;
			break;
	// 2nd circle
	case 21: spn="Agility";break;
	case 22: spn="Cunning";break;
	case 24: spn="Harm";
			pi->offspell=5;
			break;
	case 23:// Cure
	case 25:// Magic Trap
	case 26:// Magic Untrap
	case 27:// Protection
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 28: spn="Strength";break;
	// 3rd circle
	case 31: spn="Bless";break;
	case 32:// Fireball
			spn="Daemon's Breath";
			pi->offspell=6;
			break;
	case 34: spn="Poison";break;
	case 33: //Magic lock
	case 35: //Telekinesis
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 36: spn="Teleport";break;
	case 37: spn="Unlock";break;
	case 38: spn="Wall of Stone";break;
	// 4th circle
	case 41:// Arch Cure
	case 42:// Arch Protection
	case 47:// Mana Drain
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 43:// Curse
			spn="Evil's Eye";
			pi->offspell=8;
			break;
	case 44: spn="Fire Field";break;
	case 45: spn="Greater Heal";break;
	case 46: spn="Lightning";
			pi->offspell=9;
			break;
	case 48: spn="Recall";break;
	// 5th circle
	case 51: spn="Blade Spirit";break;
	case 52: spn="Dispel Field";break;
	case 54: spn="Magic Reflection";break;
	case 55: spn="Mind Blast";
			pi->offspell=11;
			break;
	case 56://Paralyze
			spn="Ghoul's Touch";
			pi->offspell=12;
			break;
	case 57: spn="Poison Field";break;
	case 53://Incognito
	case 58://Summon Creature
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	// 6th circle
	case 61: spn="Dispel";break;
	case 62: spn="Energy Bolt";
			pi->offspell=13;
			break;  
	case 63: spn="Explosion";
			pi->offspell=14;
			break;
	case 64: spn="Invisibility";break;
	case 65: spn="Mark";break;
	case 66://Mass Curse
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 67: spn="Paralyse Field";break;
	case 68: spn="Reveal";break;
	// 7th circle
	case 72: spn="Energy Field";break;
	case 73: spn="Flamestrike";
			pi->offspell=15;
			break;
	case 74: spn="Gate Travel";break;
	case 71:// Chain Lightning
	case 75:// Mana Vampire
	case 76:// Mass Dispel
	case 77:// Meteor Storm
	case 78:// Polymorph
	// 8th circle
	case 81:// Earthquake
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 82: spn="Energy Vortex";break;
	case 83: spn="Resurrection";break;
	case 84: spn="Summon Air Elemental";break;
	case 85: spn="Summon Daemon";break;
	case 86: spn="Summon earth Elemental";break;
	case 87: spn="Summon Fire Elemental";break;
	case 88: spn="Summon Water Elemental";break;
	default:
		LogError("switch reached default");
		return 0;
	}
	sprintf(pi->name2,"of %s with", spn);
	return 1;
}

// Calculate the skill of this character based on the characters baseskill and stats
void cSkills::updateSkillLevel(int c, int s)
{
	int temp = (((skill[s].st * chars[c].st) / 100 +
		(skill[s].dx * chars[c].effDex()) / 100 +
		(skill[s].in * chars[c].in) / 100)
		*(1000-chars[c].baseskill[s]))/1000+chars[c].baseskill[s];
	
		
	chars[c].skill[s] = max(static_cast<unsigned int>(chars[c].baseskill[s]), static_cast<unsigned int>(temp));
	
}

void cSkills::TDummy(int s)
{
	//unsigned int i;
	int j,serial,hit;
	int type = Combat->GetBowType(currchar[s]);
	
	if (type > 0)
	{
		sysmessage(s, "Practice archery on archery buttes !");
		return;
	}
	int skillused = Skills->GetCombatSkill(currchar[s]);
	
	if (chars[currchar[s]].onhorse)
		Combat->CombatOnHorse(currchar[s]);
	else
		Combat->CombatOnFoot(currchar[s]);
	
	hit=rand()%3;
	switch(hit)
	{
	case 0: soundeffect(s, 0x01, 0x3B);break;
	case 1: soundeffect(s, 0x01, 0x3C);break;
	case 2: soundeffect(s, 0x01, 0x3D);break;
	default:
		LogError("switch reached default");
		return;
	}
	serial=calcserial((buffer[s][1]&0x7F),buffer[s][2],buffer[s][3],buffer[s][4]);
	j = calcItemFromSer( serial );
	if (j!=-1)
	{
		if (items[j].id()==0x1070) items[j].setId(0x1071);
		if (items[j].id()==0x1074) items[j].setId(0x1075);
		tempeffect2(0, &items[j], 14, 0, 0, 0);
		RefreshItem(j);
	}
	if(chars[currchar[s]].skill[skillused] < 300)
	{
		Skills->CheckSkill(currchar[s],skillused, 0, 1000);
		if(chars[currchar[s]].skill[TACTICS] < 300)
			Skills->CheckSkill(currchar[s],TACTICS, 0, 250);  //Dupois - Increase tactics but only by a fraction of the normal rate
	}
	else
		sysmessage(s, "You feel you would gain no more from using that.");   
}

void CollectAmmo(int s, int a, int b)
{
	int c; 
	
	if (a)
	{
		c=Items->SpawnItem(s,currchar[s],a,"#",1,0x0F,0x3F,0,0,1,1);
		items[c].att=0;
		sysmessage(s,"You collect the arrows.");
	}
	
	if (b)
	{
		c=Items->SpawnItem(s,currchar[s],b,"#",1,'\x1B','\xFB',0,0,1,1);
		if(c==-1) return;//AntiChrist to preview crashes
		items[c].att=0;
		sysmessage(s,"You collect the bolts.");
	}
}

void cSkills::AButte(int s1, P_ITEM pButte)
{
	int v1,i,c;
	int arrowsquant=0;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s1]);
	int type=Combat->GetBowType(DEREF_P_CHAR(pc_currchar));
	if(pButte->id()==0x100A)
	{ // East Facing Butte
		if ((pButte->pos.x > pc_currchar->pos.x)||(pButte->pos.y != pc_currchar->pos.y))
			v1=-1;
		else v1=pc_currchar->pos.x-pButte->pos.x;
	}
	else
	{ // South Facing Butte
		if ((pButte->pos.y>pc_currchar->pos.y)||(pButte->pos.x!=pc_currchar->pos.x))
			v1=-1;
		else v1=pc_currchar->pos.y-pButte->pos.y;
	}
	
	if(v1==1)
	{
		if(pButte->more1>0)
		{
			c=Items->SpawnItem(s1,DEREF_P_CHAR(pc_currchar),pButte->more1/2,"#",1,0x0F,0x3F,0,0,1,0);
			if(c==-1) return;
			RefreshItem(c);
		}
		
		if(pButte->more2>0)
		{
			c=Items->SpawnItem(s1,DEREF_P_CHAR(pc_currchar),pButte->more2/2,"#",1,0x1B,0xFB,0,0,1,0);
			if(c==-1) return;
			RefreshItem(c);
		}
		
		i=0;
		if(pButte->more1>0) i++;
		if(pButte->more2>0) i+=2;
		
		switch(i)
		{
		case 0:
			sprintf((char*)temp,"This target is empty");
			break;
		case 1:
			sprintf((char*)temp,"You pull %d arrows from the target",pButte->more1/2);
			break;
		case 2:
			sprintf((char*)temp,"You pull %d bolts from the target",pButte->more2/2);
			break;
		case 3:
			sprintf((char*)temp,"You pull %d arrows and %d bolts from the target",pButte->more1,pButte->more2/2);
			break;
		default:
			LogError("switch reached default");
			return;
		}
		sysmessage(s1,(char*)temp);
		pButte->more1=0;
		pButte->more2=0;
	}
	
	if((v1>=5)&&(v1<=8))
	{
		if (type == 0)
		{
			sysmessage(s1, "You need to equip a bow to use this.");
			return;
		} 
		if ((pButte->more1+pButte->more2)>99)
		{
			sysmessage(s1, "You should empty the butte first!");
			return;
		}
		if (type==1) arrowsquant=getamount(DEREF_P_CHAR(pc_currchar), 0x0F3F);
		else arrowsquant=getamount(DEREF_P_CHAR(pc_currchar), 0x1BFB);
		if (arrowsquant==0) 
		{
			sysmessage(s1, "You have nothing to fire!");
			return;
		}
		if (type==1) 
		{
			delequan(DEREF_P_CHAR(pc_currchar), 0x0F3F, 1);
			pButte->more1++;
			//add moving effect here to item, not character
		}
		else
		{
			delequan(DEREF_P_CHAR(pc_currchar), 0x1BFB, 1, NULL);
			pButte->more2++;
			//add moving effect here to item, not character
		} 
		if (pc_currchar->onhorse) Combat->CombatOnHorse(DEREF_P_CHAR(pc_currchar));
		else Combat->CombatOnFoot(DEREF_P_CHAR(pc_currchar));
		
		if (pc_currchar->skill[ARCHERY] < 350)
			Skills->CheckSkill(DEREF_P_CHAR(pc_currchar),ARCHERY, 0, 1000);
		else
			sysmessage(s1, "You learn nothing from practicing here");

		switch((pc_currchar->skill[ARCHERY]+((rand()%200)-100))/100)
		{
		case -1:
		case 0:
		case 1:
			sysmessage(s1, "You miss the target");
			soundeffect(s1, 0x02, 0x38);
			break;
		case 2:
		case 3:
			sysmessage(s1, "You hit the outer ring!");
			soundeffect(s1, 0x02, 0x34);
			break;
		case 4:
		case 5:
		case 6:
			sysmessage(s1, "You hit the middle ring!");
			soundeffect(s1, 0x02, 0x34);
			break;
		case 7:
		case 8:
		case 9:
			sysmessage(s1, "You hit the inner ring!");
			soundeffect(s1, 0x02, 0x34);
			break;
		case 10:
		case 11:
			sysmessage(s1, "You hit the bullseye!!");
			soundeffect(s1, 0x02, 0x34);
			break;
		default:
			break;
		}
	}
	if ((v1>1)&&(v1<5)||(v1>8)) sysmessage(s1, "You cant use that from here.");
}

void cSkills::Meditation(UOXSOCKET s) // Morrolan - meditation(int socket)
{
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	// blackwind warmode fix.
	if (pc_currchar->war)
	{
		sysmessage(s, "Your mind is too busy with the war thoughts.");
		return;
	}
	if (Skills->GetAntiMagicalArmorDefence(DEREF_P_CHAR(pc_currchar))>15) // blackwind armor affect fix
	{
		sysmessage(s, "Regenerative forces cannot penetrate your armor.");
		pc_currchar->med = 0;
		return;
	}
	else if (pc_currchar->getWeapon() || pc_currchar->getShield())
	{
		sysmessage(s, "You cannot meditate with a weapon or shield equipped!");
		pc_currchar->med = 0;
		return;
	}
	else if ((pc_currchar->mn) == (pc_currchar->in))
	{
		sysmessage(s, "You are at peace.");
		pc_currchar->med = 0;
		return;
	}
	else if (!Skills->CheckSkill(DEREF_P_CHAR(pc_currchar), MEDITATION, 0, 1000))
	{
		sysmessage(s, "You cannot focus your concentration.");
		pc_currchar->med = 0;
		return;
	}
	else
	{
		sysmessage(s, "You enter a meditative trance.");
		pc_currchar->med = 1;
		soundeffect(s, 0x00, 0xf9);
		return;
	}
}

//AntiChrist - 5/11/99
//
//If you are a ghost and attack a player, you can PERSECUTE him
//and his mana decreases each time you try to persecute him
//decrease=3+(your int/10)
//
void cSkills::Persecute (UOXSOCKET s) //AntiChrist - persecute stuff
{
	int c=currchar[s];
	int target=chars[c].targ;

	if (chars[target].isGM()) return;

	int decrease=(chars[c].in/10)+3;

	if((chars[c].skilldelay<=uiCurrentTime) || chars[c].isGM())
	{
		if(((rand()%20)+chars[c].in)>45) //not always
		{
			if( chars[target].mn <= decrease )
				chars[target].mn = 0;
			else 
				chars[target].mn-=decrease;//decrease mana
			updatestats(target,1);//update
			sysmessage(s,"Your spiritual forces disturb the enemy!");
			sysmessage(calcSocketFromChar(target),"A damned soul is disturbing your mind!");
			SetSkillDelay(c);

			sprintf((char*)temp, "%s is persecuted by a ghost!!", chars[target].name);
					
			// Dupois pointed out the for loop was changing i which would drive stuff nuts later
				
			for (int j=0;j<now;j++)
			{
				if((inrange1(s, j) && perm[j]) && (s!=j))
				{
					chars[c].emotecolor1=0x00;
					chars[c].emotecolor2=0x26;
					npcemote(j, target, (char*)temp, 1);
				}
			}
		} else
		{
			sysmessage(s,"Your mind is not strong enough to disturb the enemy.");
		}
	} else
	{
		sysmessage(s,"You are unable to persecute him now...rest a little...");
	}
}

void loadskills()
{
	int i, noskill, l=0;
	char sect[512];

	for (i=0;i<SKILLS;i++) // lb
	{
		skill[i].st=0;
		skill[i].dx=0;
		skill[i].in=0;
		skill[i].advance_index=l;
		noskill=0;
		openscript("skills.scp");
		sprintf(sect, "SKILL %i", i);
		if (!i_scripts[skills_script]->find(sect))
		{
			noskill=1;
		}

		unsigned long loopexit=0;
		do
		{
			read2();
			if (script1[0]!='}')
			{
				if (!(strcmp("STR", (char*)script1)))
				{
					skill[i].st=str2num(script2);
				}
				else if (!(strcmp("DEX", (char*)script1)))
				{
					skill[i].dx=str2num(script2);
				}
				else if (!(strcmp("INT", (char*)script1)))
				{
					skill[i].in=str2num(script2);
				}
				else if (!(strcmp("SKILLPOINT", (char*)script1)))
				{
					wpadvance[l].skill=i;
					gettokennum((char*)script2, 0);
					wpadvance[l].base=str2num(gettokenstr);
					gettokennum((char*)script2, 1);
					wpadvance[l].success=str2num(gettokenstr);
					gettokennum((char*)script2, 2);
					wpadvance[l].failure=str2num(gettokenstr);
					l++;
				}
			}
		}
		while ( (script1[0]!='}') && (!noskill) && (++loopexit < MAXLOOPS) );
		closescript();
	}
}

void SkillVars()
{
	strcpy(skill[ALCHEMY].madeword,"mixed");
	strcpy(skill[ANATOMY].madeword,"made");
	strcpy(skill[ANIMALLORE].madeword,"made");
	strcpy(skill[ITEMID].madeword,"made");
	strcpy(skill[ARMSLORE].madeword,"made");
	strcpy(skill[PARRYING].madeword,"made");
	strcpy(skill[BEGGING].madeword,"made");
	strcpy(skill[BLACKSMITHING].madeword,"forged");
	strcpy(skill[BOWCRAFT].madeword,"bowcrafted");
	strcpy(skill[PEACEMAKING].madeword,"made");
	strcpy(skill[CAMPING].madeword,"made");
	strcpy(skill[CARPENTRY].madeword,"made");
	strcpy(skill[CARTOGRAPHY].madeword,"wrote");
	strcpy(skill[COOKING].madeword,"cooked");
	strcpy(skill[DETECTINGHIDDEN].madeword,"made");
	strcpy(skill[ENTICEMENT].madeword,"made");
	strcpy(skill[EVALUATINGINTEL].madeword,"made");
	strcpy(skill[HEALING].madeword,"made");
	strcpy(skill[FISHING].madeword,"made");
	strcpy(skill[FORENSICS].madeword,"made");
	strcpy(skill[HERDING].madeword,"made");
	strcpy(skill[HIDING].madeword,"made");
	strcpy(skill[PROVOCATION].madeword,"made");
	strcpy(skill[INSCRIPTION].madeword,"wrote");
	strcpy(skill[LOCKPICKING].madeword,"made");
	strcpy(skill[MAGERY].madeword,"envoked");
	strcpy(skill[MAGICRESISTANCE].madeword,"made");
	strcpy(skill[TACTICS].madeword,"made");
	strcpy(skill[SNOOPING].madeword,"made");
	strcpy(skill[MUSICIANSHIP].madeword,"made");
	strcpy(skill[POISONING].madeword,"made");
	strcpy(skill[ARCHERY].madeword,"made");
	strcpy(skill[SPIRITSPEAK].madeword,"made");
	strcpy(skill[STEALING].madeword,"made");
	strcpy(skill[TAILORING].madeword,"sewn");
	strcpy(skill[TAMING].madeword,"made");
	strcpy(skill[TASTEID].madeword,"made");
	strcpy(skill[TINKERING].madeword,"made");
	strcpy(skill[TRACKING].madeword,"made");
	strcpy(skill[VETERINARY].madeword,"made");
	strcpy(skill[SWORDSMANSHIP].madeword,"made");
	strcpy(skill[MACEFIGHTING].madeword,"made");
	strcpy(skill[FENCING].madeword,"made");
	strcpy(skill[WRESTLING].madeword,"made");
	strcpy(skill[LUMBERJACKING].madeword,"made");
	strcpy(skill[MINING].madeword,"smelted");
	strcpy(skill[MEDITATION].madeword,"envoked");
	strcpy(skill[STEALTH].madeword,"made");
	strcpy(skill[REMOVETRAPS].madeword,"made");
}

int cSkills::GetAntiMagicalArmorDefence(int p)
{// blackwind
	int ar = 0;

	if (ishuman(p))
	{
		int ci = 0, loopexit = 0;
		P_ITEM pi;
		vector<SERIAL> vecContainer = contsp.getData(chars[p].serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pi = FindItemBySerial(vecContainer[ci]);
			if (pi != NULL)
			if (pi->layer>1 && pi->layer < 25)
			{
				if (!(strstr(pi->name, "leather") || strstr(pi->name, "magic") ||
					strstr(pi->name, "boot")|| strstr(pi->name, "mask")))
					ar += pi->def;
			}
		}
	}
	return ar;
}

void cSkills::Snooping(P_CHAR player, P_ITEM container)
{

	UOXSOCKET s = calcSocketFromChar(DEREF_P_CHAR(player));
	P_CHAR pc_owner = GetPackOwner(container);
	if (pc_owner->isGMorCounselor())
	{
		UOXSOCKET owner_sock = calcSocketFromChar(DEREF_P_CHAR(pc_owner));
		sysmessage(s, "You can't peek into that container or you'll be jailed.");// AntiChrist
		sprintf((char*)temp, "%s is trying to snoop you!", player->name);
		sysmessage(owner_sock, (char*)temp);
		return;
	}
	else if (Skills->CheckSkill(DEREF_P_CHAR(player), SNOOPING, 0, 1000))
	{
		backpack(s, container->serial);
		sysmessage(s, "You successfully peek into that container.");
		// Karma(currchar[s],-1,-2000);//AntiChrist
		// criminal(currchar[s]);//AntiChrist
	}
	else
	{
		sysmessage(s, "You failed to peek into that container.");
		if (player->isNpc())
			npctalk(s, DEREF_P_CHAR(player), "Art thou attempting to disturb my privacy?", 0);
		else
		{
			sprintf((char*)temp, "You notice %s trying to peek into your pack!", player->name);
			UOXSOCKET owner_sock = calcSocketFromChar(DEREF_P_CHAR(pc_owner));
			if (owner_sock != -1)
				sysmessage(owner_sock, (char*)temp); 
		}
		// Karma(currchar[s],-1,-2000);//AntiChrist
		// criminal(currchar[s]);//AntiChrist
	}
	SetTimerSec(&player->objectdelay, SrvParms->objectdelay+SrvParms->snoopdelay);//adds a delay - solarin
}


/*
	By Polygon:
	Function is called when clicked on the "Cartography button
	Builds the cartography menu
*/
void cSkills::Cartography(int s)
{
	int cc=currchar[s];	// Get the current char of the client
	if (HasEmptyMap(cc))
	{
		itemmake[s].has = 1;
		MakeMenu(s, 1200, CARTOGRAPHY);
	}
	else
		sysmessage(s, "You don't have an empty map to draw on");
}

// END OF: By Polygon

/*
	By Polygon:
	Two functions that are needed for the cartography skill to
	determine if the correct map type is in pack
*/

bool cSkills::HasEmptyMap(int cc)	// Check if the player carries an empty map
{
	P_ITEM pack;	// Variable that stores the backpack
	pack = Packitem(&chars[cc]);	// Get the packitem
	if (pack == NULL)	// Does he have a backpack?
		return false;	// No? Then no cartography
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(pack->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM cand = FindItemBySerial(vecContainer[ci]);
		if (cand->id() == 0x14ED && !cand->free)
		{
			if (cand->type == 300)	// Is it the right type
				return true;	// Yay, go on with carto
		}
	}
	return false;	// Search lasted too long, abort
}

bool cSkills::DelEmptyMap(int cc)	// Delete an empty map from the player's backpack, use HasEmptyMap before!
{
	P_ITEM pack;	// Variable that stores the backpack
	pack = Packitem(&chars[cc]);	// Get the packitem
	if (pack == NULL)	// Does he have a backpack?
	{
		// No? Very strange... Give out an error message
		LogError("No backpack found in DelEmptyMap. No HasEmptyMap performed before or serious bug occured");
		return false;	// abort the opeation if you get this
	}
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(pack->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM cand = FindItemBySerial(vecContainer[ci]);
		if (cand->id()==0x14ED && !cand->free)
		{
			if (cand->type == 300)	// Is it the right type
			{
				Items->DeleItem(DEREF_P_ITEM(cand));	// Delete it
				return true;		// Go on with cartography
			}
		}
	}
	return false;	// Search lasted too long, abort (shouldn't happen, abort if ya get this)
}

//	END OF: By Polygon

/*
	By Polygon:
	Attempt to decipher a tattered treasure map
	Called when double-clicked such a map
*/

void cSkills::Decipher(P_ITEM tmap, int s)
{
	int cc=currchar[s];		// Get the current character
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	char sect[500];			// Needed for script search
	int regtouse;			// Stores the region-number of the TH-region
	int i;					// Loop variable
	int btlx, btly, blrx, blry; // Stores the borders of the tresure region (topleft x-y, lowright x-y)
	int tlx, tly, lrx, lry;		// Stores the map borders
	int x, y;					// Stores the final treasure location

	if(pc_currchar->skilldelay<=uiCurrentTime || pc_currchar->isGM())	// Char doin something?
	{
		if (CheckSkill(s, CARTOGRAPHY, tmap->morey * 10, 1000))	// Is the char skilled enaugh to decipher the map
		{
			P_ITEM nmap;	// Stores the new map
			int newmap=Items->SpawnItemBackpack2(s, 70025, 0);
			if (newmap==-1)
			{
				LogWarning("bad script item # 70025(Item Not found).");
				return;	//invalid script item
			} 
			nmap = MAKE_ITEMREF_LR(newmap);	// Get the item
			sprintf(nmap->name, "a deciphered lvl.%d treasure map", tmap->morez);	// Give it the correct name
			nmap->morez = tmap->morez;				// Give it the correct level
			strcpy(nmap->creator, pc_currchar->name);	// Store the creator
			Script *rscript=i_scripts[regions_script];	// Region script
			if (!rscript->Open())
			{
				LogWarning("Treasure hunting cSkills::Decipher : Cannot open regions-script");
				return;
			}
			sprintf(sect, "TREASURE%i", nmap->morez);
			if (!rscript->find(sect)) 
			{
				rscript->Close();
				LogWarningVar("Treasure hunting cSkills::Decipher : Unable to find 'SECTION TREASURE%X' in regions-script", nmap->morez);
				return;
			}
			rscript->NextLine();				// Get the number of areas
			regtouse = rand()%str2num(script1);	// Select a random one
			for (i = 0; i < regtouse; i++)		// Skip the ones before the correct one
			{
				rscript->NextLine();
				rscript->NextLine();
				rscript->NextLine();
				rscript->NextLine();
			}
			rscript->NextLine();				// Get the treasure region borders
			btlx = str2num(script1);
			rscript->NextLine();
			btly = str2num(script1);
			rscript->NextLine();
			blrx = str2num(script1);
			rscript->NextLine();
			blry = str2num(script1);
			rscript->Close();					// Close the script
			if ((btlx < 0) || (btly < 0) || (blrx > 0x13FF) || (blry > 0x0FFF))	// Valid region?
			{
				sprintf(sect, "Treasure Hunting cSkills::Decipher : Invalid region borders for lvl.%d , region %d", nmap->morez, regtouse+1);	// Give out detailed warning :D
				LogWarning(sect);
				return;
			}
			x = btlx + (rand()%(blrx-btlx));	// Generate treasure location
			y = btly + (rand()%(blry-btly));
			tlx = x - 250;		// Generate map borders
			tly = y - 250;
			lrx = x + 250;
			lry = y + 250;
			// Check if we are over the borders and correct errors
			if (tlx < 0)	// Too far left?
			{
				lrx -= tlx;	// Add the stuff too far left to the right border (tlx is neg. so - and - gets + ;)
				tlx = 0;	// Set tlx to correct value
			}
			else if (lrx > 0x13FF) // Too far right?
			{
				tlx -= lrx - 0x13FF;	// Subtract what is to much from the left border
				lrx = 0x13FF;	// Set lrx to correct value
			}
			if (tly < 0)	// Too far top?
			{
				lry -= tly;	// Add the stuff too far top to the bottom border (tly is neg. so - and - gets + ;)
				tly = 0;	// Set tly to correct value
			}
			else if (lry > 0x0FFF) // Too far bottom?
			{
				tly -= lry - 0x0FFF;	// Subtract what is to much from the top border
				lry = 0x0FFF;	// Set lry to correct value
			}
			nmap->more1 = tlx>>8;	// Store the map extends
			nmap->more2 = tlx%256;
			nmap->more3 = tly>>8;
			nmap->more4 = tly%256;
			nmap->moreb1 = lrx>>8;
			nmap->moreb2 = lrx%256;
			nmap->moreb3 = lry>>8;
			nmap->moreb4 = lry%256;
			nmap->morex = x;		// Store the treasure's location
			nmap->morey = y;
			Items->DeleItem(DEREF_P_ITEM(tmap));	// Delete the tattered map
		}
		else
			sysmessage(s, "You fail to decipher the map");		// Nope :P
		// Set the skill delay, no matter if it was a success or not
		SetTimerSec(&pc_currchar->skilldelay, SrvParms->skilldelay);
		soundeffect(s, 0x02, 0x49);	// Do some inscription sound regardless of success or failure
		sysmessage(s, "You put the deciphered tresure map in your pack");	// YAY
	}
	else
		sysmessage(s, "You must wait to perform another action");	// wait a bit
}

// END OF: By Polygon
