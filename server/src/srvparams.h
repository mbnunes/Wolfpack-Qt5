
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

// Forward Declarations
class Preferences;

// Structs

struct StartLocation_st
{
	QString name;
	Coord_cl pos;
};

class cSrvParams : public Preferences
{
protected:
	std::vector<ServerList_st> serverList_;
	std::vector<StartLocation_st> startLocation_;

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
public:
    cSrvParams( const QString& filename, const QString& format, const QString& version );
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

private:
	void setDefaultStartLocation();
	void setDefaultServerList();
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

#endif __SRVPARAMS_H___


