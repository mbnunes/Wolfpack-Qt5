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

// WorldMain.h: interface for the CWorldMain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__WORLDMAIN_H__)
#define __WORLDMAIN_H__ 

// Platform Specifics 
#include "platform.h"


// System includes

#include <iostream>

using namespace std ;

// forward class declaration

class CWorldMain ;

//Wolfpack Includes
#include "wolfpack.h"
#include "verinfo.h"
#include "SndPkg.h"
#include "sregions.h"
#include "debug.h"
#include "utilsys.h"


class CWorldMain  
{
public:
	virtual int announce();
	virtual void announce(int choice);
	virtual void loadnewworld();
	virtual void savenewworld(char x);
	CWorldMain();
	virtual ~CWorldMain();

	void SetLoopSaveAmt( long toSet );
	long LoopSaveAmt( void );
	bool Saving( void );
	bool RemoveItemsFromCharBody(int charserial, int type1, int type2);
private:
	bool isSaving;
	int DisplayWorldSaves;
	FILE *iWsc, *cWsc;
	unsigned long Cur, Max;
	long PerLoop;

	void SaveChar( CHARACTER i );
	void SaveItem( long i, P_ITEM pDefault );
};

#endif //
 
