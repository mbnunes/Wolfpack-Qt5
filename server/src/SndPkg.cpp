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
#include "regions.h"
#include "srvparams.h"
#include "network.h"
#include "classes.h"
#include "territories.h"
#include "wpdefmanager.h"

#undef  DBGFILE
#define DBGFILE "SndPkg.cpp"

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
	
	P_CHAR pc_currchar = currchar[s];

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

void soundeffect2(P_CHAR pc, short sound)
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

void soundeffect2(P_CHAR pc, unsigned char a, unsigned char b)
{
	int i;

	if (pc == NULL)
		return;
	sfx[2]=a;
	sfx[3]=b;
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

void soundeffect4(P_ITEM pi, UOXSOCKET s, unsigned char a, unsigned char b)
{
	if (pi == NULL)
		return;
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
	P_CHAR pc_currchar = currchar[s];

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
	P_CHAR pc_currchar = currchar[s];

	LongToCharPtr(pc_currchar->serial, &doact[1]);
	doact[5]=x>>8;
	doact[6]=x%256;
	Xsend(s, doact, 14);
	for (i=0;i<now;i++) if ((inrange1(s, i))&&(perm[i])) { Xsend(i, doact, 14); }
}

void npcaction(P_CHAR pc_npc, int x) // NPC character does a certain action
{
	int i;
	if ( pc_npc == NULL )
		return;

	LongToCharPtr(pc_npc->serial, &doact[1]);
	doact[5]=x>>8;
	doact[6]=x%256;
	for (i=0;i<now;i++) 
		if ((inrange1p(currchar[i], pc_npc))&&(perm[i])) 
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
	talk[9]=0;
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
//	cNetwork::instance()->ClearBuffers();
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
	if(s==-1) return;
	if ( s>=MAXCLIENT)
		LogCriticalVar("Socket/Index confusion @ Xsend <%s>\n",txt);
	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsprintf( msg, txt, argptr );
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
	talk[9]=0x0;
	talk[10]=0x00;	//Color1  - Previous default was 0x0040
	talk[11]=0x00;  //Color2
	talk[12]=0;     
	talk[13]=3;

	// problem1: 3d clietns crash if color set to 0 since 3.0.6g 
	// workaround: set it to non 0 if 3d client
	// but a mean pitfall remaining: there are a couple sysmessages BEFORE client dimension is known
	// hence 3d clients are recognized as 2d clients for those sysmessages
	// it's important that those sysmessages are called with color != 0 ...
	// it've tagged them with "UNKOWNDIMENSION sysmessages" in comment
	if ( clientDimension[s]==3 && talk[10]==0 && talk[11]==0) { talk[10]=0; talk[11]=0x37; }

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

	if ( clientDimension[s]==3 && color==0) { talk[10]=0; talk[11]=0x37; }

	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, msg, strlen((char*)msg)+1);
}

void itemmessage(UOXSOCKET s, char *txt, int serial, short color) 
{// The message when an item is clicked (new interface, Duke)

	P_ITEM pi = FindItemBySerial(serial);

	if ( pi == NULL )
		return;

	if ((pi->type() == 1 && color == 0x0000)||
		(pi->type() == 9 && color == 0x0000)||
		(pi->type() == 1000 && color == 0x0000)||
		(pi->id()==0x1BF2 && color == 0x0000))
		color = 0x03B2;
	    else
			if( !pi->corpse() )
			color = 0x0481;

		if ( clientDimension[s]==3 && color==0) { talk[10]=0; talk[11]=0x37; }

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
	wearitem[8]=pi->layer();
	LongToCharPtr(pi->contserial,wearitem+9);
	ShortToCharPtr(pi->color(), &wearitem[13]);
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

	int loopexit=0;
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

	switch( pCont->id() )
	{
		case 0x0E75:					// Backpack
		case 0x0E79: bpopen[6] = 0x3C;	// Box/Pouch
					break;

		case 0x0E76: bpopen[6] = 0x3D;	// Leather Bag
					break;

		case 0x0E77:					// Barrel
		case 0x0E7A:					// Square Basket
		case 0x0E7F: bpopen[6] = 0x3E;	// Keg
					break;

		case 0x0E7C: bpopen[6] = 0x4A;	// Silver Chest
					break;

		case 0x0E7D: bpopen[6] = 0x43;	// Wooden Box
					break;

		case 0x0E3D:					// Large Wooden Crate
		case 0x0E3C:					// Large Wooden Crate
		case 0x0E3F:					// Small Wooden Crate
		case 0x0E3E:					// Small Wooden Crate
		case 0x0E7E: bpopen[6] = 0x44;	// Wooden Crate
					break;

		case 0x0E80: bpopen[6] = 0x4B;	// Brass Box
					break;

		case 0x0E40:					// Metal & Gold Chest
		case 0x0E41: bpopen[6] = 0x42;	// Metal & Gold Chest
					break;

		case 0x0E43:					// Wooden & Gold chest
		case 0x0E42: bpopen[6] = 0x49; // Wooden & Gold Chest
					break;

		case 0x0990: bpopen[6]=0x41; // Round Basket
					break;

		case 0x09B2: bpopen[6]=0x3C; // Backpack 2
					break;

		case 0x09AA: bpopen[6]=0x43; // Wooden Box
					break;

		case 0x09A8: bpopen[6]=0x40; // Metal Box
					break;

		case 0x09AB: bpopen[6]=0x4A; // Metal/Silver Chest
					break;

		case 0x09A9: bpopen[6]=0x44; // Small Wooden Crate
					break;
			
		case 0x2006: bpopen[6]=0x09; // Coffin
					break;

		case 0x0A97:					// Bookcase
		case 0x0A98:					// Bookcase
		case 0x0A99:					// Bookcase
		case 0x0A9a:					// Bookcase
		case 0x0A9b:					// Bookcase
		case 0x0A9c:					// Bookcase
		case 0x0A9d:					// Bookcase
		case 0x0A9e: bpopen[6]=0x4d; // Bookcase
					break;

		case 0x0A4d:					// Fancy Armoire
		case 0x0A51:					// Fancy Armoire
		case 0x0A4c:					// Fancy Armoire
		case 0x0A50: bpopen[6]=0x4e; // Fancy Armoire
					break;

		case 0x0A4f:					// Wooden Armoire
		case 0x0A53:					// Wooden Armoire
		case 0x0A4e:					// Wooden Armoire
		case 0x0A52: bpopen[6]=0x4f; // Wooden Armoire
					break;

		case 0x0A30:					// chest of drawers (fancy)
		case 0x0A38: bpopen[6]=0x48; // chest of drawers (fancy)
					break;

		case 0x0A2c:					// chest of drawers (wood)
		case 0x0A34:					// chest of drawers (wood)
		case 0x0A3c:					// Dresser
		case 0x0A3d:					// Dresser
		case 0x0A44:					// Dresser
		case 0x0A35: bpopen[6]=0x51; // Dresser
					break;
		case 0x0Ab2: if (pCont->morex==1) bpopen[6]=0x4a;
					break;

		default:
			if( ((pCont->id()&0xFF00) >> 8) == 0x3E )
				bpopen[6]=0x4C;

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

	unsigned int ci = 0;
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
		LongToCharPtr(pi->serial, &bpitem[0]);
		ShortToCharPtr(pi->id(),bpitem+4);
		bpitem[7]=pi->amount()>>8;
		bpitem[8]=pi->amount()%256;
		bpitem[9]=pi->pos.x>>8;
		bpitem[10]=pi->pos.x%256;
		bpitem[11]=pi->pos.y>>8;
		bpitem[12]=pi->pos.y%256;
		LongToCharPtr(serial, &bpitem[13]);
		ShortToCharPtr(pi->color(), &bpitem[17]);
		bpitem[19]=pi->decaytime=0;//HoneyJar // reseting the decaytimer in the backpack
		Xsend(s, bpitem, 19);
	}
}

void backpack2(int s, SERIAL serial) // Send corpse stuff
{
	int count=0, count2;
	unsigned char bpopen2[6]="\x3C\x00\x05\x00\x00";
	unsigned char display1[8]="\x89\x00\x0D\x40\x01\x02\x03";
	unsigned char display2[6]="\x01\x40\x01\x02\x03";

	//SERIAL serial = calcserial(a1,a2,a3,a4);
	register unsigned int ci;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ++ci)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer()!=0)
		{
			++count;
		}
	}
	count2=(count*5)+7 + 1 ; // 5 bytes per object, 7 for this header and 1 for terminator
	display1[1]=count2>>8;
	display1[2]=count2%256;
	LongToCharPtr(serial, &display1[3]);
	Xsend(s, display1, 7);

	for ( ci = 0; ci < vecContainer.size(); ++ci)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer()!=0)
		{
			display2[0]=pi->layer();
			LongToCharPtr(pi->serial, &display2[1]);
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

	for ( ci = 0; ci < vecContainer.size(); ++ci)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer()!=0)
		{
			LongToCharPtr(pi->serial, &bpitem[0]);
			ShortToCharPtr(pi->id(),bpitem+4);
			bpitem[7]=pi->amount()>>8;
			bpitem[8]=pi->amount()%256;
			bpitem[9]=pi->pos.x>>8;
			bpitem[10]=pi->pos.x%256;
			bpitem[11]=pi->pos.y>>8;
			bpitem[12]=pi->pos.y%256;
			LongToCharPtr(serial, &bpitem[13]);
			ShortToCharPtr(pi->color(), &bpitem[17]);
			bpitem[19]=pi->decaytime=0;// reseting the decaytimer in the backpack	//moroallan
			Xsend(s, bpitem, 19);
		}
	}
}

void sendbpitem(UOXSOCKET s, P_ITEM pi) // Update single item in backpack
{
	unsigned char display3[2]="\x25";
	if (pi == NULL)
		return;

	P_CHAR pc_currchar = currchar[s];

	LongToCharPtr(pi->serial, &bpitem[0]);
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
	bpitem[7]=pi->amount()>>8;
	bpitem[8]=pi->amount()%256;
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
		ShortToCharPtr(pi->color(), &bpitem[17]);
	}
	bpitem[19]=pi->decaytime=0; // HoneyJar, array range is 0-19 ! //reseting the decaytimer in the backpack

	// we need to find the topmost container that the item is in
	// be it a character or another container.

	P_ITEM pio=GetOutmostCont(pi);
	if (!pio)
	{
		char ttt[222];
		sprintf(ttt,"item <%i> id <%x> has a bad contserial <%i>",pi->serial,pi->id(),pi->contserial);
		LogCritical(ttt);
		return;
	}

	P_CHAR pc = FindCharBySerial(pio->contserial);

	if (((pc != NULL)&&(inrange1p(currchar[s], pc)))|| // if item is in a character's
		//pack (or subcontainer) and player is in range
		((pc == NULL)&&(inrange2(s,pio))))	// or item is in container on ground and
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
			if(abs(currchar[j]->pos.x-x)<=VISRANGE && abs(currchar[j]->pos.y-y)<=VISRANGE) 
				Xsend(j, effect, 28);
		}

	}
}

void senditem(UOXSOCKET s, P_ITEM pi) // Send items (on ground)
{
	if( pi == NULL )
		return;
	int pack,serial;
	unsigned char itmput[21]="\x1A\x00\x13\x40\x01\x02\x03\x20\x42\x00\x32\x06\x06\x06\x4A\x0A\x00\x00\x00";
	P_CHAR pc_currchar = currchar[s];

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
			P_CHAR pc = FindCharBySerial(serial);
			if ( pc == NULL ) return;
			if ( pc->serial == serial ) 
				pack = 0;
		}
		if (pack)
		{
			if( !pi->isMulti() ) // LB client crashfix, dont show multis in BP
								// we should better move it out of pack, but thats
								// only a first bannaid
			{
				sendbpitem(s, pi);
				return;
			}
		}
	}

	if (pi->isInWorld() && inrange2(s,pi) )
	{
		LongToCharPtr(pi->serial | 0x80000000, &itmput[3]); // Enable Piles

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

		itmput[9]=pi->amount()>>8;
		itmput[10]=pi->amount()%256;
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
			ShortToCharPtr(pi->color(), &itmput[16]);
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


		if (pi->isAllMovable()) itmput[18] |= 0x20;
		if (pc_currchar->priv2&1) itmput[18] |= 0x20;
		if ((pi->isOwnerMovable() || pi->isLockedDown()) && pc_currchar->Owns(pi))
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
			backpack2(s, pi->serial);
		}
	}
}

// sends item in differnt color and position than it actually is
// used for LSd potions now, LB 5'th nov 1999
void senditem_lsd(UOXSOCKET s, P_ITEM pi,char color1, char color2, int x, int y, signed char z)
{
	unsigned char itmput[20]="\x1A\x00\x13\x40\x01\x02\x03\x20\x42\x00\x32\x06\x06\x06\x4A\x0A\x00\x00\x00";
	if (pi == NULL)
		return;
	P_CHAR pc_currchar = currchar[s];

	if ( pi->visible>=1 && !(pc_currchar->isGM()) ) return; // workaround for missing gm-check client side for visibity since client 1.26.2
	// for lsd we dont need extra work for type 1 as in send_item

	if (pi->isInWorld())
	{
		LongToCharPtr(pi->serial | 0x80000000, &itmput[3]);
		ShortToCharPtr(pi->id(),itmput+7);
		itmput[9]=pi->amount()>>8;
		itmput[10]=pi->amount()%256;
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

		if (pi->isAllMovable()) itmput[18]+=0x20;

		if (pc_currchar->priv2&1) itmput[18]+=0x20;
		if ((pi->isOwnerMovable() || pi->isLockedDown()) && pc_currchar->Owns(pi))
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
			backpack2(s, pi->serial);
		}
	}

}

// LB 3-JULY 2000
void sendperson_lsd(UOXSOCKET s, P_CHAR pc, char color1, char color2)
{
	int k,color,c1,c2,b,cc1=0,cc2=0;
	unsigned char oc[1024];

	if ( pc == NULL ) return;
	if (s==-1) return;
	int sendit;
	P_CHAR pc_currchar = currchar[s];

	if (pc_currchar->isGM()) sendit=1; else // gm ? -> yes, send everything :)
	{ // no, -> dont show hidden & logged out chars
		if (pc->isHidden() && pc != currchar[s]) sendit=0; else sendit=1; // dont show hidden persons, even lsd'ed
		if ( (pc->isPlayer()) && !online(pc)) sendit=0;
	}

	if (!online(pc) && (pc->isPlayer()) && (pc_currchar->isGM())==0 ) 
	{
		sendit=0;
		LongToCharPtr(pc->serial, &removeitem[1]);
		Xsend(s, removeitem, 5);
	}

	if (!sendit) return;

	oc[0]=0x78; // Message type 78
	LongToCharPtr(pc->serial, &oc[3]);
	ShortToCharPtr(pc->id(),  &oc[7]);

	int x=pc->pos.x; /* | 0x8000;*/ // LB bugfix

	oc[9]= (x>>8);	// Character x position
	oc[10]=x%256; // Character x position
	oc[11]=pc->pos.y>>8; // Character y position
	oc[12]=pc->pos.y%256; // Character y position

	oc[13]=pc->pos.z; // Character z position
	oc[14]=pc->dir; // Character direction
	ShortToCharPtr(pc->skin(), &oc[15]);	// Character skin color
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

	QMap<int, bool> layers;
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
		if (pi_j != NULL)
		{
			if (pc->Wears(pi_j) && (!pi_j->free))
			{
				if ( layers.contains( pi_j->layer() ) )
				{
					LongToCharPtr(pi_j->serial,oc+k+0);
					ShortToCharPtr(pi_j->id(),oc+k+4);
					oc[k+6]=pi_j->layer();
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
					layers[pi_j->layer()] = true;
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
	LongToCharPtr(pc->serial, &removeitem[1]);

	Xsend(s, removeitem, 5);

	if (currchar[s]==pc)
	{
		goxyz[0]=0x20;
		LongToCharPtr(pc->serial, &goxyz[1]);
		ShortToCharPtr(pc->id(),  &goxyz[5]);

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
	LongToCharPtr(pc->serial, &extmove[1]);
	ShortToCharPtr(pc->id(),  &extmove[5]);
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
	int tlen;
	unsigned int j,i;
	if (acctno[s]!=-1)
	{
		j=0;
		P_CHAR toDelete = NULL;
		AllCharsIterator iterChars;
		for (iterChars.Begin(); !iterChars.atEnd(); iterChars++)
		{
			P_CHAR pc = iterChars.GetData();
			if ((pc->account() == acctno[s] && !pc->free))
			{
				if (j == buffer[s][0x22]) 
					toDelete = pc;
				j++;
			}
		}
		if (toDelete != NULL)
		{
			if (!SrvParams->checkCharAge())
				Npcs->DeleteChar(toDelete);
			else
				if ((toDelete->creationday()+7) < getPlatformDay())
					Npcs->DeleteChar(toDelete);
				else
				{
					unsigned char delete_error[2] = {0x85, 0x03};
					Xsend(s, delete_error, 2);
					return;
				}
		}

		{
                        const vector<StartLocation_st>& start = SrvParams->startLocation();
			tlen=4+(5*60)+1+(start.size()*63);

			login04a[1]=tlen>>8;
			login04a[2]=tlen%256;
			if (j>=1) login04a[3]=j--; else login04a[3]=0;

			Xsend(s, login04a, 4);
			j=0;
			AllCharsIterator it;
			for (it.Begin(); !it.atEnd(); ++it)
			{
				P_CHAR pc = it.GetData();
				if ( pc->account() == acctno[s] && !pc->free)
				{
					strcpy((char*)login04b, pc->name.c_str());
					Xsend(s, login04b, 60);
					j++;
				}
			}

			memset(&login04b[0], 0, 60*sizeof(unsigned char));
			for (i=j;i<5;i++)
			{
				Xsend(s, login04b, 60);
			}

			buffer[s][0] = start.size();
			Xsend(s, buffer[s], 1);
			for ( i = 0; i < start.size(); ++i )
			{
				login04d[0] = static_cast<unsigned char>(i);
				strncpy( (char*)&login04d[1], start[i].name.latin1(), 30 );
				strncpy( (char*)&login04d[32], start[i].name.latin1(), 30 );
				login04d[31] = login04d[61] = 0;
				Xsend(s, login04d, 63);
			}
		}
	}
}

void textflags (UOXSOCKET s, P_CHAR pc, char *name)
{
	char name2[150] = {0,};

    if ( pc == NULL )
		return;

	P_CHAR pc_currchar = currchar[s];

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

	if (pc->isInvul() && pc->account()!=0)		{  if (title[11].other[0] != 0) sprintf((char*)temp, " [%s]",title[11].other); else temp[0] = 0; strcat(name2,(char*)temp); } // ripper
	if (pc->account()==0 && pc->isGM())			{  if (title[12].other[0] != 0) sprintf((char*)temp, " [%s]",title[12].other); else temp[0] = 0; strcat(name2,(char*)temp); } // ripper
	if (pc->priv2&2)							{  if (title[13].other[0] != 0) sprintf((char*)temp, " [%s]",title[13].other); else temp[0] = 0; strcat(name2,(char*)temp); }
	if (pc->guarded())							{  if (title[14].other[0] != 0) sprintf((char*)temp, " [%s]",title[14].other); else temp[0] = 0; strcat(name2,(char*)temp); } // Ripper
	if (pc->tamed() && pc->npcaitype()==32 
		&& pc_currchar->Owns(pc) && pc_currchar->guarded()) 
												{ if  (title[15].other[0] != 0) sprintf((char*)temp, " [%s]",title[15].other); else temp[0] = 0; strcat(name2,(char*)temp); } // Ripper
	if (pc->tamed() && pc->npcaitype()!=17 )		{ if  (title[16].other[0] != 0) sprintf((char*)temp, " [%s]",title[16].other); strcat(name2,(char*)temp); }
	if (pc->war)								{ if  (title[17].other[0] != 0) sprintf((char*)temp, " [%s]",title[17].other); strcat(name2,(char*)temp); } // ripper
	if ((pc->crimflag()>0)&&(pc->kills<SrvParams->maxkills())) 
												{ if  (title[18].other[0] != 0) sprintf((char*)temp, " [%s]",title[18].other); else temp[0] = 0; strcat(name2,(char*)temp); }// ripper
	if (pc->kills>=SrvParams->maxkills())				{ if  (title[19].other[0] != 0) sprintf((char*)temp, " [%s]",title[19].other); else temp[0] = 0; strcat(name2,(char*)temp); } // AntiChrist

	GuildTitle(s, pc);

	int tl,guild;
	tl=44+strlen(name2)+1;
	talk[1]=tl>>8;//AntiChrist
	talk[2]=tl%256;
	LongToCharPtr(pc->serial, &talk[3]);
	talk[7]=1;
	talk[8]=1;
	talk[9]=6; // Mode: "You see"
	guild = GuildCompare(currchar[s], pc);
	if (guild == 1) //Same guild (Green)
	{
		talk[10]=0x00;
		talk[11]=0x43;
	}
	else if (guild==2) //enemy (Orange)
	{
		talk[10]=0x00;
		talk[11]=0x30;
	}
	else if( pc->isGM() && pc->account()==0 )
	{
		talk[10]=0x00;
		talk[11]=0x35;//Admin & GM get yellow names ..Ripper
	}
	else
	{
		switch(pc->flag())
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

void teleport(P_CHAR pc) // Teleports character to its current set coordinates
{
	int i;
	if ( pc == NULL )
		return;
	UOXSOCKET k = calcSocketFromChar(pc);
	if (k!=-1)	// If a player, move them to the appropriate XYZ
	{
		LongToCharPtr(pc->serial, &removeitem[1]);

		LongToCharPtr(pc->serial, &goxyz[1]);
		goxyz[5]=pc->id1;
		goxyz[6]=pc->id2;
		ShortToCharPtr(pc->skin(), &goxyz[8]);
		if(pc->poisoned()) 
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
		//goxyz[18]=pc->dispz;
		goxyz[18]=pc->pos.z;
		Xsend(k, goxyz, 19);
		Weight->NewCalc(pc);	// Ison 2-20-99
		statwindow(k, pc);	// Ison 2-20-99
		walksequence[k]=-1;
	}
	for (i=0;i<now;i++) // Send the update to all players.
	{
		// Dupois - had to remove the && (k!=i)), doesn update the client
		// Added Oct 08, 1998
		if (perm[i])
		{
		   Xsend(i, removeitem, 5);
		   if (inrange1p(pc, currchar[i])) 
			   impowncreate(i, pc, 1);
		}
	}
	

	if (k!=-1)
	{
		//Char mapRegions
		int	StartGrid=mapRegions->StartGrid(pc->pos);
		unsigned int increment=0;
		for (unsigned int checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
		{
			for (int a=0;a<3;a++)
			{
				cRegion::raw vecEntries = mapRegions->GetCellEntries(checkgrid+a);
				cRegion::rawIterator it = vecEntries.begin();
				for ( ; it != vecEntries.end(); ++it)
				{
					P_CHAR mapchar = FindCharBySerial(*it);
					P_ITEM mapitem = FindItemBySerial(*it);
					if (mapchar != NULL)
					{
						if ((mapchar->isNpc()||online(mapchar)||pc->isGM())&&(pc != mapchar)&&(inrange1p(pc, mapchar)))
						{
							impowncreate(k, mapchar, 1);
						}
					} else if (mapitem != NULL) {
						if(iteminrange(k, mapitem,VISRANGE))
						{
							senditem(k, mapitem);
						}
					}
				}
			}
		}
		if (perm[k]) dolight(k, SrvParams->worldCurrentLevel());
	}
	cAllTerritories::getInstance()->check(pc);
}

void teleport2(P_CHAR pc) // used for /RESEND only - Morrolan, so people can find their corpses
{
	int i;
	UOXSOCKET k = calcSocketFromChar(pc);
	
	for (i=0;i<now;i++)
	{
		if ((perm[i])&&(i!=k))
		{
			LongToCharPtr(pc->serial, &removeitem[1]);
			Xsend(i, removeitem, 5);
		}
	}
	if (k!=-1)	// If a player, move them to the appropriate XYZ
	{
		LongToCharPtr(pc->serial, &goxyz[1]);
		ShortToCharPtr(pc->id(),  &goxyz[5]);
		ShortToCharPtr(pc->skin(),  &goxyz[8]);
		goxyz[10]=0;
		if (pc->isHidden()) 
			goxyz[10]=0x80;
		goxyz[11]=pc->pos.x>>8;
		goxyz[12]=pc->pos.x%256;
		goxyz[13]=pc->pos.y>>8;
		goxyz[14]=pc->pos.y%256;
		goxyz[17]=pc->dir|0x80;
		goxyz[18]=pc->dispz;
		Xsend(k, goxyz, 19);
		all_items(k);
		Weight->NewCalc(pc);	// Ison 2-20-99
		statwindow(k, pc);	// Ison 2-20-99
		walksequence[k]=-1;
	}
	for (i=0;i<now;i++) // Send the update to all players.
	{
		// Dupois - had to remove the && (k!=i)), doesn update the client
		// Added Oct 08, 1998
		if (perm[i])
		{			 
			if (inrange1p(pc, currchar[i]))
			{
				impowncreate(i, pc, 1);
			}
		}
	}
	
	if (k!=-1)
	{
		AllCharsIterator iter_char;
		for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
		{ //Tauriel only send inrange people (walking takes care of out of view)
			P_CHAR pc_i = iter_char.GetData();
			if ( ( online(pc_i) || pc_i->isNpc() || pc->isGM()) && (pc->serial!= pc_i->serial) && (inrange1p(pc, pc_i)))
			{
				impowncreate(k, pc_i, 1);
			}
		}
		if (perm[k]) 
			dolight(k, SrvParams->worldCurrentLevel());
	}
	cAllTerritories::getInstance()->check(pc);
}


void updatechar(P_CHAR pc) // If character status has been changed (Polymorph), resend him
{
	int i;
	if (pc == NULL)
		return;
	setcharflag(pc);//AntiChrist - bugfix for highlight color not being updated

	for (i=0;i<now;i++)
	{
		if (perm[i] && inrange1p(currchar[i], pc))
		{
			LongToCharPtr(pc->serial, &removeitem[1]);
			Xsend(i, removeitem, 5);
			if (currchar[i]==pc)
			{
				LongToCharPtr(pc->serial, &goxyz[1]);
				ShortToCharPtr(pc->id(),  &goxyz[5]);
				ShortToCharPtr(pc->skin(),  &goxyz[8]);
				if(pc->poisoned()) 
					goxyz[10]=0x04; 
				else 
					goxyz[10]=0x00;	//AntiChrist -- thnx to SpaceDog
				if (pc->isHidden()) 
					goxyz[10] |= 0x80;
				goxyz[11]=pc->pos.x>>8;
				goxyz[12]=pc->pos.x%256;
				goxyz[13]=pc->pos.y>>8;
				goxyz[14]=pc->pos.y%256;
				goxyz[17]=pc->dir|0x80;
				goxyz[18]=pc->pos.z;
				Xsend(i, goxyz, 19);
				walksequence[i]=-1;
			}
			if (inrange1p(pc, currchar[i]))
			{
				impowncreate(i, pc, 0);			
			}
		}
	}
}


void target(UOXSOCKET s, int a1, int a2, int a3, int a4, const QString& txt) // Send targetting cursor to client
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

		x=pc_currchar->lockSkill[i];
		if (x!=0 && x!=1 && x!=2) x=0;
		skillmid[6]=x; // leave it unlocked, regardless
		Xsend(s, skillmid, 7);
	}
	Xsend(s, skillend, 2);
}


void updatestats( P_CHAR pc, char x )
{
	int i, a = 0, b = 0;
	unsigned char updater[10]="\xA1\x01\x02\x03\x04\x01\x03\x01\x02";
	
	if ( pc == NULL )
		return;


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
	LongToCharPtr(pc->serial, &updater[1]);
	updater[5]=a>>8;
	updater[6]=a%256;
	updater[7]=b>>8;
	updater[8]=b%256;
	if (x == 0)  //Send to all, only if it's Health change
	{
		for (i=0;i<now;i++) 
			if (perm[i] && inrange1p(currchar[i], pc) ) 
				Xsend(i, updater, 9);
	} else {
		UOXSOCKET s = calcSocketFromChar(pc);
		if (s != -1)
			Xsend(s, updater, 9);
	}
}

void statwindow(int s, P_CHAR pc) // Opens the status window
{
	int x;
	unsigned char statstring[67]="\x11\x00\x42\x00\x05\xA8\x90XYZ\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x12\x00\x34\xFF\x01\x00\x00\x5F\x00\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x00\x75\x30\x01\x2C\x00\x00";
	bool ghost;
	
	if (s<0 || s>=MAXCLIENT || pc == NULL) return; // lb, fixes a few (too few) -1 crashes ...

	P_CHAR pc_currchar = currchar[s];

	if ((pc->id1==0x01 && pc->id2==0x92) || (pc->id1==0x01 && pc->id2==0x93)) ghost = true; else ghost = false;

	LongToCharPtr(pc->serial, &statstring[3]);
	strncpy((char*)&statstring[7],pc->name.c_str(), 30); // can not be more than 30 at least no without changing packet lenght

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

	if (((pc_currchar->isGM())|| pc_currchar->Owns(pc))&&(currchar[s]!=pc))
	{
		statstring[41]=0xFF;
	} 
	else if (pc_currchar->Owns(pc) && currchar[s]!=pc ) //Morrolan - from Banter
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
	UI32 y;

#pragma note("new xml format: convert section MOTD to <text> with id MOTD")
	QString motdText = DefManager->getText( "MOTD" );
	y = motdText.length() + 10;
	
	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=2;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;
	Xsend(s, updscroll, 10);
	
	Xsend(s, (char*)motdText.latin1(), motdText.length() );
}

void tips(int s, int tip) // Tip of the day window
{
	UI32 y = 10;

	if( tip == 0 ) 
		tip = 1; 

#pragma note("new xml format: convert section TIPS to <list> with id TIPS")
#pragma note("new xml format: convert section TIP to <text> linked in TIPS-list")
	QStringList tipList = DefManager->getList( "TIPS" );
	if( tipList.size() == 0 )
		return;
	else if( tip > tipList.size() )
		tip = tipList.size();

	QString tipText = DefManager->getText( tipList[ tip-1 ] );
	y = tipText.length()+10;

	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=0;
	updscroll[7]=tip;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;

	Xsend(s, updscroll, 10);

	Xsend( s, (char*)tipText.latin1(), tipText.length() );
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
	int i,tl;
	unsigned char nonuni[512];
	P_CHAR pc_currchar = currchar[s];

	if(pc_currchar->unicode())
		for (i=13;i<(buffer[s][1]<<8)+buffer[s][2];i=i+2)
		{
			nonuni[(i-13)/2]=buffer[s][i];
		}
		if(!pc_currchar->unicode())
		{
			tl=44+strlen((char*)&buffer[s][8])+1;
			talk[1]=tl>>8;
			talk[2]=tl%256;
			LongToCharPtr(pc_currchar->serial, &talk[3]);
			ShortToCharPtr(pc_currchar->id(),  &talk[7]);
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
					Xsend(i, (void*)pc_currchar->name.c_str(), 30);
					Xsend(i, &buffer[s][8], strlen((char*)&buffer[s][8])+1);
				}
			}
		} // end unicode IF
		else
		{
			tl=44+strlen((char*)&nonuni[0])+1;

			talk[1]=tl>>8;
			talk[2]=tl%256;
			LongToCharPtr(pc_currchar->serial, &talk[3]);
			ShortToCharPtr(pc_currchar->id(),  &talk[7]);
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
					Xsend(i, (void*)pc_currchar->name.c_str(), 30);
					Xsend(i, &nonuni[0], strlen((char*)&nonuni[0])+1);
				}
			}
		}
}

void itemtalk(int s, P_ITEM pi, char *txt) // Item "speech"
{
	int tl;

	if (s<=-1 || pi == NULL) return;

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
	Xsend(s, (void*)pi->name().ascii(), 30);
	Xsend(s, txt, strlen(txt)+1);
}

void npctalk(int s, P_CHAR pc_npc, const char *txt,char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (pc_npc == NULL || s==-1) return; //lb

	P_CHAR pc_currchar = currchar[s];

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
		tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		LongToCharPtr(pc_npc->serial, &talk[3]);
		ShortToCharPtr(pc_npc->serial, &talk[7]);
		talk[9]=0; // Type
		pc_npc->saycolor = 0x0481;

		talk[12]=0;
		talk[13]=pc_currchar->fonttype;

		if (pc_npc->npcaitype()==2 && SrvParams->badNpcsRed() == 0) //bad npcs speech (red)..Ripper
		{
			pc_npc->saycolor = 0x03B2;
		}
		else if (pc_npc->npcaitype()==2 && SrvParams->badNpcsRed() == 1)
		{
			pc_npc->saycolor = 0x0026;
		}
		else if(pc_npc->isNpc() && !pc_npc->tamed() && !pc_npc->guarded() && !pc_npc->war)
		{
			pc_npc->saycolor = 0x005b;
		}

		ShortToCharPtr(pc_npc->saycolor, &talk[10]);
		Xsend(s, talk, 14);
		Xsend(s, (void*)pc_npc->name.c_str(), 30);
		Xsend(s, txt, strlen(txt)+1);
	}
}

void npctalkall(P_CHAR npc, const char *txt,char antispam) // NPC speech to all in range.
{
	if (npc==NULL) return;

	int i;

	for (i=0;i<now;i++)
		if (inrange1p(npc, currchar[i])&&perm[i])
			npctalk(i, npc, txt,antispam);
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
		tl=44+strlen(txt)+1;
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
		Xsend(s, (void*)pc_npc->name.c_str(), 30);
		Xsend(s, txt, strlen(txt)+1);
	}
}

void npcemote(int s, P_CHAR pc_npc, char *txt, char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (s==-1 || pc_npc == NULL) return;

	P_CHAR pc_currchar = currchar[s];

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
		tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		LongToCharPtr(pc_npc->serial, &talk[3]);
		ShortToCharPtr(pc_npc->id(),  &talk[7]);
		talk[9]=2; // Type
		ShortToCharPtr(pc_npc->emotecolor, &talk[10]);
		talk[12]=0;
		talk[13]=pc_currchar->fonttype;
		Xsend(s, talk, 14);
		Xsend(s, (void*)pc_npc->name.c_str(), 30);
		Xsend(s, txt, strlen(txt)+1);
	}
}

// Last touch: LB 8'th April 2001 for particleSystem

// if UO_3DonlyEffect is true, sta has to be valid and contain particleSystem data (if not, crash)
// for particleSystem data layout see staticeffectUO3d and updated packetDoku
// for old 2d staticeffect stuff , the new (3d client) pararamters UO_3Donlyeffect, sta and skip_old are defaulted in such a way that they behave like they did before
// simply dont set them in that case
// the last parameter is for particlesystem optimization only (dangerous). don't use unless you know 101% what you are doing.

void staticeffect(P_CHAR pc_player, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop,  bool UO3DonlyEffekt, stat_st *sta, bool skip_old)
{
	int a0,a1,a2,a3,a4;
	unsigned char effect[29] = {0,};
	int j;
	if ( pc_player == NULL ) return;

	if (!skip_old)
	{
//		memset(&effect[0], 0, 29);	
	    effect[0]=0x70; // Effect message
	    effect[1]=0x03; // Static effect
		LongToCharPtr(pc_player->serial, &effect[2]);
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
		 if ((inrange1p(currchar[j], pc_player))&&(perm[j]))
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

			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles	
			 {

				staticeffectUO3D(pc_player, sta);

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


void movingeffect(P_CHAR pc_source, P_CHAR pc_dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt, move_st *str, bool skip_old )
{
	
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

}

void bolteffect(P_CHAR pc_player, bool UO3DonlyEffekt, bool skip_old )
{
	unsigned char effect[29] = {0,};
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
	}		
}


// staticeffect2 is for effects on items
void staticeffect2(P_ITEM pi, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt,  stat_st *str, bool skip_old )
{
	unsigned char effect[29] = {0,};
	
	if (!skip_old)
	{
//		memset(&effect[0], 0, 29);
		effect[0]=0x70; // Effect message
		effect[1]=0x02; // Static effect
		LongToCharPtr(pi->serial, &effect[2]);
		LongToCharPtr(pi->serial, &effect[6]);
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
		int j;
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
		int j;
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


void bolteffect2(P_CHAR pc_player,char a1,char a2)	// experimenatal, lb
{
	unsigned char effect[29] = {0,};
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
	}
}

//	- Movingeffect3 is used to send an object from a char
//    to another object (like purple potions)
void movingeffect3(P_CHAR pc_source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
	unsigned char effect[29] = {0,};
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
	}
}

// staticeffect3 is for effects on items
void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode)
{
	char effect[29] = {0,};
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
	}
}

void movingeffect3(P_CHAR pc_source, P_CHAR pc_dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode,unsigned char unk1,unsigned char unk2,unsigned char ajust,unsigned char type)
{
	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
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
	}
}



//	- Movingeffect2 is used to send an object from a char
//	to another object (like purple potions)
void movingeffect2(P_CHAR pc_source, P_ITEM dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
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
	}
}

void dolight(int s, char level)
{
	char light[3]="\x4F\x00";

	if ((s==-1)||(!perm[s])) return;
	P_CHAR pc_currchar = currchar[s];

	light[1]=level;
	if (SrvParams->worldFixedLevel() != 255)
	{
		light[1] = SrvParams->worldFixedLevel();
	} 
	else if (pc_currchar->fixedlight!=255)
	{
		light[1]=pc_currchar->fixedlight;
	} 
	else if (indungeon(currchar[s]))
	{
		light[1]= SrvParams->dungeonLightLevel();
	}
	else
	{
		light[1]=level;
	}
	Xsend(s, light, 2);
}

void updateskill(int s, int skillnum) // updated for client 1.26.2b by LB
{
	char update[11];
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
	x = pc_currchar->lockSkill[skillnum];
	if (x != 0 && x != 1 && x != 2) 
		x = 0;
	update[10] = x;
	
	// CRASH_IF_INVALID_SOCK(s);
	
	Xsend(s, update, 11);
}

void deathaction(P_CHAR pc, P_ITEM pi_x) // Character does a certain action
{
	int i;
	unsigned char deathact[14]="\xAF\x01\x02\x03\x04\x01\x02\x00\x05\x00\x00\x00\x00";

	if (pi_x == NULL || pc == NULL)
		return;

	LongToCharPtr(pc->serial, &deathact[1]);
	LongToCharPtr(pi_x->serial,deathact+5);
	
	for (i=0;i<now;i++) 
		if ((inrange1p(pc, currchar[i]))&&(perm[i]) && (currchar[i]!=pc)) 
			Xsend(i, deathact, 13);
	
}

void deathmenu(int s) // Character sees death menu
{
	char testact[3]="\x2C\x00";
	Xsend(s, testact, 2);
}

void impowncreate(int s, P_CHAR pc, int z) //socket, player to send
{
	int k;
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
	if (z) oc[13]=pc->dispz; // Character z position
	else oc[13]=pc->pos.z;
	oc[14]=pc->dir; // Character direction
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
#ifdef DEBUG
					ConOut("Double layer (%i) on Item (%i) on Char (%i)\n", pi->layer() , j , i);
					sprintf(temp, "Double layer (%i) on Item (%2x %2x %2x %2x) on Char (%2x %2x %2x %2x)\n",
						pi->layer(), pi->ser1, pi->ser2, pi->ser3, pi->ser4,
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
	k += 4;

	// unimportant remark: its a packet "terminator" !!! LB

	oc[1]=k>>8;
	oc[2]=k%256;
	Xsend(s, oc, k);
}

void sendshopinfo(int s, P_CHAR pc, P_ITEM pi)
{
	unsigned char m1[6096] = {0,};
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
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi_j = FindItemBySerial(vecContainer[ci]);
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
}

int sellstuff(int s, P_CHAR pc)
{
	char itemname[256];
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
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if ((pi->contserial==serial) && (pi->layer()==0x1C))
			{
				sellcont = pi;
				break;
			}
	}
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
				serial1 = pBackpack->serial;
				unsigned int ci1;
				vector<SERIAL> vecContainer2 = contsp.getData(serial1);
				for ( ci1 = 0; ci1 < vecContainer2.size(); ci1++)
				{
					P_ITEM pi_j = FindItemBySerial(vecContainer2[ci1]);
					if (pi_j != NULL) // LB crashfix
					{
						sprintf(ciname,"'%s'",pi_j->name().ascii()); // Added by Magius(CHE)
						sprintf(cinam2,"'%s'",pi_q->name().ascii()); // Added by Magius(CHE)
						strupr(ciname); // Added by Magius(CHE)
						strupr(cinam2); // Added by Magius(CHE)

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
		npctalkall(pc, "Thou doth posses nothing of interest to me.",0);
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

void sendtradestatus(P_ITEM cont1, P_ITEM cont2)
{
	unsigned char msg[30];

	P_CHAR p1 = FindCharBySerial(cont1->contserial);
	P_CHAR p2 = FindCharBySerial(cont2->contserial);
	UOXSOCKET s1 = calcSocketFromChar(p1);
	UOXSOCKET s2 = calcSocketFromChar(p2);

	if ( ( s1 == -1 ) || ( s2 == -1 ) ) return; 

	msg[0]=0x6F;//Header
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x02;//State
	LongToCharPtr(cont1->serial,msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=cont1->morez%256;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=cont2->morez%256;
	msg[16]=0; // No name in this message
	Xsend(s1, msg, 17);

	LongToCharPtr(cont2->serial,msg+4);
	msg[11]=cont2->morez%256;
	msg[15]=cont1->morez%256;
	Xsend(s2, msg, 17);
}

void endtrade(SERIAL serial)
{
	unsigned char msg[30];
	P_ITEM pi_cont1 = NULL, pi_cont2 = NULL;

	pi_cont1 = FindItemBySerial(serial);
	if (pi_cont1 == NULL) 
		return; // LB, crashfix
	pi_cont2 = FindItemBySerial(calcserial(pi_cont1->moreb1(), pi_cont1->moreb2(), pi_cont1->moreb3(), pi_cont1->moreb4()));
	if (pi_cont2 == NULL) return; // LB, crashfix
	P_CHAR pc1 = FindCharBySerial(pi_cont1->contserial);
	P_CHAR pc2 = FindCharBySerial(pi_cont2->contserial);
	P_ITEM pi_bp1 = Packitem(pc1);
	if (pi_bp1 == NULL) return;
	P_ITEM pi_bp2 = Packitem(pc2);
	if (pi_bp2 == NULL) return;
	UOXSOCKET s1 = calcSocketFromChar(pc1);
	UOXSOCKET s2 = calcSocketFromChar(pc2);

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
				pi->setContSerial(pi_bp1->serial);
				if (pi->glow != INVALID_SERIAL) 
					glowsp.insert(pc1->serial, pi->serial);
				pi->pos.x = RandomNum(50, 130);
				pi->pos.y = RandomNum(50, 130);
				pi->pos.z=9;
				if (s1 != -1)
					RefreshItem(pi);//AntiChrist
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
				pi->setContSerial(pi_bp2->serial);
				if (pi->glow != INVALID_SERIAL) 
					glowsp.insert(pc1->serial, pi->serial);
				pi->pos.x=50+(rand()%80);
				pi->pos.y=50+(rand()%80);
				pi->pos.z=9;
				if (s2 != -1)
					RefreshItem(pi);//AntiChrist
			}
	}
	Items->DeleItem(pi_cont1);
	Items->DeleItem(pi_cont2);
}

void tellmessage(int i, int s, const char *txt)
{
	int tl;
	P_CHAR pc_currchar = currchar[s];

	sprintf((char*)temp, "GM tells %s: %s", pc_currchar->name.c_str(), txt);

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


void staticeffectUO3D(P_CHAR pc_cs, stat_st *sta)
{  
   
   if ( pc_cs == NULL )
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

void movingeffectUO3D(P_CHAR pc_cs, P_CHAR pc_cd, move_st *sta)
{
   if (pc_cs == NULL || pc_cd == NULL) return;

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
	particleSystem[48]=0x0;
}

void bolteffectUO3D(P_CHAR player)
{
	Magic->doStaticEffect(player, 30);
}

void PlayDeathSound( P_CHAR pc )
{
    if ( pc == NULL )
		return;

	if (pc->xid==0x0191)
	{
		switch(RandomNum(0, 3)) // AntiChrist - uses all the sound effects
		{
		case 0:		soundeffect2( pc, 0x0150 );	break;// Female Death
		case 1:		soundeffect2( pc, 0x0151 );	break;// Female Death
		case 2:		soundeffect2( pc, 0x0152 );	break;// Female Death
		case 3:		soundeffect2( pc, 0x0153 );	break;// Female Death
		}
	}
	else if (pc->xid==0x0190)
	{
		switch( RandomNum(0, 3) ) // AntiChrist - uses all the sound effects
		{
		case 0:		soundeffect2( pc, 0x015A );	break;// Male Death
		case 1:		soundeffect2( pc, 0x015B );	break;// Male Death
		case 2:		soundeffect2( pc, 0x015C );	break;// Male Death
		case 3:		soundeffect2( pc, 0x015D );	break;// Male Death
		}
	}
	else
	{
		playmonstersound(pc, pc->xid, SND_DIE);
	}
}

// NEW STYLE CLASSES
void cPacket::send( UOXSOCKET socket )
{
	if( socket == -1 )
		return;

	Xsend( socket, data.data(), data.size() );
}

void cVariablePacket::send( UOXSOCKET socket )
{
	// Check the packet length
	if( data.size() >= 3 )
	{
		data[ 1 ] = static_cast< UI08 >( data.size() >> 8 );
		data[ 2 ] = static_cast< UI08 >( data.size() );
	}

	// Call the normal send routine
	cPacket::send( socket );
}

// 0x24 Draw Container (7 bytes) 
// BYTE cmd 
// BYTE[4] item id 
// BYTE[2] model-Gump 
cPDrawContainer::cPDrawContainer( UI16 gumpId, SERIAL serial )
{
	data.resize( 7 ); // 7 Pyte packet
	data[ 0 ] = 0x24;
	
	// Container Serial
	data[ 1 ] = static_cast< UI08 >( ( serial ) >> 24 );
	data[ 2 ] = static_cast< UI08 >( ( serial ) >> 16 );
	data[ 3 ] = static_cast< UI08 >( ( serial ) >> 8 );
	data[ 4 ] = static_cast< UI08 >( serial );

	// Gump ID
	data[ 5 ] = static_cast< UI08 >( gumpId >> 8 );
	data[ 6 ] = static_cast< UI08 >( gumpId );
}

// Add Items to Container
cPContainerItems::cPContainerItems( void )
{
	data.resize( 5 );

	data.fill( 0, 5 );
	data[ 0 ] = 0x3C;
}

void cPContainerItems::addItem( P_ITEM item )
{
	addItem( item->serial, item->id(), item->amount(), item->pos.x, item->pos.y, item->contserial, item->color() );
}

// Add an Item to the container
void cPContainerItems::addItem( SERIAL serial, UI16 model, UI16 amount, UI16 x, UI16 y, SERIAL contserial, UI16 hue )
{
	SI32 offset = data.size();

	data.resize( data.size() + 19 );

	// Item serial
	data[ (int)(offset + 0) ] = static_cast< UI08 >( serial >> 24 );
	data[ (int)(offset + 1) ] = static_cast< UI08 >( serial >> 16 );
	data[ (int)(offset + 2) ] = static_cast< UI08 >( serial >> 8 );
	data[ (int)(offset + 3) ] = static_cast< UI08 >( serial );

	// Item model
	data[ (int)(offset + 4) ] = static_cast< UI08 >( model >> 8 );
	data[ (int)(offset + 5) ] = static_cast< UI08 >( model );	

	// Unknown ?
	data[ (int)(offset + 6) ] = 0x00;

	// Amount
	data[ (int)(offset + 7) ] = static_cast< UI08 >( amount >> 8 );
	data[ (int)(offset + 8) ] = static_cast< UI08 >( amount );	

	// X + Y
	data[ (int)(offset + 9) ]  = static_cast< UI08 >( x >> 8 );
	data[ (int)(offset + 10) ] = static_cast< UI08 >( x );	
	data[ (int)(offset + 11) ] = static_cast< UI08 >( y >> 8 );
	data[ (int)(offset + 12) ] = static_cast< UI08 >( y );

	// Contserial
	data[ (int)(offset + 13) ] = static_cast< UI08 >( contserial >> 24 );
	data[ (int)(offset + 14) ] = static_cast< UI08 >( contserial >> 16 );
	data[ (int)(offset + 15) ] = static_cast< UI08 >( contserial >> 8 );
	data[ (int)(offset + 16) ] = static_cast< UI08 >( contserial );

	// Hue
	data[ (int)(offset + 17) ] = static_cast< UI08 >( hue >> 8 );
	data[ (int)(offset + 18) ] = static_cast< UI08 >( hue );	

	// Increase the itemcount:
	UI16 itemCount = ( data[ 3 ] << 8 ) | ( data[ 4 ] ) + 1;
	data[ 3 ] = static_cast< UI08 >( itemCount >> 8 );
	data[ 4 ] = static_cast< UI08 >( itemCount );
}

cBounceItem::cBounceItem( bool denyMove )
{
	data.resize( 2 );
	data[ 0 ] = 0x27;
	data[ 1 ] = denyMove ? 0 : 5;
}

cWornItems::cWornItems( SERIAL playerId, SERIAL itemId, UI08 layer, UI16 model, UI16 color )
{
	data.resize( 15 );
	data[ 0 ] = 0x2E;

	data[ 1 ] = static_cast< UI08 >( itemId >> 24 );
	data[ 2 ] = static_cast< UI08 >( itemId >> 16 );
	data[ 3 ] = static_cast< UI08 >( itemId >> 8 );
	data[ 4 ] = static_cast< UI08 >( itemId );

	data[ 5 ] = static_cast< UI08 >( model >> 8 );
	data[ 6 ] = static_cast< UI08 >( model );

	data[ 8 ] = layer;

	data[ 9 ] = static_cast< UI08 >( playerId >> 24 );
	data[ 10 ] = static_cast< UI08 >( playerId >> 16 );
	data[ 11 ] = static_cast< UI08 >( playerId >> 8 );
	data[ 12 ] = static_cast< UI08 >( playerId );

	data[ 13 ] = static_cast< UI08 >( color >> 8 );
	data[ 14 ] = static_cast< UI08 >( color );
}

cSoundEffect::cSoundEffect( UI16 soundId, const Coord_cl &pos, UI08 mode )
{
	data.resize( 12 );
	data[ 0 ] = 0x54;
	data[ 1 ] = mode;

	data[ 2 ] = static_cast< UI08 >( soundId >> 8 );
	data[ 3 ] = static_cast< UI08 >( soundId );

	data[ 4 ] = 0;
	data[ 5 ] = 0;

	// x + y + z
	data[ 6 ] = static_cast< UI08 >( pos.x >> 8 );
	data[ 7 ] = static_cast< UI08 >( pos.x );

	data[ 8 ] = static_cast< UI08 >( pos.y >> 8 );
	data[ 9 ] = static_cast< UI08 >( pos.y );

	data[ 10 ] = static_cast< UI08 >( pos.z >> 8 );
	data[ 11 ] = static_cast< UI08 >( pos.z );
}

cUnicodeSpeech::cUnicodeSpeech( cUObject *origin, const QString &message, UI16 color, UI16 font, const QString &lang, eTextType type )
{
		data.resize( 48 + ( ( message.length() + 1 ) * 2 ) + 1 );
		data.fill( (char)0x00 );

		data[ 0 ] = 0xAE;
		data[ 9 ] = type;
		data[ 10 ] = static_cast< UI08 >( color >> 8 );
		data[ 11 ] = static_cast< UI08 >( color );

		data[ 12 ] = static_cast< UI08 >( font >> 8 );
		data[ 13 ] = static_cast< UI08 >( font );
		
		// 14-17 -> Language
		data[ 14 ] = 'D';
		data[ 15 ] = 'E';
		data[ 16 ] = 'U';
		data[ 17 ] = 0;

		if( !origin )
		{
			LongToCharPtr( INVALID_SERIAL, (UI08*)&data.data()[3] );
			ShortToCharPtr( 0xFFFF, (UI08*)&data.data()[7] );
			strcpy( &data.data()[18], "System" );
		}
		else
		{
			LongToCharPtr( origin->serial, (UI08*)&data.data()[3] );

			// Model ID
			if( isCharSerial( origin->serial ) )
			{
				P_CHAR pChar = static_cast< P_CHAR >( origin );
				ShortToCharPtr( pChar->id(), (UI08*)&data.data()[7] );
				QString name = pChar->name.c_str();

				if( name.length() > 29 )
					name = name.left( 29 );

				strcpy( &data.data()[18], name.latin1() );
			}
			else if( isItemSerial( origin->serial ) )
			{
				P_ITEM pItem = static_cast< P_ITEM >( origin );
				ShortToCharPtr( pItem->id(), (UI08*)&data.data()[7] );
				QString name = pItem->getName();
				
				if( name.length() > 29 )
					name = name.left( 29 );

				strcpy( &data.data()[18], name.latin1() );
			}
			else
			{
				ShortToCharPtr( 0xFFFF, (UI08*)&data.data()[7] );
				data[18] = 0x00;
			}			
		}

		// Copy the message
		memcpy( &data.data()[49], message.unicode(), message.length()*2 );
}
