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

struct statictile_st
{
	string sName ;
	UI32	uiFlag ;
	SI08	siLayer ;
	SI16	siHeight ;
	SI32	siGump;
} ;


struct landtile_st
{
	string sName ;
	UI32 uiFlag ;
};





// A macro to check the flag is true

#define isTileSet(attribute,flag) ( {attribute & flag) )



//Class definitions
class TileCache_cl
{
public:
        /// Constructor
        TileCache_cl()   ;
        /// Cosntructor of itself
        TileCache_cl(const TileCache_cl& clData) ;

		TileCache_cl(string sDirectory) ;
        /// Desctructor
        ~TileCache_cl() ;

        /// Clear out any that we have

		bool clear( );


		// Set our directory
		void setDirectory(string sDirectory) ;

		// Cache the tiles
		bool cacheData() ;


		// get a land tile data

		landtile_st getLandTile(UI16 uiId) ;

		statictile_st getStaticTile(UI16 uiID) ;


		TileCache_cl&  operator=(const TileCache_cl& clData) ;

private:
		bool processVerdata() ;
		bool processLand() ;
		bool processStatic() ;

private:

		vector<landtile_st> vecLand ;

		vector<statictile_st> vecStatic ;


		string sFile ;
		string sVerdata ;



		// We read in the verdatamul for each file type, a great room for speed improvment on startup later

		map<SI32,verdata_st>  mapVerdata ;
		map<SI32,verdata_st>::iterator iterVerdata ;






};
//==========================================================================================

#endif



