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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#ifndef __ITEMS_H
#define __ITEMS_H

#include "wolfpack.h" //for now.

#include "uobject.h"

// Forward Class declarations
class ISerialization;

//using namespace std ;
//typedef struct item_st_
class cItem : public cUObject
{
public:
	cItem() {};
	cItem( cItem& src); // Copy constructor
	virtual ~cItem() {}
	virtual void Serialize( ISerialization &archive );
	unsigned char ser1; // Item serial number
	unsigned char ser2;
	unsigned char ser3;
	unsigned char ser4;
	
	unsigned char id1; // Item visuals as stored in the client
	unsigned char id2;
	unsigned char color1; // Hue
	unsigned char color2;
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
	
	int oldx; //Old x position - used for bouncing bugfix - AntiChrist
	int oldy; //Old y position - used for bouncing bugfix - AntiChrist
	signed char oldz; //Old z position - used for bouncing bugfix - AntiChrist
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
	int hp; //Number of hit points an item has.
	int maxhp; // Max number of hit points an item can have.
	int st; // The strength needed to equip the item
	int st2; // The strength the item gives
	int dx; // The dexterity needed to equip the item
	int dx2; // The dexterity the item gives
	int in; // The intelligence needed to equip the item
	int in2; // The intelligence the item gives
	int spd; //The speed of the weapon
	int smelt; // for item smelting
	int secureIt; // secured chests
	int wipe; //Should this item be wiped with the /wipe command
	unsigned char magic; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable, 4=Locked Down
	unsigned int gatetime;
	int gatenumber;
	unsigned int decaytime;
	//signed int destroyTimer; // Ripper for chaos/order shields
	int ownserial;
	unsigned char visible; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	int spawnserial;
	unsigned char dir;
	//char dir; // Direction, or light source type.
	unsigned char priv; // Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
	int value; // Price shopkeeper sells item at.
	int restock; // Number up to which shopkeeper should restock this item
	int trigger; //Trigger number that item activates
	int trigtype; //Type of trigger
	int trigon; //New trigger for items -Frazurbluu-
	unsigned int disabled; //Item is disabled, cant trigger.
	string disabledmsg; //Item is disabled, so display this message. -- added by Magius(CHE) §
	int tuses;    //Number of uses for trigger
	unsigned int poisoned; //AntiChrist -- for poisoning skill
	string murderer; //AntiChrist -- for corpse -- char's name who kille the char (forensic ev.)
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
	
	int glow; // LB identifies glowing objects
	unsigned char glow_c1; // for backup of old color
	unsigned char glow_c2;
	unsigned char glow_effect; 
	
	string desc;
	
	int carve; //AntiChrist - for new carve system
	bool incognito; //AntiChrist - for items under incognito effect
	unsigned int time_unused;     // LB -> used for house decay and possibly for more in future, gets saved
	unsigned int timeused_last; // helper attribute for time_unused, doesnt get saved
	int spawnregion;
	
	
	void Init(char mkser=1);
	void SetSerial(long ser);
	
	void SetContSerial(long contser);
	bool isInWorld()			{return (contserial == INVALID_SERIAL);}
	bool isMulti()				{return (id1>=0x40);	}
	
	void setOwnSerialOnly(long ownser);
	void SetOwnSerial(long ownser);
	long GetOwnSerial()			{return ownserial;}
	
	void SetSpawnSerial(long spawnser);
	void SetMultiSerial(long mulser);
	
	void setId(unsigned short id);
	void setColor(unsigned short color);
	inline unsigned short id()			{return (unsigned short)((id1<<8)+id2);}
	
	inline unsigned short color()		{return (unsigned short)((color1<<8)+color2);}
	void MoveTo(int newx, int newy, signed char newz);
	long ReduceAmount(const short amount);
	short GetContGumpType();
	void SetRandPosInCont(cItem* pCont);
	bool PileItem(cItem* pItem);
	bool ContainerPileItem(cItem* pItem);	// try to find an item in the container to stack with
	bool AddItem(cItem* pItem, short xx=-1, short yy=-1);	// Add Item to container
	int  CountItems(short ID, short col= -1);
	int  DeleteAmount(int amount, short id, short color=-1);
	int getName(char* itemname);
	string getName(void);
	int getWeight();
	void startDecay()			{this->decaytime=SrvParms->decaytimer*MY_CLOCKS_PER_SEC+uiCurrentTime;}
	void setAllMovable()		{this->magic=1;} // set it all movable..
	bool isAllMovable()         {return (magic==1);}
	void setGMMovable()		    {this->magic=2;} // set it GM movable.
	bool isGMMovable()          {return (magic==2);}
	void setOwnerMovable()		{this->magic=3;} // set it owner movable.
	bool isOwnerMovable()       {return (magic==3);}
	void setLockedDown()        {this->magic=4;} // set it locked down.
	bool isLockedDown()			{return (magic==4);}

};


int	ContainerCountItems(const int serial, short id, short color=-1);

class cAllItems
{
private:
	cItem* CreateScriptRandomItem(int s, char *sItemList);
	void GetScriptItemSetting(P_ITEM pi); // by Magius(CHE)
	unsigned char PackType(unsigned char id1, unsigned char id2);
public:
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
				unsigned char cColorId1, unsigned char cColorId2,
				int nPack, int nSend);
    P_ITEM  SpawnItem(UOXSOCKET nSocket, P_CHAR ch,
				int nAmount, char* cName, int nStackable,
				unsigned char cItemId1, unsigned char cItemId2,
				unsigned char cColorId1, unsigned char cColorId2,
				int nPack, int nSend);
	P_ITEM SpawnItem(P_CHAR pc_ch,int nAmount, char* cName, bool pileable, short id, short color, bool bPack);
	P_ITEM SpawnItemBank(P_CHAR pc_ch, int nItem);
	P_ITEM  SpawnItemBackpack2(UOXSOCKET s, int nItem, int nDigging);
	void DecayItem(unsigned int currenttime, P_ITEM pi);
	void RespawnItem(unsigned int Currenttime, P_ITEM pi);
	void AddRespawnItem(P_ITEM pItem, int x, int y);
	void CheckEquipment(P_CHAR pc_p); //AntiChrist
};


#endif
