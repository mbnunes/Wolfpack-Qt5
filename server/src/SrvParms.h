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

//##ModelId=3C5D92CD0335
struct server_st
{
	//##ModelId=3C5D92CD037B
	unsigned int packetsendstyle; // LB .. defines the way packets are send
	
	//##ModelId=3C5D92CD0385
	unsigned int eclipsemode;
	//##ModelId=3C5D92CD0399
	unsigned int eclipsetimer;
	//##ModelId=3C5D92CD03AD
	unsigned int checkcharage ;
	//##ModelId=3C5D92CD03C1
	unsigned int poisontimer;
	//##ModelId=3C5D92CD03D5
	unsigned int joinmsg;
	//##ModelId=3C5D92CD03DF
	unsigned int partmsg;
	//##ModelId=3C5D92CE000B
	unsigned int decaytimer;

 //taken from 6904t2(5/10/99) - AntiChrist
	//##ModelId=3C5D92CE001F
	unsigned int playercorpsedecaymultiplier;
	//##ModelId=3C5D92CE0033
	unsigned int lootdecayswithcorpse;

	//##ModelId=3C5D92CE0047
	unsigned int invisibiliytimer;
	//##ModelId=3C5D92CE005C
	unsigned int hungerrate;
	//##ModelId=3C5D92CE0066
	unsigned int skilldelay;
	//##ModelId=3C5D92CE007A
	unsigned int snoopdelay;
	//##ModelId=3C5D92CE008E
	unsigned int objectdelay;
	//##ModelId=3C5D92CE00A2
	unsigned int bandagedelay;					// in seconds, on top of objectdelay
	//##ModelId=3C5D92CE00B6
	unsigned int bandageincombat;				// 0=no, 1=yes; can use bandages while healer and/or patient are in combat ?
	//##ModelId=3C5D92CE00C0
	unsigned int inactivitytimeout;			// seconds of inactivity until player will be disconnected (duke)
			// LB: if no packet received from client in that time period, disconnection ! (except idle packet)
			// thats necessary, cauz in case of client/OS crashes or IP changes on the fly players didnt get disconnected

	//##ModelId=3C5D92CE00DE
	unsigned int hitpointrate;
	//##ModelId=3C5D92CE00F2
	unsigned int staminarate;
	//##ModelId=3C5D92CE0106
	unsigned int manarate;
	//##ModelId=3C5D92CE0110
	unsigned int skilladvancemodifier;
	//##ModelId=3C5D92CE0142
	unsigned int statsadvancemodifier;
	//##ModelId=3C5D92CE014C
	unsigned int gatetimer;
	//##ModelId=3C5D92CE0160
	unsigned int minecheck;
	//##ModelId=3C5D92CE0174
	unsigned int showdeathanim;
	//##ModelId=3C5D92CE0188
	unsigned int combathitmessage;
	//##ModelId=3C5D92CE0192
	unsigned int monsters_vs_animals;
	//##ModelId=3C5D92CE01A6
	unsigned int animals_attack_chance;
	//##ModelId=3C5D92CE01BA
	unsigned int animals_guarded;
	//##ModelId=3C5D92CE01CE
	unsigned int npc_base_fleeat;
	//##ModelId=3C5D92CE01E2
	unsigned int npc_base_reattackat;
	//##ModelId=3C5D92CE01F6
	unsigned int tamed_disappear; // Ripper
	//##ModelId=3C5D92CE0200
	unsigned int houseintown; // Ripper
	//##ModelId=3C5D92CE0214
	unsigned int shoprestock; // Ripper 
	//##ModelId=3C5D92CE0228
	unsigned char guardsactive;
	//##ModelId=3C5D92CE023C
	unsigned char bg_sounds;
	//##ModelId=3C5D92CE0250
	unsigned char commandPrefix; //Ripper

	//##ModelId=3C5D92CE0264
	char archivepath[256];
	//##ModelId=3C5D92CE0278
	int backup_save_ratio; // LB, each X'th save-intervall a backup is done if a path is given ...

	//##ModelId=3C5D92CE0282
	unsigned int maxabsorbtion; // Magius(CHE) --- Maximum Armour of the single pieces of armour (for player)!
	//##ModelId=3C5D92CE0296
	unsigned int maxnohabsorbtion; // Magius(CHE) --- Maximum Armour of the total armour (for NPC)!
	//##ModelId=3C5D92CE02D2
	unsigned int npcdamage; // Magius(CHE) --- DAmage divided by this number if the attacked is an NPC!
	//##ModelId=3C5D92CE02E6
	unsigned int sellbyname; // Magius(CHE)
	//##ModelId=3C5D92CE02FA
	unsigned int sellmaxitem; // Magius(CHE)
	//##ModelId=3C5D92CE0304
	unsigned int skilllevel; // Magius(CHE)
	//##ModelId=3C5D92CE0319
	unsigned int trade_system; // Magius(CHE)
	//##ModelId=3C5D92CE032D
	unsigned int rank_system; // Magius(CHE)
	//##ModelId=3C5D92CE0341
	unsigned char hungerdamage;
	//##ModelId=3C5D92CE0355
	unsigned int hungerdamagerate;
	//##ModelId=3C5D92CE035F
	char armoraffectmana; // Should armor slow mana regen ?
		
	//##ModelId=3C5D92CE0373
	char announceworldsaves;
		
	//##ModelId=3C5D92CE0387
	unsigned char auto_a_create; //Create accounts when people log in with unknown name/password
	//##ModelId=3C5D92CE0391
	unsigned int  auto_a_reload; //Auto reload accounts if file was touched, this stores the time interval in minutes.
		  
	//##ModelId=3C5D92CE03A5
	unsigned int rogue;
	//##ModelId=3C5D92CE03B9
	signed int maxstealthsteps;//AntiChrist - max number of steps allowed with stealth skill at 100.0
	//##ModelId=3C5D92CE03CD
	unsigned int runningstaminasteps;//AntiChrist - max number of steps allowed with stealth skill at 100.0
	//##ModelId=3C5D92CE03E1
	unsigned short int quittime;//Instalog
	//##ModelId=3C5D92CF0003
	int html;//HTML
	//##ModelId=3C5D92CF0017
	float boatspeed;

	//##ModelId=3C5D92CF002B
	unsigned int savespawns;


	//##ModelId=3C5D92CF003F
	float stablingfee;

	//##ModelId=3C5D92CF0053
	int attackstamina;	// AntiChrist (6) - for ATTACKSTAMINA

	//##ModelId=3C5D92CF005D
	unsigned int skillcap;//LB skill cap
	//##ModelId=3C5D92CF0071
	unsigned int statcap;//AntiChrist stat cap

	//##ModelId=3C5D92CF0085
	char specialbanktrigger[50]; //special bank trigger - AntiChrist
	//##ModelId=3C5D92CF0099
	int usespecialbank; //special bank - AntiChrist

	//##ModelId=3C5D92CF00B7
	short CheckBank; // Vendors check bank for gold- Ripper
	//##ModelId=3C5D92CF00CB
	unsigned int VendorGreet; // shop keepers greet - Ripper
	//##ModelId=3C5D92CF00D5
	unsigned int BadNpcsRed; // bad npcs red=1 or grey=0 - Ripper
	//##ModelId=3C5D92CF00E9
	unsigned int slotamount; // gold amount for slots Ripper

	//##ModelId=3C5D92CF00FD
	int cutscrollreq;//AntiChrist - cut skill requirements for scrolls

	//##ModelId=3C5D92CF0111
	int persecute;//AntiChrist - persecute thing

	//##ModelId=3C5D92CF0139
	char errors_to_console; // LB
	//##ModelId=3C5D92CF014D
	char gm_log;
	//##ModelId=3C5D92CF0157
	char speech_log;
	//##ModelId=3C5D92CF016B
	char pvp_log;
	//##ModelId=3C5D92CF017F
	char server_log;

	//##ModelId=3C5D92CF0193
	unsigned long int housedecay_secs;
	//##ModelId=3C5D92CF01A7
	unsigned int ra_port;
	//##ModelId=3C5D92CF01B1
	bool EnableRA;

	//##ModelId=3C5D92CF01C5
	unsigned char showCVCS; // Show client verification stuff at login ? (blackwind) 
	//##ModelId=3C5D92CF01D9
	unsigned int default_jail_time; // Default jail time (86400) 
	//##ModelId=3C5D92CF020C
	unsigned char attack_distance; // Attack distance bugfix (blackwind) 


	//##ModelId=3C5D92CF0220
	float goldweight;//AntiChrist - configurable gold weight
	//##ModelId=3C5D92CF0234
	char msgboardpath[256];   // Dupois - Added April 4, 1999 for msgboard.cpp - path to store BB* files (ie C:\POSTS\)
	//##ModelId=3C5D92CF0248
	int  msgpostaccess;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to post messages (0=GM only, 1 = everyone)
	//##ModelId=3C5D92CF025C
	int  msgpostremove;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to remove post messages (0=GM only, 1 = everyone)
	//##ModelId=3C5D92CF0270
	int  msgretention;        // Dupois - Added April 4, 1999 for msgboard.cpp - how long to keep posts before deleting them automatically (in days - default 30)
	//##ModelId=3C5D92CF027A
	int  escortactive;        // Dupois - Added April 4, 1999 for escort quests - are escort quests created
	//##ModelId=3C5D92CF0298
	int  escortinitexpire;    // Dupois - Added April 4, 1999 for escort quests - time to expire after spawning NPC in secs(0=never)
	//##ModelId=3C5D92CF02A2
	int  escortactiveexpire;  // Dupois - Added April 4, 1999 for escort quests - time to expire after quest has been accepted by player in secs(0=never)
	//##ModelId=3C5D92CF02B6
	int  escortdoneexpire;    // Dupois - Added April 4, 1999 for escort quests - time to expire after quest has been completed by player in secs(0=never)
	//##ModelId=3C5D92CF02CA
	int  bountysactive;       // Dupois - Added July 18, 2000 for bounty quests - whether bounties are active (1-default) or inactive (0)
	//##ModelId=3C5D92CF02D4
	int  bountysexpire;       // Dupois - Added July 18, 2000 for bounty quests - number of days when an unclaimed bounty expires and is deleted (0=never-default)
};


#endif
