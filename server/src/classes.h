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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

//---Classes code by Zippy Started 7/29/99---//

#ifndef __Classes_h
#define __Classes_h

#include "assert.h"
#include "verinfo.h"
#include "mstring.h"

#include "start_pack.h"
struct tile_st
{
 SI32 unknown1;  // longs must go at top to avoid bus errors - fur
 SI32 animation;
 unsigned char flag1;
 unsigned char flag2;
 unsigned char flag3;
 unsigned char flag4;
 unsigned char weight;
 signed char layer;
 signed char unknown2;
 signed char unknown3;
 signed char height;
 signed char name[23];	// manually padded to long to avoid bus errors - fur | There is no negative letter.
} PACK ;
#include "end_pack.h"

struct land_st
{
 char flag1;
 char flag2;
 char flag3;
 char flag4;
 char unknown1;
 char unknown2;
 char name[20];
};

struct map_st
{
	short int id;
	signed char z;
};

struct ip_block_st
{
	unsigned long address;
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
class cCombat
{
private:
	void ItemCastSpell(UOXSOCKET s, CHARACTER c, P_ITEM pi);
	int TimerOk(int c);
	void ItemSpell(int attacker, int defender);
	void doSoundEffect(CHARACTER p, int fightskill, P_ITEM pWeapon);//AntiChrist
	void doMissedSoundEffect(CHARACTER p);//AntiChrist
	void DoCombatAnimations(P_CHAR pc_attacker, P_CHAR pc_defender, int fightskill, int bowtype, int los);
public:
	int GetBowType(int i);
	int CalcAtt(int p);
	int CalcDef(P_CHAR pc, int x);
	void CombatOnHorse(int i);
	void CombatOnFoot(int i);
	void CombatHitCheckLoS(P_CHAR pAttacker, unsigned int currenttime);
	void CombatHit(int a, int d, unsigned int currenttime, short los);
	void DoCombat(int a, unsigned int currenttime);
	void SpawnGuard( CHARACTER s, CHARACTER i, int x, int y, signed char z);
};

class cCommands
{

public:
	mstring command_line;


	vector<mstring> params;
	mstring GetAllParams(void);

	void NextCall(int s, int type);
	//void RepairWorld(int s);
	void KillSpawn(int s, int r);
	void RegSpawnMax(int s, int r);
	void RegSpawnNum(int s, int r, int n);
	void KillAll(int s, int percent, const char * sysmsg);
	void AddHere(int s, char z);
	void ShowGMQue(int s, int type);
	void Wipe(int s);
	void WhoCommand(int s, int type, int buttonnum);
	void CPage(int s, char * reason);
	void GMPage(int s, char * reason);
	void MakePlace(int s, int i);
	void Command( UOXSOCKET, string );
	void MakeShop(int c);
	void DyeItem(int s);
	void SetItemTrigger(int s);
	void SetTriggerType(int s);
	void SetTriggerWord(int s);
	void SetNPCTrigger(int s);
	void DupeItem(int s, int i, int amount);
	void Possess(int s);
	int cmd_offset;
};

class cGuilds
{
private:
	void EraseMember(int c);
	void EraseGuild(int guildnumber);
	void ToggleAbbreviation(int s);
	int SearchSlot(int guildnumber, int type);
	void ChangeName(int s, char *text);
	void ChangeAbbreviation(int s, char *text);
	void ChangeTitle(int s, char *text);
	void ChangeCharter(int s, char *text);
	void ChangeWebpage(int s, char *text);
	int CheckValidPlace(int s);
	void Broadcast(int guildnumber, char *text);
	void CalcMaster(int guildnumber);
	void SetType(int guildnumber, int type);
public:
	guild_st guilds[MAXGUILDS]; //lb, moved from WOLFPACK.h cauz global variabels cant be changed in constuctors ...
	cGuilds();
	virtual ~cGuilds();
	int	GetType(int guildnumber);
	void StonePlacement(int s);
	void Menu(int s, int page);
	void Resign(int s);
	void Recruit(int s);
	void TargetWar(int s);
	void StoneMove(int s);
	int Compare(int player1, int player2);
	void GumpInput(int s, int type, int index, char *text);
	void GumpChoice(int s, int main, int sub);
	int SearchByStone(int s);
	void Title(int s, int player2);
	void Read(int guildnumber);
	void Write(FILE *wscfile);
	void CheckConsistancy(void);
};

class MapStaticIterator
{
private:
	staticrecord staticArray;
	SI32 baseX, baseY, pos;
	unsigned char remainX, remainY;
	UI32 index, length, tileid;
	bool exactCoords;

public:
	MapStaticIterator(unsigned int x, unsigned int y, bool exact = true);
	~MapStaticIterator() { };

	staticrecord *First();
	staticrecord *Next();
	void GetTile(tile_st *tile) const;
	UI32 GetPos() const { return pos; }
	UI32 GetLength() const { return length; }
};

#if ILSHENAR == 1
  const int MapTileWidth  = 288;
  const int MapTileHeight = 200;
#else
  const int MapTileWidth  = 768;
  const int MapTileHeight = 512;
#endif

class cMapStuff
{
//Variables
private:
	friend class MapStaticIterator;
	friend class cMovement;

        // moved from global vars into here - fur 11/3/1999
        UOXFile *mapfile, *sidxfile, *statfile, *verfile, *tilefile, *multifile, *midxfile;

	// tile caching items
	tile_st tilecache[0x4000];

	// static caching items
	unsigned long StaticBlocks;
	struct StaCache_st
	{
		staticrecord *Cache;
		unsigned short CacheLen;   // i've seen this goto to at least 273 - fur 10/29/1999
	};

	// map caching items
	struct MapCache
	{
		unsigned short xb;
		unsigned short yb;
		unsigned char  xo;
		unsigned char  yo;
		map_st Cache;
	};
	MapCache Map0Cache[MAP0CACHE];

	// version caching items
	versionrecord *versionCache;
	UI32 versionRecordCount;

	// caching functions
	void CacheTiles( void );
	void CacheStatics( void );

public:
	// these used to be [512], thats a little excessive for a filename.. - fur
	char mapname[80], sidxname[80], statname[80], vername[80],
	  tilename[80], multiname[80], midxname[80];
	unsigned long StaMem, TileMem, versionMemory;
	unsigned int Map0CacheHit, Map0CacheMiss;
	// ok this is rather silly, allocating all the memory for the cache, even if
	// they haven't chosen to cache?? - fur
	StaCache_st StaticCache[MapTileWidth][MapTileHeight];
	unsigned char Cache;
	
// Functions
private:
	char VerLand(int landnum, land_st *land);
	signed char MultiHeight(int i, short int x, short int y, signed char oldz);
	int MultiTile(int i, short int x, short int y, signed char oldz);
	SI32 VerSeek(SI32 file, SI32 block);
	char VerTile(int tilenum, tile_st *tile);
	bool IsTileWet(int tilenum);
	bool TileWalk(int tilenum);
	void CacheVersion();

	int DynTile( short int x, short int y, signed char oldz );
	bool DoesTileBlock(int tilenum);
	bool DoesStaticBlock(short int x, short int y, signed char oldz);

public:
	cMapStuff();
	~cMapStuff();

	void Load();

	// height functions
	bool IsUnderRoof(short int x, short int y, signed char z);
	signed char StaticTop(short int x, short int y, signed char oldz);
	signed char DynamicElevation(short int x, short int y, signed char oldz);
	signed char MapElevation(short int x, short int y);
	signed char AverageMapElevation(short int x, short int y, int &id);
	signed char TileHeight( int tilenum );
	signed char Height(short int x, short int y, signed char oldz);

	// look at tile functions
	void MultiArea(int i, int *x1, int *y1, int *x2, int *y2);
	void SeekTile(int tilenum, tile_st *tile);
	void SeekMulti(int multinum, UOXFile **mfile, SI32 *length);
	void SeekLand(int landnum, land_st *land);
	map_st SeekMap0( unsigned short x, unsigned short y );
	bool IsRoofOrFloorTile( tile_st *tile );
	bool IsRoofOrFloorTile( unitile_st *tile );

	// misc functions
	bool CanMonsterMoveHere( short int x, short int y, signed char z );

};

#include "Client.h"
// use this value whereever you need to return an illegal z value
const signed char illegal_z = -128;	// reduced from -1280 to -128, to fit in with a valid signed char

class cFishing
{
public:
	void FishTarget(P_CLIENT ps);
	void Fish(CHARACTER c);
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
	void PlayInstrumentWell(int s, int i);
	void PlayInstrumentPoor(int s, int i);
	int GetInstrument(int s);
	void RandomSteal(int s);	
	void TellScroll(char *menu_name, int player, long item_param);
	void CollectAmmo(int s, int a, int b);
	void Meditation(int s);
	int CalcRank(int s,int skill); // by Magius(CHE)
	void ApplyRank(int s,int c,int rank); // by Magius(CHE)
	void Zero_Itemmake(int s); // by Magius(CHE)
public:
	void RepairTarget(UOXSOCKET s); // Ripper
	void SmeltItemTarget(UOXSOCKET s); // Ripper
	void TasteIDTarget(int s);
	int TrackingDirection(int s, int i);
	void CreatePotion(CHARACTER s, char type, char sub, int mortar);
	char AdvanceSkill(CHARACTER s, int sk, char skillused);
	void AdvanceStats(CHARACTER s, int sk);
	void TinkerAxel(int s);
	void TinkerAwg(int s);
	void TinkerClock(int s);
	void MakeDough(int s);
	void MakePizza(int s);
	void Track(int i);
	void DoPotion(int s, int type, int sub, int mortar);
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
	void PotionToBottle(CHARACTER s, int mortar);
	char CheckSkill(int s,unsigned short int sk, int low, int high);
	void CreateBandageTarget(int s);
	void HealingSkillTarget(int s);
	void SpiritSpeak(int s);
	void ArmsLoreTarget(int s);
	void ItemIdTarget(int s);
	void Evaluate_int_Target(UOXSOCKET s);
	void AnatomyTarget(int s);
	void TameTarget(int s);
	int GetCombatSkill(int i);
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
	int EngraveAction(int s, int i, int cir, int spl);
	void updateSkillLevel(int c, int s);
	void LockPick(int s);
	void TDummy(int s);
	void NewDummy(unsigned int currenttime);
	void Tinkering(int s);
	void AButte(int s1, P_ITEM pButte);
	void Persecute(UOXSOCKET s);//AntiChrist persecute stuff
	void Cartography(int s); // By Polygon - opens the cartography skillmenu
	bool HasEmptyMap(int cc); // By Polygon - checks if player has an empty map
	bool DelEmptyMap(int cc); // By Polygon - deletes an empty map from the player's pack
	void Decipher(P_ITEM tmap, int s); // By Polygon - attempt to decipher a tattered treasure map
	int GetAntiMagicalArmorDefence(int p); // blackwind meditation armor stuff
	void Snooping(P_CHAR, P_ITEM);
};



class cWeight
{
public:
	int CheckWeight(int s, int k);
	float RecursePacks(int bp);
	int CheckWeight2(int s);
	void NewCalc(int p);
	float LockeddownWeight(P_ITEM pItem, int *total, int *total2 );
};

#include "targeting.h"


class cMagic
{
public:
	char CheckResist(CHARACTER attacker, CHARACTER defender, int circle);
	void PoisonDamage(CHARACTER p, int poison);
	void CheckFieldEffects2(unsigned int currenttime, CHARACTER c,char timecheck);
	int InitSpells( void );
	void SpellBook(UOXSOCKET s, ITEM si);
	int  SpellsInBook(ITEM i);
	char GateCollision(int s);
	bool newSelectSpell2Cast( int s, int num );
	bool requireTarget( unsigned char num );
	bool reqItemTarget( int num );
	bool reqLocTarget( int num );
	bool reqCharTarget( int num );
	bool spellReflectable( int num );
	bool travelSpell( int num );
	bool townTarget( unsigned char num );
	stat_st getStatEffects( int num );
	stat_st getStatEffects_pre( int num);
	stat_st getStatEffects_after( int num);
	stat_st getStatEffects_item( int num);
	void preParticles(int num, CHARACTER c);
	void afterParticles(int num, CHARACTER c);
	void itemParticles(int num, P_ITEM pi);
	void invisibleItemParticles(CHARACTER c, int num, short x, short y, signed char z);
	move_st getMoveEffects( int num );
	sound_st getSoundEffects( int num );
	bool aggressiveSpell( int num );
	bool fieldSpell( int num );
	void doMoveEffect( int num, int target, int source );
	void doStaticEffect( int source, int num );
	void playSound( int source, int num );
//	void NewDelReagents( CHARACTER s, reag_st reags );
	
	void DelReagents( CHARACTER s, reag_st reags );

	void NewCastSpell( UOXSOCKET s );
	void SbOpenContainer(int s);
	void PFireballTarget(int i, int k, int j);
	void NPCLightningTarget(int nAttacker, int nDefender);
	void NPCEBoltTarget(int s, int t);
	void NPCHeal(int s);
	void NPCCure(int s);
	void NPCDispel(int s, int i);
	void NPCCannonTarget(int s, int t);
	char CheckParry(int player, int circle);
	void NPCFlameStrikeTarget(int s, int t);
	void MagicArrow(int attacker, int defender, bool usemana = true);
	void ClumsySpell(int attacker, int defender, bool usemana = true);
	void FeebleMindSpell(int attacker, int defender, bool usemana = true);
	void WeakenSpell(int attacker, int defender, bool usemana = true);
	void HarmSpell(int attacker, int defender, bool usemana = true);
	void FireballSpell(CHARACTER attacker, CHARACTER defender, bool usemana = true);
	void CurseSpell(int attacker, int defender, bool usemana = true);
	void LightningSpellItem(int attacker, int defender);
	void MindBlastSpell(int attacker, int defender, bool usemana = true);
	void ParalyzeSpell(int attacker, int defender, bool usenama = true);
	void ExplosionSpell(int attacker, int defender, bool usemana = true);
	void FlameStrikeSpellItem(int attacker, int defender);
	int CheckBook(int circle, int spell, int i);
	char CheckReagents(int s, reag_st reagents);


	char CheckMana(int s, int num);
//	void DeleReagents(int s, int ash, int drake, int garlic, int ginseng, int moss, int pearl, int shade, int silk);

    void Recall(UOXSOCKET s); // we need this four ones for the commands (separatly) !!!
    void Mark(UOXSOCKET s);
    void Gate(UOXSOCKET s);
    void Heal(UOXSOCKET s);

	void BuildCannon(int s);
	bool CheckMagicReflect(CHARACTER i);
	P_CHAR CheckMagicReflect(P_CHAR &attacker, P_CHAR &defender);
	void MagicDamage(int p, int amount);
	void MagicDamage(P_CHAR pc, int amount);
	void SpellFail(int s);
	char SubtractMana(P_CHAR pc, int mana);
	void MagicTrap(int s, P_ITEM pTrap);	// moved here by AntiChrist (9/99)
	void Polymorph( int s, int gmindex, int creaturenumer); // added by AntiChrist (9/99)
	short SpellNumFromScrollID(short id);


private:

	void SummonMonster(int s, unsigned char d1, unsigned char id2, char *monstername, unsigned char color1, unsigned char color2, int x, int y, int z, int spellnum);
//	void MagicReflect(int s);
	void BoxSpell(int s, int& x1, int& x2, int& y1, int& y2, int& z1, int& z2);
	int RegMsg(CHARACTER s, reag_st failmsg);
//	void DirectDamage(int p, int amount);
	void PolymorphMenu( int s, int gmindex ); // added by AntiChrist (9/99)
};

#endif
