#ifndef _H_RACES
#define _H_RACES
#include "wolfpack.h"
class cRaceManager
{
public:
	void LoadRaceFile();
	void SetRace(P_CHAR pc, int race);
	int CheckRelation(P_CHAR pc_1, P_CHAR pc_2); // returns: 1 friend, 2 enemy, 3 neutral
	int RandomHairStyle();
	int RandomBeardStyle();
};
extern cRaceManager *RaceManager;

class cRaces
{
public:
	cRaces()
	{
		IsPlayerRace = false;
		RaceName[512] = NULL;
		SkinList[512] = NULL;
		HairBeardList[512] = NULL;
		Gender = 0;
		StartInt = 0;
		StartStr = 0;
		StartDex = 0;
		IntCap = 0;
		StrCap = 0;
		DexCap = 0;
		CanUseSkill.resize(0);
		CantUseItem.resize(0);
		RacialAlly.resize(0);
		RacialEnemy.resize(0);
		NightSight = false;
		HairMin = 0;
		HairMax = 0;
		BeardReq = 0; // 0=none, 1=male, 2=female, 3=all
		HairReq = 0; // 0=none, 1=male, 2=female, 3=all
		NoHair = false;
		NoBeard = false;
		ImuneToSpell.resize(0);
	}
	~cRaces()
	{
		IsPlayerRace = false;
		RaceName[512] = NULL;
		SkinList[512] = NULL;
		HairBeardList[512] = NULL;
		Gender = 0;
		StartInt = 0;
		StartStr = 0;
		StartDex = 0;
		IntCap = 0;
		StrCap = 0;
		DexCap = 0;
		CanUseSkill.clear();
		CantUseItem.clear();
		RacialAlly.clear();
		RacialEnemy.clear();
		NightSight = false;
		HairMin = 0;
		HairMax = 0;
		BeardReq = 0; // 0=none, 1=male, 2=female, 3=all
		HairReq = 0; // 0=none, 1=male, 2=female, 3=all
		NoHair = false;
		NoBeard = false;
		ImuneToSpell.clear();
	}
	bool IsPlayerRace;
	char RaceName[512];
	char SkinList[512];
	char HairBeardList[512];
	int Gender;
	int StartInt;
	int StartStr;
	int StartDex;
	int IntCap;
	int StrCap;
	int DexCap;
	int VisRange;
	vector<int> CanUseSkill;
	vector<int> CantUseItem;
	vector<int> RacialAlly;
	vector<int> RacialEnemy;
	vector<int> ImuneToSpell;
//	vector<int> SkillToSet;
//	vector<int> SkillSetAmt;
	bool NightSight;
	int HairMin;
	int HairMax;
	int BeardReq; // 0=none, 1=male, 2=female, 3=all
	int HairReq; // 0=none, 1=male, 2=female, 3=all
	bool NoHair;
	bool NoBeard;
	bool CheckSkillUse(int skillnum);
	bool CheckSpellImune(int spellnum);
	bool CheckItemUse(int itemnum);
};
extern vector<cRaces *> Races;
#endif


