/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#include "accounts.h"
#include "ai/ai.h"
#include "basedef.h"
#include "basics.h"
#include "commands.h"
#include "console.h"
#include "getopts.h"
#include "contextmenu.h"
#include "corpse.h"
#include "inlines.h"
#include "log.h"
#include "maps.h"
#include "multiscache.h"
#include "network.h"
#include "npc.h"
#include "player.h"
#include "scriptmanager.h"
#include "sectors.h"
#include "skills.h"
#include "spawnregions.h"
#include "config.h"
#include "territories.h"
#include "tilecache.h"
#include "Timing.h"
#include "verinfo.h"
#include "wolfpack.h"
#include "world.h"
#include "definitions.h"
#include "multi.h"
#include "server.h"

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
#include <qglobal.h>

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
			pName = PyString_FromString( param.left(param.length() - 3).latin1() );
			/* Error checking of pName left out */

			pModule = PyImport_Import(pName);
			Py_DECREF(pName);

			if (pModule != NULL)
			{
				pDict = PyModule_GetDict(pModule);
				/* pDict is a borrowed reference */

				pFunc = PyDict_GetItemString(pDict, "main");
				/* pFun: Borrowed reference */

				if (pFunc && PyCallable_Check(pFunc))
				{
					pArgs = PyTuple_New(0);
					pValue = PyObject_CallObject(pFunc, pArgs);
					Py_DECREF(pArgs);
					if (pValue != NULL)
					{
						printf("Result of call: %ld\n", PyInt_AsLong(pValue));
						Py_DECREF(pValue);
					}
					else
					{
						Py_DECREF(pModule);
						PyErr_Print();
						fprintf(stderr,"Call failed\n");
						return false;
					}
					/* pDict and pFunc are borrowed and must not be Py_DECREF-ed */
				}
				else
				{
					if (PyErr_Occurred())
						PyErr_Print();
				}
				Py_DECREF(pModule);
			}
			else
			{
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
/*	Getopts::instance()->parse_options( argc, argv );
	unsigned int i;

	keeprun = 1;
	QApplication app( argc, argv, false ); // we need one instance
	QTranslator translator( 0 ); // must be valid thru app life.

	Server::instance()->setState(STARTUP);

	Console::instance()->setAttributes( true, false, true, 60, 140, 70, 12, FONT_NOSERIF );
	Console::instance()->send(QString( "\n%1 %2 %3\n\n" ).arg(productString(), productBeta(), productVersion()));
	Console::instance()->setAttributes( false, false, false, 0xAF, 0xAF, 0xAF, 0, FONT_FIXEDWIDTH );

	Console::instance()->send("Copyright (C) 2000-2004 Wolfpack Development Team\n");
	Console::instance()->send("Wolfpack Homepage: http://www.wpdev.org/\n");
	Console::instance()->send("By using this software you agree to the license accompanying this release.\n");
	Console::instance()->send("Compiled on " __DATE__ " " __TIME__ "\n");
	Console::instance()->send("Compiled for QT " QT_VERSION_STR " (Using: ");
	Console::instance()->send(qVersion());
	Console::instance()->send(qSharedBuild() ? " Shared" : " Static");
	Console::instance()->send(")\n");
	QString pythonBuild = Py_GetVersion();
	pythonBuild = pythonBuild.left(pythonBuild.find(' '));

#if defined(Py_ENABLE_SHARED)
	pythonBuild += " Shared";
#else
	pythonBuild += " Static";
#endif

	Console::instance()->send("Compiled for Python " PY_VERSION " (Using: ");
	Console::instance()->send(pythonBuild + ")\n\n");

	QString consoleTitle = QString("%1 %2 %3").arg(productString(), productBeta(), productVersion());
	Console::instance()->setConsoleTitle( consoleTitle );

	return 0;

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
	QString languageFile = Config::instance()->getString( "General", "Language File", "", true );
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
	Definitions::instance()->load();
	Console::instance()->send( "\n" );

	// Scriptmanager can't be in the try{} block because it sometimes throws firstchance exceptions
	// we don't like
	ScriptManager::instance()->load();
	Console::instance()->send( "\n" );

	// Try to load several data files
#if !defined(_DEBUG)
	try
	{
#endif
		// Load some MUL Data
		Console::instance()->send( "Loading muls...\n" );
		TileCache::instance()->load( Config::instance()->mulPath() );
		MultiCache::instance()->load( Config::instance()->mulPath() );

		Maps::instance()->registerMap(0, "map0.mul", 768, 512, "statics0.mul", "staidx0.mul");
		Maps::instance()->registerMap(1, "map0.mul", 768, 512, "statics0.mul", "staidx0.mul");
		Maps::instance()->registerMap(2, "map2.mul", 288, 200, "statics2.mul", "staidx2.mul");
		Maps::instance()->registerMap(3, "map3.mul", 320, 256, "statics3.mul", "staidx3.mul");

		// For each map we register, register a GridMap as well
		MapObjects::instance()->addMap( 0, 6144, 4096 );
		MapObjects::instance()->addMap( 1, 6144, 4096 );
		MapObjects::instance()->addMap( 2, 2304, 1600 );
		MapObjects::instance()->addMap( 3, 2560, 2048 );

		Console::instance()->send( "Loading skills...\n" );
		Skills::instance()->load();

		Console::instance()->send( "Loading accounts...\n" );
		Accounts::instance()->load();

		Console::instance()->send( "Loading ip blocking rules...\n" );
		Network::instance()->load();

		Console::instance()->send( "Loading regions...\n" );
		Territories::instance()->load();

		Console::instance()->send( "Loading spawn regions...\n" );
		SpawnRegions::instance()->load();

		Console::instance()->send( "Loading resources...\n" );
		Resources::instance()->load();

		Console::instance()->send( "Loading contextmenus...\n" );
		ContextMenus::instance()->reload();

		Console::instance()->send( "\n" );
#if !defined(_DEBUG)
	}
	catch( wpException &exception )
	{
		Console::instance()->log( LOG_ERROR, exception.error() + "\n" );
		return 1;
	}
	catch( ... )
	{
		Console::instance()->log( LOG_ERROR, "Unknown error while loading data files.\n" );
		return 1;
	}
#endif

	keeprun = 1;
	secure = 1;

	// initial randomization call
	srand(uiCurrentTime);

	if (!PersistentBroker::instance()->openDriver( Config::instance()->databaseDriver())) {
		Console::instance()->log( LOG_ERROR, QString( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml").arg( Config::instance()->databaseDriver() ) );
		return 1;
	}

	if (!PersistentBroker::instance()->openDriver( Config::instance()->accountsDriver())) {
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml").arg( Config::instance()->accountsDriver() ) );
		return 1;
	}

	// Registers our Built-in types into factory.
	cPlayer::registerInFactory();
	cMulti::registerInFactory();
	cNPC::registerInFactory();
	cItem::registerInFactory();
	cCorpse::registerInFactory();
	cResourceItem::registerInFactory();

	// NPC AI types
	Monster_Aggressive_L0::registerInFactory();
	Monster_Aggressive_L1::registerInFactory();
	Monster_Berserk::registerInFactory();
	Human_Vendor::registerInFactory();
	Human_Stablemaster::registerInFactory();
	Human_Guard::registerInFactory();
	Human_Guard_Called::registerInFactory();
	Normal_Base::registerInFactory();
	Animal_Wild::registerInFactory();
	Animal_Domestic::registerInFactory();

	// Script NPC AI types
	QStringList aiSections = Definitions::instance()->getSections(WPDT_AI);
	QStringList::const_iterator aiit = aiSections.begin();
	while (aiit != aiSections.end()) {
		ScriptAI::registerInFactory(*aiit);
		++aiit;
	}*/

	/*try {*/
		World::instance()->load();
	/*} catch( QString &error ) {
		Console::instance()->log( LOG_ERROR, error );
		return 1;
	} catch (wpException e) {
		Console::instance()->log(LOG_ERROR, e.error() + "\n");
	} catch (...) {
		Console::instance()->log( LOG_ERROR, "An unknown error occured while loading the world.\n" );
		return 1;
	}*/

/*	uiCurrentTime = getNormalizedTime();

	// network startup
	Network::instance()->startup();

	if (Config::instance()->enableLogin()) {
        Console::instance()->send( QString( "LoginServer running on port %1\n" ).arg( Config::instance()->loginPort() ) );
		if (Config::instance()->serverList().size() < 1)
			Console::instance()->log( LOG_WARNING, "LoginServer enabled but there no Game server entries found\n Check your wolfpack.xml settings" );
	}

	if (Config::instance()->enableGame())
        Console::instance()->send( QString( "GameServer running on port %1\n" ).arg( Config::instance()->gamePort() ) );

	PyThreadState *_save;
	QWaitCondition niceLevel;
	unsigned char cycles = 0;

	ScriptManager::instance()->onServerStart();
	Console::instance()->start(); // Startup Console

	// This is our main loop
	while (keeprun) {
		++cycles;

		// Update our currenttime
		uiCurrentTime = getNormalizedTime();

		try {
			// Every 10th cycle we sleep for a while and give other threads processing time.
			if (cycles == 10) {
				cycles = 0;

				// Python threading - start
				_save = PyEval_SaveThread();

				switch( Config::instance()->niceLevel() )
				{
					case 0: break;	// very unnice - hog all cpu time
					case 1: if ( Network::instance()->count() != 0) niceLevel.wait(10); else niceLevel.wait(100); break;
					case 2: niceLevel.wait(10); break;
					case 3: niceLevel.wait(40); break;// very nice
					case 4: if ( Network::instance()->count() != 0 ) niceLevel.wait(10); else niceLevel.wait(4000); break; // anti busy waiting
					case 5: if ( Network::instance()->count() != 0 ) niceLevel.wait(40); else niceLevel.wait(5000); break;
					default: niceLevel.wait(10); break;
				}
				qApp->processEvents( 40 );

				// Python threading - end
				PyEval_RestoreThread( _save );

				// Unlock the main mutex to give the gui time for processing data
			}
		} catch (wpException e) {
			Console::instance()->log(LOG_PYTHON, e.error() + "\n");
		}

		// Perform Threadsafe Actions
		if (!actionQueue.empty()) {
			actionMutex.lock();
			while (actionQueue.begin() != actionQueue.end()) {
				eActionType type = *(actionQueue.begin());
				actionQueue.erase( actionQueue.begin() );

				switch (type) {
				case RELOAD_ACCOUNTS:
					Console::instance()->sendProgress( "Reloading Accounts" );
					lockDataMutex();
					try {
						Accounts::instance()->reload();
					} catch(wpException e) {
						Console::instance()->log(LOG_PYTHON, e.error() + "\n");
					}
					unlockDataMutex();
					Console::instance()->sendDone();
					break;

				case RELOAD_CONFIGURATION:
					Console::instance()->sendProgress( "Reloading Configuration" );
					lockDataMutex();
					try {
						Config::instance()->reload();
					} catch(wpException e) {
						Console::instance()->log(LOG_PYTHON, e.error() + "\n");
					}
					unlockDataMutex();
					Console::instance()->sendDone();
					break;

				case RELOAD_SCRIPTS:
					try {
						Definitions::instance()->reload();
					} catch(wpException e) {
						Console::instance()->log(LOG_PYTHON, e.error() + "\n");
					}

				case RELOAD_PYTHON:
					try {
						ScriptManager::instance()->reload();
					} catch(wpException e) {
						Console::instance()->log(LOG_PYTHON, e.error() + "\n");
					}
					break;

				case SAVE_WORLD:
					lockDataMutex();
					try {
						World::instance()->save();
					} catch(wpException e) {
						Console::instance()->log(LOG_PYTHON, e.error() + "\n");
					}
					unlockDataMutex();
					break;
				}
			}
			actionMutex.unlock();
		}

		// See if we should release our data lock for a while.
		Console::instance()->poll();

		lockDataMutex();

		try {
			Network::instance()->poll();
		} catch(wpException e) {
			Console::instance()->log(LOG_PYTHON, e.error() + "\n");
		}

		try {
			Timing::instance()->poll();
		} catch(wpException e) {
			Console::instance()->log(LOG_PYTHON, e.error() + "\n");
		}

		unlockDataMutex();
	}

	Server::instance()->setState(SHUTDOWN);

	ScriptManager::instance()->onServerStop();

	Network::instance()->broadcast( tr( "The server is shutting down." ) );
	Network::instance()->shutdown();

	Config::instance()->flush(); // Save config options

	// Stop Python Interpreter.
	ScriptManager::instance()->unload();
	stopPython();

	Console::instance()->stop(); // Stop the Console
*/
	return 0;
}
