
#include "srvparams.h"

// Library Includes
#include "preferences.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qhostaddress.h"

const char preferencesFileVersion[] = "1.0";

cSrvParams::cSrvParams( const QString& filename, const QString& format, const QString& version )  : Preferences(filename, format, version)
{
	// Load data into binary format
	// If value not found, create key.
	skillcap_				= getNumber("General",	"SkillCap",			700, true);
	statcap_				= getNumber("General",	"StatsCap",			300, true);
	commandPrefix_			= getString("General",	"Command Prefix",	"#", true).latin1()[0];
	skillAdvanceModifier_	= getNumber("General",	"Skill Advance Modifier", 1000, true);
	statsAdvanceModifier_	= getNumber("General",	"Stats Advance Modifier", 500, true);
	objectDelay_			= getNumber("Game Speed", "ObjectDelay", 1, true);
	bgSound_				= getNumber("General",	"BackGround Sound Chance", 2, true);
	stealing_				= getBool("General",	"Stealing Enabled",	true, true);			
	guardsActive_			= getBool("General",	"Guards Enabled",	true, true);
	partMsg_				= getBool("General",	"PartMessage",		true, true);
	joinMsg_				= getBool("General",	"JoinMessage",		true, true);
	saveSpawns_				= getBool("General",	"Save Spawned Regions", true, true);
	autoAccountCreate_		= getBool("Accounts",	"Auto Create",		false, true);
	autoAccountReload_		= getNumber("Accounts",	"Auto Reload",		10, true);
	stablingFee_			= getDouble("General",	"StablingFee",		0.25, true);
	checkCharAge_			= getBool("Accounts",	"Check Delete Age", true, true);
	announceWorldSaves_		= getBool("General",	"Announce WorldSaves", true, true);
	checkItemTime_			= getDouble("Game Speed", "Items Check Time", 1.1, true);
	checkNPCTime_			= getDouble("Game Speed", "NPCs Check Time", 1.0, true);
	checkAITime_			= getDouble("Game Speed", "AI Check Time", 1.2, true);
	niceLevel_				= getNumber("Game Speed", "Nice Level", 2, true);
	port_                   = getNumber("General",    "Port", 2593, true);
	goldWeight_             = getDouble("General",    "Gold Weight", 0.001000, true);
	playercorpsedecaymultiplier_ = getNumber("General", "Player Corpse Decay Multiplier", 0, true);
	lootdecayswithcorpse_   = getNumber("General",    "Loot Decays With Corpse", 1, true);
	invisTimer_             = getDouble("General",    "InvisTimer", 60, true);
	skillDelay_			    = getNumber("Game Speed", "SkillDelay", 7, true);
	skillLevel_				= getNumber("Game Speed", "SkillLevel", 3, true);
	bandageDelay_			= getNumber("Game Speed", "BandageDelay", 6, true);
	bandageInCombat_		= getBool("General",	"Bandage In Combat",	true, true);
	gateTimer_              = getDouble("General",    "GateTimer", 30, true);
	inactivityTimeout_		= getNumber("General",  "Inactivity Timeout", 300, true);
	showDeathAnim_		    = getNumber("General",  "Show Death Animation", 1, true);
	poisonTimer_		    = getNumber("General",  "PoisonTimer", 180, true);
	maxStealthSteps_		= getNumber("Game Speed",  "Max Stealth Steps", 10, true);
	runningStamSteps_		= getNumber("Game Speed", "Running Stamina Steps", 15, true);

	flush(); // if any key created, save it.
}

std::vector<ServerList_st>& cSrvParams::serverList()
{
	if ( serverList_.empty() ) // Empty? Try to load
	{
		if (!containGroup("LoginServer"))
			setDefaultServerList();
		setGroup("LoginServer");
		bool bKeepLooping = true;
		unsigned int i = 1;
		do
		{
			QString tmp = getString(QString("Shard %1").arg(i++), "").simplifyWhiteSpace();
			bKeepLooping = ( tmp != "" );
			if ( bKeepLooping ) // valid data.
			{
				QStringList strList = QStringList::split("=", tmp);
				if ( strList.size() == 2 )
				{
					ServerList_st server;
					server.sServer = strList[0];
					QStringList strList2 = QStringList::split(",", strList[1].stripWhiteSpace());
					server.sIP = strList2[0];
					server.uiPort = strList2[1].toUShort();
					serverList_.push_back(server);
				}
			}
		} while ( bKeepLooping );
	}
	return serverList_;
}

std::vector<StartLocation_st>& cSrvParams::startLocation()
{
	if ( startLocation_.empty() ) // Empty? Try to load
	{
		if (!containGroup("StartLocation") )
			setDefaultStartLocation();
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
	setString("StartLocation", "Location 1", "Yew=567,978,0,0");
	setString("StartLocation", "Location 2", "Minoc=2477,407,15,0");
	setString("StartLocation", "Location 3", "Britain=1496,1629,10,0");
	setString("StartLocation", "Location 4", "Moonglow=4404,1169,0,0");
	setString("StartLocation", "Location 5", "Trinsic=1844,2745,0,0");
	setString("StartLocation", "Location 6", "Magincia=3738,2223,20,0");
	setString("StartLocation", "Location 7", "Jhelom=1378,3817,0,0");
	setString("StartLocation", "Location 8", "Skara Brae=594,2227,0,0");
	setString("StartLocation", "Location 9", "Vesper=2771,977,0,0");
	flush(); // save
}

void cSrvParams::setDefaultServerList()
{
	setBool("LoginServer", "enabled", true);
	setString("LoginServer", "Shard 1", "Default=127.0.0.1,2593");
	flush(); // save.
}

