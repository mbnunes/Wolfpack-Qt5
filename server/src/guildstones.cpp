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
//	Wolfpack Homepage: http://www.wpdev.com/
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

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "guildstones.cpp"

// placeguildstone() : spawns a renamed gravestone at players location and removes deed
// Placer gets guildmaster, whatever he does ;)
// Force placer to give that damn guild a damn name :)
void cGuilds::StonePlacement(int s)
{
	int guildnumber;
	//unsigned int k; // lb, msvc++ 5.0 didnt like the guild(int x,inty) ...
	char stonename[60];
	P_CHAR pc = MAKE_CHARREF_LR(currchar[s]);
	P_ITEM pDeed=MAKE_ITEMREF_LR(pc->fx1);
	P_ITEM pStone=NULL;

	if (CheckValidPlace(s)!=1)
	{
		sysmessage(s,"You cannot place guildstones at any other location than your house");
		return;
	}
	
	if (pDeed->id()==0x14F0)
	{
		if (pc->guildnumber!=0)
		{
			itemmessage(s,"You are already in a guild.",pDeed->serial);
			return;
		}
		guildnumber=Guilds->SearchSlot(0,1);
		if (guildnumber==-1)
		{
			itemmessage(s,"There are already enough guildstones placed.",pDeed->serial);
			return;
		}
		pc->guildnumber=guildnumber;
		pStone=Items->SpawnItem(currchar[s],1,"Guildstone for an unnamed guild",0,0x0ED5,0,0);
		if (!pStone)
		{//AntiChrist - to prevent crashes
			sysmessage(s,"Cannot create guildstone");
			return;
		}
		strcpy(pc->guildtitle,"Guildmaster");
		if (pc->id() == 0x0191)	
		{
			strcpy(pc->guildtitle, "Guildmistress");	
		}


		guilds[guildnumber].free = 0;
		guilds[guildnumber].members = 1;
		guilds[guildnumber].member[1] = pc->serial;
		guilds[guildnumber].type = 0;
		guilds[guildnumber].abbreviation[0] = 0;
		strcpy(guilds[guildnumber].webpage,DEFAULTWEBPAGE);
		strcpy(guilds[guildnumber].charter,DEFAULTCHARTER);
		pStone->MoveTo(pc->pos.x,pc->pos.y,pc->pos.z);
		pStone->type = 202;
		pStone->priv = 0;
		pStone->magic = 4;
		pStone->setOwnSerialOnly(pc->serial);

		RefreshItem(pStone);//AntiChrist
		Items->DeleItem(pDeed);
		guilds[guildnumber].stone=pStone->serial;
		guilds[guildnumber].master=pc->serial;
		entrygump(s, pc->serial,100,1,40,"Enter a name for the guild.");
	}
	else
	{
		guildnumber = SearchByStone(s);
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
			RefreshItem(pStone);//AntiChrist
			Items->DeleItem(pDeed);
			pc->fx1 = 0;
			guilds[guildnumber].stone = pStone->serial;
		}
		else
			itemmessage(s,"You are not the guildmaster of this guild. Only the guildmaster may use this guildstone teleporter.",pDeed->serial);
	}
}


// guildstonemenu() : Opens the guild menu for a player
// Recognizes Guildmaster with the owner fields of the stone.
// Ofcourse checks for membership before opening any gump ;)
void cGuilds::Menu(int s, int page)
{
	int total,i,stone,guildmaster,counter,guild,recruit,war,member;
	int lentext;
	int gumpnum;
	char guildfealty[60],guildt[16],toggle[6];
	static char mygump[MAXMEMRECWAR][257];

	P_CHAR pc = MAKE_CHARREF_LR(currchar[s]);
	stone=pc->fx1;

	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1)
	{
		sysmessage(s, "You are not a member of this guild. Ask an existing guildmember to invite you into this guild.");
		return;
	}

	if ((guilds[guildnumber].stone!=items[stone].serial)&&
		(!(pc->isGM())))
	{
		itemmessage(s,"You are not a member of this guild. Ask an existing guildmember to invite you into this guild.",items[stone].serial);
		return;
	}

	strcpy(guildfealty, "yourself");
	if ((pc->guildfealty != pc->serial)&&(pc->guildfealty!=0))
	{
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (pc->guildfealty == guilds[guildnumber].member[member])
			{
				strcpy(guildfealty, chars[calcCharFromSer(guilds[guildnumber].member[member])].name);
				break;
			}
		}
	}
	else pc->guildfealty = pc->serial;	
	if (guilds[guildnumber].master==0) Guilds->CalcMaster(guildnumber);
	guildmaster = calcCharFromSer(guilds[guildnumber].master);

	switch (guilds[guildnumber].type) 
	{
		case 0:		strcpy(guildt, " Standard");	break;
		case 1:		strcpy(guildt, "n Order");		break;
		case 2:		strcpy(guildt, " Chaos");		break;
	}
	
	if (pc->guildtoggle) 
		strcpy(toggle, "On");
	else
		strcpy(toggle, "Off");

	switch(page)
	{
	case 1:
		gumpnum=9;
		gmprefix[7]=8001>>8;
		gmprefix[8]=8001%256;

		if (guildmaster<0) return;

		lentext=sprintf(mygump[0], "%s (%s %s)",guilds[guildnumber].name,chars[guildmaster].guildtitle,chars[guildmaster].name);
		strcpy(mygump[1],"Recruit someone into the guild.");
		strcpy(mygump[2],"View the current roster.");
		strcpy(mygump[3],"View the guild's charter.");
		sprintf(mygump[4],"Declare your fealty. You are currently loyal to %s.",guildfealty);
		sprintf(mygump[5],"Toggle showing the guild's abbreviation in your name to unguilded people. Currently %s.",toggle);
		strcpy(mygump[6],"Resign from the guild.");
		strcpy(mygump[7],"View list of candidates who have been sponsored to the guild.");
	    if ((pc->serial==guilds[guildnumber].master)||
			(pc->isGM()))							// Guildmaster Access?
		{															
			gumpnum=10;
			gmprefix[7]=8000>>8;
			gmprefix[8]=8000%256;
			sprintf(mygump[8],"Access %s functions.",chars[guildmaster].guildtitle);
			sprintf(mygump[9],"View list of guild that %s has declared war on.",guilds[guildnumber].name);
			sprintf(mygump[10],"View list of guilds that have declared war on %s.",guilds[guildnumber].name);
		} else {													// Normal Members access!
			sprintf(mygump[8],"View list of guilds that %s have declared war on.",guilds[guildnumber].name);
			sprintf(mygump[9],"View list of guilds that have declared war on %s.",guilds[guildnumber].name);
		}
		break;
	case 2:														// guildmaster menu
		gumpnum=14;
		lentext=sprintf(mygump[0], "%s, %s functions",guilds[guildnumber].name,chars[guildmaster].guildtitle);
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
		gmprefix[7]=8002>>8;
		gmprefix[8]=8002%256;
		break;
	case 3:														// guild type
		gumpnum=4;
		lentext=sprintf(mygump[0], "Please select the type you want your guild to be related to.");
		strcpy(mygump[1], "Select this to return to main menu.");
		strcpy(mygump[2], "Set to Standard.");
		strcpy(mygump[3], "Set to Order.");
		strcpy(mygump[4], "Set to Chaos.");
		gmprefix[7]=8003>>8;
		gmprefix[8]=8003%256;
		break;
	case 4:														// edit charter
		gumpnum=3;
		lentext=sprintf(mygump[0], "Set %s charter.",guilds[guildnumber].name);
		strcpy(mygump[1], "Select this to return to main menu.");
		strcpy(mygump[2], "Set the charter.");
		strcpy(mygump[3], "Set the webpage.");
		gmprefix[7]=8004>>8;
		gmprefix[8]=8004%256;
		break;
	case 5:														// view charter
		gumpnum=2;
		lentext=sprintf(mygump[0], "%s charter.",guilds[guildnumber].name);
		sprintf(mygump[1], "%s. Select this to return to the main menu.",guilds[guildnumber].charter);
		sprintf(mygump[2], "Visit the guild website at %s",guilds[guildnumber].webpage);
		gmprefix[7]=8005>>8;
		gmprefix[8]=8005%256;
		break;
	case 6:														// Candidates list
		gumpnum=guilds[guildnumber].recruits+1;
		lentext=sprintf(mygump[0], "%s list of candidates.",guilds[guildnumber].name);
		sprintf(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				strcpy(mygump[counter],chars[calcCharFromSer(guilds[guildnumber].recruit[recruit])].name);
			}
		}
		gmprefix[7]=8006>>8;
		gmprefix[8]=8006%256;
		break;
	case 7:														// roster
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], "%s members roster.",guilds[guildnumber].name);
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				strcpy(mygump[counter], chars[calcCharFromSer(guilds[guildnumber].member[member])].name);
			}
		}
		gmprefix[7]=8007>>8;
		gmprefix[8]=8007%256;
		break;
	case 8:														// member dismiss
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], "Dismiss what member?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				strcpy(mygump[counter],chars[calcCharFromSer(guilds[guildnumber].member[member])].name);
			}
		}
		gmprefix[7]=8008>>8;
		gmprefix[8]=8008%256;
		break;
	case 9:														// Refuse Candidates
		gumpnum=guilds[guildnumber].recruits+1;
		lentext=sprintf(mygump[0], "Refuse what candidate? %s",guilds[guildnumber].name);
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				strcpy(mygump[counter],chars[calcCharFromSer(guilds[guildnumber].recruit[recruit])].name);
			}
		}
		gmprefix[7]=8009>>8;
		gmprefix[8]=8009%256;
		break;
	case 10:														// Accept Candidates
		gumpnum=guilds[guildnumber].recruits+1;
		lentext=sprintf(mygump[0], "Accept what candidate?.%s",guilds[guildnumber].name);
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				strcpy(mygump[counter],chars[calcCharFromSer(guilds[guildnumber].recruit[recruit])].name);
			}
		}
		gmprefix[7]=8010>>8;
		gmprefix[8]=8010%256;
		break;
	case 11:														// War list
		gumpnum=guilds[guildnumber].wars+1;
		lentext=sprintf(mygump[0], "Guild that %s has declared war on.",guilds[guildnumber].name);
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (war=1;war<MAXGUILDWARS;war++)
		{
			if (guilds[guildnumber].war[war]!=0)
			{
				counter++;
				strcpy(mygump[counter],guilds[guilds[guildnumber].war[war]].name);
			}
		}
		gmprefix[7]=8011>>8;
		gmprefix[8]=8011%256;
		break;
	case 12:														// grant title
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], "Grant a title to whom?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				strcpy(mygump[counter],chars[calcCharFromSer(guilds[guildnumber].member[member])].name);
			}
		}
		gmprefix[7]=8012>>8;
		gmprefix[8]=8012%256;
		break;
	case 13:														// fealty
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], "Whom do you wish to be loyal to?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				strcpy(mygump[counter],chars[calcCharFromSer(guilds[guildnumber].member[member])].name);
			}
		}
		gmprefix[7]=8013>>8;
		gmprefix[8]=8013%256;
		break;
	case 14:														// declare War list
		int dummy;
		gumpnum=1;
		lentext=sprintf(mygump[0], "What guilds do you with to declare war?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (guild=1;guild<MAXGUILDS;guild++)
		{
			if ((guilds[guild].free==0)&&(guildnumber!=guild))
			{
				dummy=0;
				for (war=1;war<MAXGUILDWARS;war++)
				{
					if (guilds[guildnumber].war[war]==guild) dummy=1;
				}
				if (dummy!=1) {counter++;gumpnum++;strcpy(mygump[counter],guilds[guild].name);}
			}
		}
		gmprefix[7]=8014>>8;
		gmprefix[8]=8014%256;
		break;
	case 15:														// declare peace list
		gumpnum=guilds[guildnumber].wars+1;
		lentext=sprintf(mygump[0], "What guilds do you with to declare peace?");
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (war=1;war<MAXGUILDWARS;war++)
		{
			if (guilds[guildnumber].war[war]!=0)
			{
				counter++;
				strcpy(mygump[counter],guilds[guilds[guildnumber].war[war]].name);
			}
		}
		gmprefix[7]=8015>>8;
		gmprefix[8]=8015%256;
		break;
	case 16:														// War list 2
		gumpnum=1;
		lentext=sprintf(mygump[0], "Guilds that have decalred war on %s.",guilds[guildnumber].name);
		strcpy(mygump[1], "Select this to return to the menu.");
		counter=1;
		for (guild=1;guild<MAXGUILDS;guild++)
		{
			if (guilds[guild].free==0)
			{
				for (war=1;war<MAXGUILDWARS;war++)
				{
					if (guilds[guild].war[war]==guildnumber)
					{
						counter++; gumpnum++;
						strcpy(mygump[counter],guilds[guild].name);
					}
				}
			}
		}
		gmprefix[7]=8016>>8;
		gmprefix[8]=8016%256;
		break;
	}
	
	total=9+1+lentext+1;
	for (i=1;i<=gumpnum;i++)
	{
		total+=4+1+strlen(mygump[i]);
	}
	gmprefix[1] = total>>8;
	gmprefix[2] = total%256;
	gmprefix[3] = pc->ser1;
	gmprefix[4] = pc->ser2;
	gmprefix[5] = pc->ser3;
	gmprefix[6] = pc->ser4;
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
void cGuilds::Resign(int s)
{

	P_CHAR pc = MAKE_CHARREF_LR(currchar[s]);

	int guildnumber = pc->guildnumber;

	if (guildnumber==-1)
	{
		sysmessage(s, "You are in no guild");
		return;
	}

	Guilds->EraseMember(currchar[s]);
	sysmessage(s,"You are no longer in that guild.");
	if ((guilds[guildnumber].master == pc->serial) && (guilds[guildnumber].members!=0))
	{
		guilds[guildnumber].master=0;
		Guilds->CalcMaster(guildnumber);
	}
	if (guilds[guildnumber].members==0)
	{
		Guilds->EraseGuild(guildnumber);
		sysmessage(s,"You have been the last member of that guild so the stone vanishes.");
	}
	return;
}


// OKAY
// guilderaseguild() Wipes all information about a guild and removes the guildstone
void cGuilds::EraseGuild(int guildnumber)
{

	if (guildnumber<0 || guildnumber >=MAXGUILDS) return;

	int stone = calcItemFromSer(guilds[guildnumber].stone);
	if (stone==-1) return;
	int war;
	int counter;
	
	memset(&guilds[guildnumber], 0, sizeof(guild_st));
	guilds[guildnumber].free=1;
	Items->DeleItem(stone);
	for (counter=1;counter<MAXGUILDS;counter++)
	{
		if (guilds[counter].free==0)
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
}


static void RemoveShields(CHARACTER c)
{
	cwmWorldState->RemoveItemsFromCharBody(c,0x1B, 0xC3);
	cwmWorldState->RemoveItemsFromCharBody(c,0x1B, 0xC4);
	P_ITEM pPack = Packitem(&chars[c]);
	if (pPack)
	{
		pPack->DeleteAmount(666,0x1BC3);	// hope they don't have more than 666 shields ;-) (Duke)
		pPack->DeleteAmount(666,0x1BC4);
	}
}

// guilderasemember() Wipes all guild related data from a player
void cGuilds::EraseMember(int c)
{
	P_CHAR pc = MAKE_CHARREF_LR(c);
	int guildnumber = pc->guildnumber;

	if (guildnumber>=0 && guildnumber <MAXGUILDS)
	{
		int j,holding,member;
		for(j=0;j<=guilds[guildnumber].members;j++)
		{
			if (guilds[guildnumber].member[j] == pc->serial)
			{
				holding=calcCharFromSer(guilds[guildnumber].member[j]);
				if (holding>-1) 
				{
					RemoveShields(holding);
				}
			}
		}

		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member] == pc->serial)
			{
				guilds[guildnumber].member[member] = 0;
				guilds[guildnumber].members--;
				pc->guildnumber = 0;
				pc->guildtoggle = 0;
				pc->guildfealty = 0;
				pc->guildtitle[0] = 0;
				return;
			}
		}
	}
}



// guildtoggleabbreviation() Toggles the settings for showing or not showing the guild title
// Informs player about his change
void cGuilds::ToggleAbbreviation(int s)
{

	P_CHAR pc = MAKE_CHARREF_LR(currchar[s]);
	int guildnumber = pc->guildnumber;

	if (guildnumber<0 || guildnumber>MAXGUILDS) 
	{
		sysmessage(s, "you are in no guild");
		return;
	}

	if (guilds[guildnumber].type!=0)							// Check for Order/Chaos
	{
		sysmessage(s,"You are in an Order/Chaos guild, you cannot toggle your title.");
																// They may not toggle it off!
	}
	else
	{
		if (pc->guildtoggle==0)									// If set to Off then
		{
			pc->guildtoggle=1;									// Turn it On
			sysmessage(s,"You toggled your abbreviation on.");	// Tell player about the change
		}
		else													// Otherwise
		{
			pc->guildtoggle=0;					// Turn if Off
			sysmessage(s,"You toggled your abbreviation off.");	// And tell him also
		}
	}
	Guilds->Menu(s,1);											// Send him back to the menu
	return;
}


// guildrecruit() Let the guild members recruit some player into the guild.
// Checks the guild database if "to be recruited" player already in any other guild.
// puts a tag with players serial number into the guilds recruit database.
void cGuilds::Recruit(int s)
{
	int slot, dummy;
	int guildnumber = SearchByStone(s);

	if (guildnumber==-1) 
	{
		sysmessage(s,"you are in no guild");
		return;
	}

	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return; // check if user canceled operation - Morrolan
	int serial = calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
	P_CHAR pc = FindCharBySerial( serial );
	if(pc != NULL)
	{
			if (pc->guildnumber!=0) 
				sysmessage(s,"This person is already in a guild.");
			else 
			{
				if (pc->isPlayer())
				{
					slot = SearchSlot(guildnumber, 3);
					for ( dummy = 1; dummy < MAXGUILDRECRUITS; dummy++ )
					{
						if ( guilds[guildnumber].recruit[dummy] == pc->serial ) slot = 0;
					}
					if ((slot!=-1)&&(slot!=0))
					{
						guilds[guildnumber].recruits++;
						guilds[guildnumber].recruit[slot] = pc->serial;
					}
					else
					{
						if (slot==-1) sysmessage(s, "No more recruit slots free.");
						if (slot==0) sysmessage(s, "This being is already a candidate.");
					}
				} else sysmessage(s,"This is not a player.");
			}
			//break;
		//} for
	}
	Guilds->Menu(s,1);
	return;
}


// guildtargetwar() Let us target some player and add his guild to the warlist
// Checks the guild database if "to be wared" guild is already in warlist.
// puts a tag with guilds number into the guilds war database.
void cGuilds::TargetWar(int s)
{
	int slot, dummy;
	int guildnumber = Guilds->SearchByStone(s);
	char text [200];


	if (guildnumber==-1)
	{
		sysmessage(s,"you are in no guild");
		return;
	}

	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return; // check if user canceled operation - Morrolan
	SERIAL serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
	P_CHAR pc = FindCharBySerial( serial );
	if( pc != NULL)
	{
			if (pc->guildnumber==0) 
				sysmessage(s,"This person is not in a guild.");
			else if (pc->guildnumber==guildnumber) sysmessage(s, "War yourself? Nah.");
			else 
			{
				if (pc->isPlayer())
				{
					slot = Guilds->SearchSlot( guildnumber, 4 );
					for ( dummy = 1; dummy < MAXGUILDWARS; dummy++ )
					{
						if ( guilds[guildnumber].war[dummy] == pc->guildnumber ) slot = 0;
					}
					if ((slot!=-1)&&(slot!=0))
					{
						guilds[guildnumber].wars++;
						guilds[guildnumber].war[slot] = pc->guildnumber;
						sprintf(text,"%s declared war to %s",guilds[guildnumber].name,guilds[pc->guildnumber].name);
						Guilds->Broadcast(guildnumber,text);
					}
					else
					{
						if (slot==-1) sysmessage(s,"No more war slots free.");
						if (slot==0) sysmessage(s,"This guild is already in our warlist.");
					}
				}
				else sysmessage(s,"This is not a player.");
			}
			//break;
		//} for
	}
	Guilds->Menu(s,2);
	return;
}



// guildstonemove() : Let the guildmaster move the stone anytime
// removes the stone item, gives Guildmaster a teleporterstone
// guild will be still active
// teleporterstone id 0x1869
void cGuilds::StoneMove(int s)
{
	int guildnumber=Guilds->SearchByStone(s);
	if (guildnumber==-1) return;
	int stone = calcItemFromSer( guilds[guildnumber].stone );
	if (stone==-1) return;
															// Get stone
	int newstone;											// For the new stone
	char stonename[80];                                     // And for its name

	sprintf(stonename,"a guildstone teleporter for %s",guilds[guildnumber].name);
															// Give it a name
	newstone = Items->SpawnItem(s, currchar[s], 1, stonename, 0, 0x18, 0x69, 0, 0, 1, 1);	// Spawn the stone in the masters backpack
	if (newstone==-1) return; //AntiChrist
	items[newstone].type=202;								// Set Guildstone to Type 'Guild Related'
	guilds[guildnumber].stone=items[newstone].serial;		// Remember its serial number
	Items->DeleItem(stone);										// Remove the guildstone
	sysmessage(s,"Take care of that stone!");				// And tell him also
	return;													// Bye bye
}


// guildsearch() Okay this is for highlighting/guards and other stuff, so you know what relation
// player 1 and player 2 have. results are:
// 1= both in same guild (so fighting is okay, green hightlighting)
// 2= both in opposite guilds/guildtypes (so fighting is okay, orange highlighting)
// 0= no guildwarfare, or no guild relation (so no fighting, normal highlighting)
// Oh, Order/Order or Chaos/Chaos guilds (in different guilds) may not war eachother! 
int cGuilds::Compare(int player1, int player2)
{
	int counter;
	if (player1<0 || player2<0) return 0;

	int guildnumber=chars[player1].guildnumber;
    int guildnumber2=chars[player2].guildnumber;

	// one of both not in a guild -> no guildwarfare
	if (guildnumber<0 || guildnumber>=MAXGUILDS || guildnumber2<0 || guildnumber2>=MAXGUILDS) return 0;

	if ((guildnumber!=0)&&(guildnumber2!=0))
	{
		if (guildnumber==guildnumber2) {return 1;}

		if (((guilds[guildnumber].type==1)&&(guilds[guildnumber2].type==2))||
			((guilds[guildnumber].type==2)&&(guilds[guildnumber2].type==1)))
		{return 2;}

		for (counter=1;counter<MAXGUILDWARS;counter++)
		{
			if (guilds[guildnumber].war[counter]==guildnumber2)
			{
				for (counter=1;counter<MAXGUILDWARS;counter++)
				{
					if (guilds[guildnumber2].war[counter]==guildnumber)
					{return 2;}
				}
			}
		}
	}
	return 0;
}


// guildgumpinput() result branching for those niffy little gump text fields
// Called by: gumpinput()
void cGuilds::GumpInput(int s, int type, int index, char *text)
{
	if (type==100)
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
void cGuilds::GumpChoice(int s,int main,int sub)
{
	int i,member, recruit, war, guild, counter, slot;
	//int members[MAXGUILDMEMBERS];
	//int recruits[MAXGUILDRECRUITS];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);
	char text[200];

	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1) return ;

	switch(main)
	{
	case 8000:
	case 8001:									// main menu
		switch( sub )
		{
		case 1: target(s,0,1,0,220,"Select person to invite into the guild.");	break;
		case 2:	Menu(s,7);														break;
		case 3: Menu(s,5);														break;
		case 4: Menu(s,13);														break;
		case 5: ToggleAbbreviation(s);											break;
		case 6: Resign(s);														break;
		case 7: Menu(s,6);														break;
		case 8:
		default:
			if( sub == 8 && main == 8000 ) 
				Menu( s, 2 );
			else if( ( sub == 8 && main == 8001 ) || ( sub == 9 && main == 8000 ) ) 
				Menu( s, 11 );
			else if( ( sub == 9 && main == 8001 ) || ( sub == 10 && main == 8000 ) ) 
				Menu( s, 16 );
			break;
		}
		return;
	case 8002:													// guildmaster menu
		switch(sub)
		{
		case 1:  entrygump(s,pc_currchar->serial,100,1,40,"Enter a new guildname.");		break;
		case 2:  entrygump(s,pc_currchar->serial,100,2,3,"Enter a new guild abbreviation.");break;
		case 3:  Guilds->Menu(s,3);															break;
		case 4:  Guilds->Menu(s,4);															break;
		case 5:  Guilds->Menu(s,8);															break;
		case 6:  Guilds->Menu(s,14);														break;
		case 7:  target(s,0,1,0,221,"Select person to declare war to.");					break;
		case 8:  Guilds->Menu(s,15);														break;
		case 9:	 Guilds->Menu(s,10);														break;
		case 10: Guilds->Menu(s,9);															break;
		case 11: entrygump(s,pc_currchar->serial,100,3,20,"Enter new guildmastertitle.");	break;
		case 12: Guilds->Menu(s,12);														break;
		case 13: Guilds->StoneMove(s);														break;
		case 14: Guilds->Menu(s,1);															break;
		}
		return;
	case 8003:													// set type menu
		if (sub >= 2 && sub <=4) 
			Guilds->SetType(guildnumber, sub-2);
		Guilds->Menu(s,2);
		return;
	case 8004:													// edit charter menu
		switch( sub )
		{
		case 1: Menu( s, 2 );																break;
		case 2: entrygump( s, pc_currchar->serial, 100, 5, 50, "Enter a new charter." );	break;
		case 3:	entrygump( s, pc_currchar->serial, 100, 6, 50, "Enter a new URL." );		break;
		}
		return;
	case 8005:													// view charter menu
		if (sub==1) Guilds->Menu(s,1);
		if (sub==2) weblaunch(s,guilds[guildnumber].webpage);
		return;
	case 8006:													// candidates menu
		Guilds->Menu(s,1);
		return;
	case 8007:													// roster menu
		Guilds->Menu(s,1);
		return;
	case 8008:													// dismiss menu
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				if (sub==counter)
				{
					if (guilds[guildnumber].member[member]==pc_currchar->serial)
					{
						sysmessage(s,"You cannot dimiss yourself, please resign from the guild if you wish.");
					}
					else
					{
						member=calcCharFromSer(guilds[guildnumber].member[member]);
						Guilds->EraseMember(member);
						sysmessage(s,"Kicked that member out of the guild.");
						if (online(member))
							sysmessage(member,"You got dismissed out of your guild.");
					}
					break;
				}
			}
		}	
		Guilds->Menu(s,2);
		return;
	case 8009:													// refuse menu
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				if (sub==counter)
				{
					guilds[guildnumber].recruit[recruit]=0;
					guilds[guildnumber].recruits--;
					sysmessage(s,"Removed candidate from the list.");
				}
			}		
		}
		Guilds->Menu(s,2);
		return;
	case 8010:													// accept
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				if (sub==counter)
				{
					i = calcCharFromSer( guilds[guildnumber].recruit[recruit] );
					P_CHAR pc = FindCharBySerial ( guilds[guildnumber].recruit[recruit] );
					if (pc != NULL) //lb
						if (pc->guildnumber==0)
						{
							slot = Guilds->SearchSlot(guildnumber,2);
							if (slot!=-1)
							{
								guilds[guildnumber].member[slot] = pc->serial;
								guilds[guildnumber].members++;
								chars[i].guildnumber = guildnumber;
								guilds[guildnumber].recruit[recruit] = 0;
								guilds[guildnumber].recruits--;
								if (guilds[guildnumber].type!=0) pc->guildtoggle = 1;
								sysmessage(s, "This candidate is now a guildmember.");
								if ( guilds[guildnumber].type == 1 )
					            Items->SpawnItemBackpack2( calcSocketFromChar( i ), 29, 1 );
						        if ( guilds[guildnumber].type == 2 )
							    Items->SpawnItemBackpack2( calcSocketFromChar( i ), 28, 1 );
							}
							else sysmessage(s, "This guild is full, maximum amount of members reached!" );
						}
						else 
						{
							sysmessage(s,"This candidate is already in another guild, I'll remove him from your list now.");
							guilds[guildnumber].recruit[recruit] = 0;
							guilds[guildnumber].recruits--;
						}
				}
			}		
		}
		Guilds->Menu(s,2);
		return;
	case 8011:													// warlist menu
		Guilds->Menu(s,1);
		return;
	case 8012:													// grant title menu
		if (sub==1) Guilds->Menu(s,2);
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				if (sub==counter)
				{
					guilds[guildnumber].priv = calcCharFromSer( guilds[guildnumber].member[member] );
					entrygump(s,pc_currchar->serial,100,3,20,"Enter new guildtitle.");
					return;
				}
			}		
		}
		return;
	case 8013:													// fealty menu
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				if (sub==counter)
				{
					pc_currchar->guildfealty=guilds[guildnumber].member[member];
				}
			}
		}
		Guilds->Menu(s,1);
		return;
	case 8014:													// declare war menu
		counter=1;
		for (guild=1;guild<MAXGUILDS;guild++)
		{
			if ((strcmp(guilds[guildnumber].name,guilds[guild].name))&&
				(guilds[guild].free!=1))
			{
				counter++;
				if (sub==counter)
				{
					slot=Guilds->SearchSlot(guildnumber,4);
					for (war=1;war<MAXGUILDWARS;war++)
					{
						if (guilds[guildnumber].war[war]==guild) slot=0;
					}
					if ((slot!=-1)&&(slot!=0))
					{
						guilds[guildnumber].wars++;
						guilds[guildnumber].war[slot]=guild;
						sprintf(text,"%s declared war to %s",guilds[guildnumber].name,guilds[guild].name);
						Guilds->Broadcast(guildnumber,text);
					}
					else
					{
						if (slot==-1) sysmessage(s,"No more war slots free.");
						else if (slot == 0) sysmessage(s,"This guild is already in our warlist.");
					}
				}
			}
		}
		Guilds->Menu(s,2);
		return;
	case 8015:													// declare peace menu
		counter=1;
		for (war=1;war<MAXGUILDWARS;war++)
		{
			if (guilds[guildnumber].war[war]!=0)
			{
				counter++;
				if (sub==counter)
				{
					sprintf(text,"%s declared peace to %s",guilds[guildnumber].name,guilds[guilds[guildnumber].war[war]].name);
					Guilds->Broadcast(guildnumber,text);
					guilds[guildnumber].war[war]=0;
					guilds[guildnumber].wars--;
				}
			}
		}
		Guilds->Menu(s,2);
		return;
	case 8016:													// warlist menu 2
		Guilds->Menu(s,1);
		return;
	}
}




// TESTED: OKAY
// guildnamechange(character, text) copies the text info the guildstones name field 
// (guildnumber gets calculated from the double clicked guildstones), and notifies all online
// guildmambers about the change.
void cGuilds::ChangeName(int s, char *text)
{
	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1) return;

	int stone = calcItemFromSer( guilds[guildnumber].stone );
	if (stone==-1) return;
	int guild;
	bool exists = false;
	char txt[200];

	for (guild=1;guild<MAXGUILDS;guild++)
	{
		if (!(strcmp((char*)text,guilds[guild].name))) exists = true;
	}
	if (!exists)
	{
		strcpy(guilds[guildnumber].name, (char*)text);
		sprintf(items[stone].name,"Guildstone for %s",guilds[guildnumber].name);
		sprintf(txt,"Your guild got renamed to %s",guilds[guildnumber].name);
		Guilds->Broadcast(guildnumber,txt);
	}
	else sysmessage(s,"This name is already taken by another guild.");
}


// TESTED: OKAY
// guildabbreviationchange(character, text) copies the text info the guildstones abbreviation field 
// (guildnumber gets calculated from the double clicked guildstones), and notifies editing player about
// the change.
void cGuilds::ChangeAbbreviation(int s, char *text)
{
	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1) return;

	int guild;
	bool exists = false;
	char txt[200];

	for ( guild = 1; guild < MAXGUILDS; guild++ )
	{
		if (!(strcmp((char*)text,guilds[guild].abbreviation))) exists = true;
	}
	if (!exists)
	{
		strcpy(guilds[guildnumber].abbreviation,(char*)text);
		sprintf(txt,"Your guild has now the abbreviation: %s",guilds[guildnumber].abbreviation);
		Guilds->Broadcast(guildnumber,txt);
	}
	else sysmessage(s,"This abbreviation is already taken by another guild.");
	Guilds->Menu(s,2);
}





// guildtitlechange(character, text) copies the text info the characters title field (guildnumber
// gets calculated from the double clicked guildstones), changed character was buffered in the guilds
// private field (as backup buffer) and notifies editing player about the change.
void cGuilds::ChangeTitle(int s, char *text)
{
	int guildnumber=Guilds->SearchByStone(s);
	if (guildnumber==-1) return;

	int member=guilds[guildnumber].priv;

	if (member==0) member=s;
	guilds[guildnumber].priv=0;
	strcpy(chars[member].guildtitle,(char*)text);
	if (member==s) sysmessage(s,"You changed your own title.");
	else sysmessage(s,"You changed the title.");
	Guilds->Menu(s,2);
}




// TESTED: OKAY
// guildcharterchange(character, text) copies the text info the guildstones charter field (guildnumber
// gets calculated from the double clicked guildstones), and notifies editing player about the change.
void cGuilds::ChangeCharter(int s, char *text)
{
	int guildnumber=Guilds->SearchByStone(s);
	if (guildnumber==-1) return;

	strcpy(guilds[guildnumber].charter, text);
	sysmessage(s,"You changed the guilds charter.");
	Guilds->Menu(s,2);
}




// TESTED: OKAY
// guildwebpagechange(character, text) copies the text info the guildstones webpage field (guildnumber
// gets calculated from the double clicked guildstones), and notifies editing player about the change.
void cGuilds::ChangeWebpage(int s, char *text)
{
	int guildnumber = Guilds->SearchByStone(s);
	if (guildnumber==-1) return;

	strcpy(guilds[guildnumber].webpage, text);
	sysmessage(s,"You changed the guilds webpage url.");
	Guilds->Menu(s,2);
}

int cGuilds::GetType(int guildnumber)
{
	if (guildnumber<0 || guildnumber >=MAXGUILDS)
		return 0;
	else
		return guilds[guildnumber].type;
}

void cGuilds::SetType(int guildnumber, int type)
{
	int j;
	int member;
	int holding;

	if (guildnumber<0 || guildnumber >=MAXGUILDS) return;

	guilds[guildnumber].type = type;
    // Redone by Ripper and LB - march 19, 2000	
	//	I think the idea here is to either remove shields from ALL 
	//	(order/chaos) guilds members and replace them with opposite
	//	sheild types, or remove them alltogether
	switch(type)
	{
		case 1:	//	Convert into a order guild
			for(j=0;j<=guilds[guildnumber].members;j++)
			{
				holding=calcCharFromSer(guilds[guildnumber].member[j]);
                if (holding>-1) 
				{
					RemoveShields(holding);
					Items->SpawnItemBackpack2( calcSocketFromChar( holding ), 29, 1 );	// will not work for offline chars (Duke)
				}
			}
			Guilds->Broadcast( guildnumber, "Your guild is now an Order guild." );
			Guilds->Broadcast( guildnumber, "An order shield has been placed in your pack!" );
			break;
		case 2:	//	Convert guild into an choas guild
			for(j=0;j<=guilds[guildnumber].members;j++)
			{
				holding=calcCharFromSer(guilds[guildnumber].member[j]);
                if (holding>-1) 
				{
					RemoveShields(holding);
					Items->SpawnItemBackpack2( calcSocketFromChar( holding ), 28, 1 );
				}
			}
			Guilds->Broadcast( guildnumber, "Your guild is now a Chaos guild." );
			Guilds->Broadcast( guildnumber, "A chaos shield has been placed in your pack!" );
			break;
		case 0:	// Convert guild into a standard guild
		default:	//	Just let it fall through 
			//	Idea is to remove the items from this guilds members(Im gonna use your item loop for clarity)
			for(j = 0;j <= guilds[guildnumber].members; j++)
			{
				holding = calcCharFromSer(guilds[guildnumber].member[j]);
				if(holding>-1)
				{
					RemoveShields(holding);
				}
			}
			Guilds->Broadcast( guildnumber, "Your guild is now a Standard guild." );
			break;
	}

	// Im goging to leave this here cause I am not sure what it does.
	if (type!=0)
	{
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
               j = calcCharFromSer(guilds[guildnumber].member[member]);
               if (j>-1) chars[j].guildtoggle=1;
			}
		}
	}

}

// TESTED: OKAY
// Uhm.. do i need char or socket for sysmessage? hehe.. a socket ofcourse, FiXED iT!
// guildbroadcast(guildnumber, text) broadcasts message to all online members of guild
void cGuilds::Broadcast(int guildnumber, char *text)
{
	int member, c, s;

	if (guildnumber==-1) return;

	for (member=1;member<MAXGUILDMEMBERS;member++)
	{
		if (guilds[guildnumber].member[member]!=0)
		{
			c = calcCharFromSer(guilds[guildnumber].member[member]);
			s = calcSocketFromChar(c);
			if (online(c)) sysmessage(s,text);
		}
	}
}


// TESTED: OKAY (but not the best solution ;))
// guildSearchByStone(guildstone serial) get the guildnumber from the stones serial number
int cGuilds::SearchByStone(int s)
{
	int guildnumber;
	P_CHAR pc = MAKE_CHARREF_LRV(currchar[s], -1);

	int stone=items[pc->fx1].serial;

	if (pc->isGM())
	{
		for ( guildnumber = 1; guildnumber < MAXGUILDS; guildnumber++)
		{
			if ( guilds[guildnumber].stone == stone ) 
			{
				return guildnumber;
			}
		}
		return pc->guildnumber;
	}
	else
	{
		return pc->guildnumber;
	}
	return -1;
}

// TESTED: OKAY
// guildsearchslot(guildnumber, search type) searches for a free slot of the 4 possible types, results
// the free slot or -1 of no free slot available
// Types  1:guildslots, 2:memberslots, 3: recruitslots, 4: warslots
int cGuilds::SearchSlot(int guildnumber, int type)
{
	int counter;

	switch(type)
	{
	case 1:												// guildslots
		for (counter=1; counter<MAXGUILDS; counter++)
			if (guilds[counter].free==1) return counter;
	    if (guildnumber<0 || guildnumber>=MAXGUILDS) return -1;
		break;
	case 2:												// memberslots
		for (counter=1; counter<MAXGUILDMEMBERS; counter++)
			if (guilds[guildnumber].member[counter]==0) return counter;
		break;
	case 3:												// recruitslots
		for (counter=1; counter<MAXGUILDRECRUITS; counter++)
			if (guilds[guildnumber].recruit[counter]==0) return counter;
		break;
	case 4:												// warslots
		for (counter=1; counter<MAXGUILDWARS; counter++)
			if (guilds[guildnumber].war[counter]==0) return counter;
		break;
	}
	return -1;
}


// TESTED: OKAY (i think so, but ugly -> redo it!)
// guildmastercalc(guildnumber) counts all fealty settings of all members of 'guildnumber' and sets
// a new guildmaster if there is a draw then there will be no master, til the next check ;)
void cGuilds::CalcMaster(int guildnumber)
{
	int member, member2;
	int guildmember[MAXGUILDMEMBERS]={0};
	int currentmember, currentfealty, currenthighest=0, draw=0;

	if (guildnumber<0 || guildnumber>=MAXGUILDS) return;

	for (member=1;member<MAXGUILDMEMBERS;member++)
	{
		currentmember=calcCharFromSer(guilds[guildnumber].member[member]);

		if (currentmember==-1) currentfealty=0; else currentfealty=chars[currentmember].guildfealty;

		for (member2=1;member2<MAXGUILDMEMBERS;member2++)
		{
			if ((guilds[guildnumber].member[member2]==currentfealty)&&(currentfealty!=0))
				guildmember[member2]++;
		}
	}
	for (member=1;member<MAXGUILDMEMBERS;member++)
	{
		if (guildmember[member]>guildmember[currenthighest]) {draw=0;currenthighest=member;}
		else if (guildmember[member]==guildmember[currenthighest]) draw=1;
	}
	if (draw==0) guilds[guildnumber].master=guilds[guildnumber].member[currenthighest];
	return;
}



cGuilds::~cGuilds()
{
}

// TESTED: OKAY
// guildinit() initializes the guild structure fields, marks them all as free.
// Called by: main()
cGuilds::cGuilds(void)
{
	int guildnumber, member, recruit, war;

	for (guildnumber=1;guildnumber<MAXGUILDS;guildnumber++)
	{
		guilds[guildnumber].free=1;
		guilds[guildnumber].name[0] = 0;
		guilds[guildnumber].abbreviation[0] = 0;
		guilds[guildnumber].type=0;
		guilds[guildnumber].charter[0] = 0;
		guilds[guildnumber].webpage[0] = 0;
		guilds[guildnumber].stone=0;
		guilds[guildnumber].master=0;
		guilds[guildnumber].members=0;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			guilds[guildnumber].member[member]=0;
		}
		guilds[guildnumber].recruits=0;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			guilds[guildnumber].recruit[recruit]=0;
		}
		guilds[guildnumber].wars=0;
		for (war=1;war<MAXGUILDWARS;war++)
		{
			guilds[guildnumber].war[war]=0;
		}
		guilds[guildnumber].priv=0;
	}
}


// TESTED: OKAY (fine fine.. now proove that it really works.. )
// guildtitle(viewing character socket, clicked character) displays players title string, over the name
// of clicked character, name color gets calculated from the guild relationship of both players
// Called by: textflags()
void cGuilds::Title(int s,int player2)
{
	char title[150];
	char abbreviation[5];
	char guildtype[10];
	int tl;

	P_CHAR pc_player2 = MAKE_CHARREF_LR(player2);

	if (pc_player2->guildnumber<0 || pc_player2->guildnumber>=MAXGUILDS) return;

	if ((pc_player2->guildnumber!=0)&&(pc_player2->guildtoggle==1))
	{
		strcpy(abbreviation,guilds[pc_player2->guildnumber].abbreviation);
		if (!(strcmp(abbreviation,"")))
			strcpy(abbreviation,"none");
		if (guilds[pc_player2->guildnumber].type!=0)
		{
			if (guilds[pc_player2->guildnumber].type==1) strcpy(guildtype,"Order");
			else if (guilds[pc_player2->guildnumber].type==2) strcpy(guildtype,"Chaos");
			if (strcmp(pc_player2->guildtitle,"")) sprintf(title,"[%s, %s] [%s]",pc_player2->guildtitle,abbreviation,guildtype);
			else sprintf(title,"[%s] [%s]",abbreviation, guildtype);
		}
		else
		{
			if (strcmp(pc_player2->guildtitle,"")) sprintf(title,"[%s, %s]",pc_player2->guildtitle,abbreviation);
			else sprintf(title,"[%s]",abbreviation);
		}
		tl=44+strlen(title)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		talk[3]=pc_player2->ser1;
		talk[4]=pc_player2->ser2;
		talk[5]=pc_player2->ser3;
		talk[6]=pc_player2->ser4;
		talk[7]=1;
		talk[8]=1;
		talk[9]=0;
		talk[10]=pc_player2->emotecolor1;
		talk[11]=pc_player2->emotecolor2;
		talk[12]=0;
		talk[13]=3;
		Xsend(s, talk, 14);
		Xsend(s, sysname, 30);
		Xsend(s, title, strlen(title)+1);
	}
}

// TESTED: OKAY
// guildread(character) reads the guild structure from the wsc file, copies it to the 'character' structure.
// Called by: loadnewworld()
void cGuilds::Read(int guildnumber)
{
	int war=1, member=1, recruit=1,loopexit=0;

	if (guildnumber<0 || guildnumber>=MAXGUILDS)
	{
		sprintf((char*)temp,"invalid guild infmormation entry in wppcs.wsc");
		LogError((char*)temp);
		return;
	}

	do
	{
		readw2();
		if (!strcmp(script1, "NAME")) strcpy(guilds[guildnumber].name, script2);
		else if (!strcmp(script1, "ABBREVIATION")) strcpy(guilds[guildnumber].abbreviation, script2);
		else if (!strcmp(script1, "TYPE")) guilds[guildnumber].type = str2num(script2);
		else if (!strcmp(script1, "CHARTER")) strcpy(guilds[guildnumber].charter, script2);
		else if (!strcmp(script1, "WEBPAGE")) strcpy(guilds[guildnumber].webpage, script2);
		else if (!strcmp(script1, "STONE")) guilds[guildnumber].stone = str2num(script2);
		else if (!strcmp(script1, "MASTER")) guilds[guildnumber].master = str2num(script2);
		else if (!strcmp(script1, "RECRUITS")) guilds[guildnumber].recruits = str2num(script2);
		else if (!strcmp(script1, "RECRUIT")) { if (recruit<MAXGUILDRECRUITS) { guilds[guildnumber].recruit[recruit]=str2num(script2);recruit++; } }
		else if (!strcmp(script1, "MEMBERS")) guilds[guildnumber].members=str2num(script2);
		else if (!strcmp(script1, "MEMBER")) { if (member<MAXGUILDMEMBERS) { guilds[guildnumber].member[member]=str2num(script2);member++;} }
		else if (!strcmp(script1, "WARS")) guilds[guildnumber].wars = str2num(script2);
		else if (!strcmp(script1, "WAR")) { if (war<MAXGUILDWARS) { guilds[guildnumber].war[war]=str2num(script2);war++;} }
		guilds[guildnumber].free=0;
	}
	while ( (strcmp((char*)script1,"}")) && (++loopexit < MAXLOOPS) );
}



// TESTED: OKAY
// guildwrite() writes the guild structure to the wsc file.
// Called by: worldmain()
void cGuilds::Write(FILE *wscfile)
{
	int guildnumber, member, recruit, war;

	for (guildnumber=1;guildnumber<MAXGUILDS;guildnumber++)
	{
		if (guilds[guildnumber].free==0)
		{
			fprintf(wscfile,"SECTION GUILD %i\n", guildnumber);
			fprintf(wscfile,"{\n");
			fprintf(wscfile,"NAME %s\n", guilds[guildnumber].name);
			fprintf(wscfile,"ABBREVIATION %s\n", guilds[guildnumber].abbreviation);
			fprintf(wscfile,"TYPE %i\n", guilds[guildnumber].type);
			fprintf(wscfile,"CHARTER %s\n", guilds[guildnumber].charter);
			fprintf(wscfile,"WEBPAGE %s\n", guilds[guildnumber].webpage);
			fprintf(wscfile,"STONE %i\n", guilds[guildnumber].stone);
			fprintf(wscfile,"MASTER %i\n", guilds[guildnumber].master);
			fprintf(wscfile,"MEMBERS %i\n", guilds[guildnumber].members);
			for (member=1;member<MAXGUILDMEMBERS;member++)
			{
				if (guilds[guildnumber].member[member]!=0)
				{
					fprintf(wscfile,"MEMBER %i\n", guilds[guildnumber].member[member]);
				}
			}
			fprintf(wscfile,"RECRUITS %i\n", guilds[guildnumber].recruits);
			for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
			{
				if (guilds[guildnumber].recruit[recruit]!=0)
				{
					fprintf(wscfile,"RECRUIT %i\n", guilds[guildnumber].recruit[recruit]);
				}
			}
			fprintf(wscfile,"WARS %i\n", guilds[guildnumber].wars);
			for (war=1;war<MAXGUILDWARS;war++)
			{
				if (guilds[guildnumber].war[war]!=0)
				{
					fprintf(wscfile,"WAR %i\n", guilds[guildnumber].war[war]);
				}
			}
			fprintf(wscfile,"}\n\n");
		}
	}
}
/*
int cGuilds::CheckValidPlace(int x,int y)
{
	// This is still in work.. please be patient..
	return 1;
}
*/

int cGuilds::CheckValidPlace(int s)
{
	int p,los,i;
	P_CHAR pc_currchar = MAKE_CHARREF_LRV(currchar[s],0);
	int m = findmulti(pc_currchar->pos); 
	if (m==-1) return 0;
	if (!ishouse(m)) return 0;
	
	p=packitem(currchar[s]);
	if(p>-1)
	{
		los=0;
		for (int j=0;j<contsp[items[p].serial%HASHMAX].max;j++)
		{
			i=contsp[items[p].serial%HASHMAX].pointer[j];
			if (i!=-1) 
				if (items[i].type==7 && calcserial(items[i].more1,items[i].more2,items[i].more3,items[i].more4)==items[m].serial)
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
	} else if (m==-1)
		sysmessage(s, "You must be close to a house and have a key in your pack to place that.");
	return 0;
}


/// LB 11'th NOV 2000
/// correkt incorrect guild settings
/// called at startup
void cGuilds::CheckConsistancy(void )
{
   int guildnumber,members,serial,ok=1,i,error=0;
   P_CHAR pc_a;

   clConsole.send("checking guild data consistancy\n");

   //////////// check the guilds

   for (guildnumber=1; guildnumber<MAXGUILDS; guildnumber++)
   {

	   if (guilds[guildnumber].free==0)
	   {

	     // is the guildmaster still alive ?

	     ok=1;
	     serial=guilds[guildnumber].master;
	     i = calcCharFromSer( serial );

	     if (i==-1) // if not, erase the guild !
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
	        i = calcItemFromSer( serial );

		    if (i==-1)
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
        	   i = calcCharFromSer( serial );

			   if (i==-1 && serial !=0)
			   {
                  ok=0;							  
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
   for (iter_char.Begin(); iter_char.GetData() != NULL; iter_char++)
   {
     ok=1;
	 pc_a = iter_char.GetData();	
	 if (pc_a == NULL) return;
	 
	 if (pc_a->isPlayer())
     {

	    if (pc_a->guildnumber <0 || pc_a->guildnumber>=MAXGUILDS) // invalid guildnumber ?
		{  
		   sprintf((char*)temp,"player %s has invalid guild info. cancled his/her guild membership",pc_a->name);
		   LogWarning((char*)temp);
		   pc_a->guildnumber=0;
		   pc_a->guildfealty=0;
		   strcpy(pc_a->guildtitle,"");
		   pc_a->guildtoggle=0;
		   ok=0;
		}

	    if (ok)
		{
           if (guilds[pc_a->guildnumber].free && pc_a->guildnumber!=0)
		   {
			  sprintf((char*)temp,"player %s belongs to a guild that is no more. cancled his/her guild membership",pc_a->name);
			  LogWarning((char*)temp);
			  pc_a->guildnumber=0;
		      pc_a->guildfealty=0;
		      strcpy(pc_a->guildtitle,"");
		      pc_a->guildtoggle=0;
		      ok=0;
		   }
			   
		}

	 }      
	 if (!ok) error=1;
   }

   if (error) clConsole.send("guild releated errors detected and corrected, check logs\n"); else clConsole.send("no errors found\n");

}
