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

#if !defined(__JUNK_H__)
#define __JUNK_H__

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
//#include "classes.h"
//#include "guildstones.h"
//#include "network.h"
//#include "TmpEff.h"
//#include "walking2.h"
//#include "mapstuff.h"

//#include "books.h"
//#include "speech.h"
//#include "trigger.h"
#include "dragdrop.h"
#include "Trade.h"
#include "bounty.h"
//#include "regions.h"

#undef  DBGFILE
#define DBGFILE "junk.h"

// Forward declarations
class cSrvParams;
class CWorldMain;
class cBoat;
class cAccount;
class cAllItems;
class Maps;
class cSkills;
class cFishing;
class cWeight;
class cTargets;
class cMagic;
class cMovement;
class cSpeech;
class cTrade;
class cBounty;
class cAccounts;
class cMultisCache;

//-=-=-=-=-=-=-Classes Definitions=-=-=-=-=-=//
extern cSrvParams		*SrvParams;
extern CWorldMain		*cwmWorldState;
extern cAccounts		*Accounts;
extern cAllItems		*Items;
extern Maps				*Map;
extern cSkills			*Skills;
extern cFishing			*Fishing;
extern cWeight			*Weight;
extern cTargets			*Targ;
extern cMagic			*Magic;
extern cMovement		*Movement;
extern cSpeech	        *Speech;
extern cTrade	        *Trade;
extern cBounty          *Bounty;
extern cMultisCache		*MultisCache;

// Skys AI 
extern cCharStuff::cDragonAI	*DragonAI;
extern cCharStuff::cBankerAI	*BankerAI;
// END
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-//
#if !defined(MIN)
#define MIN(arga, argb) (arga<argb?arga:argb)
#endif

extern int escortRegions;

void srequest(int s);

int inrange2 (UOXSOCKET s, P_ITEM pi);
bool inrange1p (PC_CHAR pca, P_CHAR pcb);
bool iteminrange (const UOXSOCKET s, const P_ITEM pi, const int distance);
void vialtarget(int nSocket);
void MakeNecroReg(int nSocket,P_ITEM pMat, short id);
void usepotion(P_CHAR pc_p, P_ITEM pi);

unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1);
unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);

#include "basics.h"

void setcharflag(P_CHAR pc);
P_ITEM FindItemBySerial(int serial);
P_ITEM FindItemBySerPtr(unsigned char *p);
P_CHAR FindCharBySerial(int serial);
P_CHAR FindCharBySerPtr(unsigned char *p);
P_ITEM Packitem(P_CHAR pc); // Find packitem


P_ITEM GetOutmostCont(P_ITEM pItem, short rec = 10);
P_CHAR GetPackOwner(P_ITEM pItem, short rec = 10);

/////////////////////////////////// Remove Me before release!!!! ///////////////////////////
#include "network.h"
#include "globals.h"

inline UOXSOCKET toOldSocket(cUOSocket* s)
{
	return cNetwork::instance()->getuoSocketsIndex(s);
}

inline cUOSocket* fromOldSocket(UOXSOCKET s)
{
	return currchar[s]->socket();
}

////////////////////////////////////////////////////////////////////////////////////////////

#endif

