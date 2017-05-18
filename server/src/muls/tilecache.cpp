/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

//Wolfpack Includes

#include "tilecache.h"
#include "../console.h"
#include "../defines.h"
#include "../exceptions.h"
#include "../serverconfig.h"

#include <QFile>
#include <QDataStream>

using namespace std;

/*bool tile_st::isRoofOrFloorTile() const
{
	if (flag1 & 1)
		return true; // check the floor bit

	if (strstr("roof", (char *) name) || strstr("shingle", (char *) name))
		return true;

	if( strstr( "floor", (char *)name ) )
		return true;
	// now why would not want to check the z value of wooden boards first??
	// this was after the if (.. >z), i'm moving this up inside of it
	if (!strcmp((char *) name,"wooden boards"))
		return true;
	// i'll stick these back in. even if these were bogus tile names it can't hurt
	if (!strcmp((char *) name, "wooden board") ||
		!strcmp( (char *) name, "stone pavern") ||
		!strcmp( (char *) name, "stone pavers"))
		return true;

	return false;
}*/

void cTileCache::unload()
{
	cComponent::unload();
}

void cTileCache::reload()
{
	unload();
	load();
}

enum Flags
{
	None = 0x00000000,
	Background = 0x00000001,
	Weapon = 0x00000002,
	Transparent = 0x00000004,
	Translucent = 0x00000008,
	Wall = 0x00000010,
	Damaging = 0x00000020,
	Impassable = 0x00000040,
	Wet = 0x00000080,
	Unknown1 = 0x00000100,
	Surface = 0x00000200,
	Bridge = 0x00000400,
	Generic = 0x00000800,
	Window = 0x00001000,
	NoShoot = 0x00002000,
	ArticleA = 0x00004000,
	ArticleAn = 0x00008000,
	Internal = 0x00010000,
	Foliage = 0x00020000,
	PartialHue = 0x00040000,
	Unknown2 = 0x00080000,
	Map = 0x00100000,
	Container = 0x00200000,
	Wearable = 0x00400000,
	LightSource = 0x00800000,
	Animation = 0x01000000,
	NoDiagonal = 0x02000000,
	Unknown3 = 0x04000000,
	Armor = 0x08000000,
	Roof = 0x10000000,
	Door = 0x20000000,
	StairBack = 0x40000000,
	StairRight = 0x80000000
};

void cTileCache::load()
{
	path = Config::instance()->mulPath();

	// Null out our placeholder tiles first
	memset(&emptyLandTile, 0, sizeof(land_st));
	memset(&emptyStaticTile, 0, sizeof(tile_st));

	QFile input(path + "tiledata.mul");
	int sizeFile = input.size();

	if (!input.open(QIODevice::ReadOnly))
		throw wpException(QString("Error opening file %1 for reading.").arg(path + "tiledata.mul"));
	
	// Begin reading in the Land-Tiles
	quint32 i, j;

	if (sizeFile == 3188736)//7.0.9.0+
	{
		for (i = 0; i < 0x4000; ++i)
		{
			if (i == 1 || (i > 0 && (i & 0x1F) == 0))
			{
				input.read(4); // header				
			}

			input.read((char *)&landTiles[i], 30);

		}

		// Repeat the same procedure for static tiles
		// NOTE: We are only interested in the REAL static tiles, nothing of
		// that ALHPA crap above it

		for (i = 0; i < 0x10000; ++i)
		{
			// Skip the header (Use unknown)
			if ((i & 0x1F) == 0)
			{
				input.read(4); // header
			}

			input.read((char *)&staticTiles[i], 41);
			
		}
		input.close();
	}
	else {
		for (i = 0; i < 512; ++i)
		{
			// Skip the header (Use unknown)
			input.seek((i * (4 + (32 * 26))) + 4);

			// Read all 32 blocks
			for (j = 0; j < 32; ++j)
			{
				quint16 tileId = (i * 32) + j;
				input.read((char *)&landTiles[tileId], 26);

			}
		}

		if (sizeFile == 1644544)
		{
			// Repeat the same procedure for static tiles
			// NOTE: We are only interested in the REAL static tiles, nothing of
			// that ALHPA crap above it
			quint32 skipLand = 512 * (4 + (32 * 26));

			for (i = 0; i < 512; ++i)
			{
				// Skip the header (Use unknown)
				input.seek(skipLand + (i * (4 + (32 * 37))) + 4);

				// Read all 32 blocks
				for (j = 0; j < 32; ++j)
				{
					quint16 tileId = (i * 32) + j;					
					input.read((char *)&staticTiles[tileId], 37); // Length of one record: 37
				}
			}
			input.close();
		}
		else
		{
			// Repeat the same procedure for static tiles
			// NOTE: We are only interested in the REAL static tiles, nothing of
			// that ALHPA crap above it
			quint32 skipLand = 512 * (4 + (32 * 26));

			for (i = 0; i < 512; ++i)
			{
				// Skip the header (Use unknown)
				input.seek(skipLand + (i * (4 + (32 * 37))) + 4);

				// Read all 32 blocks
				for (j = 0; j < 32; ++j)
				{
					quint16 tileId = (i * 32) + j;
					input.read((char *)&staticTiles[tileId], 37); // Length of one record: 37
				}
			}
			input.close();
		}
	}	

	/* File: verdata.mul
	** This file is no longer included with the recent releases of Ultima Online.
	** I'm commenting this out for now, since the client no longer uses it.
	// Now we got to check for verdata blocks
	input.setName( path + "verdata.mul" );
	if ( input.open( IO_ReadOnly ) )
	{
	quint32 patches;
	QDataStream verdata( &input );
	verdata.setByteOrder( QDataStream::LittleEndian );

	verdata >> patches;

	// Seek trough all patches
	for ( quint32 patchId = 0; patchId < patches; ++patchId )
	{
	verdata.device()->at( 4 + ( patchId * 21 ) );

	// Read the patch
	quint32 fileId, blockId, offset, length, extra;
	verdata >> fileId >> blockId >> offset >> length >> extra;

	if ( fileId != VERFILE_TILEDATA )
	continue;

	verdata.device()->at( offset + 4 ); // Skip the 4 byte header

	if ( blockId >= 512 )
	{
	blockId -= 512;
	for ( i = 0; i < 32; ++i )
	{
	quint16 tileId = ( blockId * 32 ) + i;
	tile_st tile;
	verdata.device()->read( ( char * ) &tile, sizeof( tile_st ) );

	if ( staticTiles.find( tileId ) != staticTiles.end() )
	staticTiles.erase( staticTiles.find( tileId ) );

	staticTiles.insert( make_pair( tileId, tile ) );
	}
	}
	else
	{
	for ( i = 0; i < 32; ++i )
	{
	quint16 tileId = ( blockId * 32 ) + i;
	land_st tile;
	verdata.device()->read( ( char * ) &tile, sizeof( land_st ) );

	if ( landTiles.find( tileId ) != landTiles.end() )
	landTiles.erase( landTiles.find( tileId ) );

	landTiles.insert( make_pair( tileId, tile ) );
	}
	}
	}
	}
	input.close();
	// End verdata.mul
	*/

	cComponent::load();
}

// Old Client Version
//void cTileCache::load()
//{
//	path = Config::instance()->mulPath();
//
//	// Null out our placeholder tiles first
//	memset( &emptyLandTile, 0, sizeof( land_st ) );
//	memset( &emptyStaticTile, 0, sizeof( tile_st ) );
//
//	QFile input( path + "tiledata.mul" );
//
//	if ( !input.open( QIODevice::ReadOnly ) )
//		throw wpException( QString( "Error opening file %1 for reading." ).arg( path + "tiledata.mul" ) );
//
//	// Begin reading in the Land-Tiles
//	quint32 i, j;
//
//	for ( i = 0; i < 512; ++i )
//	{
//		// Skip the header (Use unknown)
//		input.seek( ( i * ( 4 + ( 32 * 26 ) ) ) + 4 );
//
//		// Read all 32 blocks
//		for ( j = 0; j < 32; ++j )
//		{
//			quint16 tileId = ( i * 32 ) + j;
//			input.read( ( char * ) &landTiles[tileId], 26 );
//		}
//	}
//
//	// Repeat the same procedure for static tiles
//	// NOTE: We are only interested in the REAL static tiles, nothing of
//	// that ALHPA crap above it
//	quint32 skipLand = 512 * ( 4 + ( 32 * 26 ) );
//
//	for ( i = 0; i < 512; ++i )
//	{
//		// Skip the header (Use unknown)
//		input.seek( skipLand + ( i * ( 4 + ( 32 * 37 ) ) ) + 4 );
//
//		// Read all 32 blocks
//		for ( j = 0; j < 32; ++j )
//		{
//			quint16 tileId = ( i * 32 ) + j;
//			input.read( ( char * ) &staticTiles[tileId], 37 ); // Length of one record: 37
//		}
//	}
//	input.close();
//
//
//	/* File: verdata.mul
//	** This file is no longer included with the recent releases of Ultima Online.
//	** I'm commenting this out for now, since the client no longer uses it.
//	// Now we got to check for verdata blocks
//	input.setName( path + "verdata.mul" );
//	if ( input.open( IO_ReadOnly ) )
//	{
//		quint32 patches;
//		QDataStream verdata( &input );
//		verdata.setByteOrder( QDataStream::LittleEndian );
//
//		verdata >> patches;
//
//		// Seek trough all patches
//		for ( quint32 patchId = 0; patchId < patches; ++patchId )
//		{
//			verdata.device()->at( 4 + ( patchId * 21 ) );
//
//			// Read the patch
//			quint32 fileId, blockId, offset, length, extra;
//			verdata >> fileId >> blockId >> offset >> length >> extra;
//
//			if ( fileId != VERFILE_TILEDATA )
//				continue;
//
//			verdata.device()->at( offset + 4 ); // Skip the 4 byte header
//
//			if ( blockId >= 512 )
//			{
//				blockId -= 512;
//				for ( i = 0; i < 32; ++i )
//				{
//					quint16 tileId = ( blockId * 32 ) + i;
//					tile_st tile;
//					verdata.device()->read( ( char * ) &tile, sizeof( tile_st ) );
//
//					if ( staticTiles.find( tileId ) != staticTiles.end() )
//						staticTiles.erase( staticTiles.find( tileId ) );
//
//					staticTiles.insert( make_pair( tileId, tile ) );
//				}
//			}
//			else
//			{
//				for ( i = 0; i < 32; ++i )
//				{
//					quint16 tileId = ( blockId * 32 ) + i;
//					land_st tile;
//					verdata.device()->read( ( char * ) &tile, sizeof( land_st ) );
//
//					if ( landTiles.find( tileId ) != landTiles.end() )
//						landTiles.erase( landTiles.find( tileId ) );
//
//					landTiles.insert( make_pair( tileId, tile ) );
//				}
//			}
//		}
//	}
//	input.close();
//	// End verdata.mul
//	*/
//
//	cComponent::load();
//}

signed char cTileCache::tileHeight( const tile_st& t )
{
	if ( t.flag2 & 4 )
		return ( signed char ) ( t.height / 2 );	// hey, lets just RETURN half!
	return ( t.height );
}

signed char cTileCache::tileHeight( ushort tileId )
{
	const tile_st &tile = getTile( tileId );
	// For Stairs+Ladders
	return tileHeight( tile );
}
