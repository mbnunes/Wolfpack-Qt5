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

#include "dbl_single_click.h"

#undef  DBGFILE
#define DBGFILE "dbl_single_click.cpp"


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
		pi->hp--; //Take off a hit point
	if(pi->hp<=0)
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
		sprintf((char*)temp,"Your %s has been destroyed",pi->name);
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
	int cc=currchar[s];
	if (pi == NULL)
		return;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);
	if(pc_currchar->dead)		// no ghosts playing :)
	{
		sysmessage(s,"ghosts cant do that!");
		return;
	}
	if(itemdist(cc,pi)>3)	// within 3 to play.
	{
		sysmessage(s,"you need to be closer to play!");
		return;
	}
	if(pc_currchar->CountGold() < 5)	// check his gold to see if less than 5gp.
	{
		sysmessage(s,"you dont have enough gold to play!");
		return;
	}
	delequan(cc, 0x0EED, 5, NULL);	// if 5gp or more lets delete 5gp.
	int spin=RandomNum( 0,100);	// now lets spin to win :)
	switch(spin)
	{
	case 0: Items->SpawnItemBackpack2(s,3185,1);
		sysmessage(s,"Single bars, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 1: Items->SpawnItemBackpack2(s,3186,1);
		sysmessage(s,"Double bars, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 2: Items->SpawnItemBackpack2(s,3187,1);
		sysmessage(s,"Triple bars, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 3: Items->SpawnItemBackpack2(s,2003,1);
		sysmessage(s,"Any three 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 4: Items->SpawnItemBackpack2(s,2004,1);
		sysmessage(s,"Three blue 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 5: Items->SpawnItemBackpack2(s,2005,1);
		sysmessage(s,"Three white 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 6: Items->SpawnItemBackpack2(s,2006,1);
		sysmessage(s,"Three red 7`s, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	case 7: Items->SpawnItemBackpack2(s,2007,1);
		sysmessage(s,"Jackpot, you are a winner!!");soundeffect(s, 0x00, 0x38); break;
	default : itemmessage(s,"Sorry,not a winner,please insert coins.",pi->serial); break;
	}
	soundeffect(s, 0x00, 0x57);	// my stupid spin sound hehe.
}

void doubleclick(int s) // Completely redone by Morrolan 07.20.99
{
	int i, j, c, w = 0, k, serial;
	unsigned char a1, a2, a3, a4;
	//	unsigned char map1[20] = "\x90\x40\x01\x02\x03\x13\x9D\x00\x00\x00\x00\x13\xFF\x0F\xA0\x01\x90\x01\x90";
	unsigned char map1[20] = "\x90\x40\x01\x02\x03\x13\x9D\x00\x00\x00\x00\x13\xFF\x0F\xFF\x01\x90\x01\x90";
	// By Polygon: Lower map border is 4095, not 4000, no more needed with new system anyway ;)
	unsigned char map2[12] = "\x56\x40\x01\x02\x03\x05\x00\x00\x00\x00\x00";
	// By Polygon: This one is needed to show the location on treasure maps
	unsigned char map3[12] = "\x56\x40\x01\x02\x03\x01\x00\x00\x00\x00\x00";
	int los = 0;
	bool t2a;
	int itype;
	
	a1 = buffer[s][1]&0x7F;
	a2 = buffer[s][2];
	a3 = buffer[s][3];
	a4 = buffer[s][4];
	serial = calcserial(a1, a2, a3, a4);
	
	if (isCharSerial(serial))
	{
		dbl_click_character(s, serial);
		return;
	}

	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	
	if (pc_currchar->objectdelay != 0 && !pc_currchar->isGM() && pc_currchar->objectdelay > 10 && pc_currchar->objectdelay >= uiCurrentTime || overflow)
	{
		sysmessage(s, "You must wait to perform another action.");
		return;
	}
	else
		pc_currchar->objectdelay = SrvParms->objectdelay * MY_CLOCKS_PER_SEC + uiCurrentTime;
	
	
	P_ITEM pi = FindItemBySerial(serial);
	if (pi == NULL)
		return;

	if (isItemSerial(pi->contserial) && pi->type != 1 && !pi->isInWorld())
	{// Cant use stuff that isn't in your pack.
		P_CHAR pc_p = GetPackOwner(FindItemBySerial(pi->contserial));
		if (pc_p != NULL)
			if (pc_p != pc_currchar)
				return;
	}
	else if (isCharSerial(pi->contserial) && pi->type!=-1 && !pi->isInWorld())
	{// in a character.
		P_CHAR pc_p = FindCharBySerial(pi->contserial);
		if (pc_p != NULL)
			if (pc_p != pc_currchar && pi->layer != 15 && pi->type != 1)
				return;
	}
	
	// Begin Items/Guildstones Section 
	itype = pi->type;

	// Criminal for looting an innocent corpse & unhidden if not owner..Ripper
	if (pi->corpse==1)
	{
		if (pc_currchar->hidden==1 && !pc_currchar->Owns(pi) && !pc_currchar->isGM())
		{
			pc_currchar->hidden=0;
			updatechar(DEREF_P_CHAR(pc_currchar));
		}
		if (!pc_currchar->Owns(pi) && !pc_currchar->isGM() && pc_currchar->isInnocent())
		{
			if (pi->more2==1)
			{
				criminal(DEREF_P_CHAR(pc_currchar));
			}
		}
	}
	
	if (pi->magic == 4 && pi->secureIt == 1)
	{
		if (!pc_currchar->Owns(pi))
		{
			if (!(pc_currchar->isGM()))
			{
				sysmessage(s, "That is a secured chest!");
				return;
			}
		}
	}
	
	if (pc_currchar->dead && itype != 16) // if you are dead and it's not an ankh, FORGET IT!
	{
		sysmessage(s, "You may not do that as a ghost.");
		return;
	}
	else if (!pc_currchar->isGM() && pi->layer != 0 && !pc_currchar->Wears(pi))
	{// can't use other people's things!
		if (!(pi->layer == 0x15  && SrvParms->rogue == 1)) // bugfix for snooping not working, LB
		{
			sysmessage(s, "You cannot use items equipped by other players.");
			return;
		}
	}
	// modified by AntiChrist for use with newSelectSpell2Cast
	// rewritten (Duke 18.3.2001)
		// Attempting to fix many problems -moved here- 7/2001 -Frazurbluu-
	// SWEET, since I have the inBACKPACK working I should make a trigger function detailing object location for firing -Fraz-
	else if ((IsSpellScroll(pi->id())) && (pi->magic != 4))
	{
		k = packitem(DEREF_P_CHAR(pc_currchar));
		const P_ITEM pi_k = MAKE_ITEMREF_LR(k);	// on error return
		if (pi_k != NULL)
			if ((pi->contserial == pi_k->serial))
			{
				currentSpellType[s] = 1;							// a scroll spell, so cut mana req
				short spn = Magic->SpellNumFromScrollID(pi->id());	// avoid reactive armor glitch
				if (Magic->newSelectSpell2Cast(s, spn))				// check cast !
					pi->ReduceAmount(1);							// remove scroll if successful
				// return;  -Fraz- removong return here, and moving up should also allow for triggers on the scrolls
				// after they are used-- effect --cursed scrolls ect..
			}
			else
			{
				sysmessage(s, "The scroll must be in your backpack to envoke its magic.");
			}
		return;
	}
	// Begin checking objects that we force an object delay for (std objects)
	// taken from 6904t2(5/10/99) - AntiChrist
	else if (s >= 0)
	{
		// start trigger stuff
		if (pi->trigger > 0)
		{
			if (pi->trigtype == 0)
			{
				if (pi->disabled <= uiCurrentTime) // changed by Magius(CHE) §
				{
					triggerwitem(s, DEREF_P_ITEM(pi), 1); // if players uses trigger
					return;
				} 
				else 
				{
					if (!pi->disabledmsg[0] == 0x0) 
						sysmessage(s, pi->disabledmsg); // Added by Magius(CHE) §
					else 
						sysmessage(s, "That doesnt seem to work right now.");
					return;
				}
			}
			else 
			{
				sysmessage(s, "You are not close enough to use that.");
				return;
			}
		}
		
		// check this on trigger in the event that the .trigger property is not set on the item
		// trigger code.  Check to see if item is envokable by id
		
		else if (checkenvoke(pi->id1, pi->id2))
		{
			pc_currchar->envokeitem = pi->serial;
			pc_currchar->envokeid1 = pi->id1;
			pc_currchar->envokeid2 = pi->id2;
			target(s, 0, 1, 0, 24, "What will you use this on?");
			return;
		}
		// end trigger stuff
		// BEGIN Check items by type 
	}	
	switch (pi->type)
	{
	case 16:
		// Check for 'resurrect item type' this is the ONLY type one can use if dead.
		if (pc_currchar->dead)
		{
			Targ->NpcResurrectTarget(DEREF_P_CHAR(pc_currchar));
			sysmessage(s, "You have been resurrected.");
			return;
		} 
		else 
		{
			sysmessage(s, "You are already living!");
			return;
		}
		
	case 117:// backpacks - snooping a la Zippy - add check for SrvParms->rogue later- Morrolan
		
		// Boats ->
		if (pi->type2 == 3)
		{
			if (iteminrange(s, pi, 3))
			{
				if (pi->id2 == 0x84 || pi->id2 == 0xD5 || pi->id2 == 0xD4 || pi->id2 == 0x89)
					Boats->PlankStuff(s, pi);
				else 
					sysmessage(s, "That is locked.");
			}
			else 
				sysmessage(s, "You can't reach that!");
			return;
		}
		// End Boats --^
		
	case 1: // normal containers
	case 63:
		if (pi->moreb1)
			Magic->MagicTrap(DEREF_P_CHAR(pc_currchar), pi); // added by AntiChrist
		// only 1 and 63 can be trapped, so pleaz leave it here :) - Anti
	case 65: // nodecay item spawner..Ripper
	case 66: // decaying item spawner..Ripper
		pc_currchar->objectdelay = 0;	// no delay for opening containers

		int npc, contser;
		npc=-1;
		contser = pi->contserial;
		if ((contser <= 0 && iteminrange(s, pi, 2)) ||  // Backpack in world - free access to everyone
			pc_currchar->Wears(pi))	// primary pack
		{
			pc_currchar->objectdelay = 0;
			backpack(s, serial);
			return;
		}
		if (isItemSerial(pi->contserial))
		{
			P_ITEM pio = GetOutmostCont(pi);
			if (pio == NULL) return;		// this should *not* happen, but it does ! Watch the logfiles (Duke)
			if (pc_currchar->Wears(pio) ||	// sub-pack
				(pio->isInWorld() && iteminrange(s, pio, 2)))	// in world and in range
			{
				backpack(s, serial);
				return;
			}
		}
		P_CHAR pco;
		pco = GetPackOwner(pi);
		npc = DEREF_P_CHAR(pco);
		
		if ((npcinrange(s, DEREF_P_CHAR(pco), 2)) || (iteminrange(s, pi, 2)))
		{	
			if (pco == NULL)// reorganized by AntiChrist to avoid crashes
				backpack(s, serial);
			else if (pc_currchar->serial == pco->serial || pc_currchar->isGMorCounselor() || pco->npcaitype == 17)
				backpack(s, serial);
			else
				Skills->Snooping(pc_currchar, pi);
		}
		else
		{
			sysmessage(s, "You are too far away!");
		}
		return;
	case 2: // Order gates?
		for (j = 0; j < itemcount; j++)
		{
			P_ITEM pj = &items[j];
			if (pj->type == 3)
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
		return;// order gates
	case 4: // Chaos gates?
		for (j = 0; j < itemcount; j++)
		{
			P_ITEM pj = &items[j];
			if (pj->type == 5)
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
		if (pi->moreb1)
			Magic->MagicTrap(DEREF_P_CHAR(pc_currchar), pi);
		sysmessage(s, "This item is locked.");
		return;// case 8/64 (locked container)
	case 9: // spellbook
		pc_currchar->objectdelay = 0;
		k = packitem(currchar[s]);
		if (k != -1)
			if ((pi->contserial == items[k].serial) || pc_currchar->Wears(pi) &&(pi->layer == 1))
			{
				Magic->SpellBook(s, DEREF_P_ITEM(pi));
			}
			else
			{
				sysmessage(s, "If you wish to open a spellbook, it must be equipped or in your main backpack.");
			}
			return;// spellbook
	case 10: // map?
		map1[1] = pi->ser1;
		map1[2] = pi->ser2;
		map1[3] = pi->ser3;
		map1[4] = pi->ser4;
		map2[1] = pi->ser1;
		map2[2] = pi->ser2;
		map2[3] = pi->ser3;
		map2[4] = pi->ser4;
/*
		By Polygon:
		Assign areas and map size before sending
*/
		map1[7] = pi->more1;	// Assign topleft x
		map1[8] = pi->more2;
		map1[9] = pi->more3;	// Assign topleft y
		map1[10] = pi->more4;
		map1[11] = pi->moreb1;	// Assign lowright x
		map1[12] = pi->moreb2;
		map1[13] = pi->moreb3;	// Assign lowright y
		map1[14] = pi->moreb4;
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
		pc_currchar->objectdelay = 0;
		if (pi->morex != 666 && pi->morex != 999)
			Books->openbook_old(s, pi); 
		else if (pi->morex == 666)
			Books->openbook_new(s, pi, 1);
		else 
			Books->openbook_new(s, pi, 0);
		return;// book
		
	case 12: // door(unlocked)
		pc_currchar->objectdelay = 0;
		dooruse(s, DEREF_P_ITEM(pi));
		return; // doors
	case 13: // locked door
		{
			P_ITEM pPack = Packitem(pc_currchar);
			if (pPack != NULL) // LB
			{
				vector<SERIAL> vecContainer = contsp.getData(pPack->serial);
				unsigned int j;
				for (j = 0; j < vecContainer.size(); j++) // Morrolan come back here and change this to search only backpack items 
				{
					P_ITEM pj = FindItemBySerial(vecContainer[j]);
					if (pj != NULL && pj->type == 7)
						if (((pj->more1 == pi->more1) &&(pj->more2 == pi->more2)&&
							(pj->more3 == pi->more3) &&(pj->more4 == pi->more4)))
						{
							sysmessage(s, "You quickly unlock, use, and then relock the door.");
							pc_currchar->objectdelay = 0;
							dooruse(s, DEREF_P_ITEM(pi));
							return;
						}// if
				}// for
			} // end if p!=-1
			sysmessage(s, "This door is locked.");
			return;// case 13 (locked door)
		}
	case 14: // For eating food
		pc_currchar->objectdelay = 0;
		if (pi->magic == 4)
			return; // Ripper..cant eat locked down food :)
		
		if (pc_currchar->hunger >= 6)
		{
			sysmessage(s, "You are simply too full to eat any more!");
			return;
		}
		else
		{
			switch (RandomNum(0, 2))
			{
			case 0: soundeffect2(currchar[s], 0x00, 0x3A);		break;
			case 1: soundeffect2(currchar[s], 0x00, 0x3B);		break;
			case 2: soundeffect2(currchar[s], 0x00, 0x3C);		break;
			}// switch(foodsnd)
			
			switch (pc_currchar->hunger)
			{
			case 0:  sysmessage(s, "You eat the food, but are still extremely hungry.");	break;
			case 1:  sysmessage(s, "You eat the food, but are still extremely hungry.");	break;
			case 2:  sysmessage(s, "After eating the food, you feel much less hungry.");	break;
			case 3:  sysmessage(s, "You eat the food, and begin to feel more satiated.");	break;
			case 4:  sysmessage(s, "You feel quite full after consuming the food.");		break;
			case 5:  sysmessage(s, "You are nearly stuffed, but manage to eat the food.");	break;
			case 6:  sysmessage(s, "You are simply too full to eat any more!");				break;
			default: sysmessage(s, "You are simply too full to eat any more!");				break;
			}// switch(pc_currchar->hunger)
			
			if ((pi->poisoned) &&(pc_currchar->poisoned < pi->poisoned)) 
			{
				sysmessage(s, "You have been poisoned!");
				soundeffect2(currchar[s], 0x02, 0x46); // poison sound - SpaceDog
				pc_currchar->poisoned = pi->poisoned;
				pc_currchar->poisontime = uiCurrentTime +(MY_CLOCKS_PER_SEC*(40/pc_currchar->poisoned)); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
				pc_currchar->poisonwearofftime = pc_currchar->poisontime +(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // wear off starts after poison takes effect - AntiChrist
				impowncreate(s, currchar[s], 1); // Lb, sends the green bar ! 
			}
			
			pi->ReduceAmount(1);	// Remove a food item
			pc_currchar->hunger++;
		}// else
		return; // case 14 (food)
	case 15: // -Fraz- Modified and tuned up, Wands must now be equipped or in pack
		k = packitem(currchar[s]);
		if (k != -1)
		{
			if ((pi->contserial == items[k].serial) || pc_currchar->Wears(pi) &&(pi->layer == 1))
			{
				if (pi->morez != 0)
				{
					pi->morez--;
					currentSpellType[s] = 2;
					if (Magic->newSelectSpell2Cast(s, (8*(pi->morex - 1)) + pi->morey))
					{ 
						if (pi->morez == 0)
						{
							pi->type = pi->type2;
							pi->morex = 0;
							pi->morey = 0;
							pi->offspell = 0;
						}
					}
				}
			}
			else
			{
				sysmessage(s, "If you wish to use this, it must be equipped or in your backpack.");
			}
		}
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
				sysmessage(s, "That rune is not yet marked!");
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
				sysmessage(s,  "That is out of charges.");
				return;
			}
			pi->morex--;
			sprintf((char*)temp, "Your wand now has %i charges left", pi->morex);
			sysmessage(s, (char*) temp);
			
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
			pc_currchar->smoketimer = pi->morex*MY_CLOCKS_PER_SEC + getNormalizedTime();
			Items->DeleItem(pi);
			return;
		case 186: // rename deed! -- eagle 1/29/00
			pc_currchar->inputitem = pi->serial;
			sysmessage(s, "Enter your new name.");
			Items->DeleItem(pi);
			return;// rename deed! -- eagle 1/29/00
			
		case 100:  // type 100?  this ain't in the docs... - Morrolan
			for (j = 0; j < itemcount; j++)
			{
				P_ITEM pj = &items[j];
				if (((pj->moreb1 == pi->morex) &&(pj->moreb2 == pi->morey) &&(pj->moreb3 == pi->morez))
					||((pj->morex == pi->morex) &&(pj->morey == pi->morey) &&(pj->morez == pi->morez))
					&&((j != DEREF_P_ITEM(pi)) &&(pi->morex != 0) &&(pi->morey != 0) &&(pi->morez != 0)))
				{ 
					if ((pj->morex == 0) &&(pj->morey == 0) &&(pj->morez == 0))
					{ 
						pj->morex = pj->moreb1;
						pj->morey = pj->moreb2;
						pj->morez = pj->moreb3;
						pj->visible = 0;								
						RefreshItem(pj);// AntiChrist
					} 
					else 
					{
						pj->moreb1 = pj->morex;
						pj->moreb2 = pj->morey;
						pj->moreb3 = pj->morez;
						pj->morex = 0;
						pj->morey = 0;
						pj->morez = 0;
						pj->visible = 2;							
						RefreshItem(pj);// AntiChrist
					}
				}
			}
			return; // case 100
		case 101: //??
			i = pi->morex;
			pc_currchar->id1 = i >> 8; 
			pc_currchar->id2 = i%256; 
			teleport(currchar[s]);
			pi->type = 102;
			return; // case 101
		case 102: //??
			pc_currchar->id1 = pc_currchar->xid1; 
			pc_currchar->id2 = pc_currchar->xid2; 
			teleport(currchar[s]);
			pi->type = 101;
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
			case 0: soundeffect2(currchar[s], 0x00, 0x31);		break;
			case 1: soundeffect2(currchar[s], 0x00, 0x30);		break; 
			}// switch(drinksnd)
			
			pi->ReduceAmount(1);	// Remove a drink
			sysmessage(s, "Gulp !");
			return; //	case 105 (drinks)
			case 202:
				if ( pi->id() == 0x14F0  ||  pi->id() == 0x1869 )	// Check for Deed/Teleporter + Guild Type
				{
					pc_currchar->fx1 = pi->serial;
					Guilds->StonePlacement(s);
					return;
				}
				else if (pi->id() == 0x0ED5)	// Check for Guildstone + Guild Type
				{
					pc_currchar->fx1 = pi->serial;
					Guilds->Menu(s, 1);
					return;
				}
				else 
					clConsole.send("Unhandled guild item type named: %s with ID of: %X", pi->name, pi->id());
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
				addid1[s] = pi->ser1;
				addid2[s] = pi->ser2;
				addid3[s] = pi->ser3;
				addid4[s] = pi->ser4;
				Gumps->Menu(s, pi->morex, DEREF_P_ITEM(pi));						
				return;
		case 217:			// PlayerVendors deed
			{	
				if (pi->magic == 4)
				{
					sysmessage(s, "That item is locked down.");
					return; 
				} // added by ripper, bugfixed by LB
				int m;
				P_ITEM pi_multi = findmulti(pc_currchar->pos); // boats are also multis zippy, btw !!!		
				if (pi_multi != NULL && iteminrange(s, pi_multi, 18))
				{	
					if (!ishouse(pi_multi))
						return; // LB
					const P_ITEM pi_p = Packitem(pc_currchar);
					if (pi_p != NULL)
					{
						los = 0;
						vector<SERIAL> vecContainer = contsp.getData(pi_p->serial);
						for (j = 0; j < vecContainer.size(); j++)
						{
							const P_ITEM pi_i = FindItemBySerial(vecContainer[j]);
							if ((pi_i != NULL) && (pi_p != NULL)) // lb
								if (pi_i->type == 7 && calcserial(pi_i->more1, pi_i->more2, pi_i->more3, pi_i->more4) == pi_multi->serial)
								{
									los = 1;
									break;
								}
						}
					}
				
					if (los)
					{
						m = Npcs->AddNPCxyz(-1, 2117, 0, pc_currchar->pos.x, pc_currchar->pos.y, pc_currchar->pos.z);
						
						P_CHAR pc_vendor = MAKE_CHAR_REF(m);

						if (pc_vendor == NULL) 
						{
							clConsole.send("npc-script couldnt find vendor !\n");
							return;
						}
						
						pc_vendor->npcaitype = 17;
						pc_vendor->makeInvulnerable();
						pc_vendor->hidden = 0;
						pc_vendor->stealth=-1;
						pc_vendor->dir = pc_currchar->dir;
						pc_vendor->npcWander = 0;
						pc_vendor->setInnocent();
						pc_vendor->SetOwnSerial(pc_currchar->serial);
						pc_vendor->tamed = false;
						Items->DeleItem(pi);
						sprintf((char*)temp, "Hello sir! My name is %s and i will be working for you.", pc_vendor->name);
						npctalk(s, DEREF_P_CHAR(pc_vendor), (char*)temp, 1);
						updatechar(DEREF_P_CHAR(pc_vendor));
						teleport(DEREF_P_CHAR(pc_vendor));
					}
					else 
						sysmessage(s, "You must be close to a house and have a key in your pack to place that.");
				}
				else if (pi_multi == NULL)
					sysmessage(s, "You must be close to a house and have a key in your pack to place that.");
				
				return;
			}
		// By Polygon: Clicked on a tattered treasure map, call decipher-function
		case 301: Skills->Decipher(pi, s); return;
/*
	By Polygon:
	Clicked on a deciphered treasure map
	Show a map-gump with the treasure location in it
*/
		case 302:	// Deciphered treasure map?
			map1[1] = pi->ser1;
			map1[2] = pi->ser2;
			map1[3] = pi->ser3;
			map1[4] = pi->ser4;
			map2[1] = pi->ser1;
			map2[2] = pi->ser2;
			map2[3] = pi->ser3;
			map2[4] = pi->ser4;
			map1[7] = pi->more1;	// Assign topleft x
			map1[8] = pi->more2;
			map1[9] = pi->more3;	// Assign topleft y
			map1[10] = pi->more4;
			map1[11] = pi->moreb1;	// Assign lowright x
			map1[12] = pi->moreb2;
			map1[13] = pi->moreb3;	// Assign lowright y
			map1[14] = pi->moreb4;
			map1[15] = 0x01;			// Let width and height be 256
			map1[16] = 0x00;
			map1[17] = 0x01;
			map1[18] = 0x00;
			Xsend(s, map1, 19);

			Xsend(s, map2, 11);

			// Generate message to add a map point
			map3[1] = pi->ser1;
			map3[2] = pi->ser2;
			map3[3] = pi->ser3;
			map3[4] = pi->ser4;
			int posx, posy;			// tempoary storage for map point
			int tlx, tly, lrx, lry;	// tempoary storage for map extends
			tlx = (pi->more1 << 8) + pi->more2;
			tly = (pi->more3 << 8) + pi->more4;
			lrx = (pi->moreb1 << 8) + pi->moreb2;
			lry = (pi->moreb3 << 8) + pi->moreb4;
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
	             sysmessage(s, "Crystall ball shatters.."); 
	             pi->ReduceAmount(1); 
			}
            return;// End jailball
	    case 402: // Blackwinds Reputation ball 
			{ 
                 soundeffect(currchar[s], 1, 0xec); // Play sound effect for player 
                 sysmessage(s,"Your karma is %i",pc_currchar->karma); 
                 sysmessage(s,"Your fame is %i",pc_currchar->fame); 
                 sysmessage(s,"Your Kill count is %i ",pc_currchar->kills); 
                 sysmessage(s,"You died %i times.",pc_currchar->deaths);
				 staticeffect(s, 0x37, 0x2A, 0x09, 0x06 );
				 sysmessage(s,"*The crystal ball seems to have vanished*");
                 pi->ReduceAmount(1); 
                 return; 
			}
		case 404: // Fraz'z ID wand
			{
				P_ITEM pBackpack = Packitem(pc_currchar);
				if ( pBackpack != NULL )
				{
					if ((pi->contserial == pBackpack->serial) || pc_currchar->Wears(pi) &&(pi->layer == 1))
					{
						if (pi->morex <= 0)
						{
							sysmessage(s,  "That is out of charges.");
							return;
						}
						pi->morex--;
						sprintf((char*)temp, "Your wand now has %i charges left", pi->morex);
						sysmessage(s, (char*) temp);
						target(s, 0, 1, 0, 75, "What do you wish to identify?");
					}
					else
					{
						sysmessage(s, "If you wish to use this, it must be equipped or in your backpack.");
					}
				}
				return;
			}
		case 1000: // Ripper...bank checks
			{
				sysmessage(s, "To cash this, you need to drop it on a banker.");
				return;
			}
		default:						
			break; // case (itype!) 
		}// switch (itype)
		// END Check items by type
		
		// Begin checking objects by ID
		if (pi->magic != 4) // Ripper
		{
			switch (pi->id())
			{	
				case 0x0FA9:// dye
					dyeall[s] = 0;
					target(s, 0, 1, 0, 31, "Which dye vat will you use this on?");
					return;// dye
				case 0x0FAF:
				case 0x0FB0: // Anvils
					if (!iteminrange(s, pi, 3))
					{
						sysmessage(s, "Must be closer to use this!");
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
					if (!iteminrange(s, pi, 3))
					{
						sysmessage(s, "Must be closer to use this!");
						return;
					}
					target(s, 0, 1, 0, 237, "What item would you like to Smelt?");
					return; // Ripper..Smelting items.
				case 0x0FAB:// dye vat
					addid1[s] = pi->color1;
					addid2[s] = pi->color2;
					target(s, 0, 1, 0, 32, "Select the clothing to use this on.");
					return;// dye vat
				case 0x14F0:// houses
					if ((pi->type != 103) &&(pi->type != 202))
					{  // experimental house code
						pc_currchar->making = DEREF_P_ITEM(pi);
						pc_currchar->fx1 = pi->serial; // for deleting it later
						addid3[s] = pi->morex;
						// addx2[s]=pi->serial;
						HouseManager->AddHome(s, pi->morex);
						// target(s,0,1,0,207,"Select Location for house.");
					}
					return;// house deeds
				case 0x100A:
				case 0x100B:// archery butte
					Skills->AButte(s, pi);
					return;// archery butte
				case 0x0E9C:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x00, 0x38);
					else 
						soundeffect2(currchar[s], 0x00, 0x39);
					return;
				case 0x0E9D:
				case 0x0E9E:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x00, 0x52);
					else 
						soundeffect2(currchar[s], 0x00, 0x53);
					return;
				case 0x0EB1:
				case 0x0EB2:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x00, 0x45);
					else 
						soundeffect2(currchar[s], 0x00, 0x46);
					return;
				case 0x0EB3:
				case 0x0EB4:
					if (Skills->CheckSkill(currchar[s], MUSICIANSHIP, 0, 1000))
						soundeffect2(currchar[s], 0x00, 0x4C);
					else 
						soundeffect2(currchar[s], 0x00, 0x4D);
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
					addx[s] = DEREF_P_ITEM(pi); // save the item number, AntiChrist
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
						addx[s] = DEREF_P_ITEM(pi); // save the item number, AntiChrist
						target(s, 0, 1, 0, 51, "Where do you want to dig?");
					}
					return; // mining
				case 0x0E24: // empty vial
					{
					P_ITEM pBackpack = Packitem(pc_currchar);
					if (pBackpack != NULL)
						if (pi->contserial == pBackpack->serial)
						{
							addx[s] = DEREF_P_ITEM(pi); // save the vials number, LB
							target(s, 0, 1, 0, 186, "What do you want to fill the vial with?");
						}
						else 
							sysmessage(s, "The vial is not in your pack");
						return;
					}
				case 0x0DF9: 
					pc_currchar->tailitem = pi->serial;   
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
					pc_currchar->tailitem = pi->serial;
					target(s, 0, 1, 0, 165, "Select loom to make your cloth");
					return;
				case 0x14ED: // Build cannon
					target(s, 0, 1, 0, 171, "Build this Monster!");
					Items->DeleItem(pi);
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
					pc_currchar->tailitem = pi->serial;
					target(s, 0, 1, 0, 173, "Select flour to pour this on.");  
					return;
				case 0x09C0:
				case 0x09C1: // sausages to dough
					pc_currchar->tailitem = pi->serial;
					target(s, 0, 1, 0, 174, "Select dough to put this on.");  
					return;
				case 0x0DF8: // wool to yarn 
					pc_currchar->tailitem = pi->serial;   
					target(s, 0, 1, 0, 164, "Select your spin wheel to spin wool.");      
					return;
				case 0x0F9D: // sewing kit for tailoring
					target(s, 0, 1, 0, 167, "Select material to use.");
					return;
				case 0x19B7:
				case 0x19B9:
				case 0x19BA:
				case 0x19B8: // smelt ore
					pc_currchar->smeltitem = pi->serial;
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
							pFire->type = 45;
							pFire->dir = 2;
							if (pi->isInWorld())
								pFire->MoveTo(pi->pos.x,pi->pos.y,pi->pos.z);
							else
								pFire->MoveTo(pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);

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
						pi->type = 45;					
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						sysmessage(s, "You failed to use this statue.");
					}
					return;
				case 0x1509:
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1508);
						pi->type = 45;						
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						sysmessage(s, "You failed to use this statue.");
					}
					return;
				case 0x1230:
				case 0x1246: // guillotines?
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1245);
						pi->type = 45;					
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						sysmessage(s, "You failed to use this.");
					}  
					return;
				case 0x1245: // Guillotine stop animation
					if (Skills->CheckSkill(currchar[s], ITEMID, 0, 10))
					{
						pi->setId(0x1230);
						pi->type = 45;						
						RefreshItem(pi);// AntiChrist
					}
					else
					{
						sysmessage(s, "You failed to use this.");
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
					if (pi->type == 17)
					{
						addid1[s] = pi->ser1;
						addid2[s] = pi->ser2;
						addid3[s] = pi->ser3;
						addid4[s] = pi->ser4;
						target(s, 0, 1, 0, 109, "Where is an empty bottle for your potion?");
					}
					else
					{
						addid1[s] = pi->ser1;
						addid2[s] = pi->ser2;
						addid3[s] = pi->ser3;
						addid4[s] = pi->ser4;
						target(s, 0, 1, 0, 108, "What do you wish to grind with your mortar and pestle?");
					}
					return; // alchemy
				case 0x0F9E:
				case 0x0F9F: // scissors
					target(s, 0, 1, 0, 128, "What cloth should I use these scissors on?");
					return;
				case 0x0E21: // healing
					addx[s] = DEREF_P_ITEM(pi);
					target(s, 0, 1, 0, 130, "Who will you use the bandages on?");
					return;
				case 0x1057:
				case 0x1058: // sextants
					
					// dont know how if thats an accourate way to find out t2a-ness
					t2a = (pc_currchar->pos.x >= 5121);					    
					
					getSextantCords(pc_currchar->pos.x, pc_currchar->pos.y, t2a, temp2);
					sprintf((char*)temp, "You are at: %s", temp2);
					sysmessage(s, (char*)temp);
					return;
				case 0x0E27:
				case 0x0EFF:   // Hair Dye
					usehairdye(s, DEREF_P_ITEM(pi));
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
						if (!pc_currchar->onhorse)
							action(s, 0x0D);
						else 
							action(s, 0x1d);
						soundeffect(s, 0x01, 0x3E);
						P_ITEM p_cotton = Items->SpawnItem(-1, DEREF_P_CHAR(pc_currchar), 1, "#", 1, 0x0D, 0xF9, 0, 0, 1, 1);
						if ( p_cotton == NULL )
							return;
						p_cotton->SetContSerial(Packitem(pc_currchar)->serial);
						sysmessage(s, "You reach down and pick some cotton.");
					}
					return; // cotton
				case 0x105B:
				case 0x105C:
				case 0x1053:
				case 0x1054: // tinker axle
					addid1[s] = pi->ser1;
					addid2[s] = pi->ser2;
					addid3[s] = pi->ser3;
					addid4[s] = pi->ser4;
					target(s, 0, 1, 0, 183, "Select part to combine that with.");
					return;
				case 0x1051:
				case 0x1052:
				case 0x1055:
				case 0x1056:
				case 0x105D:
				case 0x105E:
					addid1[s] = pi->ser1;
					addid2[s] = pi->ser2;
					addid3[s] = pi->ser3;
					addid4[s] = pi->ser4;
					// itemmake[s].materialid1=pi->id1;
					// itemmake[s].materialid2=pi->id2;
					target(s, 0, 1, 0, 184, "Select part to combine it with.");
					return;
				case 0x104F:
				case 0x1050:
				case 0x104D:
				case 0x104E:// tinker clock
					addid1[s] = pi->ser1;
					addid2[s] = pi->ser2;
					addid3[s] = pi->ser3;
					addid4[s] = pi->ser4;
					target(s, 0, 1, 0, 185, "Select part to combine with");
					return;
				case 0x1059:
				case 0x105A:// tinker sextant
					if (Skills->CheckSkill(currchar[s], TINKERING, 500, 1000))
					{
						sysmessage(s, "You create the sextant.");
						P_ITEM pi_sextant = Items->SpawnItem(s, currchar[s], 1, "a sextant", 0, 0x10, 0x57, 0, 0, 1, 1);
						if (pi_sextant != NULL)
							pi_sextant->priv |= 0x01;
						pi->ReduceAmount(1);
					}
					else 
						sysmessage(s, "you fail to create the sextant.");
					return;
				case 0x1070:
				case 0x1074: // training dummies
					if (iteminrange(s, pi, 1))
						Skills->TDummy(s);
					else 
						sysmessage(s, "You need to be closer to use that.");
					return;
				case 0x1071:
				case 0x1073:
				case 0x1075:
				case 0x1077:// swinging training dummy
					sysmessage(s, "You must wait for it to stop swinging !");
					return;
				case 0x1EA8: 
					slotmachine(s, pi);
					return; // Ripper
				case 0x1EBC: // tinker's tools
					target(s, 0, 1, 0, 180, "Select material to use.");
					return;
				default:
					//	clConsole.send("Unhandled item id for item: %s with id: %X.",pi->name, itemids); //check for unused items - Morrolan
					break;
			}// switch (itemids)
		}
		// END Check items by ID
		sysmessage(s, "You can't think of a way to use that item.");
}

void singleclick(UOXSOCKET s)
{
	int j,  serial;
	int amt = 0, wgt;
	char itemname[100];
	unsigned char temp2[100];
	
	temp[0] = temp2[0] = itemname[0] = 0;
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	serial = calcserial(buffer[s][1], buffer[s][2], buffer[s][3], buffer[s][4]);
	// Begin chars/npcs section
	
	if (isCharSerial(serial))
	{
		CHARACTER c = calcCharFromSer(serial);
		if (c!=-1)		
		{
			showcname(s, c, 0);
			return;
		}
	}
	// End chars/npcs section
	
	P_ITEM pi = FindItemBySerial(serial);
	if (pi == NULL)
	{
		clConsole.send("WOLFPACK.CPP: singleclick couldn't find item serial: %d\n", serial);
		return;
	}
	
	pi->getName(itemname);

	if (pi->type == 9)
	{
		int spellcount=Magic->SpellsInBook(DEREF_P_ITEM(pi));
		sprintf((char*)temp, "%i spells", spellcount);
		itemmessage(s, (char*)temp, serial,0x0481);
	}

	if (pi->type == 1000) // Ripper...used for bank checks.
	{
		int goldcount = pi->value;
		sprintf((char*)temp, "%i gold", goldcount);
		itemmessage(s, (char*)temp, serial,0x0481);
	}
	
	if (pc_currchar->getPriv()&8)
	{
		if (pi->amount > 1)
			sprintf((char*)temp, "%s [%x]: %i", itemname, pi->serial, pi->amount);
		else
			sprintf((char*)temp, "%s [%x]", itemname, pi->serial);
		itemmessage(s, (char*)temp, serial);
		return;
	}
	
	// Click in a Player Vendor item, show description, price and return
	if (!pi->isInWorld() && isItemSerial(pi->contserial))
	{
		P_CHAR pc_j = GetPackOwner(FindItemBySerial(pi->contserial));
		if (pc_j != NULL)
		{
			if (pc_j->npcaitype == 17)
			{
				if (strlen(pi->creator)>0 && pi->madewith>0)
					sprintf((char*)temp2, "%s %s by %s", pi->desc, skill[pi->madewith - 1].madeword, pi->creator); 
				else
					strcpy((char*)temp2, pi->desc); // LB bugfix
				
				sprintf((char*)temp, "%s at %igp", temp2, pi->value); // Changed by Magius(CHE)				
				itemmessage(s, (char*)temp, serial);
				return;
			}
		}
	}
	
	// From now on, we will build the message into temp, and let itemname with just the name info
	// Add amount info.
	if (!pi->pileable || pi->amount == 1)
		strncpy((char*)temp, itemname, 100);
	else 
		if (itemname[strlen(itemname) - 1] != 's') // avoid iron ingotss : x
			sprintf((char*)temp, "%ss : %i", itemname, pi->amount);
		else
			sprintf((char*)temp, "%s : %i", itemname, pi->amount);
		
	// Add creator's mark (if any)			
	if (strlen(pi->creator) > 0 && pi->madewith > 0)
		sprintf((char*)temp, "%s %s by %s", temp, skill[pi->madewith - 1].madeword, pi->creator);
	
	if (pi->type == 15) // Fraz
	{
			if (!(strcmp(pi->name2,pi->name)))
			{
			sprintf((char*)temp, "%s %i charge", temp, pi->morez);
			if (pi->morez != 1)
			strcat(temp, "s");
			}
	}
	else if (pi->type == 404 || pi->type == 181)
	{
			if (!(strcmp(pi->name2,pi->name)))
			{
			sprintf((char*)temp, "%s %i charge", temp, pi->morex);
			if (pi->morex != 1)
			strcat(temp, "s");
			}
	}	
	// Corpse highlighting...Ripper
	if (pi->corpse==1)
	{
		if(pi->more2==1)
		    itemmessage(s,"[Innocent]",serial, 0x005A);
		else if(pi->more2==2)
			itemmessage(s,"[Criminal]",serial, 0x03B2);
		else if(pi->more2==3)
			itemmessage(s,"[Murderer]",serial, 0x0026);
	}  // end highlighting
	// Let's handle secure/locked down stuff.
	if (pi->magic == 4 && pi->type != 12 && pi->type != 203)
	{
		if (pi->secureIt !=1)
			itemmessage(s, "[locked down]", serial, 0x0481);
		if (pi->secureIt == 1 && pi->magic == 4)
			itemmessage(s, "[locked down & secure]", serial, 0x0481);				
	}
	
	itemmessage(s, (char*)temp, serial);
	
	// Send the item/weight as the last line in case of containers
	if (pi->type == 1 || pi->type == 63 || pi->type == 65 || pi->type == 87)
	{
		wgt = (int) Weight->LockeddownWeight(pi, &amt, 0); // get stones and item #, LB	
		if (amt>0)
		{
			sprintf((char*)temp2, "[%i items, %i stones]", amt, wgt);
			itemmessage(s, (char*)temp2, serial);
		}
		else
			itemmessage(s, "[0 items, 0 stones]", serial);
	}
}


//Handles Double clicks over PC/NPCs

void dbl_click_character(UOXSOCKET s, SERIAL target_serial)
{
	int keyboard, y;
	unsigned char pdoll[256]="\x88\x00\x05\xA8\x90\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//	int cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	keyboard=buffer[s][1]&0x80;
	
//	CHARACTER x = calcCharFromSer( target_serial );

	P_CHAR target = FindCharBySerial( target_serial );


	if (target == NULL)
		return;


	if (((target->isNpc())&&(target->id1==(unsigned char)'\x00'))&&
		((target->id2==(unsigned char)'\xC8')||
		(target->id2==(unsigned char)'\xE2') ||
		(target->id2==(unsigned char)'\xE4') ||
		(target->id2==(unsigned char)'\xCC') ||
		(target->id2==(unsigned char)'\xDC') ||
		(target->id2==(unsigned char)'\xD2') ||
		(target->id2==(unsigned char)'\xDA') ||
		(target->id2==(unsigned char)'\xDB')))
	{//if mount
		if (chardist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(target))<2 || pc_currchar->isGM())
		{
			//AntiChrist - cannot ride animals under polymorph effect
			if (pc_currchar->polymorph)
			{
				sysmessage(s, "You cannot ride anything under polymorph effect.");
				return;
			}
			if (pc_currchar->dead)
			{
				sysmessage(s,"You are dead and cannot do that.");
				return;
			}
			if (target->war)
				sysmessage(s,"Your pet is in battle right now!");
			else
				mounthorse(s, DEREF_P_CHAR(target));
		}
		else sysmessage(s, "You need to get closer.");
		return; 
	}//if mount
	else if ((target->isNpc())&&((target->id1!=0x01)||(target->id2<0x90)||(target->id2>0x93)))
	{//if monster
		if (target->id1==0x01&&(target->id2==0x23||target->id2==0x24))
		{//if packhorse or packlhama added by JustMichael 8/31/99
			if (pc_currchar->Owns(target))
			{
				y=packitem(DEREF_P_CHAR(target));
				if (y!=-1)
				{
					backpack(s,items[y].serial);
				}
				else
				{
					clConsole.send("Pack animal %i has no backpack!\n",target->serial);
				}
			}
			else
			{
				sysmessage(s, "That is not your beast of burden!");
			}
			return;
		}
		else
		{
			sysmessage(s, "You cannot open monsters paperdolls.");
		}
		return; 
	}//if monster
	else 
	{//char
		if (target->npcaitype==17)//PlayerVendors
		{
			npctalk(s,DEREF_P_CHAR(target),"Take a look at my goods.",0);
			y=packitem(DEREF_P_CHAR(target));
			if (y!=-1) backpack(s, items[y].serial); // rippers bugfix for vendor bags not opening !!!
			return;
		}
		if (pc_currchar->serial==target->serial)
		{//dbl-click self
			if ((!keyboard)&&(unmounthorse(s)==0)) return; //on horse
			//if not on horse, treat ourselves as any other char
		}//self
		pdoll[1]=target->ser1;
		pdoll[2]=target->ser2;
		pdoll[3]=target->ser3;
		pdoll[4]=target->ser4;
		
		completetitle = complete_title(DEREF_P_CHAR(target));
		int l = strlen(completetitle);
		if (l>=60) completetitle[60]=0;
		strcpy((char*)&pdoll[5], completetitle);				
		Xsend(s, pdoll, 66);
		return;
	}//char
}

