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


#ifndef __NETWORK_H__
#define __NETWORK_H__
//Platform specifics
#include "platform.h"

// System Includes

#include <iostream>

#include "packlen.h"

using namespace std ;

//Forward class Declaration
class cNetworkStuff;



//Wolfpack includes

//#include "netsys.h"
#include "wolfpack.h"
#include "verinfo.h"
#include "debug.h"
#include "speech.h"
#include "dragdrop.h"
#include "sregions.h"
#include "rcvpkg.h"
#include "SndPkg.h"


/*
#if defined(__unix__)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#endif
*/
#include "utilsys.h"


#include "typedefs.h"

class cNetworkStuff
{
public:
	cNetworkStuff();

	void startchar(UOXSOCKET s);
	void LoginMain(UOXSOCKET s);
	void xSend(UOXSOCKET s, void *point, int length, int test);
	void Disconnect(UOXSOCKET s);
	void ClearBuffers();
	void CheckConn();
	void CheckMessage();
	void SockClose();
	void FlushBuffer(UOXSOCKET s);
	void LoadHosts_deny( void );
	void Initialize(void) { sockInit(); }

	int kr,faul; // needed because global varaibles cant be changes in constructores LB
	bool CheckForBlockedIP(sockaddr_in ip_address);

private:
	char pass1[256];
	char pass2[256];

	void SendUOX3(UOXSOCKET s, void *point, int length, int test);
	void SendOSI(UOXSOCKET s, void *point, int length, int test);
	void SendSMARTWOLF(UOXSOCKET s, void *point, int length, int test);
	void CountPackets(UOXSOCKET s, int &numpackets, long int & offsetlastfullpacket, bool & dataerror);
    bool CheckPacket(UOXSOCKET s, unsigned char packetnumber, int length, unsigned long int offset);
	void SendGoodByeMessageRaw(UOXSOCKET s);

	std::vector<ip_block_st> hosts_deny;

	signed long Authenticate( const char *username, const char *pass );
	void DoStreamCode(UOXSOCKET s);
	int  Pack(void *pvIn, void *pvOut, int len);
	void Login2(int s);
	void Relay(int s);
	void GoodAuth(int s);
	void charplay (int s);
	void CharList(int s);
	int  Receive(int s, int x, int a);
	void GetMsg(int s);
	char LogOut(int s);
	void pSplit(char *pass0);
	void sockInit();

protected:

};

// Helper Function
UOXSOCKET calcSocketFromChar(P_CHAR pc);


#endif
