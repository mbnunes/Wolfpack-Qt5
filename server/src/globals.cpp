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

// Global Variables

#include "wolfpack.h"
#include "verinfo.h"
#include "walking.h"
#include "wpscriptmanager.h"
#include "wpdefmanager.h"
#include "wptargetrequests.h"
#include "typedefs.h"

#if defined(__unix__)
termios termstate ;
#endif
unsigned int uiCurrentTime;
wp_version_info wp_version;

// Library Includes
#include <qdatetime.h>


using namespace std;

enServerState serverState;

//	Instantiate our console object
WPConsole_cl clConsole;

unsigned short int doorbase[DOORTYPES]={
0x0675, 0x0685, 0x0695, 0x06A5, 0x06B5, 0x06C5, 0x06D5, 0x06E5, 0x0839, 0x084C, 
0x0866, 0x00E8, 0x0314, 0x0324, 0x0334, 0x0344, 0x0354};

time_t	oldtime, newtime;

unsigned int polyduration=90;

//Time variables
QDateTime uoTime;
int secondsperuominute=5; //Number of seconds for a WOLFPACK minute.
int uotickcount=1;
unsigned long initialserversec ;
unsigned long initialservermill ;
//int goldamount;
int defaultpriv1;
int defaultpriv2;
unsigned int nextfieldeffecttime=0;
unsigned int nextnpcaitime=0;
unsigned int nextdecaytime=0;

// MSVC fails to compile WOLFPACK if this is unsigned, change it then
int autosaved, saveinterval;
int dosavewarning=0;	
bool heartbeat;

int err, error;
int keeprun;

//int now;

int secure; // Secure mode

// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

unsigned int raindroptime;

int tnum;

unsigned int starttime, endtime, lclock;

unsigned int fly_p=18; // flying probability = 1/fly_p each step (if it doesnt fly)
unsigned char fly_steps_max=27;

unsigned int shoprestocktime=0;

//int *spawnedguards;
// Profiling
int networkTime = 0;
int timerTime = 0;
int autoTime = 0;
int loopTime = 0;
int networkTimeCount = 1000;
int timerTimeCount = 1000;
int autoTimeCount = 1000;
int loopTimeCount = 1000;

unsigned long int serverstarttime;

// Script files that need to be cached
// Crackerjack Jul 31/99

int escortRegions = 0;


unsigned int hungerdamagetimer=0; // For hunger damage

//-=-=-=-=-=-=-Classes Definitions=-=-=-=-=-=//
cSrvParams		*SrvParams;
cAllItems		*Items;
Maps			*Map;
cSkills			*Skills;
cMagic			*Magic;
cSpeech	        *Speech;
cTrade	        *Trade;
cBounty         *Bounty;
WPScriptManager	*ScriptManager;
WPDefManager    *DefManager;
PersistentBroker* persistentBroker;


// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

Container_cl stablesp, contsp, cspawnsp, spawnsp;

list<SERIAL> guilds;

/******************************************************/

///////////////////////////////////////////
///   and the rest                        /
///////////////////////////////////////////
 int validEscortRegion[256];
 creat_st creatures[2048];

 int npcshape[5]; //Stores the coords of the bouding shape for the NPC

 jail_st jails[11];

 std::vector<std::string> clientsAllowed; // client version controll system

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

 char temp[1024];

