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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
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

class cRace
{
public:
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
	bool IsPlayerRace;
	string RaceName;
	string SkinList;
	string HairBeardList;
	int Gender;
	int StartInt;
	int StartStr;
	int StartDex;
	int IntCap;
	int StrCap;
	int DexCap;
	int VisRange;
	int BestFightSkill;
	int BaseAR;
	Coord_cl startpos;
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

class cRaces
{
private:
	vector<cRace> races;
public:
	void LoadRaceFile();
	void SetRace(P_CHAR pc, int race);
	int CheckRelation(P_CHAR pc_1, P_CHAR pc_2); // returns: 1 friend, 2 enemy, 3 neutral
	int RandomHairStyle();
	int RandomBeardStyle();
	cRace* operator[](long index)
	{
		return &races[index];
	}
};

extern cRaces Races;

#endif //__RACES_H__

