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
#include <vector>
#include <functional>
#include <algorithm>

//Forward class declarations
class cTempEffects;
class cTmpEff;
class cScriptEff;
class cAllTmpEff;

// Wolfpack includes
#include "typedefs.h"
#include "iserialization.h"

class cTempEffects : public cSerializable
{
protected:
	SERIAL		sourSer;
	SERIAL		destSer;
	std::string objectid;
public:
	unsigned int expiretime;
	unsigned char dispellable;
public:
				cTempEffects() {};
	virtual		~cTempEffects() {}
	void		setExpiretime_s(int seconds);
	void		setExpiretime_ms(float milliseconds);
	void		setDest(int ser);
	int			getDest();
	void		setSour(int ser);
	int			getSour();
	void		On(P_CHAR pc) {;}
	void		Off(P_CHAR pc) {;}
	virtual void Expire() = 0;
	virtual void		Serialize(ISerialization &archive);
	virtual std::string objectID() { return objectid;}
};

class cTmpEff : public cTempEffects 
{
public:
	unsigned char num;
	unsigned short more1;
	unsigned short more2;
	unsigned short more3;
public:
	cTmpEff() { objectid = "TmpEff"; }
	virtual ~cTmpEff() {;}
	void Init();
	void Reverse();
	void On(P_CHAR pc);
	void Off(P_CHAR pc);
	void Expire();
	virtual void Serialize(ISerialization &archive);
};

class cScriptEff : public cTempEffects
{
protected:
	std::string scriptname;
	std::string functionname;
public:
	cScriptEff() { objectid = "ScriptEff"; }
	virtual ~cScriptEff() {;}
	void Expire();
	virtual void Serialize(ISerialization &archive);
};

class cAllTmpEff  
{
private:
	
	struct ComparePredicate : public std::binary_function<cTempEffects*, cTempEffects*, bool>
	{
		bool operator()(const cTempEffects *a, const cTempEffects *b)
		{
			return a->expiretime < b->expiretime;
		}
	};

	std::vector<cTempEffects*> teffects;

public:
	cAllTmpEff()	{}  // No temp effects to start with
	void Check();
	bool Add(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur);
	bool Add(P_CHAR pc_source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
	void Insert(cTempEffects* pTE);
	void Serialize(ISerialization &archive);
	bool Exists( P_CHAR pc_source, P_CHAR pc_dest, int num );
	void Dispel( P_CHAR pc_dest );
};

#endif
