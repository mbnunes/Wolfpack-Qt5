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
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

//Wolfpack Includes

#include "mapcache.h"


//========================================================================================

//Class Method definitions

//========================================================================================
/// Constructor
//##ModelId=3C5D92E20105
MapCache_cl::MapCache_cl()
{
	clear();
	initMapSize();
}
//========================================================================================
/// Constructor of itself
//##ModelId=3C5D92E2010F
MapCache_cl::MapCache_cl(const MapCache_cl& clData)
{
	(*this) = clData ;
}

//========================================================================================
//##ModelId=3C5D92E20123
MapCache_cl::MapCache_cl(string sDirectory)
{
	setDirectory(sDirectory);

	cacheData();
}

//========================================================================================
/// Desctructor
//##ModelId=3C5D92E2012E
MapCache_cl::~MapCache_cl()
{
  clear() ;
}

//##ModelId=3C5D92E20141
void MapCache_cl::initMapSize(void)
{
	mapsize_st temp;
	temp.Height = 512;
	temp.Width  = 768;
	mapSize.insert(make_pair(0, temp));		// Britannia Trammel
	mapSize.insert(make_pair(1, temp));		// Britannia Felluca

	temp.Height = 200;
	temp.Width  = 288;
	mapSize.insert(make_pair(2, temp));		// Ilshenar Trammel
	mapSize.insert(make_pair(3, temp));		// Ilshenar Felluca

}



//========================================================================================
/// Clear out any that we have
//##ModelId=3C5D92E20137
bool MapCache_cl::clear()
{
	bool bReturn = true ;
	mapX.clear() ;
	// The destructor of the vector takes care of the vector data
//	mapY.clear() ;
	mapVerdataMap.clear() ;
	mapVerdataStatics.clear() ;
	mapVerdataStaIdx.clear() ;

	mapSize.clear();
	return bReturn ;
}

//========================================================================================
// Set our directory
//##ModelId=3C5D92E20173
void MapCache_cl::setDirectory(string sDirectory)
{

	sMap = sDirectory + "map0.mul"    ;
	sVerdata = sDirectory + "verdata.mul" ;
	sStatic = sDirectory  +"statics0.mul" ;
	sStaIdx = sDirectory + "staidx0.mul"     ;

}

//========================================================================================
// Cache the tiles
//##ModelId=3C5D92E201A5
bool MapCache_cl::cacheData()
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
//##ModelId=3C5D92E2017E
void MapCache_cl::getMapSize(UI16 uiMapID, UI16 &uiWidth, UI16 &uiHeight)
{
	if ((iterSize = mapSize.find(uiMapID)) != mapSize.end())
	{
		uiWidth = iterSize->second.Width;
		uiHeight = iterSize->second.Height;
	}
	else
		uiWidth = uiHeight = 0;
}

//========================================================================================
// get a land tile data
//##ModelId=3C5D92E201AF
vector<mapcache_st> MapCache_cl::get(UI16 uiX, UI16 uiY)
{
	vector<mapcache_st> vecReturn ;

	mapcache_st stRecord ;

	// First determine the BLOCK of we need

	UI32 uiBlock = ((uiX/8) * 512) + (uiY/8);

	// Determine the xOffset
	UI16 uiXIndex = uiX%8 ;
	UI16 uiYIndex = uiY%8 ;


	// Define the struct we need to read in the data

	#include "start_pack.h"
	struct tempmap_st
	{

		UI16 uiTileId ;
        SI08 siZAxis ;
	} PACK ;
	#include "end_pack.h"

	tempmap_st stMapInput[64];

	#include "start_pack.h"
	struct tempidx_st
	{
		UI32 uiOffset ;
		UI32 uiSize ;
		UI32 uiDummy ;
    } PACK ;
	#include "end_pack.h"

	tempidx_st stIdxInput ;

	#include "start_pack.h"
	struct tempstatic_st
	{
		UI16 uiTileId ;
		UI08 uiXOffset ;
		UI08 uiYOffset ;
		SI08 siZAxis ;
		UI16 uiDummy ;
	} PACK ;
	#include "end_pack.h"

//	tempstatic_st stStaInput ;

	bool bVerdata = false ;
	// OPen up our files we need to get to

	fstream fMap ;
	fstream fIdx ;
	fstream fVerdata ;
	fstream fStatic;

	// See if the map block is in verdata, if yes, dont open the map file

	iterVerdata = mapVerdataMap.find(uiBlock) ;
	if (mapVerdataMap.end() == iterVerdata)
	{
		// Not in the verdata
		fMap.open(sMap.c_str(), ios::in|ios::binary);
		if (fMap.is_open())
		{
			fMap.seekg((uiBlock*196 ) +4,ios::beg) ;
			fMap.read( (char*)stMapInput, 192) ;
			fMap.close() ;
		}
	}
	else
	{
		fVerdata.open(sVerdata.c_str(),ios::in | ios::binary)  ;
		if (fVerdata.is_open())
		{
			fVerdata.seekg((iterVerdata->second).siOffset+4, ios::beg) ;
			fVerdata.read( (char*)stMapInput, 192) ;
		}
	}
	// Get the statics that go with this
	// Now see about statics
	// if they patch, they seem to never patch the idx file, so we are going to assume if an
	// entry isnt for the statics block itself, it isnt patched

	UI32 uiAmountRead = 0 ;
	// Make a vector for our statics (we assume memory is better then repeated file access)
	tempstatic_st* ptrStadata=NULL ;

	if ((iterVerdata=mapVerdataStatics.find(uiBlock)) != mapVerdataStatics.end())
	{
		if (!fVerdata.is_open())
			fVerdata.open(sVerdata.c_str(),ios::in | ios::binary)  ;
		bVerdata = true ;
		fVerdata.seekg((iterVerdata->second).siOffset, ios::beg) ;
		uiAmountRead = (iterVerdata->second).siSize ;
	}
	else
	{
		bVerdata = false ;
		// not patched, so we need to cross index
		fIdx.open(sStaIdx.c_str(),ios::in|ios::binary) ;
		fStatic.open(sStatic.c_str(),ios::in|ios::binary) ;

		fIdx.seekg(uiBlock*12, ios::beg) ;
		fIdx.read( (char*) &stIdxInput,12) ;
		fIdx.close() ;
		if (stIdxInput.uiOffset != 0xFFFFFFFF)
		{
		// There really are statics here
			fStatic.seekg(stIdxInput.uiOffset,ios::beg) ;
			uiAmountRead = stIdxInput.uiSize ;
   		}
		else
			uiAmountRead = 0 ;
	}

	if (uiAmountRead != 0)
	{
		ptrStadata = new tempstatic_st[uiAmountRead/sizeof(tempstatic_st)] ;
		if (bVerdata)
		{
			fVerdata.read((char*) ptrStadata,uiAmountRead) ;
			fVerdata.close() ;
   		}
		else
		{
			fStatic.read((char*) ptrStadata,uiAmountRead);
			fStatic.close();
		}
		// convert the amount from bytes to records ;
		uiAmountRead = uiAmountRead/7;
	}

	// We need to find all the stuff here for this why
	// We build a vector of data
	// first entry is always for the land tile!
	stRecord.uiTileId = (stMapInput[uiXIndex + (uiYIndex*8)]).uiTileId ;
	stRecord.siZAxis =  (stMapInput[uiXIndex + (uiYIndex*8)]).siZAxis ;
	// we get the two tiledata from the lookup
	landtile_st stLand;
	stLand = clTiledata.getLandTile(stRecord.uiTileId);
	stRecord.uiFlag = stLand.uiFlag;
	stRecord.siHeight = 0;
	// push this on the vector
	vecReturn.push_back(stRecord);
	// We now have to scan throught all the statics, and see what we need
	for (UI32 ii=0; ii < uiAmountRead; ii++)
	{
		if (uiXIndex ==(ptrStadata+ii)->uiXOffset)
		{
			if (uiYIndex ==  (ptrStadata+ii)->uiYOffset)
			{
				// this static matches!
				stRecord.uiTileId =   (ptrStadata+ii)->uiTileId ;
				stRecord.siZAxis =   (ptrStadata+ii)->siZAxis ;
				statictile_st stStatic ;
				stStatic = clTiledata.getStaticTile(stRecord.uiTileId);
				stRecord.uiFlag = stStatic.uiFlag ;
				stRecord.siHeight = stStatic.siHeight ;
				vecReturn.push_back(stRecord) ;
			}
		}
	}
	// OK, vector is built
	// Delete the ptr we made
	if (ptrStadata != NULL)
	{
		delete [] ptrStadata ;
		ptrStadata = NULL ;
	}


/*

	// First find the map in the X
	iterX = mapX.find(uiX) ;
	if (iterX != mapX.end())
	{
		iterY = (iterX->second).find(uiY) ;
		if (iterY != (iterX->second).end())
		{
			vecReturn = iterY->second ;
   		}
		else
		{
			cerr << "Error finding Y coord : " << uiY << " in map cache" <<endl;
   		}

	}
	else
	{
		cerr << "Error finding X coord : " << uiX << " in map cache" <<endl;
  	}
*/
	return vecReturn;
}

//========================================================================================


//========================================================================================
//##ModelId=3C5D92E201C4
MapCache_cl&  MapCache_cl::operator=(const MapCache_cl& clData)
{
	mapX = clData.mapX ;
//	mapY = clData.mapY ;
	mapVerdataMap = clData.mapVerdataMap ;
	mapVerdataStatics = clData.mapVerdataStatics ;
	mapVerdataStaIdx = clData.mapVerdataStaIdx ;

	sMap = clData.sMap ;
	sVerdata = clData.sVerdata ;
	sStatic = clData.sStatic ;
	sStaIdx = clData.sStaIdx ;

	return (*this) ;
}

//========================================================================================
//##ModelId=3C5D92E201D7
bool MapCache_cl::processVerdata()
{
	bool bReturn = false ;

	fstream fVerdata ;

	fVerdata.open(sVerdata.c_str(),ios::in |ios::binary);
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
		cerr << "No verdata.mul found, continuing:"<<sVerdata <<endl;
	}

	return bReturn ;
}
//========================================================================================

//========================================================================================
//##ModelId=3C5D92E201EB
bool MapCache_cl::processMap()
{
	bool bReturn = false ;

	#include "start_pack.h"
	struct tempmap_st
	{

		UI16 uiTileId ;
        SI08 siZAxis ;
	} PACK ;
	#include "end_pack.h"

	tempmap_st stMapInput[64] ;

	#include "start_pack.h"
	struct tempidx_st
	{
		UI32 uiOffset ;
		UI32 uiSize ;
		UI32 uiDummy ;
    } PACK ;
	#include "end_pack.h"

	tempidx_st stIdxInput ;

	#include "start_pack.h"
	struct tempstatic_st
	{
		UI16 uiTileId ;
		UI08 uiXOffset ;
		UI08 uiYOffset ;
		SI08 siZAxis ;
		UI16 uiDummy ;
	} PACK ;
	#include "end_pack.h"

//	tempstatic_st stStaInput ;
	// Data we are going to save

//	mapcache_st stData ;

	UI16 uiX = 0 ;
	UI16 uiXIndex = 0 ;
	UI16 uiYIndex = 0 ;
	UI16 uiY = 0 ;

	fstream fMap ;
	fstream fIdx ;
	fstream fVerdata ;
	fstream fStatic;

	bool bVerdata ;

	fVerdata.open(sVerdata.c_str(),ios::in | ios::binary)  ;

	fIdx.open(sStaIdx.c_str(),ios::in|ios::binary) ;
	fStatic.open(sStatic.c_str(),ios::in|ios::binary) ;
	fMap.open(sMap.c_str(),ios::in|ios::binary) ;




	UI16 uiBlockX = 0 ;

	UI16 uiBlockY = 0 ;

	UI32 uiBlock = 0 ;

	if (fStatic.is_open() && fVerdata.is_open() && fIdx.is_open() & fMap.is_open())
	{
		bReturn = true ;

// 		For now we are only handling britiania
		for(uiBlockX=0; uiBlockX < 768 ; uiBlockX++ )
		{
//
			cout << "X Block : " << uiBlockX << endl;
			for (uiBlockY=0;uiBlockY < 512; uiBlockY++)
			{
				uiBlock =  (uiBlockX*512) + uiBlockY ;
				  // first see if verdata has a new block for this
				if ((iterVerdata=mapVerdataMap.find(uiBlock)) != mapVerdataMap.end())
				{
					// A verdata patch
					bVerdata = true ;
					fVerdata.seekg((iterVerdata->second).siOffset+4, ios::beg) ;
					fVerdata.read( (char*)stMapInput, 192) ;
				}
				else
				{
					bVerdata = false ;
					fMap.seekg((uiBlock*196 ) +4,ios::beg) ;
					fMap.read( (char*)stMapInput, 192) ;
      			}

				// Now see about statics
				// if they patch, they seem to never patch the idx file, so we are going to assume if an
				// entry isnt for the statics block itself, it isnt patched

				UI32 uiAmountRead = 0 ;
				if ((iterVerdata=mapVerdataStatics.find(uiBlock)) != mapVerdataStatics.end())
				{
					bVerdata = true ;
					fVerdata.seekg((iterVerdata->second).siOffset, ios::beg) ;
					uiAmountRead = (iterVerdata->second).siSize ;
     			}
				else
				{
					bVerdata = false ;
					// not patched, so we need to cross index
					fIdx.seekg(uiBlock*12, ios::beg) ;
					fIdx.read( (char*) &stIdxInput,12) ;

					if (stIdxInput.uiOffset != 0xFFFFFFFF)
					{
						// There really are statics here
						fStatic.seekg(stIdxInput.uiOffset,ios::beg) ;
						uiAmountRead = stIdxInput.uiSize ;
      				}
					else
						uiAmountRead = 0 ;
      			}


				// Our streams are now pointing to the correct statics entry, we have the right stream,
				// And the amount of data to read
				// we have all the land tiles in the array from the earlier read
				// So , we are now ready to sort it all out

				// Make a vector for our statics (we assume memory is better then repeated file access)
				tempstatic_st* ptrStadata=NULL ;
				if (uiAmountRead != 0)
				{
					ptrStadata = new tempstatic_st[uiAmountRead/sizeof(tempstatic_st)] ;

     			}
				if (bVerdata)
					fVerdata.read((char*) ptrStadata,uiAmountRead) ;
     			else
					fStatic.read((char*) ptrStadata,uiAmountRead)  ;

				// We now have in memory, the block of land tiles, and statics associated with this block

				// So now we get to actually store the data


				// for this block, we run the X's, then y's

				for (uiYIndex=0;uiYIndex < 8 ; uiYIndex++)
				{
					for (uiXIndex =0 ; uiXIndex < 8 ; uiXIndex++)
					{

						uiX = static_cast<UI16>((uiBlockX*8)) + uiXIndex ;
						uiY = static_cast<UI16>((uiBlockY*8)) + uiYIndex ;

						// See if we all ready have a section for this X

						if ((iterX=mapX.find(uiX))== mapX.end())
						{
						 	// We dont have one yet!
							YMapData* ptrY ;
							ptrY = new YMapData ;
							if (ptrY != NULL)
							{
								mapX.insert(make_pair(uiX,(*ptrY))) ;
								delete ptrY ;
        					}
        					iterX = mapX.find(uiX) ;
             			}
						if (iterX != mapX.end())
						{
							// We need to find all the stuff here for this why
							// We build a vector of data
							vector<mapcache_st>  vecData ;
							mapcache_st stRecord ;
							// first entry is always for the land tile!
							stRecord.uiTileId = (stMapInput[uiXIndex + (uiYIndex*8)]).uiTileId ;
							stRecord.siZAxis =  (stMapInput[uiXIndex + (uiYIndex*8)]).siZAxis ;
							// we get the two tiledata from the lookup
							landtile_st stLand ;
							stLand = clTiledata.getLandTile(stRecord.uiTileId) ;
							stRecord.uiFlag = stLand.uiFlag ;
							stRecord.siHeight = 0 ;

							// push this on the vector

							vecData.push_back(stRecord) ;

							// We now have to scan throught all the statics, and see what we need
							uiAmountRead = uiAmountRead/7 ;
							for (UI32 ii=0; ii < uiAmountRead; ii++)
							{
								if (uiXIndex ==(ptrStadata+ii)->uiXOffset)
								{
									if (uiYIndex ==  (ptrStadata+ii)->uiYOffset)
									{
										// this static matches!
										stRecord.uiTileId =   (ptrStadata+ii)->uiTileId ;
										stRecord.siZAxis =   (ptrStadata+ii)->siZAxis ;
										statictile_st stStatic ;
										stStatic = clTiledata.getStaticTile(stRecord.uiTileId);
										stRecord.uiFlag = stStatic.uiFlag ;
										stRecord.siHeight = stStatic.siHeight ;
          								vecData.push_back(stRecord) ;
                  					}
                       			}
                          	}
							// OK, vector is built, insert into the map!

							(iterX->second).insert(make_pair(uiY,vecData)) ;
       					}
					}
				 }


				 // release our memeory that we got for the array
				 if (ptrStadata != NULL)
				 {
				 	delete [] ptrStadata ;
					ptrStadata = NULL ;
      			}

          	}


		}
  	}
	else
	{
		cerr << "error opening map files" <<endl;
	}

	fMap.close() ;

	fVerdata.close() ;
	fIdx.close() ;
	fStatic.close() ;

	return bReturn ;
}

