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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#if !defined(__JUNK_H__)
#define __JUNK_H__

#include "admin.h"
#include "targeting.h"
/*
struct statcap_st
{
	int statmax; //250 stat points (100 str, 100 int, 50 dex?)
	int skillmax; //900 BASE skill points
	long int skilldecay; //12000 seconds
	int numsktrk; //track the last numsktrk (10) skills SUCCESSFULLY used
}; // Morrolan - stat/skill cap
*/

#include "items.h"
#include "chars.h"
#include "classes.h"
#include "guildstones.h"
#include "gumps.h"
#include "network.h"
#include "TmpEff.h"
#include "walking2.h"
#include "mapstuff.h"

extern cCharArray *CharArray;
#include "books.h"
#include "speech.h"
#include "trigger.h"
#include "dragdrop.h"
#include "Trade.h"
#include "bounty.h"
//-=-=-=-=-=-=-Classes Definitions=-=-=-=-=-=//
extern cBoat			*Boats;
extern CWorldMain		*cwmWorldState;
extern cAccount			*Accounts;
extern cAdmin			*Admin;
extern cCombat			*Combat;
extern cCommands		*Commands;
extern cGuilds			*Guilds;
extern cGump			*Gumps;
extern cAllItems		*Items;
extern cMapStuff		*Map;
extern cCharStuff		*Npcs;
extern cSkills			*Skills;
extern cFishing			*Fishing;
extern cWeight			*Weight;
extern cTargets			*Targ;
extern cNetworkStuff	*Network;
extern cMagic			*Magic;
extern cBooks           *Books;
extern cAllTmpEff       *AllTmpEff;
extern cMovement		*Movement;
extern cSpeech	        *Speech;
extern cTrigger	        *Trig;
extern cDragdrop        *Drag;
extern cTrade	        *Trade;
extern cBounty          *Bounty;
// Skys AI 
extern cCharStuff::cDragonAI	*DragonAI;
extern cCharStuff::cBankerAI	*BankerAI;
// END
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-//
#if !defined(MIN)
#define MIN(arga, argb) (arga<argb?arga:argb)
#endif

extern int escortRegions;

void charcreate(UOXSOCKET s);	// All the character creation stuff
void srequest(int s);

int inrange2 (UOXSOCKET s, P_ITEM pi);
bool inrange1p (PC_CHAR pca, P_CHAR pcb);
bool iteminrange (const UOXSOCKET s, const P_ITEM pi, const int distance);
void vialtarget(int nSocket);
void MakeNecroReg(int nSocket,P_ITEM pMat, short id);
void usepotion(int p, P_ITEM pi);

unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1);
unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);

void checkauto();	// Check automatic/timer controlled stuff (Like fighting and regeneration)
void checktimers();	// Check shutdown timers
#include "basics.h"

void setcharflag(P_CHAR pc);
P_ITEM FindItemBySerial(int serial);
P_ITEM FindItemBySerPtr(unsigned char *p);
P_CHAR FindCharBySerial(int serial);
P_CHAR FindCharBySerPtr(unsigned char *p);
P_ITEM Packitem(P_CHAR pc); // Find packitem

#include "regions.h"
extern cRegion			*mapRegions; //setup map regions Tauriel

P_ITEM GetOutmostCont(P_ITEM pItem, short rec = 10);
P_CHAR GetPackOwner(P_ITEM pItem, short rec = 10);


//////////////////////////// INLINES /////////////////////

////////////////////////////
// name:	calcCharFromPtr, calcItemFromPtr
// history: by Duke, 08.04.2000
//			added calcCharFromPtr Duke, 02.10.2000
// Purpose:	calculates the char/item index from 4 subsequent bytes pointed to by 'p',
//			intended to replace calcserial() & findbyserial as follows
//
//				old: serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
//				old: i=findbyserial(&charsp[serial%HASHMAX], serial, 1);
//
//				new: i=calcCharFromPtr(buffer[s]+7);
//
// Remarks: should be moved to WOLFPACK.cpp and declared in WOLFPACK.h
//


#undef  DBGFILE
#define DBGFILE "wolfpack.h"

#endif

