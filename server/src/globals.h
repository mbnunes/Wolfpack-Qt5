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

//#include "netsys.h"			// Needed by fd_set
// Third Party


//Forward Class Declaration

class QDateTime;
class WPScriptManager;
class WPDefManager;
class WPConsole_cl;
//Wolfpack Includes

#include "structs.h"
#include "verinfo.h"
#include "storage.h"
//#include "wpscriptmanager.h"
//#include "wpdefmanager.h"

// Global Variables
//	Instantiate our console object

extern WPConsole_cl clConsole;

extern unsigned int uiCurrentTime;
extern unsigned int raindroptime;
extern unsigned int polyduration;
extern wp_version_info wp_version;

extern unsigned int fly_p; // flying probability = 1/fly_p each step (if it doesnt fly)
extern unsigned char fly_steps_max;

extern int gatecount;
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
extern int now;
extern int secure; // Secure mode

extern int xycount;

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


/////////////////////////////////////////////
///             MAXCLIENT arrays          ///
/////////////////////////////////////////////
// maximum too cause maxclient = maximum +1

extern unsigned char  buffer[MAXCLIENT][MAXBUFFER_REAL];
extern signed char addid5[MAXCLIENT];
extern P_CHAR currchar[MAXCLIENT];
extern make_st itemmake[MAXCLIENT];
extern int tempint[MAXCLIENT];
extern unsigned char addid1[MAXCLIENT];
extern unsigned char addid2[MAXCLIENT];
extern unsigned char addid3[MAXCLIENT];
extern unsigned char addid4[MAXCLIENT];
extern int addx[MAXCLIENT];
extern int addy[MAXCLIENT];
extern int addx2[MAXCLIENT];
extern int addy2[MAXCLIENT];
extern signed char addz[MAXCLIENT];
extern int addmitem[MAXCLIENT];
extern char xtext[MAXCLIENT][31];
extern unsigned char perm[MAXCLIENT];
//extern unsigned char clientDimension[MAXCLIENT]; // stores if the client is the new 3d or old 2d one
extern std::string SocketStrings[MAXCLIENT];

//extern int spattackValue[MAXCLIENT];
extern int clickx[MAXCLIENT];
extern int clicky[MAXCLIENT];
extern int currentSpellType[MAXCLIENT]; // 0=spellcast, 1=scrollcast, 2=wand cast
extern unsigned char targetok[MAXCLIENT];

// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

extern Container_cl stablesp, cspawnsp, spawnsp;
extern std::list<SERIAL> guilds;

/******************************************************/

///////////////////////////////////////////
///   and the rest                        /
///////////////////////////////////////////

extern int gatex[MAXGATES][2],gatey[MAXGATES][2];
extern signed char gatez[MAXGATES][2];
//extern unitile_st xyblock[XYMAX];
extern skill_st skill[SKILLS+1];
extern unsigned short int doorbase[DOORTYPES];
extern char skillname[SKILLS+1][20];
extern  std::multimap <int, tele_locations_st> tele_locations; // can't use a map here :(
extern title_st title[ALLSKILLS+1];

extern creat_st creatures[2048]; //LB, stores the base-sound+sound flags of monsters, animals
extern advance_st wpadvance[1000];
//extern char spellname[71][25];
extern char foodname[enNumberOfFood][20];
extern int validEscortRegion[256];

extern int npcshape[5]; //Stores the coords of the bouding shape for the NPC

extern jail_st jails[11];

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

extern char gettokenstr[256];
extern char temp[1024];

#ifdef __unix__
extern termios termstate ;
#endif

extern WPScriptManager *ScriptManager;
extern WPDefManager *DefManager;
class PersistentBroker;
extern PersistentBroker* persistentBroker;
//

#endif




