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

// Platform Includes
#include "platform.h"

// System Includes
#include <stdio.h>
#include <string>
#include <vector>

// Wolfpack Includes
#include "items.h"
#include "typedefs.h"

// Forward Class
class ISerialization;

#define	MAXGUILDS 100
#define	MAXGUILDMEMBERS 30
#define	MAXGUILDRECRUITS 30
#define	MAXGUILDWARS 10
#define MAXMEMRECWAR 30 // max of max guildmembers, max recruits and max war
#define DEFAULTCHARTER "WOLFPACK Guildstone"


class cGuildStone : public cItem
{
public:
	enum enGuildType { standard = 0, chaos, order };
	std::string			guildName;
	std::string			abbreviation;
	enGuildType			guildType;
	std::string			charter;
	std::string			webpage;
	SERIAL				master;
	std::vector<SERIAL>	recruit;
	std::vector<SERIAL>	member;
	std::vector<SERIAL>	war;

public:
	cGuildStone() {}
	virtual ~cGuildStone() {}
	virtual void Serialize( ISerialization &archive );
	virtual std::string objectID();
	
	void addMember(P_CHAR);
	bool isMember(P_CHAR);
	void removeMember(P_CHAR);
	void Menu(UOXSOCKET s, int page);
	void CalcMaster();
	void ToggleAbbreviation(UOXSOCKET s);
	void Recruit(UOXSOCKET s);
	void GumpInput(UOXSOCKET s, int type, int index, char *text);
	void GumpChoice(UOXSOCKET s,int main,int sub);
	void ChangeName(UOXSOCKET s, char *text);
	void ChangeAbbreviation(UOXSOCKET s, char *text);
	void ChangeTitle(UOXSOCKET s, char *text);
	void ChangeCharter(UOXSOCKET s, char *text);
	void ChangeWebpage(UOXSOCKET s, char *text);
	enGuildType cGuildStone::GetType();
	void SetType(enGuildType type);
	void Broadcast(char *text);
};

inline std::string cGuildStone::objectID()
{
	return std::string("GUILDSTONE");
}

int CheckValidPlace(UOXSOCKET s);
void GuildResign(UOXSOCKET s);
int GuildCompare(P_CHAR player1, P_CHAR player2);
void StonePlacement(UOXSOCKET s);
void GuildTitle(int s, P_CHAR pc_player2);

#endif //__GUILDSTONES_H__
