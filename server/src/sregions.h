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

#ifndef _SREGIONS_H
#define _SREGIONS_H

// Platform specifics
#include "platform.h"


//System includes



// Forward class Dec
class cRespawn ;



//Wolfpack includes
#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"





//##ModelId=3C5D92CF0324
class cRespawn
{
private:
	//##ModelId=3C5D92CF0342
	bool respawning;
	//##ModelId=3C5D92CF0356
	bool doItemSpawns;
	//##ModelId=3C5D92CF0374
	int currentSpawnRegion;
	//##ModelId=3C5D92CF0388
	unsigned int currentSpawnItem;
public:
	//##ModelId=3C5D92CF03A6
	cRespawn() {respawning = doItemSpawns = false;}
	//##ModelId=3C5D92CF03A7
	~cRespawn() {}
	//##ModelId=3C5D92CF03B0
	bool AreWeRespawning() {return respawning;}
	//##ModelId=3C5D92CF03BA
	void Start() {respawning = doItemSpawns = true; currentSpawnRegion=1; currentSpawnItem=0;}
	//##ModelId=3C5D92CF03C4
	void Continue();
};

extern cRespawn	*Respawn;

bool doregionspawn(int r);
void loadspawnregions();//Zippy
void loadregions();
void checkregion(P_CHAR pc_i);
unsigned char calcRegionFromXY(int x, int y);

#endif
