//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#if !defined(__CHARS_H__)
#define __CHARS_H__

// Platform Include
#include "platform.h"

// just temporary
#include "debug.h"

#include "typedefs.h"
#include "structs.h"
#include "defines.h"
#include "uobject.h"
#include "dbdriver.h"
#include "makemenus.h"
#include "network/uotxpackets.h"

// Library Includes
#include <qmap.h>
#include <qptrlist.h>
#include <qvaluevector.h>
#include <deque>

// Forward class declaration
class QString;
class cUOSocket;
class cGuildStone;
class cTerritory;
class cAllTerritories;
class cTempEffect;
class AccountRecord;
class cMakeMenu;
class cMakeSection;

#undef  DBGFILE
#define DBGFILE "chars.h"

class cChar : public cUObject
{
	Q_OBJECT
private:
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );

//	bool changed_;

// Public Types
public:
    enum enInputMode { enNone, enRenameRune, enPricing, enDescription, enNameDeed, enHouseSign, enPageGM, enPageCouns};
	typedef QMap<ushort, cItem*> ContainerContent;
	typedef QValueVector< cChar* > Followers;
	typedef QValueVector< cTempEffect* > Effects;

	//  Chaos/Order Guild Stuff for Ripper
	enum enLayer { TradeWindow, SingleHandedWeapon, DualHandedWeapon, Shoes, Pants, Shirt, Hat, Gloves,
	Ring, Neck = 0xA, Hair, Waist, InnerTorso, Bracelet, FacialHair = 0x10,  MiddleTorso, 
	Earrings, Arms, Back, Backpack, OuterTorso, OuterLegs, InnerLegs, Mount, BuyRestockContainer,
	BuyNoRestockContainer, SellContainer, BankBox, Dragging };


	// Protected Data Members	
protected:

	ContainerContent		content_;
	Followers				followers_; // NPC owned by this character
	Followers				guardedby_; // List of NPCs that guard this character.
	Effects					effects_; // Tempeffects affecting this character (Bless, Other Status affecting spells)
	P_CHAR					owner_;

	SERIAL					trackingTarget_;
	bool					animated;
	short					GuildType;    // (0) Standard guild, (1) Chaos Guild, (2) Order guild
	bool					GuildTraitor; // (true) This character converted, (false) Neve converted, or not an order/chaos guild member
	QString					orgname_;//original name - for Incognito
	QString					title_;
	bool					sex_;
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
	bool					tamed_;
	bool					casting_; // 0/1 is the cast casting a spell?
    unsigned int			smoketimer_; // LB
	unsigned int			smokedisplaytimer_;
	unsigned int			antispamtimer_;
	unsigned int			antiguardstimer_;//AntiChrist - anti "GUARDS" spawn
	P_CHAR					guarding_; // is guarding this.	
	QString					carve_; // carve system
	int						hairserial_;//there are needed for incognito stuff
	int						beardserial_;
	unsigned int			begging_timer_;
	int						postType_;
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
	int						hunger_;  // Level of hungerness, 6 = full, 0 = "empty"
	unsigned int			hungertime_; // Timer used for hunger, one point is dropped every 20 min
	SERIAL					tailitem_;
	int						npcaitype_; // NPC ai
	int						callnum_; //GM Paging
	int						playercallnum_; //GM Paging

	int						poison_; // used for poison skill 
	unsigned int			poisoned_; // type of poison
	unsigned int			poisontime_; // poison damage timer
	unsigned int			poisontxt_; // poision text timer
	unsigned int			poisonwearofftime_; // LB, makes poision wear off ...
	short					fleeat_;
	short					reattackat_;
	unsigned int			disabled_; //Character is disabled, cant trigger.
	QString					disabledmsg_; //Character is disabled, so dysplay this message. -- added by Magius(CHE) §
	unsigned short			envokeid_;  //ID1 of item user envoked
	SERIAL					envokeitem_;
	unsigned char			split_;
	unsigned char			splitchnc_;
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

	struct stSkillValue
	{
		UINT16 value; // Skill Value (Default: 0)
		UINT16 cap; // Special Cap Value (Default: 1000)
		UINT8 lock; // 0: Up, 1: Down, 2: Locked (Default: 0)

		stSkillValue(): value( 0 ), cap( 1000 ), lock( 0 ) {}
	};

	QValueVector< stSkillValue >	skills; // Skills for this Character

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
	unsigned char			fonttype_; // Speech font to use
	UI16					saycolor_; // Color for say messages
	unsigned short			emotecolor_; // Color for emote messages
	signed short			st_; // Strength
	signed short			st2_; // Reserved for calculation
	bool					may_levitate_;
	signed char				dispz_;   
	unsigned char			dir_; //&0F=Direction
	unsigned short			xid_; // Backup of body type for ghosts
	unsigned char			priv2_;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
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
	int						robe_; // Serial number of generated death robe (If char is a ghost)
	int						karma_;
	signed int				fame_;	
	unsigned int			kills_; //PvP Kills
	unsigned int			deaths_;
	bool					dead_; // Is character dead
	unsigned char			fixedlight_; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	unsigned int			def_; // Intrinsic defense
	bool					war_; // War Mode
	SERIAL					targ_; // Current combat target
	unsigned int			timeout_; // Combat timeout (For hitting)
	unsigned int			regen_, regen2_, regen3_;//Regeneration times for mana, stamin, and str
	enInputMode				inputmode_;	// Used for entering text; 0= none, 4=rename rune
	SERIAL					inputitem_;		// serial of item the text is referring to
	SERIAL					attacker_; // Character's serial who attacked this character
	unsigned int			npcmovetime_; // Next time npc will walk
	unsigned char			npcWander_; // NPC Wander Mode
	unsigned char			oldnpcWander_; // Used for fleeing npcs
	SERIAL					ftarg_; // NPC Follow Target
	Coord_cl				ptarg_;
	int						fx1_; //NPC Wander Point 1 x
	int						fx2_; //NPC Wander Point 2 x
	int						fy1_; //NPC Wander Point 1 y
	int						fy2_; //NPC Wander Point 2 y
	signed char				fz1_; //NPC Wander Point 1 z
	cTerritory*				region_;
	unsigned int			skilldelay_;
	unsigned int			objectdelay_;
	int						making_; // skill number of skill using to make item, 0 if not making anything.
	SERIAL					lastTarget_;
	char					blocked_;
	char					dir2_;
	unsigned int			spiritspeaktimer_; // Timer used for duration of spirit speak
	int						spattack_;
	int						spadelay_;
	unsigned int			spatimer_;
	int						taming_; //Skill level required for taming
	unsigned int			summontimer_; //Timer for summoned creatures.
	UINT8					VisRange_;
	QString					profile_;

	QMap< cMakeMenu*, QPtrList< cMakeSection > >	lastselections_;
	unsigned int			food_;

	std::deque< Coord_cl >	path_;	// A* calculated path to walk for NPCs, dont save this ! (sereg)

	// Public Methods
public:
	cChar();
	cChar( const P_CHAR );
	static void registerInFactory();
	void load( char **, UINT16& );
	void save();
	void save( FlatStore::OutputFile*, bool first = false ) throw();
	bool load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile* ) throw();
	bool postload() throw();
	bool del();
    
	// A typical sequence for the following could be:
	// Remove from View( clean = false )
	// Change coordinates for the char
	// Resend( clean = false )
	// This saves bandwith and CPU time !
	void setAnimated( bool d ) { animated = d; }
	void update( bool excludeself = false ); // This is called when flags/name have been changed
	void resend( bool clean = true, bool excludeself = false ); // this is called when the char is being created or anything like that
	void makeShop( void );
	void updateHealth( void );
	void restock(); // Restocks this Vendor
	void action( UINT8 id ); // Do an action
	P_ITEM getWeapon() const;
	QString fullName( void );
	UINT16 bestSkill();
	QString reputationTitle();

	Coord_cl nextMove( void );

	// Getters
	short					guildType() const;    // (0) Standard guild, (1) Chaos Guild, (2) Order guild
	bool					guildTraitor() const; // (true) This character converted, (false) Neve converted, or not an order/chaos guild member
	QString					orgname() const;	  //original name - for Incognito
	QString					title() const;
	bool					sex() const; // false/0 - male
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
	bool					tamed() const {return tamed_;}
    unsigned int			smoketimer() const {return smoketimer_;}
	unsigned int			smokedisplaytimer() const {return smokedisplaytimer_;}
	unsigned int			antispamtimer() const { return antispamtimer_;}
	unsigned int			antiguardstimer() const { return antiguardstimer_;}
	P_CHAR					guarding() const { return guarding_; }
	QString					carve() const {return carve_;}
	SERIAL					hairserial() const {return hairserial_;}
	SERIAL					beardserial() const { return beardserial_;}
	unsigned int			begging_timer() const {return begging_timer_;}
	int						postType() const {return postType_;}
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
	int						hunger() const { return hunger_; }
	unsigned int			hungertime() const { return hungertime_;}
	SERIAL					tailitem() const { return tailitem_; }
	int						npcaitype() const { return npcaitype_;}
	int						callnum() const { return callnum_; }
	int						playercallnum() const { return playercallnum_; }
	int						poison() const { return poison_;}
	unsigned int			poisoned() const { return poisoned_; }
	unsigned int			poisontime() const { return poisontime_;}
	unsigned int			poisontxt() const { return poisontxt_;}
	unsigned int			poisonwearofftime() const { return poisonwearofftime_;}
	short					fleeat() const { return fleeat_;}
	short					reattackat() const { return reattackat_; }
	unsigned short			envokeid() const { return envokeid_;}
	SERIAL					envokeitem() const { return envokeitem_;}
	unsigned char			split() const { return split_;}
	unsigned char			splitchnc() const { return splitchnc_;}
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
	cUOSocket*				socket() const { return socket_; }
	unsigned short			weight() const { return weight_; }
	unsigned short			stones() const { return (weight_ / 10); }
	QString					lootList() const { return loot_; }
	UINT32					trackingTimer() const { return trackingTimer_; }
	unsigned char			fonttype() const { return fonttype_; }
	UI16					saycolor() const { return saycolor_; }
	unsigned short			emotecolor() const { return emotecolor_; }
	signed short			st() const { return st_; }
	signed short			st2() const { return st2_; }
	bool					may_levitate() const { return may_levitate_; }
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
	int						robe() const { return robe_; }
	int						karma() const { return karma_; }
	signed int				fame() const { return fame_; }
	unsigned int			kills() const { return kills_; }
	unsigned int			deaths() const { return deaths_; }
	bool					dead() const { return dead_; }
	unsigned char			fixedlight() const { return fixedlight_; }
	unsigned int			def() const { return def_; }
	bool					war() const { return war_; }
	SERIAL					targ() const { return targ_; }
	unsigned int			timeout() const { return timeout_; }
	unsigned int			regen() const { return regen_; }
	unsigned int		    regen2() const { return regen2_; }
	unsigned int			regen3() const { return regen3_; }
	enInputMode				inputmode() const { return inputmode_; }
	SERIAL					inputitem() const { return inputitem_; }
	SERIAL					attacker() const { return attacker_; }
	unsigned int			npcmovetime() const { return npcmovetime_; }
	unsigned char			npcWander() const { return npcWander_; }
	unsigned char			oldnpcWander() const { return oldnpcWander_; }
	SERIAL					ftarg() const { return ftarg_; }
	Coord_cl				ptarg() const { return ptarg_; }
	int						fx1() const { return fx1_; }
	int						fx2() const { return fx2_; }
	int						fy1() const { return fy1_; }
	int						fy2() const { return fy2_; }
	signed char				fz1() const { return fz1_; }
	cTerritory*				region() const { return region_; }
	unsigned int			skilldelay() const { return skilldelay_; }
	unsigned int			objectdelay() const { return objectdelay_; }
	int						making() const { return making_; }
	SERIAL					lastTarget() const { return lastTarget_; }
	char					blocked() const { return blocked_; }
	char					dir2() const { return dir2_; }
	unsigned int			spiritspeaktimer() const { return spiritspeaktimer_; }
	int						spattack() const { return spattack_; }
	int						spadelay() const { return spadelay_; }
	unsigned int			spatimer() const { return spatimer_; }
	int						taming() const { return taming_; }
	unsigned int			summontimer() const { return summontimer_; }
	int						VisRange() const { return VisRange_; }
	QPtrList< cMakeSection > lastSelections( cMakeMenu* basemenu );
	cMakeSection*			lastSection( cMakeMenu* basemenu );
	unsigned int			food() const { return food_; }
	P_CHAR					owner() const { return owner_; }
	QString					profile() const { return profile_; }
	UINT16					skillValue( UINT16 skill ) const;
	UINT16					skillCap( UINT16 skill ) const;
	UINT8					skillLock( UINT16 skill ) const;
	
	// Setters
	void					setGuildType(short d);
	void					setGuildTraitor(bool  d);
	void					setOrgname(const QString& d);//original name - for Incognito
	void					setTitle( const QString& d);
	void					setSex( bool female );
	void					setUnicode( bool d); // This is set to 1 if the player uses unicode speech, 0 if not
	void					setAccount( AccountRecord* data, bool moveFromAccToAcc = true ); // changed to signed, lb
	void					setIncognito ( bool d) { incognito_ = d; changed( SAVE );} 
	void					setPolymorph ( bool d) { polymorph_ = d; changed( SAVE );}
	void					setHairColor ( unsigned short d);
	void					setHairStyle ( unsigned short d);
	void					setBeardColor( unsigned short d);
	void					setBeardStyle( unsigned short d);
	void					setSkin( unsigned short d) { skin_ = d; changed( SAVE );}
	void					setOrgSkin( unsigned short d) { orgskin_ = d; changed( SAVE );}
	void					setXSkin( unsigned short d) { xskin_ = d; changed( SAVE );}
	void					setCreationDay( unsigned int d ) { creationday_ = d; changed( SAVE );}
	void					setStealth(int d) {stealth_ = d; changed( SAVE );}
	void					setRunning(unsigned int d) {running_ = d; changed( SAVE );}
	void					setLogout(unsigned int d) {logout_ = d; changed( SAVE );}
	void					setClientIdleTime( unsigned int d ) { clientidletime_ = d; changed( SAVE );}
	void					setSwingTarg( SERIAL d ) { swingtarg_ = d; changed( SAVE );}
	void					setHoldg( unsigned int d ) {holdg_ = d; changed( SAVE );}
	void					setFlySteps( unsigned char d) {fly_steps_ = d; changed( SAVE );}
	void					setTamed( bool d ) { tamed_ = d; changed( SAVE+TOOLTIP );}
    void					setSmokeTimer( unsigned int d ) { smoketimer_ = d; changed( SAVE );}
	void					setSmokeDisplayTimer ( unsigned int d ) { smokedisplaytimer_ = d; changed( SAVE );}
	void					setAntispamtimer ( unsigned int d ) { antispamtimer_ = d; changed( SAVE );}
	void					setAntiguardstimer( unsigned int d ) { antiguardstimer_ = d; changed( SAVE );}
	void					setCarve( const QString& d ) { carve_ = d; changed( SAVE );}
	void					setHairSerial( SERIAL d ) { hairserial_ = d; changed( SAVE );}
	void					setBeardSerial( SERIAL d ) {beardserial_ = d; changed( SAVE );}
	void 					setBegging_timer( unsigned int d ) { begging_timer_ = d; changed( SAVE );}
	void					setPostType( int d ) { postType_ = d; changed( SAVE );}
	void					setMurdererSer( SERIAL d ) { murdererSer_ = d; changed( SAVE );}
	void					setPrevPos( const Coord_cl& d ) { prevPos_ = d; changed( SAVE );}
	void					setCommandLevel( unsigned char d ) { commandLevel_ = d; changed( SAVE );}
	void					setSpawnregion ( QString d ) { spawnregion_ = d; changed( SAVE );}
	void					setStablemaster_serial (SERIAL d ) { stablemaster_serial_ = d; changed( SAVE );}  
	void					setNpc_type( unsigned char d ) { npc_type_ = d; changed( SAVE );}
	void					setTime_unused ( unsigned int d ) { time_unused_ = d; changed( SAVE );}
	void					setTimeused_last( unsigned int d ) { timeused_last_ = d; changed( SAVE );}
	void					setCasting( bool d ) { casting_ = d; changed( SAVE );}
	void					setSpawnSerial( SERIAL d ) { spawnserial_ = d; changed( SAVE );}
	void					setHidden ( unsigned char d ) { hidden_ = d; changed( SAVE );}
	void					setInvisTimeout ( unsigned int d ) { invistimeout_ = d; changed( SAVE );}
	void					setAttackFirst ( bool d ) { attackfirst_ = d; changed( SAVE );}
	void					setHunger ( int d ) { hunger_ = d; changed( SAVE );}
	void					setHungerTime ( unsigned int d ) { hungertime_ = d; changed( SAVE );}
	void					setTailItem ( SERIAL d ) { tailitem_ = d; changed( SAVE );}
	void					setNpcAIType( int d ) { npcaitype_ = d; changed( SAVE );}
	void					setCallNum ( int d ) { callnum_ = d; changed( SAVE );}
	void					setPlayerCallNum ( int d ) { playercallnum_ = d; changed( SAVE );}

	void					setPoison( int d ) { poison_ = d; changed( SAVE );}
	void					setPoisoned( unsigned int d ) {poisoned_ = d; changed( SAVE );}
	void					setPoisontime( unsigned int d ) { poisontime_ = d; changed( SAVE );}
	void					setPoisontxt( unsigned int d ) { poisontxt_ = d; changed( SAVE );}
	void					setPoisonwearofftime( unsigned int d ) {poisonwearofftime_ = d; changed( SAVE );}
	void					setFleeat( short d ) { fleeat_ = d; changed( SAVE );}
	void					setReattackat(short d) { reattackat_ = d; changed( SAVE );}
	void					setEnvokeid( unsigned short d ) { envokeid_ = d; changed( SAVE );}
	void					setEnvokeitem( SERIAL d ) { envokeitem_ = d; changed( SAVE );}
	void					setSplit(unsigned char d) {split_ = d; changed( SAVE );}
	void					setSplitchnc(unsigned char d) {splitchnc_ = d; changed( SAVE );}
	void					setRa( char d ) { ra_ = d; changed( SAVE );}
	void					setTrainer( SERIAL d ) { trainer_ = d; changed( SAVE );}
	void					setTrainingplayerin( char d ) { trainingplayerin_ = d; changed( SAVE );}
	void					setCantrain( bool d ) { cantrain_ = d; changed( SAVE );}
	void					setGuildtoggle( bool d ) { guildtoggle_ = d; changed( SAVE );}
	void					setGuildtitle( const QString& d ) { guildtitle_ = d; changed( SAVE );}
	void					setGuildfealty( SERIAL d ) { guildfealty_ = d; changed( SAVE );}
	void					setGuildstone( SERIAL d ) { guildstone_ = d; changed( SAVE );}
	void					setFlag( char d ) { flag_ = d; changed( SAVE );}
	void					setTempflagtime( unsigned int d ) { tempflagtime_ = d; changed( SAVE );}
	void					setMurderrate( unsigned int d ) { murderrate_ = d; changed( SAVE );}
	void					setCrimflag( unsigned int d ) { crimflag_ = d; changed( SAVE );}
	void					setSpelltime( unsigned int d ) { spelltime_ = d; changed( SAVE );}
	void					setSpell( int d ) { spell_ = d; changed( SAVE );}
	void					setSpellaction( int d ) { spellaction_ = d; changed( SAVE );} 
	void					setNextact( int d ) { nextact_ = d; changed( SAVE );}
	void					setPoisonserial( SERIAL d ) {poisonserial_ = d; changed( SAVE );}
	void					setSquelched( int d) { squelched_ = d; changed( SAVE );}
	void					setMutetime( int d ) { mutetime_ = d; changed( SAVE );}
	void					setMed( bool d ) { med_ = d; changed( SAVE );}
	void					setSocket( cUOSocket* d ) { socket_ = d; changed( SAVE );}
	void					setWeight( unsigned short d ) { weight_ = d; changed( SAVE );}
	void					setLootList( QString d ) { loot_ = d; changed( SAVE );}
	void					setTrackingTarg( SERIAL d ) { trackingTarget_ = d; changed( SAVE );}
	void					setTrackingTimer( UINT32 d ) { trackingTimer_ = d; changed( SAVE );}
	void					setFontType( unsigned char d ) { fonttype_ = d; changed( SAVE );}
	void					setSayColor( UI16 d ) { saycolor_ = d; changed( SAVE );}
	void					setEmoteColor( unsigned short d ) { emotecolor_ = d; changed( SAVE );}
	void					setSt( signed short d ) { st_ = d; changed( SAVE );}
	void					setSt2( signed short d ) { st2_ = d; changed( SAVE );}
	void					setMay_Levitate( bool d ) { may_levitate_ = d; changed( SAVE );}
	void					setDispz( signed char d ) { dispz_ = d; changed( SAVE );}
	void					setDir( unsigned char d ) { dir_ = d; changed( SAVE );}
	void					setXid( unsigned short d ) { xid_ = d; changed( SAVE );}
	void					setPriv2( unsigned char d ) { priv2_ = d; changed( SAVE+TOOLTIP );}
	void					setIn( signed short d ) { in_ = d; changed( SAVE );}
	void					setIn2( signed short d ) { in2_ = d; changed( SAVE );}
	void					setHp( signed short d ) { hp_ = d; changed( SAVE );}
	void					setStm( signed short d ) { stm_ = d; changed( SAVE );}
	void					setMn( signed short d ) { mn_ = d; changed( SAVE );}
	void					setMn2( signed short d ) { mn2_ = d; changed( SAVE );}
	void					setHiDamage( int d ) { hidamage_ = d; changed( SAVE );}
	void					setLoDamage( int d ) { lodamage_ = d; changed( SAVE );}
	void					setNpc( bool d ) { npc_ = d; changed( SAVE );}
	void					setShop( bool d ) { shop_ = d; changed( SAVE );}
	void					setCell( unsigned char d ) { cell_ = d; changed( SAVE );}
	void					setJailTimer( unsigned int d ) { jailtimer_ = d; changed( SAVE );}
	void					setJailSecs( int d ) { jailsecs_ = d; changed( SAVE );}
	void					setLastSection( cMakeMenu* basemenu, cMakeSection* d );
	void					setRobe( int d ) { robe_ = d; changed( SAVE );}
	void					setKarma( int d ) { karma_ = d; changed( SAVE );}
	void					setFame( signed int d ) { fame_ = d; changed( SAVE );}
	void					setKills( unsigned int d ) { kills_ = d; changed( SAVE );}
	void					setDeaths( unsigned int d ) { deaths_ = d; changed( SAVE );}
	void					setDead( bool d ) { dead_ = d; changed( SAVE+TOOLTIP );}
	void					setFixedLight( unsigned char d ) { fixedlight_ = d; changed( SAVE );}
	void					setDef( unsigned int d ) { def_ = d; changed( SAVE );}
	void					setWar( bool d ) { war_ = d; changed( SAVE );}
	void					setFood( unsigned int d ) { food_ = d; changed( SAVE );}
	void					setTarg( SERIAL d ) { targ_ = d; changed( SAVE );}
	void					setTimeOut( unsigned int d ) { timeout_ = d; changed( SAVE );}
	void					setRegen( unsigned int d ) { regen_ = d; changed( SAVE );}
	void					setRegen2( unsigned int d ) { regen2_ = d; changed( SAVE );}  
	void					setRegen3( unsigned int d ) { regen3_ = d; changed( SAVE );}
	void					setInputMode( enInputMode d ) { inputmode_ = d; changed( SAVE );}
	void					setInputItem( SERIAL d ) { inputitem_ = d; changed( SAVE );}
	void					setAttacker( SERIAL d ) { attacker_ = d; changed( SAVE );}
	void					setNpcMoveTime( unsigned int d ) { npcmovetime_ = d; changed( SAVE );}
	void					setNpcWander( unsigned char d ) { npcWander_ = d; changed( SAVE );}
	void					setOldNpcWander( unsigned char d ) { oldnpcWander_ = d; changed( SAVE );}
	void					setFtarg( SERIAL d ) { ftarg_ = d; changed( SAVE );}
	void					setPtarg( Coord_cl d ) { ptarg_ = d; changed( SAVE );}
	void					setFx1( int d ) { fx1_ = d; changed( SAVE );}
	void					setFx2( int d ) { fx2_ = d; changed( SAVE );}
	void					setFy1( int d ) { fy1_ = d; changed( SAVE );}
	void					setFy2( int d ) { fy2_ = d; changed( SAVE );}
	void					setFz1( signed char d ) { fz1_ = d; changed( SAVE );}
	void					setRegion( cTerritory* d ) { region_ = d; changed( SAVE );}
	void					setSkillDelay( unsigned int d ) { skilldelay_ = d; changed( SAVE );}
	void					setObjectDelay( unsigned int d ) { objectdelay_ = d; changed( SAVE );}
	void					setMaking( int d ) { making_ = d; changed( SAVE );}
	void					setLastTarget( SERIAL d ) { lastTarget_ = d; changed( SAVE );}
	void					setBlocked( char d ) { blocked_ = d; changed( SAVE );}
	void					setDir2( char d ) { dir2_ = d; changed( SAVE );}
	void					setSpiritSpeakTimer( unsigned int d ) { spiritspeaktimer_ = d; changed( SAVE );}
	void					setSpAttack( int d ) { spattack_ = d; changed( SAVE );}
	void					setSpaDelay( int d ) { spadelay_ = d; changed( SAVE );}
	void					setSpaTimer( unsigned int d ) { spatimer_ = d; changed( SAVE );}
	void					setTaming( int d ) { taming_ = d; changed( SAVE );}
	void					setSummonTimer( unsigned int d ) { summontimer_ = d; changed( SAVE );}
	void					setVisRange( int d ) { VisRange_ = d; changed( SAVE );}
	void					setProfile( const QString &d ) { profile_ = d; changed( SAVE );}
	void					clearLastSelections( void );

	void					setSkillValue( UINT16 skill, UINT16 value );
	void					setSkillCap( UINT16 skill, UINT16 cap );
	void					setSkillLock( UINT16 skill, UINT8 lock );

	void					pushMove( const Coord_cl &move );
	void					pushMove( UI16 x, UI16 y, SI08 z );
	void					popMove( void );
	void					clearPath( void );
	bool					hasPath( void );
	Coord_cl				pathDestination( void ) const;
	float					pathHeuristic( const Coord_cl &source, const Coord_cl &destination );
	void					findPath( const Coord_cl &goal, float sufficient_cost );

	UINT8 notority( P_CHAR pChar ); // Gets the notority toward another char
	void kill();
	void playDeathSound();
	void resurrect();
	void turnTo( cUObject *object );
	void turnTo( const Coord_cl &pos );
	void callGuards();
	P_CHAR unmount();
	void mount( P_CHAR pMount );
	bool	canMoveAll( void ) const { return priv2_&0x01; }
	bool	isFrozen( void ) const { return priv2_&0x02; }	
	bool	viewHouseIcons( void ) const { return priv2_&0x04; }	
	bool	isHiddenPermanently( void ) const { return priv2_&0x08; }
	void wear( P_ITEM );
	void updateWornItems();
	void updateWornItems( cUOSocket* );
	short effDex()				{return dx+tmpDex>0 ? dx+tmpDex : 0;}	// returns current effective Dexterity
	short realDex()				{return dx;}	// returns the true Dexterity
	short decDex()				{return dx2;}	// returns the 3 digits behind the decimal point
	void  setDex(signed short val)		{dx = val; changed( SAVE );}		// set the true Dex
	void  setDecDex(signed short val)	{dx2 = val; changed( SAVE );}	// set the 3 digits
	void  chgDex(signed short val)		{tmpDex += val; changed( SAVE );}// intended for temporary changes of Dex
	void  chgRealDex(short val) {dx += val;if(dx<1) dx=1;if(dx>100) dx=100; changed( SAVE );}	// intended for permanent changes of Dex
	bool  incDecDex(short val)	{dx2 += val;changed( SAVE );
	if (dx2>1000) {dx2-=1000;chgRealDex(1);return true;}
	else return false;}
	unsigned short id() const				{return id_;}
	void setId(unsigned short d)			{id_ = d; changed( SAVE );}
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
	bool  showSkillTitles() const;
	bool  isInnocent()	const;
	bool  isMurderer()	const;
	bool  isCriminal()	const;
	unsigned char getPriv() const;
	void setPriv(unsigned char p);
	void setShowSkillTitles( bool data );
	void showName( cUOSocket *socket );
	void makeInvulnerable();
	void makeVulnerable();
	void setMurderer();
	void setInnocent();
	void criminal();
	void setAttackFirst()		{this->attackfirst_ = true; changed( SAVE );}
	void resetAttackFirst()		{this->attackfirst_ = false; changed( SAVE );}
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
	int  CountBankGold();
	bool hasWeapon();
	bool hasShield();
	P_ITEM getBackpack();
	void SetSpawnSerial(long spawnser);
	void SetMultiSerial(long mulser);
	void setSerial(SERIAL ser);
	void MoveTo(short newx, short newy, signed char newz);
	void MoveToXY(short newx, short newy);
	bool Wears(P_ITEM pi);
	unsigned int getSkillSum();
	int getTeachingDelta(cChar* pPlayer, int skill, int sum);
	void removeItemBonus(cItem* pi);
	void giveItemBonus(cItem* pi);
	void Init(bool ser = true);
	bool isSameAs(cChar* pc) {if (!pc || pc->serial() != serial()) return false; else return true;}
	bool inGuardedArea();
	bool canPickUp(cItem* pi);
	void soundEffect( UI16 soundId, bool hearAll = true );
	void giveGold( Q_UINT32 amount, bool inBank = false );
	UINT32 takeGold( UINT32 amount, bool useBank = false );
	void emote( const QString &emote, UI16 color = 0xFFFF );
	void message( const QString &message, UI16 color = 0x3B2 );
	bool online() const;

	// Wrapper events! - darkstorm
	virtual bool onSingleClick( P_CHAR Viewer ); // Shows the name of a character to someone else
	virtual bool onWalk( UI08 Direction, UI08 Sequence ); // Walks in a specific Direction

	virtual bool onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ); // The character says something
	virtual bool onWarModeToggle( bool War ); // The character switches warmode
	virtual bool onLogin( void ); // The character enters the world
	virtual bool onLogout( void ); // The character enters the world
	virtual bool onShowPaperdoll( P_CHAR pOrigin ); // The paperdoll of this character is requested, there is no vice-versa call
	
	virtual bool onHelp( void ); // The character wants help
	virtual bool onChat( void ); // The character wants to chat
	virtual bool onSkillUse( UI08 Skill ); // The character uses %Skill
	virtual bool onCollideChar( P_CHAR Obstacle ); // This is called for the walking character first, then for the character walked on
	virtual bool onShowContext( cUObject *object ); // Shows a context menu for a specific item
	virtual bool onShowTooltip( P_CHAR sender, cUOTxTooltipList* tooltip ); // Shows a tool tip for specific object
	bool onDropOnChar( P_ITEM pItem );
	bool onPickup( P_ITEM pItem );
	QString onShowPaperdollName( P_CHAR pOrigin ); // only change the viewed name
	
	virtual void talk( const QString &message, UI16 color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	void giveNewbieItems( Q_UINT8 skill = 0xFF );

	void attackTarget( P_CHAR defender );
	void applyPoison( P_CHAR defender );
	void toggleCombat();
	UI16 calcDefense( enBodyParts bodypart, bool wearout = false );

	P_ITEM rightHandItem() const;
	P_ITEM leftHandItem() const;

	bool checkSkill( UI16 skill, SI32 min, SI32 max, bool advance = true );
	void setSkillDelay();
	void startRepeatedAction( UINT8 action, UINT16 delay );
	void stopRepeatedAction();

	// Equipment system
	void addItem( enLayer layer, cItem*, bool handleWeight = true, bool noRemove = false );
	void removeItem( enLayer layer, bool handleWeight = true );
	ContainerContent content() const;
	cItem* atLayer( enLayer layer ) const;

	// Follower/Owner system
	void setOwner( P_CHAR owner );
	void setOwnerOnly( P_CHAR d ) { owner_ = d; }
	void addFollower( P_CHAR pPet, bool noOwnerChange = false );	
	void removeFollower( P_CHAR pPet, bool noOwnerChange = false );
	Followers followers() const;

	void setGuarding( P_CHAR data );
	void setGuardingOnly( P_CHAR d ) { guarding_ = d; };
	void addGuard( P_CHAR pPet, bool noGuardingChange = false );
	void removeGuard( P_CHAR pPet, bool noGuardingChange = false );
	Followers guardedby() const;
	
	bool Owns( P_ITEM pi ) const;

	// Effect System
	void addEffect( cTempEffect *effect );
	void removeEffect( cTempEffect *effect );
	Effects effects() const;

	// Simple Property setting and getting.
	stError *setProperty( const QString &name, const cVariant &value );
	stError *getProperty( const QString &name, cVariant &value ) const;

/////
	void flagUnchanged() { changed_ = false; cUObject::flagUnchanged(); }

	// Definition loading - sereg
protected:
	virtual void processNode( const QDomElement& Tag );
	void applyStartItemDefinition( const QDomElement &Tag );
};

namespace cCharStuff
{
	void DeleteChar(P_CHAR pc_k);
	P_CHAR MemCharFree();
	void Split(P_CHAR pc_k);
	void CheckAI(unsigned int currenttime, P_CHAR pc_i);
	P_CHAR createScriptNpc( const QString &section, const Coord_cl &pos );
};


// Inline Member functions
inline bool  cChar::isPlayer() const		{return (!this->npc_);}
inline bool  cChar::isNpc()	const			{return (this->npc_);}
inline bool  cChar::isHuman() const			{return (this->id() == 0x190 || this->id() == 0x191);} 
inline bool  cChar::isTrueGM() const		{return (priv&0x01);} 
inline bool  cChar::isInvul() const			{return (priv&0x04 ?true:false);}
inline bool  cChar::canSnoop() const		{return (priv&0x40 ?true:false);}
inline bool  cChar::canBroadcast() const	{return (priv&0x02 ?true:false);}
inline bool  cChar::canSeeSerials() const 	{return (priv&0x08 ?true:false);}
inline bool  cChar::showSkillTitles() const { return (priv&0x10 ?true:false);}
inline void  cChar::setShowSkillTitles( bool data ) { data ? priv |= 0x10 : priv &= 0xEF; }

inline bool  cChar::isInnocent() const		{return (flag_&0x04 ?true:false);}
inline bool  cChar::isMurderer() const		{return (flag_&0x01 ?true:false);}
inline bool  cChar::isCriminal() const		{return (flag_&0x02 ?true:false);}
inline unsigned char cChar::getPriv() const	{return priv;}
inline void cChar::setPriv(unsigned char p)	{this->priv=p; changed( SAVE );}
inline void cChar::makeInvulnerable()		{priv |= 4; changed( SAVE );}
inline void cChar::makeVulnerable()			{priv &= 0xFB; changed( SAVE );}
inline void cChar::setMurderer()			{flag_ = 0x01; changed( SAVE );}
inline void cChar::setInnocent()			{flag_ = 0x04; changed( SAVE );}


// Getters
inline short			cChar::guildType() const		{ return GuildType; }
inline bool				cChar::guildTraitor() const		{ return GuildTraitor; }
inline QString			cChar::orgname() const			{ return orgname_; }
inline QString			cChar::title() const			{ return title_;   }
inline bool				cChar::unicode() const			{ return unicode_; }
inline AccountRecord*	cChar::account() const			{ return account_; }
inline cChar::Effects	cChar::effects() const			{ return effects_; }
inline bool				cChar::sex() const				{ return sex_; }

// Setters
inline void	cChar::setGuildType(short d)			{ GuildType = d; changed( SAVE ); }
inline void cChar::setGuildTraitor(bool  d)			{ GuildTraitor = d; changed( SAVE );}
inline void	cChar::setOrgname( const QString& d )	{ orgname_ = d; changed( SAVE );}
inline void cChar::setTitle( const QString& d )		{ title_ = d;   changed( SAVE );}
inline void cChar::setUnicode(bool d)				{ unicode_ = d; changed( SAVE );}
inline void cChar::setSex( bool female )				{ sex_ = female; changed( SAVE ); }

#endif // __CHARS_H__
