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
#include "mstring.h"

//##ModelId=3C5D92F700A1
struct ip_block_st
{
	//##ModelId=3C5D92F700C9
	unsigned long address;
	//##ModelId=3C5D92F700DD
	unsigned long mask;
};
/*
class cAdmin 
{
private:
	void ReadString();
	
public:
	cAdmin();
	void ReadIni();
	short int port;
};
*/
//##ModelId=3C5D92F703CC
class cCombat
{
private:
	//##ModelId=3C5D92F8000C
	void ItemCastSpell(UOXSOCKET s, P_CHAR pc, P_ITEM pi);
	//##ModelId=3C5D92F80034
	int TimerOk(P_CHAR pc);
	//##ModelId=3C5D92F80048
	void ItemSpell(cChar* Attacker, cChar* Defender);
	//##ModelId=3C5D92F80066
	void doSoundEffect(P_CHAR pc, int fightskill, P_ITEM pWeapon);//AntiChrist
	//##ModelId=3C5D92F8008E
	void doMissedSoundEffect(P_CHAR pc);//AntiChrist
	//##ModelId=3C5D92F800A2
	void DoCombatAnimations(P_CHAR pc_attacker, P_CHAR pc_defender, int fightskill, int bowtype, int los);
public:
	//##ModelId=3C5D92F800D4
	int GetBowType(P_CHAR pc);
	//##ModelId=3C5D92F800E8
	int CalcAtt(P_CHAR pc);
	//##ModelId=3C5D92F800FC
	int CalcDef(P_CHAR pc, int x);
	//##ModelId=3C5D92F80110
	void CombatOnHorse(P_CHAR pc);
	//##ModelId=3C5D92F80124
	void CombatOnFoot(P_CHAR pc);
	//##ModelId=3C5D92F80138
	void CombatHitCheckLoS(P_CHAR pAttacker, unsigned int currenttime);
	//##ModelId=3C5D92F80156
	void CombatHit(P_CHAR pc_attacker, P_CHAR pc_deffender, unsigned int currenttime, short los);
	//##ModelId=3C5D92F80188
	void DoCombat(P_CHAR pc_attacker, unsigned int currenttime);
	//##ModelId=3C5D92F801B0
	void SpawnGuard( P_CHAR pc_offender, P_CHAR pc_caller, int x, int y, signed char z);
};

//##ModelId=3C5D92F9013A
class cCommands
{

public:
	//##ModelId=3C5D92F90163
	mstring command_line;


	//##ModelId=3C5D92F903BC
	vector<mstring> params;
	//##ModelId=3C5D92FA0005
	mstring GetAllParams(void);

	//##ModelId=3C5D92FA0019
	void NextCall(int s, int type);
	//void RepairWorld(int s);
	//##ModelId=3C5D92FA0037
	void KillSpawn(int s, int r);
	//##ModelId=3C5D92FA0055
	void RegSpawnMax(int s, int r);
	//##ModelId=3C5D92FA00A5
	void RegSpawnNum(int s, int r, int n);
	//##ModelId=3C5D92FA00CD
	void KillAll(int s, int percent, const char * sysmsg);
	//##ModelId=3C5D92FA00F5
	void AddHere(int s, char z);
	//##ModelId=3C5D92FA011D
	void ShowGMQue(int s, int type);
	//##ModelId=3C5D92FA013B
	void Wipe(int s);
	//##ModelId=3C5D92FA014F
	void WhoCommand(int s, int type, int buttonnum);
	//##ModelId=3C5D92FA0177
	void CPage(int s, char * reason);
	//##ModelId=3C5D92FA01DB
	void GMPage(int s, char * reason);
	//##ModelId=3C5D92FA01F9
	void MakePlace(int s, int i);
	//##ModelId=3C5D92FA0221
	void Command( UOXSOCKET, string );
	//##ModelId=3C5D92FA023F
	void MakeShop(P_CHAR pc_c);
	//##ModelId=3C5D92FA0254
	void DyeItem(int s);
	//##ModelId=3C5D92FA0268
	void SetItemTrigger(int s);
	//##ModelId=3C5D92FA0286
	void SetTriggerType(int s);
	//##ModelId=3C5D92FA029A
	void SetTriggerWord(int s);
	//##ModelId=3C5D92FA02AE
	void SetNPCTrigger(int s);
	//##ModelId=3C5D92FA02C2
	void DupeItem(int s, P_ITEM pi_target, int amount);
	//##ModelId=3C5D92FA02EA
	void Possess(int s);
	//##ModelId=3C5D92FA02FE
	int cmd_offset;
};

#include "Client.h"
// use this value whereever you need to return an illegal z value
const signed char illegal_z = -128;	// reduced from -1280 to -128, to fit in with a valid signed char

//##ModelId=3C5D92FA0358
class cFishing
{
public:
	//##ModelId=3C5D92FA0380
	void FishTarget(P_CLIENT ps);
	//##ModelId=3C5D92FA0394
	void Fish(P_CHAR pc_i);
};

//##ModelId=3C5D930000FE
class cSkills
{
private:
	//##ModelId=3C5D9300014E
	void Tracking(int s, int selection);
	//##ModelId=3C5D9300016C
	int GetSubIngotAmt(int p, char id1, char id2, char color1, char color2);
	//##ModelId=3C5D930001EE
	int DeleSubIngot(int p, int id1, int id2, int color1, int color2, int amount);
	//##ModelId=3C5D930002F2
	void AnvilTarget(int s, int m);
	//##ModelId=3C5D93000324
	void AnvilTargetGold(int s, int m);
	//##ModelId=3C5D93000357
	void AnvilTargetSilver(int s, int m);
	//##ModelId=3C5D930003C5
	void AnvilTargetAdam(int s, int m);
	//##ModelId=3C5D9301004B
	void AnvilTargetMyth(int s, int m);
	//##ModelId=3C5D93010073
	void AnvilTargetBronze(int s, int m);
	//##ModelId=3C5D930100B9
	void AnvilTargetVerite(int s, int m);
	//##ModelId=3C5D93010127
	void AnvilTargetMerk(int s, int m);
	//##ModelId=3C5D9301018B
	void AnvilTargetCop(int s, int m);
	//##ModelId=3C5D9301020D
	void AnvilTargetAgap(int s, int m);
	//##ModelId=3C5D9301025E
	void Hide(int s);
	//##ModelId=3C5D93010272
	void Stealth(int s);
	//##ModelId=3C5D93010290
	void PeaceMaking(int s);
	//##ModelId=3C5D930102C2
	void PlayInstrumentWell(int s, P_ITEM pi);
	//##ModelId=3C5D930102F4
	void PlayInstrumentPoor(int s, P_ITEM pi);
	//##ModelId=3C5D9301031C
	P_ITEM GetInstrument(int s);
	//##ModelId=3C5D9301034E
	void RandomSteal(int s);	
	//##ModelId=3C5D93010376
	void TellScroll(char *menu_name, int player, long item_param);
	//##ModelId=3C5D930103A8
	void CollectAmmo(int s, int a, int b);
	//##ModelId=3C5D930103DA
	void Meditation(int s);
	//##ModelId=3C5D93020024
	int CalcRank(int s,int skill); // by Magius(CHE)
	//##ModelId=3C5D93020056
	void ApplyRank(int s, P_ITEM pi,int rank); // by Magius(CHE)
	//##ModelId=3C5D93020088
	void Zero_Itemmake(int s); // by Magius(CHE)
public:
	//##ModelId=3C5D930200B0
	void RepairTarget(UOXSOCKET s); // Ripper
	//##ModelId=3C5D930200C4
	void SmeltItemTarget(UOXSOCKET s); // Ripper
	//##ModelId=3C5D930200CF
	void TasteIDTarget(int s);
	//##ModelId=3C5D930200EC
	int TrackingDirection(UOXSOCKET s, P_CHAR pc_i);
	//##ModelId=3C5D93020101
	void CreatePotion(P_CHAR pc, char type, char sub, P_ITEM pi_mortar);
	//##ModelId=3C5D93020129
	char AdvanceSkill(P_CHAR pc, int sk, char skillused);
	//##ModelId=3C5D93020151
	void AdvanceStats(P_CHAR pc, int sk);
	//##ModelId=3C5D9302016F
	void TinkerAxel(int s);
	//##ModelId=3C5D93020179
	void TinkerAwg(int s);
	//##ModelId=3C5D9302018D
	void TinkerClock(int s);
	//##ModelId=3C5D930201A1
	void MakeDough(int s);
	//##ModelId=3C5D930201B5
	void MakePizza(int s);
	//##ModelId=3C5D930201C9
	void Track(P_CHAR pc_i);
	//##ModelId=3C5D930201DD
	void DoPotion(int s, int type, int sub, P_ITEM mortar);
	//##ModelId=3C5D93020237
	void Tailoring(int s);
	//##ModelId=3C5D9302029B
	void Fletching(int s);
	//##ModelId=3C5D930202AF
	void BowCraft(int s);
	//##ModelId=3C5D930202C3
	void Carpentry(int s);
	//##ModelId=3C5D930202D7
	void Smith(int s);
	//##ModelId=3C5D930202EB
	void MakeMenuTarget(int s, int x, int skill);
	//##ModelId=3C5D93020313
	void MakeMenu(int s, int m, int skill);
	//##ModelId=3C5D93020331
	void Mine(int s);
	//##ModelId=3C5D93020345
	void GraveDig(int s);
	//##ModelId=3C5D93020363
	void SmeltOre(int s);
	//##ModelId=3C5D93020377
	void Wheel(int s, int mat);
	//##ModelId=3C5D9302038B
	void Loom(int s);
	//##ModelId=3C5D9302039F
	void CookOnFire(int s, short id1, short id2, char* matname);
	//##ModelId=3C5D930203D2
	void TreeTarget(int s);
	//##ModelId=3C5D930203E6
	void DetectHidden(UOXSOCKET s);
	//##ModelId=3C5D93030012
	void ProvocationTarget1(UOXSOCKET s);
	//##ModelId=3C5D93030026
	void ProvocationTarget2(UOXSOCKET s);
	//##ModelId=3C5D9303003A
	void EnticementTarget1(UOXSOCKET s);
	//##ModelId=3C5D9303004E
	void EnticementTarget2(UOXSOCKET s);
	//##ModelId=3C5D93030058
	void AlchemyTarget(int s);
	//##ModelId=3C5D9303006C
	void BottleTarget(int s);
	//##ModelId=3C5D9303008A
	void PotionToBottle(P_CHAR pc, P_ITEM pi_mortar);
	//##ModelId=3C5D9303009E
	char CheckSkill(P_CHAR pc, unsigned short int sk, int low, int high);
	//##ModelId=3C5D93030102
	void CreateBandageTarget(int s);
	//##ModelId=3C5D93030120
	void HealingSkillTarget(UOXSOCKET s);
	//##ModelId=3C5D93030148
	void SpiritSpeak(int s);
	//##ModelId=3C5D9303015C
	void ArmsLoreTarget(int s);
	//##ModelId=3C5D93040352
	void ItemIdTarget(int s);
	//##ModelId=3C5D9304037A
	void Evaluate_int_Target(UOXSOCKET s);
	//##ModelId=3C5D9304038E
	void AnatomyTarget(int s);
	//##ModelId=3C5D930403A2
	void TameTarget(int s);
	//##ModelId=3C5D930403B6
	int GetCombatSkill(P_CHAR pc);
	//##ModelId=3C5D930403CA
	void SkillUse(int s, int x);
	//##ModelId=3C5D93050000
	void StealingTarget(int s);
	//##ModelId=3C5D9305000B
	void PickPocketTarget(int s);
	//##ModelId=3C5D93050029
	void CreateTrackingMenu(int s, int m);
	//##ModelId=3C5D9305003D
	void TrackingMenu(int s, int gmindex);
	//##ModelId=3C5D9305005B
	void BeggingTarget(int s);
	//##ModelId=3C5D9305006F
	void AnimalLoreTarget(int s);
	//##ModelId=3C5D93050083
	void ForensicsTarget(int s);
	//##ModelId=3C5D93050097
	void PoisoningTarget(int s);
	//##ModelId=3C5D930500A1
	int Inscribe(int s, long snum);
	//##ModelId=3C5D930500BF
	int EngraveAction(int s, P_ITEM pi, int cir, int spl);
	//##ModelId=3C5D930500E7
	void updateSkillLevel(P_CHAR pc, int s);
	//##ModelId=3C5D93050155
	void LockPick(int s);
	//##ModelId=3C5D9305017D
	void TDummy(int s);
	//##ModelId=3C5D93050191
	void Tinkering(int s);
	//##ModelId=3C5D930501A5
	void AButte(int s1, P_ITEM pButte);
	//##ModelId=3C5D930501C3
	void Persecute(UOXSOCKET s);//AntiChrist persecute stuff
	//##ModelId=3C5D930501CD
	void Cartography(int s); // By Polygon - opens the cartography skillmenu
	//##ModelId=3C5D930501F5
	bool HasEmptyMap(P_CHAR pc); // By Polygon - checks if player has an empty map
	//##ModelId=3C5D93050231
	bool DelEmptyMap(P_CHAR pc); // By Polygon - deletes an empty map from the player's pack
	//##ModelId=3C5D93050245
	void Decipher(P_ITEM tmap, int s); // By Polygon - attempt to decipher a tattered treasure map
	//##ModelId=3C5D930502D2
	int GetAntiMagicalArmorDefence(P_CHAR pc); // blackwind meditation armor stuff
	//##ModelId=3C5D930502F0
	void Snooping(P_CHAR, P_ITEM);
};



//##ModelId=3C5D930503D6
class cWeight
{
public:
	//##ModelId=3C5D9306000C
	int CheckWeight(UOXSOCKET s);
	//##ModelId=3C5D93060020
	float RecursePacks(P_ITEM bp);
	//##ModelId=3C5D93060052
	int CheckWeight2(int s);
	//##ModelId=3C5D9306005D
	void NewCalc(P_CHAR pc);
	//##ModelId=3C5D930600A2
	float LockeddownWeight(P_ITEM pItem, int *total, int *total2 );
};

#include "targeting.h"


//##ModelId=3C5D930E0248
class cMagic
{
public:
	//##ModelId=3C5D930E0284
	char CheckResist(P_CHAR attacker, P_CHAR defender, int circle);
	//##ModelId=3C5D930E02AC
	void PoisonDamage(P_CHAR pc, int poison);
	//##ModelId=3C5D930E02C0
	void CheckFieldEffects2(unsigned int currenttime, P_CHAR pc, char timecheck);
	//##ModelId=3C5D930E02E8
	int InitSpells( void );
	//##ModelId=3C5D930E02FD
	void SpellBook(UOXSOCKET s, P_ITEM pi);
	//##ModelId=3C5D930E031B
	int  SpellsInBook(P_ITEM pi);
	//##ModelId=3C5D930E0339
	char GateCollision(P_CHAR pc_player);
	//##ModelId=3C5D930E034D
	bool newSelectSpell2Cast( int s, int num );
	//##ModelId=3C5D930E0361
	bool requireTarget( unsigned char num );
	//##ModelId=3C5D930E0375
	bool reqItemTarget( int num );
	//##ModelId=3C5D930E0389
	bool reqLocTarget( int num );
	//##ModelId=3C5D930E039D
	bool reqCharTarget( int num );
	//##ModelId=3C5D930E03BC
	bool spellReflectable( int num );
	//##ModelId=3C5D930E03CF
	bool travelSpell( int num );
	//##ModelId=3C5D930E03E3
	bool townTarget( unsigned char num );
	//##ModelId=3C5D930F000F
	stat_st getStatEffects( int num );
	//##ModelId=3C5D930F0023
	stat_st getStatEffects_pre( int num);
	//##ModelId=3C5D930F0037
	stat_st getStatEffects_after( int num);
	//##ModelId=3C5D930F004B
	stat_st getStatEffects_item( int num);
	//##ModelId=3C5D930F005F
	void preParticles(int num, P_CHAR pc);
	//##ModelId=3C5D930F007D
	void afterParticles(int num, P_CHAR pc);
	//##ModelId=3C5D930F0091
	void itemParticles(int num, P_ITEM pi);
	//##ModelId=3C5D930F00AF
	void invisibleItemParticles(P_CHAR pc, int num, short x, short y, signed char z);
	//##ModelId=3C5D930F00E1
	move_st getMoveEffects( int num );
	//##ModelId=3C5D930F00F5
	sound_st getSoundEffects( int num );
	//##ModelId=3C5D930F0100
	bool aggressiveSpell( int num );
	//##ModelId=3C5D930F0113
	bool fieldSpell( int num );
	//##ModelId=3C5D930F0127
	void doMoveEffect( int num, P_CHAR target, P_CHAR source );
	//##ModelId=3C5D930F0145
	void doStaticEffect( P_CHAR source, int num );
	//##ModelId=3C5D930F0163
	void playSound( P_CHAR pc_source, int num );
//	void NewDelReagents( CHARACTER s, reag_st reags );
	
	//##ModelId=3C5D930F01B4
	void DelReagents( P_CHAR pc, reag_st reags );

	//##ModelId=3C5D930F01D2
	void NewCastSpell( UOXSOCKET s );
	//##ModelId=3C5D930F01E6
	void SbOpenContainer(int s);
	//##ModelId=3C5D930F01FA
	void PFireballTarget(P_CHAR pc_i, P_CHAR pc, int j);
	//##ModelId=3C5D930F0222
	void LightningSpell(P_CHAR pc_Attacker, P_CHAR pc_Defender, bool usemana = true);
	//##ModelId=3C5D930F0240
	void EnergyBoltSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D930F0268
	void NPCHeal(P_CHAR pc);
	//##ModelId=3C5D930F036C
	void NPCCure(P_CHAR pc);
	//##ModelId=3C5D930F0380
	void NPCDispel(P_CHAR pc_s, P_CHAR pc_i);
	//##ModelId=3C5D930F039E
	char CheckParry(P_CHAR pc_player, int circle);
	//##ModelId=3C5D930F03BC
	void MagicArrow(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D930F03E4
	void ClumsySpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D9310001A
	void FeebleMindSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D93100042
	void WeakenSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D93100074
	void HarmSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D93100092
	void FireballSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D931000BB
	void CurseSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D931000D9
	void MindBlastSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D93100101
	void ParalyzeSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usenama = true);
	//##ModelId=3C5D9310011F
	void ExplosionSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D93100147
	void FlameStrikeSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana = true);
	//##ModelId=3C5D93100165
	bool CheckBook(int circle, int spell, P_ITEM pi);
	//##ModelId=3C5D9310018D
	char CheckReagents(P_CHAR pc, reag_st reagents);


	//##ModelId=3C5D931001AB
	char CheckMana(P_CHAR pc, int num);
//	void DeleReagents(int s, int ash, int drake, int garlic, int ginseng, int moss, int pearl, int shade, int silk);

	//##ModelId=3C5D931001C9
    void Recall(UOXSOCKET s); // we need this four ones for the commands (separatly) !!!
	//##ModelId=3C5D931001DD
    void Mark(UOXSOCKET s);
	//##ModelId=3C5D931001F1
    void Gate(UOXSOCKET s);
	//##ModelId=3C5D93100205
    void Heal(UOXSOCKET s);

	//##ModelId=3C5D93100210
	void BuildCannon(int s);
	//##ModelId=3C5D93100223
	bool CheckMagicReflect(P_CHAR pc);
	//##ModelId=3C5D93100237
	P_CHAR CheckMagicReflect(P_CHAR &attacker, P_CHAR &defender);
	//##ModelId=3C5D93100255
	void MagicDamage(P_CHAR pc, int amount);
	//##ModelId=3C5D93100291
	void SpellFail(int s);
	//##ModelId=3C5D931002A5
	char SubtractMana(P_CHAR pc, int mana);
	//##ModelId=3C5D931002C3
	void MagicTrap(P_CHAR pc, P_ITEM pTrap);	// moved here by AntiChrist (9/99)
	//##ModelId=3C5D931002E1
	void Polymorph( int s, int gmindex, int creaturenumer); // added by AntiChrist (9/99)
	//##ModelId=3C5D931002FF
	short SpellNumFromScrollID(short id);


private:

	//##ModelId=3C5D93100313
	void SummonMonster(int s, unsigned char d1, unsigned char id2, char *monstername, unsigned char color1, unsigned char color2, int x, int y, int z, int spellnum);
//	void MagicReflect(int s);
	//##ModelId=3C5D931003DD
	void BoxSpell(int s, int& x1, int& x2, int& y1, int& y2, int& z1, int& z2);
	//##ModelId=3C5D93110044
	int RegMsg(P_CHAR pc, reag_st failmsg);
//	void DirectDamage(int p, int amount);
	//##ModelId=3C5D93110062
	void PolymorphMenu( int s, int gmindex ); // added by AntiChrist (9/99)
};

#endif
