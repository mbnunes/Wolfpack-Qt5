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

#include "targetrequests.h"
#include "maps.h"
#include "mapobjects.h"
#include "wpdefmanager.h"
#include "territories.h"
#include "items.h"
#include "itemsmgr.h"
#include "tilecache.h"

bool cAddItemTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;

	//QStringList arguments = QStringList::split( " ", npc_ );
	QDomElement *node = DefManager->getSection( WPDT_ITEM, item_ );

	// Check first if we even are able to create a char
	if( !node )
	{
		bool ok = false;
		hex2dec( item_ ).toULong( &ok );
		if( !ok )
		{
			socket->sysMessage( tr( "Item Definition '%1' not found" ).arg( item_ ) );
			return true;
		}
	}

	// Otherwise create our item here
	P_ITEM pItem = NULL;
	if( node )
		pItem = Items->createScriptItem( item_ );
	else
	{
		pItem = new cItem;
		pItem->Init();
		cItemsManager::getInstance()->registerItem( pItem );

		pItem->setName( "an item" );
		pItem->setId( hex2dec( item_ ).toULong() );
	}

	if( !pItem )
		return true;

	Coord_cl newPos = socket->player()->pos;
	newPos.x = target->x();
	newPos.y = target->y();
	newPos.z = target->z() + cTileCache::instance()->tileHeight( target->model() ); // Model Could be an NPC as well i dont like the idea...
	pItem->moveTo( newPos );

	// Send the item to its surroundings
	pItem->update();
	return true;
}

bool cAddNpcTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;

	//QStringList arguments = QStringList::split( " ", npc_ );
	QDomElement *node = DefManager->getSection( WPDT_NPC, npc_ );

	// Check first if we even are able to create a char
	if( !node )
	{
		bool ok = false;
		hex2dec( npc_ ).toULong( &ok );
		if( !ok )
		{
			socket->sysMessage( tr( "NPC Definition '%1' not found" ).arg( npc_ ) );
			return true;
		}
	}

	// Otherwise create our character here
	P_CHAR pChar = new cChar;
	pChar->Init();
	cCharsManager::getInstance()->registerChar( pChar );

	pChar->setPriv( 0x10 ); // No skill titles
	pChar->setNpc(1);
	Coord_cl newPos = socket->player()->pos;
	newPos.x = target->x();
	newPos.y = target->y();
	newPos.z = target->z() + cTileCache::instance()->tileHeight( target->model() ); // Model Could be a NPC as well i dont like the idea...
	pChar->moveTo( newPos );

	pChar->setRegion( cAllTerritories::getInstance()->region( pChar->pos.x, pChar->pos.y ) );
	if( node )
	{
		pChar->applyDefinition( (*node ) );
	}
	else
	{
		pChar->name = "Character";
		pChar->setId( hex2dec( npc_ ).toULong() );
		pChar->setXid(pChar->id());
	}

	// Send the char to it's surroundings
	pChar->resend( false ); // It's new so no need to remove it first

	return true;
};

bool cBuildMultiTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;
	
	QDomElement* DefSection = DefManager->getSection( WPDT_MULTI, multisection_ );
	if( DefSection->isNull() )
		return true;

	if( DefSection->attribute( "type" ) == "house" )
	{
		cHouse* pHouse = new cHouse();
		
		pHouse->build( *DefSection, target->x(), target->y(), target->z(), senderserial_, deedserial_ );
	}
	else if( DefSection->attribute( "type" ) == "boat" )
	{
		cBoat* pBoat = new cBoat();

		pBoat->build( *DefSection, target->x(), target->y(), target->z(), senderserial_, deedserial_ );
	}
	return true;
};


