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

#ifndef __CHARS_H
#define __CHARS_H

// just temporary
#undef  DBGFILE
#define DBGFILE "chars.h"
#include "debug.h"
#include "char_array.h" 

#include "coord.h"

//typedef struct char_st_
class cChar
{
public:
        enum enInputMode { enNone, enRenameRune, enPricing, enDescription, enNameDeed, enHouseSign, enPageGM, enPageCouns};
	//  Chaos/Order Guild Stuff for Ripper
	short     GuildType;    // (0) Standard guild, (1) Chaos Guild, (2) Order guild
	bool      GuildTraitor; // (true) This character converted, (false) Neve converted, or not an order/chaos guild member
	RACE race; // -Fraz- Race AddOn
	// Skyfire's NPC advancments.
	int worklocx;
	int worklocy;
	int worklocz;
	int homelocx;
	int homelocy;
	int homelocz;
	int foodlocx;
	int foodlocy;
	int foodlocz;
	bool may_levitate;
	//Skyfire - End NPC's home/work/food vars'
	unsigned char pathnum;
	path_st path[PATHNUM];
	unsigned char			ser1; // Character serial number
	unsigned char			ser2;
	unsigned char			ser3;
	unsigned char			ser4;
	int				serial;
	
	int				multis;//Multi serial
	
    bool			free;
	char			name[50];
	char			orgname[50];//original name - for Incognito
	char			title[50];
	bool			unicode; // This is set to 1 if the player uses unicode speech, 0 if not
	int				account; // changed to signed, lb
	Coord_cl		pos;
	//	int				x;
	//	int				y;
	//	signed char z;
	signed char	dispz; // Z that the char is SHOWN at. Server needs other coordinates for real movement calculations.
	// changed from unsigned to signed, LB
	
	//	Coord_cl		oldpos;
	//	unsigned int	oldx; // fix for jail bug
	//	unsigned int	oldy; // fix for jail bug
	
	//	signed char 	oldz;
	
	char			dir; //&0F=Direction
	unsigned char			id1; // Character body type
	unsigned char			id2; // Character body type
	unsigned char			xid1; // Backup of body type for ghosts
	unsigned char			xid2; // Backup of body type for ghosts
	bool			incognito;// AntiChrist - true if under incognito effect
	bool			polymorph;// AntiChrist - true if under polymorph effect
	//char			orgid1; // Backup of body type for Polymorph and incognito spell
	//char			orgid2; // Backup of body type for Polymorph and incognito spell
	int				haircolor1;// backup of hair/beard for incognito spell
	int				haircolor2;
	int				hairstyle1;
	int				hairstyle2;
	int				beardcolor1;
	int				beardcolor2;
	int				beardstyle1;
	int				beardstyle2;
	UI16			skin; // Skin color
	unsigned char	orgskin1;	// skin color backup for incognito spell
	unsigned char	orgskin2;
	int             keynumb;  // for renaming keys 
	UI16			xskin; // Backup of skin color
	unsigned int creationday ;	// Day since EPOCH this character was created on
	
protected:
	unsigned char	priv;	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	void	day(unsigned long day) ; // set the day it was created
	unsigned long day() ;	// Retrieve the day it was created
public:
	unsigned char	gmrestrict;	// for restricting GMs to certain regions
	char			priv2;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	int			            priv3[7];  // needed for Lord binarys meta-gm stuff
	unsigned char			fonttype; // Speech font to use
	UI16					saycolor; // Color for say messages
	unsigned char			emotecolor1; // Color for emote messages
	unsigned char			emotecolor2; // Color for emote messages
	int				st; // Strength
	int				st2; // Reserved for calculation
	
	// a temporary (quick&dirty) encapsulation for Dexterity. (Duke, 21.8.2001)
protected:
	short dx;		// Dexterity
	short dx2;		// holds the 3 digits behind the decimal point. Reserved for calculation
	short tmpDex;	// holds all temporary effects on Dex, eg. plate, spells, potions
public:
	short effDex()				{return dx+tmpDex>0 ? dx+tmpDex : 0;}	// returns current effective Dexterity
	short realDex()				{return dx;}	// returns the true Dexterity
	short decDex()				{return dx2;}	// returns the 3 digits behind the decimal point
	void  setDex(short val)		{dx = val;}		// set the true Dex
	void  setDecDex(short val)	{dx2 = val;}	// set the 3 digits
	void  chgDex(short val)		{tmpDex += val;}// intended for temporary changes of Dex
	void  chgRealDex(short val) {dx += val;if(dx<1) dx=1;if(dx>100) dx=100;}	// intended for permanent changes of Dex
	bool  incDecDex(short val)	{dx2 += val;
	if (dx2>1000) {dx2-=1000;chgRealDex(1);return true;}
	else return false;}
	
	int in; // Intelligence
	int in2; // Reserved for calculation
	int hp; // Hitpoints
	int stm; // Stamina
	int mn; // Mana
	int mn2; // Reserved for calculation
	int hidamage; //NPC Damage
	int lodamage; //NPC Damage
	unsigned short int baseskill[ALLSKILLS+1]; // Base skills without stat modifiers
	unsigned short int skill[ALLSKILLS+1]; // List of skills (with stat modifiers)
	char npc; // 1=Character is an NPC
	char shop; //1=npc shopkeeper
	unsigned char cell; // Reserved for jailing players
	unsigned int jailtimer; // Blackwind - Timer used for crystall ball and jail time.
	int			 jailsecs;	//             Tweak this value by using command tweak before jailing person 
	//			   or he will be jailed 1 day ( in realtime )
	
	int ownserial; // If Char is an NPC, this sets its owner
	int robe; // Serial number of generated death robe (If char is a ghost)
	int karma;
	signed int fame;
	unsigned int kills; //PvP Kills
	unsigned int deaths;
	bool dead; // Is character dead
	SERIAL packitem; // Only used during character creation
	unsigned char fixedlight; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	char speech; // For NPCs: Number of the assigned speech block
	int weight; //Total weight
	unsigned int att; // Intrinsic attack (For monsters that cant carry weapons)
	unsigned int def; // Intrinsic defense
	char war; // War Mode
	SERIAL targ; // Current combat target
	unsigned int timeout; // Combat timeout (For hitting)
	unsigned int timeout2; // memory of last shot timeout
	unsigned int regen, regen2, regen3;//Regeneration times for mana, stamin, and str
	enInputMode inputmode;	// Used for entering text; 0= none, 4=rename rune
	int inputitem;		// serial of item the text is referring to
	SERIAL attacker; // Character's serial who attacked this character
	unsigned int npcmovetime; // Next time npc will walk
	char npcWander; // NPC Wander Mode
	char oldnpcWander; // Used for fleeing npcs
	SERIAL ftarg; // NPC Follow Target
	int fx1; //NPC Wander Point 1 x
	int fx2; //NPC Wander Point 2 x
	int fy1; //NPC Wander Point 1 y
	int fy2; //NPC Wander Point 2 y
	signed char fz1; //NPC Wander Point 1 z
	int spawnserial; // Spawned by
	char hidden; // 0 = not hidden, 1 = hidden, 2 = invisible spell
	unsigned int invistimeout;
	char attackfirst; // 0 = defending, 1 = attacked first
	bool onhorse; // On a horse?
	int hunger;  // Level of hungerness, 6 = full, 0 = "empty"
	unsigned int hungertime; // Timer used for hunger, one point is dropped every 20 min
	SERIAL smeltitem;
	SERIAL tailitem;
	int npcaitype; // NPC ai
	int callnum; //GM Paging
	int playercallnum; //GM Paging
	//int pagegm; //GM Paging
	//char region;
	unsigned char region;
	unsigned long skilldelay;
	unsigned long objectdelay;
	int combathitmessage;
	int making; // skill number of skill using to make item, 0 if not making anything.
	SERIAL lastTarget;
	char blocked;
	char dir2;
	unsigned long spiritspeaktimer; // Timer used for duration of spirit speak
	int spattack;
	int spadelay;
	unsigned int spatimer;
	int taming; //Skill level required for taming
	unsigned int summontimer; //Timer for summoned creatures.
	unsigned long trackingtimer; // Timer used for the duration of tracking
	SERIAL trackingtarget; // Tracking target ID
	SERIAL trackingtargets[MAXTRACKINGTARGETS];
	unsigned int fishingtimer; // Timer used to delay the catching of fish
	
	int advobj; //Has used advance gate?
	
	int poison; // used for poison skill 
	unsigned int poisoned; // type of poison
	unsigned int poisontime; // poison damage timer
	unsigned int poisontxt; // poision text timer
	unsigned int poisonwearofftime; // LB, makes poision wear off ...
	
	short fleeat;
	short reattackat;
	int trigger; //Trigger number that character activates
	char trigword[31]; //Word that character triggers on.
	unsigned int disabled; //Character is disabled, cant trigger.
	char disabledmsg[256]; //Character is disabled, so dysplay this message. -- added by Magius(CHE) §
	char envokeid1; //ID1 of item user envoked
	char envokeid2; //ID2 of item user envoked
	SERIAL envokeitem;
	int split;
	int splitchnc;
	int targtrig; //Stores the number of the trigger the character for targeting
	char ra;  // Reactive Armor spell
	int trainer; // Serial of the NPC training the char, -1 if none.
	char trainingplayerin; // Index in skillname of the skill the NPC is training the player in
	bool cantrain;
	// Begin of Guild Related Character information (DasRaetsel)
	int guildtoggle;		// Toggle for Guildtitle								(DasRaetsel)
	char guildtitle[21];	// Title Guildmaster granted player						(DasRaetsel)
	int	guildfealty;		// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	int	guildnumber;		// Number of guild player is in (0=no guild)			(DasRaetsel)
	char flag; //1=red 2=grey 4=Blue 8=green 10=Orange
	//char tempflag; //Zippy -- Not Used
	unsigned int tempflagtime;
	// End of Guild Related Character information
	unsigned long int murderrate; //#of ticks until one murder decays //REPSYS 
	long int crimflag; //Time when No longer criminal -1=Not Criminal
	int casting; // 0/1 is the cast casting a spell?
	unsigned int spelltime; //Time when they are done casting....
	int spell; //current spell they are casting....
	int spellaction; //Action of the current spell....
	int nextact; //time to next spell action....
	int poisonserial; //AntiChrist -- poisoning skill
	
	int squelched; // zippy  - squelching
	int mutetime; //Time till they are UN-Squelched.
	int med; // 0=not meditating, 1=meditating //Morrolan - Meditation 
	//int statuse[3]; //Morrolan - stat/skill cap STR/INT/DEX in that order
	//int skilluse[TRUESKILLS][1]; //Morrolan - stat/skill cap
	unsigned char lockSkill[ALLSKILLS+1]; // LB, client 1.26.2b skill managment
	int stealth; //AntiChrist - stealth ( steps already done, -1=not using )
	unsigned int running; //AntiChrist - Stamina Loose while running
	signed long int logout; //unsigned int logout;//Time till logout for this char -1 means in the world or already logged out //Instalog
	unsigned long int clientidletime; // LB
    //unsigned long int swing;
	int swingtarg; //Tagret they are going to hit after they swing
	
	unsigned int holdg; // Gold a player vendor is holding for Owner
	//int weather;//Weather!
	char fly_steps; // number of step the creatures flies if it can fly
	unsigned long int trackingdisplaytimer;
	int menupriv; // needed fro LB's menu priv system
	//taken from 6904t2(5/10/99) - AntiChrist
	bool tamed;
	//taken from 6904t2(5/10/99) - AntiChrist
	bool guarded;							// (Abaddon) if guarded
    unsigned int smoketimer; // LB
	unsigned int smokedisplaytimer;
	
	unsigned int antispamtimer;//LB - anti spam
	
	unsigned int antiguardstimer;//AntiChrist - anti "GUARDS" spawn
	
	int carve; //AntiChrist - for new carve system
	
	int	hairserial;//there are needed for incognito stuff
	int	beardserial;
	
	unsigned long int begging_timer;
	int postType;
	int questType;
	int questDestRegion;
	int questOrigRegion;
	int questBountyReward;      // The current reward amount for the return of this chars head
	int questBountyPostSerial;  // The global posting serial number of the bounty message
	int murdererSer;            // Serial number of last person that murdered this char
	int				prevX; // fix for looping gate travel bug (bounce back problem)
	int				prevY;
	signed char 	prevZ;
	
	unsigned char commandLevel;             // 0 = player, 1 = counselor, 2 = GM
	
	int spawnregion; 
	
	int stablemaster_serial; 
	char npc_type;		// currently only used for stabling, (type==1 -> stablemaster)
	// can be used for other npc types too of course
	
	unsigned long int time_unused;     
	unsigned long int timeused_last;
	
	short id()				{return (short)((id1<<8)+id2);}
	void setId(short id)	{id1=id>>8;	id2=id&0x00FF;}
	bool  isPlayer()        {return (this->npc==0);}
	bool  isNpc()           {return (this->npc==1);}
	bool  isHuman()			{return (this->id()==0x190 || this->id()==0x191);} 
	bool  isTrueGM()		{return (priv&0x01);} 
	bool  isGM()			{return (priv&0x01 && (!gmrestrict || region==gmrestrict)) || account == 0;} 
	bool  isCounselor()		{return (priv&0x80 ?true:false);} 
	bool  isGMorCounselor()	{return (priv&0x81 ?true:false);} 
	bool  isInvul()			{return (priv&0x04 ?true:false);}
	bool  canSnoop()		{return (priv&0x40 ?true:false);}
	bool  canBroadcast()	{return (priv&0x02 ?true:false);}
	bool  canSeeSerials()	{return (priv&0x08 ?true:false);}
	bool  isInnocent()		{return (flag&0x04 ?true:false);}
	bool  isMurderer()		{return (flag&0x01 ?true:false);}
	bool  isCriminal()		{return (flag&0x02 ?true:false);}
	unsigned char getPriv()		{return priv;}
	void setPriv(unsigned char p)	{this->priv=p;}
	void makeInvulnerable()		{priv|=4;}
	void makeVulnerable()		{priv &= (0xFF-4);}
	void setMurderer()		{flag=0x01;}
	void setInnocent()		{flag=0x04;}
	void setCriminal()		{flag=0x02;}
	
	void setAttackFirst()		{this->attackfirst=1;}
	void resetAttackFirst()		{this->attackfirst=0;}
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
	// The bit for setting what effect gm movement 
    // commands shows 
    // 0 = off 
    // 1 = FlameStrike 
    // 2-6 = Sparkles
    int gmMoveEff;
	void MoveTo(short newx, short newy, signed char newz);
	void MoveToXY(short newx, short newy);
	bool Owns(cChar* pc)	{return (serial==pc->ownserial);}
	bool Owns(P_ITEM pi)	{return (serial==pi->ownserial);}
	bool Wears(P_ITEM pi)	{return (serial==pi->contserial);}
	int getSkillSum();
	int getTeachingDelta(cChar* pPlayer, int skill, int sum);
	void removeItemBonus(cItem* pi);
	void Init(bool ser = true);
	bool isSameAs(cChar* pc) {if (!pc || pc->serial != serial) return false; else return true;}
	bool inGuardedArea()	{return ::region[this->region].priv&1;}
	bool canPickUp(cItem* pi);
	unsigned dist(cChar* pc)	{return pos.distance(pc->pos);}
	unsigned dist(cItem* pi)	{return pos.distance(pi->pos);}
	int MyHome();
};

#include "CharWrap.h"
#include "char_array.h"

class cCharStuff
{
private:
	bool moreCharMemoryRequested;
	bool ResizeMemory();
	void CollectReusableSlots();
	int  GetReusableSlot();
	
public:
	void DeleteChar(int k);
	void CheckMemoryRequest();
	bool AllocateMemory(int NumberOfChars);
	int MemCharFree();
	P_ITEM AddRandomLoot(P_ITEM pBackpack, char * lootlist);
	int AddRandomNPC(int s, char *npclist, int spawnpoint);
	int AddNPCxyz(int s, int npcNum, int type, int x1, int y1, signed char z1);
	void Split(int k);
	void CheckAI(unsigned int currenttime, int i);
	int AddNPC(int s, P_ITEM pi_i, int npcNum, int x1 = 0, int y1 = 0, signed char z1 = 0);
	// Sky's AI Stuff
	class cDragonAI
	{
	public:
		void DoAI(int i,int currenttime);
		void HarmMagic(int i,int currenttime,P_CHAR pc);
		void HealMagic(int i,int currenttime);
		void Claw(int i,int currenttime);
		void Bite(int i,int currenttime);
		void Breath(int i,int currenttime);
		void DoneAI(int i,int currenttime);
	};
	
	class cBankerAI
	{
	public:
		bool DoAI(int c,int i,char *comm);
		bool Withdraw(int c,int i, char *comm);
		bool BankCheck(int c,int i, char *comm);
		bool Balance(int c, int i);
		void OpenBank(int c);
	};
};

class AllCharsIterator
{
protected:
	unsigned int pos;
public:
	AllCharsIterator()							{ pos = 0; }
	~AllCharsIterator()							{ }
	P_CHAR Begin()								{ pos = 0; return pos < charcount ? &chars[pos] : NULL;}
	P_CHAR Next()								{ return pos++ < charcount ? &chars[pos] : NULL; }
	bool atEnd()								{ return GetData() == NULL; }
	P_CHAR GetData(void)						{ return pos < charcount ? &chars[pos] : NULL; }
	AllCharsIterator& operator++(int/* inc*/)	{ pos++; return *this; }
	
};


#endif
