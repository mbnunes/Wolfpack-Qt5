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



#include "accounts.h"

#undef DBGFILE
#define DBGFILE "accounts.cpp"

cAccount::cAccount(void)
{
	unsavedaccounts = 0;
	saveratio = 0;         // Save everyaccount
}

cAccount::~cAccount(void)
{
	if (unsavedaccounts > 0)
		SaveAccounts();
}

bool cAccount::findByNumber( int number, account_st* retValue )
{
	map<int, string>::iterator it = acctnumbers_sp.find( number );
	if ( it != acctnumbers_sp.end() )
	{
		map<string, account_st>::iterator it2 = acctlist.find( (*it).second );
		if ( it2 != acctlist.end() )
		{
			retValue = &(*it2).second;
			return true;
		}
	}

	return false; // All other paths
}

void cAccount::LoadAccount( int acctnumb )
{
	unsigned long loopexit=0;
	account_st account;
	account.name.erase();
	account.pass.erase();
	account.ban = false;
	account.remoteadmin = false;
	account.number = acctnumb;
	do
	{
		read2();
		
		if (!strcmp((char*)script1, "NAME"))				account.name =  script2;
		else if (!strcmp((char*)script1, "PASS"))			account.pass = script2;
		else if (!strcmp((char*)script1, "BAN"))			account.ban = true;
		else if (!strcmp((char*)script1, "REMOTEADMIN"))	account.remoteadmin = true;
	}
	while ( (strcmp((char*)script1, "}")) && (strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS) );
	acctlist.insert(make_pair(account.name, account));
	acctnumbers_sp.insert(make_pair(account.number, account.name));
	lasttimecheck = uiCurrentTime;
}

int cAccount::Count()
{
	return acctlist.size();
}

void cAccount::LoadAccounts( void )
{
	int b,c,ac, account,loopexit=0;
	char accnumb[64]; 		
	char acc[64];
	char *t;
	lastusedacctnum = 0;

	openscript("accounts.adm");
	acctlist.clear();
	do
	{
		read2();	
		if (!(strcmp((char*)script1, "SECTION")))
		{
			   
			c = strlen((char*)script2);
			for (b=0; b<9; b++) acc[b]=script2[b]; 
			for (b=8; b<c; b++) accnumb[b-8]=script2[b];
			accnumb[b-8]=0; acc[8]=0;		
			ac = strtol(accnumb, &t, 10);
			if (strlen(t)!=0) ac=-1;

			if (strcmp(acc,"ACCOUNT ") || ac < 0 )
			{
				clConsole.send("Error loading accounts, skipping invalid account entry!\n");

			} else {
			   account=ac;
			   LoadAccount(account);
			   if (account > lastusedacctnum)
				   lastusedacctnum = account;
			}
		}
	}
	while ( (strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS) );
	closescript();	
}

void cAccount::SaveAccounts( void )
{
	map<string, account_st>::iterator iter_account;
	account_st account;
	fstream faccount ;
	string  line ;
	faccount.open("accounts.adm",ios::out) ;
	unsigned int maxacctnumb = 0;     // Saving the number of loaded accounts
	if (faccount.is_open())
	{
		for (iter_account = acctlist.begin(); iter_account != acctlist.end(); iter_account++)
		{
			account = iter_account->second;    
			faccount << "SECTION ACCOUNT " << account.number << endl ;
			faccount << "{" << endl ;
			faccount << "NAME " << account.name << endl ;
			faccount << "PASS " << account.pass << endl ;
			if (account.ban)
				faccount << "BAN" << endl ;
			if (account.remoteadmin)
				faccount << "REMOTEADMIN" << endl;
			faccount << "}" << endl << endl;
			if (maxacctnumb < account.number)
				maxacctnumb = account.number;
		}
		faccount << endl ;
		faccount << "// Note: Last used Account Number was: " << maxacctnumb << endl;
		faccount << "EOF" << endl ;
	}
	faccount.close();
	unsavedaccounts = 0;

	return;
}

bool cAccount::RemoteAdmin(int acctnum)
{
	if (acctnum < 0)
		return false;

	map<string, account_st>::iterator iter_account;
	map<int, string>::iterator iter_acctnumber = acctnumbers_sp.find(acctnum);
	if (iter_acctnumber == acctnumbers_sp.end())
		return false;

	if ((iter_account = acctlist.find(iter_acctnumber->second)) != acctlist.end())
		return iter_account->second.remoteadmin;
	else
		return false;
}

signed int cAccount::Authenticate(string username, string password)
{
	account_st account;
	map<string, account_st>::iterator iter_account;

	if ((iter_account = acctlist.find(username)) != acctlist.end())
	{
		account = iter_account->second;
		if( account.pass == password )
		{
			if( account.ban )
				return ACCOUNT_BANNED;
			else
				return account.number;
		} else
			return BAD_PASSWORD;
	}
	return LOGIN_NOT_FOUND;
}

unsigned int cAccount::CreateAccount(const string& username, const string& password)
{
	++lastusedacctnum;
	account_st account;
	account.name = username;
	account.pass = password;
	account.ban = false;
	account.remoteadmin = false;
	account.number = lastusedacctnum;
	acctlist.insert(make_pair(username, account));
	acctnumbers_sp.insert( make_pair( account.number, username ) );
	++unsavedaccounts;
	if (unsavedaccounts >= saveratio)
		SaveAccounts();
	return account.number;
}

void cAccount::CheckAccountFile(void)
{

	struct stat filestatus;
	static time_t lastchecked;
	
	stat("accounts.adm", &filestatus);

	if (difftime(filestatus.st_mtime, lastchecked) > 0.0)
		LoadAccounts();

	lastchecked = filestatus.st_mtime;
	lasttimecheck = uiCurrentTime;

}

bool cAccount::IsOnline( int acctnum )
{
	if (acctnum < 0)
		return false;
	map<int, acctman_st>::iterator iter_acctman;
	if ((iter_acctman = acctman.find(acctnum)) != acctman.end())
	{
		acctman_st dummy = iter_acctman->second;
		if ( dummy.online )
			return true;
		else
		{
			P_CHAR pc = FindCharBySerial(dummy.character);
			return pc->logout > uiCurrentTime;
		}
	} 
	else 
		return false;
}

SERIAL cAccount::GetInWorld( int acctnum )
{
	if (acctnum < 0)
		return -1;
	map<int, acctman_st>::iterator iter_acctman;
	if ((iter_acctman = acctman.find(acctnum)) != acctman.end())
	{
		acctman_st dummy = iter_acctman->second;
		return dummy.character;
	} else 
		return INVALID_SERIAL;
}

void cAccount::SetOnline( int acctnum, SERIAL serial)
{
	acctman_st dummy;
	dummy.online = true;
	dummy.character = serial;
	acctman.insert(make_pair(acctnum, dummy));
}

void cAccount::SetOffline( int acctnum )
{
	map<int, acctman_st>::iterator iter_acctman;
	if ((iter_acctman = acctman.find(acctnum)) != acctman.end())
	{
		acctman.erase(iter_acctman);
	}

}

bool cAccount::ChangePassword(unsigned int number, string password)
{
	account_st* account = NULL;
	if ( findByNumber( number, account ) )
	{
		account->pass = password;
		++unsavedaccounts;
		if (unsavedaccounts >= saveratio)
			SaveAccounts();
		return true;
	}
	return false;
}

vector<SERIAL> cAccount::characters( int number )
{
	account_st* account = NULL;
	if ( findByNumber( number, account ) )
	{
		return account->characters;
	}
	return vector<SERIAL>();
}

void cAccount::addCharacter( int number, SERIAL serial )
{
	account_st* account = NULL;
	if ( findByNumber( number, account ) )
	{
		account->characters.push_back( serial );
	}
}

void cAccount::removeCharacter( int number, SERIAL serial )
{
	account_st* account = NULL;
	if ( findByNumber( number, account ) )
	{
		register unsigned int i;
		for ( i = 0; i < account->characters.size(); ++i )
		{
			if ( account->characters[i] == serial )
			{
				account->characters.erase( account->characters.begin() + i );
				return;
			}
		}
	}
}

