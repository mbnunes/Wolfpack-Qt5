//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#include "accounts.h"
#include "ai/ai.h"
#include "basedef.h"
#include "basics.h"
#include "boats.h"
#include "commands.h"
#include "console.h"
#include "contextmenu.h"
#include "corpse.h"
#include "house.h"
#include "inlines.h"
#include "log.h"
#include "makemenus.h"
#include "maps.h"
#include "multiscache.h"
#include "network.h"
#include "npc.h"
#include "persistentbroker.h"
#include "player.h"
#include "resources.h"
#include "scriptmanager.h"
#include "sectors.h"
#include "skills.h"
#include "spawnregions.h"
#include "srvparams.h"
#include "territories.h"
#include "tilecache.h"
#include "Timing.h"
#include "verinfo.h"
#include "wolfpack.h"
#include "world.h"
#include "wpdefmanager.h"

#include "python/engine.h"
#include "python/utilities.h"

// Library Includes
#include <qapplication.h>
#include <qtranslator.h>
#include <qstring.h>
#include <qlibrary.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qmutex.h>
#include <qthread.h>

#ifdef Q_OS_UNIX
# include <sys/wait.h>
# include <fcntl.h>
#endif

using namespace std;

static bool parseParameter( const QString &param )
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
						return false;
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
			return false;
		}
		else
			Console::instance()->send( QString("The specified python script [%1] doesn't exist.").arg(param) );
	}

	return true;
}

/*!
	Initializes global objects.
*/
static void startClasses()
{
	Map				 = 0;
	Skills			 = 0;
	DefManager		 = 0;
	SrvParams		 = 0;
	persistentBroker = 0;

	SrvParams		 = new cSrvParams( "wolfpack.xml", "Wolfpack", "1.0" );
	Map				 = new Maps ( SrvParams->mulPath() );
	Skills			 = new cSkills;
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
	delete DefManager;
}

void SetGlobalVars()
{
	keeprun = 1;
	secure = 1;
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

QMutex actionMutex;
QValueVector< eActionType > actionQueue;

void queueAction( eActionType type )
{
	if (serverState == RUNNING) {
		QMutexLocker lock( &actionMutex );
		actionQueue.push_back( type );
	}
}

//#if defined(_DEBUG)
//#include <crash.h>
//#endif

#ifdef Q_OS_UNIX
void daemonize( void )
{
	int pid, fd, status;

	pid = fork();

	switch( pid )
	{
		case 0: // child
			setsid();

			if( ( fd = open( "/dev/null", O_RDWR ) ) != -1 )
			{
				dup2( fd, 0 );
				dup2( fd, 1 );
				close( fd );
			}

			break;

		case -1:
			perror( "fork" );
			break;

		default: // we forked, so silently exit the parent
			exit( 0 );
	}
}

void pidfile_add( char *filename )
{
	FILE *pidfile;
	if( ( pidfile = fopen( filename, "w+" ) ) != NULL )
	{
		fprintf( pidfile, "%i", getpid() );
		fclose( pidfile );
	}
	else
	{
		perror( "fopen" );
	}
}

void pidfile_del( char *filename )
{
	if( unlink( filename ) == -1 )
		perror( "unlink" );
}

#endif

/*!
	Main server entry point.
*/
int main( int argc, char **argv )
{
/*#if defined(_DEBUG)
	InstallCrashHandler( HANDLE_HTTPREPORT, GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE );
	SetCustomMessage("A crash occurred. Please send this bug report to developers\n");
	SetVersion(wp_version.verstring.c_str());
#endif
*/
	unsigned int i;

#ifdef Q_OS_UNIX
	bool run_background = false;
	char *pidfile;

	for( i = 1; i < argc; i++ )
	{
		if( argv[i][0] == '-' )
		{
			switch( argv[i][1] )
			{
				case 'h':
					fprintf( stderr, "Usage: %s [-d [-p file]]\n", argv[0] );
					fputs( "  -d\trun as daemon.\n", stderr );
					fputs( "  -p\tuse file as PID file.\n", stderr );
					exit(1);

				case 'd':
					run_background = true;
					daemonize();
					break;

				case 'p':
					if( run_background == true )
					{
						pidfile = argv[i+1];
						pidfile_add( pidfile );
					}
					break;
			}
		}
	}
#endif

	keeprun = 1;
	QApplication app( argc, argv, false ); // we need one instance
	QTranslator translator( 0 ); // must be valid thru app life.

	serverState = STARTUP;

	Console::instance()->setAttributes( true, false, true, 60, 140, 70, 12, FONT_NOSERIF );
	Console::instance()->send( QString( "\n%1 %2 %3\n\n" ).arg( wp_version.productstring.latin1() ).arg( wp_version.betareleasestring.latin1() ).arg( wp_version.verstring.latin1() ) );
	Console::instance()->setAttributes( false, false, false, 0xAF, 0xAF, 0xAF, 0, FONT_FIXEDWIDTH );

	Console::instance()->send( "Copyright (C) 2000-2004 Wolfpack Development Team\n");
	Console::instance()->send( "Wolfpack Homepage: http://www.wpdev.org/\n");
	Console::instance()->send( "By using this software you agree to the license accompanying this release.\n");
	Console::instance()->send( "Compiled on " __DATE__ " " __TIME__ "\n" );
	Console::instance()->send( "\n" );
	
	QString consoleTitle = QString( "%1 %2 %3" ).arg( wp_version.productstring.latin1() ).arg( wp_version.betareleasestring.latin1() ).arg( wp_version.verstring.latin1() );
	Console::instance()->setConsoleTitle( consoleTitle );

	// Startup normal Classes
	try
	{
		atexit( freeClasses );
		startClasses();
	}
	catch( ... )
	{
		Console::instance()->log( LOG_ERROR, "Couldn't start up classes.\n" );
		return 1;
	}

	// Startup Translator
	QString languageFile = SrvParams->getString( "General", "Language File", "", true );
	if ( !languageFile.isEmpty() )
	{
		if ( !translator.load( languageFile, "." ) )
		{
			Console::instance()->log( LOG_ERROR, "Couldn't load translator.\n" );
			return 1;
		}
		qApp->installTranslator( &translator );
	}

	// Try to start up python
	try
	{
		startPython( argc, argv );
	}
	catch( ... )
	{
		Console::instance()->log( LOG_ERROR, "Couldn't start up python.\n" );
		return 1;
	}

	for( i = 1; i <= argc; ++i )
		if( !parseParameter( QString( argv[ i ] ) ) )
			return 1;

	// Load data
	DefManager->load();
	Console::instance()->send( "\n" );

/*	Console::instance()->send( "SIZEOF(cItem):" + QString::number( sizeof( cItem ) ) );
	
	Sleep( 5000 );

	for( int x = 0; x < 400000; ++x )
	{
		cItem *it = cItem::createFromScript( "e75" );
	}

	return 1;*/

	// Scriptmanager can't be in the try{} block because it sometimes throws firstchance exceptions
	// we don't like
	ScriptManager::instance()->load();
	Console::instance()->send( "\n" );

	// Try to load several data files
#if !defined(_DEBUG)
	try
	{
#endif
		Console::instance()->send( "Loading skills...\n" );
		Skills->load();

		Console::instance()->send( "Loading accounts...\n" );
		Accounts::instance()->load();

		Console::instance()->send( "Loading ip blocking rules...\n" );
		cNetwork::instance()->load();

		Console::instance()->send( "Loading regions...\n" );
		AllTerritories::instance()->load();

		Console::instance()->send( "Loading spawn regions...\n" );
		SpawnRegions::instance()->load();

		Console::instance()->send( "Loading resources...\n" );
		Resources::instance()->load();

		Console::instance()->send( "Loading makemenus...\n" );
		MakeMenus::instance()->load();

		Console::instance()->send( "Loading contextmenus...\n" );
		ContextMenus::instance()->reload();

		// Load some MUL Data
		Console::instance()->send( "Loading muls...\n" );
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

		Console::instance()->send( "\n" );
#if !defined(_DEBUG)
	}
	catch( wpException &exception )
	{
		Console::instance()->log( LOG_ERROR, exception.error() );
		return 1;
	}
	catch( ... )
	{
		Console::instance()->log( LOG_ERROR, "Unknown error while loading data files.\n" );
		return 1;
	}
#endif

	SetGlobalVars();

	// initial randomization call
	srand( uiCurrentTime );

	/*
		Check for valid database driers.
	*/
	if( !persistentBroker->openDriver( SrvParams->databaseDriver() ) )
	{		
		Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml").arg( SrvParams->databaseDriver() ) );
		return 1;
	}

	if( !persistentBroker->openDriver( SrvParams->accountsDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml").arg( SrvParams->accountsDriver() ) );
		return 1;
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
	Human_Guard::registerInFactory();
	Human_Guard_Called::registerInFactory();
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

#if !defined(_DEBUG)
	try
	{
#endif
		World::instance()->load();
#if !defined(_DEBUG)
	}
	catch( QString &error )
	{
		Console::instance()->log( LOG_ERROR, error );
		return 1;
	}
	catch( ... )
	{
		Console::instance()->log( LOG_ERROR, "An unknown error occured while loading the world.\n" );
		return 1;
	}
#endif

	Console::instance()->PrepareProgress( "Initializing Multis" );
	InitMultis();
	Console::instance()->ProgressDone();

	uiCurrentTime = getNormalizedTime();

	// network startup
	Console::instance()->PrepareProgress( "Starting up Network" );
	cNetwork::startup();
	Console::instance()->ProgressDone();

	if( SrvParams->enableLogin() )
        Console::instance()->send( QString( "LoginServer running on port %1\n" ).arg( SrvParams->loginPort() ) );

	if( SrvParams->enableGame() )
        Console::instance()->send( QString( "GameServer running on port %1\n" ).arg( SrvParams->gamePort() ) );

	PyThreadState *_save;

	ScriptManager::instance()->onServerStart();

	serverState = RUNNING;

	Console::instance()->start(); // Startup Console

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
		qApp->processEvents( 40 );

		// Python threading - end
		PyEval_RestoreThread( _save );

		// Update our currenttime
		uiCurrentTime = getNormalizedTime();

		// Perform Threadsafe Actions
		actionMutex.lock();

		while( actionQueue.begin() != actionQueue.end() )
		{
			eActionType type = *(actionQueue.begin());
			actionQueue.erase( actionQueue.begin() );

			switch( type )
			{
			case RELOAD_ACCOUNTS:
				Console::instance()->PrepareProgress( "Reloading Accounts" );
				Accounts::instance()->reload();
				Console::instance()->ProgressDone();
				break;

			case RELOAD_CONFIGURATION:
				Console::instance()->PrepareProgress( "Reloading Configuration" );
				SrvParams->reload();
				Console::instance()->ProgressDone();
				break;

			case RELOAD_SCRIPTS:
				DefManager->reload();

			case RELOAD_PYTHON:
				ScriptManager::instance()->reload();
				break;

			case SAVE_WORLD:
				World::instance()->save();
				break;
			}
		}

		actionMutex.unlock();

		Console::instance()->poll();

		// Process any Network Events
		cNetwork::instance()->poll();

		checkauto();
	}

	serverState = SHUTDOWN;

	ScriptManager::instance()->onServerStop();

	cNetwork::instance()->broadcast( tr( "The server is shutting down." ) );

	Console::instance()->PrepareProgress( tr( "Shutting down network" ) );
	cNetwork::shutdown();
	Console::instance()->ProgressDone();

	SrvParams->flush(); // Save config options

	// Simply emptying of containers, no progressbar needed
	DefManager->unload();

	// Stop Python Interpreter.
	ScriptManager::instance()->unload();
	stopPython();

	Console::instance()->stop(); // Stop the Console

#ifdef Q_OS_UNIX
	pidfile_del( pidfile );
#endif

	return 0;
}
