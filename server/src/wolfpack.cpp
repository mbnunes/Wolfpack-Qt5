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


#ifndef __unix__
 
#endif

#include "wolfpack.h"

#include "basics.h"
#if defined(__unix__)
#include <signal.h>
#endif
#include "verinfo.h"
#include "cmdtable.h"
#include "speech.h"
#include "SndPkg.h"
#include "sregions.h"
#include "im.h"
#include "remadmin.h"
#include "utilsys.h"

#undef DBGFILE
#define DBGFILE "wolfpack.cpp"
#include "debug.h"

#include "wip.h"

#ifndef __unix__
HANDLE hco;
CONSOLE_SCREEN_BUFFER_INFO csbi;

#define W95		0x1
#define W98		0x2
#define WME		0x3
#define WNT4    0x4
#define W2K		0x5
#define XP      0x6

int GetWindowsVersion()
{  
	
   OSVERSIONINFO winfo;
   winfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
   GetVersionEx(&winfo);

   if(winfo.dwPlatformId==VER_PLATFORM_WIN32_NT)
   {
	  if      (winfo.dwMajorVersion<=4)                              { return WNT4; }
	  else if (winfo.dwMajorVersion==5 && winfo.dwMinorVersion == 0) { return W2K;  }
	  else if (winfo.dwMajorVersion==5 && winfo.dwMinorVersion == 1) { return XP;   }
   }
   else if(winfo.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
   {
	 if(winfo.dwMinorVersion<10)   { return W95; }
	 else
	 if(winfo.dwMinorVersion<90) { return W98; }
	 else { return WME;	 }
   }
   return W95; // Makes compiler happier.
}

#endif

void clearscreen( void )
{

	unsigned long int y;

	#ifndef __unix__
	COORD xy;

	xy.X=0;
	xy.Y=0;
	FillConsoleOutputAttribute(hco, (FOREGROUND_RED)/*( BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)*/, csbi.dwSize.X*csbi.dwSize.Y, xy, &y);
	FillConsoleOutputCharacter(hco, ' ', csbi.dwSize.X*csbi.dwSize.Y, xy, &y);
	SetConsoleCursorPosition(hco, xy);
	#endif
}

void constart( void )
{

/*	#ifndef __unix__
	hco=GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hco, &csbi);
	SetConsoleTextAttribute(hco, (FOREGROUND_RED) ); // black backround and white-text got boring, LB :)
	clearscreen();
	#else
	//initscr(); cbreak(); noecho();nonl(); intrflush(stdscr,FALSE) ;scrollok(stdscr,TRUE);immedok(stdscr,TRUE); nodelay(stdscr,TRUE);
	int status ;
	status = fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK) ;
	termios	trmStuff ;
	if (tcgetattr(STDIN_FILENO,&termstate)!=0)
	{
		cout << "error getting console state" << endl ;
		cout << strerror(errno) << endl ;
	}
	tcgetattr(STDIN_FILENO,&trmStuff) ;
	//trmStuff.c_iflag = trmStuff.c_iflag | IGNCR  ;
	trmStuff.c_lflag = trmStuff.c_lflag & (~ECHO) ;
	trmStuff.c_lflag = trmStuff.c_lflag & (~ICANON) ;
	if (tcsetattr(STDIN_FILENO,TCSANOW,&trmStuff) !=0 )
	{
		cout << "error setting terminal" << endl ;
		cout << strerror(errno) << endl ;
	}
	
	#endif
	
*/
}

//#endif

///////////
// Name:	inVisRange
// Purpose:	checks if position 1 and 2 are in visual range
// history:	by Duke, 18.11.2000
//
bool inVisRange(int x1, int y1, int x2, int y2)
{
	return (max(abs(x1-x2), abs(y1-y2)) <= VISRANGE);
}

int inrange1 (UOXSOCKET a, UOXSOCKET b) // Are players from sockets a and b in visual range
{
	if (!(a==b)
		&& inVisRange(currchar[a]->pos.x, currchar[a]->pos.y, currchar[b]->pos.x, currchar[b]->pos.y))
		return 1;
	return 0;
}

bool inrange1p (PC_CHAR pca, P_CHAR pcb) // Are characters a and b in visual range
{
	if (pca == NULL || pcb == NULL) return false;

	return inVisRange(pca->pos.x, pca->pos.y, pcb->pos.x, pcb->pos.y);
}

/*
int inrange1p (CHARACTER a, CHARACTER b) // Are characters a and b in visual range
{
	if (a<0 || a>cmem || b<0 || b>cmem) return 0; //LB

	if(a==b || inVisRange(chars[a].pos.x,chars[a].pos.y,chars[b].pos.x,chars[b].pos.y))
		return 1;
	return 0;
}
*/
///////////
// Name:	inRange
// Purpose:	checks if position 1 and 2 are in given range
// history:	by Duke, 19.11.2000
//
inline bool inRange(int x1, int y1, int x2, int y2, int range)
{
	return (max(abs(x1-x2), abs(y1-y2)) <= range);
}

int inrange2 (UOXSOCKET s, P_ITEM pi) // Is item i in visual range for player on socket s
{
	
	P_CHAR pc_currchar = currchar[s];
	int vr=Races[pc_currchar->race]->VisRange;
	if (pi == NULL) // blackwind Crash fix 
		return 0; 

	if (pi->id1==0x40 &&(pi->id2>=0x7C && pi->id2<=0x7F))	// a large house ??
		vr=BUILDRANGE;
	return inRange(pc_currchar->pos.x,pc_currchar->pos.y,pi->pos.x,pi->pos.y,vr);
}

bool iteminrange (const UOXSOCKET s, const P_ITEM pi, const int distance)
{
	P_CHAR pc_currchar = currchar[s];
	if (pc_currchar->isGM()) return 1;
	return inRange(pc_currchar->pos.x,pc_currchar->pos.y,pi->pos.x,pi->pos.y,distance);
}

unsigned char npcinrange (UOXSOCKET s, P_CHAR pc, int distance)
{
	if (pc == NULL) 
		return 0;
	P_CHAR pc_currchar = currchar[s];
	if (pc_currchar->isGM()) 
		return 1;
	return inRange(pc_currchar->pos.x,pc_currchar->pos.y, pc->pos.x, pc->pos.y, distance);
}


//================================================================================
//
// signal handlers
#if defined(__unix__)
void signal_handler(int signal)
{
//	clConsole.send("In signal handler\n") ;
	switch (signal)
	{
	case SIGHUP:
		loadspawnregions();
		loadregions();
		loadmetagm();
		loadmenuprivs();
		loadserverscript();
		Network->LoadHosts_deny();
		break ;
		
	case SIGUSR1:
		Accounts->LoadAccounts();
		break ;
	case SIGUSR2:
		cwmWorldState->savenewworld(1);
		//saveserverscript(1);
		saveserverscript();
		break ;
	case SIGTERM:
		keeprun = 0 ;
		break;
	default:
		break;
	}
}
	
#endif
// Initialize deamon
void init_deamon()
{
/*
#if defined(__unix__)
	
	int i ;
	pid_t pid ;

	if ((pid = fork() ) != 0)
		exit(0) ; //
	setsid() ;
	signal(SIGHUP, SIG_IGN) ;
	if ((pid=fork()) != 0)
	{
		fstream fPid ;
		fPid.open("wolfpack.pid",ios::out) ;
		fPid << pid <<endl;
		fPid.close() ;
		exit(0) ;
	}
	// We should close any dangling descriptors we have
	for (i=0 ; i < 64 ; i++)
	{
		close(i) ;
	}
	// Ok, we are a true deamon now, so we should setup our signal handler
	// We can use SIGHUP, SIGINT, and SIGWINCH as we should never recive them
	// So we will use SIGHUP to reload our scripts (kinda a standard for sighup to be reload)
	// We will use a SIGUSR2 to be world save
	// and SIGUSR1 for an Account reload
	signal(SIGUSR2,&signal_handler) ;
	signal(SIGHUP,&signal_handler) ;
	signal(SIGUSR1,&signal_handler) ;
	signal(SIGTERM,&signal_handler) ;
// we have this here, because convient, but should be set regardless of deamon or not.
// Keeps a disconnected socket from terminating the server.
#if !defined(__linux__)
	signal(SIGPIPE,&signal_handler) ;
#endif
	
#endif
*/
}


unsigned int dist(Coord_cl &a, Coord_cl &b) // Distance between position a and b
{
/*	int c;
	int dx=abs(xa-xb);
	int dy=abs(ya-yb);

	c=(int)(hypot(dx, dy));               // hypot is POSIX standard, also works in Win32.
*/
	return a.distance(b);
}

unsigned int chardist (P_CHAR a, P_CHAR b) // Distance between characters a and b
{
	if (a == NULL || b == NULL)
		return 30;
	return dist(a->pos, b->pos);
}


unsigned int itemdist(P_CHAR pc, P_ITEM pi)// calculates distance between item i and player a
{
	if ( pi == NULL || pc == NULL )
		return 30;

	return dist(pc->pos, pi->pos);
}

bool online(P_CHAR pc) // Is the player owning the character c online
{
	int i;

	UOXSOCKET k = calcSocketFromChar(pc); //LB crashfix
	if (k == -1 || pc->isNpc()) 
		return false;
	if(pc != NULL && Accounts->GetInWorld(pc->account) == pc->serial) 
		return true;//Instalog
	else
	{
		for (i=0;i<now;i++)
			if ((currchar[i] == pc) && (perm[i])) 
				return true;
	}
	return false;
}

int bestskill(P_CHAR pc_p) // Which skill is the highest for character p
{
	int i,a=0,b=0;
	if ( pc_p == NULL)
		return 0;
	for (i=0;i<TRUESKILLS;i++) 
		if (pc_p->baseskill[i]>b)
		{
			a=i;
			b=pc_p->baseskill[i];
		}
	return a;
}

void loadcustomtitle() // for custom titles
{
	int titlecount=0;
	char sect[512];

	for (int a=0; a<ALLSKILLS; a++)
	{
		title[a].fame[0] = 0;
		title[a].other[0] = 0;
		title[a].prowess[0] = 0;
		title[a].skill[0] = 0;
	}

	openscript("titles.scp");
	strcpy(sect,"SKILL");

	if(!i_scripts[titles_script]->find(sect))
	{
		closescript();
		return;
	}

	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			if ( !strcmp((char*)script1, "NONE") ) ;
			else strcpy(title[titlecount].skill,(char*)script1);

			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();

	script1[0]=0;
	titlecount=0;
	openscript("titles.scp");

	strcpy(sect,"PROWESS");
	if(!i_scripts[titles_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			if ( !strcmp((char*)script1, "NONE") ) ;
			else strcpy(title[titlecount].prowess,(char*)script1);
			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();

	script1[0]=0;
	titlecount=0;
	openscript("titles.scp");
	strcpy(sect,"FAME");

	if(!i_scripts[titles_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			if ( !strcmp((char*)script1, "NONE") ) ;
			else strcpy(title[titlecount].fame,(char*)script1);

			if (titlecount==23)
			{
				title[titlecount].fame[0] = '\0';
				strcpy(title[++titlecount].fame, (char*)script1);
			}

			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();

	script1[0]=0;
	titlecount=0;
	openscript("titles.scp");
	strcpy(sect,"OTHER");

	if(!i_scripts[titles_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read1(); // dont split paramters
		if (script1[0]!='}')
		{
			if ( !strcmp((char*)script1, "NONE") ) ;
			else strcpy(title[titlecount].other,(char*)script1);
			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();

}

char *title1(P_CHAR pc) // Paperdoll title for character p (1)
{
	int titlenum = 0;
	int x = pc->baseskill[bestskill(pc)];

	//if (x>=1000) titlenum=10;
	//else if (x>=960) titlenum=9;
	if (x>=1000) titlenum=8;
	else if (x>=900) titlenum=7;
	else if (x>=800) titlenum=6;
	else if (x>=700) titlenum=5;
	else if (x>=600) titlenum=4;
	else if (x>=500) titlenum=3;
	else if (x>=400) titlenum=2;
	else if (x>=300) titlenum=1;
    if(pc->isNpc())
    strcpy(prowesstitle," ");
    else
	strcpy(prowesstitle,title[titlenum].prowess);
	return prowesstitle;
}

char *title2(P_CHAR pc) // Paperdoll title for character p (2)
{

	int titlenum=0;
	int x=bestskill(pc);
	titlenum=x+1;
    if(pc->isNpc())
    strcpy(skilltitle," ");
    else
	strcpy(skilltitle,title[titlenum].skill);

	return skilltitle;
}

char *title3(P_CHAR pc) // Paperdoll title for character p (3)
{
	char thetitle[50];
	int titlenum=0;
	int k;
	unsigned int f;

	k=pc->karma;
	f=pc->fame;
	thetitle[0] = 0;

	if (k>=10000)
	{
		titlenum=3;
		if (f>=5000) titlenum=0;
		else if (f>=2500) titlenum=1;
		else if (f>=1250) titlenum=2;
	}
	else if ((5000<=k)&&(k<9999))
	{
		titlenum=7;
		if (f>=5000) titlenum=4;
		else if (f>=2500) titlenum=5;
		else if (f>=1250) titlenum=6;
	}
	else if ((2500<=k)&&(k<5000))
	{
		titlenum=11;
		if (f>=5000) titlenum=8;
		else if (f>=2500) titlenum=9;
		else if (f>=1250) titlenum=10;
	}
	else if ((1250<=k)&&(k<2500))
	{
		titlenum=15;
		if (f>=5000) titlenum=12;
		else if (f>=2500) titlenum=13;
		else if (f>=1250) titlenum=14;
	}
	else if ((625<=k)&&(k<1250))
	{
		titlenum=19;
		if (f>=5000) titlenum=16;
		else if (f>=1000) titlenum=17;
		else if (f>=500) titlenum=18;
	}
	else if ((-635<k)&&(k<625))
	{
		titlenum=23;
		if (f>=5000) titlenum=20;
		else if (f>=2500) titlenum=21;
		else if (f>=1250) titlenum=22;
	}
	else if ((-1250<k)&&(k<=-625))
	{
		titlenum=24;
		if (f>=10000) titlenum=28;
		else if (f>=5000) titlenum=27;
		else if (f>=2500) titlenum=26;
		else if (f>=1250) titlenum=25;
	}
	else if ((-2500<k)&&(k<=-1250))
	{
		titlenum=29;
		if (f>=5000) titlenum=32;
		else if (f>=2500) titlenum=31;
		else if (f>=1250) titlenum=30;
	}
	else if ((-5000<k)&&(k<=-2500))
	{
		titlenum=33;
		if (f>=10000) titlenum=37;
		else if (f>=5000) titlenum=36;
		else if (f>=2500) titlenum=35;
		else if (f>=1250) titlenum=34;
	}
	else if ((-10000<k)&&(k<=-5000))
	{
		titlenum=38;
		if (f>=5000) titlenum=41;
		else if (f>=2500) titlenum=40;
		else if (f>=1250) titlenum=39;
	}
	else if (k<=-10000)
	{
		titlenum=42;
		if (f>=5000) titlenum=45;
		else if (f>=2500) titlenum=44;
		else if (f>=1250) titlenum=43;
	}
	sprintf(thetitle,"%s ",title[titlenum].fame);
	if (titlenum==24) thetitle [0] = 0;

	if (f>=10000) // Morollans bugfix for repsys
	{
		if (pc->kills >= (unsigned)repsys.maxkills)
		{
			if (pc->id2==0x91) strcpy(fametitle,"The Murderous Lady ");//Morrolan rep
			else strcpy(fametitle,"The Murderer Lord ");
		}
		else if (pc->id2==0x91) sprintf(fametitle,"The %sLady ",thetitle);
		else sprintf(fametitle,"The %sLord ",thetitle);
	}
	else
	{
		if (pc->kills >= (unsigned)repsys.maxkills)
		{
			strcpy(fametitle,"The Murderer "); //Morrolan rep
		}
		else if (!(strcmp(thetitle," ")==0)) sprintf(fametitle,"The %s",thetitle);
		else fametitle[0] = 0;
	}
	return fametitle;
}


char *complete_title(P_CHAR pc) // generates the ENTIRE title plus criminal stuff
{
	char tempstr[1024];
	if (pc == NULL) return "error";

	if (pc->account==0 && pc->isGM()) // Ripper..special titles for admins :)
	{
		sprintf(tempstr, "%s %s %s", title[6].other, pc->name.c_str(), pc->title.c_str());
	}
	else if (pc->isGM() && pc->account!=0)
	{//GM.
		sprintf(tempstr, "%s %s", pc->name.c_str(), pc->title.c_str());
	}
	// ripper ..rep stuff
	else if ((pc->crimflag>0) && (!(pc->dead) && (pc->kills<4)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[0].other, pc->name.c_str(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=5) && (!(pc->dead) && (pc->kills<10)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[1].other, pc->name.c_str(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=10) && (!(pc->dead) && (pc->kills<20)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[2].other, pc->name.c_str(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=20) && (!(pc->dead) && (pc->kills<50)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[3].other, pc->name.c_str(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=50) && (!(pc->dead) && (pc->kills<100)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[4].other, pc->name.c_str(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=100) && (!(pc->dead)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[5].other, pc->name.c_str(), pc->title.c_str(), title1(pc), title2(pc));
	} // end of rep stuff
	else
	{//Player.
		sprintf(tempstr, "%s%s", title3(pc), pc->name.c_str());		//Repuation + Name
		{//NoTownTitle
			strcpy((char*)temp,tempstr);
			if (pc->title.size()>0)
			{//Titled & Skill
				sprintf(tempstr, "%s %s %s, %s %s", temp, Races[pc->race]->RaceName.c_str(), pc->title.c_str(), title1(pc), title2(pc));
			}
			else
			{//Just skilled
				sprintf(tempstr, "%s %s, %s %s", Races[pc->race]->RaceName.c_str(), temp, title1(pc), title2(pc));
			}
		}
	}

	strcpy(completetitle, tempstr);
	return completetitle;
}

void gcollect () // Remove items which were in deleted containers
{
	int removed = 0, rtotal = 0;
	bool bdelete;
	LogMessage("Performing Garbage Collection...");
	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); ++iter_items)
	{
		P_ITEM pi = iter_items.GetData();
		if (pi->free || pi->isInWorld()) 
			continue;
		bdelete = true;
		// find the container if theres one.
		if (isCharSerial(pi->contserial))
		{
			P_CHAR pc = FindCharBySerial(pi->contserial);
			if (pc != NULL)
				bdelete = false;
		}
		else
		{
			P_ITEM pContainer = GetOutmostCont(pi);
			if (pContainer != NULL)
				bdelete = false;
		}
		if (bdelete)
		{
			--iter_items; // Go back for a little.
			Items->DeleItem( pi ); // Warning, iterator became invalid!
			removed++;
		}
	}
	rtotal += removed;
	
	sprintf((char*)temp, " gc: Removed %i items", rtotal);
	if (rtotal > 0) LogMessage((char*)temp);
}

void loadmenuprivs()
{
	int a,b;
	char sect[512];
	int i,k,m;

	LogMessage("Loading add-menu privs...");

	for (a=0;a<64;a++)	// initializing
	{
		for (b=0;b<256;b++)
		{
			menupriv[a][b]=-1;
		}
	}

	// script loading ....
	openscript("menupriv.scp");
	i=-1;k=0;
	unsigned long loopexit=0;
	do
	{
		i++;
		sprintf(sect, "MENU_CLEARANCE %i", i);
		if (i_scripts[menupriv_script]->find(sect))
		{
			k++;m=-1;
			loopexit=0;
			do
			{
				read2();
				if (script1[0] != '}')
				{
					m++;
					menupriv[i][m] = str2num(script1);
				}
			} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		}
	} while (i<63);

	char tmp[50];
	sprintf(tmp,"%i menu-privs loaded",k-2);
	LogMessage(tmp);
	closescript();
}

void item_char_test()
{
	LogMessage("Starting item consistancy check");
	
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
	{
		P_ITEM pi = iterItems.GetData();
		char tmp[150];
		if (pi->serial==pi->contserial)
		{
			sprintf(tmp,"ALERT ! item %s [serial: %i] has dangerous container value, autocorrecting",pi->name.c_str(),pi->serial);
			LogWarning(tmp);
			pi->SetContSerial(-1);
		}

		if (pi->serial==pi->GetOwnSerial())
		{
			sprintf(tmp,"ALERT ! item %s [serial: %i] has dangerous owner value",pi->name.c_str(),pi->serial);
			LogWarning(tmp);
			pi->SetOwnSerial(-1);
		}

		if (pi->serial==pi->spawnserial)
		{
			clConsole.send("\nALERT ! item %s [serial: %i] has dangerous spawn value\n",pi->name.c_str(),pi->serial);
			pi->SetSpawnSerial(-1);
		}
	}

	// check for stabled pets that dont have a stablemaster anymore
	P_CHAR p_pet;
	int stablemaster_serial;
	AllCharsIterator iter_char;
	for ( iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		p_pet = iter_char.GetData();
		if (p_pet != NULL)
		{
			 stablemaster_serial = p_pet->stablemaster_serial;
			 if (stablemaster_serial != INVALID_SERIAL) // stabled ?
			 {
				P_CHAR pc_j = FindCharBySerial(stablemaster_serial);
				if (pc_j == NULL)
				{
					stablesp.remove(stablemaster_serial, p_pet->serial);
					p_pet->stablemaster_serial = INVALID_SERIAL;
					p_pet->timeused_last=getNormalizedTime();
					p_pet->time_unused=0;
					mapRegions->Add(p_pet);
					LogMessage("Stabled animal got freed because stablemaster died");
					clConsole.send("stabled animal got freed because stablemaster died");
				}
			 }
		}
	}
	clConsole.send(" Done!\n");
}



void savelog(const char *msg, char *logfile)
{
		FILE *file;
		file=fopen(logfile,"a");
		fprintf(file,"[%s] %s",(getRealTimeString()).c_str(),msg);

#ifdef DEBUG
		clConsole.send("DEBUG: Logging to %s\n", logfile);
#endif

		fclose(file);
}

void splitline() // For putting single words of cline into comm array
{
	int i=0;
	char *s;
	char *d;

	d=" ";
	s=strtok((char*)cline,d);
	unsigned long loopexit=0;
	while ( (s!=NULL) && (++loopexit < MAXLOOPS) )
	{
		comm[i]=(unsigned char*)s;
		i++;
		s=strtok(NULL,d);
	}
	tnum=i;
}

int makenumber (int countx) // Converts decimal string comm[count] to int
{
	if (comm[countx] == NULL)
		return 0;

	int i;
	sscanf((char*)comm[countx], "%i", &i);
	return i;
}

int hexnumber(int countx) // Converts hex string comm[count] to int
{
	// sscanf is an ANSI function to read formated data from a string.
	if (comm[countx] == NULL)
		return 0;

	int i;
	sscanf((char*)comm[countx], "%x", &i);

	return i;
}

void wornitems(UOXSOCKET s, P_CHAR pc) // Send worn items of player j
{
	pc->onhorse = false;
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer(contsp.getData(pc->serial));
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL && !pi->free)
		{
			if (pi->layer==0x19) 
				pc->onhorse = true;
			wearIt(s,pi);
		}
	}
}

void all_items(int s) // Send ALL items to player
{
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd(); ++iterItems)
	{
		P_ITEM pi = iterItems.GetData();
		if (!pi->free) 
			senditem(s, pi);
	}
}

void showcname (UOXSOCKET s, P_CHAR pc_i, char b) // Singleclick text for a character
{
	int c;
	int x;

	P_CHAR pc_currchar = currchar[s];

	if (pc_i == NULL)
		return;

	if (pc_i->squelched)
	{
		sprintf((char*)temp," [%s]",title[7].other);
		if (!(strcmp(title[7].other,""))) temp[0] = 0;
		itemmessage(s, (char*)temp ,pc_i->serial);
	}

	if ((pc_currchar->canSeeSerials()) || b)
	{
		sprintf((char*)temp, "%s [%08x]", pc_i->name.c_str(), pc_i->serial);
	}
	else
	{
		if (pc_i->isPlayer())
		{
			if (pc_currchar->isGM())
			{
				sprintf((char*)temp, "[%08x]", pc_i->serial);
				itemmessage(s,(char*)temp, pc_i->serial);
			}
			if (!online(pc_i)) sprintf((char*)temp, "%s (%s)",title[8].other, pc_i->name.c_str());
			else strcpy((char*)temp,pc_i->name.c_str());
			if (!strcmp(title[8].other,"")) temp[0] = 0;
		}
		else
		{
			temp[0] = 0;
			x=0;
			char temp2[256] = {0,};
			strcpy(temp2, pc_i->name.c_str());
			do
			{
				c=temp2[x];
				if ((c!=0))
				{
					if(c=='_')
						c=' ';
					sprintf((char*)temp, "%s%c", temp, c);
				}
				x++;
			}
			while ((c!=0) && x<50 );
		}
	}
	textflags(s, pc_i, (char*)temp);
}

/////////////////
// Name:	deathstuff
// History:	by UnKnown (Touched tabstops by Tauriel Dec 28, 1998)
// Purpose:	creates a corpse, moves items to it, take out of war mode, does animation and sound, etc.
//
void deathstuff(P_CHAR pc_player)
{
	int z, l, q, ele;
	char murderername[50]; //AntiChrist
	unsigned char clearmsg[8];
	int nType=0;

	if ( pc_player == NULL )
		return;


	if (pc_player->dead || pc_player->npcaitype == 17 || pc_player->isInvul()) 
		return;

	if(pc_player->polymorph)
	{
		pc_player->setId(pc_player->xid);
		pc_player->polymorph = false;
		teleport(pc_player);
	}

	pc_player->xid = pc_player->id(); // lb bugfix
	pc_player->xskin = pc_player->skin;
	pc_player->murdererSer = 0;	// Reset previous murderer serial # to zero

	if (pc_player->attacker != INVALID_SERIAL)
	{
		P_CHAR pc_attacker = FindCharBySerial(pc_player->serial);
		strcpy(murderername, pc_attacker->name.c_str()); 
	}
	else 
		murderername[0]=0;

	AllCharsIterator iter_char;
	for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR pc_t = iter_char.GetData();
		if (pc_t->targ == pc_player->serial && !pc_t->free)
		{
			if (pc_t->npcaitype==4) //LB change from 0x40 to 4
			{
				pc_t->summontimer=(uiCurrentTime+(MY_CLOCKS_PER_SEC*20));
				pc_t->npcWander=2;
				pc_t->setNextMoveTime();
				npctalkall(pc_t,"Thou have suffered thy punishment, scoundrel.",0);
			}
			pc_t->targ=INVALID_SERIAL;
			pc_t->timeout=0;
			if (pc_t->attacker != INVALID_SERIAL)
			{
				P_CHAR pc_attacker = FindCharBySerial(pc_t->attacker);
				pc_attacker->resetAttackFirst();
				pc_attacker->attacker = INVALID_SERIAL; // lb crashfix
			}
			pc_t->attacker = INVALID_SERIAL;
			pc_t->resetAttackFirst();

			if (pc_t->npcaitype==4 || pc_t->npcaitype==9) // Ripper...so non human npcs disapear if killed by guards.
			{
				if (pc_player->isNpc() && !pc_player->isHuman())
					Npcs->DeleteChar(pc_player);
			}

			if(pc_t->isPlayer() && !pc_t->inGuardedArea())
			{//AntiChrist
				Karma(pc_t, pc_player, (0-(pc_player->karma)));
				Fame(pc_t,pc_player->fame);
				//murder count \/
				if ((pc_player->isPlayer())&&(pc_t->isPlayer()))//Player vs Player
				{
					if(pc_player->isInnocent() && (Races.CheckRelation(pc_t,pc_player)==1) && Guilds->Compare( pc_t, pc_player ) == 0 && pc_t->attackfirst == 1)
					{
						// Ask the victim if they want to place a bounty on the murderer (need gump to be added to
						// BountyAskViction() routine to make this a little nicer ) - no time right now
						// BountyAskVictim( pc_player->serial, pc_t->serial );
						pc_player->murdererSer = pc_t->serial;

						pc_t->kills++;
						sprintf((char*)temp, "You have killed %i innocent people.", pc_t->kills);
						UOXSOCKET sock = calcSocketFromChar(pc_t);
						sysmessage(sock, (char*)temp);
						if (pc_t->kills==(unsigned)repsys.maxkills)
							sysmessage(sock, "You are now a murderer!");
						setcharflag(pc_t);//AntiChrist
					}

					if (SrvParms->pvp_log)
					{
						sprintf((char*)temp,"%s was killed by %s!\n",pc_player->name.c_str(),pc_t->name.c_str());
						savelog((char*)temp,"PvP.log");
					}
				}
			}//if !npc
			if (pc_t->isNpc() && pc_t->war) 
				npcToggleCombat(pc_t); // ripper
		}
	}

	P_ITEM pi_backpack = Packitem(pc_player);
	z=calcSocketFromChar(pc_player);
	if (z != -1) 
		unmounthorse(z);

	unsigned int ci=0;
	P_ITEM pi_j;
	vector<SERIAL> vecContainer(contsp.getData(pc_player->serial));
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi_j = FindItemBySerial(vecContainer[ci]);
		if (pi_j->type==1 && pi_j->pos.x==26 && pi_j->pos.y==0 &&
			pi_j->pos.z==0 && pi_j->id()==0x1E5E )
		{
			endtrade(pi_j->serial);
		}
	}
	ele=0;
	if(pc_player->isPlayer()) pc_player->id1=0x01; // Character is a ghost
	if (pc_player->xid==0x0191)
	{
		pc_player->setId(0x0193);	// Male or Female
	}
	else
	{
		pc_player->setId(0x0192);
	}

	PlayDeathSound(pc_player);
	pc_player->skin = 0x0000; // Undyed
	pc_player->dead = true;	// Dead
	pc_player->hp = 0;		// With no hp left
	pc_player->poisoned = 0;
	pc_player->poison = 0;	//AntiChrist
	// Make the corpse
	sprintf((char*)temp,"corpse of %s",pc_player->name.c_str());
	const P_ITEM pi_c = Items->SpawnItem(pc_player, 1, (char*)temp, 0, 0x2006, pc_player->xskin, 0);
	if(pi_c==NULL) return;//AntiChrist to preview crashes
	// Corpse highlighting.. Ripper
	if(pc_player->isPlayer())
	{
	    if (pc_player->isInnocent()) pi_c->more2=1;
	    else if (pc_player->isCriminal()) pi_c->more2=2;
	    else if (pc_player->isMurderer()) pi_c->more2=3;
        pi_c->ownserial=pc_player->serial;
	}// end corpse highlighting.

//    if(pc_player->isNpc() || pc_player->isPlayer())

	ele=pi_c->amount=pc_player->xid; // Amount == corpse type
	pi_c->morey = ishuman(pc_player);//is human?? - AntiChrist
	pi_c->carve=pc_player->carve;//store carve section - AntiChrist
	pi_c->name2 = pc_player->name;

	pi_c->type=1;
	pi_c->MoveTo(pc_player->pos.x,pc_player->pos.y,pc_player->pos.z);

	pi_c->more1=nType;
	pi_c->dir=pc_player->dir;
	pi_c->corpse=1;
	pi_c->startDecay();
	
	//JustMichael -- If it was a player set the ownerserial to the player's
	if( pc_player->isPlayer() )
	{
		pi_c->SetOwnSerial(pc_player->serial);
		pi_c->more4 = char( SrvParms->playercorpsedecaymultiplier&0xff ); // how many times longer for the player's corpse to decay
	}

	//AntiChrist -- stores the time and the murderer's name
	pi_c->murderer = murderername;
	pi_c->murdertime = uiCurrentTime;
	// Put objects on corpse

	vecContainer.clear();
	vecContainer = contsp.getData(pc_player->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi_j = FindItemBySerial(vecContainer[ci]);
		// for BONUS ITEMS - remove bonus
		pc_player->removeItemBonus(pi_j);
		if ((pi_j->trigon==1) && (pi_j->layer >0) && (pi_j->layer!=15) && (pi_j->layer<19))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
		{
			Trig->triggerwitem(z, pi_j, 1); // trigger is fired when item destroyed
		}
		if ((pi_j->contserial== pc_player->serial) && (pi_j->layer!=0x0B) && (pi_j->layer!=0x10))
		{//Let's check all items, except HAIRS and BEARD
			// Ripper...so order/chaos shields disappear when on corpse backpack.
			if( pi_j->id1 == 0x1B && ( pi_j->id2 == 0xC3 || pi_j->id2 == 0xC4 ) )
			{
				soundeffect2(pc_player, 0x01FE);
				staticeffect(pc_player, 0x37, 0x2A, 0x09, 0x06);
				Items->DeleItem( pi_j );
			}
			if (pi_j->type==1 && pi_j->layer!=0x1A && pi_j->layer!=0x1B &&
				pi_j->layer!=0x1C && pi_j->layer!=0x1D)
			{//if this is a pack but it's not a VendorContainer(like the buy container) or a bankbox
				unsigned int ci1;
				vector<SERIAL> vecContainer = contsp.getData(pi_j->serial);
				for ( ci1 = 0; ci1 < vecContainer.size(); ++ci1)
				{
					P_ITEM pi_k = FindItemBySerial(vecContainer[ci1]);
					if ( (!(pi_k->priv&0x02)) && (pi_k->type!=9))//Morrolan spellbook disappearance fix
					{//put the item in the corpse only of we're sure it's not a newbie item or a spellbook
						pi_k->layer=0;
						pi_k->SetContSerial(pi_c->serial);
						pi_k->SetRandPosInCont(pi_c);
						// Ripper...so order/chaos shields disappear when on corpse backpack.
						if( pi_k->id1 == 0x1B && ( pi_k->id2 == 0xC3 || pi_k->id2 == 0xC4 ) )
						{
							soundeffect2(pc_player, 0x01FE);
							staticeffect(pc_player, 0x37, 0x2A, 0x09, 0x06);
							Items->DeleItem( pi_k );
						}
						RefreshItem(pi_k);//AntiChrist
					}
				}
			}
			else if (pi_j->layer==0x1A)
			{//else if it's a NPC vendor special container
				clearmsg[0]=0x3B;
				clearmsg[1]=0x00;
				clearmsg[2]=0x08;
				LongToCharPtr(pc_player->serial, &clearmsg[3]);
				clearmsg[7]=0x00;
				for (l=0;l<now;l++)
					if (perm[l] && inrange1p(pc_player, currchar[l])) 
						Xsend(l, clearmsg, 8);
			}//else if it's a normal item but ( not newbie and not bank items )
			else if ((!(pi_j->priv&0x02)) && pi_j->layer!=0x1D)
			{
				if (pi_j != pi_backpack)
				{
					pi_j->SetContSerial(pi_c->serial);
				}
			}
			else if(pi_backpack != NULL && pi_j->layer!=0x1D)
			{//else if the item is newbie put it into char's backpack
				if(pi_j != pi_backpack)
				{
					pi_j->layer = 0;
					pi_j->SetContSerial(pi_backpack->serial);
				}
			}
			if ((pi_j->layer==0x15)&&(pc_player->shop==0)) pi_j->layer=0x1A;
			pi_j->pos.x = RandomNum(20, 70);
			pi_j->pos.y = RandomNum(85, 160);
			pi_j->pos.z=9;
			SndRemoveitem(pi_j->serial);
			RefreshItem(pi_j);//AntiChrist
		}
		if ((pi_j->contserial == pc_player->serial)&& ((pi_j->layer==0x0B)||(pi_j->layer==0x10)))
		{
			pi_j->name = "Hair/Beard";
			pi_j->pos.x=0x47;
			pi_j->pos.y=0x93;
			pi_j->pos.z=0;
		}
	}
	if (pc_player->isPlayer())
	{
		strcpy((char*)temp,"a Death Shroud");
		const P_ITEM pi_c = Items->SpawnItem(z, pc_player, 1, (char*)temp, 0, 0x20, 0x4E, 0, 0, 0);
		if(pi_c == NULL) return;
		pc_player->robe = pi_c->serial; 
		pi_c->SetContSerial(pc_player->serial);
		pi_c->layer=0x16;
		pi_c->def=1;
	}
	if (SrvParms->showdeathanim) 
		deathaction(pc_player, pi_c);
	if (pc_player->account!=-1) // LB
	{
		
		teleport(pc_player);
	
		q = calcSocketFromChar(pc_player);
		if (q>-1) deathmenu(q);
	}
//	if ((ele==13)||(ele==15)||(ele==16)||(ele==574))//-Frazurbluu, we're gonna remove this strange little function :)
// it becomes OSI exact with it removed! -Fraz- I DO NEED TO CHECK ENERGY VORTEXS!!!!!!!!!!!!!!!!!!!!!!
//	{		// *** This looks very strange to me! Turning the shroud into a backpack ??? Duke 9.8.2k ***
//		strcpy(pi_c->name,"a backpack");
//		pi_c->color1=0;
//		pi_c->color2=0;
//		pi_c->amount = 1;
//		pi_c->setId(0x09B2);
//		pi_c->corpse=0; 
//	}
	RefreshItem(pi_c);//AntiChrist
	if (pc_player->isNpc()) 
		Npcs->DeleteChar(pc_player);
	if(ele==65535) Items->DeleItem(pi_c);
}

int GetBankCount( P_CHAR pc, unsigned short itemid, unsigned short color )
{
	if( pc == NULL )
		return 0;
	SERIAL serial = pc->serial;
	long int goldCount = 0;
//	int counter2 = 0;
	unsigned int ci;
	vector<SERIAL> vecOwn = ownsp.getData(serial);
	for( ci = 0; ci < vecOwn.size(); ci++ )
	{
		P_ITEM pj =FindItemBySerial(vecOwn[ci]);
		if( pj != NULL )
		{
			if( pj->ownserial == serial && pj->type == 1 && pj->morex == 1 )
			{
				unsigned int counter2;
				vector<SERIAL> vecContainer = contsp.getData(pj->serial);
				for( counter2 = 0; counter2 < vecContainer.size(); counter2++ )
				{
					P_ITEM pi = FindItemBySerial(vecContainer[counter2]);
					if( pi != NULL )
					{
						if( pi->contserial == pj->serial )
						{
							if( pi->id() == itemid )
							{
								if( pi->color == color )
									goldCount += pi->amount;
							}
						}
					}
				}
			}
		}
	}
	return goldCount;
}


int DeleBankItem( P_CHAR pc, unsigned short itemid, unsigned short color, int amt )
{
	if( pc == NULL )
		return amt;
	SERIAL serial = pc->serial;
	//int counter2 = 0;
	int total = amt;
	unsigned int ci;
	vector<SERIAL> vecOwn = ownsp.getData(serial);
	for( ci = 0; ci < vecOwn.size() && total > 0; ci++ )
	{
		P_ITEM pj = FindItemBySerial(vecOwn[ci]);
		if( pj != NULL )
		{
			if( pj->ownserial == serial && pj->type == 1 && pj->morex == 1 )
			{
				unsigned int counter2;
				vector<SERIAL> vecContainer = contsp.getData(pj->serial);
				for( counter2 = 0; counter2 < vecContainer.size() && total > 0; counter2++ )
				{
					P_ITEM pi = FindItemBySerial(vecContainer[counter2]);
					if( pi != NULL )
					{
						if( pi->contserial == pj->serial )
						{
							if( pi->id() == itemid )
							{
								if( pi->color == color )
								{
									if( total >= pi->amount )
									{
										total -= pi->amount;
										Items->DeleItem( pi );
									}
									else
									{
										pi->amount -= total;
										total = 0;
										RefreshItem( pi );
									}

								}
							}
						}
					}
				}
			}
		}
	}
	return total;
}

void usehairdye(UOXSOCKET s, P_ITEM piDye)	// x is the hair dye bottle object number
{
	if (piDye == NULL)
		return;

	P_ITEM pi;
	P_CHAR pc_currchar = currchar[s];

	unsigned int ci = 0;
	vector<SERIAL> vecContainer = contsp.getData(pc_currchar->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if(pi->layer==0x10 || pi->layer==0x0B)//beard(0x10) and hair
		{
			pi->color = piDye->color;	//Now change the color to the hair dye bottle color!
			RefreshItem(pi);
		}
	}
	Items->DeleItem(piDye);	//Now delete the hair dye bottle!
}

void explodeitem(int s, P_ITEM pi)
{
	unsigned int dmg=0,len=0;
	unsigned int dx,dy,dz;
//	int cc=currchar[s];

	if (pi == NULL)
		return;
	P_CHAR pc_currchar = currchar[s];

	// - send the effect (visual and sound)
	if (!pi->isInWorld()) //bugfix LB
	{
		pi->pos.x=pc_currchar->pos.x;
		pi->pos.y=pc_currchar->pos.y;
		pi->pos.z=pc_currchar->pos.z;
		npcaction( pc_currchar, 0x15 );
		soundeffect2(pc_currchar, 0x0207);
	}
	else
	{
		staticeffect2(pi, (unsigned char)0x36, (unsigned char)0xB0, (unsigned char)0x10, (unsigned char)0x80, (unsigned char)0x00); 
		soundeffect3(pi, 0x0207);
	}

	len=pi->morex/250; //4 square max damage at 100 alchemy
	switch (pi->morez)
	{
	case 1:dmg=RandomNum( 5,10) ;break;
	case 2:dmg=RandomNum(10,20) ;break;
	case 3:dmg=RandomNum(20,40) ;break;
	default:
		clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, explodeitem()\n"); //Morrolan
		dmg=RandomNum(5,10);
	}

	if (dmg<5) dmg=RandomNum(5,10);	// 5 points minimum damage
	if (len<2) len=2;	// 2 square min damage range

	unsigned long loopexit=0;
	cRegion::RegionIterator4Chars ri(pi->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if (pc != NULL)
		{
			if (pc->isInvul() || pc->npcaitype==17)		// don't affect vendors
				continue;
			if(pc->isGM() || (pc->isPlayer() && !online(pc)))
				continue;
			dx=abs(pc->pos.x-pi->pos.x);
			dy=abs(pc->pos.y-pi->pos.y);
			dz=abs(pc->pos.z-pi->pos.z);
			if ((dx<=len)&&(dy<=len)&&(dz<=len))
			{
				pc->hp-=dmg+(2-min(dx,dy));
				updatestats(pc, 0);
				if (pc->hp<=0)
				{
					deathstuff(pc);
				}
				else
				{
					npcattacktarget(pc, pc_currchar);
					updatechar(pc);
				}
			}
		}
	}

	int chain=0;
	P_ITEM piMap;
	loopexit=0;

	int StartGrid=mapRegions->StartGrid(pi->pos.x, pi->pos.y);
	int increment=0;
	int checkgrid, a;
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				piMap = FindItemBySerial(vecEntries[k]);
				if (piMap != NULL)
				{
					if (piMap->id()==0x0F0D && piMap->type==19) // check for expl-potions
					{
						dx=abs(pi->pos.x - piMap->pos.x);
						dy=abs(pi->pos.y - piMap->pos.y);
						dz=abs(pi->pos.z - piMap->pos.z);

						if (dx<=2 && dy<=2 && dz<=2 && chain==0) // only trigger if in 2*2*2 cube
						{
							if (!(dx==0 && dy==0 && dz==0))
							{
								//chain=1; // maximum: one additional trigerred per check ..
								if (rand()%2==1) chain=1; // LB - more aggressive - :)
								tempeffect2(pc_currchar, piMap, 17, 0, 1, 0); // trigger ...
							}
						}
					}
				}
			}
		}
	}
	Items->DeleItem(pi);
}

void srequest(int s)
{
	if (buffer[s][5]==4) statwindow(s, FindCharBySerial(calcserial(buffer[s][6], buffer[s][7], buffer[s][8], buffer[s][9])));
	if (buffer[s][5]==5) skillwindow(s);
}

// Dupois - added doorsfx() to be used with dooruse()
// Added Oct 8, 1998
// Plays the proper door sfx for doors/gates/secretdoors
static void doorsfx(P_ITEM pi, int x, int y)
{
	const int OPENWOOD = 0x00EA;
	const int OPENGATE = 0x00EB;
	const int OPENSTEEL = 0x00EC;
	const int OPENSECRET = 0x00ED;
	const int CLOSEWOOD = 0x00F1;
	const int CLOSEGATE = 0x00F2;
	const int CLOSESTEEL = 0x00F3;
	const int CLOSESECRET = 0x00F4;

	if (y==0) // Request open door sfx
	{
		if (((x>=0x0695)&&(x<0x06C5))|| // Open wooden / ratan door
			((x>=0x06D5)&&(x<=0x06F4)))
			soundeffect3(pi,OPENWOOD);

		else if (((x>=0x0839)&&(x<=0x0848))|| // Open gate
			((x>=0x084C)&&(x<=0x085B))||
			((x>=0x0866)&&(x<=0x0875)))
			soundeffect3(pi,OPENGATE);

		else if (((x>=0x0675)&&(x<0x0695))|| // Open metal
			((x>=0x06C5)&&(x<0x06D5)))
			soundeffect3(pi,OPENSTEEL);

		else if ((x>=0x0314)&&(x<=0x0365)) // Open secret
			soundeffect3(pi,OPENSECRET);
	}
	else if (y==1) // Request close door sfx
	{
		if (((x>=0x0695)&&(x<0x06C5))|| // close wooden / ratan door
			((x>=0x06D5)&&(x<=0x06F4)))
			soundeffect3(pi,CLOSEWOOD);

		else if (((x>=0x0839)&&(x<=0x0848))|| // close gate
			((x>=0x084C)&&(x<=0x085B))||
			((x>=0x0866)&&(x<=0x0875)))
			soundeffect3(pi,CLOSEGATE);

		else if (((x>=0x0675)&&(x<0x0695))|| // close metal
			((x>=0x06C5)&&(x<0x06D5)))
			soundeffect3(pi,CLOSESTEEL);

		else if ((x>=0x0314)&&(x<=0x0365)) // close secret
			soundeffect3(pi,CLOSESECRET);
	}
} // doorsfx() END


void dooruse(UOXSOCKET s, P_ITEM pi)
{
	int i, db, x;//, z;
	char changed=0;
	if (pi == NULL) return;

	if ((iteminrange(s,pi,2)==0)&& s>-1) {sysmessage(s, "You cannot reach the handle from here");return;}
	for (i=0;i<DOORTYPES;i++)
	{
		db=doorbase[i];

		x=pi->id();
		if (x==(db+0))
		{
			pi->id2++;
			pi->pos.x--;
			pi->pos.y++;
			RefreshItem(pi);
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+1))
		{
			pi->id2--;
			pi->pos.x++;
			pi->pos.y--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+2))
		{
			pi->id2++;
			pi->pos.x++;
			pi->pos.y++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+3))
		{
			pi->id2--;
			pi->pos.x--;
			pi->pos.y--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+4))
		{
			pi->id2++;
			pi->pos.x--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+5))
		{
			pi->id2--;
			pi->pos.x++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+6))
		{
			pi->id2++;
			pi->pos.x++;
			pi->pos.y--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+7))
		{
			pi->id2--;
			pi->pos.x--;
			pi->pos.y++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+8))
		{
			pi->id2++;
			pi->pos.x++;
			pi->pos.y++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+9))
		{
			pi->id2--;
			pi->pos.x--;
			pi->pos.y--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+10))
		{
			pi->id2++;
			pi->pos.x++;
			pi->pos.y--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+11))
		{
			pi->id2--;
			pi->pos.x--;
			pi->pos.y++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		}
		else if (x==(db+12))
		{
			pi->id2++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+13))
		{
			pi->id2--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+14))
		{
			pi->id2++;
			pi->pos.y--;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 0);
			tempeffect2(0, pi, 13, 0, 0, 0);
		} else if (x==(db+15))
		{
			pi->id2--;
			pi->pos.y++;
			RefreshItem(pi);//AntiChrist
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		}
	}
	
	if (changed)
	{
		// house refreshment when a house owner or friend of a houe opens the house door
		int h=-1, hf=-1;
		float ds=0;
		P_CHAR pc_currchar = currchar[s];

		h=HouseManager->GetHouseNum(pc_currchar);
		if(h>=0)
		{
			hf=House[h]->FindFriend(pc_currchar);
			if(hf<0 && (House[h]->OwnerAccount!=pc_currchar->account))
				return;
			if (SrvParms->housedecay_secs!=0)
				 ds = static_cast<float>((House[h]->TimeUnused)*100) / (SrvParms->housedecay_secs);
			else ds=-1;	
			if (ds>=50) // sysmessage iff decay status >=50%
			{
				if (hf)
					sysmessage(s,"You refreshed your friend's house");
				else
					sysmessage(s,"You refreshed the house");
			}
			House[h]->TimeUnused=0;
			House[h]->LastUsed=getNormalizedTime();
		}
	}
	if (changed==0 && s>-1) sysmessage(s, "This doesnt seem to be a valid door type. Contact a GM.");
}

int validhair(int a, int b) // Is selected hair type valid
{
	
	if( a != 0x20 ) 
		return 0;
	switch( b )
	{
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
		return 1;
	default:
		return 0;
	}
	return 1;
}

int validbeard(int a, int b) // Is selected beard type valid
{
	if( a != 0x20 ) 
		return 0;
	switch( b )
	{
	case 0x3E:
	case 0x3F:
	case 0x40:
	case 0x41:
	case 0x4B:
	case 0x4C:
	case 0x4D:
		return 1;
	default:
		return 0;
	}
	return 1;
}

void charcreate( UOXSOCKET s ) // All the character creation stuff
{
	signed int ii;
	int totalstats,totalskills;
	P_CHAR pc = Npcs->MemCharFree();
	if ( pc == NULL )
		return;

	pc->Init();

	pc->name = (char*)buffer[s] + 10;
	pc->account = acctno[s];

	//	Code to support the new GuildType, and GuildTraitor members of the
	//	character strcture.
	pc->GuildType=-1;				//	Default to no guild
	pc->GuildTraitor=false;	//	Also defauot to non trator, or no guild
	pc->race=0; // Default human race
	if (buffer[s][0x46]!='\x00')
	{
		pc->setId(0x0191);
		pc->xid = 0x0191;
	}
	pc->skin = (buffer[s][0x50]|0x80 << 8) + buffer[s][0x51];
	if ( pc->skin < 0x83EA || pc->skin>0x8422 )
	{
		pc->skin = 0x83EA;
	}
	pc->xskin = pc->skin;
	pc->setPriv(defaultpriv1);
	pc->priv2=defaultpriv2;

	if (acctno[s]==0)
	{
		pc->setPriv(0xE7);
		pc->priv3[0]=0xFFFFFFFF; // lb, meta gm
		pc->priv3[1]=0xFFFFFFFF;
		pc->priv3[2]=0xFFFFFFFF;
		pc->priv3[3]=0xFFFFFFFF;
		pc->priv3[4]=0xFFFFFFFF;
		pc->priv3[5]=0xFFFFFFFF;
		pc->priv3[6]=0xFFFFFFFF;
		pc->menupriv=-1; // lb, menu priv
	}

	pc->MoveTo(str2num(start[buffer[s][0x5B]][2]), str2num(start[buffer[s][0x5B]][3]), str2num(start[buffer[s][0x5B]][4]));

	pc->dir=4;

	pc->hp=pc->st=buffer[s][0x47];
	if (pc->st>45) pc->st=45;		// fix for hack exploit
	if (pc->st<10) pc->st=10;
	totalstats=pc->st;

	short d=buffer[s][0x48];
	if (d>45) d=45;		// fix for hack exploit
	if (d<10) d=10;
	if (d+totalstats>65) d=65-totalstats;
	totalstats+=d;
	pc->setDex(d);
	pc->stm=d;

	pc->mn=pc->in=buffer[s][0x49];
	if (pc->in>45) pc->in=45;		// fix for hack exploit
	if (pc->in<10) pc->in=10;
	if (pc->in+totalstats>65) pc->in=65-totalstats;

	if (buffer[s][0x4b]>50) buffer[s][0x4b]=50; // fixes for hack exploit
	totalskills=buffer[s][0x4b];
	if (buffer[s][0x4d]>50) buffer[s][0x4d]=50;
	if (buffer[s][0x4d]+totalskills>100) buffer[s][0x4d]=100-totalskills;
	totalskills+=buffer[s][0x4d];
	if (buffer[s][0x4f]>50) buffer[s][0x4f]=50;
	if (buffer[s][0x4f]+totalskills>100) buffer[s][0x4f]=100-totalskills;

	for (ii=0;ii<TRUESKILLS;ii++)
	{
		pc->baseskill[ii]=0;
		if (ii==buffer[s][0x4a]) pc->baseskill[buffer[s][0x4a]]=buffer[s][0x4b]*10;
		if (ii==buffer[s][0x4c]) pc->baseskill[buffer[s][0x4c]]=buffer[s][0x4d]*10;
		if (ii==buffer[s][0x4e]) pc->baseskill[buffer[s][0x4e]]=buffer[s][0x4f]*10;
		Skills->updateSkillLevel(pc, ii);
	}

	if (validhair(buffer[s][0x52],buffer[s][0x53]))
	{
		const P_ITEM pi = Items->SpawnItem(s,pc,1, "#", 0, buffer[s][0x52], buffer[s][0x53], static_cast<unsigned short>(buffer[s][0x54]<<8)+buffer[s][0x55],0,0);
		if(pi == NULL) return;
		if ( pi->color < 0x044E || pi->color > 0x04AD )
		{
			pi->color = 0x044E;
		}
		pi->SetContSerial(pc->serial);
		pi->layer=0x0B;
	}

	if ( (validbeard(buffer[s][0x56],buffer[s][0x57])) && (pc->id2==0x90) )
	{
		const P_ITEM pi = Items->SpawnItem(s,pc,1, "#", 0, buffer[s][0x56], buffer[s][0x57], static_cast<unsigned short>(buffer[s][0x58]<<8) + buffer[s][0x59],0,0);
		if(pi == NULL) return;//AntiChrist to preview crashes
		if ( pi->color < 0x044E || pi->color > 0x04AD )
		{
			pi->color = 0x044E;
		}
		pi->SetContSerial(pc->serial);
		pi->layer=0x10;
	}

	{	// just to limit the scope of pi
	// - create the backpack
	P_ITEM pi = Items->SpawnItem(s,pc,1, "#", 0, 0x0E, 0x75, 0, 0,0);
	if (pi == NULL)
		return;
	pc->packitem = pi->serial;
	pi->SetContSerial(pc->serial);
	pi->layer=0x15;
	pi->type=1;
	pi->dye=1;
	}

	{	// limit the scope of pi
	const P_ITEM pi = Items->SpawnItem(s,pc,1,"#",0,0x09,0x15,0,0,0);
	if(pi == NULL) return;//AntiChrist to preview crashes

	switch (RandomNum(0, 1))
	{
	case 0:
		if ((pc->id() == 0x0190) && (pc->xid == 0x0190))
		{
			pi->setId(0x1539);
			pi->layer=0x04; // pant
		} else
		{
			pi->setId(0x1516);
			pi->layer=23; //skirt
		}
		break;
	case 1:
		if ((pc->id() == 0x0190) && (pc->xid == 0x0190))
		{
			pi->setId(0x152E);
			pi->layer=0x04;
		} else
		{
			pi->setId(0x1537);
			pi->layer=23;
		}
		break;
	}
	// pant/skirt color -> old client code, random color
	pi->color = static_cast<unsigned short>(buffer[s][102]<<8) + buffer[s][103];
	pi->SetContSerial(pc->serial);
	pi->type=0;
	pi->dye=1;
	pi->hp=10;
	pi->priv |= 0x02; // Mark as a newbie item
	}

	{	// limit the scope of pi
	const P_ITEM pi = Items->SpawnItem(s,pc,1,"#",0,0x09,0x15,0,0,0); // spawn pants
	if(pi == NULL) return;//AntiChrist to preview crashes
	if (!(rand()%2))
	{
		pi->setId(0x1EFD);
	}
	else
	{
		pi->setId(0x1517);
	}
	pi->color = static_cast<unsigned short>(buffer[s][100]<<8) + buffer[s][101];

	pi->SetContSerial(pc->serial);
	pi->layer=0x05;
	pi->dye=1;
	pi->hp=10;
	pi->def=1;
	pi->priv |= 0x02; // Mark as a newbie item
	}

	{	// limit the scope of pi
	const P_ITEM pi = Items->SpawnItem(s,pc,1,"#",0,0x17,0x0F,0x0287,0,0); // shoes
	if(pi == NULL) return;//AntiChrist to preview crashes
	pi->SetContSerial(pc->serial);
	pi->layer=0x03;
	pi->dye=1;
	pi->hp=10;
	pi->def=1;
	pi->priv |= 0x02; // Mark as a newbie item
	}

	{	// limit the scope of pi
	const P_ITEM pi = Items->SpawnItem(s,pc,1,"#",0,0x0F,0x51,0,0,0); // dagger
	if(pi == NULL) return;
	pi->SetContSerial(pc->serial);
	pi->layer=0x01;
	//pi->att=5;
	pi->hp=10;
	pi->spd=50;
	pi->lodamage=3;
	pi->hidamage=15;
	pi->itmhand=1;
	pi->priv |= 0x02; // Mark as a newbie item
	}

	currchar[s] = pc;
	newbieitems(s, pc);

	perm[s]=1;
	if(pc->st<10)
		pc->st=10;
	if(pc->in<10)
		pc->in=10;
	if(pc->effDex()<10)
		pc->setDex(10);
	Network->startchar(s);
}

int unmounthorse(UOXSOCKET s) // Get off a horse (Remove horse item and spawn new horse) 
{ 
	unsigned int ci = 0;
	int ch;
	P_ITEM pi; 
	const P_CHAR p_petowner = currchar[s];

	vector<SERIAL> vecContainer = contsp.getData(p_petowner->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer == 0x19 && !pi->free)
		{
			//////////////////////////////////////
			// Lets 'unstable' the mount.
			// Unmount the horse and take him out of
			// stable 0.
			
			bool found = false;
			int stablemaster_serial = p_petowner->serial;
			P_CHAR  p_pet = NULL;
			
			p_petowner->onhorse = false;
			
			vector<SERIAL> pets = stablesp.getData(stablemaster_serial);
			unsigned int i;
			for (i = 0; i < pets.size(); i++) 
			{ 
				p_pet = FindCharBySerial(pets[i]);
				if (p_pet != NULL) 
				{ 
					if ( p_petowner->Owns(p_pet) && p_pet->stablemaster_serial == stablemaster_serial) // already stabled and owned by claimer ? 
					{ 
						found = true; 
						break; 
					} 
				} 
			} 
			
			if (found) 
			{
				stablesp.remove(stablemaster_serial, p_pet->serial);
				
				p_pet->stablemaster_serial = INVALID_SERIAL; // actual unstabling 
				p_pet->timeused_last = getNormalizedTime(); 
				p_pet->time_unused = 0; 
				p_pet->pos.x = p_petowner->pos.x; 
				p_pet->pos.y = p_petowner->pos.y; 
				p_pet->pos.z = p_petowner->pos.z;
				p_pet->npcWander = 0;
				
				mapRegions->Remove(p_pet); 
				mapRegions->Add(p_pet); 
				
				for (ch = 0; ch < now; ch++) 
				{ 
					if (perm[ch])
						impowncreate(ch, p_pet, 0); 
				} 
			} 
			Items->DeleItem(pi); 
			return 0; 
		} 
	} 
	return -1; 
}

void endmessage(int x) // If shutdown is initialized
{
	x = 0;
	unsigned int igetclock = uiCurrentTime;
	if (endtime < igetclock)
		endtime = igetclock;
	
	if ((((endtime - igetclock)/MY_CLOCKS_PER_SEC)/60) < 1) 	
		sprintf((char*)temp, "Server going down in %i secs.", (endtime - igetclock)/MY_CLOCKS_PER_SEC); // blackwind %i Secs fix..
	else
		sprintf((char*)temp, "Server going down in %i minutes.",
		((endtime - igetclock)/MY_CLOCKS_PER_SEC)/60);
	sysbroadcast((char*)temp);
	clConsole.send(temp);
}

void illinst(int x=0) //Thunderstorm linux fix
{
	sysbroadcast("Fatal Server Error! Bailing out - Have a nice day!");
	clConsole.send("Illegal Instruction Signal caught - attempting shutdown");
	endmessage(x);
}

void scriptcommand (int s, char *script1, char *script2) // Execute command from script
{
	char tstring[1024];
	int total, ho, mi, se;
	int tmp ;
	int c;
	P_CHAR pc_currchar = currchar[s];
	strupr((char*)script1);
	strupr((char*)script2);

	if (!(strcmp("GMMENU", (char*) script1)))
	{
		gmmenu(s, str2num(script2));
		return;
	}
	if (!(strcmp("ITEMMENU", (char*)script1)))
	{
		itemmenu(s, str2num(script2));
		return;
	}
	if (!(strcmp("WEBLINK", (char*)script1)))
	{
		weblaunch(s, (char*)script2);
		return;
	}
	if (!(strcmp("SYSMESSAGE", (char*)script1)))
	{
		sysmessage(s, (char*)script2);
		return;
	}
	if (!(strcmp("GMPAGE", (char*)script1)))
	{
		Commands->GMPage(s, (char*)script2);
		return;
	}
	if (!(strcmp("CPAGE", (char*)script1)))
	{
		Commands->CPage(s, (char*)script2);
		return;
	}
	if (!(strcmp("VERSION", (char*)script1)))
	{
		sysmessage(s, idname);
		return;
	}
	//AntiChrist - no need of skill type check
	if (!(strcmp("ADDITEM", (char*)script1)))
	{
		addmitem[s]=str2num(script2);
		Skills->MakeMenuTarget(s,addmitem[s],pc_currchar->making);
		pc_currchar->making=0;
		return;
	}
	if (!(strcmp("BATCH", (char*)script1)))
	{
		executebatch=str2num(script2);
		return;
	}
	if (!(strcmp("INFORMATION", (char*)script1)))
	{
		total=(uiCurrentTime-starttime)/MY_CLOCKS_PER_SEC;
		ho=total/3600;
		total-=ho*3600;
		mi=total/60;
		total-=mi*60;
		se=total;
		total=0;
		c=0;
		int totaltotal=0;
		int cc=0;
		AllItemsIterator iterItems;
		for (iterItems.Begin(); !iterItems.atEnd();iterItems++)
		{ 
			P_ITEM pi = iterItems.GetData();
			if (!pi->free) 
				total++; 
			totaltotal++; 
		}
		AllCharsIterator iter_char;
		for (iter_char.Begin(); !iter_char.atEnd(); iter_char++) 
		{ 
			P_CHAR pc = iter_char.GetData();
			if (!pc->free) 
				c++; 
			cc++; 
		}
		sprintf(tstring, "Time up [%i:%i:%i] Connected players [%i out of %i accounts] Items [%i] Characters [%i]",
			ho,mi,se,now,Accounts->Count(),total,c);

		sysmessage(s, tstring);
		sprintf(tstring, "Runtime lag factors: Corpses not freed from memory [%i] Decayed items still in memory [%i]",
			cc-c,totaltotal-total);
		sysmessage(s,tstring);
		return;
	}
	if (!(strcmp("NPC", (char*)script1)))
	{
		addmitem[s]=str2num(script2);
		sprintf(tstring, "Select location for NPC. [Number: %i]", addmitem[s]);
		target(s, 0, 1, 0, 27, tstring);
		return;
	}
	if (!(strcmp("POLY", (char*)script1)))
	{
		tmp = hex2num(script2);
		pc_currchar->setId(tmp);
		pc_currchar->xid = tmp;
		teleport(pc_currchar);
		return;
	}
	if (!(strcmp("SKIN", (char*)script1)))
	{
		pc_currchar->skin = pc_currchar->xskin = static_cast<UI16>(hex2num(script2));
		return;
	}
	if (!(strcmp("LIGHT", (char*)script1)))
	{
		worldfixedlevel=hex2num(script2);
		if (worldfixedlevel!=255) setabovelight(worldfixedlevel);
		else setabovelight(worldcurlevel);
		return;
	}

	if (!(strcmp("DRY", (char*)script1)))
	{
		return;
	}

	if (!(strcmp("RAIN", (char*)script1)))
	{
		return;
	}
	if (!(strcmp("SNOW", (char*)script1)))
	{
		return;
	}

	if (!(strcmp("GCOLLECT", (char*)script1)))
	{
		gcollect();
		return;
	}
	if (!(strcmp("GOPLACE", (char*)script1)))
	{
		tmp=str2num(script2);
		Commands->MakePlace (s, tmp);
		if (addx[s]!=0)
		{
			pc_currchar->MoveTo(addx[s],addy[s],addz[s]);
			teleport(pc_currchar);
		}
		return;
	}
	if (!(strcmp("MAKEMENU", (char*)script1)))
	{
		Skills->MakeMenu(s, str2num(script2), pc_currchar->making);
		return;
	}
	if (!(strcmp("CREATETRACKINGMENU", (char*)script1)))
	{
		Skills->CreateTrackingMenu(s, str2num(script2));
		return;
	}
	if(!(strcmp("TRACKINGMENU", (char*)script1)))
	{
		Skills->TrackingMenu(s, str2num(script2));
		return;
	}
}

void batchcheck(int s) // Do we have to run a batch file
{
	sprintf((char*)temp, "BATCH %i", executebatch);
	openscript("menus.scp");
	if (!i_scripts[menus_script]->find((char*)temp))
	{
		closescript();
		return;
	}

	unsigned long loopexit=0;
	do
	{
		read2();
		if (script1[0]!='}') scriptcommand(s, script1, script2);
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	executebatch=0;
}

void npctalkall_runic(P_CHAR npc, char *txt,char antispam)
{
	if (npc == NULL) return;

	int i;

	for (i=0;i<now;i++)
		if (inrange1p(npc, currchar[i])&&perm[i])
			npctalk_runic(i, npc, txt,antispam);
}

void npcemoteall(P_CHAR npc, char *txt,unsigned char antispam) // NPC speech to all in range.
{
	int i;

	if (npc==NULL) return;

	for (i=0;i<now;i++)
		if (inrange1p(npc, currchar[i])&&perm[i])
			npcemote(i, npc, txt,antispam);
}

//taken from 6904t2(5/10/99) - AntiChrist
void callguards( P_CHAR pc_player )
{
	if( pc_player == NULL ) return;

	//AntiChrist - anti "GUARDS" spawn timer
	if(pc_player->antiguardstimer<uiCurrentTime)
	{
		pc_player->antiguardstimer=uiCurrentTime+(MY_CLOCKS_PER_SEC*10);
	} else return;

	if (!pc_player->inGuardedArea() || !SrvParms->guardsactive )
		return;

	cRegion::RegionIterator4Chars ri(pc_player->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if (pc != NULL)
		{
			if( chardist( pc_player, pc ) < 15 )
			{
				if( !pc->dead && !pc->isInnocent() )
				{
					Combat->SpawnGuard( pc, pc, pc->pos.x, pc->pos.y, pc->pos.z );
				}
			}
		}
	}
}

void mounthorse(UOXSOCKET s, P_CHAR pc_mount) // Remove horse char and give player a horse item 
{ 
	int j; 
	if ( pc_mount == NULL ) return;
	P_CHAR pc_currchar = currchar[s];
	
	if (npcinrange(s, pc_mount, 2) == 0 && !pc_currchar->isGM())
		return; 
	if (pc_currchar->Owns(pc_mount) || pc_currchar->isGM()) 
	{ 
		if (pc_currchar->onhorse) 
		{ 
			sysmessage(s, "You are already on a mount."); 
			return; 
		}
		strcpy((char*)temp, pc_mount->name.c_str()); 
		pc_currchar->onhorse = true; 
		const P_ITEM pi = Items->SpawnItem(pc_currchar, 1, (char*)temp, 0, 0x0915, pc_mount->skin, 0); 
		if(!pi) return;
		
		switch (pc_mount->id2) 
		{
			case 0xC8: pi->setId(0x3E9F); break; // Horse
			case 0xE2: pi->setId(0x3EA0); break; // Horse 
			case 0xE4: pi->setId(0x3EA1); break; // Horse 
			case 0xCC: pi->setId(0x3EA2); break; // Horse
			case 0xD2: pi->setId(0x3EA3); break; // Desert Ostard
			case 0xDA: pi->setId(0x3EA4); break; // Frenzied Ostard 
			case 0xDB: pi->setId(0x3EA5); break; // Forest Ostard
			case 0xDC: pi->setId(0x3EA6); break; // LLama
			case 0x34: pi->setId(0x3E9F); break; // Brown Horse
			case 0x4E: pi->setId(0x3EA0); break; // Grey Horse
			case 0x38: pi->setId(0x3EA2); break; // Dark Brown Horse
			case 0x50: pi->setId(0x3EA1); break; // Tan Horse
			case 0x74: pi->setId(0x3EB5); break; // Nightmare
			case 0x75: pi->setId(0x3EA8); break; // Silver Steed
			case 0x72: pi->setId(0x3EA9); break; // Dark Steed
			case 0x7A: pi->setId(0x3EB4); break; // Unicorn
			case 0x84: pi->setId(0x3EAD); break; // Kirin
			case 0x73: pi->setId(0x3EAA); break; // Etheral
			case 0x76: pi->setId(0x3EB2); break; // War Horse-Brit
			case 0x77: pi->setId(0x3EB1); break; // War Horse-Mage Council
			case 0x78: pi->setId(0x3EAF); break; // War Horse-Minax
			case 0x79: pi->setId(0x3EB0); break; // War Horse-Shadowlord
			case 0xAA: pi->setId(0x3EAB); break; // Etheral LLama
			case 0x3A: pi->setId(0x3EA4); break; // Forest Ostard
			case 0x39: pi->setId(0x3EA3); break; // Desert Ostard
			case 0x3B: pi->setId(0x3EA5); break; // Frenzied Ostard
			case 0x90: pi->setId(0x3EB3); break; // Seahorse
			case 0xAB: pi->setId(0x3EAC); break; // Etheral Ostard
			case 0xBB: pi->setId(0x3EB8); break; // Ridgeback
		}
		
		pi->SetContSerial(pc_currchar->serial); 
		pi->layer = 0x19; 
		pi->MoveTo(pc_mount->fx1, pc_mount->fy1, pc_mount->fz1); 
		
		pi->moreb1 = pc_mount->npcWander; 
		pi->att = pc_mount->fx2; 
		pi->def = pc_mount->fy2; 
		
		// AntiChrist bugfixes - 11/10/99 
		pi->moreb2 = pc_mount->st; 
		pi->moreb3 = pc_mount->realDex(); 
		pi->moreb4 = pc_mount->in; 
		pi->hp = pc_mount->hp; 
		pi->lodamage = pc_mount->fame; 
		pi->hidamage = pc_mount->karma; 
		pi->poisoned = pc_mount->poisoned;
		if (pc_mount->summontimer != 0) 
			pi->decaytime = pc_mount->summontimer; 
		
		wornitems(s, pc_currchar);// send update to current socket 
		
		for (j = 0; j < now; j++)// and to all inrange sockets (without re-sending to current socket) 
		{ 
			if (inrange1(s, j) && perm[j] &&(s != j))
				wornitems(j, pc_currchar); 
		} 
		////////////////////////////////// 
		// Gonna stable instead of delete a mount. 
		// This will keep their original and earned 
		// stats and will allow more roboust 
		// mount code. 
		// 
		// 
		// 
		// Gonna stable instead of delete so 
		// comment this out 
		// Npcs->DeleteChar(x); 
		int stablemaster_serial = pc_currchar->serial; 
		
		// if this is a gm lets tame the animal in the process 
		if (pc_currchar->isGM())
		{ 
			pc_mount->SetOwnSerial( pc_currchar->serial ); 
			pc_mount->npcaitype = 0; 
		} 
		
		// set stablesp && pets stablemaster serial 
		// remove it from screen! 
		int xx = pc_mount->pos.x; 
		int yy = pc_mount->pos.y; 
		signed char zz = pc_mount->pos.z; 
		int id1 = pc_mount->id1; 
		int id2 = pc_mount->id2; 
		pc_mount->id1 = 0; 
		pc_mount->id2 = 0; 
		pc_mount->pos = Coord_cl(0, 0, 0);
		
		for (int ch = 0; ch < now; ch++) 
		{ 
			if (perm[ch])
				impowncreate(ch, pc_mount, 0); 
		} 
		
		pc_mount->id1 = id1; 
		pc_mount->id2 = id2; 
		pc_mount->war = false; 
		pc_mount->attacker = INVALID_SERIAL; 
		pc_mount->pos = Coord_cl(xx, yy, zz);
		
		mapRegions->Remove(pc_mount); 
		
		pc_mount->stablemaster_serial = stablemaster_serial; // set stablemaster serial 
		
		// set timer 
		pc_mount->time_unused = 0; 
		pc_mount->timeused_last = getNormalizedTime(); 
		
		stablesp.insert(stablemaster_serial, pc_mount->serial);
		// 
		// 
		// Aldur 
		////////////////////////////////// 
	} 
	else 
	 sysmessage(s, "You dont own that creature."); 
}

void endScrn()
{
#ifdef __unix__

	int status ;
	//status = fcntl(STDIN_FILENO,F_SETFL,O_SYNC) ;
	
	if (tcsetattr(STDOUT_FILENO,TCSANOW,&termstate)!=0)
	{
		cout << "error reseting terminal console" << endl;
		cout << strerror(errno) << endl;
	}

#endif
}
#if !defined(__unix__)
//////////////////
// Name:	checkkey()
// Purpose:	Facilitate console control. SysOp keys, and localhost controls.
//
void checkkey ()
{
	int i,j=0;
	char c=0;

	if (kbhit())
		c=getch();

	if (c != 0)
	{
		c = toupper(c);
		if (c == 'S')
		{
			if (secure)
			{
				clConsole.send("WOLFPACK: Secure mode disabled. Press ? for a commands list.\n");
				secure=0;
				return;
			}
			else
			{
				clConsole.send("WOLFPACK: Secure mode re-enabled.\n");
				secure=1;
				return;
			}
		} else {
			if (secure && c != '?')  //Allows help in secure mode.
			{
				clConsole.send("WOLFPACK: Secure mode prevents keyboard commands! Press 'S' to disable.\n");
				return;
			}

			switch(c)
			{
			case '\x1B':
				keeprun=0;
				break;
			case 'Q':
			case 'q':
				clConsole.send("WOLFPACK: Immediate Shutdown initialized!\n");
				keeprun=0;
				break;
			case 'T':
			case 't':
				endtime=uiCurrentTime+(MY_CLOCKS_PER_SEC*600);
				endmessage(0);
				break;
			case '#':
				if ( !cwmWorldState->Saving() )
				{
					clConsole.send( "Saving worldfile...");
					cwmWorldState->savenewworld(1);
					//saveserverscript(1);
					saveserverscript();
					clConsole.send( "Done!\n");
				}
				break;
			case 'L':
			case 'l':
				if (showlayer)
				{
					clConsole.send("WOLFPACK: Layer display disabled.\n");
					showlayer=0;
				}
				else
				{
					clConsole.send("WOLFPACK: Layer display enabled.\n");
					showlayer=1;
				}
				break;
			case 'I':
			case 'i':
				Admin->ReadIni();
				clConsole.send("WOLFPACK: INI file reloaded.\n");
				break;
			case 'D':	// Disconnect account 0 (useful when client crashes)
			case 'd':	
				for (i=0;i<now;i++)
					if (acctno[i]==0 && perm[i]) 
					{
						Network->Disconnect(i);
						clConsole.send( "Account 0 disconnected\n");
					}
					break;
			case 'H':
			case 'h':				// Enable/Disable heartbeat
				if (heartbeat) 
					clConsole.send("WOLFPACK: Heartbeat Disabled\n");
				else 
					clConsole.send("WOLFPACK: Heartbeat Enabled\n");
				heartbeat = !heartbeat;
				break;
			case 'P':
			case 'p':				// Display profiling information
				clConsole.send("Performace Dump:\n");
				clConsole.send("Network code: %fmsec [%i samples]\n" _ (float)((float)networkTime/(float)networkTimeCount) _ networkTimeCount);
				clConsole.send("Timer code: %fmsec [%i samples]\n" _ (float)((float)timerTime/(float)timerTimeCount) _ timerTimeCount);
				clConsole.send("Auto code: %fmsec [%i samples]\n" _ (float)((float)autoTime/(float)autoTimeCount) _ autoTimeCount);
				clConsole.send("Loop Time: %fmsec [%i samples]\n" _ (float)((float)loopTime/(float)loopTimeCount) _ loopTimeCount);
//				clConsole.send("Characters: %i/%i (Dynamic)		Items: %i/%i (Dynamic)\n" _ charcount _ cmem _ itemcount _ imem);
				clConsole.send("Simulation Cycles: %f per sec\n" _ (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
				break;
			case 'W':
			case 'w':				// Display logged in chars
				clConsole.send("Current Users in the World:\n");
				j = 0;  //Fix bug counting ppl online.
				for (i=0;i<now;i++)
				{
					if(perm[i]) //Keeps NPC's from appearing on the list
					{
						clConsole.send("%i) %s [%x]\n", j, currchar[i]->name.c_str(), currchar[i]->serial);
						j++;
					}
				}
				clConsole.send("Total Users Online: %d\n", j);
				break;
			case 'A': //reload the accounts file
			case 'a':
				clConsole.send("WOLFPACK: Reloading accounts file...");
				Accounts->LoadAccounts();
				clConsole.send("Done!\n");
				break;
			case 'r':
			case 'R':
				clConsole.send("WOLFPACK: Reloading Server.scp, Spawn.scp, and Regions.scp....");
				loadspawnregions();
				loadregions();
				loadmetagm();
				loadmenuprivs();
				loadserverscript();
				clConsole.send(" Done!\n");
				clConsole.send("WOLFPACK: Reloading IP Blocking rules...");
				Network->LoadHosts_deny();
				clConsole.send("Done\n");
				break;
			case '?':
				clConsole.send("Console commands:\n");
				clConsole.send("	<Esc> or Q: Shutdown the server.\n");
				clConsole.send("	T - System Message: The server is shutting down in 10 minutes.\n");
				clConsole.send("	# - Save world\n");
				clConsole.send("	L - Toggle layer Display");
				if (showlayer)
					clConsole.send("[enabled]\n");
				else 
					clConsole.send("[disabled]\n");
				clConsole.send("	I - Reload INI file.\n");
				clConsole.send("	D - Disconnect Account 0\n");
				clConsole.send("	1 - Sysmessage: Attention Players Server being brought down!\n");
				clConsole.send("	2 - Broadcast Message 2\n");
				clConsole.send("	H - Toggle hearbeat ");
				if (heartbeat)
					clConsole.send( "[enabled]\n");
				else
					clConsole.send( "[disabled]\n");
				clConsole.send("	P - Preformance Dump\n");
				clConsole.send("	W - Display logged in characters\n");
				clConsole.send("	A - Reload accounts file\n");
				clConsole.send("	R - Reload server, spawn, regions, and meta gm scripts.\n");
				clConsole.send("	S - Toggle Secure mode ");
				if (secure)
					clConsole.send("[enabled]\n");
				else
					clConsole.send("[disabled]\n");
				clConsole.send("	? - Commands list (this)\n");
				clConsole.send("End of commands list.\n");
				break;
			default:
				clConsole.send("WOLFPACK: Key %c [%x] does not preform a fucntion.\n",c,c);
				break;
			}
		}
	}
}
#endif
void start_glow(void)	// better to make an extra function cauze in loaditem it could be the case that the
{						// glower is loaded before the pack
	AllItemsIterator it;
	for (it.Begin(); !it.atEnd();it++)
	{
		const P_ITEM pi = it.GetData();	// on error return
		if (pi->glow>0 && !pi->free)
		{
			if (!pi->isInWorld())
			{
				P_ITEM pj = FindItemBySerial(pi->contserial); // find glowing item in backpack
				P_CHAR pc_l = FindCharBySerial(pi->contserial); // find equipped glowing items
				P_CHAR pc;
				if (pc_l == NULL) pc = GetPackOwner(pj); else pc = pc_l;
				if (pc != NULL)
				{
					pc->addHalo(pi);
					pc->glowHalo(pi);
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{

	keeprun = 1; // First of all, we want to run :)
	#if defined(__unix__)
	// Under unix we go to deamon mode
	cout << "Going into deamon mode, returning local control to terminal" <<endl;
	init_deamon() ;
	// set up our console redirection
	fstream fconsole;
	fconsole.open("console.txt", ios::out);
	clConsole.setStreams(NULL, (dynamic_cast<ostream*>(&fconsole)), NULL, NULL);
	
	#endif
	#define CIAO_IF_ERROR if (error==1) { Network->SockClose(); im_clearmenus(); DeleteClasses(); exit(-1); }

	int i;
	unsigned long tempSecs;

	unsigned long loopSecs;

	unsigned long tempTime;
	unsigned int uiNextCheckConn=0;//CheckConnection rate - AntiChrist ( thnx to LB )
	unsigned long CheckClientIdle=0;
	int r;
	uiCurrentTime = serverstarttime = getNormalizedTime();

	#ifndef __unix__ // If X-Wolf mandatory flag is set don't start if wolfpack hasn't been started by X-Wolf
#if 0
//	if (0) 

	  int win = GetWindowsVersion();
	  switch (win)
	  {
	      case W95:
		  clConsole.send("Detected Windows Version: Windows 95\n");
		  break;

		  case W98:
		  clConsole.send("Detected Windows Version: Windows 98\n");
		  break;

		  case WME:
		  clConsole.send("Detected Windows Version: Windows ME\n");
		  break;

		  case WNT4:
		  clConsole.send("Detected Windows Version: Windows NT 4.0\n");
		  break;

		  case W2K:
		  clConsole.send("Detected Windows Version: Windows 2000\n");
		  break;

		  case XP:
		  clConsole.send("Detected Windows Version: Windows XP\n");
		  break;

	  }

	  if (wp_version.verstruct.flags & WPV_REQXWOLF)
	  {		  
		    // link dynamicaly so that it starts on systems that don't have the isdebuggerpresent() system call (w95)
		    bool detectable;
		  	BOOL  (WINAPI *lpfIsDebuggerPresent)   (void  ) = NULL;
	        HINSTANCE hInstLib = LoadLibraryA ("Kernel32.DLL" );
            if (hInstLib==NULL) FreeLibrary(hInstLib); else lpfIsDebuggerPresent = ( BOOL ( WINAPI* ) ( void ) )  GetProcAddress ( hInstLib, "IsDebuggerPresent" );
		    BOOL debuggerpresent=false;
			if (lpfIsDebuggerPresent==NULL) { detectable=false; } else { detectable=true; debuggerpresent = lpfIsDebuggerPresent(); }

			if (!debuggerpresent && detectable)
			{		     		   			
               clConsole.send("This is a Wolfpack version that needs to be started by X-Wolf\n");
			   clConsole.send("Wolfpack server detected that is hasn't been started by X-Wolf, quitting\n");
               FreeLibrary(hInstLib);
			   exit(666);
			} else if (!detectable)
			{
			   clConsole.send("This is a Wolfpack version that needs to be started by X-Wolf\n");
			   clConsole.send("But Wolfpack can't detect if it is started by X-Wolf or not\n");
			  
			}

			if (hInstLib!=NULL) FreeLibrary(hInstLib);		  
	  }
	  
#endif
    #endif

	//constart();
 
	clConsole.send("Starting WOLFPACK...\n");
	openings = 0;
	scpfilename[0] = 0;

	sprintf((char*)temp, "%s %s %s", wp_version.productstring.c_str() , wp_version.betareleasestring.c_str() , wp_version.verstring.c_str() );
	#ifndef __unix__
	SetConsoleTitle((char*)temp);
	#endif
#if 1
	#ifndef __unix__ // wip stuff is currently only for windows stuff
	  BOOL w = WIP_Init(); 
	  // that's all :) no wip stuff in wp-core
	  if (w==FALSE)
	  {
		  clConsole.send("Error starting WIP, WIP clients won't work");
	  }
    #endif
#endif

	StartClasses();

	CIAO_IF_ERROR;

	Map->Cache = 0;
	Admin->ReadIni();

	clConsole.send("Initializing script pointers...\n");
	for(i=0;i<NUM_SCRIPTS;i++)
	{
		if (i==custom_npc_script || i==custom_item_script)//Don't initialize these till we know the file names ;-)
			continue;
		if (i==npc_script /*||i==create_script || i==regions_script*/)
			i_scripts[i] = new Script(n_scripts[i], SCP_PRELOADABLE);
		else
			i_scripts[i] = new Script(n_scripts[i]);
	}
	clConsole.send("Done.\n");



	//Now lets load the custom scripts, if they have them defined...
	i=0;
	LoadCustomScripts();

	if (argc>1)
	{
		if (!(strcmp(argv[1],"#")))
		{
			return MenuListGenerator();
		}
	}


	CIAO_IF_ERROR; // no wolfpack.ini crashes further startup process. so stop insted of crash

	SetGlobalVars();

	SkillVars();	// Set Creator Variables

	clConsole.send("Loading skills...");
	loadskills();
	clConsole.send(" Done\n");

	clConsole.send("Building pointer arrays...");
	BuildPointerArray();
	clConsole.send(" Done. \n");

	Accounts->LoadAccounts();

	keeprun=Network->kr; //LB. for some technical reasons global varaibles CANT be changed in constructors in c++.
	error=Network->faul; // i hope i can find a cleaner solution for that, but this works !!!
	// has to here and not at the cal cause it would get overriten later

	CIAO_IF_ERROR;

	loadserverdefaults();
	loadserverscript();
	loadspawnregions();
	loadregions();

	CIAO_IF_ERROR;

	Map->Load();

	if (keeprun==0) { Network->SockClose(); im_clearmenus(); DeleteClasses(); exit(-1); }

	clConsole.send("Loading Teleport...");
	read_in_teleport();
	clConsole.send(" Done.\n");

	//Map->TileCache(); // has to be exactly here, or loadnewlorld cant access correct tiles ... LB
	// (it does access them ..)

	srand(uiCurrentTime); // initial randomization call
	clConsole.send("Loading vital scripts...\n");
	loadmetagm();
	loadmenuprivs();

	serverstarttime=getNormalizedTime();

	CIAO_IF_ERROR;

	cwmWorldState->loadnewworld();

	CIAO_IF_ERROR; // LB prevents file corruption

	// this loop is for things that have to be done after *all* items and chars have been loaded (Duke)
	P_ITEM pi;
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
	{
		pi = iterItems.GetData();
		// 1. this relies on the container the item is in
		StoreItemRandomValue(pi, -1);

		// 2. needs the char to be loaded
		if (pi->dx2 && isCharSerial(pi->contserial))	// effect on dex ? like plate eg.
		{
			P_CHAR pc = FindCharBySerial(pi->contserial);
			if (pc)
				pc->chgDex(pi->dx2);
		}
	}

	clConsole.send("Clearing all trades...");
	Trade->clearalltrades();
	clConsole.send(" Done.\n");

	//Boats --Check the multi status of every item character at start up to get them set!
	clConsole.send("Initializing multis...");
	InitMultis();
	clConsole.send(" Done.\n");
	//End Boats --^

	clConsole.send("Loading IM Menus...");
	im_loadmenus( "inscribe.gmp", TellScroll ); //loading gump for inscribe()
	clConsole.send(" Done.\n");

	gcollect();

	clConsole.send("Initializing glowing-items...");
	start_glow();
	clConsole.send(" Done!\n"); // Magius(CHE) (1)
	FD_ZERO(&conn);
	starttime=uiCurrentTime;
	endtime=0;
	lclock=0;
	clConsole.send(".");LogMessage("Initializing Queue System...");
	initque(); // Initialize gmpages[] array

	clConsole.send(".");LogMessage("Done. Loading custom titles...");
	loadcustomtitle();
	clConsole.send(" Done.\n");

	clConsole.send("Initializing Network...");
	Network->Initialize();
	clConsole.send(" Done.\n");

	cwmWorldState->announce(SrvParms->announceworldsaves);

	clConsole.send("Initializing sounds... ");
	init_creatures(); //lb, initilises the creatures array (with soudfiles and other creatures infos)
	clConsole.send("Done.\n");

	clConsole.send("Initializing Magery System... ");
	Magic->InitSpells();
	clConsole.send("Done.\n");

	clConsole.send("Loading IP Blocking rules...");
	Network->LoadHosts_deny();
	clConsole.send("Done\n");
    if (SrvParms->EnableRA)
         racInit();

#ifndef __unix__
	clearscreen(); // Moved by Magiu s(CHE (1)
	sprintf(idname, "%s %s %s [WIN32] compiled by %s\nProgrammed by: %s", wp_version.productstring.c_str() ,  wp_version.betareleasestring.c_str(), wp_version.verstring.c_str() , wp_version.compiledbystring.c_str() , wp_version.codersstring.c_str() );
#else
	sprintf(idname, "%s %s %s [LINUX] compiled by %s\nProgrammed by: %s", wp_version.productstring.c_str() ,  wp_version.betareleasestring.c_str(), wp_version.verstring.c_str() , wp_version.compiledbystring.c_str(), wp_version.codersstring.c_str() );
#endif
	clConsole.send(idname);
	clConsole.send("\n");
	clConsole.send("(Configured for connections by UO Clients supported by Ignition)\n\n");
	clConsole.send("Copyright (C) 1997, 98 Marcus Rating (Cironian)\n\n");
	clConsole.send("This program is free software; you can redistribute it and/or modify\n");
	clConsole.send("it under the terms of the GNU General Public License as published by\n");
	clConsole.send("the Free Software Foundation; either version 2 of the License, or\n");
	clConsole.send("(at your option) any later version.\n\n");
	clConsole.send("%s %s %s\n",wp_version.productstring.c_str() , wp_version.betareleasestring.c_str() , wp_version.verstring.c_str() );
	clConsole.send("Compiled on %s (%s %s)\n",__DATE__,__TIME__, wp_version.timezonestring.c_str() );
	clConsole.send("Compiled by %s at %s\n", wp_version.compiledbystring.c_str() , wp_version.emailstring.c_str() );
	clConsole.send("Programmed by: %s\n", wp_version.codersstring.c_str() );
//	if (sizeof(tile_st)!=37)
//		clConsole.send("This version of WOLFPACK was compiled incorrectly. sizeof(tile_st) = %d \n", sizeof(tile_st));
	clConsole.send("\n");
	// Server.scp status --- By Magius(CHE)
	clConsole.send( "Server Settings:\n");
	InitServerSettings();

	item_char_test(); //LB
	Guilds->CheckConsistancy(); // LB
	clConsole.send("Loading Races!\n");
	Races.LoadRaceFile();
	clConsole.send("Races Loaded!\n");
	//Weather->run() ;
	//Network->InitConnThread();

////////////////////////////////////////////////////////////////////////////////////////////////////
// Removed by Dupois July 18, 2000!
// - Crashes server during if '#' - Save World option is used and corrupts the world file
//	quite nicely I might add. safer to just use the old checkkey() call in the main's while loop.
//InitKbThread();
////////////////////////////////////////////////////////////////////////////////////////////////////
#if 1 // Duke, experimental stuff
void qsfLoad(char *fn, short depth); // Load a quest script file
	if (argc > 1)
		if (!strcmp(argv[1],"import") && argc>2)
			qsfLoad(argv[2],0);
#endif
//#if 1 // Duke, experimental stuff
//	clConsole.send("sizeof cItem: %i\n", sizeof(cItem) );
//#endif

	clConsole.send("WOLFPACK: Startup Complete.\n\n");


	if (SrvParms->server_log) savelog("-=Server Startup=-\n=======================================================================\n","server.log");

	uiCurrentTime=getNormalizedTime();
	serverstarttime=getNormalizedTime(); // dont remove, its absolutly necassairy that its 3 times in the startup sequence for several timing reasons.

	CIAO_IF_ERROR;

    if (MapTileHeight>300) 
		clConsole.send("BRITANNIA MAP SERVER\n"); 
	else 
		clConsole.send("ILSHENAR MAP SERVER \n");

    // print allowed clients
    const char * t;   
    std::vector<std::string>::const_iterator vis; 

    clConsole.send("\nAllowed clients\n");
    for (vis=clientsAllowed.begin(); vis != clientsAllowed.end();  ++vis) 
    {
      t = (*vis).c_str();  // a bit pervert to store c++ strings and operate with c strings, admitably
	                       
	  strcpy(temp2,t);	
	  strcpy(temp,t);strcat(temp,"\n");

	  if (!strcmp(temp2,"SERVER_DEFAULT") )
	  {
		  sprintf(temp3,"%s : %s\n",temp2, wp_version.clientsupportedstring.c_str() );
		  clConsole.send(temp3);
		  break;		  
	  }	
	  else if (!strcmp(temp2,"ALL") )
	  {
		  clConsole.send("ALL\n");
		  break;
	  }

	  clConsole.send(temp);	 
    }

 

    clConsole.send("\n");
   
	while (keeprun)
	{

	// Uncomment by Dupois July 18, 2000! see note above about InitKbThread()
		#if !defined(__unix__)
		checkkey();
		#endif

		switch(speed.nice)
		{
			case 0: break;	// very unnice - hog all cpu time
			case 1: if (now!=0) Sleep(10); else Sleep(100); break;
			case 2: Sleep(10); break;
			case 3: Sleep(40); break;// very nice
			case 4: if (now!=0) Sleep(10); else Sleep(4000); break; // anti busy waiting
			case 5: if (now!=0) Sleep(40); else Sleep(5000); break;

			default: Sleep(10); break;
		}
		if(loopTimeCount >= 1000)
		{
			loopTimeCount = 0;
			loopTime = 0;
		}
		loopTimeCount++;

		loopSecs = getNormalizedTime() ;  // Starting time 

		if(networkTimeCount >= 1000)
		{
			networkTimeCount = 0;
			networkTime = 0;
		}

		tempSecs = getNormalizedTime() ;

		if(Respawn->AreWeRespawning())	// pseudo-respawn-thread (Duke)
			Respawn->Continue();

		if (CheckClientIdle<=uiCurrentTime)
		{
			CheckClientIdle=((SrvParms->inactivitytimeout/2)*MY_CLOCKS_PER_SEC)+uiCurrentTime;

			for (r=0;r<now;r++)
			{
				if (!currchar[r]->free
					&& !currchar[r]->isGM()
					&& currchar[r]->clientidletime<uiCurrentTime
					&& perm[r]
					)
				{
					clConsole.send("Player %s disconnected due to inactivity !\n", currchar[r]->name.c_str());
					//sysmessage(r,"you have been idle for too long and have been disconnected!");
					char msg[3];
					msg[0]=0x53;
					msg[1]=0x07;
					Xsend(r, msg, 2);
					Network->Disconnect(r);
				}

			}
		}
		if( uiNextCheckConn<=uiCurrentTime || overflow) // Cut lag on CheckConn by not doing it EVERY loop.
		{
			Network->CheckConn();
            if (SrvParms->EnableRA)
               racCheckConn();
			uiNextCheckConn = (unsigned int)( uiCurrentTime + ( double )( 3 * MY_CLOCKS_PER_SEC ) );
		}

		Network->CheckMessage();
        if (SrvParms->EnableRA)
           racCheckInp();
		tempTime = getNormalizedTime() - tempSecs ; 
		networkTime += tempTime;
		networkTimeCount++;

		if(timerTimeCount >= 1000)
		{
			timerTimeCount = 0;
			timerTime = 0;
		}

		tempSecs = getNormalizedTime() ;		

		checktimers();

		uiCurrentTime=getNormalizedTime();//getNormalizedTime() only once
		tempTime = getNormalizedTime() - tempSecs ; 
		timerTime += tempTime;
		timerTimeCount++;

		if(autoTimeCount >= 1000)
		{
			autoTimeCount = 0;
			autoTime = 0;
		}
		
		tempSecs = getNormalizedTime() ;		

		checkauto();

		tempTime = getNormalizedTime() - tempSecs ; 
		autoTime += tempTime;
		autoTimeCount++;

		Network->ClearBuffers();

		tempTime = getNormalizedTime() - loopSecs ; 
		loopTime += tempTime;
	}

	sysbroadcast("The server is shutting down.");
	if (SrvParms->html>0)
	{
		clConsole.send("Writing offline HTML page...");
		offlinehtml();//HTML	// lb, the if prevents a crash on shutdown if html deactivated ...
		clConsole.send(" Done.\n");
	}
	clConsole.send("Clearing IM Menus...");
	im_clearmenus();
	clConsole.send(" Done.\nClosing sockets...");

	Network->SockClose();

	gcollect();		// cleanup before saving, especially items of deleted chars (Duke, 10.1.2001)

	clConsole.send(" Done.\n");
	if ( !cwmWorldState->Saving() )
	{
		do {
			cwmWorldState->savenewworld(1);
		} while ( cwmWorldState->Saving() );
	}
	clConsole.send("Saving Server.scp...\n");
	//saveserverscript(1);
	saveserverscript();
	clConsole.send("\n");
	clConsole.send("Deleting Classes...");
	DeleteClasses();
	clConsole.send("Done!\n");

	if (NewErrorsLogged())
		clConsole.send("New ERRORS have been logged. Please send the error*.log and critical*.log files to the dev team !\n");
	if (NewWarningsLogged())
		clConsole.send("New WARNINGS have been logged. Probably scripting errors. See the warnings*.log for details !\n");

	if (error) {
		clConsole.send("ERROR: Server terminated by error!\n");

		if (SrvParms->server_log) savelog("Server Shutdown by Error!\n=======================================================================\n\n\n","server.log");
	} else {
		clConsole.send("WOLFPACK: Server shutdown complete!\n");
		if (SrvParms->server_log) savelog("Server Shutdown!\n=======================================================================\n\n\n","server.log");
	}
	//endScrn() ;
	return 0;
}

int ishuman(P_CHAR pc)
{
	// Check if the Player or Npc is human! -- by Magius(CHE)
	if (pc->xid==0x0190 || pc->xid==0x0191) return 1;
	else return 0;
}

void npcact(int s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if (pc != NULL)
	{
		npcaction( pc, addid1[s]);
	}
}

void npcToggleCombat(P_CHAR pc)
{
	pc->war = !pc->war;
	Movement->CombatWalk(pc);
}

int checkBoundingBox(int xPos, int yPos, int fx1, int fy1, int fz1, int fx2, int fy2)
{
	if (xPos>=((fx1<fx2)?fx1:fx2) && xPos<=((fx1<fx2)?fx2:fx1))
		if (yPos>=((fy1<fy2)?fy1:fy2) && yPos<=((fy1<fy2)?fy2:fy1))
			if (fz1==-1 || abs(fz1-Map->Height(xPos, yPos, fz1))<=5)
				return 1;
			return 0;
}

int checkBoundingCircle(int xPos, int yPos, int fx1, int fy1, int fz1, int radius)
{
	if ( (xPos-fx1)*(xPos-fx1) + (yPos-fy1)*(yPos-fy1) <= radius * radius)
		if (fz1==-1 || abs(fz1-Map->Height(xPos, yPos, fz1))<=5)
			return 1;
		return 0;
}

void setabovelight(unsigned char lightchar)
{
	int i;
	if (lightchar != worldcurlevel)
	{
		worldcurlevel=lightchar;
		for (i=0;i<now;i++)
		{
			if (perm[i]) dolight(i, worldcurlevel);
		}
	}
}

void doworldlight(void)
{
	char c='\xFF';
	int i=-1;
	if ((hour<=3 && (!ampm)) || (hour>=10 && ampm)) i=worlddarklevel;
	if ((hour>=10&& (!ampm)) || (hour<=3 && ampm)) i=worldbrightlevel;
	if (i==-1)
	{
		i=(((60*(hour-4))+minute) * (worlddarklevel-worldbrightlevel)) / 360;
		if (ampm)
		{
			i=i+worldbrightlevel;
		} else {
			i=worlddarklevel-i;
		}
	}
	if (wtype) i += 2;
	if (moon1+moon2<4) i++;
	if (moon1+moon2<10) i++;
	c=i;
	if (c!=worldcurlevel)
		worldcurlevel=c;
}

void telltime( UOXSOCKET s )
{
	char tstring[60];
	char tstring2[60];
	int lhour;
	lhour=hour;
	
	if ((minute>=0)&&(minute<=14)) strcpy(tstring,"It is");
	else if ((minute>=15)&&(minute<=30)) strcpy(tstring,"It is a quarter past");
	else if ((minute>=30)&&(minute<=45)) strcpy(tstring,"It is half past");
	else
	{
		strcpy(tstring,"It is a quarter till");
		lhour++;
		if (lhour==0) lhour=12;
	}
	switch( lhour )
	{
	case 1: sprintf( tstring2, "%s one o'clock", tstring );		break;
	case 2: sprintf( tstring2, "%s two o'clock", tstring );		break;
	case 3: sprintf( tstring2, "%s three o'clock", tstring );	break;
	case 4: sprintf( tstring2, "%s four o'clock", tstring );	break;
	case 5: sprintf( tstring2, "%s five o'clock", tstring );	break;
	case 6: sprintf( tstring2, "%s six o'clock", tstring );		break;
	case 7: sprintf( tstring2, "%s seven o'clock", tstring );	break;
	case 8: sprintf( tstring2, "%s eight o'clock", tstring );	break;
	case 9: sprintf( tstring2, "%s nine o'clock", tstring );	break;
	case 10: sprintf( tstring2, "%s ten o'clock", tstring );	break;
	case 11: sprintf( tstring2, "%s eleven o'clock", tstring );	break;
	case 12:
		if( ampm ) 
			sprintf( tstring2, "%s midnight.", tstring );
		else 
			sprintf( tstring2, "%s noon.", tstring );
		break;
	}
	
	if (lhour==12) strcpy(tstring, tstring2);
	else if (ampm)
	{
		if ((lhour>=1)&&(lhour<6)) sprintf(tstring,"%s in the afternoon.",tstring2);
		else if ((lhour>=6)&&(lhour<9)) sprintf(tstring,"%s in the evening.",tstring2);
		else sprintf(tstring,"%s at night.",tstring2);
	}
	else
	{
		if ((lhour>=1)&&(lhour<5)) sprintf(tstring,"%s at night.",tstring2);
		else sprintf(tstring,"%s in the morning.",tstring2);
	}
	
	sysmessage(s,tstring);
}

void impaction(int s, int act)
{
	P_CHAR pc_currchar = currchar[s];
	if (pc_currchar->onhorse && (act==0x10 || act==0x11))
	{
		action(s, 0x1b);
		return;
	}
	if ((pc_currchar->onhorse || (pc_currchar->id1<1 && pc_currchar->id2<90))
		&& (act==0x22))
	{
		return;
	}
	action(s, act);
}

int chardir(P_CHAR a, P_CHAR b)	// direction from character a to char b
{
	int dir,xdif,ydif;

	xdif = b->pos.x - a->pos.x;
	ydif = b->pos.y - a->pos.y;

	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=-1;

	return dir;
}

int chardirxyz(P_CHAR pc, int x, int y)	// direction from character a to char b
{
	int dir, xdif, ydif;

	xdif = x-pc->pos.x;
	ydif = y-pc->pos.y;

	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=-1;

	return dir;
}

int fielddir(P_CHAR pc, int x, int y, int z)
{
//WARNING: unreferenced formal parameter z
	int dir=chardirxyz(pc, x, y);
	switch (dir)
	{
	case 0:
	case 4:
		return 0;
		break;
	case 2:
	case 6:
		return 1;
		break;
	case 1:
	case 3:
	case 5:
	case 7:
	case -1:
		switch(pc->dir) //crashfix, LB
		{
		case 0:
		case 4:
			return 0;
			break;
		case 2:
		case 6:
			return 1;
			break;
		case 1:
		case 3:
		case 5:
		case 7:
			return 1;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, fielddir()\n"); //Morrolan
			return 0;
		}
	default:
		clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, fielddir()\n"); //Morrolan
		return 0;
	}
	return 1;
}

char indungeon(P_CHAR pc)
{
	if (pc->pos.x<5119) 
		return 0;

	int x1 = (pc->pos.x-5119)>>8;
	int y1 = (pc->pos.y>>8);

	switch (y1)
	{
	case 5:
	case 0:	return 1;
	case 1:
		if (x1 != 0) return 1;
		return 0;
	case 2:
	case 3:
		if (x1 < 3) return 1;
		else return 0;
	case 4:
	case 6:
		if (x1 == 0) return 1;
		else return 0;
	case 7:
		if (x1 < 2) return 1;
		else return 0;
	}
	return 0;
}

void npcattacktarget(P_CHAR pc_target2, P_CHAR pc_target)
{
	if (pc_target == pc_target2) return;
	if (pc_target == NULL || pc_target2 == NULL) return;

	if (pc_target->dispz > (pc_target2->dispz +10)) return;//FRAZAI
	if (pc_target->dispz < (pc_target2->dispz -10)) return;//FRAZAI
	if (!(line_of_sight(-1,pc_target2->pos, pc_target->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING))) return; //From Leviathan - Morrolan
	playmonstersound(pc_target, pc_target->id(), SND_STARTATTACK);
	int i;
	unsigned int cdist=0 ;

	if (pc_target->dead || pc_target2->dead) return;

	if (pc_target->targ != INVALID_SERIAL)
		cdist = chardist( pc_target, FindCharBySerial(pc_target->targ));
	else cdist=30;

	if (cdist>chardist(pc_target, pc_target2))
	{
		pc_target->targ = pc_target2->serial;
		pc_target->attacker = pc_target2->serial;
		pc_target->setAttackFirst();
	}

	if (pc_target2->targ != INVALID_SERIAL)
		cdist = chardist(pc_target2, FindCharBySerial(pc_target2->targ));
	else cdist=30;

	if ((cdist > chardist(pc_target, pc_target2))&&
		((!(pc_target2->npcaitype==4)||(!((pc_target2->targ==INVALID_SERIAL)))))) // changed from 0x40 to 4, LB
	{
		pc_target2->targ = pc_target->serial;
		pc_target2->attacker = pc_target->serial;
		pc_target2->resetAttackFirst();
	}

	pc_target->unhide();
	pc_target->disturbMed();

	pc_target2->unhide();
	pc_target2->disturbMed();

	if (pc_target->isNpc())
	{
		if (!(pc_target->war)) 
			npcToggleCombat(pc_target);
		pc_target->setNextMoveTime();
	}
	if ((pc_target2->isNpc())&&!(pc_target2->npcaitype==4)) // changed from 0x40 to 4, LB
	{
		if (!(pc_target2->war)) 
			npcToggleCombat(pc_target2);
		pc_target2->setNextMoveTime();
	}
	 
	sprintf((char*)temp, "You see %s attacking %s!", pc_target2->name.c_str(), pc_target->name.c_str());

	for (i=0;i<now;i++)
		{
		 if (inrange1p(currchar[i], pc_target)&&perm[i])
		 {
			  pc_target->emotecolor = 0x0026;
			  npcemote(i, pc_target2, (char*)temp,1);
		 }
	}
}

////////////////////
// Author : LB
// purpose: converting x,y coords to sextant coords
// input : x,y coords of object 
// output: sextant: sextant cords as string (char *)
// memory for output string sextant has to be reserved by callee !
// if not -> crash (has to be >=36 bytes !)
// thanks goes to Balxan / UOAM for the basic alogithm
// could be optimized a lot, but the freuqency of being called is probably very low
/////////////////////

void getSextantCords(signed int x, signed int y, bool t2a, char *sextant)
{
   double Tx, Ty, Dx, Dy, Cx, Cy, Wx, Wy, Mx, My, Hx, Hy;
   signed int xH, xM, yH, yM;
   char xHs[20], yHs[20], xMs[20], yMs[20];

   if (t2a) // for t2a the center is in the middle
   {
	   Cy = 3112.0;
	   Cx = 5936.0;
   } else 
   {
	  // center, LB's throne *g*
      Cx = 1323.0;
      Cy = 1624.0;
   }

   //  map dimensions
   Wx = 5120.0;
   Wy = 4096.0;

   // convert input ints to float;
   Tx = (double) x;
   Ty = (double) y;

   // main calculation
   Dx = ( (Tx - Cx) * 360.0 ) / Wx;
   Dy = ( (Ty - Cy) * 360.0 ) / Wy;

   ////// now let's get minutes, hours & directions from it
   Hx = (signed int) Dx; // get hours (cut off digits after comma, no idea if there's a cleaner/better way)
   
   Mx = Dx - Hx; // get minutes
   Mx = Mx * 60;

   Hy = (signed int) Dy;
   My = Dy - Hy;
   My = My * 60;

   // convert the results to ints;
   xH = (signed int) Hx;
   xM = (signed int) Mx;
   yH = (signed int) Hy;
   yM = (signed int) My;

   // now compose result string

   numtostr(abs(xH),xHs);
   numtostr(abs(xM),xMs);
   numtostr(abs(yH),yHs);
   numtostr(abs(yM),yMs);

   strcpy((char*)sextant, xHs);
   strcat((char*)sextant,"o ");
   strcat((char*)sextant, xMs);
   strcat((char*)sextant,"' "); 
   if (xH>=0) strcat((char*)sextant,"E"); else strcat((char*)sextant,"W");

   strcat((char*)sextant, "  ");
   strcat((char*)sextant, yHs);
   strcat((char*)sextant,"o ");
   strcat((char*)sextant, yMs);
   strcat((char*)sextant,"' "); 
   if (yH>=0) strcat((char*)sextant,"S"); else strcat((char*)sextant,"N");

}

void npcsimpleattacktarget(P_CHAR pc_target2, P_CHAR pc_target)
{
	if (pc_target2 == NULL || pc_target == NULL)
		return;
	if ((pc_target->targ==pc_target2->serial)&&(pc_target2->targ==pc_target->serial)) return;

	if (pc_target->dead || pc_target2->dead) return;

	pc_target->fight(pc_target2);
	pc_target->setAttackFirst();
	pc_target2->fight(pc_target);
	pc_target2->resetAttackFirst();
}

void openbank(int s, P_CHAR pc_i)
{
	unsigned int ci;
	SERIAL serial = pc_i->serial;
	vector<SERIAL> vecOwn = ownsp.getData(serial);
	for (ci=0;ci<vecOwn.size();ci++)
	{
		P_ITEM pj = FindItemBySerial(vecOwn[ci]);
		if (pj != NULL)
		{
			if (pj->GetOwnSerial()==serial &&
				pj->type==1 && pj->morex==1)
			{
				if(SrvParms->usespecialbank)//if using specialbank
				{
					if(pj->morey==0 && pj->morez==0) //if not initialized yet for the special bank
						pj->morey=123;//convert to new special bank

					if(pj->morey==123)//check if a goldbank
					{
						wearIt(s,pj);
						backpack(s, pj->serial);
						return;
					}
				} else//else if not using specialbank
				{//don't check for goldbank
					wearIt(s,pj);
					backpack(s, pj->serial);
					return;
				}
			}
		}
	} // end of !=-1

	sprintf((char*)temp, "%s's bank box.", pc_i->name.c_str());
	const P_ITEM pic = Items->SpawnItem(s, pc_i, 1, (char*)temp,0,0x09,0xAB,0,0,0);
	if ( pic == NULL ) return;
	pic->layer=0x1d;
	pic->SetOwnSerial(pc_i->serial);
	pic->SetContSerial(pc_i->serial);
	pic->morex=1;
	if(SrvParms->usespecialbank)//AntiChrist - Special Bank
		pic->morey=123;//gold only bank
	pic->type=1;
	wearIt(s,pic);
	backpack(s, pic->serial);
}

//
//special bank - AntiChrist
//
//If activated, you can only put golds into normal banks
//and there are special banks (for now we still use normal bankers,
//but u have to say the SPECIALBANKTRIGGER word to open it)
//where u can put all the items: one notice: the special bank
//is caracteristic of regions....so in Britain you don't find
//the items you leaved in Minoc!
//All this for increasing pk-work and commerce! :)
//(and surely the Mercenary work, so now have to pay strong
//warriors to escort u during your travels!)
//
void openspecialbank(int s, P_CHAR pc)
{
	int serial;
	P_CHAR pc_currchar = currchar[s];
	serial=pc->serial;
	unsigned int ci;
	vector<SERIAL> vecOwn = ownsp.getData(serial);
	for (ci=0;ci<vecOwn.size();ci++)
	{
		P_ITEM pj = FindItemBySerial(vecOwn[ci]);
		if (pj != NULL)
		{
			if (pj->GetOwnSerial()==serial &&
				pj->type==1 && pj->morex==1 &&
				pj->morey!=123 )//specialbank and the current region - AntiChrist
			{
				if(pj->morez==0)//convert old banks into new banks
					pj->morez=pc_currchar->region;

				if(pj->morez==pc_currchar->region)
				{
					wearIt(s,pj);
					backpack(s, pj->serial);
					return;
				}
			}
		}
	} // end of !=-1

	sprintf((char*)temp, "%s's items bank box.", pc->name.c_str());
	const P_ITEM pic = Items->SpawnItem(s, pc,1,(char*)temp,0,0x09,0xAB,0,0,0);
	if(pic == NULL) return;
	pic->layer=0x1d;
	pic->SetOwnSerial(pc->serial);
	pic->SetContSerial(pc->serial);
	pic->morex=1;
	pic->morey=0;//this's a all-items bank
	pic->morez=pc_currchar->region;//let's store the region
	pic->type=1;
	wearIt(s,pic);
	backpack(s, pic->serial);
}

// streamlined by Duke 01.06.2000
int getsubamount(int serial, short id)
{
	unsigned long total=0;
	unsigned int ci;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->id()==id) total+=pi->amount;
		if (pi->type==1) total+=getsubamount(pi->serial, id);
	}
	return total;
}

int getamount(P_CHAR pc, short id)
{
	if (pc == NULL) 
		return 0;
	P_ITEM pi=Packitem(pc);
	if (pi==NULL)
		return 0;
	else
		return getsubamount(pi->serial, id);
}

// not_deleted = output parameter, returns number of items that could NOT be deleted
// somewhat dirty that it defaults to a global dummy variable, sorry for that.
// but i couldnt find any other way to keep the old signature and old name. 
// if theres a cleaner way, let me know, LB

void delequan(P_CHAR pc, short id, int amount, int *not_deleted)
{
	if ( pc == NULL )
		return;

	P_ITEM pi=Packitem(pc);
	if (pi == NULL) 
	{ 
		if (not_deleted != NULL) 
			*not_deleted = amount; 
		return; 
	}

	int nd = pi->DeleteAmount(amount, id);
	if (not_deleted != NULL)
		*not_deleted = nd;
}

void initque() // Initilizes the gmpages[] and counspages[] arrays and also jails
{
	int i;
	for(i=1;i<MAXPAGES;i++)
	{
		gmpages[i].name.erase();
		gmpages[i].reason.erase();
		gmpages[i].serial = 0;
		gmpages[i].timeofcall[0]=0;
		gmpages[i].handled=1;
	}
	for(i=1;i<MAXPAGES;i++)
	{
		counspages[i].name.erase();
		counspages[i].reason.erase();
		counspages[i].serial = 0;
		counspages[i].timeofcall[0]=0;
		counspages[i].handled=1;
	}
	jails[1].pos = Coord_cl(5276, 1164, 0); // Jail1
	jails[1].occupied = false;

	jails[2].pos = Coord_cl(5286, 1164, 0); // Jail2
	jails[2].occupied = false;

	jails[3].pos = Coord_cl(5296, 1164, 0); // Jail3
	jails[3].occupied = false;

	jails[4].pos = Coord_cl(5306, 1164, 0); // Jail4
	jails[4].occupied = false;

	jails[5].pos = Coord_cl(5276, 1174, 0); // Jail5
	jails[5].occupied = false;

	jails[6].pos = Coord_cl(5286, 1174, 0); // Jail6
	jails[6].occupied = false;

	jails[7].pos = Coord_cl(5296, 1174, 0); // Jail7
	jails[7].occupied = false;

	jails[8].pos = Coord_cl(5306, 1174, 0); // Jail8
	jails[8].occupied = false;

	jails[9].pos = Coord_cl(5283, 1184, 0); // Jail9
	jails[9].occupied = false;

	jails[10].pos = Coord_cl(5304, 1184, 0); // Jail10
	jails[10].occupied = false;
}

void donewithcall(int s, int type)
{
	P_CHAR pc_currchar = currchar[s];
	int cn = pc_currchar->callnum;
	if(cn!=0) //Player is on a call
	{
		if(type==1) //Player is a GM
		{
			gmpages[cn].handled=1;
			gmpages[cn].name.erase();
			gmpages[cn].reason.erase();
			gmpages[cn].serial = 0;
			sysmessage(s,"Call removed from the GM queue.");
		}
		else //Player is a counselor
		{
			counspages[cn].handled=1;
			counspages[cn].name.erase();
			counspages[cn].reason.erase();
			counspages[cn].serial = 0;
			sysmessage(s,"Call removed from the Counselor queue.");
		}
		pc_currchar->callnum=0;
	}
	else
	{
		sysmessage(s,"You are currently not on a call");
	}
}

P_ITEM GetOutmostCont(P_ITEM pItem, short rec)
{
	if ( rec<0								// too many recursions
		|| !pItem							// bad parm
		|| isCharSerial(pItem->contserial)	// a character
		|| pItem->isInWorld() )				// in the world
		return pItem;
	P_ITEM pOut=FindItemBySerial(pItem->contserial);	// up one level
	if (!pOut)
	{
		LogErrorVar("container of item %i not found",pItem->serial);
		LogErrorVar("ID of that item is %x",pItem->id());
	}
	return GetOutmostCont(pOut,--rec);
}

P_CHAR GetPackOwner(P_ITEM pItem, short rec)
{
	P_ITEM pio=GetOutmostCont(pItem,--rec);
	if (!pio || pio->isInWorld())
		return NULL;
	return FindCharBySerial(pio->contserial);
}

void goldsfx(int s, int goldtotal)
{
	if (goldtotal==1) soundeffect(s, 0x00, 0x35);
	if ((goldtotal>1)&&(goldtotal<6)) soundeffect(s, 0x00, 0x36);
	else soundeffect(s, 0x00, 0x37);
	return;
}

void playmonstersound(P_CHAR monster, unsigned short id, int sfx)
{
	int basesound=0,x;
	char sf; short offset;


	x=id;
	basesound=creatures[x].basesound;
	sf=creatures[x].soundflag;
	offset=sfx;

	if (basesound != 0)
	{
		switch(sf)
		{
		case 0: break; // in normal case the offset is correct
		case 1: break; // birds sounds will be implmented later

		case 2:	// only start-attack, attack & dýing sounds available
			if (sfx==1 || sfx==3) offset=-1; // idle, defend ? play nothing
			else if (sfx==2) offset=1; // correct offset
			else if (sfx==4) offset=2;
			break;
		case 3: // only start-attack, attack, defense & dying
			if (sfx==1) offset=-1; // idle -> play nothing
			else if (sfx==2) offset=1; // otherwise correct offsets
			else if (sfx==3) offset=2;
			else if (sfx==4) offset=3;
			break;
		case 4: // only a single sound
		
			if (sfx!=0) offset=-1; else offset=0;
		
			break;
		}
		basesound=basesound+offset;
		if (offset!=-1) 
			soundeffect2(monster, basesound);
		return;
	}
}

void addgold(UOXSOCKET s, int totgold)
{
	Items->SpawnItem(s, currchar[s], totgold,"#",1,0x0E,0xED,0,1,1);
}

void usepotion(P_CHAR pc_p, P_ITEM pi)//Reprogrammed by AntiChrist
{
	int s, x;

	if ( pc_p == NULL ) return;
	s = calcSocketFromChar(pc_p);
	P_CHAR pc_currchar = currchar[s];

	//blackwinds fix for potion delay 
	// disabled for now, because both objectdelay AND skilldelay are set, so you can't do anything (Duke)
	// delay should only be for another healing potion, but a good solution will need the new timers :(
#if 0
    if((pc_p->skilldelay<=uiCurrentTime) || pc_p->isGM())
	{
       if (pc_p->skill[ALCHEMY]==1000)
		   pc_p->skilldelay=uiCurrentTime + (MY_CLOCKS_PER_SEC*9);
	   else 
          pc_p->skilldelay=uiCurrentTime + (MY_CLOCKS_PER_SEC*14);
#endif

	switch(pi->morey)
	{
	case 1: // Agility Potion
		staticeffect(pc_p, 0x37, 0x3a, 0, 15);
		switch(pi->morez)
		{
		case 1:
			tempeffect(currchar[s], pc_p, 6, 5+RandomNum(1,10), 0, 0, 120);	// duration 2 minutes Duke, 31.10.2000
			sysmessage(s, "You feel more agile!");
			break;
		case 2:
			tempeffect(currchar[s], pc_p, 6, 10+RandomNum(1,20), 0, 0, 120);
			sysmessage(s, "You feel much more agile!");
			break;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
			return;
		}
		soundeffect2(pc_p, 0x01E7);
		if (s!=-1) updatestats(pc_p, 2);
		break;

	case 2: // Cure Potion
		if (pc_p->poisoned<1)
			sysmessage(s,"The potion had no effect.");
		else
		{
			switch(pi->morez)
			{
			case 1:
				x=RandomNum(1,100);
				if (pc_p->poisoned==1 && x<81) pc_p->poisoned=0;
				if (pc_p->poisoned==2 && x<41) pc_p->poisoned=0;
				if (pc_p->poisoned==3 && x<21) pc_p->poisoned=0;
				if (pc_p->poisoned==4 && x< 6) pc_p->poisoned=0;
				break;
			case 2:
				x=RandomNum(1,100);
				if (pc_p->poisoned==1) pc_p->poisoned=0;
				if (pc_p->poisoned==2 && x<81) pc_p->poisoned=0;
				if (pc_p->poisoned==3 && x<41) pc_p->poisoned=0;
				if (pc_p->poisoned==4 && x<21) pc_p->poisoned=0;
				break;
			case 3:
				x=RandomNum(1,100);
				if (pc_p->poisoned==1) pc_p->poisoned=0;
				if (pc_p->poisoned==2) pc_p->poisoned=0;
				if (pc_p->poisoned==3 && x<81) pc_p->poisoned=0;
				if (pc_p->poisoned==4 && x<61) pc_p->poisoned=0;
				break;
			default:
				clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
				return;
			}
			if (pc_p->poisoned) sysmessage(s,"The potion was not able to cure this poison."); else
			{
				staticeffect(pc_p, 0x37, 0x3A, 0, 15);
				soundeffect2(pc_p, 0x01E0); //cure sound - SpaceDog
				sysmessage(s,"The poison was cured.");
			}
		}
		impowncreate(calcSocketFromChar(pc_p), pc_p, 1); //Lb, makes the green bar blue or the blue bar blue !
		break;

	case 3: // Explosion Potion
		if (pc_currchar->inGuardedArea()) // Ripper 11-14-99
		{
			sysmessage(s," You cant use that in town!");
			return;
		}
		addid1[s] = static_cast<unsigned char>(pi->serial >> 24);
		addid2[s] = static_cast<unsigned char>(pi->serial >> 16);
		addid3[s] = static_cast<unsigned char>(pi->serial >> 8);
		addid4[s] = static_cast<unsigned char>(pi->serial % 256);
		sysmessage(s, "Now would be a good time to throw it!");
		tempeffect(currchar[s], currchar[s], 16, 0, 1, 3);
		tempeffect(currchar[s], currchar[s], 16, 0, 2, 2);
		tempeffect(currchar[s], currchar[s], 16, 0, 3, 1);
		tempeffect2(currchar[s], pi, 17, 0, 4, 0);
		target(s,0,1,0,207,"*throw*");
		return; // lb bugfix, break is wronh here because it would delete bottle

	case 4: // Heal Potion
		switch(pi->morez)
		{
		case 1:
			pc_p->hp=min(pc_p->hp+5+RandomNum(1,5)+pc_p->skill[17]/100,pc_p->st);
			sysmessage(s, "You feel better!");
			break;
		case 2:
			pc_p->hp=min(pc_p->hp+15+RandomNum(1,10)+pc_p->skill[17]/50, pc_p->st);
			sysmessage(s, "You feel more healty!");
			break;
		case 3:
			pc_p->hp=min(pc_p->hp+20+RandomNum(1,20)+pc_p->skill[17]/40, pc_p->st);
			sysmessage(s, "You feel much more healty!");
			break;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
			return;
		}
		if (s!=-1) updatestats(pc_p, 0);
		staticeffect(pc_p, 0x37, 0x6A, 0x09, 0x06); // Sparkle effect
		soundeffect2(pc_p, 0x01F2); //Healing Sound - SpaceDog
		break;
	case 5: // Night Sight Potion
		staticeffect(pc_p, 0x37, 0x6A, 0x09, 0x06);
		tempeffect(currchar[s], pc_p, 2, 0, 0, 0,(720*secondsperuominute*MY_CLOCKS_PER_SEC)); // should last for 12 UO-hours
		soundeffect2(pc_p, 0x01E3);
		break;
	case 6: // Poison Potion
		if(pc_p->poisoned < pi->morez) pc_p->poisoned=pi->morez;
		if(pi->morez>4) pi->morez=4;
		pc_p->poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // lb, poison wear off timer setting
		impowncreate(calcSocketFromChar(pc_p), pc_p, 1); //Lb, sends the green bar !
		soundeffect2(pc_p, 0x0246); //poison sound - SpaceDog
		sysmessage(s, "You poisoned yourself! *sigh*"); //message -SpaceDog
		break;
	case 7: // Refresh Potion
		switch(pi->morez)
		{
		case 1:
			pc_p->stm=min(pc_p->stm+20+RandomNum(1,10), (int)pc_p->effDex());
			sysmessage(s, "You feel more energetic!");
			break;
		case 2:
			pc_p->stm=min(pc_p->stm+40+RandomNum(1,30), (int)pc_p->effDex());
			sysmessage(s, "You feel much more energetic!");
			break;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
			return;
		}
		if (s!=-1) updatestats(pc_p, 2);
		staticeffect(pc_p, 0x37, 0x6A, 0x09, 0x06); // Sparkle effect
		soundeffect2(pc_p, 0x01F2); //Healing Sound
		break;
	case 8: // Strength Potion
		staticeffect(pc_p, 0x37, 0x3a, 0, 15);
		switch(pi->morez)
		{
		case 1:
			tempeffect(currchar[s], pc_p, 8, 5+RandomNum(1,10), 0, 0, 120);	// duration 2 minutes Duke, 31.10.2000
			sysmessage(s, "You feel more strong!");
			break;
		case 2:
			tempeffect(currchar[s], pc_p, 8, 10+RandomNum(1,20), 0, 0, 120);
			sysmessage(s, "You feel much more strong!");
			break;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
			return;
		}
		soundeffect2(pc_p, 0x01EE);
		break;

	case 9: // Mana Potion
		switch(pi->morez)
		{
		case 1:
		
			pc_p->mn=min(pc_p->mn+10+pi->morex/100, (unsigned)pc_p->in);
		
			break;
		case 2:
		
			pc_p->mn=min(pc_p->mn+20+pi->morex/50, (unsigned)pc_p->in);
		
			break;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
			return;
		}
		if (s!=-1) updatestats(pc_p, 1);
		staticeffect(pc_p, 0x37, 0x6A, 0x09, 0x06); // Sparkle effect
		soundeffect2(pc_p, 0x01E7); //agility sound - SpaceDog
		break;

	case 10: //LB's LSD potion, 5'th november 1999
		if (pi->id()!=0x1841) return; // only works with an special flask
		if (s==-1) return;
		if (LSD[s]==1)
		{
			sysmessage(s,"no,no,no,cant you get enough ?");
			return;
		}
		tempeffect(pc_p, pc_p, 20, 60+RandomNum(1,120), 0, 0); // trigger effect
		staticeffect(pc_p, 0x37, 0x6A, 0x09, 0x06); // Sparkle effect
		soundeffect5(calcSocketFromChar(pc_p), 0x00, 0xF8); // lsd sound :)
		break;

	default:
		clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, usepotion()\n"); //Morrolan
		return;
	}
	soundeffect2(pc_p, 0x0030);
	if (pc_p->id1>=1 && pc_p->id2>90 && pc_p->onhorse==0) 
		npcaction( pc_p, 0x22);
	//empty bottle after drinking - Tauriel
	if (pi->amount!=1)
	{
		pi->amount--;
	}
	//empty bottle after drinking - Tauriel
	pi->SetContSerial(-1);
	if (pi->morey!=3)
	{
		int lsd = pi->morey; // save morey before overwritten
		int kser = pi->serial;

		pi->Init(0);
		pi->SetSerial(kser);
		pi->setId(0x0F0E);

		if (lsd==10) // empty Lsd potions
		{
			pi->setId(0x183d);
		}
		pi->pileable = true;
		pi->MoveTo(pc_p->pos.x,pc_p->pos.y,pc_p->pos.z);
		pi->priv|=0x01;
	}
	else
	{
		Items->DeleItem(pi);
	}
	RefreshItem(pi);// AntiChrist
	// end empty bottle change
#if 0
	}
     else sysmessage(s, "You must wait a few seconds before you drink another potion");
#endif
}

int calcValue(P_ITEM pi, int value)
{
	int mod=10;
	if (pi == NULL)
		return value;

	if (pi->type==19)
	{
		if (pi->morex>500) mod=mod+1;
		if (pi->morex>900) mod=mod+1;
		if (pi->morex>1000) mod=mod+1;
		if (pi->morez>1) mod=mod+(3*(pi->morez-1));
		value=(value*mod)/10;
	}

	// Lines added for Rank System by Magius(CHE)
	if (pi->rank>0 && pi->rank<10 && SrvParms->rank_system==1)
	{
		value=(int) (pi->rank*value)/10;
	}
	if (value<1) value=1;
	// end addon

	// Lines added for Trade System by Magius(CHE) (2)
	if (pi->rndvaluerate<0) pi->rndvaluerate=0;
	if (pi->rndvaluerate!=0 && SrvParms->trade_system==1) {
		value+=(int) (value*pi->rndvaluerate)/1000;
	}
	if (value<1) value=1;
	// end addon

	return value;
}

int calcGoodValue(P_CHAR npcnum2, P_ITEM pi, int value,int goodtype)
{ // Function Created by Magius(CHE) for trade System
	int actreg=calcRegionFromXY(npcnum2->pos.x, npcnum2->pos.y);
	int regvalue=0;
	int x;
	if (pi == NULL)
		return value;

	int good=pi->good;

	if (good<=-1 || good >255 || actreg<=-1 || actreg>255) return value;

	if (goodtype==1) regvalue=region[actreg].goodsell[pi->good]; // Vendor SELL
	if (goodtype==0) regvalue=region[actreg].goodbuy[pi->good]; // Vendor BUY

	x=(int) (value*abs(regvalue))/1000;

	if (regvalue<0)	value-=x;
	else value+=x;

	if (value<=0) value=1; // Added by Magius(CHE) (2)

	return value;
}

void StoreItemRandomValue(P_ITEM pi,int tmpreg)
{ // Function Created by Magius(CHE) for trade System
	int max=0,min=0;

	if (pi == NULL)
		return;
	if (pi->good<0) return;
	if (tmpreg<0)
	{
		P_ITEM pio=GetOutmostCont(pi);
		if (!pio) return;
		if (pio->isInWorld())
			tmpreg=calcRegionFromXY(pio->pos.x,pio->pos.y);
		else
		{
			P_CHAR pc=FindCharBySerial(pio->contserial);
			if (!pc) return;
			tmpreg=calcRegionFromXY(pc->pos.x,pc->pos.y);
		}
	}

	if (tmpreg<0 || tmpreg>255 || pi->good<0 || pi->good>255) return;

	min=region[tmpreg].goodrnd1[pi->good];
	max=region[tmpreg].goodrnd2[pi->good];

	if (max!=0 || min!=0)
	{
		pi->rndvaluerate=(int) RandomNum(min,max);
	}
}

void loadmetagm() // LORD BINARY
{
	char sect[512];
	int i,k,mode,n;
	unsigned int const plus=1;
	int pm,ss,y;

	openscript("metagm.scp");

	for (i=0;i<256;i++)
	{
		for (int a=0;a<7;a++) metagm[i][a]=0;
	}

	i=-1;k=0;
	do
	{
		i++;
		sprintf(sect, "COMMAND_CLEARANCE %i", i);
		if (i_scripts[metagm_script]->find(sect))
		{
			k++;n=0;mode=-1;pm=-1;
			unsigned long loopexit=0;
			do
			{
				read2();
				if (script1[0]!='}')
				{
					if (!(strcmp("MODE+",(char*)script1))) { mode=1;n=1;pm=1;} // plus mode
					if (!(strcmp("MODE-",(char*)script1))) { mode=2;n=1;pm=0;}	// minus mode

					if (pm>-1 && n==0) // only check for commnad words AFTER the mode token
					{
						ss=0; y=-1;unsigned long loopexit=0;
						while((command_table[ss].cmd_name)&&(y==-1)&& (++loopexit < MAXLOOPS) ) // search for the command
						{
							if(!(strcmp(command_table[ss].cmd_name, (char*)script1))) y=ss;
							ss++;
						}

						if (y==-1) // not found ?
						{
							clConsole.send("warning: found unknown command %s in meta gm script\n",script1);
							clConsole.send("press any key to continue. . .\n");//AntiChrist-so we know if something's wrong
							//getchar();//AntiChrist-so we know if something's wrong
						} else // found it!
						{
							if (command_table[y].cmd_priv_m!=255)
							{
								           metagm[i][command_table[y].cmd_priv_m] =  (metagm[i][command_table[y].cmd_priv_m]) | (plus << (command_table[y].cmd_priv_b) ) ;
								if (pm==0) metagm[i][command_table[y].cmd_priv_m] = ~(metagm[i][command_table[y].cmd_priv_m]);
							}
						}
					}
					n=0;
				}
			}
			while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

			if (mode==-1)
			{
				error=1;
				keeprun=0;
				clConsole.send("\n Meta-Gm script parsing error, mode keyword missing, section# %i - closing wolfpack\n",i);
			}

			if (pm==0)
			{
				for (int aa=0;aa<7;aa++) if (metagm[i][aa]==0) metagm[i][aa]=0xFFFFFFFF;
			
			}
		}

	} while (i<255);

	closescript();
	clConsole.send("Meta Gm script loaded... %i priv3 shortcut/s\n",k-3);
}

void dosocketmidi(int s)
{
	Script *pScp=i_scripts[regions_script];
	if (!pScp->Open()) return;
	//openscript("regions.scp");

	char sect[512];
	P_CHAR pc_currchar = currchar[s];

	if (pc_currchar->war)
	{
		strcpy(sect, "MIDILIST COMBAT");
	}
	else
	{
		sprintf(sect, "MIDILIST %i", region[pc_currchar->region].midilist);
	}
	if (region[pc_currchar->region].midilist!=0 && !pScp->find(sect))
	{
		//closescript();
		pScp->Close();
		return;
	}

	char midiarray[50];
	int i=0;
	unsigned long loopexit=0;
	do
	{
		//read2();
		pScp->NextLineSplitted();
		if (script1[0]!='}')
		{
			if (!(strcmp("MIDI",(char*)script1)))
			{
				midiarray[i]=str2num(script2);
				i++;
			}
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();
	//closescript();
	if (i!=0)
	{
		i=rand()%(i);
		playmidi(s, 0, midiarray[i]);
	}
}


#ifndef __unix__
void Writeslot(LPSTR lpszMessage)
{
	BOOL fResult;
	HANDLE hFile;
	DWORD cbWritten;

	hFile = CreateFile("\\\\*\\mailslot\\uoxmail", GENERIC_WRITE,FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES) NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);
	fResult = WriteFile(hFile,lpszMessage,(DWORD) lstrlen(lpszMessage) + 1,
		&cbWritten,(LPOVERLAPPED) NULL);
	fResult = CloseHandle(hFile);
	if(!atoi(lpszMessage)) clConsole.send("WOLFPACK: %s\n",lpszMessage);
}
#else
void Writeslot(char * lpszMessage)
{
	FILE *f;

	f=fopen("WOLFPACK.log","a");
	if(f==NULL) return;
	fprintf(f,"%s\n",lpszMessage);
	if(!atoi(lpszMessage)) clConsole.send("WOLFPACK: %s\n",lpszMessage);
	fclose(f);
	return;
}
#endif


int numbitsset( int number )
{
	int bitsset = 0;

	while( number )
	{
		if( number & 0x1 ) bitsset++;
		number >>= 1;
	}
	return bitsset;
}

int whichbit( int number, int bit )
{
	int i, setbits = 0, whichbit = 0, intsize = sizeof(int) * 8;

	for( i=0;i<intsize;i++ )
	{
		if( number & 0x1 ) setbits++;

		if( setbits == bit )
		{
			whichbit = i+1;
			break;
		}
		number >>= 1;
	}

	return whichbit;
}



// Dupois - added to do easy item sound effects based on an
//			items id1 and id2 fields in struct items. Then just define the CASE statement
//			with the proper sound function to play for a certain item as shown.
//			Use the DEFAULT case for ranges of items (like all ingots make the same thump).
// Sounds - coins dropping (all the same no matter what amount because all id's equal 0x0EED
//			ingot dropping (makes a big thump - used the heavy gem sound)
//			gems dropping (two type broke them in half to make some sound different then others)
// NOTE	  - I wasn't sure what the different soundeffect() func's did so I just used
//			soundeffect() and it seemed to work fairly well.
// Added Oct 09, 1998
// new interface, Duke, 25.3.2001
void itemsfx(UOXSOCKET s, short item)
{
	if (item==0x0EED)
	{
		goldsfx(s, 2);
	}
	else if ((item>=0x0F0F)&&(item<=0x0F20))	// Any gem stone (typically smaller)
	{
		soundeffect(s, 0x00, 0x32);
	}
	else if ((item>=0x0F21)&&(item<=0x0F30))	// Any gem stone (typically larger)
	{
		soundeffect(s, 0x00, 0x34);
	}
	else if ((item>=0x1BE3)&&(item<=0x1BFA))	// Any Ingot
	{
		soundeffect(s, 0x00, 0x33);
	}
	soundeffect(s, 0x00, 0x42);					// play default item move sfx // 00 48
}

void init_creatures(void) // assigns the basesound, soundflag, who_am_i flag of all npcs
// LB 9.8.99/17.9/20.9
{
/////////////////////////////////////////////////////////////////////////////////////////////////
// soundflags	0: normal, 5 sounds (attack-started, idle, attack, defence, dying, see WOLFPACK.h)
//				1: birds .. only one "bird-shape" and zillions of sounds ...
//				2: only 3 sounds -> (attack,defence,dying)
//				3: only 4 sounds ->	(attack-started,attack,defnce,dying)
//				4: only 1 sound !!
//
// who_am_i bit # 1 creature can fly (must have the animations, so better dont change)
//				# 2 anti-blink: these creatures dont have animation #4, if not set creaure will randomly dissapear in battle
//								if you find a creature that blinks while fighting, set that bit
//				# 3 animal-bit		(currently not used/set)
//				# 4 water creatures (currently not used/set)
// icon: used for tracking, to set the appropriate icon
////////////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&creatures[0], 0, 2048*sizeof( creat_st ));

	creatures[0x01].basesound=0x01AB;							// Ogre
	creatures[0x01].icon=8415;

	creatures[0x02].basesound=0x016F;							// Ettin
	creatures[0x02].icon=8408;

	creatures[0x03].basesound=0x01D7;							// Zombie
	creatures[0x03].icon=8428;

	creatures[0x04].basesound=0x0174;							// Gargoyle
	creatures[0x04].who_am_i+=1; // set can_fly_bit
	creatures[0x04].icon=8409;

	creatures[0x05].basesound=0x008f;							// Eagle
	creatures[0x05].who_am_i+=1; // set can_fly bit
	creatures[0x05].who_am_i+=2; // set anti blink bit
	creatures[0x05].icon=8434;

	creatures[0x06].basesound=0x007d;							// Bird
	creatures[0x06].who_am_i+=1; //set fly bit
	creatures[0x06].soundflag=1; // birds need special treatment cause there are about 20 bird-sounds
	creatures[0x06].icon=8430;

	creatures[0x07].basesound=0x01b0;							// Orc
	creatures[0x07].icon=8416;

	creatures[0x08].basesound=0x01ba;							// corpser
	creatures[0x08].soundflag=3;
	creatures[0x08].icon=8402;

	creatures[0x09].basesound=0x0165;							// daemon
	creatures[0x09].who_am_i+=1;
	creatures[0x09].icon=8403;

	creatures[0x0a].basesound=0x0165;							// daemon 2
	creatures[0x0a].who_am_i+=1;
	creatures[0x0a].icon=8403;

	creatures[0x0c].basesound=362;								// Green dragon
	creatures[0x0c].who_am_i+=1; // flying creature
	creatures[0x0c].icon=8406;

	creatures[0x0d].basesound=263;								// air-ele
	creatures[0x0d].icon=8429;

	creatures[0x0e].basesound=268;								// earth-ele
	creatures[0x0e].icon=8407;

	creatures[0x0f].basesound=273;								// fire-ele
	creatures[0x0f].icon=8435;

	creatures[0x10].basesound=0x0116;							// water ele
	creatures[0x10].icon=8459;

	creatures[0x11].basesound=0x01b0;							// Orc	2
	creatures[0x11].icon=8416;

	creatures[0x12].basesound=0x016F;							// Ettin 2
	creatures[0x12].icon=8408;

	creatures[0x15].basesound=219;								// Giant snake
	creatures[0x15].icon=8446;

	creatures[0x16].basesound=377;								// gazer
	creatures[0x16].icon=8426;

	creatures[0x18].basesound=412;								// liche
	creatures[0x18].icon=8440; // counldnt find a better one :(

	creatures[0x1a].basesound=382;								// ghost 1
	creatures[0x1a].icon=8457;

	creatures[0x1c].basesound=387;								// giant spider
	creatures[0x1c].icon=8445;

	creatures[0x1d].basesound=158;								// gorialla
	creatures[0x1d].icon=8437;

	creatures[0x1e].basesound=402;								// harpy
	creatures[0x1e].icon=8412;

	creatures[0x1f].basesound=407;								// headless
	creatures[0x1f].icon=8458;

	creatures[0x21].basesound=417;								// lizardman
	creatures[0x23].basesound=417;
	creatures[0x24].basesound=417;
	creatures[0x25].basesound=417;
	creatures[0x26].basesound=417;
	creatures[0x21].icon=creatures[0x23].icon=
	creatures[0x24].icon=creatures[0x25].icon=
	creatures[0x26].icon=8414;

	creatures[0x27].basesound=422;								// mongbat
	creatures[0x27].who_am_i+=1; // yes, they can fly
	creatures[0x27].icon=8441;

	creatures[0x29].basesound=0x01b0;							// orc 3
	creatures[0x29].icon=8416;

	creatures[0x2a].basesound=437;								// ratman
	creatures[0x2c].basesound=437;
	creatures[0x2d].basesound=437;
	creatures[0x2a].icon=creatures[0x2c].icon=
	creatures[0x2d].icon=8419;

	creatures[0x2f].basesound=0x01ba;							// Reaper
	creatures[0x2f].icon=8442;

	creatures[0x30].basesound=397;								// giant scorprion
	creatures[0x30].icon=8420;

	creatures[0x32].basesound=452;								// skeleton 2
	creatures[0x32].icon=8423;

	creatures[0x33].basesound=456;								// slime
	creatures[0x33].icon=8424;

	creatures[0x34].basesound=219;								// Snake
	creatures[0x34].icon=8444;

	creatures[0x35].basesound=461;								// troll
	creatures[0x35].icon=8425;
	creatures[0x36].basesound=461;								// troll 2
	creatures[0x36].icon=8425;
	creatures[0x37].basesound=461;								// troll 3
	creatures[0x37].icon=8425;

	creatures[0x38].basesound=452;								// skeleton 3
	creatures[0x38].icon=8423;

	creatures[0x39].basesound=452;								// skeleton 4
	creatures[0x39].icon=8423;

	creatures[0x3a].basesound=466;								// wisp
	creatures[0x3a].icon=8448;

	creatures[0x3b].basesound=362;								// red dragon
	creatures[0x3b].who_am_i+=1; // set fly bit
	creatures[0x3c].basesound=362;								// smaller red dragon
	creatures[0x3c].who_am_i+=1;
	creatures[0x3d].basesound=362;								// smaller green dragon
	creatures[0x3d].who_am_i+=1;
	creatures[0x3b].icon=creatures[0x3c].icon=
	creatures[0x3d].icon=8406;

	creatures[0x96].basesound=477;								// sea serpant
	creatures[0x96].basesound=3;
	creatures[0x96].icon=8446; // normal serpant icon

	creatures[0x97].basesound=138;								// dolphin
	creatures[0x97].icon=8433; // correct icon ???

	creatures[0xc8].basesound=168;								// white horse
	creatures[0xc8].icon=8479;

	creatures[0xc9].basesound=105;								// cat
	creatures[0xc9].who_am_i+=2; // set blink flag
	creatures[0xc9].icon=8475;

	creatures[0xca].basesound=90;								// alligator
	creatures[0xca].icon=8410;

	creatures[0xcb].basesound=196;								// small pig
	creatures[0xcb].icon=8449;

	creatures[0xcc].basesound=168;								// brown horse
	creatures[0xcc].icon=8481;

	creatures[0xcd].basesound=201;								// rabbit
	creatures[0xcd].soundflag=2;								// rabbits only have 3 sounds, thus need special treatment
	creatures[0xcd].icon=8485;

	creatures[0xcf].basesound=214;								// wooly sheep
	creatures[0xcf].icon=8427;

	creatures[0xd0].basesound=110;								// chicken
	creatures[0xd0].icon=8401;

	creatures[0xd1].basesound=153;								// goat
	creatures[0xd1].icon=8422; // theres no goat icon, so i took a (differnt) sheep

	creatures[0xd3].basesound=95;								// brown bear
	creatures[0xd3].icon=8399;

	creatures[0xd4].basesound=95;								// grizzly bear
	creatures[0xd4].icon=8411;

	creatures[0xd5].basesound=95;								// polar bear
	creatures[0xd5].icon=8417;

	creatures[0xd6].basesound=186;								// panther
	creatures[0xd6].who_am_i+=2;
	creatures[0xd6].icon=8473;

	creatures[0xd7].basesound=392;								// giant rat
	creatures[0xd7].icon=8400;

	creatures[0xd8].basesound=120;								// cow
	creatures[0xd8].icon=8432;

	creatures[0xd9].basesound=133;								// dog
	creatures[0xd9].icon=8405;

	creatures[0xdc].basesound=183;								// llama
	creatures[0xdc].soundflag=2;
	creatures[0xdc].icon=8438;

	creatures[0xdd].basesound=224;								// walrus
	creatures[0xdd].icon=8447;

	creatures[0xdf].basesound=216;								// lamb/shorn sheep
	creatures[0xdf].soundflag=2;
	creatures[0xdf].icon=8422;

	creatures[0xe1].basesound=229;								// jackal
	creatures[0xe1].who_am_i+=2; // set anti blink bit
	creatures[0xe1].icon=8426;

	creatures[0xe2].basesound=168;								// yet another horse
	creatures[0xe2].icon=8484;

	creatures[0xe4].basesound=168;								// horse ...
	creatures[0xe4].icon=8480;

	creatures[0xe7].basesound=120;								// brown cow
	creatures[0xe7].who_am_i+=2;
	creatures[0xe7].icon=8432;

	creatures[0xe8].basesound=100;								// bull
	creatures[0xe8].who_am_i+=2;
	creatures[0xe8].icon=8431;

	creatures[0xe9].basesound=120;								// b/w cow
	creatures[0xe9].who_am_i+=2;
	creatures[0xe9].icon=8451;

	creatures[0xea].basesound=130;								// deer
	creatures[0xea].soundflag=2;
	creatures[0xea].icon=8404;

	creatures[0xed].basesound=130;								// small deer
	creatures[0xed].soundflag=2;
	creatures[0xed].icon=8404;

	creatures[0xee].basesound=204;								// rat
	creatures[0xee].icon=8483;

	creatures[0x122].basesound=196;								// Boar
	creatures[0x122].icon=8449;

	creatures[0x123].basesound=168;								// pack horse
	creatures[0x123].icon=8486;

	creatures[0x124].basesound=183;								// pack llama
	creatures[0x124].soundflag=2;
	creatures[0x124].icon=8487;

	creatures[0x23d].basesound=263;								// e-vortex
	creatures[0x23e].basesound=512;								// blade spritit
	creatures[0x23e].soundflag=4;

	creatures[0x600].basesound=115;								// cougar;
	creatures[0x600].icon=8473;

	creatures[0x190].icon=8454;
	creatures[0x191].icon=8455;
}

////////////////
// name:	bgsound
// pupose:	Plays background sounds of the game
// history:	improved/completed/rewritten by LB 9-10 August 1999
//			added griditerator (Duke, 19.11.2000)
//
void bgsound(P_CHAR pc)
{
	int sound;
	int distance=(VISRANGE+5);
	vector<P_CHAR> inrange;
	int basesound=0;
	int bigf,xx;
	char sf,bfs;

	if (pc == NULL) return;

	int y=0;
	cRegion::RegionIterator4Chars ri(pc->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if (pc != NULL)
		{
			if((pc->isPlayer())&&(!(pc->dead))&&(!(pc->war))&&(y<=10))
			{
				if (!pc->free) // lb, bugfix !
				{
					if (inRange(pc->pos.x, pc->pos.y, pc->pos.x, pc->pos.y, distance))
					{
						inrange.push_back(pc);
					}
				}
			}
		}
	}

	if (inrange.size() > 0)
	{
		sound=RandomNum(0, inrange.size()-1);
		xx = inrange[sound]->id();
		if (xx>-1 && xx<2048)
		{
			basesound=creatures[xx].basesound;
			sf=creatures[xx].soundflag;
		}
		else return;

		if (basesound!=0)
		{
			switch(sf) // play only idle sounds, if there arnt any, dont play them !
			{
			case 0: basesound++; break;	// normal case -> play idle sound
			case 1: basesound++; break;	// birds sounds will be implmented later
			case 2:	basesound=0; break;	// no idle sound, so dont play it !
			case 3: basesound=0; break;	// no idle sound, so dont play it !
			case 4:	break; // only a single sound, play it !
			}
			if (bgsound!=0) // bugfix lb
			{
				sfx[2]=basesound>>8;
				sfx[3]=basesound%256;
				sfx[6]=inrange[sound]->pos.x>>8;
				sfx[7]=inrange[sound]->pos.x%256;
				sfx[8]=inrange[sound]->pos.y>>8;
				sfx[9]=inrange[sound]->pos.y%256;
				Xsend(calcSocketFromChar(pc), sfx, 12); //bugfix, LB
			}
		}
	}
	// play random mystic-sounds also if no creature is in range ...

	bigf=rand()%3333;
	basesound=0;

	if (bigf==33)
	{
		bfs=rand()%7;
		switch(bfs)
		{
		case 0: basesound=595;break; // gnome sound
		case 1: basesound=287;break; // bigfoot 1
		case 2: basesound=288;break; // bigfoot 2
		case 3: basesound=639;break; // old snake sound
		case 4: basesound=179;break; // lion sound
		case 5: basesound=246;break; // mystic
		case 6: basesound=253;break; // mystic II
		}

		if (basesound !=0)
		{
			sfx[2] = (unsigned char) (basesound>>8);
			sfx[3] = (unsigned char) (basesound%256);
			sfx[6] = (unsigned char) (pc->pos.x>>8);
			sfx[7] = (unsigned char) (pc->pos.x%256);
			sfx[8] = (unsigned char) (pc->pos.y>>8);
			sfx[9] = (unsigned char) (pc->pos.y%256);
			Xsend(calcSocketFromChar(pc), sfx, 12); //bugfix LB
		}
	}
}

void Karma(P_CHAR pc_toChange,P_CHAR pc_Killed, int nKarma)
{	// nEffect = 1 positive karma effect
	int nCurKarma=0, nChange=0, nEffect=0;

	nCurKarma = pc_toChange->karma;

	if((nCurKarma>10000)||(nCurKarma<-10000))
		if(nCurKarma>10000)
			pc_toChange->karma=10000;
		else
			pc_toChange->karma=-10000;

	if(nCurKarma<nKarma && nKarma>0)
	{
		nChange=((nKarma-nCurKarma)/75);
		pc_toChange->karma=(nCurKarma+nChange);
		nEffect=1;
	}

	//AntiChrist - modified to avoid crashes
	//the nKilledID==-1 check and the chars[nKilledID] check were in the same line
	//That may cause some crash with some compilator caus there's no a defined
	//order in executing these if checks
	if((nCurKarma>nKarma)&&(pc_Killed == NULL))
	{
		nChange=((nCurKarma-nKarma)/50);
		pc_toChange->karma=(nCurKarma-nChange);
		nEffect=0;
	}
	else if((nCurKarma>nKarma)&&(pc_Killed->karma>0))
	{
		nChange=((nCurKarma-nKarma)/50);
		pc_toChange->karma=(nCurKarma-nChange);
		nEffect=0;
	}

	if((nChange==0)||(pc_toChange->isNpc()))
		return;
	if(nChange<=25)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained a little karma.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost a little karma.");
			return;
		}
	if(nChange<=75)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained some karma.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost some karma.");
			return;
		}
	if(nChange<=100)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained alot of karma.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost alot of karma.");
			return;
		}
	if(nChange>100)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained a huge amount of karma.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost a huge amount of karma.");
			return;
		}
}

//added by Genesis 11-8-98
void Fame(P_CHAR pc_toChange, int nFame)
{
	int nCurFame, nChange=0, nEffect=0;

	if (pc_toChange->isNpc()) //NPCs don't gain fame.
		return;

	nCurFame= pc_toChange->fame;
	if(nCurFame>nFame) // if player fame greater abort function
	{
		if(nCurFame>10000)
			pc_toChange->fame=10000;
		return;
	}
	if(nCurFame<nFame)
	{
		nChange=(nFame-nCurFame)/75;
		pc_toChange->fame=(nCurFame+nChange);
		nEffect=1;
	}
	if(pc_toChange->dead)
	{
		if(nCurFame<=0)
			pc_toChange->fame=0;
		else
		{
			nChange=(nCurFame-0)/25;
			pc_toChange->fame=(nCurFame-nChange);
		}
		pc_toChange->deaths++;
		nEffect=0;
	}
	if((nChange==0)||(pc_toChange->isNpc()))
		return;
	if(nChange<=25)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained a little fame.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost a little fame.");
			return;
		}
	if(nChange<=75)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained some fame.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost some fame.");
			return;
		}
	if(nChange<=100)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained alot of fame.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost alot of fame.");
			return;
		}
	if(nChange>100)
		if(nEffect)
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have gained a huge amount of fame.");
			return;
		}
		else
		{
			sysmessage(calcSocketFromChar(pc_toChange),
				"You have lost a huge amount of fame.");
			return;
		}
}

void enlist(int s, int listnum) // listnum is stored in items morex
{
	int x,pos;
	char sect[50];

	openscript("items.scp");
	sprintf(sect, "ITEMLIST %i", listnum);
	if (!i_scripts[items_script]->find(sect))
	{
		closescript();
		if (n_scripts[custom_item_script][0]!=0)
		{
			openscript(n_scripts[custom_item_script]);
			if (!i_scripts[custom_item_script]->find(sect))
			{
				clConsole.send("WOLFPACK: ITEMLIST not found, aborting.\n");
				closescript();//AntiChrist
				return;
			} else strcpy(sect,n_scripts[custom_item_script]);
		} else {
			clConsole.send("WOLFPACK: ITEMLIST not found, aborting.\n");
			return;
		}
	} else strcpy(sect, "items.scp");

	unsigned long loopexit=0;
	do
	{
		read2();
		if (script1[0]!='}')
		{
			x=str2num(script1);
			pos=ftell(scpfile);
			closescript();//AntiChrist
			P_ITEM pi_j = Items->SpawnItemBackpack2(s, x, 0);
			if(pi_j == NULL) return;//AntiChrist to preview crashes
			openscript(sect);
			fseek(scpfile, pos, SEEK_SET);
			strcpy((char*)script1, "DUMMY");
			RefreshItem(pi_j);//AntiChrist
		}
	}
	while((strcmp((char*)script1,"}")) && (++loopexit < MAXLOOPS) );

	closescript();//AntiChrist
}

void criminal(P_CHAR pc)//Repsys ....Ripper
{
	if (pc == NULL)
		return;
	if ((pc->isPlayer())&&!(pc->isCriminal() || pc->isMurderer()))
	{//Not an npc, not grey, not red
		
		 pc->crimflag=(repsys.crimtime*MY_CLOCKS_PER_SEC)+uiCurrentTime;
		 //printw(" Seeting Crimflag to %d \n",chars[c].crimflag) ;
		 sysmessage(calcSocketFromChar(pc),"You are now a criminal!");
		 setcharflag(pc);
		 if(pc->inGuardedArea() && SrvParms->guardsactive)//guarded
			Combat->SpawnGuard( pc, pc, pc->pos.x,pc->pos.y,pc->pos.z); // LB bugfix
	}
}

void setcharflag(P_CHAR pc)// repsys ...Ripper
{

	//First, let's see their karma.
	if (pc->karma <= -200)
	{
		pc->setMurderer();
	}	
	if (pc->isPlayer())
	{
		if (pc->isGMorCounselor())
		{
			pc->setInnocent();
			return;
		}
		else if (pc->kills >= (unsigned) repsys.maxkills)
		{
			pc->setMurderer();
			return;
		}	
		else if (pc->crimflag==-1 || pc->crimflag == 0)
		{
			pc->setInnocent();
			return;
		}
		else if (pc->crimflag>0)
		{
			pc->setCriminal();
			return;
		}		
		else
		{
			pc->setCriminal();
		}
	}
	if (pc->isNpc() && ((pc->npcaitype == 2) || // bad npc
		(pc->npcaitype == 3) ||  // bad healer
		(pc->npcaitype == 50)))   // EV & BS
	{
		if (server_data.BadNpcsRed == 0)
		{
			pc->setCriminal();
		}
		else
		{
			pc->setMurderer();
		}
	}
	else
	{
		switch (pc->npcaitype)
		{
			case 2: // bad npcs
			case 3: // bad healers
			case 50: // EV & BS
				pc->setMurderer();
				break;
			case 1: // good healers
			case 4: // teleport guard
			case 5: // beggars
			case 6: // chaos guard
			case 7: // order guard
			case 8: // banker
			case 9: // town guard
			case 17: // player vendor
				pc->setInnocent();
				break;
			default:
				if (pc->isHuman())
				{
					pc->setInnocent();
					return;
				}
				if (server_data.animals_guarded == 1 && pc->npcaitype == 0 && !pc->tamed)
				{
					if (pc->inGuardedArea())	// in a guarded region, with guarded animals, animals == blue
						pc->setInnocent();
					else				// if the region's not guarded, they're gray
						pc->setCriminal();
				}
				else if (pc->ownserial>-1 && pc->tamed)
				{
					P_CHAR pc_owner = FindCharBySerial(pc->ownserial);
					if (pc_owner != NULL)
					{
						pc->flag = pc_owner->flag;
					}
				}
				else 
					pc->setCriminal();
				break;
		}
	}
}


//Written by AntiChrist - 4/11/1999
//
//This is very useful for bandwidth i think....i replaced all the
//stupid loops in the entire code sending a item to ALL the
//sockets...without checking if the socket's character was in
//range.....very baaad!
//
//I also added the inpack check and the worned check....
//
void RefreshItem(P_ITEM pi)//Send this item to all online people in range
{//check if item is in a pack or on the ground, then use different methods
	unsigned int a;
	signed int aa ;

	if(pi == NULL) return; //just to be on the right side

	if (pi->contserial==pi->serial)
	{
		clConsole.send("\nALERT ! item %s [serial: %i] has dangerous container value, autocorrecting\n",pi->name.c_str(),pi->serial);
		pi->SetContSerial(-1);
	}

	//first check: let's check if it's on the ground....
	if(pi->isInWorld())
	{//yeah, it's on ground!
		for(a=0;a<(unsigned)now;a++)//send this item to all the sockets in range
		{
			if(perm[a] && iteminrange(a,  pi, VISRANGE))
				senditem(a, pi);
		}
		return;
	}

	//if not, let's check if it's on a char or in a pack

	if (isCharSerial(pi->contserial))//container is a player...it means it's equipped on a character!
	{
		LongToCharPtr(pi->serial,wearitem+1);
		ShortToCharPtr(pi->id(),wearitem+5);
		wearitem[8]=pi->layer;
		LongToCharPtr(pi->contserial,wearitem+9);
		ShortToCharPtr(pi->color, wearitem+13);
		P_CHAR charcont = FindCharBySerial(pi->contserial);
		for(a=0;a<(unsigned)now;a++)//send this item to all the sockets in range
		{
			if(perm[a] && inrange1p(currchar[a], charcont))
				Xsend(a, wearitem, 15);
		}
		return;
	}
	else//container is an item...it means we have to use sendbpitem()!!
	{
		for(aa=0;aa<now;aa++)//send this item to all the sockets in range
		{
			if(perm[aa])
				sendbpitem(aa, pi);//NOTE: there's already the inrange check
								 //in the sendbpitem() function, so it's unuseful
								 //to do a double check!!
		}
		return;
	}
}

void SetGlobalVars()
{
	int i=0;
	clConsole.send("Initializing global variables...");

	w_anim[0]=0; w_anim[1]=0; w_anim[2]=0;

	for (i=0; i>ALLSKILLS; i++) { strcpy(title[i].other, "old titles.scp error"); }
	completetitle = new char[1024];
	for (i=0;i<(MAXCLIENT);i++) { LSD[i]=0; DRAGGED[i]=0; EVILDRAGG[i]=0; clientDimension[i]=2; noweather[i]=1; } // LB	
	for (i=0;i<301;i++) freecharmem[i]=-1;
	for (i=0;i<501;i++) freeitemmem[i]=-1;
	//for (i=0;i<cmem;i++) talkingto[i]=0; // cmem isnt set here !
	for (i=0;i<MAXLAYERS;i++) layers[i]=0;
	
	save_counter=0;
	cmem=0;
	uoxtimeout.tv_sec=0;
	uoxtimeout.tv_usec=0;
	keeprun=1;
	error=0;
	now=0;
	secure=1;
	wtype=0;
	cmemover=0;
	cmemcheck=-1;
	xcounter=0;
	ycounter=0;
	globallight=0;
	executebatch=0;
	showlayer=0;
	autosaved = 0;
	dosavewarning = 0;
	
	
	server_data.decaytimer  = DECAYTIMER ;
    server_data.invisibiliytimer = INVISTIMER ;
    server_data.hungerrate = HUNGERRATE ;
    server_data.skilldelay = SKILLDELAY ;
    server_data.hitpointrate = REGENRATE1 ;
    server_data.staminarate = REGENRATE2 ;
    server_data.manarate = REGENRATE3 ;
    server_data.gatetimer = GATETIMER;    
	        
}

void BuildPointerArray()
{
}

void InitMultis()
{
	P_ITEM pi_multi;

	AllCharsIterator iter_char;
	for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR pc = iter_char.GetData();
		if (!pc->free)
		{
			pi_multi = findmulti(pc->pos);
			if (pi_multi != NULL)
			{
				if (pi_multi->type==117)
					pc->SetMultiSerial(pi_multi->serial);
				else
					pc->multis = INVALID_SERIAL;
			}
		}
	}

	AllItemsIterator iter_items;
	for (iter_items.Begin(); !iter_items.atEnd(); iter_items++)
	{
		P_ITEM pi = iter_items.GetData();
		if (!pi->free && !pi->isInWorld())
		{
			pi_multi = findmulti(pi->pos);
			if (pi_multi != NULL)
				if (pi_multi != pi)
					pi->SetMultiSerial(pi_multi->serial);
				else 
					pi->multis = INVALID_SERIAL;
		}
	}
}

void InitServerSettings()
{
#if 0			// Duke, reducing startup verbosity
		clConsole.send(" -Archiving ");
	if (strlen(SrvParms->archivepath)>1) // Moved by Magius(CHE (1)
		clConsole.send("Enabled. (%s)\n",SrvParms->archivepath);
	else clConsole.send("Disabled!\n");
	clConsole.send(" -Weapons & Armour Rank System ");
	if (SrvParms->rank_system==1) clConsole.send("Activated!\n");
	else clConsole.send("Disabled!\n");
	clConsole.send(" -Vendors buy by item name ");
	if (SrvParms->sellbyname==1) clConsole.send("Activated!\n");
	else clConsole.send("Disabled!\n");
	clConsole.send(" -Adv. Trade System "); // Magius(CHE)
	if (SrvParms->trade_system==1) clConsole.send("Activated!\n");	// Magius(CHE)
	else clConsole.send("Disabled!\n");	// Magius(CHE)
	clConsole.send(" -Tamed Disappear "); // Ripper
	if (SrvParms->tamed_disappear==1) clConsole.send("Activated!\n");
	else clConsole.send("Disabled!\n");
	clConsole.send(" -HouseInTown "); // Ripper
	if (SrvParms->houseintown==1) clConsole.send("Activated!\n");
	else clConsole.send("Disabled!\n");
	clConsole.send(" -ShopRestock "); // Ripper
	if (SrvParms->shoprestock==1) clConsole.send("Activated!\n");
	else clConsole.send("Disabled!\n");

	clConsole.send(" -Special Bank stuff "); // AntiChrist
	if (SrvParms->usespecialbank==1) clConsole.send("Activated!\n");//AntiChrist - Special Bank
	else clConsole.send("Disabled!\n");//AntiChrist - Special Bank


	// Dupois - Added Dec 20, 1999
	clConsole.send( " -Escort Quest System " );
	if( SrvParms->escortactive == 1 ) clConsole.send( "Activated!\n" );
	else clConsole.send( "Disabled!\n" );
	if( strlen(SrvParms->msgboardpath) > 1 )
		clConsole.send( " -Bulletin Boards: Storing files in (%s).\n",SrvParms->msgboardpath );
	else
		clConsole.send( " -Bulletin Boards: Storing files in the current WOLFPACK.EXE directory.\n" );

	// Dupois - Added July 18, 2000
	clConsole.send( " -Bounty System " );
	if( SrvParms->bountysactive == 1 ) clConsole.send( "Activated!\n" );
	else clConsole.send( "Disabled!\n" );
#endif
	// Leave this as the last function to call before exiting
	MsgBoardMaintenance();
}

void StartClasses(void)
{
	clConsole.send("Initializing classes...");

// NULL Classes out first....
	cwmWorldState = NULL;
	mapRegions = NULL;
	Accounts=NULL;
	Admin=NULL;
	Boats=NULL;
	Combat=NULL;
	Commands=NULL;
	Guilds=NULL;
	Gumps=NULL;
	Items=NULL;
	Map=NULL;
	Npcs=NULL;
	Skills=NULL;
	Weight=NULL;
	Targ=NULL;
	Network=NULL;
	Magic=NULL;
	Books=NULL;
	Respawn=NULL;
	Movement = NULL;
	Weather=NULL;
	DragonAI=NULL;
	BankerAI=NULL;

	// Classes nulled now, lets get them set up :)
	cwmWorldState=new CWorldMain;
	mapRegions = new cRegion;
	Accounts = new cAccount;
	Admin = new cAdmin;
	Boats = new cBoat;
	Combat = new cCombat;
	Commands = new cCommands;
	Guilds=new cGuilds;
	Gumps = new cGump;
	Items = new cAllItems;
	Map = new cMapStuff;
	Npcs = new cCharStuff;
	Skills = new cSkills;
	Weight = new cWeight;
	Targ = new cTargets;
	Network = new cNetworkStuff;
	Magic = new cMagic;
	Books = new cBooks;
	Respawn = new cRespawn;
	AllTmpEff = new cAllTmpEff;
	Movement = new cMovement;
	//Weather = new cWeather;
	HouseManager=new cHouseManager;
	House.resize(0); 
	// Sky's AI Stuff
	DragonAI=new cCharStuff::cDragonAI;
	BankerAI=new cCharStuff::cBankerAI;
	clConsole.send(" Done\n");
}

void DeleteClasses(void)
{
	//Weather->kill();
	delete cwmWorldState;
	delete mapRegions;
	delete Accounts;
	delete Admin;
	delete Boats;
	delete Combat;
	delete Commands;
	delete Guilds;
	delete Gumps;
	delete Items;
	delete Map;
	delete Npcs;
	delete Skills;
	delete Weight;
	delete Targ;
	delete Network;
	delete Magic;
	delete Books;
	delete Respawn;
	delete Movement;
	delete DragonAI;
	delete BankerAI;
	//delete Weather;
}

// if we can find new effects they can be added here and will be active 
// for 'go 'goiter 'goplace 'whilst and 'tell for gm's and counselors 

////////////////////////////////// 
// Function for the different gm movement effects 
// 0 = none 
// 1 = flamestrike 
// 2 - 6 = different sparkles 
// Aldur
// 
// 

void doGmMoveEff(UOXSOCKET s)
{ 
	if (s == -1) // Just to make sure ;)
		return;
	
	P_CHAR pc_currchar = currchar[s];
	if (!(pc_currchar->priv2 & 0x08))
	{ 
		switch (pc_currchar->gmMoveEff)
		{
		case 1:
			// flamestrike 
			staticeffect3(pc_currchar->pos.x + 1, pc_currchar->pos.y + 1, pc_currchar->pos.z + 10, 0x37, 0x09, 0x09, 0x19, 0); 
			soundeffect(s, 0x02, 0x08); 
			break;
		case 2:
			// sparklie (fireworks wand style) 
			staticeffect3(pc_currchar->pos.x + 1, pc_currchar->pos.y + 1, pc_currchar->pos.z + 10, 0x37, 0x3A, 0x09, 0x19, 0); 
			break;
		case 3:
			// sparklie (fireworks wand style) 
			staticeffect3(pc_currchar->pos.x + 1, pc_currchar->pos.y + 1, pc_currchar->pos.z + 10, 0x37, 0x4A, 0x09, 0x19, 0); 
			break;
		case 4:
			// sparklie (fireworks wand style) 
			staticeffect3(pc_currchar->pos.x + 1, pc_currchar->pos.y + 1, pc_currchar->pos.z + 10, 0x37, 0x5A, 0x09, 0x19, 0); 
			break;
		case 5:
			// sparklie (fireworks wand style) 
			staticeffect3(pc_currchar->pos.x + 1, pc_currchar->pos.y + 1, pc_currchar->pos.z + 10, 0x37, 0x6A, 0x09, 0x19, 0); 
			break;
		case 6:
			// sparklie (fireworks wand style) 
			staticeffect3(pc_currchar->pos.x + 1, pc_currchar->pos.y + 1, pc_currchar->pos.z + 10, 0x37, 0x7A, 0x09, 0x19, 0); 
			break;
		}
	}
	return; 
}

