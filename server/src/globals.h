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

#include "netsys.h"			// Needed by fd_set
using namespace std;


// Third Party


//Forward Class Declaration

class Script;
class QDateTime;
class WPScriptManager;
class WPDefManager;

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

extern int gDecayItem; //Last item that was checked for decay Tauriel 3/7/99
extern int gRespawnItem; //Last item that was checked for respawn Tauriel 3/7/99

extern int save_counter;//LB, world backup rate

extern unsigned int fly_p; // flying probability = 1/fly_p each step (if it doesnt fly)
extern unsigned char fly_steps_max;

extern int gatecount;
//Time variables
extern QDateTime uoTime;
extern int uotickcount;
extern int hbu;       // heartbeat update var

extern int goldamount;

extern unsigned int nextfieldeffecttime;
extern unsigned int nextnpcaitime;
extern unsigned int nextdecaytime;

extern int autosaved, dosavewarning;
extern int len_connection_addr;
extern struct sockaddr_in connection;
extern struct sockaddr_in client_addr;
extern struct hostent *he;
extern int err, error;
extern int keeprun;
extern int a_socket;
extern fd_set conn ;
extern fd_set all ;
extern fd_set errsock ;
extern int nfds;
extern timeval uoxtimeout;
extern int now;


extern FILE *scpfile, *lstfile, *wscfile, *mapfile, *sidxfile, *statfile, *verfile, *tilefile, *multifile, *midxfile;

extern unsigned char xcounter;
extern unsigned char ycounter;        //x&y counter used for placing deleted items and chars
extern int secure; // Secure mode

extern int xycount;

extern int locationcount;
extern unsigned int logoutcount;//Instalog

extern unsigned long int updatepctime;

extern UI32 VersionRecordSize;
extern UI32 MultiRecordSize;
extern UI32 LandRecordSize;
extern UI32 TileRecordSize;
extern UI32 MapRecordSize;
extern UI32 MultiIndexRecordSize;
extern UI32 StaticRecordSize;

extern int cmem ;

extern int global_lis;

extern char *cline;

extern int tnum;
extern unsigned int starttime, endtime, lclock;
extern bool overflow;

extern unsigned char globallight;
extern unsigned char wtype;

extern int executebatch;
extern int showlayer;
extern int ph1, ph2, ph3, ph4;
extern int openings;

extern unsigned char tempflag;

extern short wp_port;
extern unsigned int shoprestocktime;
extern int shoprestockrate;
extern unsigned int respawntime;
extern unsigned int gatedesttime;

extern int triggerx;
extern int triggery;
extern signed char triggerz;
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
//extern	lookuptr_st MapCells[24576]; //A very large pointer structure array

extern unsigned long int serverstarttime;


enum  enScripts { items_script = 0,npc_script,create_script,regions_script,misc_script,skills_script,
			location_script,menus_script,spells_script,speech_script,tracking_script,
			newbie_script,titles_script,advance_script,trigger_script,ntrigger_script,
			wtrigger_script,necro_script,house_script,colors_script,spawn_script,
			html_script,metagm_script,polymorph_script,hard_items,custom_npc_script,
			custom_item_script,carve_script,msgboard_script,
			fishing_script, gump_script, NUM_SCRIPTS} ;

extern Script *i_scripts[NUM_SCRIPTS]; // array of script references
extern char n_scripts[NUM_SCRIPTS][512] ; // array of script filenames

extern unsigned long initialserversec ;
extern unsigned long initialservermill ;


/////////////////////////////////////////////
///             MAXCLIENT arrays          ///
/////////////////////////////////////////////
// maximum too cause maxclient = maximum +1

extern char firstpacket[MAXCLIENT+1];
extern char noweather[MAXCLIENT+1]; //LB
extern unsigned char LSD[MAXCLIENT];
extern unsigned char DRAGGED[MAXCLIENT];
extern unsigned char EVILDRAGG[MAXCLIENT];

// meta gm variables
extern int priv3a[MAXCLIENT]; // sorry, my stupidity, (Lord Binary). basically not necassairy. but no time to rewrite now.
extern int priv3b[MAXCLIENT]; // needed until priv3target rewritten
extern int priv3c[MAXCLIENT];
extern int priv3d[MAXCLIENT];
extern int priv3e[MAXCLIENT];
extern int priv3f[MAXCLIENT];
extern int priv3g[MAXCLIENT];
// end of meta gm variables

extern int newclient[MAXCLIENT];
extern unsigned char  buffer[MAXCLIENT][MAXBUFFER_REAL];
extern char  outbuffer[MAXCLIENT][MAXBUFFER_REAL];
extern int whomenudata [(MAXCLIENT)*10]; // LB, for improved whomenu, ( is important !!!
extern int client[MAXCLIENT];
extern short int walksequence[MAXCLIENT];
extern signed char addid5[MAXCLIENT];
extern int acctno[MAXCLIENT];
extern P_CHAR currchar[MAXCLIENT];
extern make_st itemmake[MAXCLIENT];
extern int tempint[MAXCLIENT];
extern unsigned char addid1[MAXCLIENT];
extern unsigned char addid2[MAXCLIENT];
extern unsigned char addid3[MAXCLIENT];
extern unsigned char addid4[MAXCLIENT];
extern unsigned char dyeall[MAXCLIENT];
extern int addx[MAXCLIENT];
extern int addy[MAXCLIENT];
extern int addx2[MAXCLIENT];
extern int addy2[MAXCLIENT];
extern signed char addz[MAXCLIENT];
extern int addmitem[MAXCLIENT];
extern char xtext[MAXCLIENT][31];
extern unsigned char perm[MAXCLIENT];
extern unsigned char cryptclient[MAXCLIENT];
extern unsigned char usedfree[MAXCLIENT];
extern int binlength[MAXIMUM+1];
extern int boutlength[MAXIMUM+1];
extern unsigned char clientDimension[MAXCLIENT]; // stores if the client is the new 3d or old 2d one
extern std::string SocketStrings[MAXCLIENT];

//extern int spattackValue[MAXCLIENT];
extern int clickx[MAXCLIENT];
extern int clicky[MAXCLIENT];
extern int currentSpellType[MAXCLIENT]; // 0=spellcast, 1=scrollcast, 2=wand cast
extern unsigned char targetok[MAXCLIENT];

//////////////////////////////////////////////
//              MAxBuffer                  //
/////////////////////////////////////////////

extern char tbuffer[MAXBUFFER_REAL];
extern char xoutbuffer[MAXBUFFER_REAL*3];

/////////////////////////////////////////////
///             maxaccount                ///
/////////////////////////////////////////////


/////////////////////////////////////////////
///             hasmax                   ///
/////////////////////////////////////////////


// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial

extern Container_cl stablesp, cmultisp, imultisp, contsp, ownsp, cspawnsp, spawnsp, glowsp, cownsp;
extern std::list<SERIAL> guilds;

/******************************************************/

///////////////////////////////////////////
///   and the rest                        /
///////////////////////////////////////////

extern int gatex[MAXGATES][2],gatey[MAXGATES][2];
extern signed char gatez[MAXGATES][2];
extern unitile_st xyblock[XYMAX];
extern gmpage_st gmpages[MAXPAGES];
extern gmpage_st counspages[MAXPAGES];
extern skill_st skill[SKILLS+1];
extern unsigned short int doorbase[DOORTYPES];
extern char skillname[SKILLS+1][20];
extern  std::multimap <int, tele_locations_st> tele_locations; // can't use a map here :(
extern title_st title[ALLSKILLS+1];
extern unsigned char *comm[CMAX];
extern int layers[MAXLAYERS];

extern creat_st creatures[2048]; //LB, stores the base-sound+sound flags of monsters, animals
extern location_st location[4000];
extern logout_st logout[1024];//Instalog
extern region_st region[256];
extern std::vector<spawnregion_st> spawnregion;
extern advance_st wpadvance[1000];
//extern spell_st spells[100];
extern char spellname[71][25];
extern unsigned int metagm[256][7]; // for meta gm script
extern int validEscortRegion[256];

extern int freecharmem[301]; //stores pointers to deleted char struct memory
extern int freeitemmem[501]; //stores pointers to deleted item struct memory
extern int npcshape[5]; //Stores the coords of the bouding shape for the NPC

extern jail_st jails[11];

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

extern char idname[256];
extern char script1[512];
extern char script2[512];
extern char script3[512];
extern char script4[512]; // added to use for newbie items
extern char fametitle[128];
extern char skilltitle[50];
extern char prowesstitle[50];
extern char *completetitle;
extern char gettokenstr[256];
extern char goldamountstr[10];
extern char defaultpriv1str[2];
extern char defaultpriv2str[2];
extern char temp[1024];
extern char temp2[1024];
extern char temp3[1024];
extern char temp4[1024];
extern char schei___[512];

extern char mapname[512], sidxname[512], statname[512], vername[512], tilename[512], multiname[512], midxname[512];
extern char saveintervalstr[4];
extern char scpfilename[32];//AntiChrist


////////////////////////////////
//  small packet-arrays
///////////////////////////////

extern unsigned char login04a[6];
extern unsigned char login04b[61];
extern unsigned char login04c[18];
extern unsigned char login04d[64];
extern unsigned char login03[12];
extern unsigned char wppause[3];
extern unsigned char w_anim[3];
extern unsigned char restart[3];
extern unsigned char goxyz[20];
extern unsigned char wearitem[16];
extern unsigned char talk[15];
extern unsigned char sysname[31];
extern unsigned char removeitem[6];
extern unsigned char gmprefix[10];
extern unsigned char gmmiddle[5];
extern unsigned char sfx[13];
extern unsigned char doact[15];
extern unsigned char bpitem[20];
extern unsigned char gump1[22];
extern unsigned char gump2[4];
extern unsigned char gump3[3];
extern unsigned char dyevat[10];
extern unsigned char updscroll[11];
extern unsigned char spc[2];
extern unsigned char bounce[3];
extern unsigned char extmove[18];
extern unsigned char particleSystem[49];

#ifdef __unix__
extern termios termstate ;
#endif

extern WPScriptManager *ScriptManager;
extern WPDefManager *DefManager;
//

#endif




