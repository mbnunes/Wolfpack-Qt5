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
private:
	bool changed_;
	void flagChanged() { changed_ = true; } // easier to debug, compiler should make it inline;

public:
	typedef QValueVector<cItem*> ContainerContent;

public:
	const char *objectID() const
	{
        return "cItem";
	}

	virtual void	talk( const QString &message, ushort color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	void load( char **, UINT16& );
	void save();
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
	uchar			layer()			const { return layer_; }		// Layer if equipped on paperdoll
	bool			twohanded()		const { return priv_&0x20; }		// Is the weapon twohanded ?
	UI32			type()			const { return type_; }			// Used for hardcoded behaviour
	UI32			type2()			const { return type2_; }
	bool			secured()		const { return priv_&0x08; }		// Is the container secured (houses)
	SI16			speed()			const { return speed_; }		// Weapon speed
	SI16			lodamage()		const { return lodamage_; }		// Minimum damage weapon inflicts
	SI16			hidamage()		const { return hidamage_; }		// Maximum damage weapon inflicts
	bool			wipe()			const { return priv_&0x10; }		// Should the item be wiped when affected by /WIPE
	SI16			weight()		const { return weight_; }
	SI16			stones()		const { return (SI16)( weight_ / 10 ); } // Weight transformed to UO Stones
	SI16			hp()			const { return hp_; }			// Number of hitpoints an item has
	SI16			maxhp()			const { return maxhp_; }		// Maximum number of hitpoints an item has
	bool			dye()			const { return priv_&0x80; }	// Can the item be dyed
	bool			corpse()		const { return priv_&0x40; }		// Is the item a corpse
	bool			newbie()		const { return priv_&0x02; }		// Is the Item Newbie
	bool			nodecay()		const { return priv_&0x01; }		// Is the item protected from decaying
	P_CHAR			owner()			const;
	int				totalweight()	const { return totalweight_; }
	uint			antispamtimer() const { return antispamtimer_;}
	cUObject*		container()		const { return container_; }
	int				sellprice()		const { return sellprice_; } // Price this item is being bought at by normal vendors
	int				buyprice()		const { return buyprice_; } // Price this item is being sold at by normal vendors

	uint			def()			const { return def_; }
	uchar			magic()			const { return magic_; }
	uint			decaytime()		const { return decaytime_; }
	uint			poisoned()		const { return poisoned_; }
	uchar			visible()		const { return visible_;}
	uchar			priv()			const { return priv_;	}

//***************************END ADDED GETTERS************

	// Setters
	void	setId( ushort nValue ) { id_ = nValue; flagChanged();};
	void	setColor( ushort nValue ) { color_ = nValue; flagChanged();};
	void	setAmount( ushort nValue );
	void	setRestock( ushort nValue ) { restock_ = nValue; flagChanged();}
	void	setLayer( uchar nValue ) { layer_ = nValue; flagChanged();};
	void	setTwohanded( bool nValue ) { nValue ? priv_ |= 0x20 : priv_ &= 0xDF; flagChanged(); changed( TOOLTIP );};
	void	setDye( bool nValue ) {  nValue ? priv_ |= 0x80 : priv_ &= 0x7F; flagChanged();}
	void	setType( UI32 nValue ) { type_ = nValue; flagChanged();};
	void	setType2( UI32 nValue ) { type2_ = nValue; flagChanged();};	
	void	setSecured( bool nValue ) { ( nValue ) ? priv_ |= 0x08 : priv_ &= 0xF7; flagChanged(); changed( TOOLTIP );};
	void	setSpeed( SI16 nValue ) { speed_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setHidamage( SI16 nValue ) { hidamage_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setLodamage( SI16 nValue ) { lodamage_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setWipe( bool nValue ) { ( nValue ) ? priv_ &= 0x10 : priv_ |= 0xEF; flagChanged();};
	void	setNoDecay( bool nValue ) { ( nValue ) ? priv_ |= 0x01 : priv_ &= ~0x01; flagChanged(); };
	void	setWeight( SI16 nValue );
	void	setHp( SI16 nValue ) { hp_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setMaxhp( SI16 nValue ) { maxhp_ = nValue; flagChanged(); changed( TOOLTIP );};
	void	setCorpse( bool nValue ) { ( nValue ) ? priv_ |= 0x40 : priv_ &= 0xBF; flagChanged(); changed( TOOLTIP );}
	void	setNewbie( bool nValue ) { ( nValue ) ? priv_ |= 0x02 : priv_ &= 0xFD; flagChanged(); changed( TOOLTIP );}
	void	setOwner( P_CHAR nOwner );
	void	setTotalweight( int data );
	void	setAntispamtimer ( uint data ) { antispamtimer_ = data; flagChanged();}

	cItem();
	cItem( const cItem& src); // Copy constructor
	static void registerInFactory();

	bool wearOut(); // The item wears out and true is returned if it's destroyed
	void	toBackpack( P_CHAR pChar );
	void	showName( cUOSocket *socket );
//*****************************************ADDED SETTERS ***************
	void	setDef( uint data ) { def_ = data; 	flagChanged(); changed( TOOLTIP );}
	void	setMagic( uchar data ) { magic_ = data; flagChanged(); changed( TOOLTIP );}
	void	setDecayTime( uint data ) { decaytime_ = data; }
	void	setBuyprice( int data ) { buyprice_ = data; flagChanged(); changed( TOOLTIP );}
	void	setSellprice( int data ) { sellprice_ = data; flagChanged(); changed( TOOLTIP );}

	void	setPoisoned(uint data) { poisoned_ = data; flagChanged();}
	void	setVisible( uchar d ) { visible_ = d; flagChanged();}
	void	setPriv( uchar d ) { priv_ = d; flagChanged(); changed( TOOLTIP );}
	void	setContainer( cUObject* d ) { container_ = d; flagChanged(); }

//*******************************************END ADDED SETTERS**********

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

	void MoveTo( int newx, int newy, signed char newz );
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
	bool onUse( P_CHAR pChar );
	bool onWearItem( P_PLAYER pPlayer, P_CHAR pChar, unsigned char layer );
	bool onEquip( P_CHAR pChar, unsigned char layer );
	bool onUnequip( P_CHAR pChar, unsigned char layer );
	bool onCollide( P_CHAR pChar );
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
	uchar		layer_;
	SI16		lodamage_; 
	SI16		hidamage_; 
	ushort		type_;
	ushort		type2_;
	SI16		speed_;
	SI16		weight_;
	SI16		hp_;
	SI16		maxhp_;
	int			totalweight_;
	uint		antispamtimer_;
	int			sellprice_;
	int			buyprice_;

	ContainerContent content_;
	cUObject*	container_;
	uint		def_; // Item defense
	uchar		magic_; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable, 4=Locked Down
	uint		decaytime_;
	uint		poisoned_; //AntiChrist -- for poisoning skill
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
	//   7 |  80 | Dye
	uchar		priv_;
};

#endif
