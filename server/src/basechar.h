//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#ifndef CBASECHAR_H_HEADER_INCLUDED
#define CBASECHAR_H_HEADER_INCLUDED

// platform includes
#include "platform.h"

// library includes
#include <qmap.h>
#include <qvaluevector.h>
#include <qdatetime.h>

// wolfpack includes
#include "typedefs.h"
#include "uobject.h"
#include "TmpEff.h"
#include "territories.h"

// This class is the base interface for all char objects. 
class cBaseChar : public cUObject
{
	Q_OBJECT

public:
	// con-/destructors
    cBaseChar();
    cBaseChar(const cBaseChar& right);
    virtual ~cBaseChar();
 
	// operators
    cBaseChar& operator=(const cBaseChar& right);

	// type definitions
	typedef QMap<ushort, cItem*> ItemContainer;
	typedef QValueVector< cBaseChar* > CharContainer;
	typedef QValueVector< cTempEffect* > EffectContainer;
	enum enLayer { TradeWindow, SingleHandedWeapon, DualHandedWeapon, Shoes, Pants, Shirt, Hat, Gloves,
	Ring, Neck = 0xA, Hair, Waist, InnerTorso, Bracelet, FacialHair = 0x10,  MiddleTorso, 
	Earrings, Arms, Back, Backpack, OuterTorso, OuterLegs, InnerLegs, Mount, BuyRestockContainer,
	BuyNoRestockContainer, SellContainer, BankBox, Dragging };

	// implementation of interfaces
	void load( char **, UINT16& );
	void save();
	void save( FlatStore::OutputFile*, bool first = false ) throw();
	bool load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile* ) throw();
	bool postload() throw();
	bool del();

	// interface methods
	// object type specific methods
	virtual enCharTypes objectType() = 0;
	// network related methods
	virtual void update( bool excludeself = false ) = 0; 
	virtual void resend( bool clean = true, bool excludeself = false ) = 0; 
	// other methods
	virtual UINT8 notority( P_CHAR pChar ) = 0; // Gets the notority towards another char
	virtual void kill() = 0;
	virtual void showName( cUOSocket *socket ) = 0;
	virtual void fight(P_CHAR pOpponent) = 0;
	virtual void soundEffect( UI16 soundId, bool hearAll = true ) = 0;
	virtual void giveGold( Q_UINT32 amount, bool inBank = false ) = 0;
	virtual UINT32 takeGold( UINT32 amount, bool useBank = false ) = 0;

	// other public methods
	// Simple Property setting and getting for script engines.
	stError *setProperty( const QString &name, const cVariant &value );
	stError *getProperty( const QString &name, cVariant &value ) const;
	void updateHealth( void );
	void action( UINT8 id ); // Do an action
	P_ITEM getWeapon() const;
	P_ITEM getShield() const;
	void setHairColor( UINT16 d); 
	void setHairStyle( UINT16 d); 
	void setBeardColor( UINT16 d); 
	void setBeardStyle( UINT16 d); 
	void playDeathSound();
	void resurrect();
	void turnTo( cUObject *object );
	void turnTo( const Coord_cl &pos );
	void wear( P_ITEM );
	bool isHuman() const;
	bool isMurderer() const;
	bool isCriminal() const;
	bool isInnocent() const;
	void unhide();
	int  CountItems(short ID, short col= -1);
	int  CountGold();
	P_ITEM GetItemOnLayer(unsigned char layer);
	P_ITEM getBackpack();
	void SetMultiSerial(long mulser);
	void setSerial(SERIAL ser);
	void MoveTo(short newx, short newy, signed char newz);
	void MoveToXY(short newx, short newy);
	bool Wears(P_ITEM pi);
	unsigned int getSkillSum();
	void removeItemBonus(cItem* pi);
	void giveItemBonus(cItem* pi);
	void Init(bool ser = true);
	bool isSameAs(P_CHAR pc);
	bool inGuardedArea();
	void emote( const QString &emote, UI16 color = 0xFFFF );
	UI16 calcDefense( enBodyParts bodypart, bool wearout = false );
	P_ITEM rightHandItem() const;
	P_ITEM leftHandItem() const;
	bool checkSkill( UI16 skill, SI32 min, SI32 max, bool advance = true );
	cItem* atLayer( enLayer layer ) const;
	bool Owns( P_ITEM pi ) const;

	// Wrapper events
	virtual bool onSingleClick( P_CHAR Viewer ); // Shows the name of a character to someone else
	virtual bool onWalk( UI08 Direction, UI08 Sequence ); // Walks in a specific Direction
	virtual bool onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ); // The character says something
	virtual bool onWarModeToggle( bool War ); // The character switches warmode
	virtual bool onShowPaperdoll( P_CHAR pOrigin ); // The paperdoll of this character is requested, there is no vice-versa call
	virtual bool onSkillUse( UI08 Skill ); // The character uses %Skill
	virtual bool onCollideChar( P_CHAR Obstacle ); // This is called for the walking character first, then for the character walked on
	bool onDropOnChar( P_ITEM pItem );
	QString onShowPaperdollName( P_CHAR pOrigin ); // only change the viewed name

	// getters
    SERIAL			attackerSerial() const;
    UINT16			bodyArmor() const;
    UINT16			bodyID() const;
    SERIAL			combatTarget() const;
    QDateTime		creationDate() const;
    UINT32			criminalTime() const;
    UINT16			deaths() const;
    INT16			dexEff() const;
    INT16			dexterityMod() const;
    INT16			dexterity() const;
    UINT8			direction() const;
    UINT16			emoteColor() const;
    INT16			fame() const;
    UINT8			flag() const;
    bool			gender() const;
    P_CHAR			guarding() const;
    INT16			hitpoints() const;
    INT32			hunger() const;
    UINT32			hungerTime() const;
    INT16			intelligence() const;
    INT16			intelligenceMod() const;
    INT16			karma() const;
    UINT16			kills() const;
    INT16			mana() const;
    UINT16			maxHitpoints() const;
    UINT16			maxMana() const;
    UINT16			maxStamina() const;
    SERIAL			murdererSerial() const;
    UINT32			murdererTime() const;
    UINT32			nextHitTime() const;
    UINT32			nutriment() const;
    INT16			orgBodyID() const;
    QString			orgName() const;
    UINT16			orgSkin() const;
    INT32			poison() const;
    UINT32			poisoned() const;
    UINT32			poisonTime() const;
    UINT32			poisonWearOffTime() const;
    UINT32			propertyFlags() const;
	UINT32			regenHitpointsTime() const;
	UINT32			regenStaminaTime() const;
	UINT32			regenManaTime() const;
    cTerritory*		region() const;
    UINT32			runningSteps() const;
    UINT16			saycolor() const;
    UINT32			skillDelay() const;
    UINT16			skin() const;
    INT16			stamina() const;
    INT32			stealthedSteps() const;
    INT16			strength() const;
    INT16			strengthMod() const;
    SERIAL			swingTarget() const;
    QString			title() const;
    UINT16			weight() const;
	// bit flag getters
	bool			isIncognito() const;
	bool			isPolymorphed() const;
	bool			isTamed() const;
	bool			isCasting() const;
	bool			isHidden() const;
	bool			isInvisible() const;
	bool			hasReactiveArmor() const;
	bool			isMeditating() const;
	bool			isFrozen() const;
	bool			showSkillTitles() const;
	bool			isDead() const;
	bool			isAtWar() const;
	bool			isInvulnerable() const;
	bool			attackFirst() const;
	// advanced getters for data structures
	// skills
	UINT16			skillValue( UINT16 skill ) const;
	UINT16			skillCap( UINT16 skill ) const;
	UINT8			skillLock( UINT16 skill ) const;
	// effects
	EffectContainer	effects() const;
	// guards
	CharContainer	guardedby() const;
	// content
	ItemContainer	content() const;

	// setters
    void setAttackerSerial(SERIAL data);
	void setBodyArmor(UINT16 data);
    void setBodyID(UINT16 data);
    void setCombatTarget(SERIAL data);
    void setCreationDate(const QDateTime &data);
    void setCriminalTime(UINT32 data);
    void setDeaths(UINT16 data);
    void setDexEff(INT16 data);
    void setDexterityMod(INT16 data);
    void setDexterity(INT16 data);
    void setDirection(UINT8 data);
    void setEmoteColor(UINT16 data);
    void setFame(INT16 data);
    void setFlag(UINT8 data);
    void setGender(bool data);
	void setGuarding(P_CHAR data);
    void setHitpoints(INT16 data);
    void setHunger(INT32 data);
    void setHungerTime(UINT32 data);
    void setIntelligence(INT16 data);
    void setIntelligenceMod(INT16 data);
    void setKarma(INT16 data);
    void setKills(UINT16 data);
    void setMana(INT16 data);
    void setMaxHitpoints(UINT16 data);
    void setMaxMana(UINT16 data);
    void setMaxStamina(UINT16 data);
    void setMurdererSerial(SERIAL data);
    void setMurdererTime(UINT32 data);
    void setNextHitTime(UINT32 data);
    void setNutriment(UINT32 data);
    void setOrgBodyID(INT16 data);
    void setOrgName(const QString &data);
    void setOrgSkin(UINT16 data);
    void setPoison(INT32 data);
    void setPoisoned(UINT32 data);
    void setPoisonTime(UINT32 data);
    void setPoisonWearOffTime(UINT32 data);
    void setPropertyFlags(UINT32 data);
	void setRegenHitpointsTime(UINT32 data);
	void setRegenStaminaTime(UINT32 data);
	void setRegenManaTime(UINT32 data);
    void setRegion(cTerritory* data);
    void setRunningSteps(UINT32 data);
    void setSaycolor(UINT16 data);
    void setSkillDelay(UINT32 data);
    void setSkin(UINT16 data);
    virtual void setStamina(INT16 data, bool notify = true );
    void setStealthedSteps(INT32 data);
    void setStrength(INT16 data);
    void setStrengthMod(INT16 data);
    void setSwingTarget(SERIAL data);
    void setTitle(const QString &data);
    void setWeight(UINT16 data);
	// bit flag setters
	void setIncognito(bool data);
	void setPolymorphed(bool data);
	void setTamed(bool data);
	void setCasting(bool data);
	void setHidden(bool data);
	void setInvisible(bool data);
	void setReactiveArmor(bool data);
	void setMeditating(bool data);
	void setFrozen(bool data);
	void setShowSkillTitles(bool data);
	void setDead(bool data);
	void setAtWar(bool data);
	void setInvulnerable(bool data);
	void setAttackFirst(bool data);
	// advanced setters for data structures
	// skills
	void setSkillValue( UINT16 skill, UINT16 value );
	void setSkillCap( UINT16 skill, UINT16 cap );
	void setSkillLock( UINT16 skill, UINT8 lock );
	// effects
	void addEffect( cTempEffect *effect );
	void removeEffect( cTempEffect *effect );
	// guards
	void addGuard( P_CHAR pPet, bool noGuardingChange = false );
	void removeGuard( P_CHAR pPet, bool noGuardingChange = false );
	// content
	void addItem( enLayer layer, cItem*, bool handleWeight = true, bool noRemove = false );
	void removeItem( enLayer layer, bool handleWeight = true );

protected:
	// type definitions
	struct stSkillValue
	{
		UINT16 value; // Skill Value (Default: 0)
		UINT16 cap; // Special Cap Value (Default: 1000)
		UINT8 lock; // 0: Up, 1: Down, 2: Locked (Default: 0)

		stSkillValue(): value( 0 ), cap( 1000 ), lock( 0 ) {}
	};

	// other protected methods
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );
	virtual void processNode( const QDomElement &Tag );

    // The body ID for this character. cOldChar::id_
    UINT16 bodyID_;

    // The original body id, when the char is affected by magic.
    // cOldChar::xid_
    UINT16 orgBodyID_;

    // The gender of the character. cOldChar::sex_
    bool gender_;

    
    // The original skin color hue of the char. Is needed after applying
    // magical/temporal effects which change skin color.
    // cOldChar::xskin_
    UINT16 orgSkin_;

    // Flag storage for magical/temporal/skill effects like incognito,
    // polymorph, ... . 
    // Bits:
    // 01 - incognito. cOldChar::incognito_
    // 02 - polymorph. cOldChar::polymorph_
    // 03 - tamed, cOldChar::tamed_
    // 04 - casting, cOldChar::casting_
    // 05 - hidden, cOldChar::hidden_, Bit 1 & 2
    // 06 - invisible, cOldChar::invisible_, Bit 3
    // 07 - reactive armor, cOldChar::ra_
    // 08 - meditating, cOldChar::med_
    // 09 - frozen, cOldChar::priv2_, Bit 2
    // 10 - show skill titles, cOldChar::priv2, Bit 4
    // 11 - dead, cOldChar::dead
    // 12 - war, cOldChar::war
	// 13 - invulnerable, cOldChar::priv2 Bit 3
	// 14 - attack first, cOldChar::attackfirst_
    UINT32 propertyFlags_;

    // Weight of the char, including worn items.
    UINT16 weight_;

    // Base body armor value.
    UINT16 bodyArmor_;

    // Direction the char looks at.
    // cOldChar::dir_
    UINT8 direction_;

    // Dexterity of the char
    // cOldChar::dx
    INT16 dexterity_;

    // dex modifier for influencing equipped items.
    // cOldChar::dx2
    INT16 dexterityMod_;

    // Temporal effected dexterity.
    // cOldChar::tmpDex
    INT16 dexEff_;

    // maximum stamina the character can fill up
    UINT16 maxStamina_;

    // current stamina of the char.
    // cOldChar::stm_
    INT16 stamina_;

    // strength of the char
    // cOldChar::st_
    INT16 strength_;

    // temporal strength addons.
    // cOldChar::st2_
    INT16 strengthMod_;

    // Maximum hitpoints the char can fill up to.
    UINT16 maxHitpoints_;

    // current hitpoints of the char.
    // cOldChar::hp_
    INT16 hitpoints_;

    // Intelligence of the char.
    // cOldChar::in_
    INT16 intelligence_;

    // Modifier for intelligence. cOldChar::in2_
    INT16 intelligenceMod_;

    // Maximum mana the char can fill up.
    UINT16 maxMana_;

    // current mana of the char.
    // cOldChar::mn_
    INT16 mana_;

    // Karma of the char.
    INT16 karma_;

    // Fame of the char.
    INT16 fame_;

    // Kills the char has made
    UINT16 kills_;

    // Times the char has died.
    UINT16 deaths_;

    // The hunger value of the char. 6 means not hungry, 0 means starving.
    // cOldChar::hunger_
    INT32 hunger_;

    // Server clocks when next hunger check will be made.
    // cOldChar::hungertime_
    UINT32 hungerTime_;

    // the type of food the char can eat to decrease hunger
    // cOldChar::food_
    UINT32 nutriment_;

    // Ingame name-color flag.
    // Bits:
    // 01 - red
    // 02 - grey
    // 03 - blue
    // 04 - green
    // 05 - orange
    UINT8 flag_;

    // Color for emote messages.
    UINT16 emoteColor_;

    // Saves the date of creation. cOldChar::creationday_
    QDateTime creationDate_;

    // Saves the number of steps that were stealthed. value -1 indicates that
    // the char will be revealed
    INT32 stealthedSteps_;

    // Saves the number of steps the char ran.
    UINT32 runningSteps_;

    // Time, till murderer flag disappears. cOldChar::murderrate_
    UINT32 murdererTime_;

    // Time, till criminal flag wears off. value -1 indicates not criminal!
    UINT32 criminalTime_;

    // Time till a combat hit times out.
    UINT32 nextHitTime_;

    // time till next skill usage is possible
    UINT32 skillDelay_;

    // Poison value. dont ask me :/
    INT32 poison_;

    // poisoned value.dont ask me :/
    UINT32 poisoned_;

    // poison timer value. dont ask me..
    UINT32 poisonTime_;

    // poison wear off timer.dont ask me
    UINT32 poisonWearOffTime_;

    // Title of the char.
    QString title_;

    // Original name of the char.
    // cOldChar::orgname_
    QString orgName_;

    // Skin color hue of the char.
    UINT16 skin_;

    // Temporal effects that affect this character.
    EffectContainer effects_;

    // Item contents of the char (i.e. equipment).
    ItemContainer content_;

    // Skill properties of this char.
    QValueVector< stSkillValue > skills_;

    // Region the char is in.
    cTerritory* region_;

    // Color hue the char speeks with.
    UINT16 saycolor_;

    // Serial of the char, which attacked this character.
    // cOldChar::attacker_
    SERIAL attackerSerial_;

    // Serial of the char, which the character is currently fighting with.
    // cOldChar::targ_
    SERIAL combatTarget_;

    // Target the char is going to hit after it swang
    SERIAL swingTarget_;

    // Serial of the last char which murdered this character.
    // cOldChar::murdererSer_
    SERIAL murdererSerial_;

    // Chars that guard this character.
    CharContainer guardedby_;

	// Time, when next hitpoint will be regenerated.
	// cOldChar::regen_
	UINT32 regenHitpointsTime_;

	// Time, when next stamina point will be regenerated.
	// cOldChar::regen2_
	UINT32 regenStaminaTime_;

	// Time, when next mana point will be regenerated.
	// cOldChar::regen3_
	UINT32 regenManaTime_;

    // Char which the character guards.
    P_CHAR guarding_;
};

inline P_CHAR cBaseChar::guarding() const
{
    return guarding_;
}

inline void cBaseChar::setGuarding(P_CHAR data)
{
	if( data == guarding_ )
		return;

	if( guarding_ )
		guarding_->removeGuard( this );

	guarding_ = data;
	changed( SAVE|TOOLTIP );

	if( guarding_ )
		guarding_->addGuard( this );		
}

inline UINT16 cBaseChar::bodyArmor() const
{
    return bodyArmor_;
}

inline void cBaseChar::setBodyArmor(UINT16 data)
{
    bodyArmor_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::bodyID() const
{
    return bodyID_;
}

inline void cBaseChar::setBodyID(UINT16 data)
{
    bodyID_ = data;
	changed( SAVE );
}

inline QDateTime cBaseChar::creationDate() const
{
    return creationDate_;
}

inline void cBaseChar::setCreationDate(const QDateTime &data)
{
    creationDate_ = data;
	changed( SAVE );
}

UINT32 cBaseChar::criminalTime() const
{
    return criminalTime_;
}

inline void cBaseChar::setCriminalTime(UINT32 data)
{
    criminalTime_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::deaths() const
{
    return deaths_;
}

inline void cBaseChar::setDeaths(UINT16 data)
{
    deaths_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::dexEff() const
{
    return dexEff_;
}

inline void cBaseChar::setDexEff(INT16 data)
{
    dexEff_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::dexterityMod() const
{
    return dexterityMod_;
}

inline void cBaseChar::setDexterityMod(INT16 data)
{
    dexterityMod_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::dexterity() const
{
    return dexterity_;
}

inline void cBaseChar::setDexterity(INT16 data)
{
    dexterity_ = data;
	changed( SAVE );
}

inline UINT8 cBaseChar::direction() const
{
    return direction_;
}

inline void cBaseChar::setDirection(UINT8 data)
{
    direction_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::emoteColor() const
{
    return emoteColor_;
}

inline void cBaseChar::setEmoteColor(UINT16 data)
{
    emoteColor_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::fame() const
{
    return fame_;
}

inline void cBaseChar::setFame(INT16 data)
{
    fame_ = data;
	changed( SAVE );
}

inline UINT8 cBaseChar::flag() const
{
    return flag_;
}

inline void cBaseChar::setFlag(UINT8 data)
{
    flag_ = data;
	changed( SAVE );
}

bool cBaseChar::gender() const
{
    return gender_;
}

inline void cBaseChar::setGender(bool data)
{
    gender_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::hitpoints() const
{
    return hitpoints_;
}

inline void cBaseChar::setHitpoints(INT16 data)
{
    hitpoints_ = data;
	changed( SAVE );
}

inline INT32 cBaseChar::hunger() const
{
    return hunger_;
}

inline void cBaseChar::setHunger(INT32 data)
{
    hunger_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::hungerTime() const
{
    return hungerTime_;
}

inline void cBaseChar::setHungerTime(UINT32 data)
{
    hungerTime_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::intelligence() const
{
    return intelligence_;
}

inline void cBaseChar::setIntelligence(INT16 data)
{
    intelligence_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::intelligenceMod() const
{
    return intelligenceMod_;
}

inline void cBaseChar::setIntelligenceMod(INT16 data)
{
    intelligenceMod_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::karma() const
{
    return karma_;
}

inline void cBaseChar::setKarma(INT16 data)
{
    karma_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::kills() const
{
    return kills_;
}

inline void cBaseChar::setKills(UINT16 data)
{
    kills_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::mana() const
{
    return mana_;
}

inline void cBaseChar::setMana(INT16 data)
{
    mana_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::maxHitpoints() const
{
    return maxHitpoints_;
}

inline void cBaseChar::setMaxHitpoints(UINT16 data)
{
    maxHitpoints_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::maxMana() const
{
    return maxMana_;
}

inline void cBaseChar::setMaxMana(UINT16 data)
{
    maxMana_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::maxStamina() const
{
    return maxStamina_;
}

inline void cBaseChar::setMaxStamina(UINT16 data)
{
    maxStamina_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::murdererTime() const
{
    return murdererTime_;
}

inline void cBaseChar::setMurdererTime(UINT32 data)
{
    murdererTime_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::nextHitTime() const
{
    return nextHitTime_;
}

inline void cBaseChar::setNextHitTime(UINT32 data)
{
    nextHitTime_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::nutriment() const
{
    return nutriment_;
}

inline void cBaseChar::setNutriment(UINT32 data)
{
    nutriment_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::orgBodyID() const
{
    return orgBodyID_;
}

inline void cBaseChar::setOrgBodyID(INT16 data)
{
    orgBodyID_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::orgSkin() const
{
    return orgSkin_;
}

inline void cBaseChar::setOrgSkin(UINT16 data)
{
    orgSkin_ = data;
	changed( SAVE );
}

inline INT32 cBaseChar::poison() const
{
    return poison_;
}

inline void cBaseChar::setPoison(INT32 data)
{
    poison_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::poisoned() const
{
    return poisoned_;
}

inline void cBaseChar::setPoisoned(UINT32 data)
{
    poisoned_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::poisonTime() const
{
    return poisonTime_;
}

inline void cBaseChar::setPoisonTime(UINT32 data)
{
    poisonTime_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::poisonWearOffTime() const
{
    return poisonWearOffTime_;
}

inline void cBaseChar::setPoisonWearOffTime(UINT32 data)
{
    poisonWearOffTime_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::propertyFlags() const
{
    return propertyFlags_;
}

inline void cBaseChar::setPropertyFlags(UINT32 data)
{
    propertyFlags_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::runningSteps() const
{
    return runningSteps_;
}

inline void cBaseChar::setRunningSteps(UINT32 data)
{
    runningSteps_ = data;
}

inline UINT32 cBaseChar::skillDelay() const
{
    return skillDelay_;
}

inline void cBaseChar::setSkillDelay(UINT32 data)
{
    skillDelay_ = data;
}

inline INT16 cBaseChar::stamina() const
{
    return stamina_;
}

inline INT32 cBaseChar::stealthedSteps() const
{
    return stealthedSteps_;
}

inline void cBaseChar::setStealthedSteps(INT32 data)
{
    stealthedSteps_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::strength() const
{
    return strength_;
}

inline void cBaseChar::setStrength(INT16 data)
{
    strength_ = data;
	changed( SAVE );
}

inline INT16 cBaseChar::strengthMod() const
{
    return strengthMod_;
}

inline void cBaseChar::setStrengthMod(INT16 data)
{
    strengthMod_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::weight() const
{
    return weight_;
}

inline void cBaseChar::setWeight(UINT16 data)
{
    weight_ = data;
	changed( SAVE );
}

inline SERIAL cBaseChar::attackerSerial() const
{
    return attackerSerial_;
}

inline void cBaseChar::setAttackerSerial(SERIAL data)
{
    attackerSerial_ = data;
	changed( SAVE );
}

inline SERIAL cBaseChar::combatTarget() const
{
    return combatTarget_;
}

inline void cBaseChar::setCombatTarget(SERIAL data)
{
    combatTarget_ = data;
	changed( SAVE );
}

inline SERIAL cBaseChar::murdererSerial() const
{
    return murdererSerial_;
}

inline void cBaseChar::setMurdererSerial(SERIAL data)
{
    murdererSerial_ = data;
	changed( SAVE );
}

inline QString cBaseChar::orgName() const
{
    return orgName_;
}

inline void cBaseChar::setOrgName(const QString &data)
{
    orgName_ = data;
	changed( SAVE );
}

inline cTerritory* cBaseChar::region() const
{
    return region_;
}

inline void cBaseChar::setRegion(cTerritory* data)
{
    region_ = data;
}

inline UINT16 cBaseChar::saycolor() const
{
    return saycolor_;
}

inline void cBaseChar::setSaycolor(UINT16 data)
{
    saycolor_ = data;
	changed( SAVE );
}

inline UINT16 cBaseChar::skin() const
{
    return skin_;
}

inline void cBaseChar::setSkin(UINT16 data)
{
    skin_ = data;
	changed( SAVE );
}

inline SERIAL cBaseChar::swingTarget() const
{
    return swingTarget_;
}

inline void cBaseChar::setSwingTarget(SERIAL data)
{
    swingTarget_ = data;
}

inline QString cBaseChar::title() const
{
    return title_;
}

inline void cBaseChar::setTitle(const QString &data)
{
    title_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::regenHitpointsTime() const
{
    return regenHitpointsTime_;
}

inline void cBaseChar::setRegenHitpointsTime(UINT32 data)
{
    regenHitpointsTime_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::regenStaminaTime() const
{
    return regenStaminaTime_;
}

inline void cBaseChar::setRegenStaminaTime(UINT32 data)
{
    regenStaminaTime_ = data;
	changed( SAVE );
}

inline UINT32 cBaseChar::regenManaTime() const
{
    return regenManaTime_;
}

inline void cBaseChar::setRegenManaTime(UINT32 data)
{
    regenManaTime_ = data;
	changed( SAVE );
}

inline bool cBaseChar::isIncognito() const
{
	return propertyFlags_ & 0x0001;
}

inline bool cBaseChar::isPolymorphed() const
{
	return propertyFlags_ & 0x0002;
}

inline bool cBaseChar::isTamed() const
{
	return propertyFlags_ & 0x0004;
}

inline bool cBaseChar::isCasting() const
{
	return propertyFlags_ & 0x0008;
}

inline bool cBaseChar::isHidden() const
{
	return propertyFlags_ & 0x0010;
}

inline bool cBaseChar::isInvisible() const
{
	return propertyFlags_ & 0x0020;
}

inline bool cBaseChar::hasReactiveArmor() const
{
	return propertyFlags_ & 0x0040;
}

inline bool cBaseChar::isMeditating() const
{
	return propertyFlags_ & 0x0080;
}

inline bool cBaseChar::isFrozen() const
{
	return propertyFlags_ & 0x0100;
}

inline bool cBaseChar::showSkillTitles() const
{
	return propertyFlags_ & 0x0200;
}

inline bool cBaseChar::isDead() const
{
	return propertyFlags_ & 0x0400;
}

inline bool cBaseChar::isAtWar() const
{
	return propertyFlags_ & 0x0800;
}

inline bool cBaseChar::isInvulnerable() const
{
	return propertyFlags_ & 0x1000;
}

inline bool cBaseChar::attackFirst() const
{
	return propertyFlags_ & 0x2000;
}

inline void cBaseChar::setIncognito(bool data)
{
	if( data ) propertyFlags_ |= 0x0001; else propertyFlags_ &= ~0x0001; 
	changed( SAVE );
}

inline void cBaseChar::setPolymorphed(bool data)
{
	if( data ) propertyFlags_ |= 0x0002; else propertyFlags_ &= ~0x0002; 
	changed( SAVE );
}

inline void cBaseChar::setTamed(bool data)
{
	if( data ) propertyFlags_ |= 0x0004; else propertyFlags_ &= ~0x0004; 
	changed( SAVE );
}

inline void cBaseChar::setCasting(bool data)
{
	if( data ) propertyFlags_ |= 0x0008; else propertyFlags_ &= ~0x0008; 
	changed( SAVE );
}

inline void cBaseChar::setHidden(bool data)
{
	if( data ) propertyFlags_ |= 0x0010; else propertyFlags_ &= ~0x0010; 
	changed( SAVE );
}

inline void cBaseChar::setInvisible(bool data)
{
	if( data ) propertyFlags_ |= 0x0020; else propertyFlags_ &= ~0x0020; 
	changed( SAVE );
}

inline void cBaseChar::setReactiveArmor(bool data)
{
	if( data ) propertyFlags_ |= 0x0040; else propertyFlags_ &= ~0x0040; 
	changed( SAVE );
}

inline void cBaseChar::setMeditating(bool data)
{
	if( data ) propertyFlags_ |= 0x0080; else propertyFlags_ &= ~0x0080; 
	changed( SAVE );
}

inline void cBaseChar::setFrozen(bool data)
{
	if( data ) propertyFlags_ |= 0x0100; else propertyFlags_ &= ~0x0100; 
	changed( SAVE );
}

inline void cBaseChar::setShowSkillTitles(bool data)
{
	if( data ) propertyFlags_ |= 0x0200; else propertyFlags_ &= ~0x0200; 
	changed( SAVE );
}

inline void cBaseChar::setDead(bool data)
{
	if( data ) propertyFlags_ |= 0x0400; else propertyFlags_ &= ~0x0400; 
	changed( SAVE );
}

inline void cBaseChar::setAtWar(bool data)
{
	if( data ) propertyFlags_ |= 0x0800; else propertyFlags_ &= ~0x0800; 
	changed( SAVE );
}

inline void cBaseChar::setInvulnerable(bool data)
{
	if( data ) propertyFlags_ |= 0x1000; else propertyFlags_ &= ~0x1000; 
	changed( SAVE );
}

inline void cBaseChar::setAttackFirst(bool data)
{
	if( data ) propertyFlags_ |= 0x2000; else propertyFlags_ &= ~0x2000; 
	changed( SAVE );
}

inline bool cBaseChar::isHuman() const 
{ 
	return (bodyID_ == 0x190 || bodyID_ == 0x191); 
}

inline bool cBaseChar::isInnocent() const
{
	return !isMurderer() && !isCriminal();
}

inline bool cBaseChar::isSameAs(P_CHAR pc)
{
	return ( pc && pc->serial() == serial() );
}

inline P_ITEM cBaseChar::rightHandItem() const
{
	return atLayer( SingleHandedWeapon );
}

inline P_ITEM cBaseChar::leftHandItem() const
{
	return atLayer( DualHandedWeapon );
}


#endif /* CBASECHAR_H_HEADER_INCLUDED */
