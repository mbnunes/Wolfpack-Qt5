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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

//Wolfpack Includes

#include "multicache.h"


//========================================================================================

//Class Method definitions

//========================================================================================
/// Constructor
MultiCache_cl::MultiCache_cl()
{
	clear() ;
}
//========================================================================================
/// Constructor of itself
MultiCache_cl::MultiCache_cl(const MultiCache_cl& clData)
{
	(*this) = clData ;

}

//========================================================================================
MultiCache_cl::MultiCache_cl(string sDirectory)
{
	setDirectory(sDirectory) ;

	cacheData() ;


}

//========================================================================================
/// Desctructor
MultiCache_cl::~MultiCache_cl()
{
  clear() ;
}


//========================================================================================
/// Clear out any that we have
bool MultiCache_cl::clear()
{
	bool bReturn = true ;


	return bReturn ;
}

//========================================================================================
// Set our directory
void MultiCache_cl::setDirectory(string sDirectory)
{



}

//========================================================================================
// Cache the tiles
bool MultiCache_cl::cacheData()
{
	clear() ;

	// The current code is for a LOT of memeory (1GB)
	// So comment out. A better mechanism would be to cache block /character

	bool bReturn ;

	bReturn = processVerdata() ;
//	if (bReturn)
//	{
//		bReturn = processMap() ;

//     }
	 // we dont need our verdata info anuymore
//	 mapVerdataMap.clear() ;
//	 mapVerdataStatics.clear() ;
//	 mapVerdataStaIdx.clear() ;

	 return bReturn ;


}

//========================================================================================
// get a land tile data


//========================================================================================


//========================================================================================
MultiCache_cl&  MultiCache_cl::operator=(const MultiCache_cl& clData)
{


	return (*this) ;
}

//========================================================================================
bool MultiCache_cl::processVerdata()
{
	bool bReturn = false ;

/*
	fstream fVerdata ;

	fVerdata.open(sVerdata.c_str(),ios::in |ios::binary) ;
	if (fVerdata.is_open())
	{
		bReturn = true ;
		SI32  uiHeader ;

		fVerdata.read((char*)(&uiHeader),4) ;

		struct temp_st
		{
			UI32 uiFileID ;
			SI32 siBlock ;
			SI32 siOffset ;
			SI32 siSize ;
			SI32 siExtra;
   		} PACK ;

		temp_st stData ;
		// It is our file type
		verdata_st stStore ;
		for (UI32 uiIndex=0;uiIndex < uiHeader; uiIndex++)
		{
			fVerdata.read((char*)(&stData),sizeof(stData)) ;
			stStore.siOffset = stData.siOffset ;
			stStore.siSize = stData.siSize ;
			stStore.siExtra = stData.siExtra ;
			switch( stData.uiFileID)
            {
				case 0x00:

					mapVerdataMap.insert(make_pair(stData.siBlock,  stStore)) ;
					break ;
     			case 0x01:
					mapVerdataStaIdx.insert(make_pair(stData.siBlock,  stStore)) ;
					break ;
     			case 0x02:
					mapVerdataStatics.insert(make_pair(stData.siBlock,  stStore)) ;
					break ;
     			default:
					break ;
    		}
    	}

		fVerdata.close() ;

	}
	else
	{
		cerr << "error opening verdata:"<<sVerdata <<endl;
	}
*/
	return bReturn ;
}
//========================================================================================

//========================================================================================
bool MultiCache_cl::isValid(SI32 siId)
{
	return true;
}

