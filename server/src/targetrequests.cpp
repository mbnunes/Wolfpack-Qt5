/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

// Wolfpack Includes
#include "targetrequests.h"
#include "muls/maps.h"
#include "sectors.h"
#include "definitions.h"
#include "territories.h"
#include "items.h"
//#include "muls/tilecache.h"
#include "config.h"
#include "skills.h"
#include "combat.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "accounts.h"
#include "npc.h"
#include "basics.h"
#include "network/network.h"

// System Includes
#include <math.h>

bool cSkStealing::responsed( cUOSocket* socket, cUORxTarget* target )
{
	int i, skill;
	char temp2[512];
	char temp[512];
	tile_st tile;
	P_PLAYER pc_currchar = socket->player();
	int cansteal = QMAX( 1, pc_currchar->skillValue( STEALING ) / 10 );
	cansteal = cansteal * 10;

	if ( isCharSerial( target->serial() ) )
	{
		Skills::instance()->RandomSteal( socket, target->serial() );
		return true;
	}

	const P_ITEM pi = FindItemBySerial( target->serial() );
	if ( !pi )
	{
		socket->sysMessage( tr( "You cannot steal that." ) );
		return true;
	}

	if ( pi->layer() != 0		// no stealing for items on layers other than 0 (equipped!) ,
		|| pi->newbie()			// newbie items,
		|| pi->isInWorld() )	// and items not being in containers allowed !
	{
		socket->sysMessage( tr( "You cannot steal that." ) );
		return true;
	}
	if ( ( pi->totalweight() ) > cansteal ) // LB, bugfix, (no weight check)
	{
		socket->sysMessage( tr( "That is too heavy." ) );
		return true;
	}

	P_CHAR pc_npc = pi->getOutmostChar();

	/*	if (pc_npc->npcaitype() == 17)
		{
			socket->sysMessage( tr("You cannot steal that.") );
			return true;
		}*/

	if ( pc_npc == pc_currchar )
	{
		socket->sysMessage( tr( "You catch yourself red handed." ) );
		return true;
	}

	skill = pc_currchar->checkSkill( STEALING, 0, 999 );
	if ( pc_currchar->inRange( pc_npc, 1 ) )
	{
		if ( skill )
		{
			P_ITEM pi_pack = pc_currchar->getBackpack();
			if ( pi_pack == NULL )
				return true;
			pi_pack->addItem( pi );
			socket->sysMessage( tr( "You successfully steal that item." ) );
			pi->update();
		}
		else
			socket->sysMessage( tr( "You failed to steal that item." ) );

		if ( ( ( !( skill ) ) && ( rand() % 16 == 7 ) ) || ( pc_currchar->skillValue( STEALING ) < rand() % 1001 ) )
		{
			socket->sysMessage( tr( "You have been cought!" ) );

			if ( pc_npc != NULL ) //lb
			{
				if ( pc_npc->objectType() == enNPC )
					pc_npc->talk( tr( "Guards!! A thief is amoung us!" ), -1, 0x09 );

				pc_currchar->isCriminal();

				if ( pc_npc->notoriety( pc_currchar ) == 0x01 )
				{
					pc_currchar->makeCriminal();
				}

				if ( !pi->name().isNull() )
				{
					sprintf( ( char * ) temp, tr( "You notice %1 trying to steal %2 from you!" ).arg( pc_currchar->name() ).arg( pi->name() ) );
					sprintf( ( char * ) temp2, tr( "You notice %1 trying to steal %2 from %3!" ).arg( pc_currchar->name() ).arg( pi->name() ).arg( pc_npc->name() ) );
				}
				else
				{
					tile = TileCache::instance()->getTile( pi->id() );
					sprintf( ( char * ) temp, tr( "You notice %1 trying to steal %2 from you!" ).arg( pc_currchar->name() ).arg( ( char * ) tile.name ) );
					sprintf( ( char * ) temp2, tr( "You notice %1 trying to steal %2 from %3!" ).arg( pc_currchar->name() ).arg( ( char * ) tile.name ).arg( pc_npc->name() ) );
				}
				socket->sysMessage( ( char * ) temp ); //lb
			}
			for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
			{
				if ( mSock != socket && mSock->player() && mSock->player()->inRange( pc_currchar, mSock->viewRange() ) && ( rand() % 10 + 10 == 17 || ( rand() % 2 == 1 && mSock->player()->intelligence() >= pc_currchar->intelligence() ) ) )
					mSock->sysMessage( temp2 );
			}
		}
	}
	else
		socket->sysMessage( tr( "You are too far away to steal that item." ) );
	return true;
}

bool cSetTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );

	cUObject* pObject = NULL;

	if ( pItem )
		pObject = pItem;
	else if ( pChar )
		pObject = pChar;

	// Only characters and items
	if ( !pObject )
	{
		socket->sysMessage( tr( "Please select a valid character or item" ) );
		return true;
	}

	// check for rank
	if ( pChar && pChar->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>( pChar );
		if ( pp->account()->rank() >= socket->player()->account()->rank() && pp != socket->player() )
		{
			socket->sysMessage( tr( "Better do not try that!" ) );
			return true;
		}
	}

	cVariant value( this->value );
	stError* error = pObject->setProperty( key, value );

	if ( error )
	{
		socket->sysMessage( error->text );
		delete error;
	}

	if ( pChar )
		pChar->resend();
	else if ( pItem )
		pItem->update();

	return true;
}

bool cRemoveTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );

	if ( pChar )
	{
		pChar->remove();
	}
	else if ( pItem )
	{
		pItem->remove();
	}
	else
		socket->sysMessage( "You need to select either an item or a character" );
	return true;
}

bool cAddEventTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	cUObject* pObject = 0;

	if ( isCharSerial( target->serial() ) )
		pObject = FindCharBySerial( target->serial() );
	else if ( isItemSerial( target->serial() ) )
		pObject = FindItemBySerial( target->serial() );

	// We have to have a valid target
	if ( !pObject )
	{
		socket->sysMessage( tr( "You have to target a character or an item." ) );
		return true;
	}

	// Check if we already have the event
	if ( pObject->hasEvent( _event ) )
	{
		socket->sysMessage( tr( "This object already has the event '%1'" ).arg( _event ) );
		return true;
	}

	cPythonScript* script = ScriptManager::instance()->find( _event.latin1() );

	if ( !script )
	{
		socket->sysMessage( tr( "Invalid event: '%1'" ).arg( _event ) );
		return true;
	}

	pObject->addEvent( script );
	return true;
}

bool cShowTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	// Check for a valid target
	cUObject* pObject;
	P_ITEM pItem = FindItemBySerial( target->serial() );
	P_CHAR pChar = FindCharBySerial( target->serial() );
	if ( !pChar && !pItem )
	{
		socket->sysMessage( tr( "You have to target a valid object." ) );
		return true;
	}

	if ( pChar )
		pObject = pChar;
	else
		pObject = pItem;


	cVariant result;
	stError* error = pObject->getProperty( key, result );

	if ( error )
	{
		socket->sysMessage( error->text );
		delete error;
		return true;
	}

	socket->sysMessage( tr( "'%1' is '%2'" ).arg( key ).arg( result.toString() ) );
	return true;
}
