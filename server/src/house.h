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

#if !defined(__HOUSE_H__)
#define __HOUSE_H__

#include "wolfpack.h"
#include "globals.h"
#include "items.h"
//#include "typedefs.h"

// System Headers
//#include <vector>

// Forward Class declaration
class ISerialization;

class cHouse : public cItem
{
protected:
	std::vector<SERIAL> friends;
	std::vector<SERIAL> bans;
public:

	unsigned int last_used;

	cHouse() {}
	virtual ~cHouse() {}
	virtual void Serialize( ISerialization &archive );
	virtual QString objectID() const;
	
	bool isBanned(P_CHAR pc);
	void addBan(P_CHAR pc);
	void removeBan(P_CHAR pc);
	bool isFriend(P_CHAR pc);
	void addFriend(P_CHAR pc);
	void removeFriend(P_CHAR pc);
};

void BuildHouse(UOXSOCKET s, int i);
void RemoveKeys(SERIAL serial);
int check_house_decay();

#endif // __HOUSE_H__
