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
#include "boats.h"
#include "house.h"
#include "typedefs.h"
#include "itemid.h"
#include "basechar.h"
#include "npc.h"
#include "player.h"
#include "chars.h"
#include "ai/ai.h"
#include "inlines.h"
#include "walking.h"
#include "world.h"

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
	nextLightCheck = time + MY_CLOCKS_PER_SEC * 30; // Every 30 seconds
	nextHungerCheck = time + SrvParams->hungerDamageRate();
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

	// Resend the lightlevel too all clients outside of dungeons?
	bool updateLight = false;

	// Check lightlevel
	if (nextLightCheck <= time) {
		unsigned char &currentLevel = SrvParams->worldCurrentLevel();
		unsigned char newLevel;

		if (uoTime.time().hour() <= 3 || uoTime.time().hour() >= 10) {
			newLevel = SrvParams->worldDarkLevel();
		} else {
			newLevel = SrvParams->worldBrightLevel();
		}

		if (newLevel != currentLevel) {
			updateLight = true;
            currentLevel = newLevel;
		}

		// Update lightlevel every 30 seconds
		nextLightCheck = time + 30 * MY_CLOCKS_PER_SEC;
	}

	// Save the positions of connected players
	QValueVector<Coord_cl> positions;

	// Periodic checks for connected players
	for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
		if (!socket->player()) {
			continue;
		}

		checkRegeneration(socket->player(), time);
		checkPlayer(socket->player(), time);
		positions.append(socket->player()->pos());
	}

	// Check all other characters
	if (nextTamedCheck <= time || nextNpcCheck <= time) {
		cCharIterator chariter;
		for (P_CHAR character = chariter.first(); character; character = chariter.next()) {
			P_NPC npc = dynamic_cast<P_NPC>(character);

			if (npc) {
				if ((npc->isTamed() && nextTamedCheck <= time) || (!npc->isTamed() && nextNpcCheck <= time)) {
					checkRegeneration(npc, time);
					
					// Check if we are anywhere near a player
					// all other npcs are accounted as inactive
					for (QValueVector<Coord_cl>::const_iterator it = positions.begin(); it != positions.end(); ++it) {
						if ((*it).distance(npc->pos()) <= 24) {
							checkNpc(npc, time);
							break;
						}
					}
				}

				continue;
			}

			P_PLAYER player = dynamic_cast<P_PLAYER>(character);
			if (player) {
				if (!player->socket() && player->logoutTime() && player->logoutTime() >= time) {
					player->setLogoutTime(0);
					player->removeFromView(false);
					player->resend(false);
				}
				continue;
			}
		}

		if (nextTamedCheck <= time) {
			nextTamedCheck = time + SrvParams->checkTamedTime() * MY_CLOCKS_PER_SEC;
		}

		if (nextNpcCheck <= time) {
			nextNpcCheck = time + SrvParams->checkNPCTime() * MY_CLOCKS_PER_SEC;
		}
	}

	// Change all worlditems
	if (nextItemCheck <= time) {
		cItemIterator itemiter;
		for (P_ITEM item = itemiter.first(); item; item = itemiter.next()) {
			item->respawn(time);
			item->decay(time);

			switch(item->type()) {
				// Move Boats
				case 117:
				{
					bool ok = false;
					if (item->getTag( "tiller" ).isValid() && item->getTag("gatetime").toInt(&ok) <= time && ok) {
						cBoat* pBoat = dynamic_cast<cBoat*>(FindItemBySerial(item->getTag("boatserial").toInt()));
						if (pBoat) {
							pBoat->move();
							item->setTag("gatetime", time + SrvParams->boatSpeed() * MY_CLOCKS_PER_SEC);
						}
					}
					break;
				}
			}
		}

		nextItemCheck = time + SrvParams->checkItemTime() * MY_CLOCKS_PER_SEC;
	}

	// Check the TempEffects
	TempEffects::instance()->check();

	if (nextHungerCheck <= time) {
		nextHungerCheck = time + SrvParams->hungerDamageRate() * MY_CLOCKS_PER_SEC;
	}
}

void cTiming::checkRegeneration(P_CHAR character, unsigned int time) {
	// Dead characters dont regenerate
	if (character->isDead()) {
		return;
	}

	unsigned int oldHealth = character->hitpoints();
	unsigned int oldStamina = character->stamina();
	unsigned int oldMana = character->mana();

	if (character->regenHitpointsTime() <= time) {
		unsigned int interval = SrvParams->hitpointrate() * MY_CLOCKS_PER_SEC;

		// If it's not disabled hunger affects our health regeneration
		if (character->hitpoints() < character->maxHitpoints() && character->hunger() > 3 || SrvParams->hungerRate() == 0) {
			for (unsigned short c = 0; c < character->maxHitpoints() + 1; ++c) {
				if (character->regenHitpointsTime() + (c * interval) <= time && character->hitpoints() <= character->maxHitpoints()) {
					if (character->skillValue(HEALING) < 500) {
						character->setHitpoints(character->hitpoints() + 1);
					} else if (character->skillValue(HEALING) < 800) {
						character->setHitpoints(character->hitpoints() + 2);
					} else {
						character->setHitpoints(character->hitpoints() + 3);
					}

					if (character->hitpoints() > character->maxHitpoints()) {
						character->setHitpoints(character->maxHitpoints());
						break;
					}
				}
			}
		}

		character->setRegenHitpointsTime(time + interval);
	}

	if (character->regenStaminaTime() <= time) {
		unsigned int interval = SrvParams->staminarate() * MY_CLOCKS_PER_SEC;
		for (unsigned short c = 0; c < character->maxStamina() + 1; ++c) {
			if (character->regenStaminaTime() + (c * interval) <= time && character->stamina() <= character->maxStamina()) {
				if (character->stamina() >= character->maxStamina()) {
					character->setStamina(character->maxStamina());
					break;
				} else {
					character->setStamina(character->stamina() + 1);
				}
			}
		}
		character->setRegenStaminaTime(time + interval);
	}

	// OSI Style Mana regeneration by blackwind
	// if (character->in>character->mn)  this leads to the 'mana not subtracted' bug (Duke)
	if (character->regenManaTime() <= time) {
		unsigned int interval = SrvParams->manarate() * MY_CLOCKS_PER_SEC;
		for (unsigned short c = 0; c < character->maxMana() + 1 ; ++c) {
			if (character->regenManaTime() + (c * interval) <= time) {
				if (character->mana() >= character->maxMana()) {
					if (character->isMeditating()) {
						P_PLAYER player = dynamic_cast<P_PLAYER>(character);

						if (player->socket()) {
							player->socket()->clilocMessage(501846);
						}
						character->setMeditating(false);
					}
					character->setMana(character->maxMana());
					break;
				} else {
					character->setMana(character->mana() + 1);
				}
			}
		}

		if (SrvParams->armoraffectmana()) {
			int ratio = (150 / SrvParams->manarate());
			// 100 = Maximum skill (GM)
			// 50 = int affects mana regen (%50)
			int armorhandicap = ((Skills->GetAntiMagicalArmorDefence(character) + 1) / SrvParams->manarate());
			int charsmeditsecs = (1 + SrvParams->manarate() - ((((character->skillValue(MEDITATION) + 1)/10) + ((character->intelligence() + 1) / 2)) / ratio));
			if (character->isMeditating()) {
				character->setRegenManaTime(time + ((armorhandicap + charsmeditsecs/2)* MY_CLOCKS_PER_SEC));
			} else {
				character->setRegenManaTime(time + ((armorhandicap + charsmeditsecs)* MY_CLOCKS_PER_SEC));
			}
		}
		else 
			character->setRegenManaTime( time + interval );
		}

	if (character->hitpoints() > character->maxHitpoints()) {
		character->setHitpoints(character->maxHitpoints());
	}

	if (character->stamina() > character->maxStamina()) {
		character->setStamina(character->maxStamina());
	}

	if (character->mana() > character->maxMana()) {
		character->setMana(character->maxMana());
	}

	P_PLAYER player = dynamic_cast<P_PLAYER>(character);
	if (player && player->socket()) {
		if (oldHealth != player->hitpoints()) {
			player->socket()->updateHealth();
		}

		if (oldStamina != player->stamina()) {
			player->socket()->updateStamina();
		}

		if (oldMana != player->mana()) {
			player->socket()->updateMana();
		}
	} else if (oldHealth != character->hitpoints()) {
		character->updateHealth();
	}
}

void cTiming::checkPlayer(P_PLAYER player, unsigned int time) {
	cUOSocket *socket = player->socket();

	unsigned int tempuint;
	signed short tempshort;
	int timer;//, valid=0;
	
	// We are not swinging 
	// So set up a swing target and start swinging
	if (!player->isDead() && player->swingTarget() == INVALID_SERIAL) {
		Combat::combat(player);
	}

	// We are swinging and completed swinging
	else if (!player->isDead() && (player->swingTarget() >= 0 && player->nextHitTime() <= time)) {
		Combat::checkandhit(player);
	}

	// Criminal Flagging
	if (player->criminalTime() > 0 && player->criminalTime() <= time) {
		socket->sysMessage(tr("You are no longer criminal."));
		player->setCriminalTime(0);
	}

    // Murder Decay
	if (player->murdererTime() > 0 && player->murdererTime() < time) {
		if (player->kills() > 0)
			player->setKills(player->kills() - 1);

		if ( player->kills() <= SrvParams->maxkills() && SrvParams->maxkills() > 0 ) {
			socket->sysMessage( tr( "You are no longer a murderer." ) );
		} else {
			player->setMurdererTime(time + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC);
		}
	}

	// All food related things are disabled for gms
	if (player->isGMorCounselor()) {
		// Decrease food level
		if (SrvParams->hungerRate() > 1 && (player->hungerTime() <= time )) {
			if (player->hunger()) { 
				player->setHunger(player->hunger() - 1);
			}
	
			player->setHungerTime(time + SrvParams->hungerRate() * MY_CLOCKS_PER_SEC);
		}
	
		// Damage if we are starving
		if (SrvParams->hungerDamage() && nextHungerCheck <= time) {
			if (player->hitpoints() > 0 && player->hunger() < 2 && !player->isDead()) {
				socket->sysMessage(tr("You are starving."));
				player->damage(DAMAGE_HUNGER, SrvParams->hungerDamage());
			}
		}
	}
}

void cTiming::checkNpc(P_NPC npc, unsigned int time) {
	// We are stabled and don't receive events
	if (npc->stablemasterSerial() != INVALID_SERIAL) {
		return;
	}

	// Give the AI time to process events
	if (npc->aiCheckTime() <= time) {
		npc->setAICheckTime(uiCurrentTime + npc->aiCheckInterval());

		if (npc->ai()) {
			npc->ai()->check();
		}
	}

	// We are at war and want to prepare a new swing
	if (!npc->isDead() && npc->swingTarget() == INVALID_SERIAL && npc->isAtWar()) {
		Combat::combat(npc);
	
	// We completed the swing
	} else if (!npc->isDead() && npc->swingTarget() != INVALID_SERIAL && npc->nextHitTime() <= time) {
		Combat::checkandhit(npc);
	}

	// Remove summoned npcs
	if (npc->summonTime() && npc->summonTime() <= time) {
		npc->kill(0);
		return;
	}

	// Hunger for npcs
	// This only applies to tamed creatures
	if (npc->isTamed() && SrvParams->hungerRate() && npc->hungerTime() <= time) {
		if (npc->hunger()) {
			npc->setHunger(npc->hunger() - 1);
		}

		npc->setHungerTime(time + SrvParams->hungerRate() * MY_CLOCKS_PER_SEC);

		switch(npc->hunger()) {
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
				npc->setOwner(NULL);
			}
	
			npc->bark(cBaseChar::Bark_Attacking);
			npc->talk(1043255, npc->name(), 0, false, 0x26);

			if (SrvParams->tamedDisappear() == 1) {
				npc->soundEffect(0x1FE);
				cCharStuff::DeleteChar(npc);
			}
			break;
		}
	}
}
