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
#include "qstring.h"
#include "qfile.h"
#include "qdatastream.h"
#include <vector>
#include <map>

using namespace std ;

// Third Party includes

// Wolfpack Includes
#include "wpconsole.h"
#include "structs.h"
#include "tileflags.h"

extern WPConsole_cl clConsole ;

struct tile_st
{
	char flag1;
	char flag2;
	char flag3;
	char flag4;
	UINT8 weight;
	char layer;
	short unknown1;
	char unknown2;
	char quantity;
	short animation;
	char unknown3;
	char hue;
	char unknown4;
	char unknown5;
	char height;
	char name[20];

	bool isWet() const;
	bool isBlocking() const;
	bool isRoofOrFloorTile() const;
};

// Inline Methods
inline bool tile_st::isWet() const
{
	return flag1 & 0x80;
}

inline bool tile_st::isBlocking() const
{
	return flag1 & 0x40;
}

struct land_st
{
	char flag1;
	char flag2;
	char flag3;
	char flag4;
	char unknown1;
	char unknown2;
	char name[20];
};

class cTileCache
{
private:
	QString path;
	map< UINT16, tile_st > staticTiles;
	tile_st emptyStaticTile;

	map< UINT16, land_st > landTiles;
	land_st emptyLandTile;
public:
	cTileCache() {}
	virtual ~cTileCache() {};

	land_st getLand( UINT16 tileId );
	tile_st getTile( UINT16 tileId );
	signed char tileHeight( ushort tileId );
	static signed char tileHeight( const tile_st & );

    bool load( const QString &nPath );
	bool unload();
	bool reload() { unload(); load( path ); }

	static cTileCache *instance()
	{
		static cTileCache instance;
		return &instance;
	}
};

#endif
