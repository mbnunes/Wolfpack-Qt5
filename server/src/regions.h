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
#include <list>

// Library Includes

using namespace std;

// Forward class definition

class cUObject;
class cRegion;
class RegionIterator4Chars;
class RegionIterator4Items;

// wolfpack includes
#include "typedefs.h"

// Class definition
const unsigned char GridSize = 32;
const unsigned char ColSize = 128;

enum enDomain { enAll, enCharsOnly, enItemsOnly};

class cRegion
{

public:
	friend class RegionIterator4Chars;
	friend class RegionIterator4Items;

	typedef vector<SERIAL> raw;
	typedef vector<SERIAL>::iterator rawIterator;
	typedef ::RegionIterator4Chars RegionIterator4Chars;
	typedef ::RegionIterator4Items RegionIterator4Items;


	static unsigned int GetCell(const Coord_cl&);
	static unsigned int StartGrid(const Coord_cl&);
	static int myGridx(unsigned int x) throw() {return x/GridSize;};
	static int myGridy(unsigned int y) throw() {return y/GridSize;};

	bool Add(cUObject*);
	bool Remove(cUObject*);
	raw GetCellEntries(UI32 cell, enDomain type = enAll);
	static unsigned int GetColSize() {return ColSize;};

private:
//	static unsigned char GridSize;
//	static unsigned char ColSize;
	typedef map<UI32, raw >::iterator iterMapCells;

	map<UI32, raw > MapCellsItems;
	map<UI32, raw > MapCellsChars;

	bool Add (UI32, SERIAL);
	bool Remove (UI32, SERIAL);

};

class RegionIterator4Chars
{
protected:
	P_CHAR currentCharacter;
	UI32 cell;
	UI32 endCell;
	UI32 currentCell;
	cRegion::rawIterator currentIndex;
	cRegion::raw vecEntries;
	Coord_cl position;
	bool NextCell(void);
public:
	RegionIterator4Chars(const Coord_cl&); 
	void Begin(void);
	bool atEnd(void) const;
	P_CHAR GetData(void);
	// Operators
	RegionIterator4Chars& operator++(int);
	RegionIterator4Chars& operator=( const Coord_cl& );
};

class RegionIterator4Items
{
protected:
	P_ITEM currentItem;
	UI32 cell;
	UI32 endCell;
	UI32 currentCell;
	cRegion::rawIterator currentIndex;
	Coord_cl position;
	cRegion::raw vecEntries;

	bool NextCell(void);
public:
	RegionIterator4Items(const Coord_cl&); 
	void Begin(void);
	bool atEnd(void);
	P_ITEM GetData(void);
	// Operators
	RegionIterator4Items& operator++(int);
	RegionIterator4Items& operator=(const Coord_cl&);
};

#endif // __REGION_H__
