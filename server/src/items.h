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

#if !defined( __ITEMS_H )
#define __ITEMS_H

// Wolfpack Includes
#include "uobject.h"
#include "defines.h"
#include "network/uotxpackets.h"

// Library Includes
#include <qvaluevector.h>

// Forward Class declarations
class ISerialization;
class cUOSocket;


class cItem : public cUObject
{
	Q_OBJECT
	Q_PROPERTY ( ushort		id			READ id				WRITE setId				)
	Q_PROPERTY ( ushort		color		READ color			WRITE setColor			)
	Q_PROPERTY ( ushort		amount		READ amount			WRITE setAmount			)
	Q_PROPERTY ( ushort		restock		READ restock		WRITE setRestock		)
	Q_PROPERTY ( uchar		layer		READ layer			WRITE setLayer			)
	Q_PROPERTY ( int		totalweight READ totalweight	WRITE setTotalweight	)
	Q_PROPERTY ( ushort		accuracy	READ accuracy		WRITE setAccuracy		)
	Q_PROPERTY ( int		sellprice	READ sellprice		WRITE setSellprice		)
	Q_PROPERTY ( int		buyprice	READ buyprice		WRITE setBuyprice		)
	Q_PROPERTY ( uchar		more1		READ more1			WRITE setMore1			)
	Q_PROPERTY ( uchar		more2		READ more2			WRITE setMore2			)
	Q_PROPERTY ( uchar		more3		READ more3			WRITE setMore3			)
	Q_PROPERTY ( uchar		more4		READ more4			WRITE setMore4			)
	Q_PROPERTY ( uchar		morex		READ morex			WRITE setMoreX			)
	Q_PROPERTY ( uchar		morey		READ morey			WRITE setMoreY			)
	Q_PROPERTY ( uchar		morez		READ morez			WRITE setMoreZ			)
	Q_PROPERTY ( uchar		doordir		READ doordir		WRITE setDoorDir		)
	Q_PROPERTY ( uchar		dooropen	READ dooropen		WRITE setDoorOpen		)
	Q_PROPERTY ( uchar		dye			READ dye			WRITE setDye			)
	Q_PROPERTY ( uint		att			READ att			WRITE setAtt			)
	Q_PROPERTY ( uint		def			READ def			WRITE setDef			)
	Q_PROPERTY ( short		StrengthReq	READ strengthReq	WRITE setStrengthReq	)
	Q_PROPERTY ( short		strengthMod	READ strengthMod	WRITE setStrengthMod	)
	Q_PROPERTY ( short		dexterityReq READ dexterityReq	WRITE setDexterityReq	)
	Q_PROPERTY ( short		dexterityMod READ dexterityMod	WRITE setDexterityMod	)
	Q_PROPERTY ( short		intelligenceMod READ intelligenceMod	WRITE setIntelligenceMod	)
	Q_PROPERTY ( short		intelligenceReq	READ intelligenceReq	WRITE setIntelligenceReq	)
	Q_PROPERTY ( uchar		magic		READ magic			WRITE setMagic			)
	Q_PROPERTY ( uint		decaytime	READ decaytime		WRITE setDecayTime		)
	Q_PROPERTY ( uint		disabled	READ disabled		WRITE setDisabled		)
	Q_PROPERTY ( uint		poisoned	READ poisoned		WRITE setPoisoned		)
	Q_PROPERTY ( int		rank		READ rank			WRITE setRank			)
	Q_PROPERTY ( QString	creator		READ creator		WRITE setCreator		)
	Q_PROPERTY ( int		ownserial	READ ownSerial		WRITE SetOwnSerial		)
	Q_PROPERTY ( uchar		visible		READ visible		WRITE setVisible		)
	Q_PROPERTY ( uchar		priv		READ priv			WRITE setPriv			)
	Q_PROPERTY ( int		good		READ good			WRITE setGood			)
	Q_PROPERTY ( int		rndvaluerate READ rndvaluerate	WRITE setRndValueRate	)
	Q_PROPERTY ( uchar		madewith	READ madewith		WRITE setMadeWith		)

private:
	bool changed_;
	void flagChanged() { changed_ = true; } // easier to debug, compiler should make it inline;

public:
	typedef QValueVector<cItem*> ContainerContent;

public:

	virtual void	talk( const QString &message, ushort color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	void load( char **, UINT16& );
	void save();
	void save( FlatStore::OutputFile*, bool first = false ) throw();
	bool load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile* ) throw();
	bool postload() throw();
	bool del();

	void	processContainerNode( const cElement *Tag );
	virtual void update( cUOSocket *mSock = NULL );
	P_ITEM	dupe();
	void	soundEffect( UINT16 sound );
	void sendTooltip( cUOSocket* mSock );

	// Getters
	ushort			id()			const { return id_; }			// The graphical id of the item
	ushort			color()			const { return color_; }		// The Color of the item
	ushort			amount()		const { return amount_; }		// Amount of items in pile
	ushort			restock()		const { return restock_; }		// Amount of items a vendor will respawn this item to.
	QString			name2()			const { return name2_; }		// The identified name of the item
	uchar			layer()			const { return layer_; }		// Layer if equipped on paperdoll
	bool			twohanded()		const { return priv_&0x20; }		// Is the weapon twohanded ?
	UI32			type()			const { return type_; }			// Used for hardcoded behaviour
	UI32			type2()			const { return type2_; }
	uchar			offspell()		const { return offspell_; } 
	bool			secured()		const { return priv_&0x08; }		// Is the container secured (houses)
	SI16			speed()			const { return speed_; }		// Weapon speed
	SI16			lodamage()		const { return lodamage_; }		// Minimum damage weapon inflicts
	SI16			hidamage()		const { return hidamage_; }		// Maximum damage weapon inflicts
	bool			wipe()			const { return priv_&0x10; }		// Should the item be wiped when affected by /WIPE
	SI16			weight()		const { return weight_; }
	SI16			stones()		const { return (SI16)( weight_ / 10 ); } // Weight transformed to UO Stones
	SI16			hp()			const { return hp_; }			// Number of hitpoints an item has
	SI16			maxhp()			const { return maxhp_; }		// Maximum number of hitpoints an item has
	bool			corpse()		const { return priv_&0x40; }		// Is the item a corpse
	bool			newbie()		const { return priv_&0x02; }		// Is the Item Newbie
	bool			nodecay()		const { return priv_&0x01; }		// Is the item protected from decaying
	P_CHAR			owner()			const;
	int				totalweight()	const { return totalweight_; }
	uint			antispamtimer() const { return antispamtimer_;}
	ushort			accuracy()		const { return accuracy_; }		// for weapons, could be used for certain tools too.
	cUObject*		container()		const { return container_; }
	int				sellprice()		const { return sellprice_; } // Price this item is being bought at by normal vendors
	int				buyprice()		const { return buyprice_; } // Price this item is being sold at by normal vendors

	uchar			more1()			const { return more1_; }
	uchar			more2()			const { return more2_; }
	uchar			more3()			const { return more3_; }
	uchar			more4()			const { return more4_; }
	uint			morex()			const { return morex_; }
	uint			morey()			const { return morey_; }
	uint			morez()			const { return morez_; }	
	uchar			doordir()		const { return doordir_; }
	uchar			dooropen()		const { return dooropen_; }
	uchar			dye()			const { return dye_; }
	uint			att()			const { return att_; }
	uint			def()			const { return def_; }
	short			strengthReq()	const { return st_; }
	short			strengthMod()	const { return st2_; }
	short			dexterityReq()	const { return dx_; }
	short			dexterityMod()	const { return dx2_; }
	short			intelligenceReq()	const { return in_; }
	short			intelligenceMod()	const { return in2_; }
	uchar			magic()			const { return magic_; }
	uint			decaytime()		const { return decaytime_; }
	uint			disabled()		const { return disabled_; } 
	uint			poisoned()		const { return poisoned_; }
	int				rank()			const { return rank_; } 
	QString			creator()		const { return creator_;}
	uchar			visible()		const { return visible_;}
	uchar			priv()			const { return priv_;	}
	int				good()			const { return good_;	}
	int				rndvaluerate()  const { return rndvaluerate_; }
	int				madewith()		const { return madewith_;	}

//***************************END ADDED GETTERS************

//----------------------[ ADVANCED PROPERTIES GETTERS ]------------------

	short	regen( ushort id )		const { return regen_[ id ]; }			// Regeneration value
	short	bonus( ushort id )		const { return statsbonus_[ id ]; }		// Stats bonuses
	short	damage( ushort id )		const { return damage_[ id ]; }			// Damage modifiers %
	short	enh( ushort id )		const { return enhancement_[ id ]; }	// Misc enhancements %
	short	hit( ushort id )		const { return hit_[ id ]; }			// Attack and defence modifiers %
	short	req( ushort id )		const { return requirements_[ id ]; }   // Reagent, Mana and other requirements %
	short	resist( ushort id )		const { return resist_[ id ]; }			// Resisting damage %
	short	reflect( ushort id )	const { return reflect_[ id ]; }		// Reflecting damage %
	
	short	charge_count()			const { return charge_count_; }
	short	charge_spell()			const { return charge_spell_; }

	ushort	drb_base()				const { return drb_base_; }
	ushort	drb_current()			const { return drb_current_; }
	ushort	uses_base()				const { return uses_base_; }
	ushort	uses_current()			const { return uses_current_; }


//------------------[ END OF ADVANCED PROPERTIES GETTERS ]---------------

	// Setters
	void	setId( ushort nValue ) { id_ = nValue; flagChanged();};
	void	setColor( ushort nValue ) { color_ = nValue; flagChanged();};
	void	setAmount( ushort nValue );
	void	setRestock( ushort nValue ) { restock_ = nValue; flagChanged();}
	void	setName2( const QString& nValue ) { name2_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setLayer( uchar nValue ) { layer_ = nValue; flagChanged();};
	void	setTwohanded( bool nValue ) { nValue ? priv_ &= 0x20 : priv_ |= 0xDF; flagChanged(); changed( TOOLTIP );};
	void	setType( UI32 nValue ) { type_ = nValue; flagChanged();};
	void	setType2( UI32 nValue ) { type2_ = nValue; flagChanged();};	
	void	setOffspell( uchar nValue ) { offspell_ = nValue; flagChanged();};
	void	setSecured( bool nValue ) { ( nValue ) ? priv_ &= 0x08 : priv_ |= 0xF7; flagChanged(); changed( TOOLTIP );};
	void	setSpeed( SI16 nValue ) { speed_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setHidamage( SI16 nValue ) { hidamage_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setLodamage( SI16 nValue ) { lodamage_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setWipe( bool nValue ) { ( nValue ) ? priv_ &= 0x10 : priv_ |= 0xEF; flagChanged();};
	void	setNoDecay( bool nValue ) { ( nValue ) ? priv_ &= 0x01 : priv_ |= 0xFE; flagChanged(); };
	void	setWeight( SI16 nValue );
	void	setHp( SI16 nValue ) { hp_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setMaxhp( SI16 nValue ) { maxhp_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setCorpse( bool nValue ) { ( nValue ) ? priv_ |= 0x40 : priv_ &= 0xBF; flagChanged(); changed( TOOLTIP );}
	void	setNewbie( bool nValue ) { ( nValue ) ? priv_ |= 0x02 : priv_ &= 0xFD; flagChanged(); changed( TOOLTIP );}
	void	setOwner( P_CHAR nOwner );
	void	setTotalweight( int data );
	void	setAntispamtimer ( uint data ) { antispamtimer_ = data; flagChanged();}
	void	setAccuracy( ushort data ) { accuracy_ = data; flagChanged();}
	void	setCreator( const QString& d )	{ creator_ = d;	flagChanged(); changed( TOOLTIP );}

	cItem();
	cItem( const cItem& src); // Copy constructor
	static void registerInFactory();

	bool wearOut(); // The item wears out and true is returned if it's destroyed
	void	toBackpack( P_CHAR pChar );
	void	showName( cUOSocket *socket );
	void	applyRank( uchar rank );
//*****************************************ADDED SETTERS ***************
	void	setMore1( uchar data ) { more1_ = data; flagChanged();}
	void	setMore2( uchar data ) { more2_ = data; flagChanged();}
	void	setMore3( uchar data ) { more3_ = data; flagChanged();}
	void	setMore4( uchar data ) { more4_ = data; flagChanged();}
	void	setMoreX( uint data ) { morex_ = data; flagChanged();}
	void	setMoreY( uint data ) { morey_ = data; flagChanged();}
	void	setMoreZ( uint data ) { morez_ = data; flagChanged();}
	void	setDoorDir( uchar data ) { doordir_ = data; flagChanged();}
	void	setDoorOpen( uchar data ) { dooropen_ = data; flagChanged();}
	void	setDye( uchar data ) { dye_ = data; flagChanged();}
	void	setAtt(	uint data ) { att_ = data; flagChanged();}
	void	setDef( uint data ) { def_ = data; 	flagChanged(); changed( TOOLTIP );}
	void	setStrengthReq( short data ) { st_ = data; flagChanged(); changed( TOOLTIP );}
	void	setStrengthMod( short data ) { st2_ = data; flagChanged(); changed( TOOLTIP );}
	void	setDexterityReq( short data ) { dx_ = data; flagChanged(); changed( TOOLTIP );}
	void	setDexterityMod( short data ) { dx2_ = data; flagChanged(); changed( TOOLTIP );}
	void	setIntelligenceReq( short data ) { in_ = data; flagChanged(); changed( TOOLTIP );}
	void	setIntelligenceMod( short data ) { in2_ = data; flagChanged(); changed( TOOLTIP );}
	void	setMagic( uchar data ) { magic_ = data; flagChanged(); changed( TOOLTIP );}
	void	setDecayTime( uint data ) { decaytime_ = data; }
	void	setBuyprice( int data ) { buyprice_ = data; flagChanged(); changed( TOOLTIP );}
	void	setSellprice( int data ) { sellprice_ = data; flagChanged(); changed( TOOLTIP );}

	void	setDisabled(uint data) { disabled_ = data; flagChanged();}
	void	setPoisoned(uint data) { poisoned_ = data; flagChanged();}
	void	setRank(int data) { rank_ = data; flagChanged();} 
	void	setVisible( uchar d ) { visible_ = d; flagChanged();}
	void	setPriv( uchar d ) { priv_ = d; flagChanged(); changed( TOOLTIP );}
	void	setGood( int d ) { good_ = d;	flagChanged();}
	void	setRndValueRate( int d ) { rndvaluerate_ = d; flagChanged();}
	void	setMadeWith( int d )	{ madewith_ = d; flagChanged(); changed( TOOLTIP );}
	void	setContainer( cUObject* d ) { container_ = d; flagChanged(); }

//*******************************************END ADDED SETTERS**********

//----------------------[ ADVANCED PROPERTIES SETTERS ]------------------

	void	setRegen( ushort id, short data ){ regen_.replace( id, data ); }		// Regeneration value
	void	setBonus( ushort id, short data ){ statsbonus_.replace( id, data ); }	// Stats bonuses
	void	setDamage( ushort id, short data ){ damage_.replace( id, data ); }		// Damage modifiers %
	void	setEnh( ushort id, short data ){ enhancement_.replace( id, data ); }		// Misc enhancements %
	void	setHit( ushort id, short data ){ hit_.replace( id, data ); }				// Attack and defence modifiers %
	void	setReq( ushort id, short data ){ requirements_.replace( id, data ); }	// Reagent, Mana and other requirements %
	void	setResist( ushort id, short data ){ resist_.replace( id, data ); }		// Resisting damage %
	void	setReflect( ushort id, short data ){ reflect_.replace( id, data ); }		// Reflecting damage %

	void 	setChargeCount( short data ) { charge_count_ = data; }
	void	setChargeSpell( short data ) { charge_spell_ = data; }

	void	setDrb_base( ushort data ) { drb_base_ = data; }
	void	setDrb_current( ushort data ) { drb_current_ = data; }
	void	setUses_base( ushort data ) { uses_base_ = data; }
	void	setUses_current( ushort data ) { uses_current_ = data; }

//------------------[ END OF ADVANCED PROPERTIES SETTERS ]---------------

	SERIAL spawnserial;

	bool incognito; //AntiChrist - for items under incognito effect
	// ^^ NUTS !! - move that to priv

	uint time_unused;     // LB -> used for house decay and possibly for more in future, gets saved
	uint timeused_last; // helper attribute for time_unused, doesnt get saved
	
	virtual void Init( bool mkser = true );
	void setSerial(SERIAL ser);
	bool isInWorld() const			{ return (!container_); }
	bool isMulti() const				{ return ( id_ >= 0x4000 ); }
	bool isPileable();
	
	void setOwnSerialOnly(int ownser);
	void SetOwnSerial(int ownser);
	int ownSerial() const			{return ownserial_;}
	
	void SetSpawnSerial(long spawnser);
	void SetMultiSerial(long mulser);
	
	bool isShield() const { return type_ == 1009; }
	UINT16 getWeaponSkill();

	void MoveTo(int newx, int newy, signed char newz);
	long reduceAmount( short amount = 1 );
	short GetContGumpType();
	void SetRandPosInCont(cItem* pCont);
	bool PileItem(cItem* pItem);
	bool ContainerPileItem(cItem* pItem);	// try to find an item in the container to stack with
	void addItem(cItem* pItem, bool randomPos = true, bool handleWeight = true, bool noRemove = false ); // Add Item to container
	void removeItem(cItem*, bool handleWeight = true );
	void removeFromCont( bool handleWeight = true );
	ContainerContent content() const;
	bool contains( const cItem* ) const;
	int  CountItems(short ID, short col= -1) const;
	int  DeleteAmount(int amount, ushort _id, ushort _color = 0);
	QString getName( bool shortName = false );
	void startDecay();
	void setAllMovable()		{this->magic_=1; flagChanged();} // set it all movable..
	bool isAllMovable()         {return (magic_==1);}
	void setGMMovable()		    {this->magic_=2; flagChanged();} // set it GM movable.
	bool isGMMovable()          {return (magic_==2);}
	void setOwnerMovable()		{this->magic_=3; flagChanged();} // set it owner movable.
	bool isOwnerMovable()       {return (magic_==3);}
	void setLockedDown()        {this->magic_=4; flagChanged();} // set it locked down.
	bool isLockedDown()			{return (magic_==4);}

	// Public event wrappers added by darkstorm
	virtual bool onSingleClick( P_PLAYER Viewer );
	bool onDropOnChar( P_CHAR pChar );
	bool onDropOnItem( P_ITEM pItem );
	bool onDropOnGround( const Coord_cl &pos );
	bool onPickup( P_CHAR pChar );
	bool onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip ); // Shows a tool tip for specific object
	
	QPtrList< cItem > getContainment() const;

	P_ITEM getOutmostItem();
	P_CHAR getOutmostChar();

	virtual stError *setProperty( const QString &name, const cVariant &value );
	virtual stError *getProperty( const QString &name, cVariant &value ) const;

////
	virtual void flagUnchanged() { cItem::changed_ = false; cUObject::flagUnchanged(); }
	static P_ITEM createFromScript( const QString& Section );
	static P_ITEM createFromId( unsigned short id );
	void respawn( unsigned int currenttime );
	void decay( unsigned int currenttime );
	void remove();

protected:
	// Methods
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );
	virtual void	processNode( const cElement *Tag );
	void	processModifierNode( const cElement *Tag );

	// Data
	ushort		id_;
	ushort		color_;
	ushort		amount_; 
	ushort		restock_;
	QString		name2_;
	uchar		layer_;
	SI16		lodamage_; 
	SI16		hidamage_; 
	ushort		type_;
	ushort		type2_;
	uchar		offspell_; // Whats that for ?!
	SI16		speed_;
	SI16		weight_;
	SI16		hp_;
	SI16		maxhp_;
	int			totalweight_;
	uint		antispamtimer_;
	ushort		accuracy_;	// for weapons, could be used for certain tools too.
	int			sellprice_;
	int			buyprice_;

	// More values
	ContainerContent content_;
	cUObject*	container_;
	uchar		more1_; // For various stuff
	uchar		more2_;
	uchar		more3_;
	uchar		more4_;
	uint		morex_;
	uint		morey_;
	uint		morez_;
	uchar		doordir_; // Reserved for doors
	uchar		dooropen_;
	uchar		dye_; // Reserved: Can item be dyed by dye kit
	uint		att_; // Item attack
	uint		def_; // Item defense
	short		st_; // The strength needed to equip the item
	short		st2_; // The strength the item gives
	short		dx_; // The dexterity needed to equip the item
	short		dx2_; // The dexterity the item gives
	short		in_; // The intelligence needed to equip the item
	short		in2_; // The intelligence the item gives
	uchar		magic_; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable, 4=Locked Down
	uint		decaytime_;
	uint		disabled_; //Item is disabled, cant trigger.
	uint		poisoned_; //AntiChrist -- for poisoning skill
	int			rank_; //Magius(CHE) --- for rank system, this value is the LEVEL of the item from 1 to 10. Simply multiply the rank*10 and calculate the MALUS this item has from the original.
	// for example: RANK 5 ---> 5*10=50% of malus
	//   this item has same values decreased by 50%..
	// RANK 1 ---> 1*10=10% this item has 90% of malus!
	// RANK 10 --> 10*10=100% this item has no malus! RANK 10 is automatically setted if you select RANKSYSTEM 0.
	// Vars: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
	QString		creator_; // Store the name of the player made this item
	SERIAL		ownserial_;
	uchar		visible_; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible

	// Bit | Hex | Description
	//===================
	//   0 |  01 | NoDecay
	//   1 |  02 | Newbie
	//   2 |  04 | Dispellable
	//   3 |  08 | Secured (Chests)
	//   4 |  10 | Wipeable (/WIPE affects the item)
	//   5 |  20 | Twohanded
	//   6 |  40 | Corpse
	//   7 |  80 | <unused>
	uchar		priv_;

	int			good_; // Store type of GOODs to trade system! (Plz not set as UNSIGNED)  --- Magius(CHE)
	int			rndvaluerate_; // Store the value calculated base on RANDOMVALUE in region.scp. ---- MAgius(CHE) (2)
	uchar		madewith_; // Store the skills used to make this item -- Magius(CHE)

	// ADVANCED ITEM PROPERTIES
	// Charges
	ushort		charge_count_;	//Charges count
	ushort		charge_spell_;	//Charge spell

	// Durability
	ushort		drb_base_;		//Durability base
	ushort		drb_current_;   //Durability current
	ushort		uses_base_;		//Uses base
	ushort		uses_current_;	//Uses current

	// Regenerations
	// Mana, Stamina, Hit points
	QMap< ushort, short > regen_;

	// Stats bonuses 
	// Dexterity, Intelligence, Strength, Hit points, Mana, Stamina
	QMap< ushort, short > statsbonus_;

	// Damage increase % 
	// Increase, Physical, Cold, Fire, Poison, Energy, Spell
	QMap< ushort, short > damage_;

	// Enhancements %
	// Defence chance, Gold increase, Swing speed increase
	// Enhance potions, Self repair, Faster casting, Faster cast recovery
	QMap< ushort, short > enhancement_;
	
	// Hit modifiers %
	// Chance increase,	Cold,			Dispel,			Energy, 
	// Fire,			Fireball,		Harm,			Life leech,
	// Lighting,		Lower attack,	Lower defence,	Magic arrow, 
	// Mana leech,		Physical area,	Poison area,	Stamina leech
	QMap< ushort, short > hit_;
	
	// Lower requirements for resources and stats consumption %
	// Low mana cost, Low reagent cost, Low requirements (global)
	QMap< ushort, short > requirements_;
	
	// Resisting %
	// Cold resist, Energy resist, Fire resist
	// Physical resist, Poison resist
	QMap< ushort, short > resist_;

	// Reflect %
	// Reflect physical
	QMap< ushort, short > reflect_;

};

#endif
