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
// Name:	sregions.cpp
// Purpose: functions that handle the spawn regions
// History:	cut from wolfpack.cpp by Duke, 26.10.00
// Remarks:	not necessarily ALL those functions
//
#include "sregions.h"

#undef  DBGFILE
#define DBGFILE "sregions.cpp"

cRespawn	*Respawn;

bool FindSpotForItem(int r, int &x, int &y, int &z)
{
	int a=0;
	do
	{
		x=RandomNum(spawnregion[r].x1,spawnregion[r].x2);
		y=RandomNum(spawnregion[r].y1,spawnregion[r].y2);
		z=Map->MapElevation( x, y );

		if (Map->CanMonsterMoveHere(x, y, z))
		{
#if 1		// disabled for now
			return true;
#else
			for (int dir=0;dir<=7;dir++)		// check if we can move FROM here
			{
				int newX = x;
				int newY = y;
				cMovement::getXYfromDir(dir,&newX,&newY);	// get coords of the location we want to walk
				if (Movement->validNPCMove(newX, newY, z, dir))
					return true;
			}
#endif
		}
	} while (++a < 100);
	clConsole.send("WOLFPACK: Problem regionspawn [%i] found. Nothing will be spawned.\n",r);
	return false;
}

// Total rewrite by LB 21'th September 2000
// changed to bool. breaks after one spawn to allow for reallocation of memory (Duke, 15.4.2001)
bool doregionspawn(int r)//Regionspawns
{
	//clConsole.send("doregionspawn: %i start current: %i\n",r,spawnregion[r].current);

	if (spawnregion[r].current<=0) spawnregion[r].current=0;
	if (spawnregion[r].current >= spawnregion[r].max || spawnregion[r].max == 0) return false;

	int x = 0, y = 0, z = 0;
	int npc = -1, item = -1;
	int counter;

	if( spawnregion[r].totalnpclists > 0 )
	{
//		for( counter = 0; counter < spawnregion[r].totalnpclists; counter++ )
		counter = rand()%spawnregion[r].totalnpclists;
		{
			if (FindSpotForItem(r, x, y, z))
			{
				npc = Npcs->AddNPCxyz( -1, spawnregion[r].npclists[counter], 0,x,y,z );
				if (npc!=-1)
				{
					spawnregion[r].current++;
					chars[npc].spawnregion=r;
					return true;
				}
			}
		}
	}

	if( spawnregion[r].totalitemlists > 0 )
	{
//		for( counter = 0; counter < spawnregion[r].totalitemlists; counter++ )
		counter = rand()%spawnregion[r].totalitemlists;
		{
			if (FindSpotForItem(r, x, y, z))
			{
				char list[512];
				sprintf( list, "%i", spawnregion[r].itemlists[counter] ); // morrolan
				int num = Items->CreateRandomItem( list );
				item = Items->CreateScriptItem( -1, num, 0 );
				const P_ITEM pi=MAKE_ITEMREF_LRV(item,false);	// on error return
				pi->MoveTo(x,y,z);
				pi->spawnregion=r;
				RefreshItem(pi);
				spawnregion[r].current++;
				return true;
			}
		}
	}

	if( spawnregion[r].totalnpcs>0 )
	{
//		for (counter=0; counter < spawnregion[r].totalnpcs; counter++)
		counter = rand()%spawnregion[r].totalnpcs;
		{
			if (FindSpotForItem(r, x, y, z))
			{
				npc = Npcs->AddNPCxyz( -1, spawnregion[r].npcs[counter], 0,x,y,z );
				if (npc!=-1)
				{
					spawnregion[r].current++;
					chars[npc].spawnregion=r;
					return true;
				}
			}
		}
	}

	if (npc==-1 && item==-1)
	{
		//clConsole.send("Warning: Region spawn %i [%s] couldn't find anything to spawn, check scripts.\n",r-1,spawnregion[r].name);
		return false;
	}
	else
		return true;
}

void loadspawnregions()//Regionspawns
{
	int i=0;

	spawnregion_st dummy;

//	i++;
	spawnregion.clear();
	spawnregion.reserve(1); // Spawnregions start at 1, so we need to trash spawnregion[0] :(

	openscript("spawn.scp");
	unsigned long loopexit=0 ;
	do
	{
		read2();
		if((!strcmp((char*)script1, "SECTION"))&&(script2[0]=='R'))
		{
			memset(&dummy, 0, sizeof(dummy));

			loopexit=0;
			do
			{
				read2();
				if(script1[0]!='}')
				{
					if(!strcmp("NLIST", (char*)script1))
					{
						dummy.npclists[dummy.totalnpclists]=str2num(script2);
						dummy.totalnpclists++;
					}
					else if(!strcmp("NPC",(char*)script1))
					{
						dummy.npcs[dummy.totalnpcs]=str2num(script2);
						dummy.totalnpcs++;
					}
					else if(!strcmp("ILIST",(char*)script1))
					{
						dummy.itemlists[dummy.totalitemlists]=str2num(script2);
						dummy.totalitemlists++;
					}
					else if(!strcmp("MAX", (char*)script1)) dummy.max=str2num(script2);
					else if(!strcmp("X1", (char*)script1)) dummy.x1=str2num(script2);
					else if(!strcmp("X2", (char*)script1)) dummy.x2=str2num(script2);
					else if(!strcmp("Y1", (char*)script1)) dummy.y1=str2num(script2);
					else if(!strcmp("Y2", (char*)script1)) dummy.y2=str2num(script2);
					else if(!strcmp("MINTIME", (char*)script1)) dummy.mintime=str2num(script2);
					else if(!strcmp("MAXTIME", (char*)script1)) dummy.maxtime=str2num(script2);
				}
			}while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
			i++;
			spawnregion.push_back(dummy);
		}//if Section
	} while( (strcmp((char*)script1, "EOF")));
	closescript();
	clConsole.send("WOLFPACK: %i spawn regions loaded from script.\n",i);
}

static void loadpredefspawnregion(char *name)//Load predefined spawnregion //Regionspawns
{
	char sect[512];
	sprintf(sect, "PREDEFINED_SPAWN %s", name);
	openscript("spawn.scp");
	if(! i_scripts[spawn_script]->find(sect))
	{
		clConsole.send("WARNING: Undefined region spawn %s, check your regions.scp and spawn.scp files.\n",name);
		return;
	}
	spawnregion_st dummy;
	memset(&dummy, 0, sizeof(spawnregion_st));
	unsigned long loopexit=0;
	do {
		read2();
		if(!(strcmp((char*)script1, "NLIST")))
		{
			dummy.npclists[dummy.totalnpclists]=str2num(script2);
			dummy.totalnpclists++;
		}
		else if(!(strcmp("NPC",(char*)script1)))
		{
			dummy.npcs[dummy.totalnpcs]=str2num(script2);
			dummy.totalnpcs++;
		}
		else if(!(strcmp("ILIST",(char*)script1)))
		{
			dummy.itemlists[dummy.totalitemlists]=str2num(script2);
			dummy.totalitemlists++;
		}
		else if(!(strcmp("MINTIME", (char*)script1))) dummy.mintime=str2num(script2);
		else if(!(strcmp("MAXTIME", (char*)script1))) dummy.maxtime=str2num(script2);
		else if(!(strcmp("MAX", (char*)script1))) dummy.max=str2num(script2);

	} while((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	spawnregion.push_back(dummy);
	closescript();
	clConsole.send("WOLFPACK: %s loaded into spawn region #%i.\n",sect,spawnregion.size());
}

void loadregions()//New -- Zippy spawn regions
{
	int i, noregion, l=0, a=0,loopexit=0;
	int rnd1=0,rnd2=0,rnd3=0;
	char sect[512];
	int actgood=-1; // Magius(CHE)

	Script *pScp=i_scripts[regions_script];

	for (i=0;i<256;i++)
	{
		region[i].midilist=0;
		region[i].priv=0;
		region[i].snowchance=0;
		region[i].rainchance=0;
		region[i].name[0]=0;
		noregion=0;
		for (a=0;a<10;a++)
		{
			region[i].guardnum[a]=RandomNum(1000,1001);
		}
		for (a=0;a<100;a++)		// added by Magius(CHE)
		{
			region[i].goodsell[a]=0;
			region[i].goodbuy[a]=0;
			region[i].goodrnd1[a]=0;
			region[i].goodrnd2[a]=0;
		}
		a=0;		// end added by Magius(CHE)

		if (!pScp->Open()) return;
		sprintf(sect, "REGION %i", i);
		if (!pScp->find(sect)) 
		{
			noregion=1;
		}

		loopexit=0;
		do
		{
			pScp->NextLineSplitted();
			if (script1[0]!='}')
			{
				if (!(strcmp("GUARDNUM",(char*)script1)))
				{
					if (a<10)
					{
						region[i].guardnum[a]=str2num(script2);
						a++;
					}
					else
					{
						clConsole.send("ERROR: Region %i has more than 10 'GUARDNUM', The ones after 10 will not be used\n",i);
					}
				}
				if (!(strcmp("NAME",(char*)script1)))
				{
					strcpy(region[i].name,(char*)script2);
					actgood=-1; // Magius(CHE)
				}
				// Dupois - Added April 5, 1999
				// To identify whether this region is escortable or not.
				else if (!(strcmp("ESCORTS",(char*)script1)))
				{
					// Load the region number in the global array of valid escortable regions
					if ( str2num(script2) == 1 )
					{
						// Store the region index into the valid escort region array
						validEscortRegion[escortRegions] = i;
						escortRegions++;
						//clConsole.send( "WOLFPACK: loadregions() %i regions loaded so far\n", escortRegions );
					}
				} // End - Dupois
				else if (!(strcmp("GUARDOWNER",(char*)script1)))	strcpy(region[i].guardowner,(char*)script2);
				else if (!(strcmp("MIDILIST",(char*)script1)))		region[i].midilist=str2num(script2);
				else if (!(strcmp("GUARDED",(char*)script1)))
				{
					if (str2num(script2)) region[i].priv|=0x01;
				}
				else if (!(strcmp("MAGICDAMAGE",(char*)script1)))
				{
					if ((str2num(script2))) region[i].priv|=0x40; // bugfix LB 12-march-
					// changes from 0=magicdamge,1=no magic damage
					// to			1=			 0=
				}
				else if (!(strcmp("NOMAGIC",(char*)script1)))
				{
					if ((str2num(script2))) region[i].priv|=0x80;
				}
				else if (!(strcmp("MARK",(char*)script1)))
				{
					if (str2num(script2)) region[i].priv|=0x02;
				}
				else if (!(strcmp("GATE",(char*)script1)))
				{
					if (str2num(script2)) region[i].priv|=0x04;
				}
				else if (!(strcmp("RECALL",(char*)script1)))
				{
					if (str2num(script2)) region[i].priv|=0x08;
				}
				else if(!(strcmp("OWNERRACE",(char*)script1)))
				{
					region[i].RaceOwner = str2num(script2);
				}
				else if (!(strcmp("SNOWCHANCE", script1)))
				{
					gettokennum(script2, 0);
					rnd1=str2num(gettokenstr);
					gettokennum(script2, 1);
					rnd2=str2num(gettokenstr);
					rnd3=RandomNum(rnd1,rnd2);
					Weather->SnowChance[i]=rnd3;
				}
				else if (!(strcmp("RAINCHANCE", script1)))
				{
					gettokennum(script2, 0);
					rnd1=str2num(gettokenstr);
					gettokennum(script2, 1);
					rnd2=str2num(gettokenstr);
					rnd3=RandomNum(rnd1,rnd2);
					Weather->RainChance[i]=rnd3;
				}
				else if(!(strcmp("WEATHDURATION",script1)))
				{
					gettokennum(script2, 0);
					rnd1=str2num(gettokenstr);
					gettokennum(script2, 1);
					rnd2=str2num(gettokenstr);
					rnd3=RandomNum(rnd1,rnd2);
					Weather->Duration[i]=rnd3*CLOCKS_PER_SEC;
				}
				else if(!(strcmp("WEATHSTART",script1)))
				{
					if(str2num(script2)>0)
					{
						gettokennum(script2, 0);
						rnd1=str2num(gettokenstr);
						gettokennum(script2, 1);
						rnd2=str2num(gettokenstr);
						rnd3=RandomNum(rnd1,rnd2);
						Weather->Check[i]=rnd3;
						Weather->StartTime[i]=uiCurrentTime+(rnd3*CLOCKS_PER_SEC);
						Weather->CurrentRegions++;
					}
					else
					{
						Weather->Check[i]=0;
						Weather->StartTime[i]=0;
						Weather->CurrentRegions++;
					}
				}
				else if (!(strcmp("GOOD", (char*)script1))) // Magius(CHE)
				{
					actgood=str2num(script2);
				}
				else if (!(strcmp("BUYABLE", (char*)script1))) // Magius(CHE)
				{
					if (actgood>-1) region[i].goodbuy[actgood]=str2num(script2);
					else clConsole.send("WOLFPACK.cpp: ERROR in regions.scp. You must write BUYABLE after GOOD <num>!\n");
				}
				else if (!(strcmp("SELLABLE", (char*)script1))) // Magius(CHE)
				{
					if (actgood>-1) region[i].goodsell[actgood]=str2num(script2);
					else clConsole.send("WOLFPACK.cpp: ERROR in regions.scp. You must write SELLABLE after GOOD <num>!\n");
				}
				else if (!(strcmp("RANDOMVALUE", (char*)script1))) // Magius(CHE) (2)
				{
					if (actgood>-1) {
						gettokennum((char*)script2, 0);
						region[i].goodrnd1[actgood]=str2num(gettokenstr);
						gettokennum((char*)script2, 1);
						region[i].goodrnd2[actgood]=str2num(gettokenstr);
						if (region[i].goodrnd2[actgood]<region[i].goodrnd1[actgood])
						{
							clConsole.send("WOLFPACK.cpp: ERROR in regions.scp. You must write RANDOMVALUE NUM2[%i] grater than NUM1[%i].\n",region[i].goodrnd2[actgood],region[i].goodrnd1[actgood]);
							region[i].goodrnd2[actgood]=region[i].goodrnd1[actgood]=0;
						}
					}
					else clConsole.send("WOLFPACK.cpp: ERROR in regions.scp. You must write RANDOMVALUE after GOOD <num>!\n");
				}
				else if (!(strcmp("X1", (char*)script1)))
				{
					location[l].x1=str2num(script2);
				}
				else if (!(strcmp("X2", (char*)script1)))
				{
					location[l].x2=str2num(script2);
				}
				else if (!(strcmp("Y1", (char*)script1)))
				{
					location[l].y1=str2num(script2);
				}
				else if (!(strcmp("Y2", (char*)script1)))
				{
					location[l].y2=str2num(script2);
					location[l].region=i;
					l++;
				}
				else if (!(strcmp("SPAWN", (char*)script1)))
				{
					//pos=ftell(scpfile);
					//closescript();
					scpMark m=pScp->Suspend();
					loadpredefspawnregion((char*)script2);
					pScp->Resume(m);
					//long int pos;
					spawnregion[spawnregion.size()].x1=location[l-1].x1;
					spawnregion[spawnregion.size()].y1=location[l-1].y1;
					spawnregion[spawnregion.size()].x2=location[l-1].x2;
					spawnregion[spawnregion.size()].y2=location[l-1].y2;
					//openscript("regions.scp");
					//fseek(scpfile, pos, SEEK_SET);
					strcpy((char*)script1, "SOMECRAZYREGION"); // To prevent accidental exit of loop.
				}
			}
		}
		while (script1[0]!='}' && !noregion && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
	locationcount=l;
	logoutcount=0;
	//Instalog
	if (!pScp->Open()) return;
	strcpy(sect, "INSTALOG");
	if (!pScp->find(sect)) 
	{
		pScp->Close();
		return;
	}

	loopexit=0;
	do
	{
		pScp->NextLineSplitted();
		//read2();
		if(!(strcmp((char*)script1,"X1"))) logout[logoutcount].x1=str2num(script2);
		else if(!(strcmp((char*)script1,"Y1"))) logout[logoutcount].y1=str2num(script2);
		else if(!(strcmp((char*)script1,"X2"))) logout[logoutcount].x2=str2num(script2);
		else if(!(strcmp((char*)script1,"Y2")))
		{
			logout[logoutcount].y2=str2num(script2);
			logoutcount++;
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();
}

unsigned char calcRegionFromXY(int x, int y)
{
	int i;
	for (i=0;i<locationcount;i++)
	{
		if (location[i].x1<=x && location[i].y1<=y && location[i].x2>=x &&
			location[i].y2>=y)
		{
			return location[i].region;
		}
	}
	
	return 255;

}

void checkregion(int i)
{
	int calcreg, s, j;

	calcreg=calcRegionFromXY(chars[i].pos.x, chars[i].pos.y);
	if (calcreg!=chars[i].region)
	{
		s=calcSocketFromChar(i);
		if (s!=-1)
		{
			if (region[chars[i].region].name[0]!=0)
			{
				sprintf((char*)temp, "You have left %s  %s.",Races[region[chars[i].region].RaceOwner]->RaceName, region[chars[i].region].name);
				sysmessage(s, (char*)temp);
			}
			if (region[calcreg].name[0]!=0)
			{
				sprintf((char*)temp, "You have entered %s %s.",Races[region[calcreg].RaceOwner]->RaceName, region[calcreg].name);
				sysmessage(s, (char*)temp);
			}
			j=strcmp(region[calcreg].guardowner, region[chars[i].region].guardowner);
			if ( (region[calcreg].priv&0x01)!=(region[chars[i].region].priv&0x01) ||
				(region[calcreg].priv&0x01 && j))
			{
				if (region[calcreg].priv&0x01)
				{
					if (region[calcreg].guardowner[0]==0)
					{
						sysmessage(s, "You are now under the protection of the guards.");
					}
					else
					{
						sprintf((char*)temp, "You are now under the protection of %s guards.", region[calcreg].guardowner);
						sysmessage(s,(char*) temp);
					}
				}
				else
				{
					if (region[chars[i].region].guardowner[0]==0)
					{
						sysmessage(s, "You are no longer under the protection of the guards.");
					}
					else
					{
						sprintf((char*)temp, "You are no longer under the protection of %s guards.", region[chars[i].region].guardowner);
						sysmessage(s, (char*)temp);
					}
				}
			}
		}
		chars[i].region=calcreg;
		if (s!=-1) dosocketmidi(s);
		if (indungeon(i)) dolight(s,dungeonlightlevel);
	}
}

///////////////////
// Name:	Continue()
// Purpose:	handles one region/spawner per call, then takes a break
//			to allow for memory reallocation and to avoid lag
// History:	by Duke, 17.12.2000
//
void cRespawn::Continue()
{
	unsigned int i;
	for( i = currentSpawnRegion; i < spawnregion.size(); i++) //New -- Zippy region spawner
	{
		if (doregionspawn(i) == false)	// no more things to spawn in this region
			currentSpawnRegion++;
		return;			// take a break after each region 
	}

	for( i = currentSpawnItem; i < itemcount; i++)	// Item Spawner
	{
		P_ITEM pi = MAKE_ITEMREF_LR(i);
		int j, k, serial, serhash, ci;
		if (pi->type==61)
		{
			k = 0;
			serial = pi->serial;
			serhash = serial%HASHMAX;
			for (ci = 0; ci < spawnsp[serhash].max; ci++)
			{
				j=spawnsp[serhash].pointer[ci];
				if (j!=-1)
				{
					P_ITEM pj = MAKE_ITEMREF_LR(j);
					if (i!=j && pj->pos.x==pi->pos.x && pj->pos.y==pi->pos.y && pj->pos.z==pi->pos.z)
					{
						if (pi->serial==pj->spawnserial)
						{
							k=1;
							break;
						}
					}
				}
			}
			if (k==0)
			{
				Items->AddRespawnItem(i,pi->morex,0);
				currentSpawnItem++;
				return;			// take a break
			}
		}

		if (pi->type==62||pi->type==69||pi->type==125)	// NPC Spawner / Escort Quest spawner(125)
		{
			k=0;
			serial=pi->serial;
			serhash=serial%HASHMAX;
			for (ci=0;ci<cspawnsp[serhash].max;ci++)
			{
				j=cspawnsp[serhash].pointer[ci];
				if (j!=-1)
					if (pi->serial==chars[j].spawnserial)
					{
						k++;
					}
			}

			if (k<pi->amount)	// lord binary, adds spawn amount checking
			{
				int olditemcount=itemcount;
				Npcs->AddRespawnNPC(i,pi->morex,1);
				if (itemcount-olditemcount>300)
				{
					char ttt[222];
					sprintf(ttt,"script npc %d produced %d items. Current limit is 300 !",
								pi->morex,itemcount-olditemcount);
					LogWarning(ttt);
				}
				pi->gatetime=0;
				currentSpawnItem++;
				return;			// take a break
			}
		}
	}
	sysbroadcast("Respawn now complete.");
	respawning=false;	// if we get here, the respawning has finished
}
