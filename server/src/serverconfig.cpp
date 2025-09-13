/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#include "serverconfig.h"

#include "log.h"
#include "verinfo.h"
#include "basedef.h"
#include "inlines.h"
#include "preferences.h"

// Library Includes
#include <QString>
#include <QStringList>
#include <QDir>
#include <QHostAddress>
#include <QHostInfo>
#include <QDateTime>

#include <QtGlobal>
#if defined( Q_OS_WIN32 )
# include <winsock.h>
#elif defined ( Q_OS_UNIX )
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

const char preferencesFileVersion[] = "1.0";

#ifndef INADDR_NONE
#define INADDR_NONE (-1)
#endif

QHostAddress internetAddress()
{
	hostent* hostinfo;
	char name[256];
	QHostAddress result;

	if ( !gethostname( name, sizeof( name ) ) )
	{
		hostinfo = gethostbyname( name );

		if ( hostinfo )
		{
			quint32 i = 0;

			while ( hostinfo->h_addr_list[i] )
			{
				// Check if it's an INTERNET ADDRESS
				char* hIp = inet_ntoa( *( struct in_addr* ) hostinfo->h_addr_list[i++] );
				result.setAddress( hIp );
				quint32 ip = result.toIPv4Address();

				if ( ip & 0x0A000000 || // 10.x.x.x
					 ip & 0x7F000000 || // 127.x.x.x
					 ip & 0xC0A80000 || // 192.168.x.x
					 ip & 0xAC100000 || // 172.16-31.x.x 
					 ip & 0xA9FE0000 )  // 169.254.x.x
					continue;
/*
				if ( ( part1 == 127 ) || 	//this one is class A too.
					( part1 == 10 ) || ( ( part1 == 192 ) && ( part2 == 168 ) ) || ( ( part1 == 172 ) && ( part2 >= 16 ) && ( part2 <= 31 ) ) || ( ( part1 == 169 ) && ( part2 == 254 ) ) // DHCP Space Stuff
					)
				{
					continue;
				}
*/

				// We are now certain that it's a valid INET ip
				break;
			}
		}
	}
	return result;
}

cConfig::cConfig() : Preferences( "wolfpack.xml", "Wolfpack", "1.0" )
{
}

void cConfig::readData()
{
	Preferences::readData();

	// Account Group
	autoAccountCreate_ = getBool( "Accounts", "Auto Create", false, true );
	autoAccountACL_ = getString( "Accounts", "Auto Create ACL", "player", true );
	accountsDriver_ = getString( "Accounts", "Database Driver", "sqlite", true );
	accountsHost_ = getString( "Accounts", "Database Host", "", true );
	accountsName_ = getString( "Accounts", "Database Name", "accounts.db", true );
	accountsUsername_ = getString( "Accounts", "Database Username", "", true );
	accountsPassword_ = getString( "Accounts", "Database Password", "", true );
	accountsPort_ = getNumber("Accounts", "Database Port", 3306, true);	
	hashAccountPasswords_ = getBool( "Accounts", "Use MD5 Hashed Passwords", false, true );
	convertUnhashedPasswords_ = getBool( "Accounts", "Automatically Hash Loaded Passwords", false, true );
	//Old Clients Version - maxCharsPerAccount_ = wpMin<unsigned char>( 6, getNumber( "Accounts", "Maximum Number of Characters", 7, true ) );
	maxCharsPerAccount_ = getNumber( "Accounts", "Maximum Number of Characters", 7, true );
	enableIndivNumberSlots_ = getBool( "Accounts", "Enable Individual Number for Characters Slots", false, true );

	// AI
	checkAITime_ = getDouble( "AI", "Default AI Check Time", 0.5, true );
	checkAINPCsTime_ = getDouble( "AI", "Default AI NPCs Check Time", 0.0, true );
	checkAIITEMsTime_ = getDouble( "AI", "Default AI Items Check Time", 0.0, true );
	animalWildFleeRange_ = getNumber( "AI", "Wild animals flee range", 8, true );
	guardDispelTime_ = getNumber( "AI", "Guard dispel time (sec)", 25, true );
	aiNPCsCheckRange_ = getNumber( "AI", "Default AI NPCs Check Range", 12, true );
	aiITEMsCheckRange_ = getNumber( "AI", "Default AI Items Check Range", 9, true );

	// Persistency
	databasePassword_ = getString( "Database", "Database Password", "", true );
	databaseUsername_ = getString( "Database", "Database Username", "", true );
	databaseHost_ = getString( "Database", "Database Host", "", true );
	databaseDriver_ = getString( "Database", "Database Driver", "binary", true );
	databaseName_ = getString( "Database", "Database Name", "world.db", true );
	databasePort_ = getNumber("Database", "Database Port", 3306, true);
	useDatabaseTransaction_ = getBool("Database", "Use Transactions", false, true );

	// Repsys
	murderdecay_ = getNumber( "Repsys", "Murder Decay", 28800, true );
	maxkills_ = getNumber( "Repsys", "Max Kills", 5, true );
	crimtime_ = getNumber( "Repsys", "Criminal Time", 120, true );

	// Game Speed Group
	objectDelay_ = getNumber( "Game Speed", "ObjectDelay", 1, true );
	checkNPCTime_ = getDouble( "Game Speed", "NPCs Check Time", 0.2, true );
	checkTamedTime_ = getDouble( "Game Speed", "Tamed Check Time", 0.6, true );
	niceLevel_ = getNumber( "Game Speed", "Nice Level", 2, true );
	skillDelay_ = getNumber( "Game Speed", "SkillDelay", 7, true );
	hungerRate_ = getNumber( "Game Speed", "Hunger Rate", 120, true );
	spawnRegionCheckTime_ = getNumber( "Game Speed", "SpawnRegion Check Time", 5, true );
	itemDecayTime_ = getNumber( "Game Speed", "Item Decay Time", 300, true );
	npcCorpseDecayTime_ = getNumber( "Game Speed", "NPC Corpse Decay Time", 600, true );
	playerCorpseDecayTime_ = getNumber( "Game Speed", "Player Corpse Decay Time", 1200, true );
	secondsPerUOMinute_ = getNumber( "Game Speed", "Seconds Per UO Minute", 5, true );

	// General Group
	showDebug_ = getBool( "General", "Show Debug", false, true );
	usesMondainsLegacyMap_ = getBool( "General", "Uses Mondains Legacy Map", true, true );
	usesKingdomRebornMaps_ = getBool( "General", "Uses Kingdom Reborn Maps", true, true );
	enableFeluccaSeason_ = getBool( "General", "Enable Felucca Season", true, true );
	enableTrammelSeason_ = getBool( "General", "Enable Trammel Season", true, true );
	showSkillTitles_ = getBool( "General", "ShowSkillTitles", true, true );
	skillcap_ = getNumber( "General", "SkillCap", 700, true );
	statcap_ = getNumber( "General", "StatsCap", 225, true );
	commandPrefix_ = getString( "General", "Command Prefix", "'", true );
	guardsActive_ = getBool( "General", "Guards Enabled", true, true );
	hungerDamage_ = getNumber( "General", "Hunger Damage", 0, true );
	tamedDisappear_ = getNumber( "General", "Tamed Disappear", 0, true );
	shopRestock_ = getNumber( "General", "Shop Restock", 60, true );
	quittime_ = getNumber( "General", "Char Time Out", 300, true );
	showNpcTitles_ = getBool( "General", "Show Npc Titles", true, true );
	logMask_ = getNumber( "General", "Logging Mask", LOG_ALL, true );
	overwriteDefinitions_ = getBool( "General", "Overwrite Definitions", false, true );
	antiSpeedHack_ = getBool( "General", "Anti Speed Hack", true, true );
	antiSpeedHackDelay_ = getNumber( "General", "Anti Speed Hack Delay", 175, true );
	antiSpeedHackDelayMounted_ = getNumber( "General", "Anti Speed Hack Delay Mounted", 75, true );
	refreshMaxValues_ = getBool( "General", "Refresh Characters Maximum Values", true, true );
	sendAsciiNames_ = getBool( "General", "Send ASCII Character Names", false, true );
	payfrompackonly_ = getBool( "General", "Pay From Pack Only", false, true );
	enableyoung_ = getBool( "General", "Enable Young Status", true, true );
	instalogoutfromguarded_ = getBool( "General", "Insta Logout from Guarded Regions", true, true );
	percentHitsAfterRess_ = getNumber( "General", "Percent Hits after Ress", 10, true );
	percentManaAfterRess_ = getNumber( "General", "Percent Mana after Ress", 50, true );
	percentStaminaAfterRess_ = getNumber( "General", "Percent Stamina after Ress", 50, true );
	disableKarma_ = getBool( "General", "Disable Karma", false, true );
	disableFame_ = getBool( "General", "Disable Fame", false, true );
	saveInterval_ = getNumber( "General", "Save Interval", 900, true );
    mulPath_ = QDir::toNativeSeparators( getString( "General", "MulPath", "./muls/", true ) );
    logPath_ = QDir::toNativeSeparators( getString( "General", "LogPath", "./logs/", true ) );
	logRotate_ = getBool( "General", "LogRotate", true, true );
	mountRange_ = getNumber( "General", "Mount Range", 2, true );
	newTooltipPackets_ = getBool( "General", "New Tooltip Packets", true, true );
	factorMaxHits_ = getDouble( "General", "Factor for MaxHitPoints Calculation", 1.0, true );
	factorMaxMana_ = getDouble( "General", "Factor for MaxMana Calculation", 1.0, true );
	factorMaxStam_ = getDouble( "General", "Factor for MaxStamina Calculation", 1.0, true );
	simpleMaxHitsCalculation_ = getBool( "General", "Simple MaxHitPoints Calculation", false, true );
	enableTimeChangeForItems_ = getBool( "General", "Enable onTimeChange for Items", false, true );
	checkCollisionAtAllMaps_ = getBool( "General", "Check Collision at All Maps", false, true );
	SpeechNormalRange_ = getNumber( "General", "Speech Normal Range", 18, true );
	SpeechYellRange_ = getNumber( "General", "Speech Yell Range", 54, true );
	SpeechWhisperRange_ = getNumber( "General", "Speech Whisper Range", 1, true );
	WeightPercentActiveEvent_ = getNumber( "General", "Weight Percent to Active Event", 0, true );
	deleteCharDelay_ = getNumber( "General", "Minutes to Delete Character", 1440, true );

	// Network
	loginPort_ = getNumber( "Network", "Loginserver Port", 2593, true );
	gamePort_ = getNumber( "Network", "Gameserver Port", 2592, true );
	enableLogin_ = getBool( "Network", "Enable Loginserver", true, true );
	enableGame_ = getBool( "Network", "Enable Gameserver", true, true );
	allowUnencryptedClients_ = getBool( "Network", "Allow Unencrypted Clients", true, true );
	allowStatRequest_ = getBool( "Network", "Allow Stat Request", true, true );

	// Combat
	attack_distance_ = getNumber( "Combat", "Attack Distance", 13, true );
	showdamagedone_ = getBool( "Combat", "Show Damage Done", true, true );
	showdamagereceived_ = getBool( "Combat", "Show Damage Received", true, true );

	// Light
	worldBrightLevel_ = getNumber( "Light", "World Bright Level", 1, true );
	worldFixedLevel_ = getNumber( "Light", "World Fixed Level", 0, true );
	worldDarkLevel_ = getNumber( "Light", "World Dark Level", 18, true );
	dungeonLightLevel_ = getNumber( "Light", "Dungeon Level", 18, true );

	// Path Finding
	pathfind4Follow_ = getBool( "Path Finding", "Activate for Following", true, true );
	pathfind4Combat_ = getBool( "Path Finding", "Activate for Combat", false, true );
	pathfindFollowRadius_ = getNumber( "Path Finding", "Follow Radius", 10, true );
	pathfindFleeRadius_ = getNumber( "Path Finding", "Flee Radius", 10, true );

	// Binary Saves
	binarySavepath_ = getString( "Binary Save Driver", "Save Path", "world.bin", true );
	binaryBackups_ = getNumber( "Binary Save Driver", "Number Of Backups", 5, true );
	binaryCompressBackups_ = getBool( "Binary Save Driver", "Compress Backups", false, true );

	// Racial Features
	elffullnightsight_ = getBool( "Racial Features", "Elves always have full nightsight", true, true );
	elfwisdombonus_ = getDouble( "Racial Features", "Elven Wisdom Bonus", 1.2, true );
	humanstrongback_ = getDouble( "Racial Features", "Human Strong Back", 1.5, true );
	humantough_ = getDouble( "Racial Features", "Human Tough", 0.6, true );

	// Monetary System
	usenewmonetary_ = getBool( "Monetary", "Use New Monetary", false, true );
	usereversedvaluable_ = getBool( "Monetary", "Use Reversed Valuable Base", false, true );
	defaultFirstCoin_ = getString( "Monetary", "Default First Coin", "eed", true );
	defaultSecondCoin_ = getString( "Monetary", "Default Second Coin", "ef0", true );
	defaultThirdCoin_ = getString( "Monetary", "Default Third Coin", "eea", true );

	// Weather System
	enableWeather_ = getBool( "Weather", "Enable Weather System", false, true );
	rainDefaultChance_ = getNumber( "Weather", "Default Rain Chance", 10, true );
	snowDefaultChance_ = getNumber( "Weather", "Default Snow Chance", 2, true );
	rainDefaultDuration_ = getNumber( "Weather", "Default Rain Duration", 2, true );
	snowDefaultDuration_ = getNumber( "Weather", "Default Snow Duration", 2, true );
	dryDefaultDuration_ = getNumber( "Weather", "Default Dry Duration", 4, true );
	rainDefaultDurationRange_ = getNumber( "Weather", "Default Rain Duration Range", 1, true );
	snowDefaultDurationRange_ = getNumber( "Weather", "Default Snow Duration Range", 1, true );
	dryDefaultDurationRange_ = getNumber( "Weather", "Default Dry Duration Range", 1, true );
	minDefaultIntensity_ = getNumber( "Weather", "Minimum Default Intensity", 16, true );
	maxDefaultIntensity_ = getNumber( "Weather", "Maximum Default Intensity", 112, true );
	intensitybecomesstorm_ = getNumber( "Weather", "Intensity Value to Rain Becomes Storm", 100, true );
	defaultthunderchance_ = getNumber( "Weather", "Default Thunder Chance", 25, true );
	raychanceonthunder_ = getNumber( "Weather", "Ray Chance on Thunder", 8, true );
	enableWeatherSounds_ = getBool( "Weather", "Enable Weather Sounds", false, true );
	weathersoundsinterval_ = getNumber( "Weather", "Weather Sounds Interval", 20000, true );

	// Season System
	enableSeasons_ = getBool( "Season", "Enable Season System", false, true );
	enableDesolationAsSeason_ = getBool( "Season", "Enable Desolation as Season", false, true );
	daysToChageSeason_ = getNumber( "Season", "Days to Change Season", 90, true );

	// Skills
	disableFocus_ = getBool( "Skills", "Disable Focus", false, true );
	defaultFocusValue_ = getNumber( "Skills", "Default Focus Value", 500, true );
	snoopdelay_ = getNumber( "Skills", "Snoop Delay", 7, true );
	stealing_ = getBool( "Skills", "Stealing Enabled", true, true );

}

void cConfig::load()
{
	// Load data into binary format
	// If value not found, create key.
	readData();

	if ( !containGroup( "StartLocation" ) )
		setDefaultStartLocation();

	if ( !containGroup( "LoginServer" ) )
		setDefaultServerList();

	flush(); // if any key created, save it.

	// Reload Body Info
	CharBaseDefs::instance()->loadBodyInfo();

	cComponent::load();
}

void cConfig::unload()
{
	serverList_.clear();
	startLocation_.clear();
	flush();
	cComponent::unload();
}

void cConfig::reload()
{
	unload();
	load();
}

std::vector<StartLocation_st>& cConfig::startLocation()
{
	if ( startLocation_.empty() ) // Empty? Try to load
	{
		bool bKeepLooping = true;
		unsigned int i = 1;
		do
		{
			QString tmp = getString( "StartLocation", QString( "Location %1" ).arg( i++ ), "" ).simplified();
			bKeepLooping = ( tmp != "" );
			if ( bKeepLooping ) // valid data.
			{
				QStringList strList = tmp.split( "=" );
				if ( strList.size() == 2 )
				{
					StartLocation_st loc;
					loc.name = strList[0];
					QStringList strList2 = strList[1].trimmed().split( "," );
					
					if ( strList2.size() == 5 )
					{
						loc.pos.x = strList2[0].toUShort();
						loc.pos.y = strList2[1].toUShort();
						loc.pos.z = strList2[2].toShort();
						loc.pos.map = strList2[3].toUShort();
						loc.Desc = strList2[4].toUInt();
						startLocation_.push_back( loc );
					}
				}
			}
		}
		while ( bKeepLooping );
	}
	return startLocation_;
}

void cConfig::setDefaultStartLocation()
{
	setString( "StartLocation", "Location 1", "Yew=567,978,0,1,1075072");
	setString( "StartLocation", "Location 2", "Minoc=2477,407,15,1,1075073"  );
	setString( "StartLocation", "Location 3", "Britain=1496,1629,10,1,1075074" );
	setString( "StartLocation", "Location 4", "Moonglow=4404,1169,0,1,1075075" );
	setString( "StartLocation", "Location 5", "Trinsic=1844,2745,0,1,1075076" );
	setString( "StartLocation", "Location 6", "New Haven=3667,2625,0,1,1150168" );
	setString( "StartLocation", "Location 7", "Jhelom=1378,3817,0,1,1075078" );
	setString( "StartLocation", "Location 8", "Skara Brae=594,2227,0,1,1075079" );
	setString( "StartLocation", "Location 9", "Vesper=2771,977,0,1,1075080" );
	flush(); // save
}

void cConfig::setDefaultServerList()
{
	setString( "LoginServer", "Shard 1", "Your Shard Name=127.0.0.1,2592" );
	flush(); // save.
}

void cConfig::setSecondsPerUOMinute( unsigned int data )
{
	secondsPerUOMinute_ = data;
	setNumber( "Game Speed", "Seconds Per UO Minute", secondsPerUOMinute_ );
	flush();
}

void cConfig::setWorldBrightLevel( unsigned char data )
{
	worldBrightLevel_ = data;
	setNumber( "Light", "World Bright Level", data );
	flush();
}

void cConfig::setWorldFixedLevel( unsigned char data )
{
	worldFixedLevel_ = data;
	setNumber( "Light", "World Fixed Level", data );
	flush();
}

void cConfig::setWorldDarkLevel( unsigned char data )
{
	worldDarkLevel_ = data;
	setNumber( "Light", "World Dark Level", data );
	flush();
}

void cConfig::setDungeonLightLevel( unsigned char data )
{
	dungeonLightLevel_ = data;
	setNumber( "Light", "Dungeon Level", data );
	flush();
}

void cConfig::setMulPath( const QString& data )
{
	mulPath_ = data;
	setString( "General", "MulPath", data );
	flush();
}

void cConfig::setLogPath( const QString& data )
{
	logPath_ = data;
	setString( "General", "LogPath", data );
	flush();
}

unsigned char& cConfig::worldCurrentLevel()
{
	return worldCurrentLevel_;
}

void cConfig::guardsActive( bool enabled )
{
	guardsActive_ = enabled;
	setBool( "General", "Guards Enabled", enabled );
}

struct stGroupDoc
{
	const char* group;
	const char* documentation;
};

QString cConfig::getGroupDoc( const QString& group )
{
	static stGroupDoc group_doc[] =
	{
	{"AI", QT_TR_NOOP( "This group configures the NPC AI." )},
	{"Accounts", QT_TR_NOOP( "This group configures the account management." )},
	{"Database", QT_TR_NOOP( "This group configures access to the worldsave database." )},
	{"Binary Save Driver", QT_TR_NOOP("This group configures binary save driver options. You can skip these if you are not using \"binary\" as worldsave driver")},
	{"Combat", QT_TR_NOOP("Combat tweaking options")},
	{"Game Speed", QT_TR_NOOP("Game speed and timming settings. Change with caution, misconfiguration may cause lag or cpu hog")},
	{"General", QT_TR_NOOP("General settings")},
	{"LoginServer", QT_TR_NOOP("Configures the list of servers shown during the login process.")},
	{"Network", QT_TR_NOOP("Network settings.")},
	{"Path Finding", QT_TR_NOOP("AI Path finding algorithm tweaks. Change with caution.")},
	{0, 0}
	};

	// Try to find documentation for a group in our table
	unsigned int i = 0;

	while ( group_doc[i].group )
	{
		if ( group == group_doc[i].group )
		{
			return tr( group_doc[i].documentation );
		}
		++i;
	}

	return Preferences::getGroupDoc( group );
}

struct stEntryDoc
{
	const char* group;
	const char* entry;
	const char* documentation;
};

QString cConfig::getEntryDoc( const QString& group, const QString& entry )
{
	static stEntryDoc entry_doc[] =
	{
	{"Accounts", "Auto Create", QT_TR_NOOP( "If active login attempts with non-existing login names will create a new account automatically\n"
	"This is very usefull for new shards without account policy" )},
	{"Accounts", "Database Driver", QT_TR_NOOP( "Possible values are: sqlite, mysql, oci for oracle, odbc, psql for PostgreSQL, tds for Sybase Adaptative Server, db2 for IBM DB2 (v7.1 or higher), sqlite2 for Sqlite version 2.x and ibase for Borland Interbase\n"
	                                            "Avaliable drivers depends on your Qt Instalation thought" )},
	{"Accounts", "Maximum Number of Characters", QT_TR_NOOP( "Should not be more than 6, due to client restrictions" )},
	{"Accounts", "Use MD5 Hashed Passwords", QT_TR_NOOP( "This will store hashed passwords, increasing password security." )},
	{"Accounts", "Automatically Hash Loaded Passwords", QT_TR_NOOP( "If active, will convert older plain text passwords into MD5 hash" )},
	{0, 0, 0}
	};

	// Try to find documentation for an entry in our table
	unsigned int i = 0;

	while ( entry_doc[i].group && entry_doc[i].entry )
	{
		if ( group == entry_doc[i].group && entry == entry_doc[i].entry )
		{
			return tr( entry_doc[i].documentation );
		}
		++i;
	}

	return Preferences::getEntryDoc( group, entry );
}

QList<ServerList_st> cConfig::serverList()
{
	static unsigned int lastIpCheck = 0;
	static bool dynamicIP = false;
	if ( serverList_.empty() || ( dynamicIP && lastIpCheck <= Server::instance()->time() ) ) // Empty? Try to load
	{
		serverList_.clear();

		bool bKeepLooping = true;
		unsigned int i = 1;
		do
		{
			QString tmp = getString( "LoginServer", QString( "Shard %1" ).arg( i++ ), QString::null, false ).simplified();
			bKeepLooping = !tmp.isEmpty();
			if ( bKeepLooping ) // valid data.
			{
				QStringList strList = tmp.split( "=" );
				if ( strList.size() == 2 )
				{
					ServerList_st server;
					server.sServer = strList[0];
					QStringList strList2 = strList[1].trimmed().split( "," );
					QHostAddress host;
					host.setAddress( strList2[0] );
					server.address = QHostInfo::fromName( strList2[0] ).addresses().first();

					bool ok = false;
					server.uiPort = strList2[1].toUShort( &ok );
					if ( !ok )
						server.uiPort = 2593; // Unspecified defaults to 2593

					// This code will retrieve the first
					// valid Internet IP it finds
					// and replace a 0.0.0.0 with it
					if ( ( ( server.address.toIPv4Address() == 0 ) && ( lastIpCheck <= Server::instance()->time() ) ) )
					{
						dynamicIP = true;

						// We check for a new IP max. every 30 minutes
						// So we have a max. of 30 minutes downtime
						lastIpCheck = Server::instance()->time() + ( MY_CLOCKS_PER_SEC * 30 * 60 );

						server.address = internetAddress();

						// Fall back to localhost
						if ( !server.address.toIPv4Address() )
						{
							server.address.setAddress(0x7F000001);
						}
					}
					serverList_.push_back( server );
				}
			}
		}
		while ( bKeepLooping );
	}
	return serverList_;
}
