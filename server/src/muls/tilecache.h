/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#if !defined(__TILECACHE_H__)
#define __TILECACHE_H__

// Platform specifics
#include "../platform.h"
#include "../server.h"

// System Includes
#include <qstring.h>
#include <map>

// Third Party includes

// Wolfpack Includes
#include "../singleton.h"

#define dBACKGROUND 0x1
#define dWEAPON 0x2
#define dTRANSPARENT 0x4
#define dTRANSLUCENT 0x8
#define dWALL 0x10
#define dDAMAGING 0x20
#define dIMPASSABLE 0x40
#define dWET 0x80
//0x100
#define dSURFACE 0x200
#define dBRIDGE 0x400
#define dGENERIC 0x800
#define dWINDOW 0x1000
#define dNOSHOOT 0x2000
#define dA 0x4000
#define dAN 0x8000
#define dMONGEN 0x10000
#define dFOLIAGE 0x20000
#define dPARTIALHUE 0x40000
//0x80000
#define dMAP 0x100000
#define dCONTAINER 0x200000
#define dWEARABLE 0x400000
#define dLIGHTSOURCE 0x800000
#define dAMINATION 0x1000000
#define dNODIAGONAL 0x2000000
//0x4000000
#define dARMOR 0x8000000
#define dROOF 0x10000000
#define dDOOR 0x20000000
#define dSTAIRBACK 0x40000000
#define dSTAIRRIGHT 0x80000000

struct tile_st
{
	char flag1;
	char flag2;
	char flag3;
	char flag4;
	unsigned char weight;
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
	bool isTransparent() const;
	bool isNoShoot() const;
};

// Inline Methods
inline bool tile_st::isNoShoot() const
{
	return flag2 & 0x20;
}

inline bool tile_st::isWet() const
{
	return flag1 & 0x80;
}

inline bool tile_st::isBlocking() const
{
	return flag1 & 0x40;
}

inline bool tile_st::isRoofOrFloorTile() const
{
	return ( ( flag1 & 0x1 ) && ( flag2 % 0x2 ) );
}

inline bool tile_st::isTransparent() const
{
	return flag1 & 0x4;
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

	bool isBlocking() const
	{
		return flag1 & 0x40;
	}
	bool isWet() const
	{
		return flag1 & 0x80;
	}
	bool isRoofOrFloorTile() const
	{
		return flag1 & 0x01;
	}
};

class cTileCache : public cComponent
{
private:
	QString path;
	std::map<Q_UINT16, tile_st> staticTiles;
	tile_st emptyStaticTile;

	std::map<Q_UINT16, land_st> landTiles;
	land_st emptyLandTile;
public:

	land_st getLand( Q_UINT16 tileId );
	tile_st getTile( Q_UINT16 tileId );
	signed char tileHeight( ushort tileId );
	static signed char tileHeight( const tile_st& );

	void load();
	void unload();
	void reload();
};

typedef Singleton<cTileCache> TileCache;

#endif
