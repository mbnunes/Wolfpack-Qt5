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
#include "globals.h"
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
#include "chars.h"
#include "npc.h"
#include "itemid.h"
#include "basics.h"

#undef  DBGFILE
#define DBGFILE "dbl_single_click.cpp"

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
	SERIAL serial = target_serial;
	P_PLAYER pc_currchar = socket->player();

	if( !pc_currchar->isGM() && /*pc_currchar->objectDelay() > 10 && ???*/ pc_currchar->objectDelay() >= uiCurrentTime )
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
			// Innocent Corpse?
			if( pi->hasTag( "notoriety" ) && pi->getTag( "notoriety" ).toInt() == 1 ) 
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

	// Check item behaviour by it's tpye
	switch (pi->type())
	{
	case 1: // normal containers
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

				else if( !pc_currchar->pos().lineOfSight( pi->pos(), true ) )
				{
					socket->clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
					return;
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
				if (pi->getTag("boatserial").isValid())
				{
					cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial( pi->getTag("boatserial").toInt() ) );
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

			pi->reduceAmount( 1 );	// Remove a food item
			pc_currchar->setHunger( pc_currchar->hunger() + 1 );
		}
		else
		{
			socket->sysMessage( tr( "You can't eat that!" ) );
		}
		return;
		
		// Drinks
		case 105:  
			pc_currchar->soundEffect( 0x30 + RandomNum( 0, 1 ) );			
			pi->reduceAmount( 1 ); // Remove a drink
			pc_currchar->message( "Gulp!" );
			return;

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
		
	    case 402: // Blackwinds Reputation ball 
			{ 
				pc_currchar->soundEffect( 0x01ec ); // Play sound effect for player 
                socket->sysMessage(tr("Your karma is %1").arg(pc_currchar->karma())); 
                socket->sysMessage(tr("Your fame is %1").arg(pc_currchar->fame())); 
                socket->sysMessage(tr("Your Kill count is %1 ").arg(pc_currchar->kills())); 
                socket->sysMessage(tr("You died %1 times.").arg(pc_currchar->deaths()));
				pc_currchar->effect( 0x372A, 0x09, 0x06 );
				socket->sysMessage(tr("*The crystal ball seems to have vanished*"));
                pi->reduceAmount(1); 
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
						const cElement *section = DefManager->getDefinition( WPDT_MULTI, pi->getTag( "multisection" ).toString() );
						
						if( section )
						{
							UI32 houseid = 0;
							
							unsigned int i = 0;
							while( i < section->childCount() && !houseid )
							{
								const cElement *childNode = section->getChild( i++ );

								if( childNode->name() == "id" )
									houseid = hex2dec( childNode->text() ).toUInt();
								else if( childNode->name() == "ids" )
									houseid = hex2dec( childNode->getAttribute( "north" ) ).toUInt();
							}

							if( houseid != 0 )
								socket->attachTarget( new cBuildMultiTarget( pi->getTag( "multisection" ).toString() , pc_currchar->serial(), pi->serial() ), houseid );
						}
					}
					return;// deeds
				
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
					pi->remove();
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
				default:
					break;
			}
		}

		// END Check items by ID
		socket->sysMessage( tr( "You can't think of a way to use that item." ) );
}
