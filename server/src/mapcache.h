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

// Third Party includes



//Forward class declaration

class MapCache;

//Wolfpack Includes
#include "wpconsole.h"
#include "mulstructs.h"
#include "tilecache.h"

//Any externals we need

extern WPConsole_cl clConsole ;
// WE Assume this tiledata has all ready been initalized!
extern TileCache_cl clTiledata ;

// Any specific structures

struct mapcache_st
{
	UI16	uiTileId  ;
	SI16	siZAxis ;
	SI16	siHeight ;
	UI32	uiFlag ;

};

struct mapsize_st
{
	UI16	Width;
	UI16	Height;
};

// Typedefs
typedef map<UI16, vector<mapcache_st> >  YMapData ;

//Class definitions
class MapCache_cl
{
public:
    /// Constructor
    MapCache_cl()   ;
    /// Cosntructor of itself
    MapCache_cl(const MapCache_cl& clData) ;
	MapCache_cl(string sDirectory) ;
    /// Desctructor
	~MapCache_cl() ;

    /// Clear out any that we have

	bool clear( );
	void initMapSize(void);

	// Set our directory
	void setDirectory(string sDirectory);
	// Get current mapsize
	void getMapSize(UI16 mapID, UI16 &uiWidth, UI16 &uiHeight);

	// Cache the tiles
	bool cacheData() ;

	// get a land tile data
	vector<mapcache_st> get(UI16 uiX, UI16 uiY);

	MapCache_cl&  operator=(const MapCache_cl& clData) ;

private:
	bool processVerdata() ;
	bool processMap() ;

private:

	string sMap ;
	string sStatic ;
	string sStaIdx ;
	string sVerdata ;
	map<UI16,mapsize_st>			mapSize;
	map<UI16,mapsize_st>::iterator	iterSize;
	map<UI16,YMapData>				mapX;
	map<UI16,YMapData>::iterator	iterX ;

	//YMapData	mapY ;
	YMapData::iterator iterY ;



	// We read in the verdatamul for each file type, a great room for speed improvment on startup later
	map<SI32,verdata_st>  mapVerdataMap;
	map<SI32,verdata_st>  mapVerdataStatics;
	map<SI32,verdata_st>  mapVerdataStaIdx;
	map<SI32,verdata_st>::iterator iterVerdata;

};
//==========================================================================================

#endif



