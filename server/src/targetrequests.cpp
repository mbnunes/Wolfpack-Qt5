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

// Wolfpack Includes
#include "targetrequests.h"
#include "maps.h"
#include "sectors.h"
#include "wpdefmanager.h"
#include "territories.h"
#include "items.h"
//#include "tilecache.h"
#include "srvparams.h"
#include "skills.h"
#include "combat.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "house.h"
#include "boats.h"
#include "accounts.h"
#include "makemenus.h"
#include "itemid.h"
#include "npc.h"
#include "chars.h"


// System Includes
#include <math.h>

bool cAddItemTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;

	//QStringList arguments = QStringList::split( " ", npc_ );
	const cElement *node = DefManager->getDefinition( WPDT_ITEM, item_ );

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
		pItem = cItem::createFromScript( item_ );
	else
	{
		pItem = new cItem;
		pItem->Init();

		pItem->setName( "an item" );
		pItem->setId( hex2dec( item_ ).toULong() );
	}

	if( !pItem )
		return true;

	Coord_cl newPos = socket->player()->pos();
	newPos.x = target->x();
	newPos.y = target->y();
	newPos.z = target->z() + TileCache::instance()->tileHeight( target->model() ); // Model Could be an NPC as well i dont like the idea...
	pItem->moveTo( newPos );

	if( nodecay )
	{
		pItem->setDecayTime( 0 );
		pItem->setWipe( false );
	}

	// Send the item to its surroundings
	pItem->update();
	return true;
}

bool cAddNpcTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;

	//QStringList arguments = QStringList::split( " ", npc_ );
	const cElement *node = DefManager->getDefinition( WPDT_NPC, npc_ );

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
	P_NPC pChar = new cNPC;
	pChar->Init();

	Coord_cl newPos = socket->player()->pos();
	newPos.x = target->x();
	newPos.y = target->y();
	newPos.z = target->z() + TileCache::instance()->tileHeight( target->model() ); // Model Could be a NPC as well i dont like the idea...
	pChar->moveTo( newPos );

	pChar->setRegion( AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map ) );
	if( node )
	{
		pChar->applyDefinition( node );
	}
	else
	{
		pChar->setName("Character");
		pChar->setBodyID( hex2dec( npc_ ).toULong() );
		pChar->setOrgBodyID( pChar->bodyID() );
	}

	// Send the char to it's surroundings
	setcharflag( pChar );
	pChar->resend( false ); // It's new so no need to remove it first

	return true;
};

bool cBuildMultiTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
	{
		socket->sysMessage( "Invalid target." );
		return true;
	}
	
	const cElement* DefSection = DefManager->getDefinition( WPDT_MULTI, multisection_ );
	
	if( !DefSection )
	{
		socket->sysMessage( "Invalid multisection." );
		return true;
	}

	if( DefSection->getAttribute( "type" ) == "house" )
	{
		cHouse* pHouse = new cHouse();
		
		pHouse->build( DefSection, target->x(), target->y(), target->z(), senderserial_, deedserial_ );
	}
	else if( DefSection->getAttribute( "type" ) == "boat" )
	{
		cBoat* pBoat = new cBoat();

		pBoat->build( DefSection, target->x(), target->y(), target->z(), senderserial_, deedserial_ );
	}
	return true;
};

bool cSkStealing::responsed( cUOSocket *socket, cUORxTarget *target )
{
	int i, skill;
	char temp2[512];
	tile_st tile;
	P_PLAYER pc_currchar = socket->player();
	int cansteal = QMAX( 1, pc_currchar->skillValue( STEALING ) / 10 );
	cansteal = cansteal * 10;
	
	if( isCharSerial( target->serial() ) )
	{
		Skills->RandomSteal(socket, target->serial());
		return true;
	}
	
	const P_ITEM pi = FindItemBySerial(target->serial());
	if (!pi)
	{
		socket->sysMessage( tr("You cannot steal that.") );
		return true;
	}
	
	if ( pi->layer() != 0		// no stealing for items on layers other than 0 (equipped!) ,
		|| pi->newbie()			// newbie items,
		|| pi->isInWorld() )	// and items not being in containers allowed !
	{
		socket->sysMessage( tr("You cannot steal that.") );
		return true;
	}
	if( (pi->totalweight()/10) > cansteal ) // LB, bugfix, (no weight check)
	{
		socket->sysMessage( tr("That is too heavy.") );
		return true;
	}
	
	P_CHAR pc_npc = pi->getOutmostChar();
	
/*	if (pc_npc->npcaitype() == 17)
	{
		socket->sysMessage( tr("You cannot steal that.") );
		return true;
	}*/
	
	if (pc_npc == pc_currchar)
	{
		socket->sysMessage( tr("You catch yourself red handed.") );
		return true;
	}
	
	skill = pc_currchar->checkSkill(STEALING, 0, 999);
	if( pc_currchar->inRange( pc_npc, 1 ) )
	{
		if (skill)
		{
			P_ITEM pi_pack = pc_currchar->getBackpack();
			if (pi_pack == NULL) 
				return true;
			pi_pack->addItem(pi);
			socket->sysMessage( tr("You successfully steal that item.") );
			pi->update();
		} 
		else 
			socket->sysMessage( tr("You failed to steal that item.") );
		
		if (((!(skill))&&(rand()%16==7)) || (pc_currchar->skillValue(STEALING)<rand()%1001))
		{
			socket->sysMessage( tr("You have been cought!") );
			
			if (pc_npc != NULL) //lb
			{
				if (pc_npc->objectType() == enNPC) 
					pc_npc->talk( tr("Guards!! A thief is amoung us!"), -1, 0x09 );
				
				pc_currchar->isCriminal();
				
				if (pc_npc->isInnocent() && pc_currchar->attackerSerial() != pc_npc->serial() /*&& GuildCompare(pc_currchar, pc_npc)==0*/ )//AntiChrist
					pc_currchar->isCriminal();//Blue and not attacker and not guild
				
				if( !pi->name().isNull() )
				{
					sprintf((char*)temp, tr("You notice %1 trying to steal %2 from you!").arg(pc_currchar->name()).arg(pi->name()) );
					sprintf((char*)temp2, tr("You notice %1 trying to steal %2 from %3!").arg(pc_currchar->name()).arg(pi->name()).arg(pc_npc->name()) );
				} 
				else
				{
					tile = TileCache::instance()->getTile( pi->id() );
					sprintf((char*)temp, tr("You notice %1 trying to steal %2 from you!").arg(pc_currchar->name()).arg((char*)tile.name) );
					sprintf((char*)temp2,tr("You notice %1 trying to steal %2 from %3!").arg(pc_currchar->name()).arg((char*)tile.name).arg(pc_npc->name()) );
				}
				socket->sysMessage((char*)temp); //lb
			}
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
			{
				if( mSock != socket && mSock->player() && mSock->player()->inRange( pc_currchar, mSock->viewRange() ) && (rand()%10+10==17|| (rand()%2==1 && mSock->player()->intelligence() >= pc_currchar->intelligence()))) 
					mSock->sysMessage(temp2);
			}
		}
	} 
	else 
		socket->sysMessage( tr("You are too far away to steal that item.") );
	return true;
}

bool cSkRepairItem::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket )
		return true;

	P_CHAR pc = socket->player();
	if( !pc )
		return true;

	P_ITEM pi = FindItemBySerial( target->serial() );
	if( !pi || pi->isLockedDown() )
	{
		socket->sysMessage( tr("You can't repair that!") );
		return false;
	}
	else if( pc->getBackpack() && !pc->getBackpack()->contains(pi) )
	{
		socket->sysMessage( tr("The item must be in your backpack to get repaired!") );
		return true;
	}
	else if( pi->hp() == 0 )
	{
		socket->sysMessage( tr("That item can't be repaired.") );
		return true;
	}
	else if( pi->hp() >= pi->maxhp() )
	{
		socket->sysMessage( tr("That item is of top quality.") );
		return true;
	}
	else
	{
		bool anvilinrange = false;
		RegionIterator4Items ri( pc->pos(), 2 );
		for( ri.Begin(); !ri.atEnd(); ri++ )
		{
			P_ITEM pri = ri.GetData();
			if( pri && IsAnvil( pri->id() ) )
			{
				anvilinrange = true;
				break;
			}
		}
		if( !anvilinrange )
		{
			socket->sysMessage( tr("You must stand in range of an anvil!" ) );
			return true;
		}
	}

	bool hasSuccess = true;
	short dmg=4;	// damage to maxhp

	short smithing = pc->skillValue( BLACKSMITHING );
	if		((smithing>=900)) dmg=1;
	else if ((smithing>=700)) dmg=2;
	else if ((smithing>=500)) dmg=3;
	hasSuccess = pc->checkSkill(BLACKSMITHING, 0, 1000);

	if( hasSuccess )
	{
		pi->setMaxhp( pi->maxhp() - dmg );
		pi->setHp( pi->maxhp() );
		socket->sysMessage( tr("* the item has been repaired.*") );
		if( makesection_ && makesection_->baseAction() )
			pc->soundEffect( makesection_->baseAction()->succSound() );
	}
	else
	{
		pi->setHp( pi->hp() - 2 );
		pi->setMaxhp( pi->maxhp() - 1 );
		socket->sysMessage( tr("* You fail to repair the item. *") );
		socket->sysMessage( tr("* You weaken the item.*") );
		if( makesection_ && makesection_->baseAction() )
			pc->soundEffect( makesection_->baseAction()->failSound() );
	}

	return true;
}

bool cSetTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );
	
	cUObject *pObject = NULL;
	
	if( pItem )
		pObject = pItem;
	else if( pChar )
		pObject = pChar;
	
	// Only characters and items
	if( !pObject )
	{
		socket->sysMessage( tr( "Please select a valid character or item" ) );
		return true;
	}

	cVariant value( this->value );
	stError *error = pObject->setProperty( key, value );

 	if( error )
	{
		socket->sysMessage( error->text );
		delete error;
	}
		
	if( pChar )
		pChar->resend();
	else if( pItem )
		pItem->update();

	return true;
}

bool cRemoveTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );
	
	if( pChar )
	{
		if( pChar->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pChar);
			if( pp->socket() )
			{
				pp->socket()->sysMessage( tr("You cannot delete logged in characters") );
				return true;
			}

			if( pp->account() )
				pp->account()->removeCharacter( pp );

			cCharStuff::DeleteChar( pChar );
		}
		else
			cCharStuff::DeleteChar( pChar );
	}
	else if( pItem )
	{
		pItem->remove();
	}
	else
		socket->sysMessage( "You need to select either an item or a character" );
	return true;
}

bool cAddEventTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cUObject *pObject = 0;
	
	if( isCharSerial( target->serial() ) )
		pObject = FindCharBySerial( target->serial() );
	else if( isItemSerial( target->serial() ) )
		pObject = FindItemBySerial( target->serial() );
	
	// We have to have a valid target
	if( !pObject )
	{
		socket->sysMessage( tr( "You have to target a character or an item." ) );
		return true;
	}
	
	// Check if we already have the event
	if( pObject->hasEvent( _event ) )
	{
		socket->sysMessage( tr( "This object already has the event '%1'" ).arg( _event ) );
		return true;
	}
	
	cPythonScript *script = ScriptManager->find( _event );
	
	if( !script )
	{
		socket->sysMessage( tr( "Invalid event: '%1'" ).arg( _event ) );
		return true;
	}
	
	pObject->addEvent( script );
	return true;
}

bool cTileTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket->player() )
		return true;
	
	// Set the first corner and readd ourself
	if( x1 == -1 || y1 == -1 )
	{
		x1 = target->x();
		y1 = target->y();
		socket->sysMessage( tr( "Please select the second corner." ) );
		return false;
	}
	else
	{
		INT16 x2;
		if( target->x() < x1 )
		{
			x2 = x1;
			x1 = target->x();
		}
		else
			x2 = target->x();
		
		INT16 y2;
		if( target->y() < y1 )
		{
			y2 = y1;
			y1 = target->y();
		}
		else
			y2 = target->y();
		
		socket->sysMessage( tr( "Tiling from %1,%2 to %3,%4" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
		UINT16 dCount = 0;
		
		for( UINT16 x = x1; x <= x2; ++x )
			for( UINT16 y = y1; y <= y2; ++y )
			{
				// Select a Random Tile from the list
				QString id = ids[ RandomNum( 0, ids.count()-1 ) ];
				P_ITEM pItem = cItem::createFromScript( id );
				
				if( pItem )
				{
					Coord_cl position( x, y, z, socket->player()->pos().map );
					pItem->setPos( position );
					MapObjects::instance()->add( pItem );
					pItem->update();
					++dCount;
				}
			}
			
			socket->sysMessage( tr( "Created %1 items." ).arg( dCount ) );
			return true;
	}		
}

bool cSetMultiOwnerTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti || !socket->player() )
	{
		socket->sysMessage( tr( "An error occured! Send a bug report to the staff, please!" ) );
		return true;
	}
	
	if( target->x() == 0xFFFF && target->y() == 0xFFFF && target->z() == (INT8)0xFF )
		return true;
	
	P_PLAYER pc = dynamic_cast<P_PLAYER>(FindCharBySerial( target->serial() ));
	if( !pc )
	{
		socket->sysMessage( tr( "This is not a valid target!" ) );
		return false;
	}
	
	if( coowner_ )
	{
		pMulti->setCoOwner( pc );
		socket->sysMessage( tr("You have made %1 to the new co-owner of %2").arg( pc->name() ).arg( pMulti->name() ) );
		pc->socket()->sysMessage( tr("%1 has made you to the new co-owner of %2").arg( socket->player()->name() ).arg( pMulti->name() ) );
	}
	else
	{
		pMulti->setOwner( pc );
		socket->sysMessage( tr("You have made %1 to the new owner of %2").arg( pc->name() ).arg( pMulti->name() ) );
		pc->socket()->sysMessage( tr("%1 has made you to the new owner of %2").arg( socket->player()->name() ).arg( pMulti->name() ) );
	}
	return true;
}

bool cMultiAddToListTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti || !socket->player() )
	{
		socket->sysMessage( tr( "An error occured! Send a bug report to the staff, please!" ) );
		return true;
	}
	
	if( target->x() == 0xFFFF && target->y() == 0xFFFF && target->z() == (INT8)0xFF )
		return true;
	
	P_PLAYER pc = dynamic_cast<P_PLAYER>(FindCharBySerial( target->serial() ));
	if( !pc )
	{
		socket->sysMessage( tr( "This is not a valid target!" ) );
		return false;
	}
	
	if( banlist_ )
	{
		pMulti->addBan( pc );
	}
	else
	{
		pMulti->addFriend( pc );
	}
	socket->sysMessage( tr("Select another char to add to the %1!").arg( banlist_ ? tr("list of banned") : tr("list of friends") ) );
	return false;
}

bool cMultiChangeLockTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti || !socket->player() )
	{
		socket->sysMessage( tr( "An error occured! Send a bug report to the staff, please!" ) );
		return true;
	}
	
	if( target->x() == 0xFFFF && target->y() == 0xFFFF && target->z() == (INT8)0xFF )
		return true;
	
	P_ITEM pi = FindItemBySerial( target->serial() );
	if( !pi )
	{
		socket->sysMessage( tr( "This is not a valid target!" ) );
		return false;
	}
	
	if( pi->multis() == pMulti->serial() && pi->type() != 117 )
	{
		pi->setMagic((pi->magic() == 4 && pi->type() != 222) ? 0 : 4);
		pi->update();
	}
	socket->sysMessage( tr("Select another item to lock/unlock!") );
	return false;
}

bool cDyeTubDyeTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket->player() )
		return true;
	
	P_ITEM pItem = FindItemBySerial( target->serial() );
	
	if( !pItem )
		return true;
	
	if( pItem->getOutmostChar() != socket->player() && !( pItem->isInWorld() && pItem->inRange( socket->player(), 4 ) ) )
	{
		socket->sysMessage( tr( "You can't reach this object to dye it." ) );
		return true;
	}
	
	if( pItem->type() != 406 )
	{
		socket->sysMessage( tr( "You can only dye dye tubs with this." ) );
		return true;
	}
	
	pItem->setColor( _color );
	pItem->update();
	
	return true;
}

bool cShowTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	// Check for a valid target
	cUObject *pObject;
	P_ITEM pItem = FindItemBySerial( target->serial() );
	P_CHAR pChar = FindCharBySerial( target->serial() );
	if( !pChar && !pItem )
	{
		socket->sysMessage( tr( "You have to target a valid object." ) );
		return true;
	}

	if( pChar )
		pObject = pChar;
	else
		pObject = pItem;


	cVariant result;
	stError *error = pObject->getProperty( key, result );

	if( error )
	{
		socket->sysMessage( error->text );
		delete error;
		return true;
	}

	socket->sysMessage( tr( "'%1' is '%2'" ).arg( key ).arg( result.toString() ) );
	return true;
}
