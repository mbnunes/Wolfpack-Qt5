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

#if !defined(__MAPSTUFF_H__)
#define __MAPSTUFF_H__

#include "structs.h"
#include "hCache.h"
#include "qintcache.h"


#if ILSHENAR == 1
  const INT32 MapTileWidth  = 288;
  const INT32 MapTileHeight = 200;
#else
  const INT32 MapTileWidth  = 768;
  const INT32 MapTileHeight = 512;
#endif

class cMapStuff
{
//Variables
private:
	friend class MapStaticIterator;
	friend class cMovement;

    // moved from global vars INT32o here - fur 11/3/1999
    UOXFile *mapfile, *sidxfile, *statfile, *verfile, *multifile, *midxfile;

	// static caching items
	unsigned long StaticBlocks;
	struct StaCache_st
	{
		staticrecord* Cache;
		INT16 CacheLen;   // i've seen this goto to at least 273 - fur 10/29/1999
	};

	QIntCache<map_st>* MapCache;

	// version caching items
	versionrecord *versionCache;
	UI32 versionRecordCount;

	// caching functions
	void CacheStatics( void );

public:
	char mapname[80], sidxname[80], statname[80], vername[80], multiname[80], midxname[80];
	unsigned long StaMem, versionMemory;
	UINT32 Map0CacheHit, Map0CacheMiss;
	StaCache_st StaticCache[MapTileWidth][MapTileHeight];
	bool Cache;
	
// Functions
private:
	INT8 MultiHeight( P_ITEM pi, const Coord_cl& );
	UINT16 MultiTile( P_ITEM pi, const Coord_cl& );
	SI32 VerSeek(SI32 file, SI32 block);
	bool IsTileWet( UINT16 tilenum);
	void CacheVersion();
	UINT16 DynTile( const Coord_cl &pos );
	bool DoesStaticBlock( const Coord_cl& pos );

public:
	explicit cMapStuff( const QString& );
	~cMapStuff();

	void Load();

	// height functions
	bool IsUnderRoof( const Coord_cl& );
	INT8 StaticTop( const Coord_cl& );
	INT8 DynamicElevation( const Coord_cl& );
	INT8 MapElevation( const Coord_cl& );
	INT8 AverageMapElevation( const Coord_cl&, INT32 &id );
	INT8 TileHeight( UINT16 tileId );
	INT8 Height( const Coord_cl& );

	// look at tile functions
	void MultiArea( P_ITEM pi, INT32 *x1, INT32 *y1, INT32 *x2, INT32 *y2 );
	void SeekMulti( INT32 multinum, UOXFile **mfile, SI32 *length );
	map_st SeekMap( const Coord_cl& );
	bool IsRoofOrFloorTile( tile_st *tile );
	bool IsRoofOrFloorTile( unitile_st *tile );
	bool DoesTileBlock( UINT16 tilenum);

	// misc functions
	bool CanMonsterMoveHere( const Coord_cl& );

	// Map size related
	static UINT32 mapTileWidth( const Coord_cl& );
	static UINT32 mapTileHeight( const Coord_cl& );
	static UINT32 mapTileWidth( UINT32 );
	static UINT32 mapTileHeight( UINT32 );

	// static members
	static bool DoesTileBlock( tile_st &tile );
};

class MapStaticIterator
{
private:
	staticrecord staticArray;
	SI32 baseX, baseY, pos;
	Coord_cl position;
	UINT8 remainX, remainY;
	UI32 index, length, tileid;
	bool exactCoords;

public:
	MapStaticIterator(const Coord_cl& pos, bool exact = true);
	~MapStaticIterator() { };

	staticrecord* First();
	staticrecord* Next();
	void GetTile(tile_st *tile) const;
	UI32 GetPos() const { return pos; }
	UI32 GetLength() const { return length; }
};


#endif // __MAPSTUFF_H__

