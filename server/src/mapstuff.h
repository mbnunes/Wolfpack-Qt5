//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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

#include "structs.h"
#include "hCache.h"

#if !defined(__MAPSTUFF_H__)
#define __MAPSTUFF_H__

#if ILSHENAR == 1
  const int MapTileWidth  = 288;
  const int MapTileHeight = 200;
#else
  const int MapTileWidth  = 768;
  const int MapTileHeight = 512;
#endif

//##ModelId=3C5D92DD03E2
class cMapStuff
{
//Variables
private:
	friend class MapStaticIterator;
	friend class cMovement;

    // moved from global vars into here - fur 11/3/1999
    UOXFile *mapfile, *sidxfile, *statfile, *verfile, *tilefile, *multifile, *midxfile;

	// tile caching items
	//##ModelId=3C5D92DE002D
	tile_st tilecache[0x4000];

	// static caching items
	//##ModelId=3C5D92DE004A
	unsigned long StaticBlocks;
	//##ModelId=3C5D92E001FC
	struct StaCache_st
	{
		//##ModelId=3C5D92E00225
		staticrecord *Cache;
		//##ModelId=3C5D92E00238
		unsigned short CacheLen;   // i've seen this goto to at least 273 - fur 10/29/1999
	};

	// map caching items
	//##ModelId=3C5D92E00260
	struct MapCache
	{
		//##ModelId=3C5D92E00288
		unsigned short xb;
		//##ModelId=3C5D92E0029C
		unsigned short yb;
		//##ModelId=3C5D92E002B0
		unsigned char  xo;
		//##ModelId=3C5D92E002BA
		unsigned char  yo;
		//##ModelId=3C5D92E002D9
		map_st Cache;
	};
	//##ModelId=3C5D92DE0074
	MapCache Map0Cache[MAP0CACHE];

	// version caching items
	//##ModelId=3C5D92DE01A0
	versionrecord *versionCache;
	//##ModelId=3C5D92DE01C8
	UI32 versionRecordCount;

	// caching functions
	//##ModelId=3C5D92DE01F9
	void CacheTiles( void );
	//##ModelId=3C5D92DE020D
	void CacheStatics( void );

public:
	// these used to be [512], thats a little excessive for a filename.. - fur
	char mapname[80], sidxname[80], statname[80], vername[80],
	  tilename[80], multiname[80], midxname[80];
	unsigned long StaMem, TileMem, versionMemory;
	unsigned int Map0CacheHit, Map0CacheMiss;
	// ok this is rather silly, allocating all the memory for the cache, even if
	// they haven't chosen to cache?? - fur
	//##ModelId=3C5D92DE0327
	StaCache_st StaticCache[MapTileWidth][MapTileHeight];
	//##ModelId=3C5D92DE0344
	bool Cache;
	
// Functions
private:
	//##ModelId=3C5D92DE0362
	char VerLand(int landnum, land_st *land);
	//##ModelId=3C5D92DE039E
	signed char MultiHeight(P_ITEM pi, short int x, short int y, signed char oldz);
	//##ModelId=3C5D92DF0006
	int MultiTile(P_ITEM pi, short int x, short int y, signed char oldz);
	//##ModelId=3C5D92DF004C
	SI32 VerSeek(SI32 file, SI32 block);
	//##ModelId=3C5D92DF007E
	char VerTile(int tilenum, tile_st *tile);
	//##ModelId=3C5D92DF00B0
	bool IsTileWet(int tilenum);
	//##ModelId=3C5D92DF00CE
	bool TileWalk(int tilenum);
	//##ModelId=3C5D92DF00EC
	void CacheVersion();

	//##ModelId=3C5D92DF0100
	int DynTile( short int x, short int y, signed char oldz );
	//##ModelId=3C5D92DF013C
	bool DoesStaticBlock(short int x, short int y, signed char oldz);

public:
	//##ModelId=3C5D92DF0196
	cMapStuff();
	//##ModelId=3C5D92DF01A0
	~cMapStuff();

	//##ModelId=3C5D92DF01AA
	void Load();

	// height functions
	//##ModelId=3C5D92DF01B4
	bool IsUnderRoof(short int x, short int y, signed char z);
	//##ModelId=3C5D92DF01F1
	signed char StaticTop(short int x, short int y, signed char oldz);
	//##ModelId=3C5D92DF022D
	signed char DynamicElevation(short int x, short int y, signed char oldz);
	//##ModelId=3C5D92DF025F
	signed char MapElevation(short int x, short int y);
	//##ModelId=3C5D92DF0287
	signed char AverageMapElevation(short int x, short int y, int &id);
	//##ModelId=3C5D92DF02C3
	signed char TileHeight( int tilenum );
	//##ModelId=3C5D92DF02E1
	signed char Height(short int x, short int y, signed char oldz);

	// look at tile functions
	//##ModelId=3C5D92DF0313
	void MultiArea(P_ITEM pi, int *x1, int *y1, int *x2, int *y2);
	//##ModelId=3C5D92DF036D
	void SeekTile(int tilenum, tile_st *tile);
	//##ModelId=3C5D92DF0395
	void SeekMulti(int multinum, UOXFile **mfile, SI32 *length);
	//##ModelId=3C5D92DF03D1
	void SeekLand(int landnum, land_st *land);
	//##ModelId=3C5D92E0010C
	map_st SeekMap0( unsigned short x, unsigned short y );
	//##ModelId=3C5D92E00134
	bool IsRoofOrFloorTile( tile_st *tile );
	//##ModelId=3C5D92E00166
	bool IsRoofOrFloorTile( unitile_st *tile );
	//##ModelId=3C5D92E0017A
	bool DoesTileBlock(int tilenum);

	// misc functions
	//##ModelId=3C5D92E0018E
	bool CanMonsterMoveHere( short int x, short int y, signed char z );

	// static members
	//##ModelId=3C5D92E001CA
	static bool DoesTileBlock( tile_st &tile );
};

//##ModelId=3C5D92E100DB
class MapStaticIterator
{
private:
	//##ModelId=3C5D92E10104
	staticrecord staticArray;
	SI32 baseX, baseY, pos;
	unsigned char remainX, remainY;
	UI32 index, length, tileid;
	//##ModelId=3C5D92E10121
	bool exactCoords;

public:
	//##ModelId=3C5D92E1015D
	MapStaticIterator(unsigned int x, unsigned int y, bool exact = true);
	//##ModelId=3C5D92E1017B
	~MapStaticIterator() { };

	//##ModelId=3C5D92E10185
	staticrecord *First();
	//##ModelId=3C5D92E1018F
	staticrecord *Next();
	//##ModelId=3C5D92E101E9
	void GetTile(tile_st *tile) const;
	//##ModelId=3C5D92E101FD
	UI32 GetPos() const { return pos; }
	//##ModelId=3C5D92E10211
	UI32 GetLength() const { return length; }
};


#endif // __MAPSTUFF_H__

