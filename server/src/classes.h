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

//---Classes code by Zippy Started 7/29/99---//

#ifndef __Classes_h
#define __Classes_h

#include "assert.h"
#include "verinfo.h"
#include <qdom.h>
#include <map>

// Foward declarations
class QString;
class QStringList;

struct stPrivEntry
{
	QStringList commands;
	bool		implicit;
	
	stPrivEntry() { implicit = true; };
};

class cCommands
{
private:
	std::map< QString, stPrivEntry >				privlvl_commands; // replaces old metagm array...
public:
	QString command_line;
	QStringList params;
	QString GetAllParams(void);
	void NextCall(int s, int type);
	void KillSpawn(int s, QString spawnRegion );
	void RegSpawnMax(int s, QString spawnRegion );
	void RegSpawnNum(int s, QString spawnRegion, int n);
	void KillAll(int s, int percent, const char * sysmsg);
	void AddHere(int s, char z);
	void ShowGMQue(int s, int type);
	void Wipe(int s);
	void WhoCommand(int s, int type, int buttonnum);
	void CPage(int s, char * reason);
	void GMPage(int s, char * reason);
	void MakePlace(int s, int i);
	void Command( UOXSOCKET, string );
	void MakeShop(P_CHAR pc_c);
	void DyeItem(int s);
	void SetItemTrigger(int s);
	void SetTriggerType(int s);
	void SetTriggerWord(int s);
	void SetNPCTrigger(int s);
	void DupeItem(int s, P_ITEM pi_target, int amount);
	void Possess(int s);
	int cmd_offset;

	void addCmdToPrivLvl( QString privlvl, QString command );
	void rmvCmdFromPrivLvl( QString privlvl, QString command );
	bool containsCmd( QString privlvl, QString command );
	void loadPrivLvlCmds( void );
};

#include "client.h"
// use this value whereever you need to return an illegal z value
const signed char illegal_z = -128;	// reduced from -1280 to -128, to fit in with a valid signed char

class cFishing
{
public:
	void FishTarget(P_CLIENT ps);
	void Fish(P_CHAR pc_i);
};

class cSkills
{
private:
	void Tracking(int s, int selection);
	int GetSubIngotAmt(int p, char id1, char id2, char color1, char color2);
	int DeleSubIngot(int p, int id1, int id2, int color1, int color2, int amount);
	void AnvilTarget(int s, int m);
	void AnvilTargetGold(int s, int m);
	void AnvilTargetSilver(int s, int m);
	void AnvilTargetAdam(int s, int m);
	void AnvilTargetMyth(int s, int m);
	void AnvilTargetBronze(int s, int m);
	void AnvilTargetVerite(int s, int m);
	void AnvilTargetMerk(int s, int m);
	void AnvilTargetCop(int s, int m);
	void AnvilTargetAgap(int s, int m);
	void Hide(int s);
	void Stealth(int s);
	void PeaceMaking(int s);
	void PlayInstrumentWell(int s, P_ITEM pi);
	void PlayInstrumentPoor(int s, P_ITEM pi);
	P_ITEM GetInstrument(int s);
	void RandomSteal(int s);	
	void TellScroll(char *menu_name, int player, long item_param);
	void CollectAmmo(int s, int a, int b);
	void Meditation(int s);
	void ApplyRank(int s, P_ITEM pi,int rank); // by Magius(CHE)
	void Zero_Itemmake(int s); // by Magius(CHE)
public:
	// Moved here by Storm
	UI08 CalcRank( UOXSOCKET Socket, UI16 SkillValue, UI16 MinSkill, UI16 MaxSkill );

	void RepairTarget(UOXSOCKET s); // Ripper
	void SmeltItemTarget(UOXSOCKET s); // Ripper
	void TasteIDTarget(int s);
	int TrackingDirection(UOXSOCKET s, P_CHAR pc_i);
	void CreatePotion(P_CHAR pc, char type, char sub, P_ITEM pi_mortar);
	char AdvanceSkill(P_CHAR pc, int sk, char skillused);
	void AdvanceStats(P_CHAR pc, int sk);
	void TinkerAxel(int s);
	void TinkerAwg(int s);
	void TinkerClock(int s);
	void MakeDough(int s);
	void MakePizza(int s);
	void Track(P_CHAR pc_i);
	void DoPotion(int s, int type, int sub, P_ITEM mortar);
	void Tailoring(int s);
	void Fletching(int s);
	void BowCraft(int s);
	void Carpentry(int s);
	void Smith(int s);
	void MakeMenuTarget(int s, int x, int skill);
	void MakeMenu(int s, int m, int skill);
	void Mine(int s);
	void GraveDig(int s);
	void SmeltOre(int s);
	void Wheel(int s, int mat);
	void Loom(int s);
	void CookOnFire(int s, short id1, short id2, char* matname);
	void TreeTarget(int s);
	void DetectHidden(UOXSOCKET s);
	void ProvocationTarget1(UOXSOCKET s);
	void ProvocationTarget2(UOXSOCKET s);
	void EnticementTarget1(UOXSOCKET s);
	void EnticementTarget2(UOXSOCKET s);
	void AlchemyTarget(int s);
	void BottleTarget(int s);
	void PotionToBottle(P_CHAR pc, P_ITEM pi_mortar);
	char CheckSkill(P_CHAR pc, unsigned short int sk, int low, int high);
	void CreateBandageTarget(int s);
	void HealingSkillTarget(UOXSOCKET s);
	void SpiritSpeak(int s);
	void ArmsLoreTarget(int s);
	void ItemIdTarget(int s);
	void Evaluate_int_Target(UOXSOCKET s);
	void AnatomyTarget(int s);
	void TameTarget(int s);
	int GetCombatSkill(P_CHAR pc);
	void SkillUse(int s, int x);
	void StealingTarget(int s);
	void PickPocketTarget(int s);
	void CreateTrackingMenu(int s, int m);
	void TrackingMenu(int s, int gmindex);
	void BeggingTarget(int s);
	void AnimalLoreTarget(int s);
	void ForensicsTarget(int s);
	void PoisoningTarget(int s);
	int Inscribe(int s, long snum);
	int EngraveAction(int s, P_ITEM pi, int cir, int spl);
	void updateSkillLevel(P_CHAR pc, int s);
	void LockPick(int s);
	void TDummy(int s);
	void Tinkering(int s);
	void AButte(int s1, P_ITEM pButte);
	void Persecute(UOXSOCKET s);//AntiChrist persecute stuff
	void Cartography(int s); // By Polygon - opens the cartography skillmenu
	bool HasEmptyMap(P_CHAR pc); // By Polygon - checks if player has an empty map
	bool DelEmptyMap(P_CHAR pc); // By Polygon - deletes an empty map from the player's pack
	void Decipher(P_ITEM tmap, int s); // By Polygon - attempt to decipher a tattered treasure map
	int GetAntiMagicalArmorDefence(P_CHAR pc); // blackwind meditation armor stuff
	void Snooping(P_CHAR, P_ITEM);

	// Added by Darkstorm
	bool MeetRequirements( P_CHAR myChar, QDomElement Requirements, bool Notify );
};

class cWeight
{
public:
	bool CheckWeight( P_CHAR pChar );
	float RecursePacks(P_ITEM bp);
	int CheckWeight2(int s);
	void NewCalc(P_CHAR pc);
	float LockeddownWeight(P_ITEM pItem, int *total, int *total2 );
};

#include "targeting.h"
#include "magic.h"

#endif
