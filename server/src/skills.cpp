//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

/* 
*  WOLFPACK Skills
*/

#include "wolfpack.h"
#include "wpdefmanager.h"
#include "basics.h"
#include "itemid.h"
#include "guildstones.h"
#include "tracking.h"
#include "tilecache.h"
#include "combat.h"
#include "targetrequests.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "scriptmanager.h"
#include "skills.h"
#include "network.h"
#include "maps.h"
#include "network/uosocket.h"
#include "targetrequests.h"
#include "territories.h"
#include "makemenus.h"
#include "npc.h"

// System Includes
#include <math.h>

#undef DBGFILE
#define DBGFILE "skills.cpp"
#include "debug.h"

/*
List of implemented skills:
Arms Lore

*/

void cSkills::Hide( cUOSocket *socket ) 
{ 
	P_PLAYER pChar = socket->player();

	if( !pChar )
		return;

	P_CHAR aChar = FindCharBySerial( pChar->attackerSerial() );
	if( aChar && aChar->inRange( pChar, pChar->visualRange() ) )
	{
		pChar->message( tr( "You cannot hide while fighting." ) );
		return; 
	}
	
	if( pChar->isHidden() ) 
	{ 
		pChar->message( tr( "You are already hidden." ) );
		return; 
	} 
	
	if( !pChar->checkSkill( HIDING, 0, 1000 ) ) 
	{ 
		pChar->message( "You are unable to hide here." );
		return; 
	} 
	
	pChar->message( tr( "You have hidden yourself well." ) );
	pChar->setHidden( 1 );
	pChar->resend(); // Remove + Resend (GMs etc.)
	
	// Resend us to ourself
	if( pChar->socket() )
		pChar->socket()->updatePlayer();
}

void cSkills::Stealth( cUOSocket *socket )
{
	P_PLAYER pChar = socket->player();

	if( !pChar )
		return;

	if( !pChar->isHidden() )
	{
		pChar->message( tr( "You must hide first." ) );
		return;
	}

	if( pChar->skillValue( HIDING ) < 700 )
	{
		socket->sysMessage( tr( "You are not hidden well enough. Become better at hiding." ) );
		return;
	}

	if( !pChar->checkSkill( STEALTH, 0, 1000 ) ) 
	{
		socket->sysMessage( tr( "You fail to stealth with your environment." ) );
		pChar->unhide();
		return;
	}

	socket->sysMessage( tr( "You can move %1 steps unseen" ).arg( SrvParams->maxStealthSteps() ) );
	pChar->setStealthedSteps( 0 );
}

void cSkills::PeaceMaking(cUOSocket* socket)
{
	int res1, res2, j;
	P_ITEM p_inst = Skills->GetInstrument(socket);
	if (p_inst == NULL) 
	{
		socket->sysMessage( tr( "You do not have an instrument to play on!" ) );
		return;
	}
	P_CHAR pc_currchar = socket->player();
	res1=pc_currchar->checkSkill( PEACEMAKING, 0, 1000);
	res2=pc_currchar->checkSkill( MUSICIANSHIP, 0, 1000);
	if (res1 && res2)
	{
		Skills->PlayInstrumentWell(socket, p_inst);
		socket->sysMessage( tr( "You play your hypnotic music, stopping the battle.") );
		
		RegionIterator4Chars ri(pc_currchar->pos(), VISRANGE);
		for (ri.Begin(); !ri.atEnd(); ri++)
		{
			P_CHAR mapchar = ri.GetData();
			if( mapchar && mapchar->isAtWar() )
			{
				j = calcSocketFromChar(mapchar);
				if( mapchar->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>(mapchar);
					if( pp->socket() )
						pp->socket()->sysMessage( tr("You hear some lovely music, and forget about fighting.") );
				}
				else
				{
					dynamic_cast<P_NPC>(mapchar)->toggleCombat();
				}

				mapchar->setCombatTarget( INVALID_SERIAL );
				mapchar->setAttackerSerial(INVALID_SERIAL);
				mapchar->setAttackFirst(false);
			}
		}
	} 
	else 
	{
		Skills->PlayInstrumentPoor(socket, p_inst);
		socket->sysMessage( tr("You attempt to calm everyone, but fail.") );
	}
}

void cSkills::PlayInstrumentWell(cUOSocket* socket, P_ITEM pi)
{
	P_CHAR pc_currchar = socket->player();
	switch(pi->id())
	{
	case 0x0E9C:	pc_currchar->soundEffect( 0x0038 );	break;
	case 0x0E9D:
	case 0x0E9E:	pc_currchar->soundEffect( 0x0052 );	break;
	case 0x0EB1:
	case 0x0EB2:	pc_currchar->soundEffect( 0x0045 );	break;
	case 0x0EB3:
	case 0x0EB4:	pc_currchar->soundEffect( 0x004C );	break;
	}
}

void cSkills::PlayInstrumentPoor(cUOSocket* socket, P_ITEM pi)
{
	P_CHAR pc_currchar = socket->player();
	switch(pi->id())
	{
	case 0x0E9C:	pc_currchar->soundEffect( 0x0039);	break;
	case 0x0E9D:
	case 0x0E9E:	pc_currchar->soundEffect( 0x0053);	break;
	case 0x0EB1:
	case 0x0EB2:	pc_currchar->soundEffect( 0x0046);	break;
	case 0x0EB3:
	case 0x0EB4:	pc_currchar->soundEffect( 0x004D);	break;
	}
}

P_ITEM cSkills::GetInstrument(cUOSocket* socket)
{
	P_CHAR pc_currchar = socket->player();

	unsigned int ci = 0;
	P_ITEM pBackpack = pc_currchar->getBackpack();
	cItem::ContainerContent container = pBackpack->content();
	cItem::ContainerContent::iterator it = container.begin();
	for (; it != container.end(); ++it )
	{
		P_ITEM pi = *it;
		if ( IsInstrument(pi->id()) )
		{
			return pi;
		}
	}
	return 0;
}

/////////////////////////
// name:	BottleTarget
// history: unknown, revamped by Duke,23.04.2000
// Purpose: Uses the targeted potion bottle *outside* the backpack to
//			pour in the potion from the mortar
//
void cSkills::BottleTarget(int s)
{
/*	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (!pi || pi->isLockedDown()) return;	// Ripper

	if (pi->id()==0x0F0E)	// an empty potion bottle ?
	{
		pi->ReduceAmount(1);

		P_ITEM mortar = FindItemBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
		if(mortar == NULL) return;
		if (mortar->type() == 17) 
		{
			pc_currchar->emote( tr("*%1 pours the completed potion into a bottle.*").arg(pc_currchar->name));
			Skills->PotionToBottle(pc_currchar, mortar);
		}
	}
	else
		sysmessage(s,"This is not an appropriate container for a potion.");*/
}

/////////////////////////
// name:	PotionToBottle
// history: unknown, revamped by Duke,23.04.2000
// Purpose: this really creates the potion
//
void cSkills::PotionToBottle(P_PLAYER pc, P_ITEM pi_mortar)
{
	unsigned char id1,id2;
	char pn[50];

	if ( pc == NULL ) return;

	switch((10*pi_mortar->more1())+pi_mortar->more2())
	{
	case 11: id1=0x0F;id2=0x08;strcpy(pn, "an agility");				break;
	case 12: id1=0x0F;id2=0x08;strcpy(pn, "a greater agility");			break;
	case 21: id1=0x0F;id2=0x07;strcpy(pn, "a lesser cure");				break;
	case 22: id1=0x0F;id2=0x07;strcpy(pn, "a cure");					break;
	case 23: id1=0x0F;id2=0x07;strcpy(pn, "a greater cure");			break;
	case 31: id1=0x0F;id2=0x0D;strcpy(pn, "a lesser explosion");		break;
	case 32: id1=0x0F;id2=0x0D;strcpy(pn, "an explosion");				break;
	case 33: id1=0x0F;id2=0x0D;strcpy(pn, "a greater explosion");		break;
	case 41: id1=0x0F;id2=0x0C;strcpy(pn, "a lesser heal");				break;
	case 42: id1=0x0F;id2=0x0C;strcpy(pn, "a heal");					break;
	case 43: id1=0x0F;id2=0x0C;strcpy(pn, "a greater heal");			break;
	case 51: id1=0x0F;id2=0x06;strcpy(pn, "a night sight");				break;
	case 61: id1=0x0F;id2=0x0A;strcpy(pn, "a lesser poison");			break;
	case 62: id1=0x0F;id2=0x0A;strcpy(pn, "a poison");					break;
	case 63: id1=0x0F;id2=0x0A;strcpy(pn, "a greater poison");			break;
	case 64: id1=0x0F;id2=0x0A;strcpy(pn, "a deadly poison");			break;
	case 71: id1=0x0F;id2=0x0B;strcpy(pn, "a refresh");					break;
	case 72: id1=0x0F;id2=0x0B;strcpy(pn, "a total refreshment");		break;
	case 81: id1=0x0F;id2=0x09;strcpy(pn, "a strength");				break;
	case 82: id1=0x0F;id2=0x09;strcpy(pn, "a greater strength");		break;
	default:
		LogError("switch reached default");
		return;
	}
	
	P_ITEM pi_potion = 0;//Items->SpawnItem(pc, 1,"#",0, id1, id2,0,1,0);
	if (pi_potion == NULL) 
		return;
	
	pi_potion->setName( QString( "%1 potion" ).arg( pn ) );
	pi_potion->setType( 19 );
	pi_potion->setMoreX(pi_mortar->morex());
	pi_potion->setMoreY(pi_mortar->more1());
	pi_potion->setMoreZ(pi_mortar->more2());
	
	// the remainder of this function NOT (yet) revamped by Duke !
	
	// Addon for Storing creator NAME and SKILLUSED by Magius(CHE) §
	if(!pc->isGM())
	{
		pi_potion->setCreator(pc->name()); // Magius(CHE) - Memorize Name of the creator
		if (pc->skillValue(ALCHEMY)>950) pi_potion->setMadeWith(ALCHEMY+1); // Memorize Skill used - Magius(CHE)
		else pi_potion->setMadeWith(0-ALCHEMY-1); // Memorize Skill used - Magius(CHE)
	} else {
		pi_potion->setCreator("");
		pi_potion->setMadeWith(0);
	}
	
	pi_potion->update();
	pi_mortar->setType( 0 );
	// items[i].weight=100; // Ripper 11-25-99
	// AntiChrist NOTE: please! use the HARDITEMS.SCP...
	// the settings used in that script are used EVERY TIME we have an item created via
	// code ( and not via script )...so we simply can add the string:
	// WEIGHT 100
	// in the "potion section" of that file, and EVERY TIME the potions are created via scripts
	// they have those settings! :) that's easy isn't it? =P
	
	return;
}

void cSkills::SpiritSpeak(int s) // spirit speak time, on a base of 30 seconds + skill[SPIRITSPEAK]/50 + INT
{
	//	Unsure if spirit speaking should they attempt again?
	//	Suggestion: If they attempt the skill and the timer is !0 do not have it raise the skill
	
	/*if(!Skills->CheckSkill(currchar[s], SPIRITSPEAK, 0, 1000))
	{
		sysmessage(s,"You fail your attempt at contacting the netherworld.");
		return;
	}
	
	impaction(s,0x11);			// I heard there is no action...but I decided to add one
	soundeffect(s,0x02,0x4A);	// only get the sound if you are successful
	sysmessage(s,"You establish a connection to the netherworld.");
	currchar->setSpritSpeakTimer( SetTimerSec(currchar[s]->spiritspeaktimer,SrvParams->spiritspeaktimer()+currchar[s]->in) );*/
}

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
	case ARMSLORE:
		message = tr("What item do you wish to get information about?");
		targetRequest = new cSkArmsLore;
		break;
	case ITEMID:
		message = tr("What do you wish to appraise and identify?");
		targetRequest = new cSkItemID;
		break;
	case EVALUATINGINTEL:
		message = tr("What would you like to evaluate?");
		targetRequest = new cSkIntEval;
		break;
	case TAMING:
		message = tr("Tame which animal?");
		targetRequest = new cSkTame;
		break;
	case HIDING:
		Skills->Hide( socket );
		break;
	case STEALTH:
		Skills->Stealth( socket );
		break;
	case DETECTINGHIDDEN:
		message = tr("Where do you wish to search for hidden characters?");
		targetRequest = new cSkDetectHidden;
		break;
	case PEACEMAKING:
		Skills->PeaceMaking(socket);
		break;
	case PROVOCATION:
		message = tr("Whom do you wish to incite?");
		targetRequest = new cSkProvocation;
		break;
	case DISCORDANCE:
		message = tr("Whom do you wish to entice?");
		//target(s, 0, 1, 0, 81, );
		break;
	case SPIRITSPEAK:
		Skills->SpiritSpeak(s);
		break;
	case STEALING:
		if( !SrvParams->stealingEnabled() )
		{
			socket->sysMessage( tr( "That skill has been disabled." ) );
			return;
		}
		
		message = tr("What do you wish to steal?");
		targetRequest = new cSkStealing;
		break;
	case INSCRIPTION:
		message = tr("What do you wish to place a spell on?");
		//target(s, 0, 1, 0, 160, );
		break;
	case TRACKING:
		trackingMenu( socket );
		break;
	case BEGGING:
		message = tr( "Whom do you wish to annoy?" );
		targetRequest = new cSkBegging;
		break;
	case FORENSICS:
		message = tr("What corpse do you want to examine?");
		targetRequest = new cSkForensics;
		break;
	case POISONING:
		message = tr("What poison do you want to apply?");
		targetRequest = new cSkPoisoning;
		break;

	case TASTEID:
		message = tr("What do you want to taste?");
        targetRequest = new cSkTasteID;
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

	UINT16 maxWeight = QMIN( 1, pChar->skillValue( STEALING ) / 10 ); // We can steal max. 10 Stones when we are a GM
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
	bool success = pChar->checkSkill( STEALING, 0, pToSteal->weight() * 10 );
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
		
		if( pVictim->isInnocent() && pChar->attackerSerial() != pVictim->serial() && GuildCompare( pChar, pVictim ) == 0)
			pChar->makeCriminal(); // Blue and not attacker and not guild

		// Our Victim always notices it.
		if( pVictim->objectType() == enPlayer )
		{
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

///////////////////////
// name:	TellScroll
// history:	unknown, modified by LB & AntiChrist, 
//			partially revamped by Duke, 8 April 2000
// Purpose:	callback function for inscription menue
//			This is the point where control is returned after the user
//			selected a spell to inscribe/engrave from the gump menu.
//
void TellScroll( char *menu_name, int s, long snum )
{
/*	P_CHAR pc_currchar = currchar[s];
	unsigned cir,spl;
	int part;

	if(snum<=0) return;				// bad spell selction

	P_ITEM pi = FindItemBySerial(pc_currchar->lastTarget);

	cir=(int)((snum-800)/10);		// snum holds the circle/spell as used in inscribe.gmp
	spl=(((snum-800)-(cir*10))+1);	// i.e. 800 + 1-based circle*10 + zero-based spell
									// snum is also equals the item # in items.scp of the scrool to be created !
	P_ITEM pBackpack = Packitem(pc_currchar);
	if (pBackpack == NULL) return;
	
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)		// find the spellbook
	{
		P_ITEM pb = iterItems.GetData();
		if (pb->type()==9 && (pb->contserial==pBackpack->serial ||
			(pb->layer()==1 && pc_currchar->Wears(pb))))
		{
			if (!Magic->CheckBook( cir, spl-1, pb))
			{
				sysmessage(s,"You don't have this spell in your spell book!");
				return;
			}
		}
	}
	
	int num = (8*(cir-1)) + spl;	// circle & spell combined to a zero-based index
	
	Magic->Action4Spell(s,num);
	Magic->SpeakMantra4Spell(pc_currchar,num);
	
	if(!Magic->CheckReagents(pc_currchar, num)
		|| !Magic->CheckMana(pc_currchar, num))
	{
		Magic->SpellFail(s);
		return;
	}
	Magic->SubtractMana4Spell(pc_currchar, num);
	
	if (pi->id()==0x0E34)  //is it a scroll?
	{
		itemmake[s].Mat1id=0x0E34; 
		itemmake[s].needs=1; 
		itemmake[s].has = getamount(pc_currchar, 0x0E34); 
		itemmake[s].minskill=(cir-1)*100;	//set range values based on scroll level
		itemmake[s].maxskill=(cir+2)*100;

		Magic->DelReagents(pc_currchar, num);
		
		Skills->MakeMenuTarget(s,snum,INSCRIPTION); //put it in your pack
	}
	else if ((pi->att>0)||(pi->def>0)||(pi->hidamage())) //or is it an item?
	{
		part=0;
		switch(cir)
		{
		case 1:	part = CheckThreeSkills(s,  11, 401);break;
		case 2:	part = CheckThreeSkills(s,  61, 501);break;
		case 3:	part = CheckThreeSkills(s, 161, 601);break;
		case 4:	part = CheckThreeSkills(s, 261, 701);break;
		case 5:	part = CheckThreeSkills(s, 361, 801);break;
		case 6:	part = CheckThreeSkills(s, 461, 901);break;
		case 7:	part = CheckThreeSkills(s, 661,1101);break;
		case 8:	part = CheckThreeSkills(s, 761,1201);break;
		default:
			LogError("switch reached default");
			return;
		}
		
		if( part < 3 )		// failure !
		{
			switch(part)
			{
			case 0:
				sysmessage(s,"Your hand jerks and you punch a hole in the item");
				pi->setHp( pi->hp() - 3 );
				break;
			case 1:
				sysmessage(s,"Your hand slips and you dent the item");
				pi->setHp( pi->hp() - 2 );
				break;
			case 2:
				sysmessage(s,"Your hand cramps and you scratch the item");
				pi->setHp( pi->hp() - 1 );
				break;
			}
			if( pi->hp() < 1 )
				pi->setHp( 1 );
		}
		else				// success !
		{
			if (!( pi->morez == 0 ||(pi->morex == cir && pi->morey == spl))  )
				sysmessage(s,"This item already has a spell!");

			else if ( pi->morez >= (9 - pi->morex)*2)
				sysmessage(s,"Item at max charges!");

			else
			{
				Skills->EngraveAction(s, pi, cir, spl);	// check mana & set name

				if (!(pi->morex == cir && pi->morey == spl))	// not THIS spell
				{
					pi->setType2( pi->type() ); //kept type of item for returning to this type when item remain no charge 
					pi->setType( 15 );  //make it magical
					pi->morex=cir;//spell circle
					pi->morey=spl;//spell number
					sysmessage(s,"Item successfully Engraved");
				}
				else
					sysmessage(s,"You added one more charge to the Item");

				pi->morez++;  // charges
				sysmessage(s,"Item successfully Engraved");
			}
		}
	}//else if*/
}

void cSkills::Meditation( cUOSocket *socket )
{
	P_CHAR pc_currchar = socket->player();

	if (pc_currchar->isAtWar())
	{
		socket->sysMessage( tr("Your mind is too busy with the war thoughts.") );
		return;
	}
	if (Skills->GetAntiMagicalArmorDefence(pc_currchar)>15) // blackwind armor affect fix
	{
		socket->sysMessage( tr("Regenerative forces cannot penetrate your armor."));
		pc_currchar->setMeditating(false);
		return;
	}
	else if (pc_currchar->getWeapon() || pc_currchar->getShield())
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

// If you are a ghost and attack a player, you can PERSECUTE him
// and his mana decreases each time you try to persecute him
// decrease=3+(your int/10)
void cSkills::Persecute ( cUOSocket* socket )
{
	P_PLAYER pc_currchar = socket->player();

	if( !pc_currchar )
		return;

	P_CHAR target = FindCharBySerial(pc_currchar->combatTarget());
	P_PLAYER pt = dynamic_cast<P_PLAYER>(target);

	if( pt && pt->isGM() )
		return;

	int decrease = ( pc_currchar->intelligence() / 10 ) + 3;

	if((pc_currchar->skillDelay()<=uiCurrentTime) || pc_currchar->isGM())
	{
		if(((rand()%20)+pc_currchar->intelligence())>45) //not always
		{
			if( target->mana() <= decrease )
				target->setMana(0);
			else 
				target->setMana(target->mana() - decrease);
			socket->sysMessage(tr("Your spiritual forces disturb the enemy!"));

			if ( pt && pt->socket() )
			{
				pt->socket()->updateMana( target );
				pt->socket()->sysMessage( tr( "A damned soul is disturbing your mind!" ) );
			}

			pc_currchar->setSkillDelay( uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC );

			QString message = tr( "*You see %1 is being persecuted by a ghost*" ).arg( target->name() );
			cUOSocket* ptsocket = pt ? pt->socket() : NULL;
							
			for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
			{
				if( socket->inRange( s ) && s != socket && ptsocket != socket ) 
					socket->showSpeech( target, message, 0x26, 3, 0x02 );
			}
		} 
		else
		{
			socket->sysMessage( tr( "Your mind is not strong enough to disturb the enemy." ) );
		}
	} 
	else
	{
		socket->sysMessage( tr( "You are unable to persecute him now...rest a little..." ) );
	}
}

int cSkills::GetAntiMagicalArmorDefence(P_CHAR pc)
{
	int ar = 0;

	if ( pc->isHuman() )
	{
		
		unsigned int ci = 0;
		P_ITEM pi;
		cBaseChar::ItemContainer container(pc->content());
		cBaseChar::ItemContainer::const_iterator it (container.begin());
		cBaseChar::ItemContainer::const_iterator end(container.end());
		for (; it != end; ++it )
		{
			pi = *it;
			if (pi->layer() > 1 && pi->layer() < 25)
			{
				if (!(pi->name().contains("leather") || pi->name().contains("magic") ||
					  pi->name().contains("boot")    || pi->name().contains("mask")  ))
					ar += pi->def();
			}
		}
	}
	return ar;
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

/*
	By Polygon:
	Attempt to decipher a tattered treasure map
	Called when double-clicked such a map
*/

void cSkills::Decipher(P_ITEM tmap, int s)
{
/*	P_CHAR pc_currchar = currchar[s];
	char sect[500];			// Needed for script search
	int regtouse;			// Stores the region-number of the TH-region
	int i;					// Loop variable
	int btlx, btly, blrx, blry; // Stores the borders of the tresure region (topleft x-y, lowright x-y)
	int tlx, tly, lrx, lry;		// Stores the map borders
	int x, y;					// Stores the final treasure location

	if(pc_currchar->skilldelay<=uiCurrentTime || pc_currchar->isGM())	// Char doin something?
	{
		if (CheckSkill(pc_currchar, CARTOGRAPHY, tmap->morey * 10, 1000))	// Is the char skilled enaugh to decipher the map
		{
			P_ITEM nmap = Items->SpawnItemBackpack2(s, "70025", 0);
			if (nmap == NULL)
			{
				LogWarning("bad script item # 70025(Item Not found).");
				return;	//invalid script item
			} 
			char temp[256];
			sprintf(temp, "a deciphered lvl.%d treasure map", tmap->morez);	// Give it the correct name
			nmap->setName( temp );
			nmap->morez = tmap->morez;				// Give it the correct level
			nmap->creator = pc_currchar->name;	// Store the creator
			Script *rscript=i_scripts[regions_script];	// Region script
			if (!rscript->Open())
			{
				LogWarning("Treasure hunting cSkills::Decipher : Cannot open regions-script");
				return;
			}
			sprintf(sect, "TREASURE%i", nmap->morez);
			if (!rscript->find(sect)) 
			{
				rscript->Close();
				LogWarningVar("Treasure hunting cSkills::Decipher : Unable to find 'SECTION TREASURE%X' in regions-script", nmap->morez);
				return;
			}
			rscript->NextLine();				// Get the number of areas
			regtouse = rand()%str2num(script1);	// Select a random one
			for (i = 0; i < regtouse; i++)		// Skip the ones before the correct one
			{
				rscript->NextLine();
				rscript->NextLine();
				rscript->NextLine();
				rscript->NextLine();
			}
			rscript->NextLine();				// Get the treasure region borders
			btlx = str2num(script1);
			rscript->NextLine();
			btly = str2num(script1);
			rscript->NextLine();
			blrx = str2num(script1);
			rscript->NextLine();
			blry = str2num(script1);
			rscript->Close();					// Close the script
			if ((btlx < 0) || (btly < 0) || (blrx > 0x13FF) || (blry > 0x0FFF))	// Valid region?
			{
				sprintf(sect, "Treasure Hunting cSkills::Decipher : Invalid region borders for lvl.%d , region %d", nmap->morez, regtouse+1);	// Give out detailed warning :D
				LogWarning(sect);
				return;
			}
			x = btlx + (rand()%(blrx-btlx));	// Generate treasure location
			y = btly + (rand()%(blry-btly));
			tlx = x - 250;		// Generate map borders
			tly = y - 250;
			lrx = x + 250;
			lry = y + 250;
			// Check if we are over the borders and correct errors
			if (tlx < 0)	// Too far left?
			{
				lrx -= tlx;	// Add the stuff too far left to the right border (tlx is neg. so - and - gets + ;)
				tlx = 0;	// Set tlx to correct value
			}
			else if (lrx > 0x13FF) // Too far right?
			{
				tlx -= lrx - 0x13FF;	// Subtract what is to much from the left border
				lrx = 0x13FF;	// Set lrx to correct value
			}
			if (tly < 0)	// Too far top?
			{
				lry -= tly;	// Add the stuff too far top to the bottom border (tly is neg. so - and - gets + ;)
				tly = 0;	// Set tly to correct value
			}
			else if (lry > 0x0FFF) // Too far bottom?
			{
				tly -= lry - 0x0FFF;	// Subtract what is to much from the top border
				lry = 0x0FFF;	// Set lry to correct value
			}
			nmap->more1 = tlx>>8;	// Store the map extends
			nmap->more2 = tlx%256;
			nmap->more3 = tly>>8;
			nmap->more4 = tly%256;
			nmap->setMoreb1( lrx>>8 );
			nmap->setMoreb2( lrx%256 );
			nmap->setMoreb3( lry>>8 );
			nmap->setMoreb4( lry%256 );
			nmap->morex = x;		// Store the treasure's location
			nmap->morey = y;
			Items->DeleItem(tmap);	// Delete the tattered map
		}
		else
			sysmessage(s, "You fail to decipher the map");		// Nope :P
		// Set the skill delay, no matter if it was a success or not
		pc_currchar->setSkillDelay( SetTimerSec(pc_currchar->skilldelay, SrvParams->skillDelay()) );
		soundeffect(s, 0x02, 0x49);	// Do some inscription sound regardless of success or failure
		sysmessage(s, "You put the deciphered tresure map in your pack");	// YAY
	}
	else
		sysmessage(s, "You must wait to perform another action");	// wait a bit
*/
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
	
	for( i = 0; i < ALLSKILLS; ++i )
	{
		const cElement *skill = DefManager->getDefinition( WPDT_SKILL, QString::number( i ) );

		if( !skill )
			continue;

		stSkill nSkill;

		for( unsigned int j = 0; j < skill->childCount(); ++j )
		{
			const cElement *node = skill->getChild( j );

			if( node->name() == "str" )
			{
				nSkill.strength = node->text().toInt();
			}
			else if( node->name() == "dex" )
			{
				nSkill.dexterity = node->text().toInt();
			}
			else if( node->name() == "int" )
			{
				nSkill.intelligence = node->text().toInt();
			}
			else if( node->name() == "name" )
			{
				nSkill.name = node->text();
			}
			else if( node->name() == "defname" )
			{
				nSkill.defname = node->text();
			}
			else if( node->name() == "title" )
			{
				nSkill.title = node->text();
			}
			// Advancement section
			else if( node->name() == "advancement" )
			{
				stAdvancement advancement;

				advancement.base = node->getAttribute( "base", "0" ).toInt();
				advancement.failure = node->getAttribute( "failure", "0" ).toInt();
				advancement.success = node->getAttribute( "success", "0" ).toInt();

				nSkill.advancement.push_back( advancement );
			}
		}

		skills.push_back( nSkill );
	}

	// Load Strength/Dexterity/Intelligence Advancement tables
	const cElement *stat = DefManager->getDefinition( WPDT_SKILL, "strength" );
	
	if( !stat )
	{
		clConsole.log( LOG_ERROR, "Couldn't find strength advancement table." );		
	}
	else
	{
		for( INT32 i = 0; i < stat->childCount(); ++i )
		{
			const cElement *elem = stat->getChild( i );

			if( elem->name() == "advancement" )
			{
				stAdvancement advancement;

				advancement.base = elem->getAttribute( "base", "0" ).toInt();
				advancement.failure = elem->getAttribute( "failure", "0" ).toInt();
				advancement.success = elem->getAttribute( "success", "0" ).toInt();

				advStrength.push_back( advancement );
			}
		}
	}

	stat = DefManager->getDefinition( WPDT_SKILL, "dexterity" );
	
	if( !stat )
	{
		clConsole.log( LOG_ERROR, "Couldn't find dexterity advancement table." );
	}
	else
	{
		for( INT32 i = 0; i < stat->childCount(); ++i )
		{
			const cElement *elem = stat->getChild( i );

			if( elem->name() == "advancement" )
			{
				stAdvancement advancement;

				advancement.base = elem->getAttribute( "base", "0" ).toInt();
				advancement.failure = elem->getAttribute( "failure", "0" ).toInt();
				advancement.success = elem->getAttribute( "success", "0" ).toInt();

				advDexterity.push_back( advancement );
			}
		}
	}

	stat = DefManager->getDefinition( WPDT_SKILL, "intelligence" );
	
	if( !stat )
	{
		clConsole.log( LOG_ERROR, "Couldn't find intelligence advancement table." );		
	}
	else
	{
		for( INT32 i = 0; i < stat->childCount(); ++i )
		{
			const cElement *elem = stat->getChild( i );

			if( elem->name() == "advancement" )
			{
				stAdvancement advancement;

				advancement.base = elem->getAttribute( "base", "0" ).toInt();
				advancement.failure = elem->getAttribute( "failure", "0" ).toInt();
				advancement.success = elem->getAttribute( "success", "0" ).toInt();

				advIntelligence.push_back( advancement );
			}
		}
	}

	// Load Skill Ranks
	skillRanks = DefManager->getList( "SKILL_RANKS" );

	// Fill it up to 9 Ranks
	while( skillRanks.count() < 9 )
		skillRanks.push_back( "" );
}

void cSkills::unload()
{
	skills.clear();
}

// For the Paperdoll
QString cSkills::getSkillTitle( P_CHAR pChar ) const
{
/*	QString skillTitle( "" );

	// Two ways of getting an empty title:
	// a) Configuration says we don't want Skill Titles
	// b) Character has a specific flag (priv & 0x10)
	if( !pChar->showSkillTitles() || !SrvParams->showSkillTitles() || pChar->isNpc() || pChar->isGMorCounselor() )
		return skillTitle;

	// Build our Skill Title (so first of all find the highest skill)
	UINT16 skill = pChar->bestSkill();
	UINT16 skillValue = pChar->skillValue( skill );

	// Append the Skill Rank
	if( skillValue  >= 1000 )		skillTitle.append( skillRanks[8] );
	else if( skillValue >= 900 )	skillTitle.append( skillRanks[7] );
	else if( skillValue >= 800 )	skillTitle.append( skillRanks[6] );
	else if( skillValue >= 700 )	skillTitle.append( skillRanks[5] );
	else if( skillValue >= 600 )	skillTitle.append( skillRanks[4] );
	else if( skillValue >= 500 )	skillTitle.append( skillRanks[3] );
	else if( skillValue >= 400 )	skillTitle.append( skillRanks[2] );
	else if( skillValue >= 300 )	skillTitle.append( skillRanks[1] );
	else							skillTitle.append( skillRanks[0] );

	// Skill not found
	if( skill >= skills.size() )
	{
		clConsole.log( LOG_ERROR, QString( "Skill id out of range: %u" ).arg( skill ) );
		return skillTitle;
	}

	if( skills[skill].title.isNull() )
		return skillTitle;

	// Append the real Skill Title
	skillTitle.append( skills[skill].title );

	return skillTitle;
	*/
	return "";
}

const QString &cSkills::getSkillName( UINT16 skill ) const
{
	if( skill >= skills.size() )
	{
		clConsole.log( LOG_ERROR, QString( "Skill id out of range: %u" ).arg( skill ) );
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
		clConsole.log( LOG_ERROR, QString( "Skill id out of range: %u" ).arg( skill ) );
		return QString::null;
	}
	
	return skills[skill].defname;
}

bool cSkills::advanceStats( P_CHAR pChar, UINT16 skill ) const
{
	if( !pChar )
		return false;

	UINT16 realStr = pChar->strength() - pChar->strengthMod();
	UINT16 realDex = pChar->dexterity() - pChar->dexterityMod();
	UINT16 realInt = pChar->intelligence() - pChar->intelligenceMod();

	UINT32 statSum = realDex + realStr + realInt;

	// Check if we reached the StatCap
	if( SrvParams->statcap() && statSum >= SrvParams->statcap() )
		return false;

	// First of all determine which stat can be raised using this skill
	// 1: Strength, 2: Dexterity: 3: Intelligence

	UINT8 strChance = 0, dexChance = 0, intChance = 0;

	// Strength can be risen
	if( realStr < 120 && realStr < skills[ skill ].strength )
		strChance = skills[ skill ].strength - realStr;

	// Dexterity can be risen
	if( realDex < 120 && realDex < skills[ skill ].dexterity )
		dexChance = skills[ skill ].dexterity - realDex;

	// Intelligence can be risen
	if( realInt < 120 && realInt < skills[ skill ].intelligence )
		intChance = skills[ skill ].intelligence - realInt;

	// No Stat can be risen by using this skill!
	if( intChance == 0 && dexChance == 0 && strChance == 0 )
		return false;

	// Now select the skill that will actually be risen
	UINT16 choice = RandomNum( 1, intChance + dexChance + strChance );

	bool gained = false;

	// 0 - strChance, strChance - ( strChance + dexChance ), ( strChance + dexChance ) - ( strChance + dexChance + intChance )
	if( choice <= strChance )
	{
		// Raise Str
		for( INT32 i = advStrength.size() - 1; i >= 0 ; --i )
		{
			if( advStrength[i].base <= realStr && ( advStrength[ i ].success >= RandomNum( 0, 10000 ) ) )
			{
				if( !pChar->onStatGain(0, 1) ) 
				{
					pChar->setStrength( pChar->strength() + 1 );
					pChar->setMaxHitpoints( pChar->maxHitpoints() + 1 );
				}
				gained = true;
				break;
			}
		}
	}
	else if( choice > strChance && ( choice - strChance ) < dexChance )
	{
		// Raise Dex
		for( INT32 i = advDexterity.size() - 1; i >= 0 ; --i )
		{
			if( advDexterity[i].base <= realDex && ( advDexterity[ i ].success >= RandomNum( 0, 10000 ) ) )
			{
				if( !pChar->onStatGain(1, 1) ) 
				{
					pChar->setDexterity( pChar->dexterity() + 1 );
					pChar->setMaxStamina( pChar->maxStamina() + 1 );
				}
				gained = true;
				break;
			}
		}
	}
	else if( choice > strChance + dexChance && ( choice - ( strChance + dexChance ) ) < intChance )
	{
		// Raise Int
		for( INT32 i = advIntelligence.size() - 1; i >= 0 ; --i )
		{
			if( advIntelligence[i].base <= realInt && ( advIntelligence[ i ].success >= RandomNum( 0, 10000 ) ) )
			{
				if( !pChar->onStatGain(2, 1) ) 
				{
					pChar->setIntelligence( pChar->intelligence() + 1 );
					pChar->setMaxMana( pChar->maxMana() + 1 );
				}
				gained = true;
				break;
			}
		}
	}

	// If we gained a certain stat let's update the clients
	// stat values.
	if( gained )
	{
		// Atrohpy for Stats needs to be implemented here
		P_PLAYER pc = dynamic_cast<P_PLAYER>(pChar);
		if( pc && pc->socket() )
			pc->socket()->sendStatWindow();
	}

	return gained;
}

bool cSkills::advanceSkill( P_CHAR pChar, UINT16 skill, SI32 min, SI32 max, bool success ) const
{
	if( !pChar )
		return false;

	P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pChar);

	if (pChar->onSkillGain(skill, min, max, success ) ) 
		return true;

	// For GMs there is no LockState
	UINT8 lockState = ( pPlayer && pPlayer->isGM() ) ? 0 : pChar->skillLock( skill );

	// NOTE:
	// Before this change, if you used locked skills you couldn't gain
	// in stats. This is removed now
	if( advanceStats( pChar, skill ) )
		return false;

	// We don't gain in locked or lowered skills
	if( lockState )
		return false;

	// Is the skill at it's single cap already ?
	// NOTE: Later on we need to provide support for power scrolls
	if( pChar->skillValue( skill ) >= pChar->skillCap( skill ) )
		return false;

	// If our skillsum is at the serverset skill cap we need
	// to find a skill we can lower in order to gain
	UINT32 skillSum = pChar->getSkillSum();
	
	QValueVector< UINT16 > atrophySkills;

	if( SrvParams->skillcap() && skillSum >= SrvParams->skillcap() * 10 )
	{
		// 1: Lower Skill
		for( UINT32 i = 0; i < ALLSKILLS; ++i )
		{
			if( i != skill && pChar->skillLock( i ) == 1 && pChar->skillValue( i ) > 0 )
			{
				// Found a skill we can lower
				atrophySkills.push_back( i );
			}
		}

		// If we didn't find any skill we could lower, return false
		if( atrophySkills.size() == 0 )
			return false;
	}

	stAdvancement advance = {0,};
	bool found = false;

	// Find the stAdvance for our current skillLevel
	for( INT32 i = skills[ skill ].advancement.size() - 1; i >= 0; --i )
	{
		advance = skills[ skill ].advancement[ i ];

		if( advance.base <= pChar->skillValue( skill ) )
		{
			found = true;
			break;
		}
	}

	// There is not one single advancement section defined for this skill
	if( !found )
		return false;

	UINT32 chance = success ? advance.success : advance.failure;
	chance *= 10;

	bool gained = false;

	if( chance > rand() % SrvParams->skillAdvanceModifier() )
	{
		gained = true;
		pChar->setSkillValue( skill, pChar->skillValue( skill ) + 1 );

		if( pPlayer && pPlayer->socket() )
			pPlayer->socket()->sendSkill( skill );
	}

	// Let a skill fall if we really gained and statcap is reached
	if( gained && atrophySkills.size() > 0 )
	{
		// Which skill do we want to lower
		UINT16 skill = atrophySkills[ RandomNum( 0, atrophySkills.size() ) ];

		pChar->setSkillValue( skill, pChar->skillValue( skill ) - 1 );
		
		if( pPlayer && pPlayer->socket() )
			pPlayer->socket()->sendSkill( skill );
	}
	
	return gained;
}
