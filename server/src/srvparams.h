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
const unsigned int FIRST_YEAR = 1753;

class cSrvParams : public Preferences
{
protected:
	std::vector<ServerList_st> serverList_;
	std::vector<StartLocation_st> startLocation_;
	QStringList clientsAllowed_;
	
	// loaded data
	unsigned int skillcap_;
	unsigned int statcap_;
	unsigned char commandPrefix_;
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
	float checkAITime_;
	float checkFollowTime_;
	float checkTammedTime_;
	unsigned int decayTime_;
	int niceLevel_;
	unsigned short port_;
	float goldWeight_;
	unsigned int playercorpsedecaymultiplier_;
	unsigned short lootdecayswithcorpse_;
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
	int cutScrollReq_;
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
	unsigned int spiritspeaktimer_;
	unsigned int spawnRegionCheckTime_;
	unsigned int secondsPerUOMinute_;
	bool cacheMulFiles_;
	bool useSpecialBank_;
	QString specialBankTrigger_;
	unsigned int beggingRange_;
	unsigned int beggingTime_;
	unsigned char season_;
	QString worldSaveModule_;

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

	//Repsys
	long int		murderdecay_;
	unsigned int	maxkills_;
	int				crimtime_;

	// Resources
	unsigned int        minecheck_;
	unsigned int		logs_;
	unsigned long int	logtime_;
	unsigned long int	lograte_;
	unsigned int		logarea_;
	int					logtrigger_;
	int					logstamina_;
	unsigned int		ore_;
	unsigned long int	oretime_;
	unsigned long int	orerate_;
	unsigned int		orearea_;
	int					miningtrigger_;
	int					miningstamina_;

	//Tracking
	unsigned int baserange_;
	unsigned int maxtargets_;
	unsigned int basetimer_;
	unsigned int redisplaytime_;

	// Fishing
	unsigned int basetime_;
	unsigned int randomtime_;

	// Message Board
	QString msgboardPath_;
	int  msgboardPostAccess_;
	int  msgboardPostRemove_;
	int  msgboardRetention_;

	// Light
	unsigned char worldBrightLevel_;
	unsigned char worldFixedLevel_;
	unsigned char worldDarkLevel_;
	unsigned char dungeonLightLevel_;
	unsigned char worldCurrentLevel_;

public:
    cSrvParams( const QString& filename, const QString& format, const QString& version );

	virtual void reload();
	
	std::vector<ServerList_st>& serverList(); // read-only
	std::vector<StartLocation_st>& startLocation();

	// gets
	unsigned int skillcap() const;
	unsigned int statcap() const;
	unsigned char commandPrefix() const;
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
	float checkFollowTime() const;
	float checkTammedTime() const;
	int niceLevel() const;
	unsigned int decayTime() const;
	unsigned short port() const;
	float goldWeight() const;
	unsigned int playercorpsedecaymultiplier() const;
	unsigned short lootdecayswithcorpse() const;
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
	int cutScrollReq() const;
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
	unsigned int spiritspeaktimer() const;
	bool cacheMulFiles() const;
	unsigned int spawnRegionCheckTime() const;
	unsigned int secondsPerUOMinute() const;
	void setSecondsPerUOMinute( unsigned int );
	bool useSpecialBank() const;
	QString specialBankTrigger() const;
	bool isClientAllowed( const QString& );
	const QStringList& clientsAllowed() const;
	unsigned int beggingRange() const;
	unsigned int beggingTime() const;
	unsigned char season() const;
	void setSeason( unsigned char );
	QString worldSaveModule() const;


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

	// Repsys
	long int		murderdecay() const;
	unsigned int	maxkills() const;
	int				crimtime() const;

	// Resources
	unsigned int        minecheck() const;
	unsigned int		logs() const;
	unsigned long int	logtime() const;
	unsigned long int	lograte() const;
	unsigned int		logarea() const;
	int					logtrigger() const;
	int					logstamina() const;
	unsigned int		ore() const;
	unsigned long int	oretime() const;
	unsigned long int	orerate() const;
	unsigned int		orearea() const;
	int					miningtrigger() const;
	int					miningstamina() const;

	// Tracking
	unsigned int baserange() const;
	unsigned int maxtargets() const;
	unsigned int basetimer() const;
	unsigned int redisplaytime() const;

	// Fishing
	unsigned int basetime() const;
	unsigned int randomtime() const;

	// MessageBoard
	QString msgboardPath() const;
	int  msgboardPostAccess() const;
	int  msgboardPostRemove() const;
	int  msgboardRetention() const;

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

private:
	void setDefaultStartLocation();
	void setDefaultServerList();
	void readData();
};

// inline members

inline unsigned int cSrvParams::skillcap() const
{
	return skillcap_;
}

inline unsigned int cSrvParams::statcap() const
{
	return statcap_;
}

inline unsigned char cSrvParams::commandPrefix() const
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

inline unsigned short cSrvParams::port() const
{
	return port_;
}

inline float cSrvParams::goldWeight() const
{
	return goldWeight_;
}

inline unsigned int cSrvParams::playercorpsedecaymultiplier() const
{
	return playercorpsedecaymultiplier_;
}

inline unsigned short cSrvParams::lootdecayswithcorpse() const
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

inline int cSrvParams::cutScrollReq() const
{
	return cutScrollReq_;
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

inline unsigned int cSrvParams::logs() const
{
	return logs_;
}

inline unsigned long int cSrvParams::logtime() const
{
	return logtime_;
}

inline unsigned long int cSrvParams::lograte() const
{
	return lograte_;
}

inline unsigned int cSrvParams::logarea() const
{
	return logarea_;
}

inline int cSrvParams::logtrigger() const
{
	return logtrigger_;
}

inline int cSrvParams::logstamina() const
{
	return logstamina_;
}

inline unsigned int cSrvParams::ore() const
{
	return ore_;
}

inline unsigned long int cSrvParams::oretime() const
{
	return oretime_;
}

inline unsigned long int cSrvParams::orerate() const
{
	return orerate_;
}

inline unsigned int cSrvParams::orearea() const
{
	return orearea_;
}

inline int cSrvParams::miningtrigger() const
{
	return miningtrigger_;
}

inline int cSrvParams::miningstamina() const
{
	return miningstamina_;
}

inline unsigned int cSrvParams::minecheck() const
{
	return minecheck_;
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

inline unsigned int cSrvParams::basetime() const
{
	return basetime_;
}

inline unsigned int cSrvParams::randomtime() const
{
	return randomtime_;
}

inline unsigned int cSrvParams::spiritspeaktimer() const
{
	return spiritspeaktimer_;
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

inline bool cSrvParams::useSpecialBank() const
{
	return useSpecialBank_;
}

inline QString cSrvParams::specialBankTrigger() const
{
	return specialBankTrigger_;
}

inline QString cSrvParams::msgboardPath() const
{
	return msgboardPath_;
}

inline int cSrvParams::msgboardPostAccess() const
{
	return msgboardPostAccess_;
}

inline int cSrvParams::msgboardPostRemove() const
{
	return msgboardPostRemove_;
}

inline int cSrvParams::msgboardRetention() const
{
	return msgboardRetention_;
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

inline QString cSrvParams::worldSaveModule() const
{
	return worldSaveModule_;
}

#endif //__SRVPARAMS_H___


