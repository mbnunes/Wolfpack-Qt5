/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#if !defined (__CONFIG_H__)
#define __CONFIG_H__

// System headers
#include <vector>

// Our own headers
#include "preferences.h"
#include "server.h"
#include "singleton.h"
#include "coord.h"

// Library Headers
#include <qstring.h>
#include <qstringlist.h>

// Structs

struct StartLocation_st
{
	QString name;
	Coord_cl pos;
};

struct ServerList_st
{
	Q_UINT16 uiIndex;
	QString sServer;
	Q_UINT8 uiFull;
	Q_UINT8 uiTime;
	Q_UINT32 ip;
	QCString sIP;
	Q_UINT16 uiPort;
};

// Constants
const unsigned int FIRST_YEAR = 1970;

class cConfig : public Preferences, public cComponent
{
protected:
	QString getGroupDoc( const QString& group );
	QString getEntryDoc( const QString& group, const QString& entry );

	std::vector<ServerList_st> serverList_;
	std::vector<StartLocation_st> startLocation_;

	// loaded data
	bool overwriteDefinitions_;
	bool antiSpeedHack_;
	unsigned int antiSpeedHackDelay_;
	unsigned int antiSpeedHackDelayMounted_;
	bool hashAccountPasswords_;
	bool convertUnhashedPasswords_;
	bool allowUnencryptedClients_;
	bool allowStatRequest_;
	unsigned int skillcap_;
	bool showSkillTitles_;
	unsigned int statcap_;
	QString commandPrefix_;
	unsigned int skillAdvanceModifier_;
	unsigned int statsAdvanceModifier_;
	unsigned short objectDelay_;
	bool stealing_;
	bool guardsActive_;
	bool saveSpawns_;
	bool dontStackSpawnedObjects_;
	bool autoAccountCreate_;
	float checkNPCTime_;
	float checkFollowTime_;
	float checkTamedTime_;
	unsigned int itemDecayTime_;
	unsigned int corpseDecayTime_;
	int niceLevel_;
	unsigned short loginPort_;
	bool enableFeluccaSeason_;
	bool enableTrammelSeason_;
	unsigned int logMask_;
	bool enableLogin_;
	unsigned short gamePort_;
	bool enableGame_;
	unsigned int playercorpsedecaymultiplier_;
	bool lootdecayswithcorpse_;
	float invisTimer_;
	unsigned short skillDelay_;
	int skillLevel_;
	unsigned int poisonTimer_;
	signed int maxStealthSteps_;
	unsigned int runningStamSteps_;
	unsigned int hungerRate_;
	unsigned int hungerDamageRate_;
	unsigned char hungerDamage_;
	float boatSpeed_;
	unsigned int tamedDisappear_;
	unsigned int houseInTown_;
	unsigned int shopRestock_;
	unsigned int snoopdelay_;
	unsigned short int quittime_;
	unsigned long int housedecay_secs_;
	unsigned int spawnRegionCheckTime_;
	unsigned int secondsPerUOMinute_;
	bool cacheMulFiles_;
	QString databaseDriver_;
	QString databaseName_;
	QString databaseUsername_;
	QString databasePassword_;
	QString databaseHost_;
	QString accountsDriver_;
	QString accountsName_;
	QString accountsUsername_;
	QString accountsPassword_;
	QString accountsHost_;
	int saveInterval_;
	QString mulPath_;
	QString logPath_;
	bool logRotate_;
	bool categoryTagAddMenu_;
	double npcMoveTime_;
	double tamedNpcMoveTime_;
	unsigned int showNpcTitles_;
	unsigned char maxCharsPerAccount_;

	// Binary Save Driver
	unsigned int binaryBackups_;	
	QString binarySavepath_;
	bool binaryCompressBackups_;

	// AI
	float checkAITime_;
	unsigned int animalWildFleeRange_;
	unsigned int guardDispelTime_;

	// Combat
	int attackstamina_;
	unsigned char attack_distance_;

	//Repsys
	long int murderdecay_;
	unsigned int maxkills_;
	int crimtime_;

	// Resources
	unsigned int resourceitemdecaytime_;

	// Light
	unsigned char worldBrightLevel_;
	unsigned char worldFixedLevel_;
	unsigned char worldDarkLevel_;
	unsigned char dungeonLightLevel_;
	unsigned char worldCurrentLevel_;

	// Path Finding
	int pathfindMaxSteps_;
	int pathfindMaxIterations_;
	bool pathfind4Follow_;
	bool pathfind4Combat_;
	int pathfindFollowRadius_;
	float pathfindFollowMinCost_;
	int pathfindFleeRadius_;

public:
	cConfig();

	void load();
	void unload();
	void reload();

	std::vector<ServerList_st>& serverList(); // read-only
	std::vector<StartLocation_st>& startLocation();

	// gets
	bool enableFeluccaSeason() const;
	bool enableTrammelSeason() const;
	bool hashAccountPasswords() const;
	bool convertUnhashedPasswords() const;
	bool showSkillTitles() const;
	bool allowUnencryptedClients() const;
	bool allowStatRequest() const;
	unsigned int skillcap() const;
	unsigned int statcap() const;
	QString commandPrefix() const;
	unsigned int skillAdvanceModifier() const;
	unsigned int statsAdvanceModifier() const;
	bool stealingEnabled() const;
	bool guardsActive() const;
	void guardsActive( bool );
	unsigned short objectDelay() const;
	bool autoAccountCreate() const;
	bool saveSpawns() const;
	float checkNPCTime() const;
	float checkAITime() const;
	unsigned int animalWildFleeRange() const;
	float checkFollowTime() const;
	float checkTamedTime() const;
	bool antiSpeedHack() const
	{
		return antiSpeedHack_;
	}
	unsigned int antiSpeedHackDelay() const
	{
		return antiSpeedHackDelay_;
	}
	unsigned int antiSpeedHackDelayMounted() const
	{
		return antiSpeedHackDelayMounted_;
	}
	int niceLevel() const;
	unsigned int itemDecayTime() const;
	unsigned int corpseDecayTime() const;
	bool lootdecayswithcorpse() const;
	float invisTimer() const;
	unsigned short skillDelay() const;
	unsigned int poisonTimer() const;
	signed int maxStealthSteps() const;
	unsigned int runningStamSteps() const;
    unsigned int hungerRate() const;
	unsigned int hungerDamageRate() const;
	unsigned char hungerDamage() const;
	float boatSpeed() const;
	unsigned int tamedDisappear() const;
	unsigned int houseInTown() const;
	inline bool dontStackSpawnedObjects() const
	{
		return dontStackSpawnedObjects_;
	}

	inline unsigned int binaryBackups() const
	{
		return binaryBackups_;
	}

	inline const QString &binarySavepath() const {
		return binarySavepath_;
	}

	inline bool binaryCompressBackups() const {
		return binaryCompressBackups_;
	}

	unsigned int shopRestock() const;
	unsigned int snoopdelay() const;
	unsigned short int quittime() const;
	unsigned long int housedecay_secs() const;
	unsigned int default_jail_time() const;
	bool cacheMulFiles() const;
	unsigned int spawnRegionCheckTime() const;
	unsigned int secondsPerUOMinute() const;
	unsigned int logMask() const;
	void setSecondsPerUOMinute( unsigned int );
	int saveInterval() const;
	bool heartBeat() const;
	int defaultpriv2() const;
	QString mulPath() const;
	QString logPath() const;
	bool logRotate() const;
	void setMulPath( const QString& data );
	void setLogPath( const QString& data );
	bool addMenuByCategoryTag() const;
	double npcMoveTime() const;
	double tamedNpcMoveTime() const;
	unsigned int showNpcTitles() const;
	bool overwriteDefinitions() const;

	// Persistency Module
	QString databaseDriver() const;
	QString databaseHost() const;
	QString databasePassword() const;
	QString databaseUsername() const;
	QString databaseName() const;
	QString accountsDriver() const;
	QString accountsHost() const;
	QString accountsPassword() const;
	QString accountsUsername() const;
	QString accountsName() const;
	unsigned int maxCharsPerAccount() const;

	// Combat
	int attackstamina() const;
	unsigned char attack_distance() const;

	// Repsys
	long int murderdecay() const;
	unsigned int maxkills() const;
	int crimtime() const;

	// Resources
	unsigned int resitemdecaytime() const;

	// Light
	unsigned char worldBrightLevel() const;
	unsigned char worldFixedLevel() const;
	unsigned char worldDarkLevel() const;
	unsigned char dungeonLightLevel() const;
	unsigned char& worldCurrentLevel();

	void setWorldBrightLevel( unsigned char );
	void setWorldFixedLevel( unsigned char );
	void setWorldDarkLevel( unsigned char );
	void setDungeonLightLevel( unsigned char );

	// Network
	unsigned short loginPort() const;
	unsigned short gamePort() const;
	bool enableLogin() const;
	bool enableGame() const;

	// Path Finding
	int pathfindMaxSteps() const;
	int pathfindMaxIterations() const;
	bool pathfind4Follow() const;
	bool pathfind4Combat() const;
	int pathfindFollowRadius() const;
	float pathfindFollowMinCost() const;
	int pathfindFleeRadius() const;

	// AI
	unsigned int guardDispelTime() const;

private:
	void setDefaultStartLocation();
	void setDefaultServerList();
	void readData();
};

// inline members

inline double cConfig::npcMoveTime() const
{
	return npcMoveTime_;
}

inline double cConfig::tamedNpcMoveTime() const
{
	return tamedNpcMoveTime_;
}

inline unsigned int cConfig::skillcap() const
{
	return skillcap_;
}

inline unsigned int cConfig::statcap() const
{
	return statcap_;
}

inline unsigned int cConfig::logMask() const
{
	return logMask_;
}

inline QString cConfig::commandPrefix() const
{
	return commandPrefix_;
}

inline unsigned int cConfig::skillAdvanceModifier() const
{
	return skillAdvanceModifier_;
}

inline unsigned int cConfig::statsAdvanceModifier() const
{
	return statsAdvanceModifier_;
}

inline bool cConfig::stealingEnabled() const
{
	return stealing_;
}

inline bool cConfig::guardsActive() const
{
	return guardsActive_;
}

inline unsigned short cConfig::objectDelay() const
{
	return objectDelay_;
}

inline bool cConfig::allowUnencryptedClients() const
{
	return allowUnencryptedClients_;
}

inline bool cConfig::allowStatRequest() const
{
	return allowStatRequest_;
}

inline bool cConfig::autoAccountCreate() const
{
	return autoAccountCreate_;
}

inline bool cConfig::saveSpawns() const
{
	return saveSpawns_;
}

inline float cConfig::checkNPCTime() const
{
	return checkNPCTime_;
}

inline float cConfig::checkAITime() const
{
	return checkAITime_;
}

inline unsigned int cConfig::animalWildFleeRange() const
{
	return animalWildFleeRange_;
}

inline float cConfig::checkFollowTime() const
{
	return checkFollowTime_;
}

inline float cConfig::checkTamedTime() const
{
	return checkTamedTime_;
}

inline int cConfig::niceLevel() const
{
	return niceLevel_;
}

inline unsigned int cConfig::itemDecayTime() const
{
	return itemDecayTime_;
}

inline unsigned int cConfig::corpseDecayTime() const
{
	return corpseDecayTime_;
}

inline bool cConfig::lootdecayswithcorpse() const
{
	return lootdecayswithcorpse_;
}

inline unsigned short cConfig::skillDelay() const
{
	return skillDelay_;
}

inline unsigned int cConfig::poisonTimer() const
{
	return poisonTimer_;
}

inline signed int cConfig::maxStealthSteps() const
{
	return maxStealthSteps_;
}

inline unsigned int cConfig::runningStamSteps() const
{
	return runningStamSteps_;
}

inline unsigned int cConfig::hungerRate() const
{
	return hungerRate_;
}

inline unsigned int cConfig::hungerDamageRate() const
{
	return hungerDamageRate_;
}

inline unsigned char cConfig::hungerDamage() const
{
	return hungerDamage_;
}

inline float cConfig::boatSpeed() const
{
	return boatSpeed_;
}

inline unsigned int cConfig::tamedDisappear() const
{
	return tamedDisappear_;
}

inline unsigned int cConfig::houseInTown() const
{
	return houseInTown_;
}

inline unsigned int cConfig::shopRestock() const
{
	return shopRestock_;
}

inline int cConfig::attackstamina() const
{
	return attackstamina_;
}

inline unsigned char cConfig::attack_distance() const
{
	return attack_distance_;
}

inline unsigned int cConfig::snoopdelay() const
{
	return snoopdelay_;
}

inline unsigned short int cConfig::quittime() const
{
	return quittime_;
}

inline unsigned long int cConfig::housedecay_secs() const
{
	return housedecay_secs_;
}

inline long int cConfig::murderdecay() const
{
	return murderdecay_;
}

inline unsigned int cConfig::maxkills() const
{
	return maxkills_;
}

inline int cConfig::crimtime() const
{
	return crimtime_;
}

inline unsigned int cConfig::resitemdecaytime() const
{
	return resourceitemdecaytime_;
}

inline bool cConfig::cacheMulFiles() const
{
	return cacheMulFiles_;
}

inline unsigned int cConfig::spawnRegionCheckTime() const
{
	return spawnRegionCheckTime_;
}

inline unsigned int cConfig::secondsPerUOMinute() const
{
	return secondsPerUOMinute_;
}

inline unsigned char cConfig::worldBrightLevel() const
{
	return worldBrightLevel_;
}

inline unsigned char cConfig::worldFixedLevel() const
{
	return worldFixedLevel_;
}

inline unsigned char cConfig::worldDarkLevel() const
{
	return worldDarkLevel_;
}

inline unsigned char cConfig::dungeonLightLevel() const
{
	return dungeonLightLevel_;
}

inline int cConfig::saveInterval() const
{
	return saveInterval_;
}

inline unsigned int cConfig::showNpcTitles() const
{
	return showNpcTitles_;
}

inline QString cConfig::databaseDriver() const
{
	return databaseDriver_;
}

inline QString cConfig::databaseHost() const
{
	return databaseHost_;
}

inline QString cConfig::databaseName() const
{
	return databaseName_;
}

inline QString cConfig::databaseUsername() const
{
	return databaseUsername_;
}

inline QString cConfig::databasePassword() const
{
	return databasePassword_;
}

inline QString cConfig::accountsDriver() const
{
	return accountsDriver_;
}

inline QString cConfig::accountsHost() const
{
	return accountsHost_;
}

inline QString cConfig::accountsName() const
{
	return accountsName_;
}

inline QString cConfig::accountsUsername() const
{
	return accountsUsername_;
}

inline QString cConfig::accountsPassword() const
{
	return accountsPassword_;
}

inline bool cConfig::showSkillTitles() const
{
	return showSkillTitles_;
}

inline bool cConfig::enableLogin() const
{
	return enableLogin_;
}

inline bool cConfig::enableGame() const
{
	return enableGame_;
}

inline unsigned short cConfig::gamePort() const
{
	return gamePort_;
}

inline unsigned short cConfig::loginPort() const
{
	return loginPort_;
}

inline bool cConfig::addMenuByCategoryTag() const
{
	return categoryTagAddMenu_;
}

inline int cConfig::pathfindMaxSteps() const
{
	return pathfindMaxSteps_;
}

inline bool cConfig::pathfind4Follow() const
{
	return pathfind4Follow_;
}

inline bool cConfig::pathfind4Combat() const
{
	return pathfind4Combat_;
}

inline int cConfig::pathfindFollowRadius() const
{
	return pathfindFollowRadius_;
}

inline float cConfig::pathfindFollowMinCost() const
{
	return pathfindFollowMinCost_;
}

inline int cConfig::pathfindFleeRadius() const
{
	return pathfindFleeRadius_;
}

inline QString cConfig::logPath() const
{
	return logPath_;
}

inline bool cConfig::logRotate() const
{
	return logRotate_;
}

inline int cConfig::pathfindMaxIterations() const
{
	return pathfindMaxIterations_;
}

inline unsigned int cConfig::guardDispelTime() const
{
	return guardDispelTime_;
}

inline bool cConfig::hashAccountPasswords() const
{
	return hashAccountPasswords_;
}

inline bool cConfig::convertUnhashedPasswords() const
{
	return convertUnhashedPasswords_;
}

inline bool cConfig::enableFeluccaSeason() const
{
	return enableFeluccaSeason_;
}

inline bool cConfig::enableTrammelSeason() const
{
	return enableTrammelSeason_;
}

inline bool cConfig::overwriteDefinitions() const
{
	return overwriteDefinitions_;
}

inline unsigned int cConfig::maxCharsPerAccount() const
{
	return ( unsigned int ) maxCharsPerAccount_;
}

typedef SingletonHolder<cConfig> Config;

#endif //__SRVPARAMS_H___

