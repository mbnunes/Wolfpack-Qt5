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

#include "definitions.h"
#include "basics.h"
#include "tracking.h"
#include "muls/tilecache.h"
#include "combat.h"
#include "targetrequests.h"
#include "mapobjects.h"
#include "serverconfig.h"
#include "scriptmanager.h"
#include "skills.h"
#include "network/network.h"
#include "muls/maps.h"
#include "network/uosocket.h"
#include "log.h"
#include "targetrequests.h"
#include "territories.h"
#include "npc.h"
#include "console.h"

// System Includes
#include <math.h>

void cSkills::SkillUse( cUOSocket* socket, Q_UINT16 id ) // Skill is clicked on the skill list
{
	P_PLAYER pChar = socket->player();

	// No Char no Skill use
	if ( !pChar )
		return;

	/* TODO: reimplementation of jail system
		if( pChar->cell() > 0 )
		{
			socket->sysMessage( tr( "You are in jail and cant use skills here!" ) );
			return;
		}
	*/

	if ( pChar->isDead() )
	{
		socket->sysMessage( tr( "You cannot do that as a ghost." ) );
		return;
	}

	if ( id != STEALTH )
		pChar->unhide(); // Don't unhide if we're trying to stealth

	pChar->disturbMed(); // Disturb meditation if we're using a skill

	if ( pChar->isCasting() )
	{
		socket->sysMessage( tr( "You can't do that while you are casting." ) );
		return;
	}

	if ( pChar->skillDelay() > Server::instance()->time() && !pChar->isGM() )
	{
		socket->sysMessage( tr( "You must wait a few moments before using another skill." ) );
		return;
	}

	if ( pChar->onSkillUse( id ) )
		return;

	cTargetRequest* targetRequest = NULL;
	QString message;

	switch ( id )
	{
	case MEDITATION:
		Skills::instance()->Meditation( socket );
		break;
	default:
		socket->sysMessage( tr( "That skill has not been implemented yet." ) );
		return;
	}

	if ( targetRequest )
		socket->attachTarget( targetRequest );

	if ( !message.isEmpty() )
		pChar->message( message );

	pChar->setSkillDelay( Server::instance()->time() + Config::instance()->skillDelay() * MY_CLOCKS_PER_SEC );
}

void cSkills::RandomSteal( cUOSocket* socket, SERIAL victim )
{
	P_PLAYER pChar = socket->player();
	P_CHAR pVictim = FindCharBySerial( victim );

	if ( !pVictim || !pChar )
		return;

	if ( pVictim->serial() == pChar->serial() )
	{
		socket->sysMessage( tr( "Why don't you simply take it?" ) );
		return;
	}

	/*	if( pVictim->npcaitype() == 17 )
		{
			socket->sysMessage( tr( "You cannot steal from Playervendors." ) );
			return;
		}
	*/
	if ( pVictim->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>( pVictim );
		if ( pp->isGMorCounselor() )
			socket->sysMessage( tr( "You can't steal from game masters." ) );
		return;
	}

	if ( !pChar->inRange( pVictim, 1 ) )
	{
		socket->sysMessage( tr( "You are too far away to steal from that person." ) );
		return;
	}

	P_ITEM pBackpack = pVictim->getBackpack();

	if ( !pBackpack )
	{
		socket->sysMessage( tr( "Bad luck, your victim doesn't have a backpack." ) );
		return;
	}

	// We can steal max. 10 Stones when we are a GM
	float maxWeight = wpMin<ushort>( 1, pChar->skillValue( STEALING ) );
	// 1000 Skill == 100 Weight == 10 Stones

	QPtrList<cItem> containment = pBackpack->getContainment();
	Q_UINT32 chance = containment.count();

	P_ITEM pItem = containment.first();
	P_ITEM pToSteal = 0;
	bool sawOkItem = false;

	while ( !pToSteal )
	{
		// We have nothing that could be stolen?
		if ( !pItem && !sawOkItem )
		{
			socket->sysMessage( tr( "Your victim posesses nothing you could steal." ) );
			return;
		}
		// Jump back to the beginning
		else if ( !pItem && sawOkItem )
		{
			pItem = containment.first();
		}

		// Check if our chance becomes true (no spellbooks!)
		if ( pItem->totalweight() <= maxWeight && !pItem->isLockedDown() && !pItem->newbie() && pItem->type() != 9 )
		{
			sawOkItem = true; // We have items that could be stolen (just in case we reach the end of our list)

			// We have the chance of 1/chance that we reached our desired item
			if ( RandomNum( 1, ( int )chance ) == ( int )chance )
			{
				pToSteal = pItem;
				break;
			}
		}

		pItem = containment.next();
	}

	socket->sysMessage( tr( "You reach into %1's backpack and try to steal something..." ).arg( pVictim->name() ) );

	// The success of our Theft depends on the weight of the stolen item
	bool success = pChar->checkSkill( STEALING, 0, ( long int )pToSteal->weight() );
	bool caught = false;

	if ( success )
	{
		socket->sysMessage( tr( "You successfully steal %1." ).arg( pToSteal->getName() ) );
		P_ITEM pPack = pChar->getBackpack();
		pPack->addItem( pToSteal );
		// Update item onyl if still existent
		if ( !pToSteal->free )
			pToSteal->update();

		caught = pChar->skillValue( STEALING ) < RandomNum(0, 1000);
	}
	else
	{
		socket->sysMessage( tr( "You fail to steal the item." ) );

		// 1 in 5 Chance if we failed
		caught = RandomNum( 1, 5 ) == 1;
	}

	// Did we get caught?
	if ( caught )
	{
		socket->sysMessage( tr( "You have been cought!" ) );

		// Human non red NPCs need special handling
		if ( pVictim->objectType() == enNPC && pVictim->isInnocent() && pVictim->isHuman() )
		{
			P_NPC pn = dynamic_cast<P_NPC>( pVictim );
			pVictim->talk( tr( "Guards! A thief is amoung us!" ), 0xFFFF, 0x09 );
			if ( pVictim->region() && pVictim->region()->isGuarded() )
				pn->callGuards();
		}

		if ( pVictim->notoriety( pChar ) == 0x01 )
			pChar->makeCriminal();

		// Our Victim always notices it.
		if ( pVictim->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>( pVictim );
			if ( pp->socket() )
				pp->socket()->showSpeech( pChar, tr( "You notice %1 trying to steal %2 from you." ).arg( pChar->name() ).arg( pToSteal->getName( true ) ) );
		}

		QString message = tr( "You notice %1 trying to steal %2 from %3." ).arg( pChar->name() ).arg( pItem->getName() ).arg( pVictim->name() );

		for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		{
			// Everyone within 7 Tiles notices us
			if ( mSock != socket && mSock->player() && mSock->player()->serial() != pVictim->serial() && mSock->player()->inRange( pChar, 7 ) )
				mSock->showSpeech( pChar, message );
		}
	}
}

void cSkills::Meditation( cUOSocket* socket )
{
	P_CHAR pc_currchar = socket->player();

	if ( pc_currchar->attackTarget() )
	{
		socket->clilocMessage( 501845 );
	}
	else if ( pc_currchar->getWeapon() || pc_currchar->getShield() )
	{
		socket->clilocMessage( 502626 );
	}
	else if ( pc_currchar->mana() >= pc_currchar->intelligence() )
	{
		socket->clilocMessage( 501846 );
	}
	else if ( !pc_currchar->checkSkill( MEDITATION, 0, 1200 ) )
	{
		socket->clilocMessage( 501850 );
	}
	else
	{
		socket->clilocMessage( 501851 );
		pc_currchar->setMeditating( true );
		pc_currchar->soundEffect( 0xf9 );
	}
}

void cSkills::Snooping( P_PLAYER player, P_ITEM container )
{
	P_CHAR owner = container->getOutmostChar();

	if (!owner)
		return; // Snooping into something thats not equipped?!

	PyObject *args = Py_BuildValue("(NNN)", owner->getPyObject(), container->getPyObject(), player->getPyObject());

	// Event prfen
	if (player->canHandleEvent(EVENT_SNOOPING)) {
		if (player->callEventHandler(EVENT_SNOOPING, args)) {
			Py_DECREF(args);
			return;
		}
	}

	if (owner->canHandleEvent(EVENT_SNOOPING)) {
		if (owner->callEventHandler(EVENT_SNOOPING, args)) {
			Py_DECREF(args);
			return;
		}
	}

	Py_DECREF(args);

	cUOSocket* socket = player->socket();

	if ( !socket )
		return;

	P_CHAR pc_owner = container->getOutmostChar();
	P_PLAYER pp_owner = dynamic_cast<P_PLAYER>( pc_owner );

	if ( pp_owner && pp_owner->isGMorCounselor() )
	{
		pp_owner->message( tr( "%1 is trying to snoop in your pack" ).arg( player->name() ) );
		socket->sysMessage( tr( "You can't peek into that container or you'll be jailed." ) );
		return;
	}
	else if ( player->checkSkill( SNOOPING, 0, 1000 ) )
	{
		socket->sendContainer( container );
		socket->sysMessage( tr( "You successfully peek into that container." ) );
	}
	else
	{
		socket->sysMessage( tr( "You failed to peek into that container." ) );

		if ( !pp_owner ) // is NPC ?
			pc_owner->talk( tr( "Art thou attempting to disturb my privacy?" ) );
		else
			pp_owner->message( tr( "You notice %1 trying to peek into your pack!" ).arg( player->name() ) );
	}



	//	SetTimerSec(player->objectdelay(), Config::instance()->objectDelay()+Config::instance()->snoopdelay());
	player->setObjectDelay( Server::instance()->time() + ( Config::instance()->objectDelay() + Config::instance()->snoopdelay() ) * MY_CLOCKS_PER_SEC );
}

void cSkills::load()
{
	skills.clear();
	skillRanks.clear();

	// Try to get all skills first
	Q_UINT32 i;

	for ( i = 0; i < ALLSKILLS; ++i )
	{
		const cElement* skill = Definitions::instance()->getDefinition( WPDT_SKILL, QString::number( i ) );

		if ( !skill )
			continue;

		stSkill nSkill;

		for ( unsigned int j = 0; j < skill->childCount(); ++j )
		{
			const cElement* node = skill->getChild( j );
			if ( node->name() == "name" )
				nSkill.name = node->text();
			else if ( node->name() == "defname" )
				nSkill.defname = node->text();
			else if ( node->name() == "title" )
				nSkill.title = node->text();
		}

		skills.push_back( nSkill );
	}

	// Load Skill Ranks
	skillRanks = Definitions::instance()->getList( "SKILL_RANKS" );

	// Fill it up to 10 Ranks
	while ( skillRanks.count() < 10 )
		skillRanks.push_back( "" );
}

void cSkills::unload()
{
	skills.clear();
}

// For the Paperdoll
QString cSkills::getSkillTitle( P_CHAR pChar ) const
{
	QString skillTitle;
	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );

	if ( Config::instance()->showSkillTitles() && player && !player->isGM() )
	{
		unsigned short skill = 0;
		unsigned short skillValue = 0;

		for ( int i = 0; i < ALLSKILLS; ++i )
		{
			if ( pChar->skillValue( i ) > skillValue )
			{
				skill = i;
				skillValue = pChar->skillValue( i );
			}
		}

		UI08 title = wpMax<UI08>( 1, ( static_cast<int>( pChar->skillValue( skill ) ) - 300 ) / 100 );

		if ( title >= skillRanks.size() )
		{
			pChar->log( LOG_ERROR, tr("Invalid skill rank information.\n") );
			return skillTitle;
		}

		// Skill not found
		if ( skill >= skills.size() )
		{
			pChar->log( LOG_ERROR, tr( "Skill id out of range: %u.\n" ).arg( skill ) );
			return skillTitle;
		}

		skillTitle = QString( "%1 %2" ).arg( skillRanks[title] ).arg( skills[skill].title );
	}

	return skillTitle;
}

const QString& cSkills::getSkillName( Q_UINT16 skill ) const
{
	if ( skill >= skills.size() )
	{
		Console::instance()->log( LOG_ERROR, tr( "Skill id out of range: %u" ).arg( skill ) );
		return QString::null;
	}

	return skills[skill].name;
}

Q_INT16 cSkills::findSkillByDef( const QString& defname ) const
{
	QString defName = defname.upper();

	unsigned int i;
	for ( i = 0; i < skills.size(); ++i )
	{
		if ( skills[i].defname == defName )
			return i;
	}
	return -1;
}

const QString& cSkills::getSkillDef( Q_UINT16 skill ) const
{
	if ( skill >= skills.size() )
	{
		Console::instance()->log( LOG_ERROR, tr( "Skill id out of range: %u" ).arg( skill ) );
		return QString::null;
	}

	return skills[skill].defname;
}
