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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

// Platform Includes
#include "platform.h"

#include "timing.h"
#include "profile.h"

#include "basics.h"
#include "timers.h"
#include "combat.h"
#include "mapobjects.h"
#include "serverconfig.h"
#include "network/network.h"
#include "spawnregions.h"
#include "territories.h"
#include "skills.h"
#include "typedefs.h"
#include "items.h"
#include "basechar.h"
#include "npc.h"
#include "player.h"
#include "ai/ai.h"
#include "inlines.h"
#include "walking.h"
#include "world.h"
#include "uotime.h"

// Library Includes
#include <qdatetime.h>
#include <math.h>
#include <time.h>

cTiming::cTiming()
{
	unsigned int time = getNormalizedTime();

	lastWorldsave_ = 0;
	nextSpawnRegionCheck = time + Config::instance()->spawnRegionCheckTime() * MY_CLOCKS_PER_SEC;
	nextTamedCheck = ( uint )( time + Config::instance()->checkTamedTime() * MY_CLOCKS_PER_SEC );
	nextNpcCheck = ( uint )( time + Config::instance()->checkNPCTime() * MY_CLOCKS_PER_SEC );
	nextItemCheck = time + 10000; // Every 10 seconds
	nextShopRestock = time + 20 * 60 * MY_CLOCKS_PER_SEC; // Every 20 minutes
	nextHungerCheck = time + Config::instance()->hungerDamageRate();
	nextCombatCheck = time + 100; // Every 100 ms
	nextUOTimeTick = 0;
}

void cTiming::poll()
{
	unsigned int time = getNormalizedTime();

	// Check for spawn regions
	if ( nextSpawnRegionCheck <= time )
	{
		startProfiling(PF_SPAWNCHECK);
		SpawnRegions::instance()->check();
		nextSpawnRegionCheck = time + Config::instance()->spawnRegionCheckTime() * MY_CLOCKS_PER_SEC;
		stopProfiling(PF_SPAWNCHECK);
	}

	// Check for decay items
	if ( nextItemCheck <= time )
	{
		startProfiling(PF_DECAYCHECK);
		QValueVector<SERIAL> toRemove;
		DecayIterator it = decayitems.begin();

		while ( it != decayitems.end() )
		{
			if ( ( *it ).first <= time )
			{
				toRemove.append( ( *it ).second );
			}
			++it;
		}

		QValueVector<SERIAL>::iterator sit;
		for ( sit = toRemove.begin(); sit != toRemove.end(); ++sit )
		{
			P_ITEM item = FindItemBySerial( *sit );
			if ( item && item->isInWorld() && !item->nodecay() )
			{
				item->remove(); // Auto removes from the decaylist
			}
			else
			{
				removeDecaySerial( *sit );
			}
		}

		nextItemCheck = time + 5000;

		stopProfiling(PF_DECAYCHECK);
	}

	// Check for an automated worldsave
	if ( Config::instance()->saveInterval() )
	{
		startProfiling(PF_WORLDSAVE);

		// Calculate the next worldsave based on the last worldsave
		unsigned int nextSave = lastWorldsave() + Config::instance()->saveInterval() * MY_CLOCKS_PER_SEC;

		if ( nextSave <= time )
		{
			World::instance()->save();
		}

		stopProfiling(PF_WORLDSAVE);
	}

	unsigned int events = 0;

	// Check lightlevel and time
	if ( nextUOTimeTick <= time )
	{
		startProfiling(PF_UOTIMECHECK);

		unsigned char oldhour = UoTime::instance()->hour();
		UoTime::instance()->setMinutes( UoTime::instance()->getMinutes() + 1 );
		nextUOTimeTick = time + Config::instance()->secondsPerUOMinute() * MY_CLOCKS_PER_SEC;

		unsigned char & currentLevel = Config::instance()->worldCurrentLevel();
		unsigned char newLevel = currentLevel;

		unsigned char darklevel = Config::instance()->worldDarkLevel();
		unsigned char brightlevel = Config::instance()->worldBrightLevel();
		unsigned char difference = wpMax<unsigned char>( 0, static_cast<int>( darklevel ) - brightlevel );
		unsigned char hour = UoTime::instance()->hour();

		if ( hour != oldhour )
		{
			events |= cBaseChar::EventTime;
		}

		// 11 to 18 = Day
		if ( hour >= 11 && hour < 18 )
		{
			newLevel = brightlevel;

			// 22 to 6 = Night
		}
		else if ( hour >= 22 || hour < 6 )
		{
			newLevel = darklevel;

			// 6 to 10 = Dawn (Scaled)
		}
		else if ( hour >= 6 && hour < 11 )
		{
			double factor = ( ( hour - 6 ) * 60 + UoTime::instance()->minute() ) / 240.0;
			newLevel = darklevel - wpMin<int>( darklevel, roundInt( factor * difference ) );

			// 18 to 22 = Nightfall (Scaled)
		}
		else
		{
			double factor = ( ( hour - 18 ) * 60 + UoTime::instance()->minute() ) / 240.0;
			newLevel = brightlevel + roundInt( factor * difference );
		}

		if ( newLevel != currentLevel )
		{
			events |= cBaseChar::EventLight;
			currentLevel = newLevel;
		}

		stopProfiling(PF_UOTIMECHECK);
	}

	if ( nextCombatCheck <= time )
	{
		startProfiling(PF_COMBATCHECK);

		nextCombatCheck = time + 250;

		// Check for timed out fights
		QPtrList<cFightInfo> fights = Combat::instance()->fights();
		fights.setAutoDelete( false );
		QPtrList<cFightInfo> todelete;
		todelete.setAutoDelete( true );
		cFightInfo* info;
		for ( info = fights.first(); info; info = fights.next() )
		{
			P_CHAR attacker = info->attacker();
			P_CHAR victim = info->victim();

			// These checks indicate that the fight is over.
			if ( !victim || victim->free || victim->isDead() || !attacker || attacker->free || attacker->isDead() || ( info->lastaction() + 60000 <= time && !attacker->inRange( victim, Config::instance()->attack_distance() ) ) )
			{
				todelete.append( info );
				continue;
			}

			attacker->poll( time, cBaseChar::EventCombat );

			// Maybe the victim got deleted already.
			if ( !victim->free )
			{
				victim->poll( time, cBaseChar::EventCombat );
			}
		}

		stopProfiling(PF_COMBATCHECK);
	}

	// Save the positions of connected players
	QValueVector<Coord> positions;

	// Periodic checks for connected players
	for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
	{
		if ( !socket->player() )
		{
			continue;
		}

		startProfiling(PF_PLAYERCHECK);
		socket->player()->poll( time, events );
		stopProfiling(PF_PLAYERCHECK);

		checkRegeneration( socket->player(), time );
		checkPlayer( socket->player(), time );
		positions.append( socket->player()->pos() );
	}

	// Check all other characters
	if ( nextNpcCheck <= time )
	{
		cCharIterator chariter;
		for( P_CHAR character = chariter.first(); character; character = chariter.next() )
		{
			P_NPC npc = dynamic_cast<P_NPC>( character );
			if( npc && npc->stablemasterSerial() == INVALID_SERIAL )
			{
				// Check if we are anywhere near a player
				// all other npcs are accounted as inactive
				for ( QValueVector<Coord>::const_iterator it = positions.begin(); it != positions.end(); ++it )
				{
					if ( ( *it ).distance( npc->pos() ) <= 24 )
					{
						checkRegeneration( npc, time );
						checkNpc( npc, time );
						startProfiling(PF_NPCCHECK);
						npc->poll( time, events );
						stopProfiling(PF_NPCCHECK);
						break;
					}
				}
				continue;
			}

			P_PLAYER player = dynamic_cast<P_PLAYER>( character );
			if( player && player->logoutTime() && player->logoutTime() <= time )
			{
				player->onLogout();
				player->removeFromView( false );
				player->setSocket( 0 );
				player->setLogoutTime( 0 );
				player->resend( false );
			}
		}

		if ( nextTamedCheck <= time )
			nextTamedCheck = ( uint )( time + Config::instance()->checkTamedTime() * MY_CLOCKS_PER_SEC );

		if ( nextNpcCheck <= time )
			nextNpcCheck = ( uint )( time + Config::instance()->checkNPCTime() * MY_CLOCKS_PER_SEC );
	}

	// Check the Timers
	startProfiling(PF_TIMERSCHECK);
	Timers::instance()->check();
	stopProfiling(PF_TIMERSCHECK);

	if ( nextHungerCheck <= time )
		nextHungerCheck = time + Config::instance()->hungerDamageRate() * MY_CLOCKS_PER_SEC;
}

void cTiming::checkRegeneration( P_CHAR character, unsigned int time )
{
	// Dead characters dont regenerate
	if ( character->isDead() )
	{
		return;
	}

	startProfiling(PF_REGENERATION);

	if ( character->regenHitpointsTime() <= time )
	{
		// If it's not disabled hunger affects our health regeneration
		if ( character->hitpoints() < character->maxHitpoints() )
		{
			if ( !Config::instance()->hungerRate() || character->hunger() > 3 )
			{
				character->setHitpoints( character->hitpoints() + 1 );
				character->updateHealth();
				character->setRegenHitpointsTime( ( uint )( Server::instance()->time() + floor( character->getHitpointRate() * 1000 ) ) );
			}
		}
	}

	if ( character->regenStaminaTime() <= time )
	{
		if ( character->stamina() < character->maxStamina() )
		{
			character->setStamina( character->stamina() + 1 );
			character->setRegenStaminaTime( ( uint )( Server::instance()->time() + floor( character->getStaminaRate() * 1000 ) ) );

			P_PLAYER player = dynamic_cast<P_PLAYER>( character );
			if ( player && player->socket() )
			{
				player->socket()->updateStamina();
			}
		}
	}

	if ( character->regenManaTime() <= time )
	{
		if ( character->mana() < character->maxMana() )
		{
			character->setMana( character->mana() + 1 );
			character->setRegenManaTime( ( uint )( Server::instance()->time() + floor( character->getManaRate() * 1000 ) ) );

			P_PLAYER player = dynamic_cast<P_PLAYER>( character );
			if ( player )
			{
				if ( player->socket() )
				{
					player->socket()->updateMana();
				}

				if ( player->isMeditating() && character->mana() >= character->maxMana() )
				{
					player->setMeditating( false );
					player->sysmessage( 501846 );
				}
			}
		}
	}

	stopProfiling(PF_REGENERATION);
}

void cTiming::checkPlayer( P_PLAYER player, unsigned int time )
{
	startProfiling(PF_PLAYERCHECK);

	cUOSocket* socket = player->socket();

	// Criminal Flagging
	if ( player->criminalTime() != 0 && player->criminalTime() <= time )
	{
		socket->sysMessage( tr( "You are no longer criminal." ) );
		player->setCriminalTime( 0 );
	}

	// Murder Decay
	if ( player->murdererTime() > 0 && player->murdererTime() < time )
	{
		if ( player->kills() > 0 )
			player->setKills( player->kills() - 1 );

		if ( player->kills() <= Config::instance()->maxkills() && Config::instance()->maxkills() > 0 )
		{
			socket->sysMessage( tr( "You are no longer a murderer." ) );
			player->setMurdererTime( 0 );
		}
		else
		{
			player->setMurdererTime( time + Config::instance()->murderdecay() * MY_CLOCKS_PER_SEC );
		}
	}

	stopProfiling(PF_PLAYERCHECK);
}

void cTiming::checkNpc( P_NPC npc, unsigned int time )
{
	// Remove summoned npcs
	if ( npc->summoned() && npc->summonTime() <= time )
	{
		// Make pooofff and sheeesh
		npc->soundEffect( 0x1fe );
		npc->pos().effect( 0x3735, 10, 30 );
		npc->remove();
		return;
	}

	// Give the AI time to process events
	if ( npc->ai() && npc->aiCheckTime() <= time )
	{
		startProfiling(PF_AICHECK);
		npc->ai()->check();
		stopProfiling(PF_AICHECK);
	}

	// Hunger for npcs
	// This only applies to tamed creatures
	if ( npc->isTamed() && Config::instance()->hungerRate() && npc->hungerTime() <= time )
	{
		// Creatures owned by GMs won't hunger.
		if ( !npc->owner() || !npc->owner()->isGMorCounselor()) {
			if ( npc->hunger() )
			{
				npc->setHunger( npc->hunger() - 1 );
			}

			npc->setHungerTime( time + Config::instance()->hungerRate() * 60 * MY_CLOCKS_PER_SEC );

			switch ( npc->hunger() )
			{
			case 4:
				npc->emote( tr( "*%1 looks a little hungry*" ).arg( npc->name() ), 0x26 );
				break;
			case 3:
				npc->emote( tr( "*%1 looks fairly hungry*" ).arg( npc->name() ), 0x26 );
				break;
			case 2:
				npc->emote( tr( "*%1 looks extremely hungry*" ).arg( npc->name() ), 0x26 );
				break;
			case 1:
				npc->emote( tr( "*%1 looks weak from starvation*" ).arg( npc->name() ), 0x26 );
				break;
			case 0:
				npc->setWanderType( enFreely );
				npc->setTamed( false );

				if ( npc->owner() )
				{
					npc->setOwner( 0 );
				}

				npc->bark( cBaseChar::Bark_Attacking );
				npc->talk( 1043255, npc->name(), 0, false, 0x26 );

				if ( Config::instance()->tamedDisappear() == 1 )
				{
					npc->soundEffect( 0x1FE );
					npc->remove();
				}
				break;
			}
		}
	}
}

void cTiming::addDecayItem( P_ITEM item )
{
	unsigned int delay = item->decayDelay();

	if ( delay )
	{
		DecayPair pair( Server::instance()->time() + delay, item->serial() );
		decayitems.append( pair );
	}
}

void cTiming::removeDecayItem( P_ITEM item )
{
	DecayIterator it;
	register SERIAL serial = item->serial();
	for ( it = decayitems.begin(); it != decayitems.end(); ++it )
	{
		if ( ( *it ).second == serial )
		{
			decayitems.remove( it );
			return;
		}
	}
}

void cTiming::removeDecaySerial( SERIAL serial )
{
	DecayIterator it;
	for ( it = decayitems.begin(); it != decayitems.end(); ++it )
	{
		if ( ( *it ).second == serial )
		{
			decayitems.remove( it );
			return;
		}
	}
}

