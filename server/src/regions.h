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

#if !defined(__REGION_H__)
#define __REGION_H__

// Platform specifics
#include "platform.h"

// System includes

#include <map>
#include <vector>

using namespace std;

// Forward class definition


// wolfpack includes
#include "typedefs.h"

// Class definition
const unsigned char GridSize = 32;
const unsigned char ColSize = 128;

//##ModelId=3C5D92D20288
enum enDomain { enAll, enCharsOnly, enItemsOnly};

//##ModelId=3C5D92D2035A
class cRegion
{
private:
//	static unsigned char GridSize;
//	static unsigned char ColSize;
	//##ModelId=3C5D92D3021B
	typedef multimap<UI32, SERIAL>::iterator iterMapCells;

	//##ModelId=3C5D92D203AB
	multimap<UI32, SERIAL> MapCells;

	//##ModelId=3C5D92D203C9
	bool Add (UI32, SERIAL);
	//##ModelId=3C5D92D30009
	bool Remove (UI32, SERIAL);

public:
	friend class RegionIterator4Chars;
	friend class RegionIterator4Items;
	//##ModelId=3C5D92D3029E
	class RegionIterator4Chars
	{
	protected:
		//##ModelId=3C5D92D302BD
		P_CHAR currentCharacter;
		//##ModelId=3C5D92D302EF
		UI32 cell;
		//##ModelId=3C5D92D3030D
		UI32 currentCell;
		//##ModelId=3C5D92D30335
		UI32 currentIndex;
		//##ModelId=3C5D92D3037B
		vector<SERIAL> vecEntries;

		//##ModelId=3C5D92D30398
		P_CHAR NextCell(void);
	public:
		//##ModelId=3C5D92D303AC
		RegionIterator4Chars(const Coord_cl); 
		//##ModelId=3C5D92D303C0
		P_CHAR Begin(void);
		//##ModelId=3C5D92D303DE
		P_CHAR End(void);
		//##ModelId=3C5D92D4000A
		P_CHAR GetData(void);
		// Operators
		//##ModelId=3C5D92D4001E
		RegionIterator4Chars& operator++(int);
		//##ModelId=3C5D92D40032
		RegionIterator4Chars& operator=(Coord_cl);
	};
	//##ModelId=3C5D92D400B4
	class RegionIterator4Items
	{
	protected:
		//##ModelId=3C5D92D400C9
		P_ITEM currentItem;
		//##ModelId=3C5D92D400F1
		UI32 cell;
		//##ModelId=3C5D92D4010F
		UI32 currentCell;
		//##ModelId=3C5D92D40174
		UI32 currentIndex;
		//##ModelId=3C5D92D4019C
		vector<SERIAL> vecEntries;

		//##ModelId=3C5D92D401B9
		P_ITEM NextCell(void);
	public:
		//##ModelId=3C5D92D401CD
		RegionIterator4Items(const Coord_cl); 
		//##ModelId=3C5D92D401E1
		P_ITEM Begin(void);
		//##ModelId=3C5D92D401F5
		P_ITEM End(void);
		//##ModelId=3C5D92D40209
		P_ITEM GetData(void);
		// Operators
		//##ModelId=3C5D92D4021D
		RegionIterator4Items& operator++(int);
		//##ModelId=3C5D92D4023B
		RegionIterator4Items& operator=(Coord_cl);
	};

	//##ModelId=3C5D92D30027
	static unsigned int GetCell(unsigned int x, unsigned int y);
	//##ModelId=3C5D92D30045
	static unsigned int StartGrid(unsigned int x, unsigned int y);
	//##ModelId=3C5D92D30063
	static int myGridx(unsigned int x) throw() {return x/GridSize;};
	//##ModelId=3C5D92D30077
	static int myGridy(unsigned int y) throw() {return y/GridSize;};

	//##ModelId=3C5D92D3008B
	bool Add(P_ITEM);
	//##ModelId=3C5D92D3009F
	bool Add(P_CHAR);
	//##ModelId=3C5D92D300B3
	bool Remove(P_ITEM);
	//##ModelId=3C5D92D300C7
	bool Remove(P_CHAR);
	//##ModelId=3C5D92D300E5
	vector<SERIAL> GetCellEntries(UI32 cell, enDomain type = enAll);
	//##ModelId=3C5D92D30103
	unsigned int GetColSize() const {return ColSize;};

};

#endif // __REGION_H__
