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

#if !defined(__RACES_H__)
#define __RACES_H__

// Platform Specifics
#include "platform.h"

// System Includes
#include <vector>
#include <algorithm>

// Our own includes
#include "wolfpack.h"

using namespace std;

//##ModelId=3C5D92D402F9
class cRace
{
public:
	//##ModelId=3C5D92D40321
	cRace()
	{
		IsPlayerRace = false;
		Gender = 0;
		StartInt = 0;
		StartStr = 0;
		StartDex = 0;
		IntCap = 0;
		StrCap = 0;
		DexCap = 0;
		NightSight = false;
		HairMin = 0;
		HairMax = 0;
		BeardReq = 0; // 0=none, 1=male, 2=female, 3=all
		HairReq = 0; // 0=none, 1=male, 2=female, 3=all
		NoHair = false;
		NoBeard = false;
		BaseAR = 0;
		BestFightSkill = SWORDSMANSHIP; // defaults to swords
		startpos.x=0;
		startpos.y=0;
		startpos.z=0;
	}
	//##ModelId=3C5D92D4032B
	bool IsPlayerRace;
	//##ModelId=3C5D92D4034A
	string RaceName;
	//##ModelId=3C5D92D40368
	string SkinList;
	//##ModelId=3C5D92D4037C
	string HairBeardList;
	//##ModelId=3C5D92D4038F
	int Gender;
	//##ModelId=3C5D92D403AD
	int StartInt;
	//##ModelId=3C5D92D403C1
	int StartStr;
	//##ModelId=3C5D92D403D5
	int StartDex;
	//##ModelId=3C5D92D5000B
	int IntCap;
	//##ModelId=3C5D92D5001F
	int StrCap;
	//##ModelId=3C5D92D50034
	int DexCap;
	//##ModelId=3C5D92D50052
	int VisRange;
	//##ModelId=3C5D92D50070
	int BestFightSkill;
	//##ModelId=3C5D92D50084
	int BaseAR;
	//##ModelId=3C5D92D500A3
	Coord_cl startpos;
	//##ModelId=3C5D92D500CB
	vector<int> CanUseSkill;
	//##ModelId=3C5D92D500FD
	vector<int> CantUseItem;
	//##ModelId=3C5D92D50125
	vector<int> RacialAlly;
	//##ModelId=3C5D92D5016B
	vector<int> RacialEnemy;
	//##ModelId=3C5D92D50193
	vector<int> ImuneToSpell;
//	vector<int> SkillToSet;
//	vector<int> SkillSetAmt;
	//##ModelId=3C5D92D501BA
	bool NightSight;
	//##ModelId=3C5D92D501D8
	int HairMin;
	//##ModelId=3C5D92D501F6
	int HairMax;
	//##ModelId=3C5D92D50214
	int BeardReq; // 0=none, 1=male, 2=female, 3=all
	//##ModelId=3C5D92D50232
	int HairReq; // 0=none, 1=male, 2=female, 3=all
	//##ModelId=3C5D92D50250
	bool NoHair;
	//##ModelId=3C5D92D5026E
	bool NoBeard;
	//##ModelId=3C5D92D5028C
	bool CheckSkillUse(int skillnum);
	//##ModelId=3C5D92D502D2
	bool CheckSpellImune(int spellnum);
	//##ModelId=3C5D92D502F1
	bool CheckItemUse(int itemnum);
};

//##ModelId=3C5D92D5035F
class cRaces
{
private:
	//##ModelId=3C5D92D503BA
	vector<cRace> races;
public:
	//##ModelId=3C5D92D503D7
	void LoadRaceFile();
	//##ModelId=3C5D92D503E1
	void SetRace(P_CHAR pc, int race);
	//##ModelId=3C5D92D60017
	int CheckRelation(P_CHAR pc_1, P_CHAR pc_2); // returns: 1 friend, 2 enemy, 3 neutral
	//##ModelId=3C5D92D60035
	int RandomHairStyle();
	//##ModelId=3C5D92D6003F
	int RandomBeardStyle();
	//##ModelId=3C5D92D60049
	cRace* operator[](long index)
	{
		return &races[index];
	}
};

extern cRaces Races;

#endif //__RACES_H__

