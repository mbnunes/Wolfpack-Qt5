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

#include "python/tempeffect.h"

#include "platform.h"

// Wolfpack Includes
#include "TmpEff.h"
#include "items.h"
#include "globals.h"
#include "srvparams.h"
#include "network.h"
#include "wpdefmanager.h"
#include "network/uosocket.h"
#include "dbdriver.h"
#include "persistentbroker.h"
#include "skills.h"
#include "sectors.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "ai.h"
#include "basics.h"
#include "world.h"
#include "inlines.h"

#include <algorithm>
#include <typeinfo>
#include <math.h>

int cTempEffect::getDest()
{
	return destSer;
}

void cTempEffect::setDest(int ser)
{
	destSer=ser;
}

int cTempEffect::getSour()
{
	return sourSer;
}

void cTempEffect::setSour(int ser)
{
	sourSer=ser;
}

void cTempEffect::setExpiretime_s(int seconds)
{
	expiretime=uiCurrentTime+(seconds*MY_CLOCKS_PER_SEC);
}

void cTempEffect::setExpiretime_ms(float milliseconds)
{
	expiretime=uiCurrentTime+(int)floor(( milliseconds / 1000 )*MY_CLOCKS_PER_SEC);
}

/*
	Save a float to the effects_properties table.
 */
void cTempEffect::saveFloat( unsigned int id, QString key, double value )
{
	persistentBroker->executeQuery( QString( "INSERT INTO effects_properties VALUES(%1,'%2','%3','%4');" ).arg( id ).arg( persistentBroker->quoteString( key ) ).arg( "float" ).arg( value ) );
}

/*
	Save an integer to the effects_properties table.
 */
void cTempEffect::saveInt( unsigned int id, QString key, int value )
{
	persistentBroker->executeQuery( QString( "INSERT INTO effects_properties VALUES(%1,'%2','%3','%4');" ).arg( id ).arg( persistentBroker->quoteString( key ) ).arg( "int" ).arg( value ) );
}

/*
	Save a string to the effects_properties table.
 */
void cTempEffect::saveString( unsigned int id, QString key, const QString &value )
{
	persistentBroker->executeQuery( QString( "INSERT INTO effects_properties VALUES(%1,'%2','%3','%4');" ).arg( id ).arg( persistentBroker->quoteString( key ) ).arg( "string" ).arg( persistentBroker->quoteString( value.utf8() ) ) );
}

/*
	Load a float from the effects_properties table.
 */
bool cTempEffect::loadFloat( unsigned int id, QString key, double &value )
{
	cDBResult result = persistentBroker->query( QString( "SELECT value FROM effects_properties WHERE id = '%1' AND key = '%2' AND type = 'float'" ).arg( id ).arg( persistentBroker->quoteString( key ) ) );

	if( !result.fetchrow() )
	{
		result.free();
		return false;
	}

	value = result.getString( 0 ).toFloat();

	result.free();

	return true;
}

/*
	Load an integer from the effects_properties table.
 */
bool cTempEffect::loadInt( unsigned int id, QString key, int &value )
{
	cDBResult result = persistentBroker->query( QString( "SELECT value FROM effects_properties WHERE id = '%1' AND key = '%2' AND type = 'int'" ).arg( id ).arg( persistentBroker->quoteString( key ) ) );

	if( !result.fetchrow() )
	{
		result.free();
		return false;
	}

	value = result.getString( 0 ).toInt();

	result.free();

	return true;
}

/*
	Load a string from the effects_properties table.
 */
bool cTempEffect::loadString( unsigned int id, QString key, QString &value )
{
	cDBResult result = persistentBroker->query( QString( "SELECT value FROM effects_properties WHERE id = '%1' AND key = '%2' AND type = 'string'" ).arg( id ).arg( persistentBroker->quoteString( key ) ) );

	if( !result.fetchrow() )
	{
		result.free();
		return false;
	}

	value = result.getString( 0 );

	result.free();

	return true;
}

#include "console.h"

void cTempEffect::save( unsigned int id )
{
	persistentBroker->executeQuery( QString( "INSERT INTO effects VALUES(%1,'%2',%3,%4,%5,%6);" ).arg( id ).arg( persistentBroker->quoteString( objectID() ) ).arg( expiretime - uiCurrentTime ).arg( dispellable ? 1 : 0 ).arg( sourSer ).arg( destSer ) );
}

void cTempEffect::load( unsigned int id, const char **result )
{
	unsigned int offset = 2;

	expiretime = atol( result[offset++] ) + uiCurrentTime;
	dispellable = atol( result[offset++] ) == 0 ? false : true;
	sourSer = atol( result[offset++] );
	destSer = atol( result[offset++] );

	serializable = true;
}

void cTempEffects::check()
{
	cTempEffect *tEffect = NULL;
	if( !teffects.empty() )
		tEffect = *teffects.begin();

	if( !tEffect)
		return;
	
	while( tEffect && tEffect->expiretime <= uiCurrentTime )
	{
		if( isCharSerial( tEffect->getDest() ) )
		{
			P_CHAR pChar = dynamic_cast< P_CHAR >( FindCharBySerial( tEffect->getDest() ) );
			if( pChar )
				pChar->removeEffect( tEffect );
		}

		tEffect->Expire();
		std::pop_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
		teffects.pop_back();
		delete tEffect;

		if( !teffects.empty() )
			tEffect = *teffects.begin();
		else
			break;
	}
}

/*!
	Dispels all TempEffects lasting on pc_dest of a given type.
	If only Dispellable is false then all effects on this character
	of the specified type are reverted.
*/
void cTempEffects::dispel( P_CHAR pc_dest, P_CHAR pSource, const QString &type, bool silent, bool onlyDispellable )
{
	std::vector< cTempEffect* >::iterator i = teffects.begin();
	for( i = teffects.begin(); i != teffects.end(); i++ )
		if( (*i) != NULL && ( !onlyDispellable || (*i)->dispellable ) && (*i)->getDest() == pc_dest->serial() && (*i)->objectID() == type )
		{
			if( isCharSerial( (*i)->getSour() ) )
			{
				P_CHAR pChar = FindCharBySerial( (*i)->getSour() );
				if( pChar )
					pChar->removeEffect( (*i) );
			}

			(*i)->Dispel( pc_dest, pSource );
			teffects.erase( i );
		}

		std::make_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
}

void cTempEffects::dispel( P_CHAR pc_dest, P_CHAR pSource, bool silent )
{
	std::vector< cTempEffect* >::iterator i = teffects.begin();
	for( i = teffects.begin(); i != teffects.end(); i++ )
		if( (*i) != NULL && (*i)->dispellable && (*i)->getDest() == pc_dest->serial() )
		{
			if( isCharSerial( (*i)->getDest() ) )
			{
				P_CHAR pChar = FindCharBySerial( (*i)->getDest() );
				if( pChar )
					pChar->removeEffect( (*i) );
			}

			(*i)->Dispel( pSource, silent );
			teffects.erase( i );
		}

		std::make_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
}

void cTempEffects::load()
{
	// Query the Database

	cDBResult result = persistentBroker->query( "SELECT id,objectid,expiretime,dispellable,source,destination FROM effects ORDER BY expiretime ASC;" );

	while( result.fetchrow() )
	{
		unsigned int id = result.getInt( 0 );
		QString objectId = result.getString( 1 );

		cTempEffect *effect = 0;

		if( objectId == "cPythonEffect" )
		{
			effect = new cPythonEffect;
		}
		else if( objectId == "cDelayedHideChar" ) 
		{
			effect = new cDelayedHideChar;
		}
		else
		{
			throw QString( "Unknown TempEffect Type: %1" ).arg( objectId );
		}

		const char **res = (const char**)result.data(); // Skip id, objectid
		effect->load( id, res );

		insert( effect );
	}
}

void cTempEffects::save()
{
	persistentBroker->executeQuery( "DELETE FROM effects;" );
	persistentBroker->executeQuery( "DELETE FROM effects_properties;" );

	std::vector< cTempEffect* >::iterator it = teffects.begin();
	unsigned int id = 0;

	while( it != teffects.end() )
	{
		if( (*it)->isSerializable() )
			(*it)->save( id++ );

		++it;
	}
}

// cDelayedHideChar
cDelayedHideChar::cDelayedHideChar( SERIAL serial )
{
	if( !isCharSerial( serial ) || !FindCharBySerial( serial ) )
	{
		destSer = INVALID_SERIAL;
		return;
	}
	
	destSer = serial;
	setSerializable( true );
}

cDelayedHideChar::cDelayedHideChar()
{
	setSerializable( true );
}

void cDelayedHideChar::Expire()
{
	P_PLAYER pc = dynamic_cast<P_PLAYER>(FindCharBySerial( destSer ));
	if( !pc || pc->socket() ) // break if the char has relogged in the meantime
		return;

	pc->setHidden( 1 );
	pc->resend( true );
}

cRepeatAction::cRepeatAction( P_CHAR mage, UINT8 anim, UINT32 delay )
{
	_mage = mage->serial();
	_anim = anim;
	_delay = delay;
	mage->action( anim );
	expiretime = uiCurrentTime + delay;
	dispellable = false;
	serializable = false;
}

void cRepeatAction::Expire()
{
	P_CHAR pMage = FindCharBySerial( _mage );

	if( pMage )
		TempEffects::instance()->insert( new cRepeatAction( pMage, _anim, _delay ) );
}

void cTempEffects::insert( cTempEffect *pT )
{
	// If the tempeffect has a char it affects, 
	// then don't forge to add it to his effects
	if( isCharSerial( pT->getDest() ) )
	{
		P_CHAR pChar = FindCharBySerial( pT->getDest() );
		if( pChar )
			pChar->addEffect( pT );
	}

	this->teffects.push_back( pT );
	std::push_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
}

void cTempEffects::erase( cTempEffect *pT )
{
	if( pT == (*teffects.begin()) )
	{
		std::pop_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
		teffects.pop_back();
	}
	else
	{
		std::vector< cTempEffect* >::iterator it = std::find( teffects.begin(), teffects.end(), pT );
		if( it != teffects.end() )
		{
			teffects.erase( it );
			std::make_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
		}
	}
}

void cDelayedHeal::Expire()
{
	P_CHAR pSource = FindCharBySerial( destSer );
	P_CHAR pTarget = FindCharBySerial( sourSer );

	if( !pSource || !pTarget )
		return;

	if( !pSource->inRange( pTarget, 5 ) )
	{
		if( pSource->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pSource);
			if( pp->socket() )
				pp->socket()->sysMessage( tr( "You are standing too far away to apply any bandages." ) );
		}
		return;
	}
}

cDelayedHeal::cDelayedHeal( P_CHAR pSource, P_CHAR pTarget, UINT16 _amount )
{
	// Switching them here is important because we want to 
	// keep track of our current healing targets
	destSer = pSource->serial();
	sourSer = pTarget->serial();
	amount = _amount;
	objectid = "cDelayedHeal";
	serializable = false;
	dispellable = false;
}

cStablemasterRefreshTimer::cStablemasterRefreshTimer( P_NPC pNPC, AbstractAI* interface_, UINT32 time )
{
	m_npc = pNPC;
	m_interface = interface_;
	objectid = "cStablemasterRefreshTimer";
	serializable = false;
	dispellable = false;
	expiretime = uiCurrentTime + time * MY_CLOCKS_PER_SEC;
}

void cStablemasterRefreshTimer::Expire()
{
	// lets check if the npc exists, and if the
	// npc ai on the npc is the same like the one which set the timer
	if( m_npc )
	{
		AbstractAI* ai = m_npc->ai();
		if( ai && ai == m_interface )
		{
			Human_Stablemaster* pAI = dynamic_cast< Human_Stablemaster* >(ai);
			pAI->refreshStock();
		}
	}
}
