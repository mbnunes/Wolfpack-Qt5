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
Module : fishing.cpp
Purpose: store all fishing related functions
Created: Ripper...1-31-01
History: None
*/

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "fishing.cpp"

int SpawnFishingMonster(UOXSOCKET s, char* cScript, char* cList, char* cNpcID)
{
	/*This function gets the random monster number from
	the script and list specified.
	Npcs->AddRespawnNPC passing the new number*/

	P_CHAR pc_currchar = MAKE_CHARREF_LRV(currchar[s],-1);

	if (pc_currchar->inGuardedArea() && SrvParms->guardsactive) //guarded
		return -1;

	char sect[512];
	int i=0,item[256]={0};
 	openscript(cScript);
 	sprintf(sect, "%s %s", cList, cNpcID);
	if(!(strcmp("fishing.scp",cScript)))
	{
		if(!i_scripts[fishing_script]->find(sect))
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
					closescript();
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
			Npcs->AddRespawnNPC(s,item[i],0);
			return item[i];
		}
	}
	return -1;
}

int SpawnFishingItem(UOXSOCKET s,int nInPack, char* cScript, char* cList, char* cItemID)
{
 	/*This function gets the random item number from the list and recalls
 	  SpawnItemBackpack2 passing the new number*/
	char sect[512];
	int i=0,item[256]={0};
 	openscript(cScript);
	sprintf(sect, "%s %s", cList, cItemID);
	if(!(strcmp("fishing.scp",cScript)))
	{
		if(!i_scripts[fishing_script]->find(sect))
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
					closescript();
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
			Items->SpawnItemBackpack2(s,item[i],1);
			return item[i];
		}
	}
	return -1;
}

void cFishing::FishTarget(P_CLIENT ps)
{
	// fixes an exploit with fishing, LB
	int px,py,cx,cy;
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pPlayer=ps->getPlayer();
	if (!pPlayer) return;

	px=((buffer[s][0x0b]<<8)+(buffer[s][0x0c]%256));
	py=((buffer[s][0x0d]<<8)+(buffer[s][0x0e]%256));
	cx=abs(pPlayer->pos.x-px);
	cy=abs(pPlayer->pos.y-py);
//	int cz=abs(pPlayer->z-buffer[s][0x10]);

	if(!(cx<=6 && cy<=6))
	{
		ps->SysMsg("You are too far away to reach that");
		return;
	}
	// end exploit fix

	bool ok=false;
	if (IsFishWater(ShortFromCharPtr(buffer[s]+0x11)))
		ok = true;
	else
	{	// it might be offshore fishing (client returns id=0 for deep sea ) Duke, Thx for the hint goes to Avanoon guys
		map_st map = Map->SeekMap0(px, py);	// search the ground tile where they casted
		if ( map.id == 0x00A8 || map.id == 0x00A9 || map.id == 0x00AA || map.id == 0x00AB )
			ok = true;
	}
	if (ok)
	{
		action(s,0x0b);
		//if ((buffer[s][1]==1 || buffer[s][1]==0)&&(buffer[s][2]==0)&&(buffer[s][3]==1)&&(buffer[s][4]==0))
		{
			if (fishing_data.randomtime!=0)
				pPlayer->fishingtimer=rand()%fishing_data.randomtime+fishing_data.basetime;
			else
				pPlayer->fishingtimer=fishing_data.basetime;
		}
		soundeffect(s, 0x02, 0x3F);
		pPlayer->unhide();
		Fish(DEREF_P_CHAR(pPlayer));
	}
	else
		ps->SysMsg("You need to be closer to the water to fish!");
}


// LB: added fish stacking !!
void cFishing::Fish(CHARACTER i)
{
	const int max_fish_piles = 1;		// attention: thats per fish *type*, so the efffective limit of piles is *3
	const int max_fish_stacksize = 15;	// attention: rela max size = value+1

	int d,ii,min,ss,b,mc;
	int idnum;
	int s=calcSocketFromChar(i);
	int color,c1,c2;

	int fishes_around_player;
	int fish_sers[max_fish_piles];

	P_CHAR pc_i = MAKE_CHARREF_LR(i);

	if(!Skills->CheckSkill(i,FISHING, 0, 1000))
	{
		sysmessage(s,"You fish for a while, but fail to catch anything.");
		return;
	}

	if(pc_i->stm<=0)
	{
		pc_i->stm=0;
		sysmessage(s, "You are too tired to fish, you need to rest!");
		return;
	}

	pc_i->stm-=2; // lose 2 stamina each cast.

	// New Random fishing up treasures and monsters...Ripper
	unsigned short skill=pc_i->skill[FISHING];
	int fishup=(RandomNum(0,100));
		switch (fishup)
		{
		case 0:
            if(skill>=200) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "5" ); // random boots
				sysmessage( s, "You fished up an old pair of boots!" ); 
			} 
            break;
		case 1:
            if(skill>=970) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "1" ); // random paintings 
				sysmessage( s, "You fished up an ancient painting!" ); 
			} 
            break;
		case 2:
            if(skill>=950) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "2" ); // random weapons 
				sysmessage( s, "You fished up an ancient weapon!" ); 
			} 
            break;
		case 3:
            if(skill>=950) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "3" ); // random armor 
				sysmessage( s, "You fished up an ancient armor!" ); 
			} 
            break;
		case 4:
            if(skill>=700) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "4" ); // random treasure
				sysmessage( s, "You fished up some treasure!" ); 
			} 
            break;
		case 5:
            if(skill>=400) 
			{ 
				if (SpawnFishingMonster( s,"fishing.scp", "MONSTERLIST", "7" ) != -1) // random monsters 
					sysmessage( s, "You fished up a hughe fish!" );
				else
					sysmessage( s, "You wait for a while, but nothing happens");
			} 
            break;
		case 6:
            if(skill>=800) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "6" ); // random chests
				sysmessage( s, "You fished up an old chest!" );
			} 
            break;
		case 7:
            if(skill>=700) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "8" ); // random seashells
				sysmessage( s, "You fished up a seashell!" );
			} 
            break;
		case 8:
            if(skill>=700) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "9" ); // random skulls
				sysmessage( s, "You fished up a skull!" );
			} 
            break;
		case 9:
            if(skill>=900) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "10" ); // random nets
				sysmessage( s, "You fished up a net!" );
			} 
            break;
		case 10:
            if(skill>=900) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "11" ); // random gold
				sysmessage( s, "You fished up some gold!" );
			} 
            break;
		case 11:
            if(skill>=400) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "12" ); // random bones
				sysmessage( s, "You fished up some bones!" );
			} 
            break;
		default:

	ii=rand()%3;
	idnum=0xCC+ii;

	///**** exotic fish stuff *****//
	float mv=-0.087087087f*(float)pc_i->skill[FISHING]+100.087087087086f; // gm fish -> 1/13 % probability on new spawn(!) to have exotic color, neophyte: 1/92 % probability, linear interpolation in between
	int no_idea_for_variable_name = (int) mv;
	if (no_idea_for_variable_name<=0) no_idea_for_variable_name=1; // prevent modulo crashes	
	if (rand()%no_idea_for_variable_name==0) 
	{ 		
		color=(pc_i->pos.x+pc_i->pos.y);
		color+=rand()%10;
		color=color%0x03E9; 
		c1=color>>8;
		c2=color%256;		
		if ((((c1<<8)+c2)<0x0002) || (((c1<<8)+c2)>0x03E9) )
		{
			c1=0x03;
			c2=0xE9;
		}
		b=((((c1<<8)+c2)&0x4000)>>14)+((((c1<<8)+c2)&0x8000)>>15);
		if (b)
		{
			c1=0x1;
			c2=rand()%255;
		}
	} else c1=c2=0;

	/**** end of exotic fish stuff stuff */

	fishes_around_player=Items->Find_items_around_player(DEREF_P_CHAR(pc_i), 0x09, idnum, 2, 2, max_fish_piles, fish_sers); // lets search for fish in a 2*2 rectangle around the player
	
	P_ITEM pFish;
	if (fishes_around_player<=0) // no fish around -> spawn a new one
	{
		pFish=Items->SpawnItem(DEREF_P_CHAR(pc_i),1,"#",1,0x0900+idnum,(c1<<8)+c2,0);
		if(!pFish) return;//AntiChrist to prevent crashes
		pFish->type=14;
		pFish->MoveTo(pc_i->pos.x,pc_i->pos.y,pc_i->pos.z);
		RefreshItem(pFish);
	}
	else // fishes around ?
	{
		ITEM c = -1;
		min=1234567;
		mc=0; // crash prevention if for some strange reason no min is found
		for (d=0;d<fishes_around_player;d++) // lets pick the smallest pile form the return list
		{
			c = calcItemFromSer( fish_sers[d] );
			if (c>-1)
			{
				ss=items[c].amount;
				if (ss<min) { min=ss; mc=c; }
			}
		}

		if (items[mc].amount>max_fish_stacksize) // if smaleest fish-stack > max_stacksize spawn a new fish anyway
		{
			if (fishes_around_player>=max_fish_piles)
			{
				sysmessage(s,"you catch a fish, but no place for it left");
				return;
			}
			
			P_ITEM pFish=Items->SpawnItem(DEREF_P_CHAR(pc_i),1,"#",1,0x0900+idnum,(c1<<8)+c2,0);
			if(!pFish) return;
			pFish->type=14;
			pFish->MoveTo(pc_i->pos.x,pc_i->pos.y,pc_i->pos.z);
			RefreshItem(pFish);
		}
		else // if fish stack <=max_ -> just increase stack !!
		{
			if (c>-1)
			{
				items[c].amount++;
				RefreshItem(c);
			}
		}

	} // end else fishes around
	if(c2>0)
	{
		sysmessage(s,"You pull out an exotic fish!");
	}
	else
	{
	    sysmessage(s,"You pull out a fish!");
	}
	break;
	}
}

