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

//	Instantiate our console object
WPConsole_cl clConsole;

unsigned short int doorbase[DOORTYPES]={
0x0675, 0x0685, 0x0695, 0x06A5, 0x06B5, 0x06C5, 0x06D5, 0x06E5, 0x0839, 0x084C, 
0x0866, 0x00E8, 0x0314, 0x0324, 0x0334, 0x0344, 0x0354};

char skillname[SKILLS+1][20]={
"ALCHEMY", "ANATOMY", "ANIMALLORE", "ITEMID", "ARMSLORE", "PARRYING", "BEGGING", "BLACKSMITHING", "BOWCRAFT",
"PEACEMAKING", "CAMPING", "CARPENTRY", "CARTOGRAPHY", "COOKING", "DETECTINGHIDDEN", "ENTICEMENT", "EVALUATINGINTEL",
"HEALING", "FISHING", "FORENSICS", "HERDING", "HIDING", "PROVOCATION", "INSCRIPTION", "LOCKPICKING", "MAGERY",
"MAGICRESISTANCE", "TACTICS", "SNOOPING", "MUSICIANSHIP", "POISONING", "ARCHERY", "SPIRITSPEAK", "STEALING",
"TAILORING", "TAMING", "TASTEID", "TINKERING", "TRACKING", "VETERINARY", "SWORDSMANSHIP", "MACEFIGHTING", "FENCING",
"WRESTLING", "LUMBERJACKING", "MINING", "MEDITATION", "STEALTH", "REMOVETRAPS", "ALLSKILLS", "STR", "DEX", "INT", "FAME", "KARMA"
};

char spellname[71][25]={
        "Clumsy","Create Food","Feeblemind","Heal","Magic Arrow","Night Sight","Reactive Armor","Weaken",
        "Agility","Cunning","Cure","Harm","Magic Trap","Magic Untrap","Protection","Strength",
        "Bless","Fireball","Magic Lock","Poison","Telekinesis","Teleport","Unlock","Wall Of Stone",
        "Arch Cure","Arch Protection","Curse","Fire Field","Greater Heal","Lightning","Mana Drain","Recall",
        "Blade Spirits","Dispel Field","Incognito","Magic Reflection","Mind Blast","Paralyze","Poison Field","Summon Creature",
        "Dispel","Energy Bolt","Explosion","Invisibility","Mark","Mass Curse","Paralyze Field","Reveal",
        "Chain Lightning","Energy Field","Flame Strike","Gate Travel","Mana Vampire","Mass Dispel","Meteor Swarm","Polymorph",
        "Earthquake","Energy Vortex","Resurrection","Summon Air Elemental",
        "Summon Daemon","Summon Earth Elemental","Summon Fire Elemental","Summon Water Elemental",
        "Random", "Necro1", "Necro2", "Necro3", "Necro4", "Necro5"
};

char foodname[enNumberOfFood][20]={
"no food", "raw meat", "raw fish", "eggs", "crops", "fruits", "hay",
"grain", "cooked meat", "cooked fish", "pastries", "leather", "metal" };
          
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

int now;

int secure; // Secure mode

int xycount;

// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

unsigned int raindroptime;

//unsigned int cmem;
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
CWorldMain		*cwmWorldState;
cAllItems		*Items;
Maps			*Map;
cSkills			*Skills;
cFishing        *Fishing;
cWeight			*Weight;
cTargets		*Targ;
cMagic			*Magic;
cSpeech	        *Speech;
cTrade	        *Trade;
cBounty         *Bounty;
cMultisCache	*MultisCache;
WPScriptManager	*ScriptManager;
WPDefManager    *DefManager;
PersistentBroker* persistentBroker;

cCharStuff::cDragonAI	*DragonAI;

//////////////////// ARRAYS that didnt get initialized above ////////////////////

/////////////////////////////////////////////
///             MAXCLIENT arrays          ///
/////////////////////////////////////////////
// maximum too cause maxclient = maximum +1

 P_CHAR currchar[MAXCLIENT];

/////////////////////////////////////////////
///             hasmax                   ///
/////////////////////////////////////////////


// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

Container_cl stablesp, contsp, cspawnsp, spawnsp;

list<SERIAL> guilds;

/******************************************************/

///////////////////////////////////////////
///   and the rest                        /
///////////////////////////////////////////

 skill_st skill[SKILLS+1];
 std::multimap <int, tele_locations_st> tele_locations; // can't use a map here, so using a multimap with x as the key :(
 title_st title[ALLSKILLS+1];

 advance_st wpadvance[1000];
 int validEscortRegion[256];
 creat_st creatures[2048];

 int npcshape[5]; //Stores the coords of the bouding shape for the NPC

 jail_st jails[11];

 std::vector<std::string> clientsAllowed; // client version controll system

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

 char temp[1024];

