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

//##ModelId=3C5D92D7039E
class cNetworkStuff
{
public:
	//##ModelId=3C5D92D703BC
	cNetworkStuff();

	//##ModelId=3C5D92D703C6
	void startchar(UOXSOCKET s);
	//##ModelId=3C5D92D703DA
	void LoginMain(UOXSOCKET s);
	//##ModelId=3C5D92D8002E
	void xSend(UOXSOCKET s, void *point, int length, int test);
	//##ModelId=3C5D92D8006A
	void Disconnect(UOXSOCKET s);
	//##ModelId=3C5D92D80092
	void ClearBuffers();
	//##ModelId=3C5D92D8009C
	void CheckConn();
	//##ModelId=3C5D92D800B0
	void CheckMessage();
	//##ModelId=3C5D92D800C4
	void SockClose();
	//##ModelId=3C5D92D800E2
	void FlushBuffer(UOXSOCKET s);
	//##ModelId=3C5D92D80100
	void LoadHosts_deny( void );
	//##ModelId=3C5D92D8015A
	void Initialize(void) { sockInit(); }

	int kr,faul; // needed because global varaibles cant be changes in constructores LB
	//##ModelId=3C5D92D801AA
	bool CheckForBlockedIP(sockaddr_in ip_address);

private:
	//##ModelId=3C5D92D801C8
	char pass1[256];
	//##ModelId=3C5D92D801F0
	char pass2[256];

	//##ModelId=3C5D92D8020F
	void SendUOX3(UOXSOCKET s, void *point, int length, int test);
	//##ModelId=3C5D92D80241
	void SendOSI(UOXSOCKET s, void *point, int length, int test);
	//##ModelId=3C5D92D80269
	void SendSMARTWOLF(UOXSOCKET s, void *point, int length, int test);
	//##ModelId=3C5D92D8029B
	void CountPackets(UOXSOCKET s, int &numpackets, long int & offsetlastfullpacket, bool & dataerror);
	//##ModelId=3C5D92D802C3
    bool CheckPacket(UOXSOCKET s, unsigned char packetnumber, int length, unsigned long int offset);
	//##ModelId=3C5D92D901F3
	void SendGoodByeMessageRaw(UOXSOCKET s);

	//##ModelId=3C5D92D9023A
	std::vector<ip_block_st> hosts_deny;

	//##ModelId=3C5D92D90260
	signed long Authenticate( const char *username, const char *pass );
	//##ModelId=3C5D92D9027E
	void DoStreamCode(UOXSOCKET s);
	//##ModelId=3C5D92D90292
	int  Pack(void *pvIn, void *pvOut, int len);
	//##ModelId=3C5D92D902C4
	void Login2(int s);
	//##ModelId=3C5D92D902E2
	void Relay(int s);
	//##ModelId=3C5D92D90300
	void GoodAuth(int s);   
	//##ModelId=3C5D92D9031E
	void charplay (int s);
	//##ModelId=3C5D92D9033C
	void CharList(int s);
	//##ModelId=3C5D92D9035A
	int  Receive(int s, int x, int a);
	//##ModelId=3C5D92D9038D
	void GetMsg(int s);
	//##ModelId=3C5D92D903AB
	char LogOut(int s);
	//##ModelId=3C5D92D903C9
	void pSplit(char *pass0);
	//##ModelId=3C5D92D903DD
	void sockInit();

protected:

};
#endif
