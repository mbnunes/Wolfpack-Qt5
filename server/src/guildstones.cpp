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

// Guildstones.cpp
//
// Author		:	DasRaetsel
// EMail		:	sascha@toxic.franken.de
// Web			:	http://uox3.home.pages.de/
// Last Edited	:	30. May 1999

// Last touch 9'th November 2000 by LB
// removed all cases of guild[-1]
// and loads of stupid other things/bugs

#include "guildstones.h"
#include "globals.h"
#include "SndPkg.h"
#include "debug.h"
#include "wolfpack.h"
#include "iserialization.h"
#include "worldmain.h"
#include "gumps.h"
#include "network.h"
#include "multis.h"

#undef  DBGFILE
#define DBGFILE "guildstones.cpp"

// placeguildstone() : spawns a renamed gravestone at players location and removes deed
// Placer gets guildmaster, whatever he does ;)
// Force placer to give that damn guild a damn name :)
void StonePlacement(UOXSOCKET s)
{
	P_CHAR pc = currchar[s];
	P_ITEM pDeed = FindItemBySerial(pc->fx1());
	cGuildStone* pStone = NULL;

	if (CheckValidPlace(s)!=1)
	{
		sysmessage(s, "You cannot place guildstones at any other location than your house");
		return;
	}
	
	if (pDeed->id() == 0x14F0)
	{
		if (pc->guildstone() != INVALID_SERIAL)
		{
			itemmessage(s,"You are already in a guild.",pDeed->serial);
			return;
		}
		pStone = new cGuildStone;		
		if (!pStone)
		{//AntiChrist - to prevent crashes
			sysmessage(s, "Cannot create guildstone");
			return;
		}
		pStone->Init();
		pStone->setId(0x0ED5);
		pStone->setName( "Guildstone for an unnamed guild" );
		Items->GetScriptItemSetting(pStone);
		pc->setGuildstone( pStone->serial );
		if (pc->id() == 0x0191)	
			pc->setGuildtitle("Guildmistress");
		else
			pc->setGuildtitle("Guildmaster");

		pStone->webpage = DEFAULTWEBPAGE;
		pStone->charter = DEFAULTCHARTER;
		pStone->addMember( pc );
		pStone->guildType = cGuildStone::standard;
		pStone->moveTo(pc->pos);
		pStone->setType( 202 );
		pStone->priv = 0;
		pStone->setLockedDown();
		pStone->setOwnSerialOnly(pc->serial);

		pStone->update();//AntiChrist
		Items->DeleItem(pDeed);
		entrygump(s, pc->serial,100,1,40,"Enter a name for the guild.");
	}
	else
	{
/*		guildnumber = SearchByStone(s);
		if (guildnumber==-1)
		{//AntiChrist
			sysmessage(s,"There are already enough guildstones placed.");
			return;
		}
		if (( pDeed->serial==guilds[guildnumber].stone &&
			pc->serial == guilds[guildnumber].master) ||
			pc->isGM() )
		{
			sprintf(stonename, "Guildstone for %s", guilds[guildnumber].name);
			pStone = Items->SpawnItem(currchar[s], 1, stonename, 0, 0x0ED5, 0, 0);
			if (!pStone)
			{
				sysmessage(s,"Cannot create guildstone");
				return;
			}
			pStone->MoveTo(pc->pos.x,pc->pos.y,pc->pos.z);
			pStone->type = 202;
			pStone->priv = 0;		
			pStone->update();//AntiChrist
			Items->DeleItem(pDeed);
			pc->fx1 = INVALID_SERIAL;
			guilds[guildnumber].stone = pStone->serial;
		}
		else
			itemmessage(s,"You are not the guildmaster of this guild. Only the guildmaster may use this guildstone teleporter.",pDeed->serial);
*/	}
}

void cGuildStone::addMember(P_CHAR pc)
{
	member.push_back(pc->serial);
	sort(member.begin(), member.end());
}

bool cGuildStone::isMember(P_CHAR pc)
{
	return binary_search(member.begin(), member.end(), pc->serial);
}

// guildstonemenu() : Opens the guild menu for a player
// Recognizes Guildmaster with the owner fields of the stone.
// Ofcourse checks for membership before opening any gump ;)
void cGuildStone::Menu(UOXSOCKET s, int page)
{
	//int total,i, counter,guild,recruit,war,member;
	int counter = 1;
	int lentext;
	int gumpnum = 0;
	char guildfealty[60],guildt[16],toggle[6];
	static char mygump[MAXMEMRECWAR][257];

	P_CHAR pc = currchar[s];

	if (!isMember( pc ) && !pc->isGM())
	{
		sysmessage(s, "You are not a member of this guild. Ask an existing guildmember to invite you into this guild.");
		return;
	}

	strcpy(guildfealty, "yourself");
	if ((pc->guildfealty() != pc->serial)&&(pc->guildfealty() != INVALID_SERIAL))
	{
		vector<SERIAL>::iterator it = find(member.begin(), member.end(), pc->guildfealty());
		if ( it != member.end())
		{
			strcpy(guildfealty, FindCharBySerial(*it)->name.c_str());
		}
	}
	else 
		pc->setGuildfealty( pc->serial );
	if (this->ownserial == INVALID_SERIAL) 
		CalcMaster();
	
	P_CHAR guildmaster = FindCharBySerial(this->ownserial);

	switch (this->guildType) 
	{
		case standard:		strcpy(guildt, " Standard");	break;
		case order:			strcpy(guildt, "n Order");		break;
		case chaos:			strcpy(guildt, " Chaos");		break;
	}
	
	if (pc->guildtoggle()) 
		strcpy(toggle, "On");
	else
		strcpy(toggle, "Off");

	// our prefix is 0xFE (darkstorm)
	gmprefix[7] = 0xFE;

	switch(page)
	{
	case 1:
		gumpnum=9;
		gmprefix[8] = 1;

		if (guildmaster<0) return;

		lentext = sprintf(mygump[0], "%s (%s %s)", this->guildName.c_str(), guildmaster->guildtitle().latin1(), guildmaster->name.c_str());
		strcpy(mygump[1],"Recruit someone into the guild.");
		strcpy(mygump[2],"View the current roster.");
		strcpy(mygump[3],"View the guild's charter.");
		sprintf(mygump[4],"Declare your fealty. You are currently loyal to %s.",guildfealty);
		sprintf(mygump[5],"Toggle showing the guild's abbreviation in your name to unguilded people. Currently %s.",toggle);
		strcpy(mygump[6],"Resign from the guild.");
		strcpy(mygump[7],"View list of candidates who have been sponsored to the guild.");
	    if ((pc->serial == this->ownserial)|| (pc->isGM()))							// Guildmaster Access?
		{															
			gumpnum=10;
			gmprefix[8] = 0;
			sprintf(mygump[8],"Access %s functions.", guildmaster->guildtitle().latin1());
			sprintf(mygump[9],"View list of guild that %s has declared war on.", this->guildName.c_str());
			sprintf(mygump[10],"View list of guilds that have declared war on %s.",this->guildName.c_str());
		} else {													// Normal Members access!
			sprintf(mygump[8],"View list of guilds that %s have declared war on.",this->guildName.c_str());
			sprintf(mygump[9],"View list of guilds that have declared war on %s.",this->guildName.c_str());
		}
		break;
	case 2:														// guildmaster menu
		gumpnum=14;
		lentext = sprintf(mygump[0], "%s, %s functions", this->guildName.c_str(), guildmaster->guildtitle().latin1());
		strcpy(mygump[1], "Set the guild name.");
		strcpy(mygump[2], "Set the guild's abbreviation.");
		sprintf(mygump[3], "Change the type of the guild. (Currently a%s guild.)",guildt);
		strcpy(mygump[4], "Set the guild's charter.");
		strcpy(mygump[5], "Dismiss a member.");
		strcpy(mygump[6], "Declare war from menu.");
		strcpy(mygump[7], "Declare war from targeting enemy.");
		strcpy(mygump[8], "Declare peace.");
		strcpy(mygump[9], "Accept a candidate seeking membership.");
		strcpy(mygump[10],"Refuse a candidate seeking membership.");
		strcpy(mygump[11],"Set the guildmaster's title.");
		strcpy(mygump[12],"Grant a title to another member.");
		strcpy(mygump[13],"Move this guildstone.");
		strcpy(mygump[14],"Return to the main menu.");

		gmprefix[8] = 2;
		break;
	case 3:														// guild type
		gumpnum=4;
		lentext=sprintf(mygump[0], "Please select the type you want your guild to be related to.");
		strcpy(mygump[1], "Select this to return to main menu.");
		strcpy(mygump[2], "Set to Standard.");
		strcpy(mygump[3], "Set to Order.");
		strcpy(mygump[4], "Set to Chaos.");
		gmprefix[8] = 3;
		break;
	case 4:														// edit charter
		gumpnum=3;
		lentext=sprintf(mygump[0], "Set %s charter.", this->guildName.c_str());
		strcpy(mygump[1], "Select this to return to main menu.");
		strcpy(mygump[2], "Set the charter.");
		strcpy(mygump[3], "Set the webpage.");
		gmprefix[8] = 4;
		break;
	case 5:														// view charter
		gumpnum=2;
		lentext=sprintf(mygump[0], "%s charter.", this->guildName.c_str());
		sprintf(mygump[1], "%s. Select this to return to the main menu.", this->charter.c_str());
		sprintf(mygump[2], "Visit the guild website at %s", this->webpage.c_str());
		gmprefix[8] = 5;
		break;
	case 6:
	{														// Candidates list
		gumpnum = this->recruit.size();
		lentext=sprintf(mygump[0], "%s list of candidates.", this->guildName.c_str());
		sprintf(mygump[1], "Select this to return to the menu.");
		unsigned int i;
		for (i = 0; i < recruit.size(); ++i)
		{
			strcpy(mygump[counter++], FindCharBySerial(this->recruit[i])->name.c_str());
		}
		gmprefix[8] = 6;
		break;
	}
	case 7:
	{														// roster
		gumpnum = this->member.size();
		lentext=sprintf(mygump[0], "%s members roster.", this->guildName.c_str());
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		unsigned int i;
		for (i = 0; i < member.size(); ++i)
		{
			counter++;
			strcpy(mygump[counter], FindCharBySerial(this->member[i])->name.c_str());
		}
		gmprefix[8] = 7;
		break;
	}
	case 8:
	{													// member dismiss
		gumpnum = this->member.size();
		lentext=sprintf(mygump[0], "Dismiss what member?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		unsigned int i;
		for ( i = 0; i < member.size(); ++i)
		{
			counter++;
			strcpy(mygump[counter], FindCharBySerial(this->member[i])->name.c_str());
		}
		gmprefix[8] = 8;
		break;
	}
	case 9:	
	{													// Refuse Candidates
		gumpnum = this->recruit.size();
		lentext=sprintf(mygump[0], "Refuse what candidate? %s", this->guildName.c_str());
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		unsigned int i;
		for ( i = 0; i < recruit.size(); ++i)
		{
			counter++;
			strcpy(mygump[counter], FindCharBySerial(this->recruit[i])->name.c_str());
		}
		gmprefix[8] = 9;
		break;
	}
	case 10:														// Accept Candidates
	{
		gumpnum = this->recruit.size();
		lentext=sprintf(mygump[0], "Accept what candidate?.%s", this->guildName.c_str());
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		unsigned int i;
		for ( i = 0; i < recruit.size(); ++i)
		{
			counter++;
			strcpy(mygump[counter], FindCharBySerial(this->recruit[i])->name.c_str());
		}
		gmprefix[8] = 10;
		break;
	}
	case 11:														// War list
	{
		gumpnum = this->war.size();
		lentext=sprintf(mygump[0], "Guild that %s has declared war on.", this->guildName.c_str());
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		unsigned int i;
		for ( i = 0; i < war.size(); ++i )
		{
			counter++;
			cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(this->war[i]));
			strcpy(mygump[counter], pStone->guildName.c_str());
		}
		gmprefix[8] = 11;
		break;
	}
	case 12:
	{														// grant title
		gumpnum = this->member.size();
		lentext=sprintf(mygump[0], "Grant a title to whom?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter = 1;
		unsigned int i;
		for (i = 0; i < this->member.size(); ++i)
		{
			counter++;
			strcpy(mygump[counter], FindCharBySerial(this->member[i])->name.c_str());
		}
		gmprefix[8] = 12;
		break;
	}
	case 13:	
	{													// fealty
		gumpnum = this->member.size();
		lentext = sprintf(mygump[0], "Whom do you wish to be loyal to?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		unsigned int i;
		for (i = 0; i < this->member.size(); ++i)
		{
			counter++;
			strcpy(mygump[counter], FindCharBySerial(this->member[i])->name.c_str());
		}
		
		gmprefix[8] = 13;
		break;
	}
	case 14:														// declare War list
	{
		gumpnum=1;
		lentext=sprintf(mygump[0], "What guilds do you with to declare war?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		list<SERIAL>::iterator it;
		for (it = guilds.begin(); it != guilds.end(); ++it)
		{
			if ( !this->free && this->serial != *it )
			{
				unsigned int i;
				for (i = 0; i < this->war.size(); ++i)
				{
					if (this->war[i] == *it) 
					{
						++gumpnum;
						P_ITEM pStone = FindItemBySerial(*it);
						strcpy(mygump[++counter], pStone->name().ascii());
					}
				}
			}
		}

		gmprefix[8] = 14;
		break;
	}
	case 15:														// declare peace list
	{
		gumpnum = this->war.size();
		lentext = sprintf(mygump[0], "What guilds do you with to declare peace?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter = 1;
		unsigned int i;
		for (i = 0; i < this->war.size(); ++i)
		{
			if ( this->war[i] != INVALID_SERIAL)
			{
				P_ITEM pStone = FindItemBySerial( this->war[i] );
				strcpy(mygump[++counter], pStone->name().ascii());
			}
		}
		gmprefix[8] = 15;
		break;
	}
	case 16:														// War list 2
		{
		gumpnum=1;
		lentext=sprintf(mygump[0], "Guilds that have decalred war on %s.", this->name().ascii());
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		list<SERIAL>::iterator it;
		for (it = guilds.begin(); it != guilds.end(); ++it)
		{
			if ((*it) != INVALID_SERIAL)
			{
				unsigned int i;
				for ( i = 0; i < this->war.size(); ++i)
				{
					if (this->war[i] == *it)
					{
						++gumpnum;
						P_ITEM pStone = FindItemBySerial(*it);
						strcpy(mygump[++counter], pStone->name().ascii() );
					}
				}
			}
		}
		gmprefix[8] = 16;
		break;
		}
	}
	
	int total=9+1+lentext+1;
	int i;
	for (i = 1; i <= gumpnum; i++)
	{
		total+=4+1+strlen(mygump[i]);
	}
	gmprefix[1] = total>>8;
	gmprefix[2] = total%256;
	LongToCharPtr(pc->serial, &gmprefix[3]);
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, mygump[0], lentext);
	lentext = gumpnum;
	Xsend(s, &lentext, 1);
	for (i = 1; i <= gumpnum; i++)
	{
		gmmiddle[0]=0;
		gmmiddle[1]=0;
		Xsend(s,gmmiddle,4);
		lentext=strlen(mygump[i]);
		Xsend(s,&lentext,1);
		Xsend(s,mygump[i],lentext);
	}
	return;
}


// OKAY (but take another look)
// guildresign() : Player gets removed from the guilddatabase, and gets a message.
// Offcourse guilddatabase gets checked for members left, if everyone is gone, then vanish
// the guildstone. After Guildmaster resigns, the fealty of each remaining member calculates
// a new guildmaster, if there is a draw then we'll have no master until they change their minds ;)
void GuildResign(int s)
{

	P_CHAR pc = currchar[s];

	cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(pc->guildstone()));

	if (pStone == NULL)
	{
		sysmessage(s, "You are in no guild");
		return;
	}

	pStone->removeMember( currchar[s] );
	sysmessage(s,"You are no longer in that guild.");
	if ((pStone->ownserial == pc->serial) && (!pStone->member.empty()))
	{
		pStone->SetOwnSerial(INVALID_SERIAL);
		pStone->CalcMaster();
	}
	if (pStone->member.empty())
	{
		Items->DeleItem( pStone );
		sysmessage(s,"You have been the last member of that guild so the stone vanishes.");
	}
	return;
}


// OKAY
// guilderaseguild() Wipes all information about a guild and removes the guildstone
void EraseGuild(int guildnumber)
{
/*
	if (guildnumber<0 || guildnumber >=MAXGUILDS) return;

	P_ITEM pStone = FindItemBySerial(guilds[guildnumber].stone);
	if (pStone == NULL) return;
	int war;
	int counter;
	
	memset(&guilds[guildnumber], 0, sizeof(guild_st));
	guilds[guildnumber].free = true;
	Items->DeleItem(pStone);
	for (counter=1;counter<MAXGUILDS;counter++)
	{
		if (!guilds[counter].free)
		{
			for (war=1;war<MAXGUILDWARS;war++)
			{
				if (guilds[counter].war[war]==guildnumber)
				{
					guilds[counter].war[war]=0;
					guilds[counter].wars--;
				}
			}
		}
	}
	*/
}


static void RemoveShields(P_CHAR pc)
{
	cwmWorldState->RemoveItemsFromCharBody(pc->serial,0x1B, 0xC3);
	cwmWorldState->RemoveItemsFromCharBody(pc->serial,0x1B, 0xC4);
	P_ITEM pPack = Packitem(pc);
	if (pPack)
	{
		pPack->DeleteAmount(666,0x1BC3);	// hope they don't have more than 666 shields ;-) (Duke)
		pPack->DeleteAmount(666,0x1BC4);
	}
}

// guilderasemember() Wipes all guild related data from a player
void cGuildStone::removeMember(P_CHAR pc)
{
	if ( pc == NULL ) return;

	vector<SERIAL>::iterator it = find(member.begin(), member.end(), pc->serial);
	member.erase(it);
	pc->setGuildstone( INVALID_SERIAL );
	pc->setGuildfealty( INVALID_SERIAL );
	pc->setGuildtoggle( false );
	pc->setGuildtitle( QString::null );
	RemoveShields(pc);
}

// guildtoggleabbreviation() Toggles the settings for showing or not showing the guild title
// Informs player about his change
void cGuildStone::ToggleAbbreviation(UOXSOCKET s)
{
	P_CHAR pc = currchar[s];

	if (!isMember(pc)) 
	{
		sysmessage(s, "you are not a guild member");
		return;
	}

	if (this->guildType != cGuildStone::standard)		// Check for Order/Chaos
	{
		sysmessage(s, "You are in an Order/Chaos guild, you cannot toggle your title.");
	}
	else
	{
		if (!pc->guildtoggle())									// If set to Off then
		{
			pc->setGuildtoggle(true);									// Turn it On
			sysmessage(s, "You toggled your abbreviation on.");	// Tell player about the change
		}
		else													// Otherwise
		{
			pc->setGuildtoggle(false);					// Turn if Off
			sysmessage(s, "You toggled your abbreviation off.");	// And tell him also
		}
	}
	this->Menu(s, 1);										// Send him back to the menu
	return;
}


// guildrecruit() Let the guild members recruit some player into the guild.
// Checks the guild database if "to be recruited" player already in any other guild.
// puts a tag with players serial number into the guilds recruit database.
void cGuildStone::Recruit(UOXSOCKET s)
{

	if ( currchar[s]->guildstone() == INVALID_SERIAL ) 
	{
		sysmessage(s,"you are in no guild");
		return;
	}

	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return; // check if user canceled operation - Morrolan
	int serial = calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
	P_CHAR pc = FindCharBySerial( serial );
	if(pc != NULL)
	{
			if (pc->guildstone() != INVALID_SERIAL) 
				sysmessage(s,"This person is already in a guild.");
			else 
			{
				if (pc->isPlayer())
				{
					this->recruit.push_back(pc->serial);
				} 
				else sysmessage(s,"This is not a player.");
			}
			//break;
		//} for
	}
	this->Menu(s,1);
	return;
}

int GuildCompare(P_CHAR player1, P_CHAR player2)
{
	if (player1 == NULL || player2 == NULL) return 0;

	// one of both not in a guild -> no guildwarfare
	if (player1->guildstone() == INVALID_SERIAL || player2->guildstone() == INVALID_SERIAL ) return 0;

	if (player1->guildstone() == player2->guildstone()) { return 1; }

	cGuildStone* pStone1 = dynamic_cast<cGuildStone*>(FindItemBySerial(player1->guildstone()));
	cGuildStone* pStone2 = dynamic_cast<cGuildStone*>(FindItemBySerial(player2->guildstone()));

	if ( pStone1->guildType != pStone2->guildType && pStone1->guildType != cGuildStone::standard && pStone2->guildType != cGuildStone::standard)
	{
		return 2;
	}

	unsigned int i;
	for (i = 0; i < pStone1->war.size(); ++i)
	{
		if ( find(pStone1->war.begin(), pStone1->war.end(), pStone2->serial) != pStone1->war.end() )
		{
			if ( find( pStone2->war.begin(), pStone2->war.end(), pStone1->serial ) != pStone2->war.end() )
			{
				return 2;
			}
		}
	}
	return 0;
}


// guildgumpinput() result branching for those niffy little gump text fields
// Called by: gumpinput()
void cGuildStone::GumpInput(UOXSOCKET s, int type, int index, char *text)
{
	if (type == 100)
	{
		switch (index)
		{
		case 1:		ChangeName(s,text);			return;			// Guild name requester
		case 2:		ChangeAbbreviation(s,text);	return;			// Guild abbreviation requester
		case 3:		ChangeTitle(s,text);		return;			// Guild master title requester
		case 4:		ChangeTitle(s,text);		return;			// Guild member title requester
		case 5:		ChangeCharter(s,text);		return;			// Guild charter requester
		case 6:		ChangeWebpage(s,text);		return;			// Guild webpage requester
		}
	}
}


// guildgumpchoice() branches the results of those checkbox gumps
// Called by: choice() 
void cGuildStone::GumpChoice( UOXSOCKET s, UI16 MenuID, UI16 Choice )
{
//	int member, recruit, war, guild, counter, slot;
	//int members[MAXGUILDMEMBERS];
	//int recruits[MAXGUILDRECRUITS];
	P_CHAR pc_currchar = currchar[s];
	int counter = 1;

	// Truncate the first few bytes
	UI08 Page = (UI08)MenuID;

	if ( pc_currchar->guildstone() != this->serial ) 
		return;

	switch( Page )
	{
	case 0:
	case 1:									// main menu
		switch( Choice )
		{
//		case 1: target(s,0,1,0,220,"Select person to invite into the guild.");	break;
		case 2:	Menu(s,7);														break;
		case 3: Menu(s,5);														break;
		case 4: Menu(s,13);														break;
		case 5: ToggleAbbreviation(s);											break;
		case 6: GuildResign(s);													break;
		case 7: Menu(s,6);														break;
		case 8:
		default:
			if( Choice == 8 && Page == 0 ) 
				Menu( s, 2 );

			else if( ( Choice == 8 && Page == 1 ) || ( Choice == 9 && Page == 0 ) ) 
				Menu( s, 11 );
			else if( ( Choice == 9 && Page == 1 ) || ( Choice == 10 && Page == 0 ) ) 
				Menu( s, 16 );
			break;
		}
		return;
	case 2:													// guildmaster menu
		switch( Choice )
		{
		case 1:  entrygump(s,pc_currchar->serial,100,1,40,"Enter a new guildname.");		break;
		case 2:  entrygump(s,pc_currchar->serial,100,2,3,"Enter a new guild abbreviation.");break;
		case 3:  Menu(s,3);																	break;
		case 4:  Menu(s,4);																	break;
		case 5:  Menu(s,8);																	break;
		case 6:  Menu(s,14);																break;
//		case 7:  target(s,0,1,0,221,"Select person to declare war to.");					break;
		case 8:  Menu(s,15);																break;
		case 9:	 Menu(s,10);																break;
		case 10: Menu(s,9);																	break;
		case 11: entrygump(s,pc_currchar->serial,100,3,20,"Enter new guildmastertitle.");	break;
		case 12: Menu(s,12);																break;
		case 13: sysmessage(s, "Not yet");													break;
		case 14: Menu(s,1);																	break;
		}
		return;
	case 3:													// set type menu
		Menu(s,2);
		return;
	case 4:													// edit charter menu
		switch( Choice )
		{
		case 1: Menu( s, 2 );																break;
		case 2: entrygump( s, pc_currchar->serial, 100, 5, 50, "Enter a new charter." );	break;
		case 3:	entrygump( s, pc_currchar->serial, 100, 6, 50, "Enter a new URL." );		break;
		}
		return;
	case 5:													// view charter menu
		if ( Choice == 1 ) 
			Menu(s,1);
		else if ( Choice == 2 ) 
			weblaunch(s, (char*)this->webpage.c_str());

		return;
	case 6:													// candidates menu
		Menu(s,1);
		return;
	case 7:													// roster menu
		Menu(s,1);
		return;
	case 8:			// dismiss menu
		if ( static_cast<unsigned int>( Choice ) >= member.size() )
			return;

		if ( member[ Choice ] == pc_currchar->serial)
			sysmessage( s, "You can not dismiss yourself, please resign from the guild instead");

		else
		{
			P_CHAR pc_member = FindCharBySerial( member[ Choice ] );
			removeMember( pc_member );
			sysmessage(s,"Kicked that member out of the guild.");
			if (online(pc_member))
				sysmessage(calcSocketFromChar(pc_member),"You got dismissed out of your guild.");
		}
		Menu(s,2);
		return;
	case 9:													// refuse menu
		counter=1;
		if ( static_cast<unsigned int>( Choice ) >= recruit.size() )
			return;
		else
		{
			recruit.erase( recruit.begin() + Choice );
			sysmessage(s,"Removed candidate from the list.");
		}
		Menu(s,2);
		return;
	case 10:													// accept
		counter=1;
		if ( static_cast<unsigned int>( Choice ) >= recruit.size() )
			return;
		else
		{
			addMember( FindCharBySerial( recruit[ Choice ] ) );
			recruit.erase( recruit.begin() + Choice );
			sysmessage( s, "The candidate is now a guild member");
		}
		Menu(s,2);
		return;
	case 11:													// warlist menu
		Menu(s,1);
		return;
	case 12:													// grant title menu
		if ( Choice == 1 ) Menu( s, 2 );
/*		counter=1;
		for (member = 1; member < MAXGUILDMEMBERS; member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				if (sub == counter)
				{
					guilds[guildnumber].priv = guilds[guildnumber].member[member];
					entrygump(s,pc_currchar->serial,100,3,20,"Enter new guildtitle.");
					return;
				}
			}		
		}
*/		return;
	case 13:													// fealty menu
		if ( static_cast<unsigned int>( Choice ) >= member.size() )
			return;
		else
		{
			pc_currchar->setGuildfealty( member[ Choice ] );
		}
		Menu( s, 1 );
		return;
	case 14:													// declare war menu
		{
			counter=1;
			list<SERIAL>::iterator it;
			for (it = guilds.begin(); it != guilds.end(); ++it)
			{
				if (this->serial != *it)
				{
					++counter;
					if ( Choice == counter)
					{
						if ( find(this->war.begin(), this->war.end(), *it ) != this->war.end() )
							sysmessage(s,"This guild is already in our warlist.");
						else
						{
							this->war.push_back(*it);
							cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(*it));
							char text[256];
							sprintf(text,"%s declared war to %s", this->name().ascii(), pStone->name().ascii() );
							this->Broadcast(text);
							pStone->Broadcast(text);
							
						}
					}
				}
			}
		}
		Menu( s, 2 );
		return;

	// declare peace menu
	case 15:		
		{
			counter = 1;
			unsigned int i;
			for (i = 0; i < this->war.size(); ++i)
			{
				if (this->war[i] != INVALID_SERIAL) // we don't need a loop here, really.
				{
					++counter;
					if ( Choice == counter )
					{
						char text[256];
						cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(this->war[i]));
						sprintf(text,"%s declared peace to %s",this->name().ascii(), pStone->name().ascii());
						this->war.erase( this->war.begin() + i );
						this->Broadcast(text);
						pStone->Broadcast(text);
					}
				}
			}
		}
		Menu(s,2);
		return;

	case 16:													// warlist menu 2
		Menu( s, 1 );
		return;
	}
}




// TESTED: OKAY
// guildnamechange(character, text) copies the text info the guildstones name field 
// (guildnumber gets calculated from the double clicked guildstones), and notifies all online
// guildmambers about the change.
void cGuildStone::ChangeName(UOXSOCKET s, char *text)
{
	list<SERIAL>::iterator it;
	for (it = guilds.begin(); it != guilds.end(); ++it)
	{
		P_ITEM pStone = FindItemBySerial(*it);
		if (pStone->name() == text) 
		{
			sysmessage(s,"This name is already taken by another guild.");
			return;
		}
	}
	this->setName( text );
	char txt[200];
	sprintf(txt, "Your guild got renamed to %s", this->name().ascii() );
	this->Broadcast(txt);
}


// TESTED: OKAY
// guildabbreviationchange(character, text) copies the text info the guildstones abbreviation field 
// (guildnumber gets calculated from the double clicked guildstones), and notifies editing player about
// the change.
void cGuildStone::ChangeAbbreviation(UOXSOCKET s, char *text)
{
	list<SERIAL>::iterator it;
	for (it = guilds.begin(); it != guilds.end(); ++it)
	{
		cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(*it));
		if (pStone->abbreviation == text) 
		{
			sysmessage(s,"This abbreviation is already taken by another guild.");
			return;
		}
	}
	this->abbreviation = text;
	char txt[200];
	sprintf(txt, "Your guild has now the abbreviation: %s", this->abbreviation.c_str());
	this->Broadcast(txt);
	Menu(s,2);
}

// guildtitlechange(character, text) copies the text info the characters title field (guildnumber
// gets calculated from the double clicked guildstones), changed character was buffered in the guilds
// private field (as backup buffer) and notifies editing player about the change.
void cGuildStone::ChangeTitle(UOXSOCKET s, char *text)
{
	P_CHAR member = FindCharBySerial(this->priv);

	if (member == NULL) member = currchar[s];
	this->priv = INVALID_SERIAL;
	member->setGuildtitle( text );
	if (member == currchar[s]) 
		sysmessage(s,"You changed your own title.");
	else 
		sysmessage(s,"You changed the title.");
	Menu(s,2);
}

// TESTED: OKAY
// guildcharterchange(character, text) copies the text info the guildstones charter field (guildnumber
// gets calculated from the double clicked guildstones), and notifies editing player about the change.
void cGuildStone::ChangeCharter(UOXSOCKET s, char *text)
{
	this->charter = text;
	sysmessage(s,"You changed the guilds charter.");
	Menu(s,2);
}

// TESTED: OKAY
// guildwebpagechange(character, text) copies the text info the guildstones webpage field (guildnumber
// gets calculated from the double clicked guildstones), and notifies editing player about the change.
void cGuildStone::ChangeWebpage(UOXSOCKET s, char *text)
{
	this->webpage = text;
	sysmessage(s, "You changed the guilds webpage url.");
	Menu(s,2);
}

cGuildStone::enGuildType cGuildStone::GetType()
{
	return this->guildType;
}

void cGuildStone::SetType(enGuildType type)
{
	guildType = type;

	unsigned int i;
	switch(type)
	{
	case order:	//	Convert into a order guild
		for(i = 0; i < member.size(); ++i)
		{
			P_CHAR holding = FindCharBySerial( member[i] );
			if (holding != NULL) 
			{
				RemoveShields( holding );
				Items->SpawnItemBackpack2( calcSocketFromChar( holding ), "29", 1 );	// will not work for offline chars (Duke)
			}
		}
		Broadcast( "Your guild is now an Order guild." );
		Broadcast( "An order shield has been placed in your pack!" );
		break;
	case chaos:	//	Convert guild into an choas guild
		for(i = 0; i < member.size(); ++i)
		{
			P_CHAR holding = FindCharBySerial( member[i] );
			if ( holding != NULL ) 
			{
				RemoveShields( holding );
				Items->SpawnItemBackpack2( calcSocketFromChar( holding ), "28", 1 );
			}
		}
		Broadcast( "Your guild is now a Chaos guild." );
		Broadcast( "A chaos shield has been placed in your pack!" );
		break;
	case standard:	// Convert guild into a standard guild
	default:	//	Just let it fall through 
		for(i = 0; i < member.size(); ++i)
		{
			P_CHAR holding = FindCharBySerial( member[i] );
			if( holding != NULL )
			{
				RemoveShields( holding );
			}
		}
		Broadcast( "Your guild is now a Standard guild." );
		break;
	}
}

// TESTED: OKAY
// Uhm.. do i need char or socket for sysmessage? hehe.. a socket ofcourse, FiXED iT!
// guildbroadcast(guildnumber, text) broadcasts message to all online members of guild
void cGuildStone::Broadcast(char *text)
{
	unsigned int i;
	for (i = 0; i < member.size(); ++i)
	{
		P_CHAR pc = FindCharBySerial( member[i] );
		if ( pc == NULL )
			continue;
		if ( online( pc ) ) 
			sysmessage( calcSocketFromChar( pc ), text);
	}
}


// TESTED: OKAY (i think so, but ugly -> redo it!)
// guildmastercalc(guildnumber) counts all fealty settings of all members of 'guildnumber' and sets
// a new guildmaster if there is a draw then there will be no master, til the next check ;)
void cGuildStone::CalcMaster()
{

	std::map<unsigned int, unsigned int> votes; // Key is member serial and data #votes
	
	unsigned int i;
	for ( i = 0; i < member.size(); ++i)
	{
		P_CHAR pc = FindCharBySerial( member[i] );
		votes[pc->guildfealty()]++;
	}

/*	struct maxVotes : public binary_function< pair<unsigned int, unsigned int>, pair<unsigned int, unsigned int>, bool>
	{
		operator(pair<unsigned int, unsigned int> a, pair<unsigned int, unsigned int> b) 
		{ 
			return (a.second < b.second);
		}
	};*/

	std::map<unsigned int, unsigned int>::iterator it = max_element(votes.begin(), votes.end(), votes.value_comp());

	unsigned int currenthighest = it->first;
	unsigned int currenthighestvotes = it->second;
	votes.erase( it );
	// check for draw;
	it = max_element(votes.begin(), votes.end(), votes.value_comp());
	bool draw =  ( it->second == currenthighestvotes );

	if (!draw)
		this->master = currenthighest;
}

void cGuildStone::Serialize( ISerialization &archive )
{
	if ( archive.isReading() )
	{
		archive.read( "guildname", guildName );
		archive.read( "abbreviation", abbreviation );
		archive.read( "guildtype", (int&)(guildType) );
		archive.read( "charter", charter );
		archive.read( "webpate", webpage );
		archive.read( "master", master );
		unsigned int uiCount, i;
		archive.read( "recruitcount", uiCount);
		for ( i = 0; i < uiCount; ++i )
		{
			SERIAL tmp;
			archive.read("recruit", tmp);
			recruit.push_back(tmp);
		}
		archive.read( "membercount", uiCount );
		for ( i = 0; i < uiCount; ++i )
		{
			SERIAL tmp;
			archive.read("member", tmp);
			member.push_back(tmp);
		}
		archive.read( "warcount", uiCount );
		for ( i = 0; i < uiCount; ++i )
		{
			SERIAL tmp;
			archive.read("war", tmp);
			war.push_back(tmp);
		}
		sort(member.begin(), member.end()); // just to make sure.
	}
	else if ( archive.isWritting() )
	{
		archive.write( "guildname", guildName );
		archive.write( "abbreviation", abbreviation );
		archive.write( "guildtype", guildType );
		archive.write( "charter", charter );
		archive.write( "webpate", webpage );
		archive.write( "master", master );
		unsigned int i;
		archive.write( "recruitcount", recruit.size());
		for ( i = 0; i < recruit.size(); ++i )
		{
			archive.write("recruit", recruit[i]);
		}
		archive.write( "membercount", member.size() );
		for ( i = 0; i < member.size(); ++i )
		{
			archive.write("member", member[i]);
		}
		archive.write( "warcount", war.size() );
		for ( i = 0; i < war.size(); ++i )
		{
			archive.write("war", war[i]);
		}
	}
	cItem::Serialize( archive );
}

// TESTED: OKAY (fine fine.. now proove that it really works.. )
// guildtitle(viewing character socket, clicked character) displays players title string, over the name
// of clicked character, name color gets calculated from the guild relationship of both players
// Called by: textflags()
void GuildTitle(int s, P_CHAR pc_player2)
{
	char title[150];
	char abbreviation[5];
	char guildtype[10];
	int tl;

	if ( pc_player2 == NULL )
		return;

	if ( pc_player2->guildstone() != INVALID_SERIAL && pc_player2->guildtoggle() )
	{
		cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial( pc_player2->guildstone() ));
		strcpy(abbreviation, pStone->abbreviation.c_str());
		
		if (!(strcmp(abbreviation,"")))
			strcpy(abbreviation,"none");
		switch (pStone->guildType)
		{
		case cGuildStone::order:	strcpy(guildtype, "Order");		break;
		case cGuildStone::chaos:	strcpy(guildtype, "Chaos");		break;
		default:													break;		
		}

		if (!pc_player2->guildtitle().isEmpty()) 
			sprintf(title,"[%s, %s] [%s]",pc_player2->guildtitle().latin1(),abbreviation,guildtype);
		else 
			sprintf(title,"[%s] [%s]",abbreviation, guildtype);

		tl=44+strlen(title)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		LongToCharPtr(pc_player2->serial, &talk[3]);
		talk[7]=1;
		talk[8]=1;
		talk[9]=0;
		ShortToCharPtr(pc_player2->emotecolor(), &talk[10]);
		talk[12]=0;
		talk[13]=3;
		Xsend(s, talk, 14);
		Xsend(s, sysname, 30);
		Xsend(s, title, strlen(title)+1);
	}
}

int CheckValidPlace(int s)
{
	int los = 0;
	P_CHAR pc_currchar = currchar[s];
	cMulti* pi_multi = cMulti::findMulti( pc_currchar->pos ); 
	if( !pi_multi ) 
		return 0;
	if( !IsHouse( pi_multi->id() ) ) 
		return 0;
	
	if(pc_currchar->packitem() != INVALID_SERIAL)
	{
		vector<SERIAL> vecContainer = contsp.getData(pc_currchar->packitem());
		for (unsigned int j = 0; j < vecContainer.size(); j++)
		{
			P_ITEM pi = FindItemBySerial(vecContainer[j]);
			if (pi != NULL) 
				if (pi->type()==7 && calcserial(pi->more1, pi->more2, pi->more3, pi->more4) == pi_multi->serial)
				{
					los=1;
					break;
				}
		}		
		if (los)
		{
			return 1;
		} else
			sysmessage(s, "You must be close to a house and have a key in your pack to place that.");
	} else if (pi_multi == NULL)
		sysmessage(s, "You must be close to a house and have a key in your pack to place that.");
	return 0;
}


/// LB 11'th NOV 2000
/// correkt incorrect guild settings
/// called at startup
void CheckConsistancy(void )
{
/*   int guildnumber,members,serial,ok=1,error=0;
   P_CHAR pc_a;

   clConsole.send("checking guild data consistancy\n");

   //////////// check the guilds

   for (guildnumber=1; guildnumber<MAXGUILDS; guildnumber++)
   {

	   if (!guilds[guildnumber].free)
	   {

	     // is the guildmaster still alive ?

	     ok=1;
	     serial=guilds[guildnumber].master;
	     P_CHAR pc_i = FindCharBySerial( serial );

	     if (pc_i == NULL) // if not, erase the guild !
		 {
			ok=0;
			sprintf((char*)temp,"guild: %s ereased because guildmaster vanished",guilds[guildnumber].name);
			LogWarning((char*)temp);
			Guilds->EraseGuild(guildnumber);
		 }

	     // guildstone deleted ? yes -> erase guild !
	     if (ok) // don't erease twice ;)
		 {          		 
            serial=guilds[guildnumber].stone;
	        P_ITEM pi = FindItemBySerial( serial );

		    if (pi == NULL)
			{
			  ok=0;
			  sprintf((char*)temp,"guild: %s ereased because guildstone vanished",guilds[guildnumber].name);
			  LogWarning((char*)temp);
			  Guilds->EraseGuild(guildnumber);
			}

		 }

	     if (ok)
		 {

			 // check for guildmembers that don't exist anymore and remove from guild structure if so
	        for (members=1; members<MAXGUILDMEMBERS; members++)
			{
 
			   serial=guilds[guildnumber].member[members];
        	   P_CHAR pc_i = FindCharBySerial( serial );

			   if (pc_i == NULL)
			   {
                  ok = 0;							  
				  sprintf((char*)temp,"guild: %s had an member that didnt exist anymore, removed\n",guilds[guildnumber].name);
			      LogWarning((char*)temp);				
			      guilds[guildnumber].member[members]=0;
			      guilds[guildnumber].members--;			                 
			   }

			}

		 }
	   
	   }
	   if (!ok) error=1;
   }

   //////////////// now check all characters for still being in a deleted guild

   AllCharsIterator iter_char;
   for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
   {
     ok=1;
	 pc_a = iter_char.GetData();	
	 if (pc_a == NULL) return;
	 
	 if (pc_a->isPlayer())
     {

	    if (pc_a->guildnumber <0 || pc_a->guildnumber>=MAXGUILDS) // invalid guildnumber ?
		{  
		   sprintf((char*)temp,"player %s has invalid guild info. cancled his/her guild membership",pc_a->name.c_str());
		   LogWarning((char*)temp);
		   pc_a->guildnumber=0;
		   pc_a->guildfealty=0;
		   pc_a->guildtitle="";
		   pc_a->guildtoggle=false;
		   ok=0;
		}

	    if (ok)
		{
           if (guilds[pc_a->guildnumber].free && pc_a->guildnumber!=0)
		   {
			  sprintf((char*)temp,"player %s belongs to a guild that is no more. canceled his/her guild membership", pc_a->name.c_str());
			  LogWarning((char*)temp);
			  pc_a->guildnumber=0;
		      pc_a->guildfealty=0;
		      pc_a->guildtitle="";
		      pc_a->guildtoggle=false;
		      ok=0;
		   }
			   
		}

	 }      
	 if (!ok) error=1;
   }

   if (error) clConsole.send("guild releated errors detected and corrected, check logs\n"); else clConsole.send("no errors found\n");
*/
}

