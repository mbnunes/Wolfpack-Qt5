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

// Platform Includes
#include "platform.h"

#include "Timing.h"

#include "basics.h"
#include "TmpEff.h"
#include "combat.h"
#include "sectors.h"
#include "srvparams.h"
#include "network.h"
#include "spawnregions.h"
#include "territories.h"
#include "skills.h"
#include "typedefs.h"
#include "itemid.h"
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

cTiming::cTiming() {
	unsigned int time = getNormalizedTime();

	lastWorldsave_ = 0;
	nextSpawnRegionCheck = time + SrvParams->spawnRegionCheckTime() * MY_CLOCKS_PER_SEC;
	nextTamedCheck = time + SrvParams->checkTamedTime() * MY_CLOCKS_PER_SEC;
	nextNpcCheck = time + SrvParams->checkNPCTime() * MY_CLOCKS_PER_SEC;
	nextItemCheck = time + SrvParams->checkItemTime() * MY_CLOCKS_PER_SEC;
	nextShopRestock = time + 20 * 60 * MY_CLOCKS_PER_SEC; // Every 20 minutes
	nextHungerCheck = time + SrvParams->hungerDamageRate();
	nextCombatCheck = time + 100; // Every 100 ms
	nextUOTimeTick = 0;
}

inline int froundf(double f) {
	int i = floor(f);
	if (f - i >= 0.50) {
		++i;
	}
	return i;
}

void cTiming::poll() {
	unsigned int time = getNormalizedTime();

	// Check for spawn regions
	if (nextSpawnRegionCheck <= time) {
		SpawnRegions::instance()->check();
		nextSpawnRegionCheck = time + SrvParams->spawnRegionCheckTime() * MY_CLOCKS_PER_SEC;
	}

	// Check for an automated worldsave
	if (SrvParams->saveInterval()) {
		// Calculate the next worldsave based on the last worldsave
		unsigned int nextSave = lastWorldsave() + SrvParams->saveInterval() * MY_CLOCKS_PER_SEC;

		if (nextSave <= time) {
			World::instance()->save();
		}
	}

	unsigned int events = 0;

	// Check lightlevel and time
	if (nextUOTimeTick <= time) {
		unsigned char oldhour = UoTime::instance()->hour();
		UoTime::instance()->setMinutes(UoTime::instance()->getMinutes() + 1);
		nextUOTimeTick = time + SrvParams->secondsPerUOMinute() * MY_CLOCKS_PER_SEC;

		unsigned char &currentLevel = SrvParams->worldCurrentLevel();
		unsigned char newLevel = currentLevel;

		unsigned char darklevel = SrvParams->worldDarkLevel();
		unsigned char brightlevel = SrvParams->worldBrightLevel();
		unsigned char difference = QMAX(0, (int)darklevel - (int)brightlevel); // Never get below zero
		unsigned char hour = UoTime::instance()->hour();

		if (hour != oldhour) {
			events |= cBaseChar::EventTime;
		}

		// 11 to 18 = Day
		if (hour >= 11 && hour < 18) {
			newLevel = brightlevel;

		// 22 to 6 = Night
		} else if (hour >= 22 || hour < 6) {
			newLevel = darklevel;

		// 6 to 10 = Dawn (Scaled)
		} else if (hour >= 6 && hour < 11) {
			double factor = ((hour - 6) * 60 + UoTime::instance()->minute()) / 240.0;
			newLevel = darklevel - QMIN(darklevel, froundf(factor * (float)difference));

		// 18 to 22 = Nightfall (Scaled)
		} else {
			double factor = ((hour - 18) * 60 + UoTime::instance()->minute()) / 240.0;
			newLevel = brightlevel + froundf(factor * (float)difference);
		}

		if (newLevel != currentLevel) {
			events |= cBaseChar::EventLight;
            currentLevel = newLevel;
		}
	}

	if (nextCombatCheck <= time) {
		nextCombatCheck = time + 250;
		
		// Check for timed out fights
		QPtrList<cFightInfo> fights = Combat::instance()->fights();
		fights.setAutoDelete(false);
		QPtrList<cFightInfo> todelete;
		todelete.setAutoDelete(true);
		cFightInfo *info;
		for (info = fights.first(); info; info = fights.next()) {
			// 60 Seconds without melee contact and 
			// combatants are out of range...
			if (info->lastaction() + 60000 <= time) {
				if (info->victim()->isDead() || info->attacker()->isDead() && 
					!info->attacker()->inRange(info->victim(), SrvParams->attack_distance())) {
					todelete.append(info);
				}
			} else {
				P_CHAR attacker = info->attacker();
				P_CHAR victim = info->victim();

				if (attacker && !victim->free) {
					attacker->poll(time, cBaseChar::EventCombat);
				}
				if (victim && !victim->free) {
					victim->poll(time, cBaseChar::EventCombat);
				}
			}
		}
	}

	// Save the positions of connected players
	QValueVector<Coord_cl> positions;

	// Periodic checks for connected players
	for (cUOSocket *socket = Network::instance()->first(); socket; socket = Network::instance()->next()) {
		if (!socket->player()) {
			continue;
		}

		socket->player()->poll(time, events);
		checkRegeneration(socket->player(), time);
		checkPlayer(socket->player(), time);
		positions.append(socket->player()->pos());
	}

	// Check all other characters
	if (nextNpcCheck <= time) 
	{
		cCharIterator chariter;
		for (P_CHAR character = chariter.first(); character; character = chariter.next()) {
			P_NPC npc = dynamic_cast<P_NPC>(character);

			if (npc && npc->stablemasterSerial() == INVALID_SERIAL) {
				// Check if we are anywhere near a player
				// all other npcs are accounted as inactive
				for (QValueVector<Coord_cl>::const_iterator it = positions.begin(); it != positions.end(); ++it) {
					if ((*it).distance(npc->pos()) <= 24) {
						checkRegeneration(npc, time);
						checkNpc(npc, time);
						npc->poll(time, events);
						break;
					}
				}
				continue;
			}

			P_PLAYER player = dynamic_cast<P_PLAYER>(character);
			if (player && !player->socket() && player->logoutTime() && player->logoutTime() <= time) {
				player->removeFromView(false);
				player->setLogoutTime(0);
				player->resend(false);
				MapObjects::instance()->remove(player);
			}
		}

		if (nextTamedCheck <= time) 
			nextTamedCheck = time + SrvParams->checkTamedTime() * MY_CLOCKS_PER_SEC;

		if (nextNpcCheck <= time) 
			nextNpcCheck = time + SrvParams->checkNPCTime() * MY_CLOCKS_PER_SEC;
	}

	// Check the TempEffects
	TempEffects::instance()->check();

	if (nextHungerCheck <= time) 
		nextHungerCheck = time + SrvParams->hungerDamageRate() * MY_CLOCKS_PER_SEC;
}

void cTiming::checkRegeneration(P_CHAR character, unsigned int time) 
{
	// Dead characters dont regenerate
	if (character->isDead()) {
		return;
	}

	unsigned int oldStamina = character->stamina();
	unsigned int oldMana = character->mana();

	if (character->regenHitpointsTime() <= time) {
		// If it's not disabled hunger affects our health regeneration
		if (character->hitpoints() < character->maxHitpoints()) {
			if (!SrvParams->hungerRate() || character->hunger() > 3) {
				character->setHitpoints(character->hitpoints() + 1);
				character->updateHealth();
				character->setRegenHitpointsTime(uiCurrentTime + floor(character->getHitpointRate() * 1000));
			}
		}
	}

	if (character->regenStaminaTime() <= time) {
		if (character->stamina() < character->maxStamina()) {
			character->setStamina(character->stamina() + 1);
			character->setRegenStaminaTime(uiCurrentTime + floor(character->getStaminaRate() * 1000));

			P_PLAYER player = dynamic_cast<P_PLAYER>(character);
			if (player && player->socket()) {
				player->socket()->updateStamina();
			}
		}
	}

	if (character->regenManaTime() <= time) {
		if (character->mana() < character->maxMana()) {
			character->setMana(character->mana() + 1);
			character->setRegenManaTime(uiCurrentTime + floor(character->getManaRate() * 1000));

			P_PLAYER player = dynamic_cast<P_PLAYER>(character);
			if (player) {
				if (player->socket()) {
					player->socket()->updateMana();
				}

				if (player->isMeditating() && character->mana() >= character->maxMana()) {
					player->setMeditating(false);
					player->sysmessage(501846);
				}
			}
		}
	}
}

void cTiming::checkPlayer(P_PLAYER player, unsigned int time) 
{
	cUOSocket *socket = player->socket();

	// Criminal Flagging
	if (player->criminalTime() > 0 && player->criminalTime() <= time) 
	{
		socket->sysMessage(tr("You are no longer criminal."));
		player->setCriminalTime(0);
	}

    // Murder Decay
	if (player->murdererTime() > 0 && player->murdererTime() < time) 
	{
		if (player->kills() > 0)
			player->setKills(player->kills() - 1);

		if ( player->kills() <= SrvParams->maxkills() && SrvParams->maxkills() > 0 ) 
			socket->sysMessage( tr( "You are no longer a murderer." ) );
		else
			player->setMurdererTime(time + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC);
	}

	// All food related things are disabled for gms
	if (player->isGMorCounselor()) 
	{
		// Decrease food level
		if (SrvParams->hungerRate() > 1 && (player->hungerTime() <= time )) 
		{
			if (player->hunger()) 
				player->setHunger(player->hunger() - 1);
	
			player->setHungerTime(time + SrvParams->hungerRate() * MY_CLOCKS_PER_SEC);
		}
	
		// Damage if we are starving
		if (SrvParams->hungerDamage() && nextHungerCheck <= time) 
		{
			if (player->hitpoints() > 0 && player->hunger() < 2 && !player->isDead()) 
			{
				socket->sysMessage(tr("You are starving."));
				player->damage(DAMAGE_HUNGER, SrvParams->hungerDamage());
			}
		}
	}
}

void cTiming::checkNpc(P_NPC npc, unsigned int time) 
{
	// Remove summoned npcs
	if (npc->summoned() && npc->summonTime() <= time) {
		// Make pooofff and sheeesh
		npc->soundEffect(0x1fe);
		npc->pos().effect(0x3735, 10, 30);
		npc->remove();
		return;
	}

	// Give the AI time to process events
	if (npc->ai() && npc->aiCheckTime() <= time) {
		npc->setAICheckTime(time + npc->aiCheckInterval());
		npc->ai()->check();
	}

	// Hunger for npcs
	// This only applies to tamed creatures
	if (npc->isTamed() && SrvParams->hungerRate() && npc->hungerTime() <= time) 
	{
		if (npc->hunger()) 
		{
			npc->setHunger(npc->hunger() - 1);
		}

		npc->setHungerTime(time + SrvParams->hungerRate() * MY_CLOCKS_PER_SEC);

		switch(npc->hunger()) 
		{
		case 4:
			npc->emote( tr("*%1 looks a little hungry*").arg(npc->name()), 0x26);
			break;
		case 3:
			npc->emote( tr("*%1 looks fairly hungry*").arg(npc->name()), 0x26);
			break;
		case 2:
			npc->emote( tr("*%1 looks extremely hungry*").arg(npc->name()), 0x26);
			break;
		case 1:
			npc->emote( tr("*%1 looks weak from starvation*").arg(npc->name()), 0x26);
			break;
		case 0:
			npc->setWanderType(enFreely);
			npc->setTamed(false);

			if (npc->owner()) {
				npc->setOwner(0);
			}
	
			npc->bark(cBaseChar::Bark_Attacking);
			npc->talk(1043255, npc->name(), 0, false, 0x26);

			if (SrvParams->tamedDisappear() == 1) 
			{
				npc->soundEffect(0x1FE);
				npc->remove();
			}
			break;
		}
	}
}
