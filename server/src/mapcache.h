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
#if !defined(__MAPCACHE_H__)
#define __MAPCACHE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>

using namespace std;


// Third Party includes



//Forward class declaration

class MapCache;

//Wolfpack Includes


#include "wpconsole.h"

#include "mstring.h"
#include "mulstructs.h"
#include "tilecache.h"


//Any externals we need

extern WPConsole_cl clConsole ;
// WE Assume this tiledata has all ready been initalized!
extern TileCache_cl clTiledata ;

// Any specific structures

//##ModelId=3C5D92E1023A
struct mapcache_st
{
	//##ModelId=3C5D92E10263
	UI16	uiTileId  ;
	//##ModelId=3C5D92E1028B
	SI16	siZAxis ;
	//##ModelId=3C5D92E102A9
	SI16	siHeight ;
	//##ModelId=3C5D92E102D1
	UI32	uiFlag ;

};

//##ModelId=3C5D92E1030C
struct mapsize_st
{
	//##ModelId=3C5D92E10349
	UI16	Width;
	//##ModelId=3C5D92E10371
	UI16	Height;
};

// Typedefs

//##ModelId=3C5D92E1038E
typedef map<UI16, vector<mapcache_st> >  YMapData ;

//Class definitions
//##ModelId=3C5D92E200DC
class MapCache_cl
{
public:
    /// Constructor
	//##ModelId=3C5D92E20105
    MapCache_cl()   ;
    /// Cosntructor of itself
	//##ModelId=3C5D92E2010F
    MapCache_cl(const MapCache_cl& clData) ;
	//##ModelId=3C5D92E20123
	MapCache_cl(string sDirectory) ;
    /// Desctructor
	//##ModelId=3C5D92E2012E
	~MapCache_cl() ;

    /// Clear out any that we have

	//##ModelId=3C5D92E20137
	bool clear( );
	//##ModelId=3C5D92E20141
	void initMapSize(void);

	// Set our directory
	//##ModelId=3C5D92E20173
	void setDirectory(string sDirectory);
	// Get current mapsize
	//##ModelId=3C5D92E2017E
	void getMapSize(UI16 mapID, UI16 &uiWidth, UI16 &uiHeight);

	// Cache the tiles
	//##ModelId=3C5D92E201A5
	bool cacheData() ;

	// get a land tile data
	//##ModelId=3C5D92E201AF
	vector<mapcache_st> get(UI16 uiX, UI16 uiY);




	//##ModelId=3C5D92E201C4
	MapCache_cl&  operator=(const MapCache_cl& clData) ;

private:
	//##ModelId=3C5D92E201D7
	bool processVerdata() ;
	//##ModelId=3C5D92E201EB
	bool processMap() ;

private:


	//##ModelId=3C5D92E20200
	string sMap ;
	//##ModelId=3C5D92E2021E
	string sStatic ;
	//##ModelId=3C5D92E2023C
	string sStaIdx ;
	//##ModelId=3C5D92E2025A
	string sVerdata ;

	//##ModelId=3C5D92E202A1
	map<UI16,mapsize_st>			mapSize;
	//##ModelId=3C5D92E202D2
	map<UI16,mapsize_st>::iterator	iterSize;
	//##ModelId=3C5D92E20336
	map<UI16,YMapData>				mapX;
	//##ModelId=3C5D92E20372
	map<UI16,YMapData>::iterator	iterX ;

	//YMapData	mapY ;
	//##ModelId=3C5D92E203A4
	YMapData::iterator iterY ;



	// We read in the verdatamul for each file type, a great room for speed improvment on startup later
	//##ModelId=3C5D92E203CD
	map<SI32,verdata_st>  mapVerdataMap;
	//##ModelId=3C5D92E30021
	map<SI32,verdata_st>  mapVerdataStatics;
	//##ModelId=3C5D92E3005D
	map<SI32,verdata_st>  mapVerdataStaIdx;

	//##ModelId=3C5D92E30099
	map<SI32,verdata_st>::iterator iterVerdata;

};
//==========================================================================================

#endif



