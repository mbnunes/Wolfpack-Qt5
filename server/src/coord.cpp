//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================


// Include files

#include "coord.h"
#include "player.h"
#include "network.h"
#include "sectors.h"
#include "items.h"
#include "multiscache.h"
#include "maps.h"
#include "globals.h"
#include "tilecache.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"

// System Includes
#include <math.h>
#include <set>

Coord_cl Coord_cl::null( 0xFFFF, 0xFFFF, 0xFF, 0xFF );

Coord_cl::Coord_cl(void)
{
	x = y = z = map = 0;
}

Coord_cl::Coord_cl(const Coord_cl& clCoord)
{
	(*this) = clCoord;
}

Coord_cl::Coord_cl(UI16 uiX, UI16 uiY, SI08 siZ, UI08 uiMap, UI08)
{
	x = uiX;
	y = uiY;
	z = siZ;
	map = uiMap;
	//plane = uiPlane;
}


/*!
 * Calculates the distance between 2 coordinates. If they are in different planes and maps, the
 * return value is infinite (0xFFFFFFFF). The exception is if one of the planes is COORD_ALL_PLANES
 *
 * @param &src : Coordinate to compare
 *
 * @return UI32  : distance result
 */
unsigned int Coord_cl::distance( const Coord_cl &src ) const
{
	UI32 uiResult = ~0U; // Initialize with *infinite*
	if ( map == src.map )
	{
		RF64 rfResult = sqrt(static_cast<RF64>((x - src.x)*(x - src.x) + (y - src.y)*(y - src.y)/* + (z - src.z)*(z - src.z)*/));
		//		RF64 rfResult = sqrt(static_cast<RF64>((x * src.x + y * src.y + z * src.z)));
		uiResult = static_cast<UI32>(floor(rfResult)); // truncate
	}
	return uiResult;
}

UI32 Coord_cl::distance ( const Coord_cl &a, const Coord_cl &b )
{
	return a.distance( b );
}

// Operators
Coord_cl& Coord_cl::operator=(const Coord_cl& clCoord)
{
	x = clCoord.x;
	y = clCoord.y;
	z = clCoord.z;
	map = clCoord.map;
	return (*this);
}

bool Coord_cl::operator== (const Coord_cl& src) const
{
	return (x == src.x && y == src.y && z == src.z && map == src.map);
}

bool Coord_cl::operator!= (const Coord_cl& src) const
{
	return !(x == src.x && y == src.y && z == src.z && map == src.map);
}

Coord_cl Coord_cl::operator+ (const Coord_cl& src) const
{
	return Coord_cl(this->x + src.x, this->y + src.y, this->z + src.z, this->map );
}

Coord_cl Coord_cl::operator- (const Coord_cl& src) const
{
	return Coord_cl(this->x - src.x, this->y - src.y, this->z - src.z, this->map );
}

void Coord_cl::effect( UINT16 id, UINT8 speed, UINT8 duration, UINT16 hue, UINT16 renderMode )
{
	cUOTxEffect effect;
	effect.setType( ET_STAYSOURCEPOS );
	effect.setSourcePos( (*this) );
	effect.setId( id );
    effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && ( mSock->player()->pos().distance( (*this) ) <= mSock->player()->visualRange() ) )
			mSock->send( &effect );
	}
}

// Calculates the direction from one location to another
unsigned char Coord_cl::direction( const Coord_cl &dest ) const
{
	unsigned char dir;
	short xdif, ydif;

	xdif = dest.x - x;
	ydif = dest.y - y;

	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=-1;

	return dir;
}

// This didn't change much yet
// I just copied over the old code
bool Coord_cl::lineOfSight( const Coord_cl &target, bool touch )
{
	if( target.map != map )
		return false;

	if( (x == target.x) && (y == target.y) && (z == target.z) )
		return true;		// if source and target are on the same position

	SI32 n = ( target.x - x ), m = ( target.y - y ), i = 0;
	SI08 sgn_x = ( x <= target.x ) ? 1 : (-1); // signum for x
	SI08 sgn_y = ( y <= target.y ) ? 1 : (-1); // signum for y
	SI08 sgn_z = ( z <= target.z ) ? 1 : (-1); // signum for z
	if( x == target.x )
		sgn_x = 0;
	if( y == target.y )
		sgn_y = 0;
	if( z == target.z )
		sgn_z = 0;

	UI32 distance = this->distance( target );

	if( distance > 18 )
		return false;

	QValueList< Coord_cl > collisions;

	double dz_x = ( sgn_x == 0 ) ? ( (double)target.z - (double)z ) : ( ( (double)target.z - (double)z ) / ( (double)target.x - (double)x ) );
	double dz_y = ( sgn_y == 0 ) ? ( (double)target.z - (double)z ) : ( ( (double)target.z - (double)z ) / ( (double)target.y - (double)y ) );
	SI32 dz = 0; // dz is needed later for collisions of the ray with floor tiles
	if( sgn_x == 0 || m > n )
		dz = (SI32)floor( dz_y );
	else
		dz = (SI32)floor( dz_x );

	
	if( sgn_x == 0 && sgn_y == 0 && !sgn_z == 0 ) // should fix shooting through floor issues
		for( i = 0; i < abs( target.z - z ); ++i )
		{
			collisions.push_back( Coord_cl( x, y, z + sgn_z, map ) );
		}
	else if( sgn_x == 0 ) // if we are on the same x-level, just push every x/y coordinate in y-direction from src to trg into the array
		for( i = 0; i < (sgn_y * m); ++i )
		{
			collisions.push_back( Coord_cl( x, y + (sgn_y * i), z + (SI08)floor( dz_y * (double)i ), map ) );
		}
	else if ( sgn_y == 0 ) // if we are on the same y-level, just push every x/y coordinate in x-direction from src to trg into the array
		for( i = 0; i < (sgn_x * n); ++i )
		{
			collisions.push_back( Coord_cl( x + (sgn_x * i), y, z + (SI08)floor( dz_x * (double)i ), map ) );
		}
	else
	{
		for( i = 0; (sgn_x * n >= sgn_y * m) && (i < (sgn_x * n)); i++ )
		{
			SI32 gridx = x + (sgn_x * i);
			if( ( ( n == 0 ) && ( gridx == 0 ) ) ||
				( ( n + ( gridx * m ) == 0 ) ) )
				continue;
			else
			{
				// linear evaluation of extended 2x2 matrix, abbreviated
				double t = (double)sgn_x * (double)i * (double)m / (double)n + (double)y;

				// the next one is somewhat tricky, if the line of sight exactly cuts a coordinate,
				// we just have to take that coordinate...
				if( floor( t ) == t ) 
				{
					collisions.push_back( Coord_cl( gridx, floor( t ), z + (SI08)floor( dz_x * (double)i ), map ) );
				}
				// but if not, we have to take BOTH coordinates, which the calculated collision is between!
				else
				{ 
					collisions.push_back( Coord_cl( gridx, floor( t ), z + (SI08)floor( dz_x * (double)i ), map ) );
					collisions.push_back( Coord_cl( gridx, ceil( t ), z + (SI08)floor( dz_x * (double)i ), map ) );
				}
			}
		}
	
		for( i = 0; (sgn_y * m > sgn_x * n) && (i < (sgn_y * m)); ++i )
		{
			SI32 gridy = y + (sgn_y * i);
			if( ( ( m == 0 ) && ( gridy == 0 ) ) ||
				( ( m + ( gridy * n ) == 0 ) ) )
				continue;
			else
			{
				double t = (double)x + (double)n / (double)m * (double)sgn_y * (double)i;

				// the next one is somewhat tricky, if the line of sight exactly cuts a coordinate,
				// we just have to take that coordinate...
				if( floor( t ) == t ) 
				{
					collisions.push_back( Coord_cl( floor( t ), gridy, z + (SI08)floor( dz_y * (double)i ), map ) );
				}
				// but if not, we have to take BOTH coordinates, which the calculated collision is between!
				else
				{ 
					collisions.push_back( Coord_cl( floor( t ), gridy, z + (SI08)floor( dz_y * (double)i ), map ) );
					collisions.push_back( Coord_cl( ceil( t ), gridy, z + (SI08)floor( dz_y * (double)i ), map ) );
				}
			}
		}
	}

	// the next will search for multis
	QPtrList< cItem > multis;
	RegionIterator4Items ri( *this );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_ITEM pi = ri.GetData();
		if( pi && pi->id() >= 0x4000 )
		{
			multis.append( pi );
		}
	}

	map_st map1, map2;
	std::set< unsigned short > itemids;
	SI32 j;

	QValueList< Coord_cl >::iterator pit = collisions.begin();
	while( pit != collisions.end() )
	{
		// Texture mapping  
		map1 = Map->seekMap( *pit );
		map2 = Map->seekMap( Coord_cl( (*pit).x + sgn_x, (*pit).y + sgn_y, (*pit).z, map ) );
		
		StaticsIterator msi = Map->staticsIterator( *pit );
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
				( msi.atEnd() ) ) ) // make sure there is no static item!
				return false;
		}	 
			
		// Statics
		tile_st tile;
		while( !msi.atEnd() )
		{
			tile = TileCache::instance()->getTile( msi->itemid );
			if(	( (*pit).z >= msi->zoff && (*pit).z <= ( msi->zoff + tile.height ) ) )
			{
				// dont take surfaces into account under certain circumstances
				if( tile.flag2 & 0x02 )
				{
					if( !(z >= target.z && target.z >= (*pit).z) &&
						!(z <= target.z && z >= (*pit).z) )
						itemids.insert( msi->itemid );
				}
				else
					itemids.insert( msi->itemid );
			}

			++msi;
		}
			
			
		// Items
		RegionIterator4Items rj( (*pit), 0 );
		for( rj.Begin(); !rj.atEnd(); rj++ )
		{
			P_ITEM pi = rj.GetData();
			if( pi && pi->id() < 0x4000 )
			{
				tile = TileCache::instance()->getTile( pi->id() );
				if(	( (*pit).z >= pi->pos().z ) && ( (*pit).z <= ( pi->pos().z + tile.height ) ) && ( pi->visible() == 0 ) )
					itemids.insert( pi->id() );
			}
		}

		// Multis
		QPtrListIterator< cItem > mit( multis );
		P_ITEM pi;
		while( ( pi = mit.current() ) )
		{
			MultiDefinition* def = MultiCache::instance()->getMulti( pi->id() - 0x4000 );
			if ( !def )
				continue;
			QValueVector<multiItem_st> multi = def->getEntries();
			for( j = 0; j < multi.size(); ++j )
			{
				if( ( multi[j].visible ) && ( pi->pos().x + multi[j].x == (*pit).x ) &&
					( pi->pos().y + multi[j].y == (*pit).y ) )			
				{
					tile = TileCache::instance()->getTile( multi[j].tile );
					if( ( (*pit).z >= pi->pos().z + multi[j].z ) &&
						( (*pit).z <= pi->pos().z + multi[j].z + tile.height ) )
						itemids.insert( multi[j].tile );
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
			if touch, then ignore missing noshoot
		*/
		tile_st tile = TileCache::instance()->getTile( *sit );

		// No Shoot
		if( tile.flag2 & 0x20 )
			return false;

		// Touch & Impassable
		if( touch && ( tile.flag1 & 0x40 ) )
			return false;

		++sit;
	}

	return true;
}
