//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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

// Platform include
#include "platform.h"

// Wolfpack includes
#include "chars.h"
#include "basechar.h"
#include "items.h"
#include "player.h"
#include "npc.h"
#include "globals.h"
#include "multis.h"
#include "world.h"
#include "mapobjects.h"
#include "wpdefmanager.h"
#include "wpconsole.h"

// Qt Includes
#include <qstringlist.h>

// Libary Includes
#include <math.h>

#undef  DBGFILE
#define DBGFILE "chars.cpp"

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	if( !pc_k )
		return;

	P_PLAYER pp_k = dynamic_cast<P_PLAYER>(pc_k);
	P_NPC pn_k = dynamic_cast<P_NPC>(pc_k);

	if( pn_k )
	{
		pn_k->setOwner( 0 );
	}

	pc_k->setGuarding( 0 );
	
	// We need to remove the equipment here.
	cBaseChar::ItemContainer container(pc_k->content());
	cBaseChar::ItemContainer::const_iterator it (container.begin());
	cBaseChar::ItemContainer::const_iterator end(container.end());
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
	World::instance()->deleteObject( pc_k );
}

/*
void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int z;

	if ( pc_k == NULL ) return;
	
	//P_CHAR pc_c = new cChar(*pc_k);
	P_CHAR pc_c = new cChar( pc_k );
	if ( pc_c == NULL ) return;
//	pc_c->Init();
	pc_c->setSerial( World::instance()->findCharSerial() );
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
*/

P_NPC cCharStuff::createScriptNpc( const QString &section, const Coord_cl &pos )
{
	if( section.isNull() || section.isEmpty() )
		return NULL;

	const QDomElement* DefSection = DefManager->getSection( WPDT_NPC, section );

	if( !DefSection || DefSection->isNull() )
	{
		clConsole.log( LOG_ERROR, QString( "Unable to create unscripted npc: %1\n" ).arg( section ) );
		return NULL;
	}

	P_NPC pChar = new cNPC;
	pChar->Init();

	pChar->setMinDamage(1);
	pChar->setMaxDamage(1);
	pChar->setBodyArmor(1);

	pChar->moveTo( pos );

	pChar->setRegion( AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map ) );

	pChar->applyDefinition( *DefSection );

	// Now we call onCreate
	pChar->onCreate( section );

	pChar->resend( false );

	return pChar;
}

