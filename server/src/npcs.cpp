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
#include "SndPkg.h"
#include "debug.h"
#include "utilsys.h"
#include "walking.h"
#include "mapobjects.h"
#include "wpdefmanager.h"
#include "wpdefaultscript.h"
#include "network.h"
#include "classes.h"
#include "maps.h"

#include <string>

#undef  DBGFILE
#define DBGFILE "npcs.cpp"

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	if( !pc_k )
		return;

	if( pc_k->spawnSerial() != INVALID_SERIAL ) 
		cspawnsp.remove(pc_k->spawnSerial(), pc_k->serial);

	pc_k->setOwner( NULL );
	
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
	if( pc_k->multis != INVALID_SERIAL )
	{
		cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pc_k->multis ) );
		if( pMulti )
		{
			pMulti->removeChar( pc_k );
		}
	}
	

	pc_k->removeFromView( false ); // Remove the character from all in-range sockets view
	cMapObjects::getInstance()->remove( pc_k ); // taking it out of mapregions BEFORE x,y changed
	pc_k->del(); // Remove from Database
	pc_k->free = true;
	CharsManager::instance()->deleteChar( pc_k );
}

void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int z;

	if ( pc_k == NULL ) return;
	
	P_CHAR pc_c = new cChar(*pc_k);
	if ( pc_c == NULL ) return;
//	pc_c->Init();
	pc_c->setSerial(CharsManager::instance()->getUnusedSerial());
	pc_c->setFtarg(INVALID_SERIAL);
	pc_c->moveTo(pc_k->pos + Coord_cl(1, 0, 0) );
	pc_c->setKills(0);
	pc_c->setHp( pc_k->st() );
	pc_c->setStm( pc_k->realDex() );
	pc_c->setMn( pc_k->in() );
	z=rand()%35;
	if (z == 5) 
		pc_c->setSplit(1); 
	else 
		pc_c->setSplit(0);	
	updatechar(pc_c);
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

	pChar->setRegion( cAllTerritories::getInstance()->region( pChar->pos.x, pChar->pos.y ) );

	pChar->applyDefinition( *DefSection );
	pChar->resend( false );

	return pChar;
}

P_CHAR cCharStuff::createScriptNpc( int s, P_ITEM pi_i, QString Section, int posx, int posy, signed char posz )
{
	if( Section.length() == 0 )
		return NULL;

	const QDomElement* DefSection = DefManager->getSection( WPDT_NPC, Section );

	if( DefSection->isNull() )
	{
		clConsole.log( QString( "Unable to create unscripted npc: %1\n" ).arg( Section ).latin1() );
		return NULL;
	}

	int k=0, xos=0, yos=0, lb;
	short postype;				// determines how xyz of the new NPC are set, see below
	unsigned long loopexit=0;

	P_CHAR nChar = new cChar;
	nChar->Init( true );
	CharsManager::instance()->registerChar( nChar );

	nChar->setPriv( 0x10 );
	nChar->setNpc(1);
	nChar->setLoDamage(1);
	nChar->setHiDamage(1);
	nChar->setDef(1);
	nChar->setSpawnSerial( INVALID_SERIAL );

	if (posx > 0 && posy > 0)
 		postype = 3;	// take position from parms
	else if ( s != -1 && pi_i == NULL)
		postype = 2;	// take position from socket's buffer
	else if ( s == -1 && pi_i != NULL)
		postype = 1;	// take position from items[i]
	else
	{
		clConsole.send("ERROR: bad parms in call to createScriptNpc\n");
		return NULL;
	}

	// Now that we have created the NPC, lets place him
	switch (postype)
	{
	case 1:					// take position from (spawning) item
/*		if (triggerx)
		{
			nChar->pos.x=triggerx;
			nChar->pos.y=triggery;
			nChar->pos.z=triggerz;
			triggerx = nChar->serial;
		} else
		{
	   Zippy's Code chages for area spawns --> (Type 69) xos and yos (X OffSet, Y OffSet) 
	   are used to find a random number that is then added to the spawner's x and y (Using 
	   the spawner's z) and then place the NPC anywhere in a square around the spawner. 
	   This square is random anywhere from -10 to +10 from the spawner's location (for x and 
	   y) If the place chosen is not a valid position (the NPC can't walk there) then a new 
	   place will be chosen, if a valid place cannot be found in a certain # of tries (50), 
		   the NPC will be placed directly on the spawner and the server op will be warned. 
			if ((pi_i->type() == 69 || pi_i->type() == 125)&& pi_i->isInWorld())
			{
				if (pi_i->more3==0) pi_i->more3=10;
				if (pi_i->more4==0) pi_i->more4=10;
			   //signed char z, ztemp, found;
	   
				k=0;
				do
				{
					if (k>=50) //this CAN be a bit laggy. adjust as nessicary
					{
						clConsole.send("WOLFPACK: Problem area spawner found at [%i,%i,%i]. NPC placed at default location.\n",pi_i->pos.x,pi_i->pos.y,pi_i->pos.z);
						xos=0;
						yos=0;
						break;
					}
					xos=RandomNum(-pi_i->more3,pi_i->more3);
					yos=RandomNum(-pi_i->more4,pi_i->more4);
					// clConsole.send("Spawning at Offset %i,%i (%i,%i,%i) [-%i,%i <-> -%i,%i]. [Loop #: %i]\n",xos,yos,pi_i->x+xos,pi_i->y+yos,pi_i->z,pi_i->more3,pi_i->more3,pi_i->more4,pi_i->more4,k); 
					//lord binary, changed %s to %i, crash when uncommented !
					k++;
					if ((pi_i->pos.x+xos<1) || (pi_i->pos.y+yos<1)) lb=0; 
					//lord binary, fixes crash when calling npcvalid with negative coordiantes
					else lb = Movement->validNPCMove(pi_i->pos.x+xos,pi_i->pos.y+yos,pi_i->pos.z, nChar);				 
				   
					//Bug fix Monsters spawning on water:
					MapStaticIterator msi(pi_i->pos + Coord_cl(xos, yos, 0));

					staticrecord *stat;
					loopexit=0;
					while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
					{
						tile_st tile;
						msi.GetTile(&tile);
						if(!(strcmp((char *) tile.name, "water")))//Water
						{//Don't spawn on water tiles... Just add other stuff here you don't want spawned on.
							lb=0;
						}
					}
				} while (!lb);
			} // end Zippy's changes (exept for all the +xos and +yos around here....)

			nChar->pos.x=pi_i->pos.x+xos;
			nChar->pos.y=pi_i->pos.y+yos;
			nChar->pos.z=pi_i->pos.z;
			nChar->SetSpawnSerial(pi_i->serial);
			if ( pi_i->type() == 125 )
			{
				MsgBoardQuestEscortCreate( nChar );
			}
		} // end of if !triggerx*/
		break;
	case 2: // take position from Socket
		if (s!=-1)
		{
			/*
			nChar->pos.x=(buffer[s][11]<<8)+buffer[s][12];
			nChar->pos.y=(buffer[s][13]<<8)+buffer[s][14];
			nChar->pos.z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
			*/
		}
		break;
	case 3: // take position from Parms
		nChar->MoveTo( posx, posy, posz );
		break;
	} // no default coz we tested on entry to function
   
	nChar->setRegion( cAllTerritories::getInstance()->region( nChar->pos.x, nChar->pos.y ) );

	nChar->applyDefinition( *DefSection );
	nChar->resend( false );

	return nChar;
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
	cTerritory* Region = cAllTerritories::getInstance()->region( this->pos.x, this->pos.y );
	if( Region )
		return Region->isGuarded();
	else
		return false;
}

