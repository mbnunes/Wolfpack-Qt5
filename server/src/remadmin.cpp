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


// Original Idea and implementation from NoX-Wizard Emu, also released under GPL.
// Extended and embedded by Correa
/*********************************************************************************
 Remote Administration Console
 by Xanathar, June 2K1

 With this you can administer your server at distance by only using a standard
 telnet client. Support for both the standard Windows and Linux telnet clients :)

 *********************************************************************************/

#include "wolfpack.h"
#include "verinfo.h"
#include "accounts.h"

#include "debug.h"
#include "speech.h"
#include "dragdrop.h"
#include "sregions.h"
#include "rcvpkg.h"
#include "SndPkg.h"
#include "worldmain.h"
#include "srvparams.h"

#include <iostream>

using namespace std;

#ifdef __BEOS__
#ifndef __unix__
#define __unix__
#endif
#endif

#if defined(__unix__)
// for all 
#include <unistd.h>
#include <sys/ioctl.h>
// for linux only
#ifndef __BEOS__
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#endif
// for BeOS only
#ifdef __BEOS__
#include <be/NetKit.h>
#include <be/NetworkKit.h>
#include <be/net/socket.h>
#endif

#endif





#define MAXRACLIENT 128
#define MAXLENGHT 128

// status of the connections :)
// actually no username is ever required, and there is only one common password
#define RACST_CHECK_USR 0
#define RACST_CHECK_PWD 1
#define RACST_STDIN		2
#define RACST_ACCESS_DENIED -1 



// static char usernames[MAXRAsockets][MAXRAsockets];
static char inputbufs[MAXRACLIENT][MAXRACLIENT];
static int  inputptrs[MAXRACLIENT];
static int  sockets[MAXRACLIENT];
static int  status[MAXRACLIENT];
static string username[MAXRACLIENT];
static int  racSocket;
static int  rac_port = 2594;
static int raclen_connection_addr;
static struct sockaddr_in racconnection;
struct sockaddr_in rac_sockets_addr;

static int racnow = 0;

void racInit(void);
void racCheckInp(void);
void racCheckConn(void);
static void racPrintf(int sock, char *fmt, ...);
static void racDisconnect(int sock);
static void racRcv(int sock);
static void racProcessInput(int s);

// Leave enabled for now
#define g_nUseCharByCharMode true

void racInit(void)
{
	int bcode;
	clConsole.send("Initializing remote administration server...");
	racSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (racSocket < 0)
	{
		clConsole.send("[FAIL]\nERROR: Unable to create RAC socket\n");
		return;
	}
	
	rac_port = SrvParams->ra_port();
	
	
	raclen_connection_addr = sizeof(struct sockaddr_in);
	racconnection.sin_family = AF_INET;
	racconnection.sin_addr.s_addr = INADDR_ANY;	
	racconnection.sin_port = (((rac_port%256) << 8)) + (rac_port >> 8); // Port
	
	bcode = bind(racSocket, (struct sockaddr *)&racconnection, raclen_connection_addr);
	
	if (bcode != 0)
	{
#ifndef __unix__
		bcode = WSAGetLastError();
#endif
		clConsole.send("[FAIL]\nERROR: Unable to bind RAC socket - Error code: %i\n", bcode);
		return;
	}
    
	bcode = listen(racSocket, MAXRACLIENT);
	
	if (bcode != 0)
	{
		clConsole.send("[FAIL]\nERROR: Unable to set RAC socket in listen mode  - Error code: %i\n", bcode);
		return;
	}
	
#if defined(__unix__)
	ioctl(racSocket, FIONBIO, 0);
#else
	ioctlsocket(racSocket, FIONBIO, 0);
#endif
	
	clConsole.send("[ OK ]\n");
}

void racCheckConn(void)
{
	int s;
	socklen_t len;
	
	if (racnow >= MAXRACLIENT)
		return;
	
	FD_ZERO(&conn);
	FD_SET(racSocket, &conn);
	nfds = racSocket + 1;
	
	s = select(nfds, &conn, NULL, NULL, &uoxtimeout);	
	
	if (s <= 0)
		return;
	
	clConsole.send("RAC : connecting sockets...");
	len = sizeof(struct sockaddr_in);
	sockets[racnow] = accept(racSocket, (struct sockaddr *)&rac_sockets_addr, &len); 
	if ((sockets[racnow] < 0))
	{
		clConsole.send("[FAIL]\n");
		return;
	}
	if (Network->CheckForBlockedIP(rac_sockets_addr))
	{
		clConsole.send("[ BLOCKED ] IP Address: %s\n", inet_ntoa(rac_sockets_addr.sin_addr));
		closesocket(sockets[racnow]);
		return;
	}
	
	clConsole.send("[ OK ]\n");
	
	status[racnow] = RACST_CHECK_USR;
	
	inputptrs[racnow] = 0;
	
	// In terms of security, it's not wise to advertise the OS to everyone
	racPrintf(racnow, "Remote Administration Console\r\n");
	racPrintf(racnow, "username : ");
	racnow++;
}

void racCheckInp(void)
{
	int s, i, oldnow;
	int lp, loops;     // Xan : rewritten to support more than 64 concurrent socketss
	
	oldnow = racnow;
	loops = racnow / 64; // xan : we should do loops of 64 players
	
	for (lp = 0; lp <= loops; lp++)
	{
		FD_ZERO(&all);
		FD_ZERO(&errsock);
		nfds = 0;
		
		for (i = 0 + (64*lp); i<((lp < loops) ? 64 : oldnow); i++)
		{
			FD_SET(sockets[i], &all);
			FD_SET(sockets[i], &errsock);
			if (sockets[i] + 1>nfds)
				nfds = sockets[i] + 1;
		}
		
		s = select(nfds, &all, NULL, &errsock, &uoxtimeout);
		
		if (s>0)
		{
			for (i = 0 + (64*lp); i<((lp < loops) ? 64 : oldnow); i++)
			{
				if (FD_ISSET(sockets[i], &errsock))
				{
					racDisconnect(i);
				}
				
				
				if ((FD_ISSET(sockets[i], &all)) && (oldnow == racnow))
				{
					racRcv(i);
				}
			}
		}
	}
}

void racDisconnect(int s)
{
	closesocket(sockets[s]);
	
	clConsole.send("RAC : disconnecting socket...[ OK ]\n");
	// if it was the last socket opened, just throw it away :)
	if (racnow == s + 1)
	{
		racnow--;
		return;
	}
	
	// nope, it isn't the last.. so make it be the last!!
	
	int t = racnow - 1;
	
	for (int i = 0; i < MAXLENGHT; i++)
		inputbufs[s][i] = inputbufs[t][i];
	
	inputptrs[s] = inputptrs[t];
	sockets[s] = sockets[t];
	status[s] = status[t];
	
	racnow--;
}

void racRcv(int s)
{
	char buff[1024];
	int n, i = 0;
	
	n = recv(sockets[s], buff, 1024, 0);
	
	if (n <= 0 ) 
	{
		racDisconnect(s);
		return;
	}
	
	while (i < n)
	{
		if ((buff[i] == '\n') || (buff[i] == '\r'))
		{ 
			if (g_nUseCharByCharMode)
				racPrintf(s, "\n\r");
			if (inputptrs[s]>0)
			{
				racPrintf(s, "\n\r");
				racProcessInput(s); 
			}
			if (status[s] == RACST_STDIN)
				racPrintf(s, "%s $", SrvParams->serverList()[0].sServer.c_str());
			
			break;
		}
		if (inputptrs[s]>MAXLENGHT - 3)
		{
			i++;
			continue;
		}
		
		inputbufs[s][inputptrs[s]++] = buff[i];
		
		if (status[s] == RACST_ACCESS_DENIED)
			racDisconnect(s);
		
		if (g_nUseCharByCharMode)
		{
			if (status[s] != RACST_CHECK_PWD)
				racPrintf(s, "%c", inputbufs[s][inputptrs[s] - 1]);
			else 
				racPrintf(s, "*");
		}
		i++;
	}
}


void racPrintf(int s, char *txt, ...) // System message (In lower left corner)
{
	if (s==-1)
		return;
	va_list argptr;
	char msg[512];
	va_start(argptr, txt);
	vsprintf(msg, (char *)txt, argptr);
	va_end(argptr);
	send(sockets[s], msg, strlen((char*)msg) + 1, 0);
}

void racProcessInput(int s)
{
	char inp[1024];
	
	int i, j = 0;
	for (i = 0; i < inputptrs[s]; i++)
	{
		if (inputbufs[s][i] != '\x8') 
			inp[j++] = inputbufs[s][i];
		else 
			j = max(0, j-1);
	}
	
	inp[j] = '\0';
	
	string inputbuffer(inp);
	
	inputptrs[s] = 0;
	
	switch (status[s])
	{
	case RACST_CHECK_USR:
		username[s] = inputbuffer;
		racPrintf(s, "password : ");
		status[s] = RACST_CHECK_PWD;
		return;
	case RACST_CHECK_PWD:
		if (!Accounts->RemoteAdmin(Accounts->Authenticate(username[s], inputbuffer)))
		{
			racPrintf(s, "\r\nAccess Denied.");
			status[s] = RACST_ACCESS_DENIED;
			racDisconnect(s);
			return;
		}
		else 
		{
			status[s] = RACST_STDIN;
			racPrintf(s, "\r\n", SrvParams->serverList()[0].sServer.c_str());
			racPrintf(s, "\r\n------------------------------------------------------------------------\n\r");
			racPrintf(s, "Welcome to the %s administration console\n\r", SrvParams->serverList()[0].sServer.c_str());
			racPrintf(s, "\r\nType HELP to receive help on commands.\r\n");
			racPrintf(s, "\n\r");
			return;
		}
		break;
	case RACST_STDIN:
		if (inputbuffer == "pdump")
		{
			racPrintf(s, "Performace Dump:\r\n");
		
			sprintf((char*)temp, "Network code: %fmsec [%i]" _(float)((float)networkTime/(float)networkTimeCount) _ networkTimeCount);
			racPrintf(s, "%s\r\n", (char*) temp);
		
			sprintf((char*)temp, "Timer code: %fmsec [%i]" _(float)((float)timerTime/(float)timerTimeCount) _ timerTimeCount);
			racPrintf(s, "%s\r\n", (char*) temp);
		
			sprintf((char*)temp, "Auto code: %fmsec [%i]" _(float)((float)autoTime/(float)autoTimeCount) _ autoTimeCount);
			racPrintf(s, "%s\r\n", (char*) temp);
		
			sprintf((char*)temp, "Loop Time: %fmsec [%i]" _(float)((float)loopTime/(float)loopTimeCount) _ loopTimeCount);
			racPrintf(s, "%s\r\n", (char*) temp);
		
			sprintf((char*)temp, "Simulation Cycles/Sec: %f" _(1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
			racPrintf(s, "%s\r\n", (char*) temp);
			return;
		}
		else if (inputbuffer == "who")
		{
			if (now == 0)
			{
				racPrintf(s, "There are no users connected.\n\r");
				return;
			}
		
			int i, j = 0;
			racPrintf(s, "Current Users in the World:\r\n");
			for (i = 0; i < now; i++)
			{
				if (perm[i]) // Keeps NPC's from appearing on the list
				{
					j++;
					sprintf((char*)temp, "    %i) %s [%8x]\r\n", (j - 1), currchar[i]->name.c_str(), currchar[i]->serial);
					racPrintf(s, (char*)temp);
				}
			}	
			sprintf((char*)temp, "Total Users Online: %d\n\r", j);
			racPrintf(s, (char*)temp);
			racPrintf(s, "End of userlist\r\n");
			return;
		}	
		else if (inputbuffer == "quit" || inputbuffer == "exit")
		{
			racDisconnect(s);
			return; 
		}
		else if (inputbuffer == "help" || inputbuffer == "?")
		{
			racPrintf(s, "! <msg>              - broadcasts <msg> to everyone\r\n");
			racPrintf(s, "addacct <name>,<pwd> - creates a new account\r\n");
			racPrintf(s, "broadcast <msg>      - broadcasts <msg> to everyone\r\n");
			racPrintf(s, "exit                 - close the connection\r\n");
			racPrintf(s, "pdump                - displays performance dump\r\n");
			racPrintf(s, "quit                 - close the connection\r\n");
			racPrintf(s, "save                 - saves the world\r\n");
			racPrintf(s, "shutdown             - shuts the server\r\n");
			racPrintf(s, "reload               - reloads server's scripts and ini file\r\n");
			racPrintf(s, "who                  - displays logon players\r\n");
			return;
		}
		else if (inputbuffer == "save")
		{
			if (!cwmWorldState->Saving())
			{
				clConsole.send("Saving worldfile...");
				racPrintf(s, "Saving worldfile...");
				cwmWorldState->savenewworld(SrvParams->worldSaveModule());
				SrvParams->flush();
				clConsole.send("Done!\n");
				racPrintf(s, "[DONE]\r\n");
			}	
			return;
		}
		else if (inputbuffer == "shutdown")
		{
			clConsole.send("WOLFPACK: Immediate Shutdown initialized from RAC!\n");
			racPrintf(s, "Bye! :)\r\n\r\n");
			keeprun = 0;
			return;
		}
		else if (inputbuffer.find("broadcast") == 0 || inputbuffer.find("!") == 0 )
		{
			string param;
			int pos = inputbuffer.find_first_of(" ")+1;
			if (pos != 0)
				param = inputbuffer.substr(pos);
			if ((param.length() == 0))
			{
				racPrintf(s, "Syntax is : BROADCAST <message>\r\nExample : BROADCAST Warning restarting server!\r\n\n");
				return;
			}
			sysbroadcast((char*)param.c_str()); // System broadcast in bold text
		}
		else if (inputbuffer.find("addacct") == 0)
		{
			string param1, param2;
			int pos = inputbuffer.find_first_of(" ")+1;
			if (pos != 0)
				param1 = inputbuffer.substr(pos, inputbuffer.size()-inputbuffer.find_first_of(",")-1);
			pos = inputbuffer.find_first_of(",")+1;
			if (pos != 0)
				param2 = inputbuffer.substr(pos);
			if (param1.length() == 0 || param2.length() == 0)
			{
				racPrintf(s, "Syntax is : ADDACCT <name>,<password>\r\nExample : ADDACCT administrator,password\r\n");
				return;
			}
			int acct = Accounts->CreateAccount(param1, param2);
			racPrintf(s, "Account %d created\r\n  Name : %s\r\n  Pass : %s\r\n\n", acct, (char*)param1.c_str(), (char*)param2.c_str());
			return;
		}
		else if (inputbuffer == "reload")
		{
			racPrintf(s, "WOLFPACK: Reloading accounts file...");
			Accounts->LoadAccounts();
			racPrintf(s, "Done!\r\n");
			racPrintf(s, "WOLFPACK: Reloading Server.scp, Spawn.scp, and Regions.scp....");
			loadspawnregions();
			loadregions();
			loadmetagm();
			loadmenuprivs();
			SrvParams->reload();
			racPrintf(s, " Done!\r\n");
			racPrintf(s, "WOLFPACK: Reloading IP Blocking rules...");
			Network->LoadHosts_deny();
			racPrintf(s, "Done\r\n");
		}
		else
			racPrintf(s, "Command unknown. Please do not use spaces before/after the command.\r\n\n");
		break;
	default:
		return;
	}	
}





