/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined( __ITEMS_H__ )
#define __ITEMS_H__

// Wolfpack Includes
#include "uobject.h"
#include "basedef.h"
#include "defines.h"
#include "network/uotxpackets.h"
#include "singleton.h"
#include "objectdef.h"
#include "content.h"

// Library Includes
#include <qvaluevector.h>
#include <qdict.h>
#include <math.h>

// Forward Class declarations
class ISerialization;
class cUOSocket;

/*
	Notes for further memory footprint reduction:

	b) Think about the maxhp/hp properties. They're not needed
	for most of the items.
*/
#pragma pack(1)
class cItem : public cUObject
{
	OBJECTDEF( cItem )
	friend class cBaseChar;
private:
	static unsigned char classid;
	unsigned char changed_ : 1;
	cItemBaseDef* basedef_;

	void flagChanged()
	{
		changed_ = true;
	} // easier to debug, compiler should make it inline;

public:
	PyObject* callEvent( ePythonEvent event, PyObject* args = 0, bool ignoreErrors = false );
	bool callEventHandler( ePythonEvent event, PyObject* args = 0, bool ignoreErrors = false );
	bool canHandleEvent( ePythonEvent event );
	bool hasScript( const QCString& name );

	cItem();
	cItem( const cItem& src ); // Copy constructor

	static void setClassid( unsigned char id )
	{
		cItem::classid = id;
	}

	unsigned char getClassid()
	{
		return cItem::classid;
	}

	inline const char* objectID() const
	{
		return "cItem";
	}

	bool isInLockedItem();
	virtual void talk( const QString& message, UI16 color = 0xFFFF, Q_UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	virtual void talk( const Q_UINT32 MsgID, const QString& params = 0, const QString& affix = 0, bool prepend = false, UI16 color = 0xFFFF, cUOSocket* socket = 0 );
	void load( char**, Q_UINT16& );
	void save();
	bool del();

	void load( cBufferedReader& reader, unsigned int version );
	void save( cBufferedWriter& reader, unsigned int version );
	void postload( unsigned int version );

	void processContainerNode( const cElement* Tag );
	virtual void update( cUOSocket* mSock = NULL );
	P_ITEM dupe();
	void soundEffect( Q_UINT16 sound );

	// Returns Zero if the item shouldn't decay
	virtual unsigned int decayDelay();

	void sendTooltip( cUOSocket* mSock );
	PyObject* getPyObject();
	const char* className() const;

	// Getters
	ushort id() const
	{
		return id_;
	} // The graphical id of the item
	ushort color() const
	{
		return color_;
	} // The Color of the item
	ushort amount() const
	{
		return amount_;
	} // Amount of items in pile
	ushort restock();
	uchar layer() const
	{
		return layer_;
	} // Layer if equipped on paperdoll
	bool twohanded() const
	{
		return priv_ & 0x20;
	} // Is the weapon twohanded ?
	bool secured() const
	{
		return priv_ & 0x08;
	} // Is the container secured (houses)
	bool allowMeditation() const
	{
		return priv_ & 0x10;
	} // Does this armor allow meditation?
	SI16 hp() const
	{
		return hp_;
	} // Number of hitpoints an item has
	SI16 maxhp() const
	{
		return maxhp_;
	} // Maximum number of hitpoints an item has
	bool dye() const
	{
		return priv_ & 0x80;
	} // Can the item be dyed
	virtual bool corpse() const
	{
		return false;
	} // Is the item a corpse
	bool newbie() const
	{
		return priv_ & 0x02;
	} // Is the Item Newbie
	bool unprocessed() const
	{
		return priv_ & 0x40;
	}
	bool nodecay() const
	{
		return priv_ & 0x01;
	} // Is the item protected from decaying
	P_CHAR owner() const;
	float totalweight() const
	{
		return totalweight_;
	}
	cUObject* container() const
	{
		return container_;
	}

	uchar movable() const
	{
		return movable_;
	}
	uchar visible() const
	{
		return visible_;
	}
	uchar priv() const
	{
		return priv_;
	}
	QCString baseid() const
	{
		return basedef_ ? basedef_->id() : 0;
	}
	inline cItemBaseDef* basedef() const
	{
		return basedef_;
	}
	inline void setBaseid( const QCString& id )
	{
		float oldweight = weight();
		basedef_ = ItemBaseDefs::instance()->get( id );
		float newweight = weight();

		if ( oldweight != newweight )
		{
			setTotalweight( totalweight_ + ( newweight - oldweight ) );
		}

		flagChanged();
	}

	void save( cBufferedWriter& writer );
	void load( cBufferedReader& reader );

	// Basedef Properties
	inline bool isWaterSource()
	{
		return basedef_ ? basedef_->isWaterSource() : false;
	}

	inline float weight()
	{
		return basedef_ ? basedef_->weight() : 0.0f;
	}

	inline unsigned char lightsource()
	{
		return basedef_ ? basedef_->lightsource() : 0;
	}

	inline unsigned int sellprice()
	{
		return basedef_ ? basedef_->sellprice() : 0;
	}

	inline unsigned int buyprice()
	{
		return basedef_ ? basedef_->buyprice() : 0;
	}

	inline unsigned short type()
	{
		return basedef_ ? basedef_->type() : 0;
	}

	virtual QCString bindmenu()
	{
		return basedef_ ? basedef_->bindmenu() : 0;
	}

	// Setters
	void setId( ushort nValue )
	{
		id_ = nValue; flagChanged();
	};
	void setColor( ushort nValue )
	{
		color_ = nValue; flagChanged();
	};
	void setAmount( ushort nValue );
	void setRestock( ushort nValue );
	void setLayer( uchar nValue )
	{
		layer_ = nValue; flagChanged();
	};
	void setTwohanded( bool nValue )
	{
		nValue ? priv_ |= 0x20 : priv_ &= 0xDF; flagChanged(); changed( TOOLTIP );
	};
	void setDye( bool nValue )
	{
		nValue ? priv_ |= 0x80 : priv_ &= 0x7F; flagChanged();
	}
	void setSecured( bool nValue )
	{
		( nValue ) ? priv_ |= 0x08 : priv_ &= 0xF7; flagChanged(); changed( TOOLTIP );
	};
	void setAllowMeditation( bool nValue )
	{
		( nValue ) ? priv_ &= 0x10 : priv_ |= 0xEF; flagChanged();
	};
	void setNoDecay( bool nValue )
	{
		( nValue ) ? priv_ |= 0x01 : priv_ &= ~0x01; flagChanged();
	};
	void setHp( SI16 nValue )
	{
		hp_ = nValue; flagChanged(); changed( TOOLTIP );
	};
	void setMaxhp( SI16 nValue )
	{
		maxhp_ = nValue; flagChanged(); changed( TOOLTIP );
	}
	void setNewbie( bool nValue )
	{
		( nValue ) ? priv_ |= 0x02 : priv_ &= ~0x02; flagChanged(); changed( TOOLTIP );
	}
	void setUnprocessed( bool nValue )
	{
		if ( nValue )
			priv_ |= 0x40;
		else
			priv_ &= ~0x40;
	}

	void setOwner( P_CHAR nOwner );
	void setTotalweight( float data );

	bool wearOut(); // The item wears out and true is returned if it's destroyed
	void toBackpack( P_CHAR pChar );
	void showName( cUOSocket* socket );
	unsigned int getSellPrice( P_CHAR vendor );

	void setMagic( uchar data )
	{
		movable_ = data; flagChanged(); changed( TOOLTIP );
	}
	void setVisible( uchar d )
	{
		visible_ = d; flagChanged();
	}
	void setPriv( uchar d )
	{
		priv_ = d; flagChanged(); changed( TOOLTIP );
	}
	void setContainer( cUObject* d );

	virtual void Init( bool mkser = true );
	void setSerial( SERIAL ser );
	bool isInWorld() const
	{
		return ( !container_ );
	}
	bool isMulti() const
	{
		return ( id_ >= 0x4000 );
	}
	bool isPileable();

	void setOwnSerialOnly( int ownser );
	void SetOwnSerial( int ownser );
	int ownSerial() const
	{
		return ownserial_;
	}

	bool isShield()
	{
		return type() == 1008;
	}
	Q_UINT16 getWeaponSkill();

	void moveTo( const Coord& pos );
	long reduceAmount( unsigned int amount = 1 );
	short containerGumpType() const;
	void setRandPosInCont( cItem* pCont );
	bool pileItem( cItem* pItem );
	bool containerPileItem( cItem* pItem ); // try to find an item in the container to stack with
	bool canStack( cItem* pItem ); // See if this item can stack with another.
	void addItem( cItem* pItem, bool randomPos = true, bool handleWeight = true, bool noRemove = false, bool autoStack = true ); // Add Item to container
	void removeItem( cItem*, bool handleWeight = true );
	void removeFromCont( bool handleWeight = true );
	const ContainerContent& content() const; // Return a reference to the container content object
	bool contains( const cItem* ) const;
	unsigned int countItems( const QStringList& baseids ) const;
	unsigned int countItems( short ID, short col = -1 ) const;

	unsigned int removeItems( const QStringList& baseids, unsigned int amount );
	void remove();

	int deleteAmount( int amount, ushort _id, ushort _color = 0 );
	QString getName( bool shortName = false );
	void setAllMovable()
	{
		this->movable_ = 1; flagChanged();
	} // set it all movable..
	bool isAllMovable()
	{
		return ( movable_ == 1 );
	}
	void setGMMovable()
	{
		this->movable_ = 2; flagChanged();
	} // set it GM movable.
	bool isGMMovable()
	{
		return ( movable_ == 2 );
	}
	void setOwnerMovable()
	{
		this->movable_ = 3; flagChanged();
	} // set it owner movable.
	bool isOwnerMovable()
	{
		return ( movable_ == 3 );
	}
	void setLockedDown()
	{
		this->movable_ = 4; flagChanged();
	} // set it locked down.
	bool isLockedDown()
	{
		return ( movable_ == 4 );
	}

	// Public event wrappers added by darkstorm
	virtual bool onSingleClick( P_PLAYER Viewer );
	bool onUse( P_CHAR pChar );
	bool onWearItem( P_PLAYER pPlayer, P_CHAR pChar, unsigned char layer );
	bool onEquip( P_CHAR pChar, unsigned char layer );
	bool onUnequip( P_CHAR pChar, unsigned char layer );
	bool onCollide( P_CHAR pChar );
	bool onDropOnChar( P_CHAR pChar );
	bool onDropOnItem( P_ITEM pItem );
	bool onDropOnGround( const Coord& pos );
	bool onPickup( P_CHAR pChar );
	//	bool onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip ); // Shows a tool tip for specific object

	QPtrList< cItem > getContainment() const;

	P_ITEM getOutmostItem();
	P_CHAR getOutmostChar();
	Coord getOutmostPos();

	void createTooltip( cUOTxTooltipList& tooltip, cPlayer* player );
	virtual stError* setProperty( const QString& name, const cVariant& value );
	virtual PyObject* getProperty( const QString& name );

	////
	virtual void flagUnchanged()
	{
		changed_ = false;
		cUObject::flagUnchanged();
	}

	static P_ITEM createFromScript( const QString& section );
	static P_ITEM createFromList( const QString& list );
	static P_ITEM createFromId( unsigned short id );

	static void buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions );

protected:
	// Methods
	virtual void processNode( const cElement* Tag );
	void processModifierNode( const cElement* Tag );

	unsigned short id_; // Display id of the item
	unsigned short color_; // Color of this item (Note: only colors up to 0xBFF available -> 12 bit)
	unsigned short amount_; // Amount of this item

	unsigned char layer_ : 5; // The layer this item is equipped on /*BASEDEF*/ (for now -> 5 bit)
	unsigned short hp_; // Amount of hitpoints this item has
	unsigned short maxhp_; // The maximum amount of hitpoints this item can have
	float totalweight_; // The weight of this item including all contained items
	ContainerContent content_; // The content of this item
	cUObject* container_; // The object this item is contained in

	/*
		Specifies in which manner this item can be moved. (3 bit)
		0: This property is ignored
		1: This item is always movable
		2: This item cannot be moved
		3: This item can only be moved by it's owner
		4: This item has been locked down
	*/
	unsigned char movable_ : 3;

	SERIAL ownserial_; // This property specifies the owner of this item. If it is INVALID_SERIAL, this item has no owner

	/*
		This property specifies the visibility of the item. (2 bit)
		0: This property is ignored
		1: This item can only be seen by the owner
		2: This item cannot be seen
	*/
	unsigned char visible_ : 2;

	// Bit | Hex | Description
	//===================
	//   0 |  01 | NoDecay
	//   1 |  02 | Newbie
	//   2 |  04 |
	//   3 |  08 | Secured (Chests)
	//   4 |  10 | Allow meditation
	//   5 |  20 | Twohanded
	//   6 |  40 | Unprocessed (Set on load. Unset on postprocess)
	//   7 |  80 | Dye
	unsigned char priv_;
};
#pragma pack()

#endif
