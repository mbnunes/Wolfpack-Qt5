//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//      Copyright 2001 by holders identified in authors.txt
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
//	Wolfpack Homepage: http://wpdev.sourceforge.net/
//========================================================================================


// Wolfpack Includes

#include "section.h"

// Method implementations


//=========================================================================================
/// Constructor
Section_cl::Section_cl()
{
	// Initilize our iterators
	iterData = lisData.begin();
}
//=========================================================================================
/// Cosntructor of itself
Section_cl::Section_cl(const Section_cl& clData)
{
	(*this) = clData;
	iterData = lisData.begin();
}

//=========================================================================================
/// Desctructor
Section_cl::~Section_cl()
{
	// Clear our list
	lisData.clear();
}

//=========================================================================================
// Return the number in the list
SI32 Section_cl::size()
{
	return lisData.size();
}

//=========================================================================================
// Set the offset into the list
void Section_cl::offset(UI32 siOffset)
{
	// First check that the offset isn't beyond the size
	
	if (siOffset < lisData.size())
	{
		iterData = lisData.begin();
		for (UI32 siIndex = 0; siIndex < siOffset; siIndex++)
		{
			iterData++;
		}
		if (iterData == lisData.end())
		{
			// We ahd an error setting it
			iterData = lisData.begin();
		}
	}
}
//=========================================================================================
// Pop an element and increment offset

string Section_cl::pop()
{
    string sData;
	
    if (iterData != lisData.end())
    {
        sData = (*iterData);
		iterData++;
	}
	return sData;
}

//=========================================================================================
/// Operator=
Section_cl&    Section_cl::operator=(const Section_cl& clSection_cl)
{
	// clear out our old list
	lisData.clear();
	
	// Copy ouver the lis data
	lisData = clSection_cl.lisData;
	
	iterData = clSection_cl.iterData;
	
	return (*this);
}
//=========================================================================================
/// Clear out the list
void Section_cl::clear()
{
	lisData.clear();
	iterData = lisData.begin();
}

//=========================================================================================
// Insert an elment into the Section_cl
void Section_cl::insert(string sInput)
{
	lisData.push_back(sInput);
}

