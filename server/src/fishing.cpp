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
#include "srvparams.h"
#include "classes.h"
#include "mapstuff.h"
#include "network.h"
#include "skills.h"

#undef  DBGFILE
#define DBGFILE "fishing.cpp"

int SpawnFishingMonster(UOXSOCKET s, char* cScript, char* cList, char* cNpcID)
{
	/*This function gets the random monster number from
	the script and list specified.
	Npcs->AddRespawnNPC passing the new number*/

/*	P_CHAR pc_currchar = currchar[s];

	if (pc_currchar->inGuardedArea() && SrvParams->guardsActive()) //guarded
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
			Npcs->createScriptNpc(s, NULL, QString("%1").arg(item[i]));
			return item[i];
		}
	}
	*/
	return -1;
}

int SpawnFishingItem(UOXSOCKET s,int nInPack, char* cScript, char* cList, char* cItemID)
{
 	/*This function gets the random item number from the list and recalls
 	  SpawnItemBackpack2 passing the new number*/
/*	char sect[512];
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
			Items->SpawnItemBackpack2(s,QString("%1").arg(item[i]),1);
			return item[i];
		}
	}
*/
	return -1;
}

