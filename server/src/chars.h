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
#include "makemenus.h"

// Library Includes
#include <qmap.h>
#include <qptrlist.h>
#include <qvaluevector.h>

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

	bool changed_;

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
	int						hunger_;  // Level of hungerness, 6 = full, 0 = "empty"
	unsigned int			hungertime_; // Timer used for hunger, one point is dropped every 20 min
	SERIAL					tailitem_;
	int						npcaitype_; // NPC ai
	int						callnum_; //GM Paging
	int						playercallnum_; //GM Paging

	unsigned int			fishingtimer_; // Timer used to delay the catching of fish
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
	unsigned char			fonttype_; // Speech font to use
	UI16					saycolor_; // Color for say messages
	unsigned short			emotecolor_; // Color for emote messages
	signed short			st_; // Strength
	signed short			st2_; // Reserved for calculation
	bool					may_levitate_;
	unsigned char			pathnum_;
	path_st					path_[PATHNUM];
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
	unsigned char			speech_; // For NPCs: Number of the assigned speech block
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
	unsigned char			lockSkill_[ALLSKILLS+1]; // LB, client 1.26.2b skill managment
	UINT8					VisRange_;
	QString					profile_;

	QMap< cMakeMenu*, QPtrList< cMakeSection > >	lastselections_;
	unsigned int			food_;

	// Public Methods
public:
	cChar();
	cChar( const P_CHAR );
	virtual QString objectID() const;
	static void registerInFactory();
	void load( char **, UINT16& );
	void save();
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
	P_ITEM getWeapon();
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
	int						hunger() const { return hunger_; }
	unsigned int			hungertime() const { return hungertime_;}
	SERIAL					tailitem() const { return tailitem_; }
	int						npcaitype() const { return npcaitype_;}
	int						callnum() const { return callnum_; }
	int						playercallnum() const { return playercallnum_; }
	unsigned int			fishingtimer() const {return fishingtimer_;}
	int						poison() const { return poison_;}
	unsigned int			poisoned() const { return poisoned_; }
	unsigned int			poisontime() const { return poisontime_;}
	unsigned int			poisontxt() const { return poisontxt_;}
	unsigned int			poisonwearofftime() const { return poisonwearofftime_;}
	short					fleeat() const { return fleeat_;}
	short					reattackat() const { return reattackat_; }
	unsigned int			disabled() const { return disabled_; }
	QString					disabledmsg() const { return disabledmsg_; }
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
	unsigned short			baseSkill( int v ) const { return baseSkill_[v]; }
	unsigned short			skill( int v ) const {return skill_[v];} // List of skills (with stat modifiers)
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
	int						robe() const { return robe_; }
	int						karma() const { return karma_; }
	signed int				fame() const { return fame_; }
	unsigned int			kills() const { return kills_; }
	unsigned int			deaths() const { return deaths_; }
	bool					dead() const { return dead_; }
	unsigned char			fixedlight() const { return fixedlight_; }
	unsigned char			speech() const { return speech_; }
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
	unsigned char			lockSkill( int d ) const { return lockSkill_[d]; }// LB, client 1.26.2b skill managment
	int						VisRange() const { return VisRange_; }
	QPtrList< cMakeSection > lastSelections( cMakeMenu* basemenu );
	cMakeSection*			lastSection( cMakeMenu* basemenu );
	unsigned int			food() const { return food_; }
	P_CHAR					owner() const { return owner_; }
	QString					profile() const { return profile_; }
	
	// Setters
	void					setGuildType(short d);
	void					setGuildTraitor(bool  d);
	void					setOrgname(const QString& d);//original name - for Incognito
	void					setTitle( const QString& d);
	void					setUnicode( bool d); // This is set to 1 if the player uses unicode speech, 0 if not
	void					setAccount( AccountRecord* data, bool moveFromAccToAcc = true ); // changed to signed, lb
	void					setIncognito ( bool d) { incognito_ = d; changed_ = true;} 
	void					setPolymorph ( bool d) { polymorph_ = d; changed_ = true;}
	void					setHairColor ( unsigned short d) { haircolor_ = d; changed_ = true;}
	void					setHairStyle ( unsigned short d) { hairstyle_ = d; changed_ = true;}
	void					setBeardColor( unsigned short d) { beardcolor_ = d; changed_ = true;}
	void					setBeardStyle( unsigned short d) { beardstyle_ = d; changed_ = true;}
	void					setSkin( unsigned short d) { skin_ = d; changed_ = true;}
	void					setOrgSkin( unsigned short d) { orgskin_ = d; changed_ = true;}
	void					setXSkin( unsigned short d) { xskin_ = d; changed_ = true;}
	void					setCreationDay( unsigned int d ) { creationday_ = d; changed_ = true;}
	void					setStealth(int d) {stealth_ = d; changed_ = true;}
	void					setRunning(unsigned int d) {running_ = d; changed_ = true;}
	void					setLogout(unsigned int d) {logout_ = d; changed_ = true;}
	void					setClientIdleTime( unsigned int d ) { clientidletime_ = d; changed_ = true;}
	void					setSwingTarg( SERIAL d ) { swingtarg_ = d; changed_ = true;}
	void					setHoldg( unsigned int d ) {holdg_ = d; changed_ = true;}
	void					setFlySteps( unsigned char d) {fly_steps_ = d; changed_ = true;}
	void					setMenupriv(int d) { menupriv_ = d; changed_ = true;}
	void					setTamed( bool d ) { tamed_ = d; changed_ = true;}
    void					setSmokeTimer( unsigned int d ) { smoketimer_ = d; changed_ = true;}
	void					setSmokeDisplayTimer ( unsigned int d ) { smokedisplaytimer_ = d; changed_ = true;}
	void					setAntispamtimer ( unsigned int d ) { antispamtimer_ = d; changed_ = true;}
	void					setAntiguardstimer( unsigned int d ) { antiguardstimer_ = d; changed_ = true;}
	void					setCarve( const QString& d ) { carve_ = d; changed_ = true;}
	void					setHairSerial( SERIAL d ) { hairserial_ = d; changed_ = true;}
	void					setBeardSerial( SERIAL d ) {beardserial_ = d; changed_ = true;}
	void 					setBegging_timer( unsigned int d ) { begging_timer_ = d; changed_ = true;}
	void					setPostType( int d ) { postType_ = d; changed_ = true;}
	void					setQuestType ( int d ) { questType_ = d; changed_ = true;}
	void					setQuestDestRegion( int d ) { questDestRegion_ = d; changed_ = true;}
	void					setQuestOrigRegion( int d ) {questOrigRegion_ = d; changed_ = true;}
	void					setQuestBountyReward ( int d ) { questBountyReward_ = d; changed_ = true;}
	void					setQuestBountyPostSerial ( int d ) {questBountyPostSerial_ = d; changed_ = true;}
	void					setMurdererSer( SERIAL d ) { murdererSer_ = d; changed_ = true;}
	void					setPrevPos( const Coord_cl& d ) { prevPos_ = d; changed_ = true;}
	void					setCommandLevel( unsigned char d ) { commandLevel_ = d; changed_ = true;}
	void					setSpawnregion ( QString d ) { spawnregion_ = d; changed_ = true;}
	void					setStablemaster_serial (SERIAL d ) { stablemaster_serial_ = d; changed_ = true;}  
	void					setNpc_type( unsigned char d ) { npc_type_ = d; changed_ = true;}
	void					setTime_unused ( unsigned int d ) { time_unused_ = d; changed_ = true;}
	void					setTimeused_last( unsigned int d ) { timeused_last_ = d; changed_ = true;}
	void					setCasting( bool d ) { casting_ = d; changed_ = true;}
	void					setSpawnSerial( SERIAL d ) { spawnserial_ = d; changed_ = true;}
	void					setHidden ( unsigned char d ) { hidden_ = d; changed_ = true;}
	void					setInvisTimeout ( unsigned int d ) { invistimeout_ = d; changed_ = true;}
	void					setAttackFirst ( bool d ) { attackfirst_ = d; changed_ = true;}
	void					setHunger ( int d ) { hunger_ = d; changed_ = true;}
	void					setHungerTime ( unsigned int d ) { hungertime_ = d; changed_ = true;}
	void					setTailItem ( SERIAL d ) { tailitem_ = d; changed_ = true;}
	void					setNpcAIType( int d ) { npcaitype_ = d; changed_ = true;}
	void					setCallNum ( int d ) { callnum_ = d; changed_ = true;}
	void					setPlayerCallNum ( int d ) { playercallnum_ = d; changed_ = true;}

	void					setFishingtimer( unsigned int d ) { fishingtimer_ = d; changed_ = true;}
	void					setPoison( int d ) { poison_ = d; changed_ = true;}
	void					setPoisoned( unsigned int d ) {poisoned_ = d; changed_ = true;}
	void					setPoisontime( unsigned int d ) { poisontime_ = d; changed_ = true;}
	void					setPoisontxt( unsigned int d ) { poisontxt_ = d; changed_ = true;}
	void					setPoisonwearofftime( unsigned int d ) {poisonwearofftime_ = d; changed_ = true;}
	void					setFleeat( short d ) { fleeat_ = d; changed_ = true;}
	void					setReattackat(short d) { reattackat_ = d; changed_ = true;}
	void					setDisabled( unsigned int d ) { disabled_ = d; changed_ = true;}
	void					setDisabledmsg( const QString& d ) { disabledmsg_ = d; changed_ = true;}
	void					setEnvokeid( unsigned short d ) { envokeid_ = d; changed_ = true;}
	void					setEnvokeitem( SERIAL d ) { envokeitem_ = d; changed_ = true;}
	void					setSplit(unsigned char d) {split_ = d; changed_ = true;}
	void					setSplitchnc(unsigned char d) {splitchnc_ = d; changed_ = true;}
	void					setRa( char d ) { ra_ = d; changed_ = true;}
	void					setTrainer( SERIAL d ) { trainer_ = d; changed_ = true;}
	void					setTrainingplayerin( char d ) { trainingplayerin_ = d; changed_ = true;}
	void					setCantrain( bool d ) { cantrain_ = d; changed_ = true;}
	void					setGuildtoggle( bool d ) { guildtoggle_ = d; changed_ = true;}
	void					setGuildtitle( const QString& d ) { guildtitle_ = d; changed_ = true;}
	void					setGuildfealty( SERIAL d ) { guildfealty_ = d; changed_ = true;}
	void					setGuildstone( SERIAL d ) { guildstone_ = d; changed_ = true;}
	void					setFlag( char d ) { flag_ = d; changed_ = true;}
	void					setTempflagtime( unsigned int d ) { tempflagtime_ = d; changed_ = true;}
	void					setMurderrate( unsigned int d ) { murderrate_ = d; changed_ = true;}
	void					setCrimflag( unsigned int d ) { crimflag_ = d; changed_ = true;}
	void					setSpelltime( unsigned int d ) { spelltime_ = d; changed_ = true;}
	void					setSpell( int d ) { spell_ = d; changed_ = true;}
	void					setSpellaction( int d ) { spellaction_ = d; changed_ = true;} 
	void					setNextact( int d ) { nextact_ = d; changed_ = true;}
	void					setPoisonserial( SERIAL d ) {poisonserial_ = d; changed_ = true;}
	void					setSquelched( int d) { squelched_ = d; changed_ = true;}
	void					setMutetime( int d ) { mutetime_ = d; changed_ = true;}
	void					setMed( bool d ) { med_ = d; changed_ = true;}
	void					setBaseSkill( int s, unsigned short v) { baseSkill_[s] = v; changed_ = true;}
	void					setSkill( int s, unsigned short v) { skill_[s] = v; changed_ = true;}
	void					setSocket( cUOSocket* d ) { socket_ = d; changed_ = true;}
	void					setWeight( unsigned short d ) { weight_ = d; changed_ = true;}
	void					setLootList( QString d ) { loot_ = d; changed_ = true;}
	void					setTrackingTarg( SERIAL d ) { trackingTarget_ = d; changed_ = true;}
	void					setTrackingTimer( UINT32 d ) { trackingTimer_ = d; changed_ = true;}
	void					setFontType( unsigned char d ) { fonttype_ = d; changed_ = true;}
	void					setSayColor( UI16 d ) { saycolor_ = d; changed_ = true;}
	void					setEmoteColor( unsigned short d ) { emotecolor_ = d; changed_ = true;}
	void					setSt( signed short d ) { st_ = d; changed_ = true;}
	void					setSt2( signed short d ) { st2_ = d; changed_ = true;}
	void					setMay_Levitate( bool d ) { may_levitate_ = d; changed_ = true;}
	void					setPathNum( unsigned char d ) { pathnum_ = d; changed_ = true;}
	void					setPath( int p, path_st val ){ path_[p] = val; changed_ = true;}
	void					setPathX( int p, unsigned short xValue ) { path_[p].x = xValue; changed_ = true;}
	void					setPathY( int p, unsigned short yValue ) { path_[p].y = yValue; changed_ = true;}
	void					setDispz( signed char d ) { dispz_ = d; changed_ = true;}
	void					setDir( unsigned char d ) { dir_ = d; changed_ = true;}
	void					setXid( unsigned short d ) { xid_ = d; changed_ = true;}
	void					setPriv2( unsigned char d ) { priv2_ = d; changed_ = true;}
	void					setIn( signed short d ) { in_ = d; changed_ = true;}
	void					setIn2( signed short d ) { in2_ = d; changed_ = true;}
	void					setHp( signed short d ) { hp_ = d; changed_ = true;}
	void					setStm( signed short d ) { stm_ = d; changed_ = true;}
	void					setMn( signed short d ) { mn_ = d; changed_ = true;}
	void					setMn2( signed short d ) { mn2_ = d; changed_ = true;}
	void					setHiDamage( int d ) { hidamage_ = d; changed_ = true;}
	void					setLoDamage( int d ) { lodamage_ = d; changed_ = true;}
	void					setNpc( bool d ) { npc_ = d; changed_ = true;}
	void					setShop( bool d ) { shop_ = d; changed_ = true;}
	void					setCell( unsigned char d ) { cell_ = d; changed_ = true;}
	void					setJailTimer( unsigned int d ) { jailtimer_ = d; changed_ = true;}
	void					setJailSecs( int d ) { jailsecs_ = d; changed_ = true;}
	void					setLastSection( cMakeMenu* basemenu, cMakeSection* d );
	void					setRobe( int d ) { robe_ = d; changed_ = true;}
	void					setKarma( int d ) { karma_ = d; changed_ = true;}
	void					setFame( signed int d ) { fame_ = d; changed_ = true;}
	void					setKills( unsigned int d ) { kills_ = d; changed_ = true;}
	void					setDeaths( unsigned int d ) { deaths_ = d; changed_ = true;}
	void					setDead( bool d ) { dead_ = d; changed_ = true;}
	void					setFixedLight( unsigned char d ) { fixedlight_ = d; changed_ = true;}
	void					setSpeech( unsigned char d ) { speech_ = d; changed_ = true;}
	void					setDef( unsigned int d ) { def_ = d; changed_ = true;}
	void					setWar( bool d ) { war_ = d; changed_ = true;}
	void					setFood( unsigned int d ) { food_ = d; changed_ = true;}
	void					setTarg( SERIAL d ) { targ_ = d; changed_ = true;}
	void					setTimeOut( unsigned int d ) { timeout_ = d; changed_ = true;}
	void					setRegen( unsigned int d ) { regen_ = d; changed_ = true;}
	void					setRegen2( unsigned int d ) { regen2_ = d; changed_ = true;}  
	void					setRegen3( unsigned int d ) { regen3_ = d; changed_ = true;}
	void					setInputMode( enInputMode d ) { inputmode_ = d; changed_ = true;}
	void					setInputItem( SERIAL d ) { inputitem_ = d; changed_ = true;}
	void					setAttacker( SERIAL d ) { attacker_ = d; changed_ = true;}
	void					setNpcMoveTime( unsigned int d ) { npcmovetime_ = d; changed_ = true;}
	void					setNpcWander( unsigned char d ) { npcWander_ = d; changed_ = true;}
	void					setOldNpcWander( unsigned char d ) { oldnpcWander_ = d; changed_ = true;}
	void					setFtarg( SERIAL d ) { ftarg_ = d; changed_ = true;}
	void					setPtarg( Coord_cl d ) { ptarg_ = d; changed_ = true;}
	void					setFx1( int d ) { fx1_ = d; changed_ = true;}
	void					setFx2( int d ) { fx2_ = d; changed_ = true;}
	void					setFy1( int d ) { fy1_ = d; changed_ = true;}
	void					setFy2( int d ) { fy2_ = d; changed_ = true;}
	void					setFz1( signed char d ) { fz1_ = d; changed_ = true;}
	void					setRegion( cTerritory* d ) { region_ = d; changed_ = true;}
	void					setSkillDelay( unsigned int d ) { skilldelay_ = d; changed_ = true;}
	void					setObjectDelay( unsigned int d ) { objectdelay_ = d; changed_ = true;}
	void					setMaking( int d ) { making_ = d; changed_ = true;}
	void					setLastTarget( SERIAL d ) { lastTarget_ = d; changed_ = true;}
	void					setBlocked( char d ) { blocked_ = d; changed_ = true;}
	void					setDir2( char d ) { dir2_ = d; changed_ = true;}
	void					setSpiritSpeakTimer( unsigned int d ) { spiritspeaktimer_ = d; changed_ = true;}
	void					setSpAttack( int d ) { spattack_ = d; changed_ = true;}
	void					setSpaDelay( int d ) { spadelay_ = d; changed_ = true;}
	void					setSpaTimer( unsigned int d ) { spatimer_ = d; changed_ = true;}
	void					setTaming( int d ) { taming_ = d; changed_ = true;}
	void					setSummonTimer( unsigned int d ) { summontimer_ = d; changed_ = true;}
	void					setLockSkill( int index, unsigned char val ) { lockSkill_[index] = val; changed_ = true;}
	void					setVisRange( int d ) { VisRange_ = d; changed_ = true;}
	void					setProfile( const QString &d ) { profile_ = d; changed_ = true;}
	void					clearLastSelections( void );	

	UINT8 notority( P_CHAR pChar ); // Gets the notority toward another char
	void kill();
	void playDeathSound();
	void resurrect();
	void turnTo( cUObject *object );
	P_CHAR unmount();
	void mount( P_CHAR pMount );
	bool	canMoveAll( void ) { return priv2_&0x01; }
	bool	isFrozen( void ) { return priv2_&0x02; }	
	bool	viewHouseIcons( void ) { return priv2_&0x04; }	
	bool	isHiddenPermanently( void ) { return priv2_&0x08; }
	void wear( P_ITEM );
	void updateWornItems();
	void updateWornItems( cUOSocket* );
	short effDex()				{return dx+tmpDex>0 ? dx+tmpDex : 0;}	// returns current effective Dexterity
	short realDex()				{return dx;}	// returns the true Dexterity
	short decDex()				{return dx2;}	// returns the 3 digits behind the decimal point
	void  setDex(signed short val)		{dx = val; changed_ = true;}		// set the true Dex
	void  setDecDex(signed short val)	{dx2 = val; changed_ = true;}	// set the 3 digits
	void  chgDex(signed short val)		{tmpDex += val; changed_ = true;}// intended for temporary changes of Dex
	void  chgRealDex(short val) {dx += val;if(dx<1) dx=1;if(dx>100) dx=100; changed_ = true;}	// intended for permanent changes of Dex
	bool  incDecDex(short val)	{dx2 += val;changed_ = true;
	if (dx2>1000) {dx2-=1000;chgRealDex(1);return true;}
	else return false;}
	unsigned short id() const				{return id_;}
	void setId(unsigned short d)			{id_ = d; changed_ = true;}
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
	void criminal();
	void setAttackFirst()		{this->attackfirst_ = true; changed_ = true;}
	void resetAttackFirst()		{this->attackfirst_ = false; changed_ = true;}
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
	void setSerial(const SERIAL ser);
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

	// Wrapper events! - darkstorm
	virtual bool onSingleClick( P_CHAR Viewer ); // Shows the name of a character to someone else
	virtual bool onWalk( UI08 Direction, UI08 Sequence ); // Walks in a specific Direction

	virtual bool onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ); // The character says something
	virtual bool onWarModeToggle( bool War ); // The character switches warmode
	virtual bool onLogin( void ); // The character enters the world
	virtual bool onLogout( void ); // The character enters the world
	
	virtual bool onHelp( void ); // The character wants help
	virtual bool onChat( void ); // The character wants to chat
	virtual bool onSkillUse( UI08 Skill ); // The character uses %Skill
	virtual bool onCollideChar( P_CHAR Obstacle ); // This is called for the walking character first, then for the character walked on
	virtual bool onShowContext( cUObject *object ); // Shows a context menu for a specific item
	bool onDropOnChar( P_ITEM pItem );
	bool onPickup( P_ITEM pItem );

	virtual void talk( const QString &message, UI16 color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	void giveNewbieItems( Q_UINT8 skill = 0xFF );

	void attackTarget( P_CHAR defender );
	void applyPoison( P_CHAR defender );
	void toggleCombat();
	UI16 calcDefense( enBodyParts bodypart, bool wearout = false );

	P_ITEM rightHandItem();
	P_ITEM leftHandItem();

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
	
	bool Owns( P_ITEM pi );

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
inline bool  cChar::isInnocent() const		{return (flag_&0x04 ?true:false);}
inline bool  cChar::isMurderer() const		{return (flag_&0x01 ?true:false);}
inline bool  cChar::isCriminal() const		{return (flag_&0x02 ?true:false);}
inline unsigned char cChar::getPriv() const	{return priv;}
inline void cChar::setPriv(unsigned char p)	{this->priv=p; changed_ = true;}
inline void cChar::makeInvulnerable()		{priv |= 4; changed_ = true;}
inline void cChar::makeVulnerable()			{priv &= 0xFB; changed_ = true;}
inline void cChar::setMurderer()			{flag_ = 0x01; changed_ = true;}
inline void cChar::setInnocent()			{flag_ = 0x04; changed_ = true;}
inline void cChar::setCriminal()			{flag_ = 0x02; changed_ = true;}

// Getters
inline short			cChar::guildType() const		{ return GuildType; }
inline bool				cChar::guildTraitor() const		{ return GuildTraitor; }
inline QString			cChar::orgname() const			{ return orgname_; }
inline QString			cChar::title() const			{ return title_;   }
inline bool				cChar::unicode() const			{ return unicode_; }
inline AccountRecord*	cChar::account() const			{ return account_; }
inline cChar::Effects	cChar::effects() const			{ return effects_; }

// Setters
inline void	cChar::setGuildType(short d)			{ GuildType = d; changed_ = true; }
inline void cChar::setGuildTraitor(bool  d)			{ GuildTraitor = d; changed_ = true;}
inline void	cChar::setOrgname( const QString& d )	{ orgname_ = d; changed_ = true;}
inline void cChar::setTitle( const QString& d )		{ title_ = d;   changed_ = true;}
inline void cChar::setUnicode(bool d)				{ unicode_ = d; changed_ = true;}

#endif // __CHARS_H__
