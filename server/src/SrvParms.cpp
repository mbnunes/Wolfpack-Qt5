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

#include "wolfpack.h"
#include "verinfo.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "SrvParms.cpp"


//#define REPAIRWORLDTIMER 900 // Ripper...15 mins default.


// important: loaddefault has to be called before. 
// currently it is. makes no sense to change that too.

void loadclientsallowed(void)
{
   int num_c=0, s=0, loopexit=0;
   vector<string> cls;    

   do
   {
		readFullLine();

		if(!(strcmp((char*)script1,"SERVER_DEFAULT"))) { s=1; break; }
		else if(!(strcmp((char*)script1,"ALL"))) { s=2; break; }
		else if (strlen(script1)>3) 
		{ 		
			num_c++;
			cls.push_back(script1);
		}
   }
   while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );  
   
   if (num_c==0 && s==0) s=1; // no entry == pick server_default

   switch(s)
   {
	   case 0: 
               clientsAllowed.clear();  // delete the entry from loaddefaults
			   copy(cls.begin(), cls.end(), back_inserter(clientsAllowed) ); // copy temp vector to perm vector
		       break;

	   case 1: 
		       break; // already in vector from loaddefaults

	   case 2: 
		       clientsAllowed.clear();    // delete server_defualt entry from loaddefaults
			   clientsAllowed.push_back("ALL"); // now add the correct one
		       break;
   }

 
}

void loadserverdefaults(void)
{
    clientsAllowed.push_back("SERVER_DEFAULT");
	strcpy(server_data.specialbanktrigger,"WARE"); //AntiChrist - Special Bank word trigger
	server_data.usespecialbank=0;	//AntiChrist - 1= Special Bank enabled
	server_data.goldweight=0.005f;	//what a goldpiece weighs this is in hundreths of a stone! AntiChrist
	server_data.poisontimer=180; // lb
	//server_data.repairworldtimer=REPAIRWORLDTIMER; //Ripper

	server_data.decaytimer=DECAYTIMER;
	// Should we check character age for delete?
	server_data.checkcharage = 0 ;

	server_data.skillcap=999999;	// default=no cap
	server_data.statcap=999;		// default=no cap

	server_data.packetsendstyle=PSS_UOX3;

	//taken from 6904t2(5/10/99) - AntiChrist
	server_data.playercorpsedecaymultiplier=3;	// Player's corpse decays 3x the normal decay rate
	server_data.lootdecayswithcorpse=1;			// JM - 0 Loot disappears with corpse, 1 loot gets left when corpse decays
	
	server_data.auto_a_create = 0;                     // for auto accounts
	server_data.auto_a_reload = 0;
	
	server_data.invisibiliytimer=INVISTIMER;
	server_data.hungerrate=HUNGERRATE;
	server_data.hungerdamagerate=10;			// every 10 seconds
	server_data.snoopdelay=7;
	server_data.hungerdamage=2;
	server_data.skilldelay=SKILLDELAY;
	server_data.objectdelay=1;					// in seconds
	server_data.bandagedelay=6;					// in seconds
	server_data.bandageincombat=1;				// 0=no, 1=yes; can use bandages while healer and/or patient are in combat ?
	server_data.inactivitytimeout=300;			// seconds of inactivity until player will be disconnected
	server_data.hitpointrate=REGENRATE1;
	server_data.staminarate=REGENRATE2;
	server_data.manarate=REGENRATE3;
	server_data.skilladvancemodifier=1000;		// Default Modulo val for skill advance
    server_data.statsadvancemodifier=500;		// Default Modulo val for stats advance
	server_data.gatetimer=GATETIMER;
	server_data.minecheck=2;
	server_data.showdeathanim=1;				// 1 = yes/true
	server_data.combathitmessage=1;
	server_data.attackstamina=-2;				// AntiChrist - attacker looses stamina when hits
	server_data.monsters_vs_animals=0;			// By default monsters won't attack animals;
	server_data.animals_attack_chance=15;		// a 15% chance
	server_data.animals_guarded=0;				// By default players can attack animals without summoning guards
	server_data.npc_base_fleeat=NPC_BASE_FLEEAT;
	server_data.npc_base_reattackat=NPC_BASE_REATTACKAT;
	server_data.maxabsorbtion=20;				// 20 Arm (single armour piece) -- Magius(CHE)
	server_data.maxnohabsorbtion=100;			// 100 Arm (total armour) -- Magius(CHE)
	server_data.sellbyname=1;		// Values= 0(Disabled) or 1(Enabled) - The NPC Vendors buy from you if your item has the same name of his item! --- Magius(CHE)
	server_data.skilllevel=5;		// Range from 1 to 10 - This value if the difficulty to create an item using a make command: 1 is easy, 10 is difficult! - Magius(CHE)
	server_data.sellmaxitem=5;		// Set maximum amount of items that one player can sell at one time (5 is standard OSI) --- Magius(CHE)
	server_data.npcdamage=2;
	server_data.rank_system=1;		// Rank system to make various type of a single intem based on the creator's skill! - Magius(CHE)
	server_data.guardsactive=1;
	server_data.errors_to_console=0;
	server_data.bg_sounds=2;
	server_data.announceworldsaves=1;
	server_data.joinmsg=1;
	server_data.partmsg=1;
	server_data.stablingfee=0.25;
	server_data.savespawns=1;

	server_data.gm_log=0;
	server_data.pvp_log=0;
	server_data.speech_log=0;
	server_data.server_log=0;

	server_data.rogue=1;
	server_data.quittime=300;//Instalog

	server_data.msgboardpath[0] = 0;	   // Dupois - Added Dec 20, 1999 for message boards (current dir)
	server_data.backup_save_ratio=1; //LB
	
	server_data.msgpostaccess=0;           // Dupois - Added Dec 20, 1999 for message boards (GM only)
	server_data.msgpostremove=0;           // Dupois - Added Dec 20, 1999 for message boards (GM only)
	server_data.msgretention=30;           // Dupois - Added Dec 20, 1999 for message boards (30 Days)
	server_data.escortactive=1;            // Dupois - Added Dec 20, 1999 for message boards (Active)
	server_data.escortinitexpire=86400;    // Dupois - Added Dec 20, 1999 for message boards (24 hours)
	server_data.escortactiveexpire=1800;   // Dupois - Added Dec 20, 1999 for message boards (1/2 Hour)
	server_data.escortdoneexpire=1800;     // Dupois - Added Dec 20, 1999 for message boards (1/2 Hour)
		
	server_data.bountysactive=1;           // Dupois - Added July 18, 2000 for bountys (1-Active)
	server_data.bountysexpire=0;           // Dupois - Added July 18, 2000 for bountys (0-Never - in days)
	
	
	server_data.maxstealthsteps=10;// AntiChrist - stealth=100.0 --> 10 steps allowed
	server_data.runningstaminasteps=15;// AntiChrist - how often ( in steps ) stamina decreases while running
	
	server_data.boatspeed=0.75;//Boats

	server_data.cutscrollreq=1;//AntiChrist

	server_data.persecute=1;//AntiChrist

	server_data.html=-1;//HTML

	server_data.tamed_disappear=1; // Ripper
	server_data.houseintown=0; // Ripper
	server_data.shoprestock=1; //Ripper
	server_data.commandPrefix = '\''; //Ripper

	server_data.CheckBank = 2000; // Ripper..2000 default
	server_data.VendorGreet = 1; // Ripper..do greet by default.

	server_data.housedecay_secs=604800; // LB, one week

    server_data.eclipsemode=0; // Light to dark..
	server_data.ra_port = 2594;
	server_data.EnableRA = false;

	server_data.showCVCS = 1; // blackwind 
	server_data.default_jail_time=86400; // 
	server_data.attack_distance=13; // 


	cwmWorldState->SetLoopSaveAmt(-1);
		
	speed.srtime=30;
	speed.itemtime=(float)1.5;
	speed.npctime=(float)1.0; // lb, why -1 zippy ??????
    speed.npcaitime=(float)1.2;
	speed.tamednpctime=(float)0.9;//AntiChrist
	speed.npcfollowtime=(float)0.5; //Ripper
	speed.nice=2;
	speed.checkmem=3000000;
	
	resource.logs=3;
	resource.logtime=600;
	resource.logarea=10;
	resource.lograte=3;
	resource.logtrigger=0;
	resource.logstamina=-5;
	resource.ore=10;
	resource.oretime=600;
	resource.orerate=3;
	resource.orearea=10;
	resource.miningtrigger=0;
	resource.miningstamina=-5;
	//REPSYS
	repsys.crimtime=120;
	repsys.maxkills=4;
	repsys.murderdecay=420;
	//RepSys ---^
	tracking_data.baserange=TRACKINGRANGE;
	tracking_data.maxtargets=MAXTRACKINGTARGETS;
	tracking_data.basetimer=TRACKINGTIMER;
	tracking_data.redisplaytime=TRACKINGDISPLAYTIME;
	begging_data.range=3;
	begging_data.timer=300;
	strcpy(begging_data.text[0],"Could thou spare a few coins?");
	strcpy(begging_data.text[1],"Hey buddy can you spare some gold?");
	strcpy(begging_data.text[2],"I have a family to feed, think of the children.");
	fishing_data.basetime=FISHINGTIMEBASE;
	fishing_data.randomtime=FISHINGTIMER;
	spiritspeak_data.spiritspeaktimer=SPIRITSPEAKTIMER;
}

void loadspeed()//Lag Fix -- Zippy -- NEW FUNCTION
{
	int loopexit=0;
	do
	{
		readw2();
		
        if(!(strcmp((char*)script1,"NICE"))) speed.nice=str2num(script2);
		else if(!(strcmp((char*)script1,"CHECK_ITEMS"))) speed.itemtime=atof((char*)script2);
		else if(!(strcmp((char*)script1,"CHECK_SPAWNREGIONS"))) speed.srtime=str2num(script2);
		else if(!(strcmp((char*)script1,"CHECK_NPCS"))) speed.npctime=atof((char*)script2);
        else if(!(strcmp((char*)script1,"CHECK_NPCAI"))) speed.npcaitime=atof((char*)script2);
        else if(!(strcmp((char*)script1,"CHECK_TAMEDNPCS"))) speed.tamednpctime=atof((char*)script2);//AntiChrist
		else if(!(strcmp((char*)script1,"CHECK_NPCFOLLOW"))) speed.npcfollowtime=atof((char*)script2);//Ripper
		//else if(!(strcmp(script1,"LORDB_LAG_FIX"))) server_data.lordblagfix=str2num(script2);
		else if(!(strcmp((char*)script1,"CHECKMEM"))) speed.checkmem=str2num(script2);
		else if(!(strcmp((char*)script1,"CACHE_MUL"))) Map->Cache = atoi((char*)script2);	
	}
	while (  (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}


void loadcombat() // By Magius(CHE)
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"COMBAT_HIT_MESSAGE"))) server_data.combathitmessage=str2num(script2);
		else if(!(strcmp((char*)script1,"MAX_ABSORBTION"))) server_data.maxabsorbtion=str2num(script2); //MAgius(CHE)
		else if(!(strcmp((char*)script1,"MAX_NON_HUMAN_ABSORBTION"))) server_data.maxnohabsorbtion=str2num(script2); //MAgius(CHE) (2)
		else if(!(strcmp((char*)script1,"NPC_DAMAGE_RATE"))) server_data.npcdamage=str2num(script2); //MAgius(CHE) (3)
		else if(!(strcmp((char*)script1,"MONSTERS_VS_ANIMALS"))) server_data.monsters_vs_animals=str2num(script2);
		else if(!(strcmp((char*)script1,"ANIMALS_ATTACK_CHANCE"))) server_data.animals_attack_chance=str2num(script2);
		else if(!(strcmp((char*)script1,"ANIMALS_GUARDED"))) server_data.animals_guarded=str2num(script2);
		else if(!(strcmp((char*)script1,"NPC_BASE_FLEEAT"))) server_data.npc_base_fleeat=str2num(script2);
		else if(!(strcmp((char*)script1,"NPC_BASE_REATTACKAT"))) server_data.npc_base_reattackat=str2num(script2);
		else if(!(strcmp((char*)script1,"ATTACKSTAMINA"))) server_data.attackstamina=str2num(script2); // antichrist (6) - for ATTACKSTAMINA
		else if(!(strcmp((char*)script1,"ATTACK_DISTANCE"))) server_data.attack_distance=str2num(script2); // blackwind 
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadregenerate() // by Magius(CHE)
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"HITPOINTS_REGENRATE"))) server_data.hitpointrate=str2num(script2);
		else if(!(strcmp((char*)script1,"STAMINA_REGENRATE"))) server_data.staminarate=str2num(script2);
		else if(!(strcmp((char*)script1,"MANA_REGENRATE"))) server_data.manarate=str2num(script2);
		else if(!(strcmp((char*)script1,"ARMOR_AFFECT_MANA_REGEN"))) server_data.armoraffectmana=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadhunger() // by Magius(CHE)
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"HUNGERRATE"))) server_data.hungerrate=str2num(script2);
		else if(!(strcmp((char*)script1,"HUNGER_DAMAGE"))) server_data.hungerdamage=str2num(script2);
		else if(!(strcmp((char*)script1,"HUNGER_DAMAGE_RATE"))) server_data.hungerdamagerate=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadvendor() // by Magius(CHE)
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"SELLBYNAME"))) {
			server_data.sellbyname=str2num(script2);
			if (server_data.sellbyname!=0) server_data.sellbyname=1;  //MAgius(CHE)
		}
		else if(!(strcmp((char*)script1,"SELLMAXITEM"))) {
			server_data.sellmaxitem=str2num(script2); 
		}
		else if(!(strcmp((char*)script1,"TRADESYSTEM"))) {
			server_data.trade_system=str2num(script2); //Magius(CHE)
			if (server_data.trade_system!=0) server_data.trade_system=1; // Magiu(CHE)
		}
		else if(!(strcmp((char*)script1,"RANKSYSTEM"))) {
			server_data.rank_system=str2num(script2);
			if (server_data.rank_system!=0) server_data.rank_system=1; //Magius(CHE)
		}
		else if(!(strcmp( (char*)script1,"CHECKBANK"))) {
            server_data.CheckBank=str2num(script2);
		}
		else if(!(strcmp( (char*)script1,"VENDORGREET"))) {
            server_data.VendorGreet=str2num(script2);
		}
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadresources()
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"MINECHECK"))) SrvParms->minecheck=str2num(script2); // Moved by Magius(CHE)
		else if(!(strcmp((char*)script1,"LOGS_PER_AREA"))) resource.logs=str2num(script2);
		else if(!(strcmp((char*)script1,"LOG_RESPAWN_TIME"))) resource.logtime=str2num(script2);
		else if(!(strcmp((char*)script1,"LOG_RESPAWN_RATE"))) resource.lograte=str2num(script2);
		else if(!(strcmp((char*)script1,"LOG_RESPAWN_AREA"))) resource.logarea=str2num(script2);
		else if(!(strcmp((char*)script1,"LOG_TRIGGER"))) resource.logtrigger=str2num(script2);
		else if(!(strcmp((char*)script1,"LOG_STAMINA"))) resource.logstamina=str2num(script2);
		else if(!(strcmp((char*)script1,"ORE_PER_AREA"))) resource.ore=str2num(script2);
		else if(!(strcmp((char*)script1,"ORE_RESPAWN_TIME"))) resource.oretime=str2num(script2);
		else if(!(strcmp((char*)script1,"ORE_RESPAWN_RATE"))) resource.orerate=str2num(script2);
		else if(!(strcmp((char*)script1,"ORE_RESPAWN_AREA"))) resource.logarea=str2num(script2);
		else if(!(strcmp((char*)script1,"ORE_TRIGGER"))) resource.miningtrigger=str2num(script2);
		else if(!(strcmp((char*)script1,"ORE_STAMINA"))) resource.miningstamina=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadrepsys() //Repsys
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"MURDER_DECAY"))) repsys.murderdecay=str2num(script2);
		else if(!(strcmp((char*)script1,"MAXKILLS"))) repsys.maxkills=str2num(script2);
		else if(!(strcmp((char*)script1,"CRIMINAL_TIME"))) repsys.crimtime=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );

	if (!repsys.murderdecay) repsys.murderdecay=420;
	if (!repsys.maxkills) repsys.maxkills=4;
	if (!repsys.crimtime) repsys.crimtime=120;
}

void loadserver()
{
	// Server.scp completly reorganized by Magius(CHE) 3 September 1999
	int loopexit=0;
	do
	{
		readw2();
	

		if(!(strcmp((char*)script1,"SKILLCAP"))) server_data.skillcap=str2num(script2);
		else if(!(strcmp((char*)script1,"STATCAP"))) server_data.statcap=str2num(script2);

		else if(!(strcmp((char*)script1,"SKILLADVANCEMODIFIER")))
		{
            int mod=str2num(script2);
            server_data.skilladvancemodifier=(mod>=1 ? mod : 1);    // must be nonzero && positive
		}
        else if(!(strcmp((char*)script1,"STATSADVANCEMODIFIER")))
		{
            int mod=str2num(script2);
            server_data.statsadvancemodifier=(mod>=1 ? mod : 1);    // must be nonzero && positive
		}

		else if(!(strcmp((char*)script1,"USESPECIALBANK"))) server_data.usespecialbank=str2num(script2); //AntiChrist - Special Bank
		else if(!(strcmp((char*)script1,"SPECIALBANKTRIGGER")))  //AntiChrist - Special Bank
		{
			strcpy(server_data.specialbanktrigger,script2);
			for(int unsigned i = 0; i < strlen(server_data.specialbanktrigger); i++)
				server_data.specialbanktrigger[i] = toupper(server_data.specialbanktrigger[i]);
		}

		//AntiChrist
		else if(!(strcmp((char*)script1,"GOLDWEIGHT"))) server_data.goldweight=atof((char*)script2);

		else if(!(strcmp((char*)script1,"DECAYTIMER"))) server_data.decaytimer=str2num(script2);
		
		//taken from 6904t2(5/10/99) - AntiChrist
		else if(!(strcmp((char*)script1,"PLAYERCORPSEDECAYMULTIPLIER"))) server_data.playercorpsedecaymultiplier=str2num(script2);//JM
		else if(!(strcmp((char*)script1,"LOOTDECAYSWITHCORPSE"))) server_data.lootdecayswithcorpse=str2num(script2);

		else if(!(strcmp((char*)script1,"INVISTIMER"))) server_data.invisibiliytimer=str2num(script2);
		else if(!(strcmp((char*)script1,"SKILLDELAY"))) server_data.skilldelay=str2num(script2);
		else if(!(strcmp((char*)script1,"SKILLLEVEL"))) {
			server_data.skilllevel=str2num(script2); //MAgius(CHE)
			if (server_data.skilllevel<1 || server_data.skilllevel>10) server_data.skilllevel=5;
		}
		else if(!(strcmp((char*)script1,"OBJECTDELAY"))) server_data.objectdelay=str2num(script2);
		else if(!(strcmp((char*)script1,"BANDAGEDELAY"))) server_data.bandagedelay=str2num(script2);
		else if(!(strcmp((char*)script1,"BANDAGEINCOMBAT"))) server_data.bandageincombat=str2num(script2);
		else if(!(strcmp((char*)script1,"INACTIVITYTIMEOUT"))) server_data.inactivitytimeout=str2num(script2);
		else if(!(strcmp((char*)script1,"GATETIMER"))) server_data.gatetimer=str2num(script2);
		else if(!(strcmp((char*)script1,"SHOWDEATHANIM"))) server_data.showdeathanim=str2num(script2);
		else if(!(strcmp((char*)script1,"GUARDSACTIVE"))) server_data.guardsactive=str2num(script2);	
		else if(!(strcmp((char*)script1,"ANNOUNCE_WORLDSAVES"))) server_data.announceworldsaves=str2num(script2);	
		else if(!(strcmp((char*)script1,"BG_SOUNDS"))) server_data.bg_sounds=str2num(script2);
		else if(!(strcmp((char*)script1,"ARCHIVEPATH"))) strcpy(server_data.archivepath,(char*)script2);
		else if(!(strcmp((char*)script1,"BACKUP_SAVE_RATIO"))) { //LB
			server_data.backup_save_ratio=str2num(script2);
			if (server_data.backup_save_ratio<1) server_data.backup_save_ratio=1;
		}
        else if(!(strcmp((char*)script1,"POISONTIMER"))) server_data.poisontimer=str2num(script2); // lb
		//if(!(strcmp(script1,"REPAIRWORLDTIMER"))) server_data.repairworldtimer=str2num(script2); //Ripper
		else if(!(strcmp((char*)script1,"JOINMSG"))) server_data.joinmsg=str2num(script2);
		else if(!(strcmp((char*)script1,"PARTMSG"))) server_data.partmsg=str2num(script2);

		else if(!(strcmp((char*)script1,"SERVER_LOG"))) server_data.server_log=str2num(script2);
		else if(!(strcmp((char*)script1,"SPEECH_LOG"))) server_data.speech_log=str2num(script2);
		else if(!(strcmp((char*)script1,"PVP_LOG"))) server_data.pvp_log=str2num(script2);
		else if(!(strcmp((char*)script1,"GM_LOG"))) server_data.gm_log=str2num(script2);		

		else if(!(strcmp((char*)script1,"SAVESPAWNREGIONS"))) server_data.savespawns=str2num(script2);	


		else if(!(strcmp((char*)script1,"ROGUE"))) server_data.rogue = str2num(script2);
		else if(!(strcmp((char*)script1,"CHAR_TIME_OUT"))) server_data.quittime = str2num(script2);//Instalog
		else if(!(strcmp((char*)script1,"MAXSTEALTHSTEPS"))) server_data.maxstealthsteps = str2num(script2);//AntiChrist
		else if(!(strcmp((char*)script1,"RUNNINGSTAMINASTEPS"))) server_data.runningstaminasteps=str2num(script2);//AntiChrist
        else if(!(strcmp((char*)script1,"BOAT_SPEED"))) server_data.boatspeed = atof((char*)script2);//Boats
		else if(!(strcmp((char*)script1,"STABLING_FEE"))) server_data.stablingfee = atof((char*)script2);//Boats

		else if(!(strcmp((char*)script1,"HTML"))) server_data.html=str2num(script2);//HTML
        else if(!(strcmp((char*)script1,"CUT_SCROLL_REQUIREMENTS"))) server_data.cutscrollreq=str2num(script2);//AntiChrist
        else if(!(strcmp((char*)script1,"PERSECUTION"))) server_data.persecute=str2num(script2);//AntiChrist
	
		else if( !strcmp( (char*)script1, "AUTO_CREATE_ACCTS" ) ) server_data.auto_a_create = str2num( script2 );
		else if( !strcmp( (char*)script1, "CHECKCHARAGE" ) )  server_data.checkcharage = str2num( script2 );
		else if( !strcmp( (char*)script1, "AUTO_RELOAD_ACCTS" ) ) server_data.auto_a_reload = str2num( script2 );

		else if(!(strcmp((char*)script1,"MSGBOARDPATH"))) strcpy(server_data.msgboardpath,(char*)script2);               // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"MSGPOSTACCESS"))) server_data.msgpostaccess=str2num(script2);            // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"MSGPOSTREMOVE"))) server_data.msgpostremove=str2num(script2);            // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"MSGRETENTION"))) server_data.msgretention=str2num(script2);              // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"ESCORTACTIVE"))) server_data.escortactive=str2num(script2);              // Dupois - Added Dec 20, 1999 for escorts
		else if(!(strcmp((char*)script1,"ESCORTINITEXPIRE"))) server_data.escortinitexpire=str2num(script2);      // Dupois - Added Dec 20, 1999 for escorts
		else if(!(strcmp((char*)script1,"ESCORTACTIVEEXPIRE"))) server_data.escortactiveexpire=str2num(script2);  // Dupois - Added Dec 20, 1999 for escorts
		else if(!(strcmp((char*)script1,"ESCORTDONEEXPIRE"))) server_data.escortdoneexpire=str2num(script2);      // Dupois - Added Dec 20, 1999 for escorts

		else if(!(strcmp((char*)script1,"BOUNTYSACTIVE"))) server_data.bountysactive=str2num(script2);            // Dupois - Added July 18, 2000 for bountys
		else if(!(strcmp((char*)script1,"BOUNTYSEXPIRE"))) server_data.bountysexpire=str2num(script2);            // Dupois - Added July 18, 2000 for bountys

		else if(!(strcmp((char*)script1,"TAMED_DISAPPEAR"))) server_data.tamed_disappear=str2num(script2); // Ripper
		else if(!(strcmp((char*)script1,"HOUSEINTOWN"))) server_data.houseintown=str2num(script2); // Ripper
		else if(!(strcmp((char*)script1,"SHOPRESTOCK"))) server_data.shoprestock=str2num(script2); // Ripper
		else if(!(strcmp((char*)script1, "COMMANDPREFIX" ) ) ) server_data.commandPrefix=script2[0]; //Ripper
	 
		else if(!(strcmp((char*)script1,"LOOPSAVE"))) cwmWorldState->SetLoopSaveAmt( str2num( script2 ) );


		else if(!(strcmp((char*)script1,"ERRORS_TO_CONSOLE"))) server_data.errors_to_console=str2num( script2 ); // LB

		else if(!(strcmp((char*)script1,"HOUSEDECAY_SECS"))) server_data.housedecay_secs=str2num( script2 ); // LB
		else if(!(strcmp((char*)script1,"SHOW_CVCS_INFO_AT_LOGIN"))) server_data.showCVCS=str2num(script2); //blackwind 
		else if(!(strcmp((char*)script1,"DEFAULT_JAIL_TIME"))) server_data.default_jail_time=str2num(script2);// blackwind 
		else if(!(strcmp((char*)script1,"PORT"))) wp_port = (short) str2num(script2);
		else if(!(strcmp((char*)script1,"PACKETSENDSTYLE:"))) 
		{
			if ( (!strcmp ( script2, "UOX3") ) )           server_data.packetsendstyle = PSS_UOX3;
			else if ( (!strcmp ( script2, "OSI") ) )       server_data.packetsendstyle = PSS_OSI;
			else if ( (!strcmp ( script2, "SMARTWOLF") ) ) server_data.packetsendstyle = PSS_SMARTWOLF;
			else server_data.packetsendstyle = PSS_UOX3;
			//printf("PSS: %i\n", server_data.packetsendstyle);
		}

	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}


void loadtracking() // Load scriptable tracking data
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"BASE_TRACKING_RANGE"))) tracking_data.baserange=str2num(script2);
		else if(!(strcmp((char*)script1,"MAX_TRACKING_TARGETS")))
		{
			tracking_data.maxtargets=str2num(script2);
			if(tracking_data.maxtargets>MAXTRACKINGTARGETS)
				tracking_data.maxtargets=MAXTRACKINGTARGETS;
		}
		else if(!(strcmp((char*)script1,"BASE_TRACKING_TIME"))) tracking_data.basetimer=str2num(script2);
		else if(!(strcmp((char*)script1,"TRACKING_MESSAGE_REDISPLAY_TIME"))) tracking_data.redisplaytime=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadbegging()
{
	int loopexit=0;
	do
	{
		readw2();

		if(!(strcmp((char*)script1,"BEGGING_TIME"))) begging_data.timer=str2num(script2);
		else if(!(strcmp((char*)script1,"BEGGING_RANGE"))) begging_data.range=str2num(script2);
		else if(!(strcmp((char*)script1,"BEGGING_TEXT0"))) strcpy(begging_data.text[0],(char*)script2);
		else if(!(strcmp((char*)script1,"BEGGING_TEXT1"))) strcpy(begging_data.text[1],(char*)script2);
		else if(!(strcmp((char*)script1,"BEGGING_TEXT2"))) strcpy(begging_data.text[2],(char*)script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadfishing()
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"BASE_FISHING_TIME"))) fishing_data.basetime=str2num(script2);
		else if(!(strcmp((char*)script1,"RANDOM_FISHING_TIME"))) fishing_data.randomtime=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadspiritspeak()
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"SPIRITSPEAKTIMER"))) spiritspeak_data.spiritspeaktimer=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadtime_light()
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"DAY"))) day=str2num(script2);
		else if(!(strcmp((char*)script1,"HOUR"))) hour=str2num(script2);
		else if(!(strcmp((char*)script1,"MINUTE"))) minute=str2num(script2);
		else if(!(strcmp((char*)script1,"AMPM"))) ampm=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON1UPDATE"))) moon1update=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON2UPDATE"))) moon2update=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON1"))) moon1=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON2"))) moon2=str2num(script2);
		else if(!(strcmp((char*)script1,"DUNGEONLIGHTLEVEL"))) dungeonlightlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDFIXEDLEVEL"))) worldfixedlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDCURLEVEL"))) worldcurlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDBRIGHTLEVEL"))) worldbrightlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDDARKLEVEL"))) worlddarklevel=str2num(script2);
		else if(!(strcmp((char*)script1,"SECONDSPERUOMINUTE"))) secondsperuominute=str2num(script2);
		else if(!(strcmp((char*)script1,"SEASON"))) season=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS));
}

void loadremote_admin()
{
	int loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"ENABLE"))) SrvParms->EnableRA = true;
		else if(!(strcmp((char*)script1,"DISABLE"))) SrvParms->EnableRA = false;
		else if(!(strcmp((char*)script1,"PORT"))) SrvParms->ra_port = str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS));
}

void loadserverscript(char *fn) // Load a server script
{
	int loopexit=0;
	wscfile = fopen(fn, "r");
	if(wscfile==NULL)
	{
		clConsole.send("<%s> not found...defaults are used\n",fn);
		return;
	}
	do
	{
		readw2();
		if (!(strcmp((char*)script1, "SECTION")))
		{
			if(!(strcmp((char*)script2, "SERVER"))) loadserver();
			else if(!(strcmp((char*)script2, "CLIENTS_ALLOWED"))) loadclientsallowed();
			else if(!(strcmp((char*)script2, "SPEED"))) loadspeed(); // Zippy
			else if(!(strcmp((char*)script2, "RESOURCE"))) loadresources();
			else if(!(strcmp((char*)script2, "REPSYS"))) loadrepsys();
			else if(!(strcmp((char*)script2, "TRACKING"))) loadtracking();
			else if(!(strcmp((char*)script2, "BEGGING"))) loadbegging();
			else if(!(strcmp((char*)script2, "FISHING"))) loadfishing();
			else if(!(strcmp((char*)script2, "SPIRITSPEAK"))) loadspiritspeak();
			else if(!(strcmp((char*)script2, "TIME_LIGHT"))) loadtime_light();
			// added by Magius(CHE)
			else if(!(strcmp((char*)script2, "HUNGER"))) loadhunger();
			else if(!(strcmp((char*)script2, "COMBAT"))) loadcombat();
			else if(!(strcmp((char*)script2, "VENDOR"))) loadvendor();
			else if(!(strcmp((char*)script2, "REGENERATE"))) loadregenerate();
			else if(!(strcmp((char*)script2, "REMOTE_ADMIN"))) loadremote_admin();
			// end addons by Magius(CHE)
		}
	}
	while ( (strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS) );
	fclose(wscfile);
	wscfile = NULL;
}

void loadserverscript() // Load server script
{
	loadserverscript("server.scp");
	loadserverscript("shards_server.scp");
}

//void saveserverscript(char x)
void saveserverscript(void)
{
	FILE *file;
	file = fopen("server.scp", "w");
    vector<string>::const_iterator vis; 
	
	fprintf(file, "// WOLFPACK Server Script\n");
	fprintf(file, "// Generated by %s Version %s\n\n",  wp_version.betareleasestring.c_str() , wp_version.verstring.c_str() );

	fprintf(file, "SECTION CLIENTS_ALLOWED\n");
	fprintf(file, "{\n");	
   
    for (vis=clientsAllowed.begin(); vis != clientsAllowed.end();  ++vis) 
    {
	  fprintf(file, "%s\n", vis->c_str()); 
    }  

	fprintf(file, "}\n\n");

	
	fprintf(file, "SECTION SERVER\n");
	fprintf(file, "{\n");		

	fprintf(file, "PORT %i\n", wp_port);	// First entry, make it more visible.

	switch(server_data.packetsendstyle)
	{
	    case PSS_UOX3:
		{
		  fprintf(file, "PACKETSENDSTYLE: UOX3\n");
		}
		break;

		case PSS_OSI:
		{
		  fprintf(file, "PACKETSENDSTYLE: OSI\n");
		}
		break;

		case PSS_SMARTWOLF:
		{
			fprintf(file, "PACKETSENDSTYLE: SMARTWOLF\n");
		}
		break;

		default:
		{
		  fprintf(file, "PACKETSENDSTYLE: UOX3\n");
		}
		break;
	}

	fprintf(file, "SKILLCAP %i\n",server_data.skillcap);
	fprintf(file, "STATCAP %i\n",server_data.statcap);
	fprintf(file, "SKILLADVANCEMODIFIER %i\n",server_data.skilladvancemodifier);
    fprintf(file, "STATSADVANCEMODIFIER %i\n",server_data.statsadvancemodifier);
	fprintf(file, "USESPECIALBANK %i\n",server_data.usespecialbank);	//AntiChrist - specialbank
	fprintf(file, "SPECIALBANKTRIGGER %s\n",server_data.specialbanktrigger);
	fprintf(file, "GOLDWEIGHT %f\n",server_data.goldweight);//AntiChrist - gold weight
	fprintf(file, "DECAYTIMER %i\n",server_data.decaytimer);

	//taken from 6904t2(5/10/99) - AntiChrist
	fprintf(file, "PLAYERCORPSEDECAYMULTIPLIER %i\n", server_data.playercorpsedecaymultiplier);
	fprintf(file, "LOOTDECAYSWITHCORPSE %i\n", server_data.lootdecayswithcorpse);

	fprintf(file, "INVISTIMER %i\n",server_data.invisibiliytimer);
	fprintf(file, "SKILLDELAY %i\n",server_data.skilldelay);
	fprintf(file, "SKILLLEVEL %i\n",server_data.skilllevel); // By Magius(CHE)
	fprintf(file, "OBJECTDELAY %i\n",server_data.objectdelay);
	fprintf(file, "BANDAGEDELAY %i\n",server_data.bandagedelay);
	fprintf(file, "BANDAGEINCOMBAT %i\n",server_data.bandageincombat);
	fprintf(file, "INACTIVITYTIMEOUT %i\n",server_data.inactivitytimeout);
	fprintf(file, "GATETIMER %i\n",server_data.gatetimer);
	fprintf(file, "SHOWDEATHANIM %i\n",server_data.showdeathanim);
	fprintf(file, "GUARDSACTIVE %i\n",server_data.guardsactive);
	fprintf(file, "ANNOUNCE_WORLDSAVES %i\n",server_data.announceworldsaves);
	fprintf(file, "BG_SOUNDS %i\n",server_data.bg_sounds);
	fprintf(file, "ARCHIVEPATH %s\n",server_data.archivepath);
	fprintf(file, "BACKUP_SAVE_RATIO %i\n",server_data.backup_save_ratio);//LB
	fprintf(file, "POISONTIMER %i\n",server_data.poisontimer); // lb
	//fprintf(file,"REPAIRWORLDTIMER %i\n",server_data.repairworldtimer); //Ripper
	fprintf(file, "JOINMSG %i\n",server_data.joinmsg);
	fprintf(file, "PARTMSG %i\n",server_data.partmsg);
 	fprintf(file, "SAVESPAWNREGIONS %i\n",server_data.savespawns);
 

	fprintf(file, "SERVER_LOG %i\n",server_data.server_log); //Lb, splitt log to those 4
	fprintf(file, "SPEECH_LOG %i\n",server_data.speech_log);
	fprintf(file, "PVP_LOG %i\n",server_data.pvp_log);
	fprintf(file, "GM_LOG %i\n",server_data.gm_log);
	
	fprintf(file, "ROGUE %i\n",server_data.rogue);
	fprintf(file, "CHAR_TIME_OUT %i\n",server_data.quittime);//Instalog
	fprintf(file, "MAXSTEALTHSTEPS %i\n",server_data.maxstealthsteps);//Instalog
	fprintf(file, "RUNNINGSTAMINASTEPS %i\n",server_data.runningstaminasteps);//Instalog
    fprintf(file, "BOAT_SPEED %f\n",server_data.boatspeed);//Boats
	fprintf(file, "STABLING_FEE %f\n",server_data.stablingfee);//Boats

    fprintf(file, "HTML %i\n",server_data.html);
    fprintf(file, "CUT_SCROLL_REQUIREMENTS %i\n",server_data.cutscrollreq);//AntiChrist
    fprintf(file, "PERSECUTION %i\n",server_data.persecute);//AntiChrist
	fprintf(file, "AUTO_CREATE_ACCTS %i\n", server_data.auto_a_create);
	fprintf(file, "CHECKCHARAGE %i\n", server_data.checkcharage) ;
	fprintf(file, "AUTO_RELOAD_ACCTS %i\n", server_data.auto_a_reload);
    
	fprintf(file, "MSGBOARDPATH %s\n",server_data.msgboardpath);              // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "MSGPOSTACCESS %i\n",server_data.msgpostaccess);            // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "MSGPOSTREMOVE %i\n",server_data.msgpostremove);            // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "MSGRETENTION %i\n",server_data.msgretention);              // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "ESCORTACTIVE %i\n",server_data.escortactive);              // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "ESCORTINITEXPIRE %i\n",server_data.escortinitexpire);      // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "ESCORTACTIVEEXPIRE %i\n",server_data.escortactiveexpire);  // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "ESCORTDONEEXPIRE %i\n",server_data.escortdoneexpire);      // Dupois - Added Dec 20, 1999 for escorts
	
	fprintf(file, "BOUNTYSACTIVE %i\n",server_data.bountysactive);            // Dupois - Added July 18, 2000 for bountys
	fprintf(file, "BOUNTYSEXPIRE %i\n",server_data.bountysexpire);            // Dupois - Added July 18, 2000 for bountys
	
	fprintf(file, "TAMED_DISAPPEAR %i\n",server_data.tamed_disappear); // Ripper
	fprintf(file, "HOUSEINTOWN %i\n",server_data.houseintown); // Ripper
	fprintf(file, "SHOPRESTOCK %i\n",server_data.shoprestock); // Ripper
	fprintf(file, "COMMANDPREFIX %c\n", server_data.commandPrefix ); // Ripper
	fprintf(file, "LOOPSAVE %i\n",cwmWorldState->LoopSaveAmt());  
	fprintf(file, "ERRORS_TO_CONSOLE %i\n",server_data.errors_to_console);
	fprintf(file, "HOUSEDECAY_SECS %i\n",server_data.housedecay_secs);
	fprintf(file, "SHOW_CVCS_INFO_AT_LOGIN %i\n",server_data.showCVCS); // blackwind 
	fprintf(file, "DEFAULT_JAIL_TIME %i\n",server_data.default_jail_time);

	
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION REMOTE_ADMIN\n");
	fprintf(file, "{\n");
	if (server_data.EnableRA)
		fprintf(file, "ENABLE\n");
	else
		fprintf(file, "DISABLE\n");
	fprintf(file, "PORT %i\n", server_data.ra_port);
	fprintf(file, "}\n\n");
	

	fprintf(file, "SECTION SPEED\n"); //Lag Fix -- Zippy
	fprintf(file, "{\n");
    fprintf(file, "NICE %i\n",speed.nice);
	//fprintf(file,"LORDB_LAG_FIX %i\n",server_data.lordblagfix);
	fprintf(file, "CHECK_ITEMS %f\n",speed.itemtime);
	fprintf(file, "CHECK_NPCS %f\n",speed.npctime);
	fprintf(file, "CHECK_TAMEDNPCS %f\n",speed.tamednpctime);//AntiChrist
	fprintf(file, "CHECK_NPCFOLLOW %f\n",speed.npcfollowtime);//Ripper
	fprintf(file, "CHECK_NPCAI %f\n",speed.npcaitime);
	fprintf(file, "CHECK_SPAWNREGIONS %i\n",speed.srtime);
	fprintf(file, "CHECKMEM %i\n", speed.checkmem);
	fprintf(file, "CACHE_MUL %i\n",Map->Cache);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION COMBAT\n");
	fprintf(file, "{\n");
	fprintf(file, "COMBAT_HIT_MESSAGE %i\n",server_data.combathitmessage);
	fprintf(file, "MAX_ABSORBTION %i\n",server_data.maxabsorbtion);
	fprintf(file, "MAX_NON_HUMAN_ABSORBTION %i\n",server_data.maxnohabsorbtion);
	fprintf(file, "MONSTERS_VS_ANIMALS %i\n",server_data.monsters_vs_animals); 
	fprintf(file, "ANIMALS_ATTACK_CHANCE %i\n",server_data.animals_attack_chance); 
	fprintf(file, "ANIMALS_GUARDED %i\n",server_data.animals_guarded); 
	fprintf(file, "NPC_DAMAGE_RATE %i\n",server_data.npcdamage);
	fprintf(file, "NPC_BASE_FLEEAT %i\n",server_data.npc_base_fleeat);
	fprintf(file, "NPC_BASE_REATTACKAT %i\n",server_data.npc_base_reattackat);
	fprintf(file, "ATTACKSTAMINA %i\n",server_data.attackstamina);	// antichrist (6)
	fprintf(file, "ATTACK_DISTANCE %i\n",server_data.attack_distance); // blackwind 
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION VENDOR\n");
	fprintf(file, "{\n");
	fprintf(file, "SELLBYNAME %i\n",server_data.sellbyname);	// Magius(CHE)
	fprintf(file, "SELLMAXITEM %i\n",server_data.sellmaxitem);	// Magius(CHE)
	fprintf(file, "TRADESYSTEM %i\n",server_data.trade_system);	// Magius(CHE)
	fprintf(file, "RANKSYSTEM %i\n",server_data.rank_system);// Moved by Magius(CHE)
	fprintf(file, "CHECKBANK %i\n",server_data.CheckBank); // Ripper
	fprintf(file, "VENDORGREET %i\n",server_data.VendorGreet); // Ripper
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION REGENERATE\n");
	fprintf(file, "{\n");
	fprintf(file, "HITPOINTS_REGENRATE %i\n",server_data.hitpointrate);
	fprintf(file, "STAMINA_REGENRATE %i\n",server_data.staminarate);
	fprintf(file, "MANA_REGENRATE %i\n",server_data.manarate);
	fprintf(file, "ARMOR_AFFECT_MANA_REGEN %i\n",server_data.armoraffectmana);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION HUNGER\n");
	fprintf(file, "{\n");
	fprintf(file, "HUNGERRATE %i\n",server_data.hungerrate);
	fprintf(file, "HUNGER_DAMAGE %i\n",server_data.hungerdamage);
	fprintf(file, "HUNGER_DAMAGE_RATE %i\n",server_data.hungerdamagerate);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION RESOURCE\n");
	fprintf(file, "{\n");
	fprintf(file, "MINECHECK %i\n",server_data.minecheck);
	fprintf(file, "ORE_PER_AREA %i\n",resource.ore);
	fprintf(file, "ORE_RESPAWN_TIME %i\n",resource.oretime);
	fprintf(file, "ORE_RESPAWN_RATE %i\n",resource.orerate);
	fprintf(file, "ORE_RESPAWN_AREA %i\n",resource.orearea);
	fprintf(file, "ORE_TRIGGER %i\n",resource.miningtrigger);
	fprintf(file, "ORE_STAMINA %i\n",resource.miningstamina);
	fprintf(file, "LOGS_PER_AREA %i\n",resource.logs);
	fprintf(file, "LOG_RESPAWN_TIME %i\n",resource.logtime);
	fprintf(file, "LOG_RESPAWN_RATE %i\n",resource.lograte);
	fprintf(file, "LOG_RESPAWN_AREA %i\n",resource.logarea);
	fprintf(file, "LOG_TRIGGER %i\n",resource.logtrigger);
	fprintf(file, "LOG_STAMINA %i\n",resource.logstamina);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION REPSYS\n");
	fprintf(file, "{\n");
	fprintf(file, "MURDER_DECAY %i\n", repsys.murderdecay);
	fprintf(file, "MAXKILLS %i\n", repsys.maxkills);
	fprintf(file, "CRIMINAL_TIME %i\n", repsys.crimtime);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION TRACKING\n");
	fprintf(file, "{\n");
	fprintf(file, "BASE_TRACKING_RANGE %i\n",tracking_data.baserange);
	fprintf(file, "MAX_TRACKING_TARGETS %i\n",tracking_data.maxtargets);
	fprintf(file, "BASE_TRACKING_TIME %i\n",tracking_data.basetimer);
	fprintf(file, "TRACKING_MESSAGE_REDISPLAY_TIME %i\n",tracking_data.redisplaytime);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION BEGGING\n");
	fprintf(file, "{\n");

	fprintf(file, "BEGGING_RANGE %i\n",begging_data.range);
	fprintf(file, "BEGGING_TIME %i\n",begging_data.timer);
	fprintf(file, "BEGGING_TEXT0 %s\n",begging_data.text[0]);
	fprintf(file, "BEGGING_TEXT1 %s\n",begging_data.text[1]);
	fprintf(file, "BEGGING_TEXT2 %s\n",begging_data.text[2]);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION FISHING\n");
	fprintf(file, "{\n");
	fprintf(file, "BASE_FISHING_TIME %i\n",fishing_data.basetime);
	fprintf(file, "RANDOM_FISHING_TIME %i\n",fishing_data.randomtime);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION SPIRITSPEAK\n");
	fprintf(file, "{\n");
	fprintf(file, "SPIRITSPEAKTIMER %i\n",spiritspeak_data.spiritspeaktimer);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION TIME_LIGHT\n");
	fprintf(file, "{\n");
	fprintf(file, "DAY %i\n", day);
	fprintf(file, "HOUR %i\n", hour);
	fprintf(file, "MINUTE %i\n", minute);
	fprintf(file, "AMPM %i\n", ampm);
	fprintf(file, "MOON1UPDATE %i\n", moon1update);
	fprintf(file, "MOON2UPDATE %i\n", moon2update);
	fprintf(file, "MOON1 %i\n", moon1);
	fprintf(file, "MOON2 %i\n", moon2);
	fprintf(file, "DUNGEONLIGHTLEVEL %i\n", dungeonlightlevel);
	fprintf(file, "WORLDFIXEDLEVEL %i\n", worldfixedlevel);
	fprintf(file, "WORLDCURLEVEL %i\n", worldcurlevel);
	fprintf(file, "WORLDBRIGHTLEVEL %i\n", worldbrightlevel);
	fprintf(file, "WORLDDARKLEVEL %i\n", worlddarklevel);
	fprintf(file, "SECONDSPERUOMINUTE %i\n", secondsperuominute); // lb
	fprintf(file, "SEASON %i\n", season); // lb
	fprintf(file, "}\n\n");
	
	fprintf(file, "EOF\n\n");
	fclose(file);
	file = NULL;
}

