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

#include "wolfpack.h"
#include "itemid.h"
//#include "basics.h"
#include "SndPkg.h"
#include "platform.h"
#undef DBGFILE
#define DBGFILE "skiTarg.cpp"
#include "debug.h"

P_ITEM Check4Pack(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pi_pack = Packitem(pc_currchar);
	if (pi_pack == NULL)
	{
		sysmessage(s,"Time to buy a backpack");
	}
	return pi_pack;
}

bool CheckInPack(UOXSOCKET s, PC_ITEM pi)
{
	P_ITEM pPack=Check4Pack(s);
	if (!pPack) return false;
	if (pi->contserial!=pPack->serial)
	{
		sysmessage(s,"You can't use material outside your backpack");
		return false;
	}
	return true;
}

void cSkills::Tailoring(int s)// -Frazurbluu- rewrite of tailoring 7/2001
{
	const P_ITEM pi_bolts = FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi_bolts == NULL) return; 
	short int amt=0;
	short int amt1=0;
	short int col1=pi_bolts->color1; //-Frazurbluu- added color retention for tailoring from cloth
	short int col2=pi_bolts->color2;

	if (pi_bolts && !pi_bolts->isLockedDown()) // Ripper
	{
		if (IsBoltOfCloth(pi_bolts->id()))
		{
			if (CheckInPack(s,pi_bolts))
			{
			if (pi_bolts->amount>1)
				amt1=(pi_bolts->amount*50);//-Frazurbluu- changed to reflect current OSI 
			else
				amt1=50; 
			Items->DeleItem(pi_bolts); //-Fraz- delete the bolts when ready 
			const P_ITEM npi = Items->SpawnItem(s,DEREF_P_CHAR(pc_currchar),1,"cut cloth",0,0x17,0x66,col1,col2,1,1);
			if(npi == NULL) return;// crash check
			npi->weight = 10;
			npi->amount = amt1;
			npi->pileable = 1;
			RefreshItem(npi);
			Items->DeleItem(pi_bolts);
			Weight->NewCalc(DEREF_P_CHAR(pc_currchar));
			statwindow(s,DEREF_P_CHAR(pc_currchar));
			if (!npi) return;
			amt=itemmake[s].has=getamount(DEREF_P_CHAR(pc_currchar), npi->id());
				if(amt<1)
				{ 
					sysmessage(s,"You don't have enough material to make anything.");
					return;
				}
			itemmake[s].Mat1id=npi->id();
			itemmake[s].newcolor1=npi->color1;
			itemmake[s].newcolor2=npi->color2;
			MakeMenu(s,30,TAILORING);
			}
			return;
		}
		else if ( IsCloth(pi_bolts->id()) || IsCutLeather(pi_bolts->id()) || IsCutCloth(pi_bolts->id()) || IsHide(pi_bolts->id()))  
		{
			if (CheckInPack(s,pi_bolts))
			{
				int amt=itemmake[s].has = getamount(DEREF_P_CHAR(pc_currchar), pi_bolts->id());
				if(amt<1)
				{ 
					sysmessage(s,"You don't have enough material to make anything.");
					return;
				}
				itemmake[s].Mat1id=pi_bolts->id();
				itemmake[s].newcolor1=pi_bolts->color1;
				itemmake[s].newcolor2=pi_bolts->color2;
				if ( IsCutLeather(pi_bolts->id()) || IsHide(pi_bolts->id()) )
					MakeMenu(s,40,TAILORING);
				else
					MakeMenu(s,30,TAILORING);
			}
			return;
		}
		sysmessage(s,"You cannot use that material for tailoring.");
	}
}

void cSkills::Fletching(int s)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if (( IsShaft(itemmake[s].Mat1id) && IsFeather(pi->id()) ) ||	// first clicked shaft and now feather
			( IsFeather(itemmake[s].Mat1id) && IsShaft(pi->id()) ))		// or vice versa
		{
			if (CheckInPack(s,pi))
			{
				itemmake[s].Mat2id=pi->id();	// 2nd material
				itemmake[s].has=getamount(DEREF_P_CHAR(pc_currchar), itemmake[s].Mat1id);		// count both materials
				itemmake[s].has2=getamount(DEREF_P_CHAR(pc_currchar), itemmake[s].Mat2id);
				MakeMenu(s,60,BOWCRAFT);
			}
			return;
		}
	}
	sysmessage(s,"You cannot use that for fletching.");
}

void cSkills::BowCraft(int s)
{
	P_CHAR pc_currchar = currchar[s];

	action(s,pc_currchar->onhorse ? 0x1C : 0x0D);
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		short id = pi->id();
		if( IsLog(id) || IsBoard(id) )
		{
			if (CheckInPack(s,pi))
			{
				if((itemmake[s].has=getamount(DEREF_P_CHAR(pc_currchar), pi->id())) < 2)
					sysmessage(s,"You don't have enough material to make anything.");
				else 
				{
					itemmake[s].Mat1id=pi->id();
					MakeMenu(s,65,BOWCRAFT);
				}
			}
		} 
	}
}

////////////////////
// name:	Carpentry()
// history:	unknown, rewritten by Duke, 25.05.2000
// purpose:	sets up appropriate Makemenu when player targets logs or boards
//			after dclick on carpentry tool
//			
//			If logs are targetted, Makemenu 19 is called to produce boards
//			If boards, MM 20 is called for furniture etc.
//
void cSkills::Carpentry(int s)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi && !pi->isLockedDown())
	{
		short id = pi->id();
		if( IsLog(id) || IsBoard(id) ) // logs or boards
		{
		   if (CheckInPack(s,pi))
		   {
			  itemmake[s].Mat1id = pi->id();
			  itemmake[s].has=getamount(DEREF_P_CHAR(pc_currchar),pi->id());
			  short mm = IsLog(pi->id()) ? 19 : 20; // 19 = Makemenu to create boards from logs
			  MakeMenu(s,mm,CARPENTRY);
		   }
		}
	}
	else
		sysmessage(s,"You cannot use that material for carpentry.");
}

static bool ForgeInRange(int s)
{
	P_CHAR pc = currchar[s];
	bool rc = false;

	unsigned int StartGrid=mapRegions->StartGrid(pc->pos.x,pc->pos.y);
	unsigned int increment=0, checkgrid, a;
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				P_ITEM pi = FindItemBySerial(vecEntries[k]);
				if (pi != NULL)
					if(IsForge(pi->id()))
						if(iteminrange(s,pi,3))
							rc = true;
			}
		}
	}
	return rc;
}

static bool AnvilInRange(int s)
{
	P_CHAR pc = currchar[s];
	bool rc = false;

	unsigned int StartGrid=mapRegions->StartGrid(pc->pos.x,pc->pos.y);
	unsigned int increment=0, checkgrid, a;
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				P_ITEM pi = FindItemBySerial(vecEntries[k]);
				if (pi != NULL)
					if(IsAnvil(pi->id()))
						if(iteminrange(s,pi,3))
							rc = true;
			}
		}
	}
	return rc;
}

////////////////////
// name:	AnvilTarget2
// history:	by Duke, 28 March 2000
// Purpose:	little helper function for cSkills::Smith()
//			checks for anvil in reach and enough material
//			and invokes appropriate Makemenu
//
static void AnvilTarget2(int s,				// socket #
						 P_ITEM pi,			// material item
						 int ma,			// minimum amount
						 int mm,			// makemenu # to invoke from create.scp
						 char* matname)		// name of the metal
{
	if (!AnvilInRange(s))
		sysmessage(s,"The anvil is too far away.");
	else
	{
		P_CHAR pc_currchar = currchar[s];
		P_ITEM pi_pack = Packitem(pc_currchar);
		if (pi_pack == NULL) 
			return;

		int amt = pi_pack->CountItems( pi->id(), pi->color());
		if ((itemmake[s].has=amt) < ma)
		{
			char msg[100];
			sprintf(msg,"You don't have enough %s ingots to make anything.",matname);
			sysmessage(s,msg);
		}
		else
			Skills->MakeMenu(s,mm,BLACKSMITHING);
	}
}

//////////////////////////
// Function:	Smith
// History:		unknown, colored ore added by Cork,
//				28 March 2000 revamped by Duke
//				16.9.2000 removed array access and revamped s a bit more (Duke)
//
// Purpose:		checks if targeted material is ore of some kind,
//				checks if anvil is in reach and invokes appropriate Makemenu
// Remarks:		the ingottype var is problematic in a multiplayer environment!!
//
extern int ingottype;
void cSkills::Smith(int s)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		itemmake[s].Mat1id = pi->id();

		if (!CheckInPack(s,pi)) return;

		short id = pi->id();

		if (id==0x1BEF || id==0x1BF2)	// is it an ingot ?
		{
		switch(pi->color())		// AnvilTarget2 args: socket #, item, minimum amount, makemenu #, name of metal
			{
			case 0x0961:	ingottype=1;	AnvilTarget2(s, pi, 3,  1, "iron");		return;
			case 0x0466:	ingottype=2;	AnvilTarget2(s, pi, 1, 50, "golden");	return;
			case 0x0150:	ingottype=3;	AnvilTarget2(s, pi, 1,806, "agapite");	return;
			case 0x0386:	ingottype=4;	AnvilTarget2(s, pi, 1,800, "shadow");	return;
			case 0x0191:	ingottype=5;	AnvilTarget2(s, pi, 1,803, "mythril");	return;
			case 0x02E7:	ingottype=6;	AnvilTarget2(s, pi, 1,801, "bronze");	return;
			case 0x022F:	ingottype=7;	AnvilTarget2(s, pi, 1,802, "verite");	return;
			case 0x02C3:	ingottype=8;	AnvilTarget2(s, pi, 1,804, "merkite");	return;
			case 0x046E:	ingottype=9;	AnvilTarget2(s, pi, 1,814, "copper");	return;
			case 0x0000:	ingottype=10;	AnvilTarget2(s, pi, 1,813, "silver");	return;
			}
		}
		itemmake[s].Mat1id = 0;
	}
	sysmessage(s,"You cannot use that material for blacksmithing");
}

void cSkills::TasteIDTarget(int s)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(!( pi->type==19 || pi->type==14))
		{
			sysmessage(s,"You cant taste that!");
			return;
		}
		if (!CheckSkill(pc_currchar, TASTEID, 0, 250))
		{
			sysmessage(s, "You can't quite tell what this item is...");
		}
		else
		{
			if(pi->corpse)
			{
				sysmessage(s, "You have to use your forensics evalutation skill to know more on this corpse.");
				return;
			}
			
			// Identify Item by Antichrist // Changed by MagiusCHE)
			if (CheckSkill(pc_currchar, TASTEID, 250, 500))
				if (pi->name2 && (strcmp(pi->name2,"#")))
					strcpy(pi->name,pi->name2); // Item identified! -- by Magius(CHE)
				
				// ANTICHRIST -- FOR THE "#" BUG -- now you see the real name
				if(pi->name[0]=='#') pi->getName(temp2);
				else strcpy((char*)temp2, pi->name);
				sprintf((char*)temp, "You found that this item appears to be called: %s", temp2);
				sysmessage(s, (char*)temp);
				
				if (CheckSkill(pc_currchar, TASTEID, 250, 500))
				{
					if((pi->poisoned>0) || (pi->morex==4 && pi->morey==6 && pi->morez==1))
						sysmessage(s,"This item is poisoned!");
					else
						sysmessage(s,"This item shows no poison.");
					
					// Show Creator by Magius(CHE)
					if (CheckSkill(pc_currchar, TASTEID, 250, 500))
					{
						if (strlen(pi->creator)>0)
						{
							if (pi->madewith>0) sprintf((char*)temp2, "It is %s by %s",skill[pi->madewith-1].madeword,pi->creator); // Magius(CHE)
							else if (pi->madewith<0) sprintf((char*)temp2, "It is %s by %s",skill[0-pi->madewith-1].madeword,pi->creator); // Magius(CHE)
							else sprintf((char*)temp2, "It is made by %s",pi->creator); // Magius(CHE)
						} else strcpy((char*)temp2, "You don't know its creator!");
					} else strcpy((char*)temp2, "You can't know its creator!");
					sysmessage(s, (char*)temp2);
					// End Show creator
				}
		}
	}
}

struct Ore
{
	short color;
	short minskill;	// minimum skill to handle that ore
	short quota;	// relative(!) chance to mine that ore (quota/sum of all quotas)
	char *name;
};
const struct Ore OreTable[] =	// MUST be sorted by minskill
{
//	{0x0000,  0,630,"Iron"},
	{0x0386,650,126,"Shadow"},
	{0x02C3,700,112,"Merkite"},
	{0x046E,750, 98,"Copper"},
	{0x0961,790, 84,"Silver"},
	{0x02E7,800, 70,"Bronze"},
	{0x0466,850, 56,"Golden"},
	{0x0150,900, 42,"Agapite"},
	{0x022F,950, 28,"Verite"},
	{0x0191,990, 14,"Mythril"}
};
const short NumberOfOres = sizeof(OreTable)/sizeof(Ore);

static const Ore* getColorFound(short skill)
{
	int i,totalQuotas=0,minersQuota=0;

	for (i=0;i<NumberOfOres;i++)		
	{
		if(OreTable[i].minskill<=skill)		// get his part of quotas depending on his skill
			minersQuota += OreTable[i].quota;
		else
			break;
	}
	if (minersQuota < 1) return NULL;
	int r = rand()%minersQuota;

	for (i=0;i<NumberOfOres;i++)		
	{
		totalQuotas += OreTable[i].quota;	// summarize quotas until we find the ore
		if (r < totalQuotas)
			return &OreTable[i];
		if(OreTable[i].minskill > skill)
		{
			clConsole.send("Something went wrong with mining");
			break;
		}
	}
	return &OreTable[0];	// shouldn't get here, but return iron as default anyway
}

/////////////////
// name:	TryToMine
// history: by Duke, 31 March 2000
// Purpose: helper function for cSkills::Mine()
//			checks if the player's mining skill meets the requirements
//			if yes, he'll get the specified ore
//			There's a *second* CheckSkill done here. It was like this,
//			so I left it like this. That's a gameplay issue.
//
/*
static bool TryToMine(	int s,					// current char's socket #
						int minskill,			// minimum skill required for ore color
						unsigned char id1, unsigned char id2,		// item ID of ingot to be created
						unsigned char col1,unsigned char col2,	// color
						char *orename)			// first letter should be uppercase
{
	P_CHAR pc = MAKE_CHARREF_LRV(currchar[s], false);

	if(pc->skill[MINING] >= minskill)
	{
		char tmp[100];
		sprintf(tmp,"%s Ore",orename);
		Items->SpawnItem(s,DEREF_P_CHAR(pc),1,tmp,1,id1,id2,col1,col2,1,1);

		sysmessage(s,"You place some %c%s ore in your pack.",tolower(*orename),orename+1);
		return true;
	}
	return false;
}
*/

#define max_res_x 610 // max-resource cells x
#define max_res_y 410 // max-resource cells y

void cSkills::Mine(int s)
{
	int x,y,px,py,cx,cy,randnum1;	
	char floor=0;
	char mountain=0;
	static unsigned long int oretime[max_res_x][max_res_y]; //610 and 410 were 1000 in LB release
	static int oreamount[max_res_x][max_res_y];		//for now i'll put zippy values
	int a, b, c;
	unsigned long int curtime=uiCurrentTime;
	signed char z;

	map_st map;
	land_st land;

	if (s<0) return;
	P_CHAR pc = currchar[s];
	if (pc->isHidden())
	{
		pc->unhide();
	}
	if (pc->onhorse)
	{
		sysmessage(s,"You cant mine while on a horse!");
		return;
	}

	if (resource.miningstamina<0 && abs(resource.miningstamina)>pc->stm)
	{
		sysmessage(s,"You are too tired to mine.");
		return;
	}

	pc->stm+=resource.miningstamina;
	if(pc->stm<0) pc->stm=0;
	if(pc->stm>pc->effDex()) pc->stm=pc->effDex();
	updatestats(pc,2);

	if(resource.orearea<10) resource.orearea=10;
	
	if(oretime[0][0]==0)//First time done since server started
	{
		oretime[0][0]=17;//lucky number ;-)
		oreamount[0][0]=resource.ore;
		LogMessage("WOLFPACK: Mining startup, setting ore values and times...");
		for(a=1;a<max_res_x;a++)
		{
			for(b=1;b<max_res_y;b++)
			{
				oreamount[a][b]=resource.ore;
				SetTimerSec(&oretime[a][b],resource.oretime);
			}
		}
		LogMessage("Done.");
	}
	
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) 
		return; // Test if user canceled the mining request
	
	randnum1=rand()%100;
	
	px=((buffer[s][0x0b]<<8)+(buffer[s][0x0c]%256));
	py=((buffer[s][0x0d]<<8)+(buffer[s][0x0e]%256));
	// amount=world_resource_check(px,py,4,1); working... but not finished
	cx=abs(pc->pos.x-px);
	cy=abs(pc->pos.y-py);
	int cz=abs(pc->pos.z-buffer[s][0x10]);

	if(!((cx<=5)&&(cy<=5) && (cz<=25)))
	{
		sysmessage(s,"You are to far away to reach that");
		return;
	}
	
	// lb's mining bugfix for mountains :
	// ( many things like streets were interpreted as mountain)
	// the target packet doesnt send ANY usefull info on map-types.
	// (only for dynamic+static objects)
	// checking for 0x11==0 and 0x12==0 isnt sufficient, it just indicates
	// that it is a map-typ, but a lot of other things are maptypes too ...
	// so everything with z>0 and maptype==1 was a mountain, what is wrong of course.
	// workaround: we have to get the x,y,z and calculate "manually" server side
	// if its a mountain.
	// stupid (OSI studipity) cause lag adding server side, but theres no other way
	
	x=(buffer[s][0x0B]<<8)+buffer[s][0x0C];
	y=(buffer[s][0x0D]<<8)+buffer[s][0x0E];
	z=buffer[s][0x10];
	
	if (SrvParms->minecheck>0) // time consuming mountain mine check only if absolutely necassairy. (mincheck!=0 && maptype targeted && distance>5)
	{
		if ((buffer[s][0x11]!=0)&&(buffer[s][0x12]!=0)) mountain=0; // we tried to mine an static or dynamic item 
		else 		
		{
			// sorry, had to correct this because it cant and didnt work, LB 4'th JULY 2000
			// mountains are "map0's" and no statics !!!

			map=Map->SeekMap0(x,y);
			Map->SeekLand(map.id, &land);
			if ( !strcmp(land.name,"rock") || !(strcmp(land.name, "mountain"))) mountain=1; else mountain=0;
		}
	}
	
	
	// check if gravesight - added by Genesis
	if(((buffer[s][0x10]<=28))&&(buffer[s][0x11]==14)&&
		((buffer[s][0x12]==0xd3)||(buffer[s][0x12]==0xdf)||(buffer[s][0x12]==0xe0)||(buffer[s][0x12]==0xe1)||(buffer[s][0x12]==0xe2)||(buffer[s][0x12]==0xe8)))
	{
		GraveDig(s);
		return;
	}
	
	// check if cave floor
	else if (((buffer[s][0x11]==0x05)&&(((buffer[s][0x12]>=0x3b)&&(buffer[s][0x12]<=0x4f))||
		((buffer[s][0x12]>=0x51)&&(buffer[s][0x12]<=0x53))||(buffer[s][0x12]==0x6a)))&&(!((buffer[s][0x11]==0x02)&&
		(((buffer[s][0x12]>=0x5c)&&(buffer[s][0x12]<=0x76))||((buffer[s][0x12]>=0x7d)&&(buffer[s][0x12]<=0x80)))))) 
		floor=1;
		/*// check if mountain 
		else if (((buffer[s][0x10]>=1)&&(buffer[s][0x10]<=50))&&(buffer[s][0x11]==0)&&(buffer[s][0x12]==0)) 
		mountain=1; // can be adjusted 1 to 53 (1=ground - 53=top of mountain) this isn't really true but the higher the number the higher up the mountain you are 'usually'.
	*/
			
	a=pc->pos.x/resource.orearea;
	b=pc->pos.y/resource.orearea;
	if(a>=max_res_x || b>=max_res_y) return;//bad place...(A dungeon or something)
	
	if(oretime[a][b]<=curtime)
	{
		for(c=0;c<resource.ore;c++)//Find howmany periods have been by, give 1 more ore for each period.
		{
			if((oretime[a][b]+(c*resource.oretime*MY_CLOCKS_PER_SEC))<=curtime && oreamount[a][b]<resource.ore)
				oreamount[a][b]+=resource.orerate;//AntiChrist
			else break;
		}
		SetTimerSec(&oretime[a][b],resource.oretime);
	}
	
	if(oreamount[a][b]>resource.ore) oreamount[a][b]=resource.ore;
	
	if(oreamount[a][b]<=0)
	{
		sysmessage(s,"There is no metal here to mine.");
		return;
	}
	
	if (((SrvParms->minecheck==1)&&(!floor)&&(!mountain)))//Mine only mountains & floors
	{
		sysmessage(s,"You can't mine that!");
		return;
	}
	
	if (pc->onhorse)
		action(s,0x1A);
	else
		action(s,0x0b);
	soundeffect(s,0x01,0x25); 
	
	if(!Skills->CheckSkill(pc, MINING, 0, 1000)) 
	{
		sysmessage(s,"You sifted thru the dirt and rocks, but found nothing useable.");
		if(oreamount[a][b]>0 && rand()%2==1) oreamount[a][b]--;//Randomly deplete resources even when they fail 1/2 chance you'll loose ore.
		return;
	} else if(oreamount[a][b]>0) oreamount[a][b]--;
	
	
	if ((buffer[s][1]==1)&&(buffer[s][2]==0)&&
		(buffer[s][3]==1)&&(buffer[s][4]==0))
	{
		//
		//29/11/99
		//
		//AntiChrist - new implementation of triggered mining.
		//
		//Mineral creation is now done all by trigger
		//so that we can create new scripted minerals.
		//(put resource.miningtrigger=0 to disable this feature
		//and use standard mining!)
		//
		if(resource.miningtrigger>0)
		{
			pc->targtrig=resource.miningtrigger;
			P_ITEM pi = FindItemBySerial(addmitem[s]);
			Trig->triggerwitem(s, pi, 1);
			// Currently Disabled.
			return;
		} else
		{//normal mining skill
		
		//*******************************************************************
		//If mining skill is lower than 65 can only mine iron ore : Cork
		if (pc->skill[MINING]<650
			|| rand()%5)			// only a 20% chance of finding colored ore
		{
			if (!(rand()%20))			// a 5% chance for 5 small ores
				Items->SpawnItem(s,DEREF_P_CHAR(pc),5,"Iron Ore",1,'\x19','\xba', 0, 0,1,1);
			else
				Items->SpawnItem(s,DEREF_P_CHAR(pc),1,"Iron Ore",1,'\x19','\xb9', 0, 0,1,1);
			sysmessage(s,"You place some iron ore in your pack.");
			return;
		}
		else if(pc->skill[MINING]>=850 && !(rand()%18))
		{
			//Skills->CheckSkill(player,MINING,850,1000);
			SpawnRandomItem(s,1,"necro.scp","ITEMLIST","999"); 
			sysmessage(s,"You place a gem in your pack.");
		}
		else
		{
			const Ore* pOre = getColorFound(pc->skill[MINING]);	// get random ore

			char tmp[100];
			sprintf(tmp,"%s Ore",pOre->name);
			Items->SpawnItem(s,DEREF_P_CHAR(pc),1,tmp,1,0x19,0xB9, pOre->color>>8, pOre->color&0x00FF,1,1);

			sysmessage(s,"You place some %c%s ore in your pack.",tolower(pOre->name[0]),pOre->name+1);
		}
		}//end of normal mining skill
	}//if buffer[][]=......
}

void cSkills::TreeTarget(int s)
{
	int lumber=0;
	int px,py,cx,cy;
	static unsigned long logtime[max_res_x][max_res_y];//see mine for values...they were 1000 also here
	static int logamount[max_res_x][max_res_y];
	int a, b, c;
	long int curtime=uiCurrentTime;
	P_CHAR pc = currchar[s];

	//AntiChrist
	//Logging stamina
	if (resource.logstamina<0 && abs(resource.logstamina)>pc->stm)
	{
		sysmessage(s,"You are too tired to chop.");
		return;
	}

	pc->stm+=resource.logstamina;
	if(pc->stm<0) pc->stm=0;
	if(pc->stm>pc->effDex()) pc->stm=pc->effDex();
	updatestats(pc,2);

	if(resource.logarea<10) resource.logarea=10; //New -- Zippy
	
	if(logtime[0][0]==0)//First time done since server started
	{
		logtime[0][0]=17;//lucky number ;-)
		logamount[0][0]=resource.logs;
		LogMessage("Lumberjacking startup, setting tree values and times...");
		
		//for(a=1;a<410;a++)
		for(a=1;a<max_res_x;a++)//AntiChrist bug fix
		{
			for(b=1;b<max_res_y;b++)
			{
				logamount[a][b]=resource.logs;
				SetTimerSec(&logtime[a][b],resource.logtime);
			}
		}
		LogMessage("Done.");
	}

	//AntiChrist
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) 
	return; // Test if use canceled the mining request

	//AntiChrist
	px=((buffer[s][0x0b]<<8)+(buffer[s][0x0c]%256));
	py=((buffer[s][0x0d]<<8)+(buffer[s][0x0e]%256));
	// amount=world_resource_check(px,py,4,1); working... but not finished
	cx=abs(pc->pos.x-px);
	cy=abs(pc->pos.y-py);
	if(!((cx<=5)&&(cy<=5)))
	{
		sysmessage(s,"You are to far away to reach that");
		return;
	}

	a=pc->pos.x/resource.logarea; //Zippy
	b=pc->pos.y/resource.logarea;
		
	if(a>=max_res_x || b>=max_res_y) return;
	
	if(logtime[a][b]<=curtime)
	{
		for(c=0;c<resource.logs;c++)//Find howmany 10 min periods have been by, give 1 more for each period.
		{
			if((logtime[a][b]+(c*resource.logtime*MY_CLOCKS_PER_SEC))<=curtime && logamount[a][b]<resource.logs)
				logamount[a][b]+=resource.lograte;//AntiChrist
			else break;
		}
		SetTimerSec(&logtime[a][b],resource.logtime);
	}
	
	if(logamount[a][b]>resource.logs) logamount[a][b]=resource.logs;
	
	if(logamount[a][b]<=0)
	{
		sysmessage(s,"There is no more wood here to chop.");
		return;
	}
	
	P_ITEM pi_pack = Packitem(pc);
	if (pi_pack == NULL) {sysmessage(s,"No backpack to store logs"); return; } //LB
	
	if (pc->onhorse) action(s,0x1C);
	else action(s,0x0D);
	soundeffect(s,0x01,0x3E);
	
	if (!Skills->CheckSkill(pc, LUMBERJACKING, 0, 1000)) 
	{
		sysmessage(s,"You chop for a while, but fail to produce any usable wood.");
		if(logamount[a][b]>0 && rand()%2==1) logamount[a][b]--;//Randomly deplete resources even when they fail 1/2 chance you'll loose wood.
		return;
	}
	
	if(logamount[a][b]>0) logamount[a][b]--;
	
	if ((buffer[s][1]==1 || buffer[s][1]==0)&&(buffer[s][2]==0)
		&&(buffer[s][3]==1)&&(buffer[s][4]==0))
	{
		//
		//29/11/99
		//
		//AntiChrist - new implementation of triggered logging.
		//
		//Log creation is now done all by trigger
		//so that we can create new scripted minerals.
		//(put resource.logtrigger=0 to disable this feature
		//and use standard logging!)
		//
		if(resource.logtrigger>0)
		{
			//pc->targtrig=resource.logtrigger;
			P_ITEM pi = FindItemBySerial(addmitem[s]);
			Trig->triggerwitem(s, pi, 0);	// routing to wtrigrs instead of triggers.scp Duke, 5.11.2000
			return;
		} else
		{//normal mining skill
			
			P_ITEM pi_c = Items->SpawnItem(s,DEREF_P_CHAR(pc),10,"#",1,0x1B,0xE0,0,0,1,1);
			if(pi_c == NULL) return;//AntiChrist to prevent crashes
			if (pi_c->amount > 10) sysmessage(s,"You place more logs in your pack.");
			else sysmessage(s,"You place some logs in your pack.");
			
			lumber=1;
		}
	}
}

void cSkills::GraveDig(int s) // added by Genesis 11-4-98
{
	int	nAmount, nFame, nItemID;
	char iID=0;
	
	P_CHAR pc = currchar[s];

	Karma(DEREF_P_CHAR(pc),-1,-2000); // Karma loss no lower than the -2 pier
	
	if(pc->onhorse)
		action(s,0x1A);
	else
		action(s,0x0b);
	soundeffect(s,0x01,0x25);
	if(!Skills->CheckSkill(pc, MINING, 0, 800)) 
	{
		sysmessage(s,"You sifted through the dirt and found nothing.");
		return;
	}
	
	nFame = pc->fame;
	if(pc->onhorse)
		action(s,0x1A);
	else
		action(s,0x0b);
	soundeffect(s,0x01,0x25);
	int nRandnum=rand()%13;
	switch(nRandnum)
	{
	case 2:
		SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1000"); // Low level Undead - Random
		sysmessage(s,"You have disturbed the rest of a vile undead creature.");
		break;
	case 4:
		nItemID=SpawnRandomItem(s,1,"necro.scp","ITEMLIST","1001"); // Armor and shields - Random
		if((nItemID>=7026)&&(nItemID<=7035))
			sysmessage(s,"You unearthed an old shield and placed it in your pack");
		else
			sysmessage(s,"You have found an old piece armor and placed it in your pack.");
		break;
	case 5:
		//Random treasure between gems and gold
		nRandnum=rand()%2;
		if(nRandnum)
		{ // randomly create a gem and place in backpack
			SpawnRandomItem(s,1,"necro.scp","ITEMLIST","999");
			sysmessage(s,"You place a gem in your pack.");
		}
		else
		{ // Create between 1 and 15 goldpieces and place directly in backpack
			nAmount=1+(rand()%15);
			addgold(DEREF_P_CHAR(pc),nAmount);
			goldsfx(s,nAmount);
			if (nAmount==1)
				sprintf((char*)temp,"You unearthed %i gold coin.", nAmount);
			else
				sprintf((char*)temp,"You unearthed %i gold coins.", nAmount);
			sysmessage(s,(char*)temp);
		}
		break;
	case 6:
		if(nFame<500)
			SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1000"); // Low level Undead - Random
		else
			SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1001"); // Med level Undead - Random
		sysmessage(s,"You have disturbed the rest of a vile undead creature.");
		break;
	case 8:
		SpawnRandomItem(s,1,"necro.scp","ITEMLIST","1000");
		sysmessage(s,"You unearthed a old weapon and placed it in your pack.");
		break;
	case 10:
		if(nFame<1000)
			SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1001"); // Med level Undead - Random
		else
			SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1002"); // High level Undead - Random
		sysmessage(s,"You have disturbed the rest of a vile undead creature.");
		break;
	case 12:
		if(nFame>1000)
			SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1002"); // High level Undead - Random
		else
			SpawnRandomMonster(s,"necro.scp","UNDEADLIST","1001"); // Med level Undead - Random
		sysmessage(s,"You have disturbed the rest of a vile undead creature.");
		break;
	default:
		nRandnum=rand()%2;
		switch(nRandnum)
		{
			case 1:
				nRandnum=rand()%12;
				switch(RandomNum(0, 11))
				{
					case 0: iID=0x11; break;
					case 1: iID=0x12; break;
					case 2: iID=0x13; break;
					case 3: iID=0x14; break;
					case 4: iID=0x15; break;
					case 5: iID=0x16; break;
					case 6: iID=0x17; break;
					case 7: iID=0x18; break;
					case 8: iID=0x19; break;
					case 9: iID=0x1A; break;
					case 10: iID=0x1B; break;
					case 11: iID=0x1C; break;
				}
				Items->SpawnItem(s,DEREF_P_CHAR(pc),1,NULL,0,0x1b,iID,0x00,0x00,1,1);
				sysmessage(s,"You have unearthed some old bones and placed them in your pack.");
				break;
			default: // found an empty grave
				sysmessage(s,"This grave seems to be empty.");
		}
	}
}

///////////////////////
// name:	SmeltOre2
// history:	by Duke, 31 March 2000
// Purpose: helper function for cSkills::SmeltOre()
//			checks if the player's mining skill meets the requirements
//			if yes, he'll get the specified ingots and appropriate messages
// Remarks: CheckSkill does NOT use minskill (cause it was like that before)
//
static void SmeltOre2(	int s,					// current char's socket #
						int minskill,			// minimum skill required for ore color
						short id,short color,	// item ID & color of ingot to be created
						char *orename)
{
	P_CHAR pc_currchar = currchar[s];

//	int smi = pc_currchar->smeltitem;		// index of ore item
	const P_ITEM pi = FindItemBySerial(pc_currchar->smeltitem);	// on error return
	if ( pi == NULL )
		return;


	if (pc_currchar->skill[MINING] < minskill)
	{
		sysmessage(s,"You have no idea what to do with this strange ore");
		return;					
	}
	if(!Skills->CheckSkill(pc_currchar, MINING, 0, 1000))
	{
		if (pi->amount==1)
		{
			sysmessage(s,"Your hand slips and the last of your materials are destroyed.");
			Items->DeleItem(pi);
		}
		else
		{
			sysmessage(s,"Your hand slips and some of your materials are destroyed.");
			pi->amount=pi->amount/2;
			RefreshItem(pi);					// tell the client item has been changed
		}
	}
	else
	{
		char tmp[100];
		int numore=pi->amount*2;			// one ore gives two ingots
		sprintf(tmp,"%s Ingot",orename);
		
		cItem* Ingot=Items->SpawnItem(DEREF_P_CHAR(pc_currchar),numore,tmp,1,id, color,1);
		if (Ingot)
		{
			Ingot->weight = 20;	// that's 0.2 stone
			RefreshItem(Ingot);
		}

		sysmessage(s,"You have smelted your ore");
		sprintf(tmp,"You place some %c%s ingots in your pack.",tolower(*orename), orename+1);
		sysmessage(s,tmp);
		Items->DeleItem(pi);
	}
}

//////////////////////////
// Function:	SmeltOre
// History:		unknown, colored ore by Cork,
//				31 March 2000 totally revamped by Duke
//				16.9.2000 removed array access and revamped a bit more (Duke)
//
// Purpose:		checks if targeted item is a forge and in range
//				then executes the smelting function with the appropriate
//				minskill and ingot type
// Remarks:		NOTE: ingot color is different from ore color for gold, silver & copper!
//
void cSkills::SmeltOre(int s)
{
	P_CHAR pc_currchar = currchar[s];

	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(	IsForge(pi->id()) )
		{
			if(!iteminrange(s,pi,3))		//Check if the forge is in range
				sysmessage(s,"You cant smelt here.");
			else
			{
				P_ITEM pix = FindItemBySerial(pc_currchar->smeltitem);	// on error return
				if ( pix == NULL)
					return;
				switch (pix->color())
				{
					case 0x0000:	SmeltOre2(s,   0, 0x1BF2, 0x0961,"Iron");	break;
					case 0x0466:	SmeltOre2(s, 850, 0x1BF2, 0x0466,"Golden");	break;
					case 0x046E:	SmeltOre2(s, 750, 0x1BF2, 0x046E,"Copper");	break;
					case 0x0961:	SmeltOre2(s, 790, 0x1BF2, 0x0000,"Silver");	break;
					case 0x0150:	SmeltOre2(s, 900, 0x1BF2, 0x0150,"Agapite");break;
					case 0x0386:	SmeltOre2(s, 650, 0x1BF2, 0x0386,"Shadow");	break;
					case 0x022f:	SmeltOre2(s, 950, 0x1BF2, 0x022F,"Verite");	break;
					case 0x02e7:	SmeltOre2(s, 800, 0x1BF2, 0x02E7,"Bronze");	break;
					case 0x02c3:	SmeltOre2(s, 700, 0x1BF2, 0x02C3,"Merkite");break;
					case 0x0191:	SmeltOre2(s, 990, 0x1BF2, 0x0191,"Mythril");break;
					default:		
						LogError("switch reached default");
				}
			}
		}
	}
	pc_currchar->smeltitem = INVALID_SERIAL;
	Weight->NewCalc(DEREF_P_CHAR(pc_currchar));	// Ison 2-20-99
	statwindow(s, DEREF_P_CHAR(pc_currchar));		// Ison 2-20-99
}

void cSkills::Wheel(int s, int mat)//Spinning wheel
{
	int tailme=0;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return;
	
	if( pi->id() >= 0x10A4 || pi->id() <= 0x10A6 )
	{
		if(iteminrange(s,pi,3))
		{
			if (!Skills->CheckSkill(currchar[s],TAILORING, 0, 1000)) 
			{
				sysmessage(s,"You failed to spin your material.");
				return;
			}
			sysmessage(s,"You have successfully spun your material.");

//			int ti = pc_currchar->tailitem;
			const P_ITEM pti = FindItemBySerial(pc_currchar->tailitem);	// on error return
			
			if (mat==YARN)
			{
				strcpy(pti->name,"#");
				pti->setId(0x0E1D);
				pti->amount=pti->amount*3;
			}
			else if (mat==THREAD)
			{
				strcpy(pti->name,"#");
				pti->setId(0x0FA0);
				pti->amount=pti->amount*3;
			}

			pti->priv |= 0x01;
			RefreshItem(pti);
			tailme=1;
		}
	}
	pc_currchar->tailitem = INVALID_SERIAL;
	if(!tailme) sysmessage(s,"You cant tailor here.");
}

void cSkills::Loom(int s)
{
	int tailme=0;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if ( pi->id() >= 0x105F && pi->id() <= 0x1066 )
		{
			if(iteminrange(s,pi,3))
			{
//				int ti = pc_currchar->tailitem;
				const P_ITEM pti = FindItemBySerial(pc_currchar->tailitem);	// on error return
				if (pti == NULL)
					return;
				if(pti->amount<5)
				{
					sysmessage(s,"You do not have enough material to make anything!");
					return;
				}
				if (!Skills->CheckSkill(pc_currchar,TAILORING, 300, 1000)) 
				{
					sysmessage(s,"You failed to make cloth.");
					sysmessage(s,"You have broken and lost some material!");
					if (pti->amount!=0) pti->amount -= 1+(rand() % (pti->amount)); else pti->amount --; 
					if (pti->amount <=0) Items->DeleItem(pti);
					else
						RefreshItem(pti);
					return;
				}
				
				if( pti->id()==0x0E1E || pti->id()==0x0E1D || pti->id()==0x0E1F )	// yarn
				{
					sysmessage(s,"You have made your cloth.");

					strcpy(pti->name,"#");
					pti->setId(0x175D);
					pti->priv |= 0x01;
					pti->amount=static_cast<unsigned short> (pti->amount*0.25);
				}
				else if( pti->id()==0x0FA0 || pti->id()==0x0FA1 )	// thread
				{
					sysmessage(s,"You have made a bolt of cloth.");

					strcpy(pti->name,"#");
					pti->setId(0x0F95);
					pti->priv |= 1;
					pti->amount=static_cast<unsigned short> (pti->amount*0.25);
				}
				RefreshItem(pti);//AntiChrist
				tailme=1;
			}
		}
	}
	pc_currchar->tailitem = INVALID_SERIAL;
	if(!tailme) sysmessage(s,"You cant tailor here.");
}

////////////
// Name:	CookOnFire
// By:		Ripper & Duke, 07/20/00
// Purpose: so you can use raw meat on fire
//
void cSkills::CookOnFire(int s, short id1, short id2, char* matname)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi && !pi->isLockedDown()) // Ripper
	{
		P_ITEM piRaw = FindItemBySerial(addmitem[s]);
		if (CheckInPack(s,piRaw))
		{
			if(IsCookingPlace(pi->id()) )
			{
				if(iteminrange(s,pi,3))
				{
					char tmpmsg[250];
					soundeffect(s,0x01,0xDD);	// cooking sound
					if (!Skills->CheckSkill(pc_currchar,COOKING, 0, 1000)) 
					{
						sprintf(tmpmsg,"You failed to cook the %s and drop some into the ashes.",matname);
						piRaw->ReduceAmount(1+(rand() %(piRaw->amount)));
					}
					else
					{
						sprintf(tmpmsg,"You have cooked the %s,and it smells great.",matname);
						P_ITEM pi_c = Items->SpawnItem(s,DEREF_P_CHAR(pc_currchar),piRaw->amount,"#",1,id1,id2,0,0,1,1);
						if(pi_c == NULL) return;
						pi_c->type = 14;
						RefreshItem(pi_c);
						Items->DeleItem(piRaw);
					}
					sysmessage(s,tmpmsg);
				}
			}
		}
	} 
}

void cSkills::MakeDough(int s)
{
	bool tailme = false;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(pi->id()==0x103A)
		{
			if(iteminrange(s,pi,3))
			{
				if (!Skills->CheckSkill(pc_currchar,COOKING, 0, 1000)) 
				{
					sysmessage(s,"You failed to mix, and spilt your water.");
					return;
				}
				sysmessage(s,"You have mixed very well to make your dough.");
				
				const P_ITEM pti=FindItemBySerial(pc_currchar->tailitem);	// on error return
				if ( pti == NULL)
					return;
				strcpy(pti->name,"#");
				
				pti->setId(0x103D);
				pti->priv |= 0x01;
				pti->amount *= 2;
				
				RefreshItem(pti);
				tailme = true;
			}
		}
	}
	pc_currchar->tailitem=INVALID_SERIAL;
	if(!tailme) sysmessage(s, "You cant mix here.");
}

void cSkills::MakePizza(int s)
{
	int tailme=0;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(pi->id()==0x103D)
		{
			if(iteminrange(s,pi,3))
			{
				if (!Skills->CheckSkill(pc_currchar,COOKING, 0, 1000)) 
				{
					sysmessage(s,"You failed to mix.");
					Items->DeleItem(pi);
					return;
				}
				sysmessage(s,"You have made your uncooked pizza, ready to place in oven.");
				
				const P_ITEM pti = FindItemBySerial(pc_currchar->tailitem);	// on error return
				if ( pti == NULL )
					return;
				strcpy(pti->name,"#");
				
				pti->setId(0x1083);
				pti->priv |= 0x01;
				pti->amount *= 2;
				
				RefreshItem(pti);
				tailme = 1;
			}
		}
	}
	pc_currchar->tailitem = INVALID_SERIAL;
	if(!tailme) sysmessage(s,"You cant mix here.");
}

/*
* I decided to base this on how OSI will be changing detect hidden. 
* The greater your skill, the more of a range you can detect from target position.
* Hiders near the center of the detect circle are easier to detect than ones near
* the edges of the detect circle. Also low skill detecters cannot find high skilled
* hiders as easily as low skilled hiders.
*/

void cSkills::DetectHidden(UOXSOCKET s)
{
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return;
	P_CHAR pc_currchar = currchar[s];
	
	//unsigned int i;
	int x,y,z,dx,dy,j,low;
	double c,range;		//int is too restricting
	
	x=(buffer[s][11]<<8)+buffer[s][12];
	y=(buffer[s][13]<<8)+buffer[s][14];
	z=buffer[s][16];
	
	j=pc_currchar->skill[DETECTINGHIDDEN];
	
	range = (j*j/1.0E6)*Races[pc_currchar->race]->VisRange;	// this seems like an ok formula
	
	cRegion::RegionIterator4Chars ri(pc_currchar->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if (pc != NULL)
		{
			if (pc->hidden==1&&(!(pc->priv2&8))) // do not detect invis people only hidden ones
			{//do not reveal permanently hidden chars - AntiChrist
				dx=abs(pc->pos.x-x);
				dy=abs(pc->pos.y-y);

				c=hypot(dx, dy);
				low = (int)(pc->skill[HIDING]*pc->skill[HIDING]/1E3 - (range*50/Races[pc->race]->VisRange)*(range-c)/range);
				if (low<0) low=0;
				else if (low>1000) low=1000;
				
				if ((Skills->CheckSkill(pc_currchar,DETECTINGHIDDEN,low,1000))&&(c<=range))
				{
					pc->unhide();
				
					UOXSOCKET tempsock = calcSocketFromChar(pc);
					if (tempsock!=-1)
						if ((perm[tempsock])) sysmessage(tempsock,"You have been revealed!");
				}
				else sysmessage(s,"You fail to find anyone.");
			}
		}//if mapitem
	}
}

void cSkills::ProvocationTarget1(UOXSOCKET s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);

	if( pc == NULL ) return;

	P_ITEM inst = GetInstrument(s);
	if (inst == NULL) 
	{
		sysmessage(s, "You do not have an instrument to play on!");
		return;
	}
	if ( pc->isInvul() || pc->shop==1 || // invul or shopkeeper
		pc->npcaitype==0x01 || // healer
		pc->npcaitype==0x04 || // tele guard
		pc->npcaitype==0x06 || // chaos guard
		pc->npcaitype==0x07 || // order guard
		pc->npcaitype==0x09)   // city guard
	{
		sysmessage(s," You cant entice that npc!");
		return;
	}
	if (pc->inGuardedArea())
	{
		sysmessage(s," You cant do that in town!");
		return;
	}
	addid1[s]=buffer[s][7];
	addid2[s]=buffer[s][8];
	addid3[s]=buffer[s][9];
	addid4[s]=buffer[s][10];
	
	if (pc->isPlayer())
		sysmessage(s, "You cannot provoke other players.");
	else
	{
		target(s, 0, 1, 0, 80, "You play your music, inciting anger, and your target begins to look furious. Whom do you wish it to attack?");
		PlayInstrumentWell(s, inst);
	}
}

void cSkills::EnticementTarget1(UOXSOCKET s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if( pc == NULL ) return;

	P_ITEM inst = GetInstrument(s);
	if (inst == NULL) 
	{
		sysmessage(s, "You do not have an instrument to play on!");
		return;
	}
	if ( pc->isInvul() || pc->shop==1 || // invul or shopkeeper
		pc->npcaitype==0x01 || // healer
		pc->npcaitype==0x04 || // tele guard
		pc->npcaitype==0x06 || // chaos guard
		pc->npcaitype==0x07 || // order guard
		pc->npcaitype==0x09)   // city guard
	{
		sysmessage(s," You cant entice that npc!");
		return;
	}
	if (pc->inGuardedArea())
	{
		sysmessage(s," You cant do that in town!");
		return;
	}
	addid1[s]=buffer[s][7];
	addid2[s]=buffer[s][8];
	addid3[s]=buffer[s][9];
	addid4[s]=buffer[s][10];

	if (pc->isPlayer())
		sysmessage(s, "You cannot entice other players.");
	else
	{
		target(s, 0, 1, 0, 82, "You play your music, luring them near. Whom do you wish them to follow?");
		PlayInstrumentWell(s, inst);
	}
}

void cSkills::EnticementTarget2(UOXSOCKET s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if( pc == NULL ) return;
	P_CHAR pc_currchar = currchar[s];
	P_ITEM inst = GetInstrument(s);
	if (inst == NULL) 
	{
		sysmessage(s, "You do not have an instrument to play on!");
		return;
	}
	int res1 = CheckSkill(pc_currchar, ENTICEMENT, 0, 1000);
	int res2 = CheckSkill(pc_currchar, MUSICIANSHIP, 0, 1000);
	if (res1 && res2)
	{
		P_CHAR pc_target = FindCharBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
		if ( pc_target == NULL ) return;
		pc_target->ftarg = pc->serial;
		pc_target->npcWander = 1;
		sysmessage(s, "You play your hypnotic music, luring them near your target.");
		PlayInstrumentWell(s, inst);
	}
	else 
	{
		sysmessage(s, "Your music fails to attract them.");
		PlayInstrumentPoor(s, inst);
	}
}

void cSkills::ProvocationTarget2(UOXSOCKET s)
{
	cChar* Victim2 = FindCharBySerPtr(buffer[s]+7);
	if (!Victim2)
		return;

	P_CHAR Player = currchar[s];

	P_CHAR Victim1 = FindCharBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
	//P_CHAR Victim1 = MAKE_CHARREF_LR(target);

	if (Victim2->inGuardedArea())
	{
		sysmessage(s,"You cant do that in town!");
		return;
	}
	if (Victim1->isSameAs(Victim2))
	{
		sysmessage(s, "Silly bard! You can't get something to attack itself.");
		return;
	}

	P_ITEM inst = GetInstrument(s);
	if (inst == NULL) 
	{
		sysmessage(s, "You do not have an instrument to play on!");
		return;
	}
	if (CheckSkill((Player), MUSICIANSHIP, 0, 1000))
	{
		PlayInstrumentWell(s, inst);
		if (CheckSkill((Player), PROVOCATION, 0, 1000))
		{
			if (Player->inGuardedArea())
				Combat->SpawnGuard(DEREF_P_CHAR(Player), DEREF_P_CHAR(Player), Player->pos.x+1,Player->pos.y,Player->pos.z); //ripper
			sysmessage(s, "Your music succeeds as you start a fight.");
		}
		else 
		{
			sysmessage(s, "Your music fails to incite enough anger.");
			Victim2 = Player;		// make the targeted one attack the Player
		}

		Victim1->fight(Victim2);
		Victim1->setAttackFirst();
		
		Victim2->fight(Victim1);
		Victim2->resetAttackFirst();
		
		sprintf(temp, "* You see %s attacking %s *", Victim1->name, Victim2->name);
		unsigned int i;
		for (i=0;i<now;i++)
		{
			if (inrange1p(currchar[i], DEREF_P_CHAR(Victim1))&&perm[i])
			{
				itemmessage(i, temp, Victim1->serial);
			}
		}
	}
	else
	{
		PlayInstrumentPoor(s, inst);
		sysmessage(s, "You play rather poorly and to no effect.");
	}
}

//////////////////////////
// name:	AlchemyTarget
// history: unknown, revamped by Duke,21.04.2000
// Purpose:	checks for valid reg and brings up gumpmenu to select potion
//			This is called after the user dblclicked a mortar and targeted a reg
//
void cSkills::AlchemyTarget(int s)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return;
	
	switch (pi->id())
	{
	case 0x0F7B: itemmenu(s, 991);break;	// Agility,
	case 0x0F84: itemmenu(s, 992);break;	// Cure, Garlic
	case 0x0F8C: itemmenu(s, 993);break;	// Explosion, Sulfurous Ash
	case 0x0F85: itemmenu(s, 994);break;	// Heal, Ginseng
	case 0x0F8D: itemmenu(s, 995);break;	// Night sight
	case 0x0F88: itemmenu(s, 996);break;	// Poison, Nightshade
	case 0x0F7A: itemmenu(s, 997);break;	// Refresh, 
	case 0x0F86: itemmenu(s, 998);break;	// Strength,
	case 0x0E9B: break;	// Mortar
	default:
		if ( pi->id()>=0x1B11 && pi->id()<=0x1B1C )
		{
			MakeNecroReg(s,pi,pi->id());
			sysmessage(s,"You grind some bone into powder.");
		}
		else
			sysmessage(s,"That is not a valid reagent.");
	}
}

void cSkills::CreateBandageTarget(int s)//-Frazurbluu- rewrite of tailoring to current OSI
{
	const P_ITEM pi = FindItemBySerPtr(buffer[s]+7);
	short int amt=0;
	P_CHAR pc_currchar = currchar[s];

	if (pi && !pi->isLockedDown()) // Ripper
	{
		short int col1=pi->color1; //-Frazurbluu- added color retention for bandage cutting from cloth
		short int col2=pi->color2;

		if ((IsCloth(pi->id()) && (IsCutCloth(pi->id()))))
		{
			amt=pi->amount;  //-Frazurbluu- changed to reflect current OSI 
			soundeffect(s,0x02,0x48);
			sysmessage(s,"You cut some cloth into bandages, and put it in your backpack");
			P_ITEM pi_c = Items->SpawnItem(s,DEREF_P_CHAR(pc_currchar),amt,"#",0,0x0E,0x21,col1,col2,1,1);
			if(pi_c == NULL) return;
			// need to set amount and weight and pileable, note: cannot set pilable while spawning item -Fraz-
			pi_c->weight=10;
			pi_c->pileable=1;
			pi_c->att=9;
			pi_c->amount=amt;
			RefreshItem(pi_c);
			Items->DeleItem(pi);
			Weight->NewCalc(DEREF_P_CHAR(pc_currchar));
			statwindow(s,DEREF_P_CHAR(pc_currchar));
			return;
		}	
		if( IsBoltOfCloth(pi->id()) )
		{
			if (pi->amount>1)
				amt=(pi->amount*50);//-Frazurbluu- changed to reflect current OSI 
			else
				amt=50;
			soundeffect(s,0x02,0x48);
			P_ITEM pi_c = Items->SpawnItem(s,DEREF_P_CHAR(pc_currchar),1,"cut cloth",0,0x17,0x66,col1,col2,1,1);
			if(pi_c == NULL) return;
			pi_c->weight=10;
			pi_c->pileable=1;
			pi_c->amount=amt;
			RefreshItem(pi_c);
			Items->DeleItem(pi);
			Weight->NewCalc(DEREF_P_CHAR(pc_currchar));
			statwindow(s,DEREF_P_CHAR(pc_currchar));
			return;
		}
		if( IsHide(pi->id()) )
		{
			amt=pi->amount;
			soundeffect(s,0x02,0x48);
			P_ITEM pi_c = Items->SpawnItem(s,DEREF_P_CHAR(pc_currchar),1,"leather piece",0,0x10,0x67,col1,col2,1,1);
			if(pi_c == NULL) return;
			pi_c->weight=100;
			pi_c->pileable=1;
			pi_c->amount=amt;
			RefreshItem(pi_c);
			Items->DeleItem(pi);
			Weight->NewCalc(DEREF_P_CHAR(pc_currchar));
			statwindow(s,DEREF_P_CHAR(pc_currchar));
			return;
		}
		sysmessage(s,"You cannot cut anything from that item.");
	}
}

////////////////////////
// name:	HealingSkillTarget
// history: unknown, revamped by Duke, 4.06.2000
//
void cSkills::HealingSkillTarget(UOXSOCKET s)
{
	P_ITEM pib = FindItemBySerial(addmitem[s]);	// item index of bandage
	
	P_CHAR pp = FindCharBySerPtr(buffer[s]+7); // pointer to patient
	if (pp != NULL)
	{
		P_CHAR ph = currchar[s];	// points to the healer
		if (!SrvParms->bandageincombat && (pp->war || ph->war))
		{
			P_CHAR pc_attacker = FindCharBySerial(ph->attacker); // Ripper...cant heal while in a fight
			if ( (pc_attacker != NULL) && pc_attacker->war)
			{
				sysmessage(s, "You can`t heal while in a fight!");
				return;
			}
		}
		if(ph->pos.distance(pp->pos)>5)
		{
			sysmessage(s,"You are not close enough to apply the bandages.");
			return;
		}
		if ((ph->isInnocent()) &&(ph->serial != pp->serial))
		{
	       if ((pp->crimflag>0) ||(pp->isMurderer()))
		   {
		       criminal(DEREF_P_CHAR(ph));
		   }
		}
		
		if (pp->dead)
		{
			if (ph->skill[HEALING] < 800 || ph->skill[ANATOMY] < 800)
				sysmessage(s,"You are not skilled enough to resurrect");
			else
			{
				int reschance = static_cast<int>((ph->baseskill[HEALING]+ph->baseskill[ANATOMY])*0.17);
				int rescheck=RandomNum(1,100);
				CheckSkill((ph),HEALING,800,1000);
				CheckSkill((ph),ANATOMY,800,1000);
				if(reschance<=rescheck)
					sysmessage(s,"You failed to resurrect the ghost");
				else
				{
					Targ->NpcResurrectTarget(DEREF_P_CHAR(ph));
					sysmessage(s,"Because of your skill, you were able to resurrect the ghost.");
				}
			}
			return;
		}
		
		
		if (pp->poisoned>0)
		{
			if (ph->skill[HEALING]<600 || ph->skill[ANATOMY]<600)
			{
				sysmessage(s,"You are not skilled enough to cure poison.");
				sysmessage(s,"The poison in your target's system counters the bandage's effect.");
			}
			else
			{
				int curechance = static_cast<int>((ph->baseskill[HEALING]+ph->baseskill[ANATOMY])*0.67);
				int curecheck=RandomNum(1,100);
				CheckSkill((ph),HEALING,600,1000);
				CheckSkill((ph),ANATOMY,600,1000);
				if(curechance<=curecheck)
				{
					pp->poisoned=0;
					sysmessage(s,"Because of your skill, you were able to counter the poison.");
				}
				else
					sysmessage(s,"You fail to counter the poison");
				pib->ReduceAmount(1);
			}
			return;
		}
		
		if(pp->hp == pp->st)
		{
			sysmessage(s,"That being is not damaged");
			return;
		}
		
		if(pp->isHuman()) //Used on human
		{
			if (!CheckSkill((ph),HEALING,0,1000))
			{
				sysmessage(s,"You apply the bandages, but they barely help!");
				pp->hp++;
			}
			else
			{
				int healmin = (((ph->skill[HEALING]/5)+(ph->skill[ANATOMY]/5))+3); //OSI's formula for min amount healed (Skyfire)
				int healmax = (((ph->skill[HEALING]/5)+(ph->skill[ANATOMY]/2))+10); //OSI's formula for max amount healed (Skyfire)
				int j=RandomNum(healmin,healmax);
				//int iMore1 = min(pp->st, j+pp->hp)-pp->hp;
				if(j>(pp->st-pp->hp))
					j=(pp->st-pp->hp);
				if(pp->serial==ph->serial)
					tempeffect(DEREF_P_CHAR(ph),DEREF_P_CHAR(ph),35,j,0,15,0);//allow a delay
				else 
					tempeffect(DEREF_P_CHAR(ph),DEREF_P_CHAR(ph),35,j,0,5,0);// added suggestion by Ramases //-Fraz- must be checked
			}
		}
		else //Bandages used on a non-human
		{
			if (!CheckSkill((ph),VETERINARY,0,1000))
				sysmessage(s,"You are not skilled enough to heal that creature.");
			else
			{
				int healmin = (((ph->skill[HEALING]/5)+(ph->skill[VETERINARY]/5))+3); //OSI's formula for min amount healed (Skyfire)
				int healmax = (((ph->skill[HEALING]/5)+(ph->skill[VETERINARY]/2))+10); //OSI's formula for max amount healed (Skyfire)
				int j=RandomNum(healmin,healmax);
				if(j>(pp->st-pp->hp))
					j=(pp->st-pp->hp);
				pp->hp=j;
				updatestats(ph, 0);
				sysmessage(s,"You apply the bandages and the creature looks a bit healthier.");
			}
		}
		SetTimerSec(&ph->objectdelay,SrvParms->objectdelay+SrvParms->bandagedelay);
		pib->ReduceAmount(1);
	}
}

void cSkills::ArmsLoreTarget(int s)
{
	int total;
	float totalhp;
	char p2[100];
	P_CHAR pc_currchar = currchar[s];
	
	const PC_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return;

	if ( (pi->def==0 || pi->pileable)
		&& ((pi->lodamage==0 && pi->hidamage==0) && (pi->rank<1 || pi->rank>9)))
	{
		sysmessage(s, "That does not appear to be a weapon.");
		return;
	}
	if(pc_currchar->isGM())
	{
		sprintf((char*)temp, "Attack [%i] Defense [%i] Lodamage [%i] Hidamage [%i]", pi->att, pi->def, pi->lodamage, pi->hidamage);
		sysmessage(s, (char*)temp);
		return;
	}
	
	if (!CheckSkill(pc_currchar,ARMSLORE, 0, 250))
		sysmessage(s,"You are not certain...");
	else
	{
		if( pi->maxhp==0)
			sysmessage(s," Sorry this is a old item and it doesn't have maximum hp");
		else
		{
			totalhp= (float) pi->hp/pi->maxhp;
			strcpy((char*)temp,"This item ");
			if      (totalhp>0.9) strcpy((char*)p2, "is brand new."); 
			else if (totalhp>0.8) strcpy((char*)p2, "is almost new.");
			else if (totalhp>0.7) strcpy((char*)p2, "is barely used, with a few nicks and scrapes.");
			else if (totalhp>0.6) strcpy((char*)p2, "is in fairly good condition.");
			else if (totalhp>0.5) strcpy((char*)p2, "suffered some wear and tear.");
			else if (totalhp>0.4) strcpy((char*)p2, "is well used.");
			else if (totalhp>0.3) strcpy((char*)p2, "is rather battered.");
			else if (totalhp>0.2) strcpy((char*)p2, "is somewhat badly damaged.");
			else if (totalhp>0.1) strcpy((char*)p2, "is flimsy and not trustworthy.");
			else                  strcpy((char*)p2, "is falling apart.");
			strcat((char*)temp,p2);
			char temp2[33];
			sprintf(temp2," [%.1f %%]",totalhp*100);
			strcat((char*)temp,temp2);	// Magius(CHE) §
		}
		if (CheckSkill(pc_currchar,ARMSLORE, 250, 510))
		{
			if (pi->hidamage)
			{
				total = (pi->hidamage + pi->lodamage)/2;
				if      ( total > 26) strcpy((char*)p2, " Would be extraordinarily deadly.");
				else if ( total > 21) strcpy((char*)p2, " Would be a superior weapon.");
				else if ( total > 16) strcpy((char*)p2, " Would inflict quite a lot of damage and pain."); 
				else if ( total > 11) strcpy((char*)p2, " Would probably hurt your opponent a fair amount.");
				else if ( total > 6)  strcpy((char*)p2, " Would do some damage.");
				else if ( total > 3)  strcpy((char*)p2, " Would do minimal damage.");
				else                  strcpy((char*)p2, " Might scratch your opponent slightly.");
				strcat((char*)temp,p2);
				
				if (Skills->CheckSkill(pc_currchar, ARMSLORE, 500, 1000))
				{
					if      (pi->spd > 35) strcpy((char*)p2, " And is very fast.");
					else if (pi->spd > 25) strcpy((char*)p2, " And is fast.");
					else if (pi->spd > 15) strcpy((char*)p2, " And is slow.");
					else                   strcpy((char*)p2, " And is very slow.");
					strcat((char*)temp,p2);
				}
			}
			else
			{
				if      (pi->def> 12) strcpy((char*)p2, " Is superbly crafted to provide maximum protection.");
				else if (pi->def> 10) strcpy((char*)p2, " Offers excellent protection.");
				else if (pi->def> 8 ) strcpy((char*)p2, " Is a superior defense against attack.");
				else if (pi->def> 6 ) strcpy((char*)p2, " Serves as a sturdy protection.");
				else if (pi->def> 4 ) strcpy((char*)p2, " Offers some protection against blows.");
				else if (pi->def> 2 ) strcpy((char*)p2, " Provides very little protection.");
				else if (pi->def> 0 ) strcpy((char*)p2, " Provides almost no protection.");
				else                  strcpy((char*)p2, " Offers no defense against attackers.");
				strcat((char*)temp,p2);
			}
		}
		sysmessage(s, (char*)temp);

		if (!(pi->rank<1 || pi->rank>10 || SrvParms->rank_system==0))
		{
			if (Skills->CheckSkill(pc_currchar,ARMSLORE, 250, 500))
			{
				switch(pi->rank)
				{
					case 1: strcpy((char*)p2, "It seems an item with no quality!");				break;
					case 2: strcpy((char*)p2, "It seems an item very below standard quality!");	break;
					case 3: strcpy((char*)p2, "It seems an item below standard quality!");		break;
					case 4: strcpy((char*)p2, "It seems a weak quality item!");					break;
					case 5: strcpy((char*)p2, "It seems a standard quality item!");				break;
					case 6: strcpy((char*)p2, "It seems a nice quality item!");					break;
					case 7: strcpy((char*)p2, "It seems a good quality item!");					break;
					case 8: strcpy((char*)p2, "It seems a great quality item!");				break;
					case 9: strcpy((char*)p2, "It seems a beautiful quality item!");			break;
					case 10:strcpy((char*)p2, "It seems a perfect quality item!");				break;
				}
				sysmessage(s,p2);
			}
		}
	}
}

void cSkills::ItemIdTarget(int s)
{
	P_CHAR pc_currchar = currchar[s];
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if (!CheckSkill(pc_currchar, ITEMID, 0, 250))
		{
			sysmessage(s, "You can't quite tell what this item is...");
		}
		else
		{
			if(pi->corpse)
			{
				sysmessage(s, "You have to use your forensics evalutation skill to know more on this corpse.");
				return;
			}

			// Identify Item by Antichrist // Changed by MagiusCHE)
			if (CheckSkill(pc_currchar, ITEMID, 250, 500))
				if (pi->name2 && (strcmp(pi->name2,"#"))) strcpy(pi->name,pi->name2); // Item identified! -- by Magius(CHE)

			if(pi->name[0]=='#') pi->getName(temp2);
			else strcpy((char*)temp2, pi->name);
			sprintf((char*)temp, "You found that this item appears to be called: %s", temp2);
			sysmessage(s,(char*) temp);

			// Show Creator by Magius(CHE)
			if (CheckSkill(pc_currchar, ITEMID, 250, 500))
			{
				if (strlen(pi->creator)>0)
				{
					if (pi->madewith>0) sprintf((char*)temp2, "It is %s by %s",skill[pi->madewith-1].madeword,pi->creator); // Magius(CHE)
					else if (pi->madewith<0) sprintf((char*)temp2, "It is %s by %s",skill[0-pi->madewith-1].madeword,pi->creator); // Magius(CHE)
					else sprintf((char*)temp2, "It is made by %s",pi->creator); // Magius(CHE)
				} else strcpy((char*)temp2, "You don't know its creator!");
			} else strcpy((char*)temp2, "You can't know its creator!");
			sysmessage(s, (char*)temp2);
			// End Show creator

			if (!CheckSkill(pc_currchar, ITEMID, 250, 500))
			{
				sysmessage(s, "You can't tell if it is magical or not.");
			}
			else
			{
				if(pi->type!=15)
				{
					sysmessage(s, "This item has no hidden magical properties.");
				}
				else
				{
					if (!CheckSkill(pc_currchar, ITEMID, 500, 1000))
					{
						sysmessage(s,"This item is enchanted with a spell, but you cannot determine which");
					}
					else
					{
						if (!CheckSkill(pc_currchar, ITEMID, 750, 1100))
						{
							sprintf((char*)temp, "It is enchanted with the spell %s, but you cannot determine how many charges remain.",spellname[(8*(pi->morex-1))+pi->morey-1]);
							sysmessage(s,(char*)temp);
						}
						else
						{
							sprintf((char*)temp, "It is enchanted with the spell %s, and has %d charges remaining.",spellname[(8*(pi->morex-1))+pi->morey-1],pi->morez);
							sysmessage(s,(char*)temp);
						}
					}
				}
			}
		}
	}
}

///////////////
// Name:	Evaluate_int_Target
// history:	unknown, revamped by Duke, 2.10.2000
// Purpose:	implements the 'evaluate intelligence' skill
//
void cSkills::Evaluate_int_Target(UOXSOCKET s)
{

	P_CHAR pc = FindCharBySerPtr(buffer[s] + 7);
	P_CHAR pc_currchar = currchar[s];
	if(pc == NULL || pc_currchar == NULL) 
		return;

	if (pc == pc_currchar)
	{ 
        sysmessage(s, "You cannot analyze yourself!"); 
        return; 
	}


	// blackwind distance fix 
	if( pc->pos.distance(pc_currchar->pos) >= 10 ) 
	{ 
		sysmessage( s, "You need to be closer to find out" ); 
		return; 
	} 

	if (!CheckSkill(pc_currchar,EVALUATINGINTEL, 0, 1000)) 
	{
		sysmessage(s,"You are not certain..");
		return;
	}
	if ((pc->in == 0)) 
		sysmessage(s, "It looks smarter than a rock, but dumber than a piece of wood.");
	else
	{
		strcpy((char*)temp,"That person looks ");
		if		(pc->in <= 10)	strcat(temp, "slightly less intelligent than a rock.");
		else if (pc->in <= 20)	strcat(temp, "fairly stupid.");
		else if (pc->in <= 30)	strcat(temp, "not the brightest.");
		else if (pc->in <= 40)	strcat(temp, "about average.");
		else if (pc->in <= 50)	strcat(temp, "moderately intelligent.");
		else if (pc->in <= 60)	strcat(temp, "very intelligent.");
		else if (pc->in <= 70)	strcat(temp, "Extremely intelligent.");
		else if (pc->in <= 80)	strcat(temp, "extraordinarily intelligent.");
		else if (pc->in <= 90)	strcat(temp, "like a formidable intellect, well beyond the ordinary.");
		else if (pc->in <= 99)	strcat(temp, "like a definite genius.");
		else if (pc->in >=100)  strcat(temp, "superhumanly intelligent in a manner you cannot comprehend.");
		sysmessage(s, (char*)temp);
	}
}

///////////////
// Name:	AnatomyTarget
// history:	unknown, revamped by Duke, 2.10.2000
// Purpose:	implements the 'evaluate anatomy' skill
//
void cSkills::AnatomyTarget(int s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];

	if (pc == NULL || pc_currchar == NULL)
		return;

	if( dist(pc->pos, pc_currchar->pos) >= 10 )
	{
		sysmessage( s, "You need to be closer to find out more about them" );
		return;
	}
	
	if (!Skills->CheckSkill(pc_currchar,ANATOMY, 0, 1000)) 
	{
		sysmessage(s,"You are not certain..");
		return;
	}

	short dx = pc->effDex();
	if (pc->st == 0 && dx == 0) 
		sysmessage(s, "That does not appear to be a living being.");
	else
	{
		char *ps1,*ps2;
		if		(pc->st <= 10)	ps1="like they would have trouble lifting small objects ";
		else if (pc->st <= 20)	ps1="Rather Feeble";
		else if (pc->st <= 30)	ps1="Somewhat weak";
		else if (pc->st <= 40)	ps1="To be of normal strength";
		else if (pc->st <= 50)	ps1="Somewhat strong";
		else if (pc->st <= 60)	ps1="Very strong"; 
		else if (pc->st <= 70)	ps1="Extremely strong";
		else if (pc->st <= 80)	ps1="Extraordinarily strong";
		else if (pc->st <= 90)	ps1="Strong as an ox";
		else if (pc->st <= 99)	ps1="One of the strongest people you have ever seen";
		else if (pc->st >=100)  ps1="Superhumanly strong";

		if		(dx <= 10)	ps2="like they barely manage to stay standing";
		else if (dx <= 20)	ps2="Very clumsy";
		else if (dx <= 30)	ps2="Somewhat uncoordinated";
		else if (dx <= 40)	ps2="Moderately dextrous";
		else if (dx <= 50)	ps2="Somewhat agile";
		else if (dx <= 60)	ps2="Very agile";
		else if (dx <= 70)	ps2="Extremely agile";
		else if (dx <= 80)	ps2="Extraordinarily agile";
		else if (dx <= 90)	ps2="Moves like quicksilver";
		else if (dx <= 99) 	ps2="One of the fastest people you have ever seen";
		else if (dx >=100)  ps2="Superhumanly agile";
		sprintf((char*)temp,"That person looks %s and %s.", ps1, ps2); 
		sysmessage(s, (char*)temp);
	}
}

//taken from 6904t2(5/10/99) - AntiChrist
void cSkills::TameTarget(int s)
{
	int tamed=0;
	
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if ( pc == NULL ) return;
//	P_CHAR pc = MAKE_CHARREF_LR(i);
	P_CHAR pc_currchar = currchar[s];

	if(line_of_sight(-1, pc_currchar->pos, pc->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)==0)
	return;

	if(buffer[s][7]==0xFF) return;
	if (pc != NULL)
		if ((pc->isNpc() && (chardist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc)) <= 3))) //Ripper
		{
			if (pc->taming>1000||pc->taming==0)//Morrolan default is now no tame
			{
				sysmessage(s, "You can't tame that creature.");
				return;
			}
			// Below... can't tame if you already have!
			if( (pc->tamed) && pc_currchar->Owns(pc) )
			{
				sysmessage( s, "You already control that creature!" );
				return;
			}
			if( pc->tamed )
			{
				sysmessage( s, "That creature looks tame already." );
				return;
			}
			sprintf((char*)temp, "*%s starts to tame %s*",pc_currchar->name,pc->name);
			for(int a=0;a<3;a++)
			{
				switch(rand()%4)
				{
				case 0: npctalkall(DEREF_P_CHAR(pc_currchar), "I've always wanted a pet like you.",0); break;
				case 1: npctalkall(DEREF_P_CHAR(pc_currchar), "Will you be my friend?",0); break;
				case 2: sprintf((char*)temp, "Here %s.",pc->name); npctalkall(DEREF_P_CHAR(pc_currchar), (char*)temp,0); break;
				case 3: sprintf((char*)temp, "Good %s.",pc->name); npctalkall(DEREF_P_CHAR(pc_currchar), (char*)temp,0); break;
				default: 
					LogError("switch reached default");
				}
			}
			if ((!Skills->CheckSkill(pc_currchar,TAMING, 0, 1000))||
				(pc_currchar->skill[TAMING]<pc->taming)) 
			{
				sysmessage(s,"You were unable to tame it.");
				return;
			}
			npctalk(s, DEREF_P_CHAR(pc_currchar), "It seems to accept you as it's master!",0);
			tamed=1;
			pc->SetOwnSerial(pc_currchar->serial);
			pc->npcWander=0;
			if(pc->id1==0x00 && (pc->id2==0x0C || pc->id2==0x3B))
			{
				if(pc->skin != 0x0481)
				{
				    pc->npcaitype = 10;
					pc->tamed = true;
				}
				else
				{
			        pc->npcaitype=0;
			        pc->tamed = true;//AntiChrist fix
				}
			}
		}
		if (tamed==0) sysmessage(s,"You can't tame that!");
}

void cSkills::StealingTarget(int s) // re-arranged by LB 22-dec 1999
{
	int i, skill;
	char temp2[512];
	tile_st tile;
	P_CHAR pc_currchar = currchar[s];
    int cansteal = max(1,pc_currchar->baseskill[STEALING]/10);
	cansteal = cansteal * 10;
		
	if (buffer[s][7]<0x40)
	{
		Skills->RandomSteal(s);
		return;
	}

	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi)
	{
		sysmessage(s,"You cannot steal that.");
		return;
	}
	if( pi->id()==0x1E2D || pi->id()==0x1E2C )
	{
		Skills->PickPocketTarget(s);
		return;
	}
	if ( pi->layer!=0	// no stealing for items on layers other than 0 (equipped!) ,
		|| pi->priv&2	// newbie items,
		|| pi->isInWorld() )	// and items not being in containers allowed !
	{
		sysmessage(s,"You cannot steal that.");
		return;
	}
	if (pi->weight>cansteal) // LB, bugfix, (no weight check)
	{
		sysmessage(s,"That is too heavy.");
		return;
	}
	
	P_CHAR pc_npc = GetPackOwner(pi);
//	 = MAKE_CHARREF_LR(npc);
	if (pc_npc->npcaitype == 17)
	{
		sysmessage(s, "You cannot steal that.");
		return;
	}
	
	if (pc_npc == pc_currchar)
	{
		sysmessage(s,"You catch yourself red handed.");
		return;
	}

	skill = Skills->CheckSkill(pc_currchar,STEALING,0,999);
	if (npcinrange(s,DEREF_P_CHAR(pc_npc),1))
	{
		if (skill)
		{
			P_ITEM pi_pack = Packitem(pc_currchar);
			if (pi_pack == NULL) 
				return;
			pi->SetContSerial(pi_pack->serial);
			sysmessage(s,"You successfully steal that item.");
			all_items(s);
		} else sysmessage(s, "You failed to steal that item.");
		
		if (((!(skill))&&(rand()%16==7)) || (pc_currchar->skill[STEALING]<rand()%1001))
		{
			sysmessage(s,"You have been cought!");
			
			if (pc_npc != NULL) //lb
			{
				if (pc_npc->isNpc()) npctalkall(DEREF_P_CHAR(pc_npc), "Guards!! A thief is amoung us!",0);

				criminal( DEREF_P_CHAR(pc_currchar) );

				if (pc_npc->isInnocent() && pc_currchar->attacker != pc_npc->serial && Guilds->Compare(DEREF_P_CHAR(pc_currchar),DEREF_P_CHAR(pc_npc))==0)//AntiChrist
					criminal(DEREF_P_CHAR(pc_currchar));//Blue and not attacker and not guild
			
				if (pi->name!="#")
				{
					sprintf((char*)temp,"You notice %s trying to steal %s from you!",pc_currchar->name,pi->name);
					sprintf((char*)temp2,"You notice %s trying to steal %s from %s!",pc_currchar->name,pi->name,pc_npc->name);
				} else
				{
					Map->SeekTile(pi->id(),&tile);
					sprintf((char*)temp,"You notice %s trying to steal %s from you!",pc_currchar->name,tile.name);
					sprintf((char*)temp2,"You notice %s trying to steal %s from %s!",pc_currchar->name,tile.name,pc_npc->name);
				}
				sysmessage(s,(char*)temp); //lb
			}
			for(i=0;i<now;i++)
			{
				if (perm[i])
				{

				  if((i!=s)&&(inrange1p(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(currchar[i])))&&(rand()%10+10==17||(rand()%2==1 && currchar[i]->in>=pc_currchar->in))) sysmessage(s,temp2);
				}
			}

		}
	} else sysmessage(s, "You are too far away to steal that item.");
}

void cSkills::BeggingTarget(int s)
{
	int ci,gold,x,y,realgold;
	char abort;
	P_CHAR pc_currchar = currchar[s];

	addid1[s]=buffer[s][7];
	addid2[s]=buffer[s][8];
	addid3[s]=buffer[s][9];
	addid4[s]=buffer[s][10];
	SERIAL serial = calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	P_CHAR pc = FindCharBySerial( serial );

	if(online(DEREF_P_CHAR(pc)))
	{
		sysmessage(s,"Maybe you should just ask.");
		return;
	}

	if (pc != NULL)
	{
		if(chardist(DEREF_P_CHAR(pc),DEREF_P_CHAR(pc_currchar))>=5)
		{
			sysmessage(s,"You are not close enough to beg.");
			return;
		}

		if(pc->isHuman() && (pc->in != 0)) //Used on human
		{
			if (pc->begging_timer>=uiCurrentTime)
			{
				npctalk(s,DEREF_P_CHAR(pc),"Annoy someone else !",1);
				return;
			}

			npctalkall(DEREF_P_CHAR(pc_currchar), begging_data.text[rand()%3],0); // npcemoteall?
			if (!Skills->CheckSkill(pc_currchar,BEGGING, 0, 1000))
				sysmessage(s,"They seem to ignore your begging plees.");
			else
			{
				SetTimerSec(&pc->begging_timer,begging_data.timer); 
				x=pc->skill[BEGGING]/50;

				if (x<1) x=1; 
				y=rand()%x;
				y+=RandomNum(1,4); 
				if (y>25) y=25;
				// pre-calculate the random amout of gold that is "targeted"

				P_ITEM pi_p = Packitem(pc);
				gold=0;
				realgold=0;
				abort=0;

				// check for gold in target-npc pack
			
				if (pi_p != NULL)				
				{
					vector<SERIAL> vecContainer = contsp.getData(pi_p->serial);
					for (ci = 0; ci < vecContainer.size(); ci++)
					{
						P_ITEM pi_j =  FindItemBySerial(vecContainer[ci]);
						if (pi_j != NULL)
						{
							if (pi_j->id()==0x0EED )
							{
								gold+=pi_j->amount; // calc total gold in pack
								
								int k = pi_j->amount;
								if(k>=y) // enough money in that pile in pack to satisfy pre-aclculated amount
								{
									pi_j->amount-=y;
									realgold+=y; // calc gold actually given to player
									if (pi_j->amount<=0) 
										Items->DeleItem(pi_j); // delete pile if its totaly gone afteer subtracting gold
									RefreshItem(pi_j); // resend new amount
									abort = 1;
								}
								else // not enough money in this pile -> only delete it
								{
									Items->DeleItem(pi_j);
									RefreshItem(pi_j); // Refresh a deleted item?
									realgold += pi_j->amount;
								}
							}
						} // end of if j!=-1
						if (abort) break;
					} 
				}
								
				if (gold<=0)
				{				
					npctalk(s,DEREF_P_CHAR(pc),"Thou dost not look trustworthy... no gold for thee today! ",1);
					return;
				}
				npctalkall(DEREF_P_CHAR(pc),"I feel sorry for thee... here have a gold coin .",0);
				addgold(s,realgold);
				sysmessage(s,"Some gold is placed in your pack.");
			}
		}
		else
			sysmessage(s, "That would be foolish.");
	}
}

void cSkills::AnimalLoreTarget(int s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s] + 7);
	if(pc == NULL) return;
	P_CHAR pc_currchar = currchar[s];

	// blackwind distance fix 
	if( chardist( DEREF_P_CHAR(pc), DEREF_P_CHAR(pc_currchar) ) >= 10 ) 
	{ 
		sysmessage( s, "You need to be closer to find out more about them" ); 
		return; 
	} 

	if (pc->isGMorCounselor())
	{
		sysmessage(s, "Little is known of these robed gods.");
		return;
	}
	if (pc->isHuman()) // Used on human
	{
		sysmessage(s, "The human race should use dvorak!");
		return;
	}
	else // Lore used on a non-human
	{
		if (Skills->CheckSkill(pc_currchar, ANIMALLORE, 0, 1000))
		{
			sprintf((char*)temp, "Attack [%i] Defense [%i] Taming [%i] Hit Points [%i]", pc->att, pc->def, pc->taming/10, pc->hp);
			npcemote(s, DEREF_P_CHAR(pc), (char*)temp, 0);
			return;
		}
		else
		{
			sysmessage(s, "You can not think of anything relevant at this time.");
			return;
		}
	}
}

void cSkills::ForensicsTarget(int s) //AntiChrist
{
	int curtim=uiCurrentTime;
	const PC_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];

	if (!pi || !(pi->corpse))
	{
		sysmessage(s, "That does not appear to be a corpse.");
		return;
	}
	
	if(pc_currchar->isGM())
	{
		sprintf((char*)temp,"The %s is %i seconds old and the killer was %s.", pi->name, (curtim-pi->murdertime)/MY_CLOCKS_PER_SEC, pi->murderer);
		sysmessage(s, (char*)temp);
	}
	else
	{
		if (!Skills->CheckSkill(pc_currchar, FORENSICS, 0, 500)) sysmessage(s,"You are not certain about the corpse."); else
		{
			if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)<=60) strcpy((char*)temp2,"few");
			if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)>60) strcpy((char*)temp2,"many");
			if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)>180) strcpy((char*)temp2,"many many");
			sprintf((char*)temp,"The %s is %s seconds old.", pi->name, temp2);
			sysmessage(s,(char*)temp);
			if (!Skills->CheckSkill(pc_currchar, FORENSICS, 500, 1000) || *(pi->murderer)=='\0') sysmessage(s,"You can't say who was the killer."); else
			{
				sprintf((char*)temp,"The killer was %s.",pi->murderer);
				sysmessage(s,(char*)temp);
			}
		}
	}
}

void cSkills::PoisoningTarget(int s) //AntiChrist
{
//	CHARACTER cc=currchar[s];
	P_CHAR pc = currchar[s];

	P_ITEM pPoi=FindItemBySerial(pc->poisonserial);
	if (!pPoi) return;

	if(pPoi->type!=19 || pPoi->morey!=6)
	{
		sysmessage(s,"That is not a valid poison!");
		return;
	}
	
	pc->poisonserial=0;
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi)
	{
		int success=0;
		switch(pPoi->morez)
		{
		case 1:	success=Skills->CheckSkill((pc), POISONING, 0, 500);		break;//lesser poison
		case 2:	success=Skills->CheckSkill((pc), POISONING, 151, 651);		break;//poison
		case 3:	success=Skills->CheckSkill((pc), POISONING, 551, 1051);		break;//greater poison
		case 4:	success=Skills->CheckSkill((pc), POISONING, 901, 1401);		break;//deadly poison
		default:
			LogError("switch reached default");
			return;
		}
		if(success)
		{
			soundeffect2(DEREF_P_CHAR(pc), 0x02, 0x47); //poisoning effect
			//-Frazurbluu-  adding the weapons that may be posioned here..
			// also need to adjust poisoning damages..
			if(pi->poisoned<pPoi->morez) pi->poisoned=pPoi->morez;
			sysmessage(s,"You successfully poison that item.");
		} else
		{
			soundeffect2(DEREF_P_CHAR(pc), 0x02, 0x47); //poisoning effect
			sysmessage(s,"You fail to apply the poison.");
		}
		
		//empty bottle after poisoning
		P_ITEM pi_poison = pPoi;
		if (!pPoi->isInWorld()) 
			contsp.remove(pPoi->contserial, pi_poison->serial);
		unsigned char k1 = pPoi->ser1;
		unsigned char k2 = pPoi->ser2;
		unsigned char k3 = pPoi->ser3;
		unsigned char k4 = pPoi->ser4;
		int kser = pPoi->serial;

		pPoi->Init(0);
		pPoi->ser1=k1;
		pPoi->ser2=k2;
		pPoi->ser3=k3;
		pPoi->ser4=k4;
		pPoi->serial=kser;
		pPoi->setId(0x0F0E);
		pPoi->pileable=1;
		pPoi->MoveTo(pc->pos.x,pc->pos.y,pc->pos.z);
		pPoi->priv|=0x01;
		RefreshItem(pPoi);
	}
	else
	{
		sysmessage(s,"You can't poison that item.");
		return;
	}
	pc->poisonserial=0;
}

void cSkills::PickPocketTarget(int s) // PickPocket dip`s..Ripper
{
	P_CHAR pc_currchar = currchar[s];

	if (pc_currchar->skill[STEALING] < 300)
	// check if under 30 in stealing
	{
		Skills->CheckSkill(pc_currchar,STEALING, 0, 1000);
		// check their skill
		soundeffect(s, 0x02, 0x49);
		// rustling sound..dont know if right but it works :)
	}else{
		sysmessage(s, "You learn nothing from practicing here");
		// if over 30 Stealing..dont learn.
	}
}

void cSkills::LockPick(int s)
{
	int success;
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi && !pi->isLockedDown()) // Ripper
	{
		P_ITEM piPick = FindItemBySerial(addmitem[s]);
		if (piPick == NULL)
			return;
		if(pi->type==1 || pi->type==12 || pi->type==63) 
		{
			sysmessage(s, "That is not locked.");
			return;
		}
		
		if(pi->type==8 || pi->type==13 || pi->type==64)
		{
			if(pi->more1==0 && pi->more2==0 && pi->more3==0 && pi->more4==0)
			{ //Make sure it isn't an item that has a key (i.e. player house, chest..etc)
				//if(addmitem[s]==-1) 
				
				if(piPick->amount==0xFFFFFFFF)
				{
					if (currentSpellType[s] !=2)			// not a wand cast
					{
						success=Magic->SubtractMana(pc_currchar, 5);  // subtract mana on scroll or spell
						if (currentSpellType[s] == 0)	// del regs on normal spell
						{
							reag_st regs = {0,};
							regs.ash = regs.moss = 1;
							Magic->DelReagents(DEREF_P_CHAR(pc_currchar), regs);
						}

					}
					switch(pi->type)
					{
					case 8:  pi->type=1;  break;
					case 13: pi->type=12; break;
					case 64: pi->type=63; break;
					default:
						LogError("switch reached default");
						return;
					}
					soundeffect3(pi, 0x01FF);
					sysmessage(s, "You manage to pick the lock.");
				} else
					if(Skills->CheckSkill(pc_currchar, LOCKPICKING, 0, 1000))
					{
						switch(pi->type)
						{
						case 8: pi->type=1; break;
						case 13: pi->type=12; break;
						case 64: pi->type=63; break;
						default:
							LogError("switch reached default");
							return;
						}
						soundeffect3(pi, 0x0241);
						sysmessage(s, "You manage to pick the lock.");
					} else
					{
						if((rand()%100)>50) 
						{
							sysmessage(s, "You broke your lockpick!");
							if(piPick->amount>1)
							{
								piPick->ReduceAmount(1);
							}
							else
							{
								P_ITEM pi = FindItemBySerial(addmitem[s]);
							    Items->DeleItem(pi);
							}
						} else
						{
							sysmessage(s, "You fail to open the lock.");
						}
					}
			} else
			{
				sysmessage(s, "That cannot be unlocked without a key.");
			}
		}
	}
}

void cSkills::Tinkering(int s)
{
	P_CHAR pc_currchar = currchar[s];
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(pi->id()==0x1BEF || pi->id()==0x1BF2 || IsLog(pi->id()) ) 
		{
			if (CheckInPack(s,pi))
			{
				int amt;
				itemmake[s].has=amt=pc_currchar->CountItems(pi->id(), pi->color());
				if(amt<2)
				{ 
					sysmessage(s,"You don't have enough ingots to make anything.");
					return;
				}
				itemmake[s].Mat1id=pi->id();
				if ( IsLog(pi->id()) )
				{
					if (amt<4)
					{
						sysmessage(s,"You don't have enough log's to make anything.");
						return;
					} 
					else Skills->MakeMenu(s,70,TINKERING);
				}
				else
				{
					itemmake[s].Mat1color=pi->color();	// only if ingots are used
					Skills->MakeMenu(s,80,TINKERING);
				}
			}
			return;
		}
	}
	sysmessage(s,"You cannot use that material for tinkering.");
}

//////////////////////////////////
// name:	cTinkerCombine
// history: by Duke, 3.06.2000
// Purpose: handles the combining of two (tinkering-)items after the user
//			dclicked one and then targeted the second item
//
//			It's a base class for three derived classes that handle the
//			old interfaces
//
class cTinkerCombine	// Combining tinkering items
{
protected:
	char* failtext;
	short badsnd1;
	short badsnd2;
	short itembits;
	short minskill;
	short id2;
public:
	cTinkerCombine(short badsnd=0x51, char *failmsg="You break one of the parts.")
	{
		badsnd1=badsnd>>8;
		badsnd2=badsnd&0x00FF;
		failtext=failmsg;
		itembits=0;
		minskill=100;
	}
	/*
	virtual void delonfail(SOCK s)		{deletematerial(s, itemmake[s].needs/2);}
	virtual void delonsuccess(SOCK s)	{deletematerial(s, itemmake[s].needs);}
	virtual void failure(SOCK s)		{delonfail(s);playbad(s);failmsg(s);}
	*/
	virtual void failmsg(int s)			{sysmessage(s,failtext);}
	virtual void playbad(int s)			{soundeffect(s,badsnd1,badsnd2);}
	virtual void playgood(int s)		{soundeffect(s,0,0x2A);}
	virtual void checkPartID(short id)	{;}
	virtual bool decide()				{return (itembits == 3) ? true : false;}
	virtual void createIt(int s)		{;}
	static cTinkerCombine* factory(short combinetype);
	virtual void DoIt(int s)
	{
		P_ITEM piClick = FindItemBySerial( calcserial(addid1[s], addid2[s], addid3[s], addid4[s]) );
		if( piClick == NULL )
		{
			sysmessage( s, "Original part no longer exists" );
			return;
		}
		
		const P_ITEM piTarg=FindItemBySerPtr(buffer[s]+7);
		if (piTarg==NULL || piTarg->isLockedDown())
		{
			sysmessage(s,"You can't combine these.");
			return;
		}
		
		// make sure both items are in the player's backpack
		P_ITEM pPack=Check4Pack(s);
		if (pPack==NULL) return;
		if ( piTarg->contserial!=pPack->serial
			|| piClick->contserial!=pPack->serial)
		{
			sysmessage(s,"You can't use material outside your backpack");
			return;
		}
		
		// make sure the parts are of correct IDs AND they are different
		checkPartID( piClick->id() );
		checkPartID( piTarg->id() );
		if (!decide())
			sysmessage(s,"You can't combine these.");
		else
		{
			P_CHAR pc_currchar = currchar[s];

			if (pc_currchar->skill[TINKERING]<minskill)
			{
				sysmessage(s,"You aren't skilled enough to even try that!");
				return;
			}
			if( !Skills->CheckSkill( pc_currchar, TINKERING, minskill, 1000 ) )
			{
				failmsg(s);
				P_ITEM piLoser= rand()%2 ? piTarg : piClick;
				piLoser->ReduceAmount(1);
				playbad(s);
			}
			else
			{
				sysmessage(s,"You combined the parts");
				piClick->ReduceAmount(1);
				piTarg->ReduceAmount(1);		// delete both parts 
				createIt(s);						// spawn the item
				playgood(s);
			}
		}
	}
};

class cTinkCreateAwG : public cTinkerCombine
{
public:
	cTinkCreateAwG() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x105B || id==0x105C) itembits |= 0x01; // axles
		if (id==0x1053 || id==0x1054) itembits |= 0x02; // gears
	}
	virtual void createIt(int s)
	{
		Items->SpawnItem(s,DEREF_P_CHAR(currchar[s]),1,"an axle with gears",1,0x10,0x51,0,0,1,1);
	}
};

class cTinkCreateParts : public cTinkerCombine
{
public:
	cTinkCreateParts() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x1051 || id==0x1052) itembits |= 0x01; // axles with gears
		if (id==0x1055 || id==0x1056) itembits |= 0x02; // hinge
		if (id==0x105D || id==0x105E) itembits |= 0x04; // springs
	}
	virtual bool decide()
	{
		if (itembits == 3) {id2=0x59; minskill=300; return true;}	// sextant parts
		if (itembits == 5) {id2=0x4F; minskill=400; return true;}	// clock parts
		return false;
	}
	virtual void createIt(int s)
	{
		 char sztemp[15] ;
		if (id2 == 0x4F)
		  strcpy(sztemp,"clock parts") ;
		else
		  strcpy(sztemp,"sextant parts") ;
		char *pn = sztemp ;
		//#else
		//char *pn = (id2==0x4F) ? "clock parts" : "sextant parts";
		//#endif
		Items->SpawnItem(s,DEREF_P_CHAR(currchar[s]),1,pn,1,0x10,id2,0,0,1,1);
	}
};

class cTinkCreateClock : public cTinkerCombine
{
public:
	cTinkCreateClock() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x104D || id==0x104E) itembits |= 0x01; // clock frame
		if (id==0x104F || id==0x1050) itembits |= 0x02; // clock parts
	}
	virtual bool decide()   {minskill=600; return cTinkerCombine::decide();}
	virtual void createIt(int s)
	{
		Items->SpawnItem(s,DEREF_P_CHAR(currchar[s]),1,"clock",0,0x10,0x4B,0,0,1,1);
	}
};

#define cTC_AwG		11
#define cTC_Parts	22
#define cTC_Clock	33

cTinkerCombine* cTinkerCombine::factory(short combinetype)
{
	switch (combinetype)
	{
		case cTC_AwG:   return new cTinkCreateAwG();break;
		case cTC_Parts: return new cTinkCreateParts();break;	// clock/sextant parts
		case cTC_Clock: return new cTinkCreateClock();break;	// clock
		default:		return new cTinkerCombine();break;		// a generic handler
	}
	return NULL ;
}

void cSkills::TinkerAxel(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_AwG);
	ptc->DoIt(s);
}

void cSkills::TinkerAwg(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Parts);
	ptc->DoIt(s);
}

void cSkills::TinkerClock(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Clock);
	ptc->DoIt(s);
}

void cSkills::RepairTarget(UOXSOCKET s)
{ // Ripper..Repair items.
//	CHARACTER cc=currchar[s];
	P_CHAR pc = currchar[s];

	short smithing=pc->baseskill[BLACKSMITHING];

	if (smithing < 500)
	{
		sysmessage(s,"* Your not skilled enough to repair items.*");
		return;
	}

	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown())
	{
		if (!CheckInPack(s,pi)) 
			return;
		if (!pi->hp)
		{
			sysmessage(s," That item cant be repaired.");
			return;
		}
		if(!AnvilInRange(s))
		{
			sysmessage(s," Must be closer to the anvil.");
			return;
		}
		if (pi->hp>=pi->maxhp)
		{
			sysmessage(s," That item is at full strength.");
			return;
		}
		short dmg=4;	// damage to maxhp
		if		((smithing>=900)) dmg=1;
		else if ((smithing>=700)) dmg=2;
		else if ((smithing>=500)) dmg=3;

		if (Skills->CheckSkill((pc),BLACKSMITHING, 0, 1000))
		{
			pi->maxhp-=dmg;
			pi->hp=pi->maxhp;
			sysmessage(s," * the item has been repaired.*");
		}
		else
		{
			pi->hp-=2;
			pi->maxhp-=1;
			sysmessage(s," * You fail to repair the item. *");
			sysmessage(s," * You weaken the item.*");
		}
	}
}

//////////////////////////////////
// name:	SmeltItemTarget
// history: by Ripper
//			revamped by Duke, 30.11.2001
// Purpose: Smelting items.
//			Only items crafted by a (player-)Blacksmith can be smelted
//
void cSkills::SmeltItemTarget(UOXSOCKET s)
{
	unsigned short int sk=MINING;

//	CHARACTER cc=currchar[s];
	P_CHAR pc = currchar[s];

	cItem* pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return; 
	if (!CheckInPack(s,pi)) return;

	int a=1+pi->weight/100;	// number of ingots you get depends on the weight (Duke)

	if (pi->isLockedDown() || pi->rank!=30 || (pi->smelt < 1 || pi->smelt > 10 ))
	{
		sysmessage(s,"You cant smelt that item!");
		return;
	}
	if(!ForgeInRange(s))
	{
		sysmessage(s," Must be closer to the forge.");
		return;
	}
	if(pc->skill[sk]< 300 || ( pc->skill[sk]<500 && pi->smelt!=1 ) )
	{
		sysmessage(s,"You aren't skilled enough to even try that!");
		return;
	}
	if (Skills->CheckSkill((pc),sk, 0, 1000))
	{
		char* Name = NULL;
		short Color = pi->color();
		switch(pi->smelt)
		{
		case 1:	Name="#";			 	Color=0x0961;	break;
		case 2:	Name="silver ingot"; 	Color=0x0000;	break;
		case 3:	Name="golden ingot"; 					break;
		case 4:	Name="agapite ingot";					break;
		case 5:	Name="shadow ingot"; 					break;
		case 6:	Name="mythril ingot";					break;
		case 7:	Name="bronze ingot"; 					break;
		case 8:	Name="verite ingot"; 					break;
		case 9:	Name="merkite ingot";					break;
		case 10:Name="copper ingot"; 					break;
		default:
			LogError("switch reached default");
			return;
		}
		cItem* Ingot = Items->SpawnItem(DEREF_P_CHAR(pc),a,Name,1,0x1BF2,Color,1);
		if (Ingot)
		{
			Ingot->weight = 20;	// that is 0.2 stone
			RefreshItem(Ingot);
			sysmessage(s,"you smelt the item and place some ingots in your pack.");
			Items->DeleItem(pi);
		}
	}
}

