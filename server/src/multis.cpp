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

#include "multis.h"

#include "regions.h"
#include "tilecache.h"
#include "mapstuff.h"

#undef DBGFILE
#define DBGFILE "multis.cpp" 

cMulti::cMulti()
{
	cItem::Init( false );
	deedsection_ = (char*)0;
}

void cMulti::Serialize( ISerialization &archive )
{
	cItem::Serialize( archive );
}

/*
void cMulti::processNode( const QDomElement &Tag )
{
	cItem::processNode( Tag );
}
*/

bool cMulti::inMulti( const Coord_cl &srcpos )
{
	return inMulti( srcpos, this->pos, this->id() );
}

bool cMulti::inMulti( const Coord_cl &srcpos, const Coord_cl &multipos, UI16 id )
{
	SI32 length;			// signed long int on Intel
	st_multi multi;
	UOXFile *mfile;
	Map->SeekMulti( id - 0x4000, &mfile, &length );
	length = length / sizeof( st_multi );
	if (length == -1 || length >= 17000000) // Too big...
	{
		LogError( (char*)QString( "cMulti::inMulti( ... ) - Bad length in multi file. Avoiding stall.\n").latin1() );
		length = 0;
	}
	register int j;
	for ( j = 0; j < length; ++j)
	{
		mfile->get_st_multi( &multi );
		if ( multi.visible && ( multipos.x + multi.x == srcpos.x) && ( multipos.y + multi.y == srcpos.y ) )
		{
			return true;
		}
	}
	return false;
}

cMulti* cMulti::findMulti( const Coord_cl &pos )
{
	SI32 lastdistance = 30;
	cMulti* pMulti = NULL;
	SI32 currdistance;

	cRegion::RegionIterator4Items ri( pos );
	
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		pMulti = dynamic_cast< cMulti* >(ri.GetData());
		if( pMulti )
		{
			currdistance = pos.distance( pMulti->pos );
			if( currdistance <= lastdistance )
			{
				lastdistance = currdistance;
				if( !pMulti->inMulti( pos ) )
					pMulti = NULL;
			}
		}
	}

	return pMulti;
}