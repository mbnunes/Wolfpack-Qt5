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

// dragdrop.cpp: implementation of dragging and dropping
// cut from wolfpack.cpp by Duke, 23.9.2000
//////////////////////////////////////////////////////////////////////

#include "debug.h"
#include "basics.h"
#include "wolfpack.h"
#include "SndPkg.h"
#include "speech.h"
#include "itemid.h"
#include "bounty.h"
#include "trigger.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"

#undef  DBGFILE
#define DBGFILE "dragdrop.cpp"

//#include "dragdrop.h"

typedef struct _PKGx08
{
//0x08 Packet
//Drop Item(s) (14 bytes)
//* BYTE cmd
//* BYTE[4] item id
	long Iserial;
//* BYTE[2] xLoc
	short TxLoc;
//* BYTE[2] yLoc
	short TyLoc;
//* BYTE zLoc
	signed char TzLoc;
//* BYTE[4] Move Into (FF FF FF FF if normal world)
	long Tserial;
} PKGx08;

// moved here from cWeight by Duke, 12.5.2001 (not sure what this is good for ...:((
// if the item is equipped or in primary backpack return true
static bool CheckWhereItem( P_ITEM pack, P_ITEM pi, int s)
{
	if (pi && pack && s!=-1) //LB
	{
		if (!( pi->contserial==pack->serial ||
			currchar[s]->Wears(pi)))
			
			return 1;
		else
			return 0;
	}
	return 0;
}

void UpdateStatusWindow(UOXSOCKET s, P_ITEM pi)
{
	P_ITEM packnum = Packitem(currchar[s]);
	if (CheckWhereItem(packnum, pi, s))
		statwindow(s, currchar[s]);
}

static void Sndbounce5(UOXSOCKET s)
{
	bounce[1]=5;
	Xsend(s, bounce, 2);
}
// Name:	item_bounce3
// Purpose:	holds some statements that were COPIED some 50 times
// Remarks:	temporary functions to revamp the 30 occurences of the 'bouncing bugfix'
// History:	init Duke, 10.8.2000
static void item_bounce3(const P_ITEM pi)
{
	pi->SetContSerial(pi->oldcontserial);
	pi->pos = pi->oldpos;
	pi->layer=pi->oldlayer;
	pi->flags.isBeeingDragged=false;
	P_CHAR pc = FindCharBySerial(pi->oldcontserial);

	if (pi->layer > 0 && pc != NULL)
	{
		pc->st += pi->st2;
		pc->chgDex(pi->dx2);
		pc->in += pi->in;
	}
}

static void item_bounce4(const UOXSOCKET s, const P_ITEM pi)
{
	item_bounce3(pi);
	if (pi->id1>=0x40)
		senditem(s, pi);
}

static void item_bounce5(const UOXSOCKET s, const P_ITEM pi)
{
	item_bounce3(pi);
	if(pi->isInWorld())
	{
		if (pi->id1>=0x40)
			senditem(s, pi);
	}
	else
		senditem(s, pi);
}

static void item_bounce6(const P_CLIENT ps, const P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	Sndbounce5(s);
	if (ps->IsDragging())
	{
		ps->ResetDragging();
		item_bounce4(s,pi);
	}
}


// this function SCREAMES for a rewrite !! LB !!

void cDragdrop::get_item(P_CLIENT ps) // Client grabs an item
{
	int amount, update = 0, serial;
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	
	P_CHAR npc = NULL;
	
	serial = calcserial(buffer[s][1], buffer[s][2], buffer[s][3], buffer[s][4]);
	if (serial == INVALID_SERIAL || buffer[s][1] < 0x40)
		return;	// landscape or a character
	P_ITEM pi = FindItemBySerial(serial);
	if (pi == NULL)
		return;
	
	pc_currchar->disturbMed(s); // Meditation
	
	// Zippy's stealing changes
	P_ITEM px = pi;
	if (!px->isInWorld())  // Find character owning item
	{
		unsigned long loopexit = 0;
		do  // Find character owning item
		{
			if (isCharSerial(px->contserial))
			{
				npc = FindCharBySerial(px->contserial);
			}
			else  // its an item
			{
				if (px->isInWorld())
				{
					npc = NULL;
					break;
				}
				px = FindItemBySerial(px->contserial);
				// ANTICHRIST -- SECURE TRADE FIX
				if (px != NULL) // LB overwriting x is essential here, dont change it!!!
				{
					if (px->layer == 0 && px->id() == 0x1E5E)
					{
						// Trade window???
						serial = calcserial(px->moreb1, px->moreb2, px->moreb3, px->moreb4);
						if (serial == INVALID_SERIAL)
							return;
						P_ITEM pi_z = FindItemBySerial(serial);
						if ( pi_z != NULL )
							if ((pi_z->morez || px->morez))
							{
								pi_z->morez = 0;
								px->morez = 0;
								sendtradestatus(pi_z, px);
							}
					}
					// Blackwinds Looting is crime implementation
					// changed slightly by Ripper
					if (px->corpse != 0 && !pc_currchar->Owns(px))
					{
						P_CHAR co = FindCharBySerial(px->ownserial);
						if (px->more2 == 1 && GuildCompare( pc_currchar, co ) == 0)
						{
							pc_currchar->karma -= 5;
							criminal(pc_currchar);
							sysmessage(s, "You lost some karma!");
						}
						npc = 0;
					} // Criminal stuff
					if (px->corpse != 0)
						npc = 0;
				} // end if x!=-1
				
				if (px == NULL)
					npc = 0;
			}
		} while ((npc == NULL) &&(++loopexit < MAXLOOPS));
	}
	
	if (npc != NULL) // 0=corpse, hence >0 ..
	{
		if (!(pc_currchar->isGM()) && npc != pc_currchar && ! pc_currchar->Owns(npc))
		{// Own serial stuff by Zippy -^ Pack aniamls and vendors.
			bounce[1] = 0;
			Xsend(s, bounce, 2);
			if (ps->IsDragging())
			{
				ps->ResetDragging();
				item_bounce3(pi);
				pi->setOwnerMovable();
			}
			return;
		}
	}
	// End Zippy's change
	
	// Boats->
	if (px != NULL && npc != NULL)
	{
		if (px->multis>0)
			imultisp.remove(px->multis, px->serial);
		px->startDecay();
		// End Boats Change
		
		// AntiChrist -- for poisoned items
		if (px->layer>0)
		{
			npc->removeItemBonus(px);	// remove BONUS STATS given by equipped special items
		}
		if ((px->trigon==1) && (px->layer != 0) && (px->layer != 15) && (px->layer < 19))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
		{
			Trig->triggerwitem(s, pi, 1); // trigger is fired
		}	
	}
	if (pi != NULL)
	{
		if (pi->corpse != 1)
		{
			UpdateStatusWindow(s, pi);
			if (!pc_currchar->canPickUp(pi))
			{
				bounce[1] = 0;
				Xsend(s, bounce, 2);
				if (ps->IsDragging()) // only restore item if it got draggged before !!!
				{
					ps->ResetDragging();
					item_bounce4(s, pi);
				}
			}
			else
			{
				// AntiChrist bugfix for the bad bouncing bug ( disappearing items when bouncing )
				DRAGGED[s] = 1;
				
				pi->oldpos = pi->pos;	// first let's save the position
				pi->oldcontserial = pi->contserial;	// then let's save the container
				pi->oldlayer = pi->layer;	// then the layer
				
				pi->layer = 0;
				if (!pi->isInWorld())
					soundeffect(s, 0x00, 0x57);
				if (pi->amount>1)
				{
					amount = (buffer[s][5] << 8) + buffer[s][6];
					if (amount>pi->amount)
						amount = pi->amount;
					if (amount < pi->amount)
					{
						P_ITEM pi_c = new cItem(*pi);
						//	pi_c->Init(0);
#pragma note("Works for now, but need to change to prototype pattern later")
						//memcpy(pi_c, pi, sizeof(cItem));  // Tauriel reduce code faster too
						pi_c->SetSerial(cItemsManager::getInstance()->getUnusedSerial());

						pi_c->amount = pi->amount - amount;
						pi_c->SetContSerial(pi_c->contserial);
						pi_c->SetOwnSerial(pi_c->ownserial);
						pi_c->SetSpawnSerial(pi_c->spawnserial);
						
						statwindow(s, pc_currchar);
						RefreshItem(pi_c);//AntiChrist
					}
					
					if (pi->id() == 0x0EED) // gold coin
					{
						P_ITEM packnum = Packitem(currchar[s]);
						if (packnum != NULL) // lb
							if (pi->contserial == packnum->serial)
								update = 1;
					}
					
					pi->amount = amount;
					
				}
				
			/*	int amt = 0, wgt; bool tooheavy=false;				
				wgt = (int)Weight->LockeddownWeight(pi, &amt, 0);
				if(pi->contserial>0)
				{
					if (( (pc_currchar->weight+wgt) > (pc_currchar->st*WEIGHT_PER_STR)+30)) // LB -> added: drop item if too heavy
					{
					  float res=float( (pc_currchar->weight+wgt) - ((pc_currchar->st*WEIGHT_PER_STR)+30))*2;
					  int diff = pc_currchar->st;
					  diff -= (int)res;
					  if (diff<=0 && !pc_currchar->isGM() )					
					  {
						 tooheavy=true;						 						

						 bounce[1] = 0;
						 Xsend(s, bounce, 2);
						 if (ps->IsDragging()) // only restore item if it got dragged before !!!
						 {
						   ps->ResetDragging();
						   item_bounce4(s, pi);
						 }
						 sysmessage(s, "you can't pick this up, this is too heavy");					
						 return;
					 }
					}
				}

                if (!tooheavy) pc_currchar->weight+=wgt;				
				update = 1;	*/				

				// LB remark: drop item if too heavy is a good solution,
				// but there's still a small bug remaining.
				// added weight from items picked up, but not put to bp, pd,  in other words hold in ones hand,
				// is NOT subtracted when being dropped again to ground/other chars/other chars' bp's.
				// but this bug doesnt show up becasue weight is re-calculated automatically all 10 secs.
				// without adding weight of the item curently carrying in hand.
				// a correct solutions need the weight of item in hand being stored
				// , added to auto-re-calculation all x-secs code, and being subtracted if dropped.
				// because it's now only happening for leight weight items, because heavy weight itms cant be picke up anymore
				// I haven't corrected this yet. 			
				
				// Tauriel remove item from world mapcells
				mapRegions->Remove(pi); // remove this item from a map cell				
				pi->pos.x = 0;
				pi->pos.y = 0;
				pi->pos.z = 0;
				
				pi->flags.isBeeingDragged=true;
				pi->SetContSerial(-1);
			
			
			}
		}
	} // end of if i!=-1
	if (update)
		statwindow(s, pc_currchar);
}

void cDragdrop::wear_item(P_CLIENT ps) // Item is dropped on paperdoll
{
	int j;
	tile_st tile;
	int serial, letsbounce=0; // AntiChrist (5) - new ITEMHAND system
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();

	int cserial=calcserial(buffer[s][6],buffer[s][7],buffer[s][8],buffer[s][9]);
	if(cserial==INVALID_SERIAL) return;
	P_CHAR pc_k = FindCharBySerial( cserial );
	
	if( pc_k->dead )  return; //Exploit fix: Dead ppl can't equip anything.
	
	P_ITEM pi=FindItemBySerPtr(buffer[s]+1);
	if (pi == NULL) return;
	pi->flags.isBeeingDragged=false;

	Map->SeekTile(pi->id(), &tile);
		
	if (tile.layer==0)
	{
		sysmessage(s,"You can't wear that");
		Sndbounce5(s);
		if (ps->IsDragging())
		{
			ps->ResetDragging();
			item_bounce4(s,pi);
			UpdateStatusWindow(s,pi);
		}
		return;
	}

	if (pi->id1>=0x40) return; // LB, client crashfix if multi-objects are moved to PD

	if (pc_k == pc_currchar || pc_currchar->isGM())
	{
		if (pc_k != NULL) //lb
			if (pc_k == pc_currchar && pi->st>pc_k->st)
			{
				sysmessage(s,"You are not strong enough to use that.");
				Sndbounce5(s);
				if (ps->IsDragging())
				{
					ps->ResetDragging();
					item_bounce4(s,pi);
					UpdateStatusWindow(s,pi);
				}
				return;
			}
			
			if (pc_currchar->id1==0x01 && pc_currchar->id2==0x90) // Ripper...so males cant wear female armor
			if (pi->id1==0x1c && ( pi->id2==0x00 || pi->id2==0x02 || pi->id2==0x04 ||
                pi->id2==0x06 || pi->id2==0x08 || pi->id2==0x0a || pi->id2==0x0c))
			{
				sysmessage(s,"You cant wear female armor!");
				Sndbounce5(s);
				if (ps->IsDragging())
                {
					ps->ResetDragging();
					item_bounce4(s,pi);				
					UpdateStatusWindow(s,pi);
				}
				return;
			}

			if ((((pi->isGMMovable())||((tile.weight==255)&&(!pi->isAllMovable())))&&((pc_currchar->priv2&1)==0)) ||
				( (pi->isOwnerMovable()|| pi->isLockedDown()) && !pc_currchar->Owns(pi)))
			{
				item_bounce6(ps,pi);
				return;
			}
		

		// - AntiChrist (4) - checks for new ITEMHAND system
		// - now you can't equip 2 hnd weapons with 1hnd weapons nor shields!!
		serial=pc_currchar->serial;
		unsigned int ci;
		vector<SERIAL> vecContainer = contsp.getData(serial);
		for (ci = 0; ci < vecContainer.size(); ci++)
		{
			P_ITEM pi2 = FindItemBySerial(vecContainer[ci]);
			if (pi2 == NULL) return;
			if (pi2->contserial == serial)
			{
				if (pi2->layer == tile.layer)
				{
 					sysmessage(s, "You already have an armor equipped!");
					letsbounce = 1;
				}
				else if (pi2->itmhand==1 && pi->itmhand==1)
				{
					sysmessage(s,"You already have a weapon equipped!");
					letsbounce=1;
				}
				else if (pi2->itmhand==1 && pi->itmhand==2)
				{
					sysmessage(s,"You cannot equip a two handed weapon with a weapon equipped!");
					letsbounce=1;
				}
				else if (pi2->itmhand==2 && pi->itmhand==1)
				{
					sysmessage(s,"Your hands are both occupied!");
					letsbounce=1;
				}
				else if (pi2->itmhand==2 && pi->itmhand==2)
				{
					sysmessage(s,"You cannot equip a two handed weapon with a two handed weapon equipped!");
					letsbounce=1;
				}
				else if (pi2->itmhand==2 && pi->itmhand==3)
				{
					sysmessage(s,"You cannot equip a shield with a two handed weapon equipped!");
					letsbounce=1;
				}
				else if (pi2->itmhand==3 && pi->itmhand==2)
				{
					sysmessage(s,"You cannot equip a two handed weapon with a shield equipped!");
					letsbounce=1;
				}
			}
			if(letsbounce)//Let's bounce the item
			{
				Sndbounce5(s);
				if (ps->IsDragging())
				{
					ps->ResetDragging();
					item_bounce4(s,pi);
					UpdateStatusWindow(s,pi);
					itemsfx(s, pi->id());		// antichrist
				}
				return;
			}
		}
		if (!(pc_currchar->isGM())) //Ripper..players cant equip items on other players or npc`s paperdolls.
		{
			if ((pc_k != pc_currchar) && (!pc_k->isNpc()))
			{
				sysmessage(s, "You cant put items on other players!");
				item_bounce6(ps,pi);
				return;
			}
		}
		pi->SetContSerial(LongFromCharPtr(buffer[s]+6));
		pi->layer=buffer[s][5];
		// AntiChrist - now the STAT BONUS works -
		pc_currchar->st = (pc_currchar->st + pi->st2);
		pc_currchar->chgDex(pi->dx2);
		pc_currchar->in = (pc_currchar->in + pi->in2);
		if (pi->trigtype==2) // -Frazurbluu- Trigger Type 2 is my new trigger type *-
		{
			Trig->triggerwitem(s, pi, 1); // trigger is fired
		}	
		// AntiChrist -- for poisoned items
		if (showlayer)	clConsole.send("Item equipped on layer %i.\n",pi->layer);
		
		SndRemoveitem(pi->serial);
		
		LongToCharPtr(pi->serial,wearitem+1);
		ShortToCharPtr(pi->id(),wearitem+5);
		wearitem[8]=pi->layer;
		LongToCharPtr(pi->contserial,wearitem+9);
		ShortToCharPtr(pi->color, &wearitem[13]);
		Xsend(s, wearitem, 15);
		wornitems(s, pc_k);//send update to current socket
		// -Frazurbluu- Worn item triggers will need code here
		// Trigger cod ewill also need the adjustments made for skill adding
		// An apply/unapply type of variable must be added for skill gains
		// Spell Item will have to be considered, like a necklace of reflection

		for (j=0;j<now;j++)
		{
			if (perm[j] && inrange1p(pc_k, currchar[j]) && (j!=s))//and to all inrange sockets (without re-sending to current socket)//AntiChrist
				wornitems(j, pc_k);
		}
		
		//itemsfx(s, pi->id());	// Dupois - see itemsfx() for details	// Added Oct 09, 1998
		soundeffect( s, 0x00, 0x57 );
		Weight->NewCalc(pc_currchar);	// Ison 2-20-99
		statwindow(s, pc_currchar);
		
		if (pi->glow != INVALID_SERIAL)
		{
			pc_currchar->removeHalo(pi); // if gm equips on differnt player it needs to be deleted out of the hashteble
			pc_k->addHalo(pi);
			pc_k->glowHalo(pi);
		}
    }
}

static bool ItemDroppedOnPet(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR pc_target = FindCharBySerial(pp->Tserial);

	if( pc_target->hunger < 6 && pi->type == 14 )//AntiChrist new hunger code for npcs
	{
		soundeffect2(pc_currchar, 0x003A+(rand()%3));	//0x3A - 0x3C three different sounds

		if((pi->poisoned)&&(pc_target->poisoned<pi->poisoned))
		{
			soundeffect2(pc_target, 0x0246); //poison sound - SpaceDog
			pc_target->poisoned=pi->poisoned;
			pc_target->poisontime=uiCurrentTime+(MY_CLOCKS_PER_SEC*(40/pc_target->poisoned)); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			pc_target->poisonwearofftime=pc_target->poisontime+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()); //wear off starts after poison takes effect - AntiChrist
			impowncreate(s, pc_target, 1); //Lb, sends the green bar !
		}
		
		if(pi->name == "#")
			pi->getName(temp2);
		sprintf((char*)temp,"* You see %s eating %s *",pc_target->name.c_str(),temp2);
		pc_target->emotecolor = 0x0026;
		npcemoteall(pc_target,(char*)temp,1);
		pc_target->hunger++;
	}
	else
	{
		sysmessage(s,"It doesn't appear to want the item");
		Sndbounce5(s);
		if (ps->IsDragging())
		{
			ps->ResetDragging();
			item_bounce5(s,pi);
		}
	}
	return true;
}

static bool ItemDroppedOnGuard(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR target = FindCharBySerial(pp->Tserial);
	// Search for the key word "the head of"
	if( strstr( pi->name.c_str(), "the head of" ) )
	{
		// This is a head of someone, see if the owner has a bounty on them
		P_CHAR pCharIdx = FindCharBySerial( pi->ownserial );
		
		if( pCharIdx->questBountyReward > 0 )
		{
			// Give the person the bounty assuming that they are not the
			// same person as the reward is for
			if( pc_currchar->serial != pCharIdx->serial )
			{
				// give them the gold for bringing the villan to justice
				addgold( s, pCharIdx->questBountyReward );
				goldsfx( s, pCharIdx->questBountyReward );
				
				// Now thank them for their hard work
				sprintf((char*) temp, "Excellent work! You have brought us the head of %s. Here is your reward of %d gold coins.",
					pCharIdx->name.c_str(),
					pCharIdx->questBountyReward );
				npctalk( s, target, (char*)temp, 0);
				
				// Delete the Bounty from the bulletin board
				Bounty->BountyDelete( pCharIdx->serial );
				
				// Adjust their karma and back to what it was before the beheading!
				pc_currchar->karma  += 100;
			}
			else
				npctalk(s, target, "You can not claim that prize scoundrel. You are lucky I don't strike you down where you stand!",0);
			
			// Delete the item
			Items->DeleItem(pi);
			return true;
		}
	}
	return false;
}

static bool ItemDroppedOnBeggar(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR target = FindCharBySerial(pp->Tserial);
	if(pi->id()!=0x0EED)
	{
		sprintf((char*)temp,"Sorry %s i can only use gold",pc_currchar->name.c_str());
		npctalk(s,target,(char*)temp,0);
		return false;
	}
	sprintf((char*)temp,"Thank you %s for the %i gold!",pc_currchar->name.c_str(), pi->amount);
	npctalk(s,target,(char*)temp,0);
	if(pi->amount<=100)
	{
		pc_currchar->karma += 10;
		sysmessage(s,"You have gain a little karma!");
	}
	else
	{
		pc_currchar->karma += 50;
		sysmessage(s,"You have gain some karma!");
	}
		Items->DeleItem(pi);
	    return true;
}

static bool DeedDroppedOnBroker(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR target = FindCharBySerial(pp->Tserial);
	P_ITEM bankbox = pc_currchar->GetBankBox();
	int value = pi->value/1.25;
	int total = pi->value/1.25;
	
	if ((pi->morex >= 1 && pi->morex <= 14) || (pi->morex >= 16 && pi->morex <= 17) || (pi->morex >= 26 && pi->morex <= 32))
	{
		while ( value > 65000)
		{
			const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, 65000, "#", 1, 0x0E, 0xED, 0, 0, 0);
		    if(pi_gold == NULL) return false;
			bankbox->AddItem(pi_gold);
			value -= 65000;
		}
		const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, value, "#", 1, 0x0E, 0xED, 0, 0, 0);
	    if(pi_gold == NULL) return false;
		bankbox->AddItem( pi_gold );
		Items->DeleItem( pi ); // deed is consumed.
		sprintf((char*)temp,"%s thank you for your patronage, I have deposited %i gold into your bank account.",pc_currchar->name.c_str(), total);
		npctalk(s,target,(char*)temp,0);
	    statwindow(s, pc_currchar);
		return true;
	}
    else
	{
		  sprintf((char*)temp,"Sorry %s i can only accept house deeds.",pc_currchar->name.c_str());
		  npctalk(s,target,(char*)temp,0);
		  Sndbounce5(s);
		  if (ps->IsDragging())
		  {
			 ps->ResetDragging();
			 item_bounce5(s,pi);
			 return true;
		  }
	}
	return true;
}

static bool ItemDroppedOnBanker(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR target = FindCharBySerial(pp->Tserial);
	P_ITEM bankbox = pc_currchar->GetBankBox();
	int amt = pi->amount;
	int value = pi->value;
	
	if (pi->id() == 0x14F0 && pi->type == 1000)
	{
		while ( pi->value > 65000)
		{
			const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, 65000, "#", 1, 0x0E, 0xED, 0, 0, 0);
		    if(pi_gold == NULL) return false;
			bankbox->AddItem(pi_gold);
			pi->value -= 65000;
		}
		const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, pi->value, "#", 1, 0x0E, 0xED, 0, 0, 0);
	    if(pi_gold == NULL) return false;
		bankbox->AddItem( pi_gold );
		Items->DeleItem( pi ); // Check is consumed.
		sprintf((char*)temp,"%s I have cashed your check and deposited %i gold.",pc_currchar->name.c_str(), value);
		npctalk(s,target,(char*)temp,0);
	    statwindow(s, pc_currchar);
		return true;
	}
    else if (pi->id() == 0x0EED)
	{
		sprintf((char*)temp,"%s you have deposited %i gold.",pc_currchar->name.c_str(), amt);
		npctalk(s,target,(char*)temp,0);
		bankbox->AddItem(pi);
	    statwindow(s, pc_currchar);
		return true;
	}
    else
	{
		  sprintf((char*)temp,"Sorry %s i can only deposit gold",pc_currchar->name.c_str());
		  npctalk(s,target,(char*)temp,0);
		  Sndbounce5(s);
		  if (ps->IsDragging())
		  {
			 ps->ResetDragging();
			 item_bounce5(s,pi);
			 return true;
		  }
	}
	return true;
}

static bool ItemDroppedOnTrainer(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR pc_t = FindCharBySerial(pp->Tserial);

	if( pi->id() ==0x0EED )
	{ // They gave the NPC gold
		char sk=pc_t->trainingplayerin;
		npctalk(s, pc_t, "I thank thee for thy payment. That should give thee a good start on thy way. Farewell!",0);

		int sum = pc_currchar->getSkillSum();
		int delta = pc_t->getTeachingDelta(pc_currchar, sk, sum);

		if(pi->amount>delta) // Paid too much
		{
			pi->amount-=delta;
			Sndbounce5(s);
			if (ps->IsDragging())
			{
				ps->ResetDragging();
				item_bounce5(s,pi);
			}
		}
		else
		{
			if(pi->amount < delta)		// Gave less gold
				delta = pi->amount;		// so adjust skillgain
			Items->DeleItem(pi);
		}
		pc_currchar->baseskill[sk]+=delta;
		Skills->updateSkillLevel(pc_currchar, sk);
		updateskill(s,sk);

		pc_currchar->trainer=-1;
		pc_t->trainingplayerin='\xFF';
		itemsfx(s, pi->id());//AntiChrist - do the gold sound
		return true;
	}
	else // Did not give gold
	{
		npctalk(s, pc_t, "I am sorry, but I can only accept gold.",0);
		Sndbounce5(s);
		if (ps->IsDragging())
		{
			ps->ResetDragging();
			item_bounce5(s,pi);
			return true;
		}
		else
			return true;
	}//if items[i]=gold
	return true;
}

static bool ItemDroppedOnSelf(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	
	if (pi->id1>=0x40) // crashfix , prevents putting multi-objects ni your backback
	{
		sysmessage(s,"Hey, putting houses in your pack crashes your back and client !");
		pi->moveTo(pc_currchar->pos);
		RefreshItem(pi);//AntiChrist
		return true;
	}
	
	if (pi->glow != INVALID_SERIAL) // glowing items
	{
		pc_currchar->addHalo(pi);
		pc_currchar->glowHalo(pi);
	}
	
	P_ITEM pack = Packitem(pc_currchar); // LB ...
	if (pack == NULL) // if player has no pack, put it at its feet
	{
		pi->moveTo(pc_currchar->pos);
		RefreshItem(pi);//AntiChrist
	}
	else
	{
		pack->AddItem(pi); // player has a pack, put it in there
		
		Weight->NewCalc(pc_currchar);//AntiChrist bugfixes
		statwindow(s, pc_currchar);
		// That leads to VERY strange noises
		//itemsfx(s, pi->id());
		soundeffect2( pc_currchar, 0x57 );
	}
	return true;
}

static bool ItemDroppedOnChar(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR pTC = FindCharBySerial(pp->Tserial);	// the targeted character
	if (!pTC) return true;

	if (pTC != pc_currchar)
	{
		if (pTC->isNpc())
		{
			if(!pTC->isHuman())						
			{
				ItemDroppedOnPet( ps, pp, pi);
			}
			else	// Item dropped on a Human character
			{
				// Item dropped on a Guard (possible bounty quest)
				if( ( pTC->isNpc() ) && ( pTC->npcaitype == 4 ) )
				{
					if (!ItemDroppedOnGuard( ps, pp, pi) )
					{
						Sndbounce5(s);
						if (ps->IsDragging())
						{
							ps->ResetDragging();
							item_bounce5(s,pi);
						}
					}
					return true;
				}
				if ( pTC->npcaitype == 5 )
				{
					if (!ItemDroppedOnBeggar( ps, pp, pi))
					{
						Sndbounce5(s);
						if (ps->IsDragging())
						{
							ps->ResetDragging();
							item_bounce5(s,pi);
						}
					}
					return true;
				}
				if ( pTC->npcaitype == 19 )
				{
					if (!DeedDroppedOnBroker( ps, pp, pi))
					{
						Sndbounce5(s);
						if (ps->IsDragging())
						{
							ps->ResetDragging();
							item_bounce5(s,pi);
						}
					}
					return true;
				}
				if ( pTC->npcaitype == 8 )
				{
					if (!ItemDroppedOnBanker( ps, pp, pi))
					{
						Sndbounce5(s);
						if (ps->IsDragging())
						{
							ps->ResetDragging();
							item_bounce5(s,pi);
						}
					}
					return true;
				}
				
				//This crazy training stuff done by Anthracks (fred1117@tiac.net)
				if(pc_currchar->trainer!=pTC->serial)
				{
					npctalk(s, pTC, "Thank thee kindly, but I have done nothing to warrant a gift.",0);
					Sndbounce5(s);
					if (ps->IsDragging())
					{
						ps->ResetDragging();
						item_bounce5(s,pi);
					}
					return true;
				}
				else // The player is training from this NPC
				{
					ItemDroppedOnTrainer( ps, pp, pi);
					return true;
				}
			}//if human or not
		}
		else // dropped on another player
		{
			// By Polygon: Avoid starting the trade if GM drops item on logged on char (crash fix)
			if ((pc_currchar->isGM()) && !online(pTC))
			{
				// Drop the item in the players pack instead
				// Get the pack
				P_ITEM pack = Packitem(pTC);
				if (pack != NULL)	// Valid pack?
				{
					pack->AddItem(pi);	// Add it
					Weight->NewCalc(pTC);
				}
				else	// No pack, give it back to the GM
				{
					pack = Packitem(pc_currchar);
					if (pack != NULL)	// Valid pack?
					{
						pack->AddItem(pi);	// Add it
						Weight->NewCalc(pc_currchar);
					}
					else	// Even GM has no pack?
					{
						// Drop it to it's feet
						pi->moveTo(pc_currchar->pos);
						RefreshItem(pi);
					}
				}
			}
			else
			{
				P_ITEM j=Trade->tradestart(s, pTC); //trade-stuff
				if(j==NULL)
				{
					cout << "Bad trade start ptr " << endl;
					return true;
				}
				pi->SetContSerial(j->serial);
				pi->pos.x = 30;
				pi->pos.y = 30;
				pi->pos.z = 9;
				SndRemoveitem(pi->serial);
				RefreshItem(pi);
			}
		}
	}
	else // dumping stuff to his own backpack !
	{
		ItemDroppedOnSelf( ps, pp, pi);
	}
	return true;
}

void dump_item(P_CLIENT ps, PKGx08 *pp) // Item is dropped on ground or a character
{
//	tile_st tile;
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	
	P_ITEM pi = FindItemBySerial(pp->Iserial);
	if (!pi)
	{
		LogErrorVar("client sent bad itemserial %d", pp->Iserial);
		return;
	}

	Weight->NewCalc(pc_currchar);
	statwindow(s, pc_currchar);
	pi->flags.isBeeingDragged = false;
	
	//Ripper...so order/chaos shields disappear when on ground.
	if( pi->id1 == 0x1B && ( pi->id2 == 0xC3 || pi->id2 == 0xC4 ) )
	{
		soundeffect2(pc_currchar, 0x01FE);
		staticeffect(pc_currchar, 0x37, 0x2A, 0x09, 0x06);
		Items->DeleItem( pi );
		return;
	}
	
	//test UOP blocking Tauriel 1-12-99
	if (!pi->isInWorld())
	{
		item_bounce6(ps,pi);
		return;
	}
	
	if (!pc_currchar->canPickUp(pi))
	{
		item_bounce6(ps,pi);
		return;
	}
	
	if (buffer[s][5]!=(unsigned char)'\xFF')
	{
		itemsfx( s, pi->id() ); // DS

		pi->MoveTo(pp->TxLoc,pp->TyLoc,pp->TzLoc);
		pi->SetContSerial(-1);
		
		if (pi->glow != INVALID_SERIAL)
		{
			pc_currchar->removeHalo(pi);
			pc_currchar->glowHalo(pi);
		}		
		SndRemoveitem(pi->serial);
		RefreshItem(pi);
	}
	else
	{
		ItemDroppedOnChar(ps, pp, pi);
		
		Weight->NewCalc(pc_currchar);  // Ison 2-20-99
		statwindow(s, pc_currchar);
		//itemsfx(s, pi->id());	// Dupois - see itemsfx() for details// Added Oct 09, 1998
		
		//Boats !
		if (pc_currchar->multis>0) //How can they put an item in a multi if they aren't in one themselves Cut lag by not checking everytime something is put down
		{
			P_ITEM pi_multi = FindItemBySerial( pc_currchar->multis );
			if (pi_multi != NULL)
			{
				pi_multi = findmulti(pi->pos);
				if (pi_multi != NULL)
					pi->SetMultiSerial(pi_multi->serial);
			}
		}
		//End Bots
	}
}

void pack_item(P_CLIENT ps, PKGx08 *pp) // Item is put into container
{
	SERIAL serial;
	bool abort = false;
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	
	serial=pp->Tserial;
	if(serial == INVALID_SERIAL) abort = true;
	const P_ITEM pCont= FindItemBySerial( serial );
	
	serial = pp->Iserial;
	if(serial == INVALID_SERIAL) abort = true;
	const P_ITEM pItem = FindItemBySerial( serial );
	
	if (pCont == NULL)
	{
		RefreshItem(pCont);//AntiChrist
		return;
	}
	
	if (pItem == NULL || pCont == NULL) return; //LB
	pItem->flags.isBeeingDragged = false;

	if (pItem->id1>=0x40)
	{
	   abort = true; // LB crashfix that prevents moving multi objcts in BP's
       sysmessage(s, "Hey, putting houses in your pack crashes your back and client!");
	}
	P_CHAR pc_j = GetPackOwner(pCont);
	if (pc_j != NULL)
	{
		if (pc_j->npcaitype==17 && pc_j->isNpc() && !pc_currchar->Owns(pc_j))
		{
			abort = true;
			sysmessage(s, "This aint your vendor!");				
		}
	}

	if(abort)
	{//AntiChrist to preview item disappearing
		item_bounce6(ps,pItem);
		return;
	}

	if (pCont->layer==0 && pCont->id() == 0x1E5E &&	pc_currchar->Wears(pCont))
	{
		// Trade window???
		serial=calcserial(pCont->moreb1, pCont->moreb2, pCont->moreb3, pCont->moreb4);
		if(serial == INVALID_SERIAL) return;
		P_ITEM pi_z = FindItemBySerial( serial );
		
		if (pi_z != NULL)
			if ((pi_z->morez || pCont->morez))
			{
				pi_z->morez=0;
				pCont->morez=0;
				sendtradestatus(pi_z, pCont);
			}
	}
	
	//
	//AntiChrist - Special Bank Stuff
	//
	//if morey==123  - gold only bank	
	//
	if(SrvParams->useSpecialBank())//only if special bank is activated
	{
		if(pCont->morey==123 && pCont->morex==1 && pCont->type==1)
		{
			if ( pItem->id() == 0x0EED )
			{//if they're gold ok
				goldsfx(s, 2);
			} else
			{//if they're not gold..bounce on ground
				sysmessage(s,"You can only put golds in this bank box!");

				pItem->SetContSerial(-1);
				pItem->moveTo(pc_currchar->pos);
				RefreshItem(pItem);//AntiChrist
				itemsfx(s,pItem->id());
				return;
			}
		}
	}
	
	//testing UOP Blocking Tauriel 1-12-99
	if (!pItem->isInWorld())
	{
		item_bounce6(ps,pItem);
		return;
	}
	
	if (!pc_currchar->canPickUp(pItem))
	{
		Sndbounce5(s);
		if (ps->IsDragging())
		{
			ps->ResetDragging();
			item_bounce3(pItem);
			if (pCont->id1>=0x40)
				senditem(s, pCont);
		}
		return;
	}
	// - Trash container
	if (pCont->type==87)
	{
		Items->DeleItem(pItem);
		sysmessage(s, "As you let go of the item it disappears.");
		return;
	}
	// - Spell Book
	if (pCont->type==9)
	{
		if (!IsSpellScroll72(pItem->id()))
		{
			sysmessage(s, "You can only place spell scrolls in a spellbook!");
			Sndbounce5(s);
			if (ps->IsDragging())
			{
				ps->ResetDragging();
				item_bounce3(pItem);
			}
			if (pCont->id1>=0x40)
				senditem(s, pCont);
			return;
		}
		P_ITEM pBackpack = Packitem(pc_currchar);
		if (pBackpack != NULL) // lb
		{
			if (!pc_currchar->Wears(pCont) &&
				(!(pCont->contserial==pBackpack->serial)) && (!(pc_currchar->canSnoop())))
			{
				sysmessage(s, "You cannot place spells in other peoples spellbooks.");
				item_bounce6(ps,pItem);
				return;
			}
			
			if(pItem->name == "#")
				pItem->getName(temp2);
			else
				strcpy((char*)temp2, pItem->name.c_str());

			vector<SERIAL> vecContainer = contsp.getData(pCont->serial);
			for (unsigned int i = 0; i < vecContainer.size(); i++) // antichrist , bugfix for inscribing scrolls
			{
				P_ITEM pi = FindItemBySerial(vecContainer[i]);
				if (pi != NULL)
				{
					if(pi->name == "#")
						pi->getName(temp);
					else
						strcpy((char*)temp, pi->name.c_str());

					if(!(strcmp((char*)temp,(char*)temp2)) || !(strcmp((char*)temp,"All-Spell Scroll")))
					{
						sysmessage(s,"You already have that spell!");
						item_bounce6(ps,pItem);
						return;
					}
				}
			}
		}
	}
	
	// player run vendors
	if (!(pCont->pileable && pItem->pileable && pCont->id()==pItem->id()
		|| (pCont->type!=1 && pCont->type!=9)))
	{
		P_CHAR pc_j = GetPackOwner(pCont);
		if (pc_j != NULL)
		{
			if (pc_j->npcaitype==17 && pc_j->isNpc() && pc_currchar->Owns(pc_j))
			{
				pc_currchar->inputitem = pItem->serial;
				pc_currchar->inputmode = cChar::enPricing;
				sysmessage(s, "Set a price for this item.");
			}
		}

		short xx=pp->TxLoc;
		short yy=pp->TyLoc;

		pCont->AddItem(pItem,xx,yy);
		
		//itemsfx(s, pItem->id());// see itemsfx() for details - Dupois Added Oct 09, 1998
		soundeffect2( pc_currchar, 0x57 );
		statwindow(s, pc_currchar);
	}
	// end of player run vendors
	
	else
		// - Unlocked item spawner or unlockable item spawner
		if (pCont->type==63 || pCont->type==65 || pCont->type==66)
		{
			pItem->SetContSerial(pp->Tserial);

			// lb bugfix
			pItem->pos.x=pp->TxLoc;
			pItem->pos.y=pp->TyLoc;
			pItem->pos.z=pp->TzLoc;
			
			SndRemoveitem(pItem->serial);
			RefreshItem(pItem);//AntiChrist
			//itemsfx(s, pItem->id());
			soundeffect2( pc_currchar, 0x57 );			
		}
		else  // - Pileable
			if (pCont->pileable && pItem->pileable && pCont->id()==pItem->id())
			{		
				if ((pCont->amount+pItem->amount) > 65535)
				{
					pItem->amount -= (65535-pCont->amount);
					Commands->DupeItem(s, pCont, pItem->amount);
					pCont->amount = 65535;
					Items->DeleItem(pItem);
				}
				else
				{
					pCont->amount=pCont->amount+pItem->amount;
					itemsfx(s, pItem->id());
					Items->DeleItem(pItem);
				}
				SndRemoveitem(pItem->serial);
				RefreshItem(pCont);//AntiChrist
			}
			else
			{
				pItem->pos.x=pp->TxLoc;
				pItem->pos.y=pp->TyLoc;
				pItem->pos.z=pp->TzLoc;
//				pItem->SetContSerial(-1);
				pItem->SetContSerial(pp->Tserial);
				// Tauriel add item to world mapcells
				mapRegions->Add(pItem); //add this item to a map cell
				
				SndRemoveitem(pItem->serial);
				RefreshItem(pCont);//AntiChrist
			}
			
			// - Spell Book
	
			if (pCont->type==9)
				Magic->SpellBook(s, pCont); // LB, bugfix for showing(!) the wrong spell (clumsy) when a new spell is put into opened spellbook

			if (pItem->glow != INVALID_SERIAL) // LB's glowing items stuff
			{
				P_CHAR pc = GetPackOwner(pCont);
				pc_currchar->removeHalo(pItem); // if gm put glowing object in another pack, handle glowsp correctly !
				if (pc != NULL)
				{
					pc->addHalo(pItem);
					pc->glowHalo(pItem);
				}		
				
			}	
}

void cDragdrop::drop_item(P_CLIENT ps) // Item is dropped
{
	UOXSOCKET s=ps->GetSocket();

	PKGx08 pkgbuf, *pp=&pkgbuf;

	pp->Iserial=LongFromCharPtr(buffer[s]+1);
	pp->TxLoc=ShortFromCharPtr(buffer[s]+5);
	pp->TyLoc=ShortFromCharPtr(buffer[s]+7);
	pp->TzLoc=buffer[s][9];
	pp->Tserial=LongFromCharPtr(buffer[s]+10);

    //#define debug_dragg
	
	if (clientDimension[s]==3)
	{
	  // UO:3D clients send SOMETIMES two dragg packets for a single dragg action.
	  // sometimes we HAVE to swallow it, sometimes it has to be interpreted
	  // if UO:3D specific item loss problems are reported, this is probably the code to blame :)
	  // LB
	  P_ITEM pi = FindItemBySerial(pp->Iserial);

	  #ifdef debug_dragg
	    if (i!=-1) { sprintf(temp, "%04x %02x %02x %01x %04x i-name: %s EVILDRAG-old: %i\n",pp->Iserial, pp->TxLoc, pp->TyLoc, pp->TzLoc, pp->Tserial, items[i].name, EVILDRAGG[s]); clConsole.send(temp); }
		else { sprintf(temp, "blocked: %04x %02x %02x %01x %04x i-name: invalid item EVILDRAG-old: %i\n",pp->Iserial, pp->TxLoc, pp->TyLoc, pp->TzLoc, pp->Tserial, EVILDRAGG[s]); clConsole.send(temp); }
      #endif

	  if  ( (pp->TxLoc==-1) && (pp->TyLoc==-1) && (pp->Tserial==0)  && (EVILDRAGG[s]==1) )
	  {
		  EVILDRAGG[s]=0;
          #ifdef debug_dragg
		    clConsole.send("Swallow only\n");
          #endif
		  return;
	  }	 // swallow! note: previous evildrag !
	  else if ( (pp->TxLoc==-1) && (pp->TyLoc==-1) && (pp->Tserial==0)  && (EVILDRAGG[s]==0) )
	  {
          #ifdef debug_dragg
		    clConsole.send("Bounce & Swallow\n");
          #endif

		  item_bounce6(ps, pi);
		  return;
	  }
	  else if ( ( (pp->TxLoc!=-1) && (pp->TyLoc!=-1) && ( pp->Tserial!=-1)) || ( (isItemSerial(pp->Iserial)) && (isItemSerial(pp->Tserial)) ) ) EVILDRAGG[s]=1; // calc new evildrag value
	  else EVILDRAGG[s]=0;
	}

	#ifdef debug_dragg
	  else
	  {
		 ITEM i = calcItemFromSer( pp->Iserial );
	     if (i!=-1) { sprintf(temp, "blocked: %04x %02x %02x %01x %04x i-name: %s EVILDRAG-old: %i\n",pp->Iserial, pp->TxLoc, pp->TyLoc, pp->TzLoc, pp->Tserial, items[i].name, EVILDRAGG[s]); clConsole.send(temp); }
	  }
    #endif

	if ( (buffer[s][10]>=0x40) && (buffer[s][10]!=0xff) )
		pack_item(ps,pp);
	else
		dump_item(ps,pp);
}
