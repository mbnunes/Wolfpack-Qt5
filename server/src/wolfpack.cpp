//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//      Copyright 2001-2003 by holders identified in authors.txt
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


#include "wolfpack.h"
#include "world.h"
#include "verinfo.h"
#include "speech.h"
#include "territories.h"

#include "TmpEff.h"
#include "guildstones.h"
#include "combat.h"
#include "sectors.h"
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
#include "scriptmanager.h"
#include "wptargetrequests.h"
#include "python/engine.h"
#include "persistentbroker.h"
#include "corpse.h"
#include "house.h"
#include "boats.h"
#include "multiscache.h"
#include "Trade.h"
#include "network/uotxpackets.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "ai.h"
#include "sectors.h"
#include "basedef.h"

// Library Includes
#include <qapplication.h>
#include <qtranslator.h>
#include <qstring.h>
#include <qlibrary.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qmutex.h>
#include <qthread.h>
#include <fstream>


#if defined(Q_OS_UNIX)
#	include <signal.h>
#else
#	include <conio.h>
#endif

#include "python/utilities.h"

using namespace std;

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
		{
                SrvParams->reload();
                cNetwork::instance()->reload();

				QStringList oldAISections = DefManager->getSections( WPDT_AI );
				DefManager->reload(); //Reload Definitions
				AIFactory::instance()->checkScriptAI( oldAISections, DefManager->getSections( WPDT_AI ) );

				Accounts::instance()->reload();
                SpawnRegions::instance()->reload();
                AllTerritories::instance()->reload();
                Resources::instance()->reload();
                MakeMenus::instance()->reload();
                ScriptManager->reload();
				ContextMenus::instance()->reload();
				Skills->reload();

				// Update the Regions
				for( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
				{
					cTerritory *region = AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
					pChar->setRegion( region );
				}
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

void reloadScripts()
{
	clConsole.send( "Reloading definitions, scripts and wolfpack.xml\n" );

	SrvParams->reload(); // Reload wolfpack.xml
	
	QStringList oldAISections = DefManager->getSections( WPDT_AI );
	DefManager->reload(); //Reload Definitions
	AIFactory::instance()->checkScriptAI( oldAISections, DefManager->getSections( WPDT_AI ) );

	Accounts::instance()->reload();
	SpawnRegions::instance()->reload();
	AllTerritories::instance()->reload();
	Resources::instance()->reload();
	MakeMenus::instance()->reload();
	ScriptManager->reload(); // Reload Scripts
	ContextMenus::instance()->reload();
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


QMutex commandMutex;
QStringList commandQueue;

class cConsoleThread: public QThread
{
	QWaitCondition waitCondition;
public:
	~cConsoleThread() throw() {
		cancel();
		wait(); // wait for it to stop
	}

	void cancel()
	{
		waitCondition.wakeAll();
	}

protected:
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
						QMutexLocker lock( &commandMutex ); // Exception safe
						commandQueue.push_back( QString( "%1" ).arg( key ) );
					}
				}
				waitCondition.wait(10);
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

			case '#':
				World::instance()->save();
				
				SrvParams->flush();
				break;
			case 'D':	// Disconnect account 0 (useful when client crashes)
			case 'd':	
					break;
			case 'P':
			case 'p':				// Display profiling information
//				clConsole.send("Performace Dump:\n");
//				clConsole.send("Network code: %fmsec [%i samples]\n" _ (float)((float)networkTime/(float)networkTimeCount) _ networkTimeCount);
//				clConsole.send("Timer code: %fmsec [%i samples]\n" _ (float)((float)timerTime/(float)timerTimeCount) _ timerTimeCount);
//				clConsole.send("Auto code: %fmsec [%i samples]\n" _ (float)((float)autoTime/(float)autoTimeCount) _ autoTimeCount);
//				clConsole.send("Loop Time: %fmsec [%i samples]\n" _ (float)((float)loopTime/(float)loopTimeCount) _ loopTimeCount);
//				clConsole.send("Characters: %i/%i (Dynamic)		Items: %i/%i (Dynamic)\n" _ charcount _ cmem _ itemcount _ imem);
//				clConsole.send("Simulation Cycles: %f per sec\n" _ (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
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
						clConsole.send( QString("%1) %2 [%3]\n").arg(++i).arg(mSock->player()->name()).arg(QString::number( mSock->player()->serial(), 16) ) );
					}
				}

				clConsole.send( tr("Total Users Online: %1\n").arg(cNetwork::instance()->count()) );
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
				clConsole.send(tr("WOLFPACK: Key %1 [%2] does not preform a function.\n").arg( c > 32 ? c : '¿' ).arg(QString::number(c)));
				break;
			}
		}
	}
}

static void parseParameter( const QString &param )
{
	// Add what ever paramters you want
	// Right now we don't have any parameters
	if ( param.contains(".py") )
	{
		if ( QFile::exists( param ) )
		{
			PyObject *pName, *pModule, *pDict, *pFunc;
			PyObject *pArgs, *pValue;
			int i;
			
			pName = PyString_FromString( param.left(param.length() - 3).latin1() );
			/* Error checking of pName left out */
			
			pModule = PyImport_Import(pName);
			Py_DECREF(pName);
			
			if (pModule != NULL) {
				pDict = PyModule_GetDict(pModule);
				/* pDict is a borrowed reference */
				
				pFunc = PyDict_GetItemString(pDict, "main");
				/* pFun: Borrowed reference */
				
				if (pFunc && PyCallable_Check(pFunc)) {
					pArgs = PyTuple_New(0);
					pValue = PyObject_CallObject(pFunc, pArgs);
					Py_DECREF(pArgs);
					if (pValue != NULL) {
						printf("Result of call: %ld\n", PyInt_AsLong(pValue));
						Py_DECREF(pValue);
					}
					else {
						Py_DECREF(pModule);
						PyErr_Print();
						fprintf(stderr,"Call failed\n");
						return;
					}
					/* pDict and pFunc are borrowed and must not be Py_DECREF-ed */
				}
				else {
					if (PyErr_Occurred())
						PyErr_Print();
				}
				Py_DECREF(pModule);
			}
			else {
				PyErr_Print();
				fprintf(stderr, "Failed to load \"%s\"\n", param.latin1());
			}
			stopPython();
			exit(0);
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
	Map				 = 0;
	Skills			 = 0;
	ScriptManager	 = 0;
	DefManager		 = 0;
	SrvParams		 = 0;
	persistentBroker = 0;

	SrvParams		 = new cSrvParams( "wolfpack.xml", "Wolfpack", "1.0" );
	Map				 = new Maps ( SrvParams->mulPath() );
	Skills			 = new cSkills;
	ScriptManager	 = new cScriptManager;
	DefManager		 = new WPDefManager;
	persistentBroker = new PersistentBroker;
}

/*!
	Deletes global objects.
*/
static void freeClasses( void )
{
	delete SrvParams;
	delete Map;
	delete Skills;
	delete ScriptManager;
	delete DefManager;
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
	/*clConsole.send( QString::number( sizeof( cUObject ) ) );
	return 0;*/

	clConsole.send( QString( "\n%1 %2 %3 \n\n" ).arg( wp_version.productstring.c_str() ).arg( wp_version.betareleasestring.c_str() ).arg( wp_version.verstring.c_str() ) );

	clConsole.send( "Copyright (C) 1997, 98 Marcus Rating (Cironian)\n");
	clConsole.send( "Copyright (C) 2000-2003 Wolfpack Development Team\n");
	clConsole.send( "Wolfpack Homepage: http://www.wpdev.org/\n");
	clConsole.send( "By using this software you agree to the license accompanying this release.\n");
	clConsole.send( "Compiled on " __DATE__ " " __TIME__ "\n" );
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

		// For each map we register, register a GridMap as well
		MapObjects::instance()->addMap( 0, 6144, 4096 );
		MapObjects::instance()->addMap( 1, 6144, 4096 );
		MapObjects::instance()->addMap( 2, 2304, 1600 );
		MapObjects::instance()->addMap( 3, 2560, 2048 );

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
	if( !persistentBroker->openDriver( SrvParams->databaseDriver() ) )
	{		
		clConsole.log( LOG_FATAL, QString("Error trying to open %1 database driver, check your wolfpack.xml").arg(SrvParams->databaseDriver()) );
		exit( -1 );
	}

	// Registers our Built-in types into factory.
	cPlayer::registerInFactory();
	cNPC::registerInFactory();
	cItem::registerInFactory();
	cCorpse::registerInFactory();
	cBoat::registerInFactory();
	cHouse::registerInFactory();
	cResourceItem::registerInFactory();

	// NPC AI types
	Monster_Aggressive_L0::registerInFactory();
	Monster_Aggressive_L1::registerInFactory();
	Human_Vendor::registerInFactory();
	Human_Stablemaster::registerInFactory();
	Animal_Wild::registerInFactory();
	Animal_Domestic::registerInFactory();
	// Script NPC AI types
	QStringList aiSections = DefManager->getSections( WPDT_AI );
	QStringList::const_iterator aiit = aiSections.begin();
	while( aiit != aiSections.end() )
	{
		ScriptAI::registerInFactory( *aiit );
		++aiit;
	}

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

	QWaitCondition niceLevel;
	// This is our main loop
	while( keeprun )
	{		
		// Python threading - start
		_save = PyEval_SaveThread();

		switch( SrvParams->niceLevel() )
		{
			case 0: break;	// very unnice - hog all cpu time
			case 1: if ( cNetwork::instance()->count() != 0) niceLevel.wait(10); else niceLevel.wait(100); break;
			case 2: niceLevel.wait(10); break;
			case 3: niceLevel.wait(40); break;// very nice
			case 4: if ( cNetwork::instance()->count() != 0 ) niceLevel.wait(10); else niceLevel.wait(4000); break; // anti busy waiting
			case 5: if ( cNetwork::instance()->count() != 0 ) niceLevel.wait(40); else niceLevel.wait(5000); break;
			default: niceLevel.wait(10); break;
		}

		// Python threading - end
		PyEval_RestoreThread( _save );

		// It's more likely that we have a new key-press now
		// Checking every 25 loops should be enough.
		#if !defined( __unix__ )
		if( loopTimeCount % 25 == 0 )
		{
			QMutexLocker lock(&commandMutex);
			if( commandQueue.count() > 0 )
			{				
				// Interpret Command
				interpretCommand( commandQueue[0] );
				commandQueue.erase( commandQueue.begin() );
			}
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
	DefManager->unload();

	// Stop Python Interpreter.
	ScriptManager->unload();
	stopPython();

	if( SrvParams->serverLog() ) 
		savelog( "Server shutdown","server.log" );

	return 0;
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

	cCharBaseDef *basedef = BaseDefManager::instance()->getCharBaseDef( id );		

	if( !basedef )	// Nothing known about this creature
		return;

	x=id;
	basesound=basedef->basesound();
	sf=basedef->soundmode();
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


void savelog(const char *msg, char *logfile)
{
	FILE *file;
	file = fopen( logfile, "a" );
	
	QString logMessage = QString( "[%1] %2\n" ).arg( QDateTime::currentDateTime().toString() ).arg( msg );
	
	// Remove newlines
	logMessage = logMessage.replace( QRegExp( "\n" ), "" );
	
	fprintf( file, "%s", logMessage.ascii() );
	
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
