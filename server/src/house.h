#ifndef _H_HOUSE
#define _H_HOUSE
#include "wolfpack.h"

class cHouseManager
{
public:
	cHouseManager()
	{
		printf("House Manager succesfully started!\n");
		HouseFile=NULL;
		printf("exiting construcctor \n");	
	};
	~cHouseManager()
	{
		printf("House Manager succesfully shutdown!\n");
		HouseFile=NULL;
	};
	void AddHome(int s, int i);
	void HomeTarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);
	bool HomeBuildSite(int x, int y, int z, int sx, int sy) { return true;}
	void RemoveHouse(int i);
	void RemoveKeys(int serial);
	int CheckDecayStatus();
	int GetHouseNum(P_CHAR pc);
	int GetHouseNum(P_ITEM pi);
	void SaveHouses();
	void LoadHouses();
private:
	FILE *HouseFile;
};
class cHouse
{
public:
	cHouse()
	{
		id1=0;
		id2=0;
		SecureAmount=0;
		SecureTotal=0;
		LockAmount=0;
		LockTotal=0;
		serial=0;
		OwnerAccount=0;
		OwnerSerial=0;
		LastUsed=0;
		TimeUnused=0;
		FriendList.resize(0);
		BanList.resize(0);
	};
	~cHouse()
	{
		id1=0;
		id2=0;
		SecureAmount=0;
		SecureTotal=0;
		LockAmount=0;
		LockTotal=0;
		serial=0;
		OwnerAccount=0;
		OwnerSerial=0;
		LastUsed=0;
		TimeUnused=0;
		FriendList.clear();
		BanList.clear();
	};
	int AddFriend(P_CHAR pc);
	bool RemoveFriend(P_CHAR pc);
	int FindFriend(P_CHAR pc);
	int AddBan(P_CHAR pc);
	bool RemoveBan(P_CHAR pc);
	int FindBan(P_CHAR pc);
	Coord_cl pos;
	Coord_cl pos2;
	int id1;
	int id2;
	int SecureAmount;
	int SecureTotal;
	int LockAmount;
	int LockTotal;
	int serial;
	int OwnerAccount;
	int OwnerSerial;
	int LastUsed;
	int TimeUnused;
	vector<int> FriendList;
	vector<int> BanList;
};
extern cHouseManager* HouseManager;
extern std::vector<cHouse *> House;
#endif
