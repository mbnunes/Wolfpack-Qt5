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
struct account_st
{
	unsigned int number;
	string name;
	string pass;
	bool ban;
	bool remoteadmin;
};

class cAccount
{
private:
	map<string, account_st> acctlist;
	map<int, string> acctnumbers_sp;

	struct acctman_st {
		bool online;
		CHARACTER character;
	};
	map<int, acctman_st> acctman;
	int lastusedacctnum;
	unsigned int unsavedaccounts;
	unsigned int saveratio;

	void LoadAccount ( int acctnumb );
public:
	unsigned int lasttimecheck;

	cAccount( void );
	~cAccount( void );
	void SetSaveRatio ( int );
	bool IsOnline( int );
	CHARACTER GetInWorld( int );
	void SetOnline( int, CHARACTER );
	void SetOffline( int acctnum );
	void LoadAccounts( void );
	void SaveAccounts( void );
	void CheckAccountFile(void);
	int Count();
	bool RemoteAdmin(int acctnum);
	signed int Authenticate(string username, string password);
	unsigned int CreateAccount(string username, string password);
	bool ChangePassword(unsigned int number, string password);
};


#endif // __ACCOUNTS_H__

