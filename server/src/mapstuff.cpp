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

#include "mapstuff.h"

#include "wolfpack.h"
#include "progress.h"
#include "tilecache.h"
#include "debug.h"
#include "regions.h"
#include "classes.h"

#include <assert.h>

#undef  DBGFILE
#define DBGFILE "mapstuff.cpp"

//#define DEBUG_MAP_STUFF	1
#define USE_REGION_ITERATOR 1

#ifdef DEBUG_MAP_STUFF
void bitprINT32(FILE *fp, UINT8 x)
{
	for (INT32 i = 7; i >= 0; --i)
	{
		if ((x & 0x80) == 0)
			fprINT32f(fp, "0");
		else
			fprINT32f(fp, "1");
		if (4 == i)
			fprINT32f(fp, " ");
		x = x << 1;
	}
}
#endif

cMapStuff::cMapStuff( const QString& mulPath ) : versionCache(NULL), versionRecordCount(0), versionMemory(0), StaMem(0),
Cache(0), StaticBlocks(0), mapfile(NULL), sidxfile(NULL), statfile(NULL), verfile(NULL), multifile(NULL), midxfile(NULL)
{
	// after a mess of bugs with the structures not matching the physical record sizes
	// i've gotten paranoid...
	assert(sizeof(versionrecord) >= VersionRecordSize);
	assert(sizeof(st_multi) >= MultiRecordSize);
	assert(sizeof(land_st) >= LandRecordSize);
	assert(sizeof(map_st) >= MapRecordSize);
	assert(sizeof(st_multiidx) >= MultiIndexRecordSize);
	// staticrecord is not listed here because we explicitly don't read in some
	// unknown bytes to save memory
	
	MapCache = new QIntCache<map_st>( 100, 521); // should be a prime number
	MapCache->setAutoDelete( true );
	// http://www.utm.edu/research/primes/lists/small/1000.txt contains a prime table.

//	memset(tilecache, 0x00, sizeof(tilecache));

	memset(StaticCache, 0x00, sizeof(StaticCache));
	QString basePath = mulPath;
	if ( basePath.left(1) != "/" )
		basePath += "/";
	strcpy(this->mapname, basePath + "map0.mul");
	strcpy(this->sidxname, basePath + "staidx0.mul");
	strcpy(this->statname, basePath + "statics0.mul");
	strcpy(this->vername, basePath + "verdata.mul");
	strcpy(this->multiname, basePath + "multi.mul");
	strcpy(this->midxname, basePath + "multi.idx");
}

cMapStuff::~cMapStuff()
{
	if (versionCache) delete [] versionCache;
	
	delete mapfile;
	delete sidxfile;
	delete statfile;
	delete verfile;
	delete tilefile;
	delete multifile;
	delete midxfile;
	delete MapCache;
}

#define loadError( file ) clConsole.ProgressFail(); clConsole.send( "Could not open: %s", file ); LogCritical( "Could not open: " ); LogCritical( file ); keeprun = 0; return;

void cMapStuff::Load()
{
	clConsole.PrepareProgress( "Loading MUL Files" );

	// MAP0.MUL
	mapfile= new UOXFile( mapname, "rb" );
	if (mapfile==NULL || !mapfile->ready())
    {
		loadError( mapname );
    }

	// STAIDX0.MUL + STATICS0.MUL
	sidxfile= new UOXFile( sidxname, "rb" );
	if (sidxfile==NULL || !sidxfile->ready())
    {	
		loadError( sidxname );
    }
	
	statfile= new UOXFile( statname, "rb" );
	if (statfile==NULL || !statfile->ready())
    {
		loadError( statname );
	}

	// VERDATA.MUL
	verfile= new UOXFile( vername,"rb" );
	if (verfile == NULL || !verfile->ready() )
    {
		delete verfile; // Normal behavior if UO:LBR
		verfile = 0; 
    }

	// MULTI.IDX + MULTI.MUL
	midxfile = new UOXFile( midxname, "rb" );
	if( midxfile == NULL || !midxfile->ready() )
    {
		loadError( midxname );
    }

	multifile= new UOXFile( multiname, "rb" );
	if (multifile==NULL || !multifile->ready())
    {
		loadError( multiname );
	}

	clConsole.ProgressDone();

	CacheVersion();

	if( Cache )
		CacheStatics();    
}

// this stuff is rather buggy thats why I separted it from uox3.cpp
// feel free to correct it, but be carefull
// bugfixing this stuff often has a domino-effect with walking etc.
// LB 24/7/99

// oh yah, well that's encouraging.. NOT! at least LB was kind enough to
// move this out INT32o a separate file. he gets kudos for that!
//INT32 cMapStuff::TileHeight(INT32 tilenum)
INT8 cMapStuff::TileHeight( UINT16 tileId )
{
	tile_st tile = cTileCache::instance()->getTile( tileId );
	
	// For Stairs + Ladders
	if( tile.flag2 & 0x04 ) 
		return (INT8)( tile.height / 2 );
	else
		return tile.height;
}

//o-------------------------------------------------------------o
//|   Function    :  char StaticTop(INT32 x,INT32 y,INT32 oldz);
//|   Date        :  Unknown     Touched: Dec 21, 1998
//|   Programmer  :  Unknown
//o-------------------------------------------------------------o
//|   Purpose     :  Top of statics at/above given coordinates
//o-------------------------------------------------------------o
INT8 cMapStuff::StaticTop( const Coord_cl& pos )
{
	// TODO: Use Average Map elevation here
	INT8 mTop = -128;

	MapStaticIterator msi( pos );
	for( staticrecord *sItem = msi.First(); sItem; sItem = msi.Next() )
	{
		INT8 iTop = sItem->zoff + TileHeight( sItem->itemid );

		if( ( iTop <= pos.z + MaxZstep ) && ( iTop > mTop ) )
			mTop = iTop;
	}

	return mTop;
}

// author  : Lord Binary 17/8/99
// purpose : check if something is under a (static) roof.
//           I wrote this to check if a player is inside a static building (with roof)
//           to disable rain/snow when they enter buildings 
//           this only works for static buildings.
//           Its perfect for weather stecks, if you want make a in_building() 
//           you need to call it with x,y x+1,y x,y+1,x-1,y,x,y-1 
//           if they all return 1 AND the player isnt running under a underpass or bridge 
//           he/she is really in a building. though idunno yet how to check the later one (underpassing checks)
//           probably with the floor-bit

//bool cMapStuff::IsUnderRoof(INT32 x, INT32 y, INT32 z)
bool cMapStuff::IsUnderRoof(const Coord_cl& pos)
{
	MapStaticIterator msi(pos);
	staticrecord *stat;
	unsigned long loopexit=0;
	while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
	{
		tile_st tile;
		msi.GetTile(&tile);
		
		// this seems suspicious, if we are under a floor we return right away
		// i guess you can assume if they are under a floor they are outside
		// but can you promise that?? its too early for me to tell
		if ((tile.flag1&1) && (tile.height+stat->zoff)>pos.z )
			return false; // check the floor bit
		// if set -> this must be a underpassing/bridge
		
		if ((tile.height+stat->zoff)>pos.z) // a roof  must be higher than player's z !
		{
			if (strstr("roof", (char *) tile.name) || strstr("shingle", (char *) tile.name)) 
				return true;
				/*if ( !strcmp(tile.name,"slate roof") || !strcmp(tile.name,"wooden shingles") ||
				!strcmp(tile.name,"thatch roof") || !strcmp(tile.name,"stone pavers") ||
				!strcmp(tile.name,"stone roof") || !strcmp(tile.name,"palm fronds") ||
				!strcmp(tile.name,"palm frond roof") || !strcmp(tile.name,"sandstone floor") ||
				!strcmp(tile.name,"marble roof") || !strcmp(tile.name,"tile roof") ||
				!strcmp(tile.name,"tent roof") || !strcmp(tile.name,"log roof") || 
				!strcmp( tile.name, "stone pavern") || !strcmp( tile.name, "wooden shingle") ||
			!strcmp( tile.name, "wooden board") || !strcmp( tile.name, "wooden boards")) return 1;*/
			
			// now why would not want to check the z value of wooden boards first??
			// this was after the if (.. >z), i'm moving this up inside of it
			if (!strcmp((char *) tile.name,"wooden boards"))
				return true;
			// ok, well going by the commented out section above we've likely got some bugs
			// because we've left out checking for "stone pavern" and "wooden board"
			// i'll stick these back in. even if these were bogus tile names it can't hurt
			if (!strcmp((char *) tile.name,"wooden board") ||
				!strcmp( (char *) tile.name, "stone pavern") ||
				!strcmp( (char *) tile.name, "stone pavers"))
				return true;
		}
	}
	return false;
}

//
// i guess this function returns where the tile is a 'blocker' meaning you can't pass through it
bool cMapStuff::DoesTileBlock( UINT16 tileId )
{
	tile_st tile = cTileCache::instance()->getTile( tileId );
	return DoesTileBlock( tile );
}

bool cMapStuff::DoesTileBlock( tile_st &tile )
{
	return ( tile.flag1 & 0x40 );
}

// finds the "corners" of a multitile object. I use
// this for when houses are converted INT32o deeds.
void cMapStuff::MultiArea(P_ITEM pi, INT32 *x1, INT32 *y1, INT32 *x2, INT32 *y2)
{
	st_multi multi;
	UOXFile *mfile = NULL;
	SI32 length = 0;
	
	*x1 = *y1 = *x2 = *y2 = 0;
	SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/MultiRecordSize;
	if (length == -1 || length>=17000000)
		length = 0;//Too big...  bug fix hopefully (Abaddon 13 Sept 1999)
	
	for (INT32 j=0;j<length;j++)
	{
        mfile->get_st_multi(&multi);
		if(multi.x<*x1) *x1=multi.x;
		if(multi.x>*x2) *x2=multi.x;
		if(multi.y<*y1) *y1=multi.y;
		if(multi.y>*y2) *y2=multi.y;
	}
	*x1+=pi->pos.x;
	*x2+=pi->pos.x;
	*y1+=pi->pos.y;
	*y2+=pi->pos.y;
}

// return the height of a multi item at the given x,y. this seems to actually return a height
//INT32 cMapStuff::MultiHeight(INT32 i, INT32 x, INT32 y, INT32 oldz)
INT8 cMapStuff::MultiHeight(P_ITEM pi, const Coord_cl& pos)
{                                                                                                                                  	st_multi multi;                                                                                                               
	UOXFile *mfile = NULL;
	SI32 length = 0;
	SeekMulti(pi->id()-0x4000, &mfile, &length);                                                           
	length = length / MultiRecordSize;                                                                                               
	if (length == -1 || length>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
	{                                                                                                                             
		length = 0;                                                                                                           
	}
	//Check for height at and above
	
	for (INT32 j=0;j<length;j++)
	{
		mfile->get_st_multi(&multi);
		if (multi.visible && (pi->pos.x+multi.x == pos.x) && (pi->pos.y+multi.y == pos.y))
		{
			INT32 tmpTop = pi->pos.z + multi.z;
			if ((tmpTop<=pos.z+MaxZstep)&& (tmpTop>=pos.z-1))
			{
				//clConsole.send("At or above=%i\n",multi.z);
				return multi.z;
			}
			else if ((tmpTop>=pos.z-MaxZstep)&& (tmpTop<pos.z-1))
			{
				//clConsole.send("Below=%i\n",multi.z);
				return multi.z;
			}
		}                                                                                                                 
	}
	return 0;                                                                                                                     
} 

// This was fixed to actually return the *elevation* of dynamic items at/above given coordinates
//INT32 cMapStuff::DynamicElevation(INT32 x, INT32 y, INT32 oldz)
INT8 cMapStuff::DynamicElevation(const Coord_cl& pos)
{
	//INT32 z = illegal_z;
	INT8 z = illegal_z;
	const INT32 getcell = mapRegions->GetCell(pos);
	cRegion::raw vecEntries = mapRegions->GetCellEntries(getcell);
	cRegion::rawIterator it = vecEntries.begin();
	for (; it != vecEntries.end(); ++it )
	{
		P_ITEM mapitem = FindItemBySerial(*it);
		if (mapitem != NULL)
		{
			if(mapitem->isMulti())
			{
				z = MultiHeight(mapitem, pos);
				// this used to do a z++, but that doesn't take INT32o account the fact that
				// the itemp[] the multi was based on has its own elevation
				z += mapitem->pos.z + 1;
			}
			if ( ( mapitem->pos.x == pos.x ) && ( mapitem->pos.y == pos.y ) && ( !mapitem->isMulti() ) )
			{
				INT8 ztemp = mapitem->pos.z+TileHeight(mapitem->id());
				if ((ztemp <= pos.z + MaxZstep) && (ztemp > z))
				{
					z = ztemp;
				}
				
			}
		}
	}
	return z;
}

UINT16 cMapStuff::MultiTile( P_ITEM pi, const Coord_cl &pos )
{
	SI32 length = 0;
	st_multi multi;
	UOXFile *mfile = NULL;
	SeekMulti( pi->id() - 0x4000, &mfile, &length );
	length = length/MultiRecordSize;
	if( length == -1 || length >= 17000000 )
	{
		clConsole.send("cMapStuff::MultiTile->Bad length in multi file. Avoiding stall.\n");
		length = 0;
	}
	
	for( UINT32 j = 0; j < length; ++j )
	{
		mfile->get_st_multi( &multi );
		if( ( multi.visible && ( pi->pos.x + multi.x == pos.x ) && ( pi->pos.y + multi.y == pos.y )
			&& ( abs( pi->pos.z + multi.z - pos.z ) <= 1 ) ) )
		{
			INT32 mt = multi.tile;
			return mt;
		}
		
	}
	return 0;
}

// returns which dynamic tile is present at (x,y) or -1 if no tile exists
// originally by LB & just michael
UINT16 cMapStuff::DynTile( const Coord_cl &pos )
{
	const INT32 getcell = mapRegions->GetCell( pos );
	cRegion::raw vecEntries = mapRegions->GetCellEntries( getcell );
	cRegion::rawIterator it = vecEntries.begin();
	for(; it != vecEntries.end(); ++it )
    {
		P_ITEM mapitem = FindItemBySerial(*it);
		if( mapitem )
		{
			if( mapitem->isMulti() )
				return MultiTile( mapitem, pos );
			else if ( mapitem->pos == pos )
				return mapitem->id();
        }    
		
    }
	return (UINT16)-1;
}

// return the elevation of MAP0.MUL at given coordinates, we'll assume since its land
// the height is inherently 0
//INT32 cMapStuff::MapElevation(INT32 x, INT32 y)
INT8 cMapStuff::MapElevation( const Coord_cl& pos )
{
	map_st map = SeekMap( pos );
	// make sure nothing can move INT32o black areas
	if (430 == map.id || 475 == map.id || 580 == map.id || 610 == map.id ||
		611 == map.id || 612 == map.id || 613 == map.id)
		return illegal_z;
	// more partial black areas
	//if ((map.id >= 586 && map.id <= 601) || (map.id >= 610 && map.id <= 613))
	//	return illegal_z;
	return map.z;
}

// compute the 'average' map height by looking at three adjacent cells
INT8 cMapStuff::AverageMapElevation(const Coord_cl& pos, INT32 &id)
{
	// first thing is to get the map where we are standing
	map_st map1 = SeekMap( pos );
	id = map1.id;
	// if this appears to be a valid land id, <= 2 is invalid
	if (map1.id > 2 && illegal_z != MapElevation(pos))
	{
		// get three other nearby maps to decide on an average z?
		INT8 map2z = MapElevation( pos + Coord_cl( 1, 0, 0 ) );
		INT8 map3z = MapElevation( pos + Coord_cl( 0, 1, 0 ) );
		INT8 map4z = MapElevation( pos + Coord_cl( 1, 1, 0 ) );
		
		INT8 testz = 0;
		if (abs(map1.z - map4z) <= abs(map2z - map3z))
		{
			if (illegal_z == map4z)
				testz = map1.z;
			else
			{
				testz = (INT8)((map1.z + map4z) >> 1);
				if (testz%2<0) --testz;
				// ^^^ Fix to make it round DOWN, not just in the direction of zero
			}
		} else {
			if (illegal_z == map2z || illegal_z == map3z)
				testz = map1.z;
			else
			{
				testz = (INT8)((map2z + map3z) >> 1);
				if (testz%2<0) --testz;
				// ^^^ Fix to make it round DOWN, not just in the direction of zero
			}
		}
		return testz;
	}

	return illegal_z;
}

/* return whether a tile is ????
** this really doesn't look like it does anything, because id1 will never be 68
char cMapStuff::MapType(INT32 x, INT32 y) // type of MAP0.MUL at given coordinates
{
	map_st map = SeekMap0( x, y );
	const char id1 = map.id>>8;
	const char id2 = map.id % 256;
	if ( id1 != 68 )
		return 1;
	else if( id2 != 2 )
		return 1;
	return 0;
}*/


// since the version data will potentiall affect every map related operation
// we are always going to cache it.   we are going to allocate maxRecordCount as
// given by the file, but actually we aren't going to use all of them, since we
// only care about the patches made to the 6 files the server needs.  so the
// versionRecordCount hold how many we actually saved
void cMapStuff::CacheVersion()
{
	clConsole.PrepareProgress( "Caching Verdata Patches" );

	if( verfile == NULL )
	{
		clConsole.ProgressSkip();
		return;
	}

	verfile->seek(0, SEEK_SET);
	UI32 maxRecordCount = 0;
	verfile->getULong(&maxRecordCount);

	if (0 == maxRecordCount)
		return;
	if (NULL == (versionCache = new versionrecord[maxRecordCount]))
		return;

	versionMemory = maxRecordCount * sizeof(versionrecord);

	bool unhandeledPatches = false;

	for (UI32 i = 0; i < maxRecordCount; ++i)
    {
		if (verfile->eof())
		{
			clConsole.ProgressFail();
			clConsole.send( "Corrupt Verdata, please check Verdata.mul" );
			return;
		}
		versionrecord *ver = versionCache + versionRecordCount;
		assert(ver);
		verfile->get_versionrecord(ver);
	
		// see if its a record we care about
		switch(ver->file)
		{
		case VERFILE_MULTIIDX:
		case VERFILE_MULTI:
		case VERFILE_TILEDATA:
			++versionRecordCount;
			break;
		case VERFILE_MAP:
		case VERFILE_STAIDX:
		case VERFILE_STATICS:
			// at some poINT32 we may need to handle these cases, but OSI hasn't patched them as of
			// yet, so no need slowing things down processing them
			unhandeledPatches = true;
			break;
		default:
			// otherwise its for a file we don't care about
			break;
		}
   	}

	clConsole.ProgressDone();

	if( unhandeledPatches )
	{
		clConsole.send( "Unsupported Map and Static patches have been skipped\n" );
	}

	clConsole.send( "\n" );
}

SI32 cMapStuff::VerSeek(SI32 file, SI32 block)
{
	for (UI32 i = 0; i < versionRecordCount; ++i)
	{
		if (versionCache[i].file == file && versionCache[i].block == block)
		{
			verfile->seek(versionCache[i].filepos, SEEK_SET);
			return versionCache[i].length;
		}
	}
	return 0;
}

void cMapStuff::SeekMulti(INT32 multinum, UOXFile **mfile, SI32 *length)
{
	const INT32 len=VerSeek(VERFILE_MULTI, multinum);
	if (len==0)
	{
		st_multiidx multiidx;
        midxfile->seek(multinum*MultiIndexRecordSize, SEEK_SET);
		midxfile->get_st_multiidx(&multiidx);
		multifile->seek(multiidx.start, SEEK_SET);
		*mfile=multifile;
		*length=multiidx.length;
	}
	else
	{
		*mfile=verfile;
		*length=len;
	}
}

/*
** Use this iterator class anywhere you would have used SeekInit() and SeekStatic()
** Unlike those functions however, it will only return the location of tiles that match your
** (x,y) EXACTLY.  They also should be significantly faster since the iterator saves
** a lot of info that was recomputed over and over and it returns a poINT32er instead 
** of an entire structure on the stack.  You can call First() if you need to reset it.
** This iterator hides all of the map implementation from other parts of the program.
** If you supply the exact variable, it tells it whether to iterate over those items
** with your exact (x,y) otherwise it will loop over all items in the same cell as you.
** (Thats normally only used for filling the cache)
** 
** Usage:
**		MapStaticIterator msi(x, y);
**
**      staticrecord *stat;
**      tile_st tile;
**      while (stat = msi.Next())
**      {
**          msi.GetTile(&tile);
**  		    ... your code here...
**	  	}
*/
MapStaticIterator::MapStaticIterator(const Coord_cl& position, bool exact) :
baseX(position.x / 8), baseY(position.y / 8), remainX(position.x % 8), remainY(position.y % 8), length(0), index(0),
pos(0), exactCoords(exact), tileid(0)
{
	assert(baseX < cMapStuff::mapTileWidth(position));
	assert(baseY < cMapStuff::mapTileHeight(position));

	if (baseX >= cMapStuff::mapTileWidth(position))  return;
	if (baseY >= cMapStuff::mapTileHeight(position)) return;

	if ( Map->Cache )
	{
		length = Map->StaticCache[baseX][baseY].CacheLen;
	}
	else
	{
		const SI32 indexPos = (( baseX * MapTileHeight * 12L ) + ( baseY * 12L ));
		Map->sidxfile->seek(indexPos, SEEK_SET);
		if (!Map->sidxfile->eof() )
		{
			Map->sidxfile->getLong(&pos);
			if ( pos != -1 )
			{
				Map->sidxfile->getULong(&length);
				length /= StaticRecordSize;
				//clConsole.send("MSI indexpos: %ld, pos at %lx, length: %lu\n", indexPos, pos, length); 
			}
		}
	}
}

staticrecord *MapStaticIterator::First()
{
	index = 0;
	return Next();
}

staticrecord *MapStaticIterator::Next()
{
	tileid = 0;
	unsigned long loopexit=0;
	if (index >= length)
		return NULL;
	
	if ( Map->Cache )
	{
#ifdef MAP_CACHE_DEBUG
		// turn this on for debugging z-level stuff where you want to see where
		// the characters are in relation to their cell, and which places in the 8x8
		// cell have static tiles defined.
		if (index == 0)
		{
			clConsole.send("baseX: %lu, baseY: %lu, remX: %d, remY: %d\n", baseX, baseY, (INT32) remainX, (INT32) remainY);
			clConsole.send(" 01234567\n");
			char testmap[9][9];
			memset(testmap, ' ', 9*9);
			for (INT32 tmp = 0; tmp < length; ++tmp)
			{
				staticrecord *ptr = Map->StaticCache[baseX][baseY].Cache + tmp;
				testmap[ptr->yoff][ptr->xoff] = 'X';
			}
			testmap[remainY][remainX] = 'O';
			for (INT32 foo = 0; foo < 8; ++foo)
			{
				testmap[foo][8] = '\0';
				clConsole.send("%d%s\n", foo, testmap[foo]);
			}
		}
#endif
		do {
			staticrecord *ptr = (Map->StaticCache[baseX][baseY].Cache) + index++;
			if (!exactCoords || (ptr->xoff == remainX && ptr->yoff == remainY))
			{
				tileid = ptr->itemid;
				return ptr;
			}
		} while ((index < length) && (++loopexit < MAXLOOPS) );
		return NULL;
	}
	
	// this was sizeof(OldStaRec) which SHOULD be 7, but on some systems which don't know how to pack, 
	const SI32 pos2 = pos + (StaticRecordSize * index);	// skip over all the ones we've read so far
	Map->statfile->seek(pos2, SEEK_SET);
	loopexit=0;
	do {
		if ( Map->statfile->eof( ) )
			return NULL;
		
		Map->statfile->get_staticrecord(&staticArray);
		++index;
		assert(staticArray.itemid >= 0);
		// if these are ever larger than 7 we've gotten out of sync
		assert(staticArray.xoff < 0x08);
		assert(staticArray.yoff < 0x08);
		if (!exactCoords || (staticArray.xoff == remainX && staticArray.yoff == remainY))
		{
#ifdef DEBUG_MAP_STUFF
			clConsole.send("Found static at index: %lu, Length: %lu, indepos: %ld\n", index, length, pos2);
			clConsole.send("item is %d, x: %d, y: %d\n", staticArray.itemid, (INT32) staticArray.xoff, (INT32) staticArray.yoff);
#endif
			tileid = staticArray.itemid;
			return &staticArray;
		}
	} while ((index < length) && (++loopexit < MAXLOOPS));
	
	return NULL;
}

// since 99% of the time we want the tile at the requested location, here's a
// helper function.  pass in the poINT32er to a struct you want filled.
void MapStaticIterator::GetTile(tile_st *tile) const
{
	assert(tile);
	(*tile) = cTileCache::instance()->getTile( tileid );
}

/*
** some clean up to the caching and it wasn't reporting all the memory actually
** used by the StaticCache[][] in cMapStuff
*/
void cMapStuff::CacheStatics( void )
{
	StaticBlocks = ( MapTileWidth * MapTileHeight );
	const UI32 tableMemory = StaticBlocks * sizeof(staticrecord);
	const UI32 indexMemory = StaticBlocks * sizeof(StaCache_st);
	StaMem = tableMemory + indexMemory;
	/*clConsole.send("Blocks: %ld, Index: %ld, Table: %ld, Static File Size: %ld\n",
	(long) StaticBlocks, (long)sizeof(StaCache_st), (long) sizeof(staticrecord),
	(long) StaticBlocks * StaticRecordSize);*/
	
	clConsole.send( "Caching Statics0 (%ld bytes mul + %ld bytes idx = %ld bytes total)\n", 
		tableMemory, indexMemory, StaMem );
	
	// we must be in caching mode, only turn it off for now because we are
	// trying to fill the cache.
	assert(Cache);
	Cache = 0;
	
	progress_display progress(StaticBlocks);
	for( UINT32 x = 0; x < MapTileWidth; x++ )
    {
		for( UINT32 y = 0; y < MapTileHeight; y++ )
		{
			StaticCache[x][y].Cache = NULL;
			StaticCache[x][y].CacheLen = 0;
			
			MapStaticIterator msi(Coord_cl(x * 8, y * 8, 0), false);
			UI32 length = msi.GetLength();
			if (length)
			{
				StaticCache[x][y].CacheLen = length;
				StaticCache[x][y].Cache = new staticrecord[length];
				// read them all in at once!
				statfile->seek(msi.GetPos(), SEEK_SET);
				statfile->get_staticrecord(StaticCache[x][y].Cache, length);
			}
			++progress;
		}
    } 
	
	// reenable the caching now that its filled
	Cache = 1;
	clConsole.send("Done.\n");
}

map_st cMapStuff::SeekMap( const Coord_cl& position )
{
	const UI16 x1 = position.x / 8, y1 = position.y / 8, x2 = position.x % 8, y2 = position.y % 8;
	const UI32 record = ( x1 * MapTileHeight * 196 ) + ( y1 * 196 ) + ( y2 * 24 ) + ( x2 * 3 ) + 4;

	UI32 cachePosition = record + ( position.map << 28 );
	map_st* result = MapCache->find( cachePosition );
	bool shouldDelete = false;
	if ( !result )
	{
		result = new map_st;
		mapfile->seek(record, SEEK_SET);
		mapfile->get_map_st(result);
		shouldDelete = !MapCache->insert( cachePosition, result );
	}

	if ( shouldDelete )
	{
		map_st dummy = *result;
		delete result;
		return dummy;
	}
	else
		return *result;
}

// these two functions don't look like they are actually used by anything
// anymore, at least we know which bit means wet
bool cMapStuff::IsTileWet( UINT16 tilenum )
{
	tile_st tile = cTileCache::instance()->getTile( tilenum );
	return ( tile.flag1 & 0x80 );
}

// Blocking statics at/above given coordinates?
bool cMapStuff::DoesStaticBlock( const Coord_cl& pos )
{
	MapStaticIterator msi( pos );
	unsigned long loopexit=0;
	
	staticrecord *stat;
	while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
	{
		const INT32 elev = stat->zoff + TileHeight(stat->itemid);
		if( (elev >= pos.z) && (stat->zoff <= pos.z ) )
		{
			bool btemp = DoesTileBlock(stat->itemid);
			if (btemp) return true;
		}
	}
	return false;
}

// Return new height of player who walked to X/Y but from OLDZ
INT8 cMapStuff::Height(const Coord_cl& pos)
{
	// let's check in this order.. dynamic, static, then the map
	INT8 dynz = DynamicElevation(pos);
	if (illegal_z != dynz)
		return dynz;

	INT8 staticz = StaticTop(pos);
	if (illegal_z != staticz)
		return staticz;

	return MapElevation(pos);
}

// can the monster move here from an adjacent cell at elevation 'oldz'
// use illegal_z if they are teleporting from an unknown z
//bool cMapStuff::CanMonsterMoveHere(INT32 x, INT32 y, INT32 oldz)
bool cMapStuff::CanMonsterMoveHere( const Coord_cl& pos )
{
	if( pos.x >= ( mapTileWidth(pos) * 8 ) || pos.y >= ( mapTileHeight(pos) * 8 ) )
		return false;
    const INT8 elev = Height( pos );
	Coord_cl target = pos;
	target.z = elev;
	if (illegal_z == elev)
		return false;

	// is it too great of a difference z-value wise?
	if (pos.z != illegal_z)
	{
		// you can climb MaxZstep, but fall up to 15
		if (elev - pos.z > MaxZstep)
			return false;
		else if (pos.z - elev > 15)
			return false;
	}

    // get the tile id of any dynamic tiles at this spot
	Coord_cl mPos = pos;
	mPos.z = elev;
    const INT32 dt = DynTile( mPos );
	
    // if there is a dynamic tile at this spot, check to see if its a blocker
    // if it does block, might as well INT16-circuit and return right away
    if (dt >= 0 && DoesTileBlock(dt))
		return false;
	
    // if there's a static block here in our way, return false
    if( DoesStaticBlock( target ) )
		return false;
	
    return true;
}

// checks to see if the tile is either a roof or floor tile
bool cMapStuff::IsRoofOrFloorTile( tile_st *tile )
{
	if (tile->flag1&1)
		return true; // check the floor bit
	
	if (strstr("roof", (char *) tile->name) || strstr("shingle", (char *) tile->name)) 
		return true;

	if( strstr( "floor", (char *)tile->name ) )
		return true;
	// now why would not want to check the z value of wooden boards first??
	// this was after the if (.. >z), i'm moving this up inside of it
	if (!strcmp((char *) tile->name,"wooden boards"))
		return true;
	// i'll stick these back in. even if these were bogus tile names it can't hurt
	if (!strcmp((char *) tile->name, "wooden board") ||
		!strcmp( (char *) tile->name, "stone pavern") ||
		!strcmp( (char *) tile->name, "stone pavers"))
		return true;

	return false;
}

// checks to see if the tile is either a roof or floor tile
bool cMapStuff::IsRoofOrFloorTile( unitile_st *tile )
{
	tile_st newTile = cTileCache::instance()->getTile( tile->id );
	return IsRoofOrFloorTile( &newTile );
}

UINT32 cMapStuff::mapTileWidth( const Coord_cl& pos )
{
	return mapTileWidth( pos.map );
}

UINT32 cMapStuff::mapTileHeight( const Coord_cl& pos )
{
	return mapTileHeight( pos.map );
}

UINT32 cMapStuff::mapTileWidth( UINT32 mapId )
{
	static UINT32 widthTable[] =  { 768, 768, 288, 288, 0 };
	if ( mapId > 4 )
		mapId = 4;
	return widthTable[mapId];
}

UINT32 cMapStuff::mapTileHeight( UINT32 mapId )
{
	static UINT32 heightTable[] = { 512, 512, 200, 200, 0 };
	if ( mapId > 4 )
		mapId = 4;
	return heightTable[mapId];
}
