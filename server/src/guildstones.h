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
//	Wolfpack Homepage: http://wpdev.sf.com/
//========================================================================================

#if !defined (__GUILDSTONES_H__)
#define __GUILDSTONES_H__

// System Includes
#include <stdio.h>

// Wolfpack Includes
#include "typedefs.h"

#define	MAXGUILDS 100
#define	MAXGUILDMEMBERS 30
#define	MAXGUILDRECRUITS 30
#define	MAXGUILDWARS 10
#define MAXMEMRECWAR 30 // max of max guildmembers, max recruits and max war
#define DEFAULTCHARTER "WOLFPACK Guildstone"


struct guild_st
{
	bool	free;							// Guild slot used?
	char	name[41];						// Name of the guild
	char	abbreviation[4];				// Abbreviation of the guild
	int		type;							// Type of guild (0=standard/1=chaos/2=order)
	char	charter[51];					// Charter of guild
	char	webpage[51];					// Web url of guild
	SERIAL	stone;							// The serial of the guildstone
	SERIAL	master;							// The serial of the guildmaster
	int		recruits;						// Amount of recruits
	int		recruit[MAXGUILDRECRUITS + 1];	// Serials of candidates
	int		members;						// Amount of members
	int		member[MAXGUILDMEMBERS + 1];	// Serials of all the members
	int		wars;							// Amount of wars
	int		war[MAXGUILDWARS + 1];			// Numbers of Guilds we have declared war to
	int		priv;							// Some dummy to remember some values
};

class cGuilds
{
private:
	void EraseMember(int c);
	void EraseGuild(int guildnumber);
	void ToggleAbbreviation(int s);
	int SearchSlot(int guildnumber, int type);
	void ChangeName(int s, char *text);
	void ChangeAbbreviation(int s, char *text);
	void ChangeTitle(int s, char *text);
	void ChangeCharter(int s, char *text);
	void ChangeWebpage(int s, char *text);
	int CheckValidPlace(int s);
	void Broadcast(int guildnumber, char *text);
	void CalcMaster(int guildnumber);
	void SetType(int guildnumber, int type);
public:
	guild_st guilds[MAXGUILDS]; //lb, moved from WOLFPACK.h cauz global variabels cant be changed in constuctors ...
	cGuilds();
	virtual ~cGuilds();
	int	GetType(int guildnumber);
	void StonePlacement(int s);
	void Menu(int s, int page);
	void Resign(int s);
	void Recruit(int s);
	void TargetWar(int s);
	void StoneMove(int s);
	int Compare(P_CHAR player1, P_CHAR player2);
	void GumpInput(int s, int type, int index, char *text);
	void GumpChoice(int s, int main, int sub);
	int SearchByStone(int s);
	void Title(int s, int player2);
	void Read(int guildnumber);
	void Write(FILE *wscfile);
	void CheckConsistancy(void);
};

#endif //__GUILDSTONES_H__
