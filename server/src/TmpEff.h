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


class cTmpEff  
{
protected:
	int sourSer;
	int destSer;
public:
	unsigned int expiretime;
	unsigned char num;
	unsigned char more1;
	unsigned char more2;
	unsigned char more3;
	unsigned char dispellable;

public:
	cTmpEff() {;}
	virtual ~cTmpEff() {;}
	void Init();
	void setExpiretime_s(int seconds);
	void setDest(int ser);
	int  getDest();
	void setSour(int ser);
	int  getSour();
	void On(P_CHAR pc);
	void Off(P_CHAR pc);
	void Reverse();
	void Expire();
};

class cAllTmpEff  
{
private:
	cTmpEff *teffects;
	unsigned int teffectcount; // No temp effects to start with

public:
	cAllTmpEff()	{teffects=NULL;teffectcount=0;} // No temp effects to start with
//	virtual ~cAllTmpEff();
	bool Alloc(int count);
	bool ReAlloc(int newcount);
	void On();
	void Off();
	void Check();
	bool Add(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur);
	bool Add(P_CHAR pc_source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
	void Insert(cTmpEff* pTE);
	void Remove(cTmpEff* pTE);
};

#endif

 
