
#include "srvparams.h"

// Library Includes
#include "preferences.h"
#include "qstring.h"
#include "qstringlist.h"

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
	stealing				= getBool("General",	"Stealing Enabled",	true, true);			
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

	flush(); // if any key created, save it.
}

std::vector<ServerList_st>& const cSrvParams::serverList()
{
	if ( serverList_.empty() ) // Empty? Try to load
	{
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

std::vector<StartLocation_st>& const cSrvParams::startLocation()
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
