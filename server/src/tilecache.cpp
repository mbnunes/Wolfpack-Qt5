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

#include "tilecache.h"


//========================================================================================

//Class Method definitions

//========================================================================================
/// Constructor
TileCache_cl::TileCache_cl()
{
	vecStatic.clear() ;
	vecLand.clear() ;
	mapVerdata.clear() ;
}
//========================================================================================
/// Constructor of itself
TileCache_cl::TileCache_cl(const TileCache_cl& clData)
{
	(*this) = clData ;

}

//========================================================================================
TileCache_cl::TileCache_cl(string sDirectory)
{
	setDirectory(sDirectory) ;

	cacheData() ;


}

//========================================================================================
/// Desctructor
TileCache_cl::~TileCache_cl()
{
  clear() ;
}


//========================================================================================
/// Clear out any that we have
bool TileCache_cl::clear()
{
	bool bReturn = true ;
	vecStatic.clear() ;
	vecLand.clear() ;
	mapVerdata.clear() ;

	return bReturn ;
}

//========================================================================================
// Set our directory
void TileCache_cl::setDirectory(string sDirectory)
{

	sFile = sDirectory + "tiledata.mul"    ;
	sVerdata = sDirectory + "verdata.mul" ;

}

//========================================================================================
// Cache the tiles
bool TileCache_cl::cacheData()
{
	bool bReturn ;

	bReturn = processVerdata() ;
	if (bReturn)
	{
		bReturn = processLand() ;
		if (bReturn)
		{
			bReturn = processStatic() ;
   		}
     }
	 // we dont need our verdata info anuymore
	 mapVerdata.clear() ;

	 return bReturn ;


}

//========================================================================================
// get a land tile data
landtile_st TileCache_cl::getLandTile(UI16 uiId)
{
	landtile_st stLand ;
	if (uiId < vecLand.size() )
	{
		stLand = vecLand[uiId] ;
  	}
	return stLand ;
}

//========================================================================================
statictile_st TileCache_cl::getStaticTile(UI16 uiId)
{
	statictile_st stStatic ;
	if (uiId < vecStatic.size() )
	{
		stStatic = vecStatic[uiId]  ;
  	}
	return stStatic ;
}

//========================================================================================
TileCache_cl&  TileCache_cl::operator=(const TileCache_cl& clData)
{
	vecLand = clData.vecLand ;
	vecStatic = clData.vecStatic ;

	sFile = clData.sFile ;
	sVerdata = clData.sVerdata ;

	return (*this) ;
}

//========================================================================================
bool TileCache_cl::processVerdata()
{
	bool bReturn = false ;

	fstream fVerdata ;

	fVerdata.open(sVerdata.c_str(),ios::in|ios::binary) ;
	if (fVerdata.is_open())
	{
		bReturn = true ;
		UI32  uiHeader ;

		fVerdata.read((char*)(&uiHeader),4) ;

        #include "start_pack.h"
		struct temp_st
		{
			UI32 uiFileID ;
			SI32 siBlock ;
			SI32 siOffset ;
			SI32 siSize ;
			SI32 siExtra;
   		} PACK ;
		#include "end_pack.h"

		temp_st stData ;

		for (UI32 uiIndex=0;uiIndex < uiHeader; uiIndex++)
		{
			 fVerdata.read((char*)(&stData),sizeof(stData)) ;
			 if (stData.uiFileID == 0x1E)
			 {
			 	// It is our file type
				verdata_st stStore ;

				stStore.siOffset = stData.siOffset ;
				stStore.siSize = stData.siSize ;
				stStore.siExtra = stData.siExtra ;

				mapVerdata.insert(make_pair(stData.siBlock,  stStore)) ;
    		}
    	}

		fVerdata.close() ;

	}

	return bReturn ;
}
//========================================================================================
bool TileCache_cl::processLand()
{
	bool bReturn = false ;

	#include "start_pack.h"
	struct temp_st
	{
		UI32 uiFlag ;
		UI16 uiUnknown ;
		char szName[20] ;
	} PACK ;
	#include "end_pack.h"

	temp_st stInput ;

	// Data we are going to save

	landtile_st stLand ;

	fstream fTile ;
	fstream fVerdata ;

	bool bVerdata ;

	fVerdata.open(sVerdata.c_str(),ios::in | ios::binary)  ;

	fTile.open(sFile.c_str(),ios::in|ios::binary) ;
	if (fTile.is_open() && fVerdata.is_open())
	{
		bReturn = true ;
		// Lets size our vector
		vecLand.reserve(512*32) ;

		for (SI32 siIndex=0; siIndex< 512;siIndex++)
		{
			// For every block, we decide if we need to
			// get teh verdata one

			if ((iterVerdata=mapVerdata.find(siIndex)) != mapVerdata.end())
			{
				// verdata had this one
				bVerdata = true ;
				fVerdata.seekg(  (iterVerdata->second).siOffset + 4,ios::beg) ;
    		}
			else
			{
				bVerdata=false ;
				fTile.seekg( (siIndex * ((sizeof(stInput) * 32)+4)) + 4, ios::beg) ;
    		}
			// Ok, now we read in the 32 tiles
			for (UI16 uiTile=0 ; uiTile < 32 ; uiTile++)
			{
				if (bVerdata)
					fVerdata.read( (char*)&stInput,sizeof(stInput) ) ;
     			else
					fTile.read((char*)&stInput,sizeof(stInput) )  ;

				stLand.uiFlag = stInput.uiFlag ;
				stLand.sName = stInput.szName ;

				vecLand.push_back(stLand) ;
    		}

		}
		fTile.close() ;

		fVerdata.close() ;
	}

	return bReturn ;
}

//========================================================================================
bool TileCache_cl::processStatic()
{
	bool bReturn = false ;

	#include "start_pack.h"
	struct temp_st
	{

		UI32 uiFlag;
        SI08 siWeight;
        SI08 siLayer;
        SI32 siUnknownA;
        SI32 siAnim;
        SI08 siUnknownB;
        SI08 siUnknownC;
        SI08 siHeight;
        char szName[20];
	} PACK ;
	#include "end_pack.h"

	temp_st stInput ;

	// Data we are going to save

	statictile_st stStatic ;

	fstream fTile ;
	fstream fVerdata ;

	bool bVerdata ;

	fVerdata.open(sVerdata.c_str(),ios::in | ios::binary)  ;

	fTile.open(sFile.c_str(),ios::in|ios::binary) ;
	if (fTile.is_open() && fVerdata.is_open())
	{
		bReturn = true ;
		vecStatic.reserve(0xFFFF) ;
		// A tad differnt from land tiles, we index past all of the tiles
		SI32 siIndex = 0 ;

		while(!fTile.eof())
		{
			// For every block, we decide if we need to
			// get teh verdata one

			if ((iterVerdata=mapVerdata.find( siIndex + 512)) != mapVerdata.end())
			{
				// verdata had this one
				bVerdata = true ;
				fVerdata.seekg(  (iterVerdata->second).siOffset + 4,ios::beg) ;
    		}
			else
			{
				bVerdata=false ;
				fTile.seekg( (siIndex * ((sizeof(stInput) * 32)+4)) + 4 + (512 * (4 + (32*26))), ios::beg) ;
    		}
			// Ok, now we read in the 32 tiles
			for (UI16 uiTile=0 ; uiTile < 32 ; uiTile++)
			{
				if (bVerdata)
					fVerdata.read( (char*)&stInput,sizeof(stInput) ) ;
     			else
					fTile.read((char*)&stInput,sizeof(stInput) )  ;

				stStatic.uiFlag = stInput.uiFlag ;
				stStatic.sName = stInput.szName ;
				stStatic.siHeight = stInput.siHeight ;
				stStatic.siLayer = stInput.siLayer ;
				stStatic.siGump = stInput.siAnim ;

				vecStatic.push_back(stStatic) ;
    		}
			siIndex++ ;

		}

	}
	else
	{
		cerr << "Error opening tile data " <<endl;
	}
	fTile.close() ;

	fVerdata.close() ;
	return bReturn ;
}

