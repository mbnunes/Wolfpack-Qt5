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

#if !defined(__TILECACHE_H__)
#define __TILECACHE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

using namespace std ;


// Third Party includes



//Forward class declaration

class TileCache_cl ;

//Wolfpack Includes


#include "wpconsole.h"

#include "mstring.h"
#include "mulstructs.h"
#include "tileflags.h"
//Any externals we need

extern WPConsole_cl clConsole ;

// Any specific structures

//##ModelId=3C5D92B40267
struct statictile_st
{
	//##ModelId=3C5D92B40286
	string sName ;
	//##ModelId=3C5D92B402A4
	UI32	uiFlag ;
	//##ModelId=3C5D92B402C2
	SI08	siLayer ;
	//##ModelId=3C5D92B40325
	SI16	siHeight ;
	//##ModelId=3C5D92B40344
	SI32	siGump;
} ;


//##ModelId=3C5D92B4037F
struct landtile_st
{
	//##ModelId=3C5D92B403B2
	string sName ;
	//##ModelId=3C5D92B403C6
	UI32 uiFlag ;
};





// A macro to check the flag is true

#define isTileSet(attribute,flag) ( {attribute & flag) )



//Class definitions
//##ModelId=3C5D92B5015A
class TileCache_cl
{
public:
        /// Constructor
	//##ModelId=3C5D92B50178
        TileCache_cl()   ;
        /// Cosntructor of itself
	//##ModelId=3C5D92B50182
        TileCache_cl(const TileCache_cl& clData) ;

	//##ModelId=3C5D92B5018C
		TileCache_cl(string sDirectory) ;
        /// Desctructor
	//##ModelId=3C5D92B50197
        ~TileCache_cl() ;

        /// Clear out any that we have

	//##ModelId=3C5D92B501A0
		bool clear( );


		// Set our directory
	//##ModelId=3C5D92B501B4
		void setDirectory(string sDirectory) ;

		// Cache the tiles
	//##ModelId=3C5D92B501BF
		bool cacheData() ;


		// get a land tile data

	//##ModelId=3C5D92B501DC
		landtile_st getLandTile(UI16 uiId) ;

	//##ModelId=3C5D92B501F0
		statictile_st getStaticTile(UI16 uiID) ;


	//##ModelId=3C5D92B50204
		TileCache_cl&  operator=(const TileCache_cl& clData) ;

private:
	//##ModelId=3C5D92B5020F
		bool processVerdata() ;
	//##ModelId=3C5D92B50222
		bool processLand() ;
	//##ModelId=3C5D92B50236
		bool processStatic() ;

private:

	//##ModelId=3C5D92B50350
		vector<landtile_st> vecLand ;

	//##ModelId=3C5D92B50396
		vector<statictile_st> vecStatic ;


	//##ModelId=3C5D92B503BE
		string sFile ;
	//##ModelId=3C5D92B503D2
		string sVerdata ;



		// We read in the verdatamul for each file type, a great room for speed improvment on startup later

	//##ModelId=3C5D92B60031
		map<SI32,verdata_st>  mapVerdata ;
	//##ModelId=3C5D92B6008A
		map<SI32,verdata_st>::iterator iterVerdata ;






};
//==========================================================================================

#endif



