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

/////////////////////
// Name:	teleport.cpp
// Purpose: functions that handle the teleporting
// History:	cut from wolfpack.cpp by Duke, 26.10.00
// Remarks:	not necessarily ALL those functions
//

#include "telport.h"

#undef  DBGFILE
#define DBGFILE "teleport.cpp"

void teleporters(CHARACTER s)
{

	P_CHAR pc_s = MAKE_CHARREF_LR(s);

	multimap<int, tele_locations_st>::iterator iter_tele_locations;
	iter_tele_locations = tele_locations.find(pc_s->pos.x);
	
	while  (pc_s->pos.x == iter_tele_locations->second.origem.x && iter_tele_locations != tele_locations.end())
	{
		if(pc_s->pos.y == iter_tele_locations->second.origem.y)
		{
			if((iter_tele_locations->second.origem.z == illegal_z)||(pc_s->pos.z == iter_tele_locations->second.origem.z))
			{
				pc_s->MoveTo(iter_tele_locations->second.destination.x, iter_tele_locations->second.destination.y, iter_tele_locations->second.destination.z);
				teleport(pc_s);
				return;
			}
		}
		iter_tele_locations++;
	}
}

void read_in_teleport(void)
{
	FILE *fp;
	char text[256];
	char seps[]	= " ,\t\n";
	char *token;

	fp = fopen("teleport.scp","r");

	if(fp==NULL)
	{
		clConsole.send("ERROR: Teleport Data not found\n");
		error=1;
		keeprun=0;
		return;
	}

	while(!feof(fp))
	{
		fgets(text,255,fp);

		if(text[0]!=';')
		{
			tele_locations_st dummy;

			token = strtok( text, seps );
			
			dummy.origem.x = atoi(token);
			token = strtok( NULL, seps );
			dummy.origem.y = atoi(token);
			token = strtok( NULL, seps );
			if (token[0] == 'A')
				dummy.origem.z = illegal_z;
			else
				dummy.origem.z = atoi(token);

			token = strtok( NULL, seps );
			dummy.destination.x = atoi(token);
			token = strtok(NULL, seps );
			dummy.destination.y = atoi(token);
			token = strtok(NULL, seps);
			dummy.destination.z = atoi(token);

			tele_locations.insert(make_pair(dummy.origem.x, dummy));
			
		}
	}
	fclose(fp);
}

int validtelepos(int s)
{
	int z;

	z=-1;
	if ((currchar[s]->pos.x>=1397)&&(currchar[s]->pos.x<=1400)&&
		(currchar[s]->pos.y>=1622)&&(currchar[s]->pos.y<=1630))
		z=28;
	if ((currchar[s]->pos.x>=1510)&&(currchar[s]->pos.x<=1537)&&
		(currchar[s]->pos.y>=1455)&&(currchar[s]->pos.y<=1456))
		z=15;
	return z;
}

void advancementobjects(int s, int x, int allways)
{
	char sect[512];
	unsigned long loopexit=0;

	P_CHAR pc_s = MAKE_CHARREF_LR(s);

	int pos;
	if ((pc_s->advobj==0)||(allways==1))
	{
		staticeffect(s, 0x37, 0x3A, 0, 15);
		soundeffect2(pc_s, 0x01E9);
		pc_s->advobj=x;
		openscript("advance.scp");
		sprintf(sect, "ADVANCEMENT %i", x);
		if (!i_scripts[advance_script]->find(sect))
		{
			closescript();
			clConsole.send("ADVANCEMENT OBJECT: Script section not found. Aborting.\n");
			pc_s->advobj=0;
			return;
		}
		else			
			do
			{
				read2();
				if (script1[0]!='}')
				{
					if ((!(strcmp("STR",(char*)script1)))||(!(strcmp("STRENGTH",(char*)script1))))
					{
						pc_s->st= getstatskillvalue((char*)script2);
						pc_s->st2 = pc_s->st;
					}
					if ((!(strcmp("DEX",(char*)script1)))||(!(strcmp("DEXTERITY",(char*)script1))))
					{
						pc_s->setDex(getstatskillvalue((char*)script2));
					}
					if ((!(strcmp("INT",(char*)script1)))||(!(strcmp("INTELLIGENCE",(char*)script1))))
					{
						pc_s->in= getstatskillvalue((char*)script2);
						pc_s->in2 = pc_s->in;
					}
					if ((!(strcmp("ALCHEMY",(char*)script1)))||(!(strcmp("SKILL0",(char*)script1)))) pc_s->baseskill[ALCHEMY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("ANATOMY",(char*)script1)))||(!(strcmp("SKILL1",(char*)script1)))) pc_s->baseskill[ANATOMY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("ANIMALLORE",(char*)script1)))||(!(strcmp("SKILL2",(char*)script1)))) pc_s->baseskill[ANIMALLORE] = getstatskillvalue((char*)script2);
					if ((!(strcmp("ITEMID",(char*)script1)))||(!(strcmp("SKILL3",(char*)script1)))) pc_s->baseskill[ITEMID] = getstatskillvalue((char*)script2);
					if ((!(strcmp("ARMSLORE",(char*)script1)))||(!(strcmp("SKILL4",(char*)script1)))) pc_s->baseskill[ARMSLORE] = getstatskillvalue((char*)script2);
					if ((!(strcmp("PARRYING",(char*)script1)))||(!(strcmp("SKILL5",(char*)script1)))) pc_s->baseskill[PARRYING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("BEGGING",(char*)script1)))||(!(strcmp("SKILL6",(char*)script1)))) pc_s->baseskill[BEGGING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("BLACKSMITHING",(char*)script1)))||(!(strcmp("SKILL7",(char*)script1)))) pc_s->baseskill[BLACKSMITHING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("BOWCRAFT",(char*)script1)))||(!(strcmp("SKILL8",(char*)script1)))) pc_s->baseskill[BOWCRAFT] = getstatskillvalue((char*)script2);
					if ((!(strcmp("PEACEMAKING",(char*)script1)))||(!(strcmp("SKILL9",(char*)script1)))) pc_s->baseskill[PEACEMAKING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("CAMPING",(char*)script1)))||(!(strcmp("SKILL10",(char*)script1)))) pc_s->baseskill[CAMPING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("CARPENTRY",(char*)script1)))||(!(strcmp("SKILL11",(char*)script1)))) pc_s->baseskill[CARPENTRY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("CARTOGRAPHY",(char*)script1)))||(!(strcmp("SKILL12",(char*)script1)))) pc_s->baseskill[CARTOGRAPHY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("COOKING",(char*)script1)))||(!(strcmp("SKILL13",(char*)script1)))) pc_s->baseskill[COOKING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("DETECTINGHIDDEN",(char*)script1)))||(!(strcmp("SKILL14",(char*)script1)))) pc_s->baseskill[DETECTINGHIDDEN] = getstatskillvalue((char*)script2);
					if ((!(strcmp("ENTICEMENT",(char*)script1)))||(!(strcmp("SKILL15",(char*)script1)))) pc_s->baseskill[ENTICEMENT] = getstatskillvalue((char*)script2);
					if ((!(strcmp("EVALUATINGINTEL",(char*)script1)))||(!(strcmp("SKILL16",(char*)script1)))) pc_s->baseskill[EVALUATINGINTEL] = getstatskillvalue((char*)script2);
					if ((!(strcmp("HEALING",(char*)script1)))||(!(strcmp("SKILL17",(char*)script1)))) pc_s->baseskill[HEALING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("FISHING",(char*)script1)))||(!(strcmp("SKILL18",(char*)script1)))) pc_s->baseskill[FISHING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("FORENSICS",(char*)script1)))||(!(strcmp("SKILL19",(char*)script1)))) pc_s->baseskill[FORENSICS] = getstatskillvalue((char*)script2);
					if ((!(strcmp("HERDING",(char*)script1)))||(!(strcmp("SKILL20",(char*)script1)))) pc_s->baseskill[HERDING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("HIDING",(char*)script1)))||(!(strcmp("SKILL21",(char*)script1)))) pc_s->baseskill[HIDING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("PROVOCATION",(char*)script1)))||(!(strcmp("SKILL22",(char*)script1)))) pc_s->baseskill[PROVOCATION] = getstatskillvalue((char*)script2);
					if ((!(strcmp("INSCRIPTION",(char*)script1)))||(!(strcmp("SKILL23",(char*)script1)))) pc_s->baseskill[INSCRIPTION] = getstatskillvalue((char*)script2);
					if ((!(strcmp("LOCKPICKING",(char*)script1)))||(!(strcmp("SKILL24",(char*)script1)))) pc_s->baseskill[LOCKPICKING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("MAGERY",(char*)script1)))||(!(strcmp("SKILL25",(char*)script1)))) pc_s->baseskill[MAGERY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("MAGICRESISTANCE",(char*)script1)))||(!(strcmp("RESIST",(char*)script1)))||(!(strcmp("SKILL26",(char*)script1)))) pc_s->baseskill[MAGICRESISTANCE] = getstatskillvalue((char*)script2);
					if ((!(strcmp("TACTICS",(char*)script1)))||(!(strcmp("SKILL27",(char*)script1)))) pc_s->baseskill[TACTICS] = getstatskillvalue((char*)script2);
					if ((!(strcmp("SNOOPING",(char*)script1)))||(!(strcmp("SKILL28",(char*)script1)))) pc_s->baseskill[SNOOPING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("MUSICIANSHIP",(char*)script1)))||(!(strcmp("SKILL29",(char*)script1)))) pc_s->baseskill[MUSICIANSHIP] = getstatskillvalue((char*)script2);
					if ((!(strcmp("POISONING",(char*)script1)))||(!(strcmp("SKILL30",(char*)script1)))) pc_s->baseskill[POISONING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("ARCHERY",(char*)script1)))||(!(strcmp("SKILL31",(char*)script1)))) pc_s->baseskill[ARCHERY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("SPIRITSPEAK",(char*)script1)))||(!(strcmp("SKILL32",(char*)script1)))) pc_s->baseskill[SPIRITSPEAK] = getstatskillvalue((char*)script2);
					if ((!(strcmp("STEALING",(char*)script1)))||(!(strcmp("SKILL33",(char*)script1)))) pc_s->baseskill[STEALING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("TAILORING",(char*)script1)))||(!(strcmp("SKILL34",(char*)script1)))) pc_s->baseskill[TAILORING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("TAMING",(char*)script1)))||(!(strcmp("SKILL35",(char*)script1)))) pc_s->baseskill[TAMING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("TASTEID",(char*)script1)))||(!(strcmp("SKILL36",(char*)script1)))) pc_s->baseskill[TASTEID] = getstatskillvalue((char*)script2);
					if ((!(strcmp("TINKERING",(char*)script1)))||(!(strcmp("SKILL37",(char*)script1)))) pc_s->baseskill[TINKERING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("TRACKING",(char*)script1)))||(!(strcmp("SKILL38",(char*)script1)))) pc_s->baseskill[TRACKING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("VETERINARY",(char*)script1)))||(!(strcmp("SKILL39",(char*)script1)))) pc_s->baseskill[VETERINARY] = getstatskillvalue((char*)script2);
					if ((!(strcmp("SWORDSMANSHIP",(char*)script1)))||(!(strcmp("SKILL40",(char*)script1)))) pc_s->baseskill[SWORDSMANSHIP] = getstatskillvalue((char*)script2);
					if ((!(strcmp("MACEFIGHTING",(char*)script1)))||(!(strcmp("SKILL41",(char*)script1)))) pc_s->baseskill[MACEFIGHTING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("FENCING",(char*)script1)))||(!(strcmp("SKILL42",(char*)script1)))) pc_s->baseskill[FENCING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("WRESTLING",(char*)script1)))||(!(strcmp("SKILL43",(char*)script1)))) pc_s->baseskill[WRESTLING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("LUMBERJACKING",(char*)script1)))||(!(strcmp("SKILL44",(char*)script1)))) pc_s->baseskill[LUMBERJACKING] = getstatskillvalue((char*)script2);
					if ((!(strcmp("MINING",(char*)script1)))||(!(strcmp("SKILL45",(char*)script1)))) pc_s->baseskill[MINING] = getstatskillvalue((char*)script2);

					// lb, new skills
					if ((!(strcmp("MEDITATION",(char*)script1)))||(!(strcmp("SKILL46",(char*)script1)))) pc_s->baseskill[MEDITATION] = getstatskillvalue((char*)script2);
					if ((!(strcmp("STEALTH",(char*)script1)))||(!(strcmp("SKILL47",(char*)script1)))) pc_s->baseskill[STEALTH] = getstatskillvalue((char*)script2);
					if ((!(strcmp("REMOVETRAPS",(char*)script1)))||(!(strcmp("SKILL48",(char*)script1)))) pc_s->baseskill[REMOVETRAPS] = getstatskillvalue((char*)script2);

					if ((!(strcmp("DYEHAIR",(char*)script1))))
					{
						unsigned int ci;
						vector<SERIAL> vecContainer = contsp.getData(pc_s->serial);
						P_ITEM pi_hair = NULL;
						for (ci = 0; ci < vecContainer.size(); ci++)
						{
							P_ITEM pi_temp = FindItemBySerial(vecContainer[ci]);
							if (pi_temp != NULL)
								if (pi_temp->layer == 0x0B)
								{
									pi_hair = pi_temp;
									break;
								}
						}
						if (pi_hair != NULL)
						{
							x=hex2num(script2);
							pi_hair->color1=x>>8;
							pi_hair->color2=x%256;
							RefreshItem(pi_hair);//AntiChrist
							teleport(pc_s);
						}
					}

					if ((!(strcmp("DYEBEARD",(char*)script1))))
					{
						int ci;
						P_ITEM pi_beard = NULL;
						vector<SERIAL> vecContainer = contsp.getData(pc_s->serial);
						for (ci = 0; ci < vecContainer.size();ci++)
						{
							P_ITEM pi_temp = FindItemBySerial(vecContainer[ci]);
							if (pi_temp != NULL)
								if (pi_temp->layer == 0x10)
								{
									pi_beard = pi_temp;
									break;
								}
						}
						if (pi_beard != NULL)
						{
							x=hex2num(script2);
							pi_beard->color1=x>>8;
							pi_beard->color2=x%256;
							RefreshItem(pi_beard);//AntiChrist
							teleport(pc_s);
						}
					}

					if (!(strcmp("KILLHAIR",(char*)script1)))
					{
						unsigned int ci;
						vector<SERIAL> vecContainer = contsp.getData(pc_s->serial);
						for (ci=0;ci<vecContainer.size();ci++)
						{
							P_ITEM pi_temp = FindItemBySerial(vecContainer[ci]);
							if (pi_temp != NULL)
								if (pi_temp->layer == 0x0B)
								{
									Items->DeleItem(pi_temp);
									break;
								}
						}
					}

					if (!(strcmp("KILLBEARD",(char*)script1)))
					{
						unsigned int ci;
						vector<SERIAL> vecContainer = contsp.getData(pc_s->serial);
						for (ci=0;ci<vecContainer.size();ci++)
						{
							P_ITEM pi_temp = FindItemBySerial(vecContainer[ci]);
							if (pi_temp != NULL)
								if (pi_temp->layer==0x10)
								{
									Items->DeleItem(pi_temp);
									break;
								}
						}
					}

					if (!(strcmp("KILLPACK",(char*)script1)))
					{
						unsigned int ci;
						vector<SERIAL> vecContainer = contsp.getData(pc_s->serial);
						for (ci=0;ci<vecContainer.size();ci++)
						{
							P_ITEM pi = FindItemBySerial(vecContainer[ci]);
							if (pi != NULL)
								if (pi->layer==0x15)
								{
									Items->DeleItem(pi);
									break;
								}
						}
					}
					if (!(strcmp("ITEM",(char*)script1)))
					{
						x=str2num(script2);
						pos=ftell(scpfile);
						closescript();	/* lord binary */
						P_ITEM retitem = Targ->AddMenuTarget(-1, 0, x);
						openscript("advance.scp");
						fseek(scpfile, pos, SEEK_SET);
						strcpy((char*)script1, "DUMMY");
						P_ITEM pPack = Packitem(pc_s);
						if (retitem != NULL)
						{
							retitem->pos.x=50+(rand()%80);
							retitem->pos.y=50+(rand()%80);
							retitem->pos.z=9;
							if(retitem->layer==0x0b || retitem->layer==0x10)
							{
								retitem->SetContSerial(pc_s->serial);
							}
							else
							{
								if(pPack != NULL) 
									retitem->SetContSerial(pPack->serial);
							}
							RefreshItem(retitem);//AntiChrist
							teleport(pc_s);
						}
					}

					if (!(strcmp((char*)script1,"SKIN")))
					{
						pc_s->skin = pc_s->xskin = static_cast<UI16>(hex2num(script2));
						teleport(pc_s);
					}

					if (!(strcmp("POLY",(char*)script1)))
					{
						x=hex2num(script2);
						pc_s->id1=x>>8;
						pc_s->xid1=x>>8;
						pc_s->id2=x%256;
						pc_s->xid2=x%256;
						teleport(pc_s);
					}

					if (!(strcmp("ADVOBJ",(char*)script1)))
					{
						x=str2num(script2);
						pc_s->advobj=x;
					}
				}
			}
			while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
			closescript();
	}
	else sysmessage(calcSocketFromChar(pc_s),"You have already used an advancement object with this character.");
}

void monstergate(int s, int x)
{
	int tmp, z, lovalue, hivalue;
	char sect[512];
	
	P_CHAR pc_s = MAKE_CHARREF_LR(s);
	P_ITEM pBackpack = NULL;
	P_ITEM pRetitem = NULL;
	
	if (pc_s->isNpc()) return;
	
	sprintf(sect, "NPC %i", x);
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return;
	
	pc_s->title[0] = 0;
	
	AllItemsIterator iterItem;
	for(iterItem.Begin(); !iterItem.atEnd(); iterItem++)
	{
		P_ITEM pi = iterItem.GetData();
		if (pc_s->Wears(pi) &&
			pi->layer!=0x15 && pi->layer!=0x1D &&
			pi->layer!=0x10 && pi->layer!=0x0B && (!pi->free))
		{
			if (pBackpack == NULL)
			{
				pBackpack = Packitem(pc_s);
				
			}
			if (pBackpack == NULL)
			{
				scpMark m=pScp->Suspend();
				pBackpack = Items->SpawnItem(calcSocketFromChar(pc_s),pc_s,1,"#",0,0x0E,0x75,0,0,0,0);
				if (pBackpack == NULL)
					return;
				pc_s->packitem = pBackpack->serial;
				pScp->Resume(m);
				
				pBackpack->SetContSerial(pc_s->serial);
				pBackpack->layer=0x15;
				pBackpack->type=1;
				pBackpack->dye=1;
				pRetitem = pBackpack;
			}
			pi->pos.x = RandomNum(50, 130);
			pi->pos.y = RandomNum(50, 130);
			pi->pos.z=9;
			pi->SetContSerial(pBackpack->serial);
			pi->layer=0x00;
			
			SndRemoveitem(pi->serial);
			RefreshItem(pi);//AntiChrist
		}
		else if (pc_s->Wears(pi) &&
			(pi->layer == 0x0B || pi->layer == 0x10))
		{
			Items->DeleItem(pi);
		}
	}
	
	unsigned long loopexit=0;
	do
	{
		pScp->NextLineSplitted();
		
		if (script1[0]!='}')
		{
			if (!(strcmp("NAME",(char*)script1)))
			{
				strcpy(pc_s->name,(char*)script2);
			}
			if (!(strcmp("NAMELIST", (char*)script1)))
			{
				scpMark m=pScp->Suspend();
				setrandomname(s,(char*)script2);
				pScp->Resume(m);
				
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			if (!(strcmp("TITLE",(char*)script1))) strcpy(pc_s->title,(char*)script2);
			if (!(strcmp("KARMA",(char*)script1))) pc_s->karma=str2num(script2);
			if (!(strcmp("FAME",(char*)script1))) pc_s->fame=str2num(script2);
			if (!(strcmp("ID",(char*)script1)))
			{
				tmp=hex2num(script2);
				pc_s->id1=tmp>>8;
				pc_s->id2=tmp%256;
				pc_s->xid1=pc_s->id1;
				pc_s->xid2=pc_s->id2;
			}
			if (!(strcmp("SKIN",(char*)script1)))
			{
				pc_s->skin = pc_s->xskin = hex2num(script2);
			}
			
			if (!(strcmp("GOLD", (char*)script1)))
			{
				scpMark m=pScp->Suspend();
				pRetitem = Items->SpawnItem(calcSocketFromChar(pc_s),pc_s,1,"#",1,0x0E,0xED,0,0,1,0);
				if(pRetitem == NULL) return;
				pScp->Resume(m);
				
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
				gettokennum((char*)script2, 0);
				lovalue=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				hivalue=str2num(gettokenstr);
				if (hivalue==0)
				{
					if (lovalue/2!=0) pRetitem->amount=lovalue/2 + (rand()%(lovalue/2));
					else pRetitem->amount=lovalue/2;
				}
				else
				{
					if ((hivalue-lovalue)!=0) pRetitem->amount=lovalue + (rand()%(hivalue-lovalue));
					else pRetitem->amount=lovalue;
				}
				
			}
			if (!(strcmp("LOOT",(char*)script1)))
			{
				scpMark m=pScp->Suspend();
				pRetitem = Npcs->AddRandomLoot(pBackpack, (char*)script2);
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
			}
			if (!(strcmp("ITEM",(char*)script1)))
			{
				int storeval=str2num(script2);
				scpMark m=pScp->Suspend();
				pRetitem = Targ->AddMenuTarget(-1, 0, storeval);
				pScp->Resume(m);
				if (pRetitem == NULL)
				{
					pRetitem->SetContSerial(pc_s->serial);
					if (pRetitem->layer==0)
					{
						clConsole.send("Warning: Bad NPC Script %d with problem item %d executed!\n", x, storeval);
					}
				}
				strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
			}
			
			if (!(strcmp("PACKITEM",(char*)script1)))
			{
				int storeval=str2num(script2);
				scpMark m=pScp->Suspend();
				pRetitem = Targ->AddMenuTarget(-1, 0, storeval);
				pScp->Resume(m);
				if (pRetitem == NULL)
				{
					pRetitem->SetContSerial(pBackpack->serial);
					pRetitem->pos.x=50+(rand()%80);
					pRetitem->pos.y=50+(rand()%80);
					pRetitem->pos.z=9;
				}
				strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				
			}
			if (!(strcmp("COLOR",(char*)script1)))
			{
				if (pRetitem == NULL)
				{
					pRetitem->color1=(hex2num(script2))>>8;
					pRetitem->color2=(hex2num(script2))%256;
				}
			}
			if (!(strcmp("POISON",(char*)script1))) pc_s->poison=str2num(script2);
			
			//--------------------- NEW STAT & SKILL FORMAT ----------------
			//Handle Stats
			if ((!(strcmp("STR",(char*)script1)))||(!(strcmp("STRENGTH",(char*)script1))))
			{
				pc_s->st = getstatskillvalue((char*)script2);
				pc_s->st2 = pc_s->st;
				pc_s->hp = pc_s->st;
			}
			if ((!(strcmp("DEX",(char*)script1)))||(!(strcmp("DEXTERITY",(char*)script1))))
			{
				pc_s->setDex(getstatskillvalue((char*)script2));
				pc_s->stm = pc_s->realDex();
			}
			if ((!(strcmp("INT",(char*)script1)))||(!(strcmp("INTELLIGENCE",(char*)script1))))
			{
				pc_s->in = getstatskillvalue((char*)script2);
				pc_s->in2 = pc_s->in;
				pc_s->mn = pc_s->in;
			}
			//Done Handling Stats
			
			//Handle Skills
			if ((!(strcmp("ALCHEMY",(char*)script1)))||(!(strcmp("SKILL0",(char*)script1))))
				pc_s->baseskill[ALCHEMY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("ANATOMY",(char*)script1)))||(!(strcmp("SKILL1",(char*)script1))))
				pc_s->baseskill[ANATOMY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("ANIMALLORE",(char*)script1)))||(!(strcmp("SKILL2",(char*)script1))))
				pc_s->baseskill[ANIMALLORE] = getstatskillvalue((char*)script2);
			if ((!(strcmp("ITEMID",(char*)script1)))||(!(strcmp("SKILL3",(char*)script1))))
				pc_s->baseskill[ITEMID] = getstatskillvalue((char*)script2);
			if ((!(strcmp("ARMSLORE",(char*)script1)))||(!(strcmp("SKILL4",(char*)script1))))
				pc_s->baseskill[ARMSLORE] = getstatskillvalue((char*)script2);
			if ((!(strcmp("PARRYING",(char*)script1)))||(!(strcmp("SKILL5",(char*)script1))))
				pc_s->baseskill[PARRYING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("BEGGING",(char*)script1)))||(!(strcmp("SKILL6",(char*)script1))))
				pc_s->baseskill[BEGGING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("BLACKSMITHING",(char*)script1)))||(!(strcmp("SKILL7",(char*)script1))))
				pc_s->baseskill[BLACKSMITHING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("BOWCRAFT",(char*)script1)))||(!(strcmp("SKILL8",(char*)script1))))
				pc_s->baseskill[BOWCRAFT] = getstatskillvalue((char*)script2);
			if ((!(strcmp("PEACEMAKING",(char*)script1)))||(!(strcmp("SKILL9",(char*)script1))))
				pc_s->baseskill[PEACEMAKING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("CAMPING",(char*)script1)))||(!(strcmp("SKILL10",(char*)script1))))
				pc_s->baseskill[CAMPING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("CARPENTRY",(char*)script1)))||(!(strcmp("SKILL11",(char*)script1))))
				pc_s->baseskill[CARPENTRY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("CARTOGRAPHY",(char*)script1)))||(!(strcmp("SKILL12",(char*)script1))))
				pc_s->baseskill[CARTOGRAPHY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("COOKING",(char*)script1)))||(!(strcmp("SKILL13",(char*)script1))))
				pc_s->baseskill[COOKING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("DETECTINGHIDDEN",(char*)script1)))||(!(strcmp("SKILL14",(char*)script1))))
				pc_s->baseskill[DETECTINGHIDDEN] = getstatskillvalue((char*)script2);
			if ((!(strcmp("ENTICEMENT",(char*)script1)))||(!(strcmp("SKILL15",(char*)script1))))
				pc_s->baseskill[ENTICEMENT] = getstatskillvalue((char*)script2);
			if ((!(strcmp("EVALUATINGINTEL",(char*)script1)))||(!(strcmp("SKILL16",(char*)script1))))
				pc_s->baseskill[EVALUATINGINTEL] = getstatskillvalue((char*)script2);
			if ((!(strcmp("HEALING",(char*)script1)))||(!(strcmp("SKILL17",(char*)script1))))
				pc_s->baseskill[HEALING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("FISHING",(char*)script1)))||(!(strcmp("SKILL18",(char*)script1))))
				pc_s->baseskill[FISHING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("FORENSICS",(char*)script1)))||(!(strcmp("SKILL19",(char*)script1))))
				pc_s->baseskill[FORENSICS] = getstatskillvalue((char*)script2);
			if ((!(strcmp("HERDING",(char*)script1)))||(!(strcmp("SKILL20",(char*)script1))))
				pc_s->baseskill[HERDING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("HIDING",(char*)script1)))||(!(strcmp("SKILL21",(char*)script1))))
				pc_s->baseskill[HIDING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("PROVOCATION",(char*)script1)))||(!(strcmp("SKILL22",(char*)script1))))
				pc_s->baseskill[PROVOCATION] = getstatskillvalue((char*)script2);
			if ((!(strcmp("INSCRIPTION",(char*)script1)))||(!(strcmp("SKILL23",(char*)script1))))
				pc_s->baseskill[INSCRIPTION] = getstatskillvalue((char*)script2);
			if ((!(strcmp("LOCKPICKING",(char*)script1)))||(!(strcmp("SKILL24",(char*)script1))))
				pc_s->baseskill[LOCKPICKING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("MAGERY",(char*)script1)))||(!(strcmp("SKILL25",(char*)script1))))
				pc_s->baseskill[MAGERY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("MAGICRESISTANCE",(char*)script1)))||(!(strcmp("RESIST",(char*)script1)))||(!(strcmp("SKILL26",(char*)script1))))
				pc_s->baseskill[MAGICRESISTANCE] = getstatskillvalue((char*)script2);
			if ((!(strcmp("TACTICS",(char*)script1)))||(!(strcmp("SKILL27",(char*)script1))))
				pc_s->baseskill[TACTICS] = getstatskillvalue((char*)script2);
			if ((!(strcmp("SNOOPING",(char*)script1)))||(!(strcmp("SKILL28",(char*)script1))))
				pc_s->baseskill[SNOOPING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("MUSICIANSHIP",(char*)script1)))||(!(strcmp("SKILL29",(char*)script1))))
				pc_s->baseskill[MUSICIANSHIP] = getstatskillvalue((char*)script2);
			if ((!(strcmp("POISONING",(char*)script1)))||(!(strcmp("SKILL30",(char*)script1))))
				pc_s->baseskill[POISONING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("ARCHERY",(char*)script1)))||(!(strcmp("SKILL31",(char*)script1))))
				pc_s->baseskill[ARCHERY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("SPIRITSPEAK",(char*)script1)))||(!(strcmp("SKILL32",(char*)script1))))
				pc_s->baseskill[SPIRITSPEAK] = getstatskillvalue((char*)script2);
			if ((!(strcmp("STEALING",(char*)script1)))||(!(strcmp("SKILL33",(char*)script1))))
				pc_s->baseskill[STEALING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("TAILORING",(char*)script1)))||(!(strcmp("SKILL34",(char*)script1))))
				pc_s->baseskill[TAILORING] = getstatskillvalue((char*)script2);
			// Taming skill disabled for now for npcs - as they wont be training people
			// nor taming things on their own for a while. - Eventually.. when they can
			// train players how the taming skill, they themselves will need it. Then
			// it will require use of the "TOTAME" property outlined above.
			// if ((!(strcmp("TAMING",script1)))||(!(strcmp("SKILL35",script1))))
			//		pc_s->baseskill[TAMING] = getstatskillvalue(script2);
			if ((!(strcmp("TASTEID",(char*)script1)))||(!(strcmp("SKILL36",(char*)script1))))
				pc_s->baseskill[TASTEID] = getstatskillvalue((char*)script2);
			if ((!(strcmp("TINKERING",(char*)script1)))||(!(strcmp("SKILL37",(char*)script1))))
				pc_s->baseskill[TINKERING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("TRACKING",(char*)script1)))||(!(strcmp("SKILL38",(char*)script1))))
				pc_s->baseskill[TRACKING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("VETERINARY",(char*)script1)))||(!(strcmp("SKILL39",(char*)script1))))
				pc_s->baseskill[VETERINARY] = getstatskillvalue((char*)script2);
			if ((!(strcmp("SWORDSMANSHIP",(char*)script1)))||(!(strcmp("SKILL40",(char*)script1))))
				pc_s->baseskill[SWORDSMANSHIP] = getstatskillvalue((char*)script2);
			if ((!(strcmp("MACEFIGHTING",(char*)script1)))||(!(strcmp("SKILL41",(char*)script1))))
				pc_s->baseskill[MACEFIGHTING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("FENCING",(char*)script1)))||(!(strcmp("SKILL42",(char*)script1))))
				pc_s->baseskill[FENCING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("WRESTLING",(char*)script1)))||(!(strcmp("SKILL43",(char*)script1))))
				pc_s->baseskill[WRESTLING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("LUMBERJACKING",(char*)script1)))||(!(strcmp("SKILL44",(char*)script1))))
				pc_s->baseskill[LUMBERJACKING] = getstatskillvalue((char*)script2);
			if ((!(strcmp("MINING",(char*)script1)))||(!(strcmp("SKILL45",(char*)script1))))
				pc_s->baseskill[MINING] = getstatskillvalue((char*)script2);
			
			// lb, new skills
			if ((!(strcmp("MEDITATION",(char*)script1)))||(!(strcmp("SKILL46",(char*)script1))))
				pc_s->baseskill[MEDITATION] = getstatskillvalue((char*)script2);
			if ((!(strcmp("STEALTH",(char*)script1)))||(!(strcmp("SKILL47",(char*)script1))))
				pc_s->baseskill[STEALTH] = getstatskillvalue((char*)script2);
			if ((!(strcmp("REMOVETRAPS",(char*)script1)))||(!(strcmp("SKILL48",(char*)script1))))
				pc_s->baseskill[REMOVETRAPS] = getstatskillvalue((char*)script2);
			//Done Handling Skills
			
			//Handle Extras
			if ((!(strcmp("DAMAGE",(char*)script1)))||(!(strcmp("ATT",(char*)script1))))
			{
				gettokennum((char*)script2, 0);
				lovalue=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				hivalue=str2num(gettokenstr);
				pc_s->lodamage = lovalue;
				pc_s->hidamage = lovalue;
				if(hivalue) pc_s->hidamage = hivalue;
			}
			if (!(strcmp("DEF",(char*)script1))) pc_s->def = getstatskillvalue((char*)script2);
			//Done Handling Extras
			
			//Handle Obsolete Stuff
			if (!(strcmp("LODAMAGE",(char*)script1))) pc_s->lodamage=str2num(script2);
			if (!(strcmp("HIDAMAGE",(char*)script1))) pc_s->hidamage=str2num(script2);
			if (!(strcmp("SKILL", (char*)script1)))
			{
				gettokennum((char*)script2, 0);
				z=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				pc_s->baseskill[z]=str2num(gettokenstr);
			}
			//Done Handling Obsolete Stuff
	}
	//--------------- DONE NEW STAT & SKILL FORMAT ---------------------
 }
 while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
 pScp->Close();
 
 //Now find real 'skill' based on 'baseskill' (stat modifiers)
 for(z=0;z<TRUESKILLS;z++)
 {
	 Skills->updateSkillLevel(s,z);
 }
 updatechar(pc_s);
 staticeffect(s, 0x37, 0x3A, 0, 15);
 soundeffect2(pc_s, 0x01E9);
}

// new function 
////////////////////////////////// 
// This will be just a body type switching 
// and coloring item. 
// Uses the integer number of the monster type 
// from the npc file. 
// Gate has to be of type 84. 
// 
// 
void polycolorgate(int s, int x) 
{ 
	int tmp; 
	char sect[256]; 

	P_CHAR pc_s = MAKE_CHARREF_LR(s);

	if (pc_s->isNpc())
		return; 

	sprintf(sect, "NPC %i", x); 
	Script *pScpBase = i_scripts[npc_script]; 
	Script *pScp = pScpBase->Select(sect, custom_npc_script); 

	if (!pScp)
		return; 

	unsigned long loopexit = 0; 
	do 
	{ 
		pScp->NextLineSplitted(); 
		if (script1[0] != '}') 
		{ 
			if (!(strcmp("ID", (char*)script1))) 
			{ 
				tmp = hex2num(script2); 
				pc_s->id1 = tmp >> 8; 
				pc_s->id2 = tmp%256; 
				pc_s->xid1 = pc_s->id1; 
				pc_s->xid2 = pc_s->id2; 
			} 
			else if (!(strcmp("SKIN", (char*)script1))) 
			{ 
				pc_s->skin = pc_s->xskin = static_cast<UI16>(hex2num(script2));
			} 
		} 
	} 
	while ((script1[0] != '}') &&(++loopexit < MAXLOOPS)); 
	pScp->Close(); 
  
	updatechar(pc_s); 
	staticeffect(s, 0x37, 0x3A, 0, 15); 
	soundeffect2(pc_s, 0x01E9); 
} 
// 
// 
// Aldur 
////////////////////////////////// 

void objTeleporters(int s)
{
	P_CHAR pc_s = MAKE_CHARREF_LR(s);

	int x = pc_s->pos.x, y = pc_s->pos.y;
	
	int	StartGrid = mapRegions->StartGrid(pc_s->pos.x, pc_s->pos.y);
	//	int	getcell=mapRegions->GetCell(pc_s->x,pc_s->y);
	
	unsigned int increment = 0;
	for (unsigned int checkgrid = StartGrid + (increment*mapRegions->GetColSize()); increment < 3; increment++, checkgrid = StartGrid + (increment*mapRegions->GetColSize()))
	{
		for (int a = 0; a < 3; a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid + a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				P_ITEM pmi = FindItemBySerial(vecEntries[k]);
				if (pmi != NULL)
				{
					if (pmi->pos.x == x && pmi->pos.y == y &&
						((abs(pmi->pos.z) + 10) >= abs(pc_s->pos.z)) &&((abs(pmi->pos.z) - 10) <= abs(pc_s->pos.z)))
					{
						if ((pmi->type == 60) && (pmi->morex + pmi->morey + pmi->morez >0))
						{
							pc_s->MoveTo(pmi->morex,pmi->morey,pmi->morez);
							teleport(pc_s);
						}
						
						// advancement objects
						if ((pmi->type == 80)&&(pc_s->isPlayer()))
							if (pmi->more1 != 0 || pmi->more2 != 0 || pmi->more3 != 0 || pmi->more4 != 0)
							{
								if (pc_s->ser1 == pmi->more1 && pc_s->ser2 == pmi->more2 && pc_s->ser3 == pmi->more3 && pc_s->ser4 == pmi->more4)
									advancementobjects(s, pmi->morex, 0);
							}
							else
								advancementobjects(s, pmi->morex, 0);
							
							if ((pmi->type == 81)&&(pc_s->isPlayer()))
								if (pmi->more1 != 0 || pmi->more2 != 0 || pmi->more3 != 0 || pmi->more4 != 0)
								{
									if (pc_s->ser1 == pmi->more1 && pc_s->ser2 == pmi->more2 && pc_s->ser3 == pmi->more3 && pc_s->ser4 == pmi->more4)
										advancementobjects(s, pmi->morex, 1);
								}
								else
									advancementobjects(s, pmi->morex, 1);
								// The above code lets you restrict a gate's use by setting its MORE values to a char's
								// serial #
								
								// damage objects
								if (!(pc_s->isInvul()) && (pmi->type == 85))
								{
									pc_s->hp = pc_s->hp - (pmi->morex + RandomNum(pmi->morey, pmi->morez));
									if (pc_s->hp < 1)
										pc_s->hp = 0;
									updatestats(pc_s, 0);
									if (pc_s->hp <= 0)
										deathstuff(DEREF_P_CHAR(pc_s));
								}
								// monster gates
								if (pmi->type == 82)
									monstergate(s, pmi->morex);
								////////////////////////////////// 
								// This will be just a body type switching 
								// item. 
								// 
								// 
								// 
								if (pmi->type == 84) 
									polycolorgate(s, pmi->morex); 
								// 
								// 
								// Aldur 
								//////////////////////////////////
								
								// sound objects
								if (pmi->type == 86)
								{
									if (RandomNum(1, 100) <= pmi->morez)
										soundeffect3(pmi, (pmi->morex << 8) + pmi->morey);
								}

								if(pmi->type == 88)
								{
									Races.SetRace(pc_s,pmi->morex);
								}
					}
				}
			}
		}
	}
}

