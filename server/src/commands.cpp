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
#include "debug.h"
#include "cmdtable.h"
#include "sregions.h"
#include "SndPkg.h"

#undef  DBGFILE
#define DBGFILE "commands.cpp"

// inline command to do targeting - coulda made this a
// macro but this is pretty much as fast...
static inline void _do_target(int s, TARGET_S *ts) {
	target(s, ts->a1, ts->a2, ts->a3, ts->a4, ts->txt);
	return;
}

/* extensively modified 8/2/99 crackerjack@crackerjack.net -
 * see cmdtable.cpp for more details */
void cCommands::Command(UOXSOCKET s, string speech) // Client entred a '/' command like /ADD
{
	int i=9;
	unsigned char *comm;
	unsigned char nonuni[512];
	int y,loopexit=0;

	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if (pc_currchar->unicode)
		cCommands::cmd_offset = 1;
	else
		cCommands::cmd_offset = 1;

	cCommands::command_line = speech;
	cCommands::params = cCommands::command_line.split(" ");
	strcpy((char*)nonuni, speech.c_str());
	strcpy((char*)tbuffer, (char*)nonuni);

	strupr((char*)nonuni);
	cline = (char*) &nonuni[0];
	splitline();
	if (tnum<1)
		return;
	// Let's ignore the command prefix;
	comm = &nonuni[1];

	i=0; y=-1;loopexit=0;
	while((command_table[i].cmd_name)&&(y==-1) && (++loopexit < MAXLOOPS)) {
		if(!(strcmp((char*)command_table[i].cmd_name, (char*)comm))) y=i;
		i++;
	}

	if(y==-1) {
		sysmessage(s, "Unrecognized command.");
		return;
	} else {
		if((pc_currchar->isTrueGM() && !pc_currchar->isGM()) ||		// a restricted GM outside his region(s)
			(pc_currchar->account!=0)&&(command_table[y].cmd_priv_m!=255)&&
			(!(pc_currchar->priv3[command_table[y].cmd_priv_m]&
			(0-0xFFFFFFFF<<command_table[y].cmd_priv_b))))
		{
			sysmessage(s, "Access denied.");
			return;
		}

		switch(command_table[y].cmd_type) {
		case CMD_FUNC:
			(*((CMD_EXEC)command_table[y].cmd_extra)) (s);
			break;
		case CMD_ITEMMENU:
			itemmenu(s, (int)command_table[y].cmd_extra);
			break;
		case CMD_TARGET:
			_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			break;
		case CMD_TARGETX:
			if(tnum==2) {
				addx[s]=makenumber(1);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes one number as an argument.");
			}
			break;
		case CMD_TARGETXY:
			if(tnum==3) {
				addx[s]=makenumber(1);
				addy[s]=makenumber(2);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes two numbers as arguments.");
			}
			break;
		case CMD_TARGETXYZ:
			if(tnum==4) {
				addx[s]=makenumber(1);
				addy[s]=makenumber(2);
				addz[s]=makenumber(3);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes three numbers as arguments.");
			}
			break;
		case CMD_TARGETHX:
			if(tnum==2) {
				addx[s]=hexnumber(1);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes one hex number as an argument.");
			}
			break;
		case CMD_TARGETHXY:
			if(tnum==3) {
				addx[s]=hexnumber(1);
				addy[s]=hexnumber(2);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes two hex numbers as arguments.");
			}
			break;
		case CMD_TARGETHXYZ:
			if(tnum==4) {
				addx[s]=hexnumber(1);
				addy[s]=hexnumber(2);
				addz[s]=hexnumber(3);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes three hex numbers as arguments.");
			}
			break;
		case CMD_TARGETID1:
			if(tnum==2) {
				addid1[s]=makenumber(1);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes one number as an argument.");
			}
			break;
		case CMD_TARGETID2:
			if(tnum==3) {
				addid1[s] = static_cast<unsigned char>(makenumber(1));
				addid2[s] = static_cast<unsigned char>(makenumber(2));
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes two numbers as arguments.");
			}
			break;
		case CMD_TARGETID3:
			if(tnum==4) {
				addid1[s] = static_cast<unsigned char>(makenumber(1));
				addid2[s] = static_cast<unsigned char>(makenumber(2));
				addid3[s] = static_cast<unsigned char>(makenumber(3));
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes three numbers as arguments.");
			}
			break;
		case CMD_TARGETID4:
			if(tnum==5) {
				addid1[s] = static_cast<unsigned char>(makenumber(1));
				addid2[s] = static_cast<unsigned char>(makenumber(2));
				addid3[s] = static_cast<unsigned char>(makenumber(3));
				addid4[s] = static_cast<unsigned char>(makenumber(4));
				//clConsole.send("1: %i 2: %i 3: %i 4: %i\n",addid1[s],addid2[s],addid2[s],addid3[s],addid4[s]);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes four numbers as arguments.");
			}
			break;
		case CMD_TARGETHID1:
			if(tnum==2) {
				addid1[s] = static_cast<unsigned char>(hexnumber(1));
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes one hex number as an argument.");
			}
			break;
		case CMD_TARGETHID2:
			if(tnum==3) {
				addid1[s] = static_cast<unsigned char>(hexnumber(1));
				addid2[s] = static_cast<unsigned char>(hexnumber(2));
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes two hex numbers as arguments.");
			}
			break;
		case CMD_TARGETHID3:
			if(tnum==4) {
				addid1[s] = static_cast<unsigned char>(hexnumber(1));
				addid2[s] = static_cast<unsigned char>(hexnumber(2));
				addid3[s] = static_cast<unsigned char>(hexnumber(3));
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes three hex numbers as arguments.");
			}
			break;
		case CMD_TARGETHID4:
			if(tnum==5) {
				addid1[s] = static_cast<unsigned char>(hexnumber(1));
				addid2[s] = static_cast<unsigned char>(hexnumber(2));
				addid3[s] = static_cast<unsigned char>(hexnumber(3));
				addid4[s] = static_cast<unsigned char>(hexnumber(4));
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes four hex numbers as arguments.");
			}
			break;
		case CMD_TARGETTMP:
			if(tnum==2) {
				tempint[s]=makenumber(1);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes a number as an argument.");
			}
			break;
		case CMD_TARGETHTMP:
			if(tnum==2) {
				tempint[s]=hexnumber(1);
				_do_target(s, (TARGET_S *)command_table[y].cmd_extra);
			} else {
				sysmessage(s, "This command takes a hex number as an argument.");
			}
			break;
		default:
			sysmessage(s, "BUG: Command has a bad command type set!");
			break;
		}
		return;
	}

	sysmessage(s, "BUG: Should never reach end of command() function!");
}

mstring cCommands::GetAllParams(void)
{
	int pos = 0;
	mstring dummy;
	pos = command_line.find_first_of(" ");
	if (pos != mstring::npos)
	{
		dummy = command_line.substr(pos);
		dummy.trim();
	}
	return dummy;
}


void cCommands::MakeShop(int c)
{
	P_CHAR pc_c = MAKE_CHARREF_LR(c);
	pc_c->shop=1;
	if (pc_c->GetItemOnLayer(0x1A) == NULL)
	{
		P_ITEM p1A=Items->SpawnItem(c,1,"#",0,0x2AF8,0,0);
		if(p1A)
		{
			p1A->SetContSerial(pc_c->serial);
			p1A->layer=0x1A;
			p1A->type=1;
			p1A->priv |= 0x02;
		}
	}
	
	if (pc_c->GetItemOnLayer(0x1B) == NULL)
	{
		P_ITEM p1B=Items->SpawnItem(c,1,"#",0,0x2AF8,0,0);
		if(p1B)
		{
			p1B->SetContSerial(pc_c->serial);
			p1B->layer=0x1B;
			p1B->type=1;
			p1B->priv |= 0x02;
		}
	}
	
	if (pc_c->GetItemOnLayer(0x1C) == NULL)
	{
		P_ITEM p1C=Items->SpawnItem(c,1,"#",0,0x2AF8,0,0);
		if(p1C)
		{
			p1C->SetContSerial(pc_c->serial);
			p1C->layer=0x1C;
			p1C->type=1;
			p1C->priv |= 0x02;
		}
	}
}

void cCommands::NextCall(int s, int type)
{
	// Type is the same as it is in showgmqueue()
	
	int i, serial;
	int x = 0;
	
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	
	if (pc_currchar->callnum != 0)
	{
		donewithcall(s, type);
	}
	if (type == 1) // Player is a GM
	{
		for (i = 1; i < MAXPAGES; i++)
		{
			if (gmpages[i].handled == 0)
			{
				serial = gmpages[i].serial;
				P_CHAR pc_player = FindCharBySerial(serial);
				if (pc_player != NULL)
				{
					sysmessage(s, "");
					sprintf((char*)temp, "Transporting to next call: %s", gmpages[i].name.c_str());
					sysmessage(s, (char*)temp);
					sprintf((char*)temp, "Problem: %s.", gmpages[i].reason.c_str());
					sysmessage(s, (char*)temp);
					sprintf((char*)temp, "Serial number %x", gmpages[i].serial);
					sysmessage(s, (char*)temp);
					sprintf((char*)temp, "Paged at %s.", gmpages[i].timeofcall);
					sysmessage(s, (char*)temp);
					gmpages[i].handled = 1;
					pc_currchar->MoveTo(pc_player->pos.x, pc_player->pos.y, pc_player->pos.z);
					pc_currchar->callnum = i;
					teleport(DEREF_P_CHAR(pc_currchar));
					x++;
				}// if
				if (x > 0)
					break;
			}// if
		}// for
		if (x == 0)
			sysmessage(s, "The GM queue is currently empty");
	} // end first IF
	else // Player is only a counselor
	{
		x = 0;
		for (i = 1; i < MAXPAGES; i++)
		{
			if (counspages[i].handled == 0)
			{
				serial = counspages[i].serial;
				P_CHAR pc_player = FindCharBySerial(serial);
				if (pc_player != NULL)
				{
					sysmessage(s, "");
					sprintf((char*)temp, "Transporting to next call: %s", counspages[i].name.c_str());
					sysmessage(s, (char*)temp);
					sprintf((char*)temp, "Problem: %s.", counspages[i].reason.c_str());
					sysmessage(s, (char*)temp);
					sprintf((char*)temp, "Serial number %x", counspages[i].serial);
					sysmessage(s, (char*)temp);
					sprintf((char*)temp, "Paged at %s.", counspages[i].timeofcall);
					sysmessage(s, (char*)temp);
					gmpages[i].handled = 1;
					pc_currchar->MoveTo(pc_player->pos.x, pc_player->pos.y, pc_player->pos.z);
					pc_currchar->callnum = i;
					teleport(DEREF_P_CHAR(pc_currchar));
					x++;
					break;
				}// if
			}// else
			if (x > 0)
				break;
		}// for
	    if (x == 0)
			sysmessage(s, "The Counselor queue is currently empty");
	}// if
}

void cCommands::KillSpawn(int s, int r)  //courtesy of Revana
{
	int killed=0;

	char temp[512];

	r++; // synch with 1-indexed real storage, casue 0 is no region indicator, LB

	if (r<=0 || r>=255) return;

	sysmessage(s,"Killing spawn, this may cause lag...");

	AllCharsIterator iter_char;
	for(iter_char.Begin(); iter_char.GetData() != NULL; iter_char++)
	{
		P_CHAR toCheck = iter_char.GetData();
		if(toCheck->spawnregion==r && !toCheck->free)
		{
			bolteffect(DEREF_P_CHAR(toCheck), true);
			soundeffect2(DEREF_P_CHAR(toCheck), 0x00, 0x29);
			Npcs->DeleteChar(DEREF_P_CHAR(toCheck));
            killed++;
		}
	}

	AllItemsIterator iter_item;
	for(iter_item.Begin(); iter_item.GetData() != NULL; iter_item++)
	{
		P_ITEM toCheck = iter_item.GetData();
		if(toCheck->spawnregion == r && !toCheck->free)
		{			
			Items->DeleItem(DEREF_P_ITEM(toCheck));
            killed++;
		}
	}

	gcollect();
	sysmessage(s, "Done.");
	sprintf(temp, "%i of Spawn %i have been killed.",killed,r-1);
	sysmessage(s, temp);
}

void cCommands::RegSpawnMax (int s, int r) // rewrite LB
{
	int i, spawn;
	unsigned int currenttime=uiCurrentTime;
	char *temps;

	r++;
	if (r<=0 || r>=255) return;

	temps = new char[100];

	spawn = (spawnregion[r].max-spawnregion[r].current);

	sprintf(temps, "Region %d is Spawning %d items/NPCs, this will cause some lag.", r-1, spawn);
	sysbroadcast(temps);

	for(i=1;i<spawn;i++)
	{
		doregionspawn(r);

	}	
	
	spawnregion[r].nexttime=currenttime+(MY_CLOCKS_PER_SEC*60*RandomNum(spawnregion[r].mintime,spawnregion[r].maxtime));	
	sprintf(temps, "Done. %d total NPCs/items spawned in Spawnregion %d.",spawn,r-1);
	sysmessage(s, temps);

	if (temps!=NULL) delete [] temps;
}

void cCommands::RegSpawnNum (int s, int r, int n) // rewrite by LB
{
	int i, spawn=0;
	unsigned int currenttime=uiCurrentTime;
	char *temps;

	r++;
	if (r<=0 || r>=255) return;

	temps = new char[100];

		spawn = (spawnregion[r].max-spawnregion[r].current);
		if (n > spawn) 
		{		
			sprintf(temps, "%d too many for region %d, spawning %d to reach MAX:%d instead.",n, r-1, spawn, spawnregion[r].max);
			sysmessage(s, temps);
			n=spawn;
		}		
		sprintf(temps, "Region %d is Spawning: %d NPCs/items, this will cause some lag.", r-1, spawn);
		sysbroadcast(temps);

		for(i=1;i<spawn;i++)
		{
			doregionspawn(r);
		}//for	
	
		spawnregion[r].nexttime=currenttime+(MY_CLOCKS_PER_SEC*60*RandomNum(spawnregion[r].mintime,spawnregion[r].maxtime));

		sprintf(temps, "Done. %d total NPCs/items spawned in Spawnregion %d.",spawn,r-1);
		sysmessage(s, temps);	
	    if (temps!=NULL) delete [] temps; // fixing memory leak, LB

}//regspawnnum

void cCommands::KillAll(int s, int percent, const char* sysmsg)
{
	sysmessage(s,"Killing all characters, this may cause some lag...");
	sysbroadcast(sysmsg);
	AllCharsIterator iter_char;
	for (iter_char.Begin(); iter_char.GetData() != NULL; iter_char++)
	{
		P_CHAR pc = iter_char.GetData();
		if(!pc->isGM())
		{
			if(rand()%100+1<=percent)
			{
				bolteffect(DEREF_P_CHAR(pc), true);
				soundeffect2(DEREF_P_CHAR(pc), 0x00, 0x29);
				deathstuff(DEREF_P_CHAR(pc));
			}
		}
	}
	sysmessage(s, "Done.");
}

//o---------------------------------------------------------------------------o
//|   Function -  void cpage(int s,char *reason)
//|   Date     -  UnKnown
//|   Programmer  -  UnKnown 
//o---------------------------------------------------------------------------o
//|   Purpose     -  
//o---------------------------------------------------------------------------o
void cCommands::CPage(int s, char *reason) // Help button (Calls Counselor Call Menus up)
{
	int i, a1, a2, a3, a4, x;
	int x2=0;

	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	
	x=0;
	a1=pc_currchar->ser1;
	a2=pc_currchar->ser2;
	a3=pc_currchar->ser3;
	a4=pc_currchar->ser4;
	
	for(i=1;i<MAXPAGES;i++)
	{
		if(counspages[i].handled==1)
		{
			counspages[i].handled=0;
			counspages[i].name = pc_currchar->name;
			counspages[i].reason = reason;
			counspages[i].serial = pc_currchar->serial;
			time_t current_time = time(0);
			struct tm *local = localtime(&current_time);
			sprintf(counspages[i].timeofcall, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
			sprintf((char*)temp,"%s [%d][%d][%d][%d] called at %s, %s",counspages[i].name.c_str(),a1,a2,a3,a4,counspages[i].timeofcall,counspages[i].reason.c_str());
			if(heartbeat) Writeslot((char*)temp);
			pc_currchar->playercallnum=i;
			pc_currchar->pagegm=2;
			x2++;
			break;
		}
	}
	if(x2==0)
	{
		sysmessage(s,"The Counselor Queue is currently full. Contact the shard operator");
		sysmessage(s,"and ask them to increase the size of the queue.");
	}
	else
	{
		if(strcmp(reason,"OTHER"))
		{
			pc_currchar->pagegm=0;
			sprintf((char*)temp, "Counselor Page from %s [%x %x %x %x]: %s",
				pc_currchar->name, a1, a2, a3, a4, reason);
			for (i=0;i<now;i++)
				if (chars[currchar[i]].isCounselor() && perm[i])
				{
					x=1;
					sysmessage(i, (char*)temp);
				}
			if (x==1)
			{
				sysmessage(s, "Available Counselors have been notified of your request.");
			}
			else sysmessage(s, "There was no Counselor available to take your call.");
		}
		else sysmessage(s,"Please enter the reason for your Counselor request");
	}
}

//o---------------------------------------------------------------------------o
//|   Function :  void gmpage(int s,char *reason)
//|   Date     :  Unknown
//|   Programmer  :  Unknown
//o---------------------------------------------------------------------------o
//|   Purpose     :  Help button (Calls GM Call Menus up)
//o---------------------------------------------------------------------------o
void cCommands::GMPage(int s, char *reason)
{
	int i, a1, a2, a3, a4, x=0;
	int x2=0;
	
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	a1=pc_currchar->ser1;
	a2=pc_currchar->ser2;
	a3=pc_currchar->ser3;
	a4=pc_currchar->ser4;
	
	for(i=1;i<MAXPAGES;i++)
	{
		if(gmpages[i].handled==1)
		{
			gmpages[i].handled=0;
			gmpages[i].name = pc_currchar->name;
			gmpages[i].reason = reason;
			gmpages[i].serial = pc_currchar->serial;
			time_t current_time = time(0);
			struct tm *local = localtime(&current_time);
			sprintf(gmpages[i].timeofcall, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
			sprintf((char*)temp,"%s [%d][%d][%d][%d] called at %s, %s",gmpages[i].name.c_str(),a1,a2,a3,a4,gmpages[i].timeofcall,gmpages[i].reason.c_str());
			if(heartbeat) Writeslot((char*)temp);
			pc_currchar->playercallnum=i;
			pc_currchar->pagegm=1;
			x2++;
			break;
		}
	}
	if (x2==0)
	{
		sysmessage(s,"The GM Queue is currently full. Contact the shard operator");
		sysmessage(s,"and ask them to increase the size of the queue.");
	}
	else
	{
		if(strcmp(reason,"OTHER"))
		{
			pc_currchar->pagegm=0;
			sprintf((char*)temp, "Page from %s [%x %x %x %x]: %s",
				pc_currchar->name, a1, a2, a3, a4, reason);
			for (i=0;i<now;i++) if (chars[currchar[i]].isGM() && perm[i])
			{
				x=1;
				sysmessage(i, (char*)temp);
			}
			if (x==1)
			{
				sysmessage(s, "Available Game Masters have been notified of your request.");
			}
			else sysmessage(s, "There was no Game Master available to take your call.");
		}
		else sysmessage(s,"Please enter the reason for your GM request");
	}
}

void cCommands::DyeItem(int s) // Rehue an item
{
	int body,c1,c2,b,k;
	int serial=calcserial(buffer[s][1],buffer[s][2],buffer[s][3],buffer[s][4]);
	P_ITEM pi = FindItemBySerial(serial);
	if (pi != NULL)
	{
			c1=buffer[s][7];
			c2=buffer[s][8];
			
               
			   if(!(dyeall[s]))
               {
				 if ((((c1<<8)+c2)<0x0002) ||
				    	(((c1<<8)+c2)>0x03E9))
				 {
					c1=0x03;
					c2=0xE9;
				 }
			   }
		
           	b=((((c1<<8)+c2)&0x4000)>>14)+((((c1<<8)+c2)&0x8000)>>15);	       
			if (!b)
            {
              pi->color1=c1;
			  pi->color2=c2;
			}

			if (((c1<<8)+c2)==17969)
			{
				pi->color1=c1;
				pi->color2=c2;
			}
			RefreshItem(DEREF_P_ITEM(pi));//AntiChrist
			
			soundeffect( s, 0x02, 0x3e ); // plays the dye sound, LB
			return;
	}

	serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
		if( !(pc_currchar->isGM() ) ) return; // Only gms dye characters
		k=(buffer[s][7]<<8)+buffer[s][8];


		 body=(pc->id1<<8)+pc->id2;
         b=k&0x4000; 

		 if( ( ( k>>8 ) < 0x80 ) && body >= 0x0190 && body <= 0x0193 ) k+= 0x8000;

		 if (b==16384 && (body >=0x0190 && body<=0x03e1)) k=0xf000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

         if (k!=0x8000) 
		 {	
		 
			pc->skin = pc->xskin = k;
			updatechar(DEREF_P_CHAR(pc));
         }
	}
	soundeffect( s, 0x02, 0x3e ); // plays the dye sound, LB
}


void cCommands::SetItemTrigger(int s)
{
	int serial;
	 
	
  serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
  P_ITEM pi = FindItemBySerial(serial);
  if (pi != NULL)
  {
		sysmessage(s,"Item triggered");
		pi->trigger=addx[s];
  }
}

void cCommands::SetTriggerType(int s)
{
	int serial;
	 
	
  serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
  P_ITEM pi = FindItemBySerial(serial);
  if (pi != NULL)
  {
		sysmessage(s,"Trigger type set");
		pi->trigtype=addx[s];
  }
}

void cCommands::SetTriggerWord(int s)
{
	int serial;
	 
	
  serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
  P_CHAR pc = FindCharBySerial(serial);
  if (pc != NULL)
  {
		sysmessage(s,"Trigger word set");
		strcpy(pc->trigword,xtext[s]);
  }
}

void cCommands::AddHere(int s, char z)
{
	int pileable=0;
	short id = (addid1[s]<<8)+addid2[s];
	tile_st tile;
	
	Map->SeekTile(id, &tile);
	if (tile.flag2&0x08) pileable=1;
	
	P_ITEM pi=Items->SpawnItem(currchar[s], 1, "#", pileable, id, 0, 0);
	if(pi)
	{
		P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	
		pi->MoveTo(pc_currchar->pos.x,pc_currchar->pos.y,z);
		pi->doordir=0;
		pi->priv=0;		

		RefreshItem(pi);//AntiChrist
	}
	addid1[s]=0;
	addid2[s]=0;
}


void cCommands::SetNPCTrigger(int s)
{
	int i,serial;
	 
	
  serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
  i = calcCharFromSer( serial );
  if (i!=-1)
  {
		//   if (chars[i].npc)
		//   {
    sysmessage(s,"NPC triggered");
    chars[i].trigger=addx[s];
		//   }else{
		//    sysmessage(s,"You can not trigger Player Characters");
		//   }
  }
}


void cCommands::WhoCommand(int s, int type,int buttonnum)
{
	char sect[512];
	short int length, length2, textlines;
	int k,c;
	unsigned int line, i;
	char menuarray[7*(MAXCLIENT)+50][50];  /** lord binary **/
	char menuarray1[7*(MAXCLIENT)+50][50]; /** the ( IS important !!! **/
	unsigned int linecount=0;
	unsigned int linecount1=0;
	int serial,serhash;
	
	
	k=buttonnum;
	
	serial=whomenudata[buttonnum];
	serhash=serial%HASHMAX;
	c = calcCharFromSer( serial ); // find selected char ...
	if (c==-1) 
	{
		sysmessage(s,"selected character not found");
		return;
	}
	P_CHAR pc_c = MAKE_CHARREF_LR(c);
	
	//--static pages
	strcpy(menuarray[linecount++], "nomove");
	strcpy(menuarray[linecount++], "noclose");
	strcpy(menuarray[linecount++], "page 0");
	strcpy(menuarray[linecount++], "resizepic 0 0 5120 260 280");    //The background
	strcpy(menuarray[linecount++], "button 20 240 2130 2129 1 0 1"); //OKAY
	strcpy(menuarray[linecount++], "text 20 10 300 0");           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
	strcpy(menuarray[linecount++], "text 20 30 300 1");
	
	//--Command Button Page 
	strcpy(menuarray[linecount++], "page 1");
	strcpy(menuarray[linecount++], "text 20 60 300 2");	//goto text
	strcpy(menuarray[linecount++], "button 150 60 1209 1210 1 0 200"); //goto button
	strcpy(menuarray[linecount++], "text 20 80 300 3");	//gettext
	strcpy(menuarray[linecount++], "button 150 80 1209 1210 1 0 201"); //get button
	strcpy(menuarray[linecount++], "text 20 100 300 4");	//Jail text
	strcpy(menuarray[linecount++], "button 150 100 1209 1210 1 0 202"); //Jail button
	strcpy(menuarray[linecount++], "text 20 120 300 5");	//Release text
	strcpy(menuarray[linecount++], "button 150 120 1209 1210 1 0 203"); //Release button
	strcpy(menuarray[linecount++], "text 20 140 300 6");	//Kick user text
	strcpy(menuarray[linecount++], "button 150 140 1209 1210 1 0 204"); //kick button
	strcpy(menuarray[linecount++], "text 20 180 300 7");

	length=21;
	length2=1;
	
	for(line=0;line<linecount;line++)
	{
		if (strlen(menuarray[line])==0)
			break;
		{
			length+=strlen(menuarray[line])+4;
			length2+=strlen(menuarray[line])+4;
		}
	}
	
	length+=3;
	textlines=0;
	line=0;
	
	sprintf(menuarray1[linecount1++], "User %i selected (account %i)",buttonnum,pc_c->account);
	sprintf(menuarray1[linecount1++], "Name: %s",pc_c->name);   
	sprintf(menuarray1[linecount1++], "Goto Character:");
	sprintf(menuarray1[linecount1++], "Get Character:");
	sprintf(menuarray1[linecount1++], "Jail Character:");
	sprintf(menuarray1[linecount1++], "Release Character:");
	sprintf(menuarray1[linecount1++], "Kick Character:");
	sprintf(menuarray1[linecount1++], "Serial#[%i %i %i %i]",pc_c->ser1,pc_c->ser2,chars[currchar[k]].ser3,pc_c->ser4);   
	
	
	for(line=0;line<linecount1;line++)
	{
		if (strlen(menuarray1[line])==0)
			break;
		length+=strlen(menuarray1[line])*2 +2;
		textlines++;
	}
	
	gump1[1]=length>>8;
	gump1[2]=length%256;
	gump1[7]=0;
	gump1[8]=0;
	gump1[9]=0;
	gump1[10]=type; // Gump Number
	gump1[19]=length2>>8;
	gump1[20]=length2%256;
	Xsend(s, gump1, 21);
	
	for(line=0;line<linecount;line++)
	{
		sprintf(sect, "{ %s }", menuarray[line]);
		Xsend(s, sect, strlen(sect));
	}
	
	gump2[1]=textlines>>8;
	gump2[2]=textlines%256;
	
	Xsend(s, gump2, 3);
	
	for(line=0;line<linecount1;line++)
	{
		if (strlen(menuarray1[line])==0)
			break;
		{
			gump3[0]=strlen(menuarray1[line])>>8;
			gump3[1]=strlen(menuarray1[line])%256;
			Xsend(s, gump3, 2);
			gump3[0]=0;
			for (i=0;i<strlen(menuarray1[line]);i++)
			{
				gump3[1]=menuarray1[line][i];
				Xsend(s, gump3, 2);
			}
		}
	}
}

void cCommands::MakePlace(int s, int i) // Decode a teleport location number into X/Y/Z
{
	int x = 0, y = 0, z = 0, loopexit = 0;
	
	openscript("location.scp");
	sprintf((char*)temp, "LOCATION %i", i);
	if (i_scripts[location_script]->find((char*)temp))
	{
		do
		{
			read2();
			if (!strcmp((char*)script1,"X"))
			{
				x = str2num(script2);
			}
			else if (!strcmp((char*)script1,"Y"))
			{
				y = str2num(script2);
			}
			else if (!strcmp((char*)script1,"Z"))
			{
				z = str2num(script2);
			}
		}
		while ( (strcmp((char*)script1,"}")) && (++loopexit < MAXLOOPS) );
	}
	addx[s] = x;
	addy[s] = y;
	addz[s] = z;
	closescript();
}


void cCommands::DupeItem(int s, int i, int amount)
{
	int p, c;
	if (items[i].corpse) return;
	p=packitem(currchar[s]);
	if(p==-1) return;//AntiChrist

	c=Items->MemItemFree();
	items[c].Init(0);
	memcpy(&items[c], &items[i], sizeof(cItem));
	items[c].SetSerial(itemcount2);
	itemcount2++;
	
	items[c].SetContSerial(items[p].serial);
	items[c].SetOwnSerial(items[i].ownserial);
	items[c].SetSpawnSerial(items[i].spawnserial);
	items[c].layer=0;	// it's created in a backpack
	items[c].amount=amount;
	
	RefreshItem(c);//AntiChrist
}

void cCommands::ShowGMQue(int s, int type) // Shows next unhandled call in the GM queue
{
	
	// Type is 0 if it is a Counselor doing the command (or a GM doing /cq) and 1 if it is a GM
	
	int i;
	int x=0;
	
	if(type==1) //Player is a GM
	{
		for(i=1;i<MAXPAGES;i++)
		{
			if (gmpages[i].handled==0)
			{
				if(x==0)
				{
					sysmessage(s,"");
					sprintf((char*)temp,"Next unhandled page from %s", gmpages[i].name.c_str());
					sysmessage(s,(char*)temp);
					sprintf((char*)temp,"Problem: %s.", gmpages[i].reason.c_str());
					sysmessage(s,(char*)temp);
					sprintf((char*)temp,"Serial number %x", gmpages[i].serial);
					sysmessage(s,(char*)temp);
					sprintf((char*)temp,"Paged at %s.", gmpages[i].timeofcall);
					sysmessage(s,(char*)temp);
				}
				x++;
			}
		}
		if (x>0)
		{
			sprintf((char*)temp,"Total pages in queue: %i",x);
			sysmessage(s,"");
			sysmessage(s,(char*)temp);
		}
		else sysmessage(s,"The GM queue is currently empty");
	} //end of first if
	else //Player is a counselor so show counselor queue
	{
		for(i=1;i<MAXPAGES;i++)
		{
			if (counspages[i].handled==0)
			{
				if(x==0)
				{
					sysmessage(s,"");
					sprintf((char*)temp,"Next unhandled page from %s", counspages[i].name.c_str());
					sysmessage(s,(char*)temp);
					sprintf((char*)temp,"Problem: %s.", counspages[i].reason.c_str());
					sysmessage(s,(char*)temp);
					sprintf((char*)temp,"Serial number %x", counspages[i].serial);
					sysmessage(s,(char*)temp);
					sprintf((char*)temp,"Paged at %s.", counspages[i].timeofcall);
					sysmessage(s,(char*)temp);
				}
				x++;
			}
		}
		if (x>0)
		{
			sprintf((char*)temp,"Total pages in queue: %i",x);
			sysmessage(s,"");
			sysmessage(s,(char*)temp);
		}
		else sysmessage(s,"The Counselor queue is currently empty");
	}
}
// new wipe function, basically it prints output on the console when someone wipes so that
// if a malicious GM wipes the world you know who to blame

void cCommands::Wipe(int s)
{
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	
	clConsole.send("WOLFPACK: %s has initiated an item wipe\n",pc_currchar->name);
	
	P_ITEM pi;
	AllItemsIterator aii;
	for(pi=aii.First(); (pi=aii.Next())!=aii.End(); )
	{
		if(pi->isInWorld() && pi->wipe==0)
		{
			Items->DeleItem(pi);
		}
	}
	sysbroadcast("All items have been wiped."); 
}

void cCommands::Possess(int s) 
{
	P_CHAR pPos = FindCharBySerPtr(buffer[s]+7);	// char to posess
	if (!pPos) return;
	
	if (pPos->shop)
	{
		sysmessage(s,"You cannot use shopkeepers.");
		return;
	}
	if (pPos->isPlayer())
	{
		sysmessage( s, "You can only possess NPCs." );
		return;
	}

	unsigned char tmp;
	P_CHAR pc_currchar  = MAKE_CHARREF_LR(currchar[s]);

	if( pPos->npc == 17 ) // Char's old body
	{
		tmp = pPos->getPriv();
		pPos->setPriv(pc_currchar->getPriv());
		pc_currchar->setPriv(tmp);
		
		tmp = pPos->priv2;
		pPos->priv2 = pc_currchar->priv2;
		pc_currchar->priv2 = tmp;

		tmp = pPos->commandLevel;
		pPos->commandLevel = pc_currchar->commandLevel;
		pc_currchar->commandLevel = tmp;

		for(int i = 0; i < 7; i++)
		{ 
           int tempi = pPos->priv3[i]; 
           pPos->priv3[i] = pc_currchar->priv3[i]; 
           pc_currchar->priv3[i] = tempi;
		}

		pPos->npc = 0;
		pc_currchar->npc = 1;
		pc_currchar->account = -1;
		currchar[s] = DEREF_P_CHAR(pPos);
		Network->startchar( s );
		sysmessage( s, "Welcome back to your old body." );
	}
	else if( pPos->isNpc() )
	{
		tmp = pPos->getPriv();
		pPos->setPriv(pc_currchar->getPriv());
		pc_currchar->setPriv(tmp);
		
		tmp = pPos->priv2;
		pPos->priv2 = pc_currchar->priv2;
		pc_currchar->priv2 = tmp;
		
		tmp = pPos->commandLevel;
		pPos->commandLevel = pc_currchar->commandLevel;
		pc_currchar->commandLevel = tmp;

		for(int i = 0; i < 7; i++)
		{ 
           int tempi = pPos->priv3[i]; 
           pPos->priv3[i] = pc_currchar->priv3[i]; 
           pc_currchar->priv3[i] = tempi;
		}
		
		pPos->npc = 0;
		pPos->account = pc_currchar->account;
		pc_currchar->npc = 17;
		pc_currchar->npcWander = 0;
		currchar[s] = DEREF_P_CHAR(pPos);
		Network->startchar( s );
		sprintf((char*)temp,"Welcome to %s's body!", pPos->name );
		sysmessage(s, (char*)temp);
	}
	else
		sysmessage( s, "Possession error %s." );
}







