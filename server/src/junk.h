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

#include "items.h"
#include "chars.h"
#include "Trade.h"
#include "bounty.h"

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
extern cSrvParams				*SrvParams;
extern CWorldMain				*cwmWorldState;
extern cAllItems				*Items;
extern Maps						*Map;
extern cSkills					*Skills;
extern cFishing					*Fishing;
extern cWeight					*Weight;
extern cTargets					*Targ;
extern cMagic					*Magic;
extern cSpeech					*Speech;
extern cTrade					*Trade;
extern cBounty					*Bounty;
extern cMultisCache				*MultisCache;
extern cCharStuff::cDragonAI	*DragonAI;
extern cCharStuff::cBankerAI	*BankerAI;
// END
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-//


extern int escortRegions;

bool inrange1p (PC_CHAR pca, P_CHAR pcb);

unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1);
unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);

#include "basics.h"

void setcharflag(P_CHAR pc);
P_ITEM FindItemBySerial(int serial);
P_CHAR FindCharBySerial(int serial);

/////////////////////////////////// Remove Me before release!!!! ///////////////////////////
#include "network.h"
#include "globals.h"

inline UOXSOCKET toOldSocket(const cUOSocket* s)
{
	return cNetwork::instance()->getuoSocketsIndex(s);
}

////////////////////////////////////////////////////////////////////////////////////////////

#endif

