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
#include "secmapstore.h"

//Class Method Implementations
//========================================================================================
/// Constructor
Secmapstore_cl::Secmapstore_cl() : Basestore_cl()
{

		mapSection.clear() ;
        iterData = NULL ;
		iterSection = mapSection.begin() ;

		ptrSectionmap = NULL ;

		sSectionName.erase() ;
		sSectionType.erase() ;
}

/*
//========================================================================================
/// Cosntructor of itself
Secmapstore_cl::Secmapstore_cl(const Secmapstore_cl& clData)
{
	(*this) = clData ;
}
*/
//========================================================================================
/// Desctructor
Secmapstore_cl::~Secmapstore_cl()
{
	// clear our map
	clear() ;
}

//========================================================================================
/// Clear out any that we have
void Secmapstore_cl::clear(char* szSection)
{
	string sSection(szSection) ;

	clear(sSection) ;
}

//========================================================================================
/// Clear out any that we have
void Secmapstore_cl::clear(string sSection)
{
	// clear just a single section
	iterSection = mapSection.find(sSection) ;
	if (iterSection != mapSection.end())
	{
		mapSection.erase(iterSection) ;
  	}


}

//========================================================================================
/// Clear out any that we have
void Secmapstore_cl::clear(char* szSection, char* szType)
{
	string sSection(szSection) ;
	string sType(szType) ;

	clear(sSection,szType) ;
}

//========================================================================================
/// Clear out any that we have
void Secmapstore_cl::clear(string sSection, string sType)
{
	// clear just a single section
	iterSection = mapSection.find(sSection)   ;
	if (iterSection != mapSection.end())
	{
		// now see if the type secton is there
		iterData = (iterSection->second).find(sType) ;
		if (iterData != (iterSection->second).end())
		{
			(iterSection->second).erase(iterData) ;
   		}
  	}


}



//========================================================================================
/// Clear out any that we have
void Secmapstore_cl::clear()
{

	mapSection.clear() ;
	iterData = NULL;
	iterSection = mapSection.begin() ;

}

//========================================================================================
/// Begining of a section
void Secmapstore_cl::startSection(string& sLine)
{

	// We expect the section to be two values , so we get to parse it
	mstring sData ;
	vector<mstring> vecData ;


	sData = sLine ;
	vecData = sData.splitAny(MYWHITESPACE) ;

	if (vecData.size() > 1)
	{
		// valid section type

		sSectionName = vecData[0] ;
		sSectionType = vecData[1] ;



		// Create the new section

		// See if this section has all ready been loaded
		if ((iterSection = mapSection.find(vecData[0]))!= mapSection.end())
		{
			// this section class was here, see if the type is new
			iterData = (iterSection->second).find(vecData[1]) ;
			if (iterData != (iterSection->second).end())
			{
				// this was a dup, we delete the old one
				(iterSection->second).erase(iterData) ;
    		}
        }
		ptrSectionmap = new Sectionmap_cl ;
	}
}



//========================================================================================
void Secmapstore_cl::addEntry(string& sLine)
{
	if (ptrSectionmap != NULL)
	{
		ptrSectionmap->insert(sLine) ;
	}
}
//========================================================================================
void Secmapstore_cl::endSection(string& sLine)
{
	// Insert the section into the map

	if (ptrSectionmap != NULL)
	{
		// First we need to get the SECTION if possible
		if ((iterSection = mapSection.find(sSectionName))== mapSection.end())
		{
			//we dont have this section in at all!
			wpMapSection* ptrWPMapSection ;
			ptrWPMapSection = new wpMapSection ;
			mapSection.insert(make_pair(sSectionName,(*ptrWPMapSection))) ;
			iterSection = mapSection.find(sSectionName) ;
			delete ptrWPMapSection ;
   		}

		if (iterSection != mapSection.end())
		{
			(iterSection->second).insert(make_pair(sSectionType,(*ptrSectionmap))) ;
			delete ptrSectionmap ;
			ptrSectionmap  = NULL ;
   		}

		sSectionName.erase() ;
		sSectionType.erase() ;

	}

}

//========================================================================================
// Retieve a section
Sectionmap_cl Secmapstore_cl::get(char* szSection, char* szType)
{
	string sSection(szSection) ;
	string sType(szType) ;
	return get(sSection,sType) ;
}

//========================================================================================
// Retieve a section
Sectionmap_cl Secmapstore_cl::get(string sSection, string sType)
{
	Sectionmap_cl clReturn ;

	iterSection = mapSection.find(sSection) ;

	if (iterSection != mapSection.end())
	{
		// ok, pull out the section
		iterData = (iterSection->second).find(sType) ;
		if (iterData != (iterSection->second).end())
		{


			clReturn = iterData->second ;
   		}
	}

	return clReturn ;
}
/*
//========================================================================================
// Operator functions
Secmapstore_cl&  Secmapstore_cl::operator=(const Secmapstore_cl& clData)
{
	// get our map

	mapSection =  clData.mapSection ;

	ptrSectionmap = NULL;
	iterSection = mapSection.end() ;

	iterData = NULL ;

	return (*this) ;
}
*/






