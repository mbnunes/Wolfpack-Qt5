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
#include "speech.h"
#include "worldmain.h"
#include "trigger.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"

// Library Includes
#include "qdatetime.h"

#undef  DBGFILE
#define DBGFILE "speech.cpp"

//bool InputSpeech(char* comm, cChar* pPlayer, UOXSOCKET s)
bool InputSpeech(string& comm, cChar* pPlayer, UOXSOCKET s)
{
	int i;

	if (pPlayer->inputmode != cChar::enNone)		// Speech is directly used to set a member
	{
		P_ITEM pTarget = FindItemBySerial(pPlayer->inputitem);
		switch (pPlayer->inputmode)
		{
		case cChar::enPricing:// Pricing an item //PlayerVendors
			i = str2num(comm);
			if (i>0)
			{
				pTarget->value = i;
				sysmessage(s, "This item's price has been set to %i.", i);
			}
			else 
			{
				sysmessage(s, "No price entered, ignored.");
			}
			pPlayer->inputmode = cChar::enDescription;
			sysmessage(s, "Enter a description for this item.");
			return true;
		case cChar::enDescription:// Describing an item
			pTarget->desc = comm;
			sysmessage(s, "This item is now described as %s, ", comm.c_str());
			pPlayer->inputmode = cChar::enNone;
			pPlayer->inputitem = INVALID_SERIAL;
			return true;
		case cChar::enRenameRune:
			pTarget->setName( QString( "Rune to: %1" ).arg( comm.c_str() ) );
			sysmessage(s, "Rune renamed to: Rune to %s", comm.c_str());
			pPlayer->inputmode = cChar::enNone;
			pPlayer->inputitem = INVALID_SERIAL;
			return true;
		case cChar::enNameDeed: // eagle rename deed
			{
				//char temp[50] = {0,};
				//strncpy(temp, comm, 49);
				//pPlayer->name = temp;
				pPlayer->name = comm ;
				sysmessage(s, "Your new name is: %s", comm.c_str());
				pPlayer->inputmode = cChar::enNone;
				pPlayer->inputitem = INVALID_SERIAL;
			}
			return true;
		case cChar::enHouseSign: // house sign rename
			{
				//char temp[50] = {0,};
				//strncpy(temp, comm, 49);
				//pTarget->name = temp;
				pTarget->setName( comm.c_str() );
				sysmessage(s, "Renamed to: %s", comm.c_str());
				pPlayer->inputmode = cChar::enNone;
				pPlayer->inputitem=INVALID_SERIAL;
			}
			return true;
		case cChar::enPageGM:
			{
				gmpages[pPlayer->playercallnum].reason = comm;
				sprintf(temp, "GM Page from %s [%x]: %s",pPlayer->name.c_str(), pPlayer->serial, comm.c_str());
				int x = 0;
				for (i = 0; i < now; i++)
				{
					if ((currchar[i]->isGM()) && perm[i])
					{
						x = 1;
						sysmessage(i, temp);
					}
				}
				if (x == 1)
					sysmessage(s, "Available Game Masters have been notified of your request.");
				else 
					sysmessage(s, "There was no Game Master available, page queued.");
				pPlayer->inputmode = cChar::enNone;
				return true;
			}
		case cChar::enPageCouns:
			{
				counspages[pPlayer->playercallnum].reason = comm;
				sprintf(temp, "Counselor Page from %s [%x]: %s", pPlayer->name.c_str(), pPlayer->serial, comm.c_str());
				int x = 0;
				for (i = 0; i < now; i++)
				{
					if (currchar[i]->isCounselor() && perm[i])
					{
						x = 1;
						sysmessage(i, (char*)temp);
					}	
				}
				if (x == 1)
					sysmessage(s, "Available Counselors have been notified of your request.");
				else	 
					sysmessage(s, "There was no Counselor available to take your call.");
				pPlayer->inputmode = cChar::enNone;
				return true;
			}
		default:
			break;	// do nothing
		}
	}
	return false;
}

bool StableSpeech(cChar* pMaster, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (pMaster->npc_type()!=1)	// is it a stablemaster ?
		return 0;

    //if (!strstr( comm, "STABLE"))	// lets check if the keyword stable is in the commandstring, if not return !
	if (comm.find("STABLE") == string::npos)
		return 0;
      
	/////////////////////////////////////////////////////////////////////
	//// so far we have a stablemaster! lets see if the caller has a pet
	//// if so check if the pets name is in the commandstring
	//// if not return
    ///////////////////////////////////////////////////////////////////
	
	bool found = false;
	P_CHAR p_pet = NULL;
	cRegion::RegionIterator4Chars ri(pPlayer->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		p_pet = ri.GetData();
		if (pPlayer->dist(p_pet) <= 8)
		{
			if (pPlayer->Owns(p_pet) && p_pet->stablemaster_serial==0) //owner of the pet ? and not already stabled ?
			{
				//char pntmp[150];
				//strcpy(pntmp, p_pet->name.c_str());
				//strupr(pntmp);
				string pntmp = p_pet->name ;
				transform(pntmp.begin(), pntmp.end(), pntmp.begin(), ::toupper);
				//if (strstr( comm, pntmp)) //if petname is in
				if (comm.find(pntmp)!= string::npos)
				{
					found=true;
					break;
				}
			}
		}
	}

	if (!found) 
	{ 
		npctalk(s,pMaster,"which pet?",0);
		return 1;
	} 

	/////////////////////////////////////////////////////////////
	/// now we have a pet and stablemaster -> time to STABLE :-)
    ////////////////////////////////////////////////////////////
	
    // set stablesp && pets stablemaster serial
	
	// remove it from screen!

	int xx=p_pet->pos.x;
	int yy=p_pet->pos.y;
	short id=p_pet->id(); 
	p_pet->setId(0); 
	p_pet->pos.x=0;
	p_pet->pos.y=0;

	for (int ch=0; ch<now; ch++)
	{	
		if (perm[ch]) impowncreate(ch, p_pet, 0); 
	}

	p_pet->setId(id); 
	p_pet->war=false;
	p_pet->attacker = INVALID_SERIAL;
	pPlayer->war=false;
	pPlayer->targ=INVALID_SERIAL;
	p_pet->pos.x=xx;
	p_pet->pos.y=yy;

	mapRegions->Remove(p_pet);

	p_pet->setStablemaster_serial(pMaster->serial);

	// set timer for fee calculation

    p_pet->setTime_unused(0);
	p_pet->setTimeused_last( getNormalizedTime() );

	stablesp.insert(pMaster->serial, p_pet->serial);

	sprintf(temp,"Your pet is now stabled, say retrieve or claim %s to claim your pet",p_pet->name.c_str());
	npctalk(s,pMaster,temp,0);

	return 1;
}

bool UnStableSpeech(cChar* pMaster, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (pMaster->npc_type()!=1)	// is it a stablemaster ?
		return 0;

//    if (!(strstr( comm, "CLAIM") || strstr( comm, "RETRIEVE")))	// lets check if the keyword CLAIM is in the commandstring, if not return !
	if ((comm.find("CLAIM") == string::npos) && (comm.find("RETRIEVE") == string::npos))
	
		return 0;

	/////////////////////////////////////////////////////////////////////
	//// so far we have a stablemaster! lets see if the stablemaster has a pet
	//// stabled the petowner owns
	//// if not return
    ///////////////////////////////////////////////////////////////////
	unsigned int ci;	
	P_CHAR  p_pet = NULL;
	bool found = false;
	vector<SERIAL> pets = stablesp.getData(pMaster->serial);
    for (ci = 0; ci < pets.size(); ci++)
	{
		p_pet = FindCharBySerial(pets[ci]);
		if (p_pet != NULL)
		{
			 if (pPlayer->Owns(p_pet) && p_pet->stablemaster_serial!=0) // already stabled and owned by claimer ?
			 {
			 	string search3 =p_pet->name ;
				 transform(search3.begin(), search3.end(), search3.begin(), ::toupper);
				//char search3[150];
				//strcpy(search3,p_pet->name.c_str());
		        //strupr(search3);
		        	//if (strstr( comm, search3)) //if petname is in
				if (comm.find(search3) != string::npos)
				{
					found=true;
					break;
				}
			}
		}
	}
	if (!found) 
	{ 
		npctalk(s,pMaster, "sorry, I can't return that pet",0);
		return 1;
	} 

	/////////////////////////////////////////////////////////////
	/// now we have the claimed pet and stablemaster -> time to UNSTABLE :-)
    ////////////////////////////////////////////////////////////

	/// calc fee
	float f_fee = ( (p_pet->time_unused()) / 600.0f) * SrvParams->stablingFee() ; // (fee per 10 minutes) * number of 10 minute blocks
	int fee = ( (int) f_fee) + 5; // 5 basefee

	sprintf((char*)temp,"That's %i gold pieces",fee);
	npctalk(s,pMaster,(char*)temp,0);

	/////////// check if customer can pay ! ///////////////    
	int gold = pPlayer->CountGold();
	if (gold<fee) // not enough gold in bp, dont try to subtract gold !
	{
		sprintf((char*)temp,"you can't afford that %i",fee);
		npctalk(s,pMaster,(char*)temp,0);
		return 1;
	}
	else
	{
		delequan( pPlayer, 0x0EED, fee, NULL );
	}
	
    //// if paid -> return pet
	
	// remove from hash table
	stablesp.remove(pMaster->serial, p_pet->serial);
	
	p_pet->setStablemaster_serial(INVALID_SERIAL); // actual unstabling
	
	p_pet->setTimeused_last(getNormalizedTime());
	p_pet->setTime_unused(0);
	
	mapRegions->Remove(p_pet);	
	mapRegions->Add( p_pet );
	
	for (int ch=0; ch<now; ch++)
	{	
		if (perm[ch]) impowncreate(ch, p_pet, 0); 
	}
	
	npctalk(s,pMaster,"Thx! Here's your pet",0);
	return 1;
}

bool ShieldSpeech(cChar* pGuard, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (pPlayer->dist(pGuard) > 3)	// lets be close to talk :)
		return false;
				
	if(pGuard->npcaitype == 6)	// chaos guard
	{
		//if (strstr( comm, "CHAOS SHIELD")) //Ripper...if in chaos guild get a new shield.
		if (comm.find("CHAOS SHIELD") != string::npos)
		{	// if they say chaos shield
			if(pPlayer->guildstone == INVALID_SERIAL)	// if not in a guild.
			{
				npctalk(s,pGuard,"You must be in a chaos guild to get a shield!",1);
			}
			else 
			{
				cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(pPlayer->guildstone));
				if ( pStone == NULL )
				{
					npctalk(s,pGuard,"You must be in a chaos guild to get a shield!",1);
					return true;
				}
				if ( pStone->guildType != cGuildStone::chaos )
				{
					npctalk(s, pGuard, "Sorry but you are not in a chaos guild!", 1);
					return true;
				}

				if(pPlayer->CountItems(0x1BC3)>0)					// if they have a chaos shield in their pack lets stop here.
				{
					npctalk(s,pGuard,"you already have a shield!",1);
				}
				else
				{
					cwmWorldState->RemoveItemsFromCharBody(currchar[s]->serial,0x1B, 0xC3);
					// if they are wearing a chaos shield lets just delete it.
					Items->SpawnItemBackpack2( s,28,1 );	// lets give them a new chaos shield.
					npctalk(s,pGuard,"Hi fellow guild member,here is your new shield.",1);
				}
			}
			return true;
		}
	}
	else if(pGuard->npcaitype == 7)	// order guard
	{
		//if (strstr( comm, "ORDER SHIELD")) //Ripper...if in order guild get a new shield.
			// if they say order shield
		if (comm.find("ORDER SHIELD") != string::npos)
		{
			if(pPlayer->guildstone == INVALID_SERIAL)	// if not in a guild.
			{
				npctalk(s,pGuard,"You must be in a order guild to get a shield!",1);
			}
			else
			{
				cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(pPlayer->guildstone));
				if ( pStone == NULL )
				{
					npctalk(s,pGuard,"You must be in a chaos guild to get a shield!",1);
					return true;
				}
				if ( pStone->guildType != cGuildStone::order )
				{
					npctalk(s, pGuard, "Sorry but you are not in an order guild!", 1);
					return true;
				}

				if(pPlayer->CountItems(0x1BC4)>0)					// if they have a chaos shield in their pack lets stop here.
				{
					npctalk(s,pGuard,"you already have a shield!",1);
				}
				else
				{
					cwmWorldState->RemoveItemsFromCharBody(currchar[s]->serial,0x1B, 0xC4);
					// if they are wearing an order shield lets just delete it.
					Items->SpawnItemBackpack2( s, 29, 1 );	// lets give them a new chaos shield.
					npctalk(s,pGuard,"Hi fellow guild member, here is your new shield.",1);
				}
			}
			return true;
		}
	}
	return false;
}

bool QuestionSpeech(cChar* pc, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (pc->npcaitype==2 || !pc->isHuman() || pPlayer->dist(pc) > 3)
		return 0;
	
    //if (strstr( comm, "NAME")) //Ripper...say name and a npc will tell you there name :).
	if (comm.find("NAME")!=string::npos)
	{
		sprintf(temp, "hello my name is %s.", pc->name.c_str());
		npctalkall(pc,temp,0);
		return 1;
	}
	
    //if (strstr( comm, "TIME")) //Ripper...say time and the npc gives the time.
	if (comm.find("TIME") != string::npos)
	{
		npctalkall(pc, (char*)tr("it is now %1").arg(uoTime.toString()).latin1(),0);
		return 1;
	}
	
    
    //if (strstr( comm, "LOCATION") || strstr( comm, "WHERE AM I")) //Ripper...gives location of char.
	if (comm.find("LOCATION") != string::npos)
	{
		if (strlen(region[pPlayer->region].name)>0)
			sprintf(temp, "You are in %s",region[pPlayer->region].name); 
		else strcpy(temp,"You are in the wilderness");
		npctalkall(pc,temp,0);
		
		sprintf(temp, "%i %i (%i)",pPlayer->pos.x,pPlayer->pos.y,pPlayer->pos.z); 
		npctalkall(pc,temp,0);
		return 1;
	}
	return 0;
}

bool TriggerSpeech(cChar* pc, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (abs(pPlayer->pos.x-pc->pos.x)<=4 &&
		abs(pPlayer->pos.y-pc->pos.y)<=4 &&
		abs(pPlayer->pos.z-pc->pos.z)<=5)
	{
		if (pc->trigger)
		{
			if (!pc->trigword.empty())
			{
				//char twtmp[150];
				//strcpy(twtmp, pc->trigword.c_str());
				//strupr(twtmp);
				string twtmp ;
				transform(pc->trigword.begin(), pc->trigword.end(), twtmp.begin(), ::toupper);
				//if (strstr( comm, twtmp))
				if (comm.find(twtmp) != string::npos)
				{
					if (pc->disabled>0 && pc->disabled>uiCurrentTime)//AntiChrist
					{
						npctalkall(pc,"I'm a little busy now! Leave me be!",0);
					}
					else
					{
						Trig->triggernpc(s, pc, 1);
					}
					return 1;
				}
			}
		}
	}
	return 0;
}

bool EscortSpeech(cChar* pEscortee, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	// Dupois - Added Dec 20, 1999
	// Escort text matches
	if (pPlayer->dist(pEscortee) > 1 || pEscortee->questType()!=ESCORTQUEST )
		return 0;	// not close enough / not an escortee
	
	//char *response1=strstr( comm, "I WILL TAKE THEE");
	//char *response2=strstr( comm, "DESTINATION");
	//if ( response1 || response2 )	// If either of the above responses match
	//{
		// I WILL TAKE THEE
		// If this is a request for hire
	//	if ( response1 )
		bool bpunt = false ;
		if (comm.find("I WILL TAKE THEE") != string::npos)
		{
			if ( pEscortee->ftarg == INVALID_SERIAL )
			{
				pEscortee->ftarg = currchar[s]->serial;		// Set the NPC to follow the PC
				pEscortee->npcWander = 1;			// Set the NPC to wander freely
				pEscortee->npcaitype = 0;           // Set AI to 0
				// Set the expire time if nobody excepts the quest
				pEscortee->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParams->escortactiveexpire() ) );
				// Send out the rant about accepting the escort
				sprintf(temp, "Lead on! Payment shall be made when we arrive at %s.", region[pEscortee->questDestRegion()].name);
				npctalkall(pEscortee,temp, 0);
				MsgBoardQuestEscortRemovePost( pEscortee );	// Remove post from message board
				return 1;	// Return 1 so that we indicate that we handled the message
			}
			else
			{
				//response2 = response1;	// If the current NPC already has an ftarg then respond to query for quest
				bpunt = true ;
			}
		}
		
		// DESTINATION
		// If this is a request to find out where a NPC wants to go and the PC is within range of the NPC and the NPC is waiting for an ESCORT
		//if (response2)
		if ((comm.find("DESTINATION") != string::npos) || bpunt)
		{
			if ( pEscortee->ftarg == currchar[s]->serial )
			{
				// Send out the rant about accepting the escort
				sprintf(temp, "Lead on to %s. I shall pay thee when we arrive.", region[pEscortee->questDestRegion()].name);
			}
			else if ( pEscortee->ftarg == INVALID_SERIAL )  // If nobody has been accepted for the quest yet
			{
				// Send out the rant about accepting the escort
				sprintf(temp, "I am seeking an escort to %s. Wilt thou take me there?", region[pEscortee->questDestRegion()].name);
			}
			else // The must be enroute
			{
				// Send out a message saying we are already being escorted
				P_CHAR pPlayer = FindCharBySerial(pEscortee->ftarg);
				sprintf(temp, "I am already being escorted to %s by %s.", region[pEscortee->questDestRegion()].name, pPlayer->name.c_str() );
			}
			npctalkall(pEscortee,temp, 0);
			return 1;	// Return success ( we handled the message )
		}
	//}
	return 0;	// speech was not handled
}

bool BankerSpeech(cChar* pBanker, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if( pBanker->npcaitype != 8 )	// not a banker
		return 0;
	if (pPlayer->dist(pBanker) > 12)
		return 0;
	//if (strstr(comm,"BANK") || strstr(comm,"BALANCE") || strstr(comm,"WITHDRAW") || strstr(comm,"CHECK"))
	if ((comm.find("BANK") != string::npos) || (comm.find("BALANCE")!=string::npos) | (comm.find("WITHDRAW")!=string::npos) || (comm.find("CHECK")!= string::npos))
	{
	    BankerAI->DoAI(s, pBanker, comm);
	    return 1;
	}
    return 0;	// speech was NOT handled
}

bool TrainerSpeech(cChar* pTrainer, string& comm, cChar* pPlayer, UOXSOCKET s) 
{
	if (pPlayer->dist(pTrainer) > 3 || !pTrainer->isHuman())
		return 0;
	//if (!(strstr(comm,"TRAIN") || strstr(comm,"TEACH") || strstr(comm,"LEARN"))) //if the player wants to train
	if ((comm.find("TRAIN") == string::npos) && (comm.find("TEACH") == string::npos) && (comm.find("LEARN") == string::npos))
		return 0;

	int i,skill=-1;
	pPlayer->trainer=-1; //this is to prevent errors when a player says "train <skill>" then don't pay the npc
	for(i=0;i<ALLSKILLS;i++)
	{
		
		//if(strstr(comm, skillname[i]))
		if (comm.find(skillname[i]) != string::npos)
		{
			skill=i;  //Leviathan fix
			break;
		}
	}
		
	if(skill==-1) // Didn't ask to be trained in a specific skill - Leviathan fix
	{
		if(pPlayer->trainer==-1) //not being trained, asking what skills they can train in
		{
			pTrainer->trainingplayerin='\xFF'; // Like above, this is to prevent  errors when a player says "train <skill>" then doesn't pay the npc
			strcpy(temp,"I can teach thee the following skills: ");
			int j,y = 0;
			for(j=0;j<ALLSKILLS;j++)
			{
				if(pTrainer->baseskill[j]>=10 && pPlayer->baseskill[j]<250)
				{
					sprintf(temp2,"%s, ", skillname[j]);
					strlwr(temp2);
					if(!y) temp2[0]=toupper(temp2[0]); // If it's the first skill,  capitalize it.
					strcat(temp,temp2);
					y++;
				}
			}
			if(y && pTrainer->cantrain) // skills and a trainer ?
			{
				temp[strlen(temp)-2]='.'; // Make last character a . not a ,  just to look nicer
				npctalk(s, pTrainer, temp,0);
			}
			else
			{
				npctalk(s, pTrainer, "I am sorry, but I have nothing to teach thee",0);
			}
			return 1;
		}
	}
	else // They do want to learn a specific skill
	{
		if(pTrainer->baseskill[skill]>10 && pTrainer->cantrain)
		{
			strcpy(temp2,skillname[skill]);
			strlwr(temp2);
			sprintf(temp,"Thou wishest to learn of %s?",temp2);
			if(pPlayer->baseskill[skill]>=250)
			{
				strcat(temp, " I can teach thee no more than thou already knowest!");
			}
			else
			{
				unsigned int sum = pPlayer->getSkillSum();
				if (sum >= SrvParams->skillcap() * 10)
					strcat(temp, " I can teach thee no more. Thou already knowest too much!");
				else
				{
					int delta = pTrainer->getTeachingDelta(pPlayer, skill, sum);
					int perc = (pPlayer->baseskill[skill] + delta)/10;
					
					sprintf(temp2, " Very well I, can train thee up to the level of %i percent for %i gold. Pay for less and I shall teach thee less.",perc,delta);
					strcat(temp, temp2);
					pPlayer->trainer=pTrainer->serial;
					pTrainer->trainingplayerin=skill;
				}
			}
			npctalk(s, pTrainer, temp,0);
		}
		else
		{
			npctalk(s, pTrainer, "I am sorry but I cannot train thee in that skill.",0);
		}
		return 1;
	}
	return 0;	// speech was NOT handled
}




bool PetCommand(cChar* pPet, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (!(pPlayer->Owns(pPet) || pPlayer->isGM())) //owner of the char || a GM
		return 0;
	if (pPet->npcaitype==17)	// player vendor
		return 0;
	if (pPlayer->dist(pPet) > 7)	// too far away to hear us
		return 0;
	
	//char petname[60];
	//strcpy(petname,pPet->name.c_str());
	//strupr(petname);
	string petname ;
	petname = pPet->name ;
	transform(petname.begin(), petname.end(), petname.begin(), ::toupper);
	bool bAllCommand = false;
	//if ( !strstr( comm, petname) )	//if petname is not in
	if (comm.find(petname) == string::npos)
		//if ( strstr ( comm, "ALL") )
		if (comm.find("ALL") != string::npos)
			bAllCommand = true;
		else
			return false;
	
	bool bReturn = false;
	
	//if (strstr( comm, " FOLLOW"))
	if (comm.find(" FOLLOW") != string::npos)
	{
		pPlayer->setGuarded(false);
		//if (strstr( comm, " ME"))	//if me is in
		if (comm.find(" ME") != string::npos)
		{
			pPet->ftarg = currchar[s]->serial;
			pPet->npcWander=1;
			playmonstersound(pPet, pPet->id(), SND_STARTATTACK);
		}
		else
		{
			addx[s] = pPet->serial;
			target(s, 0, 1, 0, 117, "Click on the target to follow.");
		}
		bReturn = true;
	}
	//else if (strstr( comm, " KILL") || strstr( comm, " ATTACK"))
	else if ((comm.find(" KILL") != string::npos) || (comm.find(" ATTACK") != string::npos))
	{
		if (pPet->inGuardedArea()) // Ripper..No pet attacking in town.
		{
			sysmessage(s,"You cant have pets attack in town!");
			return 0;
		}
		pPlayer->setGuarded(false);
		addx[s]=pPet->serial;
		target(s, 0, 1, 0, 118, "Select the target to attack.");//AntiChrist
		bReturn = true;
	}
	//else if (strstr( comm, " FETCH") || strstr( comm, " GET"))
	else if ((comm.find(" FETCH") != string::npos) || (comm.find(" GET") != string::npos))
	{
		pPlayer->setGuarded(false);
		addx[s]=pPet->serial;
		target(s, 0, 1, 0, 124, "Click on the object to fetch.");
		bReturn = true;
	}
	//else if (strstr( comm, " COME"))
	else if (comm.find(" COME") !=string::npos)
	{
		pPlayer->setGuarded(false);
		pPet->ftarg = pPlayer->serial;
		pPet->npcWander=1;
		pPet->setNextMoveTime();
		sysmessage(s, "Your pet begins following you.");
		bReturn = true;
	}
	//else if (strstr(comm," GUARD")) //if guard
	else if (comm.find(" GUARD") != string::npos)
	{
		addx[s] = pPet->serial;	// the pet's serial
		addy[s] = 0;
		//if (strstr( comm, " ME"))
		if (comm.find(" ME") != string::npos)
			addy[s]=1;	// indicates we already know whom to guard (for future use)
		// for now they still must click on themselves (Duke)
		target(s, 0, 1, 0, 120, "Click on the char to guard.");
		bReturn = true;
	}
	//else if (strstr( comm, " STOP")||strstr( comm, " STAY"))
	else if ((comm.find(" STOP") != string::npos) || (comm.find(" STAY") != string::npos))
	{
		pPlayer->setGuarded(false);
		pPet->ftarg = INVALID_SERIAL;
		pPet->targ = INVALID_SERIAL;
		if (pPet->war) 
			npcToggleCombat(pPet);
		pPet->npcWander=0;
		bReturn = true;
	}
	//else if (strstr( comm, " TRANSFER"))
	else if (comm.find(" TRANSFER") != string::npos)
	{
		pPlayer->setGuarded(false);
		addx[s]=pPet->serial;
		target(s, 0, 1, 0, 119, "Select character to transfer your pet to.");
		bReturn = true;
	}
	//else if (strstr( comm, " RELEASE"))
	else if (comm.find(" RELEASE") != string::npos)
	{
		pPlayer->setGuarded(false);
		if (pPet->summontimer)
		{
			pPet->summontimer=uiCurrentTime;
		}
		pPet->ftarg = INVALID_SERIAL;
		pPet->npcWander=2;
		pPet->SetOwnSerial(-1);
		pPet->setTamed(false);
		sprintf(temp, "*%s appears to have decided that it is better off without a master *", pPet->name.c_str());
		npctalkall(pPet,temp,0);
		{
			if(SrvParams->tamedDisappear()==1)
				soundeffect2(pPet, 0x01FE);
				Npcs->DeleteChar(pPet) ;
		}
		bReturn = true;
	}
	if ( bReturn && bAllCommand )
		return false; // give other pets opotunity to process command
	else
		return bReturn;
}

void PlVGetgold(int s, cChar* pVendor)//PlayerVendors
{
	unsigned int pay=0, give=pVendor->holdg(), t=0;
	P_CHAR pPlayer = currchar[s];
	
	if (pPlayer->Owns(pVendor))
	{
		if (pVendor->holdg()<1)
		{
			npctalk(s,pVendor,"I have no gold waiting for you.",0);
			pVendor->setHoldg(0);
			return;
		}
		else if(pVendor->holdg() <= 65535)
		{
			if (pVendor->holdg()>9)
			{
				pay=(int)(pVendor->holdg()*0.1);
				give-=pay;
			}
			else
			{
				pay=pVendor->holdg();
				give=0;
			}
			pVendor->setHoldg(0);
		}
		else
		{
			t=pVendor->holdg()-65535;
			pVendor->setHoldg(65535);
			pay=6554;
			give=58981;
		}
		if (give)
			Items->SpawnItem(s, currchar[s],give,"#",1,0x0E,0xED,0,1,1);
		sprintf((char*)temp, "Today's purchases total %i gold. I am keeping %i gold for my self. Here is the remaining %i gold. Have a nice day.",pVendor->holdg,pay,give);
		npctalk(s,pVendor,(char*)temp,0);
		pVendor->setHoldg(t);
	}
	else
		npctalk(s,pVendor,"I don't work for you!",0);
}

bool VendorChkName(cChar* pVendor, string& comm)
{
	if ((comm.find("VENDOR")!= string::npos) || (comm.find("SHOPKEEPER") != string::npos))
		return true ;
	else
	{
		string name = pVendor->name ;
		//Convert the name to upper case
		transform(name.begin(),name.end(),name.begin(),::toupper) ;
		// ok, now see if in the substring
		if (comm.find(name) != string::npos)
			return true ;
	}
	return false ;
}




bool PlayerVendorSpeech(cChar* pVendor, string& comm, cChar* pPlayer, UOXSOCKET s)
{

	if (!(pVendor->npcaitype == 17))
	     return 0;

	if (pPlayer->dist(pVendor) > 4)
		return 0;

	if (!VendorChkName(pVendor,comm))
		return false;

	//if (strstr(comm, " BROWSE") || strstr(comm, " VIEW") || strstr(comm, " LOOK"))
	if ((comm.find(" BROWSE") != string::npos) || (comm.find(" VIEW") != string::npos) || (comm.find(" LOOK") != string::npos))
	{
		npctalk(s,pVendor,"Take a look at my goods.",1);
		P_ITEM pi_backpack = Packitem(pVendor);
		if (pi_backpack != NULL)
		{
			backpack(s, pi_backpack->serial);
		    return true;
		}
	}
	//if (strstr(comm, " BUY") || strstr(comm, " PURCHASE"))
	if ((comm.find(" BUY") != string::npos) || (comm.find(" PURCHASE") != string::npos))
	{
		addx[s]=pVendor->serial;
		npctalk(s,pVendor,"What would you like to buy?",0);
		target(s,0,1,0,224," ");
		return true;
	}

	if (!pPlayer->Owns(pVendor))
			return 0;

	//if (strstr( comm, " COLLECT") || strstr( comm, " GOLD") || strstr( comm, " GET"))
	if ((comm.find(" COLLECT") != string::npos) || (comm.find(" GOLD") != string::npos) || (comm.find(" GET") != string::npos))
	{
		PlVGetgold(s, pVendor);
		return true;
	}
	//if (strstr( comm, "PACKUP"))
	if (comm.find("PACKUP") != string::npos)
	{
		P_ITEM pDeed = Items->SpawnItem(pPlayer, 1, "employment deed", 0, 0x14F0, 0, 1);
		if (pDeed)
		{
			pDeed->type = 217;
			pDeed->value = 2000;
			RefreshItem( pDeed );
			Npcs->DeleteChar( pVendor );
			sysmessage(s, "Packed up vendor %s.", pVendor->name.c_str());
			return true;
		}
	}
	return false;
}

bool VendorSpeech(cChar* pVendor, string& comm, cChar* pPlayer, UOXSOCKET s)
{
	if (pVendor->npcaitype == 17)
		return false;

	if (pPlayer->dist(pVendor) > 4)
		return false;

	if (!VendorChkName(pVendor, comm))
		return false;

    //if (strstr(comm, " BUY"))
	if (comm.find(" BUY") != string::npos)
	{
	    Targ->BuyShop(s, pVendor);
		return true;
	}
	//if (strstr( comm, " SELL"))
	if (comm.find(" SELL") != string::npos)
	{
		sellstuff(s, pVendor);						
		return true;
	}
	return false;
}


/////////////////
// name:	response
// purpose:	tries to get a response from an npc standing around
// history:	heavily revamped/rewritten by Duke, Oct 2001
// remark:	The new logic tries to minimize the # of strstr() calls by *first* checking
//			what kind of npcs are standing around and then checking only those keywords
//			that they might be interested in.
//			This is especially usefull in crowded places.

int cSpeech::response(UOXSOCKET s, P_CHAR pPlayer, string& SpeechUpr)
{
	string comm=SpeechUpr;

    //if (strstr( comm, "#EMPTY") && online(currchar[s]) && !pPlayer->dead && pPlayer->isGM())
	if ((comm.find("#EMPTY") != string::npos) && online(currchar[s]) && !pPlayer->dead && pPlayer->isGM())
	{ // restricted to GMs for now. It's too powerful (Duke, 5.6.2001)
		target(s, 0, 1, 0, 71, "Select container to empty:");
		return 1;
	}

    if (!online(pPlayer) || pPlayer->dead)
		return 0;

	P_CHAR pc;
	cRegion::RegionIterator4Chars ri(pPlayer->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		pc = ri.GetData();

		if (pc->isPlayer())		// only npcs will respond automagically, players still have to do that themselves ;)
			continue;

		if (pPlayer->dist(pc) > 16)	// at least they should be on the screen
			continue;

		if (pPlayer->isSameAs(pc))	// not talking to ourselves
			continue;
		
		if ( pc->onTalkToNPC( pPlayer, comm.c_str() ) )
			return 1;

		if (StableSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (UnStableSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (ShieldSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (QuestionSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (TriggerSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (EscortSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (BankerSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (TrainerSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (PetCommand(pc, comm, pPlayer, s))
			return 1;

		if (PlayerVendorSpeech(pc, comm, pPlayer, s))
			return 1;
		
		if (VendorSpeech(pc, comm, pPlayer, s))
			return 1;
	}
	
	return 0;
}



void cSpeech::talking(int s, const QString& speech) // PC speech
{
/*
	Unicode speech format
	byte = char, short = char[2], int = char[4], wchar = char[2] = unicode character

	Message Sent By Client:
	0xAD - Unicode Speech Request
	BYTE cmd(0xAD)
	short msgsize 1, 2
	byte type(0 = say, 2 = emote, 8 = whisper, 9 = yell) 3
	short color 4, 5
	short font 6, 7
	BYTE[4] lang(null terminated, "enu " for US english.) 8, 9, 10, 11
	wchar[?] text(null terminated, ?=(msgsize - 12)/2) 13
  
	Message Sent By Server:
	0xAE - Unicode Speech Message
	BYTE cmd(0xAE) 0
	short msgsize 1, 2
	BYTE[4] ser(ser of speaker, all 0xFF if none) 3, 4, 5, 6
	BYTE[2] model(id of speaker, all 0xFF if none)7, 8
	BYTE type 9
	short color 10, 11
	short font 12, 13
	BYTE[4] language(same as before) 14, 15, 16, 17
	BYTE[30] speaker's name(normal chars, not wchars) 18 - 48
	WCHAR[?] text(null terminated, ?=(msgsize - 48)/2

    Importnat note regarding 0xAD: since 2.0.7 clients send between lang and text 0...10 bytes. (we can ignore them safely)
	Those bytes get cut out in network.cpp correctly, so the buffer THIS functions sees is actualy what is written above.
    The actual data the client sends is differently though.
	Just noted this to prevent from debugging if somebody finds out client data doesn't fit to this description (LB) 

*/
	
	char nonuni[512];
	unsigned char talk2[19];
	char unicodetext[512];
	char lang[4];
	char name[50] = {0,};	// it **IS** important to 0 out the remaining gaps
	
	P_CHAR pc_currchar = currchar[s];	
	strcpy(nonuni, speech.latin1());

	// len+font+color+type = same postion for non unicode and unicode speech packets
	// but 8 ... x DIFFER a lot for unicode and non unicode packets !!!

	strncpy(name, pc_currchar->name.c_str(), 50);

	char speech_type       = buffer[s][3]; 
	UI16 speech_color	   = ShortFromCharPtr(&buffer[s][4]);
	char speech_fontbyte1  = buffer[s][6];
	char speech_fontbyte2  = buffer[s][7];

	int ucl = ( speech.length() * 2 ) + 2;
	int tl = ucl + 48 ;

	if (pc_currchar->unicode())
	{
		lang[0]=buffer[s][8];
		lang[1]=buffer[s][9];
		lang[2]=buffer[s][10];
		lang[3]=buffer[s][11];
		
		memcpy(unicodetext, &buffer[s][12], ucl);
	}
	else
	{
		lang[0]='E';
		lang[1]='N';
		lang[2]='U';
		lang[3]=0;
		
		char2wchar(nonuni);		// we are sending unicode response no matter if the speech request was non unicode or not
								// so convert to uni-text in case of non unicode
		memcpy(unicodetext, temp, ucl);  
	}

	/*
	clConsole.send("speech: %s\n",nonuni);
	clConsole.send("unicode speech:\n");
	for ( a=0; a < tl-48; a++) clConsole.send("%02i ",unicodetext[a]);
	clConsole.send("\n");*/

	// Call the scripted-test (special commands etc.)

	//// Very important: do not use buffer[s][] anymore in this function !!!!
	//// unicode text that gets send is in unicodetext, nonunicode text for normal string processing in non uni code
	string punt(nonuni);
	if (InputSpeech(punt, pc_currchar, s))	// handle things like renaming or describing an item
		return;

	if (pc_currchar->squelched)					// not allowed to talk
	{
		sysmessage(s, "You have been squelched.");
		return;
	}
			
	// AntiChrist
	pc_currchar->unhide();
		
	if (nonuni[0] == SrvParams->commandPrefix() )
	{
		Commands->Command(s, speech.latin1());
		return;
	}

	if ( speech_type == '\x09' && pc_currchar->canBroadcast() )
	{
		broadcast(s);
		return;
	}
	
	talk2[0] = 0xAE;
	ShortToCharPtr(tl, &talk2[1]);
	LongToCharPtr(pc_currchar->serial, &talk2[3]);
	ShortToCharPtr(pc_currchar->id(), &talk2[7]);
	talk2[9] =  speech_type;
	ShortToCharPtr(speech_color, &talk2[10]);
	talk2[12] = speech_fontbyte1;
	talk2[13] = speech_fontbyte2;
	
	talk2[14] = lang[0];
	talk2[15] = lang[1];
	talk2[16] = lang[2];
	talk2[17] = lang[3];
	
	Xsend(s, talk2, 18);
	Xsend(s, name, 30);   
	Xsend(s, unicodetext, ucl);   
	
	if (speech_type == 0 || speech_type == 2)
	{
		pc_currchar->saycolor = speech_color;
	}
	if (SrvParams->speechLog()) // Logging bugfixed by LB
	{
		char temp2[512];
		sprintf(temp2, "%s.speech_log", pc_currchar->name.c_str());
		sprintf((char*)temp, "%s [%x] [%i] said:\n%s\n", pc_currchar->name.c_str(), pc_currchar->serial, pc_currchar->account, nonuni);
		savelog((char*)temp, (char*)temp2);
	}
	
	//char SpeechUpr[512];
	//strcpy(SpeechUpr, nonuni);
	//strupr(SpeechUpr);
	string SpeechUpr(nonuni) ;
	transform(SpeechUpr.begin(), SpeechUpr.end(), SpeechUpr.begin(), ::toupper);
	//if (!strcmp(SpeechUpr, "I RESIGN FROM MY GUILD"))
	if (SpeechUpr == string("I RESIGN FROM MY GUILD"))
	{
		GuildResign(s);
	}
	
	if (response(s,pc_currchar,SpeechUpr))
		return;  // Vendor responded already
	
	//if (strstr(SpeechUpr, "GUARDS"))
	if (SpeechUpr.find("GUARDS") != string::npos)
		callguards(currchar[s]);
	
	if (Boats->Speech(s, SpeechUpr))
		return;
	
	house_speech(s, SpeechUpr); // houses crackerjack 8/12/99			
	
	int i, j;
	for (i = 0; i < now; i++)
	{
		// AntiChrist - don't check line of sight for talking!!!
		if (inrange1(i, s) && perm[i] && i!=s)//&&line_of_sight(s, pc_currchar->pos.x, pc_currchar->pos.y, pc_currchar->pos.z, chars[currchar[i]].x, chars[currchar[i]].y, chars[currchar[i]].z, WALLS_CHIMNEYS + DOORS + FLOORS_FLAT_ROOFING))
		{
			Xsend(i, talk2, 18);
			Xsend(i, name, 30);
			if (pc_currchar->dead				// a ghost is talking
				&& !currchar[i]->dead		// Ghost can talk normally to other ghosts
				&& !currchar[i]->isGMorCounselor()// GM/Counselors can see ghosts talking always  Seers?
				&& currchar[i]->spiritspeaktimer == 0)
			{
				unsigned char ghostspeech[512];
				memcpy(&ghostspeech, &unicodetext, ucl);
				for (j = 1; j < ucl-2 ; j += 2)	// -2: dont override /0 /0 terminator !
				{
					if (ghostspeech[j] != 32)	// keep the blanks
						ghostspeech[j] = (ghostspeech[j]%2) ? 'O' : 'o';
				}
				Xsend(i, ghostspeech, ucl);		// send 'ghostified' speech "OOoooOo  Ooo"
			}
			else
				Xsend(i, unicodetext, ucl);   
		}
	}
	
	if (pc_currchar->dead) return; // this makes it so npcs do not respond to dead people
	
	cChar* pc=NULL;
	cChar* pNpc=NULL;
	cRegion::RegionIterator4Chars ri(pc_currchar->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{	
		pc = ri.GetData();
		if (!pc->isSameAs(pc_currchar) 
			&& pc->isNpc()
			&& pc->dist(pc_currchar) <= 2)
		{
			pNpc=pc;
			break;
		}
	}
	if (pNpc && pNpc->speech)
	{
		Script *pScp=i_scripts[speech_script];
		if (!pScp->Open())
			return;
		char sect[512];
		sprintf(sect, "SPEECH %i", pNpc->speech);
		if (!pScp->find(sect)) 
		{
			pScp->Close();
			return;
		}
		int match = 0;
		strcpy(sect, "NO DEFAULT TEXT DEFINED");
		unsigned long loopexit = 0;
		do
		{
			pScp->NextLineSplitted();
			if (script1[0] != '}')
			{
				if (!(strcmp("DEFAULT", (char*)script1)))
				{
					strcpy(sect, (char*)script2);
				}
				if (!(strcmp("ON", (char*)script1)))
				{
					char scpUpr[500];
					strcpy(scpUpr,script2);
					strupr(scpUpr);
					if (SpeechUpr.find(scpUpr)!= string::npos)
						match=1;
				}
				if (!(strcmp("SAY", (char*)script1)))
				{
					if (match == 1)
					{
						npctalk(s, pNpc, (char*)script2, 0);
						match = 2;
					}
				}
				
				if (!(strcmp("TRG", (char*)script1))) // Added by Magius(CHE) §
				{							  
					if (match == 1)
					{
						pNpc->trigger = str2num(script2);
						scpMark m=pScp->Suspend();
						
						Trig->triggernpc(s, pNpc, 1);
						
						pScp->Resume(m);
						strcpy((char*)script1, "DUMMY");
						
						match = 2;
					}
				}
			}
		}
		while (script1[0] != '}'  && (++loopexit < MAXLOOPS));
		if (match == 0)
		{
			npctalk(s, pNpc, sect, 0);
		}
		pScp->Close();
	}
}

void cSpeech::wchar2char (const char* str)
{
	memset(&temp[0], 0, 1024);
	bool end = false;
	for (int i = 0; !end && i<1022 ; i++)
	{
		if (str[i] == 0 && str[i+1] == 0) end = true; // bugfix LB ... was str[i-1] not so good for i=0
		temp[i] = str[i*2];
	}
}



void cSpeech::char2wchar (const char* str)
{
	memset(&temp[0], 0, 1024);
	unsigned int size = strlen(str);
	
	// client wants to have a 0 as very fist byte.
	// after that 0 the unicode text
	// after it two(!) 0's as termintor

	unsigned int j=1;
	// temp[0]=0; //redundant, plz leave as comment

	for (unsigned int i = 0; i < size; i++) // bugfix LB ... temp[i+1] = str[i] .... wrong
	{
		temp[j]   = str[i];
		// temp[j+1] = 0; // redundant line, plz leave as comment
		j+=2;
	}

	// basicly redundant as well, plz leave as comment
	// temp[j]=0;
	// temp[j+1]=0;
}

