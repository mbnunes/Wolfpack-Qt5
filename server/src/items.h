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

#if !defined( __ITEMS_H )
#define __ITEMS_H

// Wolfpack Includes
#include "uobject.h"
#include "defines.h"

// Library Includes
#include <qvaluevector.h>

// Forward Class declarations
class ISerialization;
class cUOSocket;

class cItem : public cUObject
{
	Q_OBJECT
	friend class cChar; // temporary
	bool changed_;
public:
	typedef QValueVector<cItem*> ContainerContent;
protected:
	UI16		id_;
	UI16		color_;
	UI16		amount_; 
	UI16		restock_;
	UI16		amount2_; 
	QString		name_;
	QString		name2_;
	SI08		layer_;
	QString		murderer_;
	SI16		lodamage_; 
	SI16		hidamage_; 
	UI16		type_;
	UI16		type2_;
	UI08		offspell_; // Whats that for ?!
	SI16		speed_;
	SI16		weight_;
	SI16		hp_;
	SI16		maxhp_;
	QString		spawnregion_;
	INT32		totalweight_;
	QString		carve_;
	unsigned int	antispamtimer_;
	UI16		accuracy_;	// for weapons, could be used for certain tools too.
	INT32		sellprice_;
	INT32		buyprice_;
	INT32		price_; // This price is only used for player vendor items

	// More values
	UI08 moreb1_;
	UI08 moreb2_;
	UI08 moreb3_;
	UI08 moreb4_;
	ContainerContent content_;
	cUObject *container_;

//******************** ADDED FROM PUBLIC *******************
	

	unsigned char more1_; // For various stuff
	unsigned char more2_;
	unsigned char more3_;
	unsigned char more4_;
	unsigned int morex_;
	unsigned int morey_;
	unsigned int morez_;
	unsigned char doordir_; // Reserved for doors
	unsigned char dooropen_;
	unsigned char dye_; // Reserved: Can item be dyed by dye kit
	unsigned int att_; // Item attack
	unsigned int def_; // Item defense
	signed short st_; // The strength needed to equip the item
	signed short st2_; // The strength the item gives
	signed short dx_; // The dexterity needed to equip the item
	signed short dx2_; // The dexterity the item gives
	signed short in_; // The intelligence needed to equip the item
	signed short in2_; // The intelligence the item gives
	unsigned char magic_; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable, 4=Locked Down
	unsigned int gatetime_;
	int gatenumber_;
	unsigned int decaytime_;
	unsigned int disabled_; //Item is disabled, cant trigger.
	QString disabledmsg_; //Item is disabled, so display this message. -- added by Magius(CHE) §
	unsigned int poisoned_; //AntiChrist -- for poisoning skill
	long int murdertime_; //AntiChrist -- for corpse -- when the people has been killed
	int rank_; //Magius(CHE) --- for rank system, this value is the LEVEL of the item from 1 to 10. Simply multiply the rank*10 and calculate the MALUS this item has from the original.
	// for example: RANK 5 ---> 5*10=50% of malus
	//   this item has same values decreased by 50%..
	// RANK 1 ---> 1*10=10% this item has 90% of malus!
	// RANK 10 --> 10*10=100% this item has no malus! RANK 10 is automatically setted if you select RANKSYSTEM 0.
	// Vars: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
	unsigned char dir_;
	QString desc_;
	QString creator_; // Store the name of the player made this item
	SERIAL ownserial_;
	uchar visible_; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	uchar priv_;
	int good_; // Store type of GOODs to trade system! (Plz not set as UNSIGNED)  --- Magius(CHE)
	int rndvaluerate_; // Store the value calculated base on RANDOMVALUE in region.scp. ---- MAgius(CHE) (2)
	uchar madewith_; // Store the skills used to make this item -- Magius(CHE)


//********************END ADDED FROM PUBLIC *************

	virtual void	processNode( const QDomElement &Tag );
	void	processModifierNode( const QDomElement &Tag );
public:

	virtual void	talk( const QString &message, UI16 color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	void load( char **, UINT16& );
	void save();
	bool del();

	void	processContainerNode( const QDomElement &Tag );
	virtual void update( cUOSocket *mSock = NULL );
	P_ITEM	dupe();
	void	soundEffect( UINT16 sound );

	// Getters
	UI16			id()			const { return id_; }			// The graphical id of the item
	UI16			color()			const { return color_; }		// The Color of the item
	UI16			amount()		const { return amount_; }		// Amount of items in pile
	UI16			restock()		const { return restock_; }		// Amount of items a vendor will respawn this item to.
	UI16			amount2()		const { return amount2_; }		// Used to track things like number of yards left in a roll of cloth
	const QString	&name2()		const { return name2_; }		// The identified name of the item
	const QString	&name()			const { return name_; }		// The identified name of the item
	UI08			layer()			const { return layer_; }		// Layer if equipped on paperdoll
	bool			twohanded()		const { return priv_&0x20; }		// Is the weapon twohanded ?
	const QString	&murderer()		const { return murderer_; }		// If it's a corpse, this holds the name of the murderer
	UI32			type()			const { return type_; }			// Used for hardcoded behaviour
	UI32			type2()			const { return type2_; }
	UI08			offspell()		const { return offspell_; } 
	bool			secured()		const { return priv_&0x08; }		// Is the container secured (houses)
	SI16			speed()			const { return speed_; }		// Weapon speed
	SI16			lodamage()		const { return lodamage_; }		// Minimum damage weapon inflicts
	SI16			hidamage()		const { return hidamage_; }		// Maximum damage weapon inflicts
	bool			wipe()			const { return priv_&0x10; }		// Should the item be wiped when affected by /WIPE
	SI16			weight()		const { return weight_; }
	SI16			stones()		const { return (SI16)( weight_ / 10 ); } // Weight transformed to UO Stones
	SI16			hp()			const { return hp_; }			// Number of hitpoints an item has
	SI16			maxhp()			const { return maxhp_; }		// Maximum number of hitpoints an item has
	QString			spawnregion()	const { return spawnregion_; }
	UI08			moreb1()		const { return moreb1_; }
	UI08			moreb2()		const { return moreb2_; }
	UI08			moreb3()		const { return moreb3_; }
	UI08			moreb4()		const { return moreb4_; }
	bool			corpse()		const { return priv_&0x40; }		// Is the item a corpse
	bool			newbie()		const { return priv_&0x02; }		// Is the Item Newbie
	P_CHAR			owner() const;
	INT32			totalweight()	const { return totalweight_; }
	QString			carve()			const { return carve_; }
	unsigned int	antispamtimer() const { return antispamtimer_;}
	UI16			accuracy()		const { return accuracy_; }		// for weapons, could be used for certain tools too.
	cUObject		*container()    const { return container_; }
	INT32			sellprice()		const { return sellprice_; } // Price this item is being bought at by normal vendors
	INT32			buyprice()		const { return buyprice_; } // Price this item is being sold at by normal vendors
	INT32			price()			const { return price_; } // Price this item is being sold at by player vendors

	unsigned char	more1()			const { return more1_; }
	unsigned char	more2()			const { return more2_; }
	unsigned char	more3()			const { return more3_; }
	unsigned char	more4()			const { return more4_; }
	unsigned int	morex()			const { return morex_; }
	unsigned int	morey()			const { return morey_; }
	unsigned int	morez()			const { return morez_; }	
	unsigned char	doordir()		const { return doordir_; }
	unsigned char	dooropen()		const { return dooropen_; }
	unsigned char	dye()			const { return dye_; }
	unsigned int	att()			const { return att_; }
	unsigned int	def()			const { return def_; }
	signed short	st()			const { return st_; }
	signed short	st2()			const { return st2_; }
	signed short	dx()			const { return dx_; }
	signed short	dx2()			const { return dx2_; }
	signed short	in()			const { return in_; }
	signed short	in2()			const { return in2_; }
	unsigned char	magic()			const { return magic_; }
	unsigned int	gatetime()		const { return gatetime_; }
	int				gatenumber()	const { return gatenumber_; }
	unsigned int	decaytime()		const { return decaytime_; }
	unsigned int	disabled()		const { return disabled_; } 
	QString			disabledmsg()	const { return disabledmsg_; } 
	unsigned int	poisoned()		const { return poisoned_; }
	long int		murdertime()	const { return murdertime_; } 
	int				rank()			const { return rank_; } 
	uchar			direction()		const { return dir_;  }
	QString			description()	const { return desc_; }
	QString			creator()		const { return creator_;}
	uchar			visible()		const { return visible_;}
	uchar			priv()			const { return priv_;	}
	int				good()			const { return good_;	}
	int				rndvaluerate()  const { return rndvaluerate_; }
	int				madewith()		const { return madewith_;	}

//***************************END ADDED GETTERS************


	// Setters
	void	setId( UI16 nValue ) { id_ = nValue; changed_ = true;};
	void	setColor( UI16 nValue ) { color_ = nValue; changed_ = true;};
	void	setAmount( UI16 nValue );
	void	setRestock( UI16 nValue ) { restock_ = nValue; changed_ = true;}
	void	setAmount2( UI16 nValue ) { amount2_ = nValue; changed_ = true;}; //Used to track things like number of yards left in a roll of cloth
	void	setName( const QString& nValue ) { name_ = nValue; changed_ = true;};
	void	setName2( const QString& nValue ) { name2_ = nValue; changed_ = true;};
	void	setLayer( SI08 nValue ) { layer_ = nValue; changed_ = true;};
	void	setTwohanded( bool nValue ) { nValue ? priv_ &= 0x20 : priv_ |= 0xDF; changed_ = true;};
	void	setMurderer( const QString& nValue ) { murderer_ = nValue; changed_ = true;};
	void	setType( UI32 nValue ) { type_ = nValue; changed_ = true;};
	void	setType2( UI32 nValue ) { type2_ = nValue; changed_ = true;};	
	void	setOffspell( UI08 nValue ) { offspell_ = nValue; changed_ = true;};
	void	setSecured( bool nValue ) { ( nValue ) ? priv_ &= 0x08 : priv_ |= 0xF7; changed_ = true;};
	void	setSpeed( SI16 nValue ) { speed_ = nValue; changed_ = true;};
	void	setHidamage( SI16 nValue ) { hidamage_ = nValue; changed_ = true;};
	void	setLodamage( SI16 nValue ) { lodamage_ = nValue; changed_ = true;};
	void	setWipe( bool nValue ) { ( nValue ) ? priv_ &= 0x10 : priv_ |= 0xEF; changed_ = true;};
	void	setWeight( SI16 nValue );
	void	setHp( SI16 nValue ) { hp_ = nValue; changed_ = true;};
	void	setMaxhp( SI16 nValue ) { maxhp_ = nValue; changed_ = true;};
	void	setSpawnRegion( const QString& nValue ) { spawnregion_ = nValue; changed_ = true;};
	void	setMoreb1( UI08 nValue ) { moreb1_ = nValue; changed_ = true;};
	void	setMoreb2( UI08 nValue ) { moreb2_ = nValue; changed_ = true;};
	void	setMoreb3( UI08 nValue ) { moreb3_ = nValue; changed_ = true;};
	void	setMoreb4( UI08 nValue ) { moreb4_ = nValue; changed_ = true;};
	void	setCorpse( bool nValue ) { ( nValue ) ? priv_ |= 0x40 : priv_ &= 0xBF; changed_ = true;}
	void	setNewbie( bool nValue ) { ( nValue ) ? priv_ |= 0x02 : priv_ &= 0xFD; changed_ = true;}
	void	setOwner( P_CHAR nOwner );
	void	setTotalweight( INT32 data );
	void	setCarve( const QString& data ) { carve_ = data; changed_ = true;}
	void	setAntispamtimer ( unsigned int data ) { antispamtimer_ = data; changed_ = true;}
	void	setAccuracy( UI16 data ) { accuracy_ = data; changed_ = true;}
	void	setDirection( uchar d )	 { dir_ = d; changed_ = true;}
	void	setDescription( const QString& d ) { desc_ = d; changed_ = true;}
	void	setCreator( const QString& d )	{ creator_ = d;	changed_ = true;}

	cItem();
	cItem( const cItem& src); // Copy constructor
	virtual QString objectID() const;
	static void registerInFactory();

	bool wearOut(); // The item wears out and true is returned if it's destroyed
	P_ITEM	getCorpse( void ); // Get the corpse this item is in
	void	toBackpack( P_CHAR pChar );
	void	showName( cUOSocket *socket );
	void	applyRank( UI08 rank );
//*****************************************ADDED SETTERS ***************
	void	setMore1( unsigned char data ) { more1_ = data; changed_ = true;}
	void	setMore2( unsigned char data ) { more2_ = data; changed_ = true;}
	void	setMore3( unsigned char data ) { more3_ = data; changed_ = true;}
	void	setMore4( unsigned char data ) { more4_ = data; changed_ = true;}
	void	setMoreX( unsigned int data ) { morex_ = data; changed_ = true;}
	void	setMoreY( unsigned int data ) { morey_ = data; changed_ = true;}
	void	setMoreZ( unsigned int data ) { morez_ = data; changed_ = true;}
	void	setDoorDir( unsigned char data ) { doordir_ = data; changed_ = true;}
	void	setDoorOpen( unsigned char data ) { dooropen_ = data; changed_ = true;}
	void	setDye( unsigned char data ) { dye_ = data; changed_ = true;}
	void	setAtt(	unsigned int data ) { att_ = data; changed_ = true;}
	void	setDef( unsigned int data ) { def_ = data; changed_ = true;}
	void	setSt( signed short data ) { st_ = data; changed_ = true;}
	void	setSt2( signed short data ) { st2_ = data; changed_ = true;}
	void	setDx( signed short data ) { dx_ = data; changed_ = true;}
	void	setDx2( signed short data ) { dx2_ = data; changed_ = true;}
	void	setIn( signed short data ) { in_ = data; changed_ = true;}
	void	setIn2( signed short data ) { in2_ = data; changed_ = true;}
	void	setMagic( unsigned char data ) { magic_ = data; changed_ = true;}
	void	setGateTime( unsigned int data ) { gatetime_ = data; changed_ = true;}
	void	setGateNumber( int data ) { gatenumber_ = data; changed_ = true;}
	void	setDecayTime( unsigned int data ) { decaytime_ = data; changed_ = true;}
	void	setPrice( INT32 data ) { price_ = data; changed_ = true;}
	void	setBuyprice( INT32 data ) { buyprice_ = data; changed_ = true;}
	void	setSellprice( INT32 data ) { sellprice_ = data; changed_ = true;}

	void	setDisabled(unsigned int data) { disabled_ = data; changed_ = true;}
	void	setDisabledMsg(QString data) { disabledmsg_ = data; changed_ = true;}
	void	setPoisoned(unsigned int data) { poisoned_ = data; changed_ = true;}
	void	setMurderTime(long int data) { murdertime_ = data; changed_ = true;}
	void	setRank(int data) { rank_ = data; changed_ = true;} 
	void	setVisible( uchar d ) { visible_ = d; changed_ = true;}
	void	setPriv( uchar d ) { priv_ = d; changed_ = true;}
	void	setGood( int d ) { good_ = d;	changed_ = true;}
	void	setRndValueRate( int d ) { rndvaluerate_ = d; changed_ = true;}
	void	setMadeWith( int d )	{ madewith_ = d; changed_ = true;}
	void	setContainer( cUObject* d ) { container_ = d; changed_ = true; }

//*******************************************END ADDED SETTERS**********

	SERIAL spawnserial;

	// Bit | Hex | Description
	//===================
	//   0 |  01 | Decay
	//   1 |  02 | Newbie
	//   2 |  04 | Dispellable
	//   3 |  08 | Secured (Chests)
	//   4 |  10 | Wipeable (/WIPE affects the item)
	//   5 |  20 | Twohanded
	//   6 |  40 | Corpse
	//   7 |  80 | <unused>
	
	// Note by Magius: Value range to -ALLSKILLS-1 to ALLSKILLS+1
	//    To calculate skill used to made this item:
	//       if is a positive value, substract 1 it.
	//          Ex) madewith=34 , 34-1=33 , 33=STEALING
	//       if is a negative value, add 1 from it and invert value.
	//          Ex) madewith=-34 , -34+1=-33 , Abs(-33)=33=STEALING.
	//       0 = NULL
	//    So... a positive value is used when the item is made by a
	//       player with 95.0+ at that skill. Infact in this way when
	//       you click on the item appear its name and the name of the
	//       creator. A Negative value if the player is not enought
	//       skilled!
	
	bool incognito; //AntiChrist - for items under incognito effect
	// ^^ NUTS !! - move that to priv

	unsigned int time_unused;     // LB -> used for house decay and possibly for more in future, gets saved
	unsigned int timeused_last; // helper attribute for time_unused, doesnt get saved
	
	virtual void Init( bool mkser = true );
	void setSerial(const SERIAL ser);
	bool isInWorld()			{ return (!container_); }
	bool isMulti()				{ return ( id_ >= 0x4000 ); }
	bool isPileable();
	
	void setOwnSerialOnly(long ownser);
	void SetOwnSerial(long ownser);
	long ownSerial() const			{return ownserial_;}
	
	void SetSpawnSerial(long spawnser);
	void SetMultiSerial(long mulser);
	
	bool isShield() { return type_ == 1009; }
	UINT16 getWeaponSkill();

	void MoveTo(int newx, int newy, signed char newz);
	long ReduceAmount(const short amount = 1);
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
	int  DeleteAmount(int amount, unsigned short _id, unsigned short _color = 0);
	QString getName( bool shortName = false );
	void startDecay();
	void setAllMovable()		{this->magic_=1; changed_ = true;} // set it all movable..
	bool isAllMovable()         {return (magic_==1);}
	void setGMMovable()		    {this->magic_=2; changed_ = true;} // set it GM movable.
	bool isGMMovable()          {return (magic_==2);}
	void setOwnerMovable()		{this->magic_=3; changed_ = true;} // set it owner movable.
	bool isOwnerMovable()       {return (magic_==3);}
	void setLockedDown()        {this->magic_=4; changed_ = true;} // set it locked down.
	bool isLockedDown()			{return (magic_==4);}

	// Public event wrappers added by darkstorm
	virtual bool onSingleClick( P_CHAR Viewer );
	bool onDropOnChar( P_CHAR pChar );
	bool onDropOnItem( P_ITEM pItem );
	bool onDropOnGround( const Coord_cl &pos );
	bool onPickup( P_CHAR pChar );
	
	QPtrList< cItem > getContainment() const;

	P_ITEM getOutmostItem();
	P_CHAR getOutmostChar();

	stError *setProperty( const QString &name, const cVariant &value );
	stError *getProperty( const QString &name, cVariant &value ) const;

////
	void flagUnchanged() { changed_ = false; cUObject::flagUnchanged(); }

protected:
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );
};


//forward declaration
class QDomElement;

class cAllItems
{
private:
	unsigned char PackType(unsigned char id1, unsigned char id2);
	
public:
	// Added by DarkStorm
	P_ITEM createScriptItem( const QString& Section ); // Creates an item from an item-section
	P_ITEM createListItem( QString Section ); // Creates an Item from an item-list

	cAllItems() {}
	void DeleItem(P_ITEM pi);
	char isFieldSpellItem(P_ITEM pi);
	P_ITEM SpawnItem(P_CHAR pc_ch,int nAmount, const char* cName, bool pileable, short id, short color, bool bPack);
	P_ITEM SpawnItemBank(P_CHAR pc_ch, QString nItem);
	P_ITEM  SpawnItemBackpack2(UOXSOCKET s, QString nItem, int nDigging);
	void DecayItem(unsigned int currenttime, P_ITEM pi);
	void RespawnItem(unsigned int Currenttime, P_ITEM pi);
	void AddRespawnItem(P_ITEM pItem, QString itemSect, bool spawnInItem);
	void CheckEquipment(P_CHAR pc_p); //AntiChrist
	void GetScriptItemSetting(P_ITEM pi); // by Magius(CHE)
};

inline QString cItem::objectID() const
{
        return "cItem";
}

#endif
