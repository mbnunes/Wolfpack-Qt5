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


// -- Region class  Added by Tauriel 3/6/1999
// Rewritten by Correa 10/06/2001


#include "wolfpack.h"
#include "regions.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "regions.cpp"


// Private Add.
bool cRegion::Add(UI32 cell, SERIAL serial)
{
	if ( isItemSerial(serial) )
		MapCellsItems[cell].push_back(serial);
	else if ( isCharSerial( serial ) )
		MapCellsChars[cell].push_back(serial);
	else return false;
	return true;
}

bool cRegion::Remove(UI32 cell, SERIAL serial)
{
	bool bStatus = false;
	if ( isItemSerial( serial ) )
	{
		iterMapCells it = MapCellsItems.find(cell);
		if ( it != MapCellsItems.end() )
		{
			rawIterator it2 = find(it->second.begin(), it->second.end(), serial);
			if ( it2 != it->second.end() )
			{
				it->second.erase( it2 );
				bStatus = true;
			}
		}
	}
	else if ( isCharSerial( serial ) )
	{
		iterMapCells it = MapCellsChars.find( cell );
		if ( it != MapCellsChars.end() )
		{
			rawIterator it2 = find( it->second.begin(), it->second.end(), serial);
			if ( it2 != it->second.end())
			{
				it->second.erase( it2 );
				bStatus = true;
			}
		}
	}
	return bStatus;
}

// - Adds the item to a cell
bool cRegion::Add(cUObject* pi) 
{
	unsigned int uiCell;
	if (pi == NULL) 
		return false;
	uiCell = GetCell(pi->pos);

    if (uiCell<=32999) 
	{
		Add(uiCell, pi->serial); //set item in pointer array
		return true;
	} else return false;
}

// - Removes the item from a cell
bool cRegion::Remove(cUObject* pi)
{
	if (pi == NULL)
		return false;
	unsigned int uiCell = GetCell(pi->pos);

	if(uiCell <= 32999)
		Remove(uiCell, pi->serial);
	else 
		return false;

	return true;
}

//- Returns the cell the character/item is in
unsigned int cRegion::GetCell(const Coord_cl& pos)
{
	int cell = myGridx(pos.x) + myGridy(pos.y) + (myGridx(pos.x) * (ColSize-1));
	return (unsigned int) ((cell<0) ? 0 : cell);  // - Return 0 if negative otherwise cell #
}

cRegion::raw cRegion::GetCellEntries(UI32 cell, enDomain type)
{
	raw vecValue;
	iterMapCells iterData;
	
	switch (type)
	{
	case enAll:
		{
			iterMapCells it = MapCellsItems.find( cell );
			if ( it != MapCellsItems.end() )
				vecValue = it->second;
			iterMapCells it2 = MapCellsChars.find( cell );
			if ( it2 != MapCellsChars.end() )
			{
				vecValue.reserve( vecValue.size() + it2->second.size() );
				copy(it2->second.begin(), it2->second.end(), back_inserter(vecValue));
			}
		}
		break;
	case enCharsOnly:
		iterData = MapCellsChars.find( cell );
		if ( iterData != MapCellsChars.end() )
			return iterData->second;
		break;
	case enItemsOnly:
		iterData = MapCellsItems.find( cell );
		if ( iterData != MapCellsItems.end() )
			return iterData->second;
		break;
	default:
		clConsole.send("Warning: Fallout from GetCellEntries, domain = %i\n", type);
	}
	return vecValue;
}

// - Get starting grid for lookup 96x96 box
// - (we check the 8 surrounding cells and the cell char/item is in)
unsigned int cRegion::StartGrid(const Coord_cl& pos)
{
	int gridx=myGridx(pos.x)-1, gridy=myGridy(pos.y)-1;
	if (gridx<0) gridx=0;
	if (gridy<0) gridy=0;
	return (unsigned int) (gridx + gridy + (gridx * (ColSize-1)));
}

// ==========================================================================================
// Iterator4Chars
// ==========================================================================================

RegionIterator4Chars::RegionIterator4Chars(const Coord_cl& pos)
{
	cell = currentCell = ::cRegion::StartGrid( pos ); // they are centered.
	currentIndex = 0;
	position = pos; // remember where we started.
	currentCharacter = NULL;
}

bool RegionIterator4Chars::NextCell(void)
{
	++currentCell;	//adjacent cell
	if (currentCell%cRegion::GetColSize() > cell%cRegion::GetColSize()+1)
		currentCell += cRegion::GetColSize()-3;	//next col
	if (atEnd())
		return false;	// upper right corner of the box reached
	vecEntries = mapRegions->GetCellEntries(currentCell, enCharsOnly);
	currentIndex = vecEntries.begin();
	return true;
}

P_CHAR RegionIterator4Chars::GetData(void)
{
	if ( currentCharacter == NULL && !vecEntries.empty() )
		currentCharacter =  FindCharBySerial(*currentIndex);
	else if (currentCharacter->serial != *currentIndex)
		currentCharacter =  FindCharBySerial(*currentIndex);
	if (currentCharacter == NULL && currentIndex != vecEntries.end())
	{	// Dam, invalid entrie!
		// go next and recurse.
		(*this)++;
		return GetData();
	}
	return currentCharacter;
}

void RegionIterator4Chars::Begin(void)
{
	currentCell = cell;
	Coord_cl lastmapPos( cMapStuff::mapTileWidth(position)*8, cMapStuff::mapTileHeight(position)*8, position.z, position.map, position.plane );
	endCell = min ( cell + 2 * cRegion::GetColSize() + 2, static_cast<long unsigned int>(cRegion::GetCell(lastmapPos))); // Find out where we stop.
	vecEntries = mapRegions->GetCellEntries(currentCell, enCharsOnly);
	currentIndex = vecEntries.begin();
	while ( vecEntries.empty() && !atEnd() ) // make sure we start with something at least.
		(*this)++;
}

bool RegionIterator4Chars::atEnd(void) const
{
	return ( currentCell >= endCell ); // Equal to last region?
}

RegionIterator4Chars& RegionIterator4Chars::operator++ (int)
{
	++currentIndex;
	if (currentIndex >= vecEntries.end() || vecEntries.empty())
		NextCell();
	if ( vecEntries.empty() && !atEnd())
		(*this)++; // recurse;
	return *this;
}

RegionIterator4Chars& RegionIterator4Chars::operator= (const Coord_cl& pos)
{
	cell = currentCell = ::cRegion::GetCell(pos);
	position = pos; // Remember where we started for consistency check.
	return *this;
}

// ==========================================================================================
// Iterator4Items
// ==========================================================================================

RegionIterator4Items::RegionIterator4Items(const Coord_cl& pos)
{
	cell = currentCell = ::cRegion::StartGrid( pos ); // they are centered.
	position = pos; // remember where we started.
	currentItem = NULL;
}

bool RegionIterator4Items::NextCell(void)
{
	++currentCell;	//adjacent cell
	if (currentCell%cRegion::GetColSize() > cell%cRegion::GetColSize()+1)
		currentCell += cRegion::GetColSize()-3;	//next col
	if (atEnd())
		return false;	// upper right corner of the box reached
	vecEntries = mapRegions->GetCellEntries(currentCell, enItemsOnly);
	currentIndex = vecEntries.begin();
	return true;
}

P_ITEM RegionIterator4Items::GetData(void)
{
	if (currentItem == NULL && !vecEntries.empty())
		currentItem = FindItemBySerial(*currentIndex);
	else if (currentItem->serial != *currentIndex)
		currentItem =  FindItemBySerial(*currentIndex);
	if (currentItem == NULL && currentIndex != vecEntries.end())
	{	// Dam, invalid entrie!
		// go next and recurse.
		(*this)++;
		return GetData();
	}
	return currentItem;
}

void RegionIterator4Items::Begin(void)
{
	currentCell = cell;
	Coord_cl lastmapPos( cMapStuff::mapTileWidth(position)*8, cMapStuff::mapTileHeight(position)*8, position.z, position.map, position.plane );
	endCell = min ( cell + 2 * cRegion::GetColSize() + 2, static_cast<long unsigned int>(cRegion::GetCell(lastmapPos))); // Find out where we stop.
	vecEntries = mapRegions->GetCellEntries(currentCell, enItemsOnly);
	currentIndex = vecEntries.begin();
	while ( vecEntries.empty() && !atEnd() ) // make sure we start with something at least.
		(*this)++;
}

bool RegionIterator4Items::atEnd(void)
{
	return (!( currentCell < endCell )); // Equal to last region?
}

RegionIterator4Items& RegionIterator4Items::operator++ (int)
{
	++currentIndex;
	if (currentIndex >= vecEntries.end() || vecEntries.empty())
		NextCell();
	if ( vecEntries.empty() && !atEnd())
	{
		(*this)++; // recurse;
	}
	return *this;
}

RegionIterator4Items& RegionIterator4Items::operator= (const Coord_cl& pos)
{
	cell = currentCell = ::cRegion::GetCell(pos);
	position = pos; // Remember where we started for consistency check.
	return *this;
}


