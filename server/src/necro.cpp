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
//	Wolfpack Homepage: http://www.wpdev.com/
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
	int loopexit=0;
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
			Npcs->AddRespawnNPC(nCharID,item[i],0);
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
	int loopexit=0;
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
	if(buffer[nSocket][11]==0xFF && buffer[nSocket][12]==0xFF && buffer[nSocket][13]==0xFF && buffer[nSocket][14]==0xFF) 
		return; // check if user canceled operation
	
	P_ITEM pVial=MAKE_ITEMREF_LR(addx[nSocket]);
	if (!pVial) return; // should never happen
	
	int nTargetID=-1,nDagger=-1,nDist=0,item,serhash,i,dag,ci;
	
	int serial=calcserial(buffer[nSocket][7],buffer[nSocket][8],buffer[nSocket][9],buffer[nSocket][10]);
	int nItemID = calcCharFromSer( serial ); // targeted a living creature ?
	if (nItemID!=-1)
		nTargetID=nItemID;
	
	nItemID = calcItemFromSer( serial ); // or a corpse/differnt item ?
	if ((nItemID!=-1) && (nTargetID==-1)) nTargetID=nItemID;
	
	int cc = currchar[nSocket];
	serial=chars[cc].serial; // search for a dagger in the players hand
	serhash=serial%HASHMAX;
	item=-1;
	for (ci=0;ci<contsp[serhash].max;ci++)
	{
		i=contsp[serhash].pointer[ci];
		if (i!=-1)
		{
			if (items[i].id()==0x0F51 || items[i].id()==0x0F52) dag=1; else dag=0;
			if (((items[i].layer==1)&&(items[i].contserial==serial) && dag))
			{
				item=i;
				break;
			}
		}
	}
	
	nDagger=item;
	
	if(pVial && nTargetID!=-1 && nDagger!=-1)
	{
		pVial->more1=0;
		if((chars[nTargetID].isPlayer() || chars[nTargetID].isNpc()) && items[nTargetID].corpse!=1)
		{
			// checkskill hmmm what skill/s has/have to added here LB ...
			
			if( nTargetID == cc)
			{
				if(chars[nTargetID].hp<=10)
				{
					sysmessage(nSocket,"You are too wounded to continue.");
					return;
				}
				else
				{
					sysmessage(nSocket,"You prick your finger and fill the vial.");
					chars[nTargetID].hp=(chars[nTargetID].hp-((rand()%6)+2));
					MakeNecroReg(nSocket,pVial,0x0E24);
					return;
				}
			}
			else
			{
				nDist=chardist(cc,nTargetID);
				if((inrange1p(cc,nTargetID))&&
					(nDist<=2))
				{
					sprintf((char*)temp,"%s has pricked you with a dagger and sampled your blood.",chars[cc].name);
					if(chars[nTargetID].isNpc())
					{
						Karma(cc,nTargetID,(0-(chars[nTargetID].karma)));
						if((chars[nTargetID].id1==0x00)&&((chars[nTargetID].id2==0x0c)||(chars[nTargetID].id2>=0x3b)&&(chars[nTargetID].id2<=0x3d)))
							pVial->more1=1;
						chars[nTargetID].hp=(chars[nTargetID].hp-((rand()%6)+2));
						MakeNecroReg(nSocket,pVial,0x0E24);
						// Guard be summuned if in town and good npc
						// if good flag criminal
						// if evil npc attack necromancer but don't flag criminal
					}
					else
					{
						Karma(cc,nTargetID,(0-(chars[nTargetID].karma)));
						chars[nTargetID].hp=(chars[nTargetID].hp-((rand()%6)+2));
						sysmessage(calcSocketFromChar(nTargetID),(char*)temp);
						MakeNecroReg(nSocket,pVial,0x0E24);
						// flag criminal						
					}
				}
				else 
				{
					sysmessage(nSocket,"That individual is not anywhere near you.");
					return;
				}
			}
		}
		else
			if(items[nTargetID].corpse==1)
			{
				pVial->more1=items[nTargetID].more1;
				Karma(cc,-1,-1000);
				if(items[nTargetID].more2<4)
				{
					sysmessage(nSocket,"You take a sample of blood from the corpse.");
					MakeNecroReg(nSocket,pVial,0x0E24);
					items[nTargetID].more2++;
				}
				else sysmessage(nSocket,"You examine the corpse but, decide any further blood samples would be to contaminated.");
			}
			else sysmessage(nSocket,"That is not a person or a corpse!");
	}		
	if(nDagger==-1) sysmessage(nSocket,"You do not have a dagger equipped.");	
}

void MakeNecroReg(int nSocket,P_ITEM pMat,short id)
{
	int iItem=0;
	int cc=currchar[nSocket];
	int p=packitem(cc);

	if( id>=0x1B11 && id<=0x1B1C ) // Make bone powder.
	{
		sprintf((char*)temp,"%s is grinding some bone into powder.", chars[cc].name);
		npcemoteall(cc, (char*)temp,1);
		tempeffect(cc, cc, 9, 0, 0, 0);
		tempeffect(cc, cc, 9, 0, 3, 0);
		tempeffect(cc, cc, 9, 0, 6, 0);
		tempeffect(cc, cc, 9, 0, 9, 0);
		iItem=Items->SpawnItem(nSocket,cc,1,"bone powder",1,0x0F,0x8F,0,0,1,1);
		if(iItem==-1) return;//AntiChrist to preview crashes
		items[iItem].morex=666;
		items[iItem].more1=1; // this will fill more with info to tell difference between ash and bone
		Items->DeleItem(pMat);
		
	}
	if( id==0x0E24 ) // Make vial of blood.
	{
		if(pMat->more1==1)
		{
			iItem=Items->SpawnItem(nSocket,cc,1,"#",1,0x0F,0x82,0,0,1,1);
			if(iItem==-1) return;//AntiChrist to preview crashes
			items[iItem].value=15;
			items[iItem].morex=666;
		}
		else
		{
			iItem=Items->SpawnItem(nSocket,cc,1,"#",1,0x0F,0x7D,0,0,1,1);
			if(iItem==-1) return;//AntiChrist to preview crashes
			items[iItem].value=10;
			items[iItem].morex=666;
		}
		pMat->ReduceAmount(1);
	}
}
