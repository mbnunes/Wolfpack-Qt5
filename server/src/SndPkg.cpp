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

#undef  DBGFILE
#define DBGFILE "SndPkg.cpp"

#include "debug.h"
#include "basics.h"
#include "SndPkg.h"
#include "sregions.h"
#include "itemid.h"
#include "oem.h"

//keep the target highlighted so that we know who we're attacking =)
//26/10/99//new packet
void SndAttackOK(UOXSOCKET s, int serial)
{
	unsigned char attackok[6]="\xAA\x00\x00\x00\x00";//AntiChrist! 26/10/99
	LongToCharPtr(serial,attackok+1);
	Xsend(s, attackok, 5);
}

void SndDyevat(UOXSOCKET s, int serial, short id)
{
	LongToCharPtr(serial,(dyevat+1));
	ShortToCharPtr(id,dyevat+7);
	Xsend(s, dyevat, 9);
}

void SndUpdscroll(UOXSOCKET s, short txtlen, char* txt)
{
	ShortToCharPtr(txtlen+10,updscroll+1);
	updscroll[3]=2;
	ShortToCharPtr(txtlen,updscroll+8);
	Xsend(s, updscroll, 10);
	Xsend(s, txt, txtlen);
}

void SndRemoveitem(int serial)
{
	LongToCharPtr(serial,removeitem+1);
	for(int j=0;j<now;j++)
		if (perm[j])
			Xsend(j, removeitem, 5);
}

void SndShopgumpopen(UOXSOCKET s, int serial)
{
	unsigned char shopgumpopen[8]="\x24\x00\x00\x00\x01\x00\x30";
	LongToCharPtr(serial,shopgumpopen+1);
	Xsend(s, shopgumpopen, 7);
}

void soundeffect(int s, unsigned char a, unsigned char b) // Play sound effect for player
{
	int i;
	
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	sfx[2] = a;
	sfx[3] = b;
	sfx[6] = pc_currchar->pos.x>>8;
	sfx[7] = pc_currchar->pos.x%256;
	sfx[8] = pc_currchar->pos.y>>8;
	sfx[9] = pc_currchar->pos.y%256;
	for (i=0;i<now;i++)//send to all inrange sockets, included the current socket
		if ((perm[i])&&((inrange1(s,i))||(s==i)))
		{
			Xsend(i, sfx, 12);
		}
}

void soundeffect2(PC_CHAR pc, short sound)
{
	int i;

	ShortToCharPtr(sound, sfx+2);
	sfx[6]=pc->pos.x>>8;
	sfx[7]=pc->pos.x%256;
	sfx[8]=pc->pos.y>>8;
	sfx[9]=pc->pos.y%256;
	for (i=0;i<now;i++)
		if ((perm[i])&&(inrange1p(pc, currchar[i])))
		{
			Xsend(i, sfx, 12);
		}
}

void soundeffect2(CHARACTER p, unsigned char a, unsigned char b)
{
	int i;

	P_CHAR pc = MAKE_CHARREF_LR(p);
	sfx[2]=a;
	sfx[3]=b;
	sfx[6]=pc->pos.x>>8;
	sfx[7]=pc->pos.x%256;
	sfx[8]=pc->pos.y>>8;
	sfx[9]=pc->pos.y%256;
	for (i=0;i<now;i++)
		if ((perm[i])&&(inrange1p(p, currchar[i])))
		{
			Xsend(i, sfx, 12);
		}
}

void soundeffect3(P_ITEM pi, short sound)
{
	int j;

	ShortToCharPtr(sound,sfx+2);
	ShortToCharPtr(pi->pos.x,sfx+6);
	ShortToCharPtr(pi->pos.y,sfx+8);
	for (j=0;j<now;j++)
		if (perm[j] && inrange2(j,pi) )
		{
			Xsend(j, sfx, 12);
		}
}

void soundeffect4(int p, UOXSOCKET s, unsigned char a, unsigned char b)
{
	const PC_ITEM pi=MAKE_ITEMREF_LR(p);	// on error return
	sfx[2]=a;
	sfx[3]=b;
	sfx[6]=pi->pos.x>>8;
	sfx[7]=pi->pos.x%256;
	sfx[8]=pi->pos.y>>8;
	sfx[9]=pi->pos.y%256;
	Xsend(s, sfx, 12);
}

void soundeffect5(UOXSOCKET s, unsigned char a, unsigned char b)
{
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	sfx[2]=a;
	sfx[3]=b;
	sfx[6]=pc_currchar->pos.x>>8;
	sfx[7]=pc_currchar->pos.x%256;
	sfx[8]=pc_currchar->pos.y>>8;
	sfx[9]=pc_currchar->pos.y%256;
	Xsend(s, sfx, 12);

}

void action(int s, int x) // Character does a certain action
{
	int i;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	LongToCharPtr(pc_currchar->serial, &doact[1]);
	doact[5]=x>>8;
	doact[6]=x%256;
	Xsend(s, doact, 14);
	for (i=0;i<now;i++) if ((inrange1(s, i))&&(perm[i])) { Xsend(i, doact, 14); }
}

void npcaction(int npc, int x) // NPC character does a certain action
{
	int i;
	P_CHAR pc_npc      = MAKE_CHARREF_LR(npc);

	LongToCharPtr(pc_npc->serial, &doact[1]);
	doact[5]=x>>8;
	doact[6]=x%256;
	for (i=0;i<now;i++) 
		if ((inrange1p(currchar[i], npc))&&(perm[i])) 
			Xsend(i, doact, 14);
}


/*
// s: player socket, I: send bolt if it rains or not ?

void weather(int s, char bolt) // Send new weather to player
{
	char wdry[5]="\x65\x00\x00\x00";
	char wrain[5]="\x65\x01\x46\x00";
	char wsnow[5]="\x65\x02\x46\xEC";

	int i=calcCharFromSer(pc_currchar->serial),n;

	for (int j=0;j<now;j++)
	{
		if (noweather[currchar[j]] && wtype!=0)
		{
			Xsend(s,wdry,4);
			return;
		}
	}
	// send wdry to non moving(!) players if it rains or snows and they are inside buildings

	if (wtype==0) Xsend(s, wdry, 4);

	if (wtype==1)
	{
		if (bolt)
		{
			n=1;
			for (int a=0;a<n;a++) // reduce if too laggy (client only lag though)
			{
				if (rand()%2)
				{
					soundeffect2(i, 0x00, 0x28);
					bolteffect(i);
				}
				else
				{
					soundeffect2(i, 0x00, 0x29);
					bolteffect(i);
				}
			}
		}

	raindroptime=uiCurrentTime+MY_CLOCKS_PER_SEC*(6+rand()%24);
	Xsend(s, wrain, 4);
	}
		if (wtype==2)
		{
			if (rand()%2)
			{
				soundeffect2(i, 0x00, 0x14);
			}
			else
			{
				soundeffect2(i, 0x00, 0x15);
			}
			Xsend(s, wsnow, 4);
		}
}
*/
void sysbroadcast(const char *txt) // System broadcast in bold text
{
	int tl, i;

	tl=44+strlen(txt)+1;
	talk[1]=tl>>8;
	talk[2]=tl%256;
	talk[3]=1;
	talk[4]=1;
	talk[5]=1;
	talk[6]=1;
	talk[7]=1;
	talk[8]=1;
	talk[9]=1;
	talk[10]=0x08;
	talk[11]=0x4d;
	talk[12]=0;
	talk[13]=0;
	for (i=0;i<now;i++)
	{
		if (perm[i])
		{
			Xsend(i, talk, 14);
			Xsend(i, sysname, 30);
			Xsend(i, (char*)txt, strlen(txt)+1);
		}
	}
//	Network->ClearBuffers();
}

void sysmessage(UOXSOCKET s, char *txt, ...) // System message (In lower left corner)
{
	if(s==-1) return;
	if ( s>=MAXCLIENT)
		LogCriticalVar("Socket/Index confusion @ Xsend <%s>\n",txt);
	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsprintf( msg, translate(txt), argptr );
	va_end( argptr );
	int tl = 44 + strlen( msg ) + 1;
	talk[1]=tl>>8;
	talk[2]=tl%256;
	talk[3]=1;
	talk[4]=1;
	talk[5]=1;
	talk[6]=1;
	talk[7]=1;
	talk[8]=1;
	talk[9]=0;
	talk[10]=0x00;	//Color1  - Previous default was 0x0040
	talk[11]=0x00;  //Color2
	talk[12]=0;     
	talk[13]=3;
	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, msg, strlen((char*)msg)+1);
}

void sysmessage(UOXSOCKET s, short color, char *txt, ...) // System message (In lower left corner)
{
	if(s==-1) return;
	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsprintf( msg, (char *)txt, argptr );
	va_end( argptr );
	int tl = 44 + strlen( msg ) + 1;
	talk[1]=tl>>8;
	talk[2]=tl%256;
	talk[3]=1;
	talk[4]=1;
	talk[5]=1;
	talk[6]=1;
	talk[7]=1;
	talk[8]=1;
	talk[9]=0;
	talk[10]=color>>8;
	talk[11]=color%256;			 
	talk[12]=0;
	talk[13]=3;
	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, msg, strlen((char*)msg)+1);
}

void itemmessage(UOXSOCKET s, char *txt, int serial, short color) 
{// The message when an item is clicked (new interface, Duke)

	P_ITEM pi = FindItemBySerial(serial);

	if ( pi == NULL )
		return;

	if ((pi->type == 1 && color == 0x0000)||
		(pi->type == 9 && color == 0x0000)||
		(pi->type == 1000 && color == 0x0000)||
		(pi->id()==0x1BF2 && color == 0x0000))
		color = 0x03B2;
	    else
			if(!(pi->corpse == 1))
		color = 0x0481;

	int tl=44+strlen(txt)+1;
	talk[1]=tl>>8;
	talk[2]=tl%256;
	LongToCharPtr(serial,talk+3);
	talk[7]=1;
	talk[8]=1;
	talk[9]=6; // Mode: "You see"
	ShortToCharPtr(color,talk+10);
	talk[12]=0;
	talk[13]=3;
	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, txt, strlen(txt)+1);
}

void wearIt(const UOXSOCKET s, const P_ITEM pi)
{
	LongToCharPtr(pi->serial,wearitem+1);
	ShortToCharPtr(pi->id(),wearitem+5);
	wearitem[8]=pi->layer;
	LongToCharPtr(pi->contserial,wearitem+9);
	wearitem[13]=pi->color1;
	wearitem[14]=pi->color2;
	Xsend(s, wearitem, 15);
}

void backpack(UOXSOCKET s, SERIAL serial) // Send Backpack (with items)
{
	int count=0;
	unsigned char bpopen[13]="\x24\x40\x0B\x00\x1A\x00\x3C\x3C\x00\x05\x00\x00";
	if(serial==INVALID_SERIAL) return;
	P_ITEM pCont=FindItemBySerial(serial);
	if (!pCont)
	{
		LogErrorVar("backpack() couldn't find backpack: %d.",serial);
		return;
	}

	int ci=0,loopexit=0;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	count = vecContainer.size();
	bpopen[10]=count>>8;
	bpopen[11]=count%256;
	count=(count*19)+5;
	bpopen[8]=count>>8;
	bpopen[9]=count%256;
	LongToCharPtr(serial, &bpopen[1]);
	bpopen[5]=0x00;
	bpopen[6]=0x47;

	// Lil' rewrite, LB 10'th Okt 2000

	switch(pCont->id1)
	{
		case 0x0E:
			switch (pCont->id2)
			{
				case 0x75:					// Backpack
				case 0x79: bpopen[6]=0x3C;	// Box/Pouch
							break;

				case 0x76: bpopen[6]=0x3D;	// Leather Bag
							break;

				case 0x77:					// Barrel
				case 0x7A:					// Square Basket
				case 0x7F: bpopen[6]=0x3E;	// Keg
							break;

				case 0x7C: bpopen[6]=0x4A;	// Silver Chest
							break;

				case 0x7D: bpopen[6]=0x43;	// Wooden Box
							break;


				case 0x3D:					// Large Wooden Crate
				case 0x3C:					// Large Wooden Crate
				case 0x3F:					// Small Wooden Crate
				case 0x3E:					// Small Wooden Crate
				case 0x7E: bpopen[6]=0x44;	// Wooden Crate
							break;

				case 0x80: bpopen[6]=0x4B;	// Brass Box
							break;

				case 0x40:					// Metal & Gold Chest
				case 0x41: bpopen[6]=0x42;	// Metal & Gold Chest
							break;


				case 0x43:					// Wooden & Gold chest
				case 0x42: bpopen[6]=0x49; // Wooden & Gold Chest
							break;

			}
			break;

		case 0x09:
			switch (pCont->id2)
			{
				case 0x90: bpopen[6]=0x41; // Round Basket
							break;

				case 0xB2: bpopen[6]=0x3C; // Backpack 2
							break;

				case 0xAA: bpopen[6]=0x43; // Wooden Box
							break;

				case 0xA8: bpopen[6]=0x40; // Metal Box
							break;

				case 0xAB: bpopen[6]=0x4A; // Metal/Silver Chest
							break;

				case 0xA9: bpopen[6]=0x44; // Small Wooden Crate
							break;
			}
			break;

		case 0x20:
			if (pCont->id2=='\x06') bpopen[6]=0x09; // Coffin
			break;

		case 0x0A:
			switch (pCont->id2)
			{
				case 0x97:					// Bookcase
				case 0x98:					// Bookcase
				case 0x99:					// Bookcase
				case 0x9a:					// Bookcase
				case 0x9b:					// Bookcase
				case 0x9c:					// Bookcase
				case 0x9d:					// Bookcase
				case 0x9e:	bpopen[6]=0x4d; // Bookcase
							break;

				case 0x4d:					// Fancy Armoire
				case 0x51:					// Fancy Armoire
				case 0x4c:					// Fancy Armoire
				case 0x50:	bpopen[6]=0x4e; // Fancy Armoire
							break;

				case 0x4f:					// Wooden Armoire
				case 0x53:					// Wooden Armoire
				case 0x4e:					// Wooden Armoire
				case 0x52:	bpopen[6]=0x4f; // Wooden Armoire
							break;

				case 0x30:					// chest of drawers (fancy)
				case 0x38:	bpopen[6]=0x48; // chest of drawers (fancy)
							break;

				case 0x2c:					// chest of drawers (wood)
				case 0x34:					// chest of drawers (wood)
				case 0x3c:					// Dresser
				case 0x3d:					// Dresser
				case 0x44:					// Dresser
				case 0x35:	bpopen[6]=0x51; // Dresser
							break;
				case 0xb2:	if (pCont->morex==1) bpopen[6]=0x4a;
							break;

			}
			break;

		case 0x3e:
			 bpopen[6]=0x4C;
			 break;

		default:
			break;

	}

	/*if ((pi->id1=='\x0E')&&(pi->id2=='\x75')) bpopen[6]=0x3C; // Backpack
	if ((pi->id1=='\x0E')&&(pi->id2=='\x76')) bpopen[6]=0x3D; // Leather Bag
	if ((pi->id1=='\x0E')&&(pi->id2=='\x77')) bpopen[6]=0x3E; // Barrel
	if ((pi->id1=='\x09')&&(pi->id2=='\x90')) bpopen[6]=0x41; // Round Basket
	if ((pi->id1=='\x0E')&&(pi->id2=='\x79')) bpopen[6]=0x3C; // Box/Pouch
	if ((pi->id1=='\x0E')&&(pi->id2=='\x7A')) bpopen[6]=0x3F; // Square Basket
	if ((pi->id1=='\x0E')&&(pi->id2=='\x7C')) bpopen[6]=0x4A; // Silver Chest
	if ((pi->id1=='\x0E')&&(pi->id2=='\x7D')) bpopen[6]=0x43; // Wooden Box
	if ((pi->id1=='\x0E')&&(pi->id2=='\x7E')) bpopen[6]=0x44; // Wooden Crate
	if ((pi->id1=='\x0E')&&(pi->id2=='\x7F')) bpopen[6]=0x3E; // Keg
	if ((pi->id1=='\x0E')&&(pi->id2=='\x80')) bpopen[6]=0x4B; // Brass Box
	if ((pi->id1=='\x20')&&(pi->id2=='\x06')) bpopen[6]=0x09; // Coffin
	// Misc. Containers
	if ((pi->id1=='\x09')&&(pi->id2=='\xB2')) bpopen[6]=0x3C; // Backpack 2
	if ((pi->id1=='\x09')&&(pi->id2=='\xAA')) bpopen[6]=0x3F; // Wooden Box
	if ((pi->id1=='\x09')&&(pi->id2=='\xA8')) bpopen[6]=0x40; // Metal Box
	if ((pi->id1=='\x09')&&(pi->id2=='\xAB')) bpopen[6]=0x4A; // Metal/Silver Chest
	if ((pi->id1=='\x0E')&&(pi->id2=='\x40')) bpopen[6]=0x42; // Metal & Gold Chest
	if ((pi->id1=='\x0E')&&(pi->id2=='\x41')) bpopen[6]=0x42; // Metal & Gold Chest
	if ((pi->id1=='\x09')&&(pi->id2=='\xA9')) bpopen[6]=0x44; // Small Wooden Crate
	if ((pi->id1=='\x0E')&&(pi->id2=='\x3E')) bpopen[6]=0x44; // Small Wooden Crate
	if ((pi->id1=='\x0E')&&(pi->id2=='\x3F')) bpopen[6]=0x44; // Small Wooden Crate
	if ((pi->id1=='\x0E')&&(pi->id2=='\x3C')) bpopen[6]=0x44; // Large Wooden Crate
	if ((pi->id1=='\x0E')&&(pi->id2=='\x3D')) bpopen[6]=0x44; // Large Wooden Crate
	// Bookcases
	if ((pi->id1=='\x0A')&&(pi->id2=='\x97')) bpopen[6]=0x4D; // Bookcase
	if ((pi->id1=='\x0A')&&(pi->id2=='\x98')) bpopen[6]=0x4D; // Bookcase
	if ((pi->id1=='\x0A')&&(pi->id2=='\x99')) bpopen[6]=0x4D; // Bookcase
	if ((pi->id1=='\x0A')&&(pi->id2=='\x9A')) bpopen[6]=0x4D; // Bookcase
	if ((pi->id1=='\x0A')&&(pi->id2=='\x9B')) bpopen[6]=0x4D; // Bookcase
	if ((pi->id1=='\x0A')&&(pi->id2=='\x9C')) bpopen[6]=0x4D; // Bookcase
	// Armoires
	if ((pi->id1=='\x0A')&&(pi->id2=='\x4D')) bpopen[6]=0x4E; // Fancy Armoire
	if ((pi->id1=='\x0A')&&(pi->id2=='\x51')) bpopen[6]=0x4E; // Fancy Armoire
	if ((pi->id1=='\x0A')&&(pi->id2=='\x4F')) bpopen[6]=0x4F; // Wooden Armoire
	if ((pi->id1=='\x0A')&&(pi->id2=='\x53')) bpopen[6]=0x4F; // Wooden Armoire
	// Open Armoires
	// Hey, who knows - you might use these sometime
	if ((pi->id1=='\x0A')&&(pi->id2=='\x4C')) bpopen[6]=0x4E; // Fancy Armoire (open)
	if ((pi->id1=='\x0A')&&(pi->id2=='\x50')) bpopen[6]=0x4E; // Fancy Armoire (open)
	if ((pi->id1=='\x0A')&&(pi->id2=='\x4E')) bpopen[6]=0x4F; // Wooden Armoire (open)
	if ((pi->id1=='\x0A')&&(pi->id2=='\x52')) bpopen[6]=0x4F; // Wooden Armoire (open)
	// Chest Drawers
	if ((pi->id1=='\x0A')&&(pi->id2=='\x30')) bpopen[6]=0x48; // chest of drawers (fancy)
	if ((pi->id1=='\x0A')&&(pi->id2=='\x38')) bpopen[6]=0x48; // chest of drawers (fancy)
	if ((pi->id1=='\x0A')&&(pi->id2=='\x2C')) bpopen[6]=0x51; // chest of drawers (wood)
	if ((pi->id1=='\x0A')&&(pi->id2=='\x34')) bpopen[6]=0x51; // chest of drawers (wood)
	// Dressers
	if ((pi->id1=='\x0A')&&(pi->id2=='\x3C')) bpopen[6]=0x51; // Dresser
	if ((pi->id1=='\x0A')&&(pi->id2=='\x3D')) bpopen[6]=0x51; // Dresser
	if ((pi->id1=='\x0A')&&(pi->id2=='\x44')) bpopen[6]=0x51; // Dresser
	if ((pi->id1=='\x0A')&&(pi->id2=='\x35')) bpopen[6]=0x51; // Dresser
	// Wooden Chests
	if ((pi->id1=='\x0E')&&(pi->id2=='\x42')) bpopen[6]=0x49; // Wooden & Gold Chest
	if ((pi->id1=='\x0E')&&(pi->id2=='\x43')) bpopen[6]=0x49; // Wooden & Gold Chest
	// Bank box
	if ((pi->id1=='\x09')&&(pi->id2=='\xB2') && pi->morex==1) bpopen[6]=0x4A; // Bank box
	if (pi->id1==0x3E) bpopen[6]=0x4C;//Hold //Boats*/

	Xsend(s, bpopen, 12);

	ci=0;
	loopexit=0;
	P_ITEM pi;
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi == NULL)
			continue;
		//fix location of items if they mess up. (needs tweaked for container types)
		if (pi->pos.x>150) pi->pos.x=150;
		if (pi->pos.y>140) pi->pos.y=140;
		//end fix
		bpitem[0]=pi->ser1;
		bpitem[1]=pi->ser2;
		bpitem[2]=pi->ser3;
		bpitem[3]=pi->ser4;
		ShortToCharPtr(pi->id(),bpitem+4);
		bpitem[7]=pi->amount>>8;
		bpitem[8]=pi->amount%256;
		bpitem[9]=pi->pos.x>>8;
		bpitem[10]=pi->pos.x%256;
		bpitem[11]=pi->pos.y>>8;
		bpitem[12]=pi->pos.y%256;
		LongToCharPtr(serial, &bpitem[13]);
		bpitem[17]=pi->color1;
		bpitem[18]=pi->color2;
		bpitem[19]=pi->decaytime=0;//HoneyJar // reseting the decaytimer in the backpack
		Xsend(s, bpitem, 19);
	}
}

void backpack2(int s, int a1, int a2, int a3, int a4) // Send corpse stuff
{
	int count=0, count2;
	unsigned char bpopen2[6]="\x3C\x00\x05\x00\x00";
	unsigned char display1[8]="\x89\x00\x0D\x40\x01\x02\x03";
	unsigned char display2[6]="\x01\x40\x01\x02\x03";

	int serial=calcserial(a1,a2,a3,a4);
	int ci=0,loopexit=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer!=0)
		{
			count++;
		}
	}
	count2=(count*5)+7 + 1 ; // 5 bytes per object, 7 for this header and 1 for terminator
	display1[1]=count2>>8;
	display1[2]=count2%256;
	display1[3]=a1;
	display1[4]=a2;
	display1[5]=a3;
	display1[6]=a4;
	Xsend(s, display1, 7);

	ci=0;
	loopexit=0;
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer!=0)
		{
			display2[0]=pi->layer;
			display2[1]=pi->ser1;
			display2[2]=pi->ser2;
			display2[3]=pi->ser3;
			display2[4]=pi->ser4;
			Xsend(s, display2, 5);
		}
	}
	char nul = 0;
	Xsend(s, &nul, 1);	// Terminate with a 0

	bpopen2[3]=count>>8;
	bpopen2[4]=count%256;
	count2=(count*19)+5;
	bpopen2[1]=count2>>8;
	bpopen2[2]=count2%256;
	Xsend(s, bpopen2, 5);

	ci=0;
	loopexit=0;
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer!=0)
		{
			bpitem[0]=pi->ser1;
			bpitem[1]=pi->ser2;
			bpitem[2]=pi->ser3;
			bpitem[3]=pi->ser4;
			ShortToCharPtr(pi->id(),bpitem+4);
			bpitem[7]=pi->amount>>8;
			bpitem[8]=pi->amount%256;
			bpitem[9]=pi->pos.x>>8;
			bpitem[10]=pi->pos.x%256;
			bpitem[11]=pi->pos.y>>8;
			bpitem[12]=pi->pos.y%256;
			bpitem[13]=a1;
			bpitem[14]=a2;
			bpitem[15]=a3;
			bpitem[16]=a4;
			bpitem[17]=pi->color1;
			bpitem[18]=pi->color2;
			bpitem[19]=pi->decaytime=0;// reseting the decaytimer in the backpack	//moroallan
			Xsend(s, bpitem, 19);
		}
	}
}

void sendbpitem(UOXSOCKET s, ITEM ii) // Update single item in backpack
{
	int x,c;

	unsigned char display3[2]="\x25";
	const P_ITEM pi=MAKE_ITEMREF_LR(ii);	// on error return
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	bpitem[0]=pi->ser1;
	bpitem[1]=pi->ser2;
	bpitem[2]=pi->ser3;
	bpitem[3]=pi->ser4;
	//AntiChrist - world light sources stuff
	//if player is a gm, this item
	//is shown like a candle (so that he can move it),
	//....if not, the item is a normal
	//invisible light source!
	if(pc_currchar->isGM() && pi->id()==0x1647)
	{///let's show the lightsource like a candle
		bpitem[4]=0x0A;
		bpitem[5]=0x0F;
	} else
	{//else like a normal item
		ShortToCharPtr(pi->id(),bpitem+4);
	}
	bpitem[7]=pi->amount>>8;
	bpitem[8]=pi->amount%256;
	bpitem[9]=pi->pos.x>>8;
	bpitem[10]=pi->pos.x%256;
	bpitem[11]=pi->pos.y>>8;
	bpitem[12]=pi->pos.y%256;
	LongToCharPtr(pi->contserial,bpitem+13);
	if(pc_currchar->isGM() && pi->id()==0x1647)
	{///let's show the lightsource like a blue item
		bpitem[17]=0;
		bpitem[18]='\xC6';
	} else
	{//else like a normal item
		bpitem[17]=pi->color1;
		bpitem[18]=pi->color2;
	}
	bpitem[19]=pi->decaytime=0; // HoneyJar, array range is 0-19 ! //reseting the decaytimer in the backpack

	// we need to find the topmost container that the item is in
	// be it a character or another container.

	c = -1;
	x = -1;
//	serial=pi->contserial;
//	count=0;
	P_ITEM pio=GetOutmostCont(pi);
	if (!pio)
	{
		char ttt[222];
		sprintf(ttt,"item <%i> id <%x> has a bad contserial <%i>",pi->serial,pi->id(),pi->contserial);
		LogCritical(ttt);
	}
	else
	{
		P_CHAR pc=FindCharBySerial(pio->contserial);
		if (pc)
			c=DEREF_P_CHAR(pc);
		else
			x=DEREF_P_ITEM(pio);
	}

	if (((c!=-1)&&(inrange1p(currchar[s],c)))|| // if item is in a character's
		//pack (or subcontainer) and player is in range
		((c==-1)&&(inrange2(s,pio))))	// or item is in container on ground and
		// container is in range
	{
		Xsend(s, display3, 1);
		Xsend(s, bpitem, 19);
	}
	Weight->NewCalc(currchar[s]);	// Ison 2-20-99
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
			if(abs(chars[currchar[j]].pos.x-x)<=VISRANGE && abs(chars[currchar[j]].pos.y-y)<=VISRANGE) 
				Xsend(j, effect, 28);
		}
	}
}


void senditem(UOXSOCKET s, ITEM ii) // Send items (on ground) (old interface)
{
	const P_ITEM pi=MAKE_ITEMREF_LR(ii);	// on error return
	senditem( s,  pi);
}

void senditem(UOXSOCKET s, P_ITEM pi) // Send items (on ground)
{
	int j,pack,serial;
	unsigned char itmput[21]="\x1A\x00\x13\x40\x01\x02\x03\x20\x42\x00\x32\x06\x06\x06\x4A\x0A\x00\x00\x00";
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if ( pi->visible>=1 && !(pc_currchar->isGM()) )
	return;

	// meaning of the item's attribute visible
	// Visible 0 -> visible to everyone
	// Visible 1 -> only visible to owner and gm's (for owners normal for gm's grayish/hidden color)
	// visible 2 -> only visble to gm's (greyish/hidden color)

	if ( pi->visible>=2 && !(pc_currchar->isGM()) ) return; // workaround for missing gm-check client side for visibity since client 1.26.2

	if (!pi->isInWorld())
	{
		pack=1;
		if (isCharSerial(pi->contserial))
		{
			serial=pi->contserial;
			if(serial==-1) return;
			j=calcCharFromSer(serial);
			if (j!=-1)
				if (chars[j].serial==serial) pack=0;
		}
		if (pack)
		{
			if (pi->id1<0x40) // LB client crashfix, dont show multis in BP
								// we should better move it out of pack, but thats
								// only a first bannaid
			{
				sendbpitem(s,DEREF_P_ITEM(pi));
				return;
			}
		}
	}

	if (pi->isInWorld() && inrange2(s,pi) )
	{
		itmput[3]=(pi->ser1)+0x80; // Enable Piles
		itmput[4]=pi->ser2;
		itmput[5]=pi->ser3;
		itmput[6]=pi->ser4;

		//if player is a gm, this item
		//is shown like a candle (so that he can move it),
		//....if not, the item is a normal
		//invisible light source!
		if(pc_currchar->isGM() && pi->id()==0x1647)
		{///let's show the lightsource like a candle
			itmput[7]=0x0A;
			itmput[8]=0x0F;
		} else
		{//else like a normal item
			ShortToCharPtr(pi->id(),itmput+7);
		}

		itmput[9]=pi->amount>>8;
		itmput[10]=pi->amount%256;
		itmput[11]=pi->pos.x>>8;
		itmput[12]=pi->pos.x%256;
		itmput[13]=(pi->pos.y>>8)+0xC0; // Enable Dye and Move
		itmput[14]=pi->pos.y%256;
		itmput[15]=pi->pos.z;

		if(pc_currchar->isGM() && pi->id()==0x1647)
		{///let's show the lightsource like a blue item
			itmput[16]=0;
			itmput[17]=0xC6;
		} else
		{
			itmput[16]=pi->color1;
			itmput[17]=pi->color2;
		}

		itmput[18]=0;

		int dontsendcandidate=0;
		if (pi->visible==1)
		{
			if (!pc_currchar->Owns(pi))
			{
				dontsendcandidate=1;
				itmput[18]|=0x80;
			}
		}

		if (dontsendcandidate && !pc_currchar->isGM()) return; // LB 9-12-99, client 1.26.2 visibility correction

		if (pi->visible==2)
		{
			itmput[18]|=0x80;
		}


		if (pi->magic==1) itmput[18] |= 0x20;
		if (pc_currchar->priv2&1) itmput[18] |= 0x20;
		if ((pi->magic==3 || pi->magic==4) && pc_currchar->Owns(pi))
			itmput[18] |= 0x20;
		if (pc_currchar->priv2&4)
		{
			if (pi->id()>=0x4000 && pi->id()<=0x40FF) // LB, 25-dec-1999 litle bugfix for treasure multis, ( == changed to >=)
			{
				itmput[7]=0x14;
				itmput[8]=0xF0;
			}
		}

		int dir=0;
		if (pi->dir)
		{
			dir=1;
			itmput[19]=itmput[18];
			itmput[18]=itmput[17];
			itmput[17]=itmput[16];
			itmput[16]=itmput[15];
			itmput[15]=pi->dir;
			itmput[2]=0x14;
			itmput[11]|=0x80;
		}

		itmput[2]=0x13+dir;
		Xsend(s, itmput, 19+dir);

		if (IsCorpse(pi->id()))
		{
			backpack2(s, pi->ser1, pi->ser2, pi->ser3, pi->ser4);
		}
	}
}

// sends item in differnt color and position than it actually is
// used for LSd potions now, LB 5'th nov 1999
void senditem_lsd(UOXSOCKET s, ITEM i,char color1, char color2, int x, int y, signed char z)
{
	unsigned char itmput[20]="\x1A\x00\x13\x40\x01\x02\x03\x20\x42\x00\x32\x06\x06\x06\x4A\x0A\x00\x00\x00";
	const P_ITEM pi=MAKE_ITEMREF_LR(i);	// on error return
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if ( pi->visible>=1 && !(pc_currchar->isGM()) ) return; // workaround for missing gm-check client side for visibity since client 1.26.2
	// for lsd we dont need extra work for type 1 as in send_item

	if (pi->isInWorld())
	{
		itmput[3]=(pi->ser1)+0x80; // Enable Piles
		itmput[4]=pi->ser2;
		itmput[5]=pi->ser3;
		itmput[6]=pi->ser4;
		ShortToCharPtr(pi->id(),itmput+7);
		itmput[9]=pi->amount>>8;
		itmput[10]=pi->amount%256;
		itmput[11]=x>>8;
		itmput[12]=x%256;
		itmput[13]=(y>>8)+0xC0; // Enable Dye and Move
		itmput[14]=y%256;
		itmput[15]=z;
		itmput[16]=color1;
		itmput[17]=color2;
		itmput[18]=0;

		if (pi->visible==1)
		{
			if (!pc_currchar->Owns(pi))
			{
				itmput[18]|=0x80;
			}
		}
		if (pi->visible==2)
		{
			itmput[18]|=0x80;
		}

		if (pi->visible==3)
		{
			if ((pc_currchar->id1==0x03 && pc_currchar->id2==0xDB) || !pc_currchar->isGM())
				itmput[18]|=0x80;
		}

		if (pi->magic==1) itmput[18]+=0x20;

		if (pc_currchar->priv2&1) itmput[18]+=0x20;
		if ((pi->magic==3 || pi->magic==4) && pc_currchar->Owns(pi))
			itmput[18]+=0x20;
		if (pc_currchar->priv2&4)
		{
			if (pi->id()>=0x4000 && pi->id()<=0x40FF)
			{
				itmput[7]='\x14';
				itmput[8]='\xF0';
			}
		}
		if (pi->dir)
		{
			itmput[19]=itmput[18];
			itmput[18]=itmput[17];
			itmput[17]=itmput[16];
			itmput[16]=itmput[15];
			itmput[15]=pi->dir;
			itmput[2]=0x14;
			itmput[11]|=0x80;
			Xsend(s, itmput, 20);
		} else
		{
			itmput[2]=0x13;
			Xsend(s, itmput, 19);
		}

		if (IsCorpse(pi->id()) )
		{
			backpack2(s, pi->ser1, pi->ser2, pi->ser3, pi->ser4);
		}
	}

}

// LB 3-JULY 2000
void sendperson_lsd(UOXSOCKET s, CHARACTER c, char color1, char color2)
{
	int j, k,ci,color,c1,c2,b,cc1=0,cc2=0;
	unsigned char oc[1024];
	CHARACTER i=c;

	if ( (i < 0) || (i > cmem)) return;
	P_CHAR pc = MAKE_CHARREF_LR(i)
	if (s==-1) return;
	int sendit;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if (pc_currchar->isGM()) sendit=1; else // gm ? -> yes, send everything :)
	{ // no, -> dont show hidden & logged out chars
		if (pc->isHidden() && i!=currchar[s]) sendit=0; else sendit=1; // dont show hidden persons, even lsd'ed
		if ( (pc->isPlayer()) && !online(i)) sendit=0;
	}

	if (!online(i) && (pc->isPlayer()) && (pc_currchar->isGM())==0 ) 
	{
		sendit=0;
		removeitem[1]=pc->ser1;
		removeitem[2]=pc->ser2;
		removeitem[3]=pc->ser3;
		removeitem[4]=pc->ser4;
		Xsend(s, removeitem, 5);
	}

	if (!sendit) return;

	oc[0]=0x78; // Message type 78
	oc[3]=pc->ser1; // Character serial number
	oc[4]=pc->ser2; // Character serial number
	oc[5]=pc->ser3; // Character serial number
	oc[6]=pc->ser4; // Character serial number
	oc[7]=pc->id1; // Character art id
	oc[8]=pc->id2; // Character art id

	int x=pc->pos.x; /* | 0x8000;*/ // LB bugfix

	oc[9]= (x>>8);	// Character x position
	oc[10]=x%256; // Character x position
	oc[11]=pc->pos.y>>8; // Character y position
	oc[12]=pc->pos.y%256; // Character y position

	oc[13]=pc->pos.z; // Character z position
	oc[14]=pc->dir; // Character direction
	ShortToCharPtr(pc->skin, &oc[15]);	// Character skin color
	oc[17]=0; // Character flags

	/*if (pc->hidden || !(online(i)||pc->npc)) oc[17]=oc[17]|0x80;
	// Show hidden state correctly
	if (pc->poisoned) oc[17]=oc[17]|0x04; //AntiChrist -- thnx to SpaceDog */

	k=19;

	oc[18]=3;

	if (rand()%3==0) oc[18]=rand()%6;

	color = RandomNum(0, 0x03E9);
	color += RandomNum(0, 8);
	color %= 0x03E9;
	if (color < 0x0002 || color > 0x03E9)
		color = 0x03E9;
	if (color & 0xC000)
	{
		color = 0x1000;
		color |= RandomNum(0, 255);
	}
	ShortToCharPtr(color, &oc[15]);

	for (j=0;j<MAXLAYERS;j++) layers[j] = 0;

	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
		if (pi_j != NULL)
		{
			if (pc->Wears(pi_j) && (!pi_j->free))
			{
				if ( layers[pi_j->layer] == 0 )
				{
					LongToCharPtr(pi_j->serial,oc+k+0);
					ShortToCharPtr(pi_j->id(),oc+k+4);
					oc[k+6]=pi_j->layer;
					k += 7;
					if /*(pi_j->color1!=0 || pi_j->color2!=0)*/ (1==1)
					{
						oc[k-3] |= 0x80;

						if ( rand()%9<7 ) // send fake colors, 2/3 base color
						{
							oc[k+0]=color1;
							oc[k+1]=color2;
						}
						else // 1/3 new color
						{
							color=rand()%0x3e9;
							color+=rand()%8;
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
								oc[k+0]=c1;
								oc[k+1]=c2;
						}
						k=k+2;
					}
					layers[pi_j->layer] = 1;
				}
				else
				{
					;
				}
			}
		}
	}

	oc[k+0]=0;// Not well understood. It's a serial number. I set this to my serial number,
	oc[k+1]=0;// and all of my messages went to my paperdoll gump instead of my character's
	oc[k+2]=0;// head, when I was a character with serial number 0 0 0 1.
	oc[k+3]=0;
	k=k+4;

	oc[1]=k>>8;
	oc[2]=k%256;


	// without 0x1d client doesnt show 0x77 changes
	removeitem[0]=0x1d;
	removeitem[1]=pc->ser1;
	removeitem[2]=pc->ser2;
	removeitem[3]=pc->ser3;
	removeitem[4]=pc->ser4;

	Xsend(s, removeitem, 5);

	if (currchar[s]==i)
	{
		goxyz[0]=0x20;
		goxyz[1]=pc->ser1;
		goxyz[2]=pc->ser2;
		goxyz[3]=pc->ser3;
		goxyz[4]=pc->ser4;
		goxyz[5]=pc->id1;
		goxyz[6]=pc->id2;

		/*if (rand()%10==0)
		{
			goxyz[5]=0;
			goxyz[6]=rand()%0xff;
		}*/

		goxyz[8]=cc1;
		goxyz[9]=cc2;

		/*if(pc->poisoned) goxyz[10]=0x04; else goxyz[10]=0x00;	//AntiChrist -- thnx to SpaceDog
		if (pc->hidden) goxyz[10]=goxyz[10]|0x80;*/

		goxyz[10]=0;
		goxyz[11]=pc->pos.x>>8;
		goxyz[12]=pc->pos.x%256;
		goxyz[13]=pc->pos.y>>8;
		goxyz[14]=pc->pos.y%256;
		goxyz[17]=pc->dir|0x80;
		goxyz[18]=pc->pos.z;
		Xsend(s, goxyz, 19);
		walksequence[s]=-1;
	}


	Xsend(s, oc, k);

	extmove[0]=0x77;
	extmove[1]=pc->ser1;
	extmove[2]=pc->ser2;
	extmove[3]=pc->ser3;
	extmove[4]=pc->ser4;
	extmove[5]=pc->id1;
	extmove[6]=pc->id2;
	extmove[7]=pc->pos.x>>8;
	extmove[8]=pc->pos.x%256;
	extmove[9]=pc->pos.y>>8;
	extmove[10]=pc->pos.y%256;
	extmove[11]=pc->pos.z;
	extmove[12]=pc->dir|0x80;

	extmove[13]=cc1;
	extmove[14]=cc2;

	extmove[15]=0;
	extmove[16]=3;

	Xsend(s,extmove,17);
}

void chardel (UOXSOCKET s) // Deletion of character
{
	int k, tlen;
	unsigned int j,i;
	if (acctno[s]!=-1)
	{
		j=0;
		k=-1;
		for (i=0;i<charcount;i++)
		{
			if ((chars[i].account==acctno[s] && !chars[i].free))
			{
				if (j==buffer[s][0x22]) k=i;
				j++;
			}
		}
		if (k!=-1)
		{
			if (!SrvParms->checkcharage)
				Npcs->DeleteChar(k);
			else
				if ((chars[k].creationday+7) < getPlatformDay())
					Npcs->DeleteChar(k);
				else
				{
					unsigned char delete_error[2] = {0x85, 0x03};
					Xsend(s, delete_error, 2);
					return;
				}
		}

		{
			tlen=4+(5*60)+1+(startcount*63);
	
			login04a[1]=tlen>>8;
			login04a[2]=tlen%256;
			if (j>=1) login04a[3]=j--; else login04a[3]=0;

			Xsend(s, login04a, 4);
			j=0;
			for (i=0;i<charcount;i++)
			{
				if ( chars[i].account==acctno[s] && !chars[i].free)
				{
					strcpy((char*)login04b, chars[i].name);
					Xsend(s, login04b, 60);
					j++;
				}
			}

			memset(&login04b[0], 60, sizeof(unsigned char));
			for (i=j;i<5;i++)
			{
				Xsend(s, login04b, 60);
			}

			buffer[s][0]=startcount;
			Xsend(s, buffer[s], 1);
			for (i=0;i<startcount;i++)
			{
				login04d[0]=i;
				for (j=0;j<=strlen(start[i][0]);j++) login04d[j+1]=start[i][0][j];
				for (j=0;j<=strlen(start[i][1]);j++) login04d[j+32]=start[i][1][j];
				Xsend(s, login04d, 63);
			}
		}
	}
}

void textflags (int s, int i, char *name)
{
	int a1, a2, a3, a4;
	char name2[150];

    P_CHAR pc = MAKE_CHARREF_LR(i);

	a1=pc->ser1;
	a2=pc->ser2;
	a3=pc->ser3;
	a4=pc->ser4;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	*(name2)='\0';//AntiChrist

	if( pc->isPlayer() && !pc->isGMorCounselor() && (pc->fame >= 10000 ) ) // Morollan, only normal players have titles now
	{ // adding Lord/Lady to title overhead
		if (pc->id2==0x91) 
		{ 
			if (strcmp(title[9].other,"")) sprintf(name2,"%s ",title[9].other);
		}
		else if (pc->id2==0x90) 
		{
			if (strcmp(title[10].other,"")) sprintf(name2,"%s ",title[10].other);
		}
	
	}

	strcat(name2,name);//AntiChrist

	if (pc->isInvul() && pc->account!=0)     {  if (strcmp(title[11].other,"")) sprintf((char*)temp, " [%s]",title[11].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); } // ripper
	if (pc->account==0 && pc->isGM())		{  if (strcmp(title[12].other,"")) sprintf((char*)temp, " [%s]",title[12].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); } // ripper
	if (pc->priv2&2)                           {  if (strcmp(title[13].other,"")) sprintf((char*)temp, " [%s]",title[13].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); }
	if (pc->guarded)                           {  if (strcmp(title[14].other,"")) sprintf((char*)temp, " [%s]",title[14].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); } // Ripper
	if (pc->tamed && pc->npcaitype==32 
		&& pc_currchar->Owns(pc) && pc_currchar->guarded) 
													{ if  (strcmp(title[15].other,"")) sprintf((char*)temp, " [%s]",title[15].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); } // Ripper
	if (pc->tamed && pc->npcaitype!=17 )  { if  (strcmp(title[16].other,"")) sprintf((char*)temp, " [%s]",title[16].other); strcat(name2,(char*)temp); }
	if (pc->war)                               { if  (strcmp(title[17].other,"")) sprintf((char*)temp, " [%s]",title[17].other); strcat(name2,(char*)temp); } // ripper
	if ((pc->crimflag>0)&&(pc->kills<repsys.maxkills)) 
													{ if  (strcmp(title[18].other,"")) sprintf((char*)temp, " [%s]",title[18].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); }// ripper
	if (pc->kills>=repsys.maxkills)            { if  (strcmp(title[19].other,"")) sprintf((char*)temp, " [%s]",title[19].other); else sprintf((char*)temp,""); strcat(name2,(char*)temp); } // AntiChrist

	Guilds->Title(s,i);

	int tl,guild,race;
	tl=44+strlen(name2)+1;
	talk[1]=tl>>8;//AntiChrist
	talk[2]=tl%256;
	talk[3]=a1;
	talk[4]=a2;
	talk[5]=a3;
	talk[6]=a4;
	talk[7]=1;
	talk[8]=1;
	talk[9]=6; // Mode: "You see"
	guild=Guilds->Compare(currchar[s],i);
	race = Races.CheckRelation(pc, pc_currchar);
	if (guild == 1 || race == 1) //Same guild (Green)
	{
		talk[10]=0x00;
		talk[11]=0x43;
	}
	else if (guild==2 || race == 2) //enemy (Orange)
	{
		talk[10]=0x00;
		talk[11]=0x30;
	}
	else if( pc->isGM() && pc->account==0 )
	{
		talk[10]=0x00;
		talk[11]=0x35;//Admin & GM get yellow names ..Ripper
	}
	else
	{
		switch(pc->flag)
		{
		case 0x01:	talk[10]=0x00;talk[11]=0x26;break;//red
		case 0x04:	talk[10]=0x00;talk[11]=0x5A;break;//blue
		case 0x08:	talk[10]=0x00;talk[11]=0x49;break;//green
		case 0x10:	talk[10]=0x00;talk[11]=0x30;break;//orange
		default:	talk[10]=0x03;talk[11]=0xB2;		//grey
		}
	}
	talk[12]=0;
	talk[13]=3;
	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, name2, strlen(name2)+1);
}

void teleport(int s) // Teleports character to its current set coordinates
{
	int i;
	P_CHAR pc = MAKE_CHARREF_LR(s)
	UOXSOCKET k = calcSocketFromChar(pc);
	if (k!=-1)	// If a player, move them to the appropriate XYZ
	{
		LongToCharPtr(pc->serial, &removeitem[1]);

		LongToCharPtr(pc->serial, &goxyz[1]);
		goxyz[5]=pc->id1;
		goxyz[6]=pc->id2;
		ShortToCharPtr(pc->skin, &goxyz[8]);
		if(pc->poisoned) 
			goxyz[10] |= 0x04; 
		else 
			goxyz[10] = 0x00; //AntiChrist -- thnx to SpaceDog
		if (pc->isHidden()) 
			goxyz[10] |= 0x80;
		goxyz[11]=pc->pos.x>>8;
		goxyz[12]=pc->pos.x%256;
		goxyz[13]=pc->pos.y>>8;
		goxyz[14]=pc->pos.y%256;
		goxyz[17]=pc->dir|0x80;
		goxyz[18]=pc->dispz;
		Xsend(k, goxyz, 19);
		Weight->NewCalc(s);	// Ison 2-20-99
		statwindow(k, s);	// Ison 2-20-99
		walksequence[k]=-1;
	}
	for (i=0;i<now;i++) // Send the update to all players.
	{
		// Dupois - had to remove the && (k!=i)), doesn update the client
		// Added Oct 08, 1998
		if (perm[i])
		{
		   Xsend(i, removeitem, 5);
		   if (inrange1p(s, currchar[i])) 
			   impowncreate(i, DEREF_P_CHAR(pc), 1);
		}
	}
	

	if (k!=-1)
	{
		//Char mapRegions
		int	StartGrid=mapRegions->StartGrid(pc->pos.x,pc->pos.y);
		unsigned int increment=0;
		for (unsigned int checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
		{
			for (int a=0;a<3;a++)
			{
				vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
				for ( unsigned int w = 0; w < vecEntries.size(); w++)
				{
					P_CHAR mapchar = FindCharBySerial(vecEntries[w]);
					P_ITEM mapitem = FindItemBySerial(vecEntries[w]);
					if (mapchar != NULL)
					{
						i = DEREF_P_CHAR(mapchar);
						if ((chars[i].isNpc()||online(i)||pc->isGM())&&(s!=i)&&(inrange1p(s, i)))
						{
							impowncreate(k, i, 1);
						}
					} else if (mapitem != NULL) {
						if(iteminrange(k, DEREF_P_ITEM(mapitem),Races[pc->race]->VisRange))
						{
							senditem(k, DEREF_P_ITEM(mapitem));
						}
					}
				}
			}
		}
		if (perm[k]) dolight(k, worldcurlevel);
	}
	checkregion(s);
}

void teleport2(CHARACTER s) // used for /RESEND only - Morrolan, so people can find their corpses
{
	int i;
	P_CHAR pc = MAKE_CHARREF_LR(s)
	UOXSOCKET k = calcSocketFromChar(pc);

	for (i=0;i<now;i++)
		if ((perm[i])&&(i!=k))
		{
			removeitem[1]=pc->ser1;
			removeitem[2]=pc->ser2;
			removeitem[3]=pc->ser3;
			removeitem[4]=pc->ser4;
			Xsend(i, removeitem, 5);
		}
		if (k!=-1)	// If a player, move them to the appropriate XYZ
		{
			goxyz[1]=pc->ser1;
			goxyz[2]=pc->ser2;
			goxyz[3]=pc->ser3;
			goxyz[4]=pc->ser4;
			goxyz[5]=pc->id1;
			goxyz[6]=pc->id2;
			ShortToCharPtr(pc->skin, &goxyz[8]);
			goxyz[10]=0;
			if (pc->isHidden()) goxyz[10]=0x80;
			goxyz[11]=pc->pos.x>>8;
			goxyz[12]=pc->pos.x%256;
			goxyz[13]=pc->pos.y>>8;
			goxyz[14]=pc->pos.y%256;
			goxyz[17]=pc->dir|0x80;
			goxyz[18]=pc->dispz;
			Xsend(k, goxyz, 19);
			all_items(k);
			Weight->NewCalc(DEREF_P_CHAR(pc));	// Ison 2-20-99
			statwindow(k, DEREF_P_CHAR(pc));	// Ison 2-20-99
			walksequence[k]=-1;
		}
		for (i=0;i<now;i++) // Send the update to all players.
		{
			 // Dupois - had to remove the && (k!=i)), doesn update the client
			 // Added Oct 08, 1998
			if (perm[i])
			{			 
			   if (inrange1p(DEREF_P_CHAR(pc), currchar[i]))
			   {
				 impowncreate(i, DEREF_P_CHAR(pc), 1);
			   }
			}
		}

		if (k!=-1)
		{
			AllCharsIterator iter_char;
			for (iter_char.Begin(); iter_char.GetData() != NULL; iter_char++)
			{ //Tauriel only send inrange people (walking takes care of out of view)
				P_CHAR pc_i = iter_char.GetData();
				if ( ( online(DEREF_P_CHAR(pc_i)) || pc_i->isNpc() || pc->isGM()) && (pc->serial!= pc_i->serial) && (inrange1p(s, DEREF_P_CHAR(pc_i))))
				{
					impowncreate(k, DEREF_P_CHAR(pc_i), 1);
				}
			}
			if (perm[k]) dolight(k, worldcurlevel);
		}
		checkregion(DEREF_P_CHAR(pc));
}


void updatechar(CHARACTER c) // If character status has been changed (Polymorph), resend him
{
	int i;
	P_CHAR pc = MAKE_CHARREF_LR(c);
	setcharflag(pc);//AntiChrist - bugfix for highlight color not being updated

	for (i=0;i<now;i++)
		if (perm[i] && inrange1p(currchar[i], c))
		{
			removeitem[1]=pc->ser1;
			removeitem[2]=pc->ser2;
			removeitem[3]=pc->ser3;
			removeitem[4]=pc->ser4;
			Xsend(i, removeitem, 5);
			if (currchar[i]==c)
			{
				goxyz[1]=pc->ser1;
				goxyz[2]=pc->ser2;
				goxyz[3]=pc->ser3;
				goxyz[4]=pc->ser4;
				goxyz[5]=pc->id1;
				goxyz[6]=pc->id2;
				ShortToCharPtr(pc->skin, &goxyz[8]);
				if(pc->poisoned) goxyz[10]=0x04; else goxyz[10]=0x00;	//AntiChrist -- thnx to SpaceDog
				if (pc->isHidden()) goxyz[10]=goxyz[10]|0x80;
				goxyz[11]=pc->pos.x>>8;
				goxyz[12]=pc->pos.x%256;
				goxyz[13]=pc->pos.y>>8;
				goxyz[14]=pc->pos.y%256;
				goxyz[17]=pc->dir|0x80;
				goxyz[18]=pc->pos.z;
				Xsend(i, goxyz, 19);
				walksequence[i]=-1;
			}
			if (inrange1p(c, currchar[i]))
			{
				impowncreate(i, DEREF_P_CHAR(pc), 0);			
			}
		}
}


void target(UOXSOCKET s, int a1, int a2, int a3, int a4, char *txt) // Send targetting cursor to client
{
	unsigned char tarcrs[20]="\x6C\x01\x40\x01\x02\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	targetok[s]=1;
	tarcrs[2]=a1;
	tarcrs[3]=a2;
	tarcrs[4]=a3;
	tarcrs[5]=a4;
	sysmessage(s, txt);
	Xsend(s, tarcrs, 19);
}

void skillwindow(int s) // Opens the skills list, updated for client 1.26.2b by LB
{
	int i;
	unsigned char skillstart[5]="\x3A\x01\x5d\x00"; // hack for that 3 new skills+1.26.2 client, LB 4'th dec 1999
	unsigned char skillmid[8] = "\x00\x00\x00\x00\x00\x00\x00"; // changed for 1.26.2 clients [size 7 insted of 4]
	unsigned char skillend[3]="\x00\x00";
	char x;

	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	Xsend(s, skillstart, 4);
	for (i=0;i<TRUESKILLS;i++)
	{
		Skills->updateSkillLevel(DEREF_P_CHAR(pc_currchar), i);
		skillmid[1]=i+1;
		skillmid[2]=pc_currchar->skill[i]>>8;
		skillmid[3]=pc_currchar->skill[i]%256;
		skillmid[4]=pc_currchar->baseskill[i]>>8;
		skillmid[5]=pc_currchar->baseskill[i]%256;

		x=pc_currchar->lockSkill[i];
		if (x!=0 && x!=1 && x!=2) x=0;
		skillmid[6]=x; // leave it unlocked, regardless
		Xsend(s, skillmid, 7);
	}
	Xsend(s, skillend, 2);
}


void updatestats( CHARACTER c, char x )
{
	int i, a = 0, b = 0;
	char updater[10]="\xA1\x01\x02\x03\x04\x01\x03\x01\x02";
	
	P_CHAR pc = MAKE_CHARREF_LR(c)


	switch (x)
	{
	case 0:
		a=pc->st;
		b=pc->hp;
		break;
	case 1:
		a=pc->in;
		b=pc->mn;
		break;
	case 2:
		a=pc->effDex();
		b=pc->stm;
		break;
	}
	
	updater[0]=0xA1+x;
	updater[1]=pc->ser1;
	updater[2]=pc->ser2;
	updater[3]=pc->ser3;
	updater[4]=pc->ser4;
	updater[5]=a>>8;
	updater[6]=a%256;
	updater[7]=b>>8;
	updater[8]=b%256;
	if (x == 0)  //Send to all, only if it's Health change
	{
		for (i=0;i<now;i++) 
			if (perm[i] && inrange1p(currchar[i], c) ) 
				Xsend(i, updater, 9);
	} else {
		UOXSOCKET s = calcSocketFromChar(pc);
		if (s != -1)
			Xsend(s, updater, 9);
	}
}

void statwindow(int s, int i) // Opens the status window
{
	int x;
	unsigned char statstring[67]="\x11\x00\x42\x00\x05\xA8\x90XYZ\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x12\x00\x34\xFF\x01\x00\x00\x5F\x00\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x00\x75\x30\x01\x2C\x00\x00";
	bool ghost;
	
	if (s<0 || s>=MAXCLIENT || i<0 || i>cmem) return; // lb, fixes a few (too few) -1 crashes ...

	P_CHAR pc = MAKE_CHARREF_LR(i);
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if ((pc->id1==0x01 && pc->id2==0x92) || (pc->id1==0x01 && pc->id2==0x93)) ghost = true; else ghost = false;

	statstring[3]=pc->ser1;
	statstring[4]=pc->ser2;
	statstring[5]=pc->ser3;
	statstring[6]=pc->ser4;
	strncpy((char*)&statstring[7],pc->name, 30); // can not be more than 30 at least no without changing packet lenght

	if (!ghost)
	{
		statstring[37]=pc->hp>>8;
		statstring[38]=pc->hp%256;
	} 
	else
	{
		statstring[37] = statstring[38] = 0;
	}
	//Changed, so ghosts can see their maximum hit points.
	statstring[39]=pc->st>>8;
	statstring[40]=pc->st%256;

	if (((pc_currchar->isGM())|| pc_currchar->Owns(pc))&&(currchar[s]!=i))
	{
		statstring[41]=0xFF;
	} 
	else if (pc_currchar->Owns(pc) && currchar[s]!=i ) //Morrolan - from Banter
	{
		statstring[41]=0xFF;
	}
	else
	{
		statstring[41]=0x00;
	}

	if (ghost) statstring[41]=0x00;

	// packet #42 has some problems, dont try to be smart and replace the workaround by
	// if (ghost) statstring[42]=0; else statstring[42]=1, LB

	if ((pc->id1==0x01)&&(pc->id2==0x91)) statstring[43]=1;
	else if ((pc->id1==0x01) && (pc->id2==0x93)) statstring[43]=1;
	else statstring[43]=0; // LB, prevents very female looking male players ... :-)

	//Changed so ghosts can see their str, dex and int, their char haven't lose those attributes.
	statstring[44]=pc->st>>8;
	statstring[45]=pc->st%256;
	statstring[46]=pc->effDex()>>8;
	statstring[47]=pc->effDex()%256;
	statstring[48]=pc->in>>8; // Real INT
	statstring[49]=pc->in%256;

	if (!ghost)
	{
		statstring[50]=pc->stm>>8;
		statstring[51]=pc->stm%256;
		statstring[54]=pc->mn>>8;
		statstring[55]=pc->mn%256;
	}
	else
	{
		// Sets to 0 stamina and mana
		for (int a = 50; a <= 57; a++) 
			statstring[a] = 0;
	}
	// ghosts will see their mana as 0/x, ie 0/100
	// This will show red bars when status are displayed as percentages (little status window)
	statstring[52]=pc->effDex()>>8; // MaxStamina
	statstring[53]=pc->effDex()%256;
	statstring[56]=pc->in>>8; // MaxMana
	statstring[57]=pc->in%256;

	
	x = pc->CountGold();
	statstring[58]=x>>24;
	statstring[59]=x>>16;
	statstring[60]=x>>8;
	statstring[61]=x%256;
	
	x = Combat->CalcDef(pc,0);
	statstring[62]=x>>8; // AC
	statstring[63]=x%256;
	x = (int)(pc->weight);
	statstring[64]=x>>8;
	statstring[65]=x%256;
	Xsend(s, statstring, 66);
	
}

void updates(UOXSOCKET s) // Update Window
{
	int x, y, pos, j;
	char temp[512];

	openscript("misc.scp");
	if (!i_scripts[misc_script]->find("MOTD"))
	{
		closescript();
		return;
	}
	pos=ftell(scpfile);
	x=-1;
	y=-2;
	unsigned long loopexit=0;
	do
	{
		read1();
		x++;
		y+=strlen((char*)script1)+1;
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
	y+=10;
	fseek(scpfile, pos, SEEK_SET);
	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=2;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;
	Xsend(s, updscroll, 10);
	for (j=0;j<x;j++)
	{
		read1();
		sprintf(temp, "%s ", script1);
		Xsend(s, temp, strlen(temp));
	}
	closescript();
}

void tips(int s, int i) // Tip of the day window
{
	int x, y, pos, j;
	char temp[512];

	if (i==0) i=1;
	openscript("misc.scp");
	if (!i_scripts[misc_script]->find("TIPS"))
	{
		closescript();
		return;
	}
	x=i;
	unsigned long loopexit=0;
	do
	{
		read2();
		if (!(strcmp("TIP", (char*)script1))) x--;
	}
	while ((x>0)&&script1[0]!='}'&&script1[0]!=0 && (++loopexit < MAXLOOPS) );
	closescript();
	if (!(strcmp("}", (char*)script1)))
	{
		tips(s, 1);
		return;
	}
	openscript("misc.scp");
	sprintf(temp, "TIP %i", str2num(script2));
	if (!i_scripts[misc_script]->find(temp))
	{
		closescript();
		return;
	}
	pos=ftell(scpfile);
	x=-1;
	y=-2;
	loopexit=0;
	do
	{
		read1();
		x++;
		y+=strlen((char*)script1)+1;
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
	y+=10;
	fseek(scpfile, pos, SEEK_SET);
	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=0;
	updscroll[7]=i;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;
	Xsend(s, updscroll, 10);
	for (j=0;j<x;j++)
	{
		read1();
		sprintf(temp, "%s ", script1);
		Xsend(s, temp, strlen(temp));
	}
	closescript();
}


void deny(UOXSOCKET k, CHARACTER s, int sequence)
{
	unsigned char walkdeny[9]="\x21\x00\x01\x02\x01\x02\x00\x01";

	P_CHAR pc = MAKE_CHARREF_LR(s)

	walkdeny[1]=sequence;
	walkdeny[2]=pc->pos.x>>8;
	walkdeny[3]=pc->pos.x%256;
	walkdeny[4]=pc->pos.y>>8;
	walkdeny[5]=pc->pos.y%256;
	walkdeny[6]=pc->dir;
	walkdeny[7]=pc->dispz;
	Xsend(k, walkdeny, 8);
	walksequence[k]=-1;
}

void weblaunch(int s, char *txt) // Direct client to a web page
{
	int l;
	char launchstr[4]="\xA5\x00\x00";

	sysmessage(s, "Launching your web browser. Please wait...");
	l=strlen(txt)+4;
	launchstr[1]=l>>8;
	launchstr[2]=l%256;
	Xsend(s, launchstr, 3);
	Xsend(s, txt, strlen(txt)+1);
}

void broadcast(int s) // GM Broadcast (Done if a GM yells something)
{
	int i,tl;
	unsigned char nonuni[512];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if(pc_currchar->unicode)
		for (i=13;i<(buffer[s][1]<<8)+buffer[s][2];i=i+2)
		{
			nonuni[(i-13)/2]=buffer[s][i];
		}
		if(!(pc_currchar->unicode))
		{
			tl=44+strlen((char*)&buffer[s][8])+1;
			talk[1]=tl>>8;
			talk[2]=tl%256;
			talk[3]=pc_currchar->ser1;
			talk[4]=pc_currchar->ser2;
			talk[5]=pc_currchar->ser3;
			talk[6]=pc_currchar->ser4;
			talk[7]=pc_currchar->id1;
			talk[8]=pc_currchar->id2;
			talk[9]=1;
			talk[10]=buffer[s][4];
			talk[11]=buffer[s][5];
			talk[12]=buffer[s][6];
			talk[13]=pc_currchar->fonttype;
			for (i=0;i<now;i++)
			{
				if (perm[i])
				{
					Xsend(i, talk, 14);
					Xsend(i, pc_currchar->name, 30);
					Xsend(i, &buffer[s][8], strlen((char*)&buffer[s][8])+1);
				}
			}
		} // end unicode IF
		else
		{
			tl=44+strlen((char*)&nonuni[0])+1;

			talk[1]=tl>>8;
			talk[2]=tl%256;
			talk[3]=pc_currchar->ser1;
			talk[4]=pc_currchar->ser2;
			talk[5]=pc_currchar->ser3;
			talk[6]=pc_currchar->ser4;
			talk[7]=pc_currchar->id1;
			talk[8]=pc_currchar->id2;
			talk[9]=1;
			talk[10]=buffer[s][4];
			talk[11]=buffer[s][5];
			talk[12]=buffer[s][6];
			talk[13]=pc_currchar->fonttype;

			for (i=0;i<now;i++)
			{
				if (perm[i])
				{
					Xsend(i, talk, 14);
					Xsend(i, pc_currchar->name, 30);
					Xsend(i, &nonuni[0], strlen((char*)&nonuni[0])+1);
				}
			}
		}
}

void itemtalk(int s, int item, char *txt) // Item "speech"
{
	int tl;
	const P_ITEM pi=MAKE_ITEMREF_LR(item);	// on error return

	if (s<=-1) return;

	tl=44+strlen(txt)+1;
	talk[1]=tl>>8;
	talk[2]=tl%256;
	LongToCharPtr(pi->serial,talk+3);
	ShortToCharPtr(pi->id(),talk+7);
	talk[9]=0; // Type
	talk[10]=0x04;
	talk[11]=0x81;
	talk[12]=0;
	talk[13]=3;
	Xsend(s, talk, 14);
	Xsend(s, pi->name, 30);
	Xsend(s, txt, strlen(txt)+1);
}

void npctalk(int s, cChar* pNpc, char *txt,char antispam) // NPC speech
{
	npctalk(s,DEREF_P_CHAR(pNpc),txt,antispam);
}

void npctalk(int s, int npc, char *txt,char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (npc==-1 || s==-1) return; //lb

	P_CHAR pc_npc      = MAKE_CHARREF_LR(npc);
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if (antispam)
	{
		if (pc_npc->antispamtimer<uiCurrentTime)
		{
			pc_npc->antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
		tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		talk[3]=pc_npc->ser1;
		talk[4]=pc_npc->ser2;
		talk[5]=pc_npc->ser3;
		talk[6]=pc_npc->ser4;
		talk[7]=pc_npc->id1;
		talk[8]=pc_npc->id2;
		talk[9]=0; // Type
		pc_npc->saycolor = 0x0481;

		talk[12]=0;
		talk[13]=pc_currchar->fonttype;

		if (pc_npc->npcaitype==2 && server_data.BadNpcsRed == 0) //bad npcs speech (red)..Ripper
		{
			talk[10]=pc_npc->saycolor = 0x03B2;
		}
		else if (pc_npc->npcaitype==2 && server_data.BadNpcsRed == 1)
		{
			talk[10]=pc_npc->saycolor = 0x0026;
		}
		else if(pc_npc->isNpc() && !pc_npc->tamed && !pc_npc->guarded && !pc_npc->war)
		{
			pc_npc->saycolor = 0x005b;
		}

		ShortToCharPtr(pc_npc->saycolor, &talk[10]);
		Xsend(s, talk, 14);
		Xsend(s, pc_npc->name, 30);
		Xsend(s, txt, strlen(txt)+1);
	}
}

void npctalkall(cChar* pc, char *txt,char antispam) // NPC speech to all in range.
{
	npctalkall(DEREF_P_CHAR(pc), txt, antispam);
}

void npctalkall(int npc, char *txt,char antispam) // NPC speech to all in range.
{
	if (npc==-1) return;

	int i;

	for (i=0;i<now;i++)
		if (inrange1p(npc, currchar[i])&&perm[i])
			npctalk(i, npc, txt,antispam);
}

void npctalk_runic(int s, int npc, char *txt,char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (npc==-1 || s==-1) return; //lb
	P_CHAR pc_npc = MAKE_CHARREF_LR(npc);

	if (antispam)
	{
		if (pc_npc->antispamtimer<uiCurrentTime)
		{
			pc_npc->antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
		tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		talk[3]=pc_npc->ser1;
		talk[4]=pc_npc->ser2;
		talk[5]=pc_npc->ser3;
		talk[6]=pc_npc->ser4;
		talk[7]=pc_npc->id1;
		talk[8]=pc_npc->id2;
		talk[9]=0;

		// color here

		talk[10]=0;
		talk[11]=1; // black

		// big problems with contrast/readability, plz find out a better one, LB !

		talk[12]=0;
		talk[13]=8;

		Xsend(s, talk, 14);
		Xsend(s, pc_npc->name, 30);
		Xsend(s, txt, strlen(txt)+1);
	}
}

void npcemote(int s, int npc, char *txt, char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (s==-1 || npc==-1) return;

	P_CHAR pc_npc = MAKE_CHARREF_LR(npc);
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if (antispam)
	{
		if (pc_npc->antispamtimer<uiCurrentTime)
		{
			pc_npc->antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
		tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		talk[3]=pc_npc->ser1;
		talk[4]=pc_npc->ser2;
		talk[5]=pc_npc->ser3;
		talk[6]=pc_npc->ser4;
		talk[7]=pc_npc->id1;
		talk[8]=pc_npc->id2;
		talk[9]=2; // Type
		talk[10]=pc_npc->emotecolor1=0x00;
		talk[11]=pc_npc->emotecolor2=0x26;
		talk[12]=0;
		talk[13]=pc_currchar->fonttype;
		Xsend(s, talk, 14);
		Xsend(s, pc_npc->name, 30);
		Xsend(s, txt, strlen(txt)+1);
	}
}

// Last touch: LB 8'th April 2001 for particleSystem

// if UO_3DonlyEffect is true, sta has to be valid and contain particleSystem data (if not, crash)
// for particleSystem data layout see staticeffectUO3d and updated packetDoku
// for old 2d staticeffect stuff , the new (3d client) pararamters UO_3Donlyeffect, sta and skip_old are defaulted in such a way that they behave like they did before
// simply dont set them in that case
// the last parameter is for particlesystem optimization only (dangerous). don't use unless you know 101% what you are doing.

void staticeffect(CHARACTER player, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop,  bool UO3DonlyEffekt, stat_st *sta, bool skip_old)
{
	int a0,a1,a2,a3,a4;
	char effect[29];
	int j;
	P_CHAR pc_player = MAKE_CHARREF_LR(player);

	if (!skip_old)
	{
		memset(&effect[0], 0, 29);	
	    effect[0]=0x70; // Effect message
	    effect[1]=0x03; // Static effect
	    effect[2]=pc_player->ser1;
	    effect[3]=pc_player->ser2;
	    effect[4]=pc_player->ser3;
	    effect[5]=pc_player->ser4;
	    //[6] to [9] are the target ser, not applicable here.
	    effect[10]=eff1;// Object id of the effect
	    effect[11]=eff2;
	    effect[12]=pc_player->pos.x>>8;
	    effect[13]=pc_player->pos.x%256;
	    effect[14]=pc_player->pos.y>>8;
	    effect[15]=pc_player->pos.y%256;
	    effect[16]=pc_player->pos.z;
	    //[17] to [21] are the target's position, not applicable here.
	    effect[22]=speed;
	    effect[23]=loop; // 0 is really long. 1 is the shortest.
	    effect[24]=0; // This value is unknown
	    effect[25]=0; // This value is unknown
	    effect[26]=1; // CLient side crashfix
	    effect[27]=0; // This value is used for moving effects that explode on impact.
	 }

	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {
	   for (j=0;j<now;j++)
	   {
		 if ((inrange1p(currchar[j],player))&&(perm[j]))
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
		 if ((inrange1p(currchar[j],player))&&(perm[j]))
		 {
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd 
			 {
				 Xsend(j, effect, 28);

			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
			 {

				staticeffectUO3D(player, sta);

				// allow to fire up to 4 layers at same time (like on OSI servers)
				a0 = sta->effect[10];

				a1 = ( ( a0 & 0x000000ff )       );
				a2 = ( ( a0 & 0x0000ff00 ) >> 8  );
				a3 = ( ( a0 & 0x00ff0000 ) >> 16 );
				a4 = ( ( a0 & 0xff000000 ) >> 24 );

				if (a1!=0xff) { particleSystem[46] = a1; Xsend(j, particleSystem, 49); } 					
				if (a2!=0xff) { particleSystem[46] = a2; Xsend(j, particleSystem, 49); }				
				if (a3!=0xff) { particleSystem[46] = a3; Xsend(j, particleSystem, 49); }
				if (a4!=0xff) { particleSystem[46] = a4; Xsend(j, particleSystem, 49); }

				//sprintf(temp, "a0: %x a1: %x a2: %x a3: %x a4: %x \n",a0,a1,a2,a3,a4);
				//ConOut(temp);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); } // attention: a simple else is wrong !
		 } // end inrange
	   } // end for
	} // end UO:3D effect

	// remark: if a UO:3D effect is send and ALL clients are UO:3D ones, the pre-calculation of the 2-d packet
	// is redundant. but we can never know, and probably it will take years till the 2d cliet dies.
	// I think it's too infrequnet to consider this as optimization.
}


void movingeffect(int source, int dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt, move_st *str, bool skip_old )
{
	
	char effect[29];
	int j;

	P_CHAR pc_source = MAKE_CHARREF_LR(source);
	P_CHAR pc_dest   = MAKE_CHARREF_LR(dest);


	if (!skip_old)
	{
	   effect[0]=0x70; // Effect message
	   effect[1]=0x00; // Moving effect
	   effect[2]=pc_source->ser1;
	   effect[3]=pc_source->ser2;
	   effect[4]=pc_source->ser3;
	   effect[5]=pc_source->ser4;
	   effect[6]=pc_dest->ser1;
	   effect[7]=pc_dest->ser2;
	   effect[8]=pc_dest->ser3;
	   effect[9]=pc_dest->ser4;
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
		 if ( (inrange1p(currchar[j],DEREF_P_CHAR(pc_source)))&&(inrange1p(currchar[j],DEREF_P_CHAR(pc_dest)))&&(perm[j]))
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
		 if ( (inrange1p(currchar[j],DEREF_P_CHAR(pc_source)))&&(inrange1p(currchar[j],DEREF_P_CHAR(pc_dest)))&&(perm[j]))
		 {
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd 
			 {
				 Xsend(j, effect, 28);

			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
			 {

				movingeffectUO3D(DEREF_P_CHAR(pc_source), DEREF_P_CHAR(pc_dest), str);			
				Xsend(j, particleSystem, 49);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
		 }
	   }
	}		

}

void bolteffect(int player, bool UO3DonlyEffekt, bool skip_old )
{
	char effect[29];
	int  j;

	P_CHAR pc_player = MAKE_CHARREF_LR(player);

	if (!skip_old)
	{
	  memset(&effect[0], 0, 29);	
	  effect[0]=0x70; // Effect message
	  effect[1]=0x01; // Bolt effect
	  effect[2]=pc_player->ser1;
	  effect[3]=pc_player->ser2;
	  effect[4]=pc_player->ser3;
	  effect[5]=pc_player->ser4;
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
		 if ((inrange1p(currchar[j],player))&&(perm[j]))
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
		 if ((inrange1p(currchar[j],player))&&(perm[j]))
		 {
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd 
			 {
				 Xsend(j, effect, 28);

			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
			 {

				bolteffectUO3D(player);			
				Xsend(j, particleSystem, 49);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
		 }
	   }
	}		
}


// staticeffect2 is for effects on items
void staticeffect2(P_ITEM pi, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt,  stat_st *str, bool skip_old )
{
	char effect[29];
	int j;
	
	if (!skip_old)
	{
		memset(&effect[0], 0, 29);
		effect[0]=0x70; // Effect message
		effect[1]=0x02; // Static effect
		effect[2]=pi->ser1;
		effect[3]=pi->ser2;
		effect[4]=pi->ser3;
		effect[5]=pi->ser4;
		effect[6]=pi->ser1;
		effect[7]=pi->ser2;
		effect[8]=pi->ser3;
		effect[9]=pi->ser4;
		//[6] to [9] are the target ser, not applicable here.
		effect[10]=eff1;// Object id of the effect
		effect[11]=eff2;
		effect[12]=pi->pos.x>>8;
		effect[13]=pi->pos.x%256;
		effect[14]=pi->pos.y>>8;
		effect[15]=pi->pos.y%256;
		effect[16]=pi->pos.z;
		effect[17]=pi->pos.x>>8;
		effect[18]=pi->pos.x%256;
		effect[19]=pi->pos.y>>8;
		effect[20]=pi->pos.y%256;
		effect[21]=pi->pos.z;
		//[17] to [21] are the target's position, not applicable here.
		effect[22]=speed;
		effect[23]=loop; // 0 is really long. 1 is the shortest.
		effect[24]=0; // This value is unknown
		effect[25]=0; // This value is unknown
		effect[26]=1; // LB, client side crashfix
		effect[27]=explode; // This value is used for moving effects that explode on impact.
	}
	
	if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	{
		for (j=0;j<now;j++)
		{
			if (inrange2(j,pi) && perm[j])
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
			if (inrange2(j,pi) && perm[j])
			{
				if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd 
				{
					Xsend(j, effect, 28);
				}
				else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
				{
					itemeffectUO3D(pi, str);			
					Xsend(j, particleSystem, 49);
				}
				else if (clientDimension[j] != 2 && clientDimension[j] !=3 )
				{ sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
			}
		}
	}		
}


void bolteffect2(int player,char a1,char a2)	// experimenatal, lb
{
	char effect[29] = {0,};
	int j,x2,x,y2,y;
	P_CHAR pc_player = MAKE_CHARREF_LR(player);

	effect[0]=0x70; // Effect message
	effect[1]=0x00; // effect from source to dest
	effect[2]=pc_player->ser1;
	effect[3]=pc_player->ser2;
	effect[4]=pc_player->ser3;
	effect[5]=pc_player->ser4;

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
		if ((inrange1p(currchar[j],player))&&(perm[j]))
		{
			Xsend(j, effect, 28);
		}
	}
}

//	- Movingeffect3 is used to send an object from a char
//    to another object (like purple potions)
void movingeffect3(CHARACTER source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
	char effect[29];
	int j;

	P_CHAR pc_source = MAKE_CHARREF_LR(source);

	memset (&effect, 0, 29);
	effect[0]=0x70; // Effect message
	effect[1]=0x00; // Moving effect
	effect[2]=pc_source->ser1;
	effect[3]=pc_source->ser2;
	effect[4]=pc_source->ser3;
	effect[5]=pc_source->ser4;
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
	}
}

// staticeffect3 is for effects on items
void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode)
{
	char effect[29];
	int j;
	memset (&effect, 0, 29);

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
		if (inVisRange(x, y, chars[currchar[j]].pos.x, chars[currchar[j]].pos.y))
		{
			Xsend(j, effect, 28);
		}
	}
}

void movingeffect3(int source, int dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode,unsigned char unk1,unsigned char unk2,unsigned char ajust,unsigned char type)
{
	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
	char effect[29];
	int j;

	P_CHAR pc_source = MAKE_CHARREF_LR(source);
	P_CHAR pc_dest   = MAKE_CHARREF_LR(dest);

	effect[0]=0x70; // Effect message
	effect[1]=type; // Moving effect
	effect[2]=pc_source->ser1;
	effect[3]=pc_source->ser2;
	effect[4]=pc_source->ser3;
	effect[5]=pc_source->ser4;
	effect[6]=pc_dest->ser1;
	effect[7]=pc_dest->ser2;
	effect[8]=pc_dest->ser3;
	effect[9]=pc_dest->ser4;
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
		if ((inrange1p(currchar[j],source))&&(inrange1p(currchar[j],dest))&&(perm[j]))
		{
			Xsend(j, effect, 28);
		}
	}
}



//	- Movingeffect2 is used to send an object from a char
//	to another object (like purple potions)
void movingeffect2(int source, int dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
	char effect[29];
	int j;
	const P_ITEM pi=MAKE_ITEMREF_LR(dest);	// on error return
	P_CHAR pc_source = MAKE_CHARREF_LR(source);

	effect[0]=0x70; // Effect message
	effect[1]=0x00; // Moving effect
	effect[2]=pc_source->ser1;
	effect[3]=pc_source->ser2;
	effect[4]=pc_source->ser3;
	effect[5]=pc_source->ser4;
	effect[6]=pi->ser1;
	effect[7]=pi->ser2;
	effect[8]=pi->ser3;
	effect[9]=pi->ser4;
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
		if ((inrange1p(currchar[j],source) || inrange2(j,pi)) && (perm[j]))
		{
			Xsend(j, effect, 28);
		}
	}
}

void dolight(int s, char level)
{
	char light[3]="\x4F\x00";

	if ((s==-1)||(!perm[s])) return;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	light[1]=level;
	if(Races[pc_currchar->race]->NightSight)
	{
		light[1]=pc_currchar->fixedlight;
		Xsend(s, light, 2);
	}
	if (worldfixedlevel!=255)
	{
		light[1]=worldfixedlevel;
	} else {
		if (pc_currchar->fixedlight!=255)
		{
			light[1]=pc_currchar->fixedlight;
		} else {
			if (indungeon(currchar[s]))
			{
				light[1]=dungeonlightlevel;
			}
			else
			{
				light[1]=level;
			}
		}
	}

	Xsend(s, light, 2);
}

void updateskill(int s, int skillnum) // updated for client 1.26.2b by LB
{
	char update[11];
	char x;
	
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	
	update[0] = 0x3A; // Skill Update Message
	update[1] = 0x00; // Length of message
	update[2] = 0x0B; // Length of message
	update[3] = '\xFF'; // single list
	
	update[4] = 0x00;
	update[5] = (char)skillnum;
	update[6] = pc_currchar->skill[skillnum] >> 8;
	update[7] = pc_currchar->skill[skillnum]%256;
	update[8] = pc_currchar->baseskill[skillnum] >> 8;
	update[9] = pc_currchar->baseskill[skillnum]%256;
	x = pc_currchar->lockSkill[skillnum];
	if (x != 0 && x != 1 && x != 2) 
		x = 0;
	update[10] = x;
	
	// CRASH_IF_INVALID_SOCK(s);
	
	Xsend(s, update, 11);
}

void deathaction(int s, int x) // Character does a certain action
{
	int i;
	unsigned char deathact[14]="\xAF\x01\x02\x03\x04\x01\x02\x00\x05\x00\x00\x00\x00";
	P_CHAR pc = MAKE_CHARREF_LR(s);

	P_ITEM pi_x = MAKE_ITEM_REF(x);

	deathact[1]=pc->ser1;
	deathact[2]=pc->ser2;
	deathact[3]=pc->ser3;
	deathact[4]=pc->ser4;
	LongToCharPtr(pi_x->serial,deathact+5);
	
	for (i=0;i<now;i++) 
		if ((inrange1p(s, currchar[i]))&&(perm[i]) && (currchar[i]!=s)) 
			Xsend(i, deathact, 13);
	
}

void deathmenu(int s) // Character sees death menu
{
	char testact[3]="\x2C\x00";
	Xsend(s, testact, 2);
}

void impowncreate(int s, int i, int z) //socket, player to send
{
	int j, k,ci;
	unsigned char oc[1024];

	if ( (i < 0) || (i > cmem))
	{
#ifdef DEBUG
		ConOut("impowncreate -> i overflow. (%i)", i);
#endif
		i = 0;
	}
    P_CHAR pc = MAKE_CHARREF_LR(i);

	if (s==-1) return; //lb
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	if (pc->stablemaster_serial>0) return; // dont **show** stabled pets

	int sendit;
	if (pc->isHidden() && i!=currchar[s] && (pc_currchar->isGM())==0) sendit=0; else sendit=1;

	if (!online(i) && (pc->isPlayer()) && (pc_currchar->isGM())==0 ) 
	{
		sendit=0;
		removeitem[1]=pc->ser1;
		removeitem[2]=pc->ser2;
		removeitem[3]=pc->ser3;
		removeitem[4]=pc->ser4;
		Xsend(s, removeitem, 5);
	}
	// hidden chars can only be seen "grey" by themselves or by gm's
	// other wise they are invisible=dont send the packet
	if (!sendit) return;

	oc[0]=0x78; // Message type 78

	oc[3]=pc->ser1; // Character serial number
	oc[4]=pc->ser2; // Character serial number
	oc[5]=pc->ser3; // Character serial number
	oc[6]=pc->ser4; // Character serial number
	oc[7]=pc->id1; // Character art id
	oc[8]=pc->id2; // Character art id
	oc[9]=pc->pos.x>>8;	// Character x position
	oc[10]=pc->pos.x%256; // Character x position
	oc[11]=pc->pos.y>>8; // Character y position
	oc[12]=pc->pos.y%256; // Character y position
	if (z) oc[13]=pc->dispz; // Character z position
	else oc[13]=pc->pos.z;
	oc[14]=pc->dir; // Character direction
	ShortToCharPtr(pc->skin, &oc[15]); // Character skin color
	oc[17]=0; // Character flags
	if (pc->isHidden() || !(online(i)||pc->isNpc())) oc[17]=oc[17]|0x80; // Show hidden state correctly
	if (pc->poisoned) oc[17]=oc[17]|0x04; //AntiChrist -- thnx to SpaceDog

	k=19;
	int guild,race;
	guild=Guilds->Compare(currchar[s],i);
	race = Races.CheckRelation(pc_currchar, pc);
	if (guild == 1 || race == 1)//Same guild (Green)
		oc[18]=2;
	else if (guild==2 || race == 2) // Enemy guild.. set to orange
		oc[18]=5;
	else
		switch(pc->flag)
	{//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
		case 0x01: oc[18]=6; break;// If a bad, show as red.
		case 0x04: oc[18]=1; break;// If a good, show as blue.
		case 0x08: oc[18]=2; break; //green (guilds)
		case 0x10: oc[18]=5; break;//orange (guilds)
		default:   oc[18]=3; break;//grey (Can be pretty much any number.. I like 3 :-)
	}

	for (j=0;j<MAXLAYERS;j++) layers[j] = 0;

	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if (pc->Wears(pi) && !pi->free)
			{
				if ( layers[pi->layer] == 0 )
				{
					LongToCharPtr(pi->serial,oc+k+0);
					ShortToCharPtr(pi->id(),oc+k+4);
					oc[k+6]=pi->layer;
					k=k+7;
					if (pi->color1!=0 || pi->color2!=0)
					{
						oc[k-3] |= 0x80;
						oc[k+0]=pi->color1;
						oc[k+1]=pi->color2;
						k=k+2;
					}
					layers[pi->layer] = 1;
				}
				else
				{
#ifdef DEBUG
					ConOut("Double layer (%i) on Item (%i) on Char (%i)\n", pi->layer , j , i);
					sprintf(temp, "Double layer (%i) on Item (%2x %2x %2x %2x) on Char (%2x %2x %2x %2x)\n",
						pi->layer, pi->ser1, pi->ser2, pi->ser3, pi->ser4,
						pc->ser1, pc->ser2, pc->ser3, pc->ser4);
					sysbroadcast(temp);
#endif
				}
			}
	}

	oc[k+0]=0;// Not well understood. It's a serial number. I set this to my serial number,
	oc[k+1]=0;// and all of my messages went to my paperdoll gump instead of my character's
	oc[k+2]=0;// head, when I was a character with serial number 0 0 0 1.
	oc[k+3]=0;
	k=k+4;

	// unimportant remark: its a packet "terminator" !!! LB

	oc[1]=k>>8;
	oc[2]=k%256;
	Xsend(s, oc, k);
}

void sendshopinfo(int s, int c, P_ITEM pi)
{
	unsigned char m1[6096];
	unsigned char m2[6096];
	char itemname[256];
	char cFoundItems=0;
	memset(m1,0,6096);
	memset(m2,0,6096);
	memset(itemname,0,256);
	int j, k, m1t, m2t, value,serial,serhash,ci;
               
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
	serhash=serial%HASHMAX;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
		j=DEREF_P_ITEM(pi_j);
		if (j!=-1)
			if ((pi_j->contserial==serial) &&
				(m2[7]!=255) && (pi_j->amount!=0) ) // 255 items max per shop container
			{
				if (m2t>6000 || m1t>6000) break;

				LongToCharPtr(pi_j->serial,m1+m1t+0);//Item serial number
				ShortToCharPtr(pi_j->id(),m1+m1t+4);
				m1[m1t+6]=0;			//Always zero
				m1[m1t+7]=pi_j->amount>>8;//Amount for sale
				m1[m1t+8]=pi_j->amount%256;//Amount for sale
				m1[m1t+9]=j>>8;//pi_j->x/256; //Item x position
				m1[m1t+10]=j%256;//pi_j->x%256;//Item x position
				m1[m1t+11]=j>>8;//pi_j->y/256;//Item y position
				m1[m1t+12]=j%256;//pi_j->y%256;//Item y position
				LongToCharPtr(pi->serial,m1+m1t+13); //Container serial number
				m1[m1t+17]=pi_j->color1;//Item color
				m1[m1t+18]=pi_j->color2;//Item color
				m1[4]++; // Increase item count.
				m1t=m1t+19;
				value=pi_j->value;
				value=calcValue(j, value);
				if (SrvParms->trade_system==1) value=calcGoodValue(c,j,value,0); // by Magius(CHE)
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
}

int sellstuff(int s, int i)
{
	char itemname[256];
	int m1t, pack, z, value;
	int serial,serhash,ci,serial1,serhash1,ci1;
	unsigned char m1[2048];
	unsigned char m2[2];
	char ciname[256]; // By Magius(CHE)
	char cinam2[256]; // By Magius(CHE)

    P_CHAR pc = MAKE_CHARREF_LRV(i, 0);

	P_ITEM sellcont = NULL;

	serial=pc->serial;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if ((pi->contserial==serial) &&
				(pi->layer==0x1C))
			{
				sellcont = pi;
				break;
			}
	}
	if (sellcont == NULL) return 0;

	m2[0]=0x33;
	m2[1]=0x01;
	Xsend(s, m2, 2);

	pack=packitem(currchar[s]);
	if (pack==-1) return 0; //LB

	m1[0]=0x9E; // Header
	m1[1]=0; // Size
	m1[2]=0; // Size
	m1[3]=pc->ser1;
	m1[4]=pc->ser2;
	m1[5]=pc->ser3;
	m1[6]=pc->ser4;
	m1[7]=0; // Num items
	m1[8]=0; // Num items
	m1t=9;

	serial = sellcont->serial;
	vecContainer.clear();
	vecContainer = contsp.getData(serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi_q = FindItemBySerial(vecContainer[ci]);
		if (pi_q != NULL)
		{
			if ((pi_q->contserial==serial))
			{
				serial1=items[pack].serial;
				vector<SERIAL> vecContainer2 = contsp.getData(serial1);
				for ( ci1 = 0; ci1 < vecContainer2.size(); ci1++)
				{
					P_ITEM pi_j = FindItemBySerial(vecContainer2[ci1]);
					if (pi_j != NULL) // LB crashfix
					{
						sprintf(ciname,"'%s'",pi_j->name); // Added by Magius(CHE)
						sprintf(cinam2,"'%s'",pi_q->name); // Added by Magius(CHE)
						strupr(ciname); // Added by Magius(CHE)
						strupr(cinam2); // Added by Magius(CHE)

						if (pi_j->contserial==serial1 &&
							pi_j->id()==pi_q->id()  &&
							pi_j->type==pi_q->type && (m1[8]<60) &&
							((SrvParms->sellbyname==0)||(SrvParms->sellbyname==1 && (!strcmp(ciname,cinam2))))) // If the names are the same! --- Magius(CHE)
						{
							LongToCharPtr(pi_j->serial,m1+m1t+0);
							ShortToCharPtr(pi_j->id(),m1+m1t+4);
							ShortToCharPtr(pi_j->color(),m1+m1t+6);
							ShortToCharPtr(pi_j->amount,m1+m1t+8);
							value=pi_q->value;
							value=calcValue(DEREF_P_ITEM(pi_j), value);
							if (SrvParms->trade_system==1) value=calcGoodValue(i,DEREF_P_ITEM(pi_j),value,1); // by Magius(CHE)
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
		npctalkall(i, "Thou doth posses nothing of interest to me.",0);
	}
	m2[0]=0x33;
	m2[1]=0x00;
	Xsend(s, m2, 2);
	return 1;
}

void playmidi(int s, char num1, char num2)
{
	char msg[3];
	msg[0]=0x6D;
	msg[1]=num1;
	msg[2]=num2;
	Xsend(s, msg, 3);
}

void sendtradestatus(int cont1, int cont2)
{
	unsigned char msg[30];
	int p1, p2, s1, s2;

	p1=calcCharFromSer(items[cont1].contserial);
	p2=calcCharFromSer(items[cont2].contserial);
	s1=calcSocketFromChar(p1);
	s2=calcSocketFromChar(p2);

	if ( ( s1 == -1 ) || ( s2 == -1 ) ) return; 

	msg[0]=0x6F;//Header
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x02;//State
	LongToCharPtr(items[cont1].serial,msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=items[cont1].morez%256;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=items[cont2].morez%256;
	msg[16]=0; // No name in this message
	Xsend(s1, msg, 17);

	LongToCharPtr(items[cont2].serial,msg+4);
	msg[11]=items[cont2].morez%256;
	msg[15]=items[cont1].morez%256;
	Xsend(s2, msg, 17);
}

void endtrade(int b1, int b2, int b3, int b4)
{
	unsigned char msg[30];
	P_ITEM pi_cont1 = NULL, pi_cont2 = NULL;

	pi_cont1 = FindItemBySerial(calcserial(b1, b2, b3, b4));
	if (pi_cont1 == NULL) 
		return; // LB, crashfix
	pi_cont2 = FindItemBySerial(calcserial(pi_cont1->moreb1, pi_cont1->moreb2, pi_cont1->moreb3, pi_cont1->moreb4));
	if (pi_cont2 == NULL) return; // LB, crashfix
	P_CHAR pc1 = FindCharBySerial(pi_cont1->contserial);
	P_CHAR pc2 = FindCharBySerial(pi_cont2->contserial);
	P_ITEM pi_bp1 = Packitem(pc1);
	if (pi_bp1 == NULL) return;
	P_ITEM pi_bp2 = Packitem(pc2);
	if (pi_bp2 == NULL) return;
	UOXSOCKET s1 = calcSocketFromChar(DEREF_P_CHAR(pc1));
	UOXSOCKET s2 = calcSocketFromChar(DEREF_P_CHAR(pc2));

	msg[0]=0x6F;//Header Byte
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x01;//State byte
	LongToCharPtr(pi_cont1->serial,msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=0;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=0;
	msg[16]=0;
	if (s1 > -1)	// player may have been disconnected (Duke)
		Xsend(s1, msg, 17);

	msg[0]=0x6F;//Header Byte
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x01;//State byte
	LongToCharPtr(pi_cont2->serial,msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=0;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=0;
	msg[16]=0;
	if (s2 > -1)	// player may have been disconnected (Duke)
		Xsend(s2, msg, 17);

	vector<SERIAL> vecContainer = contsp.getData(pi_cont1->serial);
	unsigned int ci;
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if ((pi->contserial==pi_cont1->serial))
			{
				if (pi->glow != INVALID_SERIAL) 
					glowsp.remove(pc2->serial, pi->serial); // lb, glowing stuff
				pi->SetContSerial(pi_bp1->serial);
				if (pi->glow != INVALID_SERIAL) 
					glowsp.insert(pc1->serial, pi->serial);
				pi->pos.x = RandomNum(50, 130);
				pi->pos.y = RandomNum(50, 130);
				pi->pos.z=9;
				if (s1 != -1)
					RefreshItem(DEREF_P_ITEM(pi));//AntiChrist
			}
	}
	vecContainer.clear();
	vecContainer = contsp.getData(pi_cont2->serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if ((pi->contserial==pi_cont2->serial))
			{
				if (pi->glow != INVALID_SERIAL) 
					glowsp.remove(pc2->serial, pi->serial); // lb, glowing stuff
				pi->SetContSerial(pi_bp2->serial);
				if (pi->glow != INVALID_SERIAL) 
					glowsp.insert(pc1->serial, pi->serial);
				pi->pos.x=50+(rand()%80);
				pi->pos.y=50+(rand()%80);
				pi->pos.z=9;
				if (s2 != -1)
					RefreshItem(DEREF_P_ITEM(pi));//AntiChrist
			}
	}
	Items->DeleItem(pi_cont1);
	Items->DeleItem(pi_cont2);
}

void tellmessage(int i, int s, char *txt)
{
	int tl;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

	sprintf((char*)temp, "GM tells %s: %s", pc_currchar->name, txt);

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
	Xsend(i, temp, strlen((char*)temp)+1);
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


void staticeffectUO3D(int player, stat_st *sta)
{  
   
   PC_CHAR pc_cs=MAKE_CHARREF_LOGGED(player,err);
   if (err) return;

   // please no optimization of p[...]=0's yet :)

   particleSystem[0]=0xc7;
   particleSystem[1]=0x3;

   particleSystem[2]=pc_cs->ser1;
   particleSystem[3]=pc_cs->ser2;
   particleSystem[4]=pc_cs->ser3;
   particleSystem[5]=pc_cs->ser4;

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

   particleSystem[42]=pc_cs->ser1;
   particleSystem[43]=pc_cs->ser2;
   particleSystem[44]=pc_cs->ser3;
   particleSystem[45]=pc_cs->ser4;
   
   particleSystem[46]=0; // layer, gets set afterwards for multi layering

   particleSystem[47]=0x0; // has to be always 0 for all types
   particleSystem[48]=0x0;
  	   	   
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

void movingeffectUO3D(CHARACTER source, CHARACTER dest, move_st *sta)
{
   PC_CHAR pc_cs=MAKE_CHARREF_LOGGED(source,err);
   if (err) return;
   PC_CHAR pc_cd=MAKE_CHARREF_LOGGED(dest, err);
   if (err) return;

   particleSystem[0]=0xc7;
   particleSystem[1]=0x0;

   particleSystem[2]=pc_cs->ser1;
   particleSystem[3]=pc_cs->ser2;
   particleSystem[4]=pc_cs->ser3;
   particleSystem[5]=pc_cs->ser4;

   particleSystem[6]=pc_cd->ser1;
   particleSystem[7]=pc_cd->ser2;
   particleSystem[8]=pc_cd->ser3;
   particleSystem[9]=pc_cd->ser4;

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
   particleSystem[48]=0x0;

}

// same sta-layout as staticeffectuo3d
void itemeffectUO3D(P_ITEM pi, stat_st *sta)
{
	// please no optimization of p[...]=0's yet :)
	
	particleSystem[0]=0xc7;
	particleSystem[1]=0x2;
	
	if ( !sta->effect[11] ) 
	{
		particleSystem[2]=pi->ser1;
		particleSystem[3]=pi->ser2;
		particleSystem[4]=pi->ser3;
		particleSystem[5]=pi->ser4;
	}
	else
	{
		particleSystem[2]=0x00;
		particleSystem[3]=0x00;
		particleSystem[4]=0x00;
		particleSystem[5]=0x00;
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
	
	particleSystem[42]=pi->ser1;
	particleSystem[43]=pi->ser2;
	particleSystem[44]=pi->ser3;
	particleSystem[45]=pi->ser4;
	
	particleSystem[46]=0xff; 
	
	particleSystem[47]=0x0; 
	particleSystem[48]=0x0;
}

void bolteffectUO3D(CHARACTER player)
{
	Magic->doStaticEffect(player, 30);
}

void PlayDeathSound( CHARACTER i )
{
    P_CHAR pc = MAKE_CHARREF_LR(i);

	if (pc->xid1==0x01 && pc->xid2==0x91)
	{
		switch(RandomNum(0, 3)) // AntiChrist - uses all the sound effects
		{
		case 0:		soundeffect2( i, 0x01, 0x50 );	break;// Female Death
		case 1:		soundeffect2( i, 0x01, 0x51 );	break;// Female Death
		case 2:		soundeffect2( i, 0x01, 0x52 );	break;// Female Death
		case 3:		soundeffect2( i, 0x01, 0x53 );	break;// Female Death
		}
	}
	else if (pc->xid1==0x01 && pc->xid2==0x90)
	{
		switch( RandomNum(0, 3) ) // AntiChrist - uses all the sound effects
		{
		case 0:		soundeffect2( i, 0x01, 0x5A );	break;// Male Death
		case 1:		soundeffect2( i, 0x01, 0x5B );	break;// Male Death
		case 2:		soundeffect2( i, 0x01, 0x5C );	break;// Male Death
		case 3:		soundeffect2( i, 0x01, 0x5D );	break;// Male Death
		}
	}
	else
	{
		playmonstersound(i, pc->xid1, pc->xid2, SND_DIE);
	}
}

