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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "wolfpack.h"
#include "itemid.h"
#include "guildstones.h"
#include "combat.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "tilecache.h"
#include "classes.h"
#include "maps.h"
#include "network.h"
#include "wpdefmanager.h"
#include "makemenus.h"
#include "skills.h"
#include "network/uosocket.h"

#undef DBGFILE
#define DBGFILE "skiTarg.cpp"
#include "debug.h"

//////////////////////////////////
// name:	cTinkerCombine
// history: by Duke, 3.06.2000
// Purpose: handles the combining of two (tinkering-)items after the user
//			dclicked one and then targeted the second item
//
//			It's a base class for three derived classes that handle the
//			old interfaces

class cTinkerCombine	// Combining tinkering items
{
protected:
	char* failtext;
	unsigned char badsnd1;
	unsigned char badsnd2;
	short itembits;
	short minskill;
	short id2;
public:
	cTinkerCombine(short badsnd=0x51, char *failmsg="You break one of the parts.")
	{
		badsnd1 = static_cast<unsigned char>(badsnd>>8);
		badsnd2 = static_cast<unsigned char>(badsnd&0x00FF);
		failtext=failmsg;
		itembits=0;
		minskill=100;
	}
	/*
	virtual void delonfail(SOCK s)		{deletematerial(s, itemmake[s].needs/2);}
	virtual void delonsuccess(SOCK s)	{deletematerial(s, itemmake[s].needs);}
	virtual void failure(SOCK s)		{delonfail(s);playbad(s);failmsg(s);}
	*/
	virtual void failmsg(int s)			{ /*sysmessage(s,failtext);*/ }
	virtual void playbad(int s)			{/*soundeffect(s,badsnd1,badsnd2);*/}
	virtual void playgood(int s)		{/*soundeffect(s,0,0x2A);*/}
	virtual void checkPartID(short id)	{;}
	virtual bool decide()				{return (itembits == 3) ? true : false;}
	virtual void createIt(int s)		{;}
	static cTinkerCombine* factory(short combinetype);
	virtual void DoIt(int s)
	{
	}
};


class cTinkCreateAwG : public cTinkerCombine
{
public:
	cTinkCreateAwG() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x105B || id==0x105C) itembits |= 0x01; // axles
		if (id==0x1053 || id==0x1054) itembits |= 0x02; // gears
	}
	virtual void createIt(int s)
	{
		Items->SpawnItem(s, currchar[s],1,"an axle with gears",1,0x10,0x51,0,1,1);
	}
};

class cTinkCreateParts : public cTinkerCombine
{
public:
	cTinkCreateParts() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x1051 || id==0x1052) itembits |= 0x01; // axles with gears
		if (id==0x1055 || id==0x1056) itembits |= 0x02; // hinge
		if (id==0x105D || id==0x105E) itembits |= 0x04; // springs
	}
	virtual bool decide()
	{
		if (itembits == 3) {id2=0x59; minskill=300; return true;}	// sextant parts
		if (itembits == 5) {id2=0x4F; minskill=400; return true;}	// clock parts
		return false;
	}
	virtual void createIt(int s)
	{
		 char sztemp[15] ;
		if (id2 == 0x4F)
		  strcpy(sztemp,"clock parts") ;
		else
		  strcpy(sztemp,"sextant parts") ;
		char *pn = sztemp ;
		//#else
		//char *pn = (id2==0x4F) ? "clock parts" : "sextant parts";
		//#endif
		Items->SpawnItem(s, currchar[s],1,pn,1,0x10,id2,0,1,1);
	}
};

class cTinkCreateClock : public cTinkerCombine
{
public:
	cTinkCreateClock() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x104D || id==0x104E) itembits |= 0x01; // clock frame
		if (id==0x104F || id==0x1050) itembits |= 0x02; // clock parts
	}
	virtual bool decide()   {minskill=600; return cTinkerCombine::decide();}
	virtual void createIt(int s)
	{
		Items->SpawnItem(s,currchar[s],1,"clock",0,0x10,0x4B,0,1,1);
	}
};

#define cTC_AwG		11
#define cTC_Parts	22
#define cTC_Clock	33

cTinkerCombine* cTinkerCombine::factory(short combinetype)
{
	switch (combinetype)
	{
		case cTC_AwG:   return new cTinkCreateAwG();break;
		case cTC_Parts: return new cTinkCreateParts();break;	// clock/sextant parts
		case cTC_Clock: return new cTinkCreateClock();break;	// clock
		default:		return new cTinkerCombine();break;		// a generic handler
	}
	return NULL ;
}

void cSkills::TinkerAxel(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_AwG);
	ptc->DoIt(s);
}

void cSkills::TinkerAwg(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Parts);
	ptc->DoIt(s);
}

void cSkills::TinkerClock(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Clock);
	ptc->DoIt(s);
}

