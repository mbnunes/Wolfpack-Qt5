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
static bool CheckWhereItem( int pack, P_ITEM pi, int s)
{
	if (pi && pack!=-1 && s!=-1) //LB
	{
		if (!( pi->contserial==items[pack].serial ||
			chars[currchar[s]].Wears(pi)))
			
			return 1;
		else
			return 0;
	}
	return 0;
}

void UpdateStatusWindow(UOXSOCKET s, P_ITEM pi)
{
	int packnum=packitem(currchar[s]);
	if (CheckWhereItem(packnum, pi, s))
		statwindow(s,currchar[s]);
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
	pi->pos.x=pi->oldx;
	pi->pos.y=pi->oldy;
	pi->pos.z=pi->oldz;
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

void get_item(P_CLIENT ps) // Client grabs an item
{
	int x,  npc=-1, amount, update = 0, serial;
//	tile_st tile;
	int z;// antichrist for trade fix
	UOXSOCKET s = ps->GetSocket();
	int cc = ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);
	
	
	serial = calcserial(buffer[s][1], buffer[s][2], buffer[s][3], buffer[s][4]);
	if (serial == INVALID_SERIAL || buffer[s][1] < 0x40)
		return;	// landscape or a character
	P_ITEM pi = FindItemBySerial(serial);
	if (pi == NULL)
		return;
	
	pc_currchar->disturbMed(s); // Meditation
	
	// Zippy's stealing changes  
	x = DEREF_P_ITEM(pi);
	if (!items[x].isInWorld())  // Find character owning item
	{
		unsigned long loopexit = 0;
		do  // Find character owning item
		{
			if (isCharSerial(items[x].contserial))
			{
				npc = calcCharFromSer(items[x].contserial);
			}
			else  // its an item
			{
				if (items[x].isInWorld())
				{
					npc=-1;
					break;
				}
				x = calcItemFromSer(items[x].contserial);
				// ANTICHRIST -- SECURE TRADE FIX
				if (x!=-1) // LB overwriting x is essential here, dont change it!!!
				{
					if (items[x].layer == 0 && items[x].id() == 0x1E5E)
					{
						// Trade window???
						serial = calcserial(items[x].moreb1, items[x].moreb2, items[x].moreb3, items[x].moreb4);
						if (serial == INVALID_SERIAL)
							return;
						z = calcItemFromSer(serial);
						if (z!=-1)
							if ((items[z].morez || items[x].morez))
							{
								items[z].morez = 0;
								items[x].morez = 0;
								sendtradestatus(z, x);
							}
					}
					// Blackwinds Looting is crime implementation
					// changed slightly by Ripper
					if (items[x].corpse != 0 && !pc_currchar->Owns(&items[x])) 
					{ 
						P_CHAR co = FindCharBySerial(items[x].ownserial);
						if (items[x].more2 == 1 && Guilds->Compare(cc, DEREF_P_CHAR(co)) == 0) 
						{ 
							pc_currchar->karma -= 5; 
							criminal(cc);
							sysmessage(s, "You lost some karma!"); 
						} 
						npc = 0;
					} // Criminal stuff
					if (items[x].corpse != 0)
						npc = 0;
				} // end if x!=-1
				
				if (x==-1)
					npc = 0; 
			}
		} while ((npc==-1) &&(++loopexit < MAXLOOPS));
	}
	
	if (npc>0) // 0=corpse, hence >0 ..
	{
		if (!(pc_currchar->isGM()) && npc != cc && ! pc_currchar->Owns(&chars[npc]))
		{// Own serial stuff by Zippy -^ Pack aniamls and vendors.
			bounce[1] = 0;
			Xsend(s, bounce, 2);
			if (ps->IsDragging())
			{
				ps->ResetDragging();
				item_bounce3(pi);
				pi->magic = 3;
			} 
			return;
		}
	}
	// End Zippy's change
	
	// Boats->
	if (x!=-1 && npc!=-1)
	{
		if (items[x].multis>0)
			imultisp.remove(items[x].multis, items[x].serial);
		items[x].startDecay();
		// End Boats Change
		
		// AntiChrist -- for poisoned items
		if (items[x].layer>0)
		{
			chars[npc].removeItemBonus(&items[x]);	// remove BONUS STATS given by equipped special items
		}
		if ((items[x].trigon==1) && (items[x].layer != 0) && (items[x].layer != 15) && (items[x].layer < 19))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
		{
			triggerwitem(s, DEREF_P_ITEM(pi), 1); // trigger is fired
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
				
				pi->oldx = pi->pos.x;	// first let's save the position
				pi->oldy = pi->pos.y;
				pi->oldz = pi->pos.z;
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
						P_ITEM pi_c = Items->MemItemFree();
						//	pi_c->Init(0);
#pragma note("Replace by a copy constructor before finishing items[]")
						memcpy(pi_c, pi, sizeof(cItem));  // Tauriel reduce code faster too
						pi_c->SetSerial(itemcount2);
						itemcount2++;

						pi_c->amount = pi->amount - amount;
						pi_c->SetContSerial(pi_c->contserial);
						pi_c->SetOwnSerial(pi_c->ownserial);
						pi_c->SetSpawnSerial(pi_c->spawnserial);
						
						statwindow(s,cc);
						RefreshItem(pi_c);//AntiChrist
					}
					
					if (pi->id() == 0x0EED) // gold coin
					{
						int packnum = packitem(currchar[s]);
						if (packnum!=-1) // lb
							if (pi->contserial == items[packnum].serial)
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
	if (update) statwindow(s, DEREF_P_CHAR(pc_currchar));
}

void wear_item(P_CLIENT ps) // Item is dropped on paperdoll
{
	int j, k;
	tile_st tile;
	int serial, serhash, ci, letsbounce=0; // AntiChrist (5) - new ITEMHAND system
	UOXSOCKET s=ps->GetSocket();
	int cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);

	int cserial=calcserial(buffer[s][6],buffer[s][7],buffer[s][8],buffer[s][9]);
	if(cserial==-1) return;
	k=calcCharFromSer( cserial );
	
	if( chars[k].dead )  //Exploit fix: Dead ppl can't equip anything.
		return;
	
	P_ITEM pi=FindItemBySerPtr(buffer[s]+1);
	if (!pi) return;
	pi->flags.isBeeingDragged=false;

//	if (clientDimension[s]==3)
//	{
	Map->SeekTile(pi->id(), &tile);
		
	// sprintf(temp, "Tiledata: name: %s flag1: %i flag2: %i flag3: %i flag4: %i layer: %i\n", tile.name, tile.flag1, tile.flag2, tile.flag3, tile.flag4, tile.layer);
	// clConsole.send(temp);
		
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
//	}

	if (pi->id1>=0x40) return; // LB, client crashfix if multi-objects are moved to PD

	if (k==cc || pc_currchar->isGM()) 
	{
		if (k!=-1) //lb
			if (k==cc && pi->st>chars[k].st)
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

//			if (clientDimension[s]==2) Map->SeekTile(pi->id(), &tile);
			if ((((pi->magic==2)||((tile.weight==255)&&(pi->magic!=1)))&&((pc_currchar->priv2&1)==0)) ||
				( (pi->magic==3|| pi->magic==4) && !pc_currchar->Owns(pi)))
			{
				item_bounce6(ps,pi);
				return;
			}
		

		// - AntiChrist (4) - checks for new ITEMHAND system
		// - now you can't equip 2 hnd weapons with 1hnd weapons nor shields!!
		serial=pc_currchar->serial;
		serhash=serial%HASHMAX;
		vector<SERIAL> vecContainer = contsp.getData(serial);
		for (ci=0;ci<vecContainer.size();ci++)
		{
			P_ITEM pi2 = FindItemBySerial(vecContainer[ci]);
			if (pi2 != NULL && pi2->contserial == serial)
			{
				if (pi2->itmhand==1 && pi->itmhand==1)
				{
					sysmessage(s,"You already have a weapon equipped!");
					letsbounce=1;
				}
				else if (pi2->itmhand==2 && pi->itmhand==1)
				{
					sysmessage(s,"Your hands are both occupied!");
					letsbounce=1;
				}
				else if (pi2->itmhand==1 && pi->itmhand==2)
				{
					sysmessage(s,"You cannot equip a two handed weapon with a weapon equipped!");
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
				else if (pi2->layer == tile.layer) 
				{
 					sysmessage(s, "You already have an armor equipped!");
					letsbounce = 1;
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
			if ((k != cc) && (!chars[k].isNpc()))
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
			triggerwitem(s, DEREF_P_ITEM(pi), 1); // trigger is fired
		}	
		// AntiChrist -- for poisoned items
		if (showlayer)	clConsole.send("Item equipped on layer %i.\n",pi->layer);
		
		SndRemoveitem(pi->serial);
		
		LongToCharPtr(pi->serial,wearitem+1);
		ShortToCharPtr(pi->id(),wearitem+5);
		wearitem[8]=pi->layer;
		LongToCharPtr(pi->contserial,wearitem+9);
		wearitem[13]=pi->color1;
		wearitem[14]=pi->color2;
		Xsend(s, wearitem, 15);
		wornitems(s, k);//send update to current socket
		// -Frazurbluu- Worn item triggers will need code here
		// Trigger cod ewill also need the adjustments made for skill adding
		// An apply/unapply type of variable must be added for skill gains
		// Spell Item will have to be considered, like a necklace of reflection

		for (j=0;j<now;j++)
		{
			if (perm[j] && inrange1p(k, currchar[j]) && (j!=s))//and to all inrange sockets (without re-sending to current socket)//AntiChrist
				wornitems(j, k);
		}
		
		itemsfx(s, pi->id());	// Dupois - see itemsfx() for details	// Added Oct 09, 1998
		Weight->NewCalc(cc);	// Ison 2-20-99
		statwindow(s,cc);
		
		if (pi->glow>0)
		{
			pc_currchar->removeHalo(pi); // if gm equips on differnt player it needs to be deleted out of the hashteble
			chars[k].addHalo(pi);
			chars[k].glowHalo(pi);
		}
    }
}

static bool ItemDroppedOnPet(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_target = FindCharBySerial(pp->Tserial);

	if(pc_target->hunger<6 && pi->type==14)//AntiChrist new hunger code for npcs
	{
		soundeffect2(cc, 0x00, 0x3A+(rand()%3));	//0x3A - 0x3C three different sounds

		if((pi->poisoned)&&(pc_target->poisoned<pi->poisoned)) 
		{
			soundeffect2(DEREF_P_CHAR(pc_target), 0x02, 0x46); //poison sound - SpaceDog
			pc_target->poisoned=pi->poisoned;
			pc_target->poisontime=uiCurrentTime+(MY_CLOCKS_PER_SEC*(40/pc_target->poisoned)); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			pc_target->poisonwearofftime=pc_target->poisontime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); //wear off starts after poison takes effect - AntiChrist
			impowncreate(s,DEREF_P_CHAR(pc_target),1); //Lb, sends the green bar ! 
		}
		
		if(pi->name[0]=='#') pi->getName(temp2);
		sprintf((char*)temp,"* You see %s eating %s *",pc_target->name,temp2);
		pc_target->emotecolor1=0x00;
		pc_target->emotecolor2=0x26;
		npcemoteall(DEREF_P_CHAR(pc_target),(char*)temp,1);
		pc_target->hunger++;
	} else
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
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(cc,true);
	int t=calcCharFromSer(pp->Tserial);
	// Search for the key word "the head of"
	if( strstr( pi->name, "the head of" ) )
	{
		// This is a head of someone, see if the owner has a bounty on them
		int nCharIdx = calcCharFromSer( pi->ownserial );
		
		if( chars[nCharIdx].questBountyReward > 0 )
		{
			// Give the person the bounty assuming that they are not the
			// same person as the reward is for
			if( pc_currchar->serial != chars[nCharIdx].serial )
			{
				// give them the gold for bringing the villan to justice
				addgold( s, chars[nCharIdx].questBountyReward );
				goldsfx( s, chars[nCharIdx].questBountyReward );
				
				// Now thank them for their hard work
				sprintf((char*) temp, "Excellent work! You have brought us the head of %s. Here is your reward of %d gold coins.",
					chars[nCharIdx].name,
					chars[nCharIdx].questBountyReward );
				npctalk( s, t, (char*)temp, 0);
				
				// Delete the Bounty from the bulletin board
				BountyDelete( chars[nCharIdx].serial );
				
				// Adjust their karma and fame back to what it was before the beheading!
				pc_currchar->fame   += 100;
				pc_currchar->karma  += 100;
			}
			else
				npctalk(s, t, "You can not claim that prize scoundrel. You are lucky I don't strike you down where you stand!",0);
			
			// Delete the item
			Items->DeleItem(pi);
			return true;
		}
	}
	return true;
}

static bool ItemDroppedOnBeggar(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(cc,true);
	int t=calcCharFromSer(pp->Tserial);
	if(pi->id()!=0x0EED)
	{
		sprintf((char*)temp,"Sorry %s i can only use gold",pc_currchar->name);
		npctalk(s,t,(char*)temp,0);
		Sndbounce5(s);
		if (ps->IsDragging())
		{
			ps->ResetDragging();
			item_bounce5(s,pi);
			return true;
		}
	}
	else
	{
		sprintf((char*)temp,"Thank you %s for the %i gold!",pc_currchar->name,pi->amount);
		npctalk(s,t,(char*)temp,0);
		if(pi->amount<=100)
		{
			pc_currchar->karma += 10;
			sysmessage(s,"You have gain a little karma!");
		}
		else if(pi->amount>100)
		{
			pc_currchar->karma += 50;
			sysmessage(s,"You have gain some karma!");
		}
		Items->DeleItem(pi);
		return true;
	}
	return true;
}

static bool ItemDroppedOnBanker(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(cc,true);
	int t=calcCharFromSer(pp->Tserial);
	P_ITEM bankbox = pc_currchar->GetBankBox();
	int amt = pi->amount;
	int value = pi->value;
	
	if (pi->id() == 0x14F0 && pi->type == 1000)
	{
		 int n=Items->SpawnItem(cc,DEREF_P_CHAR(pc_currchar),value,"#",1,0x0E,0xED,0,0,0,0);
	     if(n==-1) return false;
	     const P_ITEM pi_n=MAKE_ITEMREF_LRV(n,false);
		 sprintf((char*)temp,"%s I have cashed your check and deposited %i gold.",pc_currchar->name, value);
		 npctalk(s,t,(char*)temp,0);
		 bankbox->AddItem(pi_n);
	     statwindow(s, DEREF_P_CHAR(pc_currchar));
		 return true;
	}
    else
	{	    
	if (pi->id() == 0x0EED)
	{
		sprintf((char*)temp,"%s you have deposited %i gold.",pc_currchar->name, amt);
		npctalk(s,t,(char*)temp,0);
		bankbox->AddItem(pi);
	    statwindow(s, DEREF_P_CHAR(pc_currchar));
		return true;
	}
    else
	{
		  sprintf((char*)temp,"Sorry %s i can only deposit gold",pc_currchar->name);
		  npctalk(s,t,(char*)temp,0);
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
}

static bool ItemDroppedOnTrainer(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(cc,true);
	int t=calcCharFromSer(pp->Tserial);

	if( pi->id() ==0x0EED )
	{ // They gave the NPC gold
		char sk=chars[t].trainingplayerin;
		npctalk(s, t, "I thank thee for thy payment. That should give thee a good start on thy way. Farewell!",0);

		int sum = pc_currchar->getSkillSum();
		int delta = chars[t].getTeachingDelta(pc_currchar, sk, sum);

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
		Skills->updateSkillLevel(DEREF_P_CHAR(pc_currchar), sk);
		updateskill(s,sk);

		pc_currchar->trainer=-1;
		chars[t].trainingplayerin='\xFF';
		itemsfx(s, pi->id());//AntiChrist - do the gold sound
		return true;
	}
	else // Did not give gold
	{
		npctalk(s, t, "I am sorry, but I can only accept gold.",0);
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
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(cc,true);
	
	if (pi->id1>=0x40) // crashfix , prevents putting multi-objects ni your backback
	{
		sysmessage(s,"Hey, putting houses in your pack crashes your back and client !");
		pi->MoveTo(pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		RefreshItem(pi);//AntiChrist
		return true;
	}
	
	if (pi->glow>0) // glowing items
	{
		pc_currchar->addHalo(pi);
		pc_currchar->glowHalo(pi);
	}
	
	int pack=packitem(cc); // LB ...
	if (pack==-1) // if player has no pack, put it at its feet
	{ 
		pi->MoveTo(pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		RefreshItem(pi);//AntiChrist
	}
	else
	{
		items[pack].AddItem(pi); // player has a pack, put it in there
		
		Weight->NewCalc(cc);//AntiChrist bugfixes
		statwindow(s,cc);
		itemsfx(s, pi->id());
	}
	return true;
}

static bool ItemDroppedOnChar(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(cc,true);
	P_CHAR pTC=FindCharBySerial(pp->Tserial);	// the targeted character
	if (!pTC) return true;

	if (DEREF_P_CHAR(pTC)!=cc)
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
					ItemDroppedOnGuard( ps, pp, pi);
					return true;
				}
				if ( pTC->npcaitype == 5 )
				{
					ItemDroppedOnBeggar( ps, pp, pi);
					return true;
				}
				if ( pTC->npcaitype == 8 )
				{
					ItemDroppedOnBanker( ps, pp, pi);
					return true;
				}
				
				//This crazy training stuff done by Anthracks (fred1117@tiac.net)
				if(pc_currchar->trainer!=pTC->serial)
				{
					npctalk(s, DEREF_P_CHAR(pTC), "Thank thee kindly, but I have done nothing to warrant a gift.",0);
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
			if ((pc_currchar->isGM()) && !online(DEREF_P_CHAR(pTC)))
			{
				// Drop the item in the players pack instead
				// Get the pack
				int pack = packitem(DEREF_P_CHAR(pTC));
				if (pack != -1)	// Valid pack?
				{
					items[pack].AddItem(pi);	// Add it
					Weight->NewCalc(DEREF_P_CHAR(pTC));
				}
				else	// No pack, give it back to the GM
				{
					pack = packitem(DEREF_P_CHAR(pc_currchar));
					if (pack != -1)	// Valid pack?
					{
						items[pack].AddItem(pi);	// Add it
						Weight->NewCalc(DEREF_P_CHAR(pc_currchar));
					}
					else	// Even GM has no pack?
					{
						// Drop it to it's feet
						pi->MoveTo(pc_currchar->pos.x, pc_currchar->pos.y, pc_currchar->pos.z);
						RefreshItem(pi);
					}
				}
			}
			else
			{
				int j=tradestart(s, DEREF_P_CHAR(pTC)); //trade-stuff
				pi->SetContSerial(items[j].serial);
				pi->pos.x=30;
				pi->pos.y=30;
				pi->pos.z=9;
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
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);
	
	P_ITEM pi=FindItemBySerial(pp->Iserial);
	if (!pi)
	{
		LogErrorVar("client sent bad itemserial %d",pp->Iserial);
		return;
	}

	Weight->NewCalc(cc);
	statwindow(s,cc);
	pi->flags.isBeeingDragged=false;
	
	//Ripper...so order/chaos shields disappear when on ground.
	if( pi->id1 == 0x1B && ( pi->id2 == 0xC3 || pi->id2 == 0xC4 ) )
	{
		soundeffect2(cc, 0x01, 0xFE);
		staticeffect(cc, 0x37, 0x2A, 0x09, 0x06);
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
		pi->MoveTo(pp->TxLoc,pp->TyLoc,pp->TzLoc);
		pi->SetContSerial(-1);
		
		if (pi->glow)
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
		
		Weight->NewCalc(cc);  // Ison 2-20-99
		statwindow(s,cc);
		itemsfx(s, pi->id());	// Dupois - see itemsfx() for details// Added Oct 09, 1998
		
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
	int nCont=-1, nItem=-1;
	int j, z, serial, serhash;
//	tile_st tile;
	bool abort=false;
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc=ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);
	
	serial=pp->Tserial;
	if(serial == INVALID_SERIAL) abort=true;
	serhash=serial%HASHMAX;
	nCont = calcItemFromSer( serial );
	
	serial=pp->Iserial;
	if(serial == INVALID_SERIAL) abort=true;
	serhash=serial%HASHMAX;
	nItem = calcItemFromSer( serial );
	
	if (nCont==-1)
	{
		RefreshItem(nCont);//AntiChrist
		return;
	} 
	
	if (nItem==-1) return; //LB
	const P_ITEM pCont=MAKE_ITEMREF_LR(nCont);	// on error return  (This one could be const ! Duke)
	const P_ITEM pItem=MAKE_ITEMREF_LR(nItem);	// on error return
	pItem->flags.isBeeingDragged=false;

	if (pItem->id1>=0x40) 
	{ 
	   abort=true; // LB crashfix that prevents moving multi objcts in BP's
       sysmessage(s,"Hey, putting houses in your pack crashes your back and client!");
	}
	j=GetPackOwner(DEREF_P_ITEM(pCont));
	if (j>-1)
	if (chars[j].npcaitype==17 && chars[j].isNpc() && !pc_currchar->Owns(&chars[j]))
	{
	   abort=true;
	   sysmessage(s,"This aint your vendor!");				
	}

	if(abort)
	{//AntiChrist to preview item disappearing
		item_bounce6(ps,pItem);
		return;
	}

	if (pCont->layer==0 && pCont->id() == 0x1E5E &&
		pc_currchar->Wears(pCont))
	{
		// Trade window???
		serial=calcserial(pCont->moreb1, pCont->moreb2, pCont->moreb3, pCont->moreb4);
		if(serial == INVALID_SERIAL) return;
		z = calcItemFromSer( serial );
		
		if (z!=-1)
			if ((items[z].morez || pCont->morez))
			{
				items[z].morez=0;
				pCont->morez=0;
				sendtradestatus(z, DEREF_P_ITEM(pCont));
			}
	}
	
	//
	//AntiChrist - Special Bank Stuff
	//
	//if morey==123  - gold only bank	
	//
	if(SrvParms->usespecialbank)//only if special bank is activated
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
				pItem->MoveTo(pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
				RefreshItem(nItem);//AntiChrist
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
				senditem(s, DEREF_P_ITEM(pCont));
		}
		return;
	}
	// - Trash container
	if (pCont->type==87)
	{
		Items->DeleItem(nItem);
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
				senditem(s, DEREF_P_ITEM(pCont));
			return;
		}
		z=packitem(cc);
		if (z!=-1) // lb
		{
			if (!pc_currchar->Wears(pCont) &&
				(!(pCont->contserial==items[z].serial)) && (!(pc_currchar->canSnoop())))
			{
				sysmessage(s, "You cannot place spells in other peoples spellbooks.");
				item_bounce6(ps,pItem);
				return;
			}
			
			if(pItem->name[0]=='#')
				pItem->getName(temp2);
			else
				strcpy((char*)temp2,pItem->name);

			vector<SERIAL> vecContainer = contsp.getData(pCont->serial);
			for (int i=0;i<vecContainer.size();i++) // antichrist , bugfix for inscribing scrolls
			{
				int ci=calcItemFromSer(vecContainer[i]);
				if (ci!=-1)
				{
					if(items[ci].name[0]=='#')
						items[ci].getName(temp);
					else
						strcpy((char*)temp,items[ci].name);

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
		j=GetPackOwner(DEREF_P_ITEM(pCont));
		if (j>-1) // bugkilling, LB, was j=!-1, arghh, C !!!
		{
			if (chars[j].npcaitype==17 && chars[j].isNpc() && pc_currchar->Owns(&chars[j]))
			{
				pc_currchar->inputitem=nItem;
				pc_currchar->inputmode=1;
				sysmessage(s, "Set a price for this item.");
			}
		}

		short xx=pp->TxLoc;
		short yy=pp->TyLoc;

		pCont->AddItem(pItem,xx,yy);
		
		itemsfx(s, pItem->id());// see itemsfx() for details - Dupois Added Oct 09, 1998
		statwindow(s,cc);
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
			RefreshItem(nItem);//AntiChrist
			itemsfx(s, pItem->id());
			
		}
		else  // - Pileable
			if (pCont->pileable && pItem->pileable && pCont->id()==pItem->id())
			{		
				if ((pCont->amount+pItem->amount) > 65535)
				{
					pItem->amount -= (65535-pCont->amount);
					Commands->DupeItem(s, DEREF_P_ITEM(pCont), pItem->amount);
					pCont->amount = 65535;
					Items->DeleItem(nItem);
				}
				else
				{
					pCont->amount=pCont->amount+pItem->amount;
					itemsfx(s, pItem->id());
					Items->DeleItem(nItem);
				}
				SndRemoveitem(pItem->serial);
				RefreshItem(DEREF_P_ITEM(pCont));//AntiChrist
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
				RefreshItem(DEREF_P_ITEM(pCont));//AntiChrist
			}
			
			// - Spell Book
	
			if (pCont->type==9)
				Magic->SpellBook(s,DEREF_P_ITEM(pCont)); // LB, bugfix for showing(!) the wrong spell (clumsy) when a new spell is put into opened spellbook

			if (pItem->glow>0) // LB's glowing items stuff
			{
				int p=GetPackOwner(DEREF_P_ITEM(pCont)); 
				pc_currchar->removeHalo(pItem); // if gm put glowing object in another pack, handle glowsp correctly !
				//removefromptr(&glowsp[pc_currchar->serial%HASHMAX],nItem);
				if (p!=-1) 
				{
					chars[p].addHalo(pItem);
					chars[p].glowHalo(pItem);
				}		   
				
			}	
}

void drop_item(P_CLIENT ps) // Item is dropped
{
	UOXSOCKET s=ps->GetSocket();
//	CHARACTER cc=ps->GetCurrChar();

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
	  ITEM i = calcItemFromSer(pp->Iserial);

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

		  item_bounce6(ps, &items[i]); 
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
