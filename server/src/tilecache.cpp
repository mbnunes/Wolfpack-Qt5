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

//Wolfpack Includes

#include "tilecache.h"

bool cTileCache::load( const QString &nPath )
{
	clConsole.PrepareProgress( "Preloading Tiledata.mul" );

	path = nPath;

	// Null out our placeholder tiles first
	memset( &emptyLandTile, 0, sizeof( land_st ) );
	memset( &emptyStaticTile, 0, sizeof( tile_st ) );

	QFile input( path + "tiledata.mul"  );
	
	if( !input.open( IO_ReadOnly ) )
	{
		clConsole.ProgressFail();
		clConsole.send( "Couldn't open tiledata.mul!" );
		return false;
	}

	// Begin reading in the Land-Tiles
	UINT32 i, j;

	for( i = 0; i < 512; ++i )
	{
		// Skip the header (Use unknown)
		input.at( ( i * ( 4 + ( 32 * 26 ) ) ) + 4 );

		// Read all 32 blocks
		for( j = 0; j < 32; ++j )
		{
			UINT16 tileId = ( i * 32 ) + j;
			land_st landTile;
			input.readBlock( (char*)&landTile, 26 );

			// It's not an empty tile, so let's save it
			// We only compare until the first char of the name as
			// the junk behind the 00 doesnt interest us
			if( memcmp( &emptyLandTile, &landTile, ( 26 - 19 ) ) )
				landTiles.insert( make_pair( tileId, landTile ) );
		}
	}

	// Repeat the same procedure for static tiles
	// NOTE: We are only interested in the REAL static tiles, nothing of
	// that ALHPA crap above it
	UINT32 skipLand = 512 * ( 4 + ( 32 * 26	 ) );

	for( i = 0; i < 512; ++i )
	{
		// Skip the header (Use unknown)
		input.at( skipLand + ( i * ( 4 + ( 32 * 37 ) ) ) + 4 );

		// Read all 32 blocks
		for( j = 0; j < 32; ++j )
		{
			UINT16 tileId = ( i * 32 ) + j;
			tile_st staticTile;
			input.readBlock( (char*)&staticTile, 37 ); // Length of one record: 37

			// It's not an empty tile, so let's save it
			// We only compare until the first char of the name as
			// the junk behind the 00 doesnt interest us
			if( memcmp( &emptyStaticTile, &staticTile, 18 ) )
				staticTiles.insert( make_pair( tileId, staticTile ) );
		}
	}
	input.close();

	// Now we got to check for verdata blocks
	input.setName( path + "verdata.mul" );
	if( input.open( IO_ReadOnly ) )
	{
		UINT32 patches;
		QDataStream verdata( &input );
		verdata.setByteOrder( QDataStream::LittleEndian );

		verdata >> patches;

		// Seek trough all patches
		for( UINT32 patchId = 0; patchId < patches; ++patchId )
		{
			verdata.device()->at( 4 + ( patchId * 21 ) );

			// Read the patch
			UINT32 fileId, blockId, offset, length, extra;
			verdata >> fileId >> blockId >> offset >> length >> extra;

			if( fileId != VERFILE_TILEDATA )
				continue;

			verdata.device()->at( offset + 4 ); // Skip the 4 byte header

			if( blockId >= 512 )
			{
				blockId -= 512;
				for( i = 0; i < 32; ++i )
				{
					UINT16 tileId = ( blockId * 32 ) + i;
					tile_st tile;
					verdata.device()->readBlock( (char*)&tile, sizeof( tile_st ) );

					if( staticTiles.find( tileId ) != staticTiles.end() )
						staticTiles.erase( staticTiles.find( tileId ) );
	
					staticTiles.insert( make_pair( tileId, tile ) );
				}
			}
			else
			{
				for( i = 0; i < 32; ++i )
				{
					UINT16 tileId = ( blockId * 32 ) + i;
					land_st tile;
					verdata.device()->readBlock( (char*)&tile, sizeof( land_st ) );

					if( landTiles.find( tileId ) != landTiles.end() )
						landTiles.erase( landTiles.find( tileId ) );
	
					landTiles.insert( make_pair( tileId, tile ) );
				}
			}
		}
	}
	input.close();

	clConsole.ProgressDone();

	return true;
}

// Get's a land-tile out of the cache
land_st cTileCache::getLand( UINT16 tileId )
{
	if( landTiles.find( tileId ) == landTiles.end() )
		return emptyLandTile;
	else
		return landTiles.find( tileId )->second;
}

// The same for static-tiles
tile_st cTileCache::getTile( UINT16 tileId )
{
	if( staticTiles.find( tileId ) == staticTiles.end() )
		return emptyStaticTile;
	else
		return staticTiles.find( tileId )->second;
}
