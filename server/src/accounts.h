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

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__
//Platform specifics
#include "platform.h"


//System include

#include <fstream>
#include <string>
#include <map>
#include <sys/stat.h>

using namespace std;

// Third Party


// Forward Class declaration
class cAccount;


// Wolfpack Includes

#include "wolfpack.h"
#include "debug.h"



// Authenticate return codes

#define LOGIN_NOT_FOUND -3
#define BAD_PASSWORD -4
#define ACCOUNT_BANNED -5
#define ACCOUNT_WIPE -6


using namespace std;
//##ModelId=3C5D932E0226
struct account_st
{
	//##ModelId=3C5D932E0258
	unsigned int number;
	//##ModelId=3C5D932E0276
	string name;
	//##ModelId=3C5D932E029F
	string pass;
	//##ModelId=3C5D932E02B2
	bool ban;
	//##ModelId=3C5D932E02C6
	bool remoteadmin;
};

//##ModelId=3C5D932E038F
class cAccount
{
private:
	//##ModelId=3C5D932F000D
	map<string, account_st> acctlist;
	//##ModelId=3C5D932F0084
	map<int, string> acctnumbers_sp;

	//##ModelId=3C5D932F0368
	struct acctman_st {
		//##ModelId=3C5D932F0386
		bool online;
		//##ModelId=3C5D932F03A5
		SERIAL character;
	};
	//##ModelId=3C5D932F00E8
	map<int, acctman_st> acctman;
	//##ModelId=3C5D932F0105
	int lastusedacctnum;
	//##ModelId=3C5D932F0123
	unsigned int unsavedaccounts;
	//##ModelId=3C5D932F0141
	unsigned int saveratio;

	//##ModelId=3C5D932F015F
	void LoadAccount ( int acctnumb );
public:
	//##ModelId=3C5D932F017D
	unsigned int lasttimecheck;

	//##ModelId=3C5D932F01AF
	cAccount( void );
	//##ModelId=3C5D932F01B9
	~cAccount( void );
	//##ModelId=3C5D932F01CE
	void SetSaveRatio ( int );
	//##ModelId=3C5D932F01E2
	bool IsOnline( int );
	//##ModelId=3C5D932F01F6
	SERIAL GetInWorld( int );
	//##ModelId=3C5D932F020A
	void SetOnline( int, CHARACTER );
	//##ModelId=3C5D932F0228
	void SetOffline( int acctnum );
	//##ModelId=3C5D932F0246
	void LoadAccounts( void );
	//##ModelId=3C5D932F025A
	void SaveAccounts( void );
	//##ModelId=3C5D932F026E
	void CheckAccountFile(void);
	//##ModelId=3C5D932F0282
	int Count();
	//##ModelId=3C5D932F028C
	bool RemoteAdmin(int acctnum);
	//##ModelId=3C5D932F02A0
	signed int Authenticate(string username, string password);
	//##ModelId=3C5D932F02C8
	unsigned int CreateAccount(string username, string password);
	//##ModelId=3C5D932F02E6
	bool ChangePassword(unsigned int number, string password);
};


#endif // __ACCOUNTS_H__

