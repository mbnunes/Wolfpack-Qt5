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
#include "srvparams.h"
#include "mapstuff.h"
#include "network.h"

#undef  DBGFILE
#define DBGFILE "sregions.cpp"

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
					//Weather->SnowChance[i]=rnd3;
				}
				else if (!(strcmp("RAINCHANCE", script1)))
				{
					gettokennum(script2, 0);
					rnd1=str2num(gettokenstr);
					gettokennum(script2, 1);
					rnd2=str2num(gettokenstr);
					rnd3=RandomNum(rnd1,rnd2);
					//Weather->RainChance[i]=rnd3;
				}
				else if(!(strcmp("WEATHDURATION",script1)))
				{
					gettokennum(script2, 0);
					rnd1=str2num(gettokenstr);
					gettokennum(script2, 1);
					rnd2=str2num(gettokenstr);
					rnd3=RandomNum(rnd1,rnd2);
					//Weather->Duration[i]=rnd3*CLOCKS_PER_SEC;
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
						//Weather->Check[i]=rnd3;
						//Weather->StartTime[i]=uiCurrentTime+(rnd3*CLOCKS_PER_SEC);
						//Weather->CurrentRegions++;
					}
					else
					{
						//Weather->Check[i]=0;
						//Weather->StartTime[i]=0;
						//Weather->CurrentRegions++;
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

void checkregion(P_CHAR pc_i)
{
	int calcreg, s, j;

	////////////////////////////////////////
	/// "UNKOWNDIMENSION sysmessages"
	//// using color (!=0) is essential within entire function 
	///  if not 3d clients >=3.0.6g die
	///  sysmessage(s, "xxx") does use color 0 !
	//// you can change 0x37 to your liking, but not to 0
	/////////////////////////////////////////////////////////////////////

	calcreg=calcRegionFromXY(pc_i->pos.x, pc_i->pos.y);
	if (calcreg!=pc_i->region)
	{
		s = calcSocketFromChar(pc_i);
		if (s!=-1)
		{
			if (region[pc_i->region].name[0]!=0)
			{
				sprintf((char*)temp, "You have left %s.",region[pc_i->region].name);
				sysmessage(s, 0x37, (char*)temp);
			}
			if (region[calcreg].name[0]!=0)
			{
				sprintf((char*)temp, "You have entered %s.",region[calcreg].name);
				sysmessage(s, 0x37, (char*)temp);
			}
			j=strcmp(region[calcreg].guardowner, region[pc_i->region].guardowner);
			if ( (region[calcreg].priv&0x01)!=(region[pc_i->region].priv&0x01) ||
				(region[calcreg].priv&0x01 && j))
			{
				if (region[calcreg].priv&0x01)
				{
					if (region[calcreg].guardowner[0]==0)
					{
						sysmessage(s, 0x37, "You are now under the protection of the guards.");
					}
					else
					{
						sprintf((char*)temp, "You are now under the protection of %s guards.", region[calcreg].guardowner);
						sysmessage(s, 0x37, (char*) temp);
					}
				}
				else
				{
					if (region[pc_i->region].guardowner[0]==0)
					{
						sysmessage(s, 0x37, "You are no longer under the protection of the guards.");
					}
					else
					{
						sprintf((char*)temp, "You are no longer under the protection of %s guards.", region[pc_i->region].guardowner);
						sysmessage(s, 0x37, (char*)temp);
					}
				}
			}
		}
		pc_i->region=calcreg;
		if (s!=-1) dosocketmidi(s);
		if (indungeon(pc_i)) 
			dolight( s, SrvParams->dungeonLightLevel() );
	}
}

