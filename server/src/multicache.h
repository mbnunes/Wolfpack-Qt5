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
#if !defined(__MULTICACHE_H__)
#define __MULTICACHE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>

using namespace std ;


// Third Party includes



//Forward class declaration

class MultiCache_cl ;

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



// Typedefs



//Class definitions
//##ModelId=3C5D92DA0063
class MultiCache_cl
{
public:
        /// Constructor
	//##ModelId=3C5D92DA008B
        MultiCache_cl()   ;
        /// Cosntructor of itself
	//##ModelId=3C5D92DA0095
        MultiCache_cl(const MultiCache_cl& clData) ;

	//##ModelId=3C5D92DA00A0
		MultiCache_cl(string sDirectory) ;
        /// Desctructor
	//##ModelId=3C5D92DA00B3
        ~MultiCache_cl() ;

        /// Clear out any that we have

	//##ModelId=3C5D92DA00BD
		bool clear( );


		// Set our directory
	//##ModelId=3C5D92DA00C7
		void setDirectory(string sDirectory) ;

		// Cache the tiles
	//##ModelId=3C5D92DA00DB
		bool cacheData() ;


	//##ModelId=3C5D92DA00E5
		MultiCache_cl&  operator=(const MultiCache_cl& clData) ;


	//##ModelId=3C5D92DA00F9
		bool isValid(SI32 siId) ;

private:
	//##ModelId=3C5D92DA010D
		bool processVerdata() ;
	//##ModelId=3C5D92DA0117
		bool processMap() ;

private:


/*
		string sMap ;
		string sStatic ;
		string sStaIdx ;
		string sVerdata ;


		map<UI16,YMapData>  mapX;
		map<UI16,YMapData>::iterator iterX ;

		//YMapData	mapY ;
		YMapData::iterator iterY ;





		// We read in the verdatamul for each file type, a great room for speed improvment on startup later

		map<SI32,verdata_st>  mapVerdataMap ;
		map<SI32,verdata_st>  mapVerdataStatics ;
		map<SI32,verdata_st>  mapVerdataStaIdx ;

		map<SI32,verdata_st>::iterator iterVerdata ;

*/




};
//==========================================================================================

#endif



