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
	//##ModelId=3C5D931D03BC
	unsigned char			gmrestrict;	// for restricting GMs to certain regions
	//##ModelId=3C5D931D03D0
	unsigned char			priv2;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	//##ModelId=3C5D931E0006
	int			            priv3[7];  // needed for Lord binarys meta-gm stuff
	//##ModelId=3C5D931E0024
	unsigned char			fonttype; // Speech font to use
	//##ModelId=3C5D931E0134
	UI16					saycolor; // Color for say messages
	//##ModelId=3C5D931E0151
	unsigned short			emotecolor; // Color for emote messages
	//##ModelId=3C5D931E0179
	signed short			st; // Strength
	//##ModelId=3C5D931E0197
	signed short			st2; // Reserved for calculation
	//##ModelId=3C5D931E01B5
	signed short			in; // Intelligence
	//##ModelId=3C5D931E01D3
	signed short			in2; // Reserved for calculation
	//##ModelId=3C5D931E01F1
	signed short			hp; // Hitpoints
	//##ModelId=3C5D931E0237
	signed short			stm; // Stamina
	//##ModelId=3C5D931E0255
	signed short			mn; // Mana
	//##ModelId=3C5D931E0273
	signed short			mn2; // Reserved for calculation
	//##ModelId=3C5D931E0291
	int						hidamage; //NPC Damage
	//##ModelId=3C5D931E02A5
	int						lodamage; //NPC Damage
	//##ModelId=3C5D931E02C3
	unsigned short int		baseskill[ALLSKILLS+1]; // Base skills without stat modifiers
	//##ModelId=3C5D931E02E1
	unsigned short int		skill[ALLSKILLS+1]; // List of skills (with stat modifiers)
	//##ModelId=3C5D931E0300
	bool					npc;	// true = Character is an NPC
	//##ModelId=3C5D931E031E
	bool					shop;	// true = npc shopkeeper
	//##ModelId=3C5D931E033C
	unsigned char			cell; // Reserved for jailing players
	//##ModelId=3C5D931E035A
	unsigned int			jailtimer; // Blackwind - Timer used for crystall ball and jail time.
	//##ModelId=3C5D931E0378
	int						jailsecs;	//             Tweak this value by using command tweak before jailing person 
	//			   or he will be jailed 1 day ( in realtime )
	
	//##ModelId=3C5D931E0396
	int						ownserial; // If Char is an NPC, this sets its owner
	//##ModelId=3C5D931E03AA
	int						robe; // Serial number of generated death robe (If char is a ghost)
	//##ModelId=3C5D931E03C8
	int						karma;
	//##ModelId=3C5D931E03E6
	signed int				fame;
	//##ModelId=3C5D931F001C
	unsigned int			kills; //PvP Kills
	//##ModelId=3C5D931F003A
	unsigned int			deaths;
	//##ModelId=3C5D931F0058
	bool					dead; // Is character dead
	//##ModelId=3C5D931F0077
	SERIAL					packitem; // Serial of backpack
	//##ModelId=3C5D931F0094
	unsigned char			fixedlight; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	//##ModelId=3C5D931F00B2
	unsigned char			speech; // For NPCs: Number of the assigned speech block
	//##ModelId=3C5D931F00D0
	int						weight; //Total weight
	//##ModelId=3C5D931F00EE
	unsigned int			att; // Intrinsic attack (For monsters that cant carry weapons)
	//##ModelId=3C5D931F010C
	unsigned int			def; // Intrinsic defense
	//##ModelId=3C5D931F012A
	bool					war; // War Mode
	//##ModelId=3C5D931F0153
	SERIAL					targ; // Current combat target
	//##ModelId=3C5D931F017A
	unsigned int			timeout; // Combat timeout (For hitting)
	//##ModelId=3C5D931F0198
	unsigned int			timeout2; // memory of last shot timeout
	unsigned int			regen, regen2, regen3;//Regeneration times for mana, stamin, and str
	//##ModelId=3C5D931F01C1
	enInputMode				inputmode;	// Used for entering text; 0= none, 4=rename rune
	//##ModelId=3C5D931F02DA
	SERIAL					inputitem;		// serial of item the text is referring to
	//##ModelId=3C5D931F030C
	SERIAL					attacker; // Character's serial who attacked this character
	//##ModelId=3C5D931F0329
	unsigned int			npcmovetime; // Next time npc will walk
	//##ModelId=3C5D931F0347
	unsigned char			npcWander; // NPC Wander Mode
	//##ModelId=3C5D931F036F
	unsigned char			oldnpcWander; // Used for fleeing npcs
	//##ModelId=3C5D931F03A1
	SERIAL					ftarg; // NPC Follow Target
	//##ModelId=3C5D931F03D3
	int						fx1; //NPC Wander Point 1 x
	//##ModelId=3C5D93200013
	int						fx2; //NPC Wander Point 2 x
	//##ModelId=3C5D9320003B
	int						fy1; //NPC Wander Point 1 y
	//##ModelId=3C5D93200063
	int						fy2; //NPC Wander Point 2 y
	//##ModelId=3C5D932000BE
	signed char				fz1; //NPC Wander Point 1 z
	//##ModelId=3C5D9320012D
	SERIAL					spawnserial; // Spawned by
	//##ModelId=3C5D93200172
	unsigned char			hidden; // 0 = not hidden, 1 = hidden, 2 = invisible spell
	//##ModelId=3C5D93200190
	unsigned int			invistimeout;
	//##ModelId=3C5D932001B8
	bool					attackfirst; // 0 = defending, 1 = attacked first
	//##ModelId=3C5D932001E0
	bool					onhorse; // On a horse?
	//##ModelId=3C5D93200208
	int						hunger;  // Level of hungerness, 6 = full, 0 = "empty"
	//##ModelId=3C5D93200230
	unsigned int			hungertime; // Timer used for hunger, one point is dropped every 20 min
	//##ModelId=3C5D93200263
	SERIAL					smeltitem;
	//##ModelId=3C5D932002BC
	SERIAL					tailitem;
	//##ModelId=3C5D932002E4
	int						npcaitype; // NPC ai
	//##ModelId=3C5D9320030C
	int						callnum; //GM Paging
	//##ModelId=3C5D9320033E
	int						playercallnum; //GM Paging
	//int pagegm; //GM Paging
	//char region;
	//##ModelId=3C5D93200371
	unsigned char			region;
	//##ModelId=3C5D932003A3
	unsigned int			skilldelay;
	//##ModelId=3C5D932003D5
	unsigned int			objectdelay;
	//##ModelId=3C5D9321001F
	int						combathitmessage;
	//##ModelId=3C5D9321006F
	int						making; // skill number of skill using to make item, 0 if not making anything.
	//##ModelId=3C5D932100A2
	SERIAL					lastTarget;
	//##ModelId=3C5D932100C9
	char					blocked;
	//##ModelId=3C5D932100FB
	char					dir2;
	//##ModelId=3C5D9321012D
	unsigned int			spiritspeaktimer; // Timer used for duration of spirit speak
	//##ModelId=3C5D93210155
	int						spattack;
	//##ModelId=3C5D93210191
	int						spadelay;
	//##ModelId=3C5D932101B9
	unsigned int			spatimer;
	//##ModelId=3C5D932101EB
	int						taming; //Skill level required for taming
	//##ModelId=3C5D9321021E
	unsigned int			summontimer; //Timer for summoned creatures.
	//##ModelId=3C5D93210246
	unsigned int			trackingtimer; // Timer used for the duration of tracking
	//##ModelId=3C5D93210283
	SERIAL					trackingtarget; // Tracking target ID
	//##ModelId=3C5D932102AB
	SERIAL					trackingtargets[MAXTRACKINGTARGETS];
	//##ModelId=3C5D932102DC
	unsigned int			fishingtimer; // Timer used to delay the catching of fish
	
	//##ModelId=3C5D93210304
	int						advobj; //Has used advance gate?
	
	//##ModelId=3C5D93210336
	int						poison; // used for poison skill 
	//##ModelId=3C5D93210368
	unsigned int			poisoned; // type of poison
	//##ModelId=3C5D9321039A
	unsigned int			poisontime; // poison damage timer
	//##ModelId=3C5D932103CC
	unsigned int			poisontxt; // poision text timer
	//##ModelId=3C5D9322000C
	unsigned int			poisonwearofftime; // LB, makes poision wear off ...
	
	//##ModelId=3C5D932200E9
	short					fleeat;
	//##ModelId=3C5D93220201
	short					reattackat;
	//##ModelId=3C5D93220233
	int						trigger; //Trigger number that character activates
	//##ModelId=3C5D93220270
	string					trigword; //Word that character triggers on.
	//##ModelId=3C5D9322028D
	unsigned int			disabled; //Character is disabled, cant trigger.
	//##ModelId=3C5D932202CA
	string					disabledmsg; //Character is disabled, so dysplay this message. -- added by Magius(CHE) §
	//##ModelId=3C5D932202E7
	unsigned short			envokeid;  //ID1 of item user envoked
	//##ModelId=3C5D9322031A
	SERIAL					envokeitem;
	//##ModelId=3C5D9322034B
	unsigned char			split;
	//##ModelId=3C5D9322037D
	unsigned char			splitchnc;
	//##ModelId=3C5D932203B0
	int						targtrig; //Stores the number of the trigger the character for targeting
	//##ModelId=3C5D932203E2
	char					ra;  // Reactive Armor spell
	//##ModelId=3C5D93230036
	int						trainer; // Serial of the NPC training the char, -1 if none.
	//##ModelId=3C5D93230068
	char					trainingplayerin; // Index in skillname of the skill the NPC is training the player in
	//##ModelId=3C5D9323009A
	bool					cantrain;
	// Begin of Guild Related Character information (DasRaetsel)
	//##ModelId=3C5D932300CC
	bool					guildtoggle;		// Toggle for Guildtitle								(DasRaetsel)
	//##ModelId=3C5D93230109
	string					guildtitle;			// Title Guildmaster granted player						(DasRaetsel)
	//##ModelId=3C5D93230131
	SERIAL					guildfealty;		// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	//##ModelId=3C5D93230163
	SERIAL					guildstone;			// Number of guild player is in (0=no guild)			(DasRaetsel)
	//##ModelId=3C5D9323019E
	char					flag; //1=red 2=grey 4=Blue 8=green 10=Orange
	//##ModelId=3C5D932301DA
	unsigned int			tempflagtime;
	// End of Guild Related Character information
	//##ModelId=3C5D93230248
	unsigned int			murderrate; //#of ticks until one murder decays //REPSYS 
	//##ModelId=3C5D9323027B
	long int				crimflag; //Time when No longer criminal -1=Not Criminal
	//##ModelId=3C5D932302B7
	int						casting; // 0/1 is the cast casting a spell?
	//##ModelId=3C5D932302E9
	unsigned int			spelltime; //Time when they are done casting....
	//##ModelId=3C5D93230325
	int						spell; //current spell they are casting....
	//##ModelId=3C5D93230361
	int						spellaction; //Action of the current spell....
	//##ModelId=3C5D9323039D
	int						nextact; //time to next spell action....
	//##ModelId=3C5D932303E3
	SERIAL					poisonserial; //AntiChrist -- poisoning skill
	
	//##ModelId=3C5D93240023
	int						squelched; // zippy  - squelching
	//##ModelId=3C5D9324005F
	int						mutetime; //Time till they are UN-Squelched.
	//##ModelId=3C5D9324009B
	bool					med; // 0=not meditating, 1=meditating //Morrolan - Meditation 
	//int statuse[3]; //Morrolan - stat/skill cap STR/INT/DEX in that order
	//int skilluse[TRUESKILLS][1]; //Morrolan - stat/skill cap
	//##ModelId=3C5D932400CD
	unsigned char			lockSkill[ALLSKILLS+1]; // LB, client 1.26.2b skill managment
	//##ModelId=3C5D93240109
	int						stealth; //AntiChrist - stealth ( steps already done, -1=not using )
	//##ModelId=3C5D93240146
	unsigned int			running; //AntiChrist - Stamina Loose while running
	//##ModelId=3C5D9324018C
	unsigned int			logout; //unsigned int logout;//Time till logout for this char -1 means in the world or already logged out //Instalog
	//##ModelId=3C5D932401BE
	unsigned int			clientidletime; // LB
	//##ModelId=3C5D93240205
	SERIAL					swingtarg; //Tagret they are going to hit after they swing
	
	//##ModelId=3C5D9324022C
	unsigned int			holdg; // Gold a player vendor is holding for Owner
	//##ModelId=3C5D93240268
	unsigned char			fly_steps; // number of step the creatures flies if it can fly
	//##ModelId=3C5D932402A4
	unsigned int			trackingdisplaytimer;
	//##ModelId=3C5D932402D6
	int						menupriv; // needed fro LB's menu priv system
	//taken from 6904t2(5/10/99) - AntiChrist
	//##ModelId=3C5D93240312
	bool					tamed;
	//taken from 6904t2(5/10/99) - AntiChrist
	//##ModelId=3C5D9324034E
	bool					guarded;							// (Abaddon) if guarded
	//##ModelId=3C5D93240394
    unsigned int			smoketimer; // LB
	//##ModelId=3C5D932403D0
	unsigned int			smokedisplaytimer;
	
	//##ModelId=3C5D93250025
	unsigned int			antispamtimer;//LB - anti spam
	
	//##ModelId=3C5D93250061
	unsigned int			antiguardstimer;//AntiChrist - anti "GUARDS" spawn
	
	//##ModelId=3C5D9325009D
	int						carve; //AntiChrist - for new carve system
	
	//##ModelId=3C5D932500CF
	int						hairserial;//there are needed for incognito stuff
	//##ModelId=3C5D9325010B
	int						beardserial;
	
	//##ModelId=3C5D93250147
	unsigned int			begging_timer;
	//##ModelId=3C5D93250183
	int						postType;
	//##ModelId=3C5D932501BF
	int						questType;
	//##ModelId=3C5D932501F1
	int						questDestRegion;
	//##ModelId=3C5D9325022D
	int						questOrigRegion;
	//##ModelId=3C5D93250269
	int						questBountyReward;      // The current reward amount for the return of this chars head
	//##ModelId=3C5D932502A5
	int						questBountyPostSerial;  // The global posting serial number of the bounty message
	//##ModelId=3C5D932502E3
	SERIAL					murdererSer;            // Serial number of last person that murdered this char
	//##ModelId=3C5D93250314
	int						prevX; // fix for looping gate travel bug (bounce back problem)
	//##ModelId=3C5D93250350
	int						prevY;
	//##ModelId=3C5D9325038C
	signed char 			prevZ;
	
	//##ModelId=3C5D932503C8
	unsigned char			commandLevel;             // 0 = player, 1 = counselor, 2 = GM
	
	//##ModelId=3C5D9326001C
	unsigned int			spawnregion; 
	
	//##ModelId=3C5D93260063
	SERIAL					stablemaster_serial; 
	//##ModelId=3C5D93260094
	unsigned char			npc_type;		// currently only used for stabling, (type==1 -> stablemaster)
	// can be used for other npc types too of course
	
	//##ModelId=3C5D932600F8
	unsigned int			time_unused;     
	//##ModelId=3C5D93260134
	unsigned int			timeused_last;
	// The bit for setting what effect gm movement 
    // commands shows 
    // 0 = off 
    // 1 = FlameStrike 
    // 2-6 = Sparkles
	//##ModelId=3C5D93260170
    int						gmMoveEff;

	// Protected Data Members	
protected:
	//##ModelId=3C5D932601B7
	unsigned char			priv;	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	//##ModelId=3C5D932601FD
	void	day(unsigned long day) ; // set the day it was created
	//##ModelId=3C5D93260243
	unsigned long day() ;	// Retrieve the day it was created
	// a temporary (quick&dirty) encapsulation for Dexterity. (Duke, 21.8.2001)
	//##ModelId=3C5D93260257
	signed short			dx;		// Dexterity
	//##ModelId=3C5D93260293
	signed short			dx2;		// holds the 3 digits behind the decimal point. Reserved for calculation
	//##ModelId=3C5D932602D9
	signed short			tmpDex;	// holds all temporary effects on Dex, eg. plate, spells, potions

	// Public Methods
public:
	//##ModelId=3C5D9326031F
	virtual ~cChar() {}
	//##ModelId=3C5D93260329
	virtual void Serialize(ISerialization &archive);
	//##ModelId=3C5D9326035B
	virtual string objectID();
	//##ModelId=3C5D9326036F
	short effDex()				{return dx+tmpDex>0 ? dx+tmpDex : 0;}	// returns current effective Dexterity
	//##ModelId=3C5D93260379
	short realDex()				{return dx;}	// returns the true Dexterity
	//##ModelId=3C5D93260383
	short decDex()				{return dx2;}	// returns the 3 digits behind the decimal point
	//##ModelId=3C5D9326038D
	void  setDex(signed short val)		{dx = val;}		// set the true Dex
	//##ModelId=3C5D932603C9
	void  setDecDex(signed short val)	{dx2 = val;}	// set the 3 digits
	//##ModelId=3C5D9327001D
	void  chgDex(signed short val)		{tmpDex += val;}// intended for temporary changes of Dex
	//##ModelId=3C5D9327004F
	void  chgRealDex(short val) {dx += val;if(dx<1) dx=1;if(dx>100) dx=100;}	// intended for permanent changes of Dex
	//##ModelId=3C5D93270082
	bool  incDecDex(short val)	{dx2 += val;
	if (dx2>1000) {dx2-=1000;chgRealDex(1);return true;}
	else return false;}
	
	
	//##ModelId=3C5D932700E6
	unsigned short id() const				{return static_cast<unsigned short>((id1<<8)+id2);}
	//##ModelId=3C5D932700F1
	void setId(unsigned short id)			{id1 = id>>8;	id2 = id&0x00FF;}
	//##ModelId=3C5D93270122
	bool  isPlayer() const;
	//##ModelId=3C5D93270137
	bool  isNpc() const;
	//##ModelId=3C5D9327014A
	bool  isHuman()	const;
	//##ModelId=3C5D9327015E
	bool  isTrueGM() const;	
	//##ModelId=3C5D93270168
	bool  isGM() const;
	//##ModelId=3C5D932701AE
	bool  isCounselor()	const;
	//##ModelId=3C5D932701C2
	bool  isGMorCounselor()	const; 
	//##ModelId=3C5D932701CC
	bool  isInvul() const;
	//##ModelId=3C5D932701E0
	bool  canSnoop()const;
	//##ModelId=3C5D932701EB
	bool  canBroadcast() const;
	//##ModelId=3C5D932701FE
	bool  canSeeSerials() const;
	//##ModelId=3C5D93270212
	bool  isInnocent()	const;
	//##ModelId=3C5D9327021C
	bool  isMurderer()	const;
	//##ModelId=3C5D93270230
	bool  isCriminal()	const;
	//##ModelId=3C5D93270244
	unsigned char getPriv();
	//##ModelId=3C5D9327024E
	void setPriv(unsigned char p);
	//##ModelId=3C5D9327028A
	void makeInvulnerable();
	//##ModelId=3C5D93270294
	void makeVulnerable();
	//##ModelId=3C5D9327029E
	void setMurderer();
	//##ModelId=3C5D932702B2
	void setInnocent();
	//##ModelId=3C5D932702BC
	void setCriminal();
	
	//##ModelId=3C5D932702D0
	void setAttackFirst()		{this->attackfirst = true;}
	//##ModelId=3C5D932702DA
	void resetAttackFirst()		{this->attackfirst = false;}
	//##ModelId=3C5D932702E4
	void fight(cChar* pOpponent);
	//##ModelId=3C5D93270316
	void setNextMoveTime(short tamediv=1);
	//##ModelId=3C5D93270349
	void disturbMed(UOXSOCKET s=-1);
	//##ModelId=3C5D93270385
	void unhide();
	//##ModelId=3C5D9327038F
	bool isHidden() { return hidden > 0 ? true : false; }
	//##ModelId=3C5D93270399
	bool isHiddenBySpell() { return hidden & 2 ? true : false; }
	//##ModelId=3C5D932703A3
	bool isHiddenBySkill() { return (hidden & 1); }
	//##ModelId=3C5D932703AD
	int  CountItems(short ID, short col= -1);
	//##ModelId=3C5D93280029
	int  CountGold()			{return CountItems(0x0EED);}
	//##ModelId=3C5D93280033
	P_ITEM GetItemOnLayer(unsigned char layer);
	//##ModelId=3C5D93280065
	P_ITEM GetBankBox( short type = 1);
	//##ModelId=3C5D93280097
	int  CountBankGold();
	//##ModelId=3C5D932800AB
	void addHalo(P_ITEM pi);
	//##ModelId=3C5D932800DD
	void removeHalo(P_ITEM pi);
	//##ModelId=3C5D9328010F
	void glowHalo(P_ITEM pi);
	//##ModelId=3C5D9328014B
	P_ITEM getWeapon();
	//##ModelId=3C5D93280155
	P_ITEM getShield();
	//##ModelId=3C5D9328015F
	P_ITEM getBackpack();
	//##ModelId=3C5D93280169
	void setOwnSerialOnly(long ownser);
	//##ModelId=3C5D932801AF
	void SetOwnSerial(long ownser);
	//##ModelId=3C5D932801EC
	long GetOwnSerial()	{return ownserial;}
	//##ModelId=3C5D932801F6
	void SetSpawnSerial(long spawnser);
	//##ModelId=3C5D93280228
	void SetMultiSerial(long mulser);
	//##ModelId=3C5D9328025A
	void setSerial(SERIAL ser);
	//##ModelId=3C5D9328028C
	void MoveTo(short newx, short newy, signed char newz);
	//##ModelId=3C5D9328030E
	void MoveToXY(short newx, short newy);
	//##ModelId=3C5D93280372
	bool Owns(P_CHAR pc)	{return (serial==pc->ownserial);}
	//##ModelId=3C5D932803A4
	bool Owns(P_ITEM pi);
	//##ModelId=3C5D932803D6
	bool Wears(P_ITEM pi);
	//##ModelId=3C5D9329002A
	unsigned int getSkillSum();
	//##ModelId=3C5D93290034
	int getTeachingDelta(cChar* pPlayer, int skill, int sum);
	//##ModelId=3C5D932900B6
	void removeItemBonus(cItem* pi);
	//##ModelId=3C5D932900E9
	void Init(bool ser = true);
	//##ModelId=3C5D9329011B
	bool isSameAs(cChar* pc) {if (!pc || pc->serial != serial) return false; else return true;}
	//##ModelId=3C5D9329014D
	bool inGuardedArea();
	//##ModelId=3C5D93290157
	bool canPickUp(cItem* pi);
	//##ModelId=3C5D9329019D
	unsigned int dist(cChar* pc);
	//##ModelId=3C5D932901CF
	unsigned int dist(cItem* pi);
	//##ModelId=3C5D93290201
	int MyHome();
};

//##ModelId=3C5D932A0054
class cCharStuff
{
public:
	//##ModelId=3C5D932A0072
	void DeleteChar(P_CHAR pc_k);
	//##ModelId=3C5D932A0086
	P_CHAR MemCharFree();
	//##ModelId=3C5D932A0090
	P_ITEM AddRandomLoot(P_ITEM pBackpack, char * lootlist);
	//##ModelId=3C5D932A00AE
	int getRandomNPC(char *npclist);
	//##ModelId=3C5D932A00B9
	P_CHAR AddNPCxyz(int s, int npcNum, int type, int x1, int y1, signed char z1);
	//##ModelId=3C5D932A00F4
	void Split(P_CHAR pc_k);
	//##ModelId=3C5D932A00FF
	void CheckAI(unsigned int currenttime, P_CHAR pc_i);
	//##ModelId=3C5D932A011C
	P_CHAR AddNPC(int s, P_ITEM pi_i, int npcNum, int x1 = 0, int y1 = 0, signed char z1 = 0);
	// Sky's AI Stuff
	//##ModelId=3C5D932A0253
	class cDragonAI
	{
	public:
		//##ModelId=3C5D932A0267
		void DoAI(P_CHAR pc_i,int currenttime);
		//##ModelId=3C5D932A0285
		void HarmMagic(P_CHAR pc_i, unsigned int currenttime,P_CHAR pc);
		//##ModelId=3C5D932A02A3
		void HealMagic(P_CHAR pc_i, unsigned int currenttime);
		//##ModelId=3C5D932A02B7
		void Claw(int i,int currenttime);
		//##ModelId=3C5D932A02D5
		void Bite(int i,int currenttime);
		//##ModelId=3C5D932A02E9
		void Breath(P_CHAR pc_i,int currenttime);
		//##ModelId=3C5D932A0307
		void DoneAI(P_CHAR pc_i,int currenttime);
	};
	
	//##ModelId=3C5D932A03E3
	class cBankerAI
	{
	public:
		//##ModelId=3C5D932B000F
		bool DoAI(int c, P_CHAR pBanker,char *comm);
		//##ModelId=3C5D932B002D
		bool Withdraw(int c, P_CHAR pBanker, char *comm);
		//##ModelId=3C5D932B004B
		bool BankCheck(int c, P_CHAR pBanker, char *comm);
		//##ModelId=3C5D932B006A
		bool Balance(int c, P_CHAR pBanker);
		//##ModelId=3C5D932B0087
		void OpenBank(UOXSOCKET c);
	};
};


// Inline Member functions
//##ModelId=3C5D93270122
inline bool  cChar::isPlayer() const		{return (!this->npc);}
//##ModelId=3C5D93270137
inline bool  cChar::isNpc()	const			{return (this->npc);}
//##ModelId=3C5D9327014A
inline bool  cChar::isHuman() const			{return (this->id() == 0x190 || this->id() == 0x191);} 
//##ModelId=3C5D9327015E
inline bool  cChar::isTrueGM() const		{return (priv&0x01);} 
//##ModelId=3C5D93270168
inline bool  cChar::isGM() const			{return (priv&0x01 && (!gmrestrict || region==gmrestrict)) || account == 0;} 
//##ModelId=3C5D932701AE
inline bool  cChar::isCounselor() const		{return (priv&0x80 ? true : false);} 
//##ModelId=3C5D932701C2
inline bool  cChar::isGMorCounselor() const	{return (priv&0x81 ?true:false);} 
//##ModelId=3C5D932701CC
inline bool  cChar::isInvul() const			{return (priv&0x04 ?true:false);}
//##ModelId=3C5D932701E0
inline bool  cChar::canSnoop() const		{return (priv&0x40 ?true:false);}
//##ModelId=3C5D932701EB
inline bool  cChar::canBroadcast() const	{return (priv&0x02 ?true:false);}
//##ModelId=3C5D932701FE
inline bool  cChar::canSeeSerials() const 	{return (priv&0x08 ?true:false);}
//##ModelId=3C5D93270212
inline bool  cChar::isInnocent() const		{return (flag&0x04 ?true:false);}
//##ModelId=3C5D9327021C
inline bool  cChar::isMurderer() const		{return (flag&0x01 ?true:false);}
//##ModelId=3C5D93270230
inline bool  cChar::isCriminal() const		{return (flag&0x02 ?true:false);}
//##ModelId=3C5D93270244
inline unsigned char cChar::getPriv()	{return priv;}
//##ModelId=3C5D9327024E
inline void cChar::setPriv(unsigned char p)	{this->priv=p;}
//##ModelId=3C5D9327028A
inline void cChar::makeInvulnerable()	{priv |= 4;}
//##ModelId=3C5D93270294
inline void cChar::makeVulnerable()		{priv &= 0xFB;}
//##ModelId=3C5D9327029E
inline void cChar::setMurderer()		{flag = 0x01;}
//##ModelId=3C5D932702B2
inline void cChar::setInnocent()		{flag = 0x04;}
//##ModelId=3C5D932702BC
inline void cChar::setCriminal()		{flag=0x02;}


#endif
