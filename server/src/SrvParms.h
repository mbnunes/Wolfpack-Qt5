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

#ifndef __Srv_Params
#define __Srv_Params

void loadserverdefaults(void);
void loadserverscript(void);
void loadclientsallowed(void);


void saveserverscript(void);
#define SrvParms (&server_data)	// in preparation of making sever_data a class.Use SrvParms->xxx instead of server_dat.xxx (Duke)


#define STABLOCKCACHESIZE 1000
#define NPCSPEED .3	// was .2  5 steps every second

#define TRACKINGRANGE 10        // tracking range is at least TRACKINGRANGE, additional distance is calculated by Skill,INT
#define TRACKINGTIMER 30        // tracking last at least TRACKINGTIMER seconds, additional time is calculated by Skill,INT, & DEX
#define TRACKINGDISPLAYTIME 30  // tracking will display above the character every TRACKINGDISPLAYTIME seconds

#define FISHINGTIMEBASE 10              // it takes at least FISHINGTIMEBASE sec to fish
#define FISHINGTIMER 5
#define DEFAULTJAILTIME 86400	// blackwind
#define ECLIPSETIMER .5

#define DECAYTIMER 300  // 5 minute decay rate
#define INVISTIMER 60 // invisibility spell lasts 1 minute
#define HUNGERRATE 6000 // 15 minutes
#define SKILLDELAY 5   // Skill usage delay     (5 seconds)
#define REGENRATE1 8 // Seconds to heal ONE hp
#define REGENRATE2 3 // Seconds to heal ONE stm
#define REGENRATE3 5 // Seconds to heal ONE mn
#define GATETIMER 30

struct server_st
{
	unsigned int packetsendstyle; // LB .. defines the way packets are send
	
	unsigned int eclipsemode;
	unsigned int eclipsetimer;
	unsigned int checkcharage ;
	unsigned int poisontimer;
	unsigned int joinmsg;
	unsigned int partmsg;
	unsigned int decaytimer;

 //taken from 6904t2(5/10/99) - AntiChrist
	unsigned int playercorpsedecaymultiplier;
	unsigned int lootdecayswithcorpse;

	unsigned int invisibiliytimer;
	unsigned int hungerrate;
	unsigned int skilldelay;
	unsigned int snoopdelay;
	unsigned int objectdelay;
	unsigned int bandagedelay;					// in seconds, on top of objectdelay
	unsigned int bandageincombat;				// 0=no, 1=yes; can use bandages while healer and/or patient are in combat ?
	unsigned int inactivitytimeout;			// seconds of inactivity until player will be disconnected (duke)
			// LB: if no packet received from client in that time period, disconnection ! (except idle packet)
			// thats necessary, cauz in case of client/OS crashes or IP changes on the fly players didnt get disconnected

	unsigned int hitpointrate;
	unsigned int staminarate;
	unsigned int manarate;
	unsigned int skilladvancemodifier;
	unsigned int statsadvancemodifier;
	unsigned int gatetimer;
	unsigned int minecheck;
	unsigned int showdeathanim;
	unsigned int combathitmessage;
	unsigned int monsters_vs_animals;
	unsigned int animals_attack_chance;
	unsigned int animals_guarded;
	unsigned int npc_base_fleeat;
	unsigned int npc_base_reattackat;
	unsigned int tamed_disappear; // Ripper
	unsigned int houseintown; // Ripper
	unsigned int shoprestock; // Ripper 
	unsigned char guardsactive;
	unsigned char bg_sounds;
	unsigned char commandPrefix; //Ripper

	char archivepath[256];
	int backup_save_ratio; // LB, each X'th save-intervall a backup is done if a path is given ...

	unsigned int maxabsorbtion; // Magius(CHE) --- Maximum Armour of the single pieces of armour (for player)!
	unsigned int maxnohabsorbtion; // Magius(CHE) --- Maximum Armour of the total armour (for NPC)!
	unsigned int npcdamage; // Magius(CHE) --- DAmage divided by this number if the attacked is an NPC!
	unsigned int sellbyname; // Magius(CHE)
	unsigned int sellmaxitem; // Magius(CHE)
	unsigned int skilllevel; // Magius(CHE)
	unsigned int trade_system; // Magius(CHE)
	unsigned int rank_system; // Magius(CHE)
	unsigned char hungerdamage;
	unsigned int hungerdamagerate;
	char armoraffectmana; // Should armor slow mana regen ?
		
	char announceworldsaves;
		
	unsigned char auto_a_create; //Create accounts when people log in with unknown name/password
	unsigned int  auto_a_reload; //Auto reload accounts if file was touched, this stores the time interval in minutes.
		  
	unsigned int rogue;
	signed int maxstealthsteps;//AntiChrist - max number of steps allowed with stealth skill at 100.0
	unsigned int runningstaminasteps;//AntiChrist - max number of steps allowed with stealth skill at 100.0
	unsigned short int quittime;//Instalog
	int html;//HTML
	float boatspeed;

	unsigned int savespawns;


	float stablingfee;

	int attackstamina;	// AntiChrist (6) - for ATTACKSTAMINA

	unsigned int skillcap;//LB skill cap
	unsigned int statcap;//AntiChrist stat cap

	char specialbanktrigger[50]; //special bank trigger - AntiChrist
	int usespecialbank; //special bank - AntiChrist

	short CheckBank; // Vendors check bank for gold- Ripper
	unsigned int VendorGreet; // shop keepers greet - Ripper
	unsigned int BadNpcsRed; // bad npcs red=1 or grey=0 - Ripper
	unsigned int slotamount; // gold amount for slots Ripper

	int cutscrollreq;//AntiChrist - cut skill requirements for scrolls

	int persecute;//AntiChrist - persecute thing

	char errors_to_console; // LB
	char gm_log;
	char speech_log;
	char pvp_log;
	char server_log;

	unsigned long int housedecay_secs;
	unsigned int ra_port;
	bool EnableRA;

	unsigned char showCVCS; // Show client verification stuff at login ? (blackwind) 
	unsigned int default_jail_time; // Default jail time (86400) 
	unsigned char attack_distance; // Attack distance bugfix (blackwind) 


	float goldweight;//AntiChrist - configurable gold weight
	char msgboardpath[256];   // Dupois - Added April 4, 1999 for msgboard.cpp - path to store BB* files (ie C:\POSTS\)
	int  msgpostaccess;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to post messages (0=GM only, 1 = everyone)
	int  msgpostremove;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to remove post messages (0=GM only, 1 = everyone)
	int  msgretention;        // Dupois - Added April 4, 1999 for msgboard.cpp - how long to keep posts before deleting them automatically (in days - default 30)
	int  escortactive;        // Dupois - Added April 4, 1999 for escort quests - are escort quests created
	int  escortinitexpire;    // Dupois - Added April 4, 1999 for escort quests - time to expire after spawning NPC in secs(0=never)
	int  escortactiveexpire;  // Dupois - Added April 4, 1999 for escort quests - time to expire after quest has been accepted by player in secs(0=never)
	int  escortdoneexpire;    // Dupois - Added April 4, 1999 for escort quests - time to expire after quest has been completed by player in secs(0=never)
	int  bountysactive;       // Dupois - Added July 18, 2000 for bounty quests - whether bounties are active (1-default) or inactive (0)
	int  bountysexpire;       // Dupois - Added July 18, 2000 for bounty quests - number of days when an unclaimed bounty expires and is deleted (0=never-default)
};


#endif
