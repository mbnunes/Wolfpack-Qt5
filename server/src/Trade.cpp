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

/////////////////////
// Name:	Trade.cpp
// Purpose: functions that are related to trade
// History:	cut from wolfpack.cpp by Duke, 26.10.00
// Remarks:	not necessarily ALL those functions
//

#include "Trade.h"

#undef  DBGFILE
#define DBGFILE "Trade.cpp"

void buyaction(int s)
{
	char clearmsg[8];
	int clear, i, j;
	P_ITEM buyit[256];
	int amount[512];
	int layer[512];
	int playergoldtotal;
	int goldtotal;
	int itemtotal;
	int npc;
	int soldout;
	int tmpvalue=0; // Fixed for adv trade system -- Magius(CHE) §
	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);
	P_ITEM pi_pack = Packitem(pc_currchar);
	if (pi_pack == NULL) 
		return; //LB no player-pack - no buy action possible - and no crash too ;-)
	npc=calcCharFromSer(buffer[s][3], buffer[s][4], buffer[s][5], buffer[s][6]);

	if (npc <= 0) return;

	clear=0;
	goldtotal=0;
	soldout=0;
	itemtotal=(((256*(buffer[s][1]))+buffer[s][2])-8)/7;
	if (itemtotal>256) return; //LB

	for(i = 0; i < itemtotal; i++)
	{
		layer[i]=buffer[s][8+(7*i)];
		buyit[i] = FindItemBySerial(calcserial(buffer[s][8+(7*i)+1], buffer[s][8+(7*i)+2], buffer[s][8+(7*i)+3], buffer[s][8+(7*i)+4]));
		amount[i]=(256*(buffer[s][8+(7*i)+5]))+buffer[s][8+(7*i)+6];

		if (buyit[i] != NULL)
		{
			buyit[i]->rank=10;
			// Fixed for adv trade system -- Magius(CHE) §
			tmpvalue = buyit[i]->value;
			tmpvalue = calcValue(buyit[i], tmpvalue);
			if (SrvParms->trade_system==1) tmpvalue = calcGoodValue(s, buyit[i], tmpvalue,0);
			goldtotal += (amount[i]*tmpvalue);
			// End Fix for adv trade system -- Magius(CHE) §
		}
	}

	bool useBank;
	useBank = (goldtotal >= SrvParms->CheckBank );

	if( useBank )
		playergoldtotal = GetBankCount( DEREF_P_CHAR(pc_currchar), 0x0EED );
	else
		playergoldtotal = pc_currchar->CountGold();

	if ((playergoldtotal>=goldtotal)||(pc_currchar->isGM()))
	{
		for (i = 0; i < itemtotal; i++)
		{
			if (buyit[i] != NULL)
			{
				if (buyit[i]->amount < amount[i])
				{
					soldout = 1;
				}
			}
		}
		if (soldout)
		{
			npctalk(s, npc, "Alas, I no longer have all those goods in stock. Let me know if there is something else thou wouldst buy.",0);
			clear = 1;
		}
		else
		{
			if (pc_currchar->isGM())
			{
				sprintf((char*)temp, "Here you are, %s. Someone as special as thee will receive my wares for free of course.", pc_currchar->name);
			}
			else
			{
				if(useBank)
				{
					sprintf((char*)temp, "Here you are, %s. %d gold coin%s will be deducted from your bank account.  I thank thee for thy business.",
					pc_currchar->name, goldtotal, (goldtotal==1) ? "" : "s");
				    goldsfx(s, goldtotal);
				}
			    else
				{
				    sprintf((char*)temp, "Here you are, %s.  That will be %d gold coin%s.  I thank thee for thy business.",
					pc_currchar->name, goldtotal, (goldtotal==1) ? "" : "s");
				    goldsfx(s, goldtotal);	// Dupois, SFX for gold movement. Added Oct 08, 1998
				}
			}
			npctalkall(npc, (char*)temp,0);
			npcaction(npc,0x20);		// bow (Duke, 17.3.2001)

			clear = 1;
			if( !(pc_currchar->isGM() ) ) 
			{
				if( useBank )
					DeleBankItem( DEREF_P_CHAR(pc_currchar), 0x0EED, 0, goldtotal );
				else
					delequan( DEREF_P_CHAR(pc_currchar), 0x0EED, goldtotal, NULL );
			}
			for (i=0;i<itemtotal;i++)
			{
				P_ITEM pi = buyit[i];
				if (pi != NULL)
				{
					if (pi->amount>amount[i])
					{
						if (pi->pileable)
						{
							Commands->DupeItem(s, buyit[i], amount[i]);
						}
						else
						{
							for (j=0;j<amount[i];j++)
							{
								Commands->DupeItem(s, buyit[i], 1);
							}
						}
						pi->amount-=amount[i];
						pi->restock+=amount[i];
					}
					else
					{
						switch(layer[i])
						{
						case 0x1A:
							if (pi->pileable)
							{
								Commands->DupeItem(s, buyit[i], amount[i]);
							}
							else
							{
								for (j=0;j<amount[i];j++)
								{
									Commands->DupeItem(s, buyit[i], 1);
								}
							}
							pi->amount=pi->amount-amount[i];
							pi->restock=pi->restock+amount[i];
							break;
						case 0x1B:
							if (pi->pileable)
							{
								pi->SetContSerial(pi_pack->serial);
								RefreshItem(buyit[i]);//AntiChrist
							}
							else
							{
								for (j=0;j<amount[i]-1;j++)
								{
									Commands->DupeItem(s, buyit[i], 1);
								}
								pi->SetContSerial(pi_pack->serial);
								pi->amount = 1;
								RefreshItem(buyit[i]);//AntiChrist
							}
							break;
						default:
							clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, buyaction()\n"); //Morrolan
						}
					}
				}
			}
		}
	}
	else
	{
		npctalkall(npc, "Alas, thou dost not possess sufficient gold for this purchase!",0);
	}

	if (clear)
	{
		clearmsg[0]=0x3B;
		clearmsg[1]=0x00;
		clearmsg[2]=0x08;
		clearmsg[3]=buffer[s][3];
		clearmsg[4]=buffer[s][4];
		clearmsg[5]=buffer[s][5];
		clearmsg[6]=buffer[s][6];
		clearmsg[7]=0x00;
		Xsend(s, clearmsg, 8);
	}
	Weight->NewCalc(DEREF_P_CHAR(pc_currchar));	// Ison 2-20-99
	statwindow(s, DEREF_P_CHAR(pc_currchar));
}

void restock(int s)
{
	int a,serial;
	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); iter_items++)
	{
		P_ITEM pi = iter_items.GetData();
		// Dupois - added this check to only restock items that ... well ... have a restock value >0
		// Added Oct 25, 1998
		if (pi->restock && isItemSerial(pi->contserial))
		{
			serial = pi->contserial;
			if(serial > 0)
			{
				P_ITEM ci = FindItemBySerial(serial);
				if (ci != NULL)
				{
					if ((ci->layer==0x1A))
					{
						if (s)
						{
							pi->amount += pi->restock;
							pi->restock = 0;
						}
						else
						{
							if (pi->restock > 0)
							{
								a = min(pi->restock, (pi->restock/2)+1);
								pi->amount+=a;
								pi->restock-=a;
							}
						}
					}
				}
			}
		}
		// MAgius(CHE): All items in shopkeeper need a new randomvaluerate.
		if (SrvParms->trade_system==1) StoreItemRandomValue(pi,-1);// Magius(CHE) (2)
	}
}

// this is a q&d fix for 'sell price higher than buy price' bug (Duke, 30.3.2001)
static bool items_match(P_ITEM pi1, P_ITEM pi2)
{
	if (pi1->id()==pi2->id() &&
		pi1->type==pi2->type &&
		!(pi1->id()==0x14F0 && (pi1->morex!=pi2->morex)) &&			// house deeds only differ by morex
		!(IsShield(pi1->id()) && strcmp(pi1->name2,pi2->name2)) &&	// magic shields only differ by name2
		!(IsMetalArmour(pi1->id()) && pi1->color()!=pi2->color()) )	// color checking for armour
		return true;
	return false;
}
 
void sellaction(int s)
{
	int n, i, amt, value=0, totgold=0;

	P_ITEM pRestock = NULL;
	P_ITEM pNoRestock = NULL;
	P_ITEM pSellCont = NULL;

	if (buffer[s][8]!=0)
	{
		n=calcCharFromSer(buffer[s][3], buffer[s][4], buffer[s][5], buffer[s][6]);
		if (n<0 || n>=cmem) return;

		P_ITEM pi;
		unsigned int ci;
		vector<SERIAL> vecContainer = contsp.getData(chars[n].serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pi = FindItemBySerial(vecContainer[ci]);
			if (pi->layer==0x1A) pRestock = pi;				// Buy Restock container
			else if (pi->layer==0x1B) pNoRestock = pi;		// Buy no restock container
			else if (pi->layer==0x1C) pSellCont = pi;		// Sell container
		}

		// Pre Calculate Total Amount of selling items to STOPS if the items if greater than SELLMAXITEM - Magius(CHE)
		int maxsell=0;
		i=buffer[s][8];
		if (i>256) return;
		for (i=0;i<buffer[s][8];i++)
		{
			amt=ShortFromCharPtr(buffer[s]+9+(6*i)+4);
			maxsell+=amt;
		}
		if (maxsell>SrvParms->sellmaxitem)
		{
			char tmpmsg[256];
			sprintf(tmpmsg,"Sorry %s but i can buy only %i items at time!",chars[currchar[s]].name,SrvParms->sellmaxitem);
			npctalkall(n, tmpmsg,0);
			return;
		}

		for (i=0;i<buffer[s][8];i++)
		{
			P_ITEM pSell=FindItemBySerPtr(buffer[s]+9+(6*i));	// the item to sell
			if (!pSell) continue;
			amt=ShortFromCharPtr(buffer[s]+9+(6*i)+4);
			
			// player may have taken items out of his bp while the sell menu was up ;-)
			if (pSell->amount<amt)
			{
				npctalkall(n, "Cheating scum! Leave now, before I call the guards!",0);
				return;
			}

			// Search the buy restock Container
			P_ITEM join = NULL;
			ci=0;
			P_ITEM pi;
			vector<SERIAL> vecContainer = contsp.getData(pRestock->serial);
			for ( ci = 0; ci < vecContainer.size(); ci++)
			{
				pi = FindItemBySerial(vecContainer[ci]);
				if (items_match(pi,pSell))
					join = pi;
			}

			// Search the sell Container to determine the price
			ci=0;
			vecContainer.clear();
			vecContainer = contsp.getData(pSellCont->serial);
			for ( ci = 0; ci < vecContainer.size(); ci++)
			{
				pi = FindItemBySerial(vecContainer[ci]);
				if (items_match(pi,pSell))
				{
					value=pi->value;
					value=calcValue(pSell, value);
					if (SrvParms->trade_system==1)
						value=calcGoodValue(s, pSell, value, 1); // Fixed for adv trade --- by Magius(CHE) §
					break;	// let's take the first match
				}
			}
			totgold+=(amt*value);	// add to the bill

			if (join != NULL)	// The item goes to the container with restockable items
			{
				join->amount+=amt;
				join->restock-=amt;
				pSell->ReduceAmount(amt);
			}
			else
			{
				pSell->SetContSerial(pNoRestock->serial);
				SndRemoveitem(pSell->serial);
				if (pSell->amount!=amt)
					Commands->DupeItem(s, pSell, pSell->amount-amt);
			}
		}
		addgold(s, totgold);
		goldsfx(s, totgold);	// Dupois, SFX for gold movement	// Added Oct 08, 1998
	}

	char clearmsg[9];
	clearmsg[0]=0x3B;
	clearmsg[1]=0x00;
	clearmsg[2]=0x08;
	clearmsg[3]=buffer[s][3];
	clearmsg[4]=buffer[s][4];
	clearmsg[5]=buffer[s][5];
	clearmsg[6]=buffer[s][6];
	clearmsg[7]=0x00;
	Xsend(s, clearmsg, 8);
}

P_ITEM tradestart(int s, int i)
{
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	P_CHAR pc_i        = MAKE_CHAR_REF(i);
	unsigned char msg[90];

	P_ITEM pi_bps = Packitem(pc_currchar);
	P_ITEM pi_bpi = Packitem(pc_i);
	UOXSOCKET s2 = calcSocketFromChar(DEREF_P_CHAR(pc_i));

	if (pi_bps == NULL) //LB
	{
		sysmessage(s, "Time to buy a backpack!");
		sysmessage(s2, "%s doesnt have a backpack!", pc_currchar->name);
		return 0;
	}
	if (pi_bpi == NULL)
	{
		sysmessage(s2, "Time to buy a backpack!");
		sysmessage(s, "%s doesnt have a backpack!", pc_i->name);
		return 0;
	}

	P_ITEM pi_ps = Items->SpawnItem(s2, DEREF_P_CHAR(pc_currchar), 1, "#", 0, 0x1E, 0x5E, 0, 0, 0, 0);
	if(pi_ps == NULL) 
		return 0;
	pi_ps->pos = Coord_cl(26, 0, 0);
	pi_ps->SetContSerial(pc_currchar->serial);
	pi_ps->layer=0;
	pi_ps->type=1;
	pi_ps->dye=0;
	sendbpitem(s, pi_ps);
	if (s2 != INVALID_UOXSOCKET) 
		sendbpitem(s2, pi_ps);

	P_ITEM pi_pi = Items->SpawnItem(s2,i,1,"#",0,0x1E,0x5E,0,0,0,0);
	if (pi_pi == NULL) 
		return 0;
	pi_pi->pos = Coord_cl(26, 0, 0);
	pi_pi->SetContSerial(pc_i->serial);
	pi_pi->layer=0;
	pi_pi->type=1;
	pi_pi->dye=0;
	sendbpitem(s, pi_pi);
	if (s2 != INVALID_UOXSOCKET) 
		sendbpitem(s2, pi_pi);

	pi_pi->moreb1 = pi_ps->ser1;
	pi_pi->moreb2 = pi_ps->ser2;
	pi_pi->moreb3 = pi_ps->ser3;
	pi_pi->moreb4 = pi_ps->ser4;
	pi_ps->moreb1 = pi_pi->ser1;
	pi_ps->moreb2 = pi_pi->ser2;
	pi_ps->moreb3 = pi_pi->ser3;
	pi_ps->moreb4 = pi_pi->ser4;
	pi_ps->morez  = 0;
	pi_pi->morez  = 0;

	msg[0] = 0x6F; // Header Byte
	msg[1] = 0; // Size
	msg[2] = 47; // Size
	msg[3] = 0; // Initiate
	LongToCharPtr(pc_i->serial,msg+4);
	LongToCharPtr(pi_ps->serial,msg+8);
	LongToCharPtr(pi_pi->serial,msg+12);
	msg[16]=1;
	strcpy((char*)&(msg[17]), pc_i->name);
	Xsend(s, msg, 47);

	if (s2 != INVALID_UOXSOCKET)
	{
		msg[0]=0x6F; // Header Byte
		msg[1]=0;    // Size
		msg[2]=47;   // Size
		msg[3]=0;    // Initiate
		LongToCharPtr(pc_currchar->serial,msg+4);
		LongToCharPtr(pi_pi->serial,msg+8);
		LongToCharPtr(pi_ps->serial,msg+12);
		msg[16]=1;
		strcpy((char*)&(msg[17]), pc_currchar->name);
		Xsend(s2, msg, 47);
	}
	return pi_ps;
}

void clearalltrades()
{
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
	{
		P_ITEM pi = iterItems.GetData();
		if (pi->type==1 && pi->pos.x==26 && pi->pos.y==0 && pi->pos.z==0 &&
			pi->id()==0x1E5E)
		{
			P_CHAR pc = FindCharBySerial(pi->contserial);
			P_ITEM pBackpack = Packitem(pc);
			SERIAL serial = pi->serial;
			unsigned int ci;
			vector<SERIAL> vecContainer = contsp.getData(serial);
			for (ci = 0; ci < vecContainer.size(); ci++)
			{
				P_ITEM pj = FindItemBySerial(vecContainer[ci]);
				if (pj != NULL)
					if ((pj->contserial==serial))
					{
						if(pBackpack != NULL)
						{
							pBackpack->AddItem(pj);
						}
					}
			}
			Items->DeleItem(pi);
			clConsole.send("Trade cleared\n");
		}
	}
}

void trademsg(int s)
{
	P_ITEM cont1, cont2;
	switch(buffer[s][3])
	{
	case 0://Start trade - Never happens, sent out by the server only.
		break;
	case 2://Change check marks. Possibly conclude trade
		cont1 = FindItemBySerPtr(&buffer[s][4]);
		if (cont1 != NULL) 
			cont2 = FindItemBySerial(calcserial(cont1->moreb1, cont1->moreb2, cont1->moreb3, cont1->moreb4)); 
		else 
			cont2 = NULL;
		if (cont2 != NULL) // lb crashfix
		{
			cont1->morez=buffer[s][11];
			sendtradestatus(cont1, cont2);
			if (cont1->morez && cont2->morez)
			{
				dotrade(cont1, cont2);
				endtrade(buffer[s][4], buffer[s][5], buffer[s][6], buffer[s][7]);
			}
		}
		break;
	case 1://Cancel trade. Send each person cancel messages, move items.
		endtrade(buffer[s][4], buffer[s][5], buffer[s][6], buffer[s][7]);
		break;
	default:
		clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, trademsg()\n"); //Morrolan
	}
}

void dotrade(P_ITEM cont1, P_ITEM cont2)
{
	int serial,ci;

	P_CHAR p1 = FindCharBySerial(cont1->contserial);
	if(p1 == NULL) return;
	P_CHAR p2 = FindCharBySerial(cont2->contserial);
	if(p2 == NULL) return;
	if(cont1->morez==0 || cont2->morez==0)
	{//not checked - give items to previous owners - AntiChrist
		P_CHAR t;
		t  = p1;
		p1 = p2;
		p2 = t;
	}
	P_ITEM bp1 = Packitem(p1);
	if(bp1 == NULL) return;
	P_ITEM bp2 = Packitem(p2);
	if(bp2 == NULL) return;
	UOXSOCKET s1 = calcSocketFromChar(p1);
	UOXSOCKET s2 = calcSocketFromChar(p2);

	serial = cont1->serial;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for (ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if ((pi->contserial==serial))
			{
				if (pi->glow != INVALID_SERIAL) 
					glowsp.remove(p2->serial, pi->serial); // lb, glowing stuff
				bp2->AddItem(pi);
				if (pi->glow != INVALID_SERIAL) 
					glowsp.insert(p1->serial, pi->serial);
				if (s1!=-1)
					RefreshItem(pi);//AntiChrist
				if (s2!=-1) sendbpitem(s2, pi);
					RefreshItem(pi);//AntiChrist
			}
	}
	serial = cont2->serial;
	vecContainer.clear();
	vecContainer = contsp.getData(serial);
	for (ci=0;ci<vecContainer.size();ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
			if ((pi->contserial==serial))
			{
				if (pi->glow != INVALID_SERIAL) 
					glowsp.remove(p2->serial, pi->serial); // lb, glowing stuff
				bp1->AddItem(pi);
				if (pi->glow != INVALID_SERIAL) 
					glowsp.insert(chars[currchar[s1]].serial, pi->serial);
				if (s2 != INVALID_UOXSOCKET)
					RefreshItem(pi);//AntiChrist
				if (s1 != INVALID_UOXSOCKET) sendbpitem(s1, pi);
					RefreshItem(pi);//AntiChrist
			}
	}
}

