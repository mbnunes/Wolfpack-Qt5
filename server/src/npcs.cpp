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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "wolfpack.h"
#include "itemid.h"
#include "territories.h"
#include "debug.h"
#include "utilsys.h"
#include "walking.h"
#include "mapobjects.h"
#include "wpdefmanager.h"
#include "wpdefaultscript.h"
#include "network.h"
#include "classes.h"
#include "maps.h"
#include "multis.h"

#include <string>

#undef  DBGFILE
#define DBGFILE "npcs.cpp"

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	if( !pc_k )
		return;

	if( pc_k->spawnSerial() != INVALID_SERIAL ) 
		cspawnsp.remove(pc_k->spawnSerial(), pc_k->serial());

	pc_k->setOwner( 0 );
	pc_k->setGuarding( 0 );
	
	// We need to remove the equipment here.
	cChar::ContainerContent container(pc_k->content());
	cChar::ContainerContent::const_iterator it (container.begin());
	cChar::ContainerContent::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pItem = *it;
		if( !pItem )
			continue;

		Items->DeleItem( pItem );
	}

	// multi check
	if( pc_k->multis() != INVALID_SERIAL )
	{
		cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pc_k->multis() ) );
		if( pMulti )
		{
			pMulti->removeChar( pc_k );
		}
	}

	pc_k->removeFromView( false ); // Remove the character from all in-range sockets view
	MapObjects::instance()->remove( pc_k ); // taking it out of mapregions BEFORE x,y changed
	pc_k->del(); // Remove from Database
	pc_k->free = true;
	CharsManager::instance()->deleteChar( pc_k );
}

void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int z;

	if ( pc_k == NULL ) return;
	
	//P_CHAR pc_c = new cChar(*pc_k);
	P_CHAR pc_c = new cChar( pc_k );
	if ( pc_c == NULL ) return;
//	pc_c->Init();
	pc_c->setSerial(CharsManager::instance()->getUnusedSerial());
	pc_c->setFtarg(INVALID_SERIAL);
	pc_c->moveTo(pc_k->pos() + Coord_cl(1, 0, 0) );
	pc_c->setKills(0);
	pc_c->setHp( pc_k->st() );
	pc_c->setStm( pc_k->realDex() );
	pc_c->setMn( pc_k->in() );
	z=rand()%35;
	if (z == 5) 
		pc_c->setSplit(1); 
	else 
		pc_c->setSplit(0);	
	pc_c->update();
}

P_CHAR cCharStuff::createScriptNpc( const QString &section, const Coord_cl &pos )
{
	if( section.isNull() || section.isEmpty() )
		return NULL;

	const QDomElement* DefSection = DefManager->getSection( WPDT_NPC, section );

	if( !DefSection || DefSection->isNull() )
	{
		clConsole.log( QString( "Unable to create unscripted npc: %1\n" ).arg( section ).latin1() );
		return NULL;
	}

	P_CHAR pChar = new cChar;
	pChar->Init();
	CharsManager::instance()->registerChar( pChar );

	pChar->setPriv( 0x10 );
	pChar->setNpc(1);
	pChar->setLoDamage(1);
	pChar->setHiDamage(1);
	pChar->setDef(1);
	pChar->setSpawnSerial( INVALID_SERIAL );

	pChar->moveTo( pos );

	pChar->setRegion( cAllTerritories::getInstance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map ) );

	pChar->applyDefinition( *DefSection );

	// Now we call onCreate
	pChar->onCreate( section );

	pChar->resend( false );

	return pChar;
}

////////////
// Name:	inGuardedArea
// history:	by Duke, 13.1.2002
// Purpose:	checks if the char is in a guarded region
// Remark:	the recalculation of the region is necessary because it is not maintained properly :(
//			I think it is better to do this only when needed
//
bool cChar::inGuardedArea()
{
	cTerritory* Region = cAllTerritories::getInstance()->region( this->pos().x, this->pos().y, this->pos().map );
	if( Region )
		return Region->isGuarded();
	else
		return false;
}

