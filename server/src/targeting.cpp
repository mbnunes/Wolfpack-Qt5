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
#include "targeting.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "tilecache.h"
#include "wpscriptmanager.h"
#include "targetrequests.h"
#include "maps.h"
#include "classes.h"
#include "network/uosocket.h"
#include "gumps.h"
#include "network.h"
#include "skills.h"
#include "multis.h"
#include "weight.h"
#include "house.h"

#undef DBGFILE
#define DBGFILE "targeting.cpp"
//#include "debug.h"


void cTargets::PlVBuy(int s)//PlayerVendors
{
/*	if (s == -1) 
		return;
	int v = addx[s];
	P_CHAR pc = FindCharBySerial(v);
	if (pc->free) return;
	P_CHAR pc_currchar = currchar[s];

	P_ITEM pBackpack = pc_currchar->getBackpack();
	if (!pBackpack) {sysmessage(s,"Time to buy a backpack"); return; } //LB

	int serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi=FindItemBySerial(serial);		// the item
	if (pi==NULL) return;
	if (pi->isInWorld()) return;
	int price=pi->value;

	P_ITEM np = dynamic_cast<P_ITEM>(pi->container());		// the pack
	P_CHAR npc = np->getOutmostChar();				// the vendo
	if(npc != pc || pc->npcaitype() != 17) return;

	if( pc->owner() == pc_currchar )
	{
		pc->talk( tr("I work for you, you need not buy things from me!") );
		return;
	}

	int gleft = pc_currchar->CountGold();
	if (gleft<pi->value)
	{
		pc->talk( tr("You cannot afford that."), -1, 0 );
		return;
	}
	pBackpack->DeleteAmount(price,0x0EED);	// take gold from player

	pc->talk( tr("Thank you."), -1, 0 );
	pc->setHoldg(pc->holdg() + pi->value); // putting the gold to the vendor's "pocket"

	// sends item to the proud new owner's pack
	pBackpack->addItem(pi);
	pi->update();*/

}


static void KeyTarget(int s, P_ITEM pi) // new keytarget by Morollan
{
	/*if (pi)
	{
		if ((pi->more1==0)&&(pi->more2==0)&&
			(pi->more3==0)&&(pi->more4==0))
		{
			if ( pi->type() == 7 && (iteminrange(s,pi,2) || (!pi->isInWorld()) ) )
			{
				if (!Skills->CheckSkill(currchar[s], TINKERING, 400, 1000))
				{
					sysmessage(s,"You fail and destroy the key blank.");
					Items->DeleItem(pi);
				}
				else
				{
					pi->more1=addid1[s];
					pi->more2=addid2[s];
					pi->more3=addid3[s];
					pi->more4=addid4[s];
					sysmessage(s, "You copy the key."); //Morrolan can copy keys
				}
			}
			return;
		}//if
		else if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
			(pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
			(addid1[s]==(unsigned char)'\xFF'))
		{
			if (((pi->type()==1)||(pi->type()==63))&&(iteminrange(s,pi,2)))
			{
				if(pi->type()==1) pi->setType( 8 );
				if(pi->type()==63) pi->setType( 64 );
				sysmessage(s, "You lock the container.");
				return;
			}
			else if ((pi->type()==7)&&(iteminrange(s,pi,2)))
			{
				currchar[s]->inputitem = pi->serial;
				currchar[s]->inputmode = cChar::enDescription;
				sysmessage(s,"Enter new name for key.");//morrolan rename keys
				return;
			}
			else if ((pi->type()==8)||(pi->type()==64)&&(iteminrange(s,pi,2)))
			{
				if(pi->type()==8) pi->setType( 1 );
				if(pi->type()==64) pi->setType( 63 );
				sysmessage(s, "You unlock the container.");
				return;
			}
			else if ((pi->type()==12)&&(iteminrange(s,pi,2)))
			{
				pi->setType( 13 );
				sysmessage(s, "You lock the door.");
				return;
			}
			else if ((pi->type()==13)&&(iteminrange(s,pi,2)))
			{
				pi->setType( 12 );
				sysmessage(s, "You unlock the door.");
				return;
			}
			else if (pi->id()==0x0BD2)
			{
				sysmessage(s, "What do you wish the sign to say?");
				currchar[s]->inputitem = pi->serial; //Morrolan sign kludge
				currchar[s]->inputmode=cChar::enHouseSign;
				return;
			}

			//Boats ->
			else if( pi->type() == 117 && ( pi->type2() == 3 || pi->type2() == 2 ) )
			{
				cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial(pi->tags.get("boatserial").toUInt()));
				if( pBoat != NULL )
				{
					pBoat->switchPlankState( pi );
					pi->update();
				}
			}
			//End Boats --^
		}//else if
		else
		{
			if (pi->type()==7) sysmessage (s, "That key is not blank!");
			else if (pi->more1=='\x00') sysmessage(s, "That does not have a lock.");
			else sysmessage(s, "The key does not fit into that lock.");
			return;
		}//else
		return;
	}//if*/
}

void cTargets::IstatsTarget(int s)
{
}

// public !!!
P_ITEM cTargets::AddMenuTarget(int s, int x, int addmitem) //Tauriel 11-22-98 updated for new items
{
	if (s>=0)
		if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return NULL;

	P_ITEM pi = Items->createScriptItem(QString::number(addmitem));
	if (pi == NULL) return NULL;
	if (x)
		pi->update();
	return pi;
}

// public !!!
P_CHAR cTargets::NpcMenuTarget(int s)
{
/*	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return NULL;
	return cCharStuff::createScriptNpc(s, NULL, QString("%1").arg(addmitem[s]));*/
	return 0;
}

void cTargets::VisibleTarget (int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))//item
	{
		P_ITEM pi = FindItemBySerial(serial);
		if(pi != NULL)
		{
			pi->visible=addx[s];
			pi->update();
		}
	}
	else
	{//char
		P_CHAR pc = FindCharBySerial(serial);
		if(pc != NULL)
		{
			pc->setHidden( addx[s] );
			pc->update();
		}
	}
}


//public !!
void cTargets::Wiping(int s) // Clicking the corners of wiping calls this function - Crwth 01/11/1999
{
/*	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return;

	if (clickx[s]==-1 && clicky[s]==-1) {
		clickx[s]=(buffer[s][11]<<8)+buffer[s][12];
		clicky[s]=(buffer[s][13]<<8)+buffer[s][14];
//		if (addid1[s]) target(s,0,1,0,199,"Select second corner of inverse wiping box.");
//		else target(s,0,1,0,199,"Select second corner of wiping box.");
		return;
	}

	int x1=clickx[s],x2=(buffer[s][11]<<8)+buffer[s][12];
	int y1=clicky[s],y2=(buffer[s][13]<<8)+buffer[s][14];

	clickx[s]=-1;clicky[s]=-1;

	int c;
	if (x1>x2) {c=x1;x1=x2;x2=c;}
	if (y1>y2) {c=y1;y1=y2;y2=c;}
	if (addid1[s]==1)
	{ // addid1[s]==1 means to inverse wipe
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{
			P_ITEM pi=iterItems.GetData();
			if (!(pi->pos.x>=x1 && pi->pos.x<=x2 && pi->pos.y>=y1 && pi->pos.y<=y2) && pi->isInWorld() && !pi->wipe())
			{
				iterItems--;
				Items->DeleItem(pi);
			}
		}
	}
	else
	{
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{
			P_ITEM pi=iterItems.GetData();
			if (pi->pos.x>=x1 && pi->pos.x<=x2 && pi->pos.y>=y1 && pi->pos.y<=y2 && pi->isInWorld() && !pi->wipe())
			{
				iterItems--;
				Items->DeleItem(pi);
			}
		}
	}*/
}

void cTargets::AttackTarget(int s)
{
	P_CHAR target  = FindCharBySerial(addx[s]);
	P_CHAR target2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));
	if ( !target2 || !target ) 
		return;

    if (target->inGuardedArea()) // Ripper..No pet attacking in town.
	{
        sysmessage(s,"You cant have pets attack in town!");
        return;
	}
	target2->attackTarget( target );
}

void cTargets::FollowTarget(int s)
{
	// IS NOW STORED IN socket->tempInt(); !!!
    // LEGACY
	P_CHAR char1 = FindCharBySerial(addx[s]);
	P_CHAR char2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));

	if ( !(char1 && char2) ) // They were not found, could be bad formed packet.
		return;

	char1->setFtarg( char2->serial );
	char1->setNpcWander(1);
}

void cTargets::TransferTarget(int s)
{
/*	P_CHAR pc1 = FindCharBySerial(addx[s]);
	P_CHAR pc2 = FindCharBySerial(calcserial(buffer[s][7], buffer[s][8], buffer[s][9], buffer[s][10]));
	if ( !( pc1 && pc2 ) ) // They were not found, could be bad formed packet.
		return;

	pc1->emote( tr("%1 will now take %2 as his master.").arg(pc1->name.latin1()).arg(pc2->name.latin1()), -1 );

	if (pc1->ownserial() != -1) 
		pc1->SetOwnSerial(-1);
	pc1->SetOwnSerial(pc2->serial);
	pc1->setNpcWander(1);

	pc1->setFtarg(INVALID_SERIAL);
	pc1->setNpcWander(0);*/
}

void cTargets::BuyShopTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if ((pc->serial==serial))
		{
			Targ->BuyShop(s, pc);
			return;
		}
		sysmessage(s, "Target shopkeeper not found...");
	}
}

int cTargets::BuyShop( UOXSOCKET s, P_CHAR pc )
{
	/*P_ITEM pCont1=NULL, pCont2=NULL;

	if ( pc == NULL )
		return 0;

	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if( pi->layer() == 0x1A )
		{
			pCont1=pi;
		}
		else if( pi->layer() == 0x1B )
		{
			pCont2=pi;
		}
	}

	if (!pCont1 || !pCont2)
	{
		return 0;
	}

	impowncreate(s, pc, 0); // Send the NPC again to make sure info is current. (OSI does this we might not have to)
	sendshopinfo(s, pc, pCont1); // Send normal shop items
	sendshopinfo(s, pc, pCont2); // Send items sold to shop by players
	SndShopgumpopen(s,pc->serial);
	statwindow(s, currchar[s]); // Make sure the gold total has been sent.
	return 1;*/
	return 1;
}

// 
// 
// Aldur 
////////////////////////////////// 

// 
// 
// Aldur 
//////////////////////////////////

void cTargets::SetSpeechTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->isPlayer())
		{
			sysmessage(s,"You can only change speech for npcs.");
			return;
		}
		pc->setSpeech( addx[s] );
	}
}

static void SetSpAttackTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setSpAttack(tempint[s]);
	}
}

void cTargets::SetSpaDelayTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setSpaDelay( tempint[s] );
	}
}

void cTargets::SetPoisonTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		pc->setPoison(tempint[s]);
	}
}

////////////////
// name:		CanTrainTarget
// history:		by Antrhacks 1-3-99
// Purpose:		Used for training by NPC's
//
void cTargets::CanTrainTarget(int s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->isPlayer())
		{
			sysmessage(s, "Only NPC's may train.");
			return;
		}
		pc->setCantrain(!pc->cantrain());	//turn on if off, off if on
	}
}


////////////////
// name:		NpcResurrectTarget
// history:		by UnKnown (Touched tabstops by Tauriel Dec 28, 1998)
// Purpose:		Resurrects a character
//
bool cTargets::NpcResurrectTarget(P_CHAR pc)
{
	pc->resurrect();
	return true;
}

void cTargets::NewXTarget(int s) // Notice a high similarity to th function above? Wonder why. - Gandalf
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	if (isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.x = addx[s];
		pi->moveTo(pos);
		pi->update();
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.x = addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::NewYTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);
	if (isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.y = addx[s];
		pi->moveTo(pos);
		pi->update();
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.y = addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::IncXTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.x += addx[s];
		pi->moveTo(pos);
		pi->update();
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.x += addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::IncYTarget(int s)
{
	SERIAL serial = LongFromCharPtr(buffer[s]+7);

	if(isItemSerial(serial))
	{
		P_ITEM pi = FindItemBySerial(serial);
		if (pi == NULL)
			return;
		Coord_cl pos(pi->pos);
		pos.y += addx[s];
		pi->moveTo(pos);
		pi->update();
	}
	else if (isCharSerial(serial))
	{
		P_CHAR pc = FindCharBySerial(serial);
		if (pc == NULL)
			return;
		Coord_cl pos(pc->pos);
		pos.y += addx[s];
		pc->moveTo(pos);
		teleport(pc);
	}
}

void cTargets::HouseOwnerTarget(int s) // crackerjack 8/10/99 - change house owner
{
	int os, i;
	int o_serial = LongFromCharPtr(buffer[s]+7);
	if(o_serial==-1) return;
	P_CHAR pc = FindCharBySerial(o_serial);
	if ( pc == NULL ) return;

	SERIAL serial = calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	P_ITEM pSign = FindItemBySerial(serial);
	if ( pSign == NULL )
		return;

	serial = calcserial(pSign->more1(), pSign->more2(), pSign->more3(), pSign->more4());
	P_ITEM pHouse = FindItemBySerial(serial);
	if ( pHouse == NULL )
		return;
	
	if(pc->serial == currchar[s]->serial)
	{
		sysmessage(s, "you already own this house!");
		return;
	}
	
	pSign->SetOwnSerial(o_serial);
	
	pHouse->SetOwnSerial(o_serial);
	
	dynamic_cast< cHouse* >(pHouse)->removeKeys();
	
	os=-1;
	for(i=0;i<now && os==-1;i++)
	{
		if( currchar[i]->serial == pc->serial && perm[i]) os=i;
	}
	
	P_ITEM pi3=NULL;
	if(os!=-1)
	{
		pi3 = Items->SpawnItem(os, pc, 1, "a house key", 0, 0x10, 0x0F, 0, 1,1);//gold key for everything else
		if(pi3 == NULL) return;
	}
	else
	{
		pi3 = Items->SpawnItem(pc, 1, "a house key", 0, 0x100F,0,0);//gold key for everything else
		if(!pi3) return;
		pi3->moveTo(pc->pos);
		pi3->update();
	}
	pi3->setMore1( static_cast<unsigned char>((pHouse->serial&0xFF000000)>>24) );
	pi3->setMore2( static_cast<unsigned char>((pHouse->serial&0x00FF0000)>>16) );
	pi3->setMore3( static_cast<unsigned char>((pHouse->serial&0x0000FF00)>>8) );
	pi3->setMore4( static_cast<unsigned char>((pHouse->serial&0x000000FF)) );
	pi3->setType( 7 );
	
	sysmessage(s, "You have transferred your house to %s.", pc->name.latin1());
	sprintf((char*)temp, "%s has transferred a house to %s.", currchar[s]->name.latin1(), pc->name.latin1());

	int k;
	for(k=0;k<now;k++)
		if(k!=s && ( (perm[k] && inrange1p(currchar[k], currchar[s]) )||
			(currchar[k]->serial==o_serial)))
			sysmessage(k, (char*)temp);
}

void cTargets::HouseEjectTarget(int s) // crackerjack 8/11/99 - kick someone out of house
{
/*	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if (!pc)
		return;
	int serial=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	if(serial == INVALID_SERIAL) return;
	P_ITEM pHouse = FindItemBySerial(serial);
	if(pHouse != NULL)
	{
		int sx, sy, ex, ey;
		Map->MultiArea(pHouse, &sx,&sy,&ex,&ey);
		if(pc->serial == currchar[s]->serial)
		{
			sysmessage(s,"Do you really want to do that?!");
			return;
		}
		if(pc->pos.x>=sx&&pc->pos.y>=sy&&pc->pos.x<=ex&&pc->pos.y<=ey)
		{
			Coord_cl pos(pc->pos);
			pos.x = ex;
			pos.y = ey;
			pc->moveTo( pos );
			teleport(pc);
			sysmessage(s, "Player ejected.");
		}
		else
			sysmessage(s, "That is not inside the house.");
	}
*/
}

void cTargets::HouseBanTarget(int s) 
{
	Targ->HouseEjectTarget(s);	// first, eject the player

	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];
	if (!pc)
		return;
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_home->pos ) );
	if( pHouse )
	{
		if (pc->serial == pc_home->serial) return;
		pHouse->addBan(pc);
		sysmessage(s, "%s has been banned from this house.", pc->name.latin1());
	}
}

void cTargets::HouseFriendTarget(int s) // crackerjack 8/12/99 - add somebody to friends list
{
	P_CHAR Friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];

	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_home->pos ) );

	if(Friend && pHouse)
	{
		if(Friend->serial == pc_home->serial)
		{
			sysmessage(s,"You are already the owner!");
			return;
		}
		pHouse->addFriend(Friend);
		sysmessage(s, "%s has been made a Friend of the house.", Friend->name.latin1());
	}
}

void cTargets::HouseUnBanTarget(int s)
{
	P_CHAR pc_banned = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_owner->pos ) );

	if(pc_banned && pHouse)
	{
		if(pc_banned->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeBan(pc_banned);
		sysmessage(s,"%s has been UnBanned!",pc_banned->name.latin1());
	}
	return;
}

void cTargets::HouseUnFriendTarget(int s)
{
	P_CHAR pc_friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_owner->pos ) );

	if(pc_friend && pHouse)
	{
		if(pc_friend->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeFriend(pc_friend);
		sysmessage(s,"%s is no longer a Friend of this home!", pc_friend->name.latin1());
	}
	return;
}

void cTargets::HouseLockdown( UOXSOCKET s ) // Abaddon
// PRE:		S is the socket of a valid owner/coowner and is in a valid house
// POST:	either locks down the item, or puts a message to the owner saying he's a moron
// CODER:	Abaddon
// DATE:	17th December, 1999
{
	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);

	if( pi != NULL )
	{
		short id = pi->id();

		// not needed anymore, cause called from house_sped that already checks that ...

		// time to lock it down!

		if (Items->isFieldSpellItem ( pi ))
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if( IsAnvil( id ) )
		{
			sysmessage(s, "You cant lockdown anvils!");
			return;
		}
		if( IsForge( id ) )
		{
			sysmessage(s, "You cant lockdown forges!");
			return;
		}

		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi )
		{
			if(pi->isLockedDown())
			{
				sysmessage(s,"That item is already locked down, release it first!");
				return;
			}
			pi->setLockedDown();	// LOCKED DOWN!
			pi->setOwnSerialOnly(currchar[s]->serial);
			pi->update();
			return;
		}
		else
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}
}

void cTargets::HouseSecureDown( UOXSOCKET s ) // Ripper
// For locked down and secure chests
{
	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		// time to lock it down!
		if (Items->isFieldSpellItem ( pi ) )
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if( pi->isLockedDown() )
		{
			sysmessage(s,"That item is already locked down, release it first!");
			return;
		}

		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi && pi->type() == 1 )
		{
		    pi->setLockedDown();	// LOCKED DOWN!
			pi->setSecured( true );
			pi->setOwnSerialOnly(currchar[s]->serial);
			pi->update();
			return;
		}
		if( pi->type() != 1 )
		{
			sysmessage(s,"You can only secure chests!");
			return;
		}
		else
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}
}

void cTargets::HouseRelease( UOXSOCKET s ) // Abaddon & Ripper
// PRE:		S is the socket of a valid owner/coowner and is in a valid house, the item is locked down
// POST:	either releases the item from lockdown, or puts a message to the owner saying he's a moron
// CODER:	Abaddon
// DATE:	17th December, 1999
// update: 5-8-00
{
	int ser = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		if (pi->secured() && !pc_currchar->Owns(pi))
		{
			sysmessage(s,"You cannot do that!");
			return;
		}
		if (Items->isFieldSpellItem(pi))
		{
			sysmessage(s,"you cannot release this!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant release doors or signs!");
			return;
		}

		// time to unlock it!
		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi && pi->isLockedDown() || pi->type() == 1 )
		{
			pi->setAllMovable();	// Default as stored by the client, perhaps we should keep a backup?
			pi->setSecured( false );
			pi->update();
			return;
		}
		else if( pi_multi == NULL )
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}
}

void cTargets::SetMurderCount( int s )
{
	int serial=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc = FindCharBySerial(serial);
	if( pc != NULL )
	{
		pc->setKills( addmitem[s] );
		setcharflag(pc);
	}
}

void cTargets::FetchTarget(UOXSOCKET s) // Ripper
{
	sysmessage(s,"Fetch is not available at this time.");
}

void cTargets::GuardTarget( UOXSOCKET s )
{
	P_CHAR pPet = FindCharBySerial(addx[s]);
	if (pPet == NULL)
	{
		sysmessage(s, "Currently can't guard anyone but yourself.."); 
		LogError("Lost pet serial");
		return;
	}

	P_CHAR pToGuard = FindCharBySerPtr(buffer[s]+7);
	if( !pToGuard || pPet->owner() != pToGuard )
	{
		sysmessage( s, "Currently can't guard anyone but yourself!" );
		return;
	}
	pPet->setNpcAIType( 32 ); // 32 is guard mode
	pPet->setFtarg(currchar[s]->serial);
	pPet->setNpcWander(1);
	sysmessage(s, "Your pet is now guarding you.");
//	currchar[s]->setGuarded(true);
}

void cTargets::ResurrectionTarget( UOXSOCKET s )
{
	int serial=LongFromCharPtr(buffer[s]+7);
	if( serial == INVALID_SERIAL ) return;
	P_CHAR pc = FindCharBySerial(serial);
	if (pc != NULL)
	{
		if (pc->dead())
		{
			Targ->NpcResurrectTarget(pc);
			return;
		}
	}
}


void cTargets::LoadCannon(int s)
{
	/*int serial=LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(serial);
	if (pi != NULL)
	{
		//if((pi->id1==0x0E && pi->id2==0x91) && pi->morez==0)
		if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
			(pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
			(addid1[s]==(unsigned char)'\xFF'))
		{
			if ((pi->morez==0)&&(iteminrange(s, pi, 2)))
			{
				if(pi->morez==0)
				pi->setType( 15 );
				pi->morex=8;
				pi->morey=10;
				pi->morez=1;
				sysmessage(s, "You load the cannon.");
			}
			else
			{
				if (pi->more1=='\x00') sysmessage(s, "That doesn't work in cannon.");
				else sysmessage(s, "That object doesn't fit into cannon.");
			}
		}
	}*/
}

/*
void cTargets::AddItem( UOXSOCKET s )
{
	if( s == -1 )
		return;

	QString ItemID = xtext[ s ];
	
	const QDomElement *ItemNode = DefManager->getSection( WPDT_ITEM, ItemID );
	
	// No Item found...
	if( ItemNode->isNull() )
	{
		sysmessage( s, "There is no such item '%s'", xtext[ s ] );
		return;
	}

	// ...Otherwise get x+y+z coordinates first
	UI16 TargetX, TargetY;
	SI08 TargetZ;

	TargetX = ( buffer[s][11] << 8 ) + buffer[s][12];
	TargetY = ( buffer[s][13] << 8 ) + buffer[s][14];
	TargetZ = buffer[s][16] + TileCache::instance()->tileHeight( ( buffer[s][17] << 8) + buffer[s][18] );

	P_ITEM Item = Items->createScriptItem( ItemID );

	// No item created = fail
	if( Item == NULL )
	{
		sysmessage( s, "Unable to create item '%s'", xtext[ s ] );
		return;
	}

//	Item->setContSerial( -1 );
	Item->MoveTo( TargetX, TargetY, TargetZ );
	Item->update();
}
*/
