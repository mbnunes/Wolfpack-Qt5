//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#if !defined(__GLOBALS_H__)
#define __GLOBALS_H__
// Platform specifics
#include "platform.h"

// System Includes

#include <vector>
#include <map>
#include <list>

#if defined(__unix__)
#include <termios.h>
#endif
// Third Party


//Forward Class Declaration

class QDateTime;
class cScriptManager;
class WPDefManager;
class WPConsole_cl;
//Wolfpack Includes

#include "structs.h"
#include "storage.h"

enum enServerState { STARTUP = 0, RUNNING, SCRIPTRELOAD, SHUTDOWN };

// Global Variables

extern enServerState serverState;

//	Instantiate our console object
extern WPConsole_cl clConsole;

extern unsigned int uiCurrentTime;
extern unsigned int raindroptime;
extern unsigned int polyduration;
//extern wp_version_info wp_version;

//Time variables
extern QDateTime uoTime;
extern int uotickcount;

//extern int goldamount;

extern unsigned int nextfieldeffecttime;
extern unsigned int nextnpcaitime;
extern unsigned int nextdecaytime;

extern int autosaved, dosavewarning;
extern int err, error;
extern int keeprun;
//extern int now;
extern int secure; // Secure mode

extern unsigned int starttime, endtime, lclock;

extern unsigned int shoprestocktime;

extern unsigned int hungerdamagetimer; // Used for hunger damage
// Profiling
extern int networkTime;
extern int timerTime;
extern int autoTime;
extern int loopTime;
extern int networkTimeCount;
extern int timerTimeCount;
extern int autoTimeCount;
extern int loopTimeCount;

extern unsigned long int serverstarttime;

extern unsigned long initialserversec;
extern unsigned long initialservermill ;



// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

extern Container_cl stablesp;
extern std::list<SERIAL> guilds;

/******************************************************/

///////////////////////////////////////////
///   and the rest                        /
///////////////////////////////////////////

extern unsigned short int doorbase[DOORTYPES];

extern creat_st creatures[2048]; //LB, stores the base-sound+sound flags of monsters, animals

extern int npcshape[5]; //Stores the coords of the bouding shape for the NPC

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

extern char temp[1024];

// Forward declarations
class cSrvParams;
class cBoat;
class cAccount;
class cAllItems;
class Maps;
class cSkills;
class cTargets;
class cMovement;
class cSpeech;
class cBounty;
class cAccounts;
class PersistentBroker;

//-=-=-=-=-=-=-Classes Definitions=-=-=-=-=-=//
extern cSrvParams				*SrvParams;
extern cAllItems				*Items;
extern Maps						*Map;
extern cSkills					*Skills;
extern cSpeech					*Speech;
extern cBounty					*Bounty;
extern cScriptManager			*ScriptManager;
extern WPDefManager				*DefManager;
extern PersistentBroker			*persistentBroker;
// END
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-//


#endif




