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
};
class cHouse
{
public:
	void RemoveHouse(ITEM i);
	void RemoveKeys();
	void CheckDecayStatus();
	int x1;
	int x2;
	int y1;
	int y2;
};
extern cHouseManager* HouseManager;
extern std::vector<cHouse *> House;
#endif