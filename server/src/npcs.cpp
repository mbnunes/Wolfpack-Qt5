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
#include "sregions.h"
#include "SndPkg.h"
#include "debug.h"
#include "utilsys.h"

#undef  DBGFILE
#define DBGFILE "npcs.cpp"

#define CHAR_RESERVE 100	// minimum of free slots that should be left in the array.
							// otherwise, more memory will be allocated in the mainloop (Duke)

int addrandomcolor(P_CHAR pc_s, char *colorlist)
{
	char sect[512];
	int i,j,storeval;
	i=0; j=0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();

		//sprintf(pc_s->name, "Error Colorlist %s Not Found(1)", colorlist);
		// LB: wtf should this do apart from crashing ? copying an error message in a chars name ??
		// very weired! think it should look like this:

		clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);

		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					storeval=hex2num(script1);
				}
			}
		}
		while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (storeval);
}

static int addrandomhaircolor(P_CHAR pc_s, char *colorlist)
{
	char sect[512];
	int i,j,haircolor = 0x044e;
	i=0; j=0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();
		clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);
		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					haircolor=hex2num(script1);
				}
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (haircolor);
}

void setrandomname(P_CHAR pc_s, char * namelist)
{
	char sect[512];
	int i=0,j=0;

	sprintf(sect, "RANDOMNAME %s", namelist);
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp)
	{
		sprintf(pc_s->name, "Error Namelist %s Not Found", namelist);
		return;
	}

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);
		if (!pScp) return;

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					strcpy(pc_s->name,(char*)script1);
					break;
				}
				else j++;
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
}

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	int j;//,serial; //Zippy lag
	//int ptr,ci;

	removeitem[1]=pc_k->ser1;
	removeitem[2]=pc_k->ser2;
	removeitem[3]=pc_k->ser3;
	removeitem[4]=pc_k->ser4;

	if (pc_k->spawnregion>0 && pc_k->spawnregion<255)
	{
		spawnregion[pc_k->spawnregion].current--;
	}


	if (pc_k->spawnserial != INVALID_SERIAL) 
		cspawnsp.remove(pc_k->spawnserial, pc_k->serial);
	if (pc_k->ownserial != INVALID_SERIAL) 
		cownsp.remove(pc_k->ownserial, pc_k->serial);
	
	for (j=0;j<now;j++)
	{
		if (perm[j]) 
			Xsend(j, removeitem, 5);		
	}
	
	if (pc_k != NULL) 
		mapRegions->Remove(pc_k); // taking it out of mapregions BEFORE x,y changed, LB
	
	pc_k->free = true;
	cCharsManager::getCharsManager().unregisterChar( pc_k );
	delete pc_k;
	pc_k = NULL;
}

P_CHAR cCharStuff::MemCharFree()			// Find a free char slot
{
	P_CHAR pc = new cChar;
	return pc;
}

P_ITEM cCharStuff::AddRandomLoot(P_ITEM pBackpack, char * lootlist)
{
	char sect[512];
	int i,j, storeval,loopexit=0;
	P_ITEM retitem = NULL;
	storeval=-1;
	i=0; j=0;

	sprintf(sect, "LOOTLIST %s", lootlist);
	
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return NULL;

	loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++; // Count number of entries on list.
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					storeval=str2num(script1);	//script1 = ITEM#

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if(retitem!=NULL)
					{
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						retitem->SetContSerial(pBackpack->serial);
					}
					break;;    
				}
				else j++;
			}
		}	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
	return retitem;
}

/*** s: socket ***/
P_CHAR cCharStuff::AddRandomNPC(int s, char * npclist, int spawnpoint)
{
	//This function gets the random npc number from the list and recalls
	//addrespawnnpc passing the new number
	char sect[512];
	unsigned int uiTempList[100];
	int i=0,k=0;
	sprintf(sect, "NPCLIST %s", npclist);

	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return NULL;

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			uiTempList[i]=str2num(script1);
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS));
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		k=uiTempList[i];
	}
	if(k!=0)
	{
		if (spawnpoint==-1)
		{
			addmitem[s]=k;
			return Targ->NpcMenuTarget(s);
			//return -1;
		}
	}
	return NULL;
}

//o---------------------------------------------------------------------------o
//| Function   : AddNPC (3 interfaces)
//| Programmer : Duke, 23.05.2000
//o---------------------------------------------------------------------------o
//| Purpose    : creates the scripted NPC given by npcNum
//|              The position of the NPC can be given in three different ways:
//|				 1. by parms x1 y1 z1 (trigger)
//|				 2. by passing a socket (GM add)
//|				 3. by passing an item index (spawn rune)
//|
//| Remarks    : This function was created from the former AddRespawnNPC() and
//|				 AddNPCxyz() that were 95% identical
//o---------------------------------------------------------------------------o
/*
int cCharStuff::AddRespawnNPC(int s, int npcNum, int type)
{
	if (type == 1)
		return AddNPC(-1, s, npcNum, 0,0,0);	// 's' is an item index
	else
		return AddNPC(s, NULL, npcNum, 0,0,0);	// 's' is a socket
}*/
P_CHAR cCharStuff::AddNPCxyz(int s, int npcNum, int type, int x1, int y1, signed char z1) //Morrolan - replacement for old Npcs->AddNPCxyz(), fixes a LOT of problems.
{
	if (type == 0)
		return AddNPC(s, NULL, npcNum, x1,y1,z1);	// 's' maybe(!) is a socket
	if (type == 1)
		clConsole.send("ERROR: type == 1 not supported!\n");
	return NULL;
}

P_CHAR cCharStuff::AddNPC(int s, P_ITEM pi_i, int npcNum, int x1, int y1, signed char z1)
{
	int tmp, z,c, lovalue, hivalue;
	int k=0, xos=0, yos=0, lb;
	char sect[512];
	int haircolor = -1; //(we need this to remember the haircolor)
	short postype;				// determines how xyz of the new NPC are set, see below
	short fx1,fx2,fy1,fy2,fz1;	// temp. hold the rectangle or circle for npcwander from script
	fx1=fx2=fy1=fy2=fz1=0;

	if (x1 > 0 && y1 > 0)
 		postype = 3;	// take position from parms
	else if ( s > -1 && pi_i == NULL)
		postype = 2;	// take position from socket's buffer
	else if ( s == -1 && pi_i != NULL)
		postype = 1;	// take position from items[i]
	else
	{
		clConsole.send("ERROR: bad parms in call to AddNPC\n");
		return NULL;
	}

	P_ITEM pBackpack = NULL;
	P_ITEM retitem = NULL;
	P_ITEM shoppack1 = NULL;
	P_ITEM shoppack2 = NULL;
	P_ITEM shoppack3 = NULL;
	//
	// First things first...lets find out what NPC# we should spawn
	//
	Script *pScpBase=i_scripts[npc_script];
	sprintf(sect, "NPC %i", npcNum);
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return NULL;
	
	unsigned long loopexit=0;
	do
	{
		pScp->NextLineSplitted();
		if (script1[0]!='}')
		{
			if (!(strcmp("NPCLIST", (char*)script1)))
			{
				pScp->Close();
				return Npcs->AddRandomNPC(s,(char*)script2,1);
			}
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	pScp->Close();
	
	//
	// Now lets spawn him/her
	//
	P_CHAR pc_c = Npcs->MemCharFree ();
	if ( pc_c == NULL )
		return NULL;
	pc_c->Init();
	

	pc_c->setPriv(0x10);
	pc_c->npc=1;
	pc_c->att=1;
	pc_c->def=1;
	pc_c->spawnserial=-1;
	
	pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp)
	{
		Npcs->DeleteChar(pc_c);
		return NULL;
	}

	loopexit=0;
	do
	{
		pScp->NextLineSplitted();

		if (script1[0]!='}') {
			switch(script1[0])
			{
			case 'A':
			case 'a':
			if (!strcmp("ALCHEMY",(char*)script1))		pc_c->baseskill[ALCHEMY] = getstatskillvalue((char*)script2);
			else if (!strcmp("ANATOMY",(char*)script1))		pc_c->baseskill[ANATOMY] = getstatskillvalue((char*)script2);
			else if (!strcmp("ARCHERY",(char*)script1))		pc_c->baseskill[ARCHERY] = getstatskillvalue((char*)script2);
			else if (!strcmp("ARMSLORE",(char*)script1))		pc_c->baseskill[ARMSLORE] = getstatskillvalue((char*)script2);
			else if (!strcmp("ANIMALLORE",(char*)script1))	pc_c->baseskill[ANIMALLORE] = getstatskillvalue((char*)script2);
			break;

			case 'B':
			case 'b':
			if (!strcmp("BACKPACK", (char*)script1))
			{
				if (pBackpack == NULL)
				{
					scpMark m=pScp->Suspend();
					pBackpack = Items->SpawnItem(-1, pc_c,1,"Backpack",0,0x0E,0x75,0,0,0,0);
					if(pBackpack == NULL)
					{
						Npcs->DeleteChar(pc_c);
						return NULL;
					}
					pc_c->packitem = pBackpack->serial;
					pBackpack->pos.x=0;
					pBackpack->pos.y=0;
					pBackpack->pos.z=0;
					pBackpack->SetContSerial(pc_c->serial);
					pBackpack->layer=0x15;
					pBackpack->type=1;
					pBackpack->dye=1;
					
					retitem = pBackpack;
					pScp->Resume(m);
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				}
			}
			else if (!strcmp("BEGGING",(char*)script1))			pc_c->baseskill[BEGGING] = getstatskillvalue((char*)script2);
			else if (!strcmp("BLACKSMITHING",(char*)script1))	pc_c->baseskill[BLACKSMITHING] = getstatskillvalue((char*)script2);
			else if (!strcmp("BOWCRAFT",(char*)script1))			pc_c->baseskill[BOWCRAFT] = getstatskillvalue((char*)script2);
			break;

			case 'C':
			case 'c':

			if (!strcmp("COLOR",(char*)script1)) {
				if (retitem != NULL)
				{
					retitem->color1=(hex2num(script2))>>8;
					retitem->color2=(hex2num(script2))%256;
				}
			}
			else if (!strcmp("CARVE",(char*)script1)) pc_c->carve=str2num(script2);
			else if (!strcmp("CAMPING",(char*)script1)) pc_c->baseskill[CAMPING] = getstatskillvalue((char*)script2);
			else if (!strcmp("CARPENTRY",(char*)script1)) pc_c->baseskill[CARPENTRY] = getstatskillvalue((char*)script2);
			else if (!strcmp("CARTOGRAPHY",(char*)script1)) pc_c->baseskill[CARTOGRAPHY] = getstatskillvalue((char*)script2);
			else if (!strcmp("CANTRAIN",(char*)script1)) pc_c->cantrain=true;
			else if (!strcmp("COOKING",(char*)script1)) pc_c->baseskill[COOKING] = getstatskillvalue((char*)script2);
			else if (!strcmp("COLORMATCHHAIR",(char*)script1))
			{
				if (retitem != NULL && haircolor!=-1)
				{
					retitem->color1=(haircolor)>>8;
					retitem->color2=(haircolor)%256;
				}
			}
			else if (!strcmp("COLORLIST",(char*)script1))
			{
				scpMark m=pScp->Suspend();
				int storeval = addrandomcolor(pc_c, (char*)script2);
				if (retitem != NULL)
				{
					retitem->color1=(storeval)>>8;
					retitem->color2=(storeval)%256;
				}
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			break;

			case 'D':
			case 'd':

			if (!strcmp("DIRECTION",(char*)script1)) {
				if (!strcmp("NE",(char*)script2)) pc_c->dir=1;
				else if (!strcmp("E",(char*)script2)) pc_c->dir=2;
				else if (!strcmp("SE",(char*)script2)) pc_c->dir=3;
				else if (!strcmp("S",(char*)script2)) pc_c->dir=4;
				else if (!strcmp("SW",(char*)script2)) pc_c->dir=5;
				else if (!strcmp("W",(char*)script2)) pc_c->dir=6;
				else if (!strcmp("NW",(char*)script2)) pc_c->dir=7;
				else if (!strcmp("N",(char*)script2)) pc_c->dir=0;
			}
			else if (!strcmp("DEX",(char*)script1) || !strcmp("DEXTERITY",(char*)script1)) 
			{
				pc_c->setDex(getstatskillvalue((char*)script2));
				pc_c->stm = pc_c->realDex();
			}
			else if (!strcmp("DEF",(char*)script1)) pc_c->def = getstatskillvalue((char*)script2);
			else if (!strcmp("DETECTINGHIDDEN",(char*)script1)) pc_c->baseskill[DETECTINGHIDDEN] = getstatskillvalue((char*)script2);
			else if (!strcmp("DAMAGE",(char*)script1) || !strcmp("ATT",(char*)script1)) {
				gettokennum((char*)script2, 0);
				lovalue=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				hivalue=str2num(gettokenstr);
				pc_c->lodamage = lovalue;
				pc_c->hidamage = lovalue;
				if(hivalue) {
					pc_c->hidamage = hivalue;
				}
			}
			break;

			case 'E':
			case 'e':

			if (!(strcmp("EMOTECOLOR",(char*)script1))) {
				pc_c->emotecolor1=(hex2num(script2))>>8;
				pc_c->emotecolor2=(hex2num(script2))%256;
			}
			else if (!strcmp("ENTICEMENT",(char*)script1)) pc_c->baseskill[ENTICEMENT] = getstatskillvalue((char*)script2);
			else if (!strcmp("EVALUATINGINTEL",(char*)script1)) pc_c->baseskill[EVALUATINGINTEL] = getstatskillvalue((char*)script2);
			break;

			case 'F':
			case 'f':

			if (!strcmp("FISHING",(char*)script1)) pc_c->baseskill[FISHING] = getstatskillvalue((char*)script2);
			else if (!strcmp("FORENSICS",(char*)script1)) pc_c->baseskill[FORENSICS] = getstatskillvalue((char*)script2);
			else if (!strcmp("FX1",(char*)script1)) fx1=str2num(script2);  // new NPCWANDER implementation
			else if (!strcmp("FX2",(char*)script1)) fx2=str2num(script2);
			else if (!strcmp("FLEEAT",(char*)script1)) pc_c->fleeat=str2num(script2);
			else if (!strcmp("FAME",(char*)script1)) pc_c->fame=str2num(script2);
			else if (!strcmp("FENCING",(char*)script1)) pc_c->baseskill[FENCING] = getstatskillvalue((char*)script2);
			else if (!strcmp("FY1",(char*)script1)) fy1=str2num(script2);
			else if (!strcmp("FY2",(char*)script1)) fy2=str2num(script2);
			else if (!strcmp("FZ1",(char*)script1)) fz1=str2num(script2);
			break;

			case 'G':
			case 'g':

			if (!strcmp("GOLD", (char*)script1))
			{
				if (pBackpack != NULL)
				{ 
					scpMark m=pScp->Suspend();
					P_ITEM pGold = Items->SpawnItem(pc_c,1,"#",1,0x0EED,0,1);
					if(!pGold)
					{
						Npcs->DeleteChar(pc_c);
						return NULL;
					}
					pScp->Resume(m);

					pGold->priv|=0x01;
					gettokennum((char*)script2, 0);
					lovalue=str2num(gettokenstr);
					gettokennum((char*)script2, 1);
					hivalue=str2num(gettokenstr);
					if (hivalue==0)
					{
						if (lovalue/2!=0) pGold->amount=lovalue/2 + (rand()%(lovalue/2));
						else pGold->amount=0;
					} else
					{
						if (hivalue-lovalue!=0) pGold->amount=lovalue + (rand()%(hivalue-lovalue));
						else pGold->amount=lovalue;
					}
				}
				else
					clConsole.send("Warning: Bad NPC Script %d with problem no backpack for gold.\n", npcNum);
			}
			break;

			case 'H':
			case 'h':

			if (!strcmp("HEALING",(char*)script1)) pc_c->baseskill[HEALING] = getstatskillvalue((char*)script2);
			else if (!strcmp("HIDAMAGE",(char*)script1)) pc_c->hidamage=str2num(script2);
			else if (!strcmp("HERDING",(char*)script1)) pc_c->baseskill[HERDING] = getstatskillvalue((char*)script2);
			else if (!strcmp("HIDING",(char*)script1)) pc_c->baseskill[HIDING] = getstatskillvalue((char*)script2);
			else if (!strcmp("HAIRCOLOR",(char*)script1))
			{
				scpMark m=pScp->Suspend();
				
				if (retitem != NULL) // LB, ouple of bugfixes
				{
					haircolor=addrandomhaircolor(pc_c,(char*)script2);
					if (haircolor!=-1)
					{
						retitem->color1=(haircolor)>>8;
						retitem->color2=(haircolor)%256;
					}
				}
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			break;

			case 'I':
			case 'i':

			if (!strcmp("ID",(char*)script1)) 
			{
				tmp=hex2num(script2);
				pc_c->id1=tmp>>8;
				pc_c->id2=tmp%256;
				pc_c->xid1=pc_c->id1;
				pc_c->xid2=pc_c->id2;
			}
			else if (!strcmp("ITEM",(char*)script1)) 
			{
				int storeval=str2num(script2);

				scpMark m=pScp->Suspend();
				retitem = Targ->AddMenuTarget(-1, 0, storeval);
				pScp->Resume(m);

				if ( retitem != NULL )
				{
					retitem->SetContSerial(pc_c->serial);
					if (retitem->layer==0) {
						clConsole.send("Warning: Bad NPC Script %d with problem item %d executed!\n", npcNum, storeval);
					}
				}
				strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
			}
			else if (!strcmp("INT",(char*)script1) || !strcmp("INTELLIGENCE",(char*)script1)) {
				pc_c->in  = getstatskillvalue((char*)script2);
				pc_c->in2 = pc_c->in;
				pc_c->mn  = pc_c->in;
			}
			//Done Handling Stats
			//Handle Skills
			else if (!strcmp("ITEMID",(char*)script1)) pc_c->baseskill[ITEMID] = getstatskillvalue((char*)script2);
			else if (!strcmp("INSCRIPTION",(char*)script1)) pc_c->baseskill[INSCRIPTION] = getstatskillvalue((char*)script2);
			break;
			
			case 'K':
			case 'k':
			if (!strcmp("KARMA",(char*)script1)) pc_c->karma=str2num(script2);
			break;

			case 'L':
			case 'l':

			if (!strcmp("LOOT",(char*)script1))
			{
				if (pBackpack != NULL)
				{
					scpMark m=pScp->Suspend();
					retitem = Npcs->AddRandomLoot(pBackpack, script2);
					pScp->Resume(m);

					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no backpack for loot.\n", npcNum);
				}
			}
			else if (!strcmp("LODAMAGE",(char*)script1)) pc_c->lodamage=str2num(script2);
			else if (!strcmp("LUMBERJACKING",(char*)script1)) pc_c->baseskill[LUMBERJACKING] = getstatskillvalue((char*)script2);
			else if (!strcmp("LOCKPICKING",(char*)script1)) pc_c->baseskill[LOCKPICKING] = getstatskillvalue((char*)script2);
			break;

			case 'M':
			case 'm':
			if ((!(strcmp("MACEFIGHTING",(char*)script1)))||(!(strcmp("SKILL41",(char*)script1)))) pc_c->baseskill[MACEFIGHTING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MINING",(char*)script1)))||(!(strcmp("SKILL45",(char*)script1)))) pc_c->baseskill[MINING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MAGERY",(char*)script1)))||(!(strcmp("SKILL25",(char*)script1)))) pc_c->baseskill[MAGERY] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MAGICRESISTANCE",(char*)script1)))||(!(strcmp("RESIST",(char*)script1)))||(!(strcmp("SKILL26",(char*)script1)))) pc_c->baseskill[MAGICRESISTANCE] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MUSICIANSHIP",(char*)script1)))||(!(strcmp("SKILL29",(char*)script1)))) pc_c->baseskill[MUSICIANSHIP] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MEDITATION",(char*)script1)))||(!(strcmp("SKILL46",(char*)script1)))) pc_c->baseskill[MEDITATION] = getstatskillvalue((char*)script2);
			break;

			case 'N':
			case 'n':

			if (!(strcmp("NAME",(char*)script1))) strcpy(pc_c->name, (char*)script2);
			else if (!(strcmp("NAMELIST", (char*)script1))) {
				scpMark m=pScp->Suspend();
				setrandomname(pc_c,(char*)script2);
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			else if (!(strcmp((char*)script1, "NOTRAIN"))) pc_c->cantrain=false;
			else if (!(strcmp("NPCWANDER",(char*)script1))) pc_c->npcWander=str2num(script2);
			else if (!(strcmp("NPCAI",(char*)script1))) pc_c->npcaitype=hex2num(script2);
			break;

			case 'O':
			case 'o':
			if (!(strcmp("ONHORSE",(char*)script1))) pc_c->onhorse=true;
			break;

			case 'P':
			case 'p':

			if (!strcmp("PARRYING",(char*)script1)) pc_c->baseskill[PARRYING] = getstatskillvalue((char*)script2);
			else if (!(strcmp("PRIV1",(char*)script1))) pc_c->setPriv(str2num(script2));
			else if (!(strcmp("PRIV2",(char*)script1))) pc_c->priv2=str2num(script2);
			else if (!(strcmp("POISON",(char*)script1))) pc_c->poison=str2num(script2);
			else if ((!(strcmp("PEACEMAKING",(char*)script1)))||(!(strcmp("SKILL9",(char*)script1)))) pc_c->baseskill[PEACEMAKING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("PROVOCATION",(char*)script1)))||(!(strcmp("SKILL22",(char*)script1)))) pc_c->baseskill[PROVOCATION] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("POISONING",(char*)script1)))||(!(strcmp("SKILL30",(char*)script1)))) pc_c->baseskill[POISONING] = getstatskillvalue((char*)script2);
			else if (!(strcmp("PACKITEM",(char*)script1)))
			{
				if (pBackpack != NULL) 
				{
					int storeval=str2num(script2);

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if (retitem != NULL)
					{
						retitem->SetContSerial(pBackpack->serial);
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no backpack for packitem.\n", npcNum);
				}
			}
			break;

			case 'R':
			case 'r':
			if (!(strcmp("RACE",(char*)script1))) pc_c->race=str2num(script2);
			else if (!(strcmp("REATTACKAT",(char*)script1))) pc_c->reattackat=str2num(script2);
			else if ((!(strcmp("REMOVETRAPS",(char*)script1)))||(!(strcmp("SKILL48",(char*)script1)))) pc_c->baseskill[REMOVETRAPS] = getstatskillvalue((char*)script2);
			else if (!(strcmp("RSHOPITEM",(char*)script1)))
			{
				if ( shoppack1 == NULL)
				{
					AllItemsIterator iterItems;
					for(iterItems.Begin(); !iterItems.atEnd(); iterItems++)
					{
						P_ITEM pz = iterItems.GetData();
						if (!pz->free)
						{
							if (pc_c->Wears(pz) &&
								pz->layer==0x1A)
							{
								shoppack1 = pz;
								break;
							}
						}
					}
					//if (shoppack1 == -1)
					  //LogError("Error creating shoppack1\n");
				}
				if ( shoppack1 != NULL )
				{
					int storeval=str2num(script2);

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if (retitem != NULL)
					{
						retitem->SetContSerial(shoppack1->serial);
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						if (retitem->name2 && (strcmp(retitem->name2,"#"))) strcpy(retitem->name,retitem->name2); // Item identified! -- by Magius(CHE)					}
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no shoppack1 for item.\n", npcNum);
				}
			}
			break;

			case 'S':
			case 's':

			if (!(strcmp("SKIN",(char*)script1))) 
			{
				pc_c->skin = pc_c->xskin = hex2num(script2);
			}
			else if (!(strcmp("SHOPKEEPER", (char*)script1))) 
			{
				scpMark m=pScp->Suspend();
				Commands->MakeShop(pc_c); 
				pScp->Resume(m);
			}
			else if (!(strcmp("SELLITEM",(char*)script1))) 
			{
				if (shoppack3 == NULL) 
				{
					AllItemsIterator iterItems;
					for(iterItems.Begin(); !iterItems.atEnd(); iterItems++) 
					{
						P_ITEM pz = iterItems.GetData();
						if (!pz->free)
						{
							if (pc_c->Wears(pz) && pz->layer==0x1C)
							{
								shoppack3 = pz;
								break;
							}
						}
					}
				}
				if ( shoppack3 != NULL ) 
				{
					int storeval=str2num(script2);

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if (retitem != NULL)
					{
						retitem->SetContSerial(shoppack3->serial);
						retitem->value=retitem->value/2;
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						if (retitem->name2 && (strcmp(retitem->name2,"#"))) 
							strcpy(retitem->name,retitem->name2); // Item identified! -- by Magius(CHE)					}
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no shoppack3 for item.\n", npcNum);
				}
			}
			else if (!(strcmp("SHOPITEM",(char*)script1)))
			{
				if ( shoppack2 == NULL )
				{
					AllItemsIterator iterItems;
					for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
					{
						P_ITEM pz = iterItems.GetData();
						if (!pz->free)
						{
							if (pc_c->Wears(pz) && pz->layer==0x1B)
							{
								shoppack2 = pz;
								break;
							}
						}
					}
				}
				if (shoppack2 != NULL)
				{
					int storeval = str2num(script2);

					scpMark m = pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if ( retitem != NULL)
					{
						retitem->SetContSerial(shoppack2->serial);
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						if (retitem->name2 && (strcmp(retitem->name2,"#"))) strcpy(retitem->name,retitem->name2); // Item identified! -- by Magius(CHE)					}
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no shoppack2 for item.\n", npcNum);
				}
			}
			else if (!(strcmp("SPATTACK",(char*)script1))) pc_c->spattack=str2num(script2);
			else if (!(strcmp("SPEECH",(char*)script1))) pc_c->speech=str2num(script2);
			else if (!(strcmp("SPLIT",(char*)script1))) pc_c->split=str2num(script2);
			else if ((!(strcmp("STR",(char*)script1)))||(!(strcmp("STRENGTH",(char*)script1)))) {
				pc_c->st  = getstatskillvalue((char*)script2);
				pc_c->st2 = pc_c->st;
				pc_c->hp  = pc_c->st;
			}
			else if (!(strcmp("SPLITCHANCE",(char*)script1))) pc_c->splitchnc=str2num(script2);
			else if (!(strcmp("SAYCOLOR",(char*)script1))) 			pc_c->saycolor = static_cast<UI16>(hex2num(script2));
			else if (!(strcmp("SPADELAY",(char*)script1))) pc_c->spadelay=str2num(script2);
			else if ((!(strcmp("SPIRITSPEAK",(char*)script1)))||(!(strcmp("SKILL32",(char*)script1)))) pc_c->baseskill[SPIRITSPEAK] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("STEALTH",(char*)script1)))||(!(strcmp("SKILL47",(char*)script1)))) pc_c->baseskill[STEALTH] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("SWORDSMANSHIP",(char*)script1)))||(!(strcmp("SKILL40",(char*)script1)))) pc_c->baseskill[SWORDSMANSHIP] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("SNOOPING",(char*)script1)))||(!(strcmp("SKILL28",(char*)script1)))) pc_c->baseskill[SNOOPING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("STEALING",(char*)script1)))||(!(strcmp("SKILL33",(char*)script1)))) pc_c->baseskill[STEALING] = getstatskillvalue((char*)script2);
			else if (!(strcmp("SKINLIST",(char*)script1)))
			{
				scpMark m=pScp->Suspend();
				pc_c->xskin = pc_c->skin = addrandomcolor(pc_c,(char*)script2);
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			else if (!(strcmp("SKILL", (char*)script1)))
			{
				gettokennum((char*)script2, 0);
				z=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				pc_c->baseskill[z]=str2num(gettokenstr);
			}

            else if (!(strcmp("STABLEMASTER",(char*)script1))) pc_c->npc_type=1;
			break;

			case 'T':
			case 't':
			if (!(strcmp("TITLE",(char*)script1))) strcpy(pc_c->title, script2);
			else if ((!(strcmp("TOTAME", (char*)script1)))||(!(strcmp("TAMING", (char*)script1)))) pc_c->taming=str2num(script2);
			else if (!(strcmp("TRIGGER",(char*)script1)))	pc_c->trigger=str2num(script2);
			else if (!(strcmp("TRIGWORD",(char*)script1)))	strcpy(pc_c->trigword,(char*)script2);
			else if ((!(strcmp("TACTICS",(char*)script1)))||(!(strcmp("SKILL27",(char*)script1)))) pc_c->baseskill[TACTICS] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TAILORING",(char*)script1)))||(!(strcmp("SKILL34",(char*)script1)))) pc_c->baseskill[TAILORING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TAMING",(char*)script1)))||(!(strcmp("SKILL35",(char*)script1)))) pc_c->baseskill[TAMING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TASTEID",(char*)script1)))||(!(strcmp("SKILL36",(char*)script1)))) pc_c->baseskill[TASTEID] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TINKERING",(char*)script1)))||(!(strcmp("SKILL37",(char*)script1)))) pc_c->baseskill[TINKERING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TRACKING",(char*)script1)))||(!(strcmp("SKILL38",(char*)script1)))) pc_c->baseskill[TRACKING] = getstatskillvalue((char*)script2);
			break;

			case 'V':
			case 'v':
			if (!(strcmp("VALUE",(char*)script1))) if (retitem != NULL) retitem->value=(str2num(script2));
			else if (!strcmp("VETERINARY",(char*)script1)) pc_c->baseskill[VETERINARY] = getstatskillvalue((char*)script2);
			break;
			
			case 'W':
			case 'w':
			if ((!(strcmp("WRESTLING",(char*)script1)))||(!(strcmp("SKILL43",(char*)script1)))) pc_c->baseskill[WRESTLING] = getstatskillvalue((char*)script2);
			break;

			default:
				clConsole.send("Warning: Fall out of switch statement in npcs.cpp AddNPC()\n");
			}
			
			//Done Handling Skills
			//Handle Extras

			//Done Handling Obsolete Stuff
			//--------------- DONE NEW STAT & SKILL FORMAT ---------------------
      }
   }
   while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
   pScp->Close();

   // Now that we have created the NPC, lets place him
   switch (postype)
   {
   case 1:					// take position from (spawning) item
	   if (triggerx)
	   {
		   pc_c->pos.x=triggerx;
		   pc_c->pos.y=triggery;
		   pc_c->dispz=pc_c->pos.z=triggerz;
		   triggerx = pc_c->serial;
	   } else
	   {
	   /*Zippy's Code chages for area spawns --> (Type 69) xos and yos (X OffSet, Y OffSet) 
	   are used to find a random number that is then added to the spawner's x and y (Using 
	   the spawner's z) and then place the NPC anywhere in a square around the spawner. 
	   This square is random anywhere from -10 to +10 from the spawner's location (for x and 
	   y) If the place chosen is not a valid position (the NPC can't walk there) then a new 
	   place will be chosen, if a valid place cannot be found in a certain # of tries (50), 
		   the NPC will be placed directly on the spawner and the server op will be warned. */
		   if ((pi_i->type==69 || pi_i->type==125)&& pi_i->isInWorld())
		   {
			   if (pi_i->more3==0) pi_i->more3=10;
			   if (pi_i->more4==0) pi_i->more4=10;
			   //signed char z, ztemp, found;
	   
			   k=0;
			   do
			   {
				   if (k>=50) //this CAN be a bit laggy. adjust as nessicary
				   {
					   clConsole.send("WOLFPACK: Problem area spawner found at [%i,%i,%i]. NPC placed at default location.\n",pi_i->pos.x,pi_i->pos.y,pi_i->pos.z);
					   xos=0;
					   yos=0;
					   break;
				   }
				   xos=RandomNum(-pi_i->more3,pi_i->more3);
				   yos=RandomNum(-pi_i->more4,pi_i->more4);
				   // clConsole.send("Spawning at Offset %i,%i (%i,%i,%i) [-%i,%i <-> -%i,%i]. [Loop #: %i]\n",xos,yos,pi_i->x+xos,pi_i->y+yos,pi_i->z,pi_i->more3,pi_i->more3,pi_i->more4,pi_i->more4,k); /** lord binary, changed %s to %i, crash when uncommented ! **/
				   k++;
				   if ((pi_i->pos.x+xos<1) || (pi_i->pos.y+yos<1)) lb=0; /* lord binary, fixes crash when calling npcvalid with negative coordiantes */
				   else lb = Movement->validNPCMove(pi_i->pos.x+xos,pi_i->pos.y+yos,pi_i->pos.z, pc_c);				 
				   
				   //Bug fix Monsters spawning on water:
				   MapStaticIterator msi(pi_i->pos.x + xos, pi_i->pos.y + yos);

				   staticrecord *stat;
				   loopexit=0;
				   while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
				   {
					   tile_st tile;
					   msi.GetTile(&tile);
					   if(!(strcmp((char *) tile.name, "water")))//Water
					   {//Don't spawn on water tiles... Just add other stuff here you don't want spawned on.
						   lb=0;
					   }
				   }
			   } while (!lb);
		   } // end Zippy's changes (exept for all the +xos and +yos around here....)

		   pc_c->pos.x=pi_i->pos.x+xos;
		   pc_c->pos.y=pi_i->pos.y+yos;
		   pc_c->dispz=pc_c->pos.z=pi_i->pos.z;
		   pc_c->SetSpawnSerial(pi_i->serial);
		   if ( pi_i->type == 125 )
		   {
			  MsgBoardQuestEscortCreate( pc_c );
		   }
	   } // end of if !triggerx
		break;
	case 2: // take position from Socket
		if (s!=-1)
		{
			pc_c->pos.x=(buffer[s][11]<<8)+buffer[s][12];
			pc_c->pos.y=(buffer[s][13]<<8)+buffer[s][14];
			pc_c->dispz=pc_c->pos.z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
		}
		break;
	case 3: // take position from Parms
		pc_c->pos.x=x1;
		pc_c->pos.y=y1;
		pc_c->dispz=pc_c->pos.z=z1;
		break;
	} // no default coz we tested on entry to function

	// now we have a position, let's set the borders
	switch (pc_c->npcWander)
	{
	case 2:		// circle
		pc_c->fx1=pc_c->pos.x;
		pc_c->fy1=pc_c->pos.y;
		pc_c->fz1=pc_c->pos.z;
		pc_c->fx2=(fx2>=0) ? fx2 : 2;	// radius; if not given from script,default=2
		break;
	case 3:		// box
		if (fx1 || fx2 || fy1 || fy2)	// any offset for rectangle given from script ?
		{
			pc_c->fx1=pc_c->pos.x+fx1;
			pc_c->fx2=pc_c->pos.x+fx2;
			pc_c->fy1=pc_c->pos.y+fy1;
			pc_c->fy2=pc_c->pos.y+fy2;
			pc_c->fz1= -1;			// irrelevant for box
		}
		break;
	//default: do nothing
	}
   
	pc_c->region = calcRegionFromXY(pc_c->pos.x, pc_c->pos.y);
   
   //Now find real 'skill' based on 'baseskill' (stat modifiers)
   for(z=0;z<TRUESKILLS;z++)
   {
	   Skills->updateSkillLevel(pc_c, z);
   }
   
   updatechar(pc_c);

   // Dupois - Added April 4, 1999
   // After the NPC has been fully initialized, then post the message (if its a quest spawner) type==125
   if (postype==1) // lb crashfix
   {
   }
   // End - Dupois

   //Char mapRegions
   mapRegions->Remove(pc_c);
   mapRegions->Add(pc_c);
   return pc_c;
}

void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int c,serial,z;

	if ( pc_k == NULL ) return;
	
	P_CHAR pc_c = Npcs->MemCharFree();
	if ( pc_c == NULL ) return;
	pc_c->Init();
	serial=pc_c->serial;
	memcpy(pc_c, pc_k, sizeof(cChar));
	pc_c->ser1=serial>>24;
	pc_c->ser2=serial>>16;
	pc_c->ser3=serial>>8;
	pc_c->ser4=serial%256;
	pc_c->serial=serial;
	pc_c->ftarg = INVALID_SERIAL;
	pc_c->MoveTo(pc_k->pos.x+1, pc_k->pos.y, pc_k->pos.z);
	pc_c->kills=0;
	pc_c->hp=pc_k->st;
	pc_c->stm=pc_k->realDex();
	pc_c->mn=pc_k->in;
	z=rand()%35;
	if (z==5) pc_c->split=1; else pc_c->split=0;	
	updatechar(pc_c);
}

