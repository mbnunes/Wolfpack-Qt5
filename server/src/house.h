#ifndef _H_HOUSE
#define _H_HOUSE
#include "wolfpack.h"

class cHouseManager
{
public:
	cHouseManager()
	{
		printf("House Manager succesfully started!\n");
	};
	~cHouseManager()
	{
		printf("House Manager succesfully shutdown!\n");
	};
	void AddHome(int s, int i);
	void HomeTarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);
	bool HomeBuildSite(int x, int y, int z, int sx, int sy);
	void RemoveHouse(ITEM i);
	void RemoveKeys();
	int GetHouseNum(P_CHAR pc);
	void SaveHouses();
	void LoadHouses();
};
class cHouse
{
public:
	cHouse()
	{
		id1=0;
		id2=0;
		x1=0;
		x2=0;
		y1=0;
		y2=0;
		z=0;
		SecureAmount=0;
		SecureTotal=0;
		LockAmount=0;
		LockTotal=0;
		serial=0;
		OwnerAccount=0;
		OwnerSerial=0;
		FriendList.resize(0);
		BanList.resize(0);
	};
	~cHouse()
	{
		id1=0;
		id2=0;
		x1=0;
		x2=0;
		y1=0;
		y2=0;
		z=0;
		SecureAmount=0;
		SecureTotal=0;
		LockAmount=0;
		LockTotal=0;
		serial=0;
		OwnerAccount=0;
		OwnerSerial=0;
		FriendList.resize(0);
		BanList.resize(0);
	};
	void CheckDecayStatus();
	void AddHouseItem(P_ITEM i);
	void RemoveHouseItem(P_ITEM i);
	int AddFriend(P_CHAR pc);
	bool RemoveFriend(P_CHAR pc);
	int FindFriend(P_CHAR pc);
	int AddBan(P_CHAR pc);
	bool RemoveBan(P_CHAR pc);
	int FindBan(P_CHAR pc);
	int id1;
	int id2;
	int x1;
	int x2;
	int y1;
	int y2;
	int z;
	int SecureAmount;
	int SecureTotal;
	int LockAmount;
	int LockTotal;
	int serial;
	int OwnerAccount;
	int OwnerSerial;
	vector<int> FriendList;
	vector<int> BanList;
};
extern cHouseManager* HouseManager;
extern std::vector<cHouse *> House;
#endif