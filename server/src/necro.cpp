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
Module : necro.cpp
Purpose: store all necromancy related functions
Created: Genesis 11-12-1998
History: None
*/

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "necro.cpp"

int SpawnRandomMonster(int nCharID, char* cScript, char* cList, char* cNpcID)
{
	/*This function gets the random monster number from the from
	the script and list specified.
	Npcs->AddRespawnNPC passing the new number*/

	char sect[512];
	int i=0,item[256]={0};
 	openscript(cScript);
 	sprintf(sect, "%s %s", cList, cNpcID);
	if(!(strcmp("necro.scp",cScript)))
	{
		if(!i_scripts[necro_script]->find(sect))
		{
  			closescript();
  			return -1;
 		}
	}
	else
	{
		if(!i_scripts[npc_script]->find(sect))
 		{
  			closescript();
			if (n_scripts[custom_npc_script][0]!=0)
			{
				openscript(n_scripts[custom_npc_script]);
				if (!i_scripts[custom_npc_script]->find(sect))
				{
					closescript(); //AntiChrist
					return -1;
				}
			} else return -1;
		}
	}
	unsigned long loopexit=0;
 	do
 	{
  		read1();
		if(script1[0]!='}')
		{
			item[i]=str2num(script1);
			i++;
		}
 	}
 	while(script1[0]!='}' && (++loopexit < MAXLOOPS) );
 	closescript();
 	if(i>0)
 	{
  		i=rand()%(i);
		if(item[i]!=-1)
		{
			Npcs->AddNPC(nCharID, NULL, item[i]);
			return item[i];
		}
	}
	return -1;
}

int SpawnRandomItem(int nCharID,int nInPack, char* cScript, char* cList, char* cItemID)
{
 	/*This function gets the random item number from the list and recalls
 	  SpawnItemBackpack2 passing the new number*/
	char sect[512];
	int i=0,item[256]={0};
 	openscript(cScript);
	sprintf(sect, "%s %s", cList, cItemID);
	if(!(strcmp("necro.scp",cScript)))
	{
		if(!i_scripts[necro_script]->find(sect))
		{
			closescript();
			return -1;
		}
	}
	else
	{
		if(!i_scripts[items_script]->find(sect))
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
	}
	unsigned long loopexit=0;
 	do
 	{
  		read1();
  		if (script1[0]!='}')
  		{
			item[i]=str2num(script1);
   			i++;
		}
	}
	while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
 	if(i>0)
 	{
  		i=rand()%(i);
		if(item[i]!=-1)
		if(nInPack)
		{
			Items->SpawnItemBackpack2(nCharID,item[i],1);
			return item[i];
		}
	}
	return -1;
}

void vialtarget(int nSocket) // bug & crashfixed by LB 25 september 1999
{
	cItem* Vial=MAKE_ITEMREF_LR(addx[nSocket]);
	if (!Vial) return; // should never happen
	
//	int nTargetID=-1;
	
	
	int cc = currchar[nSocket];
	cChar* Player = MAKE_CHARREF_LR(cc);

	cItem* Weapon = Player->getWeapon(); // search for a dagger in the players hand
	if (!Weapon || !IsDagger(Weapon->id()) )
	{
		sysmessage(nSocket,"You do not have a dagger equipped.");
		return;
	}
	
	int serial=LongFromCharPtr(buffer[nSocket]+7);
	if (isCharSerial(serial))
	{
		cChar* Victim = FindCharBySerial(serial);
		if (!Victim)
			return;
		Vial->more1=0;
		if(!Victim->npc)
		{
			// checkskill hmmm what skill/s has/have to added here LB ...
			
			if( Victim->isSameAs(Player) )
			{
				if(Victim->hp<=10)
				{
					sysmessage(nSocket,"You are too wounded to continue.");
					return;
				}
				sysmessage(nSocket,"You prick your finger and fill the vial.");
			}
			else
			{
				if (Player->dist(Victim) > 2)
				{
					sysmessage(nSocket,"That individual is not anywhere near you.");
					return;
				}
				int vv=DEREF_P_CHAR(Victim);
				if (Victim->npc)
				{
					if( Victim->id1==0x00 &&( Victim->id2==0x0c || (Victim->id2>=0x3b && Victim->id2<=0x3d) ))
						Vial->more1=1;
					// Guard be summuned if in town and good npc
					// if good flag criminal
					// if evil npc attack necromancer but don't flag criminal
				}
				else
				{
					sprintf(temp,"%s has pricked you with a dagger and sampled your blood.",Player->name);
					sysmessage(calcSocketFromChar(vv),temp);
					// flag criminal						
				}
				Karma(cc,vv,(0-(Victim->karma)));
			}
			Victim->hp -= (rand()%6)+2;
			MakeNecroReg(nSocket,Vial,0x0E24);
		}
	}
	else
	{
		cItem* Corpse = FindItemBySerial(serial);
		if (!Corpse)
			return;
		if (!Corpse->corpse)
			sysmessage(nSocket,"That is not a person or a corpse!");
		else
		{
			Vial->more1=Corpse->more1;
			Karma(cc,-1,-1000);
			if (Corpse->more2<4)
			{
				sysmessage(nSocket,"You take a sample of blood from the corpse.");
				MakeNecroReg(nSocket,Vial,0x0E24);
				Corpse->more2++;
			}
			else
				sysmessage(nSocket,"You examine the corpse but, decide any further blood samples would be to contaminated.");
		}
	}
}

void MakeNecroReg(int nSocket, P_ITEM pMat, short id)
{
	P_ITEM pItem = NULL;
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[nSocket]);

	if( id>=0x1B11 && id<=0x1B1C ) // Make bone powder.
	{
		sprintf((char*)temp,"%s is grinding some bone into powder.", pc_currchar->name);
		npcemoteall(DEREF_P_CHAR(pc_currchar), (char*)temp,1);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 0, 0);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 3, 0);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 6, 0);
		tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 9, 0, 9, 0);
		pItem = Items->SpawnItem(nSocket,DEREF_P_CHAR(pc_currchar),1,"bone powder",1,0x0F,0x8F,0,0,1,1);
		if(pItem == NULL) return;//AntiChrist to preview crashes
		pItem->morex = 666;
		pItem->more1=1; // this will fill more with info to tell difference between ash and bone
		Items->DeleItem(pMat);
		
	}
	if( id==0x0E24 ) // Make vial of blood.
	{
		if(pMat->more1==1)
		{
			pItem = Items->SpawnItem(nSocket,DEREF_P_CHAR(pc_currchar),1,"#",1,0x0F,0x82,0,0,1,1);
			if(pItem==NULL) return;//AntiChrist to preview crashes
			pItem->value=15;
			pItem->morex=666;
		}
		else
		{
			pItem = Items->SpawnItem(nSocket,DEREF_P_CHAR(pc_currchar),1,"#",1,0x0F,0x7D,0,0,1,1);
			if(pItem==NULL) return;//AntiChrist to preview crashes
			pItem->value=10;
			pItem->morex=666;
		}
		pMat->ReduceAmount(1);
	}
}
