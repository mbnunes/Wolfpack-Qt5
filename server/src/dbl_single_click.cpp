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

// Wolfpack Includes
#include "books.h"
#include "globals.h"
#include "guildstones.h"
#include "srvparams.h"
#include "skills.h"
#include "network.h"
#include "gumps.h"
#include "targetrequests.h"
#include "wpdefmanager.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "resources.h"
#include "multis.h"
#include "targetactions.h"
#include "boats.h"
#include "magic.h"
#include "chars.h"
#include "npc.h"
#include "itemid.h"

#undef  DBGFILE
#define DBGFILE "dbl_single_click.cpp"

// Shows the Spellbook gump when using a spellbook
void useSpellBook( cUOSocket *socket, P_PLAYER mage, P_ITEM spellbook )
{
	mage->setObjectDelay( 0 );

	if( ( spellbook->container() != mage ) && ( spellbook->getOutmostChar() != mage  ) )
	{
		socket->sysMessage( tr( "The spellbook needs to be in your hands or in your backpack." ) );
		return;
	}

	Magic->openSpellBook( mage, spellbook );
}

// Use a wand
void useWand( cUOSocket *socket, P_CHAR mage, P_ITEM wand )
{
	unsigned int tempuint;
	// Is it in our backpack or on our body ?
	if( ( wand->container() != mage )  && ( wand->getOutmostChar() != mage ) )
	{
		socket->sysMessage( tr( "If you wish to use this, it must be equipped or in your backpack." ) );
		return;
	}

	// Here it is either in our backpack or on our body
	if( wand->morez() == 0 )
	{
		socket->sysMessage( tr( "This Items magic is depleted." ) );
		return;
	}

	// morex: circle, morey: spell
	UI16 spellId = ( (wand->morex()-1) * 8 ) + wand->morey();
	
	Magic->prepare( mage, spellId, 2 );
	wand->setMoreX((tempuint = wand->morex())-- );
//	wand->morex--; // Reduce our charges

	if( wand->morex() == 0 )
	{
		wand->setType( wand->type2() );
		wand->setMoreX(0);
		wand->setMoreY(0);
		wand->setOffspell( 0 );
	}
}

void usehairdye(cUOSocket* socket, P_ITEM piDye)
{
	if (piDye == NULL)
		return;

	P_ITEM pi;
	P_PLAYER pc_currchar = socket->player();

	cPlayer::ItemContainer container(pc_currchar->content());
	cPlayer::ItemContainer::const_iterator it (container.begin());
	cPlayer::ItemContainer::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		pi = *it;
		if(pi->layer()==0x10 || pi->layer()==0x0B)//beard(0x10) and hair
		{
			pi->setColor( piDye->color() );	//Now change the color to the hair dye bottle color!
			pi->update();
		}
	}
	Items->DeleItem(piDye);	//Now delete the hair dye bottle!
}


bool isInLockedItem( P_ITEM pItem )
{
	if( pItem->container() && pItem->container()->isItem() )
	{
		P_ITEM pCont = dynamic_cast< P_ITEM >( pItem->container() );
		
		if( pCont->type() == 8 || pCont->type() == 64 )
			return false;
		else
			return isInLockedItem( pCont );
	}
	else
		return false;
}

void dbl_click_item(cUOSocket* socket, SERIAL target_serial) throw()
{
	unsigned int tempuint;

	SERIAL serial = target_serial;
	P_PLAYER pc_currchar = socket->player();

	UOXSOCKET s = calcSocketFromChar( socket->player() ); // for Legacy code

	if( !pc_currchar->isGM() && pc_currchar->objectDelay() > 10 && pc_currchar->objectDelay() >= uiCurrentTime )
	{
		socket->sysMessage(tr("You must wait to perform another action."));
		return;
	}
	else
		pc_currchar->setObjectDelay( SrvParams->objectDelay() * MY_CLOCKS_PER_SEC + uiCurrentTime );
	
	
	P_ITEM pi = FindItemBySerial( serial );
	
	if( !pi )
		return;

	if( pi->container() && pi->container()->isItem() && pi->type() != 1 && !pi->isInWorld())
	{ // Cant use stuff that isn't in your pack.
		P_CHAR pc_p = pi->getOutmostChar();
		if( pc_p && pc_currchar != pc_p )
				return;
	}
	else if( pi->container() && pi->container()->isChar() && pi->type() != 1 && !pi->isInWorld() )
	{	// in a character.
		P_CHAR pc_p = dynamic_cast<P_CHAR>(pi->container());
		if (pc_p != NULL)
			if( pc_p != pc_currchar && pi->layer() != 15 && pi->type() != 1 )
				return;
	}

	// Begin Items/Guildstones Section 
	int itype = pi->type();

	// Criminal for looting an innocent corpse & unhidden if not owner...
	if( pi->corpse() )
	{
		if (pc_currchar->isInvisible() && !pc_currchar->Owns(pi) && !pc_currchar->isGM())
		{
			pc_currchar->setHidden( 0 );
			pc_currchar->resend( false );
		}

		// TODO: Add a XML option for this
		if( !pc_currchar->Owns( pi ) && !pc_currchar->isGM() && pc_currchar->isInnocent() )
		{
			if( pi->more2() == 1 ) 
			{
				pc_currchar->makeCriminal();
			}
		}
	}
	
	// Secure containers
	if( pi->isLockedDown() && pi->secured() )
	{
		if( !pc_currchar->Owns( pi ) && !pc_currchar->isGM() )
		{
			socket->sysMessage( tr( "That is a secured chest!" ) );
			return;
		}
	}
	
	// Dead ppl can only use ankhs
	if( pc_currchar->isDead() && itype != 16 )
	{
		socket->sysMessage( tr( "Your ghostly hand passes trough the object." ) );
		return;
	}

	// You can only use equipment on your own char
	if( !pc_currchar->isGM() && pi->container() && pi->container()->isChar() && pi->container() != pc_currchar )
	{
		if( pi->layer() != 15 || !SrvParams->stealingEnabled() )
		{
			socket->sysMessage( tr( "You cannot use items equipped by other players." ) );
			return;
		}
	}

	// Call both events here
	if( pc_currchar->onUse( pi ) )
		return;

	if( pi->onUse( pc_currchar ) )
		return;

	// Spell Scroll
	if( IsSpellScroll( pi->id() ) && !pi->isLockedDown() )
	{
		P_CHAR owner = pi->getOutmostChar();

		if( owner != pc_currchar )
		{
			socket->sysMessage( tr( "The scroll must be in your possessions to envoke its magic." ) );
			return;
		}

		UI16 model = Magic->calcSpellId( pi->id() );
		
		if( Magic->prepare( pc_currchar, model, 1 ) )
			pi->ReduceAmount( 1 );

		return;
	}

	// Check item behaviour by it's tpye
	switch (pi->type())
	{
	case 1: // normal containers
	case 63:
		if( pi->moreb1() )
			Magic->MagicTrap( pc_currchar, pi );

	case 65: // nodecay item spawner..Ripper
	case 66: // decaying item spawner..Ripper
		{
			pc_currchar->setObjectDelay( 0 );	// no delay for opening containers
			
			if( pc_currchar->isGM() )
			{
				socket->sendContainer( pi );
				return;
			}

			if( pi->layer() > 0x18 )
			{
				socket->sysMessage( tr( "You can't see this." ) );
				return;
			}

			if( isInLockedItem( pi ) )
			{
				socket->sysMessage( tr( "You have to unlock it before taking a look." ) );
				return;
			}

			if( !pi->container() )
			{
				if( !pi->inRange( pc_currchar, 2 ) )
				{
					socket->clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
					return;
				}

				else if( !lineOfSight( pc_currchar->pos(), pi->pos(), WALLS_CHIMNEYS|DOORS|FLOORS_FLAT_ROOFING ) )
				{
					socket->clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
					return;
				}
				
				if( pi->corpse() )
				{
					if( pc_currchar->isHuman() )
						pc_currchar->action( 0x20 );

					pc_currchar->emote( tr( "*%1 loots the body of %2*" ).arg( pc_currchar->name() ).arg( pi->name2() ), 0x26 );
				}
				
				socket->sendContainer( pi );
				return;
			}
			else if( pi->container()->isItem() )
			{
				P_ITEM pOCont = pi->getOutmostItem();

				// Check if we can reach the top-container
				if( !pOCont->container() )
				{
					if( !pOCont->inRange( pc_currchar, 2 ) )
					{
						socket->clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
						return;
					}

					socket->sendContainer( pi );
				}
				else
				{
					P_CHAR pChar = dynamic_cast< P_CHAR >( pOCont->container() );
					if( pChar && pChar != pc_currchar )
					{
						if( !pChar->inRange( pc_currchar, 2 ) )
							socket->sysMessage( tr( "You must stand nearer to snoop!" ) );
						else						
							Skills->Snooping( pc_currchar, pi );
					}
					else if( pChar == pc_currchar )
						socket->sendContainer( pi );
				}

				return;
			}
			else if( pi->container()->isChar() )
			{
				// Equipped on another character
				P_CHAR pChar = dynamic_cast< P_CHAR >( pi->container() );

				if( pChar && pChar != pc_currchar )
				{
					if( !pChar->inRange( pc_currchar, 2 ) )
						socket->sysMessage( tr( "You must stand nearer to snoop!" ) );
					else						
						Skills->Snooping( pc_currchar, pi );
				}
				else if( pChar == pc_currchar )
					socket->sendContainer( pi );

				return;
			}

			socket->sysMessage( tr( "You can't open this container." ) );
			return;
		}
		return;

	case 16:
		// Check for 'resurrect item type' this is the ONLY type one can use if dead.
		if( pc_currchar->isDead() )
		{
			pc_currchar->resurrect();
			socket->sysMessage( tr( "You have been resurrected." ) );
			return;
		} 
		else 
		{
			socket->sysMessage( tr( "You are already living!" ) );
			return;
		}
		
	case 117:		
		// Boats ->
		if (pi->type2() == 3)
		{
			if( pc_currchar->inRange( pi, 3 ) )
			{
				if (pi->tags().get("boatserial").isValid())
				{
					cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial( pi->tags().get("boatserial").toInt() ) );
					pBoat->handlePlankClick( socket, pi );
				}
				else 
					socket->sysMessage(tr("That is locked."));
			}
			else
				socket->clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
		}
		else if( pi->type2() == 222 )
		{
			cMulti* pMulti = dynamic_cast< cMulti* >( pi );
			if( pMulti && ( pMulti->owner() == pc_currchar || pMulti->coOwner() == pc_currchar || pc_currchar->isGM() ) && socket )
			{
				cMultiGump* pGump = new cMultiGump( pc_currchar->serial(), pMulti->serial() );
				socket->send( pGump );
			}
		}
		// End Boats --^
		return;
		
	case 2: // Order gates?
		return;// order gates
	case 4: // Chaos gates?
		return;// chaos gates
	case 6: // teleport item (ring?)
		return;// case 6
	case 7: // key
		return;// case 7 (keys)
	case 8: // locked item spawner
	case 64: // locked container //Morrolan traps?
		// Added traps effects by AntiChrist
		pc_currchar->setObjectDelay( 0 );
		if (pi->moreb1())
			Magic->MagicTrap(pc_currchar, pi);
		socket->sysMessage( tr("This item is locked.") );
		return;// case 8/64 (locked container)

	// Spellbook
	case 9:
		useSpellBook( socket, pc_currchar, pi );
		return;

	// Book
	case 11:
	{
		cBook* pBook = dynamic_cast< cBook* >(pi);
		if( pBook )
		{
			pc_currchar->setObjectDelay( 0 );
			pBook->open( socket );
		}
		return;
	}	
	case 12: // door(unlocked)
		pc_currchar->setObjectDelay( 0 );
		dooruse( socket, pi );
		return;
	case 13: // locked door
		{
			if( pi->multis() != INVALID_SERIAL )
			{
				cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pi->multis() ) );
				if( pMulti )
				{
					if( pMulti->authorized( pc_currchar ) )
					{
						socket->sysMessage( tr( "You quickly unlock, use, and then relock the door." ) );
						pc_currchar->setObjectDelay( 0 );
						dooruse( socket, pi );
						return;
					}
				}
			}
			else
			{
				P_ITEM pPack = pc_currchar->getBackpack();
				if( pPack )
				{
					cItem::ContainerContent container = pPack->content();
					cItem::ContainerContent::const_iterator it(container.begin());
					for( ; it != container.end(); ++it )
					{
						P_ITEM pj = *it;
						if( pj && pj->type() == 7 && 
							pj->tags().get( "linkserial" ).isValid() && pj->tags().get( "linkserial" ).toInt() == pi->serial() )
						{
							socket->sysMessage( tr( "You quickly unlock, use, and then relock the door." ) );
							pc_currchar->setObjectDelay( 0 );
							dooruse( socket, pi );
							return;
						}
					}
				}
			}

			socket->sysMessage( tr( "This door is locked." ) );
			return;
		}

	// Food, OSI style
	case 14:
		pc_currchar->setObjectDelay( 0 );
		
		if( pi->isLockedDown() )
			return; // Ripper..cant eat locked down food :)
		
		if( pi->container() && pi->getOutmostChar() != pc_currchar )
		{
			socket->clilocMessage( 0x7A482 ); // You can't eat that, it belongs to someone else.
		}

		if( pc_currchar->hunger() < 0 )
			pc_currchar->setHunger( 0 );

		if( pc_currchar->hunger() >= 6 )
		{
			socket->clilocMessage( 0x7A483 ); // You are simply too full to eat any more!
			return;
		}

		if( pi->type2() == COOKEDMEAT || pi->type2() == COOKEDFISH || pi->type2() == PASTRIES || pi->type2() == FRUIT || pi->type2() == OTHER || pi->type2() == VEGETABLES )
		{
			pc_currchar->soundEffect( 0x3A + RandomNum( 0, 2 ) );
			
			switch( pc_currchar->hunger() )
			{
			case 0:
			case 1:
				socket->clilocMessage( 0x7A484 ); // You eat the food, but are still extremely hungry.
				break;

			case 2:
				socket->clilocMessage( 0x7A486 ); // After eating the food, you feel much less hungry.
				break;

			case 3:
				socket->clilocMessage( 0x7A485 ); // You eat the food, and begin to feel more satiated.
				break;

			case 4:
				socket->clilocMessage( 0x7A487 ); // You feel quite full after consuming the food.
				break;

			case 5:
				socket->clilocMessage( 0x7A488 ); // You manage to eat the food, but you are stuffed!
				break;
			}

			if ((pi->poisoned()) &&(pc_currchar->poisoned() < pi->poisoned())) 
			{
				socket->sysMessage(tr("You have been poisoned!"));
				pc_currchar->soundEffect( 0x246 ); // poison sound
				pc_currchar->setPoisoned( pi->poisoned() );
				pc_currchar->setPoisonTime( uiCurrentTime +(MY_CLOCKS_PER_SEC*(40/pc_currchar->poisoned()))); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
				pc_currchar->setPoisonWearOffTime( pc_currchar->poisonTime() +(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()) ); // wear off starts after poison takes effect - AntiChrist
				pc_currchar->resend( false );
			}
			
			pi->ReduceAmount( 1 );	// Remove a food item
			pc_currchar->setHunger( pc_currchar->hunger() + 1 );
		}
		else
		{
			socket->sysMessage( tr( "You can't eat that!" ) );
		}
		return;
	
	// Wands
	case 15:
		useWand( socket, pc_currchar, pi );
		return;

	case 18: // crystal ball?
		switch (RandomNum(0, 9))
		{
		case 0: socket->showSpeech(pi, tr("Seek out the mystic llama herder."));									break;
		case 1: socket->showSpeech(pi, tr("Wherever you go, there you are."));										break;
		case 2: socket->showSpeech(pi, tr("Quick! Lord Binary is giving away gold at the castle!"));				break;
		case 3: socket->showSpeech(pi, tr("Ripper is watching you every move."));									break;
		case 4: socket->showSpeech(pi, tr("The message appears to be too cloudy to make anything out of it."));		break;
		case 5: socket->showSpeech(pi, tr("You have just lost five strength.. not!"));								break;
		case 6: socket->showSpeech(pi, tr("You're really playing a game you know"));								break;
		case 7: socket->showSpeech(pi, tr("You will be successful in all you do."));								break;
		case 8: socket->showSpeech(pi, tr("You are a person of culture."));											break;
		case 9: socket->showSpeech(pi, tr("Give me a break! How much good fortune do you expect!"));				break;
		}// switch
		//soundeffect(s, 0x01, 0xEC);
		return;// case 18 (crystal ball?)

		// Teleport object
		case 104: 
			pc_currchar->removeFromView( false );
			pc_currchar->MoveTo( pi->morex(), pi->morey(), pi->morez() );
			pc_currchar->resend( false );
			return;
		
		// Drinks (This needs some other effects as well)
		case 105:  
			pc_currchar->soundEffect( 0x30 + RandomNum( 0, 1 ) );			
			pi->ReduceAmount( 1 ); // Remove a drink
			pc_currchar->message( "Gulp!" );
			return;

		case 202:
//				if ( pi->id() == 0x14F0  ||  pi->id() == 0x1869 )	// Check for Deed/Teleporter + Guild Type
//				{
//					pc_currchar->setFx1( pi->serial() );
//					StonePlacement(socket);
//					return;
//				}
//				else if (pi->id() == 0x0ED5)	// Check for Guildstone + Guild Type
//				{
//					pc_currchar->setFx1( pi->serial() );
//					cGuildStone *pStone = dynamic_cast<cGuildStone*>(pi);
//					if ( pStone != NULL )
//						pStone->Menu(s, 1);
//					return;
//				}
//				else 
					clConsole.send("Unhandled guild item type named: %s with ID of: %X", pi->name().ascii(), pi->id());
				return;
				// End of guild stuff

		// PlayerVendors deed
		case 217:			
			{	
				if (pi->isLockedDown())
				{
					socket->sysMessage(tr("That item is locked down."));
					return; 
				}

				cMulti* pi_multi = cMulti::findMulti( pc_currchar->pos() );
				if( pi_multi && pc_currchar->inRange( pi_multi, 18 ) )
				{	
					if ( !IsHouse( pi_multi->id() ) )
						return;
					int los = 0;
					const P_ITEM pi_p = pc_currchar->getBackpack();
					if( pi_p )
					{
						los = 0;
						cItem::ContainerContent container = pi_p->content();
						cItem::ContainerContent::const_iterator it(container.begin());
						for( ; it != container.end(); ++it )
						{
							const P_ITEM pi_i = *it;
							if ( pi_i ) // lb
								if (pi_i->type() == 7 && calcserial(pi_i->more1(), pi_i->more2(), pi_i->more3(), pi_i->more4()) == pi_multi->serial())
								{
									los = 1;
									break;
								}
						}
					}
				
					if (los)
					{
						P_NPC pc_vendor = cCharStuff::createScriptNpc( "2117", pc_currchar->pos() );
						
						if (pc_vendor == NULL) 
						{
							clConsole.send("npc-script couldnt find vendor in npc-section 2117!\n");
							return;
						}
						
						pc_vendor->setNpcAIType( 17 );
						pc_vendor->setInvulnerable( true );
						pc_vendor->setHidden( 0 );
						pc_vendor->setStealth(-1);
						pc_vendor->setDirection( pc_currchar->direction() );
						pc_vendor->setNpcWander(0);
						pc_vendor->setInnocent();
						pc_vendor->setOwner( pc_currchar );
						pc_vendor->setTamed(false);
						Items->DeleItem(pi);
						pc_vendor->talk( tr("Hello sir! My name is %1 and i will be working for you.").arg(pc_vendor->name()), -1, 0 );
						pc_vendor->update();
					}
					else 
						socket->sysMessage(tr("You must be close to a house and have a key in your pack to place that."));
				}
				else if (pi_multi == NULL)
					socket->sysMessage(tr("You must be close to a house and have a key in your pack to place that."));
				
				return;
			}
			break;

		case 222:	// player clicks on a house item (sign) to set ban/friendlists, rename
			{
				cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( pi->multis() ) );
				if( pMulti && ( pMulti->owner() == pc_currchar || pMulti->coOwner() == pc_currchar || pc_currchar->isGM() ) && socket )
				{
					cMultiGump* pGump = new cMultiGump( pc_currchar->serial(), pMulti->serial() );
					socket->send( pGump );
				}
			}
			return;
		
		// By Polygon: Clicked on a tattered treasure map, call decipher-function
		case 301: 
			Skills->Decipher(pi, s); 
			return;
/*
	By Polygon:
	Clicked on a deciphered treasure map
	Show a map-gump with the treasure location in it
*/
		case 302:	// Deciphered treasure map?
			return;
// END OF: By Polygon
	    case 402: // Blackwinds Reputation ball 
			{ 
				pc_currchar->soundEffect( 0x01ec ); // Play sound effect for player 
                socket->sysMessage(tr("Your karma is %1").arg(pc_currchar->karma())); 
                socket->sysMessage(tr("Your fame is %1").arg(pc_currchar->fame())); 
                socket->sysMessage(tr("Your Kill count is %1 ").arg(pc_currchar->kills())); 
                socket->sysMessage(tr("You died %1 times.").arg(pc_currchar->deaths()));
				pc_currchar->effect( 0x372A, 0x09, 0x06 );
				socket->sysMessage(tr("*The crystal ball seems to have vanished*"));
                pi->ReduceAmount(1); 
                return; 
			}
		case 404: // Fraz'z ID wand
			{
				P_ITEM pBackpack = pc_currchar->getBackpack();
				if ( pBackpack != NULL )
				{
					if ((pi->container() == pBackpack) || pc_currchar->Wears(pi) &&(pi->layer() == 1))
					{
						if (pi->morex() <= 0)
						{
							socket->sysMessage( tr("That is out of charges.") );
							return;
						}
						pi->setMoreX((tempuint=pi->morex())--);
						//pi->morex--;
						socket->sysMessage( tr("Your wand now has %1 charges left").arg( pi->morex()) );
//						target(s, 0, 1, 0, 75, "What do you wish to identify?");
					}
					else
					{
						socket->sysMessage(tr("If you wish to use this, it must be equipped or in your backpack."));
					}
				}
				return;
			}

		// Dyes
		case 405:
			{
				cUOTxDyeTub dyetub;
				dyetub.setSerial( pi->serial() );
				dyetub.setModel( 0xFAB );
				socket->send( &dyetub );
			}
			return;

		// Dying tub
		case 406:
			socket->sysMessage( tr( "What do you want to dye?" ) );
			socket->attachTarget( new cDyeTarget( pi->color() ) );
			return;

		case 1000: // Ripper...bank checks
			{
				socket->sysMessage(tr("To cash this, you need to drop it on a banker."));
				return;
			}

		// 1001: Sword Weapons (Swordsmanship)
		case 1001: 
		// 1002: Axe Weapons (Swordsmanship + Lumberjacking)
		case 1002: 
		// 1005: Fencing
		case 1005:
		// 1003: Macefighting (Staffs)
		case 1003:
		// 1004: Macefighting (Maces/WarHammer)
		case 1004:
		// 1006: Bows
		case 1006:
		// 1007: Crossbows
		case 1007:
		// 1008: Shields
		case 1008:
			break;

		/* Skill related types */
		// 1100: Blacksmithing Tools
		case 1100:
			Skills->Blacksmithing( socket );
			return;

		// 1101: Anvil
		case 1101:
			if( !pc_currchar->inRange( pi, 3 ) )
				socket->sysMessage( tr( "Must be closer to use this!" ) );
			else
				socket->sysMessage( "Select item to be repaired." );
				// TODO: Reimplement repairing items.
			return;

		// 1102: Ore
		case 1102:
			socket->sysMessage( "Where do you want to smelt the ore?" );
			socket->attachTarget( new cConvertResource( QString("RESOURCE_INGOT"), pi ) );
			return;

		// 1103: Carpentry Tools
		case 1103:
			Skills->Carpentry( socket );
			return;

		// 1104: Mining Tools
		case 1104:
			if( !pi->wearOut() )
			{
				socket->sysMessage( tr( "Where do you want to dig?" ) );
				socket->attachTarget( new cFindResource( "RESOURCE_ORE" ) );
			}
			return;

		// 1105: Spell Scroll
		case 1105:
			socket->sysMessage( tr( "Casting from scrolls is currently not supported." ) );
			return;

		// 1106: Tailoring Tools
		case 1106:
			Skills->Tailoring( socket );
			return;

		// 1107: Alchemy Tools
		case 1107:
			MakeMenus::instance()->callMakeMenu( socket, "CRAFTMENU_ALCHEMY" );
			return;

		// 1108: Tailoring Tools
		case 1108:
			Skills->Fletching( socket );
			return;

		// 1201: Necromancy Book
		// 1202: Paladin Book
		case 1201:
		case 1202:
			useSpellBook( socket, pc_currchar, pi );
			return;

		default:						
			break;
		}

		// END Check items by type
		
		// Begin checking objects by ID
		if (!pi->isLockedDown()) // Ripper
		{
			switch (pi->id())
			{	
				case 0x14F0:// deeds
					if ((pi->type() != 103) &&(pi->type() != 202))
					{  
						const QDomElement* DefSection = DefManager->getSection( WPDT_MULTI, pi->tags().get( "multisection" ).toString() );
						if( !DefSection->isNull() )
						{
							UI32 houseid = 0;
							QDomNode childNode = DefSection->firstChild();
							while( !childNode.isNull() && houseid == 0 )
							{
								if( childNode.isElement() && childNode.nodeName() == "id" )
									houseid = hex2dec(childNode.toElement().text()).toUInt();
								else if( childNode.isElement() && childNode.nodeName() == "ids" )
									houseid = hex2dec(childNode.toElement().attribute( "north" )).toUInt();
								childNode = childNode.nextSibling();
							}
							if( houseid != 0 )
								socket->attachTarget( new cBuildMultiTarget( pi->tags().get( "multisection" ).toString() , pc_currchar->serial(), pi->serial() ), houseid );
						}
					}
					return;// deeds
				case 0x0E9C:
					if( pc_currchar->checkSkill( MUSICIANSHIP, 0, 1000 ) )
						pc_currchar->soundEffect( 0x0038 );
					else 
						pc_currchar->soundEffect( 0x0039 );
					return;
				case 0x0E9D:
				case 0x0E9E:
					if( pc_currchar->checkSkill( MUSICIANSHIP, 0, 1000 ) )
						pc_currchar->soundEffect( 0x0052 );
					else 
						pc_currchar->soundEffect( 0x0053 );
					return;
				case 0x0EB1:
				case 0x0EB2:
					if( pc_currchar->checkSkill( MUSICIANSHIP, 0, 1000 ) )
						pc_currchar->soundEffect( 0x0045 );
					else 
						pc_currchar->soundEffect( 0x0046 );
					return;
				case 0x0EB3:
				case 0x0EB4:
					if( pc_currchar->checkSkill( MUSICIANSHIP, 0, 1000 ) )
						pc_currchar->soundEffect( 0x004C );
					else 
						pc_currchar->soundEffect( 0x004D );
					return;

				case 0x0E24: // empty vial
					{
					P_ITEM pBackpack = pc_currchar->getBackpack();
					if (pBackpack != NULL)
						if (pi->container() == pBackpack)
						{
//							addmitem[s] = pi->serial(); // save the vials number, LB
//							target(s, 0, 1, 0, 186, "What do you want to fill the vial with?");
						}
						else 
							socket->sysMessage(tr("The vial is not in your pack"));
						return;
					}
				case 0x0FA0:
				case 0x0FA1: // thread to Bolt
				case 0x0E1D:
				case 0x0E1F:
				case 0x0E1E:  // yarn to cloth
//					pc_currchar->setTailItem(  pi->serial() );
//					target(s, 0, 1, 0, 165, "Select loom to make your cloth");
					return;
				case 0x1BD1:
				case 0x1BD2:
				case 0x1BD3:
				case 0x1BD4:
				case 0x1BD5:
				case 0x1BD6:	// make shafts
//					itemmake[s].Mat1id = pi->id();
//					target(s, 0, 1, 0, 172, "What would you like to use this with?"); 
					return;
				case 0x0E73: // cannon ball
//					target(s, 0, 1, 0, 170, "Select cannon to load."); 
					Items->DeleItem(pi);
					return;
				case 0x0FF8:
				case 0x0FF9: // pitcher of water to flour
//					pc_currchar->setTailItem( pi->serial() );
//					target(s, 0, 1, 0, 173, "Select flour to pour this on.");  
					return;
				case 0x09C0:
				case 0x09C1: // sausages to dough
//					pc_currchar->setTailItem( pi->serial() );
//					target(s, 0, 1, 0, 174, "Select dough to put this on.");  
					return;
				case 0x0DF8: // wool to yarn 
//					pc_currchar->setTailItem( pi->serial() );
//					target(s, 0, 1, 0, 164, "Select your spin wheel to spin wool.");      
					return;
				case 0x0F9D: // sewing kit for tailoring
//					target(s, 0, 1, 0, 167, "Select material to use.");
					return;
				case 0x1508: // magic statue?
					if( pc_currchar->checkSkill( ITEMID, 0, 10))
					{
						pi->setId(0x1509);
						pi->setType( 45 );
						pi->update();// AntiChrist
					}
					else
					{
						socket->sysMessage(tr("You failed to use this statue."));
					}
					return;
				case 0x1509:
					if( pc_currchar->checkSkill( ITEMID, 0, 10))
					{
						pi->setId(0x1508);
						pi->setType( 45 );
						pi->update();// AntiChrist
					}
					else
					{
						socket->sysMessage(tr("You failed to use this statue."));
					}
					return;

				//case 0x0DBF:
				//case 0x0DC0:// fishing
//					target(s, 0, 1, 0, 45, "Fish where?");
				//	return;
				//case 0x0E9B: // Mortar for Alchemy
				//	if (pi->type() == 17)
				//	{
				//		addid1[s] = static_cast<unsigned char>((pi->serial()&0xFF000000)>>24);
			//			addid2[s] = static_cast<unsigned char>((pi->serial()&0x00FF0000)>>16);
			//			addid3[s] = static_cast<unsigned char>((pi->serial()&0x0000FF00)>>8);
			//			addid4[s] = static_cast<unsigned char>((pi->serial()&0x000000FF));
//						target(s, 0, 1, 0, 109, "Where is an empty bottle for your potion?");
			//		}
			//		else
			//		{
			///			addid1[s] = static_cast<unsigned char>((pi->serial()&0xFF000000)>>24);
			//			addid2[s] = static_cast<unsigned char>((pi->serial()&0x00FF0000)>>16);
			//			addid3[s] = static_cast<unsigned char>((pi->serial()&0x0000FF00)>>8);
			//			addid4[s] = static_cast<unsigned char>((pi->serial()&0x000000FF));
//						target(s, 0, 1, 0, 108, "What do you wish to grind with your mortar and pestle?");
			//		}
			//		return; // alchemy
				case 0x0E21: // healing
					{
						cSkHealing* target = new cSkHealing( pi->serial() );
						socket->attachTarget( target );
						socket->sysMessage( tr("Who will you use the bandages on?") );
					}
					return;
				case 0x1057:
				case 0x1058: // sextants
					getSextantCords(pc_currchar->pos().x, pc_currchar->pos().y, socket->isT2A(), temp);
					socket->sysMessage( tr("You are at: %1").arg(temp) );
					return;
				case 0x0E27:
				case 0x0EFF:   // Hair Dye
					usehairdye(socket, pi);
					return;
				case 0x14FB:
				case 0x14FC:
				case 0x14FD:
				case 0x14FE: // lockpicks
					{
						cSkLockpicking* target = new cSkLockpicking( pi->serial() );
						socket->attachTarget( target );
						socket->sysMessage( tr("What lock would you like to pick?") );
					}
					return;
				case 0x0C4F:
				case 0x0C50:
				case 0x0C51:
				case 0x0C52:
				case 0x0C53:
				case 0x0C54: // cotton plants
					{
						pc_currchar->action( 0xD );
						pc_currchar->soundEffect( 0x13E );

						P_ITEM p_cotton = Items->SpawnItem(pc_currchar, 1, "#", 1, 0x0DF9, 0, 1);
						if ( !p_cotton )
							return;
						
						p_cotton->toBackpack( pc_currchar );
						
						if( socket )
							socket->sysMessage( tr( "You reach down and pick some cotton." ) );
					}
					return; // cotton
				case 0x105B:
				case 0x105C:
				case 0x1053:
				case 0x1054: // tinker axle
//					addid1[s] = static_cast<unsigned char>((pi->serial()&0xFF000000)>>24);
//					addid2[s] = static_cast<unsigned char>((pi->serial()&0x00FF0000)>>16);
//					addid3[s] = static_cast<unsigned char>((pi->serial()&0x0000FF00)>>8);
//					addid4[s] = static_cast<unsigned char>((pi->serial()&0x000000FF));
//					target(s, 0, 1, 0, 183, "Select part to combine that with.");
					return;
				case 0x1051:
				case 0x1052:
				case 0x1055:
				case 0x1056:
				case 0x105D:
				case 0x105E:
//					addid1[s] = static_cast<unsigned char>((pi->serial()&0xFF000000)>>24);
//					addid2[s] = static_cast<unsigned char>((pi->serial()&0x00FF0000)>>16);
//					addid3[s] = static_cast<unsigned char>((pi->serial()&0x0000FF00)>>8);
//					addid4[s] = static_cast<unsigned char>((pi->serial()&0x000000FF));
					// itemmake[s].materialid1=pi->id1;
					// itemmake[s].materialid2=pi->id2;
//					target(s, 0, 1, 0, 184, "Select part to combine it with.");
					return;
				case 0x104F:
				case 0x1050:
				case 0x104D:
				case 0x104E:// tinker clock
//					addid1[s] = static_cast<unsigned char>((pi->serial()&0xFF000000)>>24);
//					addid2[s] = static_cast<unsigned char>((pi->serial()&0x00FF0000)>>16);
//					addid3[s] = static_cast<unsigned char>((pi->serial()&0x0000FF00)>>8);
//					addid4[s] = static_cast<unsigned char>((pi->serial()&0x000000FF));
//					target(s, 0, 1, 0, 185, "Select part to combine with");
					return;
				case 0x1059:
				case 0x105A:// tinker sextant
					if( pc_currchar->checkSkill( TINKERING, 500, 1000))
					{
						socket->sysMessage(tr("You create the sextant."));
						P_ITEM pi_sextant = Items->SpawnItem(pc_currchar, 1, "a sextant", 0, 0x1057, 0, 1);
						if (pi_sextant != NULL)
							pi_sextant->setPriv( pi_sextant->priv() | 0x01 );
						pi->ReduceAmount(1);
					}
					else 
						socket->sysMessage(tr("you fail to create the sextant."));
					return;
				default:
					break;
			}
		}

		// END Check items by ID
		socket->sysMessage( tr( "You can't think of a way to use that item." ) );
}

/*!
  Handles Double clicks over PC/NPCs
 */
void showPaperdoll( cUOSocket *socket, P_CHAR pTarget, bool hotkey )
{
	if( !socket )
		return;

	P_PLAYER pChar = socket->player();

	if( !pChar || !pTarget )
		return;

	if( pTarget->onShowPaperdoll( pChar ) )
		return;

	// For players we'll always show the Paperdoll
	if( pTarget->isHuman() || !pTarget->isNpc() )
	{
		// Theres one exception for player vendors, when you double click them
		// Their packs should open instead of their paperdoll
		if( pTarget->npcaitype() == 17 )
		{
			pTarget->talk( tr( "Take a look at my goods" ) );

			if( pTarget->getBackpack() )
				socket->sendContainer( pTarget->getBackpack() );

			return;
		}

        // If we're mounted (item on layer 25) and *not* using a hotkey
		// We're trying to unmount ourself
		if( !hotkey && ( pTarget == pChar ) && pChar->unmount() )
			return; // We have been unmounted

		socket->sendPaperdoll( pTarget );
	}

	UINT16 body = pTarget->bodyID();

	// Is that faster ??

	switch( body )
	{
	case 0x0034:
	case 0x004E:
	case 0x0050:
	case 0x003A:
	case 0x0039:
	case 0x003B:
	case 0x0074:
	case 0x0075:
	case 0x0072:
	case 0x007A:
	case 0x0084:
	case 0x0073:
	case 0x0076:
	case 0x0077:
	case 0x0078:
	case 0x0079:
	case 0x00AA:
	case 0x00AB:
	case 0x00BB:
	case 0x0090:
	case 0x00C8:
	case 0x00E2:
	case 0x00E4:
	case 0x00CC:
	case 0x00DC:
	case 0x00D2:
	case 0x00DA:
	case 0x00DB:
	case 0x0317:
	case 0x0319:
	case 0x031A:
	case 0x031F:
		// Try to mount the rideable animal
		if( pChar->dist( pTarget ) <  2 || pChar->isGM() )
		{
			if( !pChar->isHuman() )
			{
				socket->sysMessage( tr( "You are unable to ride an animal" ) );
				return;
			}
			if( pChar->isDead() )
			{
				socket->clilocMessage( 0x7A4D5, "", 0x3b2 ); // You can't do that when you're dead.

				return;
			}
			if( pTarget->war() )
				socket->sysMessage( tr("Your pet is in battle right now!" ) );
			else
				pChar->mount( pTarget );
		}
		else
			socket->sysMessage( tr( "This is too far away" ) );

		break;
	case 0x123:
	case 0x124:
		if( pTarget->owner() == pChar )
				socket->sendContainer( pTarget->getBackpack() );
	};
}
