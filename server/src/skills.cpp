//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

/* 
*  WOLFPACK Skills
*/

#include "wpdefmanager.h"
#include "basics.h"
#include "itemid.h"
#include "tracking.h"
#include "tilecache.h"
#include "combat.h"
#include "targetrequests.h"
#include "sectors.h"
#include "srvparams.h"
#include "scriptmanager.h"
#include "skills.h"
#include "network.h"
#include "maps.h"
#include "network/uosocket.h"
#include "log.h"
#include "targetrequests.h"
#include "territories.h"
#include "makemenus.h"
#include "npc.h"
#include "console.h"

// System Includes
#include <math.h>

void cSkills::SkillUse( cUOSocket *socket, UINT16 id) // Skill is clicked on the skill list
{
	P_PLAYER pChar = socket->player();

	// No Char no Skill use
	if( !pChar )
		return;

/* TODO: reimplementation of jail system
	if( pChar->cell() > 0 )
	{
		socket->sysMessage( tr( "You are in jail and cant use skills here!" ) );
		return;
	}
*/

	if( pChar->isDead() )
	{
		socket->sysMessage( tr( "You cannot do that as a ghost." ) );
		return;
	}

	if( id != STEALTH )
		pChar->unhide(); // Don't unhide if we're trying to stealth

	pChar->disturbMed(); // Disturb meditation if we're using a skill

	if( pChar->isCasting() )
	{
		socket->sysMessage( tr( "You can't do that while you are casting." ) );
		return;
	}

	if( pChar->skillDelay() > uiCurrentTime && !pChar->isGM() )
	{
		socket->sysMessage( tr( "You must wait a few moments before using another skill." ) );
		return;
	}

	if( pChar->onSkillUse( id ) )
		return;

	cTargetRequest *targetRequest = NULL;
	QString message;
	int s = -1;

	switch( id )
	{
	case STEALING:
		if( !SrvParams->stealingEnabled() )
		{
			socket->sysMessage( tr( "That skill has been disabled." ) );
			return;
		}
		
		message = tr("What do you wish to steal?");
		targetRequest = new cSkStealing;
		break;
	case TRACKING:
		trackingMenu( socket );
		break;
	case FORENSICS:
		message = tr("What corpse do you want to examine?");
		targetRequest = new cSkForensics;
		break;
	case POISONING:
		message = tr("What poison do you want to apply?");
		targetRequest = new cSkPoisoning;
		break;

	case MEDITATION:
		if( !SrvParams->armoraffectmana() )
		{
			socket->sysMessage( tr( "Meditation is disabled." ) );
			return;
		}
        
		Skills->Meditation( socket );
		break;
	case CARTOGRAPHY:
		Skills->Cartography( socket );
		break;
	case CARPENTRY:
		Skills->Carpentry( socket );
		break;
	case BLACKSMITHING:
		Skills->Blacksmithing( socket );
		break;
	case BOWCRAFT:
		Skills->Fletching( socket );
		break;
	case TAILORING:
		Skills->Tailoring( socket );
		break;
	case TINKERING:
		Skills->Tinkering( socket );
		break;
	default:
		socket->sysMessage( tr( "That skill has not been implemented yet." ) );
		return;
	}

	if( targetRequest )
		socket->attachTarget( targetRequest );
	
	if( message )
		pChar->message( message );

	pChar->setSkillDelay( uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC );
}

void cSkills::RandomSteal( cUOSocket* socket, SERIAL victim )
{
	P_PLAYER pChar = socket->player();
	P_CHAR pVictim = FindCharBySerial( victim );

	if( !pVictim || !pChar )
		return;

	if( pVictim->serial() == pChar->serial() )
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
	if( pVictim->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pVictim);
		if( pp->isGMorCounselor() )
			socket->sysMessage( tr( "You can't steal from game masters." ) );
		return;
	}

	if( !pChar->inRange( pVictim, 1 ) )
	{
		socket->sysMessage( tr( "You are too far away to steal from that person." ) );
		return;
	}

	P_ITEM pBackpack = pVictim->getBackpack();

	if( !pBackpack ) 
	{
		socket->sysMessage( tr( "Bad luck, your victim doesn't have a backpack." ) );
		return;
	}

	float maxWeight = (float)QMIN( 1, pChar->skillValue( STEALING ) ); // We can steal max. 10 Stones when we are a GM
	// 1000 Skill == 100 Weight == 10 Stones

	QPtrList< cItem > containment = pBackpack->getContainment();
	UINT32 chance = containment.count();

	P_ITEM pItem = containment.first();
	P_ITEM pToSteal = 0;
	bool sawOkItem = false;

	while( !pToSteal )
	{
		// We have nothing that could be stolen?
		if( !pItem && !sawOkItem )
		{
			socket->sysMessage( tr( "Your victim posesses nothing you could steal." ) );
			return;
		}
		// Jump back to the beginning
		else if( !pItem && sawOkItem )
		{
			pItem = containment.first();
		}

		// Check if our chance becomes true (no spellbooks!)
		if( pItem->totalweight() <= maxWeight && !pItem->isLockedDown() && !pItem->newbie() && pItem->type() != 9 )
		{
			sawOkItem = true; // We have items that could be stolen (just in case we reach the end of our list)

			// We have the chance of 1/chance that we reached our desired item
			if( RandomNum( 1, chance ) == chance )
			{
				pToSteal = pItem;
				break;
			}
		}

		pItem = containment.next();
	}

	socket->sysMessage( tr( "You reach into %1's backpack and try to steal something..." ).arg( pVictim->name() ) );

	// The success of our Theft depends on the weight of the stolen item
	bool success = pChar->checkSkill( STEALING, 0, pToSteal->weight() );
	bool caught = false;

	if( success )
	{
		socket->sysMessage( tr( "You successfully steal %1." ).arg( pToSteal->getName() ) );
		P_ITEM pPack = pChar->getBackpack();
		pPack->addItem( pToSteal );
		// Update item onyl if still existent
		if( !pToSteal->free )
			pToSteal->update();

		caught = pChar->skillValue( STEALING ) < rand() % 1001;
	}
	else
	{
		socket->sysMessage( tr( "You fail to steal the item." ) );

		// 1 in 5 Chance if we failed
		caught = RandomNum( 1, 5 ) == 1;
	}

	// Did we get caught?
	if( caught )
	{
		socket->sysMessage( tr( "You have been cought!" ) );

		// Human non red NPCs need special handling
		if( pVictim->objectType() == enNPC && pVictim->isInnocent() && pVictim->isHuman() )
		{
			P_NPC pn = dynamic_cast<P_NPC>(pVictim);
			pVictim->talk( tr( "Guards! A thief is amoung us!" ), -1, 0x09 );
			if( pVictim->region() && pVictim->region()->isGuarded() )
				pn->callGuards();
		}
		
		if (pVictim->notoriety(pChar) == 0x01) {
			pChar->makeCriminal();
		}

		// Our Victim always notices it.
		if (pVictim->objectType() == enPlayer) {
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pVictim);
			if( pp->socket() )
				pp->socket()->showSpeech( pChar, tr( "You notice %1 trying to steal %2 from you." ).arg( pChar->name() ).arg( pToSteal->getName( true ) ) );
		}

		QString message = tr( "You notice %1 trying to steal %2 from %3." ).arg( pChar->name() ).arg( pItem->getName() ).arg( pVictim->name() );	

		for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
		{
			// Everyone within 7 Tiles notices us
			if( mSock != socket && mSock->player() && mSock->player()->serial() != pVictim->serial() && mSock->player()->inRange( pChar, 7 ) ) 
				mSock->showSpeech( pChar, message );
		}
		
	} 
}

void cSkills::Meditation( cUOSocket *socket )
{
	P_CHAR pc_currchar = socket->player();

	if (pc_currchar->isAtWar())
	{
		socket->sysMessage( tr("Your mind is too busy with the war thoughts.") );
		return;
	}
	if (pc_currchar->getWeapon() || pc_currchar->getShield())
	{
		socket->sysMessage( tr("You cannot meditate with a weapon or shield equipped!"));
		pc_currchar->setMeditating( false );
		return;
	}
	else if ( pc_currchar->mana() == pc_currchar->intelligence() )
	{
		socket->sysMessage( tr("You are at peace."));
		pc_currchar->setMeditating( false );
		return;
	}
	else if (!pc_currchar->checkSkill( MEDITATION, 0, 1000))
	{
		socket->sysMessage( tr("You cannot focus your concentration."));
		pc_currchar->setMeditating( false );
		return;
	}
	else
	{
		socket->sysMessage( tr("You enter a meditative trance."));
		pc_currchar->setMeditating( true );
		pc_currchar->soundEffect(0x00f9, false);
		return;
	}
}

void cSkills::Snooping( P_PLAYER player, P_ITEM container )
{
	cUOSocket *socket = player->socket();
	
	if( !socket )
		return;

	P_CHAR pc_owner = container->getOutmostChar();
	P_PLAYER pp_owner = dynamic_cast<P_PLAYER>(pc_owner);

	if( pp_owner && pp_owner->isGMorCounselor() )
	{
		pp_owner->message( tr( "%1 is trying to snoop in your pack" ).arg( player->name() ) );
		socket->sysMessage( tr( "You can't peek into that container or you'll be jailed." ) );
		return;
	}
	else if( player->checkSkill( SNOOPING, 0, 1000 ) )
	{
		socket->sendContainer( container );
		socket->sysMessage( tr( "You successfully peek into that container." ) );
	}
	else
	{
		socket->sysMessage( tr( "You failed to peek into that container." ) );

		if( !pp_owner ) // is NPC ?
			pc_owner->talk( tr( "Art thou attempting to disturb my privacy?" ) );
		else
			pp_owner->message( tr( "You notice %1 trying to peek into your pack!" ).arg( player->name() ) );
	}

	

//	SetTimerSec(player->objectdelay(), SrvParams->objectDelay()+SrvParams->snoopdelay());
	player->setObjectDelay( SetTimerSec(player->objectDelay(), SrvParams->objectDelay()+SrvParams->snoopdelay()) );
}

void cSkills::Cartography( cUOSocket* socket )
{
	MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_CARTOGRAPHY" );
}

////////////////////
// name:	Carpentry()
// history:	unknown, Duke, 25.05.2000, rewritten for 13.x sereg, 16.08.2002
// purpose:	sets up appropriate Makemenu when player dclick on carpentry tool
//			

void cSkills::Carpentry( cUOSocket* socket )
{
	MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_CARPENTRY" );
}


void cSkills::Fletching( cUOSocket* socket )
{
	MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_FLETCHING" );
}

void cSkills::Tailoring( cUOSocket* socket )
{
	MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_TAILORING" );
}

void cSkills::Blacksmithing( cUOSocket* socket )
{
	P_CHAR pc = socket->player();
	bool foundAnvil = false;
	
	RegionIterator4Items rIter( pc->pos() );
	for( rIter.Begin(); !rIter.atEnd(); rIter++ )
	{
		P_ITEM pi = rIter.GetData();

		if( IsAnvil( pi->id() ) && pc->inRange( pi, 3 ) )
		{
			foundAnvil = true;
			break;
		}
	}

	if( !foundAnvil )
	{
		StaticsIterator sIter = Map->staticsIterator( pc->pos(), false );
		while( !sIter.atEnd() )
		{
			if( IsAnvil( sIter->itemid ) )
			{
				foundAnvil = true;
				break;
			}
			sIter++;
		}
	}

	if( !foundAnvil )
	{
		socket->sysMessage( tr( "You must stand in range of an anvil!" ) );
		return;
	}

	MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_BLACKSMITHING" );
}

void cSkills::Tinkering( cUOSocket* socket )
{
	MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_TINKERING" );
}

void cSkills::load()
{
	// Try to get all skills first
	UINT32 i;
	
	for (i = 0; i < ALLSKILLS; ++i) {
		const cElement *skill = DefManager->getDefinition(WPDT_SKILL, QString::number(i));

		if (!skill) {
			continue;
		}

		stSkill nSkill;

		for (unsigned int j = 0; j < skill->childCount(); ++j) {
			const cElement *node = skill->getChild( j );
			if( node->name() == "name" ) {
				nSkill.name = node->text();
			} else if( node->name() == "defname" ) {
				nSkill.defname = node->text();
			} else if( node->name() == "title" ) {
				nSkill.title = node->text();
			}
		}

		skills.push_back( nSkill );
	}

	// Load Skill Ranks
	skillRanks = DefManager->getList("SKILL_RANKS");

	// Fill it up to 10 Ranks
	while (skillRanks.count() < 10) {
		skillRanks.push_back("");
	}
}

void cSkills::unload()
{
	skills.clear();
}

// For the Paperdoll
QString cSkills::getSkillTitle(P_CHAR pChar) const {
	QString skillTitle("");
	P_PLAYER player = dynamic_cast<P_PLAYER>(pChar);

	if (SrvParams->showSkillTitles() && player && !player->isGM()) {
		unsigned short skill = 0;
		unsigned short skillValue = 0;
		
		for (int i = 0; i < ALLSKILLS; ++i) {
			if (pChar->skillValue(i) > skillValue) {
                skill = i;
				skillValue = pChar->skillValue(i);
			}
		}
		
		unsigned char title = std::max(1, ((int)pChar->skillValue(skill) - 300) / 100);

		if (title >= skillRanks.size()) {
			pChar->log(LOG_ERROR, "Invalid skill rank information.\n");
			return skillTitle;
		}

		// Skill not found
		if (skill >= skills.size()) {
			pChar->log(LOG_ERROR, QString("Skill id out of range: %u.\n").arg(skill));
			return skillTitle;
		}

		skillTitle = QString("%1 %2").arg(skillRanks[title]).arg(skills[skill].title);
	}

	return skillTitle;
}

const QString &cSkills::getSkillName( UINT16 skill ) const
{
	if( skill >= skills.size() )
	{
		Console::instance()->log( LOG_ERROR, QString( "Skill id out of range: %u" ).arg( skill ) );
		return QString::null;
	}

	return skills[skill].name;	
}

INT16 cSkills::findSkillByDef( const QString &defname ) const
{
	QString defName = defname.upper();

	unsigned int i;
	for( i = 0; i < skills.size(); ++i )
	{
		if( skills[i].defname == defName )
			return i;
	}
	return -1;
}

const QString &cSkills::getSkillDef( UINT16 skill ) const
{
	if( skill >= skills.size() )
	{
		Console::instance()->log( LOG_ERROR, QString( "Skill id out of range: %u" ).arg( skill ) );
		return QString::null;
	}
	
	return skills[skill].defname;
}
