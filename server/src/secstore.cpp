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

// Wolfapck Includes
#include "secstore.h"

// Class Method Implementations
//========================================================================================
/// Constructor
Secstore_cl::Secstore_cl() : Basestore_cl()
{
	mapData.clear();
	iterData = mapData.begin();
	
	ptrSection = NULL;
	
	sSectionName.erase();
}
/*
//========================================================================================
/// Cosntructor of itself
Secstore_cl::Secstore_cl(const Secstore_cl& clData)
{
	(*this) = clData;
}
*/
//========================================================================================
/// Desctructor
Secstore_cl::~Secstore_cl()
{
	// clear our map
	clear();
}

//========================================================================================
/// Clear out any that we have
void Secstore_cl::clear()
{
	mapData.clear();
	iterData = mapData.begin();

}

//========================================================================================
/// Begining of a section
void Secstore_cl::startSection(string& sLine)
{
	// see if we all ready had this section
	iterData = mapData.find(sLine);
	if (iterData != mapData.end())
	{
		// This is a duplicate!
		// Delete the old one
		mapData.erase(iterData);
	}
	// create a new section
	ptrSection = new Section_cl;
	sSectionName = sLine;
}

//========================================================================================
void Secstore_cl::addEntry(string& sLine)
{
	if (ptrSection != NULL)
	{
		ptrSection->insert(sLine);
	}
}
//========================================================================================
void Secstore_cl::endSection(string& sLine)
{
	// Insert the section into the map
	if (ptrSection != NULL)
	{
		mapData.insert(make_pair(sSectionName, (*ptrSection)));
		
		sSectionName.erase();
		
		// Delete the old section
		delete ptrSection;
		ptrSection = NULL;
	}
}
//=======================================================================================
// Retrieve a section (chararcter input)
Section_cl Secstore_cl::get(char* szLine)
{
	string sLine ;
	sLine = szLine ;
	return get(sLine);
}

//========================================================================================
// Retieve a section
Section_cl Secstore_cl::get(string sLine)
{
	Section_cl clReturn;
	
	iterData = mapData.find(sLine);
	
	if (iterData != mapData.end())
	{
		// ok, pull out the section
		
		clReturn = iterData->second;
	}
	
	return clReturn;
}

/*
//========================================================================================
// Operator functions
Secstore_cl&  Secstore_cl::operator=(const Secstore_cl& clData)
{
	// get our map
	
	mapData = clData.mapData;
	iterData = clData.iterData;
	// make a copy of the section if any
	if (clData.ptrSection != NULL)
	{
		ptrSection = new Section_cl;
		
		(*ptrSection) = (*clData.ptrSection);
	}
	else
		ptrSection = NULL;
	
	sSectionName = clData.sSectionName;
	
	return (*this);
}
*/






