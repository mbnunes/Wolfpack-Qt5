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

#include "qptrlist.h"
#include <map>
// Platform Include
#include "platform.h"

// just temporary
#include "debug.h"

//#include "guildstones.h"
#include "typedefs.h"
#include "structs.h"
#include "defines.h"
#include "uobject.h"
#include "accounts.h"
#include "makemenus.h"

// Forward class declaration
class QString;
class cUOSocket;
class cGuildStone;
class cTerritory;

#undef  DBGFILE
#define DBGFILE "chars.h"


//typedef struct char_st_
class cChar : public cUObject
{
// Public Data Members
public:
	UINT8 notority( P_CHAR pChar ); // Gets the notority toward another char
	void kill();
	void resurrect();
	void turnTo( cUObject *object );
	P_CHAR unmount();
	void mount( P_CHAR pMount );

    enum enInputMode { enNone, enRenameRune, enPricing, enDescription, enNameDeed, enHouseSign, enPageGM, enPageCouns};
	//  Chaos/Order Guild Stuff for Ripper
	
	
	bool	canMoveAll( void ) { return priv2_&0x01; }
	bool	isFrozen( void ) { return priv2_&0x02; }	
	bool	viewHouseIcons( void ) { return priv2_&0x04; }	
	bool	isHiddenPermanently( void ) { return priv2_&0x08; }

	
	
	
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
	//int pagegm; //GM Paging
	//char region;
	cTerritory*				region;
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
	
	//int statuse[3]; //Morrolan - stat/skill cap STR/INT/DEX in that order
	//int skilluse[TRUESKILLS][1]; //Morrolan - stat/skill cap
	unsigned char			lockSkill[ALLSKILLS+1]; // LB, client 1.26.2b skill managment
    // commands shows 
    // 0 = off 
    // 1 = FlameStrike 
    // 2-6 = Sparkles
    int						gmMoveEff;
	int						VisRange;

	// Protected Data Members	
protected:
	SERIAL					trackingTarget_;
	bool					animated;
	short					GuildType;    // (0) Standard guild, (1) Chaos Guild, (2) Order guild
	bool					GuildTraitor; // (true) This character converted, (false) Neve converted, or not an order/chaos guild member
	QString					orgname_;//original name - for Incognito
	QString					title_;
	bool					unicode_; // This is set to 1 if the player uses unicode speech, 0 if not
	unsigned short			id_;
	AccountRecord*			account_; // changed to signed, lb
	bool					incognito_;// AntiChrist - true if under incognito effect
	bool					polymorph_;// AntiChrist - true if under polymorph effect
	unsigned short			haircolor_; // backup of hair/beard for incognito spell
	unsigned short			hairstyle_;
	unsigned short			beardcolor_;
	unsigned short			beardstyle_;
	UI16					skin_; // Skin color
	unsigned short			orgskin_;	// skin color backup for incognito spell
	UI16					xskin_; // Backup of skin color
	unsigned int			creationday_;	// Day since EPOCH this character was created on

	int						stealth_; //AntiChrist - stealth ( steps already done, -1=not using )
	unsigned int			running_; //AntiChrist - Stamina Loose while running
	unsigned int			logout_; //unsigned int logout;//Time till logout for this char -1 means in the world or already logged out //Instalog
	unsigned int			clientidletime_; // LB
	SERIAL					swingtarg_; //Tagret they are going to hit after they swing
	unsigned int			holdg_; // Gold a player vendor is holding for Owner
	unsigned char			fly_steps_; // number of step the creatures flies if it can fly
	int						menupriv_; // needed fro LB's menu priv system
	bool					tamed_;
	bool					guarded_;							// (Abaddon) if guarded
	bool					casting_; // 0/1 is the cast casting a spell?
    unsigned int			smoketimer_; // LB
	unsigned int			smokedisplaytimer_;
	unsigned int			antispamtimer_;
	unsigned int			antiguardstimer_;//AntiChrist - anti "GUARDS" spawn
	QString					carve_; // carve system
	int						hairserial_;//there are needed for incognito stuff
	int						beardserial_;
	unsigned int			begging_timer_;
	int						postType_;
	int						questType_;
	int						questDestRegion_;
	int						questOrigRegion_;
	int						questBountyReward_;      // The current reward amount for the return of this chars head
	int						questBountyPostSerial_;  // The global posting serial number of the bounty message
	SERIAL					murdererSer_;            // Serial number of last person that murdered this char
	Coord_cl				prevPos_;
	unsigned char			commandLevel_;             // 0 = player, 1 = counselor, 2 = GM
	QString					spawnregion_; 
	SERIAL					stablemaster_serial_; 
	unsigned char			npc_type_;		// currently only used for stabling, (type==1 -> stablemaster)
	// can be used for other npc types too of course
	unsigned int			time_unused_;     
	unsigned int			timeused_last_;
	SERIAL					spawnserial_; // Spawned by
	unsigned char			hidden_; // 0 = not hidden, 1 = hidden, 2 = invisible spell
	unsigned int			invistimeout_;
	bool					attackfirst_; // 0 = defending, 1 = attacked first
	bool					onhorse_; // On a horse?
	int						hunger_;  // Level of hungerness, 6 = full, 0 = "empty"
	unsigned int			hungertime_; // Timer used for hunger, one point is dropped every 20 min
	SERIAL					smeltitem_;
	SERIAL					tailitem_;
	int						npcaitype_; // NPC ai
	int						callnum_; //GM Paging
	int						playercallnum_; //GM Paging

	unsigned int			fishingtimer_; // Timer used to delay the catching of fish
	int						advobj_; //Has used advance gate?
	int						poison_; // used for poison skill 
	unsigned int			poisoned_; // type of poison
	unsigned int			poisontime_; // poison damage timer
	unsigned int			poisontxt_; // poision text timer
	unsigned int			poisonwearofftime_; // LB, makes poision wear off ...
	short					fleeat_;
	short					reattackat_;
	int						trigger_; //Trigger number that character activates
	QString					trigword_; //Word that character triggers on.
	unsigned int			disabled_; //Character is disabled, cant trigger.
	QString					disabledmsg_; //Character is disabled, so dysplay this message. -- added by Magius(CHE) §
	unsigned short			envokeid_;  //ID1 of item user envoked
	SERIAL					envokeitem_;
	unsigned char			split_;
	unsigned char			splitchnc_;
	int						targtrig_; //Stores the number of the trigger the character for targeting
	char					ra_;  // Reactive Armor spell
	SERIAL					trainer_; // Serial of the NPC training the char, -1 if none.
	char					trainingplayerin_; // Index in skillname of the skill the NPC is training the player in
	bool					cantrain_;
	// Begin of Guild Related Character information (DasRaetsel)
	bool					guildtoggle_;		// Toggle for Guildtitle								(DasRaetsel)
	QString					guildtitle_;			// Title Guildmaster granted player						(DasRaetsel)
	SERIAL					guildfealty_;		// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	SERIAL					guildstone_;			// Number of guild player is in (0=no guild)			(DasRaetsel)
	char					flag_; //1=red 2=grey 4=Blue 8=green 10=Orange
	unsigned int			tempflagtime_;
	UINT32					trackingTimer_;
	// End of Guild Related Character information
	unsigned int			murderrate_; //#of ticks until one murder decays //REPSYS 
	long int				crimflag_; //Time when No longer criminal -1=Not Criminal
	unsigned int			spelltime_; //Time when they are done casting....
	int						spell_; //current spell they are casting....
	int						spellaction_; //Action of the current spell....
	int						nextact_; //time to next spell action....
	SERIAL					poisonserial_; //AntiChrist -- poisoning skill
	int						squelched_; // zippy  - squelching
	unsigned int			mutetime_; //Time till they are UN-Squelched.
	bool					med_; // 0=not meditating, 1=meditating //Morrolan - Meditation 
	unsigned short  		baseSkill_[ALLSKILLS+1]; // Base skills without stat modifiers
	unsigned short  		skill_[ALLSKILLS+1]; // List of skills (with stat modifiers)
	cUOSocket*				socket_;
	unsigned short			weight_;

	
	unsigned char			priv;	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	void	day(unsigned long day) ; // set the day it was created
	unsigned long day() ;	// Retrieve the day it was created
	// a temporary (quick&dirty) encapsulation for Dexterity. (Duke, 21.8.2001)
	signed short			dx;		// Dexterity
	signed short			dx2;		// holds the 3 digits behind the decimal point. Reserved for calculation
	signed short			tmpDex;	// holds all temporary effects on Dex, eg. plate, spells, potions

	QString					loot_; // holds the lootlist section
//BEGIN ADDED FROM PUBLIC*****************************************
	unsigned char			fonttype_; // Speech font to use
	UI16					saycolor_; // Color for say messages
	unsigned short			emotecolor_; // Color for emote messages
	signed short			st_; // Strength
	signed short			st2_; // Reserved for calculation

	RACE					race_; // -Fraz- Race AddOn
	// Skyfire's NPC advancments.
	bool					may_levitate_;
	//Skyfire - End NPC's home/work/food vars'
	unsigned char			pathnum_;
	path_st					path_[PATHNUM];

	signed char				dispz_;   // Z that the char is SHOWN at. Server needs other coordinates for real movement calculations.
	// changed from unsigned to signed, LB
	
	unsigned char			dir_; //&0F=Direction
	unsigned short			xid_; // Backup of body type for ghosts
	unsigned char			priv2_;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	
	signed short			in_; // Intelligence
	signed short			in2_; // Reserved for calculation
	signed short			hp_; // Hitpoints
	signed short			stm_; // Stamina
	signed short			mn_; // Mana
	signed short			mn2_; // Reserved for calculation

	int						hidamage_; //NPC Damage
	int						lodamage_; //NPC Damage
	bool					npc_;	// true = Character is an NPC
	bool					shop_;	// true = npc shopkeeper
	unsigned char			cell_; // Reserved for jailing players
	unsigned int			jailtimer_; // Blackwind - Timer used for crystall ball and jail time.
	int						jailsecs_;	//             Tweak this value by using command tweak before jailing person 
	//			   or he will be jailed 1 day ( in realtime )

//END ADDED FROM PUBLIC ******************************************
	std::map< cMakeMenu*, QPtrList< cMakeSection > >	lastselections_;

	// Public Methods
public:
	cChar();
	virtual void Serialize(ISerialization &archive);
	virtual QString objectID() const;
    
	// A typical sequence for the following could be:
	// Remove from View( clean = false )
	// Change coordinates for the char
	// Resend( clean = false )
	// This saves bandwith and CPU time !
	void setAnimated( bool data ) { animated = data; }
	void update( void ); // This is called when flags/name have been changed
	void resend( bool clean = true ); // this is called when the char is being created or anything like that
	void makeShop( void );
	void updateHealth( void );
	void action( UINT8 id ); // Do an action
	QString fullName( void );

	// Getters
	short					guildType() const;    // (0) Standard guild, (1) Chaos Guild, (2) Order guild
	bool					guildTraitor() const; // (true) This character converted, (false) Neve converted, or not an order/chaos guild member
	QString					orgname() const;	  //original name - for Incognito
	QString					title() const;
	bool					unicode() const; // This is set to 1 if the player uses unicode speech, 0 if not
	AccountRecord*			account() const; // changed to signed, lb
	bool					incognito() const { return incognito_;	}
	bool					polymorph() const { return polymorph_;	}
	unsigned short			haircolor() const { return haircolor_;	}
	unsigned short			hairstyle() const { return hairstyle_;	}
	unsigned short			beardcolor() const{ return beardcolor_; }
	unsigned short			beardstyle() const{ return beardstyle_;	}
	UI16					skin() const	  { return skin_;		}
	unsigned short			orgskin() const	  { return orgskin_;	}
	UI16					xskin() const     { return xskin_;		}
	SERIAL					trackingTarget() const { return trackingTarget_; }
	unsigned int			creationday() const{return creationday_;}
	int						stealth() const { return stealth_; }
	unsigned int			running() const { return running_; }
	unsigned int			logout() const {return logout_;} 
	unsigned int			clientidletime() const {return clientidletime_;}
	SERIAL					swingtarg() const { return swingtarg_; }
	unsigned int			holdg() const {return holdg_;}
	unsigned char			fly_steps() const {return fly_steps_;} // number of step the creatures flies if it can fly
	int						menupriv() const {return menupriv_;}
	bool					tamed() const {return tamed_;}
	bool					guarded() const {return guarded_;}
    unsigned int			smoketimer() const {return smoketimer_;}
	unsigned int			smokedisplaytimer() const {return smokedisplaytimer_;}
	unsigned int			antispamtimer() const { return antispamtimer_;}
	unsigned int			antiguardstimer() const { return antiguardstimer_;}
	QString					carve() const {return carve_;}
	SERIAL					hairserial() const {return hairserial_;}
	SERIAL					beardserial() const { return beardserial_;}
	unsigned int			begging_timer() const {return begging_timer_;}
	int						postType() const {return postType_;}
	int						questType() const {return questType_;}
	int						questDestRegion() const {return questDestRegion_;}
	int						questOrigRegion() const {return questOrigRegion_;}
	int						questBountyReward() const {return questBountyReward_;}
	int						questBountyPostSerial() const {return questBountyPostSerial_;}
	SERIAL					murdererSer() const {return murdererSer_;}
	Coord_cl				prevPos() const { return prevPos_; }
	unsigned char			commandLevel() const { return commandLevel_;}             // 0 = player, 1 = counselor, 2 = GM
	QString					spawnregion() const {return spawnregion_;} 
	SERIAL					stablemaster_serial() const {return stablemaster_serial_;} 
	unsigned char			npc_type() const {return npc_type_;}
	unsigned int			time_unused() const { return time_unused_;}
	unsigned int			timeused_last() const { return timeused_last_;}
	bool					casting() const { return casting_;	}
	SERIAL					spawnSerial() const { return spawnserial_;}
	unsigned char			hidden() const { return hidden_; } // 0 = not hidden, 1 = hidden, 2 = invisible spell
	unsigned int			invistimeout() const { return invistimeout_;}
	bool					attackfirst() const { return attackfirst_; }
	bool					onHorse() const { return onhorse_; }
	int						hunger() const { return hunger_; }
	unsigned int			hungertime() const { return hungertime_;}
	SERIAL					smeltitem() const { return smeltitem_; }
	SERIAL					tailitem() const { return tailitem_; }
	int						npcaitype() const { return npcaitype_;}
	int						callnum() const { return callnum_; }
	int						playercallnum() const { return playercallnum_; }
	unsigned int			fishingtimer() const {return fishingtimer_;}
	int						advobj() const { return advobj_;}
	int						poison() const { return poison_;}
	unsigned int			poisoned() const { return poisoned_; }
	unsigned int			poisontime() const { return poisontime_;}
	unsigned int			poisontxt() const { return poisontxt_;}
	unsigned int			poisonwearofftime() const { return poisonwearofftime_;}
	short					fleeat() const { return fleeat_;}
	short					reattackat() const { return reattackat_; }
	int						trigger() const { return trigger_; }
	QString					trigword() const { return trigword_; }
	unsigned int			disabled() const { return disabled_; }
	QString					disabledmsg() const { return disabledmsg_; }
	unsigned short			envokeid() const { return envokeid_;}
	SERIAL					envokeitem() const { return envokeitem_;}
	unsigned char			split() const { return split_;}
	unsigned char			splitchnc() const { return splitchnc_;}
	int						targtrig() const { return targtrig_;}
	char					ra() const { return ra_;}
	SERIAL					trainer() const { return trainer_;}
	char					trainingplayerin() const { return trainingplayerin_;}
	bool					cantrain() const { return cantrain_; }
	bool					guildtoggle() const { return guildtoggle_; }
	QString					guildtitle() const { return guildtitle_; }
	SERIAL					guildfealty() const { return guildfealty_;}
	cGuildStone*			getGuildstone();
	SERIAL					guildstone() const { return guildstone_; }
	char					flag() const { return flag_;}
	unsigned int			tempflagtime() const { return tempflagtime_; }
	unsigned int			murderrate() const { return murderrate_;}
	long int				crimflag() const { return crimflag_;}
	unsigned int			spelltime() const { return spelltime_;}
	int						spell() const { return spell_;}
	int						spellaction() const { return spellaction_;}
	int						nextact() const { return nextact_;}
	SERIAL					poisonserial() const { return poisonserial_;}
	int						squelched() const { return squelched_;}
	int						mutetime() const { return mutetime_; }
	bool					med() const { return med_;}
	unsigned short			baseSkill( int v ) const { return baseSkill_[v]; }
	unsigned short			skill( int v ) const {return skill_[v];} // List of skills (with stat modifiers)
	cUOSocket*				socket() const { return socket_; }
	unsigned short			weight() const { return weight_; }
	unsigned short			stones() const { return (weight_ / 10); }
	QString					lootList() const { return loot_; }
	UINT32					trackingTimer() const { return trackingTimer_; }

//BEGIN ADDED GETTERS**********************************************
	unsigned char			fonttype() const { return fonttype_; }
	UI16					saycolor() const { return saycolor_; }
	unsigned short			emotecolor() const { return emotecolor_; }
	signed short			st() const { return st_; }
	signed short			st2() const { return st2_; }
	RACE					race() const { return race_; }
	bool					may_levitate() const { return may_levitate_; }
	unsigned char			pathnum() const { return pathnum_; }
	path_st					path( int val ) const { return path_[val]; }
	unsigned short			pathX( int val ) const { return path_[val].x; }
	unsigned short			pathY( int val ) const { return path_[val].y; }
	signed char				dispz() const { return dispz_; }   
	unsigned char			dir() const { return dir_; }
	unsigned short			xid() const { return xid_; }
	unsigned char			priv2() const { return priv2_; }
	signed short			in() const { return in_; }
	signed short			in2() const { return in2_; }
	signed short			hp() const { return hp_; }
	signed short			stm() const { return stm_; }
	signed short			mn() const { return mn_; }
	signed short			mn2() const { return mn2_; }
	int						hidamage() const { return hidamage_; }
	int						lodamage() const { return lodamage_; }
	bool					npc() const { return npc_; }
	bool					shop() const { return shop_; }
	unsigned char			cell() const { return cell_; }
	unsigned int			jailtimer() const { return jailtimer_; }
	int						jailsecs() const { return jailsecs_; }
//END ADDED GETTERS***********************************************
	QPtrList< cMakeSection > lastSelections( cMakeMenu* basemenu )
	{ 
		std::map< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastselections_.find( basemenu );
		if( it != lastselections_.end() )
			return it->second;
		else
			return QPtrList< cMakeSection >();
	}

	cMakeSection*			lastSection( cMakeMenu* basemenu )
	{
		std::map< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastselections_.find( basemenu );
		QPtrList< cMakeSection > lastsections_;
		if( it != lastselections_.end() )
			 lastsections_ = it->second;
		else return NULL;

		if( lastsections_.count() > 0 )
			return lastsections_.at(0);
		else
			return NULL;
	}
	
	// Setters
	void					setGuildType(short data);
	void					setGuildTraitor(bool  data);
	void					setOrgname(const QString& data);//original name - for Incognito
	void					setTitle( const QString& data);
	void					setUnicode( bool data); // This is set to 1 if the player uses unicode speech, 0 if not
	void					setAccount( AccountRecord* data, bool moveFromAccToAcc = true ); // changed to signed, lb
	void					setIncognito ( bool data) { incognito_ = data; } 
	void					setPolymorph ( bool data) { polymorph_ = data; }
	void					setHairColor ( unsigned short data) { haircolor_ = data; }
	void					setHairStyle ( unsigned short data) { hairstyle_ = data; }
	void					setBeardColor( unsigned short data) { beardcolor_ = data;}
	void					setBeardStyle( unsigned short data) { beardstyle_ = data;}
	void					setSkin( unsigned short data) { skin_ = data; }
	void					setOrgSkin( unsigned short data) { orgskin_ = data;}
	void					setXSkin( unsigned short data) { xskin_ = data; }
	void					setCreationDay( unsigned int data ) { creationday_ = data; }
	void					setStealth(int data) {stealth_ = data;}
	void					setRunning(unsigned int data) {running_ = data;}
	void					setLogout(unsigned int data) {logout_ = data;}
	void					setClientIdleTime( unsigned int data ) { clientidletime_ = data;}
	void					setSwingTarg( SERIAL data ) { swingtarg_ = data; }
	void					setHoldg( unsigned int data ) {holdg_ = data; }
	void					setFlySteps( unsigned char data) {fly_steps_ = data; }
	void					setMenupriv(int data) { menupriv_ = data;}
	void					setTamed( bool data ) { tamed_ = data; }
	void					setGuarded( bool data ) { guarded_ = data; }
    void					setSmokeTimer( unsigned int data ) { smoketimer_ = data; }
	void					setSmokeDisplayTimer ( unsigned int data ) { smokedisplaytimer_ = data; }
	void					setAntispamtimer ( unsigned int data ) { antispamtimer_ = data;}
	void					setAntiguardstimer( unsigned int data ) { antiguardstimer_ = data;}
	void					setCarve( QString data ) { carve_ = data;}
	void					setHairSerial( SERIAL data ) { hairserial_ = data;}
	void					setBeardSerial( SERIAL data ) {beardserial_ = data;}
	void 					setBegging_timer( unsigned int data ) { begging_timer_ = data;}
	void					setPostType( int data ) { postType_ = data;}
	void					setQuestType ( int data ) { questType_ = data;}
	void					setQuestDestRegion( int data ) { questDestRegion_ = data;}
	void					setQuestOrigRegion( int data ) {questOrigRegion_ = data;}
	void					setQuestBountyReward ( int data ) { questBountyReward_ = data;}
	void					setQuestBountyPostSerial ( int data ) {questBountyPostSerial_ = data;}
	void					setMurdererSer( SERIAL data ) { murdererSer_ = data;}
	void					setPrevPos( const Coord_cl& data ) { prevPos_ = data; }
	void					setCommandLevel( unsigned char data ) { commandLevel_ = data;	}
	void					setSpawnregion ( QString data ) { spawnregion_ = data;	}
	void					setStablemaster_serial (SERIAL data ) { stablemaster_serial_ = data; }  
	void					setNpc_type( unsigned char data ) { npc_type_ = data; }
	void					setTime_unused ( unsigned int data ) { time_unused_ = data; }
	void					setTimeused_last( unsigned int data ) { timeused_last_ = data;}
	void					setCasting( bool data ) { casting_ = data;	}
	void					setSpawnSerial( SERIAL data ) { spawnserial_ = data;}
	void					setHidden ( unsigned char data ) { hidden_ = data;}
	void					setInvisTimeout ( unsigned int data ) { invistimeout_ = data;}
	void					setAttackFirst ( bool data ) { attackfirst_ = data;}
	void					setOnHorse ( bool data ) { onhorse_ = data; }
	void					setHunger ( int data ) { hunger_ = data; }
	void					setHungerTime ( unsigned int data ) { hungertime_ = data;}
	void					setSmeltItem ( SERIAL data ) { smeltitem_ = data;}
	void					setTailItem ( SERIAL data ) { tailitem_ = data;}
	void					setNpcAIType( int data ) { npcaitype_ = data;}
	void					setCallNum ( int data ) { callnum_ = data;}
	void					setPlayerCallNum ( int data ) { playercallnum_ = data;}

	void					setFishingtimer( unsigned int data ) { fishingtimer_ = data;}
	void					setAdvobj( int data ) { advobj_ = data; }
	void					setPoison( int data ) { poison_ = data;}
	void					setPoisoned( unsigned int data ) {poisoned_ = data;}
	void					setPoisontime( unsigned int data ) { poisontime_ = data;}
	void					setPoisontxt( unsigned int data ) { poisontxt_ = data;}
	void					setPoisonwearofftime( unsigned int data ) {poisonwearofftime_ = data;}
	void					setFleeat( short data ) { fleeat_ = data; }
	void					setReattackat(short data) { reattackat_ = data; }
	void					setTrigger( int data ) { trigger_ = data; }
	void					setTrigword( const QString& data ) { trigword_ = data;}
	void					setDisabled( unsigned int data ) { disabled_ = data;}
	void					setDisabledmsg( const QString& data ) { disabledmsg_ = data;}
	void					setEnvokeid( unsigned short data ) { envokeid_ = data;}
	void					setEnvokeitem( SERIAL data ) { envokeitem_ = data; }
	void					setSplit(unsigned char data) {split_ = data;}
	void					setSplitchnc(unsigned char data) {splitchnc_ = data;}
	void					setTargtrig( int data ) { targtrig_ = data; }
	void					setRa( char data ) { ra_ = data;}
	void					setTrainer( SERIAL data ) { trainer_ = data;}
	void					setTrainingplayerin( char data ) { trainingplayerin_ = data;}
	void					setCantrain( bool data ) { cantrain_ = data;}
	void					setGuildtoggle( bool data ) { guildtoggle_ = data;}
	void					setGuildtitle( const QString& data ) { guildtitle_ = data;}
	void					setGuildfealty( SERIAL data ) { guildfealty_ = data;}
	void					setGuildstone( SERIAL data ) { guildstone_ = data; }
	void					setFlag( char data ) { flag_ = data; }
	void					setTempflagtime( unsigned int data ) { tempflagtime_ = data;}
	void					setMurderrate( unsigned int data ) { murderrate_ = data;}
	void					setCrimflag( unsigned int data ) { crimflag_ = data;}
	void					setSpelltime( unsigned int data ) { spelltime_ = data;}
	void					setSpell( int data ) { spell_ = data;}
	void					setSpellaction( int data ) { spellaction_ = data;}
	void					setNextact( int data ) { nextact_ = data;}
	void					setPoisonserial( SERIAL data ) {poisonserial_ = data;}
	void					setSquelched( int data) { squelched_ = data;}
	void					setMutetime( int data ) { mutetime_ = data;}
	void					setMed( bool data ) { med_ = data;}
	void					setBaseSkill( int s, unsigned short v) { baseSkill_[s] = v; }
	void					setSkill( int s, unsigned short v) { skill_[s] = v;}
	void					setSocket( cUOSocket* data ) { socket_ = data; }
	void					setWeight( unsigned short data ) { weight_ = data; }
	void					setLootList( QString data ) { loot_ = data; }
	void					setTrackingTarg( SERIAL data ) { trackingTarget_ = data; }
	void					setTrackingTimer( UINT32 data ) { trackingTimer_ = data; }

//ADDED SETTERS******************************************************
	void					setFontType( unsigned char data ) { fonttype_ = data; }
	void					setSayColor( UI16 data ) { saycolor_ = data; }
	void					setEmoteColor( unsigned short data ) { emotecolor_ = data; }
	void					setSt( signed short data ) { st_ = data; }
	void					setSt2( signed short data ) { st2_ = data; }
	void					setRace( RACE data ) { race_ = data; }
	void					setMay_Levitate( bool data ) { may_levitate_ = data; }
	void					setPathNum( unsigned char data ) { pathnum_ = data; }
	void					setPath( int p, path_st val ){ path_[p] = val; }
	void					setPathX( int p, unsigned short xValue ) { path_[p].x = xValue; }
	void					setPathY( int p, unsigned short yValue ) { path_[p].y = yValue; }
	void					setDispz( signed char data ) { dispz_ = data; }
	void					setDir( unsigned char data ) { dir_ = data; }
	void					setXid( unsigned short data ) { xid_ = data; }
	void					setPriv2( unsigned char data ) { priv2_ = data; }
	void					setIn( signed short data ) { in_ = data; }
	void					setIn2( signed short data ) { in2_ = data; }
	void					setHp( signed short data ) { hp_ = data; }
	void					setStm( signed short data ) { stm_ = data; }
	void					setMn( signed short data ) { mn_ = data; }
	void					setMn2( signed short data ) { mn2_ = data; }
	
	void					setHiDamage( int data ) { hidamage_ = data; }
	void					setLoDamage( int data ) { lodamage_ = data; }
	void					setNpc( bool data ) { npc_ = data; }
	void					setShop( bool data ) { shop_ = data; }
	void					setCell( unsigned char data ) { cell_ = data; }
	void					setJailTimer( unsigned int data ) { jailtimer_ = data; }
	void					setJailSecs( int data ) { jailsecs_ = data; }
//END SETTERS********************************************************
	void					setLastSection( cMakeMenu* basemenu, cMakeSection* data )
	{
		std::map< cMakeMenu*, QPtrList< cMakeSection > >::iterator mit = lastselections_.find( basemenu );
		QPtrList< cMakeSection > lastsections_;
//		lastsections_.setAutoDelete( true ); NEVER DELETE THE SECTIONS :) THEY ARE DELETED WITH THEIR MAKEMENU PARENTS
		if( mit != lastselections_.end() )
			lastsections_ = mit->second;
		else
		{
			lastsections_.append( data );
			lastselections_.insert( make_pair< cMakeMenu*, QPtrList< cMakeSection > >(basemenu, lastsections_) );
			return;
		}
			
		QPtrListIterator< cMakeSection > it( lastsections_ );
		while( it.current() )
		{
			if( data == it.current() )
				return;
			++it;
		}
		lastsections_.prepend( data );
		while( lastsections_.count() > 10 )
			lastsections_.removeLast();

		mit->second = lastsections_;
		return;
	}

	void					clearLastSelections( void )
	{
		lastselections_.clear();
	}

	void wear( P_ITEM );
	void updateWornItems();
	void updateWornItems( cUOSocket* );

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
	unsigned short id() const				{return id_;}
	void setId(unsigned short data)			{id_ = data;}
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
	unsigned char getPriv() const;
	void setPriv(unsigned char p);
	void showName( cUOSocket *socket );
	void makeInvulnerable();
	void makeVulnerable();
	void setMurderer();
	void setInnocent();
	void setCriminal();
	void setAttackFirst()		{this->attackfirst_ = true;}
	void resetAttackFirst()		{this->attackfirst_ = false;}
	void fight(cChar* pOpponent);
	void setNextMoveTime(short tamediv=1);
	void disturbMed();
	void unhide();
	bool isHidden() { return hidden() > 0 ? true : false; }
	bool isHiddenBySpell() { return hidden() & 2 ? true : false; }
	bool isHiddenBySkill() { return (hidden() & 1); }
	int  CountItems(short ID, short col= -1);
	int  CountGold()			{return CountItems(0x0EED);}
	P_ITEM GetItemOnLayer(unsigned char layer);
	P_ITEM getBankBox( void );
	void openBank( UOXSOCKET socket = -1 );
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
	void giveItemBonus(cItem* pi);
	void Init(bool ser = true);
	bool isSameAs(cChar* pc) {if (!pc || pc->serial != serial) return false; else return true;}
	bool inGuardedArea();
	bool canPickUp(cItem* pi);
	unsigned int dist(cChar* pc);
	unsigned int dist(cItem* pi);
	void soundEffect( UI16 soundId, bool hearAll = true );
	void giveGold( Q_UINT32 amount, bool inBank = false );
	UINT32 takeGold( UINT32 amount, bool useBank = false );
	void emote( const QString &emote, UI16 color = 0xFFFF );
	void message( const QString &message, UI16 color = 0x3B2 );

	// Wrapper events! - darkstorm
	bool onShowCharName( P_CHAR Viewer ); // Shows the name of a character to someone else
	bool onWalk( UI08 Direction, UI08 Sequence ); // Walks in a specific Direction

	bool onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ); // The character says something
	bool onTalkToNPC( P_CHAR Talker, const QString &Text ); // Someone talks to the NPC
	bool onWarModeToggle( bool War ); // The character switches warmode
	bool onEnterWorld( void ); // The character enters the world
	bool onHelp( void ); // The character wants help
	bool onChat( void ); // The character wants to chat
	bool onSkillUse( UI08 Skill ); // The character uses %Skill
	bool onCollideChar( P_CHAR Obstacle ); // This is called for the walking character first, then for the character walked on

	virtual void talk( const QString &message, UI16 color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	void giveNewbieItems( Q_UINT8 skill = 0xFF );

	// Definition loading - sereg
protected:
	virtual void processNode( const QDomElement& Tag );
};

class cCharStuff
{
public:
	void DeleteChar(P_CHAR pc_k);
	P_CHAR MemCharFree();
	void Split(P_CHAR pc_k);
	void CheckAI(unsigned int currenttime, P_CHAR pc_i);
	P_CHAR createScriptNpc( const QString &section, const Coord_cl &pos );
	P_CHAR createScriptNpc( int s, P_ITEM pi_i, QString Section, int posx = 0, int posy = 0, signed char posz = 0 );
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
		bool DoAI(int c, P_CHAR pBanker, const QString& comm);
		bool Withdraw(int c, P_CHAR pBanker, const string& comm);
		bool BankCheck(int c, P_CHAR pBanker, const string& comm);
		bool Balance(int c, P_CHAR pBanker);
		void OpenBank(UOXSOCKET c);
	};
};


// Inline Member functions
inline bool  cChar::isPlayer() const		{return (!this->npc_);}
inline bool  cChar::isNpc()	const			{return (this->npc_);}
inline bool  cChar::isHuman() const			{return (this->id() == 0x190 || this->id() == 0x191);} 
inline bool  cChar::isTrueGM() const		{return (priv&0x01);} 
inline bool  cChar::isGM() const			{return  priv&0x01 || account_ == 0 || ( account() && ( account()->acl() == "admin" || account()->acl() == "gm" ) );} 
inline bool  cChar::isCounselor() const		{return (priv&0x80 || ( account() && ( account()->acl() == "counselor") ) );} 
inline bool  cChar::isGMorCounselor() const	{return (priv&0x81 || ( account() && ( account()->acl() == "admin" || account()->acl() == "gm" || account()->acl() == "counselor" ) ) );} 
inline bool  cChar::isInvul() const			{return (priv&0x04 ?true:false);}
inline bool  cChar::canSnoop() const		{return (priv&0x40 ?true:false);}
inline bool  cChar::canBroadcast() const	{return (priv&0x02 ?true:false);}
inline bool  cChar::canSeeSerials() const 	{return (priv&0x08 ?true:false);}
inline bool  cChar::isInnocent() const		{return (flag_&0x04 ?true:false);}
inline bool  cChar::isMurderer() const		{return (flag_&0x01 ?true:false);}
inline bool  cChar::isCriminal() const		{return (flag_&0x02 ?true:false);}
inline unsigned char cChar::getPriv() const	{return priv;}
inline void cChar::setPriv(unsigned char p)	{this->priv=p;}
inline void cChar::makeInvulnerable()		{priv |= 4;}
inline void cChar::makeVulnerable()			{priv &= 0xFB;}
inline void cChar::setMurderer()			{flag_ = 0x01;}
inline void cChar::setInnocent()			{flag_ = 0x04;}
inline void cChar::setCriminal()			{flag_ = 0x02;}

// Getters
inline short			cChar::guildType() const		{ return GuildType; }
inline bool				cChar::guildTraitor() const		{ return GuildTraitor; }
inline QString			cChar::orgname() const			{ return orgname_; }
inline QString			cChar::title() const			{ return title_;   }
inline bool				cChar::unicode() const			{ return unicode_; }
inline AccountRecord*	cChar::account() const			{ return account_; }

// Setters
inline void	cChar::setGuildType(short data)				{ GuildType = data; }
inline void cChar::setGuildTraitor(bool  data)			{ GuildTraitor = data; }
inline void	cChar::setOrgname( const QString& data )	{ orgname_ = data; }
inline void cChar::setTitle( const QString& data )		{ title_ = data;   }
inline void cChar::setUnicode(bool data)				{ unicode_ = data; }

#endif
