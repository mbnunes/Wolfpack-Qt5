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

// Platform Includes
#include "platform.h"

#include "timing.h"

#include "basics.h"
#include "timers.h"
#include "combat.h"
#include "sectors.h"
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
	nextTamedCheck = time + Config::instance()->checkTamedTime() * MY_CLOCKS_PER_SEC;
	nextNpcCheck = time + Config::instance()->checkNPCTime() * MY_CLOCKS_PER_SEC;
	nextItemCheck = time + 10000; // Every 10 seconds
	nextShopRestock = time + 20 * 60 * MY_CLOCKS_PER_SEC; // Every 20 minutes
	nextHungerCheck = time + Config::instance()->hungerDamageRate();
	nextCombatCheck = time + 100; // Every 100 ms
	nextUOTimeTick = 0;
}

inline int froundf( double f )
{
	int i = floor( f );
	if ( f - i >= 0.50 )
	{
		++i;
	}
	return i;
}

void cTiming::poll()
{
	unsigned int time = getNormalizedTime();

	// Check for spawn regions
	if ( nextSpawnRegionCheck <= time )
	{
		SpawnRegions::instance()->check();
		nextSpawnRegionCheck = time + Config::instance()->spawnRegionCheckTime() * MY_CLOCKS_PER_SEC;
	}

	// Check for decay items
	if (nextItemCheck <= time) {
		QValueVector<SERIAL> toRemove;
		DecayIterator it = decayitems.begin();

		while (it != decayitems.end()) {
			if ((*it).first <= time) {
				toRemove.append((*it).second);
			}
			++it;
		}

		QValueVector<SERIAL>::iterator sit;
		for (sit = toRemove.begin(); sit != toRemove.end(); ++sit) {
			P_ITEM item = FindItemBySerial(*sit);
			if (item && item->isInWorld()) {
				item->remove(); // Auto removes from the decaylist
			} else {
				removeDecaySerial(*sit);
			}
		}

		nextItemCheck += 5000;
	}

	// Check for an automated worldsave
	if ( Config::instance()->saveInterval() )
	{
		// Calculate the next worldsave based on the last worldsave
		unsigned int nextSave = lastWorldsave() + Config::instance()->saveInterval() * MY_CLOCKS_PER_SEC;

		if ( nextSave <= time )
		{
			World::instance()->save();
		}
	}

	unsigned int events = 0;

	// Check lightlevel and time
	if ( nextUOTimeTick <= time )
	{
		unsigned char oldhour = UoTime::instance()->hour();
		UoTime::instance()->setMinutes( UoTime::instance()->getMinutes() + 1 );
		nextUOTimeTick = time + Config::instance()->secondsPerUOMinute() * MY_CLOCKS_PER_SEC;

		unsigned char & currentLevel = Config::instance()->worldCurrentLevel();
		unsigned char newLevel = currentLevel;

		unsigned char darklevel = Config::instance()->worldDarkLevel();
		unsigned char brightlevel = Config::instance()->worldBrightLevel();
		unsigned char difference = QMAX( 0, ( int ) darklevel - ( int ) brightlevel ); // Never get below zero
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
			newLevel = darklevel - QMIN( darklevel, froundf( factor * ( float ) difference ) );

			// 18 to 22 = Nightfall (Scaled)
		}
		else
		{
			double factor = ( ( hour - 18 ) * 60 + UoTime::instance()->minute() ) / 240.0;
			newLevel = brightlevel + froundf( factor * ( float ) difference );
		}

		if ( newLevel != currentLevel )
		{
			events |= cBaseChar::EventLight;
			currentLevel = newLevel;
		}
	}

	if ( nextCombatCheck <= time )
	{
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
	}

	// Save the positions of connected players
	QValueVector<Coord_cl> positions;

	// Periodic checks for connected players
	for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
	{
		if ( !socket->player() )
		{
			continue;
		}

		socket->player()->poll( time, events );
		checkRegeneration( socket->player(), time );
		checkPlayer( socket->player(), time );
		positions.append( socket->player()->pos() );
	}

	// Check all other characters
	if ( nextNpcCheck <= time )
	{
		cCharIterator chariter;
		for ( P_CHAR character = chariter.first(); character; character = chariter.next() )
		{
			P_NPC npc = dynamic_cast<P_NPC>( character );

			if ( npc && npc->stablemasterSerial() == INVALID_SERIAL )
			{
				// Check if we are anywhere near a player
				// all other npcs are accounted as inactive
				for ( QValueVector<Coord_cl>::const_iterator it = positions.begin(); it != positions.end(); ++it )
				{
					if ( ( *it ).distance( npc->pos() ) <= 24 )
					{
						checkRegeneration( npc, time );
						checkNpc( npc, time );
						npc->poll( time, events );
						break;
					}
				}
				continue;
			}

			P_PLAYER player = dynamic_cast<P_PLAYER>( character );
			if ( player && !player->socket() && player->logoutTime() && player->logoutTime() <= time )
			{
				player->removeFromView( false );
				player->setLogoutTime( 0 );
				player->resend( false );
				MapObjects::instance()->remove( player );
			}
		}

		if ( nextTamedCheck <= time )
			nextTamedCheck = time + Config::instance()->checkTamedTime() * MY_CLOCKS_PER_SEC;

		if ( nextNpcCheck <= time )
			nextNpcCheck = time + Config::instance()->checkNPCTime() * MY_CLOCKS_PER_SEC;
	}

	// Check the Timers
	Timers::instance()->check();

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

	if ( character->regenHitpointsTime() <= time )
	{
		// If it's not disabled hunger affects our health regeneration
		if ( character->hitpoints() < character->maxHitpoints() )
		{
			if ( !Config::instance()->hungerRate() || character->hunger() > 3 )
			{
				character->setHitpoints( character->hitpoints() + 1 );
				character->updateHealth();
				character->setRegenHitpointsTime( Server::instance()->time() + floor( character->getHitpointRate() * 1000 ) );
			}
		}
	}

	if ( character->regenStaminaTime() <= time )
	{
		if ( character->stamina() < character->maxStamina() )
		{
			character->setStamina( character->stamina() + 1 );
			character->setRegenStaminaTime( Server::instance()->time() + floor( character->getStaminaRate() * 1000 ) );

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
			character->setRegenManaTime( Server::instance()->time() + floor( character->getManaRate() * 1000 ) );

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
}

void cTiming::checkPlayer( P_PLAYER player, unsigned int time )
{
	cUOSocket* socket = player->socket();

	// Criminal Flagging
	if ( player->criminalTime() > 0 && player->criminalTime() <= time )
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

	// All food related things are disabled for gms
	if ( player->isGMorCounselor() )
	{
		// Decrease food level
		if ( Config::instance()->hungerRate() > 1 && ( player->hungerTime() <= time ) )
		{
			if ( player->hunger() )
				player->setHunger( player->hunger() - 1 );

			player->setHungerTime( time + Config::instance()->hungerRate() * MY_CLOCKS_PER_SEC );
		}

		// Damage if we are starving
		if ( Config::instance()->hungerDamage() && nextHungerCheck <= time )
		{
			if ( player->hitpoints() > 0 && player->hunger() < 2 && !player->isDead() )
			{
				socket->sysMessage( tr( "You are starving." ) );
				player->damage( DAMAGE_HUNGER, Config::instance()->hungerDamage() );
			}
		}
	}
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
		npc->setAICheckTime( time + Config::instance()->checkAITime() * MY_CLOCKS_PER_SEC );
		npc->ai()->check();
	}

	// Hunger for npcs
	// This only applies to tamed creatures
	if ( npc->isTamed() && Config::instance()->hungerRate() && npc->hungerTime() <= time )
	{
		if ( npc->hunger() )
		{
			npc->setHunger( npc->hunger() - 1 );
		}

		npc->setHungerTime( time + Config::instance()->hungerRate() * MY_CLOCKS_PER_SEC );

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

void cTiming::addDecayItem(P_ITEM item) {
	unsigned int delay = item->decayDelay();

	if (delay) {
		DecayPair pair(Server::instance()->time() + delay, item->serial());
		decayitems.append(pair);
	}
}

void cTiming::removeDecayItem(P_ITEM item) {
	DecayIterator it;
	register SERIAL serial = item->serial();
	for (it = decayitems.begin(); it != decayitems.end(); ++it) {
		if ((*it).second == serial) {
			decayitems.remove(it);
			return;
		}
	}
}

void cTiming::removeDecaySerial(SERIAL serial) {
	DecayIterator it;
	for (it = decayitems.begin(); it != decayitems.end(); ++it) {
		if ((*it).second == serial) {
			decayitems.remove(it);
			return;
		}
	}
}

