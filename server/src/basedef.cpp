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

#include "basedef.h"
#include "globals.h"
#include "wpdefmanager.h"
#include "basics.h"

#include <string.h>

cCharBaseDef *cBaseDefManager::getCharBaseDef( unsigned short id )
{
	if( id >= 0x400 )
		return 0;

	return chardefs[ id ];
}

void cBaseDefManager::load()
{
	for( unsigned int i = 0; i < 0x400; ++i )
	{
		// Check for <basechar id="">
		const cElement *element = DefManager->getDefinition( WPDT_CHARBASE, "0x" + QString::number( i, 16 ) );

		if( !element )
		{
			chardefs[i] = 0;
			continue;
		}

		chardefs[i] = new cCharBaseDef;
		chardefs[i]->basesound_ = hex2dec( element->getAttribute( "basesound", "0" ) ).toUShort();
		chardefs[i]->soundmode_ = hex2dec( element->getAttribute( "soundmode", "0" ) ).toUShort();
		chardefs[i]->shrinked_ = hex2dec( element->getAttribute( "shrinked", "0" ) ).toUShort();
		chardefs[i]->flags_ = hex2dec( element->getAttribute( "flags", "0" ) ).toUInt();	
		chardefs[i]->type_ = hex2dec( element->getAttribute( "type", "0" ) ).toUShort();
	}
}

void cBaseDefManager::unload()
{
	unsigned short i = 0;
	while( i < 0x400 )
		delete chardefs[i++];

	memset( &chardefs, 0, 0x400 * sizeof( cCharBaseDef* ) );
}

cBaseDefManager::cBaseDefManager()
{
	memset( &chardefs, 0, 0x400 * sizeof( cCharBaseDef* ) );
}

cBaseDefManager::~cBaseDefManager()
{
	unload();
}
