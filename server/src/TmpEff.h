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

// TmpEff.h: interface for the TmpEff class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__TMPEFF_H__)
#define __TMPEFF_H__ 

//Platform specifics
#include "platform.h"

//System includes

#include <iostream>

using namespace std ;


//Forward class declarations
class cTmpEff ;
class cAllTmpEff ;

// Wolfpack includes
#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"


//##ModelId=3C5D92B202A0
class cTmpEff  
{
protected:
	//##ModelId=3C5D92B202BE
	int sourSer;
	//##ModelId=3C5D92B202D2
	int destSer;
public:
	//##ModelId=3C5D92B202F0
	unsigned int expiretime;
	//##ModelId=3C5D92B2032C
	unsigned char num;
	//##ModelId=3C5D92B301A7
	unsigned short more1;
	//##ModelId=3C5D92B301B1
	unsigned short more2;
	//##ModelId=3C5D92B301C5
	unsigned short more3;
	//##ModelId=3C5D92B301D9
	unsigned char dispellable;

public:
	//##ModelId=3C5D92B3025B
	cTmpEff() {;}
	//##ModelId=3C5D92B30265
	virtual ~cTmpEff() {;}
	//##ModelId=3C5D92B3026F
	void Init();
	//##ModelId=3C5D92B30279
	void setExpiretime_s(int seconds);
	//##ModelId=3C5D92B3028D
	void setDest(int ser);
	//##ModelId=3C5D92B30298
	int  getDest();
	//##ModelId=3C5D92B302A2
	void setSour(int ser);
	//##ModelId=3C5D92B302C0
	int  getSour();
	//##ModelId=3C5D92B302CA
	void On(P_CHAR pc);
	//##ModelId=3C5D92B302DE
	void Off(P_CHAR pc);
	//##ModelId=3C5D92B302F2
	void Reverse();
	//##ModelId=3C5D92B302FC
	void Expire();
};

//##ModelId=3C5D92B400AE
class cAllTmpEff  
{
private:
	//##ModelId=3C5D92B400D7
	cTmpEff *teffects;
	//##ModelId=3C5D92B400EA
	unsigned int teffectcount; // No temp effects to start with

public:
	//##ModelId=3C5D92B40144
	cAllTmpEff()	{teffects=NULL;teffectcount=0;} // No temp effects to start with
//	virtual ~cAllTmpEff();
	//##ModelId=3C5D92B4014E
	bool Alloc(int count);
	//##ModelId=3C5D92B40163
	bool ReAlloc(int newcount);
	//##ModelId=3C5D92B40177
	void On();
	//##ModelId=3C5D92B40181
	void Off();
	//##ModelId=3C5D92B4018B
	void Check();
	//##ModelId=3C5D92B40195
	bool Add(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur);
	//##ModelId=3C5D92B401DB
	bool Add(P_CHAR pc_source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
	//##ModelId=3C5D92B40221
	void Insert(cTmpEff* pTE);
	//##ModelId=3C5D92B4022B
	void Remove(cTmpEff* pTE);
};

#endif

 
