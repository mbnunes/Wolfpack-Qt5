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

// Platform Includes
#include "platform.h"

#include <string>
using namespace std ;


// New NetworkIO Testing stuff
#include "network/asyncnetio.h"
#include "network/uosocket.h"
#include "network/listener.h"

cAsyncNetIO* netio;

//Wolfpack Includes
#include "network.h"
#include "gumps.h"
#include "walking2.h"
#include "books.h"
#include "srvparams.h"
#include "classes.h"
#include "territories.h"
#include "scriptc.h"

// Library Includes
#include "qstringlist.h"

#define PACKET_LEN_DYNAMIC	0x0000
#define PACKET_LEN_NONE		0xffff

#undef  DBGFILE
#define DBGFILE "Network.cpp"

// Authenticate return codes

#define LOGIN_NOT_FOUND -3
#define BAD_PASSWORD -4
#define ACCOUNT_BANNED -5
#define ACCOUNT_WIPE -6

cNetwork *cNetwork::instance_;

cNetwork::cNetwork( void )
{
	listener_ = new cListener( SrvParams->port() );
	netIo_ = new cAsyncNetIO;

	listener_->start();
	netIo_->start();
}

cNetwork::~cNetwork( void )
{
	netIo_->cancel();
	listener_->cancel();

	delete netIo_;
	delete listener_;
}

void cNetwork::poll( void )
{
	// Check for new Connections
	if ( listener_->haveNewConnection() )
	{
		QSocketDevice *socket = listener_->getNewConnection(); 
		netIo_->registerSocket( socket );
		uoSockets.push_back( new cUOSocket(socket) );

		// Notify the admin
		clConsole.send( QString( "Socket connected [%1]\n" ).arg( socket->address().toString() ) );
	}

	// fast return
	if( uoSockets.size() < 1 )
		return;

	// Check for new Packets
	vector< cUOSocket* >::iterator uoIterator;

	for( uoIterator = uoSockets.begin(); uoIterator != uoSockets.end(); ++uoIterator )
	{
		cUOSocket *uoSocket = (*uoIterator);

		// Check for disconnected sockets
		if( !uoSocket->socket()->isValid() )
		{
			clConsole.send( QString( "Socket disconnected [%1]\n" ).arg( uoSocket->socket()->address().toString() ) );
			netIo_->unregisterSocket( uoSocket->socket() );
			uoSockets.erase( uoIterator );
		}
		else
			uoSocket->recieve();
	}	
}

// Code for wrapping old Stuff
void cNetwork::xSend( int s, const void *point, int length, int test) // Buffering send function
{
	cUOSocket *socket = uoSockets[ s ];
	
	if( !socket )
		return;

	QByteArray data( length );
	memcpy( data.data(), point, length );
	cUOPacket *packet = new cUOPacket( data );

	socket->send( packet );
}

// Load IP Blocking rules
void cNetwork::load( void )
{
}

// Reload IP Blocking rules
void cNetwork::reload( void )
{
	unload();
	load();
}

// Unload IP Blocking rules
void cNetwork::unload( void )
{
	hosts_deny.clear();
}

/*void cNetworkStuff::Disconnect (int s) // Force disconnection of player //Instalog
{
	int i;

	if (now < 0) { strcpy((char*)temp,"error in now-managment!\n"); LogError((char*)temp2); now=0; return; }
	
	time_t ltime;
	time( &ltime );

	sprintf((char*)temp,"WOLFPACK: Client %i disconnected. [Total:%i]\n",s,now-1);
	clConsole.send(temp);
	
	if (SrvParams->serverLog()) 
		savelog((char*)temp,"server.log");

	if (perm[s] && (currchar[s]->account() == acctno[s])&&(SrvParams->partMsg()))
		if (currchar[s]->isPlayer()) // bugfix lb, removes lamas that leave the realm :)
		{
			sprintf((char*)temp,"%s has left the realm",currchar[s]->name.c_str());
			sysbroadcast((char*)temp);//message upon leaving a server
		}

	if (acctno[s]!=-1) Accounts->SetOffline(acctno[s]); //Bug clearing logged in accounts!
	acctno[s]=-1;

	//Instalog
	char val=0;
	if (currchar[s] != NULL && perm[s])
	{
		if (!currchar[s]->free && online(currchar[s]))
			val=LogOut(s);
	}

	if (val)
	{
	   for (i=0;i<now;i++)
	   if ((i!=s)&&(perm[i]))
	   {
		  LongToCharPtr(currchar[s]->serial, &removeitem[1]);
	      Xsend(i, removeitem, 5);
	   }
	}

	FlushBuffer(s);

	closesocket(client[s]);

	int j;
	for (j=s;j<now-1;j++)
	{
		client[j]=client[j+1];
		newclient[j]=newclient[j+1];
		cryptclient[j]=cryptclient[j+1];
		currchar[j]=currchar[j+1];
		acctno[j]=acctno[j+1];		
		perm[j]=perm[j+1];
		binlength[j]=binlength[j+1];
		boutlength[j]=boutlength[j+1];
		usedfree[j]=usedfree[j+1];
		itemmake[j]=itemmake[j+1];
		walksequence[j]=walksequence[j+1];
		LSD[j]=LSD[j+1];
		noweather[j]=noweather[j+1];
		firstpacket[j]=firstpacket[j+1];
		clientDimension[j]=clientDimension[j+1];

		int MB = MAXBUFFER_REAL;	

		memcpy(&buffer[j], &buffer[j+1], MB);       // probably not nec.
		memcpy(&outbuffer[j], &outbuffer[j+1], MB); // very important
		memcpy(&xtext[j], &xtext[j+1], 31);
		
		addid1[j]=addid1[j+1];
		addid2[j]=addid2[j+1];
		addid3[j]=addid3[j+1];
		addid4[j]=addid4[j+1];
		addx[j]=addx[j+1];
		addy[j]=addy[j+1];
		addz[j]=addz[j+1];
        addid5[j]=addid5[j+1];
        tempint[j]=tempint[j+1];
		dyeall[j]=dyeall[j+1];
		addx2[j]=addx2[j+1];
		addy2[j]=addy2[j+1];
		addz[j]=addz[j+1];
		addmitem[j]=addmitem[j+1];
		clickx[j]=clickx[j+1];
		clicky[j]=clicky[j+1];
		targetok[j]=targetok[j+1];
		currentSpellType[j]=currentSpellType[j+1];
			
	}

	currchar[now]=NULL;
	now--;	
}

void cNetworkStuff::LoginMain(int s)
{
	signed long int i;
	unsigned char noaccount[3]="\x82\x00";
	unsigned char acctused[3]="\x82\x01";
	unsigned char nopass[3]="\x82\x03";
	unsigned char acctblock[3]="\x82\x02";

	acctno[s]=-1;

	pSplit((char*)&buffer[s][31]);
	i = Accounts->Authenticate((char*)&buffer[s][1], (char*)pass1);

	if( i >= 0 )
		acctno[s] = i;
	else
	{
		switch(i)  //Let's check for the error message
		{
		case BAD_PASSWORD:
			Xsend(s, nopass, 2);
			//Disconnect(s); // disconnecting here is very wrong / LB
			return;
		case ACCOUNT_BANNED:
			Xsend(s, acctblock, 2);	
			return;
		case LOGIN_NOT_FOUND:
			if( !SrvParams->autoAccountCreate() )
			{
				Xsend(s, noaccount, 2);		
				return;
			} else {
				// Auto create is enable, let's create the new account.
				std::string dummylogin = (char*)&buffer[s][1], dummypass = (char*)&buffer[s][31];

				// Let's check if password isn't blank:
				if (dummypass.size() == 0)
				{
					// User forgot password, let's send a message and return
					Xsend(s, nopass, 2);
					return;
				}
				acctno[s] = Accounts->CreateAccount(dummylogin, dummypass);
			}
		}
	}

	if (Accounts->IsOnline(acctno[s]))
	{		
	  //acctinuse[acctno[s]]=0;
	  //acctno[s]=-1;
	  Xsend(s, acctused, 2);		
	  return;
	}
	if (acctno[s]!=-1)
	{
		Login2(s);
	}
}

void cNetworkStuff::Login2(int s)
{
	unsigned long int i;
	unsigned short tlen;
	unsigned long int ip;
	unsigned char newlist1[7]="\xA8\x01\x23\xFF\x00\x01";
	unsigned char newlist2[41]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x12\x01\x7F\x00\x00\x01";
    sprintf((char*)temp,"Client [%s] connected [first] using Account '%s'.\n", sock_ntop(client_addr).c_str(), &buffer[s][1]);

	if (SrvParams->serverLog())
		savelog((char*)temp,"server.log");

	vector<ServerList_st>& serv = SrvParams->serverList();

	tlen=6+(serv.size()*40);
	newlist1[1]=tlen>>8;
	newlist1[2]=tlen%256;
	newlist1[4]=serv.size()>>8;
	newlist1[5]=serv.size()%256;
	Xsend(s, newlist1, 6);

	vector<ServerList_st>::iterator it = serv.begin();
	for (i = 0; it != serv.end(); ++i,++it)
	{
		newlist2[0]=(unsigned char) ((i+1)>>8);
		newlist2[1]=(unsigned char) ((i+1)%256);

		strcpy((char*)&newlist2[2], (*it).sServer.c_str());
		ip = inet_addr((*it).sIP.c_str());
		newlist2[39]=(unsigned char) (ip>>24);
		newlist2[38]=(unsigned char) (ip>>16);
		newlist2[37]=(unsigned char) (ip>>8);
		newlist2[36]=(unsigned char) (ip%256);
		Xsend(s, newlist2, 40);
	}
}

void cNetworkStuff::Relay(int s) // Relay player to a certain IP
{
	unsigned long int ip;
	ServerList_st serv = SrvParams->serverList()[buffer[s][2]-1];
	ip = inet_addr(serv.sIP.c_str());

	login03[4]=(unsigned char) (ip>>24);
	login03[3]=(unsigned char) (ip>>16);
	login03[2]=(unsigned char) (ip>>8);
	login03[1]=(unsigned char) (ip%256);
	login03[5] = serv.uiPort>>8;
	login03[6] = serv.uiPort%256;
	srand(ip+acctno[s]+now+uiCurrentTime); // Perform randomize
	login03[7]=127;
	login03[8]=0;
	login03[9]=0;
	login03[10]=1;
	Xsend(s, login03, 11);
}

void cNetworkStuff::GoodAuth(int s)
{
	int tlen;
	unsigned int i, j;
	unsigned char login04b[61] = {0,};
	
	vector<StartLocation_st>& start = SrvParams->startLocation();
	tlen=4+(5*60)+1+(start.size()*63);
	login04a[1]=tlen>>8;
	login04a[2]=tlen%256;

//	login04a[3]=5;
//	j=0;
	
	// calc # of chars in advance, also let's save their position.
	vector<P_CHAR> accounts_chars;
	accounts_chars.clear(); // Just to make sure we get a clean vector.
	AllCharsIterator iter_char;
	for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR toCheck = iter_char.GetData();
		if ( toCheck->account() == acctno[s] && !toCheck->free )			
		{		
			accounts_chars.push_back(toCheck);
		}
	}

	login04a[3] = accounts_chars.size(); //Number of characters found
	if (login04a[3]>5)
	{
		login04a[3]=5;	
		LogErrorVar ("more than 5 chars on account# %i\n", acctno[s]);
	}
	Xsend(s, login04a, 4);

	j=0;

	for (i = 0; i < accounts_chars.size(); i++)
	{
		strcpy((char*)login04b, accounts_chars[i]->name.c_str());
		Xsend(s, login04b, 60);
		j++;
	}

	memset(&login04b[0], 0, 60*sizeof(unsigned char));
	//for(i = 0; i<60;i++)login04b[i] = 0;

	for (i=j;i<5;i++)
	{
		Xsend(s, login04b, 60);
	}

	buffer[s][0] = start.size();
	Xsend(s, buffer[s], 1);

	for ( i = 0; i < start.size(); ++i )
	{
		login04d[0] = static_cast<unsigned char>(i);
		strncpy( (char*)&login04d[1], start[i].name.latin1(), 30 );
		strncpy( (char*)&login04d[32], start[i].name.latin1(), 30 );
		login04d[31] = login04d[61] = 0;
		Xsend(s, login04d, 63);
	}

	//Instalog really necessary?
}

void cNetworkStuff::CharList(int s) // Gameserver login and character listing
{
	signed long int i;
	unsigned char noaccount[3]="\x82\x00";
	unsigned char nopass[3]="\x82\x03";
	unsigned char acctblock[3]="\x82\x02";

	acctno[s]=-1;

	pSplit((char*)&buffer[s][35]);
	i = Authenticate((char*)&buffer[s][5], pass1);

	if( i >= 0 )
		acctno[s] = i;
	else
	{
		switch(i)  //Let's check for the error message
		{
		case BAD_PASSWORD:
			Xsend(s, nopass, 2);
			//Disconnect(s); // disconnecting here is very wrong / LB
			return;
		case ACCOUNT_BANNED:
			Xsend(s, acctblock, 2);	
			return;
		case LOGIN_NOT_FOUND:
			if( !SrvParams->autoAccountCreate() )
			{
				Xsend(s, noaccount, 2);		
				return;
			}
		}
	}
	if (acctno[s] >= 0)
		GoodAuth(s);
}

void cNetworkStuff::pSplit (char *pass0) // Split login password into Wolfpack password and UO password
{
	int i,loopexit=0;
	i=0;
	pass1[0]=0;
	while ( (pass0[i]!='/') && (pass0[i]!=0) && (++loopexit < MAXLOOPS) ) i++;
	strncpy(pass1,pass0,i);
	pass1[i]=0;
	if (pass0[i]!=0) strcpy(pass2, pass0+i+1);
}*/

/*void cNetworkStuff::charplay (int s) // After hitting "Play Character" button //Instalog
{
	int j;
	P_CHAR pc_selected = NULL;
	if (acctno[s]>-1)
	{
		j=0;

		AllCharsIterator iter_char;
		for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
		{
			P_CHAR toCheck = iter_char.GetData();
			if ((toCheck->account() == acctno[s])&&(toCheck->isPlayer())&&(!toCheck->free))
			{
				if (j==buffer[s][0x44]) {					
					pc_selected = toCheck;
					break;
				}
				j++;
			}
		}

		if (pc_selected != NULL)
		{
			if (Accounts->GetInWorld(acctno[s]) != INVALID_SERIAL) //JM's crashfix
			{
				if ((pc_selected->logout()<=getPlatformTime() || overflow))
					Accounts->SetOffline(acctno[s]);
			} else Accounts->SetOffline(acctno[s]);

			if (Accounts->GetInWorld(acctno[s]) == INVALID_SERIAL || pc_selected->isGM())//AntiChrist
			{
				Accounts->SetOnline(acctno[s], pc_selected->serial);
				pc_selected->setLogout(0);
				currchar[s] = pc_selected;
				startchar(s);
			} else {
#ifdef DEBUG
				clConsole.send("DEBUG: Attempted %i, %i from this account (%i) is in world.\n",k,inworld[acctno[s]],acctno[s]);
#endif
				char msg[3];
				msg[0]=0x53;
				msg[1]=0x05;
				Xsend(s, msg, 2);
			}
		}
	}
	else
	{	
		//Disconnect(s);
	}
}

void cNetworkStuff::startchar(int s) // Send character startup stuff to player
{
	unsigned char modeset[6]="\x72\x00\x00\x32\x00";
	unsigned char techstuff[21]="\x69\x00\x05\x01\x00\x69\x00\x05\x02\x00\x69\x00\x05\x03\x00\x55\x5B\x0C\x13\x03";
	unsigned char startup[38]="\x1B\x00\x05\xA8\x90\x00\x00\x00\x00\x01\x90\x06\x08\x06\x49\x00\x0A\x04\x00\x00\x00\x7F\x00\x00\x00\x00\x00\x07\x80\x09\x60\x00\x00\x00\x00\x00\x00";
	unsigned char setseason[4]="\xBC\x00\x01";
	unsigned char world[7]="\xBF\x00\x06\x00\x08\x00";

	P_CHAR pc_currchar = currchar[s];

	if (pc_currchar->pos.map == 2) world[5]=0x02;
	Xsend(s, world, 6);

	unsigned char features[3];
	features[0]=0xb9;
	features[1]=0x0;
	features[2]=0x3;

	Xsend(s, features,3);

	int i,serial;

	perm[s]=1;
	targetok[s]=0;	

	LongToCharPtr(pc_currchar->serial, &startup[1]);
	ShortToCharPtr(pc_currchar->id(), &startup[9]);
	startup[11]=pc_currchar->pos.x>>8;
	startup[12]=pc_currchar->pos.x%256;
	startup[13]=pc_currchar->pos.y>>8;
	startup[14]=pc_currchar->pos.y%256;
	startup[16]=pc_currchar->pos.z;
	startup[17]=pc_currchar->dir;
	startup[28]=0;

	if(pc_currchar->poisoned()) 
		startup[28]=0x04; 
	else 
		startup[28]=0x00;
	pc_currchar->spiritspeaktimer = 0;	// initially set spiritspeak timer to 0

	pc_currchar->setStealth(-1);//AntiChrist
	if (! (pc_currchar->isGMorCounselor())) pc_currchar->setHidden( 0 );//AntiChrist
	pc_currchar->setBegging_timer(0);

	Xsend(s, startup, 37);
	pc_currchar->war=false;
	wornitems(s, pc_currchar);
	teleport((pc_currchar));
	Xsend(s, modeset, 5);
	impowncreate(s, pc_currchar, 0);
	Xsend(s, techstuff, 20);
	Weight->NewCalc(pc_currchar);	// Ison 2-20-99

	////////////////////////////////////////
	/// "UNKOWNDIMENSION sysmessages"
	//// using color (!=0) is essential within entire function
	///  if not 3d clients >=3.0.6g die
	///  sysmessage(s, "xxx") does use color 0 !
	/// you can change 0x37 to your liking, but not to 0
	/////////////////////////////////////////////////////////////////////
*/
	/*sysmessage(s, 0x37, tr(QString("Welcome to %1 !").arg(SrvParams->serverList()[0].sServer.c_str())));
	sysmessage(s, 0x37, tr(QString("Running on %1 %2 %3 ").arg(wp_version.productstring.c_str()).arg(wp_version.betareleasestring.c_str()).arg(wp_version.verstring.c_str())) );
	sysmessage(s, 0x37, tr(QString("Current developers: %1").arg(wp_version.codersstring.c_str())) );*/
	
	/*
	// Testing Gumps
	cGump myGump( 0x12345678, 0x00000001, false, false, false, 200, 300 );
	myGump.addBackground( 0x53, 400, 300 );
	myGump.startPage( 1 );
	myGump.addText( 10, 10, "Mein Text auf Seite 1", 0x480 );
	myGump.addPageButton( 10, 40, 0x2907, 0x290A, 2 );
	myGump.startPage( 2 );
	myGump.addText( 10, 10, "Mein Text auf Seite 2", 0x480 );
	myGump.addPageButton( 10, 40, 0x2907, 0x290A, 1 );
	myGump.send( s );*/
/*
	pc_currchar->region=255;
	cAllTerritories::getInstance()->check(pc_currchar);
	//Tauriel set packitem at login
	serial=pc_currchar->serial;

	vector<SERIAL> vecContainer = contsp.getData(serial);
	for (i=0;i<vecContainer.size();i++)
	{
		const P_ITEM pi = FindItemBySerial(vecContainer[i]);
		if (pi != NULL)
		{
			if (pi->contserial==serial && (pi->layer()==0x15))
			{
				pc_currchar->packitem = pi->serial;
			}
		}
	}

	setseason[ 1 ] = SrvParams->season();
	Xsend( s, setseason, 3 );

	if (SrvParams->joinMsg())
		sysbroadcast( QString( "%1 entered the realm" ).arg( pc_currchar->name.c_str() ).latin1() );

	Accounts->SetOnline(acctno[s], pc_currchar->serial);
	teleport(pc_currchar);

	pc_currchar->setMurderrate(uiCurrentTime+SrvParams->murderdecay()*MY_CLOCKS_PER_SEC);

	updates(s);

	if( SrvParams->showCVCS() )
	{
		QString message;
		if (SrvParams->clientsAllowed().contains("ALL"))
		{
			sysmessage(s, 0x37, "The recommanded client version for this shard is: %s.", wp_version.clientsupportedstring.latin1() );
			return;
		}
		else if (SrvParams->clientsAllowed().contains("SERVER_DEFAULT"))
		{
			sysmessage(s, 0x37, "The recommanded client version for this shard is: %s.", wp_version.clientsupportedstring.latin1() );
			return;
		}
		else
		{	
			message = "This shard requires client version[s]: ";
		}
		
		// remark: although it doesn't look good [without], don't add /n's
		// the (2-d) client doesn't like them
		
		temp2[0] = 0;
		QStringList::const_iterator it = SrvParams->clientsAllowed().begin();
		for (; it != SrvParams->clientsAllowed().end(); ++it)
		{
			if ( *it != "SERVER_DEFAULT" )
				message += *it;
			else
				message += wp_version.clientsupportedstring;
		}
		
		message += " The recommanded version is: ";
		message += wp_version.clientsupportedstring;
		
		sysmessage(s, 0x37, (char*)message.latin1());
	}
}

//Boats->added multi checking to instalog.
char cNetworkStuff::LogOut(int s)//Instalog
{
	P_CHAR pc_currchar = currchar[s];
	if (pc_currchar == NULL)
		return 0;
	int i, valid=0;
	int x = pc_currchar->pos.x, y = pc_currchar->pos.y;
	P_ITEM pi_multi = NULL;
	unsigned int a;
	for(a=0;a<logoutcount;a++)
	{
		if (logout[a].x1<=x && logout[a].y1<=y && logout[a].x2>=x && logout[a].y2>=y)
		{
			valid=1;
			break;
		}
	}
	//GM always have no logout time - AntiChrist
	if (pc_currchar->isGMorCounselor() || pc_currchar->account()==0) valid=1;
	if (pc_currchar->dead)
		valid = 1; // They ain't going to die again ;)

	if (pc_currchar->multis==-1)
		pi_multi = findmulti(pc_currchar->pos);
	else
		pi_multi = FindItemBySerial( pc_currchar->multis );
	
	if (pi_multi != NULL && !valid)//It they are in a multi... and it's not already valid (if it is why bother checking?)
	{
		P_ITEM pPack = Packitem(pc_currchar);
		if (pPack != NULL)
		{
			vector<SERIAL> vecContainer = contsp.getData(pPack->serial);
			for (unsigned int a = 0; a < vecContainer.size(); a++)
			{
				P_ITEM pi_ci = FindItemBySerial(vecContainer[a]);
				if (pi_ci != NULL)
				if (pi_ci->type() == 7 && (calcserial(pi_ci->more1, pi_ci->more2, pi_ci->more3, pi_ci->more4) == pi_multi->serial))
				{//a key to this multi
					valid=1;//Log 'em out now!
					break;
				}
			}
		}
	}

	if (valid)//||region[pc_currchar->region].priv&0x17)
	{
		Accounts->SetOffline(pc_currchar->account());
		pc_currchar->setLogout(0); // LB bugfix, was timeout
	} else {
		if (perm[s])
		{
			Accounts->SetOffline(pc_currchar->account());  // Allows next login.
		    pc_currchar->setLogout(uiCurrentTime+SrvParams->quittime()*MY_CLOCKS_PER_SEC);
		}
	}

	for (i=0;i<now;i++)
	{
		if(perm[i] && i!=s && inrange1p(currchar[i], pc_currchar))
			impowncreate(i, pc_currchar, 0);
	}

	return valid;
}


// rewrite    : LB 4'th october 2000
// parameters : x = # of bytes
//            : a = buffer offset

int cNetworkStuff::Receive(int s, int x, int a) // Old socket receive function (To be replaced soon)
{
	int count,loopexit=0;
	
	if ( (x+a) >= MAXBUFFER_REAL) return 0;

	do
	{
		if((count = recv(client[s], (char*)&buffer[s][a], x, MSG_NOSIGNAL))==SOCKET_ERROR)
		{
#ifndef __unix__
			errno = WSAGetLastError();
#endif
			if (errno != WSAECONNRESET)
				LogErrorVar("Socket Recv Error %i\n",errno) ;
		}
	
	}
	while ( (count!=x) && (count>0) && (++loopexit < MAXBUFFER_ASYNCH ));

	return count;
}

void cNetworkStuff::sockInit()
{
	int bcode;

	kr=1;
	faul=0;

	a_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (a_socket < 0 )
	{
		clConsole.send("ERROR: Unable to create socket\n");
		keeprun=0;
		error=1;
		kr=0;
		faul=1;
		return;
	}*/
/*
#ifdef __linux__

	bcode = setsockopt(a_socket, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
	if (bcode!=0)
	{
		clConsole.send("ERROR: Unable to init. socket - Error code: %i\n",bcode);
		keeprun=0;
		error=1;
		kr=0;
		faul=1;
		return;
	}

#endif
*//*
	len_connection_addr=sizeof (struct sockaddr_in);
	connection.sin_family = AF_INET;
	connection.sin_addr.s_addr = INADDR_ANY;	
	connection.sin_port = htons(wp_port); //Port
	#if defined(__unix__)
	int setreuse = 1 ;
	linger stLinger ;
	stLinger.l_onoff = 0 ;
	stLinger.l_linger = 0 ;
	setsockopt(a_socket,SOL_SOCKET,SO_LINGER,&stLinger,sizeof(stLinger)) ;
	setsockopt(a_socket,SOL_SOCKET,SO_REUSEADDR,&setreuse,sizeof(setreuse)) ;
	#endif
	bcode = bind(a_socket, (struct sockaddr *)&connection, len_connection_addr);

	if (bcode!=0)
	{
		clConsole.send("ERROR: Unable to bind socket 1 - Error code: %i\n",bcode);
		keeprun=0;
		error=1;
		kr=0;
		faul=1;
		return;
	}

	bcode = listen(a_socket, MAXCLIENT);

	if (bcode!=0)
	{
		clConsole.send("ERROR: Unable to set socket in listen mode  - Error code: %i\n",bcode);
		keeprun=0;
		error=1;
		kr=0;
		faul=1;
		return;
	}
	// Ok, we need to set this socket (or listening one as non blocking).  The reason is we d a
	// select, and then do an accept.  However, if the client has terminated the connection between the small
	// time from the select and accept, we would block (accept is blocking).  So, set it non blocking
	
#if defined(__unix__)
	int iocltmp = 0;
	ioctl(a_socket,FIONBIO, &iocltmp);
#else
	ioctlsocket(a_socket,FIONBIO,0) ;
#endif

	login03[1]  = 0;
	login03[2]  = 0;
	login03[3]  = 0;
	login03[4]  = 0;
	login03[7]  = 0;
	login03[8]  = 0;
	login03[9]  = 0;
	login03[10] = 0;
	ph1 = ph2 = ph3 = ph4 = 0;
	
	
}


void cNetworkStuff::SockClose () // Close all sockets for shutdown
{
	int i;
	closesocket(a_socket);
	for (i=0;i<MAXCLIENT;i++) closesocket(client[i]);
}

void cNetworkStuff::CheckConn() // Check for connection requests
{
	int s;
	socklen_t len;
	
	if (now<MAXIMUM)
	{
		FD_ZERO(&conn);
		FD_SET(a_socket, &conn);
		nfds=a_socket+1;

		s=select(nfds, &conn, NULL, NULL, &uoxtimeout);	

		if (s>0)
		{	
			len = sizeof(struct sockaddr_in);
			client[now] = accept(a_socket, (struct sockaddr *)&client_addr, &len);
			if ( client[now]<0 )
			{
				clConsole.send("ERROR: Error at client connection!\n");
				error=1;			
				keeprun=1;			
				return;
			}
			if ( CheckForBlockedIP( client_addr ) )
			{
				clConsole.send("IPBlocking: Blocking IP address [%s] listed in hosts_deny\n", inet_ntoa( client_addr.sin_addr ));
				closesocket(client[now]);
			}
			else
			{
				newclient[now]=1;
				acctno[now]=-1;
				perm[now]=0;
				binlength[now]=0;
				boutlength[now]=0;
				cryptclient[now]=0;		
				usedfree[now]=0;
				walksequence[now]=-1;
	
				noweather[now]=1;
				LSD[now]=0;

				clientDimension[now]=2;
			
			// currchar, inworld, client ip CANT be set here ! important would cause bugs !

			// probably not necassairy, targeting arrays
				addid1[now]=0;addid2[now]=0;addid3[now]=0;addid4[now]=0;
				addid5[now]=0;
	   		    tempint[now]=0;
				addid1[now]=0;addid2[now]=0;addid3[now]=0;addid4[now]=0;
				dyeall[now]=0;
				addx[now]=0;addy[now]=0;
				addx2[now]=0;addy2[now]=0;addz[now]=0;
				addmitem[now]=0;
				clickx[now]=0;clicky[now]=0; targetok[now]=0;
				currentSpellType[now]=0;
				
				global_lis++; // not 100% correct, but only cosmetical stuff, hence ok not to be 100% correct :>
                          // doesnt get correct status if kicked out due to worng pw etc.

            	if (global_lis % 2 == 0) sprintf((char*)temp2, "connecting"); else sprintf((char*)temp2, "connected");

				sprintf((char*)temp,"WOLFPACK: Client %i [%i.%i.%i.%i] %s [Total:%i].\n",now,client_addr.sin_addr.s_addr&0xFF _ (client_addr.sin_addr.s_addr&0xFF00)>>8 _ (client_addr.sin_addr.s_addr&0xFF0000)>>16 _ (client_addr.sin_addr.s_addr&0xFF000000)>>24, temp2, now+1);
				clConsole.send(temp);

				if (SrvParams->serverLog()) 
					savelog((char*)temp,"server.log");

				now++;
			}
			return;

		}
		else if (s<0)
		{
			clConsole.send("ERROR: Select (Conn) failed!\n");
			keeprun=1;
			error=1;
			return;
		}
	}
}

void cNetworkStuff::CheckMessage() // Check for messages from the clients
{
	int s, i, oldnow;
	
	FD_ZERO(&all);
	FD_ZERO(&errsock);
	nfds=0;

	for (i=0;i<now;i++)
	{
		FD_SET(client[i],&all);
		FD_SET(client[i],&errsock);
		if (client[i]+1>nfds) nfds=client[i]+1;

	}

	s = select(nfds, &all, NULL, &errsock, &uoxtimeout);

	if (s>0)
	{
		oldnow=now;
		for (i=0;i<oldnow;i++)
		{
			if (FD_ISSET(client[i],&errsock))
			{
				Disconnect(i);
			}

		
			if ((FD_ISSET(client[i],&all))&&(oldnow==now))
			{
				GetMsg(i);
				if (executebatch) batchcheck(i);
				                                 				
			}

		}
	}
}
void cNetworkStuff::GetMsg(int s) // Receive message from client
{
	int count, j, serial, length, dyn_length, loopexit=0, fb;
	unsigned char packet;
	int  myoffset,  myj, mysize, subcommand, subsubcommand ;
	unsigned char mytempbuf[512] ;
	char client_lang[4];

	P_CHAR pc_target = NULL;
	P_ITEM pi_target = NULL;
	string cpps;
		
	if (newclient[s])
	{
		if((count=recv(client[s], (char*)buffer[s], 4, MSG_NOSIGNAL))==SOCKET_ERROR)
		{
#ifndef __unix__
			errno = WSAGetLastError();
#endif
			if (errno != WSAECONNRESET)
				LogErrorVar("Socket Recv Error %i\n",errno) ;
		}
		
		newclient[s]=0;
		firstpacket[s]=1;
				
	}
	else
	{
		fb = Receive(s, 1, 0);
		if (fb >0)
		{	
			
			packet = buffer[s][0];
			length = packlen_cl::size(packet);						

			//clConsole.send("packet: %x length: %x firstpacket: %i\n",packet,length,firstpacket[s]);

			// Lets assure the data is valid, this stops server freezes caused by receiving nonsense data
			// (remark: useres that dont use ignition do that)
			if (firstpacket[s] && packet != 0x80 && packet != 0x91 )
			{
				Disconnect(s);
				clConsole.send("received garbage from a client, disconnected it to prevent bad things.\n User probably didnt use ignition\n");
				return;
			}
	

			if (length==PACKET_LEN_NONE)
			{				
				clConsole.send("received unknown packet from user\n\n");
*/
				/*FD_ZERO(&all); FD_SET(client[s],&all);nfds=client[s]+1;
				if (select(nfds, &all, NULL, NULL, &uoxtimeout)>0) Receive(s, MAXBUFFER-2, 0);				
				// I think thats supposed to be a re-synch attempt for next msg-in case of getting garbage (LB)
				*//*

				cNetworkStuff::Disconnect(s);
				return;
			}

			int readstat ;

            if (length==PACKET_LEN_DYNAMIC)
			{
				if ((readstat = Receive(s, 2, 1)) > 0)
				{
					dyn_length = (int) (  ( (int) buffer[s][1]<<8) + (int) buffer[s][2] );				
					// clConsole.send("dyn-length: %i\n",dyn_length);
					readstat = Receive(s, dyn_length-3, 3) ;				
				}


			} else
			{
				readstat = Receive(s, length-1, 1);

			}
		
			if (readstat > SOCKET_ERROR)
			{
				P_CHAR pc_currchar = currchar[s];
				if ( pc_currchar != NULL && packet !=0x73 && packet!=0x80 && packet!=0xA4 && packet!=0xA0 && packet!=0x90 && packet!=0x91 )
					pc_currchar->setClientIdleTime(SrvParams->inactivityTimeout()*MY_CLOCKS_PER_SEC+uiCurrentTime);
        		    // LB, client activity-timestamp !!! to detect client crashes, ip changes etc and disconnect in that case
        		    // 0x73 (idle packet) also counts towards client idle time

				cClient cli(s);
				P_CLIENT ps = &cli;

				switch(packet)
				{

				case 0x04:
					// Expermintal for God clent
                    if (pc_currchar->isGM())
					{
						char packet[] = "\x2B\x01";
						Xsend(s, packet, 2);
                        //clConsole.send("%s connected in with God Client!\n", pc_currchar->name);
                    }
					else
					{
						sysmessage(s, "Access Denied!!!");
                        Disconnect(s);
						clConsole.send("%s tried connecting in with God Client but has no priviledges!\n", pc_currchar->name.c_str());
                    }
					break;

				case 0x01:// Main Menu on the character select screen
				// remark LB : no longer send :(
					Disconnect(s);
					break;

				case 0x80:// First Login					
					firstpacket[s]=0;
					LoginMain(s);			
					break;

				case 0xA0:// Server Select				
					Relay(s);
					break;

				case 0x91:// Second Login
					firstpacket[s]=0;
					cryptclient[s]=1;
					CharList(s);
					break;

				case 0x83:// Character Delete
					chardel(s);
					break;

				case 0x00:// Character Create				
					charcreate(s);
					break;

				case 0x5D:// Character Select							
					charplay(s);			
					break;

				case 0x02:// Walk				
					Movement->Walking(pc_currchar, buffer[s][1], buffer[s][2]);
					pc_currchar->disturbMed(s);
					break;

				case 0x73:// Keep alive				
					Xsend(s, buffer[s], 2);
					break;

				case 0x22:// Resync Request			
					teleport((pc_currchar));
					break;

				case 0x03:// Speech
					{
						pc_currchar->setUnicode( false );
						QString punt = (char*)&buffer[s][8];
						//strcpy((char*)nonuni, (char*)&buffer[s][8]);
						//Speech->talking(s, (char*)nonuni);
						Speech->talking(s,punt) ;
					}
					break;

				// Thx a lot to Beosil for laying out the basics of the new structure of that packet since client 2.0.7
				// Thx a lot to Punt for the implementation of it (4'th Feb 2001)
        			// Fixed a few erratas in that initial packet interpretation (LB 14-Feb 2001)

				case 0xAD: // Unicode Speech			
					{
						pc_currchar->setUnicode(true);
						// Check for command word versions of this packet														
						
						if ( (buffer[s][3]) >=0xc0 )
						{			
							// Get the trigger
							
							buffer[s][3] = buffer[s][3] & 0x0F ; // set to normal (cutting off the ascii indicator since we are converting back to unicode)					
							*/
							//int num_words,/*idx=0,*/ num_unknown;				
							
							// number of distict trigger words
							/*num_words = ( (static_cast<int>(buffer[s][12])) << 24 ) + ( (static_cast<int>(buffer[s][13])) << 16 );
							num_words = num_words & 0xfff00000;
							num_words = (num_words >> 20);*/
							
							/*************************************/
							// plz dont delete yet
							// trigger word index in/from speech.mul, not required [yet]
							/*idx = ( (static_cast<int>(buffer[s][13])) << 24 ) + ( (static_cast<int>(buffer[s][14])) << 16);
							idx = idx & 0x0fff0000;
							idx = ( (idx << 4) >> 20) ;*/						
							//cout << "#keywords was " << hex << num_words << "\n" << hex << static_cast<int>(buffer[s][12]) << " " << hex << static_cast<int> (buffer[s][13]) << " " << static_cast<int> (buffer[s][14]) << " " << static_cast<int> (buffer[s][15]) << endl ;
							// cout << "idx: " << idx << endl;
							/*************************************/
							
							/*if ((num_words %2) == 1)  // odd number ?
							{
								num_unknown = ( num_words / 2 )  * 3;
							} else
							{
								num_unknown = ( ( num_words / 2 ) * 3 ) - 1 ;
							}
							
							myoffset = 15 + num_unknown;					
							
							//
							//	Now adjust the buffer
							int iWord ;
							//int iTempBuf ;
							iWord = static_cast<int> ((buffer[s][1] << 8)) + static_cast<int> (buffer[s][2]) ;
							myj = 12 ;
							
							//cout << "Max length characters will be " << dec << (iWord - myoffset) << endl ;
							mysize = iWord - myoffset ;
							int i;
							for (i=0; i < mysize ; i++)
							{
								mytempbuf[i] = buffer[s][i+myoffset] ;
							}
							
							for (i=0; i < mysize ; i++)
							{							
								myj++ ;
								buffer[s][myj] = mytempbuf[i] ;
								//iTempBuf = static_cast<int> (mytempbuf[i]) ;
								//cout << "Copying value of " << hex << iTempBuf << endl ;
								myj++;
								buffer[s][myj] = 0 ;
								
							}
							
							iWord = (((iWord - myoffset ) * 2) + 12) ;
							//cout << "Setting buffer size to " << dec << iWord << endl ;
							buffer[s][1] = static_cast<unsigned char> ( ( ( iWord & 0xFF00 ) >>8 ) ) ;
							buffer[s][2] = static_cast<unsigned char> ( iWord & 0x00FF ) ;															
						}	
						
						//Speech->wchar2char((char*)&buffer[s][13]);
						QString speech;
						speech.setUnicodeCodes((ushort*)&buffer[s][13], (buffer[s][1]<<8)+buffer[s][2]);
						//strncpy((char*)nonuni, temp, ((buffer[s][1]<<8)+buffer[s][2])/2);
						//ring punt = (char*) nonuni ;
						Speech->talking(s, speech);
					}
					break;

				case 0x06:// Doubleclick			
					doubleclick(s);
					break;

				case 0x09:// Singleclick
					singleclick(s);
					break;

				case 0x6C:// Targeting		
					if(targetok[s]) Targ->MultiTarget(ps);
					break;

				case 0x13:// Equip Item
					cDragItems::getInstance()->equipItem( ps );
					break;

				case 0x07:// Get Item		
					cDragItems::getInstance()->grabItem( ps );
					break;

				case 0x08:// Drop Item
					cDragItems::getInstance()->dropItem( ps );
					break;

				case 0x72:// Combat Mode			
					pc_currchar->war = buffer[s][1];
					pc_currchar->targ = INVALID_SERIAL;
					Xsend(s, buffer[s], 5);
					if (pc_currchar->dead && pc_currchar->war) // Invisible ghost, resend.
					{
						updatechar(pc_currchar);
					}
					Movement->CombatWalk(pc_currchar);
					dosocketmidi(s);
					pc_currchar->disturbMed(s);
					break;

				case 0x12:// Ext. Command
					unsigned int i;
					
					// Switch on the sub commands
					switch (static_cast<unsigned char>(buffer[s][3]))
					{
					
					case 0xC7:
					// Action
						if (!pc_currchar->onHorse())
						{
							if (!(strcmp((char*)&buffer[s][4],"bow"))) action(s, 0x20);
							if (!(strcmp((char*)&buffer[s][4],"salute"))) action(s, 0x21);
						}
						break;
					
					case 0x24:
					// skill
						i=4;
						while ( (buffer[s][i]!=' ') && (++loopexit < MAXLOOPS) ) i++;
						buffer[s][i]=0;
						Skills->SkillUse(s, str2num((char*)&buffer[s][4]));
						break;
					
					case 0x56: // Casted a macro'd spell
					case 0x27: // Casted a spell out of a spellbook
						{
							if( pc_currchar->priv2 & 2 )
							{
								sysmessage( s, "You cannot cast spells while frozen." );
								break;
							}

							if( pc_currchar->casting() )
							{
								sysmessage( s, "You are already casting a spell." );
								break;
							}

							QStringList parts = QStringList::split( " ", (char*)&buffer[s][4] );

							if( parts.count() == 1 )
								Magic->prepare( pc_currchar, parts[0].toInt(), 0 );
							else if( parts.count() == 2 )
							{
								// Try to retrieve the SpellBook selected
								P_ITEM spellBook = FindItemBySerial( parts[1].toUInt() );
								if( spellBook != NULL )
									Magic->prepare( pc_currchar, parts[0].toInt(), 0, spellBook );
							}
						}
						break;
					case 0x43:
						// Open Spell Books
						Magic->openSpellBook( pc_currchar, NULL );						
						break;
					case 0x58:
					// door Macro
						// search for closest door and open it ...			
						break;
					default:
						break;
					}
					break; // Lord Binary !!!!

				case 0x9B:// GM Page
                    if( !pc_currchar->onHelp() )
					    ShowMenu(s, 4096);
					break;

				case 0x7D:// Choice			
					MenuChoice( s );
					break;

				case 0xB5:
                    if( !pc_currchar->onChat() )
					    sysmessage( s, "Sorry, but the Chat is not yet supported" );
					break;

				case 0x95:// Color Select			
					Commands->DyeItem(s);
					break;

				case 0x34:// Status Request							
					srequest(s);
					break;

				case 0x75:// Rename Character //Lag Fix -- Zippy //Bug Fix -- Zippy	
					{
						serial=calcserial(buffer[s][1],buffer[s][2],buffer[s][3],buffer[s][4]);
						if(serial==-1) return;
						pc_target = FindCharBySerial( serial );
						char temp[50];
						if(pc_target != NULL)
							strncpy(temp, (char*)&buffer[s][5], 50);
						pc_target->name = temp;
					}
					break;
		
				case 0x66:// Read Book
					int size;
					size=dyn_length;
					serial=calcserial(buffer[s][3],buffer[s][4],buffer[s][5],buffer[s][6]);
					pi_target = FindItemBySerial( serial );
					if (pi_target != NULL)
					{
						P_ITEM pBook= pi_target;
						if (pBook->morex!=666 && pBook->morex!=999)
							Books->readbook_readonly_old(s, pBook, (buffer[s][9]<<8)+buffer[s][10]);  // call old books read-method
						if (pBook->morex==666) // writeable book -> copy page data send by client to the class-page buffer
						{
							for (j=13;j<=size;j++) // copy (written) page data in class-page buffer
							{
								Books->pagebuffer[s][j-13]=buffer[s][j];
							}
							Books->readbook_writeable(s,pBook,(buffer[s][9]<<8)+buffer[s][10],(buffer[s][11]<<8)+buffer[s][12] );
						}
						if (pBook->morex==999) Books->readbook_readonly(s,pBook,(buffer[s][9]<<8)+buffer[s][10]); // new books readonly
					}
					break;

            		// client sends them out if the titel and/or author gets changed on a writable book
		    	// its NOT send (anymore?) when a book is closed as many packet docus state.
			// LB 7-dec 1999
				case 0x93:

					int serial,j;
					char author[31],title[61],ch;
	
					serial=calcserial(buffer[s][1],buffer[s][2],buffer[s][3],buffer[s][4]);
					pi_target = FindItemBySerial( serial );
					if (pi_target == NULL) return;				
					Books->a_t=1;			
           			j=9;ch=1;
					while(ch!=0)
					{
                   		ch=buffer[s][j];
						title[j-9]=ch;
						Books->titlebuffer[s][j-9]=ch;
						j++;
						if (j>69) ch=0;
					}
				

					j=69;ch=1;
					while(ch!=0)
					{
           				ch=buffer[s][j];
						author[j-69]=ch;
						Books->authorbuffer[s][j-69]=ch;
						j++;
				  		if (j>99) ch=0;
					}
					break;

				case 0x3a: // client 1.26.2b+ skill managment packet

               		// -> 0,1,2,3 -> ignore them
               		// -> 4 = skill number
               		// -> 5 = 0 raising (up), 1 falling=candidate for atrophy, 2 = locked
               		pc_currchar->lockSkill[buffer[s][4]] = buffer[s][5]; // save skill managment changes
            		break;

				case 0xA7:// Get Tip			
					tips(s, (buffer[s][1]<<8)+buffer[s][2]+1);
					break;

				case 0xA4:// Spy			
					break;

				case 0x05:// Attack				
					RcvAttack(ps);
					break;
				case 0xB1:// Gumpmenu choice
					cGumps::instance()->Button(s, (buffer[s][13]<<8)+buffer[s][14], LongFromCharPtr(buffer[s]+3), buffer[s][10]);
					break;

				case 0xAC:// Textentry input			
					cGumps::instance()->Input(s);
					break;

				case 0x2C:// Resurrect menu choice			
					if(buffer[s][1]==0x02)
					{
						if( ( pc_currchar->murdererSer() > 0 ) && SrvParams->bountysactive() )
						{
							sprintf( (char*)temp, "To place a bounty on %s, use the command BOUNTY <Amount>.",
						        FindCharBySerial(pc_currchar->murdererSer())->name.c_str() );
							sysmessage( s,(char*) temp );
						}
						sysmessage(s, "You are now a ghost.");
					}
					if(buffer[s][1]==0x01) sysmessage(s, "The connection between your spirit and the world is too weak.");
					break;

				case 0x3B:// Buy from vendor...		
					Trade->buyaction(s);
					break;

				case 0x9F:// Sell to vendor...			
					Trade->sellaction(s);
					break;

				case 0x69:// Client text change			
					break;

				case 0x6F:// Secure Trading message			
					Trade->trademsg(s);
					break;

				case 0xB6:// T2A Popuphelp request			
					break;

				case 0xB8:// T2A Profile request			
					if (buffer[s][3] == 0) //Read request
					{
						P_CHAR pc = FindCharBySerial(calcserial(buffer[s][4], buffer[s][5], buffer[s][6], buffer[s][7]));
						if (pc->isPlayer())
						{
							unsigned char PACKET0xB8[100] = {0xB8, 0x00,};
							unsigned int tlen = 7;
							
							LongToCharPtr(pc->serial, &PACKET0xB8[4]);
							strcpy((char*)&PACKET0xB8[8], complete_title(pc));
							tlen += strlen(complete_title(pc))+1;
							PACKET0xB8[tlen] = 0;
							tlen++;*/
							//strcpy((char*)&PACKET0xB8[tlen], "Can not determine this account's age.");
							//tlen += strlen((char*)&PACKET0xB8[tlen]);*/
							/*len += 4;
							PACKET0xB8[2] = (unsigned char) tlen;
							Xsend(s, PACKET0xB8, tlen);
						}
					}
					else if (buffer[s][3] == 1) // Write request
					{
					}
					else
						clConsole.send("Unkown packet 0xB8 request");
					break;

				// Dupois
				case 0x71:// Message Board Item			
					MsgBoardEvent(s);
					break;
				// Dupois - End
								
				case 0xBB:
           			break;

				// Client version packet ... OSI clients always send that one on login.			
				// even on re-logins
       			case 0xBD:

				// client version control system (LB)
				
					strcpy((char*)temp3,(char*) &buffer[s][3]); // copy client version data		
					if ( strlen (temp3) > 10) clientDimension[s] = 3; else clientDimension[s] = 2;
					sprintf(temp,"You are using a %iD client, version %s", clientDimension[s], temp3);
					sysmessage(s,temp);

			   		if (SrvParams->isClientAllowed(temp3) ) 
						break; 
					else
					{
						clConsole.send("Client %i disconnected by Client Version Control System\n", s);
						sysmessage(s, "This client version is not allowed on this server.");
						Disconnect(s);
					}
           			break;

				case 0xBF:
				
				// can't beleive this mega multipurpose packet isn't used :)
				// thought it's about time to change this , LB 30-March 2001
				// note: bf packet is used server and client side, here are only the client side ones
                // I have encountered
					subcommand = static_cast<int> ( (static_cast<int> (buffer[s][3]) << 8 ) + buffer[s][4] );			

				// please don't remove the // unknowns ... want to have them as dokumentation
					switch (subcommand)
					{
						case 5:
							// Sub Sub
							subsubcommand = static_cast<int> ( (static_cast<int> (buffer[s][3]) << 8 ) + buffer[s][4] );

							switch( subsubcommand )
							{
							case 0x09: // Wrestling Disarm
								sysmessage( s, QString( "Wrestling Disarm is unsupported" ) );
								break;
							case 0x0A: // Wrestling Stun
								sysmessage( s, QString( "Wrestling Stun is unsupported" ) );
								break;
							case 0x05: // Unknown - Just send once on login
								break;
							default:
								sysmessage( s, QString( "Unsupported extended Macro message: %1" ).arg( subsubcommand ) );
								break;
							};
							
							break; // unknown, sent once on login

				   		case 6:
					
					     		subsubcommand = buffer[s][5];  // party system
							 // .... party system implemation (call) goes here :)
							 // ....
							 // ....
							// we should at least notify the user that we dont support partys
								sysmessage( s, "Sorry but the party system is not yet implemented" );
							 break;

						case 11: // client language, might be used for server localisation

						// please no strcpy or memcpy optimization here, because the input ain't 0-termianted and memcpy might be overkill					
						   	client_lang[0]=buffer[s][5];
                       		client_lang[1]=buffer[s][6];
							client_lang[2]=buffer[s][7];
							client_lang[3]=0;					
						   // do dometihng with language information from client
						   // ...
						   	break;
				
						case 12: break; // close gumps, client message

				   		case 14: // UO:3D menus
					  		action(s, buffer[s][8]);					
					  		break;

						case 15: break; // unknown, sent once on login
					
				   		default: break;

					}

					break;
	
				default:
					break;

    				} // end switch
				
			}
			else
				Disconnect(s) ; // Error on a read
  		} // end if recv >0
  		else								
  		{	
	  //clConsole.send("FB: %i perm: %i\n",fb,perm[s]);
		  Disconnect(s); // extremly important (and tricky too ;-) !!!
	  // osi client closes socket AFTRER the first 4 bytes and re-opens it afterward.
	  // this line handles this correctly
		  }

 	} // end if newclient[]

}

signed long cNetworkStuff::Authenticate( const char *username, const char *pass )
{
	// We want case sensitive password/usernames
	std::string dummyusername = username, dummypass = pass;
	return Accounts->Authenticate(dummyusername, dummypass);
}

void cNetworkStuff::LoadHosts_deny(void)
{
	unsigned long loopexit = 0;
	std::string  sScript1;
	std::string  sToken1;
	
	if (hosts_deny.size() != 0)
		hosts_deny.clear();

	if (!openscript("hosts_deny.scp", false))
		return;

	do
	{
		read2();	
		if (strcmp(script1, "EOF")) // If script1 != "EOF"
		{
			//let's load a IP addresss/NetMask
			ip_block_st ip_block;
			unsigned long ip_address;
			sScript1 = script1 ;
			string::size_type siEnd = sScript1.find("/") ;
			sToken1 = sScript1.substr(0,siEnd) ;
			
			//char* straddr = strtok(script1, "/");
			//ip_address = inet_addr(straddr);
			ip_address = inet_addr(sToken1.c_str()) ;
			if (ip_address != INADDR_NONE)
				ip_block.address = ip_address;
			else
				continue;
			// Get the rest of the string, after the '/' token
			if (siEnd != std::string::npos)
			{
				sToken1 = sScript1.substr(siEnd+1) ;
				
			//straddr = strtok(NULL, "/");
				//ip_address = inet_addr(straddr);
				ip_address = inet_addr(sToken1.c_str()) ;
				if (ip_address != INADDR_NONE)
					ip_block.mask = ip_address;
				else
					ip_block.mask = ~0; // mask is not required. (fills all bits with 1's)
			}
			else
				ip_block.mask = ~0 ;
				
			hosts_deny.push_back(ip_block);
			
		}
	}
	while ( (strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS) );
	closescript();	
}

bool cNetworkStuff::CheckForBlockedIP(sockaddr_in ip_address)
{
	unsigned int i;
	for ( i = 0; i < hosts_deny.size(); i++ )
		if ((ip_address.sin_addr.s_addr&hosts_deny[i].mask) == (hosts_deny[i].address&hosts_deny[i].mask))
			return true;

	return false;
}

void cNetworkStuff::SendUOX3(UOXSOCKET s, const void *point, int length, int test)
{
	// clConsole.send("xSend [%i] with %i -> ", s, length);

	if (boutlength[s]+length>=MAXBUFFER_ASYNCH) FlushBuffer(s);
	memcpy(&outbuffer[s][boutlength[s]], point, length);
	boutlength[s]+=length;

	// packetcount etc here !

	// clConsole.send("%i\n", boutlength[s]);

}

void cNetworkStuff::SendOSI(UOXSOCKET s, void *point, int length, int test)
{	
	memcpy(&outbuffer[s][boutlength[s]], point, length);
	boutlength[s]+=length;
	FlushBuffer(s);
}

void cNetworkStuff::SendSMARTWOLF(UOXSOCKET s, void *point, int length, int test)
{	
	clConsole.send("SMARTWOLF PacketSendStyle not implementined, using UOX3 Style");
	cNetworkStuff::SendUOX3(s, point, length, test);

}
*/
// LB 1'st Sept 2001
// Heart of sychrounous-send
/*
void cNetworkStuff::CountPackets(UOXSOCKET s, int &numpackets, long int & offsettolastfullpacket, bool & dataerror)
{
	int buff_len = boutlength[s];

	int ctr = 0;
	int pl, dyn_length;	
	unsigned char packet;
	bool er;

	numpackets = 0;
	dataerror = false;
	offsettolastfullpacket = -1L;

	while( ctr<buff_len)
	{

		// get current packet and it's length

		packet       = outbuffer[s][ctr+0];
		pl           = packlen_cl::size(packet);	
		dyn_length   = 0;

		if (pl==PACKET_LEN_NONE)
		{
			dataerror = true;
			break;
		}
		else if (pl==PACKET_LEN_DYNAMIC)
		{
			dyn_length = (int) (  ( (int) outbuffer[s][ctr+1]<<8) + (int) outbuffer[s][ctr+2] );				
		}

		if (dyn_length!=0) pl = dyn_length;

		 /// here we have the correct packet length in variable  pl, packet # in packet
         /// now let's see if data is correct

		if (pl>=MAXBUFFER_REAL)
		{
			dataerror = true;
			break;
		}

		/// check if packet is fully(!) in buffer

		if ( (ctr+pl) >= buff_len )
		{
           offsettolastfullpacket = ctr;
		   break;
		}

        er = cNetworkStuff::CheckPacket(s, packet, pl, ctr);

		if (er || numpackets>=1976)
		{
			dataerror=true;
			break;
		}	

		//// ok, everytihng ok, lets count it and goto next one;

        numpackets++;
		ctr+=pl;		

	}

}

bool cNetworkStuff::CheckPacket(UOXSOCKET s, unsigned char packetnumber, int length, unsigned long int offset)
{
	bool ok=true;

	switch(packetnumber)
	{
	    case 0x7E:
		{
			ok=true;
		}
		break;

		default:
		{
		    ok=true;
		}
		break;

	}

	return ok;
}
*/
// when we have erros in send, i.g synch error, lets try to fire an error message to client before disconnecting
// of course xsend can't be used, because it's called for erorrs IN xsend.
/*void cNetworkStuff::SendGoodByeMessageRaw(UOXSOCKET s)
{

}*/

UOXSOCKET calcSocketFromChar(P_CHAR pc)
{
	if (pc == NULL || pc->npc)
	{
		return -1;
	}
	register int j;
	for (j = 0; j < now; ++j)
	{
		if (currchar[j] == pc && perm[j]) 
			return j;
	}
	return -1;
}
