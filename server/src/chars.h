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

#ifndef __CHARS_H
#define __CHARS_H

// Platform Include
#include "platform.h"


// just temporary
#include "debug.h"

#include "typedefs.h"
#include "structs.h"
#include "defines.h"
#include "uobject.h"

#undef  DBGFILE
#define DBGFILE "chars.h"


//typedef struct char_st_
class cChar : public cUObject
{
// Public Data Members
public:
    enum enInputMode { enNone, enRenameRune, enPricing, enDescription, enNameDeed, enHouseSign, enPageGM, enPageCouns};
	//  Chaos/Order Guild Stuff for Ripper
	short					GuildType;    // (0) Standard guild, (1) Chaos Guild, (2) Order guild
	bool					GuildTraitor; // (true) This character converted, (false) Neve converted, or not an order/chaos guild member
	RACE					race; // -Fraz- Race AddOn
	// Skyfire's NPC advancments.
	bool					may_levitate;
	//Skyfire - End NPC's home/work/food vars'
	unsigned char			pathnum;
	path_st					path[PATHNUM];

	string					orgname;//original name - for Incognito
	string					title;
	bool					unicode; // This is set to 1 if the player uses unicode speech, 0 if not
	int						account; // changed to signed, lb
	signed char				dispz;   // Z that the char is SHOWN at. Server needs other coordinates for real movement calculations.
	// changed from unsigned to signed, LB
	
	unsigned char			dir; //&0F=Direction
	unsigned char			id1; // Character body type
	unsigned char			id2; // Character body type
	unsigned short			xid; // Backup of body type for ghosts
	bool					incognito;// AntiChrist - true if under incognito effect
	bool					polymorph;// AntiChrist - true if under polymorph effect
	unsigned short			haircolor; // backup of hair/beard for incognito spell
	unsigned short			hairstyle;
	unsigned short			beardcolor;
	unsigned short			beardstyle;
	UI16					skin; // Skin color
	unsigned short			orgskin;	// skin color backup for incognito spell
	UI16					xskin; // Backup of skin color
	unsigned int			creationday;	// Day since EPOCH this character was created on
	unsigned char			gmrestrict;	// for restricting GMs to certain regions
	unsigned char			priv2;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	int			            priv3[7];  // needed for Lord binarys meta-gm stuff
	unsigned char			fonttype; // Speech font to use
	UI16					saycolor; // Color for say messages
	unsigned short			emotecolor; // Color for emote messages
	signed short			st; // Strength
	signed short			st2; // Reserved for calculation
	signed short			in; // Intelligence
	signed short			in2; // Reserved for calculation
	signed short			hp; // Hitpoints
	signed short			stm; // Stamina
	signed short			mn; // Mana
	signed short			mn2; // Reserved for calculation
	int						hidamage; //NPC Damage
	int						lodamage; //NPC Damage
	unsigned short int		baseskill[ALLSKILLS+1]; // Base skills without stat modifiers
	unsigned short int		skill[ALLSKILLS+1]; // List of skills (with stat modifiers)
	bool					npc;	// true = Character is an NPC
	bool					shop;	// true = npc shopkeeper
	unsigned char			cell; // Reserved for jailing players
	unsigned int			jailtimer; // Blackwind - Timer used for crystall ball and jail time.
	int						jailsecs;	//             Tweak this value by using command tweak before jailing person 
	//			   or he will be jailed 1 day ( in realtime )
	int						ownserial; // If Char is an NPC, this sets its owner
	int						robe; // Serial number of generated death robe (If char is a ghost)
	int						karma;
	signed int				fame;
	unsigned int			kills; //PvP Kills
	unsigned int			deaths;
	bool					dead; // Is character dead
	SERIAL					packitem; // Serial of backpack
	unsigned char			fixedlight; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	unsigned char			speech; // For NPCs: Number of the assigned speech block
	int						weight; //Total weight
	unsigned int			att; // Intrinsic attack (For monsters that cant carry weapons)
	unsigned int			def; // Intrinsic defense
	bool					war; // War Mode
	SERIAL					targ; // Current combat target
	unsigned int			timeout; // Combat timeout (For hitting)
	unsigned int			timeout2; // memory of last shot timeout
	unsigned int			regen, regen2, regen3;//Regeneration times for mana, stamin, and str
	enInputMode				inputmode;	// Used for entering text; 0= none, 4=rename rune
	SERIAL					inputitem;		// serial of item the text is referring to
	SERIAL					attacker; // Character's serial who attacked this character
	unsigned int			npcmovetime; // Next time npc will walk
	unsigned char			npcWander; // NPC Wander Mode
	unsigned char			oldnpcWander; // Used for fleeing npcs
	SERIAL					ftarg; // NPC Follow Target
	int						fx1; //NPC Wander Point 1 x
	int						fx2; //NPC Wander Point 2 x
	int						fy1; //NPC Wander Point 1 y
	int						fy2; //NPC Wander Point 2 y
	signed char				fz1; //NPC Wander Point 1 z
	SERIAL					spawnserial; // Spawned by
	unsigned char			hidden; // 0 = not hidden, 1 = hidden, 2 = invisible spell
	unsigned int			invistimeout;
	bool					attackfirst; // 0 = defending, 1 = attacked first
	bool					onhorse; // On a horse?
	int						hunger;  // Level of hungerness, 6 = full, 0 = "empty"
	unsigned int			hungertime; // Timer used for hunger, one point is dropped every 20 min
	SERIAL					smeltitem;
	SERIAL					tailitem;
	int						npcaitype; // NPC ai
	int						callnum; //GM Paging
	int						playercallnum; //GM Paging
	//int pagegm; //GM Paging
	//char region;
	unsigned char			region;
	unsigned int			skilldelay;
	unsigned int			objectdelay;
	int						combathitmessage;
	int						making; // skill number of skill using to make item, 0 if not making anything.
	SERIAL					lastTarget;
	char					blocked;
	char					dir2;
	unsigned int			spiritspeaktimer; // Timer used for duration of spirit speak
	int						spattack;
	int						spadelay;
	unsigned int			spatimer;
	int						taming; //Skill level required for taming
	unsigned int			summontimer; //Timer for summoned creatures.
	unsigned int			trackingtimer; // Timer used for the duration of tracking
	SERIAL					trackingtarget; // Tracking target ID
	SERIAL					trackingtargets[MAXTRACKINGTARGETS];
	unsigned int			fishingtimer; // Timer used to delay the catching of fish
	int						advobj; //Has used advance gate?
	int						poison; // used for poison skill 
	unsigned int			poisoned; // type of poison
	unsigned int			poisontime; // poison damage timer
	unsigned int			poisontxt; // poision text timer
	unsigned int			poisonwearofftime; // LB, makes poision wear off ...
	short					fleeat;
	short					reattackat;
	int						trigger; //Trigger number that character activates
	string					trigword; //Word that character triggers on.
	unsigned int			disabled; //Character is disabled, cant trigger.
	string					disabledmsg; //Character is disabled, so dysplay this message. -- added by Magius(CHE) §
	unsigned short			envokeid;  //ID1 of item user envoked
	SERIAL					envokeitem;
	unsigned char			split;
	unsigned char			splitchnc;
	int						targtrig; //Stores the number of the trigger the character for targeting
	char					ra;  // Reactive Armor spell
	int						trainer; // Serial of the NPC training the char, -1 if none.
	char					trainingplayerin; // Index in skillname of the skill the NPC is training the player in
	bool					cantrain;
	// Begin of Guild Related Character information (DasRaetsel)
	bool					guildtoggle;		// Toggle for Guildtitle								(DasRaetsel)
	string					guildtitle;			// Title Guildmaster granted player						(DasRaetsel)
	SERIAL					guildfealty;		// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	SERIAL					guildstone;			// Number of guild player is in (0=no guild)			(DasRaetsel)
	char					flag; //1=red 2=grey 4=Blue 8=green 10=Orange
	unsigned int			tempflagtime;
	// End of Guild Related Character information
	unsigned int			murderrate; //#of ticks until one murder decays //REPSYS 
	long int				crimflag; //Time when No longer criminal -1=Not Criminal
	int						casting; // 0/1 is the cast casting a spell?
	unsigned int			spelltime; //Time when they are done casting....
	int						spell; //current spell they are casting....
	int						spellaction; //Action of the current spell....
	int						nextact; //time to next spell action....
	SERIAL					poisonserial; //AntiChrist -- poisoning skill
	int						squelched; // zippy  - squelching
	int						mutetime; //Time till they are UN-Squelched.
	bool					med; // 0=not meditating, 1=meditating //Morrolan - Meditation 
	//int statuse[3]; //Morrolan - stat/skill cap STR/INT/DEX in that order
	//int skilluse[TRUESKILLS][1]; //Morrolan - stat/skill cap
	unsigned char			lockSkill[ALLSKILLS+1]; // LB, client 1.26.2b skill managment
	int						stealth; //AntiChrist - stealth ( steps already done, -1=not using )
	unsigned int			running; //AntiChrist - Stamina Loose while running
	unsigned int			logout; //unsigned int logout;//Time till logout for this char -1 means in the world or already logged out //Instalog
	unsigned int			clientidletime; // LB
	SERIAL					swingtarg; //Tagret they are going to hit after they swing
	unsigned int			holdg; // Gold a player vendor is holding for Owner
	unsigned char			fly_steps; // number of step the creatures flies if it can fly
	unsigned int			trackingdisplaytimer;
	int						menupriv; // needed fro LB's menu priv system
	//taken from 6904t2(5/10/99) - AntiChrist
	bool					tamed;
	//taken from 6904t2(5/10/99) - AntiChrist
	bool					guarded;							// (Abaddon) if guarded
    unsigned int			smoketimer; // LB
	unsigned int			smokedisplaytimer;
	unsigned int			antispamtimer;//LB - anti spam
	unsigned int			antiguardstimer;//AntiChrist - anti "GUARDS" spawn
	int						carve; //AntiChrist - for new carve system
	int						hairserial;//there are needed for incognito stuff
	int						beardserial;
	unsigned int			begging_timer;
	int						postType;
	int						questType;
	int						questDestRegion;
	int						questOrigRegion;
	int						questBountyReward;      // The current reward amount for the return of this chars head
	int						questBountyPostSerial;  // The global posting serial number of the bounty message
	SERIAL					murdererSer;            // Serial number of last person that murdered this char
	int						prevX; // fix for looping gate travel bug (bounce back problem)
	int						prevY;
	signed char 			prevZ;
	unsigned char			commandLevel;             // 0 = player, 1 = counselor, 2 = GM
	unsigned int			spawnregion; 
	SERIAL					stablemaster_serial; 
	unsigned char			npc_type;		// currently only used for stabling, (type==1 -> stablemaster)
	// can be used for other npc types too of course
	unsigned int			time_unused;     
	unsigned int			timeused_last;
	// The bit for setting what effect gm movement 
    // commands shows 
    // 0 = off 
    // 1 = FlameStrike 
    // 2-6 = Sparkles
    int						gmMoveEff;

	int VisRange ;
	// Protected Data Members	
protected:
	unsigned char			priv;	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	void	day(unsigned long day) ; // set the day it was created
	unsigned long day() ;	// Retrieve the day it was created
	// a temporary (quick&dirty) encapsulation for Dexterity. (Duke, 21.8.2001)
	signed short			dx;		// Dexterity
	signed short			dx2;		// holds the 3 digits behind the decimal point. Reserved for calculation
	signed short			tmpDex;	// holds all temporary effects on Dex, eg. plate, spells, potions

	// Public Methods
public:
	virtual ~cChar() {}
	virtual void Serialize(ISerialization &archive);
	virtual string objectID();
	short effDex()				{return dx+tmpDex>0 ? dx+tmpDex : 0;}	// returns current effective Dexterity
	short realDex()				{return dx;}	// returns the true Dexterity
	short decDex()				{return dx2;}	// returns the 3 digits behind the decimal point
	void  setDex(signed short val)		{dx = val;}		// set the true Dex
	void  setDecDex(signed short val)	{dx2 = val;}	// set the 3 digits
	void  chgDex(signed short val)		{tmpDex += val;}// intended for temporary changes of Dex
	void  chgRealDex(short val) {dx += val;if(dx<1) dx=1;if(dx>100) dx=100;}	// intended for permanent changes of Dex
	bool  incDecDex(short val)	{dx2 += val;
	if (dx2>1000) {dx2-=1000;chgRealDex(1);return true;}
	else return false;}
	unsigned short id() const				{return static_cast<unsigned short>((id1<<8)+id2);}
	void setId(unsigned short id)			{id1 = id>>8;	id2 = id&0x00FF;}
	bool  isPlayer() const;
	bool  isNpc() const;
	bool  isHuman()	const;
	bool  isTrueGM() const;	
	bool  isGM() const;
	bool  isCounselor()	const;
	bool  isGMorCounselor()	const; 
	bool  isInvul() const;
	bool  canSnoop()const;
	bool  canBroadcast() const;
	bool  canSeeSerials() const;
	bool  isInnocent()	const;
	bool  isMurderer()	const;
	bool  isCriminal()	const;
	unsigned char getPriv();
	void setPriv(unsigned char p);
	void makeInvulnerable();
	void makeVulnerable();
	void setMurderer();
	void setInnocent();
	void setCriminal();
	void setAttackFirst()		{this->attackfirst = true;}
	void resetAttackFirst()		{this->attackfirst = false;}
	void fight(cChar* pOpponent);
	void setNextMoveTime(short tamediv=1);
	void disturbMed(UOXSOCKET s=-1);
	void unhide();
	bool isHidden() { return hidden > 0 ? true : false; }
	bool isHiddenBySpell() { return hidden & 2 ? true : false; }
	bool isHiddenBySkill() { return (hidden & 1); }
	int  CountItems(short ID, short col= -1);
	int  CountGold()			{return CountItems(0x0EED);}
	P_ITEM GetItemOnLayer(unsigned char layer);
	P_ITEM GetBankBox( short type = 1);
	int  CountBankGold();
	void addHalo(P_ITEM pi);
	void removeHalo(P_ITEM pi);
	void glowHalo(P_ITEM pi);
	P_ITEM getWeapon();
	P_ITEM getShield();
	P_ITEM getBackpack();
	void setOwnSerialOnly(long ownser);
	void SetOwnSerial(long ownser);
	long GetOwnSerial()	{return ownserial;}
	void SetSpawnSerial(long spawnser);
	void SetMultiSerial(long mulser);
	void setSerial(SERIAL ser);
	void MoveTo(short newx, short newy, signed char newz);
	void MoveToXY(short newx, short newy);
	bool Owns(P_CHAR pc)	{return (serial==pc->ownserial);}
	bool Owns(P_ITEM pi);
	bool Wears(P_ITEM pi);
	unsigned int getSkillSum();
	int getTeachingDelta(cChar* pPlayer, int skill, int sum);
	void removeItemBonus(cItem* pi);
	void Init(bool ser = true);
	bool isSameAs(cChar* pc) {if (!pc || pc->serial != serial) return false; else return true;}
	bool inGuardedArea();
	bool canPickUp(cItem* pi);
	unsigned int dist(cChar* pc);
	unsigned int dist(cItem* pi);
	int MyHome();
};

class cCharStuff
{
public:
	void DeleteChar(P_CHAR pc_k);
	P_CHAR MemCharFree();
	P_ITEM AddRandomLoot(P_ITEM pBackpack, char * lootlist);
	int getRandomNPC(char *npclist);
	P_CHAR AddNPCxyz(int s, int npcNum, int type, int x1, int y1, signed char z1);
	void Split(P_CHAR pc_k);
	void CheckAI(unsigned int currenttime, P_CHAR pc_i);
	P_CHAR AddNPC(int s, P_ITEM pi_i, int npcNum, int x1 = 0, int y1 = 0, signed char z1 = 0);
	// Sky's AI Stuff
	class cDragonAI
	{
	public:
		void DoAI(P_CHAR pc_i,int currenttime);
		void HarmMagic(P_CHAR pc_i, unsigned int currenttime,P_CHAR pc);
		void HealMagic(P_CHAR pc_i, unsigned int currenttime);
		void Claw(int i,int currenttime);
		void Bite(int i,int currenttime);
		void Breath(P_CHAR pc_i,int currenttime);
		void DoneAI(P_CHAR pc_i,int currenttime);
	};
	class cBankerAI
	{
	public:
		bool DoAI(int c, P_CHAR pBanker,string& comm);
		bool Withdraw(int c, P_CHAR pBanker, string& comm);
		bool BankCheck(int c, P_CHAR pBanker, string& comm);
		bool Balance(int c, P_CHAR pBanker);
		void OpenBank(UOXSOCKET c);
	};
};


// Inline Member functions
inline bool  cChar::isPlayer() const		{return (!this->npc);}
inline bool  cChar::isNpc()	const			{return (this->npc);}
inline bool  cChar::isHuman() const			{return (this->id() == 0x190 || this->id() == 0x191);} 
inline bool  cChar::isTrueGM() const		{return (priv&0x01);} 
inline bool  cChar::isGM() const			{return (priv&0x01 && (!gmrestrict || region==gmrestrict)) || account == 0;} 
inline bool  cChar::isCounselor() const		{return (priv&0x80 ? true : false);} 
inline bool  cChar::isGMorCounselor() const	{return (priv&0x81 ?true:false);} 
inline bool  cChar::isInvul() const			{return (priv&0x04 ?true:false);}
inline bool  cChar::canSnoop() const		{return (priv&0x40 ?true:false);}
inline bool  cChar::canBroadcast() const	{return (priv&0x02 ?true:false);}
inline bool  cChar::canSeeSerials() const 	{return (priv&0x08 ?true:false);}
inline bool  cChar::isInnocent() const		{return (flag&0x04 ?true:false);}
inline bool  cChar::isMurderer() const		{return (flag&0x01 ?true:false);}
inline bool  cChar::isCriminal() const		{return (flag&0x02 ?true:false);}
inline unsigned char cChar::getPriv()	{return priv;}
inline void cChar::setPriv(unsigned char p)	{this->priv=p;}
inline void cChar::makeInvulnerable()	{priv |= 4;}
inline void cChar::makeVulnerable()		{priv &= 0xFB;}
inline void cChar::setMurderer()		{flag = 0x01;}
inline void cChar::setInnocent()		{flag = 0x04;}
inline void cChar::setCriminal()		{flag=0x02;}


#endif
