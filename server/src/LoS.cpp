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

/////////////////////
// Name:	LoS.cpp
// History:	cut from wolfpack.cpp by Duke, 26.10.00
//

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"
#include "mapobjects.h"
#include "mapstuff.h"
#include "tilecache.h"
#include "itemid.h"

#include <set>

#undef  DBGFILE
#define DBGFILE "LoS.cpp"

int lineOfSight( const Coord_cl &source, const Coord_cl &target, int checkfor )
{
/*
Char (source.x, source.y, source.z) is the char(pc/npc), Target (target.x, target.y, target.z) is the target.
s is for pc's, in case a message needs to be sent.
the checkfor is what is checked for along the line of sight.
Look at uox3.h to see options. Works like npc magic.

#define TREES_BUSHES 1 // Trees and other large vegetaion in the way
#define WALLS_CHIMNEYS 2	// Walls, chimineys, ovens, etc... in the way
#define DOORS 4 // Doors in the way
#define ROOFING_SLANTED 8	// So can't tele onto slanted roofs, basically
#define FLOORS_FLAT_ROOFING 16	// For attacking between floors
#define LAVA_WATER 32	// Don't know what all to use this for yet
  
Just or (|) the values for the diff things together to get what to search for.
So put in place of the paramater checkfor for example
	
if (lineOfSight( source.x, source.y, source.z, target.x, target.y, target.z, WALLS_CHIMNEYS | DOORS | ROOFING_SLANTED))
	  
		
This whole thing is based on the Pythagorean Theorem.  What I have done is
taken the coordinates from both chars and created a right triange with the
hypotenuse as the line of sight.  Then by sectioning off side "a" into a number
of equal lengths and finding the other sides lengths according to that one,  I
have been able to find the coordinates of the tiles that lie along the line of
sight(line "c").  Then these tiles are searched from an item that would block
the line of sight.
*/
	
	bool blocked = false;
	bool not_blocked = true;

	if( target.map != source.map )
		return blocked;

	if( target.x == -1 && target.y == -1 )  
		return not_blocked;		// target canceled

	if( (source.x == target.x) && (source.y == target.y) && (source.z == target.z) )
		return not_blocked;		// if source and target are on the same position

//	target.z += 15; // standard eye height of most bodies

	SI32 n = ( target.x - source.x ), m = ( target.y - source.y ), i = 0;
	SI08 sgn_x = ( source.x <= target.x ) ? 1 : (-1); // signum for x
	SI08 sgn_y = ( source.y <= target.y ) ? 1 : (-1); // signum for y
	SI08 sgn_z = ( source.z <= target.z ) ? 1 : (-1); // signum for z
	if( source.x == target.x )
		sgn_x = 0;
	if( source.y == target.y )
		sgn_y = 0;
	if( source.z == target.z )
		sgn_z = 0;

//	line3D lineofsight = line3D( vector3D( kox1, koy1, koz1 ), vector3D( (koxn-kox1), (koym-koy1), (koz2-koz1) ) );

	UI32 distance = source.distance( target );

	if( distance > 18 )
		return blocked;

	QValueList< Coord_cl > collisions;

	double dz_x = ( sgn_x == 0 ) ? ( (double)target.z - (double)source.z ) : ( ( (double)target.z - (double)source.z ) / ( (double)target.x - (double)source.x ) );
	double dz_y = ( sgn_y == 0 ) ? ( (double)target.z - (double)source.z ) : ( ( (double)target.z - (double)source.z ) / ( (double)target.y - (double)source.y ) );
	SI32 dz = 0; // dz is needed later for collisions of the ray with floor tiles
	if( sgn_x == 0 || m > n )
		dz = (SI32)floor( dz_y );
	else
		dz = (SI32)floor( dz_x );

	
	if( sgn_x == 0 && sgn_y == 0 && !sgn_z == 0 ) // should fix shooting through floor issues
		for( i = 0; i < abs( target.z - source.z ); i++ )
		{
			collisions.push_back( Coord_cl( source.x, source.y, source.z + sgn_z, source.map ) );
		}
	else if( sgn_x == 0 ) // if we are on the same x-level, just push every x/y coordinate in y-direction from src to trg into the array
		for( i = 0; i < (sgn_y * m); i++ )
		{
			collisions.push_back( Coord_cl( source.x, source.y + (sgn_y * i), source.z + (SI08)floor( dz_y * (double)i ), source.map ) );
		}
	else if ( sgn_y == 0 ) // if we are on the same y-level, just push every x/y coordinate in x-direction from src to trg into the array
		for( i = 0; i < (sgn_x * n); i++ )
		{
			collisions.push_back( Coord_cl( source.x + (sgn_x * i), source.y, source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
		}
	else
	{
		for( i = 0; (sgn_x * n >= sgn_y * m) && (i < (sgn_x * n)); i++ )
		{
			SI32 gridx = source.x + (sgn_x * i);
			if( ( ( n == 0 ) && ( gridx == 0 ) ) ||
				( ( n + ( gridx * m ) == 0 ) ) )
				continue;
			else
			{
				// linear evaluation of extended 2x2 matrix, abbreviated
				double t = (double)sgn_x * (double)i * (double)m / (double)n + (double)source.y;

				// the next one is somewhat tricky, if the line of sight exactly cuts a coordinate,
				// we just have to take that coordinate...
				if( floor( t ) == t ) 
				{
					collisions.push_back( Coord_cl( gridx, floor( t ), source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
				}
				// but if not, we have to take BOTH coordinates, which the calculated collision is between!
				else
				{ 
					collisions.push_back( Coord_cl( gridx, floor( t ), source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
					collisions.push_back( Coord_cl( gridx, ceil( t ), source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
				}
			}
		}
	
		for( i = 0; (sgn_y * m > sgn_x * n) && (i < (sgn_y * m)); i++ )
		{
			SI32 gridy = source.y + (sgn_y * i);
			if( ( ( m == 0 ) && ( gridy == 0 ) ) ||
				( ( m + ( gridy * n ) == 0 ) ) )
				continue;
			else
			{
				double t = (double)source.x + (double)n / (double)m * (double)sgn_y * (double)i;

				// the next one is somewhat tricky, if the line of sight exactly cuts a coordinate,
				// we just have to take that coordinate...
				if( floor( t ) == t ) 
				{
					collisions.push_back( Coord_cl( floor( t ), gridy, source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
				}
				// but if not, we have to take BOTH coordinates, which the calculated collision is between!
				else
				{ 
					collisions.push_back( Coord_cl( floor( t ), gridy, source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
					collisions.push_back( Coord_cl( ceil( t ), gridy, source.z + (SI08)floor( dz_x * (double)i ), source.map ) );
				}
			}
		}
	}

	// the next will search for multis
	QPtrList< cItem > multis;
	RegionIterator4Items ri( source );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_ITEM pi = ri.GetData();
		if( pi && pi->id() >= 0x4000 )
		{
			multis.append( pi );
		}
	}

	map_st map1, map2;
	std::set< UI16 > itemids;
	SI32 length, j, k;

	QValueList< Coord_cl >::iterator pit = collisions.begin();
	while( pit != collisions.end() )
	{
		MapStaticIterator msi( *pit );
		// Texture mapping  
		map1 = Map->SeekMap( *pit );
		map2 = Map->SeekMap( Coord_cl( (*pit).x + sgn_x, (*pit).y + sgn_y, (*pit).z, source.map ) );
		
		if( (map1.id != 2) && (map2.id != 2) ) 
		{
			// Mountain walls
			if( ( ( map1.z < map2.z ) &&						// 1) lineofsight collides with a map "wall"
				( (*pit).z < ( map2.z ) ) &&
				( (*pit).z >= ( map1.z ) ) ) ||
				( ( map1.z > map2.z ) &&
				( (*pit).z < ( map1.z ) ) &&
				( (*pit).z >= ( map2.z ) ) ) ||
//				( ( (*pit).z == map1.z ) &&				// 2) lineofsight cuts a map "floor"
//				( dz != 0 ) ) ||
				( ( ( map1.id >= 431  && map1.id <= 432  ) ||	// 3) lineofsight cuts a mountain
				( map1.id >= 467  && map1.id <= 475  ) ||
				( map1.id >= 543  && map1.id <= 560  ) ||
				( map1.id >= 1754 && map1.id <= 1757 ) ||
				( map1.id >= 1787 && map1.id <= 1789 ) ||
				( map1.id >= 1821 && map1.id <= 1824 ) ||
				( map1.id >= 1851 && map1.id <= 1854 ) ||
				( map1.id >= 1881 && map1.id <= 1884 ) ) &&
				( msi.First() == NULL ) ) ) // make sure there is no static item!
				return blocked;
		}	 
			
		// Statics
		tile_st tile;
		staticrecord *stat = msi.First();
		while( stat )
		{
			msi.GetTile( &tile );
			if(	( (*pit).z >= stat->zoff && (*pit).z <= ( stat->zoff + tile.height ) ) )
//				||	( tile.height <= 2 && abs( (*pit).z - stat->zoff ) <= abs( dz ) ) )
				itemids.insert( stat->itemid );

			stat = msi.Next();
		}
			
			
		// Items
		RegionIterator4Items rj( (*pit), 0 );
		for( rj.Begin(); !rj.atEnd(); rj++ )
		{
			P_ITEM pi = rj.GetData();
			if( pi && pi->id() < 0x4000 )
			{
				tile = cTileCache::instance()->getTile( pi->id() );
				if(	( (*pit).z >= pi->pos.z ) && ( (*pit).z <= ( pi->pos.z + tile.height ) ) && ( pi->visible == 0 ) )
					itemids.insert( pi->id() );
			}
		}

		// Multis
		QPtrListIterator< cItem > mit( multis );
		P_ITEM pi;
		while( pi = mit.current() )
		{
			st_multi multi;
			UOXFile *mfile;
			Map->SeekMulti( pi->id() - 0x4000, &mfile, &length );
			if( length == -1 || length >= 17000000 )//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
			{
				clConsole.send( tr("lineOfSight: Bad length in multi file. Avoiding stall") );
				length = 0;
			}
			for( j = 0; j < length; j++ )
			{
				mfile->get_st_multi( &multi );
				if( ( multi.visible ) && ( pi->pos.x + multi.x == (*pit).x ) &&
					( pi->pos.y + multi.y == (*pit).y ) )			
				{
					tile = cTileCache::instance()->getTile( multi.tile );
					if( ( (*pit).z >= pi->pos.z + multi.z ) &&
						( (*pit).z <= pi->pos.z + multi.z + tile.height ) )
						itemids.insert( multi.tile );
				}
			}
			++mit;
		}
		++pit;
	}
 
	std::set< UI16 >::iterator sit = itemids.begin();
	while( sit != itemids.end() )
	{
		/*
		#define TREES_BUSHES 1 // Trees and other large vegetaion in the way
		#define WALLS_CHIMNEYS 2	// Walls, chimineys, ovens, etc... in the way
		#define DOORS 4 // Doors in the way
		#define ROOFING_SLANTED 8	// So can't tele onto slanted roofs, basically
		#define FLOORS_FLAT_ROOFING 16	// For attacking between floors
		#define LAVA_WATER 32	// Don't know what all to use this for yet
		*/
		if( ( checkfor & TREES_BUSHES && IsTree_Bush( *sit ) ) ||
			( checkfor & WALLS_CHIMNEYS && IsWall_Chimney( *sit ) ) ||
			( checkfor & DOORS && IsDoor( *sit ) ) ||
			( checkfor & ROOFING_SLANTED && IsRoofing_Slanted( *sit ) ) ||
			( checkfor & FLOORS_FLAT_ROOFING && IsFloor_Flat_Roofing( *sit ) ) ||
			( checkfor & LAVA_WATER && IsLavaWater( *sit ) ) )
			return blocked;
		++sit;
	}
	return not_blocked;
} //function

