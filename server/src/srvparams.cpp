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

#ifndef __unix__
#include "winsock.h"
#endif

#include "srvparams.h"
#include "globals.h"
#include "verinfo.h"
#include "preferences.h"

// Library Includes
#include <qstring.h>
#include <qstringlist.h>
#include <qhostaddress.h>
#include <qdatetime.h>

const char preferencesFileVersion[] = "1.0";

#ifdef __unix__
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#endif

#ifndef INADDR_NONE
#define INADDR_NONE (-1)
#endif

Q_INT32 resolveName( const QString& data )
{
	if ( data.isEmpty() )
		return INADDR_NONE;
//	we do a dns lookup on this

	Q_INT32 uiValue = inet_addr((char*)data.latin1()) ;
	if (uiValue == INADDR_NONE)
	{
		hostent* ptrHost = gethostbyname((char*)data.latin1());
 		if(ptrHost != 0)
		{
	        char** ptrPtr = ptrHost->h_addr_list;
                //We only use the first one
            if(*ptrPtr != NULL)
            {
            //      I can think of no other way
	            memcpy(&uiValue,*ptrPtr,sizeof(in_addr)) ;
		    }
		}
	}

	// inet_addr returns the ip in reverse order
	Q_INT32 part1 = 0, part2 = 0, part3 = 0, part4 = 0;
	part1 = ( (uiValue & 0x000000FF) << 24 );
	part2 = ( (uiValue & 0x0000FF00) << 8 );
	part3 = ( (uiValue & 0x00FF0000) >> 8 );
	part4 = ( (uiValue & 0xFF000000) >> 24 );

	return (part1 + part2 + part3 + part4);
}

cSrvParams::cSrvParams( const QString& filename, const QString& format, const QString& version )  : Preferences(filename, format, version)
{
	// Load data into binary format
	// If value not found, create key.
	readData();
	if (!containGroup("StartLocation") )
		setDefaultStartLocation();
	if (!containGroup("LoginServer"))
		setDefaultServerList();
	flush(); // if any key created, save it.
}

void cSrvParams::readData()
{
	// Account Group
	autoAccountCreate_		= getBool("Accounts",	"Auto Create",		false, true);
	autoAccountReload_		= getNumber("Accounts",	"Auto Reload",		10, true);
	checkCharAge_			= getBool("Accounts",	"Check Delete Age", true, true);
	maxLoginAttempts_		= getNumber("Accounts", "Max Login Attempts", 5, true);
	resetAttemptCount_		= getNumber("Accounts", "Reset Attempts Count", 3*60, true);
	accountsBlockTime_		= getNumber("Accounts", "Block Failed Logins", 3*60, true);
	accountsArchiver_		= getString("Accounts", "Archiver Module", "xml", true);


	// Persistency
	databasePassword_		= getString("Database", "password", QString::null, true);
	databaseUsername_		= getString("Database", "username", QString::null, true);
	databaseHost_			= getString("Database", "host", "localhost", true);
	databaseName_			= getString("Database", "name", "wolfpack", true);
	databaseDriver_			= getString("Database", "driver", "QMYSQL3", true);


	// Remote Admin
	ra_port_		        = getNumber("Remote Admin",	"Port", 2594, true);
	EnableRA_			    = getBool("Remote Admin",	"Enable", false, true);

	// Repsys
	murderdecay_		    = getNumber("Repsys", "Murder Decay", 28800, true);
	maxkills_		        = getNumber("Repsys", "Max Kills", 5, true);
	crimtime_		        = getNumber("Repsys", "Criminal Time", 120, true);
    
	// Game Speed Group
	objectDelay_			= getNumber("Game Speed", "ObjectDelay", 1, true);
	checkItemTime_			= getDouble("Game Speed", "Items Check Time", 1.1, true);
	checkNPCTime_			= getDouble("Game Speed", "NPCs Check Time", 1.0, true);
	checkTammedTime_		= getDouble("Game Speed", "Tamed Check Time", 0.5, true);
	checkAITime_			= getDouble("Game Speed", "AI Check Time", 1.2, true);
	niceLevel_				= getNumber("Game Speed", "Nice Level", 2, true);
	skillDelay_			    = getNumber("Game Speed", "SkillDelay", 7, true);
	skillLevel_				= getNumber("Game Speed", "SkillLevel", 3, true);
	bandageDelay_			= getNumber("Game Speed", "BandageDelay", 6, true);
	maxStealthSteps_		= getNumber("Game Speed", "Max Stealth Steps", 10, true);
	runningStamSteps_		= getNumber("Game Speed", "Running Stamina Steps", 15, true);
	hungerRate_				= getNumber("Game Speed", "Hunger Rate", 6000, true);
	hungerDamageRate_		= getNumber("Game Speed", "Hunger Damage Rate", 10, true);
	boatSpeed_              = getDouble("Game Speed", "Boat Speed", 0.750000, true);
	snoopdelay_			    = getNumber("Game Speed", "Snoop Delay", 7, true);
	housedecay_secs_	    = getNumber("Game Speed", "House Decay-Sec.", 604800, true);
	default_jail_time_	    = getNumber("Game Speed", "Default Jail Time", 86400, true);
	spiritspeaktimer_	    = getNumber("Game Speed", "Spirit Speak Timer", 60, true);
	spawnRegionCheckTime_   = getNumber("Game Speed", "SpawnRegion Check Time", 300, true);
	decayTime_				= getNumber("Game Speed", "Decay Time", 300, true);
	secondsPerUOMinute_     = getNumber("Game Speed", "Seconds Per UO Minute", 5, true);
	beggingTime_            = getNumber("Game Speed", "Begging Time", 120, true);
	checkTammedTime_		= getDouble("Game Speed", "Tamed Check Time", 1.0, true);
    
	// General Group
	showSkillTitles_		= getBool("General", "ShowSkillTitles", true, true );
	skillcap_				= getNumber("General",	"SkillCap",			700, true);
	statcap_				= getNumber("General",	"StatsCap",			300, true);
	commandPrefix_			= getString("General",	"Command Prefix",	"'", true);
	skillAdvanceModifier_	= getNumber("General",	"Skill Advance Modifier", 1000, true);
	statsAdvanceModifier_	= getNumber("General",	"Stats Advance Modifier", 500, true);
	bgSound_				= getNumber("General",	"BackGround Sound Chance", 2, true);
	stealing_				= getBool("General",	"Stealing Enabled",	true, true);			
	guardsActive_			= getBool("General",	"Guards Enabled",	true, true);
	partMsg_				= getBool("General",	"PartMessage",		true, true);
	joinMsg_				= getBool("General",	"JoinMessage",		true, true);
	saveSpawns_				= getBool("General",	"Save Spawned Regions", true, true);
	stablingFee_			= getDouble("General",	"StablingFee",		0.25, true);
	announceWorldSaves_		= getBool("General",	"Announce WorldSaves", true, true);
	goldWeight_             = getDouble("General",    "Gold Weight", 0.001000, true);
	playercorpsedecaymultiplier_ = getNumber("General", "Player Corpse Decay Multiplier", 1, true);
	lootdecayswithcorpse_   = getBool("General",    "Loot Decays With Corpse", true, true);
	invisTimer_             = getDouble("General",    "InvisTimer", 60, true);
	bandageInCombat_		= getBool("General",	"Bandage In Combat",	true, true);
	gateTimer_              = getDouble("General",    "GateTimer", 30, true);
	inactivityTimeout_		= getNumber("General",  "Inactivity Timeout", 300, true);
	showDeathAnim_		    = getNumber("General",  "Show Death Animation", 1, true);
	poisonTimer_		    = getNumber("General",  "PoisonTimer", 180, true);
	serverLog_		        = getBool("General",	"Server Log", false, true);
	speechLog_		        = getBool("General",	"Speech Log", false, true);
	pvpLog_		            = getBool("General",	"PvP Log", false, true);
	gmLog_		            = getBool("General",	"GM Log", false, true);
	backupSaveRatio_		= getNumber("General",  "Backup Save Ratio", 1, true);
	hungerDamage_			= getNumber("General",  "Hunger Damage", 0, true);
	html_			        = getNumber("General",  "Html", -1, true);	
	persecute_              = getNumber("General",  "Persecution", 1, true);
	tamedDisappear_         = getNumber("General",  "Tamed Disappear", 0, true);
	houseInTown_            = getNumber("General",  "House In Town", 0, true);
	shopRestock_            = getNumber("General",  "Shop Restock", 1, true);
	badNpcsRed_             = getNumber("General",  "Bad Npcs Red", 1, true);
	slotAmount_             = getNumber("General",  "Slot Amount", 5, true);
	hungeraffectsskills_	= getBool("General",  "Hunger affects Skills", true, true);
	belowminskillfails_		= getBool("General",  "Below Min Skill Fails", false, true);
	escortactive_           = getNumber("General",  "Escort Active", 1, true);
	escortinitexpire_       = getNumber("General",  "Escorting Expire", 86400, true);
	escortactiveexpire_     = getNumber("General",  "Escort Active Expire", 1800, true);
	escortdoneexpire_       = getNumber("General",  "Escort Done Expire", 1800, true);
	bountysactive_          = getNumber("General",  "Bounty Active", 1, true);
	bountysexpire_          = getNumber("General",  "Bounty Expire", 0, true);
	quittime_               = getNumber("General",  "Char Time Out", 300, true);
	errors_to_console_      = getNumber("General",  "Errors To Console", 0, true);
	showCVCS_               = getNumber("General",  "Show CVCS", 0, true);
	cacheMulFiles_			= getBool  ("General",  "Cache Mul Files", true, true);
	beggingRange_           = getNumber("General",  "Begging Range", 3, true);
	clientsAllowed_			= QStringList::split(",", getString("General", "Allowed Clients", "SERVER_DEFAULT", true).upper());
	
	// We're saving the elapsed seconds since server-start (INGAME SECONDS!)
	// fromString( QString::number(FIRST_YEAR) + "-" + getString("General", "Initial Date/Time", "01-18T01:00:00", true), Qt::ISODate);
	uoTime.setTime_t( getNumber( "General", "UO Time", 0, true ) );
	
	saveInterval_			= getNumber("General", "Save Interval", 900, true);
	heartBeat_				= getBool("General", "HeartBeat", false, true);
	defaultpriv1_			= getNumber("General", "DefaultPrivileage1", 0, true);
	defaultpriv2_			= getNumber("General", "DefaultPrivileage2", 0, true);
	mulPath_				= getString("General", "MulPath", "./", true);

	// Network
	loginPort_				= getNumber( "Network",		"Loginserver Port", 2593, true );
	gamePort_               = getNumber( "Network",		"Gameserver Port", 2592, true );
	enableLogin_			= getBool( "Network",		"Enable Loginserver", true, true );
	enableGame_				= getBool( "Network",		"Enable Gameserver", true, true );
	allowUnencryptedClients_ = getBool("Network",	"Allow Unencrypted Clients", true, true );

	// Combat
	combatHitMessage_		= getBool("Combat", "Hit Message", true, true );
	maxAbsorbtion_		    = getNumber("Combat", "Max Absorbtion", 20, true );
	maxnohabsorbtion_		= getNumber("Combat", "Max Non Human Absorbtion", 100, true );
	monsters_vs_animals_	= getNumber("Combat", "Monsters vs Animals", 0, true );
	animals_attack_chance_	= getNumber("Combat", "Animals Attack Chance", 15, true );
	animals_guarded_	    = getNumber("Combat", "Animals Guarded", 0, true );
	npcdamage_	            = getNumber("Combat", "Npc Damage", 2, true );
	npc_base_fleeat_	    = getNumber("Combat", "Npc Base Flee At", 20, true );
	npc_base_reattackat_	= getNumber("Combat", "Npc Base Reattack At", 40, true );
	attackstamina_	        = getNumber("Combat", "Attack Stamina", -2, true );
	attack_distance_	    = getNumber("Combat", "Attack Distance", 13, true );

	// Vendor
	sellbyname_	            = getNumber("Vendor", "Sell By Name", 1, true );
	sellmaxitem_	        = getNumber("Vendor", "Sell Max Item", 5, true );
	trade_system_	        = getNumber("Vendor", "Trade System", 0, true );
	rank_system_	        = getNumber("Vendor", "Rank System", 0, true );
	checkBank_	            = getNumber("Vendor", "Check Bank", 2000, true );
	vendorGreet_	        = getNumber("Vendor", "Vendor Greet", 1, true );
	showNpcTitles_			= getNumber("Vendor",  "Show Npc Titles", 1, true);

	// Regenerate
	hitpointrate_			= getNumber("Regenerate", "Hitpoints Regenerate", 8, true);
	staminarate_			= getNumber("Regenerate", "Stamina Regenerate", 3, true);
	manarate_				= getNumber("Regenerate", "Mana Regenerate", 5, true);
	armoraffectmana_		= getNumber("Regenerate", "Armor Affect Mana Regenerate", 0, true);

	// Resources
	resourceitemdecaytime_	= getNumber("Resources", "ResourceItem Decay Time (not empty)", 60*60*12, true);

	// Tracking
	baserange_			    = getNumber("Tracking", "Base Tracking Range", 10, true);
	maxtargets_			    = getNumber("Tracking", "Max Tracking Targets", 20, true);
	basetimer_			    = getNumber("Tracking", "Base Tracking Time", 5, true);
	redisplaytime_			= getNumber("Tracking", "Tracking Message Redisplay Time", 5, true);

	// Fishing
	basetime_			    = getNumber("Fishing", "Base Fishing Time", 10, true);
	randomtime_			    = getNumber("Fishing", "Random Fishing Time", 5, true);

	// Message Board
	msgboardPath_			= getString("MessageBoard", "Path", "./", true);
	msgboardPostAccess_     = getNumber("MessageBoard", "Post Access", 1, true);
	msgboardPostRemove_     = getNumber("MessageBoard", "Post Remove", 0, true);
	msgboardRetention_      = getNumber("MessageBoard", "Retention", 30, true);

	// Light
	worldBrightLevel_       = getNumber("Light", "World Bright Level", 1, true);
	worldFixedLevel_		= getNumber("Light", "World Fixed Level", 0, true);
	worldDarkLevel_			= getNumber("Light", "World Dark Level", 18, true);
	dungeonLightLevel_		= getNumber("Light", "Dungeon Level", 18, true);
	season_		            = getNumber("Light", "Set Season", 0, true);

	// Magic
	cutScrollReq_			= getBool( "Cut Scroll Requirements", false, true );
	walkDisturbsCast_		= getBool( "Walking Disturbs Casting", true, true );
	precasting_				= getBool( "Precasting", true, true );
}

void cSrvParams::reload()
{
	serverList_.clear();
	startLocation_.clear();
	Preferences::reload();
	readData();
}

std::vector<StartLocation_st>& cSrvParams::startLocation()
{
	if ( startLocation_.empty() ) // Empty? Try to load
	{
		setGroup("StartLocation");
		bool bKeepLooping = true;
		unsigned int i = 1;
		do
		{
			QString tmp = getString(QString("Location %1").arg(i++), "").simplifyWhiteSpace();
			bKeepLooping = ( tmp != "" );
			if ( bKeepLooping ) // valid data.
			{
				QStringList strList = QStringList::split("=", tmp);
				if ( strList.size() == 2 )
				{
					StartLocation_st loc;
					loc.name = strList[0];
					QStringList strList2 = QStringList::split(",", strList[1].stripWhiteSpace());
					if ( strList2.size() == 4 )
					{
						loc.pos.x = strList2[0].toUShort();
						loc.pos.y = strList2[1].toUShort();
						loc.pos.z = strList2[2].toUShort();
						loc.pos.map = strList2[3].toUShort();
						startLocation_.push_back(loc);
					}
				}
			}
		} while ( bKeepLooping );
	}
	return startLocation_;
}

void cSrvParams::setDefaultStartLocation()
{
	setString("StartLocation", "Location 1", "Yew=567,978,0,1");
	setString("StartLocation", "Location 2", "Minoc=2477,407,15,1");
	setString("StartLocation", "Location 3", "Britain=1496,1629,10,1");
	setString("StartLocation", "Location 4", "Moonglow=4404,1169,0,1");
	setString("StartLocation", "Location 5", "Trinsic=1844,2745,0,1");
	setString("StartLocation", "Location 6", "Magincia=3738,2223,20,1");
	setString("StartLocation", "Location 7", "Jhelom=1378,3817,0,1");
	setString("StartLocation", "Location 8", "Skara Brae=594,2227,0,1");
	setString("StartLocation", "Location 9", "Vesper=2771,977,0,1");
	flush(); // save
}

void cSrvParams::setDefaultServerList()
{
	setString("LoginServer", "Shard 1", "Default=127.0.0.1,2592");
	flush(); // save.
}

void cSrvParams::setSecondsPerUOMinute( unsigned int data )
{
	secondsPerUOMinute_ = data;
	setNumber("Game Speed", "Seconds Per UO Minute", secondsPerUOMinute_);
	flush();
}

void cSrvParams::setWorldBrightLevel( unsigned char data )
{
	worldBrightLevel_ = data;
	setNumber("Light", "World Bright Level", data);
	flush();
}

void cSrvParams::setWorldFixedLevel( unsigned char data )
{
	worldFixedLevel_ = data;
	setNumber("Light", "World Fixed Level", data);
	flush();
}

void cSrvParams::setWorldDarkLevel( unsigned char data )
{
	worldDarkLevel_ = data;
	setNumber("Light", "World Dark Level", data);
	flush();
}

void cSrvParams::setDungeonLightLevel( unsigned char data )
{
	dungeonLightLevel_ = data;
	setNumber("Light", "Dungeon Level", data);
	flush();
}


bool cSrvParams::isClientAllowed( const QString& data )
{
	if ( clientsAllowed_.contains(data))
		return true;
	else if ( clientsAllowed_.contains("ALL"))
		return true;
	else if ( data == wp_version.clientsupportedstring && clientsAllowed_.contains("SERVER_DEFAULT"))
		return true;
	return false;
}

unsigned char& cSrvParams::worldCurrentLevel()
{
	return worldCurrentLevel_;
}


