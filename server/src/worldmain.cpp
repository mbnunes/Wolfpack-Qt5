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
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// WorldMain.cpp: implementation of the CWorldMain class.
//
//////////////////////////////////////////////////////////////////////

#include "worldmain.h"
#undef  DBGFILE
#define DBGFILE "worldmain.cpp"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CWorldMain::CWorldMain()
{
	announce(false);

	iWsc = cWsc = NULL;
	Cur = 0;
	PerLoop = -1;
	isSaving = false;
}

CWorldMain::~CWorldMain()
{
	if (iWsc)
		fclose(iWsc);
	if (cWsc)
		fclose(cWsc);
}


void loadchar(int x) // Load a character from WSC
{
	unsigned long k,b,c1 ;
	int i ;
	int j,a=0, loops=0;

	x=Npcs->MemCharFree();
	if (x == -1) return;
	P_CHAR pc = MAKE_CHARREF_LR(x);
	pc->Init(false);

	pc->hungertime=uiCurrentTime+(SrvParms->hungerrate*MY_CLOCKS_PER_SEC/2);	// avoid loss of one hungerpoint for each restart (Duke, 6.6.2001)

	do
	{
		readw2();
		switch(script1[0])
		{
		case 'A':
		case 'a':
			if (!strcmp((char*)script1, "ACCOUNT"))				{ pc->account=str2num(script2);  }
			else if (!strcmp((char*)script1, "ALLMOVE"))		{ pc->priv2=str2num(script2);}
			else if (!strcmp((char*)script1, "ATT"))			{ pc->att=str2num(script2);}
			else if (!strcmp((char*)script1, "ADVOBJ"))			{ pc->advobj=str2num(script2);}
		break;

		case 'B':
		case 'b':
			if (!(strcmp((char*)script1, "BODY")))
			{
				i=str2num(script2);
				pc->id1=i>>8;
				pc->id2=i%256;
			}
		break;

		case 'C':
		case 'c':
			if (!strcmp((char*)script1, "CREATIONDAY"))			{ pc->creationday = str2num(script2); }
			else if (!strcmp((char*)script1, "CARVE"))			{ pc->carve=str2num(script2); }
			else if (!strcmp((char*)script1, "CANTRAIN"))		  pc->cantrain = true; 
		break;

		case 'D':
		case 'd':
			if (!strcmp((char*)script1, "DISPZ"))				  pc->dispz=str2num(script2);
			else if (!strcmp((char*)script1, "DIR"))			  pc->dir=str2num(script2);
			else if (!strcmp((char*)script1, "DEXTERITY"))		  pc->setDex(str2num(script2));
			else if (!strcmp((char*)script1, "DEXTERITY2"))		  pc->setDecDex(str2num(script2));
			else if (!strcmp((char*)script1, "DEATHS"))			  pc->deaths=str2num(script2);
			else if (!strcmp((char*)script1, "DEAD"))			  pc->dead = true;   // dead only appears in file if it's true.
			else if (!strcmp((char*)script1, "DEF"))			  pc->def=str2num(script2);
			else if (!strcmp((char*)script1, "DISABLED"))		  pc->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC);
			else if (!strcmp((char*)script1, "DISABLEMSG"))		  strncpy(pc->disabledmsg,(char*)script2,255);
		break;

		case 'E':
		case 'e':
			if (!strcmp((char*)script1, "EMOTE"))
			{
				i=str2num(script2);
				pc->emotecolor1=i>>8;
				pc->emotecolor2=i%256;
			}
		break;

		case 'F':
		case 'f':
			if (!strcmp((char*)script1, "FAME"))				  pc->fame=str2num(script2);
			else if (!strcmp((char*)script1, "FIXEDLIGHT"))		  pc->fixedlight=str2num(script2);
			else if (!strcmp((char*)script1, "FONT"))			  pc->fonttype=str2num(script2);
			else if (!strcmp((char*)script1, "FLEEAT"))			  pc->fleeat=str2num(script2);
			else if (!strcmp((char*)script1, "FX1"))			  pc->fx1=str2num(script2);
			else if (!strcmp((char*)script1, "FY1"))			  pc->fy1=str2num(script2);
			else if (!strcmp((char*)script1, "FZ1"))			  pc->fz1=str2num(script2);
			else if (!strcmp((char*)script1, "FX2"))			  pc->fx2=str2num(script2);
			else if (!strcmp((char*)script1, "FY2"))			  pc->fy2=str2num(script2);
			else if (!strcmp((char*)script1, "FOODX"))			  pc->foodlocx=str2num(script2);
			else if (!strcmp((char*)script1, "FOODY"))			  pc->foodlocy=str2num(script2);
			else if (!strcmp((char*)script1, "FOODZ"))			  pc->foodlocz=str2num(script2);
		break;

		case 'G': 
		case 'g': 
			if (!strcmp((char*)script1, "GUILD TYPE")) 
			{ 
				i = str2num(script2);  // Get the guild type 
				if(i<-1||i>MAX_GUILDTYPE) 
					i=-1;     // Default to no guild type 
				pc->GuildType=i; 
			} 
			else if (!strcmp((char*)script1, "GUILD TRAITOR")) 
			{ 
				if(!strcmp((char*)script2, "YES")) 
					pc->GuildTraitor=true; // yes this character has switch guilds 
				else 
					pc->GuildTraitor=false; 
			} 
			else if (!strcmp((char*)script1, "GUILDTOGGLE"))   pc->guildtoggle=str2num(script2);
			else if (!strcmp((char*)script1, "GUILDNUMBER"))   pc->guildnumber=str2num(script2);
			else if (!strcmp((char*)script1, "GMRESTRICT"))    pc->gmrestrict=str2num(script2); 
		    else if (!strcmp((char*)script1, "GUILDTITLE"))    strcpy(pc->guildtitle,(char*)script2);
		    else if (!strcmp((char*)script1, "GUILDFEALTY"))   pc->guildfealty=str2num(script2);
			////////////////////////////////// 
			// Reads in the Gm movement effect int 
			// 
			// 
			else if (!strcmp((char*)script1, "GMMOVEEFF"))	   pc->gmMoveEff=str2num(script2);
  
			// 
			// 
			//Aldur 
			////////////////////////////////// 
		break;

		case 'H':
		case 'h':
			if (!strcmp((char*)script1, "HITPOINTS"))			  pc->hp=str2num(script2);
			else if (!strcmp((char*)script1, "HIDAMAGE"))		  pc->hidamage=str2num(script2);
			else if (!strcmp((char*)script1, "HIDDEN"))			  pc->hidden=str2num(script2);
			else if (!strcmp((char*)script1, "HUNGER"))			  pc->hunger=str2num(script2);
			else if (!strcmp((char*)script1, "HOLDGOLD"))		  pc->holdg=str2num(script2);
			else if (!strcmp((char*)script1, "HOMEX"))			  pc->homelocx=str2num(script2); 
			else if (!strcmp((char*)script1, "HOMEY"))			  pc->homelocy=str2num(script2); 
			else if (!strcmp((char*)script1, "HOMEZ"))			  pc->homelocz=str2num(script2); 
		break;

		case 'I':
		case 'i':
			if (!strcmp((char*)script1, "INTELLIGENCE"))		  pc->in=str2num(script2);
			else if (!strcmp((char*)script1, "INTELLIGENCE2"))	  pc->in2=str2num(script2);
		break;

		case 'J': 
		case 'j': 
			if (!strcmp((char*)script1, "JAILTIMER"))			  pc->jailtimer=uiCurrentTime+str2num(script2); 
			else if(!strcmp((char*)script1, "JAILCELL"))		  { pc->cell=str2num(script2); jails[pc->cell].occupied = true;} 
			else if(!strcmp((char*)script1, "JAILSECS"))		  pc->jailsecs=str2num(script2); 
		break;

		case 'K':
		case 'k':
			if (!strcmp((char*)script1, "KARMA"))				  pc->karma=str2num(script2);
			else if (!strcmp((char*)script1, "KILLS"))			  pc->kills=str2num(script2);
		break;

		case 'L':
		case 'l':
			if (!strcmp((char*)script1, "LODAMAGE"))		      pc->lodamage=str2num(script2);
		break;

		case 'M':
		case 'm':
			if (!strcmp((char*)script1, "MENUPRIV"))			  pc->menupriv=str2num(script2);
			else if (!strcmp((char*)script1, "MANA"))			  pc->mn=str2num(script2);
		break;

		case 'N':
		case 'n':
			if (!strcmp((char*)script1, "NPC"))					  pc->npc=str2num(script2);
			else if (!strcmp((char*)script1, "NPCWANDER"))		  pc->npcWander=str2num(script2);
			else if (!strcmp((char*)script1, "NOTRAIN"))		  pc->cantrain=false;
			else if (!strcmp((char*)script1, "NPCTYPE"))		  pc->npc_type=str2num(script2);
			else if (!strcmp((char*)script1, "NAME"))			  strcpy(pc->name,(char*)script2);
			else if (!strcmp((char*)script1, "NPCAITYPE"))		  pc->npcaitype=str2num(script2);
		break;

		case 'O':
		case 'o':
			if (!strcmp((char*)script1, "OLDNPCWANDER"))		  pc->oldnpcWander=str2num(script2);
			else if (!strcmp((char*)script1, "OLDX"))			  jails[pc->cell].oldpos.x=str2num(script2);
			else if (!strcmp((char*)script1, "OLDY"))			  jails[pc->cell].oldpos.y=str2num(script2);
			else if (!strcmp((char*)script1, "OLDZ"))			  jails[pc->cell].oldpos.z=str2num(script2);
			else if (!strcmp((char*)script1, "OWN"))
			{
				i=str2num(script2);
				pc->ownserial=i;
				//taken from 6904t2(5/10/99) - AntiChrist
				if (pc->ownserial!=-1)
				{
					setptr(&cownsp[i%HASHMAX], DEREF_P_CHAR(pc)); //Load into charsp array
					pc->tamed = true;				// Abaddon	// bugfix JM/LB
				}
			}
		break;

		case 'P':
		case 'p':
			if (!strcmp((char*)script1, "PRIV"))				  pc->setPriv(str2num(script2));
			else if (!strcmp((char*)script1, "PRIV3a"))			  pc->priv3[0]=str2num(script2);
			else if (!strcmp((char*)script1, "PRIV3b"))			  pc->priv3[1]=str2num(script2);
			else if (!strcmp((char*)script1, "PRIV3c"))			  pc->priv3[2]=str2num(script2);
			else if (!strcmp((char*)script1, "PRIV3d"))			  pc->priv3[3]=str2num(script2);
			else if (!strcmp((char*)script1, "PRIV3e"))			  pc->priv3[4]=str2num(script2);
			else if (!strcmp((char*)script1, "PRIV3f"))			  pc->priv3[5]=str2num(script2);
			else if (!strcmp((char*)script1, "PRIV3g"))			  pc->priv3[6]=str2num(script2);
			else if (!strcmp((char*)script1, "POISON"))			  pc->poison=str2num(script2);
			else if (!strcmp((char*)script1, "POISONED"))		  pc->poisoned=str2num(script2);
		break;

		case 'Q':
		case 'q':
			if (!strcmp((char*)script1, "QUESTTYPE"))			  pc->questType=str2num(script2);
			else if (!strcmp((char*)script1, "QUESTDESTREGION"))  pc->questDestRegion=str2num(script2);
			else if (!strcmp((char*)script1, "QUESTORIGREGION"))  pc->questOrigRegion=str2num(script2);
			else if (!strcmp((char*)script1, "QUESTBOUNTYPOSTSERIAL")) pc->questBountyPostSerial=str2num(script2);
			else if (!strcmp((char*)script1, "QUESTBOUNTYREWARD")) pc->questBountyReward=str2num(script2);
		break;

		case 'R':
		case 'r':
			if (!strcmp((char*)script1, "RACE"))		  pc->race=str2num(script2);
			else if (!strcmp((char*)script1, "ROBE"))				pc->robe = str2num(script2);
			else if (!strcmp((char*)script1, "REATTACKAT"))		  pc->reattackat=str2num(script2);
		// no cells have been marked as -1, but no cell=0
		// with that line its not necassairy to take it manually out.
		break;

		case 'S':
		case 's':
			if (!strcmp((char*)script1, "SERIAL"))
			{
				unsigned int i = str2num(script2);
				if (charcount2<=i) charcount2=i+1;
				pc->ser1=i>>24;
				pc->ser2=i>>16;
				pc->ser3=i>>8;
				pc->ser4=i%256;
				pc->serial=i;
				setptr(&charsp[i%HASHMAX], DEREF_P_CHAR(pc)); //Load into charsp array
			}
			else if (!strcmp((char*)script1, "SAY"))				pc->saycolor = static_cast<UI16>(str2num(script2));
			else if (!strcmp((char*)script1, "STRENGTH"))			pc->st=str2num(script2);
			else if (!strcmp((char*)script1, "STRENGTH2"))			pc->st2=str2num(script2);
			else if (!strcmp((char*)script1, "STAMINA"))			pc->stm=str2num(script2);
			else if (!strcmp((char*)script1, "SUMMONREMAININGSECONDS"))
			{//AntiChrist - let's restore remaining seconds!
				i=str2num(script2);
				pc->summontimer=uiCurrentTime+(i*MY_CLOCKS_PER_SEC);
			}
			else if (!strcmp((char*)script1, "SHOP"))			  pc->shop=str2num(script2);
			else if (!strcmp((char*)script1, "SPEECH"))			  pc->speech=str2num(script2);
			else if ((script1[0]=='S')&&(script1[1]=='K')&&(script1[2]=='I')&&
				(script1[3]=='L')&&(script1[4]=='L'))
			{
				pc->baseskill[j=str2num(&script1[5])]=str2num(script2);
				Skills->updateSkillLevel(DEREF_P_CHAR(pc), j);
			}
			else if (!strcmp((char*)script1, "SKIN"))			  pc->skin = static_cast<UI16>(str2num(script2));
			else if (!strcmp((char*)script1, "SPATTACK"))		  pc->spattack=str2num(script2);
			else if (!strcmp((char*)script1, "SPADELAY"))		  pc->spadelay=str2num(script2);
			else if (!strcmp((char*)script1, "SPLIT"))			  pc->split=str2num(script2);
			else if (!strcmp((char*)script1, "SPLITCHANCE"))	  pc->splitchnc=str2num(script2);
			else if (!strncmp((char*)script1, "SKL", 3 ))		// for skill locking
			{
				pc->lockSkill[j=str2num(&script1[3])]=str2num(script2);
			}
			else if (!strcmp((char*)script1, "SPAWNREGION"))
			{
				pc->spawnregion = str2num(script2);
				if (pc->spawnregion < 0 || pc->spawnregion >= spawnregion.size())
				{
					pc->spawnregion = 0;
				} else
					spawnregion[pc->spawnregion].current++;
			}
			else if (!strcmp((char*)script1, "SPAWN"))
			{
				pc->spawnserial=str2num(script2);;
				if (pc->spawnserial != INVALID_SERIAL) 
					cspawnsp.insert(pc->spawnserial, pc->serial);
			}
			else if (!strcmp((char*)script1, "STABLEMASTER"))
			{
				pc->stablemaster_serial=str2num(script2);
				if (pc->stablemaster_serial>0)
					stablesp.insert(pc->stablemaster_serial, pc->serial);
				//setptr(&stablesp[pc->stablemaster_serial%HASHMAX], DEREF_P_CHAR(pc));
			}
		break;

		case 'T':
		case 't':
			if (!strcmp((char*)script1, "TITLE"))				  strcpy(pc->title,(char*)script2);
			else if (!strcmp((char*)script1, "TAMING"))			  pc->taming=str2num(script2);
			else if (!strcmp((char*)script1, "TRIGGER"))		  pc->trigger=str2num(script2);
			else if (!strcmp((char*)script1, "TRIGWORD"))		  strcpy(pc->trigword,(char*)script2);
			else if (!strcmp((char*)script1, "TIME_UNUSED"))	  pc->time_unused=str2num(script2);
		break;

		case 'X':
		case 'x':
			if (!strcmp((char*)script1, "X"))					  pc->pos.x=str2num(script2);
			else if (!strcmp((char*)script1, "XBODY"))
			{
				i=str2num(script2);
				pc->xid1=i>>8;
				pc->xid2=i%256;
			}
			else if (!strcmp((char*)script1, "XSKIN"))			  pc->xskin = static_cast<UI16>(str2num(script2));
		break;

		case 'W':
		case 'w':
			if (!strcmp((char*)script1, "WAR"))					  pc->war=str2num(script2);
			else if (!strcmp((char*)script1, "WORKX"))			{ pc->worklocx=str2num(script2);}
			else if (!strcmp((char*)script1, "WORKY"))			{ pc->worklocy=str2num(script2);}
			else if (!strcmp((char*)script1, "WORKZ"))			{ pc->worklocz=str2num(script2);}
		break;

		case 'Y':
		case 'y':
			if (!strcmp((char*)script1, "Y"))					  pc->pos.y=str2num(script2);
		break;

		case 'Z':
		case 'z':
			if (!strcmp((char*)script1, "Z"))					  pc->dispz=pc->pos.z=str2num(script2);
		break;
		}
		loops++;
 }
 while (strcmp((char*)script1, "}") && loops<=200);
 if (pc->isNpc() && pc->war) pc->war=0;


 ////////////////////////////////////////////////////////////////////

 // LB autocorrect for same faulty previous code
 int zeta;
 for (zeta = 0;zeta<ALLSKILLS;zeta++) if (pc->lockSkill[zeta]!=0 && pc->lockSkill[zeta]!=1 && pc->lockSkill[zeta]!=2) pc->lockSkill[zeta]=0;

 //AntiChrist bugfix for hiding
 pc->priv2 &= 0xf7; // unhide - AntiChrist
 pc->hidden = 0;
 pc->stealth = -1;

 //AntiChrist bugfix for magic reflect
 pc->priv2=pc->priv2&0xBF;


 pc->region = calcRegionFromXY(pc->pos.x, pc->pos.y); //LB bugfix

 pc->antispamtimer=0;//LB - AntiSpam -
 pc->antiguardstimer=0; //AntiChrist - AntiSpam for "GUARDS" call - to avoid (laggy) guards multi spawn
 // lord binarys body/skin autocorrection code
 k = pc->id();
 if (k<=0x3e1)
 {
			c1 = pc->skin;
			b = c1&0x4000;
			if ((b==16384 && (k >=0x0190 && k<=0x03e1)) || c1==0x8000)
			{
				if (c1!=0xf000)
				{
					pc->skin = pc->xskin = 0xF000;
					clConsole.send("char/player: %s : %i correted problematic skin hue\n",pc->name,pc->serial);
				}
			}
 } else	// client crashing body --> delete if non player esle put onl”x a warning on server screen
	 // we dont want to delete that char, dont we ?
 {
	if (pc->account==-1)
	{
		Npcs->DeleteChar(DEREF_P_CHAR(pc));
	} else
	{
		pc->id1 = 0x01;
		pc->id2 = 0x90;
		clConsole.send("player: %s with bugged body-value detected, restored to male shape\n",pc->name);
	}
 }

 //a = mapRegions->AddItem(x+CharacterOffset);
 if(pc->stablemaster_serial == 0)
 { 
    mapRegions->Add(pc); 
 } 

 // begin of meta gm stuff

 for (int u=0;u<7;u++)
 {
	 if (pc->isPlayer())
	 {
		 if (pc->priv3[u]==0) // dont overwrite alreday saved settings
		 {
			 if (!pc->isGMorCounselor()) pc->priv3[u]=metagm[2][u]; //normal player defaults
			 if (pc->isCounselor()) pc->priv3[u]=metagm[1][u]; // couscelor defaults
			 if (pc->isGM()) pc->priv3[u]=metagm[0][u]; // gm defaults
				if (pc->account==0) pc->priv3[u]=0xffffffff;
		 }
	 }
 }

 if (pc->isPlayer() && pc->account==0) pc->menupriv=-1;


 int max_x = MapTileWidth  * 8;
 int max_y = MapTileHeight * 8;
   if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account ==-1) || ((pc->pos.x>max_x || pc->pos.y>max_y) && pc->account == -1))
// if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account ==-1) || ((pc->pos.x>max_x || pc->pos.y>max_y || pc->pos.x<0 || pc->pos.y<0) && pc->account==-1))
 {
	 Npcs->DeleteChar(DEREF_P_CHAR(pc)); //character in an invalid location
 }
if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account != -1) || (( pc->pos.x>max_x || pc->pos.y>max_y ) && pc->account !=-1))
// if ((pc->pos.x < 100 && pc->pos.y < 100 && pc->account !=-1) || ((pc->pos.x>max_x || pc->pos.y>max_y || pc->pos.x<0 || pc->pos.y<0) && pc->account!=-1))
 {
	 pc->MoveTo(900,300,30); //player in an invalid location
 }
 setcharflag(pc);//AntiChrist
}

void loaditem (int x) // Load an item from WSC
{
	unsigned long int i,b;
	int si;
	int loops=0;
	char bad=0;

	const P_ITEM pi = Items->MemItemFree();
	if (pi == NULL) return;

	pi->Init(0);
	pi->ser1=0x40;

	/*pi->id1='\x0F';
	pi->id2='\xA6'; */

	// who did this why ?
	// 0xfa6 is a client crasher casue it doesnt exist..
	// changed to 1, LB

	pi->setId(0x0001);


	do
	{
		readw2();
		switch (script1[0])
		{
		case 'a':
		case 'A':
			if (!(strcmp((char*)script1, "AMOUNT"))) { pi->amount=str2num(script2); }
			else if (!(strcmp((char*)script1, "ATT"))) { pi->att=str2num(script2); }
			break;

		case 'c':
		case 'C':
			if (!(strcmp((char*)script1, "COLOR")))
			{
				i = str2num(script2);
	
				b=((i&0x4000)>>14)+((i&0x8000)>>15);
				if(i == 32767)
				{ 
					b = 0; // Corrects the etheral color so horses you are rideing maintain their color.(Aldur)
				} 
				if (!b)
				{
					pi->color1=i>>8;
					pi->color2=i%256;
				} else
				{
					pi->color1=0; // bugged color found, leave it undyed
					pi->color2=0;
					clConsole.send("item# %i with problematic hue corrected\n",pi->serial);
				}
			}
			else if (!(strcmp((char*)script1, "CONT")))
			{
				i=str2num(script2);
				pi->SetContSerial(i);
			}
			else if (!(strcmp((char*)script1, "CORPSE"))) { pi->corpse=str2num(script2); }
			else if (!(strcmp((char*)script1, "CARVE"))) { pi->carve=str2num(script2); }
			else if (!(strcmp((char*)script1, "CREATOR")))	{ strcpy(pi->creator,(char*)script2); } // by Magius(CHE)
			break;

		case 'd':
		case 'D':
			if (!(strcmp((char*)script1, "DISABLEMSG"))){ strcpy(pi->disabledmsg,(char*)script2);} // Added by Magius(CHE) §
			else if (!(strcmp((char*)script1, "DISABLED"))) { pi->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC); }//AntiChrist
			else if (!(strcmp((char*)script1, "DIR"))) { pi->dir=str2num(script2); }
			else if (!(strcmp((char*)script1, "DOORFLAG"))) { pi->doordir=str2num(script2); }
			else if (!(strcmp((char*)script1, "DYEABLE"))) { pi->dye=str2num(script2); }
			else if (!(strcmp((char*)script1, "DEF"))) { pi->def=str2num(script2); }
			else if (!(strcmp((char*)script1, "DESC"))) { strcpy(pi->desc,(char*)script2); }
			else if (!(strcmp((char*)script1, "DX"))) { pi->dx=str2num(script2);}
			else if (!(strcmp((char*)script1, "DX2"))) { pi->dx2=str2num(script2);}
			else if (!(strcmp((char*)script1, "DECAYTIME"))) {si = str2num(script2);pi->decaytime = (si<0) ? 1 : si;}
			break;

		case 'g':
		case 'G':
			if (!(strcmp((char*)script1, "GOOD"))) { pi->good=str2num(script2);} // Added by Magius(CHE)
			else if (!(strcmp((char*)script1, "GLOW"))) { pi->glow=str2num(script2); }
			else if (!(strcmp((char*)script1, "GLOWBC")))
			{
				i=str2num(script2);
				pi->glow_c1=i>>8;
				pi->glow_c2=i%256;
			}
			else if (!(strcmp((char*)script1, "GLOWTYPE"))) { pi->glow_effect=str2num(script2);  }
			break;
		
		case 'h':
		case 'H':
			if (!(strcmp((char*)script1, "HIDAMAGE"))) { pi->hidamage=str2num(script2); }
			else if (!(strcmp((char*)script1, "HP"))) { pi->hp=str2num(script2); }
			break;
		
		case 'i':
		case 'I':
			if (!(strcmp((char*)script1, "IN"))) { pi->in=str2num(script2); }
			else if (!(strcmp((char*)script1, "IN2"))) { pi->in2=str2num(script2); }
			else if (!(strcmp((char*)script1, "ITEMHAND"))) { pi->itmhand=str2num(script2); }
			else if (!(strcmp((char*)script1, "ID")))
			{
				i=str2num(script2);
				pi->setId(i);
				if (i>=0x4000)
				{
					SI32 length;
					UOXFile *mfile;
					Map->SeekMulti(i-0x4000, &mfile, &length);
					length=length/sizeof(st_multi);
					if (length<=-1 || length>170000)
					{
						clConsole.send("\nbad item, serial: %i name: %s multi-l: %i\n",pi->serial,pi->name,length);
						clConsole.send("deleted\n");
						bad=1;
					}
				}
			}
			break;
		
		case 'l':
		case 'L':
			if (!(strcmp((char*)script1, "LAYER"))) { pi->layer=str2num(script2); }
			else if (!(strcmp((char*)script1, "LODAMAGE"))) { pi->lodamage=str2num(script2); }
			break;
		
		case 'm':
		case 'M':
			if (!(strcmp((char*)script1, "MORE")))
			{
				i=str2num(script2);
				pi->more1=(unsigned char)(i>>24);
				pi->more2=(unsigned char)(i>>16);
				pi->more3=(unsigned char)(i>>8);
				pi->more4=(unsigned char)(i%256);
			}
			else if (!(strcmp((char*)script1, "MORE2")))
			{
				i=str2num(script2);
				pi->moreb1=(unsigned char)(i>>24);
				pi->moreb2=(unsigned char)(i>>16);
				pi->moreb3=(unsigned char)(i>>8);
				pi->moreb4=(unsigned char)(i%256);
			}
			else if (!(strcmp((char*)script1, "MOVABLE"))) { pi->magic=str2num(script2); }
			else if (!(strcmp((char*)script1, "MAXHP"))) { pi->maxhp=str2num(script2); }
			else if (!(strcmp((char*)script1, "MOREX"))) { pi->morex=str2num(script2); }
			else if (!(strcmp((char*)script1, "MOREY"))) { pi->morey=str2num(script2); }
			else if (!(strcmp((char*)script1, "MOREZ"))) { pi->morez=str2num(script2); }
			break;
		
		case 'n':
		case 'N':
			if (!(strcmp((char*)script1, "NAME")))		{ strcpy(pi->name,(char*)script2); }
			else if (!(strcmp((char*)script1, "NAME2")))	{ strcpy(pi->name2,(char*)script2); }
			break;
		
		case 'o':
		case 'O':
			if (!(strcmp((char*)script1, "OWNER")))
			{
				i=str2num(script2);
				pi->SetOwnSerial(i);
			}
			else if (!(strcmp((char*)script1, "OFFSPELL"))) { pi->offspell=str2num(script2); }
			break;
		
		case 'p':
		case 'P':
			if (!(strcmp((char*)script1, "PILEABLE"))) { pi->pileable=str2num(script2); }
			else if (!(strcmp((char*)script1, "POISONED"))) { pi->poisoned=str2num(script2); }
			else if (!(strcmp((char*)script1, "PRIV"))) { pi->priv=str2num(script2); }
			break;

		case 'r':
		case 'R':
			if (!(strcmp((char*)script1, "RACEHATE"))) { pi->racehate=str2num(script2); }
			else if (!(strcmp((char*)script1, "RESTOCK"))) { pi->restock=str2num(script2);}
			else if (!(strcmp((char*)script1, "RANK")))
			{
				pi->rank=str2num(script2); // By Magius(CHE)
				if (pi->rank<=0) { pi->rank=10; }
			}
			break;
		
		case 's':
		case 'S':
			if (!(strcmp((char*)script1, "SK_MADE")))	{ pi->madewith=str2num(script2); }// by Magius(CHE)
			else if (!(strcmp((char*)script1, "SERIAL")))
			{
				i=str2num(script2);
				if (itemcount2<=i) itemcount2=i+1;
				pi->ser1=(unsigned char)(i>>24);
				pi->ser2=(unsigned char)(i>>16);
				pi->ser3=(unsigned char)(i>>8);
				pi->ser4=(unsigned char)(i%256);
				pi->serial=i; //Tauriel
				setptr(&itemsp[i%HASHMAX], x); //set item in pointer array
			}
			else if (!(strcmp((char*)script1, "SMELT"))) { pi->smelt=str2num(script2); }
			else if (!(strcmp((char*)script1, "SECUREIT"))) { pi->secureIt=str2num(script2); }
			else if (!(strcmp((char*)script1, "ST"))) { pi->st=str2num(script2); }
			else if (!(strcmp((char*)script1, "ST2"))) { pi->st2=str2num(script2); }
			else if (!(strcmp((char*)script1, "SPD"))) { pi->spd=str2num(script2); }
			else if (!(strcmp((char*)script1, "SPAWN")))
			{
				i = str2num(script2);
				pi->SetSpawnSerial(i);
			}
			else if (!(strcmp((char*)script1, "SPAWNREGION")))	// new spawnregion recognition, LB
			{
				pi->spawnregion=str2num(script2);
				if (pi->spawnregion<0)
				{
					pi->spawnregion=0;
				}
				else
					spawnregion[pi->spawnregion].current++;
			}
			break;
		
		case 't':
		case 'T':
			if (!(strcmp((char*)script1, "TYPE"))) { pi->type=str2num(script2); }
			else if (!(strcmp((char*)script1, "TYPE2"))) { pi->type2=str2num(script2); }
			else if (!(strcmp((char*)script1, "TRIGGER"))) { pi->trigger=str2num(script2); }
			else if (!(strcmp((char*)script1, "TIME_UNUSED"))) { pi->time_unused=str2num(script2); } // house decay etc, LB
			else if (!(strcmp((char*)script1, "TRIGTYPE"))) { pi->trigtype=str2num(script2); }
			else if (!(strcmp((char*)script1, "TRIGON"))) { pi->trigon=str2num(script2); }//equiiped trigger setting -Frazurbluu-
			break;
		
		case 'u':
		case 'U':
			if (!(strcmp((char*)script1, "USES"))) { pi->tuses=str2num(script2); }
			break;
		
		case 'v':
		case 'V':
			if (!(strcmp((char*)script1, "VISIBLE"))) { pi->visible=str2num(script2); }
			else if (!(strcmp((char*)script1, "VALUE"))) { pi->value=str2num(script2); }
			break;
		
		case 'x':
		case 'X':
			if (!(strcmp((char*)script1, "X"))) { pi->pos.x=str2num(script2); }
			break;
		
		case 'y':
		case 'Y':
			if (!(strcmp((char*)script1, "Y"))) { pi->pos.y=str2num(script2); }
			break;
		
		case 'w':
		case 'W':
			if (!(strcmp((char*)script1, "WIPE"))) { pi->wipe=str2num(script2); }
			else if (!(strcmp((char*)script1, "WEIGHT")))
			{
				i=str2num(script2);
				pi->weight=i; // Ison 2-20-99
			}
			break;
		
		case 'z':
		case 'Z':
			if (!(strcmp((char*)script1, "Z"))) { pi->pos.z=str2num(script2); }
		break;
		default:
			break;
		}

		loops++;
	}
	while (strcmp((char*)script1, "}") && loops<=200);
	// StoreItemRandomValue(x,-1); // Magius(CHE) (2)


	pi->timeused_last=getNormalizedTime();


	//add item weight if item doesn't have it yet
	if (pi->weight<=0) // LB, changed from 29 to 0
	{
		pi->weight=0;
		pi->weight=pi->getWeight();
	}

	if (pi->maxhp==0) pi->maxhp=pi->hp;
	// Tauriel adding region pointers

	if (pi->isInWorld())
	{
		int max_x = MapTileWidth  * 8;
		int max_y = MapTileHeight * 8;
		mapRegions->Add(pi); // it reurns 1 if inalid, if invalid it DOESNT get added !!!
		if (pi->pos.x>max_x || pi->pos.y>max_y) 
		//if (pi->pos.x<0 || pi->pos.y<0 || pi->pos.x>max_x || pi->pos.y>max_y)	// lord bianry
		{
			Items->DeleItem(pi);	//these are invalid locations, delete them!
		}
	}
	if (bad) 
		Items->DeleItem(pi);
}

void CWorldMain::loadnewworld() // Load world from WOLFPACK.WSC
{
//	unsigned char memerr=0; // Changed by MAgius(che) (1)
	char outper[4];	// Magius(CHE) (1)
	unsigned int i=0;
	unsigned int percent=0,a=0,pred=0,maxm=0; // Magius(CHE) (1)
	*outper='\0';

	cmem=0;
	imem=0;
	clConsole.send("Loading World, Building map Regions, checking Item weight...\n");
	charcount=0;
	itemcount=0;
	charcount2=1;
	itemcount2=0x40000000;
	wscfile=fopen("wppcs.wsc","r");
	if(!wscfile)
	{
		clConsole.send("WARNING: wppcs.wsc not found. Defaulting to wolfpack.wsc\n");
		if (! Npcs->AllocateMemory(100))
			return;
	}
	else
	{
		//Get number for initial character memory needed ->
		int cnum=0;
		readw3();
		if (!(strcmp((char*)script1, "INITMEM"))) cnum=str2num(script2)+C_W_O_1 + 5000; // large initial memory as a q&d workaround for realloc prob (Duke)
		maxm=cnum; // Magius(CHE) (1)
		if (! Npcs->AllocateMemory(cnum))
			return;

		// check if the map-mode (ilshenar or britannia) and wordfiles match , LB
		
		bool ok;
		bool ilsh = MapTileHeight<300;
		bool s=false;

		readw3();

		if (!(strcmp((char*)script1, "ILSHENAR"))) ok = ilsh; 	       // worldfiles with ilsh tag runnning on an ilsh shard -> ok					
		else if (!(strcmp((char*)script1, "BRITANNIA"))) ok = !ilsh;  // worldfiles with brit tag running on a brit shard -> ok		
		else ok = !ilsh;                                       // old britannian worldfiles without map-type tag running on an ilsh shard -> not good
	                                                           // old britannian worldfiles without map-type tag running on a brit sahrd -> ok
		                                                        
        if (!(strcmp((char*)script1, "SECTION"))) s=true;             // old wordfiles that dont have the ilsh or brit tag might have a section tag next, prevents skipping of the first one in that case

		if (ilsh && !ok) 
		{ 		
			LogCritical("You are runnning an ILSHENAR map shard, you tryed to load BRITANNIA map shard wordfiles\n");
			exit(-1);
		}

		if (!ilsh && !ok)
		{		
			LogCritical("You are runnning a BRITANNIA map shard, you tryed to load ILSHENAR map shard wordfiles\n");
			exit(-1);
		}

		clConsole.send("Loading characters ");	// MAgius(CHE) (1)
		unsigned long loopexit=0;
		do
		{
			if (!s) readw3(); else s=false;
			if (!(strcmp((char*)script1, "SECTION")))
			{
				if (!(strcmp((char*)script2, "CHARACTER")))
				{
					loadchar(str2num(script3));
					a++;
					pred=percent; //Magius(CHE) (1)
					if(maxm==0) maxm++;
					percent=(int) (a*100)/(maxm);
					if (strlen(outper)>0)
					{ //Magius(CHE) (1)
						for (i=1;i<=strlen(outper)+1;i++) clConsole.send("\b");
						*outper='\0';
					}
				}
			}
			if (percent>pred) // Changed by MAgius(CHE) (1)
			{
				numtostr(percent,outper);
				clConsole.send("%s%%",outper);
				pred=percent;
			}
		} while (strcmp((char*)script1,"EOF") && !feof(wscfile) && (++loopexit < MAXLOOPS*10) );
		fclose(wscfile);
		if (strlen(outper)>0)
		{ //Magius(CHE) (1)
			for (i=1;i<=strlen(outper)+1;i++) clConsole.send("\b");
			*outper='\0';
		}
		clConsole.send("Done.\n"); // Magius(CHE)


		wscfile=fopen("wpitems.wsc", "r");
		if (wscfile==NULL)
		{
			clConsole.send("ERROR: wpitems.wsc not found. No items will be loaded.\n");
			if (! Items->AllocateMemory(100)) return;
		}
		else
		{
			//Get number for initial character memory needed ->
			int inum=0;
			readw3();
			if (!(strcmp((char*)script1, "INITMEM")))
				inum=str2num(script2)+I_W_O_1;
			maxm=inum; // Magius(CHE) (1)

			if (! Items->AllocateMemory(inum)) return;

			if (SrvParms->trade_system==1) clConsole.send("Loading items ");	// AntiChrist - sorry magius, but it's better to see in this way i think
			else clConsole.send("Loading items ");	// MAgius(CHE) (2)
			a=0; // Magius(CHE) (2)
			loopexit=0;
			do {
				readw3();
				if (!strcmp((char*)script1, "SECTION"))
				{
					if (!strcmp((char*)script2, "WORLDITEM")) {
						loaditem(str2num(script3));
					}

					else if (!strcmp((char*)script2, "GUILD"))
						Guilds->Read(str2num(script3));
					a++;
					pred=percent; //Magius(CHE) (1)
					if(maxm==0) maxm++;
					percent=(int) (a*100)/(maxm);
					if (strlen(outper)>0) { //Magius(CHE) (1)
						for (i=1;i<=strlen(outper)+1;i++) clConsole.send("\b");
						*outper='\0';
					}
				}
				if (percent>pred) // Changed by MAgius(CHE) (1)
				{
					numtostr(percent,outper);
					clConsole.send("%s%%",outper);
					pred=percent;
				}
			} while (strcmp((char*)script1, "EOF") && !feof(wscfile) && (++loopexit < MAXLOOPS*10) );
			fclose(wscfile);
		}
		if (strlen(outper)>0) { //Magius(CHE) (1)
			for (i=1;i<=strlen(outper)+1;i++) clConsole.send("\b");
			*outper='\0';
		}
		clConsole.send("Done.\n"); // Magius(CHE)
		clConsole.send("World Loaded.\n"); // Changed by MAgius(CHE) (1)
		clConsole.send("Loading Houses.\n");
		HouseManager->LoadHouses();
		clConsole.send("Done!\n");
		return;
	}
	if (! Items->AllocateMemory(100)) return;

	wscfile=fopen("wolfpack.wsc", "r");
	//if(wscfile==NULL)
	if(!wscfile)
		clConsole.send("ERROR: World data not found, using blank world instead.\n");
	else {
		unsigned long loopexit=0;
		do
		{
			readw3();
			if (!(strcmp((char*)script1, "SECTION")))
			{
				if (!(strcmp((char*)script2, "CHARACTER")))
					loadchar(str2num(script3));
				if (!(strcmp((char*)script2, "WORLDITEM")))
					loaditem(str2num(script3));
				if (!(strcmp((char*)script2, "GUILD")))
					Guilds->Read(str2num(script3));
			}
		}
		while (strcmp((char*)script1, "EOF") && !feof(wscfile) && (++loopexit < MAXLOOPS*10));
		fclose(wscfile);
		wscfile = NULL;
		clConsole.send("Done.\n");
	}
}

//o---------------------------------------------------------------------------o
//|	Class		:	CWorldMain::savenewworld(char x)
//|	Date		:	Unknown
//|	Programmer	:	Unknown		
//o---------------------------------------------------------------------------o
//| Purpose		:	Save current world state. Stores all values in an easily
//|					readable script file "*.wsc". This stores all world items
//|					and NPC/PC character information for a given shard
//o---------------------------------------------------------------------------o
void CWorldMain::savenewworld(char x)
{
//	int multi=-1;
//	unsigned int del=0;
//	unsigned int currenttime=uiCurrentTime;
	static unsigned long ocCount, oiCount;
	static char curAmount = 0;

	AllTmpEff->Off();

	if ( !Saving() )
	{
		//saveserverscript(x);
		saveserverscript();
		if (SrvParms->server_log) savelog("Server data save\n","server.log");
		// Dupois - If shard op has defined an ARCHIVEPATH in SERVER.SCP
		//          then archive the WSC to the directory specified.
		// Added Oct 29, 1998
		if (strlen(SrvParms->archivepath)>1)
		{
			save_counter++; // LB - added world backup rate
			if(( save_counter % SrvParms->backup_save_ratio ) == 0 ) fileArchive("wppcs.wsc", "wpitems.wsc", SrvParms->archivepath);
		}
		// End - Dupois

		//	gcollect();
		if ( announce() )
		{
			sysbroadcast("World data saving....");
			clConsole.send("Worldsave Started!\n" );
			clConsole.send("items: %i / %i\n",itemcount,imem );
		}


		cWsc=fopen("wppcs.wsc", "w");
		if (cWsc == NULL) 
		{
			clConsole.send("Error, couldn't open wppcs.wsc for writing. Check file permissions.\n");
			return;
		}
	
		fprintf(cWsc, "// Wolfpack World Script (WSC)[TEXT]\n");
		fprintf(cWsc, "// Generated by %s Version %s\n",  wp_version.betareleasestring.c_str() , wp_version.verstring.c_str() );
		fprintf(cWsc, "// This file replaces the function of the *.UOX files in pre-0.50 versions[TEXT]\n");
		fprintf(cWsc, "// Wolfpack CHARACTERS.\n");
		fprintf(cWsc, "INITMEM %i //Do NOT edit this line!\n",charcount);
		if (MapTileHeight<300) fprintf(cWsc, "ILSHENAR MAP WORLD\n\n"); else fprintf(cWsc, "BRITANNIA MAP WORLD\n\n");

		iWsc=fopen("wpitems.wsc", "w");
		if (iWsc == NULL)
		{
			clConsole.send("Error, couldn't open wpitems.wsc for writing. Check file permissions.\n");
			return;
		}
	
		fprintf(iWsc, "// Wolfpack World Script (WSC)[TEXT]\n");
		fprintf(iWsc, "// Generated by Wolfpack Version %s\n", wp_version.verstring.c_str() );
		fprintf(iWsc, "// This file replaces the function of the *.UOX files in pre-0.50 versions[TEXT]\n");
		fprintf(iWsc, "// Wolfpack items and Guild information.\n");
		fprintf(iWsc, "INITMEM %i //Do NOT edit this line!\n",itemcount); // lb bugfix, cut& paste bug, was charcount ...
		if (MapTileHeight<300) 
			fprintf(iWsc, "ILSHENAR MAP WORLD\n\n"); 
		else 
			fprintf(iWsc, "BRITANNIA MAP WORLD\n\n");

		oiCount = itemcount;
		ocCount = charcount;
		Cur = 0;
		isSaving = true;
	}

	if ( PerLoop > 0 )
		Max = min((Cur + PerLoop), max(oiCount, ocCount));
	else
		Max = max(oiCount, ocCount);

	P_ITEM piDefault = new cItem;	// create an item with the default values
	piDefault->Init(0);

	for ( ; Cur<Max ; Cur++)
	{
		if ( Cur < ocCount )
			SaveChar( Cur );
		if ( Cur < oiCount )
			SaveItem( Cur, piDefault);
	}	// end for loop
	if ( Cur == max(oiCount, ocCount) )
	{ //Done saving
		if ( announce() )
		{
			sysbroadcast("Worldsave Done!\n");
			clConsole.send("Worldsave Done!\n");
		}

		fprintf(cWsc,"\nEOF\n\n");
		fclose(cWsc);

		Guilds->Write( iWsc );
		fprintf(iWsc, "EOF\n\n");
		fclose(iWsc);

		Cur = Max = ocCount = oiCount = 0;
		isSaving = false;

		iWsc = cWsc = NULL;

	}
	if( announce() )
	{
		sysbroadcast("Saving Houses....");
		clConsole.send("Saving Houses....");
	}
	HouseManager->SaveHouses();
	if( announce() )
	{
		sysbroadcast("Done!\n");
		clConsole.send("Done!\n");
	}
	AllTmpEff->On();
	uiCurrentTime = getNormalizedTime();
}

int CWorldMain::announce()
{
	return DisplayWorldSaves;
}

void CWorldMain::announce(int choice)
{
	if(choice<1)
		DisplayWorldSaves=0;
	else
		DisplayWorldSaves=1;
}

void CWorldMain::SetLoopSaveAmt( long toSet )
{
	if ( toSet <= 0 )
		PerLoop = -1;
	else
		PerLoop = toSet;
}

long CWorldMain::LoopSaveAmt( void )
{
	return PerLoop;
}

bool CWorldMain::Saving( void )
{
	return isSaving;
}

void CWorldMain::SaveChar( CHARACTER i )
{
	char valid=0;
	int j;
	P_CHAR pc = MAKE_CHARREF_LR(i);
	P_CHAR pc_reference = new cChar;
	pc_reference->Init(false);

	if (!pc->free)
		valid=1;
	if (!SrvParms->savespawns && pc->spawnregion>0) valid=0;
	if (valid)
	{
			fprintf(cWsc, "SECTION CHARACTER %i\n", i);
			fprintf(cWsc, "{\n");
			fprintf(cWsc, "SERIAL %i\n", pc->serial);
			//AntiChrist - special incognito related stuff - 12/99
			if(pc->incognito)
			{//save original name
				fprintf(cWsc, "NAME %s\n", pc->orgname);
			} else
			{
				fprintf(cWsc, "NAME %s\n", pc->name);
			}

			fprintf(cWsc, "TITLE %s\n", pc->title);
			fprintf(cWsc, "ACCOUNT %i\n", pc->account);
			if (pc->creationday != pc_reference->creationday)
				fprintf(cWsc, "CREATIONDAY %i\n", pc->creationday);
		    if (pc->gmMoveEff != pc_reference->gmMoveEff) 
                fprintf(cWsc, "GMMOVEEFF %i\n", pc->gmMoveEff);
			if(pc->GuildType != pc_reference->GuildType)
				fprintf(cWsc,"GUILDTYPE %i\n",pc->GuildType);
			else
				fprintf(cWsc,"GUILDTYPE -1\n");	//	Make sure that it gets written in oldchars made before this fix
			if(pc->GuildTraitor)
				fprintf(cWsc,"GUILDTRAITOR YES\n");
			else
				fprintf(cWsc,"GUILDTRAITOR NO\n");	//	Make sure that it gets written to for old chars made before this fix		
			if (pc->pos.x != pc_reference->pos.x)
				fprintf(cWsc, "X %i\n", pc->pos.x);
			if (pc->pos.y != pc_reference->pos.y)
				fprintf(cWsc, "Y %i\n", pc->pos.y);
			if (pc->pos.z != pc_reference->pos.z)
				fprintf(cWsc, "Z %i\n", pc->pos.z);
			if (pc->dispz != pc_reference->dispz)
				fprintf(cWsc, "DISPZ %i\n", pc->dispz);
			if (pc->cell != pc_reference->cell)
			{
				fprintf(cWsc, "JAILCELL %i\n", pc->cell);
				fprintf(cWsc, "OLDX %i\n", jails[pc->cell].oldpos.x);
				fprintf(cWsc, "OLDY %i\n", jails[pc->cell].oldpos.y);
				fprintf(cWsc, "OLDZ %i\n", jails[pc->cell].oldpos.z);
			}
			if (pc->dir != pc_reference->dir)
				fprintf(cWsc, "DIR %i\n", pc->dir);
			if (pc->race != pc_reference->race)
				fprintf(cWsc, "RACE %i\n", pc->race);
			//AntiChrist - incognito and polymorph spell special stuff - 12/99
			if(pc->incognito || pc->polymorph)
			{//if under incognito spell, don't save BODY but the original XBODY
				if ((pc->xid1<<8)+pc->xid2)
					fprintf(cWsc, "BODY %i\n", (pc->xid1<<8)+pc->xid2);
			} else
			{//else backup body normally
				if (pc->id() != pc_reference->id())
					fprintf(cWsc, "BODY %i\n", pc->id());
			}
			if ((pc->xid1<<8)+pc->xid2)
				fprintf(cWsc, "XBODY %i\n", (pc->xid1<<8)+pc->xid2);
			//AntiChrist - incognito spell special stuff - 12/99
			if(pc->incognito)
			{//if under incognito spell, don't save SKIN but the original XSKIN
				if (pc->xskin)
					fprintf(cWsc, "SKIN %i\n", pc->xskin);
			} else
			{//else backup skin normally
				if (pc->skin)
					fprintf(cWsc, "SKIN %i\n", pc->skin);
			}

			if (pc->xskin)
				fprintf(cWsc, "XSKIN %i\n", pc->xskin);
			if (pc->getPriv() != pc_reference->getPriv())
				fprintf(cWsc, "PRIV %i\n", pc->getPriv());


		    // beginn of meta-gm save
			if (pc->priv3[0] != pc_reference->priv3[0])
				fprintf(cWsc, "PRIV3a %i\n", pc->priv3[0]);
			if (pc->priv3[1] != pc_reference->priv3[1])
				fprintf(cWsc, "PRIV3b %i\n", pc->priv3[1]);
	        if (pc->priv3[2] != pc_reference->priv3[2])
				fprintf(cWsc, "PRIV3c %i\n", pc->priv3[2]);
			if (pc->priv3[3] != pc_reference->priv3[3])
				fprintf(cWsc, "PRIV3d %i\n", pc->priv3[3]);
            if (pc->priv3[4] != pc_reference->priv3[4])
				fprintf(cWsc, "PRIV3e %i\n", pc->priv3[4]);
			if (pc->priv3[5] != pc_reference->priv3[5])
				fprintf(cWsc, "PRIV3f %i\n", pc->priv3[5]);
			if (pc->priv3[6] != pc_reference->priv3[6])
				fprintf(cWsc, "PRIV3g %i\n", pc->priv3[6]);
			// end of meta-gm save

			if (pc->stablemaster_serial != pc_reference->stablemaster_serial)
			    fprintf(cWsc, "STABLEMASTER %i\n", pc->stablemaster_serial);
			if (pc->npc_type != pc_reference->npc_type)
				fprintf(cWsc, "NPCTYPE %i\n", pc->npc_type);
			if (pc->time_unused != pc_reference->time_unused)
				fprintf(cWsc, "TIME_UNUSED %i\n", pc->time_unused);
			                  
			if (pc->priv2 != pc_reference->priv2)
				fprintf(cWsc, "ALLMOVE %i\n", pc->priv2);
			if (pc->fonttype != pc_reference->fonttype)
				fprintf(cWsc, "FONT %i\n", pc->fonttype);
			if (pc->saycolor != pc_reference->saycolor)
				fprintf(cWsc, "SAY %i\n", pc->saycolor);
			if ((pc->emotecolor1<<8)+pc->emotecolor2 != (pc_reference->emotecolor1<<8) + pc_reference->emotecolor2)
				fprintf(cWsc, "EMOTE %i\n", (pc->emotecolor1<<8)+pc->emotecolor2);
			if (pc->st != pc_reference->st)
				fprintf(cWsc, "STRENGTH %i\n", pc->st);
			if (pc->st2 != pc_reference->st2)
				fprintf(cWsc, "STRENGTH2 %i\n", pc->st2);
			if (pc->realDex() != pc_reference->realDex())
				fprintf(cWsc, "DEXTERITY %i\n", pc->realDex());
			if (pc->decDex() != pc_reference->decDex())
				fprintf(cWsc, "DEXTERITY2 %i\n", pc->decDex());
			if (pc->in != pc_reference->in)
				fprintf(cWsc, "INTELLIGENCE %i\n", pc->in);
			if (pc->in2 != pc_reference->in2)
				fprintf(cWsc, "INTELLIGENCE2 %i\n", pc->in2);
			if (pc->hp != pc_reference->hp)
				fprintf(cWsc, "HITPOINTS %i\n", pc->hp);
			if (pc->spawnregion != pc_reference->spawnregion)
				fprintf(cWsc, "SPAWNREGION %i\n", pc->spawnregion);
			if (pc->stm != pc_reference->stm)
				fprintf(cWsc, "STAMINA %i\n", pc->stm);
			if (pc->mn != pc_reference->mn)
				fprintf(cWsc, "MANA %i\n", pc->mn);
			if (pc->isNpc() != pc_reference->isNpc())
				fprintf(cWsc, "NPC %i\n", pc->isNpc());
			if (pc->holdg != pc_reference->holdg) // bugfix lb, holdgold value never saved !!!
                fprintf(cWsc, "HOLDGOLD %i\n", pc->holdg);
			
			if (pc->shop != pc_reference->shop)
				fprintf(cWsc, "SHOP %i\n", pc->shop);
			if (pc->ownserial != pc_reference->ownserial)
				fprintf(cWsc, "OWN %i\n", pc->ownserial);
			if (pc->robe != pc_reference->robe)
				fprintf(cWsc, "ROBE %i\n", pc->robe);
			if (pc->karma != pc_reference->karma)
				fprintf(cWsc, "KARMA %i\n", pc->karma);
			if (pc->fame != pc_reference->fame)
				fprintf(cWsc, "FAME %i\n", pc->fame);
			if (pc->kills != pc_reference->kills)
				fprintf(cWsc, "KILLS %i\n", pc->kills);
			if (pc->deaths != pc_reference->deaths)
				fprintf(cWsc, "DEATHS %i\n", pc->deaths);
			if (pc->dead != pc_reference->dead)
				fprintf(cWsc, "DEAD 1\n");
//			if (pc->packitem)
//				fprintf(cWsc, "PACKITEM %i\n", pc->packitem);
			if (pc->fixedlight != pc_reference->fixedlight)
				fprintf(cWsc, "FIXEDLIGHT %i\n", pc->fixedlight);
			if (pc->speech != pc_reference->speech)
				fprintf(cWsc, "SPEECH %i\n", pc->speech);
			if (pc->trigger != pc_reference->trigger)
				fprintf(cWsc, "TRIGGER %i\n", pc->trigger);
			if (strcmp(pc->trigword, pc_reference->trigword))
				fprintf(cWsc, "TRIGWORD %s\n", pc->trigword);
			if (strcmp(pc->disabledmsg, pc_reference->disabledmsg))
				fprintf(cWsc, "DISABLEMSG %s\n", pc->disabledmsg); // Added by Magius(CHE) §
			
			for (j=0;j<TRUESKILLS;j++)
			{
				// Don't save the default value given by initchar
				if ((pc->baseskill[j] != pc_reference->baseskill[j])) 
				  fprintf(cWsc, "SKILL%i %i\n", j, pc->baseskill[j]);
				 
				if( pc->lockSkill[j] != pc_reference->lockSkill[j] ) 
					fprintf(cWsc, "SKL%i %i\n", j, pc->lockSkill[j] );
				 
			}
			if (!pc->cantrain)
				fprintf(cWsc, "NOTRAIN\n");
			else
				fprintf(cWsc, "CANTRAIN\n");

			if (pc->att != pc_reference->att)
				fprintf(cWsc, "ATT %i\n", pc->att);
			if (pc->def != pc_reference->def)
				fprintf(cWsc, "DEF %i\n", pc->def);
			if (pc->lodamage != pc_reference->lodamage)
				fprintf(cWsc, "LODAMAGE %i\n", pc->lodamage);
			if (pc->hidamage != pc_reference->hidamage)
				fprintf(cWsc, "HIDAMAGE %i\n", pc->hidamage);
			if (pc->war != pc_reference->war)
				fprintf(cWsc, "WAR %i\n", pc->war);
			if (pc->npcWander != pc_reference->npcWander)
				fprintf(cWsc, "NPCWANDER %i\n", pc->npcWander);
			if (pc->oldnpcWander != pc_reference->oldnpcWander)
				fprintf(cWsc, "OLDNPCWANDER %i\n", pc->oldnpcWander);
			if (pc->carve != pc_reference->carve)
				fprintf(cWsc, "CARVE %i\n", pc->carve);
			if (pc->fx1 != pc_reference->fx1)
				fprintf(cWsc, "FX1 %i\n", pc->fx1);
			if (pc->fy1 != pc_reference->fy1)
				fprintf(cWsc, "FY1 %i\n", pc->fy1);
			if (pc->fz1 != pc_reference->fz1)
				fprintf(cWsc, "FZ1 %i\n", pc->fz1);
			if (pc->fx2 != pc_reference->fx2)
				fprintf(cWsc, "FX2 %i\n", pc->fx2);
			if (pc->fy2 != pc_reference->fy2)
				fprintf(cWsc, "FY2 %i\n", pc->fy2);
			if (pc->spawnserial != pc_reference->spawnserial)
				fprintf(cWsc, "SPAWN %i\n", pc->spawnserial);
			if (pc->hidden != pc_reference->hidden)
				fprintf(cWsc, "HIDDEN %i\n", pc->hidden);
			if (pc->hunger != pc_reference->hunger)
				fprintf(cWsc, "HUNGER %i\n", pc->hunger);
			if (pc->npcaitype != pc_reference->npcaitype)
				fprintf(cWsc, "NPCAITYPE %i\n", pc->npcaitype);
			if (pc->spattack != pc_reference->spattack)
				fprintf(cWsc, "SPATTACK %i\n", pc->spattack);
			if (pc->spadelay != pc_reference->spadelay)
				fprintf(cWsc, "SPADELAY %i\n", pc->spadelay);
			if (pc->taming != pc_reference->taming)
				fprintf(cWsc, "TAMING %i\n", pc->taming);
			if (pc->summontimer != pc_reference->summontimer)//AntiChrist
				fprintf(cWsc, "SUMMONREMAININGSECONDS %i\n", (pc->summontimer/MY_CLOCKS_PER_SEC));

			if (pc->advobj != pc_reference->advobj)
				fprintf(cWsc, "ADVOBJ %i\n", pc->advobj);
			if (pc->poison != pc_reference->poison)
				fprintf(cWsc, "POISON %i\n", pc->poison);
			if (pc->poisoned != pc_reference->poisoned)
				fprintf(cWsc, "POISONED %i\n", pc->poisoned);
			if (pc->fleeat != pc_reference->fleeat)
				fprintf(cWsc, "FLEEAT %i\n", pc->fleeat);
			if (pc->reattackat != pc_reference->reattackat)
				fprintf(cWsc, "REATTACKAT %i\n", pc->reattackat);
			if (pc->split != pc_reference->split)
				fprintf(cWsc, "SPLIT %i\n", pc->split);
			if (pc->splitchnc != pc_reference->splitchnc)
				fprintf(cWsc, "SPLITCHANCE %i\n", pc->splitchnc);
// Begin of Guild related things (DasRaetsel)
			if (pc->guildtoggle != pc_reference->guildtoggle)
				fprintf(cWsc, "GUILDTOGGLE %i\n", pc->guildtoggle);  
			if (pc->guildnumber != pc_reference->guildnumber)
				fprintf(cWsc, "GUILDNUMBER %i\n", pc->guildnumber);  
			if (strcmp(pc->guildtitle, pc_reference->guildtitle))
				fprintf(cWsc, "GUILDTITLE %s\n", pc->guildtitle);  
			if (pc->guildfealty != pc_reference->guildfealty)
				fprintf(cWsc, "GUILDFEALTY %i\n", pc->guildfealty);  
			if (pc->guildfealty != pc_reference->guildfealty)
				fprintf(cWsc, "MURDERRATE %i\n",pc->murderrate);
			if (pc->menupriv != pc_reference->menupriv)
				fprintf(cWsc, "MENUPRIV %i\n", pc->menupriv);  
			if (pc->homelocx != pc_reference->homelocx)
				fprintf(cWsc, "HOMEX %i\n", pc->homelocx); 
			if (pc->homelocy != pc_reference->homelocy)
				fprintf(cWsc, "HOMEY %i\n", pc->homelocy); 
			if (pc->homelocz != pc_reference->homelocz)
				fprintf(cWsc, "HOMEZ %i\n", pc->homelocz); 
			if (pc->worklocx != pc_reference->worklocx)
				fprintf(cWsc, "WORKX %i\n", pc->worklocx); 
			if (pc->worklocy != pc_reference->worklocy)
				fprintf(cWsc, "WORKY %i\n", pc->worklocy); 
			if (pc->worklocz != pc_reference->worklocz)
				fprintf(cWsc, "WORKZ %i\n", pc->worklocz);
			if (pc->foodlocx != pc_reference->foodlocx)
				fprintf(cWsc, "FOODX %i\n", pc->foodlocx); 
			if (pc->foodlocy != pc_reference->foodlocy)
				fprintf(cWsc, "FOODY %i\n", pc->foodlocy); 
			if (pc->foodlocz != pc_reference->foodlocz)
				fprintf(cWsc, "FOODZ %i\n", pc->foodlocz); 
			// Dupois - Escort quests
			if (pc->questType != pc_reference->questType)
				fprintf(cWsc, "QUESTTYPE %i\n", pc->questType);  
			if (pc->questDestRegion != pc_reference->questDestRegion)
				fprintf(cWsc, "QUESTDESTREGION %i\n", pc->questDestRegion);  
			if (pc->questOrigRegion != pc_reference->questOrigRegion)
				fprintf(cWsc, "QUESTORIGREGION %i\n", pc->questOrigRegion);  
			if (pc->questBountyPostSerial != pc_reference->questBountyPostSerial)
				fprintf(cWsc, "QUESTBOUNTYPOSTSERIAL %i\n", pc->questBountyPostSerial);  
			if (pc->questBountyReward != pc_reference->questBountyReward)
				fprintf(cWsc, "QUESTBOUNTYREWARD %i\n", pc->questBountyReward);  
			if(pc->jailsecs) { 
				fprintf(cWsc,"JAILTIMER %i\n", pc->jailtimer-uiCurrentTime); 
				fprintf(cWsc,"JAILSECS %i\n",pc->jailsecs); 
			} 
			if (pc->gmrestrict != pc_reference->gmrestrict)
				fprintf(cWsc, "GMRESTRICT %i\n", pc->gmrestrict); 
			
			fprintf(cWsc, "}\n\n");	
	}
	delete pc_reference;
	pc_reference = NULL;
}

#if 0
	#define save_int(a,b) fprintf(iWsc,"%s %i\n",a,b)
	#define save_str(a,b) fprintf(iWsc,"%s %s\n",a,b)
	#define save_txt(txt) fprintf(iWsc,"%s\n",txt)
	#define save_sect
#else
	char *sp;
	#define save_int(a,b)	sp+=sprintf(sp,"%s %i\n",a,b)
	#define save_str(a,b)	sp+=sprintf(sp,"%s %s\n",a,b)
	#define save_txt(txt)	sp+=sprintf(sp,"%s\n",txt)
	//#define save_sect()		fputs(sectbuf,iWsc)
	#define save_sect()		save_sect2(sectbuf,iWsc)

	void save_sect2(char *sectbuf,FILE *iWsc)
	{
		fputs(sectbuf,iWsc);
	}

#endif

static void decay1(P_ITEM pi, int i)
{
	long serial, serhash, j;
	int k, ci;
	if (pi->corpse==1)
	{
		serial=pi->serial;
		vector<SERIAL> vecContainer = contsp.getData(pi->serial);
		for (ci=0;ci<vecContainer.size();ci++)
		{
			P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
			if( pi_j != NULL )
			{
				if ((pi_j->contserial==pi->serial) &&
					(pi_j->layer!=0x0B)&&(pi_j->layer!=0x10))
				{
					pi_j->SetContSerial(-1);
					pi_j->MoveTo(pi->pos.x, pi->pos.y, pi->pos.z);
					Items->DeleItem(pi_j);
				}
/*				if ((pi_j->contserial==pi->serial) &&
					(pi_j->free==0)&&
					((pi_j->layer==0x0B)||(pi_j->layer==0x10)))
				{
					Items->DeleItem(pi_j);
				}*/
			}
		}
	}
	else
	{
		if ( pi->multis == -1 )
		{
			P_ITEM pi_multi = findmulti( pi->pos );
			if( pi_multi == NULL )
			{
				Items->DeleItem(i);
			}
		}
	}
}

////////////
// name:	swapDragInfo
// pupose:	exchange current and OLD Position of an item
// history:	by Duke, 17.10.01
//
void swapDragInfo(P_ITEM pi)
{
	Coord_cl tmpPos;
	int tmpSer;
	signed char tmpLayer;

	tmpPos=pi->pos;
	tmpSer=pi->contserial;
	tmpLayer=pi->layer;

	pi->pos.x=pi->oldx;
	pi->pos.y=pi->oldy;
	pi->pos.z=pi->oldz;
	pi->contserial=pi->oldcontserial;
	pi->layer=pi->oldlayer;

	pi->oldx=tmpPos.x;
	pi->oldy=tmpPos.y;
	pi->oldz=tmpPos.z;
	pi->oldcontserial=tmpSer;
	pi->oldlayer=tmpLayer;
}

void CWorldMain::SaveItem( long i, P_ITEM pDefault)
{
	P_ITEM pi = MAKE_ITEMREF_LR(i);

	if (pi->priv&0x01 && pi->isInWorld() && !pi->free)
	{
		if( pi->decaytime == 0)
		{
			pi->startDecay();
		}
		else if (pi->decaytime<uiCurrentTime)
		{
			decay1(pi,i);
		}
	}

	char sectbuf[4000];
	if (!SrvParms->savespawns && pi->spawnregion>0) // if it was spawned by spawregions AND save spawnregions set to 0,
		return;		// dont save
	if (pi->flags.isBeeingDragged == true)	// dragged items need to be saved at OLD position
	{
		swapDragInfo(pi);
	}
	// removed y > 200 (wind errors) and 6044 changed to 6144 (Abaddon)
	if (pi->free==0 && ( !pi->isInWorld() || (pi->pos.x > 200 && pi->pos.x < 6144 && pi->pos.y < 4096 )))
	{
		sp=sectbuf;
		save_int("SECTION WORLDITEM", i);
		save_txt("{");
		save_int("SERIAL", pi->serial);
		save_str("NAME", pi->name);
		save_int("ID", pi->id());
		
		if (strcmp(pi->name2,pDefault->name2))			{save_str("NAME2",		pi->name2);}
		if (strlen(pi->creator)>0)						{save_str("CREATOR",	pi->creator );} // by Magius(CHE)
		if (pi->madewith	!= pDefault->madewith)		{save_int("SK_MADE",	pi->madewith );} // by Magius(CHE)
		if (pi->pos.x		!= pDefault->pos.x)			{save_int("X",			pi->pos.x);}
		if (pi->pos.y		!= pDefault->pos.y)			{save_int("Y",			pi->pos.y);}
		if (pi->pos.z		!= pDefault->pos.z)			{save_int("Z",			pi->pos.z);}
		if (pi->color()		!= pDefault->color())		{save_int("COLOR",		pi->color());}
		if (pi->contserial	!= INVALID_SERIAL)			{save_int("CONT",		pi->contserial);}
		if (pi->layer		!= pDefault->layer)			{save_int("LAYER",		pi->layer);}
		if (pi->itmhand		!= pDefault->itmhand)		{save_int("ITEMHAND",	pi->itmhand);}
		if (pi->type		!= pDefault->type)			{save_int("TYPE",		pi->type);}
		if (pi->type2		!= pDefault->type2)			{save_int("TYPE2",		pi->type2);}
		if (pi->offspell	!= pDefault->offspell)		{save_int("OFFSPELL",	pi->offspell);}
		if ((pi->more1<<24)+(pi->more2<<16)+(pi->more3<<8)+pi->more4) 
			{save_int("MORE", (pi->more1<<24)+(pi->more2<<16)+(pi->more3<<8)+pi->more4);}
		if ((pi->moreb1<<24)+(pi->moreb2<<16)+(pi->moreb3<<8)+pi->moreb4)
			{save_int("MORE2", (pi->moreb1<<24)+(pi->moreb2<<16)+(pi->moreb3<<8)+pi->moreb4);}
		if (pi->morex		!= pDefault->morex)			{save_int("MOREX",		pi->morex);}
		if (pi->morey		!= pDefault->morey)			{save_int("MOREY",		pi->morey);}
		if (pi->morez		!= pDefault->morez)			{save_int("MOREZ",		pi->morez);}
		if (pi->amount		!= pDefault->amount)		{save_int("AMOUNT",		pi->amount);}
		if (pi->pileable	!= pDefault->pileable)		{save_int("PILEABLE",	pi->pileable);}
		if (pi->doordir		!= pDefault->doordir)		{save_int("DOORFLAG",	pi->doordir);}
		if (pi->dye			!= pDefault->dye)			{save_int("DYEABLE",	pi->dye);}
		if (pi->decaytime	> 0)						{save_int("DECAYTIME",	pi->decaytime-uiCurrentTime);}
		if (pi->corpse		!= pDefault->corpse)		{save_int("CORPSE",		pi->corpse);}
		if (pi->att			!= pDefault->att)			{save_int("ATT",		pi->att);}
		if (pi->def			!= pDefault->def)			{save_int("DEF",		pi->def);}
		if (pi->hidamage	!= pDefault->hidamage)		{save_int("HIDAMAGE",	pi->hidamage);}
		if (pi->lodamage	!= pDefault->lodamage)		{save_int("LODAMAGE",	pi->lodamage);}
		if (pi->racehate	!= pDefault->racehate)		{save_int("RACEHATE",	pi->racehate);}
		if (pi->st			!= pDefault->st)			{save_int("ST",			pi->st);}
		if (pi->time_unused != pDefault->time_unused)	{save_int("TIME_UNUSED",pi->time_unused);} 
		if (pi->weight		!= pDefault->weight)		{save_int("WEIGHT",		pi->weight);}
		if (pi->hp			!= pDefault->hp)			{save_int("HP",			pi->hp);}
		if (pi->maxhp		!= pDefault->maxhp)			{save_int("MAXHP",		pi->maxhp );}
		if (pi->rank		!= pDefault->rank)			{save_int("RANK",		pi->rank );}
		if (pi->st2			!= pDefault->st2)			{save_int("ST2",		pi->st2);}
		if (pi->dx			!= pDefault->dx)			{save_int("DX",			pi->dx);}
		if (pi->dx2			!= pDefault->dx2)			{save_int("DX2",		pi->dx2);}
		if (pi->in			!= pDefault->in)			{save_int("IN",			pi->in);}
		if (pi->in2			!= pDefault->in2)			{save_int("IN2",		pi->in2);}
		if (pi->trigon		!= pDefault->trigon)		{save_int("TRIGON",		pi->trigon);}//-Frazurbluu- part of the ACC replacement
		if (pi->spd			!= pDefault->spd)			{save_int("SPD",		pi->spd);}
		if (pi->poisoned	!= pDefault->poisoned)		{save_int("POISONED",	pi->poisoned);}
		if (pi->wipe		!= pDefault->wipe)			{save_int("WIPE",		pi->wipe);}
		if (pi->magic		!= pDefault->magic)			{save_int("MOVABLE",	pi->magic);}
		if (pi->ownserial	!= pDefault->ownserial)		{save_int("OWNER",		pi->ownserial);}
		if (pi->visible		!= pDefault->visible)		{save_int("VISIBLE",	pi->visible);}
		if (pi->spawnserial != INVALID_SERIAL)			{save_int("SPAWN",		pi->spawnserial);}
		if (pi->dir			!= pDefault->dir)			{save_int("DIR",		pi->dir);}
		if (pi->priv		!= pDefault->priv)			{save_int("PRIV",		pi->priv);}
		if (pi->value		!= pDefault->value)			{save_int("VALUE",		pi->value);}
		if (pi->restock		!= pDefault->restock)		{save_int("RESTOCK",	pi->restock);}
		if (pi->trigger		!= pDefault->trigger)		{save_int("TRIGGER",	pi->trigger);}
		if (pi->trigtype	!= pDefault->trigtype)		{save_int("TRIGTYPE",	pi->trigtype);}
		if (pi->disabled	!= pDefault->disabled)		{save_int("DISABLED",	pi->disabled);}
		if (pi->spawnregion	!= pDefault->spawnregion)	{save_int("SPAWNREGION",pi->spawnregion);}
		if (pi->tuses		!= pDefault->tuses)			{save_int("USES",		pi->tuses);}
		if (pi->good		!= pDefault->good)			{save_int("GOOD",		pi->good);} // Magius(CHE)
		if (pi->secureIt	!= pDefault->secureIt)		{save_int("SECUREIT",	pi->secureIt);}
		if (pi->smelt		!= pDefault->smelt)		    {save_int("SMELT",		pi->smelt);}
		if (pi->glow		!= pDefault->glow)			{save_int("GLOW",		pi->glow);}
		if ((pi->glow_c1<<8) + pi->glow_c2)
			{save_int("GLOWBC", (pi->glow_c1<<8) + pi->glow_c2);}
		if (pi->glow_effect != pDefault->glow_effect)	{save_int("GLOWTYPE",	pi->glow_effect);}
		if (strlen(pi->desc)>0)							{save_str("DESC",		pi->desc);}	// save out our vendor description
		
		save_txt("}\n");
		save_sect();
	}

	if (pi->flags.isBeeingDragged == true)	// dragged items need to be saved at OLD position
	{
		swapDragInfo(pi);		// swap it back
	}
}

//o--------------------------------------------------------------------------
//|	Function		-	int CWorldMain::RemoveItemFromCharBody(int charserial, int type1, int type2);
//|	Date				-	Unknown
//|	Programmer	-	Abaddon
//o--------------------------------------------------------------------------
//|	Purpose			-	Remove the specified item from a characters paper doll
//o--------------------------------------------------------------------------
//|	Returns			-	true if removed, false otherwise
//o--------------------------------------------------------------------------
bool CWorldMain::RemoveItemsFromCharBody( int charserial, int type1, int type2 )
{ 
	int serial, ci, i;
	if (charserial<=-1) return false;
 	serial=chars[charserial].serial;
 	bool foundMatch = false;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for (ci=0;ci<vecContainer.size();ci++)
 	{
		P_ITEM pci = FindItemBySerial(vecContainer[ci]);

 		if (pci != NULL)
 		{
			if( ( pci->layer == 0x01 || pci->layer == 0x02 ) && ( pci->contserial == serial ) )
 			{
 				// in first hand, or second hand
 				if( pci->id1 == type1 && pci->id2 == type2 )
				{
 					Items->DeleItem( pci );
 					foundMatch = true;
 				}
 			}
 		}
	}
	return foundMatch;
}
