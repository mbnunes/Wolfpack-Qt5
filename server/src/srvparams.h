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
	QStringList clientsAllowed_;
	
	// loaded data
	bool allowUnencryptedClients_;
	unsigned int skillcap_;
	bool showSkillTitles_;
	unsigned int statcap_;
	QString commandPrefix_;
	unsigned int skillAdvanceModifier_;
	unsigned int statsAdvanceModifier_;
	unsigned short objectDelay_;
	unsigned char bgSound_;
	bool stealing_;
	bool guardsActive_;
	bool partMsg_;
	bool joinMsg_;
	bool saveSpawns_;
	bool autoAccountCreate_;
	bool serverLog_;
	bool speechLog_;
	bool pvpLog_;
	bool gmLog_;
	unsigned int autoAccountReload_;
	float stablingFee_;
	bool checkCharAge_;
	bool announceWorldSaves_;
	float checkItemTime_;
	float checkNPCTime_;
	float checkFollowTime_;
	float checkTammedTime_;
	unsigned int decayTime_;
	int niceLevel_;
	unsigned short loginPort_;
	bool enableLogin_;
	unsigned short gamePort_;
	bool enableGame_;
	float goldWeight_;
	unsigned int playercorpsedecaymultiplier_;
	bool lootdecayswithcorpse_;
	float invisTimer_;
	unsigned short skillDelay_;
	int skillLevel_;
	unsigned short bandageDelay_;
	bool bandageInCombat_;
	float gateTimer_;
	unsigned int inactivityTimeout_;
	unsigned int showDeathAnim_;
	unsigned int poisonTimer_;
	signed int maxStealthSteps_;
	unsigned int runningStamSteps_;
	int backupSaveRatio_;
	unsigned int hungerRate_;
	unsigned int hungerDamageRate_;
	unsigned char hungerDamage_;
	float boatSpeed_;
	int html_;
	int persecute_;
	unsigned int tamedDisappear_;
	unsigned int houseInTown_;
	unsigned int shopRestock_;
	unsigned int badNpcsRed_;
	unsigned int slotAmount_;
	unsigned int snoopdelay_;
	int  escortactive_;
	int  escortinitexpire_;
	int  escortactiveexpire_;
	int  escortdoneexpire_;
	int  bountysactive_;
	int  bountysexpire_;
	unsigned short int quittime_;
	char errors_to_console_;
	unsigned long int housedecay_secs_;
	unsigned int default_jail_time_;
	unsigned char showCVCS_;
	unsigned int spawnRegionCheckTime_;
	unsigned int secondsPerUOMinute_;
	bool cacheMulFiles_;
	unsigned int beggingRange_;
	unsigned int beggingTime_;
	unsigned char season_;
	QString databaseName_;
	QString databaseUsername_;
	QString databasePassword_;
	QString databaseHost_;
	int saveInterval_;
	bool heartBeat_;
	int defaultpriv2_;
	QString mulPath_;
	int maxLoginAttempts_;
	int resetAttemptCount_;
	int accountsBlockTime_;
	QString accountsArchiver_;
	bool categoryTagAddMenu_;
	QString saveModule_;
	QString loadModule_;
	QString savePath_;
	QString savePrefix_;
	double npcMoveTime_;
	double tamedNpcMoveTime_;

	// AI
	float checkAITime_;
	unsigned int stablemasterRefreshTime_;
	float stablemasterGoldPerRefresh_;

	// Remote Admin
	unsigned int ra_port_;
	bool EnableRA_;

	// Regenerate
	unsigned int hitpointrate_;
	unsigned int staminarate_;
	unsigned int manarate_;
	char armoraffectmana_;

	// Combat
	bool combatHitMessage_;
	unsigned int maxAbsorbtion_;
	unsigned int maxnohabsorbtion_;
	unsigned int monsters_vs_animals_;
	unsigned int animals_attack_chance_;
	unsigned int animals_guarded_;
	unsigned int npcdamage_;
	unsigned int npc_base_fleeat_;
	unsigned int npc_base_reattackat_;
	int attackstamina_;
	unsigned char attack_distance_;

	// Vendor
	unsigned int sellbyname_;
	unsigned int sellmaxitem_;
	unsigned int trade_system_;
	unsigned int rank_system_;
	short checkBank_;
	unsigned int vendorGreet_;
	unsigned int showNpcTitles_;

	// added by darkstorm: make-menus
	bool belowminskillfails_;
	bool hungeraffectsskills_;

	//Repsys
	long int		murderdecay_;
	unsigned int	maxkills_;
	int				crimtime_;

	// Resources
	unsigned int resourceitemdecaytime_;

	//Tracking
	unsigned int baserange_;
	unsigned int maxtargets_;
	unsigned int basetimer_;
	unsigned int redisplaytime_;

	// Light
	unsigned char worldBrightLevel_;
	unsigned char worldFixedLevel_;
	unsigned char worldDarkLevel_;
	unsigned char dungeonLightLevel_;
	unsigned char worldCurrentLevel_;

	// Magic
	bool precasting_;
	bool walkDisturbsCast_;
	bool cutScrollReq_;

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
	unsigned char bgSound() const;
	unsigned short objectDelay() const;
	bool partMsg() const;
	bool joinMsg() const;
	unsigned int autoAccountReload() const;
	bool autoAccountCreate() const;
	bool saveSpawns() const;
	bool serverLog() const;
	bool speechLog() const;
	bool pvpLog() const;
	bool gmLog() const;
	float stablingFee() const;
	bool checkCharAge() const;
	bool announceWorldSaves() const;
	void announceWorldSaves(bool enable);
	float checkItemTime() const;
	float checkNPCTime() const;
	float checkAITime() const;
	float stablemasterGoldPerRefresh() const;
	unsigned int stablemasterRefreshTime() const;
	float checkFollowTime() const;
	float checkTammedTime() const;
	int niceLevel() const;
	unsigned int decayTime() const;
	float goldWeight() const;
	unsigned int playercorpsedecaymultiplier() const;
	bool lootdecayswithcorpse() const;
	float invisTimer() const;
	unsigned short skillDelay() const;
	int skillLevel() const;
	unsigned short bandageDelay() const;
	bool bandageInCombat() const;
	float gateTimer() const;
	unsigned int inactivityTimeout() const;
	unsigned int showDeathAnim() const;
	unsigned int poisonTimer() const;
	signed int maxStealthSteps() const;
	unsigned int runningStamSteps() const;
	int backupSaveRatio() const;
	unsigned int hungerRate() const;
	unsigned int hungerDamageRate() const;
	unsigned char hungerDamage() const;
	float boatSpeed() const;
	int html() const;
	bool cutScrollReq() const;
	bool precasting() const;
	bool walkDisturbsCast() const;
	int persecute() const;
	unsigned int tamedDisappear() const;
	unsigned int houseInTown() const;
	unsigned int shopRestock() const;
	unsigned int badNpcsRed() const;
	unsigned int slotAmount() const;
	unsigned int snoopdelay() const;
	int  escortactive() const;
	int  escortinitexpire() const;
	int  escortactiveexpire() const;
	int  escortdoneexpire() const;
	int  bountysactive() const;
	int  bountysexpire() const;
	unsigned short int quittime() const;
	char errors_to_console() const;
	unsigned long int housedecay_secs() const;
	unsigned int default_jail_time() const;
	unsigned char showCVCS() const;
	bool cacheMulFiles() const;
	unsigned int spawnRegionCheckTime() const;
	unsigned int secondsPerUOMinute() const;
	void setSecondsPerUOMinute( unsigned int );
	bool isClientAllowed( const QString& );
	const QStringList& clientsAllowed() const;
	unsigned int beggingRange() const;
	unsigned int beggingTime() const;
	unsigned char season() const;
	void setSeason( unsigned char );
	int saveInterval() const;
	bool heartBeat() const;
	int defaultpriv2() const;
	QString mulPath() const;
	void setMulPath( QString data ) { mulPath_ = data; }
	int MaxLoginAttempts() const;
	int AccountBlockTime() const;
	int resetAttemptCount() const;
	QString accountsArchiver() const;
	bool addMenuByCategoryTag() const;
	double npcMoveTime() const;
	double tamedNpcMoveTime() const;

	// Persistency Module
	QString databaseHost() const;
	QString databasePassword() const;
	QString databaseUsername() const;
	QString databaseName() const;

	// Remote Admin
	unsigned int ra_port() const;
	bool EnableRA() const;

	// Regenerate
	unsigned int hitpointrate() const;
	unsigned int staminarate() const;
	unsigned int manarate() const;
	char armoraffectmana() const;

	// Combat
	bool combatHitMessage() const;
	unsigned int maxAbsorbtion() const;
	unsigned int maxnohabsorbtion() const;
	unsigned int monsters_vs_animals() const;
	unsigned int animals_attack_chance() const;
	unsigned int animals_guarded() const;
	unsigned int npcdamage() const;
	unsigned int npc_base_fleeat() const;
	unsigned int npc_base_reattackat() const;
	int attackstamina() const;
	unsigned char attack_distance() const;

	// Vendor
	unsigned int sellbyname() const;
	unsigned int sellmaxitem() const;
	unsigned int trade_system() const;
	unsigned int rank_system() const;
	short checkBank() const;
	unsigned int vendorGreet() const;
	unsigned int showNpcTitles() const;

	bool BelowMinSkillFails() const;
	bool HungerAffectsSkills() const;

	// Repsys
	long int		murderdecay() const;
	unsigned int	maxkills() const;
	int				crimtime() const;

	// Resources
	unsigned int resitemdecaytime() const;

	// Tracking
	unsigned int baserange() const;
	unsigned int maxtargets() const;
	unsigned int basetimer() const;
	unsigned int redisplaytime() const;

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

	// Worldsave
	QString savePath() const;
	QString saveModule() const;
	QString loadModule() const;
	QString savePrefix() const;

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

inline unsigned char cSrvParams::bgSound() const
{
	return bgSound_;
}

inline unsigned short cSrvParams::objectDelay() const
{
	return objectDelay_;
}

inline bool cSrvParams::partMsg() const
{
	return partMsg_;
}

inline bool cSrvParams::allowUnencryptedClients() const
{
	return allowUnencryptedClients_;
}

inline bool cSrvParams::serverLog() const
{
	return serverLog_;
}

inline bool cSrvParams::speechLog() const
{
	return speechLog_;
}

inline bool cSrvParams::pvpLog() const
{
	return pvpLog_;
}

inline bool cSrvParams::gmLog() const
{
	return gmLog_;
}

inline bool cSrvParams::joinMsg() const
{
	return joinMsg_;
}

inline unsigned int cSrvParams::autoAccountReload() const
{
	return autoAccountReload_;
}

inline bool cSrvParams::autoAccountCreate() const
{
	return autoAccountCreate_;
}

inline bool cSrvParams::saveSpawns() const
{
	return saveSpawns_;
}

inline float cSrvParams::stablingFee() const
{
	return stablingFee_;
}

inline bool cSrvParams::checkCharAge() const
{
	return checkCharAge_;
}

inline bool cSrvParams::announceWorldSaves() const
{
	return announceWorldSaves_;
}

inline void cSrvParams::announceWorldSaves(bool enable)
{
	announceWorldSaves_ = enable;
	setBool("General", "Announce WorldSaves", enable);
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

inline float cSrvParams::stablemasterGoldPerRefresh() const
{
	return stablemasterGoldPerRefresh_;
}

inline unsigned int cSrvParams::stablemasterRefreshTime() const
{
	return stablemasterRefreshTime_;
}

inline float cSrvParams::checkFollowTime() const
{
	return checkFollowTime_;
}

inline float cSrvParams::checkTammedTime() const
{
	return checkTammedTime_;
}

inline int cSrvParams::niceLevel() const
{
	return niceLevel_;
}

inline unsigned int cSrvParams::decayTime() const
{
	return decayTime_;
}

inline float cSrvParams::goldWeight() const
{
	return goldWeight_;
}

inline unsigned int cSrvParams::playercorpsedecaymultiplier() const
{
	return playercorpsedecaymultiplier_;
}

inline bool cSrvParams::lootdecayswithcorpse() const
{
	return lootdecayswithcorpse_;
}

inline float cSrvParams::invisTimer() const
{
	return invisTimer_;
}

inline unsigned short cSrvParams::skillDelay() const
{
	return skillDelay_;
}

inline int cSrvParams::skillLevel() const
{
	return skillLevel_;
}

inline unsigned short cSrvParams::bandageDelay() const
{
	return bandageDelay_;
}

inline bool cSrvParams::bandageInCombat() const
{
	return bandageInCombat_;
}

inline float cSrvParams::gateTimer() const
{
	return gateTimer_;
}

inline unsigned int cSrvParams::inactivityTimeout() const
{
	return inactivityTimeout_;
}

inline unsigned int cSrvParams::showDeathAnim() const
{
	return showDeathAnim_;
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

inline int cSrvParams::backupSaveRatio() const
{
	return backupSaveRatio_;
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

inline bool cSrvParams::combatHitMessage() const
{
	return combatHitMessage_;
}

inline float cSrvParams::boatSpeed() const
{
	return boatSpeed_;
}

inline int cSrvParams::html() const
{
	return html_;
}

inline bool cSrvParams::cutScrollReq() const
{
	return cutScrollReq_;
}

inline bool cSrvParams::precasting() const
{
	return precasting_;
}

inline bool cSrvParams::walkDisturbsCast() const
{
	return walkDisturbsCast_;
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

inline unsigned int cSrvParams::badNpcsRed() const
{
	return badNpcsRed_;
}

inline unsigned int cSrvParams::slotAmount() const
{
	return slotAmount_;
}

inline unsigned int cSrvParams::maxAbsorbtion() const
{
	return maxAbsorbtion_;
}

inline unsigned int cSrvParams::maxnohabsorbtion() const
{
	return maxnohabsorbtion_;
}

inline unsigned int cSrvParams::monsters_vs_animals() const
{
	return monsters_vs_animals_;
}

inline unsigned int cSrvParams::animals_attack_chance() const
{
	return animals_attack_chance_;
}

inline unsigned int cSrvParams::animals_guarded() const
{
	return animals_guarded_;
}

inline unsigned int cSrvParams::npcdamage() const
{
	return npcdamage_;
}

inline unsigned int cSrvParams::npc_base_fleeat() const
{
	return npc_base_fleeat_;
}

inline int cSrvParams::attackstamina() const
{
	return attackstamina_;
}

inline unsigned int cSrvParams::npc_base_reattackat() const
{
	return npc_base_reattackat_;
}

inline unsigned char cSrvParams::attack_distance() const
{
	return attack_distance_;
}

inline unsigned int cSrvParams::sellbyname() const
{
	return sellbyname_;
}

inline unsigned int cSrvParams::sellmaxitem() const
{
	return sellmaxitem_;
}

inline unsigned int cSrvParams::trade_system() const
{
	return trade_system_;
}

inline unsigned int cSrvParams::rank_system() const
{
	return rank_system_;
}

inline short cSrvParams::checkBank() const
{
	return checkBank_;
}

inline unsigned int cSrvParams::vendorGreet() const
{
	return vendorGreet_;
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

inline bool cSrvParams::BelowMinSkillFails() const
{
	return belowminskillfails_; 
}

inline bool cSrvParams::HungerAffectsSkills() const
{
	return hungeraffectsskills_;
}

inline unsigned int cSrvParams::snoopdelay() const
{
	return snoopdelay_;
}

inline int cSrvParams::escortactive() const
{
	return escortactive_;
}

inline int cSrvParams::escortinitexpire() const
{
	return escortinitexpire_;
}

inline int cSrvParams::escortactiveexpire() const
{
	return escortactiveexpire_;
}

inline int cSrvParams::escortdoneexpire() const
{
	return escortdoneexpire_;
}

inline int cSrvParams::bountysactive() const
{
	return bountysactive_;
}

inline int cSrvParams::bountysexpire() const
{
	return bountysexpire_;
}

inline unsigned short int cSrvParams::quittime() const
{
	return quittime_;
}

inline char cSrvParams::errors_to_console() const
{
	return errors_to_console_;
}

inline unsigned long int cSrvParams::housedecay_secs() const
{
	return housedecay_secs_;
}

inline unsigned int cSrvParams::default_jail_time() const
{
	return default_jail_time_;
}

inline unsigned char cSrvParams::showCVCS() const
{
	return showCVCS_;
}

inline unsigned int cSrvParams::ra_port() const
{
	return ra_port_;
}

inline bool cSrvParams::EnableRA() const
{
	return EnableRA_;
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

inline unsigned int cSrvParams::baserange() const
{
	return baserange_;
}

inline unsigned int cSrvParams::maxtargets() const
{
	return maxtargets_;
}

inline unsigned int cSrvParams::basetimer() const
{
	return basetimer_;
}

inline unsigned int cSrvParams::redisplaytime() const
{
	return redisplaytime_;
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

inline const QStringList& cSrvParams::clientsAllowed() const
{
	return clientsAllowed_;
}

inline unsigned int cSrvParams::beggingRange() const
{
	return beggingRange_;
}

inline unsigned int cSrvParams::beggingTime() const
{
	return beggingTime_;
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

inline unsigned char cSrvParams::season() const
{
	return season_;
}

inline void cSrvParams::setSeason( unsigned char data)
{
	season_ = data;
}

inline int cSrvParams::saveInterval() const
{
	return saveInterval_;
}

inline bool cSrvParams::heartBeat() const
{
	return heartBeat_;
}

inline int cSrvParams::defaultpriv2() const
{
	return defaultpriv2_;
}

inline unsigned int cSrvParams::showNpcTitles() const
{
	return showNpcTitles_;
}

inline int cSrvParams::MaxLoginAttempts() const
{
	return maxLoginAttempts_;
}

inline int cSrvParams::AccountBlockTime() const
{
	return accountsBlockTime_;
}

inline int cSrvParams::resetAttemptCount() const
{
	return resetAttemptCount_;
}

inline QString cSrvParams::accountsArchiver() const
{
	return accountsArchiver_;
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

inline QString cSrvParams::saveModule() const
{
	return saveModule_;
}

inline QString cSrvParams::loadModule() const
{
	return loadModule_;
}

inline QString cSrvParams::savePrefix() const
{
	return savePrefix_;
}

inline QString cSrvParams::savePath() const
{
	return savePath_;
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

inline int cSrvParams::pathfindMaxIterations() const
{
	return pathfindMaxIterations_;
}

#endif //__SRVPARAMS_H___


