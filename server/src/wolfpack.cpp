//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

// Platform Includes
#ifndef __unix__
#include <conio.h>
#endif

#if defined(__unix__)
#include <signal.h>
#endif

#include "wolfpack.h"
#include "world.h"
#include "verinfo.h"
#include "speech.h"
#include "territories.h"

#include "books.h"
#include "TmpEff.h"
#include "guildstones.h"
#include "combat.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "network.h"
#include "exceptions.h"
#include "gumps.h"
#include "commands.h"
#include "spawnregions.h"
#include "Timing.h"
#include "tilecache.h"
#include "accounts.h"
#include "makemenus.h"
#include "skills.h"
#include "resources.h"
#include "contextmenu.h"
#include "maps.h"
#include "wpdefmanager.h"
#include "wpscriptmanager.h"
#include "wptargetrequests.h"
#include "python/engine.h"
#include "newmagic.h"
#include "spellbook.h"
#include "persistentbroker.h"
#include "corpse.h"
#include "house.h"
#include "boats.h"
#include "multiscache.h"
#include "Trade.h"
#include "network/uotxpackets.h"
#include "magic.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"

// Library Includes
#include <qapplication.h>
#include <qtranslator.h>
#include <qstring.h>
#include <qlibrary.h>
#include <qdatetime.h>
#include <zthread/Thread.h>
#include <zthread/FastMutex.h>
#include <fstream>

using namespace std;

#undef DBGFILE
#define DBGFILE "wolfpack.cpp"
#include "debug.h"

///////////
// Name:	inVisRange
// Purpose:	checks if position 1 and 2 are in visual range
// history:	by Duke, 18.11.2000
//
bool inVisRange(int x1, int y1, int x2, int y2)
{
	return (QMAX(abs(x1-x2), abs(y1-y2)) <= VISRANGE);
}

bool inrange1p (PC_CHAR pca, P_CHAR pcb) // Are characters a and b in visual range
{
	if (pca == NULL || pcb == NULL) return false;

	return inVisRange(pca->pos().x, pca->pos().y, pcb->pos().x, pcb->pos().y);
}

///////////
// Name:	inRange
// Purpose:	checks if position 1 and 2 are in given range
// history:	by Duke, 19.11.2000
//
inline bool inRange(int x1, int y1, int x2, int y2, int range)
{
	return (QMAX(abs(x1-x2), abs(y1-y2)) <= range);
}

//================================================================================
//
// signal handlers
#if defined( Q_OS_UNIX )
void signal_handler(int signal)
{
	cCharIterator iter;

	switch (signal)
	{
	case SIGHUP:
                SrvParams->reload();
                cNetwork::instance()->reload();
                DefManager->reload();
				Accounts::instance()->reload();
                SpawnRegions::instance()->reload();
                AllTerritories::instance()->reload();
                Resources::instance()->reload();
                MakeMenus::instance()->reload();
                ScriptManager->reload();
				ContextMenus::instance()->reload();
				NewMagic->load();
				Skills->reload();

				// Update the Regions
				for( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
				{
					cTerritory *region = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
					pChar->setRegion( region );
				}
		break ;
		
	case SIGUSR1:
		Accounts::instance()->reload();
		break ;
	case SIGUSR2:
		World::instance()->save();
		SrvParams->flush();
		break ;
	case SIGTERM:
		keeprun = 0 ;
		break;
	default:
		break;
	}
}
	
#endif

void savelog(const char *msg, char *logfile)
{
		FILE *file;
		file = fopen( logfile, "a" );
		
		QString logMessage = QString( "[%1] %2\n" ).arg( QDateTime::currentDateTime().toString() ).arg( msg );

		// Remove newlines
		logMessage = logMessage.replace( QRegExp( "\n" ), "" );

		fprintf( file, "%s", logMessage.ascii() );

#ifdef DEBUG
		clConsole.send("DEBUG: Logging to %s\n", logfile);
#endif

		fclose( file );
}

int DeleBankItem( P_PLAYER pc, unsigned short itemid, unsigned short color, int amt )
{
	if( pc == NULL )
		return amt;

	P_ITEM pBank = pc->getBankBox();

	if( pBank )
		return pBank->DeleteAmount( amt, itemid, color );
	else
		return 0;
}

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
			pi->soundEffect( OPENWOOD );

		else if (((x>=0x0839)&&(x<=0x0848))|| // Open gate
			((x>=0x084C)&&(x<=0x085B))||
			((x>=0x0866)&&(x<=0x0875)))
			pi->soundEffect( OPENGATE );

		else if (((x>=0x0675)&&(x<0x0695))|| // Open metal
			((x>=0x06C5)&&(x<0x06D5)))
			pi->soundEffect( OPENSTEEL );

		else if ((x>=0x0314)&&(x<=0x0365)) // Open secret
			pi->soundEffect( OPENSECRET );
	}
	else if (y==1) // Request close door sfx
	{
		if (((x>=0x0695)&&(x<0x06C5))|| // close wooden / ratan door
			((x>=0x06D5)&&(x<=0x06F4)))
			pi->soundEffect( CLOSEWOOD );

		else if (((x>=0x0839)&&(x<=0x0848))|| // close gate
			((x>=0x084C)&&(x<=0x085B))||
			((x>=0x0866)&&(x<=0x0875)))
			pi->soundEffect( CLOSEGATE );

		else if (((x>=0x0675)&&(x<0x0695))|| // close metal
			((x>=0x06C5)&&(x<0x06D5)))
			pi->soundEffect( CLOSESTEEL );

		else if ((x>=0x0314)&&(x<=0x0365)) // close secret
			pi->soundEffect( CLOSESECRET );
	}
}

void dooruse( cUOSocket *socket, P_ITEM pi )
{
	if( !pi )
		return;
#pragma fixme("dunno how to replace the old tempeffect2, commented out but this introduces a bug here!! please review.")
	if( socket && socket->player() && !socket->player()->inRange( pi, 2 ) )
	{
		socket->sysMessage( tr( "You cannot reach the handle from here." ) );
		return;
	}

	UINT16 x = pi->id();
	UINT16 db;

	for( UINT32 i = 0; i < DOORTYPES; ++i )
	{
		db = doorbase[i];

		if( x == ( db + 0 ) )
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.x--;
			pos.y++;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if( x == ( db + 1 ) )
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.x++;
			pos.y--;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if (x==(db+2))
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.x++;
			pos.y++;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if (x==(db+3))
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.x--;
			pos.y--;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if (x==(db+4))
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.x--;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if (x==(db+5))
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.x++;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if (x==(db+6))
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.x++;
			pos.y--;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if (x==(db+7))
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.x--;
			pos.y++;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if (x==(db+8))
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.x++;
			pos.y++;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if (x==(db+9))
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.x--;
			pos.y--;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if (x==(db+10))
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.x++;
			pos.y--;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if (x==(db+11))
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.x--;
			pos.y++;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if (x==(db+12))
		{
			pi->setId( pi->id() + 1 );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if (x==(db+13))
		{
			pi->setId( pi->id() - 1 );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}
		else if( x == ( db + 14 ) )
		{
			pi->setId( pi->id() + 1 );
			Coord_cl pos( pi->pos() );
			pos.y--;
			pi->moveTo( pos );
			doorsfx(pi, x, 0);
//			tempeffect2(0, pi, 13, 0, 0, 0);
		}
		else if( x == ( db + 15 ) )
		{
			pi->setId( pi->id() - 1 );
			Coord_cl pos( pi->pos() );
			pos.y++;
			pi->moveTo( pos );
			doorsfx(pi, x, 1);
			pi->setDoorOpen(0);
		}

		// This is *so* obvious...
		if( x != pi->id() )
		{
			pi->update();
			break;
		}
	}
	
	if( socket && ( x != pi->id() ) )
	{
		// house refreshment when a house owner or friend of a houe opens the house door
		float ds = 0;
		P_CHAR pChar = socket->player();

		if( !pChar )
			return;
		
		cHouse* pHouse = dynamic_cast<cHouse*>( cMulti::findMulti( pi->pos() ) );
		if( !pHouse )
			return;

		if( !( pChar->Owns( pHouse ) || pHouse->isFriend( pChar ) ) )
			return;

		if( SrvParams->housedecay_secs() != 0 )
			ds = static_cast<float>((pHouse->time_unused)*100) / (SrvParams->housedecay_secs());
		else
			ds = -1;

		if( ds >= 50 ) // sysmessage if decay status >= 50%
		{
			socket->sysMessage( tr( "You refreshed the house" ) );
		}
		pHouse->time_unused = 0;
		pHouse->last_used = getNormalizedTime();
	}
}

void endmessage(int x) // If shutdown is initialized
{
	x = 0;
	unsigned int igetclock = uiCurrentTime;
	if (endtime < igetclock)
		endtime = igetclock;

	QString message;

	if( ( ( ( endtime - igetclock ) / MY_CLOCKS_PER_SEC ) / 60 ) < 1 ) 	
		message = tr( "Server going down in %1 secs." ).arg( ( endtime - igetclock ) / MY_CLOCKS_PER_SEC );
	else
		message = tr( "Server going down in %1 minutes." ).arg( ( ( endtime - igetclock ) / MY_CLOCKS_PER_SEC ) / 60 );
	
	cNetwork::instance()->broadcast( message );
	clConsole.send( message );
}

void reloadScripts()
{
	clConsole.send( "Reloading definitions, scripts and wolfpack.xml\n" );

	SrvParams->reload(); // Reload wolfpack.xml
	DefManager->reload(); //Reload Definitions
	Accounts::instance()->reload();
	SpawnRegions::instance()->reload();
	AllTerritories::instance()->reload();
	Resources::instance()->reload();
	MakeMenus::instance()->reload();
	ScriptManager->reload(); // Reload Scripts
	ContextMenus::instance()->reload();
	NewMagic->load();
	Skills->reload();

	// Update the Regions
	cCharIterator iter;
	for( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
	{
		cTerritory *region = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
		pChar->setRegion( region );
	}

	cNetwork::instance()->reload(); // This will be integrated into the normal definition system soon
}


ZThread::FastMutex commandMutex;
QStringList commandQueue;

class cConsoleThread: public ZThread::Thread
{
public:
	~cConsoleThread() throw() {
		join(); // wait for it to stop
	}

	virtual void run() throw()
	{
		#if !defined(__unix__)
		// Check for a new key constantly and put it into the command-queue
		char key = 0;

		try
		{
			
			while( keeprun )
			{
				if ( kbhit() )
				{
					key = getch();
	
					if( key != 0 )
					{
						try {
							commandMutex.acquire();
							commandQueue.push_back( QString( "%1" ).arg( key ) );
							commandMutex.release();
						} catch ( ZThread::Synchronization_Exception &e ) {
							commandMutex.release();
						}
					}
				}
				this->sleep(10);
			}			
		}
		catch( ... )
		{
//			commandMutex.release();
		}
		#endif
	}
};

// This function is used to interpret a command
// sent by the console.
void interpretCommand( const QString &command )
{
	cUOSocket *mSock;
	int i;
	char c = command.latin1()[0];

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
				World::instance()->save();
				
				SrvParams->flush();
				break;
			case 'D':	// Disconnect account 0 (useful when client crashes)
			case 'd':	
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
				clConsole.send( "Current Users in the World:\n" );

				mSock = cNetwork::instance()->first();
				i = 0;
				
				for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
				{
					if( mSock->player() )
					{
						clConsole.send( "%i) %s [%x]\n", ++i, mSock->player()->name().latin1(), mSock->player()->serial() );
					}
				}

				clConsole.send( "Total Users Online: %d\n", cNetwork::instance()->count() );
				break;
			case 'A': //reload the accounts file
			case 'a':
				Accounts::instance()->reload();
				break;
			case 'r':
			case 'R':
				reloadScripts();

				break;
			case '?':
				clConsole.send("Console commands:\n");
				clConsole.send("	<Esc> or Q: Shutdown the server.\n");
				clConsole.send("	T - System Message: The server is shutting down in 10 minutes.\n");
				clConsole.send("	# - Save world\n");
				clConsole.send("	D - Disconnect Account 0\n");
				clConsole.send("	1 - Sysmessage: Attention Players Server being brought down!\n");
				clConsole.send("	2 - Broadcast Message 2\n");
				clConsole.send("	P - Preformance Dump\n");
				clConsole.send("	W - Display logged in characters\n");
				clConsole.send("	A - Reload accounts\n");
				clConsole.send("	R - Reload scripts\n");
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

#include <qfile.h>

static void parseParameter( const QString &param )
{
	// Add what ever paramters you want
	// Right now we don't have any parameters
	if ( param.contains(".py") )
	{
		if ( QFile::exists( param ) )
		{
			FILE* f = fopen(param, "r");
			if ( f )
			{
				PyRun_SimpleFile(f, const_cast<char*>(param.latin1()));
				exit(0);
			}
		}
		else
			clConsole.error( QString("The specified python script [%1] doesn't exist.").arg(param) );
	}
}

/*!
	Initializes global objects.
*/
static void startClasses()
{
	Items			 = 0;
	Map				 = 0;
	Skills			 = 0;
	Magic			 = 0;
	ScriptManager	 = 0;
	DefManager		 = 0;
	SrvParams		 = 0;
	NewMagic		 = 0;
	persistentBroker = 0;

	SrvParams		 = new cSrvParams( "wolfpack.xml", "Wolfpack", "1.0" );
	Items			 = new cAllItems;
	Map				 = new Maps ( SrvParams->mulPath() );
	Skills			 = new cSkills;
	Magic			 = new cMagic;
	ScriptManager	 = new WPScriptManager;
	DefManager		 = new WPDefManager;
	NewMagic		 = new cNewMagic;
	persistentBroker = new PersistentBroker;
}

/*!
	Deletes global objects.
*/
static void freeClasses( void )
{
	delete SrvParams;
	delete Items;
	delete Map;
	delete Skills;
	delete Magic;
	delete ScriptManager;
	delete DefManager;
	delete NewMagic;
}

void SetGlobalVars()
{
	keeprun=1;
	error=0;
	secure=1;
	autosaved = 0;
	dosavewarning = 0;
}

static void InitMultis()
{
	cItemIterator iter_items;
	P_ITEM pItem;

	for( pItem = iter_items.first(); pItem; pItem = iter_items.next() )
	{
		cMulti *pMulti = dynamic_cast< cMulti* >( pItem );
		if( pMulti )
		{
			pMulti->checkChars();
			pMulti->checkItems();
		}
	}
}

//#if defined(_DEBUG)
//#include <crash.h>
//#endif

/*!
	Main server entry point.
*/
int main( int argc, char *argv[] )
{
/*#if defined(_DEBUG)
	InstallCrashHandler( HANDLE_HTTPREPORT, GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE );
	SetCustomMessage("A crash occurred. Please send this bug report to developers\n");
	SetVersion(wp_version.verstring.c_str());
#endif
*/
	QApplication app( argc, argv, false ); // we need one instance
	QTranslator translator( 0 ); // must be valid thru app life.

	// Unix Signal Handling
#if defined( Q_OS_UNIX )	
	signal( SIGHUP,  &signal_handler ); // Reload Scripts
	signal( SIGUSR1, &signal_handler ); // Save World
	signal( SIGUSR2, &signal_handler ); // Reload Accounts
	signal( SIGTERM, &signal_handler ); // Terminate Server
	signal( SIGPIPE, SIG_IGN );			// Ignore SIGPIPE
#endif	

	#define CIAO_IF_ERROR if (error==1) { cNetwork::instance()->shutdown(); exit( -1 ); }

	unsigned long tempSecs;
	unsigned long loopSecs;
	unsigned long tempTime;
	uiCurrentTime = serverstarttime = getNormalizedTime();

	serverState = STARTUP;
	// Print a seperator somehow
	clConsole.send( QString( "\n%1 %2 %3 \n\n" ).arg( wp_version.productstring.c_str() ).arg( wp_version.betareleasestring.c_str() ).arg( wp_version.verstring.c_str() ) );

	clConsole.send( "Copyright (C) 1997, 98 Marcus Rating (Cironian)\n");
	clConsole.send( "Copyright (C) 2000-2003 Wolfpack Development Team\n");
	clConsole.send( "Wolfpack Homepage: http://www.wpdev.org/\n");
	clConsole.send( "By using this software you agree to the license accompanying this release.\n");
	clConsole.send( "Compiled on %s (%s %s)\n",__DATE__,__TIME__, wp_version.timezonestring.c_str() );
	clConsole.send( "\n" );
	
	QString consoleTitle = QString( "%1 %2 %3" ).arg( wp_version.productstring.c_str() ).arg( wp_version.betareleasestring.c_str() ).arg( wp_version.verstring.c_str() );
	clConsole.setConsoleTitle( consoleTitle );

	// Startup normal Classes
	try
	{
		atexit( freeClasses );
		startClasses();
	}
	catch( ... )
	{
		clConsole.log( LOG_FATAL, "Couldn't start up classes." );
		exit( -1 );
	}

	// Startup Translator
	try
	{
		QString languageFile = SrvParams->getString( "General", "Language File", "", true );
		if ( !languageFile.isEmpty() )
		{
			translator.load( languageFile, "." );
			qApp->installTranslator( &translator );
		}
	}
	catch( ... )
	{
		clConsole.log( LOG_FATAL, "Couldn't load translator." );
		exit( -1 );
	}

	// Try to start up python
	try
	{
		startPython( argc, argv );
	}
	catch( ... )
	{
		clConsole.log( LOG_FATAL, "Couldn't start up python." );
		exit( -1 );
	}

	unsigned int i;

	for( i = 1; i <= argc; ++i )
		parseParameter( QString( argv[ i ] ) );


	// Load data
	DefManager->load();
	clConsole.send( "\n" );

	// Scriptmanager can't be in the try{} block because it sometimes throws firstchance exceptions
	// we don't like
	ScriptManager->load();
	clConsole.send( "\n" );

	// Try to load several data files
	try
	{
		clConsole.send( "Loading skills...\n" );
		Skills->load();

		clConsole.send( "Loading accounts...\n" );
		Accounts::instance()->load();

		clConsole.send( "Loading spells...\n" );
		Magic->load();
		NewMagic->load();

		clConsole.send( "Loading ip blocking rules...\n" );
		cNetwork::instance()->load();

		clConsole.send( "Loading regions...\n" );
		AllTerritories::instance()->load();

		clConsole.send( "Loading spawn regions...\n" );
		SpawnRegions::instance()->load();

		clConsole.send( "Loading resources...\n" );
		Resources::instance()->load();

		clConsole.send( "Loading makemenus...\n" );
		MakeMenus::instance()->load();

		clConsole.send( "Loading contextmenus...\n" );
		ContextMenus::instance()->reload();

		// Load some MUL Data
		clConsole.send( "Loading muls...\n" );
		TileCache::instance()->load( SrvParams->mulPath() );
		MultiCache::instance()->load( SrvParams->mulPath() );

		Map->registerMap(0, "map0.mul", 768, 512, "statics0.mul", "staidx0.mul");
		Map->registerMap(1, "map0.mul", 768, 512, "statics0.mul", "staidx0.mul");
		Map->registerMap(2, "map2.mul", 288, 200, "statics2.mul", "staidx2.mul");	
		Map->registerMap(3, "map3.mul", 320, 256, "statics3.mul", "staidx3.mul");	

		clConsole.send( "\n" );
	}
	catch( wpException &exception )
	{
		clConsole.log( LOG_FATAL, exception.error() );
		exit( -1 );
	}
	catch( ... )
	{
		clConsole.log( LOG_FATAL, "Unknown error while loading data files." );
		exit( -1 );
	}


	SetGlobalVars();
	keeprun = 1;

	// initial randomization call
	srand( uiCurrentTime );
	serverstarttime = getNormalizedTime();

	// Try to open our driver
	if( !persistentBroker->openDriver( "mysql" ) )
	{
		exit( -1 );
	}

	// Don't connect if everything is empty
	if( !SrvParams->databaseHost().isEmpty() || !SrvParams->databaseName().isEmpty() || !SrvParams->databaseUsername().isEmpty() || !SrvParams->databasePassword().isEmpty() )
	{
		try
		{
			persistentBroker->connect( SrvParams->databaseHost(), SrvParams->databaseName(), SrvParams->databaseUsername(), SrvParams->databasePassword() );
		}
		catch( QString &e )
		{
			clConsole.log( LOG_FATAL, QString( "An error occured while connecting to the database: %1\n" ).arg( e ) );
			exit( -1 );
		}
	}

	// Registers our Built-in types into factory.
	cPlayer::registerInFactory();
	cNPC::registerInFactory();
	cItem::registerInFactory();
	cBook::registerInFactory();
	cSpellBook::registerInFactory();
	cCorpse::registerInFactory();
	cBoat::registerInFactory();
	cHouse::registerInFactory();
	cResourceItem::registerInFactory();

	try
	{
		World::instance()->load();
	}
	catch( QString &error )
	{
		clConsole.log( LOG_FATAL, error );
		exit( -1 );
	}
	catch( ... )
	{
		clConsole.log( LOG_FATAL, "An unknown error occured while loading the world." );
		exit( -1 );
	}

	clConsole.PrepareProgress( "Initializing Multis" );
	InitMultis();
	clConsole.ProgressDone();

	starttime = uiCurrentTime;
	endtime = 0;
	lclock = 0;

	init_creatures(); // This initializes *fixed* data that should DEFINETLY be swaped out to the scripts !!!
	
	if( SrvParams->serverLog() )
		savelog( "Server startup", "server.log" );

	uiCurrentTime = getNormalizedTime();
	serverstarttime = getNormalizedTime(); // dont remove, its absolutly necassairy that its 3 times in the startup sequence for several timing reasons.

	CIAO_IF_ERROR;

    // print allowed clients	
	clConsole.send( "Allowed clients: " );

	if( SrvParams->clientsAllowed().contains( "ALL" ) )
		clConsole.send( "All\n\n" );
	else
		clConsole.send( SrvParams->clientsAllowed().join( ", " ) + "\n\n" );

	clConsole.PrepareProgress( "Starting up Network" );
	cNetwork::startup();
	clConsole.ProgressDone();
	CIAO_IF_ERROR;

	if( SrvParams->enableLogin() )
        clConsole.send( QString( "LoginServer running on port %1\n" ).arg( SrvParams->loginPort() ) );

	if( SrvParams->enableGame() )
        clConsole.send( QString( "GameServer running on port %1\n" ).arg( SrvParams->gamePort() ) );

	PyThreadState *_save;

	ScriptManager->onServerStart();

	// Start the Console Input thread
	cConsoleThread consoleThread;
	consoleThread.start();

	serverState = RUNNING;

	// This is our main loop
	while( keeprun )
	{		
		// Python threading - start
		_save = PyEval_SaveThread();

		switch( SrvParams->niceLevel() )
		{
			case 0: break;	// very unnice - hog all cpu time
			case 1: if ( cNetwork::instance()->count() != 0) ZThread::Thread::sleep(10); else ZThread::Thread::sleep(100); break;
			case 2: ZThread::Thread::sleep(10); break;
			case 3: ZThread::Thread::sleep(40); break;// very nice
			case 4: if ( cNetwork::instance()->count() != 0 ) ZThread::Thread::sleep(10); else ZThread::Thread::sleep(4000); break; // anti busy waiting
			case 5: if ( cNetwork::instance()->count() != 0 ) ZThread::Thread::sleep(40); else ZThread::Thread::sleep(5000); break;
			default: ZThread::Thread::sleep(10); break;
		}

		// Python threading - end
		PyEval_RestoreThread( _save );

		// It's more likely that we have a new key-press now
		// Checking every 25 loops should be enough.
		#if !defined( __unix__ )
		if( loopTimeCount % 25 == 0 )
		{
			commandMutex.acquire();
			if( commandQueue.count() > 0 )
			{				
				// Interpret Command
				interpretCommand( commandQueue[0] );
				commandQueue.erase( commandQueue.begin() );
			}
			commandMutex.release();
		}
		#endif

		if( loopTimeCount >= 1000 )
		{
			loopTimeCount = 0;
			loopTime = 0;
		}

		++loopTimeCount;

		loopSecs = getNormalizedTime();  // Starting time

		if( networkTimeCount >= 1000 )
		{
			networkTimeCount = 0;
			networkTime = 0;
		}

		tempSecs = getNormalizedTime();

		// Process any Network Events
		cNetwork::instance()->poll();

		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
			P_PLAYER player = mSock->player();
			if ( player && !player->isGM() && player->clientIdleTime() && player->clientIdleTime() < uiCurrentTime )
			{
				clConsole.send( tr("Player %1 disconnected due to inactivity !\n").arg( player->name() ) );
				cUOTxMessageWarning packet;
				packet.setReason( cUOTxMessageWarning::Idle );
				mSock->send( &packet );
				mSock->disconnect();
			}
		}

		tempTime = getNormalizedTime() - tempSecs ;
		networkTime += tempTime;
		++networkTimeCount;

		if( timerTimeCount >= 1000 )
		{
			timerTimeCount = 0;
			timerTime = 0;
		}

		tempSecs = getNormalizedTime() ;		

		checktimers();

		uiCurrentTime = getNormalizedTime();
		tempTime = getNormalizedTime() - tempSecs;
		timerTime += tempTime;
		++timerTimeCount;

		if( autoTimeCount >= 1000 )
		{
			autoTimeCount = 0;
			autoTime = 0;
		}
		
		tempSecs = getNormalizedTime() ;		

		checkauto();

		tempTime = getNormalizedTime() - tempSecs;
		autoTime += tempTime;
		++autoTimeCount;

		tempTime = getNormalizedTime() - loopSecs;

		loopTime += tempTime;
		
		qApp->processEvents( 40 );
	}

	serverState = SHUTDOWN;

	consoleThread.cancel();

	ScriptManager->onServerStop();

	cNetwork::instance()->broadcast( tr( "The server is shutting down." ) );

	clConsole.PrepareProgress( tr( "Shutting down network" ) );
	cNetwork::shutdown();
	clConsole.ProgressDone();

	SrvParams->flush(); // Save config options

	// Simply emptying of containers, no progressbar needed
	Magic->unload();
	NewMagic->unload();
	DefManager->unload();

	// Stop Python Interpreter.
	ScriptManager->unload();
	stopPython();

	if( SrvParams->serverLog() ) 
		savelog( "Server shutdown","server.log" );

	return 0;
}

bool ishuman(P_CHAR pc)
{
	// Check if the Player or Npc is human! -- by Magius(CHE)
	if (pc->orgBodyID()==0x0190 || pc->orgBodyID()==0x0191) return true;
	else return false;
}

int chardir(P_CHAR a, P_CHAR b)	// direction from character a to char b
{
	int dir,xdif,ydif;

	xdif = b->pos().x - a->pos().x;
	ydif = b->pos().y - a->pos().y;

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

	xdif = x-pc->pos().x;
	ydif = y-pc->pos().y;

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
		switch(pc->direction()) //crashfix, LB
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

//   numtostr(abs(xH),xHs);
//   numtostr(abs(xM),xMs);
//   numtostr(abs(yH),yHs);
//   numtostr(abs(yM),yMs);

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

// streamlined by Duke 01.06.2000
int getsubamount(int serial, short id)
{
	unsigned long total=0;
	unsigned int ci;
	P_ITEM pi;
	P_ITEM pContainer = FindItemBySerial(serial);
	cItem::ContainerContent container(pContainer->content());
	cItem::ContainerContent::const_iterator it (container.begin());
	cItem::ContainerContent::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		pi = *it;

		if( pi->id() == id )
			total += pi->amount();

		if( pi->type() == 1 ) 
			total += getsubamount( pi->serial(), id );
	}
	return total;
}

int getamount(P_CHAR pc, short id)
{
	if (pc == NULL)
		return 0;
	P_ITEM pi=pc->getBackpack();
	if (pi==NULL)
		return 0;
	else
		return getsubamount(pi->serial(), id);
}

// not_deleted = output parameter, returns number of items that could NOT be deleted
// somewhat dirty that it defaults to a global dummy variable, sorry for that.
// but i couldnt find any other way to keep the old signature and old name.
// if theres a cleaner way, let me know, LB

void delequan(P_CHAR pc, short id, int amount, int *not_deleted)
{
	if ( pc == NULL )
		return;

	P_ITEM pi=pc->getBackpack();
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

void goldsfx( cUOSocket *socket, UINT16 amount, bool hearall )
{
	if( !socket || !socket->player() )
		return;

	UINT16 sEffect = 0x37;

	if( amount == 1 )
		sEffect = 0x35;
	else if( amount > 1 && amount < 6 )
		sEffect = 0x36;

	if( !hearall )
		socket->soundEffect( sEffect );
	else
	{
		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
			if( mSock->inRange( socket ) )
				mSock->soundEffect( sEffect );
	}
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
		basesound += offset;
		if (offset!=-1)
			monster->soundEffect( basesound );
		return;
	}
}

void addgold(cUOSocket* socket, int totgold)
{
	Items->SpawnItem(socket->player(), totgold,"#",true,0x0EED,0,1);
}

int calcValue(P_ITEM pi, int value)
{
	int mod=10;
	if (pi == NULL)
		return value;

	if( pi->type() == 19 )
	{
		if (pi->morex()>500) 
			mod	=mod+1;
		if (pi->morex()>900) mod=mod+1;
		if (pi->morex()>1000) mod=mod+1;
		if (pi->morez()>1) mod=mod+(3*(pi->morez()-1));
		value=(value*mod)/10;
	}

	// Lines added for Rank System by Magius(CHE)
	if (pi->rank()>0 && pi->rank()<10 && SrvParams->rank_system()==1)
	{
		value=(int) (pi->rank()*value)/10;
	}
	if (value<1) value=1;
	// end addon

	// Lines added for Trade System by Magius(CHE) (2)
	if (pi->rndvaluerate()<0) pi->setRndValueRate(0);
	if (pi->rndvaluerate()!=0 && SrvParams->trade_system()==1) {
		value+=(int) (value*pi->rndvaluerate())/1000;
	}
	if (value<1) value=1;
	// end addon

	return value;
}

int calcGoodValue(P_CHAR npcnum2, P_ITEM pi, int value,int goodtype)
{ // Function Created by Magius(CHE) for trade System
	cTerritory* Region = AllTerritories::instance()->region( npcnum2->pos().x, npcnum2->pos().y, npcnum2->pos().map );

	int regvalue=0;
	int x;
	if( pi == NULL || Region == NULL )
		return value;

	int good=pi->good();

	if (good<=-1 || good >255 ) return value;

	if (goodtype==1) regvalue=Region->tradesystem_[good].sellable;	// Vendor SELL
	if (goodtype==0) regvalue=Region->tradesystem_[good].buyable;	// Vendor BUY

	x=(int) (value*abs(regvalue))/1000;

	if (regvalue<0)	value-=x;
	else value+=x;

	if (value<=0) value=1; // Added by Magius(CHE) (2)

	return value;
}

void StoreItemRandomValue(P_ITEM pi,QString tmpreg)
{ // Function Created by Magius(CHE) for trade System
	int max=0,min=0;

	if( pi == NULL )
		return;
	if (pi->good()<0) return;

	if (tmpreg == "none" )
	{
		P_ITEM pio = pi->getOutmostItem();
		if (!pio) return;
		cTerritory* Region;
		if (pio->isInWorld())
		{
			Region = AllTerritories::instance()->region( pio->pos().x, pio->pos().y, pio->pos().map );
			if( Region != NULL )
				tmpreg = Region->name();
		}
		else
		{
			P_CHAR pc = dynamic_cast<P_CHAR>(pio->container());
			if (!pc) return;
			Region = AllTerritories::instance()->region( pc->pos().x, pc->pos().y, pc->pos().map );
			if( Region != NULL )
				tmpreg = Region->name();
		}
		return;
	}

	if( pi->good() < 0 || pi->good() > 255 ) 
		return;

	cTerritory* Region = AllTerritories::instance()->region( tmpreg );

	min=Region->tradesystem_[pi->good()].rndmin;
	max=Region->tradesystem_[pi->good()].rndmax;

	if (max!=0 || min!=0)
	{
		pi->setRndValueRate( RandomNum(min,max) );
	}
}


void init_creatures(void) // assigns the basesound, soundflag, who_am_i flag of all npcs
// LB 9.8.99/17.9/20.9

// completed: LB 27.3.2002 (covering ALL monsters LBR, T2A, Third dawn + old ones)

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
// type (disjunct) it belongs to: 
//
//            [ #  4: anything else
//              # 3: NPC ]  ( only 0,1,2 set there )
//
//              # 2: animal / pet (mounts too, even if rideable dragon)
//              # 1: sea creature
//              # 0: monster
//             			
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
	creatures[0x05].type=2;

	creatures[0x06].basesound=0x007d;							// Bird
	creatures[0x06].who_am_i+=1; //set fly bit
	creatures[0x06].soundflag=1; // birds need special treatment cause there are about 20 bird-sounds
	creatures[0x06].icon=8430;
	creatures[0x06].type=2;

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

	creatures[0x0b].basesound = 387;                            // dread spider
	creatures[0x0b].icon=  8445;

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

	creatures[0x13].basesound = 387;                            // dread spider 2
	creatures[0x13].icon=8445;

	creatures[0x14].basesound = 904;                            // frost spider
	creatures[0x14].icon=8445;

	creatures[0x15].basesound=219;								// Giant snake
	creatures[0x15].icon=8446;

	creatures[0x16].basesound=377;								// gazer
	creatures[0x16].icon=8426;

	creatures[0x17].basesound=229;								// dire wolf
	creatures[0x17].icon=8476;
	creatures[0x17].type=2;

	creatures[0x18].basesound=412;								// liche
	creatures[0x18].icon=8440; // counldnt find a better one :(

	creatures[0x19].basesound=229;								// grey wolf
    creatures[0x19].icon=8476;
	creatures[0x19].type=2;

	creatures[0x1a].basesound=382;								// ghost 1
	creatures[0x1a].icon=8457;

	creatures[0x1b].basesound=229;								// grey wolf
    creatures[0x1b].icon=8476;
	creatures[0x1b].type=2;

	creatures[0x1c].basesound=387;								// giant spider
	creatures[0x1c].icon=8445;

	creatures[0x1d].basesound=158;								// gorialla
	creatures[0x1d].icon=8437;
	creatures[0x1d].type=2;

	creatures[0x1e].basesound=402;								// harpy
	creatures[0x1e].icon=8412;

	creatures[0x1f].basesound=407;								// headless
	creatures[0x1f].icon=8458;

	// 0x20 is *really* missing, crashes (2d) client if instanciated

	creatures[0x21].basesound=417;								// lizardman
	creatures[0x23].basesound=417;
	creatures[0x24].basesound=417;

	creatures[0x21].icon=creatures[0x23].icon=
	creatures[0x24].icon=8414;

	creatures[0x22].basesound=creatures[0x25].basesound=229;    // wolves
	creatures[0x22].icon=creatures[0x25].icon=8476;
	creatures[0x22].type=creatures[0x25].type=2;


	creatures[0x26].basesound=696;								// black daemon
	creatures[0x26].icon=9605;
	creatures[0x26].who_am_i+=1; // set fly bit

	creatures[0x27].basesound=422;								// mongbat
	creatures[0x27].who_am_i+=1; // yes, they can fly
	creatures[0x27].icon=8441;

	creatures[0x28].basesound=696;								// elder daemon
	creatures[0x28].icon=9606;
	creatures[0x28].who_am_i+=1;

	creatures[0x29].basesound=0x01b0;							// orc 3
	creatures[0x29].icon=8416;

	creatures[0x2a].basesound=437;								// ratman
	creatures[0x2c].basesound=437;
	creatures[0x2d].basesound=437;
	creatures[0x2a].icon=creatures[0x2c].icon=
	creatures[0x2d].icon=8419;

	creatures[0x2b].basesound=696;								// ice daemon
	creatures[0x2b].icon=9607;
	creatures[0x2b].who_am_i+=1;
	
	creatures[0x2e].basesound=362;								// ancient worm
	creatures[0x2e].icon=8452;
	creatures[0x2e].who_am_i+=1;

	creatures[0x2f].basesound=0x01ba;							// Reaper
	creatures[0x2f].icon=8442;

	creatures[0x30].basesound=397;								// giant scorprion
	creatures[0x30].icon=8420;

	creatures[0x31].basesound=362;								// ancient worm white
	creatures[0x31].icon=8452;
	creatures[0x31].who_am_i+=1;

	creatures[0x32].basesound=452;								// skeleton 2
	creatures[0x32].icon=8423;

	creatures[0x33].basesound=456;								// slime
	creatures[0x33].icon=8424;

	creatures[0x34].basesound=219;								// Snake
	creatures[0x34].icon=8444;
	creatures[0x34].type=2;

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
	creatures[0x3e].basesound=362;								// wyvern
	creatures[0x3e].who_am_i+=1;
	creatures[0x3b].icon=creatures[0x3c].icon=
	creatures[0x3d].icon=8406;
	creatures[0x3e].icon=9684;

    creatures[0x3f].basesound=115;                              // cougar
	creatures[0x3f].icon=9603;
	creatures[0x3f].type=2;

	creatures[0x40].basesound=115;                              // snow leopard
	creatures[0x40].icon=9635;
	creatures[0x40].type=2;

	creatures[0x41].basesound=115;                              // snow leopard 2
	creatures[0x41].icon=9635;
	creatures[0x41].type=2;

	creatures[0x42].basesound=639;                              // swamp tentacle
	creatures[0x42].icon=9672;

	creatures[0x43].basesound=372;                              // stone gargoyle
	creatures[0x43].icon=9614;

	creatures[0x44].basesound=377;                              // gazer
	creatures[0x44].icon=9615;

	creatures[0x45].basesound=875;                              // gazer 2
	creatures[0x45].icon=8436;

	creatures[0x46].basesound=590;                             // terathen warrior
	creatures[0x46].icon=9673;

    creatures[0x47].basesound=590;                             // terathen drone
	creatures[0x47].icon=9674;

	creatures[0x48].basesound=600;                             // terathen queen
	creatures[0x48].icon=9675;

	creatures[0x49].basesound=402;                             // stone harpy
	creatures[0x49].icon=9620;
	creatures[0x49].who_am_i+=1;

	creatures[0x4a].basesound=959;                             // Imp
	creatures[0x4a].who_am_i+=1;
	creatures[0x4a].icon=9631;

	creatures[0x4b].basesound=604;                             // Cyclop
	creatures[0x4b].icon=8493;

	creatures[0x4c].basesound=609;                             // Titan
	creatures[0x4c].icon=9677;

	creatures[0x4d].basesound=1100;                            // kraken
	creatures[0x4d].icon=9634;

	creatures[0x4e].basesound=412;                            // liche lord
	creatures[0x4e].icon=8440;

	creatures[0x4f].basesound=417;                            // liche lord 2
	creatures[0x4f].icon=8440;
    
	creatures[0x50].basesound=858;                            // giant frog 1
	creatures[0x50].icon=8495;

	creatures[0x51].basesound=863;                            // bullfrog
	creatures[0x51].icon=8496;
	creatures[0x51].type=2;

	creatures[0x52].basesound=417;                            // liche lord 3
	creatures[0x52].icon=8440;

	creatures[0x53].basesound=427;                            // ogre 2
	creatures[0x53].icon=8395;

	creatures[0x54].basesound=427;                            // ogre 3
	creatures[0x54].icon=8395;

	creatures[0x55].basesound=640;                            // ophedian mage
	creatures[0x55].icon=9643;

	creatures[0x56].basesound=635;                            // ophedian warrior
	creatures[0x56].icon=9644;

	creatures[0x57].basesound=645;                            // ophedian queen
	creatures[0x57].icon=9645;

	creatures[0x58].basesound=153;                            // mountain goat 2
	creatures[0x58].icon=9600;
	creatures[0x58].type=2;

	creatures[0x59].basesound=387;                             // giant snake ice
	creatures[0x59].icon=9664;

	creatures[0x5a].basesound=387;                             // giant snake lava
	creatures[0x5a].icon=9665;

	creatures[0x5b].basesound=904;                             // giant snake silver
	creatures[0x5b].icon=9666;
	
	creatures[0x5c].basesound=904;                             // giant snake silver
	creatures[0x5c].icon=9666;

	creatures[0x5d].basesound=904;                             // giant snake silver
	creatures[0x5d].icon=9666;

	creatures[0x5e].basesound=456;                             // ice slime
	creatures[0x5e].icon=8424;

    /// 0x5f not there, crashing 2d clients if instanciated

	creatures[0x60].basesound=456;                             // ice slime
	creatures[0x60].icon=8424;

	creatures[0x61].basesound=133;                             // hell hound
	creatures[0x61].icon=9622;
	creatures[0x61].type=2;

	creatures[0x62].basesound=133;                             // hell hound 2
	creatures[0x62].icon=9622;
	creatures[0x62].type=2;

	creatures[0x63].basesound=229;                             // wolf 2
	creatures[0x63].icon=9679;
	creatures[0x63].type=2;

	creatures[0x64].basesound=229;                             // wolf 3
	creatures[0x64].icon=9679;
	creatures[0x64].type=2;

	creatures[0x65].basesound=679;                             // centaur
	creatures[0x65].icon=9601;

	creatures[0x66].basesound=768;                             // exodus
	creatures[0x66].icon=8453;
	creatures[0x66].who_am_i+=1;

	creatures[0x67].basesound=704;                             // asian dragon
	creatures[0x67].icon=9619;
	creatures[0x67].who_am_i+=1;

	creatures[0x68].basesound=1160;                            // skeletal dragon
	creatures[0x68].icon=8406;
	creatures[0x68].who_am_i+=1;

	creatures[0x69].basesound=722;                            // dragon
	creatures[0x69].icon=8406;
	creatures[0x69].who_am_i+=1;

	creatures[0x6a].basesound=717;                            // shadow wyrm
	creatures[0x6a].icon=8406;
	creatures[0x6a].who_am_i+=1;

	creatures[0x6b].basesound=268;                            // agapite elemental
	creatures[0x6b].icon=9689;

	creatures[0x6c].basesound=268;                            // bronze elemental
	creatures[0x6c].icon=9689;

	creatures[0x6d].basesound=268;                            // copper elemental
	creatures[0x6d].icon=9689;

	creatures[0x6e].basesound=268;                            // agapite elemental
	creatures[0x6e].icon=8407;

	creatures[0x6f].basesound=268;                            // agapite elemental
	creatures[0x6f].icon=8407;

	creatures[0x70].basesound=268;                            // valorite elemental
	creatures[0x70].icon=8407;

	creatures[0x70].basesound=268;                            // verite elemental
	creatures[0x70].icon=8407;

	creatures[0x71].basesound=168;                            // horse dark steed
	creatures[0x71].icon=8481;
	creatures[0x71].type=2;

	creatures[0x72].basesound=168;                            // horse ethernal
	creatures[0x72].icon=8480;
	creatures[0x72].type=2;

	creatures[0x73].basesound=168;                            // horse nightmare
	creatures[0x73].icon=9628;
	creatures[0x73].type=2;

	creatures[0x74].basesound=168;                            // horse silver steed
	creatures[0x74].icon=9629;
	creatures[0x74].type=2;

	creatures[0x75].basesound=168;                            // horse war brittain
	creatures[0x75].icon=8481;
	creatures[0x75].type=2;

	creatures[0x76].basesound=168;                            // horse war mage council
	creatures[0x76].icon=8481;
	creatures[0x76].type=2;

	creatures[0x77].basesound=168;                            // horse mage council
	creatures[0x77].icon=8481;
	creatures[0x77].type=2;

	creatures[0x78].basesound=168;                            // horse minax
	creatures[0x78].icon=8481;
	creatures[0x78].type=2;

	creatures[0x79].basesound=168;                            // horse shadowlord
	creatures[0x79].icon=8481;
	creatures[0x79].type=2;

	creatures[0x7a].basesound=1212;                           // horse unicorn
	creatures[0x7a].icon=9678;
	creatures[0x7a].type=2;

	creatures[0x7b].basesound=757;                            // ethereal
	creatures[0x7b].icon=9609;
	creatures[0x7b].who_am_i++;
	
	creatures[0x7c].basesound=589;                            // evil mage 1
	creatures[0x7c].icon=9610;
	
	creatures[0x7d].basesound=639;                            // evil mage 2
	creatures[0x7d].icon=9611;

	creatures[0x7e].basesound=599;                            // evil mage 3
	creatures[0x7e].icon=9611;

	creatures[0x7f].basesound=105;                            // hell cat
	creatures[0x7f].icon=9623;
	creatures[0x7f].type=2;

	creatures[0x80].basesound=1133;                           // pixie
	creatures[0x80].icon=9654;

	creatures[0x81].basesound=846;                            // swamp tentacle
	creatures[0x81].icon=9672;

	creatures[0x82].basesound=372;                            // gargolye 3
	creatures[0x82].icon=8409;

	creatures[0x83].basesound=880;                            // efreet genie
	creatures[0x83].icon=9616;

	creatures[0x84].basesound=965;                            // kirin
	creatures[0x84].icon=9632;
	creatures[0x84].type=2;

	creatures[0x85].basesound=90;                            // alligator
	creatures[0x85].icon=8479;
	creatures[0x85].type=2;

	creatures[0x86].basesound=711;                           // Komodo dragon
	creatures[0x86].icon=9633;
	creatures[0x86].who_am_i++;
	creatures[0x86].type=2; // interesting but true

	creatures[0x87].basesound=427;                           // ogre lord artic
	creatures[0x87].icon=9640;

	creatures[0x88].basesound=640;                           // ophedian archmage
	creatures[0x88].icon=9641;

	creatures[0x89].basesound=635;                           // ophedian knight
	creatures[0x89].icon=9642;

	creatures[0x8a].basesound=1113;                          // orc lord
	creatures[0x8a].icon=9648;

	creatures[0x8b].basesound=1113;                          // orc lord
	creatures[0x8b].icon=9648;

	creatures[0x8c].basesound=427;                           // orc shaman
	creatures[0x8c].icon=9649;

	creatures[0x8d].basesound=1095;                          // paladin
	creatures[0x8d].icon=8454;

	creatures[0x8e].basesound=437;                          // ratman
	creatures[0x8e].icon=9655;

	creatures[0x8f].basesound=437;                          // ratman 2
	creatures[0x8f].icon=9655;

	creatures[0x90].basesound=1144;                          // sea horse
	creatures[0x90].icon=9658;
	creatures[0x90].type=1;

	creatures[0x91].basesound=477;							 // sea serpant
	creatures[0x91].basesound=3;
	creatures[0x91].icon=8443;
	creatures[0x91].type=1;

	creatures[0x92].basesound=1149;                          // shadowlord
	creatures[0x92].icon=9610;

	creatures[0x93].basesound=1165;                          // skeleton knight
	creatures[0x93].icon=9661;

	creatures[0x94].basesound=1165;                          // skeleton mage
	creatures[0x94].icon=9662;

	creatures[0x95].basesound=1200;                          // succubidus
	creatures[0x95].icon=9631; // actaully imp icon, can't find better one

	creatures[0x96].basesound=477;								// sea serpant
	creatures[0x96].basesound=3;
	creatures[0x96].icon=8443;
	creatures[0x96].type=1;

	creatures[0x97].basesound=138;								// dolphin
	creatures[0x97].icon=8433;
	creatures[0x97].type=1;
	
	creatures[0x98].basesound=1170;								// terethan
	creatures[0x98].icon=9674;

	creatures[0x99].basesound=889;								// walking dead ghoul
	creatures[0x99].icon=9671;

	creatures[0x9a].basesound=471;								// rotting corpse
	creatures[0x9a].icon=9659;

	creatures[0x9b].basesound=471;								// mummie
	creatures[0x9b].icon=9639;

	// 0x9c isn't there, crashing 2d client if instanciated

	creatures[0x9d].basesound=904;								// black widow
	creatures[0x9d].icon=9667;

	creatures[0x9e].basesound=517;								// acid elemental
	creatures[0x9e].icon=9686;
	creatures[0x9e].soundflag=2;

	creatures[0x9f].basesound=517;								// blood elemental
	creatures[0x9f].icon=9688;
	creatures[0x9f].soundflag=2;

	creatures[0xa0].basesound=517;								// blood elemental
	creatures[0xa0].icon=9688;
	creatures[0xa0].soundflag=2;

	creatures[0xa1].basesound=263;								// ice elemental
	creatures[0xa1].icon=9690;	

	creatures[0xa2].basesound=517;								// poision elemental
	creatures[0xa2].icon=9691;
	creatures[0xa2].soundflag=2;

	creatures[0xa3].basesound=268;								// snow elemental
	creatures[0xa3].icon=9692;	

	creatures[0xa4].basesound=263;								// energy vortex
	creatures[0xa4].icon=9688;	

	creatures[0xa5].basesound=466;								// wisp
	creatures[0xa5].icon=8448;	

	creatures[0xa6].basesound=268;								// gold elemental
	creatures[0xa6].icon=9689;

	creatures[0xa7].basesound=1218;								// brown bear
	creatures[0xa7].icon=8478;
	creatures[0xa7].type=2;

	creatures[0xa8].basesound=959;								// shadow imp
	creatures[0xa8].icon=9631;

	//// 0xa9 not in art files, hence crahsing 2d client if instanciated

	creatures[0xaa].basesound=1011;								// ethereal llama
	creatures[0xaa].icon=8438;
	creatures[0xaa].type=2;

	creatures[0xab].basesound=183;								// ethereal ostard
	creatures[0xab].icon=9652;
	creatures[0xab].soundflag=2;
	creatures[0xab].type=2;

	creatures[0xac].basesound=362;								// dragon red
	creatures[0xac].icon=8406;
	creatures[0xac].who_am_i+=1;

	creatures[0xad].basesound=904;								// black widow
	creatures[0xad].icon=9667;

	creatures[0xae].basesound=1200;								// scubbidus big
	creatures[0xae].icon=9631;
	creatures[0xae].who_am_i+=1;

	creatures[0xaf].basesound=762;								// ethereal warrior male
	creatures[0xaf].icon=9609;

	creatures[0xb0].basesound=1132;								// pixie big
	creatures[0xb0].icon=9654;
	creatures[0xb0].who_am_i+=1;


	creatures[0xb2].basesound=168;								// nightmare
	creatures[0xb2].icon=9628;
	creatures[0xb2].type=2;

	creatures[0xb3].basesound=168;								// nightmare
	creatures[0xb3].icon=9628;
	creatures[0xb3].type=2;

	creatures[0xb4].basesound=704;								// white wyrm
	creatures[0xb4].icon=8406;
	creatures[0xb4].who_am_i+=1;

	creatures[0xb5].basesound=432;								// orc scout
	creatures[0xb5].icon=9647;

	creatures[0xb6].basesound=432;								// orc bomber
	creatures[0xb6].icon=9648;

	// b7...ba : savage npc's no sound

	creatures[0xbb].basesound=869;								// ridgeback
	creatures[0xbb].icon=9650; // no better icon there (ostard)
	creatures[0xbb].type=2;

	creatures[0xbc].basesound=869;								// ridgeback
	creatures[0xbc].icon=9650; 
	creatures[0xbc].type=2;

	creatures[0xbd].basesound=1114;								// orc-lord
	creatures[0xbd].icon=9648; 

	/// be .. c7 : not in artfiules, crashing client if instanciated

	creatures[0xc8].basesound=168;								// white horse
	creatures[0xc8].icon=8479;
	creatures[0xc8].type=2;

	creatures[0xc9].basesound=105;								// cat
	creatures[0xc9].who_am_i+=2; // set blink flag
	creatures[0xc9].icon=8475;
	creatures[0xc9].type=2;

	creatures[0xca].basesound=90;								// alligator
	creatures[0xca].icon=8410;
	creatures[0xca].type=2;

	creatures[0xcb].basesound=196;								// small pig
	creatures[0xcb].icon=8449;
	creatures[0xcb].type=2;

	creatures[0xcc].basesound=168;								// brown horse
	creatures[0xcc].icon=8481;
	creatures[0xcc].type=2;

	creatures[0xcd].basesound=201;								// rabbit
	creatures[0xcd].soundflag=2;								// rabbits only have 3 sounds, thus need special treatment
	creatures[0xcd].icon=8485;
	creatures[0xcd].type=2;

	creatures[0xcf].basesound=214;								// wooly sheep
	creatures[0xcf].icon=8427;
	creatures[0xcf].type=2;

	creatures[0xd0].basesound=110;								// chicken
	creatures[0xd0].icon=8401;
	creatures[0xd0].type=2;

	creatures[0xd1].basesound=153;								// goat
	creatures[0xd1].icon=8422; // theres no goat icon, so i took a (differnt) sheep
	creatures[0xd1].type=2;

	creatures[0xd3].basesound=95;								// brown bear
	creatures[0xd3].icon=8399;
	creatures[0xd3].type=2;

	creatures[0xd4].basesound=95;								// grizzly bear
	creatures[0xd4].icon=8411;
	creatures[0xd4].type=2;

	creatures[0xd5].basesound=95;								// polar bear
	creatures[0xd5].icon=8417;
	creatures[0xd5].type=2;

	creatures[0xd6].basesound=186;								// panther
	creatures[0xd6].who_am_i+=2;
	creatures[0xd6].icon=8473;
	creatures[0xd6].type=2;

	creatures[0xd7].basesound=392;								// giant rat
	creatures[0xd7].icon=8400;

	creatures[0xd8].basesound=120;								// cow
	creatures[0xd8].icon=8432;
	creatures[0xd8].type=2;

	creatures[0xd9].basesound=133;								// dog
	creatures[0xd9].icon=8405;
	creatures[0xd9].type=2;

	creatures[0xda].basesound=1013;							    // forest ostard
	creatures[0xda].icon=9651; 
	creatures[0xda].type=2;

	creatures[0xdb].basesound=1013;							    // frenzied ostard
	creatures[0xdb].icon=9652; 
	creatures[0xdb].type=2;

	creatures[0xdc].basesound=183;								// llama
	creatures[0xdc].soundflag=2;
	creatures[0xdc].icon=8438;
	creatures[0xdc].type=2;

	creatures[0xdd].basesound=224;								// walrus
	creatures[0xdd].icon=8447;
	creatures[0xdd].type=1;

	creatures[0xdf].basesound=216;								// lamb/shorn sheep
	creatures[0xdf].soundflag=2;
	creatures[0xdf].icon=8422;
	creatures[0xdf].type=2;

	creatures[0xe1].basesound=229;								// jackal
	creatures[0xe1].who_am_i+=2; // set anti blink bit
	creatures[0xe1].icon=8426;
	creatures[0xe1].type=2;

	creatures[0xe2].basesound=168;								// yet another horse
	creatures[0xe2].icon=8484;
	creatures[0xe2].type=2;

	creatures[0xe4].basesound=168;								// horse ...
	creatures[0xe4].icon=8480;
	creatures[0xe4].type=2;

	creatures[0xe7].basesound=120;								// brown cow
	creatures[0xe7].who_am_i+=2;
	creatures[0xe7].icon=8432;
	creatures[0xe7].type=2;

	creatures[0xe8].basesound=100;								// bull
	creatures[0xe8].who_am_i+=2;
	creatures[0xe8].icon=8431;
	creatures[0xe8].type=2;

	creatures[0xe9].basesound=120;								// b/w cow
	creatures[0xe9].who_am_i+=2;
	creatures[0xe9].icon=8451;
	creatures[0xe9].type=2;

	creatures[0xea].basesound=130;								// deer
	creatures[0xea].soundflag=2;
	creatures[0xea].icon=8404;
	creatures[0xea].type=2;

	creatures[0xed].basesound=130;								// small deer
	creatures[0xed].soundflag=2;
	creatures[0xed].icon=8404;
	creatures[0xed].type=2;

	creatures[0xee].basesound=204;								// rat
	creatures[0xee].icon=8483;
	creatures[0xee].type=2;

	/// 0xef - 0x121  no monster, but not crashing client 

	creatures[0x122].basesound=196;								// Boar
	creatures[0x122].icon=8449;
	creatures[0x122].type=2;

	creatures[0x123].basesound=168;								// pack horse
	creatures[0x123].icon=8486;
	creatures[0x123].type=2;

	creatures[0x124].basesound=183;								// pack llama
	creatures[0x124].soundflag=2;
	creatures[0x124].icon=8487;
	creatures[0x124].type=2;

	/// 0x125 - 0x189  no monster but not crashing client 

	creatures[0x190].icon=8454;
	creatures[0x191].icon=8455;

	creatures[0x192].icon=9712;
	creatures[0x193].icon=9713;
	
	/// 0x194 - 0x23c  no monster, but not crashing client 

	creatures[0x23d].basesound=263;								// e-vortex
	creatures[0x23e].basesound=512;								// blade spritit
	creatures[0x23e].soundflag=4;

	/// 0x23f-0x2ef no monster but not crashing client 

	//// LBR monster here.
	//// because none of them has neither deicated sounds nor icons, 
	//// --> lots of creative freedom ;)

	creatures[0x2f0].basesound=268;								// iron golem
	creatures[0x2f0].icon=9649;

	creatures[0x2f1].basesound=372;								// gargoyle slave
	creatures[0x2f1].icon=9613;
	creatures[0x2f1].who_am_i+=1;

	creatures[0x2f2].basesound=372;								// gargoyle enforcer
	creatures[0x2f2].icon=9613;
	creatures[0x2f2].who_am_i+=1;

	creatures[0x2f3].basesound=372;								// gargoyle guard
	creatures[0x2f3].icon=9613;
	creatures[0x2f3].who_am_i+=1;

	creatures[0x2f4].basesound=877;								// clockwork overseer
	creatures[0x2f4].icon=9615;

	creatures[0x2f5].basesound=877;								// clockwork minion
	creatures[0x2f5].icon=9615;

	creatures[0x2f6].basesound=372;								// gargoyle shopkeeper
	creatures[0x2f6].icon=9613;
	creatures[0x2f6].who_am_i+=1;


	// 2f7-2fa: no monster, but not crashing client

	creatures[0x2fb].basesound=877;								// clockwork minion
	creatures[0x2fb].icon=9615; // gazer icon

    // 2fc-306 : NPCs -> no sound (LB, juggernaut, meer's etc)

	creatures[0x307].basesound=849;								// plague beast
	creatures[0x307].icon=8424; // no better icon than slime

	creatures[0x308].basesound=863;								// horde demon
	creatures[0x308].icon=8495; // frog icon !!!

	creatures[0x309].basesound=1110;							// doppelgänger
	creatures[0x309].icon=9685; // zombie icon !

	creatures[0x30a].basesound=877;								// swarm
	creatures[0x30a].icon=9615; // very gazor like again

	creatures[0x30b].basesound=760;								// bogling
	creatures[0x30b].icon=8407; // earth ele icon

	creatures[0x30c].basesound=768;								// bog thing
	creatures[0x30c].icon=9602; // corsper icon

	creatures[0x30d].basesound=838;								// fire ant worker
	creatures[0x30d].icon=9673; // therathan icon

	creatures[0x30e].basesound=838;								// fire ant warrior
	creatures[0x30e].icon=9674; // therathan icon

	creatures[0x30f].basesound=838;								// fire ant queen
	creatures[0x30f].icon=9675; // therathan icon

	creatures[0x310].basesound=357;								// archane daemon
	creatures[0x310].icon=9605; 
	
	creatures[0x311].basesound=696;								// four armed daemon
	creatures[0x311].icon=9606; 

    creatures[0x312].basesound=904;								// chariot
	creatures[0x312].icon=9667; 

	creatures[0x313].basesound=397;							    // ant lion
	creatures[0x313].icon=9657; // giant scorp !

	creatures[0x314].basesound=1006;							// phinx
	creatures[0x314].icon=9679; // wolf ! 

	creatures[0x315].basesound=1105;							// quagmire
	creatures[0x315].icon=9668; // spider ! any better idea ? 

	creatures[0x316].basesound=655;							    // sand vortex
	creatures[0x316].icon=9689; 

	creatures[0x317].basesound=701;							    // giant beetle
	creatures[0x317].icon=9667; // spider :/
	creatures[0x317].type=2;

	creatures[0x318].basesound=357;								// chaos daemon
	creatures[0x318].icon=9604; 	

	creatures[0x319].basesound=168;							    // skeletal mount
	creatures[0x319].icon=9627; // noraml horse icon
	creatures[0x319].type=2;

	creatures[0x31a].basesound=362;							    // swamp dragon
	creatures[0x31a].icon=9619; 
	creatures[0x31a].who_am_i+=1;
	creatures[0x31a].type=2;


	creatures[0x31b].basesound=697;							    // horde daemon (sesamy street?!)
	creatures[0x31b].icon=9604; 

	creatures[0x31c].basesound=697;							    // horde daemon big
	creatures[0x31c].icon=9604; 

	creatures[0x31d].basesound=273;							    // fire dragon
	creatures[0x31d].icon=8406; 
	creatures[0x31d].who_am_i+=1;

	creatures[0x31e].basesound=362;							    // rust dragon
	creatures[0x31e].icon=8406; 
	creatures[0x31e].who_am_i+=1;

	creatures[0x31f].basesound=362;							    // swamp dragon
	creatures[0x31f].icon=8406; 
	creatures[0x31f].who_am_i+=1;
	creatures[0x31f].type=2;


	// 0x320 .. 0x333 no monsters, but not crashing client

	creatures[0x334].basesound=168;							    // horse, dappeled brown
	creatures[0x334].icon=9624; 
	creatures[0x334].type=2;


	// 0x335 .. 0x337 no monsters, but not crashing client

	creatures[0x338].basesound=168;							    // horse, dark brown
	creatures[0x338].icon=9626; 
	creatures[0x338].type=2;


	creatures[0x339].basesound=1013;							// dessert ostard
	creatures[0x339].icon=9650; 
	creatures[0x339].type=2;


	creatures[0x33a].basesound=1013;							// forest ostard
	creatures[0x33a].icon=9651; 
	creatures[0x33a].type=2;

	creatures[0x33b].basesound=1013;							// frenzied ostard
	creatures[0x33b].icon=9652; 
	creatures[0x33b].type=2;


	creatures[0x33c].basesound=183;								// llama
	creatures[0x33c].soundflag=2;
	creatures[0x33c].icon=8438;
	creatures[0x33c].type=2;


	// 0x33d .. 0x33e no monsters, but not crashing client

	creatures[0x33f].basesound=191;								// birds: parrot
	creatures[0x33f].icon=8474;
	creatures[0x33f].type=2;

	
	creatures[0x340].basesound=750;								// birds: phenix
	creatures[0x340].icon=8434;
	creatures[0x340].type=2;


	creatures[0x341].basesound=629;								// birds: turkey
	creatures[0x341].icon=8474;
	creatures[0x341].type=2;

	creatures[0x342].basesound=674;								// hellcat
	creatures[0x342].icon=9623;
	creatures[0x342].type=2;

	creatures[0x343].basesound=201;								// jack rabbit
	creatures[0x343].icon=8485;
	creatures[0x343].soundflag=2;
	creatures[0x343].type=2;


	// 0x344 no monster, but not crashing client

	creatures[0x345].basesound=219;								// ice snake
	creatures[0x345].icon=9664;
	creatures[0x345].type=2;

	creatures[0x346].basesound=219;								// lava snake
	creatures[0x346].icon=9665;
	creatures[0x346].type=2;

	creatures[0x347].basesound=869;								// ridgeback
	creatures[0x347].icon=9650; // ostard ...
	creatures[0x347].type=2;

	creatures[0x348].basesound=869;								// ridgeback giant
	creatures[0x348].icon=9650; // ostard ...
	creatures[0x348].type=2;

	creatures[0x349].basesound=869;								// ridgeback flame
	creatures[0x349].icon=9650; // ostard ...
	creatures[0x349].type=2;

	creatures[0x34a].basesound=869;								// ridgeback hatchling
	creatures[0x34a].icon=9650; // ostard ...
	creatures[0x34a].type=2;

	creatures[0x34b].basesound=204;								// rat
	creatures[0x34b].icon=8483;
	creatures[0x34b].type=2;


	// 0x34c-d no monsters, but not crashing client

	creatures[0x34e].basesound=168;								// horse: dappled grey
	creatures[0x34e].icon=9625; 
	creatures[0x34e].type=2;

	// 0x34f  no monster, but not crashing client

	creatures[0x350].basesound=168;								// horse: tan
	creatures[0x350].icon=9630; 
	creatures[0x350].type=2;

	// 0x351-0x3c9 no monsters, but not crashing client

	// 0x3ca: player ghost
	// 0x3de: LB
	// 0x3df blackthorn
	// 03e2: dupre

	// anything between those 4 and 0x3e6: no monsters, but not crashing client

	creatures[0x3e6].basesound=168;								// kirin_kirin
	creatures[0x3e6].icon=9632; 
	creatures[0x3e6].type=2;

	// npc's

	//creatures[

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

	RegionIterator4Chars ri(pc->pos());
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_PLAYER pc = dynamic_cast<P_PLAYER>(ri.GetData());
		if( pc )
		{
			if( !pc->isDead() && !pc->isAtWar() )
			{
				if (!pc->free) // lb, bugfix !
				{
					if (inRange(pc->pos().x, pc->pos().y, pc->pos().x, pc->pos().y, distance))
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
		xx = inrange[sound]->bodyID();
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
				/*sfx[2]=basesound>>8;
				sfx[3]=basesound%256;
				sfx[6]=inrange[sound]->pos().x>>8;
				sfx[7]=inrange[sound]->pos().x%256;
				sfx[8]=inrange[sound]->pos().y>>8;
				sfx[9]=inrange[sound]->pos().y%256;
				Xsend(calcSocketFromChar(pc), sfx, 12); //bugfix, LB*/
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
			/*sfx[2] = (unsigned char) (basesound>>8);
			sfx[3] = (unsigned char) (basesound%256);
			sfx[6] = (unsigned char) (pc->pos().x>>8);
			sfx[7] = (unsigned char) (pc->pos().x%256);
			sfx[8] = (unsigned char) (pc->pos().y>>8);
			sfx[9] = (unsigned char) (pc->pos().y%256);
			Xsend(calcSocketFromChar(pc), sfx, 12); //bugfix LB*/
		}
	}
}

/*!
	Give karma credit for killing someone.
*/
void Karma( P_CHAR pc_toChange, P_CHAR pc_Killed, int nKarma )
{
	// nEffect = 1 positive karma effect
	int nCurKarma = 0, nChange = 0, nEffect = 0;

	nCurKarma = pc_toChange->karma();

	if((nCurKarma>10000)||(nCurKarma<-10000))
		if(nCurKarma>10000)
			pc_toChange->setKarma(10000);
		else
			pc_toChange->setKarma(-10000);

	if( nCurKarma < nKarma && nKarma > 0 )
	{
		nChange=((nKarma-nCurKarma)/75);
		pc_toChange->setKarma(nCurKarma+nChange);
		nEffect=1;
	}

	//AntiChrist - modified to avoid crashes
	//the nKilledID==-1 check and the chars[nKilledID] check were in the same line
	//That may cause some crash with some compilator caus there's no a defined
	//order in executing these if checks
	if( ( nCurKarma > nKarma ) && ( !pc_Killed ) )
	{
		nChange = ( ( nCurKarma - nKarma ) / 50 );
		pc_toChange->setKarma( nCurKarma - nChange );
		nEffect = 0;
	}
	else if( ( nCurKarma > nKarma ) && ( pc_Killed->karma() > 0 ) )
	{
		nChange=((nCurKarma-nKarma)/50);
		pc_toChange->setKarma(nCurKarma-nChange);
		nEffect=0;
	}

	// For NPCs we dont need to send a message
	// If nothing changed we dont need to do that either
	P_PLAYER pp = dynamic_cast<P_PLAYER>(pc_toChange);

	if( !nChange || !pp || !pp->socket() )
		return;

	UINT32 message;

	if( nChange <= 25 )
	{
		if( nEffect )
			message = 0xF8CB3; // You have gained a little karma.
		else
			message = 0xF8CB7; // You have lost a little karma.
	}
	else if( nChange <= 75 )
	{
		if( nEffect )
			message = 0xF8CB4; // You have gained some karma.
		else
			message = 0xF8CB8; // You have lost some karma.
	}
	else if( nChange <= 100 )
	{
		if( nEffect )
			message = 0xF8CB5; // You have gained a good amount of karma.
		else
			message = 0xF8CB9; // You have lost a good amount of karma.
	}
	else if( nChange > 100 )
	{
		if( nEffect )
			message = 0xF8CB6; // You have gained a lot of karma.
		else
			message = 0xF8CBA; // You have lost a lot of karma.
	}

	pp->socket()->clilocMessage( message );
}

/*!
	Give fame credit for killing someone.
*/
void Fame( P_CHAR pChar, int nFame )
{
	P_PLAYER pc_toChange = dynamic_cast<P_PLAYER>( pChar );
	if( !pc_toChange )
		return;

	int nCurFame, nChange=0;
	bool gain = false;

	pc_toChange->setFame( QMIN( 10000, pc_toChange->fame() ) );

	nCurFame = pc_toChange->fame();

	// We already have more than that.
	if( nCurFame > nFame )
		return;

	// Loose Fame when we died
	if( pc_toChange->isDead() )
	{
		// Fame / 25 is our loss
		nChange = nCurFame / 25;
		pc_toChange->setFame( QMAX( 0, nCurFame - nChange ) );
		pc_toChange->setDeaths( pc_toChange->deaths() + 1 );
		gain = false;
	}
	else
	{
		nChange = ( nFame - nCurFame ) / 75;
		pc_toChange->setFame( nCurFame+nChange );
		gain = true;
	}

	// Nothing changed or we can't recieve the message
	if( !nChange || !pc_toChange->socket() )
		return;

	QString message;

	if( nChange <= 25 )
	{
		if( gain )
			message = tr( "You have gained a little fame.", 0, "Fame Messages" );
		else
			message = tr( "You have lost a little fame.", 0, "Fame Messages" );
	}
	else if( nChange <= 75 )
	{
		if( gain )
			message = tr( "You have gained some fame.", 0, "Fame Messages" );
		else
			message = tr( "You have lost some fame.", 0, "Fame Messages" );
	}
	else if( nChange <= 100 )
	{
		if( gain )
			message = tr( "You have gained alot of fame.", 0, "Fame Messages" );
		else
			message = tr( "You have lost alot of fame.", 0, "Fame Messages" );
	}
	else if( nChange > 100 )
	{
		if( gain )
			message = tr( "You have gained a huge amount of fame.", 0, "Fame Messages" );
		else
			message = tr( "You have lost a huge amount of fame.", 0, "Fame Messages" );
	}

	pc_toChange->socket()->sysMessage( message );
}

/*!
	Update the flagging of this character based
	on his reputation.
*/
void setcharflag( P_CHAR pc )
{
	//First, let's see their karma.
	if( pc->karma() <= -200 )
	{
		pc->setMurdererTime( uiCurrentTime + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC );
	}

	if( pc->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pc);
		if (pp->isGMorCounselor())
		{
			pp->setMurdererTime( 0 );
			pp->setCriminalTime( 0 );
			return;
		}
		else if (pc->kills() >= (unsigned) SrvParams->maxkills())
		{
			pp->setMurdererTime( uiCurrentTime + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC );
			return;
		}	
		else if (pc->criminalTime() == 0)
		{
			pp->setMurdererTime( 0 );
			return;
		}
		else if (pc->criminalTime() > 0)
		{
			pp->makeCriminal();
			return;
		}		
		else
		{
			pp->makeCriminal();
		}
	}
	else if (pc->objectType() == enNPC) // && ((pc->npcaitype() == 2) || // bad npc
		//(pc->npcaitype() == 3) ||  // bad healer
		//(pc->npcaitype() == 50)))   // EV & BS
	{
		if (SrvParams->badNpcsRed() == 0)
		{
			pc->setCriminalTime( uiCurrentTime + SrvParams->crimtime() * MY_CLOCKS_PER_SEC );
		}
		else
		{
			pc->setMurdererTime( uiCurrentTime + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC );
		}
	}
	else
	{
/*		switch (pc->npcaitype())
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
			case 11: // good npcs
			case 17: // player vendor
			case 18: // escort npcs
			case 19: // real estate brokers
				pc->setInnocent();
				break;
			default:
				if (pc->isHuman())
				{
					pc->setMurdererTime( 0 );
					pc->setCriminalTime( 0 );
					return;
				}
				if (SrvParams->animals_guarded() == 1 && pc->npcaitype() == 0 && !pc->isHuman() && !pc->tamed())
				{
					if (pc->inGuardedArea())	// in a guarded region, with guarded animals, animals == blue
						pc->setMurdererTime( 0 );
						pc->setCriminalTime( 0 );
					else				// if the region's not guarded, they're gray
						pc->criminal();
				}
				else if( pc->owner() && !pc->isHuman() )
				{
						pc->setFlag( pc->owner()->flag() );
				}
				else
					pc->criminal();
				break;
		}*/
	}
}


