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

#if !defined (__SRVPARAMS_H__)
#define __SRVPARAMS_H__

// System headers
#include <vector>


// Our own headers
#include "structs.h"
#include "coord.h"

// Library Headers
#include "qstring.h"
#include "preferences.h"
#include "qstringlist.h"

// Forward Declarations
class Preferences;

// Structs

struct StartLocation_st
{
	QString name;
	Coord_cl pos;
};

// Constants
const unsigned int FIRST_YEAR = 1970;

class cSrvParams : public Preferences
{
protected:
	std::vector<ServerList_st> serverList_;
	std::vector<StartLocation_st> startLocation_;
	
	// loaded data
	bool allowUnencryptedClients_;
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
	bool autoAccountCreate_;
	float checkItemTime_;
	float checkNPCTime_;
	float checkFollowTime_;
	float checkTamedTime_;
	unsigned int itemDecayTime_;
	unsigned int corpseDecayTime_;
	int niceLevel_;
	unsigned short loginPort_;
	bool enableLogin_;
	unsigned short gamePort_;
	bool enableGame_;
	unsigned int playercorpsedecaymultiplier_;
	bool lootdecayswithcorpse_;
	float invisTimer_;
	unsigned short skillDelay_;
	int skillLevel_;
	unsigned short bandageDelay_;
	bool bandageInCombat_;
	unsigned int poisonTimer_;
	signed int maxStealthSteps_;
	unsigned int runningStamSteps_;
	unsigned int hungerRate_;
	unsigned int hungerDamageRate_;
	unsigned char hungerDamage_;
	float boatSpeed_;
	int persecute_;
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
	bool categoryTagAddMenu_;
	double npcMoveTime_;
	double tamedNpcMoveTime_;
	unsigned int showNpcTitles_;

	// AI
	float checkAITime_;
	unsigned int animalWildFleeRange_;

	// Regenerate
	unsigned int hitpointrate_;
	unsigned int staminarate_;
	unsigned int manarate_;
	char armoraffectmana_;

	// Combat
	int attackstamina_;
	unsigned char attack_distance_;

	//Repsys
	long int		murderdecay_;
	unsigned int	maxkills_;
	int				crimtime_;

	// Resources
	unsigned int resourceitemdecaytime_;

	// Light
	unsigned char worldBrightLevel_;
	unsigned char worldFixedLevel_;
	unsigned char worldDarkLevel_;
	unsigned char dungeonLightLevel_;
	unsigned char worldCurrentLevel_;

	// Path Finding
	int		pathfindMaxSteps_;
	int		pathfindMaxIterations_;
	bool	pathfind4Follow_;
	bool	pathfind4Combat_;
	int		pathfindFollowRadius_;
	float	pathfindFollowMinCost_;
	int		pathfindFleeRadius_;

public:
    cSrvParams( const QString& filename, const QString& format, const QString& version );

	virtual void reload();
	
	std::vector<ServerList_st>& serverList(); // read-only
	std::vector<StartLocation_st>& startLocation();

	// gets
	bool showSkillTitles() const;
	bool allowUnencryptedClients() const;
	unsigned int skillcap() const;
	unsigned int statcap() const;
	QString commandPrefix() const;
	unsigned int skillAdvanceModifier() const;
	unsigned int statsAdvanceModifier() const;
	bool stealingEnabled() const;
	bool guardsActive() const;
	void guardsActive(bool);
	unsigned short objectDelay() const;
	bool autoAccountCreate() const;
	bool saveSpawns() const;
	float checkItemTime() const;
	float checkNPCTime() const;
	float checkAITime() const;
	unsigned int animalWildFleeRange() const;
	float checkFollowTime() const;
	float checkTamedTime() const;
	int niceLevel() const;
	unsigned int itemDecayTime() const;
	unsigned int corpseDecayTime() const;
	bool lootdecayswithcorpse() const;
	float invisTimer() const;
	unsigned short skillDelay() const;
	unsigned short bandageDelay() const;
	bool bandageInCombat() const;
	unsigned int poisonTimer() const;
	signed int maxStealthSteps() const;
	unsigned int runningStamSteps() const;
	unsigned int hungerRate() const;
	unsigned int hungerDamageRate() const;
	unsigned char hungerDamage() const;
	float boatSpeed() const;
	int persecute() const;
	unsigned int tamedDisappear() const;
	unsigned int houseInTown() const;
	unsigned int shopRestock() const;
	unsigned int snoopdelay() const;
	unsigned short int quittime() const;
	unsigned long int housedecay_secs() const;
	unsigned int default_jail_time() const;
	bool cacheMulFiles() const;
	unsigned int spawnRegionCheckTime() const;
	unsigned int secondsPerUOMinute() const;
	void setSecondsPerUOMinute( unsigned int );
	int saveInterval() const;
	bool heartBeat() const;
	int defaultpriv2() const;
	QString mulPath() const;
	QString logPath() const;
	void setMulPath( const QString& data );
	void setLogPath( const QString& data );
	bool addMenuByCategoryTag() const;
	double npcMoveTime() const;
	double tamedNpcMoveTime() const;
	unsigned int showNpcTitles() const;

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

	// Regenerate
	unsigned int hitpointrate() const;
	unsigned int staminarate() const;
	unsigned int manarate() const;
	char armoraffectmana() const;

	// Combat
	int attackstamina() const;
	unsigned char attack_distance() const;

	// Repsys
	long int		murderdecay() const;
	unsigned int	maxkills() const;
	int				crimtime() const;

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

private:
	void setDefaultStartLocation();
	void setDefaultServerList();
	void readData();
};

// inline members

inline double cSrvParams::npcMoveTime() const
{
	return npcMoveTime_;
}

inline double cSrvParams::tamedNpcMoveTime() const
{
	return tamedNpcMoveTime_;
}

inline unsigned int cSrvParams::skillcap() const
{
	return skillcap_;
}

inline unsigned int cSrvParams::statcap() const
{
	return statcap_;
}

inline QString cSrvParams::commandPrefix() const
{
	return commandPrefix_;
}

inline unsigned int cSrvParams::skillAdvanceModifier() const
{
	return skillAdvanceModifier_;
}

inline unsigned int cSrvParams::statsAdvanceModifier() const
{
	return statsAdvanceModifier_;
}

inline bool cSrvParams::stealingEnabled() const
{
	return stealing_;
}

inline bool cSrvParams::guardsActive() const
{
	return guardsActive_;
}

inline void cSrvParams::guardsActive(bool enabled)
{
	guardsActive_ = enabled;
	setBool("General" "Guards Enabled", enabled);
}

inline unsigned short cSrvParams::objectDelay() const
{
	return objectDelay_;
}

inline bool cSrvParams::allowUnencryptedClients() const
{
	return allowUnencryptedClients_;
}

inline bool cSrvParams::autoAccountCreate() const
{
	return autoAccountCreate_;
}

inline bool cSrvParams::saveSpawns() const
{
	return saveSpawns_;
}

inline float cSrvParams::checkItemTime() const
{
	return checkItemTime_;
}

inline float cSrvParams::checkNPCTime() const
{
	return checkNPCTime_;
}

inline float cSrvParams::checkAITime() const
{
	return checkAITime_;
}

inline unsigned int cSrvParams::animalWildFleeRange() const
{
	return animalWildFleeRange_;
}

inline float cSrvParams::checkFollowTime() const
{
	return checkFollowTime_;
}

inline float cSrvParams::checkTamedTime() const
{
	return checkTamedTime_;
}

inline int cSrvParams::niceLevel() const
{
	return niceLevel_;
}

inline unsigned int cSrvParams::itemDecayTime() const
{
	return itemDecayTime_;
}

inline unsigned int cSrvParams::corpseDecayTime() const
{
	return corpseDecayTime_;
}

inline bool cSrvParams::lootdecayswithcorpse() const
{
	return lootdecayswithcorpse_;
}

inline unsigned short cSrvParams::skillDelay() const
{
	return skillDelay_;
}

inline unsigned short cSrvParams::bandageDelay() const
{
	return bandageDelay_;
}

inline bool cSrvParams::bandageInCombat() const
{
	return bandageInCombat_;
}

inline unsigned int cSrvParams::poisonTimer() const
{
	return poisonTimer_;
}

inline signed int cSrvParams::maxStealthSteps() const
{
	return maxStealthSteps_;
}

inline unsigned int cSrvParams::runningStamSteps() const
{
	return runningStamSteps_;
}

inline unsigned int cSrvParams::hungerRate() const
{
	return hungerRate_;
}

inline unsigned int cSrvParams::hungerDamageRate() const
{
	return hungerDamageRate_;
}

inline unsigned char cSrvParams::hungerDamage() const
{
	return hungerDamage_;
}

inline float cSrvParams::boatSpeed() const
{
	return boatSpeed_;
}

inline int cSrvParams::persecute() const
{
	return persecute_;
}

inline unsigned int cSrvParams::tamedDisappear() const
{
	return tamedDisappear_;
}

inline unsigned int cSrvParams::houseInTown() const
{
	return houseInTown_;
}

inline unsigned int cSrvParams::shopRestock() const
{
	return shopRestock_;
}

inline int cSrvParams::attackstamina() const
{
	return attackstamina_;
}

inline unsigned char cSrvParams::attack_distance() const
{
	return attack_distance_;
}

inline unsigned int cSrvParams::hitpointrate() const
{
	return hitpointrate_;
}

inline unsigned int cSrvParams::staminarate() const
{
	return staminarate_;
}

inline unsigned int cSrvParams::manarate() const
{
	return manarate_;
}

inline char cSrvParams::armoraffectmana() const
{
	return armoraffectmana_;
}

inline unsigned int cSrvParams::snoopdelay() const
{
	return snoopdelay_;
}

inline unsigned short int cSrvParams::quittime() const
{
	return quittime_;
}

inline unsigned long int cSrvParams::housedecay_secs() const
{
	return housedecay_secs_;
}

inline long int cSrvParams::murderdecay() const
{
	return murderdecay_;
}

inline unsigned int cSrvParams::maxkills() const
{
	return maxkills_;
}

inline int cSrvParams::crimtime() const
{
	return crimtime_;
}

inline unsigned int cSrvParams::resitemdecaytime() const
{
	return resourceitemdecaytime_;
}

inline bool cSrvParams::cacheMulFiles() const
{
	return cacheMulFiles_;
}

inline unsigned int cSrvParams::spawnRegionCheckTime() const
{
	return spawnRegionCheckTime_;
}

inline unsigned int cSrvParams::secondsPerUOMinute() const
{
	return secondsPerUOMinute_;
}

inline unsigned char cSrvParams::worldBrightLevel() const
{
	return worldBrightLevel_;
}

inline unsigned char cSrvParams::worldFixedLevel() const
{
	return worldFixedLevel_;
}

inline unsigned char cSrvParams::worldDarkLevel() const
{
	return worldDarkLevel_;
}

inline unsigned char cSrvParams::dungeonLightLevel() const
{
	return dungeonLightLevel_;
}

inline int cSrvParams::saveInterval() const
{
	return saveInterval_;
}

inline unsigned int cSrvParams::showNpcTitles() const
{
	return showNpcTitles_;
}

inline QString cSrvParams::databaseDriver() const 
{
	return databaseDriver_;
}

inline QString cSrvParams::databaseHost() const 
{
	return databaseHost_;
}

inline QString cSrvParams::databaseName() const
{
	return databaseName_;
}

inline QString cSrvParams::databaseUsername() const
{
	return databaseUsername_;
}

inline QString cSrvParams::databasePassword() const
{
	return databasePassword_;
}

inline QString cSrvParams::accountsDriver() const 
{
	return accountsDriver_;
}

inline QString cSrvParams::accountsHost() const 
{
	return accountsHost_;
}

inline QString cSrvParams::accountsName() const
{
	return accountsName_;
}

inline QString cSrvParams::accountsUsername() const
{
	return accountsUsername_;
}

inline QString cSrvParams::accountsPassword() const
{
	return accountsPassword_;
}

inline bool cSrvParams::showSkillTitles() const
{
	return showSkillTitles_; 
}

inline bool cSrvParams::enableLogin() const
{
	return enableLogin_; 
}

inline bool cSrvParams::enableGame() const
{
	return enableGame_; 
}

inline unsigned short cSrvParams::gamePort() const
{
	return gamePort_;
}

inline unsigned short cSrvParams::loginPort() const
{
	return loginPort_;
}

inline bool cSrvParams::addMenuByCategoryTag() const
{
	return categoryTagAddMenu_;
}

inline int cSrvParams::pathfindMaxSteps() const
{
	return pathfindMaxSteps_;
}

inline bool cSrvParams::pathfind4Follow() const
{
	return pathfind4Follow_;
}

inline bool cSrvParams::pathfind4Combat() const
{
	return pathfind4Combat_;
}

inline int cSrvParams::pathfindFollowRadius() const
{
	return pathfindFollowRadius_;
}

inline float cSrvParams::pathfindFollowMinCost() const
{
	return pathfindFollowMinCost_;
}

inline int cSrvParams::pathfindFleeRadius() const
{
	return pathfindFleeRadius_;
}

inline QString cSrvParams::logPath() const
{
	return logPath_;
}

inline int cSrvParams::pathfindMaxIterations() const
{
	return pathfindMaxIterations_;
}

#endif //__SRVPARAMS_H___


