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

#ifndef __ITEMS_H
#define __ITEMS_H

//#include "wolfpack.h" //for now.

//#include "WPDefaultScript.h"
#include "uobject.h"
#include "defines.h"

// Forward Class declarations
class ISerialization;

class cItem : public cUObject
{
private:
	UI16 id_;
	UI16 color_;
	UI16 amount_; 
	UI16 amount2_; 
	QString name2_;
	QString name_;
	SI08 layer_;
	UI08 itemhand_;
	QString murderer_;
	SI16 lodamage_; 
	SI16 hidamage_; 
	UI16 type_;
	UI16 type2_;
	UI08 offspell_; // Whats that for ?!
	SI16 speed_;
	SI16 racehate_; 
	SI16 weight_;
	SI16 hp_; //Number of hit points an item has.
	SI16 maxhp_; // Max number of hit points an item can have.

public:
	// Getters
	UI16			id()		const { return id_; };		 // The graphical id of the item
	UI16			color()		const { return color_; };	 // The Color of the item
	UI16			amount()	const { return amount_; };	 // Amount of items in pile
	UI16			amount2()	const { return amount2_; };  // Used to track things like number of yards left in a roll of cloth
	const QString	&name2()	const { return name2_; };	 // The identified name of the item
	const QString	&name()		const { return name_; };	 // Returns the item's name
	UI08			layer()		const { return layer_; };	 // Layer if equipped on paperdoll
	UI08			itemhand()	const { return itemhand_; }; // ITEMHAND system - AntiChrist
	const QString	&murderer() const { return murderer_; }; // If it's a corpse, this holds the name of the murderer
	UI32			type()		const { return type_; };	 // Used for hardcoded behaviour
	UI32			type2()		const { return type2_; };
	UI08			offspell()	const { return offspell_; }; 
	bool			secured()	const { return priv&0x08; }; // Is the container secured (houses)
	SI16			speed()		const { return speed_; };	 // Weapon speed
	SI16			lodamage()	const { return lodamage_; }; // Minimum damage weapon inflicts
	SI16			hidamage()	const { return hidamage_; }; // Maximum damage weapon inflicts
	bool			wipe()		const { return priv&0x10; }; // Should the item be wiped when affected by /WIPE
	bool			pileable()	const { return priv&0x20; }; // Can Item be piled
	SI16			racehate()	const { return racehate_; }; // Race ID this weapon does double damage to
	SI16			weight()	const { return weight_; };
	SI16			hp()		const { return hp_; }; // Number of hitpoints an item has
	SI16			maxhp()		const { return maxhp_; }; // Maximum number of hitpoints an item has

	// Setters
	void	setId( UI16 nValue ) { id_ = nValue; };
	void	setColor( UI16 nValue ) { color_ = nValue; };
	void	setAmount( UI16 nValue ) { amount_ = nValue; }; // Amount of items in pile
	void	setAmount2( UI16 nValue ) { amount2_ = nValue; }; //Used to track things like number of yards left in a roll of cloth
	void	setName2( const QString nValue ) { name2_ = nValue; };
	void	setName( const QString nValue ) { name_ = nValue; };
	void	setLayer( SI08 nValue ) { layer_ = nValue; };
	void	setItemhand( UI08 nValue ) { itemhand_ = nValue; };
	void	setMurderer( const QString nValue ) { murderer_ = nValue; };
	void	setType( UI32 nValue ) { type_ = nValue; };
	void	setType2( UI32 nValue ) { type2_ = nValue; };	
	void	setOffspell( UI08 nValue ) { offspell_ = nValue; };
	void	setSecured( bool nValue ) { ( nValue ) ? priv &= 0x08 : priv |= 0xF7; };
	void	setSpeed( SI16 nValue ) { speed_ = nValue; };
	void	setContSerial( UI32 nValue ); // Defined in items.cpp
	void	setHidamage( SI16 nValue ) { hidamage_ = nValue; };
	void	setLodamage( SI16 nValue ) { lodamage_ = nValue; };
	void	setWipe( bool nValue ) { ( nValue ) ? priv &= 0x10 : priv |= 0xEF; };
	void	setPileable( bool nValue ) { ( nValue ) ? priv &= 0x20 : priv |= 0xDF; };
	void	setRacehate( SI16 nValue ) { racehate_ = nValue; };
	void	setWeight( SI16 nValue ) { weight_ = nValue; };
	void	setHp( SI16 nValue ) { hp_ = nValue; };
	void	setMaxhp( SI16 nValue ) { maxhp_ = nValue; };

	cItem() {};
	cItem( cItem& src); // Copy constructor
	virtual ~cItem() {}
	virtual void Serialize( ISerialization &archive );
	virtual string objectID();
	
	SERIAL contserial;
	
	Coord_cl oldpos; //Old position - used for bouncing bugfix - AntiChrist
	SERIAL oldcontserial; //Old contserial - used for bouncing bugfix - Antichrist
	signed char oldlayer; // Old layer - used for bouncing bugfix - AntiChrist

	unsigned char more1; // For various stuff
	unsigned char more2;
	unsigned char more3;
	unsigned char more4;
	unsigned char moreb1;
	unsigned char moreb2;
	unsigned char moreb3;
	unsigned char moreb4;
	unsigned int morex;
	unsigned int morey;
	unsigned int morez;
	unsigned char doordir; // Reserved for doors
	unsigned char dooropen;
	unsigned char dye; // Reserved: Can item be dyed by dye kit
	unsigned char corpse; // Is item a corpse
	unsigned int att; // Item attack
	unsigned int def; // Item defense
	signed short st; // The strength needed to equip the item
	signed short st2; // The strength the item gives
	signed short dx; // The dexterity needed to equip the item
	signed short dx2; // The dexterity the item gives
	signed short in; // The intelligence needed to equip the item
	signed short in2; // The intelligence the item gives
	int smelt; // for item smelting
	unsigned char magic; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable, 4=Locked Down
	unsigned int gatetime;
	int gatenumber;
	unsigned int decaytime;
	//signed int destroyTimer; // Ripper for chaos/order shields
	SERIAL ownserial;
	unsigned char visible; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	SERIAL spawnserial;
	unsigned char dir;
	//char dir; // Direction, or light source type.

	// Bit | Description
	//===================
	//   0 | Decay
	//   1 | Newbie
	//   2 | Dispellable
	//   3 | Secured (Chests)
	//   4 | Wipeable (/WIPE affects the item)
	//   5 | Pileable
	UI08 priv;
	
	int value; // Price shopkeeper sells item at.
	int restock; // Number up to which shopkeeper should restock this item
	int trigger; //Trigger number that item activates
	int trigtype; //Type of trigger
	int trigon; //New trigger for items -Frazurbluu-
	unsigned int disabled; //Item is disabled, cant trigger.
	string disabledmsg; //Item is disabled, so display this message. -- added by Magius(CHE) §
	int tuses;    //Number of uses for trigger
	unsigned int poisoned; //AntiChrist -- for poisoning skill
	long int murdertime; //AntiChrist -- for corpse -- when the people has been killed
	int rank; //Magius(CHE) --- for rank system, this value is the LEVEL of the item from 1 to 10. Simply multiply the rank*10 and calculate the MALUS this item has from the original.
	// for example: RANK 5 ---> 5*10=50% of malus
	//   this item has same values decreased by 50%..
	// RANK 1 ---> 1*10=10% this item has 90% of malus!
	// RANK 10 --> 10*10=100% this item has no malus! RANK 10 is automatically setted if you select RANKSYSTEM 0.
	// Vars: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
	string creator; // Store the name of the player made this item -- Magius(CHE)
	int good; // Store type of GOODs to trade system! (Plz not set as UNSIGNED)  --- Magius(CHE)
	int rndvaluerate; // Store the value calculated base on RANDOMVALUE in region.scp. ---- MAgius(CHE) (2)
	int madewith; // Store the skills used to make this item -- Magius(CHE)
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
	
	SERIAL glow; // LB identifies glowing objects
	unsigned short glow_color; // for backup of old color
	unsigned char glow_effect; 
	
	string desc;
	
	int carve; //AntiChrist - for new carve system
	bool incognito; //AntiChrist - for items under incognito effect
	// ^^ NUTS !! - move that to priv

	unsigned int time_unused;     // LB -> used for house decay and possibly for more in future, gets saved
	unsigned int timeused_last; // helper attribute for time_unused, doesnt get saved
	int spawnregion;
	
	
	void Init(bool mkser = true);
	void SetSerial(long ser);
	bool isInWorld()			{ return (contserial == INVALID_SERIAL); }
	bool isMulti()				{ return ( id_ >= 0x4000 ); }
	
	void setOwnSerialOnly(long ownser);
	void SetOwnSerial(long ownser);
	long GetOwnSerial()			{return ownserial;}
	
	void SetSpawnSerial(long spawnser);
	void SetMultiSerial(long mulser);
	
//	inline unsigned short color()		{return (unsigned short)((color1<<8)+color2);}
	void MoveTo(int newx, int newy, signed char newz);
	long ReduceAmount(const short amount);
	short GetContGumpType();
	void SetRandPosInCont(cItem* pCont);
	bool PileItem(cItem* pItem);
	bool ContainerPileItem(cItem* pItem);	// try to find an item in the container to stack with
	bool AddItem(cItem* pItem, short xx=-1, short yy=-1);	// Add Item to container
	int  CountItems(short ID, short col= -1);
	int  DeleteAmount(int amount, unsigned short _id, unsigned short _color = 0);
	int getName(char* itemname);
	int getWeight();
	QString getName(void);
	void startDecay();
	void setAllMovable()		{this->magic=1;} // set it all movable..
	bool isAllMovable()         {return (magic==1);}
	void setGMMovable()		    {this->magic=2;} // set it GM movable.
	bool isGMMovable()          {return (magic==2);}
	void setOwnerMovable()		{this->magic=3;} // set it owner movable.
	bool isOwnerMovable()       {return (magic==3);}
	void setLockedDown()        {this->magic=4;} // set it locked down.
	bool isLockedDown()			{return (magic==4);}

	// Public event wrappers added by darkstorm
	bool onShowItemName( P_CHAR Viewer );
	bool onTalkToItem( P_CHAR Talker, const QString &Text );	
};


int	ContainerCountItems(const int serial, short id, short color=-1);

//forward declaration
class QDomElement;

class cAllItems
{
private:
	cItem* CreateScriptRandomItem(int s, char *sItemList);
	unsigned char PackType(unsigned char id1, unsigned char id2);
	
public:
	// Added by DarkStorm
	P_ITEM createScriptItem( QString Section ); // Creates an item from an item-section
	P_ITEM createListItem( QString Section ); // Creates an Item from an item-list
	void processScriptItemNode( P_ITEM madeItem, QDomElement &Node );
	void processItemContainerNode( P_ITEM contItem, QDomElement &Node );
	void applyItemSection( P_ITEM Item, QString &Section );
	void applyItemSection( P_ITEM Item, QDomElement* Section );

	cAllItems() {}
	void DeleItem(P_ITEM pi);
	P_ITEM  MemItemFree();
	char isFieldSpellItem(P_ITEM pi);
	P_ITEM CreateFromScript(UOXSOCKET s, int itemnum);
	P_ITEM  CreateScriptItem(int s, int itemnum, int nSpawned);
	int  CreateRandomItem(char *sItemList);
	P_ITEM  SpawnItem(UOXSOCKET nSocket,
				int nAmount, char* cName, int nStackable,
				unsigned char cItemId1, unsigned char cItemId2,
				unsigned short cColorId, 
				int nPack, int nSend);
    P_ITEM  SpawnItem(UOXSOCKET nSocket, P_CHAR ch,
				int nAmount, char* cName, int nStackable,
				unsigned char cItemId1, unsigned char cItemId2,
				unsigned short cColorId,
				int nPack, int nSend);
	P_ITEM SpawnItem(P_CHAR pc_ch,int nAmount, char* cName, bool pileable, short id, short color, bool bPack);
	P_ITEM SpawnItemBank(P_CHAR pc_ch, int nItem);
	P_ITEM  SpawnItemBackpack2(UOXSOCKET s, int nItem, int nDigging);
	void DecayItem(unsigned int currenttime, P_ITEM pi);
	void RespawnItem(unsigned int Currenttime, P_ITEM pi);
	void AddRespawnItem(P_ITEM pItem, int x, int y);
	void CheckEquipment(P_CHAR pc_p); //AntiChrist
	void GetScriptItemSetting(P_ITEM pi); // by Magius(CHE)
};


#endif
