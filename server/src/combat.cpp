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
#include "combat.h"
#include "basechar.h"
#include "npc.h"
#include "basics.h"
#include "config.h"
#include <qstring.h>

/*!
	Play the soundeffect for getting hit.
*/
void cCombat::playGetHitSoundEffect( P_CHAR pChar )
{
	if ( pChar->body() == 0x191 )
	{
		UI16 sound = hex2dec( Definitions::instance()->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_FEMALE" ) ).toUShort();
		if ( sound > 0 )
			pChar->soundEffect( sound );
		else
			pChar->soundEffect( 0x14b );
	}
	else if ( pChar->body() == 0x190 )
	{
		UI16 sound = hex2dec( Definitions::instance()->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_MALE" ) ).toUShort();
		if ( sound > 0 )
			pChar->soundEffect( sound );
		else
			pChar->soundEffect( 0x156 );
	}
	else
		pChar->bark( cBaseChar::Bark_GetHit );
}

// play the get hit animation
void cCombat::playGetHitAnimation( P_CHAR pChar )
{
	if ( pChar->isHuman() )
	{
		pChar->action( 20 );
	}
	else
	{
		pChar->action( 10 );
	}
}

void cCombat::spawnGuard( P_CHAR pOffender, P_CHAR pCaller, const Coord_cl& pos )
{
	if ( !pOffender || !pCaller )
		return;

	if ( pOffender->isDead() || pCaller->isDead() )
		return;

	cTerritory* pRegion = pCaller->region();

	if ( pRegion == NULL )
		return;

	if ( pRegion->isGuarded() && Config::instance()->guardsActive() )
	{
		QString guardsect = pRegion->getGuardSect();

		P_NPC pGuard = ( guardsect.isNull() ? NULL : cNPC::createFromScript( guardsect, pos ) );

		if ( !pGuard )
			return;

		// Send guard to surrounding Players
		pGuard->resend( false );
		pGuard->soundEffect( 0x1FE );
		pGuard->effect( 0x372A, 0x09, 0x06 );
		pGuard->fight( pOffender );
	}
}

cFightInfo::cFightInfo( P_CHAR attacker, P_CHAR victim, bool legitimate )
{
	attacker_ = attacker;
	victim_ = victim;
	legitimate_ = legitimate;
	victimDamage_ = 0;
	attackerDamage_ = 0;
	lastaction_ = getNormalizedTime();

	// Register the fight info object with the combat system.
	attacker->fights().append( this );
	victim->fights().append( this );
	Combat::instance()->fights().append( this );
}

cFightInfo::~cFightInfo()
{
	// Unregister the fight info object with the combat system.
	if ( attacker_ )
	{
		attacker_->fights().remove( this );
	}

	if ( victim_ )
	{
		victim_->fights().remove( this );
	}
	Combat::instance()->fights().remove( this );
}

void cFightInfo::refresh()
{
	lastaction_ = Server::instance()->time();
}

void cFightInfo::clear()
{
	if ( victim_ )
	{
		victim_->fights().remove( this );
	}

	if ( attacker_ )
	{
		attacker_->fights().remove( this );
	}

	attacker_ = 0;
	victim_ = 0;
}
