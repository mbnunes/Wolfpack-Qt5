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
// Name:	SndPkg.cpp
// Purpose: functions that send packages to the Client
// History:	cut from wolfpack.cpp by Duke, 25.10.00
// Remarks:	not necessarily ALL those functions
//

#include "wolfpack.h"
#include "utilsys.h"
#include "debug.h"
#include "basics.h"
#include "SndPkg.h"
#include "itemid.h"
#include "guildstones.h"
#include "combat.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "network.h"
#include "classes.h"
#include "territories.h"
#include "wpdefmanager.h"
#include "skills.h"

#include "network/uotxpackets.h"
#include "network/uosocket.h"

#undef  DBGFILE
#define DBGFILE "SndPkg.cpp"


void sysbroadcast( const char *txt ) // System broadcast in bold text
{
	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		socket->sysMessage( txt, 0x84d, 0 );
}

void sysmessage(UOXSOCKET s, const QString& txt)
{
	sysmessage(s, (char*)txt.latin1());
}

void sysmessage(UOXSOCKET s, short color, const QString& txt)
{
	sysmessage(s, color, (char*)txt.latin1());
}

void sysmessage(UOXSOCKET s, char *txt, ...) // System message (In lower left corner)
{
	qWarning("sendmessage disabled, use cUOSocket::sendMessage() instead");
}

void sysmessage(UOXSOCKET s, short color, char *txt, ...) // System message (In lower left corner)
{
	qWarning("sendmessage disabled, use cUOSocket::sendMessage() instead");
}

void wearIt(const UOXSOCKET s, const P_ITEM pi)
{
	qWarning("wearIt() disabled");
/*	LongToCharPtr(pi->serial,wearitem+1);
	ShortToCharPtr(pi->id(),wearitem+5);
	wearitem[8]=pi->layer();
	LongToCharPtr(pi->contserial,wearitem+9);
	ShortToCharPtr(pi->color(), &wearitem[13]);
	Xsend(s, wearitem, 15);
*/
}



void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop)
{//AntiChrist
	char effect[29];
	int i, j;
	for (i=0;i<29;i++)
	{
		effect[i]=0;
	}
	effect[0]=0x70; // Effect message
	effect[1]=0x02; // Stay at x, y, z effect
	//[2] to [9] are	not applicable here.
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=x>>8;
	effect[13]=x%256;
	effect[14]=y>>8;
	effect[15]=y%256;
	effect[16]=z;
	//[17] to [21] are not applicable here.
	effect[22]=speed;
	effect[23]=loop; // 0 is really long.	1 is the shortest.
	//[24] to [25] are not applicable here.
	effect[26]=1; // LB possible client crashfix
	effect[27]=0;

	for (j=0;j<now;j++)
	{
		if (perm[j])
		{
			if(abs(currchar[j]->pos.x-x)<=VISRANGE && abs(currchar[j]->pos.y-y)<=VISRANGE) 
				Xsend(j, effect, 28);
		}

	}
}

void teleport(P_CHAR pc) // Teleports character to its current set coordinates
{
	pc->resend( true );
	
	if( pc->socket() )
		pc->socket()->resendWorld( false );

	cAllTerritories::getInstance()->check( pc );
}


void skillwindow(int s) // Opens the skills list, updated for client 1.26.2b by LB
{
	int i;
	unsigned char skillstart[5]="\x3A\x01\x5d\x00"; // hack for that 3 new skills+1.26.2 client, LB 4'th dec 1999
	unsigned char skillmid[8] = "\x00\x00\x00\x00\x00\x00\x00"; // changed for 1.26.2 clients [size 7 insted of 4]
	unsigned char skillend[3]="\x00\x00";
	char x;

	P_CHAR pc_currchar = currchar[s];

	Xsend(s, skillstart, 4);
	for (i=0;i<TRUESKILLS;i++)
	{
		Skills->updateSkillLevel(pc_currchar, i);
		skillmid[1]=i+1;
		skillmid[2]=pc_currchar->skill(i)>>8;
		skillmid[3]=pc_currchar->skill(i)%256;
		skillmid[4]=pc_currchar->baseSkill(i)>>8;
		skillmid[5]=pc_currchar->baseSkill(i)%256;

		x=pc_currchar->lockSkill(i);
		if (x!=0 && x!=1 && x!=2) x=0;
		skillmid[6]=x; // leave it unlocked, regardless
		Xsend(s, skillmid, 7);
	}
	Xsend(s, skillend, 2);
}

void weblaunch(int s, char *txt) // Direct client to a web page
{
	int l;
	char launchstr[4]="\xA5\x00\x00";

	sysmessage(s, tr("Launching your web browser. Please wait...").latin1());
	l=strlen(txt)+4;
	launchstr[1]=l>>8;
	launchstr[2]=l%256;
	Xsend(s, launchstr, 3);
	Xsend(s, txt, strlen(txt)+1);
}

void broadcast(int s) // GM Broadcast (Done if a GM yells something)
{
}

void npctalk_runic(int s, P_CHAR pc_npc, const char *txt,char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (pc_npc == NULL || s==-1) return; //lb

	if (antispam)
	{
		if (pc_npc->antispamtimer()<uiCurrentTime)
		{
			pc_npc->setAntispamtimer(uiCurrentTime+MY_CLOCKS_PER_SEC*10);
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
	/*	tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		LongToCharPtr(pc_npc->serial,  &talk[3]);
		ShortToCharPtr(pc_npc->serial, &talk[7]);
		talk[9]=0;

		// color here

		talk[10]=0;
		talk[11]=1; // black

		// big problems with contrast/readability, plz find out a better one, LB !

		talk[12]=0;
		talk[13]=8;

		Xsend(s, talk, 14);
		Xsend(s, (void*)pc_npc->name.latin1(), 30);
		Xsend(s, txt, strlen(txt)+1);*/
	}
}

void npcemote(int s, P_CHAR pc_npc, const char *txt, char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (antispam)
	{
		if (pc_npc->antispamtimer()<uiCurrentTime)
		{
			pc_npc->setAntispamtimer(uiCurrentTime+MY_CLOCKS_PER_SEC*10);
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
		pc_npc->emote( txt );
	}
}


void movingeffect(P_CHAR pc_source, P_CHAR pc_dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt, move_st *str, bool skip_old )
{
/*	
	unsigned char effect[29];
	int j;

	if ( pc_source == NULL || pc_dest == NULL) return;

	if (!skip_old)
	{
	   effect[0]=0x70; // Effect message
	   effect[1]=0x00; // Moving effect
	   LongToCharPtr(pc_source->serial, &effect[2]);
	   LongToCharPtr(pc_dest->serial,   &effect[6]);
	   effect[10]=eff1;// Object id of the effect
	   effect[11]=eff2;
	   effect[12]=pc_source->pos.x>>8;
	   effect[13]=pc_source->pos.x%256;
	   effect[14]=pc_source->pos.y>>8;
	   effect[15]=pc_source->pos.y%256;
	   effect[16]=pc_source->pos.z;
	   effect[17]=pc_dest->pos.x>>8;
	   effect[18]=pc_dest->pos.x%256;
	   effect[19]=pc_dest->pos.y>>8;
	   effect[20]=pc_dest->pos.y%256;
	   effect[21]=pc_dest->pos.z;
	   effect[22]=speed;
	   effect[23]=loop; // 0 is really long. 1 is the shortest.
	   effect[24]=0; // This value is unknown
	   effect[25]=0; // This value is unknown
	   effect[26]=0; //1; // LB, potential crashfix
	   effect[27]=explode; // This value is used for moving effects that explode on impact.
	}


	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {
	   for (j=0;j<now;j++)
	   {
		 if ( (inrange1p(currchar[j],pc_source))&&(inrange1p(currchar[j],pc_dest))&&(perm[j]))
		 {
			Xsend(j, effect, 28);
		 }
	   }
	   return;
	} 
	else
	{
		// UO3D effect -> let's check which client can see it
	   for (j=0;j<now;j++)
	   {
		 if ( (inrange1p(currchar[j],pc_source))&&(inrange1p(currchar[j],pc_dest))&&(perm[j]))
		 {
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd 
			 {
				 Xsend(j, effect, 28);

			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
			 {

				movingeffectUO3D(pc_source, pc_dest, str);			
				Xsend(j, particleSystem, 49);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
		 }
	   }
	}		
*/
}

void bolteffect(P_CHAR pc_player, bool UO3DonlyEffekt, bool skip_old )
{
/*	unsigned char effect[29] = {0,};
	int  j;

	if ( pc_player == NULL )
		return;

	if (!skip_old)
	{
//	  memset(&effect[0], 0, 29);	
	  effect[0]=0x70; // Effect message
	  effect[1]=0x01; // Bolt effect
	  LongToCharPtr(pc_player->serial, &effect[2]);
	  //[6] to [11] are not applicable here.
	  effect[12]=pc_player->pos.x>>8;
	  effect[13]=pc_player->pos.x%256;
	  effect[14]=pc_player->pos.y>>8;
	  effect[15]=pc_player->pos.y%256;
	  effect[16]=pc_player->pos.z;
	  //[17] to [27] are not applicable here.
	  effect[26]=1; // LB possible client crashfix
	  effect[27]=0;
	}

	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {
	   for (j=0;j<now;j++)
	   {
		 if ((inrange1p(currchar[j],pc_player))&&(perm[j]))
		 {			
			Xsend(j, effect, 28);
		 }
	   }
	   return;
	} 
	else
	{
		// UO3D effect -> let's check which client can see it
	   for (j=0;j<now;j++)
	   {
		 if ((inrange1p(currchar[j],pc_player))&&(perm[j]))
		 {
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd 
			 {
				 Xsend(j, effect, 28);

			 } 
			 else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
			 {

				bolteffectUO3D(pc_player);			
				Xsend(j, particleSystem, 49);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
		 }
	   }
	}		*/
}



void bolteffect2(P_CHAR pc_player,char a1,char a2)	// experimenatal, lb
{
/*	unsigned char effect[29] = {0,};
	int j,x2,x,y2,y;
	if ( pc_player == NULL )
		return;

	effect[0]=0x70; // Effect message
	effect[1]=0x00; // effect from source to dest
	LongToCharPtr(pc_player->serial, &effect[2]);

	effect[10]=a1;
	effect[11]=a2;

	y=rand()%36;
	x=rand()%36;

	if (rand()%2==0) x=x*-1;
	if (rand()%2==0) y=y*-1;
	x2=pc_player->pos.x+x;
	y2=pc_player->pos.y+y;
	if (x2<0) x2=0;
	if (y2<0) y2=0;
	if (x2>6144) x2=6144;
	if (y2>4096) y2=4096;

	// ConOut("bolt: %i %i %i %i %i %i\n",x2,y2,pc_player->pos.x,pc_player->pos.y,x,y);

	effect[12]=pc_player->pos.x>>8; // source coordinates
	effect[13]=pc_player->pos.x%256;
	effect[14]=pc_player->pos.y>>8;
	effect[15]=pc_player->pos.y%256;
	effect[16]=0;

	effect[17]=x2>>8;	//target coordiantes
	effect[18]=x2%256;
	effect[19]=y2>>8;
	effect[20]=y2%256;
	effect[21]=127;

	//[22] to [27] are not applicable here.

	effect[26]=1; // client crash bugfix
	effect[27]=0;

	for (j=0;j<now;j++)
	{
		if ((inrange1p(currchar[j],pc_player))&&(perm[j]))
		{
			Xsend(j, effect, 28);
		}
	}*/
}

//	- Movingeffect3 is used to send an object from a char
//    to another object (like purple potions)
void movingeffect3(P_CHAR pc_source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
/*	unsigned char effect[29] = {0,};
	int j;

	if ( pc_source == NULL ) return;

	effect[0]=0x70; // Effect message
	effect[1]=0x00; // Moving effect
	LongToCharPtr(pc_source->serial, &effect[2]);
	effect[6]=0;
	effect[7]=0;
	effect[8]=0;
	effect[9]=0;
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=pc_source->pos.x>>8;
	effect[13]=pc_source->pos.x%256;
	effect[14]=pc_source->pos.y>>8;
	effect[15]=pc_source->pos.y%256;
	effect[16]=pc_source->pos.z;
	effect[17]=x>>8;
	effect[18]=x%256;
	effect[19]=y>>8;
	effect[20]=y%256;
	effect[21]=z;
	effect[22]=speed;
	effect[23]=loop; // 0 is really long.  1 is the shortest.
	effect[24]=0; // This value is unknown
	effect[25]=0; // This value is unknown
	effect[26]=0; // This value is unknown
	effect[27]=explode; // This value is used for moving effects that explode on impact.
	for (j=0;j<now;j++)
	{   // - If in range of source person or destination position and online send effect
		//if ((inrange1p(currchar[j],source) || inrange2(j,dest)) && (perm[j]))
		//{
			Xsend(j, effect, 28);
		//}
	}*/
}

// staticeffect3 is for effects on items
void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode)
{
/*	char effect[29] = {0,};
	int j;
//	memset (&effect, 0, 29);

	effect[0]=0x70; // Effect message
	effect[1]=0x02; // Static effect
	//[6] to [9] are the target ser, not applicable here.
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=x>>8;
	effect[13]=x%256;
	effect[14]=y>>8;
	effect[15]=y%256;
	effect[16]=z;
	effect[17]=x>>8;
	effect[18]=x%256;
	effect[19]=y>>8;
	effect[20]=y%256;
	effect[21]=z;
	//[17] to [21] are the target's position, not applicable here.
	effect[22]=speed;
	effect[23]=loop; // 0 is really long.  1 is the shortest.
	effect[24]=0; // This value is unknown
	effect[25]=0; // This value is unknown
	effect[26]=1; // LB changed to 1
	effect[27]=explode; // This value is used for moving effects that explode on impact.
	for (j=0;j<now;j++)
	{  // if inrange of effect and online send effect
		if (inVisRange(x, y, currchar[j]->pos.x, currchar[j]->pos.y))
		{
			Xsend(j, effect, 28);
		}
	}*/
}

void movingeffect3(P_CHAR pc_source, P_CHAR pc_dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode,unsigned char unk1,unsigned char unk2,unsigned char ajust,unsigned char type)
{
/*	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
	unsigned char effect[29];
	int j;

	if (pc_source == NULL || pc_dest == NULL)
		return;

	effect[0]=0x70; // Effect message
	effect[1]=type; // Moving effect
	LongToCharPtr(pc_source->serial, &effect[2]);
	LongToCharPtr(pc_dest->serial,   &effect[6]);
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=pc_source->pos.x>>8;
	effect[13]=pc_source->pos.x%256;
	effect[14]=pc_source->pos.y>>8;
	effect[15]=pc_source->pos.y%256;
	effect[16]=pc_source->pos.z;
	effect[17]=pc_dest->pos.x>>8;
	effect[18]=pc_dest->pos.x%256;
	effect[19]=pc_dest->pos.y>>8;
	effect[20]=pc_dest->pos.y%256;
	effect[21]=pc_dest->pos.z;
	effect[22]=speed;
	effect[23]=loop; // 0 is really long. 1 is the shortest.
	effect[24]=unk1; // This value is unknown
	effect[25]=unk2; // This value is unknown
	effect[26]=ajust; // LB, potential crashfix
	effect[27]=explode; // This value is used for moving effects that explode on impact.
	for (j=0;j<now;j++)
	{
		if ((inrange1p(currchar[j],pc_source))&&(inrange1p(currchar[j],pc_dest))&&(perm[j]))
		{
			Xsend(j, effect, 28);
		}
	}*/
}



//	- Movingeffect2 is used to send an object from a char
//	to another object (like purple potions)
void movingeffect2(P_CHAR pc_source, P_ITEM dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
/*	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
	unsigned char effect[29] = {0,};
	int j;
	const P_ITEM pi = dest;	// on error return
	if ( pc_source == NULL )
		return;

	effect[0]=0x70; // Effect message
	effect[1]=0x00; // Moving effect
	LongToCharPtr(pc_source->serial, &effect[2]);
	LongToCharPtr(pi->serial,        &effect[6]);
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=pc_source->pos.x>>8;
	effect[13]=pc_source->pos.x%256;
	effect[14]=pc_source->pos.y>>8;
	effect[15]=pc_source->pos.y%256;
	effect[16]=pc_source->pos.z;
	effect[17]=pi->pos.x>>8;
	effect[18]=pi->pos.x%256;
	effect[19]=pi->pos.y>>8;
	effect[20]=pi->pos.y%256;
	effect[21]=pi->pos.z;
	effect[22]=speed;
	effect[23]=loop; // 0 is really long. 1 is the shortest.
	effect[24]=0; // This value is unknown
	effect[25]=0; // This value is unknown
	effect[26]=0; //1; // LB potential crashfix
	effect[27]=explode; // This value is used for moving effects that explode on impact.
	for (j=0;j<now;j++)
	{	// - If in range of source person or destination position and online send effect
		if ((inrange1p(currchar[j],pc_source) || inrange2(j,pi)) && (perm[j]))
		{
			Xsend(j, effect, 28);
		}
	}*/
}

void updateskill(int s, int skillnum) // updated for client 1.26.2b by LB
{
/*	char update[11];
	char x;
	
	P_CHAR pc_currchar = currchar[s];
	
	update[0] = 0x3A; // Skill Update Message
	update[1] = 0x00; // Length of message
	update[2] = 0x0B; // Length of message
	update[3] = '\xFF'; // single list
	
	update[4] = 0x00;
	update[5] = (char)skillnum;
	update[6] = pc_currchar->skill(skillnum) >> 8;
	update[7] = pc_currchar->skill(skillnum)%256;
	update[8] = pc_currchar->baseSkill(skillnum) >> 8;
	update[9] = pc_currchar->baseSkill(skillnum)%256;
	x = pc_currchar->lockSkill(skillnum);
	if (x != 0 && x != 1 && x != 2) 
		x = 0;
	update[10] = x;
	
	// CRASH_IF_INVALID_SOCK(s);
	
	Xsend(s, update, 11);*/
}

void deathaction(P_CHAR pc, P_ITEM pi_x) // Character does a certain action
{
/*	int i;
	unsigned char deathact[14]="\xAF\x01\x02\x03\x04\x01\x02\x00\x05\x00\x00\x00\x00";

	if (pi_x == NULL || pc == NULL)
		return;

	LongToCharPtr(pc->serial, &deathact[1]);
	LongToCharPtr(pi_x->serial,deathact+5);
	
	for (i=0;i<now;i++) 
		if ((inrange1p(pc, currchar[i]))&&(perm[i]) && (currchar[i]!=pc)) 
			Xsend(i, deathact, 13);*/
	
}

void deathmenu(int s) // Character sees death menu
{
	char testact[3]="\x2C\x00";
	Xsend(s, testact, 2);
}

void impowncreate(int s, P_CHAR pc, int z) //socket, player to send
{
	qWarning("impowncreate() function is disabled, use cChar:: appropriate methods instead");
/*	int k;
	unsigned char oc[1024];

    if ( pc == NULL )
		return;

	if (s==-1) return; //lb
	P_CHAR pc_currchar = currchar[s];

	if (pc->stablemaster_serial() != INVALID_SERIAL) return; // dont **show** stabled pets

	int sendit;
	if (pc->isHidden() && pc!=currchar[s] && (pc_currchar->isGM())==0) sendit=0; else sendit=1;

	if (!online(pc) && (pc->isPlayer()) && (pc_currchar->isGM())==0 ) 
	{
		sendit=0;
		LongToCharPtr(pc->serial, &removeitem[1]);
		Xsend(s, removeitem, 5);
	}
	// hidden chars can only be seen "grey" by themselves or by gm's
	// other wise they are invisible=dont send the packet
	if (!sendit) return;

	oc[0]=0x78; // Message type 78

	LongToCharPtr(pc->serial, &oc[3]);
	ShortToCharPtr(pc->id(),  &oc[7]);
	oc[9]=pc->pos.x>>8;	// Character x position
	oc[10]=pc->pos.x%256; // Character x position
	oc[11]=pc->pos.y>>8; // Character y position
	oc[12]=pc->pos.y%256; // Character y position
	if (z) oc[13]=pc->dispz(); // Character z position
	else oc[13]=pc->pos.z;
	oc[14]=pc->dir(); // Character direction
	ShortToCharPtr(pc->skin(), &oc[15]); // Character skin color
	oc[17]=0; // Character flags
	if (pc->isHidden() || !(online(pc)||pc->isNpc())) oc[17]=oc[17]|0x80; // Show hidden state correctly
	if (pc->poisoned()) oc[17]=oc[17]|0x04; //AntiChrist -- thnx to SpaceDog

	k=19;
	int guild;
	guild = GuildCompare( currchar[s], pc );
	if (guild == 1)//Same guild (Green)
		oc[18]=2;
	else if (guild==2) // Enemy guild.. set to orange
		oc[18]=5;
	else
		switch(pc->flag())
	{//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
		case 0x01: oc[18]=6; break;// If a bad, show as red.
		case 0x04: oc[18]=1; break;// If a good, show as blue.
		case 0x08: oc[18]=2; break; //green (guilds)
		case 0x10: oc[18]=5; break;//orange (guilds)
		default:   oc[18]=3; break;//grey (Can be pretty much any number.. I like 3 :-)
	}

	QMap<int, bool> layers;
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if (pc->Wears(pi) && !pi->free)
			{
				if ( layers.contains( pi->layer() ) )
				{
					LongToCharPtr(pi->serial,oc+k+0);
					ShortToCharPtr(pi->id(),oc+k+4);
					oc[k+6]=pi->layer();
					k += 7;
					if ( pi->color() != 0 )
					{
						oc[k-3] |= 0x80;
						ShortToCharPtr(pi->color(), &oc[k+0]);
						k += 2;
					}
					layers[pi->layer()] = true;
				}
				else
				{
				}
			}
	}

	oc[k+0]=0;// Not well understood. It's a serial number. I set this to my serial number,
	oc[k+1]=0;// and all of my messages went to my paperdoll gump instead of my character's
	oc[k+2]=0;// head, when I was a character with serial number 0 0 0 1.
	oc[k+3]=0;
	k += 4;

	// unimportant remark: its a packet "terminator" !!! LB

	oc[1]=k>>8;
	oc[2]=k%256;
	Xsend(s, oc, k);
*/
}

void sendshopinfo(int s, P_CHAR pc, P_ITEM pi)
{
	qWarning("sendshopinfo() function is disabled");
/*	unsigned char m1[6096] = {0,};
	unsigned char m2[6096] = {0,};
	char itemname[256] = {0,};
	char cFoundItems=0;
	int k, m1t, m2t, value,serial;
               
	m1[0]=0x3C; // Container content message
	m1[1]=0;// Size of message
	m1[2]=0;// Size of message
	m1[3]=0;//  Count of items
	m1[4]=0;// Count of items
	m2[0]=0x74;// Buy window details message
	m2[1]=0;// Size of message
	m2[2]=8;// Size of message
	LongToCharPtr(pi->serial,m2+3); //Container serial number
	m2[7]=0; // Count of items;
	m1t=5;
	m2t=8;
	serial=pi->serial;
	cItem::ContainerContent container(pi->content());
	cItem::ContainerContent::const_iterator it (container.begin());
	cItem::ContainerContent::const_iterator end(container.end());
	for ( uint ci = 0; it != end; ++it, ++ci )
	{
		P_ITEM pi_j = *it;
		if (pi_j != NULL)
			if ((pi_j->contserial==serial) &&
				(m2[7]!=255) && (pi_j->amount()!=0) ) // 255 items max per shop container
			{
				if (m2t>6000 || m1t>6000) break;

				LongToCharPtr(pi_j->serial,m1+m1t+0);//Item serial number
				ShortToCharPtr(pi_j->id(),m1+m1t+4);
				m1[m1t+6]=0;			//Always zero
				m1[m1t+7]=pi_j->amount()>>8;//Amount for sale
				m1[m1t+8]=pi_j->amount()%256;//Amount for sale
				m1[m1t+9]=ci;//pi_j->pos.x>>8; //Item x position
				m1[m1t+10]=ci;//pi_j->pos.x%256;//Item x position
				//m1[m1t+11]=pi_j->pos.y>>8;//Item y position
				//m1[m1t+12]=pi_j->pos.y%256;//Item y position
				LongToCharPtr(pi->serial,m1+m1t+13); //Container serial number
				ShortToCharPtr(pi->color(), &m1[m1t+17]);
				m1[4]++; // Increase item count.
				m1t=m1t+19;
				value=pi_j->value;
				value=calcValue(pi_j, value);
				if (SrvParams->trade_system()==1) value=calcGoodValue(currchar[s], pi_j, value, 0); // by Magius(CHE)
				m2[m2t+0]=value>>24;// Item value/price
				m2[m2t+1]=value>>16;//Item value/price
				m2[m2t+2]=value>>8; // Item value/price
				m2[m2t+3]=value%256; // Item value/price
				m2[m2t+4]=pi_j->getName(itemname); // Item name length

				for(k=0;k<m2[m2t+4];k++)
				{
				  	m2[m2t+5+k]=itemname[k];
				}

				m2t=m2t+(m2[m2t+4])+5;
				m2[7]++;
				cFoundItems=1; //we found items so send message
				
			}
	}

	m1[1]=m1t>>8;
	m1[2]=m1t%256;
	m2[1]=m2t>>8;
	m2[2]=m2t%256;

	if (cFoundItems==1)
	{
		Xsend(s, m1, m1t);
		Xsend(s, m2, m2t);
	}
*/
}

int sellstuff(int s, P_CHAR pc)
{
	qWarning("sellstuff() function is disabled");
/*	char itemname[256];
	int m1t, z, value;
	int serial,serial1;
	unsigned char m1[2048];
	unsigned char m2[2];
	char ciname[256]; // By Magius(CHE)
	char cinam2[256]; // By Magius(CHE)

    if ( pc == NULL ) return 0;
	P_CHAR pc_currchar = currchar[s];
	P_ITEM sellcont = NULL;

	serial = pc->serial;
	sellcont = pc->atLayer( cChar::SellContainer );

	if (sellcont == NULL) return 0;

	m2[0]=0x33;
	m2[1]=0x01;
	Xsend(s, m2, 2);

	P_ITEM pBackpack = Packitem(pc_currchar);
	if (pBackpack == NULL) return 0; //LB

	m1[0]=0x9E; // Header
	m1[1]=0; // Size
	m1[2]=0; // Size
	LongToCharPtr(pc->serial, &m1[3]);
	m1[7]=0; // Num items
	m1[8]=0; // Num items
	m1t=9;

	cItem::ContainerContent container(sellcont->content());
	cItem::ContainerContent::const_iterator it (container.begin());
	cItem::ContainerContent::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pi_q = *it;
		if (pi_q != NULL)
		{
			if ((pi_q->contserial==serial))
			{
				serial1 = pBackpack->serial;
				cItem::ContainerContent container2(pBackpack->content());
				cItem::ContainerContent::const_iterator it2 (container2.begin());
				cItem::ContainerContent::const_iterator end2(container2.end());
				for (; it2 != end2; ++it )
				{
					P_ITEM pi_j = *it2;
					if (pi_j != NULL) // LB crashfix
					{
						sprintf(ciname,"'%s'",pi_j->name().upper().ascii()); // Added by Magius(CHE)
						sprintf(cinam2,"'%s'",pi_q->name().upper().ascii()); // Added by Magius(CHE)

						if (pi_j->contserial==serial1 &&
							pi_j->id()==pi_q->id()  &&
							pi_j->type()==pi_q->type() && (m1[8]<60) &&
							((SrvParams->sellbyname()==0)||(SrvParams->sellbyname()==1 && (!strcmp(ciname,cinam2))))) // If the names are the same! --- Magius(CHE)
						{
							LongToCharPtr(pi_j->serial,m1+m1t+0);
							ShortToCharPtr(pi_j->id(),m1+m1t+4);
							ShortToCharPtr(pi_j->color(),m1+m1t+6);
							ShortToCharPtr(pi_j->amount(),m1+m1t+8);
							value=pi_q->value;
							value=calcValue(pi_j, value);
							if (SrvParams->trade_system()==1) value=calcGoodValue(pc, pi_j, value, 1); // by Magius(CHE)
							m1[m1t+10]=value>>8;
							m1[m1t+11]=value%256;
							m1[m1t+12]=0;// Unknown... 2nd length byte for string?
							m1[m1t+13]=pi_j->getName(itemname);
							m1t=m1t+14;
							for(z=0;z<m1[m1t-1];z++)
							{
								m1[m1t+z]=itemname[z];
							}
							m1t=m1t+m1[m1t-1];
							m1[8]++;
						}
					}
				}
			}
		}
	}

	m1[1]=m1t>>8;
	m1[2]=m1t%256;
	if (m1[8]!=0)
	{
		Xsend(s, m1, m1t);
	}
	else
	{
		pc->talk( tr("Thou doth posses nothing of interest to me."), -1, 0 );
	}
	m2[0]=0x33;
	m2[1]=0x00;
	Xsend(s, m2, 2);
	return 1;
*/
	return 0;
}

void playmidi(int s, char num1, char num2)
{
	char msg[3];
	msg[0]=0x6D;
	msg[1]=num1;
	msg[2]=num2;
	Xsend(s, msg, 3);
}

void sendtradestatus(P_ITEM cont1, P_ITEM cont2)
{
}

void endtrade(SERIAL serial)
{
}

void tellmessage(int i, int s, const char *txt)
{
/*	int tl;
	P_CHAR pc_currchar = currchar[s];

	sprintf((char*)temp, "GM tells %s: %s", pc_currchar->name.latin1(), txt);

	tl=44+strlen((char*)temp)+1;
	talk[1]=tl>>8;
	talk[2]=tl%256;
	talk[3]=1;
	talk[4]=1;
	talk[5]=1;
	talk[6]=1;
	talk[7]=1;
	talk[8]=1;
	talk[9]=0;
	talk[10]=0x00; //First Part  \_Yellow
	talk[11]=0x35; //Second Part /
	talk[12]=0;
	talk[13]=3;
	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, temp, strlen((char*)temp)+1);
	Xsend(i, talk, 14);//So Person who said it can see too
	Xsend(i, sysname, 30);
	Xsend(i, temp, strlen((char*)temp)+1);*/
}



// particleSystem core functions, LB 2-April 2001

// sta_str layout:

// 0..3 already used in 2d-staticeffect
// effect 4  -> tile1
// effect 5  -> tile2
// effect 6  -> speed1
// effect 7  -> speed1
// effect 8  -> effect1
// effect 9  -> effect2
// effect 10 -> reserved, dont use
// effect 11 -> 
// effect 12 -> 


void staticeffectUO3D(P_CHAR pc_cs, stat_st *sta)
{  
   
/*   if ( pc_cs == NULL )
	   return;

   // please no optimization of p[...]=0's yet :)

	particleSystem[0]=0xc7;
	particleSystem[1]=0x3;

	LongToCharPtr(pc_cs->serial, &particleSystem[2]);

	particleSystem[6]=0x0; // always 0 for this type
	particleSystem[7]=0x0;
	particleSystem[8]=0x0;
	particleSystem[9]=0x0;

	particleSystem[10]=sta->effect[4]; // tileid1
	particleSystem[11]=sta->effect[5]; // tileid2

	particleSystem[12]=(pc_cs->pos.x)>>8;
	particleSystem[13]=(pc_cs->pos.x)%256;
	particleSystem[14]=(pc_cs->pos.y)>>8;
	particleSystem[15]=(pc_cs->pos.y)%256;
	particleSystem[16]=(pc_cs->pos.z);

	particleSystem[17]=(pc_cs->pos.x)>>8; 
	particleSystem[18]=(pc_cs->pos.x)%256;
	particleSystem[19]=(pc_cs->pos.y)>>8;
	particleSystem[20]=(pc_cs->pos.y)%256;
	particleSystem[21]=(pc_cs->pos.z);

	particleSystem[22]= sta->effect[6]; // unkown1 
	particleSystem[23]= sta->effect[7]; // unkown2

	particleSystem[24]=0x0; // only non zero for type 0
	particleSystem[25]=0x0;

	particleSystem[26]=0x1;
	particleSystem[27]=0x0;

	particleSystem[28]=0x0;
	particleSystem[29]=0x0;
	particleSystem[30]=0x0;
	particleSystem[31]=0x0;
	particleSystem[32]=0x0;
	particleSystem[33]=0x0;
	particleSystem[34]=0x0;
	particleSystem[35]=0x0;

	particleSystem[36]=sta->effect[8]; // effekt #
	particleSystem[37]=sta->effect[9];

	particleSystem[38]=sta->effect[11];
	particleSystem[39]=sta->effect[12];
	  
	particleSystem[40]=0x00;
	particleSystem[41]=0x00;

	LongToCharPtr(pc_cs->serial, &particleSystem[42]);
	   
	particleSystem[46]=0; // layer, gets set afterwards for multi layering

	particleSystem[47]=0x0; // has to be always 0 for all types
	particleSystem[48]=0x0;*/
  	   	   
}

// move_st layout:
// 0..4 already used in 2d-move_effect

// effect 5  -> tile1
// effect 6  -> tile2
// effect 7  -> speed1
// effect 8  -> speed2
// effect 9  -> effect1
// effect 10 -> effect2
// effect 11 -> impact effect1
// effect 12 -> impact effect2
// effect 13 -> unkown1, does nothing, but gets set on OSI shards
// effect 14 -> unkown2
// effect 15 -> adjust  
// effect 16 -> explode on impact

void movingeffectUO3D(P_CHAR pc_cs, P_CHAR pc_cd, move_st *sta)
{
 /*  if (pc_cs == NULL || pc_cd == NULL) return;

	particleSystem[0]=0xc7;
	particleSystem[1]=0x0;

	LongToCharPtr(pc_cs->serial, &particleSystem[2]);
	LongToCharPtr(pc_cd->serial, &particleSystem[6]);

	particleSystem[10]=sta->effect[5]; // tileid1
	particleSystem[11]=sta->effect[6]; // tileid2

	particleSystem[12]=(pc_cs->pos.x)>>8;
	particleSystem[13]=(pc_cs->pos.x)%256;
	particleSystem[14]=(pc_cs->pos.y)>>8;
	particleSystem[15]=(pc_cs->pos.y)%256;
	particleSystem[16]=(pc_cs->pos.z);

	particleSystem[17]=(pc_cd->pos.x)>>8; 
	particleSystem[18]=(pc_cd->pos.x)%256;
	particleSystem[19]=(pc_cd->pos.y)>>8;
	particleSystem[20]=(pc_cd->pos.y)%256;
	particleSystem[21]=(pc_cd->pos.z);
	   
	particleSystem[22]= sta->effect[7]; // speed1 
	particleSystem[23]= sta->effect[8]; // speed2

	particleSystem[24]=0x0; 
	particleSystem[25]=0x0;

	particleSystem[26]=sta->effect[15]; // adjust
	particleSystem[27]=sta->effect[16]; // explode

	particleSystem[28]=0x0;
	particleSystem[29]=0x0;
	particleSystem[30]=0x0;
	particleSystem[31]=0x0;
	particleSystem[32]=0x0;
	particleSystem[33]=0x0;
	particleSystem[34]=0x0;
	particleSystem[35]=0x0;

	particleSystem[36]=sta->effect[9]; //  moving effekt 
	particleSystem[37]=sta->effect[10];
	particleSystem[38]=sta->effect[11]; // effect on explode
	particleSystem[39]=sta->effect[12];

	particleSystem[40]=sta->effect[13]; // ??
	particleSystem[41]=sta->effect[14];

	particleSystem[42]=0x00;
	particleSystem[43]=0x00;
	particleSystem[44]=0x00;
	particleSystem[45]=0x00;
	   
	particleSystem[46]=0xff; // layer, has to be 0xff in that modus

	particleSystem[47]=sta->effect[17];
	particleSystem[48]=0x0;*/

}

// same sta-layout as staticeffectuo3d
void itemeffectUO3D(P_ITEM pi, stat_st *sta)
{
/*	// please no optimization of p[...]=0's yet :)
	
	particleSystem[0]=0xc7;
	particleSystem[1]=0x2;
	
	if ( !sta->effect[11] ) 
	{
		LongToCharPtr(pi->serial, &particleSystem[2]);
	}
	else
	{
		LongToCharPtr(0, &particleSystem[2]);
	}
	
	particleSystem[6]=0x0; // always 0 for this type
	particleSystem[7]=0x0;
	particleSystem[8]=0x0;
	particleSystem[9]=0x0;
	
	particleSystem[10]=sta->effect[4]; // tileid1
	particleSystem[11]=sta->effect[5]; // tileid2
	
	particleSystem[12]=(pi->pos.x)>>8;
	particleSystem[13]=(pi->pos.x)%256;
	particleSystem[14]=(pi->pos.y)>>8;
	particleSystem[15]=(pi->pos.y)%256;
	particleSystem[16]=(pi->pos.z);
	
	particleSystem[17]=(pi->pos.x)>>8; 
	particleSystem[18]=(pi->pos.x)%256;
	particleSystem[19]=(pi->pos.y)>>8;
	particleSystem[20]=(pi->pos.y)%256;
	particleSystem[21]=(pi->pos.z);
	
	particleSystem[22]= sta->effect[6]; // unkown1 
	particleSystem[23]= sta->effect[7]; // unkown2
	
	particleSystem[24]=0x0; // only non zero for type 0
	particleSystem[25]=0x0;
	
	particleSystem[26]=0x1;
	particleSystem[27]=0x0;
	
	particleSystem[28]=0x0;
	particleSystem[29]=0x0;
	particleSystem[30]=0x0;
	particleSystem[31]=0x0;
	particleSystem[32]=0x0;
	particleSystem[33]=0x0;
	particleSystem[34]=0x0;
	particleSystem[35]=0x0;
	
	particleSystem[36]=sta->effect[8]; // effekt #
	particleSystem[37]=sta->effect[9];
	
	particleSystem[38]=0; // unknown
	particleSystem[39]=1;
	
	particleSystem[40]=0x00;
	particleSystem[41]=0x00;
	
	LongToCharPtr(pi->serial, &particleSystem[42]);
	
	particleSystem[46]=0xff; 
	
	particleSystem[47]=0x0; 
	particleSystem[48]=0x0;*/
}

void bolteffectUO3D(P_CHAR player)
{
	Magic->doStaticEffect(player, 30);
}

void PlayDeathSound( P_CHAR pc )
{
    if ( pc == NULL )
		return;

	if (pc->xid()==0x0191)
	{
		switch(RandomNum(0, 3)) // AntiChrist - uses all the sound effects
		{
		case 0:		pc->soundEffect( 0x0150 );	break;// Female Death
		case 1:		pc->soundEffect( 0x0151 );	break;// Female Death
		case 2:		pc->soundEffect( 0x0152 );	break;// Female Death
		case 3:		pc->soundEffect( 0x0153 );	break;// Female Death
		}
	}
	else if (pc->xid()==0x0190)
	{
		switch( RandomNum(0, 3) ) // AntiChrist - uses all the sound effects
		{
		case 0:		pc->soundEffect( 0x015A );	break;// Male Death
		case 1:		pc->soundEffect( 0x015B );	break;// Male Death
		case 2:		pc->soundEffect( 0x015C );	break;// Male Death
		case 3:		pc->soundEffect( 0x015D );	break;// Male Death
		}
	}
	else
	{
		playmonstersound( pc, pc->xid(), SND_DIE );
	}
}

