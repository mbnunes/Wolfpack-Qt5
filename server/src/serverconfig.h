/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
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
#include <QString>
#include <QStringList>
#include <QList>
#include <QHostAddress>

// Structs

struct StartLocation_st
{
	QString name;
	Coord pos;
};

struct ServerList_st
{
	quint16 uiIndex;
	QString sServer;
	quint8 uiFull;
	quint8 uiTime;
	QHostAddress address;
	quint16 uiPort;
};

// Constants
const unsigned int FIRST_YEAR = 1970;

class cConfig : public Preferences, public cComponent
{
	OBJECTDEF( cConfig )
protected:
	QString getGroupDoc( const QString& group );
	QString getEntryDoc( const QString& group, const QString& entry );

	QList<ServerList_st> serverList_;
	std::vector<StartLocation_st> startLocation_;

	// loaded data
	bool overwriteDefinitions_;
	bool antiSpeedHack_;
	unsigned int antiSpeedHackDelay_;
	unsigned int antiSpeedHackDelayMounted_;
	bool hashAccountPasswords_;
	bool convertUnhashedPasswords_;
	bool enableIndivNumberSlots_;
	bool allowUnencryptedClients_;
	bool allowStatRequest_;
	unsigned int skillcap_;
	bool showSkillTitles_;
	unsigned int statcap_;
	QString commandPrefix_;
	unsigned short objectDelay_;
	unsigned char mountRange_;
	bool stealing_;
	bool guardsActive_;
	bool autoAccountCreate_;
	QString autoAccountACL_;
	float checkNPCTime_;
	float checkFollowTime_;
	float checkTamedTime_;
	unsigned int itemDecayTime_;
	unsigned int npcCorpseDecayTime_;
	unsigned int playerCorpseDecayTime_;
	int niceLevel_;
	unsigned short loginPort_;
	bool usesMondainsLegacyMap_;
	bool enableFeluccaSeason_;
	bool enableTrammelSeason_;
	unsigned int logMask_;
	bool enableLogin_;
	unsigned short gamePort_;
	bool enableGame_;
	unsigned short skillDelay_;
	unsigned int hungerRate_;
	unsigned int hungerDamageRate_;
	unsigned char hungerDamage_;
	unsigned int tamedDisappear_;
	unsigned int shopRestock_;
	unsigned int snoopdelay_;
	unsigned short int quittime_;
	unsigned int spawnRegionCheckTime_;
	unsigned int secondsPerUOMinute_;
	QString databaseDriver_;
	QString databaseName_;
	QString databaseUsername_;
	QString databasePassword_;
	QString databaseHost_;
	int databasePort_;
	bool useDatabaseTransaction_;
	QString accountsDriver_;
	QString accountsName_;
	QString accountsUsername_;
	QString accountsPassword_;
	QString accountsHost_;
	int accountsPort_;
	int saveInterval_;
	QString mulPath_;
	QString logPath_;
	bool logRotate_;
	unsigned int showNpcTitles_;
	unsigned char maxCharsPerAccount_;
	bool refreshMaxValues_;
	bool newTooltipPackets_;
	bool sendAsciiNames_;
	bool payfrompackonly_;
	bool enableyoung_;
	bool instalogoutfromguarded_;
	unsigned int percentHitsAfterRess_;
	unsigned int percentManaAfterRess_;
	unsigned int percentStaminaAfterRess_;
	bool disableKarma_;
	bool disableFame_;
	float factorMaxHits_;
	float factorMaxMana_;
	float factorMaxStam_;
	bool simpleMaxHitsCalculation_;
	bool enableTimeChangeForItems_;
	bool elffullnightsight_;
	float elfwisdombonus_;
	float humanstrongback_;
	float humantough_;

	// Binary Save Driver
	unsigned int binaryBackups_;
	QString binarySavepath_;
	bool binaryCompressBackups_;

	// AI
	float checkAITime_;
	float checkAINPCsTime_;
	float checkAIITEMsTime_;
	unsigned int animalWildFleeRange_;
	unsigned int guardDispelTime_;
	unsigned int aiNPCsCheckRange_;
	unsigned int aiITEMsCheckRange_;

	// Combat
	unsigned char attack_distance_;
	bool showdamagedone_;
	bool showdamagereceived_;

	//Repsys
	long int murderdecay_;
	unsigned int maxkills_;
	int crimtime_;

	// Light
	unsigned char worldBrightLevel_;
	unsigned char worldFixedLevel_;
	unsigned char worldDarkLevel_;
	unsigned char dungeonLightLevel_;
	unsigned char worldCurrentLevel_;

	// Path Finding
	bool pathfind4Follow_;
	bool pathfind4Combat_;
	int pathfindFollowRadius_;
	int pathfindFleeRadius_;

	// New Monetary
	bool usenewmonetary_;
	bool usereversedvaluable_;
	QString defaultFirstCoin_;
	QString defaultSecondCoin_;
	QString defaultThirdCoin_;

	// Weather System
	bool enableWeather_;
	bool enableWeatherSounds_;
	int rainDefaultChance_;
	int snowDefaultChance_;
	int rainDefaultDuration_;
	int dryDefaultDuration_;
	int snowDefaultDuration_;
	int rainDefaultDurationRange_;
	int snowDefaultDurationRange_;
	int dryDefaultDurationRange_;
	int minDefaultIntensity_;
	int maxDefaultIntensity_;
	int intensitybecomesstorm_;
	int defaultthunderchance_;
	int raychanceonthunder_;
	int weathersoundsinterval_;

	// Season System
	bool enableSeasons_;
	bool enableDesolationAsSeason_;
	int daysToChageSeason_;

public:
	cConfig();

	void load();
	void unload();
	void reload();

	QList<ServerList_st> serverList(); // read-only
	std::vector<StartLocation_st>& startLocation();

	// gets
	bool usesMondainsLegacyMap() const;
	bool enableFeluccaSeason() const;
	bool enableTrammelSeason() const;
	bool hashAccountPasswords() const;
	bool convertUnhashedPasswords() const;
	bool enableIndivNumberSlots() const;
	bool showSkillTitles() const;
	bool allowUnencryptedClients() const;
	bool allowStatRequest() const;
	unsigned int skillcap() const;
	unsigned int statcap() const;
	QString commandPrefix() const;
	bool stealingEnabled() const;
	bool guardsActive() const;
	void guardsActive( bool );
	unsigned short objectDelay() const;
	bool autoAccountCreate() const;
	float checkNPCTime() const;
	float checkAITime() const;
	float checkAINPCsTime() const;
	float checkAIITEMsTime() const;
	unsigned int animalWildFleeRange() const;
	unsigned int aiNPCsCheckRange() const;
	unsigned int aiITEMsCheckRange() const;
	float checkFollowTime() const;
	float checkTamedTime() const;
	float factorMaxHits() const;
	float factorMaxMana() const;
	float factorMaxStam() const;
	bool simpleMaxHitsCalculation() const;
	bool enableTimeChangeForItems() const;

	bool refreshMaxValues() const
	{
		return refreshMaxValues_;
	}
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
	unsigned int percentHitsAfterRess() const
	{
		return percentHitsAfterRess_;
	}
	unsigned int percentManaAfterRess() const
	{
		return percentManaAfterRess_;
	}
	unsigned int percentStaminaAfterRess() const
	{
		return percentStaminaAfterRess_;
	}
	bool disableKarma() const
	{
		return disableKarma_;
	}
	bool disableFame() const
	{
		return disableFame_;
	}
	bool newTooltipPackets() const;
	int niceLevel() const;
	unsigned int itemDecayTime() const;
	unsigned int npcCorpseDecayTime() const;
	unsigned int playerCorpseDecayTime() const;
	unsigned char mountRange() const;
	unsigned short skillDelay() const;
	unsigned int hungerRate() const;
	unsigned int hungerDamageRate() const;
	unsigned char hungerDamage() const;
	float boatSpeed() const;
	unsigned int tamedDisappear() const;

	inline unsigned int binaryBackups() const
	{
		return binaryBackups_;
	}

	inline const QString& binarySavepath() const
	{
		return binarySavepath_;
	}

	inline bool binaryCompressBackups() const
	{
		return binaryCompressBackups_;
	}

	unsigned int shopRestock() const;
	unsigned int snoopdelay() const;
	unsigned short int quittime() const;
	unsigned int default_jail_time() const;
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
	unsigned int showNpcTitles() const;
	bool overwriteDefinitions() const;
	bool sendAsciiNames() const;
	bool payfrompackonly() const;
	bool enableyoung() const;
	bool instalogoutfromguarded() const;

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
	bool useDatabaseTransaction() const;
	int databasePort() const;
	int accountsPort() const;
	unsigned int maxCharsPerAccount() const;

	// Combat
	unsigned char attack_distance() const;
	bool showDamageDone() const;
	bool showDamageReceived() const;

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
	bool pathfind4Follow() const;
	bool pathfind4Combat() const;
	int pathfindFollowRadius() const;
	int pathfindFleeRadius() const;

	// AI
	unsigned int guardDispelTime() const;

	// Racial Features
	bool elffullnightsight() const;
	float elfwisdombonus() const;
	float humanstrongback() const;
	float humantough() const;

	// New Monetary system
	bool usenewmonetary() const;
	bool usereversedvaluable() const;
	QString defaultFirstCoin() const;
	QString defaultSecondCoin() const;
	QString defaultThirdCoin() const;

	// Weather System
	bool enableWeather() const;
	bool enableWeatherSounds() const;
	int rainDefaultChance() const;
	int snowDefaultChance() const;
	int rainDefaultDuration() const;
	int snowDefaultDuration() const;
	int dryDefaultDuration() const;
	int rainDefaultDurationRange() const;
	int snowDefaultDurationRange() const;
	int dryDefaultDurationRange() const;
	int minDefaultIntensity() const;
	int maxDefaultIntensity() const;
	int intensityBecomesStorm() const;
	int defaultThunderChance() const;
	int rayChanceonThunder() const;
	int weatherSoundsInterval() const;

	// Season System
	bool enableSeasons() const;
	bool enableDesolationAsSeason() const;
	int daysToChageSeason() const;

private:
	void setDefaultStartLocation();
	void setDefaultServerList();
	void readData();
};

// inline members

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

inline float cConfig::checkNPCTime() const
{
	return checkNPCTime_;
}

inline float cConfig::checkAITime() const
{
	return checkAITime_;
}

inline float cConfig::checkAINPCsTime() const
{
	return checkAINPCsTime_;
}

inline float cConfig::checkAIITEMsTime() const
{
	return checkAIITEMsTime_;
}

inline unsigned int cConfig::animalWildFleeRange() const
{
	return animalWildFleeRange_;
}

inline unsigned int cConfig::aiNPCsCheckRange() const
{
	return aiNPCsCheckRange_;
}

inline unsigned int cConfig::aiITEMsCheckRange() const
{
	return aiITEMsCheckRange_;
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

inline unsigned int cConfig::playerCorpseDecayTime() const
{
	return playerCorpseDecayTime_;
}

inline unsigned int cConfig::npcCorpseDecayTime() const
{
	return npcCorpseDecayTime_;
}

inline unsigned short cConfig::skillDelay() const
{
	return skillDelay_;
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

inline unsigned int cConfig::tamedDisappear() const
{
	return tamedDisappear_;
}

inline unsigned int cConfig::shopRestock() const
{
	return shopRestock_;
}

inline unsigned char cConfig::attack_distance() const
{
	return attack_distance_;
}

inline bool cConfig::showDamageDone() const
{
	return showdamagedone_;
}

inline bool cConfig::showDamageReceived() const
{
	return showdamagereceived_;
}

inline unsigned int cConfig::snoopdelay() const
{
	return snoopdelay_;
}

inline unsigned short int cConfig::quittime() const
{
	return quittime_;
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
inline int cConfig::databasePort() const
{
	return databasePort_;
}

inline int cConfig::accountsPort() const
{
	return accountsPort_;
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

inline bool cConfig::useDatabaseTransaction() const
{
	return useDatabaseTransaction_;
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

inline bool cConfig::enableIndivNumberSlots() const
{
	return enableIndivNumberSlots_;
}

inline bool cConfig::usesMondainsLegacyMap() const
{
	return usesMondainsLegacyMap_;
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

inline unsigned char cConfig::mountRange() const
{
	return mountRange_;
}

inline bool cConfig::newTooltipPackets() const
{
	return newTooltipPackets_;
}

inline bool cConfig::sendAsciiNames() const
{
	return sendAsciiNames_;
}

inline bool cConfig::payfrompackonly() const
{
	return payfrompackonly_;
}

inline bool cConfig::enableyoung() const
{
	return enableyoung_;
}

inline bool cConfig::instalogoutfromguarded() const
{
	return instalogoutfromguarded_;
}

inline unsigned int cConfig::maxCharsPerAccount() const
{
	return ( unsigned int ) maxCharsPerAccount_;
}
inline float cConfig::factorMaxHits() const
{
	return factorMaxHits_;
}
inline float cConfig::factorMaxMana() const
{
	return factorMaxMana_;
}
inline float cConfig::factorMaxStam() const
{
	return factorMaxStam_;
}
inline bool cConfig::simpleMaxHitsCalculation() const
{
	return simpleMaxHitsCalculation_;
}
inline bool cConfig::enableTimeChangeForItems() const
{
	return enableTimeChangeForItems_;
}
// Racial features
inline bool cConfig::elffullnightsight() const
{
	return elffullnightsight_;
}
inline float cConfig::elfwisdombonus() const
{
	return elfwisdombonus_;
}
inline float cConfig::humanstrongback() const
{
	return humanstrongback_;
}
inline float cConfig::humantough() const
{
	return humantough_;
}
// Monetary System
inline bool cConfig::usenewmonetary() const
{
	return usenewmonetary_;
}
inline bool cConfig::usereversedvaluable() const
{
	return usereversedvaluable_;
}
inline QString cConfig::defaultFirstCoin() const
{
	return defaultFirstCoin_;
}
inline QString cConfig::defaultSecondCoin() const
{
	return defaultSecondCoin_;
}
inline QString cConfig::defaultThirdCoin() const
{
	return defaultThirdCoin_;
}
// Weather System
inline bool cConfig::enableWeather() const
{
	return enableWeather_;
}
inline bool cConfig::enableWeatherSounds() const
{
	return enableWeatherSounds_;
}
inline int cConfig::rainDefaultChance() const
{
	return rainDefaultChance_;
}
inline int cConfig::snowDefaultChance() const
{
	return snowDefaultChance_;
}
inline int cConfig::rainDefaultDuration() const
{
	return rainDefaultDuration_;
}
inline int cConfig::snowDefaultDuration() const
{
	return snowDefaultDuration_;
}
inline int cConfig::dryDefaultDuration() const
{
	return dryDefaultDuration_;
}
inline int cConfig::rainDefaultDurationRange() const
{
	return rainDefaultDurationRange_;
}
inline int cConfig::snowDefaultDurationRange() const
{
	return snowDefaultDurationRange_;
}
inline int cConfig::dryDefaultDurationRange() const
{
	return dryDefaultDurationRange_;
}

inline int cConfig::minDefaultIntensity() const
{
	return minDefaultIntensity_;
}

inline int cConfig::maxDefaultIntensity() const
{
	return maxDefaultIntensity_;
}
inline int cConfig::intensityBecomesStorm() const
{
	return intensitybecomesstorm_;
}
inline int cConfig::defaultThunderChance() const
{
	return defaultthunderchance_;
}
inline int cConfig::rayChanceonThunder() const
{
	return raychanceonthunder_;
}
inline int cConfig::weatherSoundsInterval() const
{
	return weathersoundsinterval_;
}
// Season System
inline bool cConfig::enableSeasons() const
{
	return enableSeasons_;
}
inline bool cConfig::enableDesolationAsSeason() const
{
	return enableDesolationAsSeason_;
}
inline int cConfig::daysToChageSeason() const
{
	return daysToChageSeason_;
}

typedef Singleton<cConfig> Config;

#endif //__SRVPARAMS_H___

