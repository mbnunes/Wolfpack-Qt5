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

#include "uobject.h"
#include "defines.h"

// Forward Class declarations
class ISerialization;

class cItem : public cUObject
{
public:
	cItem() {};
	cItem( cItem& src); // Copy constructor
	virtual ~cItem() {}
	virtual void Serialize( ISerialization &archive );
	virtual string objectID();
	
	unsigned char id1; // Item visuals as stored in the client
	unsigned char id2;
	unsigned short color; // Hue
	unsigned short amount; // Amount of items in pile
	unsigned short amount2; //Used to track things like number of yards left in a roll of cloth
	struct						// Attention, this is a bit field
	{
		bool isBeeingDragged;	// true while the item hangs on the mouse cursor
	} flags;
	
	string name2;
	SERIAL contserial;
	signed char layer; // Layer if equipped on paperdoll
	int itmhand; // ITEMHAND system - AntiChrist
	unsigned int type; // For things that do special things on doubleclicking
	unsigned int type2;
	
	Coord_cl oldpos; //Old position - used for bouncing bugfix - AntiChrist
	SERIAL oldcontserial; //Old contserial - used for bouncing bugfix - Antichrist
	signed char oldlayer; // Old layer - used for bouncing bugfix - AntiChrist
	
	unsigned char offspell;
	int weight;
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
	bool pileable; // Can item be piled
	unsigned char dye; // Reserved: Can item be dyed by dye kit
	unsigned char corpse; // Is item a corpse
	unsigned int att; // Item attack
	unsigned int def; // Item defense
	int lodamage; //Minimum Damage weapon inflicts
	int hidamage; //Maximum damage weapon inflicts
	int racehate; //Race # that weapon does x2 damage to -Fraz-
	int wpsk; //The skill needed to use the item
	signed short hp; //Number of hit points an item has.
	signed short maxhp; // Max number of hit points an item can have.
	signed short st; // The strength needed to equip the item
	signed short st2; // The strength the item gives
	signed short dx; // The dexterity needed to equip the item
	signed short dx2; // The dexterity the item gives
	signed short in; // The intelligence needed to equip the item
	signed short in2; // The intelligence the item gives
	signed short spd; //The speed of the weapon
	int smelt; // for item smelting
	int secureIt; // secured chests
	bool wipe; //Should this item be wiped with the /wipe command
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
	unsigned char priv; // Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
	int value; // Price shopkeeper sells item at.
	int restock; // Number up to which shopkeeper should restock this item
	int trigger; //Trigger number that item activates
	//##ModelId=3C5D92E9008C
	int trigtype; //Type of trigger
	//##ModelId=3C5D92E900AA
	int trigon; //New trigger for items -Frazurbluu-
	//##ModelId=3C5D92E900C8
	unsigned int disabled; //Item is disabled, cant trigger.
	//##ModelId=3C5D92E900F2
	string disabledmsg; //Item is disabled, so display this message. -- added by Magius(CHE) §
	//##ModelId=3C5D92E90105
	int tuses;    //Number of uses for trigger
	//##ModelId=3C5D92E90123
	unsigned int poisoned; //AntiChrist -- for poisoning skill
	//##ModelId=3C5D92E9014C
	string murderer; //AntiChrist -- for corpse -- char's name who kille the char (forensic ev.)
	//##ModelId=3C5D92E90187
	long int murdertime; //AntiChrist -- for corpse -- when the people has been killed
	//##ModelId=3C5D92E901AF
	int rank; //Magius(CHE) --- for rank system, this value is the LEVEL of the item from 1 to 10. Simply multiply the rank*10 and calculate the MALUS this item has from the original.
	// for example: RANK 5 ---> 5*10=50% of malus
	//   this item has same values decreased by 50%..
	// RANK 1 ---> 1*10=10% this item has 90% of malus!
	// RANK 10 --> 10*10=100% this item has no malus! RANK 10 is automatically setted if you select RANKSYSTEM 0.
	// Vars: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
	//##ModelId=3C5D92E901D8
	string creator; // Store the name of the player made this item -- Magius(CHE)
	//##ModelId=3C5D92E901EB
	int good; // Store type of GOODs to trade system! (Plz not set as UNSIGNED)  --- Magius(CHE)
	//##ModelId=3C5D92E90335
	int rndvaluerate; // Store the value calculated base on RANDOMVALUE in region.scp. ---- MAgius(CHE) (2)
	//##ModelId=3C5D92E90371
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
	
	//##ModelId=3C5D92E903B9
	SERIAL glow; // LB identifies glowing objects
	//##ModelId=3C5D92E903E0
	unsigned short glow_color; // for backup of old color
	//##ModelId=3C5D92EA002A
	unsigned char glow_effect; 
	
	//##ModelId=3C5D92EA005D
	string desc;
	
	//##ModelId=3C5D92EA007A
	int carve; //AntiChrist - for new carve system
	//##ModelId=3C5D92EA00C0
	bool incognito; //AntiChrist - for items under incognito effect
	//##ModelId=3C5D92EA00FC
	unsigned int time_unused;     // LB -> used for house decay and possibly for more in future, gets saved
	//##ModelId=3C5D92EA0138
	unsigned int timeused_last; // helper attribute for time_unused, doesnt get saved
	//##ModelId=3C5D92EA01A6
	int spawnregion;
	
	
	//##ModelId=3C5D92EA01CE
	void Init(bool mkser = true);
	//##ModelId=3C5D92EA023C
	void SetSerial(long ser);
	
	//##ModelId=3C5D92EA0297
	void SetContSerial(long contser);
	//##ModelId=3C5D92EB0111
	bool isInWorld()			{return (contserial == INVALID_SERIAL);}
	//##ModelId=3C5D92EB0126
	bool isMulti()				{return (id1>=0x40);	}
	
	//##ModelId=3C5D92EB0130
	void setOwnSerialOnly(long ownser);
	//##ModelId=3C5D92EB014E
	void SetOwnSerial(long ownser);
	//##ModelId=3C5D92EB01D0
	long GetOwnSerial()			{return ownserial;}
	
	//##ModelId=3C5D92EB01E4
	void SetSpawnSerial(long spawnser);
	//##ModelId=3C5D92EB020C
	void SetMultiSerial(long mulser);
	
	//##ModelId=3C5D92EB0360
	void setId(unsigned short id);
	//##ModelId=3C5D92EB0388
	void setColor(unsigned short color);
	//##ModelId=3C5D92EB03CE
	inline unsigned short id()	const		{return (unsigned short)((id1<<8)+id2);}
	
//	inline unsigned short color()		{return (unsigned short)((color1<<8)+color2);}
	//##ModelId=3C5D92EC004B
	void MoveTo(int newx, int newy, signed char newz);
	//##ModelId=3C5D92EC00D7
	long ReduceAmount(const short amount);
	//##ModelId=3C5D92EC00FF
	short GetContGumpType();
	//##ModelId=3C5D92EC0109
	void SetRandPosInCont(cItem* pCont);
	//##ModelId=3C5D92EC0131
	bool PileItem(cItem* pItem);
	//##ModelId=3C5D92EC014F
	bool ContainerPileItem(cItem* pItem);	// try to find an item in the container to stack with
	//##ModelId=3C5D92EC0177
	bool AddItem(cItem* pItem, short xx=-1, short yy=-1);	// Add Item to container
	//##ModelId=3C5D92EC01BD
	int  CountItems(short ID, short col= -1);
	//##ModelId=3C5D92EC01F9
	int  DeleteAmount(int amount, unsigned short _id, unsigned short _color = 0);
	//##ModelId=3C5D92EC0249
	int getName(char* itemname);
	//##ModelId=3C5D92EC0267
	string getName(void);
	//##ModelId=3C5D92EC0285
	int getWeight();
	void startDecay();
	void setAllMovable()		{this->magic=1;} // set it all movable..
	//##ModelId=3C5D92EC02A3
	bool isAllMovable()         {return (magic==1);}
	//##ModelId=3C5D92EC02AE
	void setGMMovable()		    {this->magic=2;} // set it GM movable.
	//##ModelId=3C5D92EC02B8
	bool isGMMovable()          {return (magic==2);}
	//##ModelId=3C5D92EC02C2
	void setOwnerMovable()		{this->magic=3;} // set it owner movable.
	//##ModelId=3C5D92EC02CC
	bool isOwnerMovable()       {return (magic==3);}
	//##ModelId=3C5D92EC02D6
	void setLockedDown()        {this->magic=4;} // set it locked down.
	//##ModelId=3C5D92EC02EA
	bool isLockedDown()			{return (magic==4);}

};


int	ContainerCountItems(const int serial, short id, short color=-1);

//##ModelId=3C5D92EE009E
class cAllItems
{
private:
	//##ModelId=3C5D92EE00BC
	cItem* CreateScriptRandomItem(int s, char *sItemList);
	//##ModelId=3C5D92EE00DA
	unsigned char PackType(unsigned char id1, unsigned char id2);
public:
	//##ModelId=3C5D92EE00F8
	cAllItems() {}
	//##ModelId=3C5D92EE0102
	void DeleItem(P_ITEM pi);
	//##ModelId=3C5D92EE0134
	P_ITEM  MemItemFree();
	//##ModelId=3C5D92EE013E
	char isFieldSpellItem(P_ITEM pi);
	//##ModelId=3C5D92EE0149
	P_ITEM CreateFromScript(UOXSOCKET s, int itemnum);
	//##ModelId=3C5D92EE01C0
	P_ITEM  CreateScriptItem(int s, int itemnum, int nSpawned);
	//##ModelId=3C5D92EE01DE
	int  CreateRandomItem(char *sItemList);
	//##ModelId=3C5D92EE0210
	P_ITEM  SpawnItem(UOXSOCKET nSocket,
				int nAmount, char* cName, int nStackable,
				unsigned char cItemId1, unsigned char cItemId2,
				unsigned short cColorId, 
				int nPack, int nSend);
	//##ModelId=3C5D92EE027E
    P_ITEM  SpawnItem(UOXSOCKET nSocket, P_CHAR ch,
				int nAmount, char* cName, int nStackable,
				unsigned char cItemId1, unsigned char cItemId2,
				unsigned short cColorId,
				int nPack, int nSend);
	//##ModelId=3C5D92EE0365
	P_ITEM SpawnItem(P_CHAR pc_ch,int nAmount, char* cName, bool pileable, short id, short color, bool bPack);
	//##ModelId=3C5D92EF0013
	P_ITEM SpawnItemBank(P_CHAR pc_ch, int nItem);
	//##ModelId=3C5D92EF0045
	P_ITEM  SpawnItemBackpack2(UOXSOCKET s, int nItem, int nDigging);
	//##ModelId=3C5D92EF008B
	void DecayItem(unsigned int currenttime, P_ITEM pi);
	//##ModelId=3C5D92EF00C7
	void RespawnItem(unsigned int Currenttime, P_ITEM pi);
	//##ModelId=3C5D92EF00F9
	void AddRespawnItem(P_ITEM pItem, int x, int y);
	//##ModelId=3C5D92EF0153
	void CheckEquipment(P_CHAR pc_p); //AntiChrist
	//##ModelId=3C5D92EF0167
	void GetScriptItemSetting(P_ITEM pi); // by Magius(CHE)
};


#endif
