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


//##ModelId=3C5D92A90094
class CWorldMain  
{
public:
	//##ModelId=3C5D92A900A8
	virtual int announce();
	//##ModelId=3C5D92A9010D
	virtual void announce(int choice);
	//##ModelId=3C5D92A9012B
	virtual void loadnewworld();
	//##ModelId=3C5D92A90135
	virtual void savenewworld(char x);
	//##ModelId=3C5D92A90153
	CWorldMain();
	//##ModelId=3C5D92A9015D
	virtual ~CWorldMain();

	//##ModelId=3C5D92A90171
	void SetLoopSaveAmt( long toSet );
	//##ModelId=3C5D92A90185
	long LoopSaveAmt( void );
	//##ModelId=3C5D92A90190
	bool Saving( void );
	//##ModelId=3C5D92A901A3
	bool RemoveItemsFromCharBody(int charserial, int type1, int type2);
private:
	//##ModelId=3C5D92A901D5
	bool isSaving;
	//##ModelId=3C5D92A901E9
	int DisplayWorldSaves;
	FILE *iWsc, *cWsc;
	unsigned long Cur, Max;
	//##ModelId=3C5D92A90207
	long PerLoop;

	//##ModelId=3C5D92A90225
	void SaveChar( P_CHAR );
	//##ModelId=3C5D92A90239
	void SaveItem( P_ITEM pi, P_ITEM pDefault );
};

#endif //
 
