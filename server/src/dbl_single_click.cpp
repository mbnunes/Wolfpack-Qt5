//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "trigger.h"
#include "dbl_single_click.h"
#include "guildstones.h"
#include "srvparams.h"
#include "classes.h"
#include "network.h"
#include "gumps.h"
#include "targetrequests.h"
#include "wpdefmanager.h"
#include "network/uosocket.h"

#undef  DBGFILE
#define DBGFILE "dbl_single_click.cpp"

// Loads a Cannonball
class cLoadCannon: public cTargetRequest
{
protected:
	SERIAL cannonBall;
public:
	cLoadCannon( SERIAL nBall ) { cannonBall = nBall; };
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		socket->sysMessage( "You try to put the cannon ball into the cannon but fail" );
		return true;
	}
};

void useCannonBall( UOXSOCKET socket, P_CHAR user, P_ITEM cannonball )
{
	if( cannonball->isLockedDown() )
	{
		sysmessage( socket, "This is too heavy for you" );
		return;
	}

	// Display a target
	attachTargetRequest( socket, new cLoadCannon( cannonball->serial ) );
}

// Shows the Spellbook gump when using a spellbook
void useSpellBook( UOXSOCKET socket, P_CHAR mage, P_ITEM spellbook )
{
	mage->objectdelay = 0;

	if( ( spellbook->contserial != mage->serial ) && ( GetPackOwner( spellbook, 10 ) != mage  ) )
	{
		sysmessage( socket, "The spellbook needs to be in your hands or in your backpack." );
		return;
	}

	Magic->openSpellBook( mage, spellbook );
}

// Use a wand
void useWand( UOXSOCKET socket, P_CHAR mage, P_ITEM wand )
{
	// Is it in our backpack or on our body ?
	if( ( wand->contserial != mage->serial )  && ( GetPackOwner( wand, 10 ) != mage ) )
	{
		sysmessage( socket, "If you wish to use this, it must be equipped or in your backpack." );
		return;
	}

	// Here it is either in our backpack or on our body
	if( wand->morez == 0 )
	{
		sysmessage( socket, "This Items magic is depleted." );
		return;
	}

	// morex: circle, morey: spell
	UI16 spellId = ( (wand->morex-1) * 8 ) + wand->morey;
	
	Magic->prepare( mage, spellId, 2 );

	wand->morex--; // Reduce our charges

	if( wand->morex == 0 )
	{
		wand->setType( wand->type2() );
		wand->morex = 0;
		wand->morey = 0;
		wand->setOffspell( 0 );
	}
}

//////////////////
// name:	Item_ToolWearOut (2 interfaces)
// history:	first impl by Ripper in various places
//			collected here by Duke, 7.11.2000
// purpose:	apply wear out to item, delete if necessary
// remark:	1st interface will become a method of the item class,
//			2nd is only for convenience here
//
static bool Item_ToolWearOut(P_ITEM pi)
{
	if((rand()%4)==0)
		pi->setHp( pi->hp() - 1 ); //Take off a hit point

	if( pi->hp() <= 0 )
	{	 
		Items->DeleItem(pi);
		return true;
	}
	else
		return false;
}

static bool Item_ToolWearOut(UOXSOCKET s, P_ITEM pi)
{
	if(Item_ToolWearOut(pi))	// has item been destroyed ??
	{	 
		sprintf((char*)temp,"Your %s has been destroyed",pi->name().ascii());
		sysmessage(s,(char*)temp);
		return true;
	}
	else
		return false;
}

//////////////////
// name:	slotmachine
// history:	by Ripper, 5.12.2000
// purpose:	takes 5gp to spin it and gives random wins
//
void slotmachine(UOXSOCKET s, P_ITEM pi)
{
	if (pi == NULL)
		return;
	P_CHAR pc_currchar = currchar[s];
	if(pc_currchar->dead)		// no ghosts playing :)
	{
		sysmessage(s,"ghosts cant do that!");
		return;
	}
	if(pc_currchar->CountGold() < SrvParams->slotAmount())	// check his gold to see if has enough.
	{
		sysmessage(s,"you dont have enough gold to play!");
		return;
	}
	delequan(pc_currchar, 0x0EED, SrvParams->slotAmount(), NULL);	// lets delete the coins played.
	int spin=RandomNum( 0,100);	// now lets spin to win :)
	switch(spin)
	{
	case 0: Items->SpawnItemBackpack2(s,"3185",1);
		sysmessage(s,"Single bars, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 1: Items->SpawnItemBackpack2(s,"3186",1);
		sysmessage(s,"Double bars, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 2: Items->SpawnItemBackpack2(s,"3187",1);
		sysmessage(s,"Triple bars, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 3: Items->SpawnItemBackpack2(s,"2003",1);
		sysmessage(s,"Any three 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 4: Items->SpawnItemBackpack2(s,"2004",1);
		sysmessage(s,"Three blue 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 5: Items->SpawnItemBackpack2(s,"2005",1);
		sysmessage(s,"Three white 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 6: Items->SpawnItemBackpack2(s,"2006",1);
		sysmessage(s,"Three red 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 7: Items->SpawnItemBackpack2(s,"2007",1);
		sysmessage(s,"Jackpot, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	default : itemmessage(s,"Sorry,not a winner,please insert coins.",pi->serial); break;
	}
	soundeffect(s, 0x00, 0x57);	// my stupid spin sound hehe.
}

void dbl_click_item(cUOSocket* socket, SERIAL target_serial)
{
	//	unsigned char map1[20] = "\x90\x40\x01\x02\x03\x13\x9D\x00\x00\x00\x00\x13\xFF\x0F\xA0\x01\x90\x01\x90";
	unsigned char map1[20] = "\x90\x40\x01\x02\x03\x13\x9D\x00\x00\x00\x00\x13\xFF\x0F\xFF\x01\x90\x01\x90";
	// By Polygon: Lower map border is 4095, not 4000, no more needed with new system anyway ;)
	unsigned char map2[12] = "\x56\x40\x01\x02\x03\x05\x00\x00\x00\x00\x00";
	// By Polygon: This one is needed to show the location on treasure maps
	unsigned char map3[12] = "\x56\x40\x01\x02\x03\x01\x00\x00\x00\x00\x00";
	
	SERIAL serial = target_serial;
	P_CHAR pc_currchar = socket->player();

	UOXSOCKET s = calcSocketFromChar( socket->player() ); // for Legacy code

	int w = 0;
	
	if( !pc_currchar->isGM() && pc_currchar->objectdelay > 10 && pc_currchar->objectdelay >= uiCurrentTime )
	{
		socket->sysMessage(tr("You must wait to perform another action."));
		return;
	}
	else
		pc_currchar->objectdelay = SrvParams->objectDelay() * MY_CLOCKS_PER_SEC + uiCurrentTime;
	
	
	P_ITEM pi = FindItemBySerial( serial );
	
	if( !pi )
		return;

	// Eventually we want the users to script things like
	// "v-cards" and others so we need them to check that
	// the item is in their range on their own !!
	//-------
	// Call both events here
	if( pc_currchar->onUse( pi ) )
		return;

	if( pi->onUse( pc_currchar ) )
		return;

	// -- end - DarkStorm

	if( isItemSerial( pi->contserial ) && pi->type() != 1 && !pi->isInWorld())
	{ // Cant use stuff that isn't in your pack.
		P_CHAR pc_p = GetPackOwner(FindItemBySerial(pi->contserial));
		if( pc_p && pc_currchar != pc_p )
				return;
	}
	else if( isCharSerial( pi->contserial ) && pi->type() != 1 && !pi->isInWorld() )
	{// in a character.
		P_CHAR pc_p = FindCharBySerial(pi->contserial);
		if (pc_p != NULL)
			if( pc_p != pc_currchar && pi->layer() != 15 && pi->type() != 1 )
				return;
	}
	
	// Begin Items/Guildstones Section 
	int itype = pi->type();

	// Criminal for looting an innocent corpse & unhidden if not owner...
	if( pi->corpse() )
	{
		if (pc_currchar->hidden() && !pc_currchar->Owns(pi) && !pc_currchar->isGM())
		{
			pc_currchar->setHidden( 0 );
			pc_currchar->resend( false );
		}

		// TODO: Add a XML option for this
		if( !pc_currchar->Owns( pi ) && !pc_currchar->isGM() && pc_currchar->isInnocent() )
		{
			if( pi->more2 == 1 ) 
			{
				criminal( pc_currchar );
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
	if( pc_currchar->dead && itype != 16 )
	{
		socket->sysMessage( tr( "Your ghostly hand passes trough the object." ) );
		return;
	}

	// You can only use equipment on your own char
	if( !pc_currchar->isGM() && isCharSerial( pi->contserial ) && pi->contserial != pc_currchar->serial )
	{
		if( pi->layer() != 15 || !SrvParams->stealingEnabled() )
		{
			socket->sysMessage( tr( "You cannot use items equipped by other players." ) );
			return;
		}
	}
	// Spell Scroll
	else if( IsSpellScroll( pi->id() ) && !pi->isLockedDown() )
	{
		P_CHAR owner = GetPackOwner( pi, 64 );

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
	case 16:
		// Check for 'resurrect item type' this is the ONLY type one can use if dead.
		if( pc_currchar->dead )
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
				if (pi->tags.get("boatserial").isValid())
				{
					cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial( pi->tags.get("boatserial").toUInt() ) );
					pBoat->handlePlankClick( s, pi );
				}
				else 
					socket->sysMessage(tr("That is locked."));
			}
			else 
				socket->sysMessage(tr("You can't reach that!"));
		}
		// End Boats --^
		return;
		
	case 1: // normal containers
	case 63:
		if( pi->moreb1() )
			Magic->MagicTrap(pc_currchar, pi); // added by AntiChrist
		// only 1 and 63 can be trapped, so pleaz leave it here :) - Anti
	case 65: // nodecay item spawner..Ripper
	case 66: // decaying item spawner..Ripper
		{
			pc_currchar->objectdelay = 0;	// no delay for opening containers
			
			SERIAL contser = pi->contserial;
			if ( ( contser == INVALID_SERIAL && pc_currchar->inRange( pi, 2 ) ) ||  // Backpack in world - free access to everyone
				pc_currchar->Wears( pi ) )	// primary pack
			{
				if( pi->corpse() )
				{
					if( pc_currchar->isHuman() )
						pc_currchar->action( 0x20 );

					pc_currchar->emote( tr( "*%1 loots the body of %2*" ).arg( pc_currchar->name.c_str() ).arg( pi->name2() ), 0x26 );
				}

				pc_currchar->objectdelay = 0;
				socket->sendContainer( pi );
				return;
			}
			if (isItemSerial(pi->contserial))
			{
				P_ITEM pio = GetOutmostCont(pi);
				if( !pio ) 
					return;		// this should *not* happen, but it does ! Watch the logfiles (Duke)

				if( pc_currchar->Wears( pio ) || ( pio->isInWorld() && pc_currchar->inRange( pi, 2 ) ) ) // in world and in range
				{
					socket->sendContainer( pi );
					return;
				}
			}
			P_CHAR pco;
			pco = GetPackOwner(pi);
			
			if( pc_currchar->inRange( pco, 2 ) || pc_currchar->inRange( pi, 2 ) )
			{	
				if (pco == NULL)// reorganized by AntiChrist to avoid crashes
					socket->sendContainer( pi );
				else if (pc_currchar->serial == pco->serial || pc_currchar->isGMorCounselor() || pco->npcaitype() == 17)
					socket->sendContainer( pi );
				else
					Skills->Snooping( pc_currchar, pi );
			}
			else
			{
				socket->sysMessage(tr("You are too far away!"));
			}
		}
		return;
	case 2: // Order gates?
		{
			AllItemsIterator iterItems;
			for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
			{
				P_ITEM pj = iterItems.GetData();
				if (pj->type() == 3)
				{
					if (pj->morez == 1)
					{
						pj->morez = 2;
						pj->pos.z = pj->pos.z + 17;
						RefreshItem(pj);// AntiChrist
						w = 1;
					}
					else if (pj->morez == 2)
					{
						pj->morez = 1;
						pj->pos.z = pj->pos.z - 17;
						RefreshItem(pj);// AntiChrist
						w = 0;
					}
				}
			}
		}
		return;// order gates
	case 4: // Chaos gates?
		{
			AllItemsIterator iterItems;
			for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
			{
				P_ITEM pj = iterItems.GetData();
				if (pj->type() == 5)
				{
					if (pj->morez == 3)
					{
						pj->morez = 4;
						pj->pos.z = pj->pos.z + 17;							
						RefreshItem(pj);// AntiChrist
						w = 1;
					}
					else if (pj->morez == 4)
					{
						pj->morez = 3;
						pj->pos.z = pj->pos.z - 17;							
						RefreshItem(pj);// AntiChrist
						w = 0;
					}
				}
			}
			//						if (w==1) sysmessage(s, "Chaos Gate opened.");
			//						else sysmessage(s, "Chaos Gate closed.");
		}
		return;// chaos gates
	case 6: // teleport item (ring?)
		target(s, 0, 1, 0, 2, "Select teleport target.");
		return;// case 6
	case 7: // key
		addid1[s] = pi->more1;
		addid2[s] = pi->more2;
		addid3[s] = pi->more3;
		addid4[s] = pi->more4;
		
		if (pi->more1 == 255)
			addid1[s] = 255;
		
		target(s, 0, 1, 0, 11, "Select item to use the key on.");
		return;// case 7 (keys)
	case 8: // locked item spawner
	case 64: // locked container //Morrolan traps?
		// Added traps effects by AntiChrist
		pc_currchar->objectdelay = 0;
		if (pi->moreb1())
			Magic->MagicTrap(pc_currchar, pi);
		sysmessage(s, "This item is locked.");
		return;// case 8/64 (locked container)
	case 9: // spellbook
		useSpellBook( s, pc_currchar, pi );
		return;
	case 10: // map?
		LongToCharPtr(pi->serial, &map1[1]);
		LongToCharPtr(pi->serial, &map2[1]);
/*
		By Polygon:
		Assign areas and map size before sending
*/
		map1[7] = pi->more1;	// Assign topleft x
		map1[8] = pi->more2;
		map1[9] = pi->more3;	// Assign topleft y
		map1[10] = pi->more4;
		map1[11] = pi->moreb1();	// Assign lowright x
		map1[12] = pi->moreb2();
		map1[13] = pi->moreb3();	// Assign lowright y
		map1[14] = pi->moreb4();
		int width, height;		// Tempoary storage for w and h;
		width = 134 + (134 * pi->morez);	// Calculate new w and h
		height = 134 + (134 * pi->morez);
		map1[15] = width>>8;
		map1[16] = width%256;
		map1[17] = height>>8;
		map1[18] = height%256;
//		END OF: By Polygon

		Xsend(s, map1, 19);
		Xsend(s, map2, 11);
		return;// maps
	case 11: // book (not spellbooks)
	{
		cBook* pBook = dynamic_cast< cBook* >(pi);
		if( pBook )
		{
			pc_currchar->objectdelay = 0;
			pBook->open( socket );
		}
		return;
	}	
	case 12: // door(unlocked)
		pc_currchar->objectdelay = 0;
		dooruse( socket, pi );
		return;
	case 13: // locked door
		{
			P_ITEM pPack = pc_currchar->getBackpack();
			if( pPack )
			{
				vector<SERIAL> vecContainer = contsp.getData(pPack->serial);
				for( UINT32 j = 0; j < vecContainer.size(); ++j )
				{
					P_ITEM pj = FindItemBySerial(vecContainer[j]);
					if( pj && pj->type() == 7 )
						if( ( ( pj->more1 == pi->more1 ) &&( pj->more2 == pi->more2 ) && (pj->more3 == pi->more3) &&(pj->more4 == pi->more4)))
						{
							socket->sysMessage( tr( "You quickly unlock, use, and then relock the door." ) );
							pc_currchar->objectdelay = 0;
							dooruse( socket, pi );
							return;
						}
				}
			}
			socket->sysMessage( tr( "This door is locked." ) );
			return;
		}
	case 14: // For eating food
		pc_currchar->objectdelay = 0;
		if (pi->isLockedDown())
			return; // Ripper..cant eat locked down food :)
		
		if (pc_currchar->hunger() >= 6)
		{
			socket->sysMessage(tr("You are simply too full to eat any more!"));
			return;
		}
		else
		{
			switch (RandomNum(0, 2))
			{
			case 0: soundeffect2(pc_currchar, 0x003A);		break;
			case 1: soundeffect2(pc_currchar, 0x003B);		break;
			case 2: soundeffect2(pc_currchar, 0x003C);		break;
			}// switch(foodsnd)
			
			switch (pc_currchar->hunger())
			{
			case 0:  socket->sysMessage(tr("You eat the food, but are still extremely hungry."));		break;
			case 1:  socket->sysMessage(tr("You eat the food, but are still extremely hungry."));		break;
			case 2:  socket->sysMessage(tr("After eating the food, you feel much less hungry."));		break;
			case 3:  socket->sysMessage(tr("You eat the food, and begin to feel more satiated."));		break;
			case 4:  socket->sysMessage(tr("You feel quite full after consuming the food."));			break;
			case 5:  socket->sysMessage(tr("You are nearly stuffed, but manage to eat the food."));		break;
			case 6:  
			default: socket->sysMessage(tr("You are simply too full to eat any more!"));				break;
			}// switch(pc_currchar->hunger)
			
			if ((pi->poisoned) &&(pc_currchar->poisoned() < pi->poisoned)) 
			{
				socket->sysMessage(tr("You have been poisoned!"));
				soundeffect2(pc_currchar, 0x0246); // poison sound - SpaceDog
				pc_currchar->setPoisoned( pi->poisoned );
				pc_currchar->setPoisontime( uiCurrentTime +(MY_CLOCKS_PER_SEC*(40/pc_currchar->poisoned()))); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
				pc_currchar->setPoisonwearofftime( pc_currchar->poisontime() +(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()) ); // wear off starts after poison takes effect - AntiChrist
				impowncreate(s, pc_currchar, 1); // Lb, sends the green bar ! 
			}
			
			pi->ReduceAmount(1);	// Remove a food item
			pc_currchar->setHunger( pc_currchar->hunger()+1 );
		}// else
		return; // case 14 (food)
	// Wands
	case 15:
		useWand( s, pc_currchar, pi );
		return; // case 15 (magic items)
	case 18: // crystal ball?
		switch (RandomNum(0, 9))
		{
		case 0: itemmessage(s, "Seek out the mystic llama herder.", pi->serial);									break;
		case 1: itemmessage(s, "Wherever you go, there you are.", pi->serial);										break;
		case 2: itemmessage(s, "Quick! Lord Binary is giving away gold at the castle!", pi->serial);				break;
		case 3: itemmessage(s, "Ripper is watching you every move.", pi->serial);									break;
		case 4: itemmessage(s, "The message appears to be too cloudy to make anything out of it.", pi->serial);		break;
		case 5: itemmessage(s, "You have just lost five strength.. not!", pi->serial)			;					break;
		case 6: itemmessage(s, "You're really playing a game you know", pi->serial);								break;
		case 7: itemmessage(s, "You will be successful in all you do.", pi->serial);								break;
		case 8: itemmessage(s, "You are a person of culture.", pi->serial);											break;
		case 9: itemmessage(s, "Give me a break! How much good fortune do you expect!", pi->serial);				break;
		}// switch
		soundeffect(s, 0x01, 0xEC);
		return;// case 18 (crystal ball?)
		case 19: // potions
			usepotion(currchar[s], pi);
			return; // case 19 (potions)					
			
		case 50: // rune
			if (pi->morex == 0 && pi->morey == 0 && pi->morez == 0)
			{
				socket->sysMessage(tr("That rune is not yet marked!"));
			}
			else
			{
				pc_currchar->inputmode = cChar::enRenameRune;
				pc_currchar->inputitem = pi->serial;
				sysmessage(s, "Enter new rune name.");
			}
			return;// case 50 (rune)
			// taken from 6904t2(5/10/99) - AntiChrist
		case 181: // Fireworks wands
			int wx, wy, wi;
			if (pi->morex <= 0)
			{
				socket->sysMessage(tr("That is out of charges."));
				return;
			}
			pi->morex--;
			sprintf((char*)temp, "Your wand now has %i charges left", pi->morex);
			socket->sysMessage((char*) temp);
			
			for (wi = 0; wi <(rand()%4 + 1); wi++)
			{
				wx = (pc_currchar->pos.x +(rand()%11 - 5));
				wy = (pc_currchar->pos.y +(rand()%11 - 5));
				movingeffect3(currchar[s], (unsigned short)(wx), (unsigned short)(wy), pc_currchar->pos.z + 10, (unsigned char)(0x36), (unsigned char)(0xE4), 17, 0, rand()%2);
				switch (RandomNum(0, 4))
				{
				case 0:	staticeffect3(wx, wy, pc_currchar->pos.z + 10, 0x37, 0x3A, 0x09, 0, 0);	break;
				case 1:	staticeffect3(wx, wy, pc_currchar->pos.z + 10, 0x37, 0x4A, 0x09, 0, 0);	break;
				case 2:	staticeffect3(wx, wy, pc_currchar->pos.z + 10, 0x37, 0x5A, 0x09, 0, 0);	break;
				case 3:	staticeffect3(wx, wy, pc_currchar->pos.z + 10, 0x37, 0x6A, 0x09, 0, 0);	break;
				case 4: staticeffect3(wx, wy, pc_currchar->pos.z + 10, 0x37, 0x7A, 0x09, 0, 0);	break;
				}
			}
			return;
			
		case 185: // let's smoke! :)
			pc_currchar->setSmokeTimer(pi->morex*MY_CLOCKS_PER_SEC + getNormalizedTime());
			Items->DeleItem(pi);
			return;
		case 186: // rename deed! -- eagle 1/29/00
			pc_currchar->inputitem = pi->serial;
			sysmessage(s, "Enter your new name.");
			Items->DeleItem(pi);
			return;// rename deed! -- eagle 1/29/00
		case 187: // Ripper...slotmachine
            if( pc_currchar->inRange( pi, 1 ) )
			{ 
	             slotmachine(s, pi);
				 return;
			} 
            else 
			{ 
	             sysmessage(s, "You need to be closer to use that."); 
			}
            return;// Ripper
		case 100:  // type 100?  this ain't in the docs... - Morrolan
			{
				AllItemsIterator it;
				for (it.Begin(); !it.atEnd(); it++)
				{
					P_ITEM pj = it.GetData();
					if (((pj->moreb1() == pi->morex) &&(pj->moreb2() == pi->morey) &&(pj->moreb3() == pi->morez))
						||((pj->morex == pi->morex) &&(pj->morey == pi->morey) &&(pj->morez == pi->morez))
						&&((pj != pi) &&(pi->morex != 0) &&(pi->morey != 0) &&(pi->morez != 0)))
					{ 
						if ((pj->morex == 0) &&(pj->morey == 0) &&(pj->morez == 0))
						{ 
							pj->morex = pj->moreb1();
							pj->morey = pj->moreb2();
							pj->morez = pj->moreb3();
							pj->visible = 0;								
							RefreshItem(pj);// AntiChrist
						} 
						else 
						{
							pj->setMoreb1( pj->morex );
							pj->setMoreb2( pj->morey );
							pj->setMoreb3( pj->morez );
							pj->morex = 0;
							pj->morey = 0;
							pj->morez = 0;
							pj->visible = 2;							
							RefreshItem(pj);// AntiChrist
						}
					}
				}
			}
			return; // case 100
		case 101: //??
			pc_currchar->setId( pi->morex ); 
			teleport(pc_currchar);
			pi->setType( 102 );
			return; // case 101
		case 102: //??
			pc_currchar->setId(pc_currchar->xid); 
			teleport(pc_currchar);
			pi->setType( 101 );
			return; // case 102
		case 103: // Army enlistment
			enlist(s, pi->morex);
			Items->DeleItem(pi);
			return; // case 103 (army enlistment object)
		case 104: // Teleport object
			pc_currchar->MoveTo(pi->morex,pi->morey,pi->morez);
			teleport(currchar[s]);
			return; // case 104 (teleport object (again?))
		case 105:  // For drinking
			switch (RandomNum(0, 1))
			{
			case 0: soundeffect2(currchar[s], 0x31);		break;
			case 1: soundeffect2(currchar[s], 0x30);		break; 
			}// switch(drinksnd)
			
			pi->ReduceAmount(1);	// Remove a drink
			sysmessage(s, "Gulp !");
			return; //	case 105 (drinks)
			case 202:
				if ( pi->id() == 0x14F0  ||  pi->id() == 0x1869 )	// Check for Deed/Teleporter + Guild Type
				{
					pc_currchar->fx1 = pi->serial;
					StonePlacement(s);
					return;
				}
				else if (pi->id() == 0x0ED5)	// Check for Guildstone + Guild Type
				{
					pc_currchar->fx1 = pi->serial;
					cGuildStone *pStone = dynamic_cast<cGuildStone*>(pi);
					if ( pStone != NULL )
						pStone->Menu(s, 1);
					return;
				}
				else 
					clConsole.send("Unhandled guild item type named: %s with ID of: %X", pi->name().ascii(), pi->id());
				return;
				// End of guild stuff
		case 203: // Open a gumpmenu - Crackerjack 8/9/99
				if (! pc_currchar->Owns(pi)) // bugfix LB 5.10.99
				{
					if (!(pc_currchar->isGM()))
					{
						sysmessage(s, "You can not use that.");
						return;
					}
				}
				addid1[s] = static_cast<unsigned char>((pi->serial&0xFF000000)>>24);
				addid2[s] = static_cast<unsigned char>((pi->serial&0x00FF0000)>>16);
				addid3[s] = static_cast<unsigned char>((pi->serial&0x0000FF00)>>8);
				addid4[s] = static_cast<unsigned char>((pi->serial&0x000000FF));

//				cGumps::instance()->Menu( s, pi->morex, pi );
				return;
		// Cannon Ball
		case 204:
			useCannonBall( s, pc_currchar, pi );
			return;
		case 217:			// PlayerVendors deed
			{	
				if (pi->isLockedDown())
				{
					socket->sysMessage(tr("That item is locked down."));
					return; 
				} // added by ripper, bugfixed by LB
				P_ITEM pi_multi = findmulti(pc_currchar->pos); // boats are also multis zippy, btw !!!		
				if (pi_multi != NULL && pc_currchar->inRange( pi_multi, 18 ) )
				{	
					if (!IsHouse(pi_multi->id()))
						return; // LB
					int los = 0;
					const P_ITEM pi_p = Packitem(pc_currchar);
					if (pi_p != NULL)
					{
						los = 0;
						vector<SERIAL> vecContainer = contsp.getData(pi_p->serial);
						unsigned int j;
						for (j = 0; j < vecContainer.size(); j++)
						{
							const P_ITEM pi_i = FindItemBySerial(vecContainer[j]);
							if ((pi_i != NULL) && (pi_p != NULL)) // lb
								if (pi_i->type() == 7 && calcserial(pi_i->more1, pi_i->more2, pi_i->more3, pi_i->more4) == pi_multi->serial)
								{
									los = 1;
									break;
								}
						}
					}
				
					if (los)
					{
						P_CHAR pc_vendor = Npcs->createScriptNpc(-1, NULL, "2117", pc_currchar->pos.x, pc_currchar->pos.y, pc_currchar->pos.z);
						
						if (pc_vendor == NULL) 
						{
							clConsole.send("npc-script couldnt find vendor in npc-section 2117!\n");
							return;
						}
						
						pc_vendor->setNpcAIType( 17 );
						pc_vendor->makeInvulnerable();
						pc_vendor->setHidden( 0 );
						pc_vendor->setStealth(-1);
						pc_vendor->dir = pc_currchar->dir;
						pc_vendor->npcWander = 0;
						pc_vendor->setInnocent();
						pc_vendor->SetOwnSerial(pc_currchar->serial);
						pc_vendor->setTamed(false);
						Items->DeleItem(pi);
						sprintf((char*)temp, "Hello sir! My name is %s and i will be working for you.", pc_vendor->name.c_str());
						npctalk(s, pc_vendor, (char*)temp, 1);
						updatechar(pc_vendor);
						teleport(pc_vendor);
					}
					else 
						socket->sysMessage(tr("You must be close to a house and have a key in your pack to place that."));
				}
				else if (pi_multi == NULL)
					socket->sysMessage(tr("You must be close to a house and have a key in your pack to place that."));
				
				return;
			}
		
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
			LongToCharPtr(pi->serial, &map1[1]);
			LongToCharPtr(pi->serial, &map2[1]);
			map1[7] = pi->more1;	// Assign topleft x
			map1[8] = pi->more2;
			map1[9] = pi->more3;	// Assign topleft y
			map1[10] = pi->more4;
			map1[11] = pi->moreb1();	// Assign lowright x
			map1[12] = pi->moreb2();
			map1[13] = pi->moreb3();	// Assign lowright y
			map1[14] = pi->moreb4();
			map1[15] = 0x01;			// Let width and height be 256
			map1[16] = 0x00;
			map1[17] = 0x01;
			map1[18] = 0x00;
			Xsend(s, map1, 19);

			Xsend(s, map2, 11);

			// Generate message to add a map point
			LongToCharPtr(pi->serial, &map3[1]);
			int posx, posy;			// tempoary storage for map point
			int tlx, tly, lrx, lry;	// tempoary storage for map extends
			tlx = (pi->more1 << 8) + pi->more2;
			tly = (pi->more3 << 8) + pi->more4;
			lrx = (pi->moreb1() << 8) + pi->moreb2();
			lry = (pi->moreb3() << 8) + pi->moreb4();
			posx = (256 * (pi->morex - tlx)) / (lrx - tlx);	// Generate location for point
			posy = (256 * (pi->morey - tly)) / (lry - tly);
			map3[7] = posx>>8;	// Store the point position
			map3[8] = posx%256;
			map3[9] = posy>>8;
			map3[10] = posy%256;
			Xsend(s, map3, 11);	// Fire data to client :D
			return;
// END OF: By Polygon
		case 401: // Blackwinds JAIL BALL 
            if (pc_currchar->jailsecs>0) 
			{ 
	             sprintf((char*)temp, "You have %i seconds left in the jail", (pc_currchar->jailtimer - uiCurrentTime) / MY_CLOCKS_PER_SEC); 
	             itemmessage(s, (char*) temp, pi->serial); 
			} 
            else 
			{ 
	             socket->sysMessage(tr("Crystall ball shatters..")); 
	             pi->ReduceAmount(1);
				 RefreshItem(pi);
			}
            return;// End jailball
	    case 402: // Blackwinds Reputation ball 
			{ 
                 soundeffect2(pc_currchar, 0x01ec); // Play sound effect for player 
                 socket->sysMessage(tr("Your karma is %1").arg(pc_currchar->karma)); 
                 socket->sysMessage(tr("Your fame is %1").arg(pc_currchar->fame)); 
                 socket->sysMessage(tr("Your Kill count is %1 ").arg(pc_currchar->kills)); 
                 socket->sysMessage(tr("You died %1 times.").arg(pc_currchar->deaths));
				 staticeffect(pc_currchar, 0x37, 0x2A, 0x09, 0x06 );
				 socket->sysMessage(tr("*The crystal ball seems to have vanished*"));
                 pi->ReduceAmount(1); 
                 return; 
			}
		case 404: // Fraz'z ID wand
			{
				P_ITEM pBackpack = Packitem(pc_currchar);
				if ( pBackpack != NULL )
				{
					if ((pi->contserial == pBackpack->serial) || pc_currchar->Wears(pi) &&(pi->layer() == 1))
					{
						if (pi->morex <= 0)
						{
							sysmessage(s,  "That is out of charges.");
							return;
						}
						pi->morex--;
						sprintf((char*)temp, "Your wand now has %i charges left", pi->morex);
						socket->sysMessage((char*) temp);
						target(s, 0, 1, 0, 75, "What do you wish to identify?");
					}
					else
					{
						socket->sysMessage(tr("If you wish to use this, it must be equipped or in your backpack."));
					}
				}
				return;
			}
		case 1000: // Ripper...bank checks
			{
				socket->sysMessage(tr("To cash this, you need to drop it on a banker."));
				return;
			}
		default:						
			break; // case (itype!) 
		}// switch (itype)
		// END Check items by type
		
		// Begin checking objects by ID
		if (!pi->isLockedDown()) // Ripper
		{
			switch (pi->id())
			{	
				case 0x0FA9:// dye
					dyeall[s] = 0;
					target(s, 0, 1, 0, 31, "Which dye vat will you use this on?");
					return;// dye
				case 0x0FAF:
				case 0x0FB0: // Anvils
					if( !pc_currchar->inRange( pi, 3 ) )
					{
						socket->sysMessage(tr("Must be closer to use this!"));
						return;
					}
					target(s, 0, 1, 0, 236, "Select item to be repaired.");
					return;
				case 0x0FB1: // small forge
				case 0x1982: // partial lg forge
				case 0x197A: // partial lg forge
				case 0x197E: // partial lg forge
				case 0x1986: // partial lg forge
				case 0x198A: // partial lg forge
				case 0x198E: // partial lg forge
					if( !pc_currchar->inRange( pi, 3 ) )
					{
						socket->sysMessage(tr("Must be closer to use this!"));
						return;
					}
					target(s, 0, 1, 0, 237, "What item would you like to Smelt?");
					return; // Ripper..Smelting items.
				case 0x0FAB:// dye vat
					addid1[s] = static_cast<unsigned char>(pi->color()>>8);
					addid2[s] = static_cast<unsigned char>(pi->color()%256);
					target(s, 0, 1, 0, 32, "Select the clothing to use this on.");
					return;// dye vat
				case 0x14F0:// deeds
					if ((pi->type() != 103) &&(pi->type() != 202))
					{  
						QDomElement* DefSection = DefManager->getSection( WPDT_MULTI, pi->tags.get( "multisection" ).toString() );
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
								socket->attachTarget( new cBuildMultiTarget( pi->tags.get( "multisection" ).toString() , pc_currchar->serial, pi->serial ), houseid );
						}
					}
					return;// deeds
				case 0x100A:
				case 0x100B:// archery butte
					Skills->AButte(s, pi);
					return;// archery butte
				case 0x0E9C:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x0038);
					else 
						soundeffect2(currchar[s], 0x0039);
					return;
				case 0x0E9D:
				case 0x0E9E:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x0052);
					else 
						soundeffect2(currchar[s], 0x0053);
					return;
				case 0x0EB1:
				case 0x0EB2:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x0045);
					else 
						soundeffect2(currchar[s], 0x0046);
					return;
				case 0x0EB3:
				case 0x0EB4:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x004C);
					else 
						soundeffect2(currchar[s], 0x004D);
					return;
				case 0x0F43:// Axe, Double Axe and Bardiche
				case 0x0F44:
				case 0x0F45:
				case 0x0F46:
				case 0x0F47:
				case 0x0F48:
				case 0x0F49:
				case 0x0F4A:
				case 0x0F4B:
				case 0x0F4C:
				case 0x0F4D:
				case 0x0F4E:// Axe, Double Axe and Bardiche
				case 0x13AF:
				case 0x13B0:
				case 0x13FA:
				case 0x13FB:// Large Battle Axe
				case 0x1442:
				case 0x1443:// Two Handed Axe
				case 0x143E:// Halberd
				case 0x143F:
					addmitem[s] = pi->serial; // save the item number, AntiChrist
					target(s, 0, 1, 0, 76, "What would you like to use that on ?");
					return;// axes
					
				case 0x102A:// Hammer
				case 0x102B:
				case 0x0FBB:// tongs
				case 0x0FBC:
				case 0x13E3:// smith's hammers
				case 0x13E4:
				case 0x0FB4:// sledge hammers
				case 0x0FB5:
					if (!Item_ToolWearOut(s, pi))
						target(s, 0, 1, 0, 50, "Select material to use.");
					return; // Smithy
					
				case 0x1026:// Chisels
				case 0x1027:
				case 0x1028:// Dove Tail Saw
				case 0x1029:
				case 0x102C:// Moulding Planes
				case 0x102D:
				case 0x102E:// Nails
				case 0x102F:
				case 0x1030:// Jointing plane
				case 0x1031:
				case 0x1032:// Smoothing plane
				case 0x1033:
				case 0x1034:// Saw
				case 0x1035:
					target(s, 0, 1, 0, 134, "Select material to use.");
					return; // carpentry
					
				case 0x0E85:// pickaxes
				case 0x0E86:
				case 0x0F39:// shovels
				case 0x0F3A:
					if (!Item_ToolWearOut(s, pi))
					{
						addmitem[s] = pi->serial; // save the item number, AntiChrist
						target(s, 0, 1, 0, 51, "Where do you want to dig?");
					}
					return; // mining
				case 0x0E24: // empty vial
					{
					P_ITEM pBackpack = Packitem(pc_currchar);
					if (pBackpack != NULL)
						if (pi->contserial == pBackpack->serial)
						{
							addmitem[s] = pi->serial; // save the vials number, LB
							target(s, 0, 1, 0, 186, "What do you want to fill the vial with?");
						}
						else 
							socket->sysMessage(tr("The vial is not in your pack"));
						return;
					}
				case 0x0DF9: 
					pc_currchar->setTailItem( pi->serial );
					target(s, 0, 1, 0, 166, "Select spinning wheel to spin cotton.");
					return;
					/*
					case 0x09F1: // Raw meat to Cooked
					pc_currchar->tailitem = x;   
					target(s, 0, 1, 0, 168, "Select where to cook meat on.");
					return;
					*/
				case 0x0FA0:
				case 0x0FA1: // thread to Bolt
				case 0x0E1D:
				case 0x0E1F:
				case 0x0E1E:  // yarn to cloth
					pc_currchar->setTailItem(  pi->serial );
					target(s, 0, 1, 0, 165, "Select loom to make your cloth");
					return;
				case 0x1BD1:
				case 0x1BD2:
				case 0x1BD3:
				case 0x1BD4:
				case 0x1BD5:
				case 0x1BD6:	// make shafts
					itemmake[s].Mat1id = pi->id();
					target(s, 0, 1, 0, 172, "What would you like to use this with?"); 
					return;
				case 0x0E73: // cannon ball
					target(s, 0, 1, 0, 170, "Select cannon to load."); 
					Items->DeleItem(pi);
					return;
				case 0x0FF8:
				case 0x0FF9: // pitcher of water to flour
					pc_currchar->setTailItem( pi->serial );
					target(s, 0, 1, 0, 173, "Select flour to pour this on.");  
					return;
				case 0x09C0:
				case 0x09C1: // sausages to dough
					pc_currchar->setTailItem( pi->serial );
					target(s, 0, 1, 0, 174, "Select dough to put this on.");  
					return;
				case 0x0DF8: // wool to yarn 
					pc_currchar->setTailItem( pi->serial );
					target(s, 0, 1, 0, 164, "Select your spin wheel to spin wool.");      
					return;
				case 0x0F9D: // sewing kit for tailoring
					target(s, 0, 1, 0, 167, "Select material to use.");
					return;
				case 0x19B7:
				case 0x19B9:
				case 0x19BA:
				case 0x19B8: // smelt ore
					pc_currchar->setSmeltItem( pi->serial );
					target(s, 0, 1, 0, 52, "Select forge to smelt ore on.");// smelting  for all ore changed by Myth 11/12/98
					return;
				case 0x1E5E:
				case 0x1E5F: // Message board opening
					MsgBoardEvent(s);
					return;
				case 0x0EC4:// skinning knife 1 - antichrist
				case 0x0EC5:// skinning knife 2
				case 0x0F51:// Dagger 1
				case 0x0F52:// Dagger 2
				case 0x0EC2:// cleaver 1
				case 0x0EC3:// cleaver 2 - antichrist
				case 0x0F5E:
				case 0x0F61:// Broad Sword Long Sword1
				case 0x13B5:
				case 0x13B6:
				case 0x13BA:// Scimitar Long Sword2 Viking Sword 
				case 0x13FE:
				case 0x13FF:// Katana
				case 0x1400:
				case 0x1401:// Kryss
				case 0x1440:
				case 0x1441:
					target(s, 0, 1, 0, 86, "What would you like to use that on ?");
					return;
				case 0x0DE1:
				case 0x0DE2: // camping
					if (Skills->CheckSkill(currchar[s], CAMPING, 0, 500)) // Morrolan TODO: insert logout code for campfires here
					{
						P_ITEM pFire = Items->SpawnItem(currchar[s], 1, "#", 0, 0x0DE3, 0, 0);
						if (pFire)
						{
							pFire->setType( 45 );
							pFire->dir = 2;
							if (pi->isInWorld())
								pFire->moveTo(pi->pos);
							else
								pFire->moveTo(pc_currchar->pos);

							pFire->priv |= 1;
//							pFire->decaytime = (uiCurrentTime +(SrvParms->decaytimer*MY_CLOCKS_PER_SEC));							
							pFire->startDecay();
							RefreshItem(pFire);// AntiChrist
							pi->ReduceAmount(1);
						}
					}
					else
					{
						sysmessage(s, "You fail to light a fire.");
					}
					return; // camping
				case 0x1508: // magic statue?
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1509);
						pi->setType( 45 );
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						socket->sysMessage(tr("You failed to use this statue."));
					}
					return;
				case 0x1509:
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1508);
						pi->setType( 45 );
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						socket->sysMessage(tr("You failed to use this statue."));
					}
					return;
				case 0x1230:
				case 0x1246: // guillotines?
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1245);
						pi->setType( 45 );				
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						socket->sysMessage(tr("You failed to use this."));
					}  
					return;
				case 0x1245: // Guillotine stop animation
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1230);
						pi->setType( 45 );					
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						socket->sysMessage(tr("You failed to use this."));
					}
					return;
				case 0x1039:  // closed flour sack
							  /*
							  if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
							  {
							  pi->id1 = 0x10;
							  pi->id2 = 0x3A;
							  pi->type = 45;						
							  RefreshItem(x);// AntiChrist
							  }
							  else
							  {
							  sysmessage(s, "You failed to use this.");
							  }
					*/
					return;
				case 0x103A: // open flour sack
							 /*
							 if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
							 {
							 pi->id1 = 0x10;
							 pi->id2 = 0x39;
							 pi->type = 45;						
							 RefreshItem(x);// AntiChrist
							 }
							 else
							 {
							 sysmessage(s, "You failed to use this.");
							 }
					*/
					return;
				case 0x0DBF:
				case 0x0DC0:// fishing
					target(s, 0, 1, 0, 45, "Fish where?");
					return;
				case 0x104B:
				case 0x104C:
				case 0x1086: // Clock and bracelet
					telltime(s);
					return;
				case 0x0E9B: // Mortar for Alchemy
					if (pi->type() == 17)
					{
						addid1[s] = static_cast<unsigned char>((pi->serial&0xFF000000)>>24);
						addid2[s] = static_cast<unsigned char>((pi->serial&0x00FF0000)>>16);
						addid3[s] = static_cast<unsigned char>((pi->serial&0x0000FF00)>>8);
						addid4[s] = static_cast<unsigned char>((pi->serial&0x000000FF));
						target(s, 0, 1, 0, 109, "Where is an empty bottle for your potion?");
					}
					else
					{
						addid1[s] = static_cast<unsigned char>((pi->serial&0xFF000000)>>24);
						addid2[s] = static_cast<unsigned char>((pi->serial&0x00FF0000)>>16);
						addid3[s] = static_cast<unsigned char>((pi->serial&0x0000FF00)>>8);
						addid4[s] = static_cast<unsigned char>((pi->serial&0x000000FF));
						target(s, 0, 1, 0, 108, "What do you wish to grind with your mortar and pestle?");
					}
					return; // alchemy
				case 0x0F9E:
				case 0x0F9F: // scissors
					target(s, 0, 1, 0, 128, "What cloth should I use these scissors on?");
					return;
				case 0x0E21: // healing
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 130, "Who will you use the bandages on?");
					return;
				case 0x1057:
				case 0x1058: // sextants
					
					getSextantCords(pc_currchar->pos.x, pc_currchar->pos.y, socket->isT2A(), temp2);
					sprintf((char*)temp, "You are at: %s", temp2);
					sysmessage(s, (char*)temp);
					return;
				case 0x0E27:
				case 0x0EFF:   // Hair Dye
					usehairdye(s, pi);
					return;
				case 0x14FB:
				case 0x14FC:
				case 0x14FD:
				case 0x14FE: // lockpicks
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 162, "What lock would you like to pick?");
					return;
				case 0x097A: // Raw Fish steaks
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 49, "What would you like to cook this on?");
					return;
				case 0x09b9: // Raw Bird
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 54, "What would you like to cook this on?");
					return;
				case 0x1609: // Raw Lamb
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 55, "What would you like to cook this on?");
					return;
				case 0x09F1: // Raw Ribs
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 68, "What would you like to cook this on?");
					return;
				case 0x1607: // Raw Chicken Legs
					addmitem[s] = pi->serial;
					target(s, 0, 1, 0, 69, "What would you like to cook this on?");
					return;
				case 0x0C4F:
				case 0x0C50:
				case 0x0C51:
				case 0x0C52:
				case 0x0C53:
				case 0x0C54: // cotton plants
					{
						if (!pc_currchar->onHorse())
							action(s, 0x0D);
						else 
							action(s, 0x1d);
						soundeffect(s, 0x01, 0x3E);
						P_ITEM p_cotton = Items->SpawnItem(-1, pc_currchar, 1, "#", 1, 0x0D, 0xF9, 0, 1, 1);
						if ( p_cotton == NULL )
							return;
						p_cotton->setContSerial(Packitem(pc_currchar)->serial);
						sysmessage(s, "You reach down and pick some cotton.");
					}
					return; // cotton
				case 0x105B:
				case 0x105C:
				case 0x1053:
				case 0x1054: // tinker axle
					addid1[s] = static_cast<unsigned char>((pi->serial&0xFF000000)>>24);
					addid2[s] = static_cast<unsigned char>((pi->serial&0x00FF0000)>>16);
					addid3[s] = static_cast<unsigned char>((pi->serial&0x0000FF00)>>8);
					addid4[s] = static_cast<unsigned char>((pi->serial&0x000000FF));
					target(s, 0, 1, 0, 183, "Select part to combine that with.");
					return;
				case 0x1051:
				case 0x1052:
				case 0x1055:
				case 0x1056:
				case 0x105D:
				case 0x105E:
					addid1[s] = static_cast<unsigned char>((pi->serial&0xFF000000)>>24);
					addid2[s] = static_cast<unsigned char>((pi->serial&0x00FF0000)>>16);
					addid3[s] = static_cast<unsigned char>((pi->serial&0x0000FF00)>>8);
					addid4[s] = static_cast<unsigned char>((pi->serial&0x000000FF));
					// itemmake[s].materialid1=pi->id1;
					// itemmake[s].materialid2=pi->id2;
					target(s, 0, 1, 0, 184, "Select part to combine it with.");
					return;
				case 0x104F:
				case 0x1050:
				case 0x104D:
				case 0x104E:// tinker clock
					addid1[s] = static_cast<unsigned char>((pi->serial&0xFF000000)>>24);
					addid2[s] = static_cast<unsigned char>((pi->serial&0x00FF0000)>>16);
					addid3[s] = static_cast<unsigned char>((pi->serial&0x0000FF00)>>8);
					addid4[s] = static_cast<unsigned char>((pi->serial&0x000000FF));
					target(s, 0, 1, 0, 185, "Select part to combine with");
					return;
				case 0x1059:
				case 0x105A:// tinker sextant
					if (Skills->CheckSkill(currchar[s], TINKERING, 500, 1000))
					{
						socket->sysMessage(tr("You create the sextant."));
						P_ITEM pi_sextant = Items->SpawnItem(s, currchar[s], 1, "a sextant", 0, 0x10, 0x57, 0, 1, 1);
						if (pi_sextant != NULL)
							pi_sextant->priv |= 0x01;
						pi->ReduceAmount(1);
					}
					else 
						socket->sysMessage(tr("you fail to create the sextant."));
					return;
				case 0x1EBC: // tinker's tools
					target(s, 0, 1, 0, 180, "Select material to use.");
					return;
				default:
					//	clConsole.send("Unhandled item id for item: %s with id: %X.",pi->name, itemids); //check for unused items - Morrolan
					break;
			}// switch (itemids)
		}
		// END Check items by ID
		socket->sysMessage(tr("You can't think of a way to use that item."));
}

//Handles Double clicks over PC/NPCs
void showPaperdoll( cUOSocket *socket, P_CHAR pTarget, bool hotkey )
{
	if( !socket )
		return;

	P_CHAR pChar = socket->player();

	if( !pChar || !pTarget )
		return;

	UOXSOCKET s = calcSocketFromChar( pChar ); // for legacy :(

	// For players we'll always show the Paperdoll
	if( pTarget->isHuman() || !pTarget->isNpc() )
	{
		// Theres one exception for player vendors, when you double click them
		// Their packs should open instead of their paperdoll
		if( pTarget->npcaitype() == 17 )
		{
			pTarget->talk( tr( "Take a look at my goods" ) );

			if( pTarget->packitem != INVALID_SERIAL )
				socket->sendContainer( pTarget->getBackpack() );

			return;
		}

        // If we're mounted (item on layer 25) and *not* using a hotkey
		// We're trying to unmount ourself
		if( !hotkey && ( pTarget == pChar ) && pChar->unmount() )
			return; // We have been unmounted

		socket->sendPaperdoll( pTarget );
	}

	UINT16 body = pTarget->id();

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
		if( chardist( pChar, pTarget ) <  2 || pChar->isGM() )
		{
			if( !pChar->isHuman() )
			{
				socket->sysMessage( tr( "You are unable to ride an animal" ) );
				return;
			}
			if( pChar->dead )
			{
				socket->sysMessage( tr("You are dead and cannot do that." ) );
				return;
			}
			if( pTarget->war )
				socket->sysMessage( tr("Your pet is in battle right now!" ) );
			else
				mounthorse( s, pTarget );
		}
		else
			socket->sysMessage( tr( "This is too far away" ) );

		break;
	case 0x123:
	case 0x124:
		if( pChar->Owns( pTarget ) )
				socket->sendContainer( pTarget->getBackpack() );
	};
}
