/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#include "server.h"
#include "accounts.h"
#include "ai/ai.h"
#include "ai/ai_commoner.h"
#include "ai/ai_mage.h"
#include "ai/ai_bladespirit.h"
#include "ai/ai_energyvortex.h"
#include "exceptions.h"
#include "basedef.h"
#include "basics.h"
#include "commands.h"
#include "console.h"
#include "optionparser.h"
#include "contextmenu.h"
#include "corpse.h"
#include "inlines.h"
#include "log.h"
#include "muls/maps.h"
#include "muls/multiscache.h"
#include "network/network.h"
#include "npc.h"
#include "player.h"
#include "scriptmanager.h"
#include "mapobjects.h"
#include "action.h"
#include "skills.h"
#include "spawnregions.h"
#include "serverconfig.h"
#include "territories.h"
#include "muls/tilecache.h"
#include "timing.h"
#include "verinfo.h"
#include "world.h"
#include "definitions.h"
#include "multi.h"
#include "persistentbroker.h"
#include "profile.h"
#include "python/engine.h"
#include "verinfo.h"
#include "network/network.h"

// Qt Includes
#include <QWaitCondition>
#include <QCoreApplication>
#include <QTextCodec>
#include <QLocale>
#include <QTranslator>
#include <QDir>

#if defined(MYSQL_DRIVER)
# if defined(Q_OS_WIN32)
#  include <winsock.h>
# endif
# include <mysql.h>
#endif

#if defined(Q_OS_UNIX)
#include <signal.h>
#endif

#include "content.h"

cComponent::cComponent()
{
	this->loaded = false;
}

cComponent::~cComponent()
{
}

void cComponent::reload()
{
	unload();
	load();
}

void cComponent::load()
{
	loaded = true;
}

void cComponent::unload()
{
	loaded = false;
}

class cServer::Private
{
public:
	QList<cComponent*> components;
	bool running;
	enServerState state;
	bool secure;
	QMutex actionMutex;
	unsigned int time;
	QList<cAction*> actionQueue;

	Private() : running( true ), state( STARTUP ), secure( true ), time( 0 )
	{
	}
};

void cServer::queueAction( enActionType type )
{
	switch ( type )
	{
	case RELOAD_SCRIPTS:
		queueAction( new cActionReloadScripts );
		break;
	case RELOAD_PYTHON:
		queueAction( new cActionReloadPython );
		break;
	case RELOAD_ACCOUNTS:
		queueAction( new cActionReloadAccounts );
		break;
	case RELOAD_CONFIGURATION:
		queueAction( new cActionReloadConfiguration );
		break;
	case SAVE_WORLD:
		queueAction( new cActionSaveWorld );
		break;
	case SAVE_ACCOUNTS:
		queueAction( new cActionSaveAccounts );
		break;
	default:
		break;
	}
}

void cServer::queueAction( cAction* action )
{
	if ( d->state == RUNNING )
	{
		QMutexLocker lock( &d->actionMutex );
		d->actionQueue.push_back( action );
	}
	else
	{
		delete action; // Delete it right away
	}
}

void cServer::pollQueuedActions()
{
	// Perform Threadsafe Actions
	if ( !d->actionQueue.empty() )
	{
		d->actionMutex.lock();
		cAction *action = *( d->actionQueue.begin() );
		d->actionQueue.erase( d->actionQueue.begin() );
		d->actionMutex.unlock();

		try
		{
			action->execute();
		}
		catch ( wpException& e )
		{
			Console::instance()->log( LOG_PYTHON, e.error() + "\n" );
		}

		delete action;
	}
}

#if defined(CRASHHANDLER)
#include "bugreport/crashhandler.h"

LONG CALLBACK exceptionCatcher( _EXCEPTION_POINTERS* exception )
{
	QString message = GetFaultReason( exception );
	message += "\n";
	message += "Stack Trace:\n";

	DWORD options = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
	const char* buff = GetFirstStackTraceString( options, exception );
	do
	{
		message.append( buff );
		message.append( "\n" );
		buff = GetNextStackTraceString( options, exception );
	}
	while ( buff );

	throw wpException( " " + message );
}
#endif

cServer::cServer()
{
#if defined(CRASHHANDLER)
	SetUnhandledExceptionFilter( exceptionCatcher );
#endif

	d = new Private;
}

cServer::~cServer()
{
	delete d;
}

bool cServer::isRunning()
{
	return d->running;
}

void cServer::cancel()
{
	d->running = false;
}

void cServer::setSecure( bool secure )
{
	d->secure = secure;
}

bool cServer::getSecure()
{
	return d->secure;
}

void myMessageOutput( QtMsgType /*type*/, const char */*msg*/ )
{
 /*   switch ( type ) {
        case QtDebugMsg:
			// This is crazy...
			// Log->print(LOG_DEBUG, tr("QT Debug: %1\n").arg(msg));
            break;
		case QtWarningMsg:
			Console::instance()->log(LOG_WARNING, msg);
            break;
        case QtFatalMsg:
			Console::instance()->log(LOG_ERROR, msg);
			break;
		default:
			Console::instance()->log(LOG_ERROR, msg);
			break;
    }*/
}

#include "python/pyprofiler.h"
#include <QSqlDatabase>

void cServer::run()
{
#if !defined( DEBUG )
    //qInstallMsgHandler(myMessageOutput);
    qInstallMessageHandler(0);
#endif
	// If have no idea where i should put this otherwise
#if defined(Q_OS_UNIX)
	signal( SIGPIPE, SIG_IGN );
#endif

	QEventLoop eventLoop;

	CommandLineOptions::instance()->addOption("-c", "configFile", "Alternative config file to use. Default=wolfpack.xml", true);
	CommandLineOptions::instance()->addOption("-d", "isDaemon", OptionParser::Posix, "Run as a daemon.");
	CommandLineOptions::instance()->addOption("-p", "pidFile", OptionParser::Posix, "Specify a pid file to use", true);
	CommandLineOptions::instance()->addOption("-a", "cdTo", "Specify the working directory to run", true );

	CommandLineOptions::instance()->parse();

	if ( CommandLineOptions::instance()->value("cdTo").isValid() )
		QDir::setCurrent( CommandLineOptions::instance()->value("cdTo").toString() );

	// Register Components
	registerComponent( Config::instance(), QT_TR_NOOP( "configuration" ), true, false );

	// We want to start this independently
	//registerComponent(PythonEngine::instance(), "python", false, true, "configuration");

	registerComponent( Maps::instance(), QT_TR_NOOP( "maps" ), true, false, "configuration" );
	registerComponent( MapObjects::instance(), QT_TR_NOOP( "sectormaps" ), false, true, "maps" );
	registerComponent( TileCache::instance(), QT_TR_NOOP( "tiledata" ), true, false, "configuration" );
	registerComponent( MultiCache::instance(), QT_TR_NOOP( "multis" ), true, false, "configuration" );

	registerComponent( Definitions::instance(), QT_TR_NOOP( "definitions" ), true, false, "configuration" );
	registerComponent( ScriptManager::instance(), QT_TR_NOOP( "scripts" ), true, false, "definitions" );
	registerComponent( ContextMenus::instance(), QT_TR_NOOP( "contextmenus" ), true, false, "scripts" );
	registerComponent( SpawnRegions::instance(), QT_TR_NOOP( "spawnregions" ), true, false, "definitions" );
	registerComponent( Territories::instance(), QT_TR_NOOP( "territories" ), true, false, "definitions" );

	// Accounts come before world
	registerComponent( Accounts::instance(), QT_TR_NOOP( "accounts" ), true, false );
	registerComponent( World::instance(), QT_TR_NOOP( "world" ), false, true );

	registerComponent( Network::instance(), QT_TR_NOOP( "network" ), true, false );

	setState( STARTUP );

	// Set the default conversion codec (This is what OSI is using)
	// ISO-8859-15 (MIB: 111)
#if defined(Q_OS_UNIX)
	QTextCodec::setCodecForLocale( QTextCodec::codecForMib(111) );
	QTextCodec::setCodecForCStrings( QTextCodec::codecForMib(111) );
#endif

	// Load wolfpack.xml
#if defined( WP_USE_ALTERNATIVE_CONFIG_FILE )
	Config::instance()->setFileName( WP_USE_ALTERNATIVE_CONFIG_FILE );
#endif

	if ( CommandLineOptions::instance()->value("configFile").isValid() )
		Config::instance()->setFileName( CommandLineOptions::instance()->value("configFile").toString() );

	Config::instance()->load();

#if !defined( QT_NO_TRANSLATION )
	// Start the QT translator
	if ( QLocale::system().language() != QLocale::English )
	{
		QString languageFile = Config::instance()->getString( "General", "Language File", QString( "wolfpack_" ) + QLocale::system().name() + QString( ".qm" ), true );
		if ( !languageFile.isEmpty() && QFile::exists( languageFile ) )
		{
			QTranslator* translator = new QTranslator( this );
			if ( !translator->load( languageFile, "." ) )
			{
				Console::instance()->log( LOG_WARNING, "Couldn't load translator.\n" );
				delete translator;
				// return false;
			}
			else
			{
				QCoreApplication::installTranslator( translator );
			}
		}
		else
		{
			Console::instance()->log( LOG_WARNING, "Couldn't find Language File: " + languageFile + "\n" );
		}
	}
#endif

	// After trying to at least load language files for the benefict of non-English ppl.
	if ( !Config::instance()->fileState() )
	{
		Console::instance()->log( LOG_WARNING, tr("Your configuration file [wolfpack.xml] have just been created with default settings.\n") );
		Console::instance()->log( LOG_WARNING, tr("You might have to change it accordingly before running again\n") );
	}

	// Start Python
	PythonEngine::instance()->load();

	// Print a header and useful version informations
	setupConsole();

	// Load all subcomponents
	try
	{
		load();
	}
	catch ( wpException& e )
	{
		Console::instance()->log( LOG_ERROR, e.error() + "\n" );
		cancel();
	}


	try
	{
		// Open the Worldsave and Account Database drivers.
		if ( Config::instance()->databaseDriver() != "binary" && !PersistentBroker::instance()->openDriver( Config::instance()->databaseDriver() ) )
		{
			throw wpException( tr( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->databaseDriver() ) );
		}

		if ( !PersistentBroker::instance()->openDriver( Config::instance()->accountsDriver() ) )
		{
			throw wpException( tr( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
		}

		setState( RUNNING );

		ScriptManager::instance()->onServerStart();	// Notify all scripts about server startup
		Console::instance()->start(); // Notify the console about the server startup

		PyThreadState* _save;
		unsigned char cycles = 0;

		clearProfilingInfo();

		while ( isRunning() )
		{
			// Every 10th cycle we sleep for a while and give other threads processing time.
			if ( ++cycles == 10 )
			{
				startProfiling( PF_NICENESS );

				cycles = 0;
				_save = PyEval_SaveThread(); // Python threading - start
				switch ( Config::instance()->niceLevel() )
				{
				case 0:
					break;	// very unnice - hog all cpu time
				case 1:
					if ( Network::instance()->count() != 0 )
						msleep( 10 );
					else
						msleep( 100 ); break;
				case 2:
					msleep( 10 ); break;
				case 3:
					msleep( 40 ); break;// very nice
				case 4:
					if ( Network::instance()->count() != 0 )
						msleep( 10 );
					else
						msleep( 4000 ); break; // anti busy waiting
				case 5:
					if ( Network::instance()->count() != 0 )
						msleep( 40 );
					else
						msleep( 5000 ); break;
				default:
					msleep( 10 ); break;
				}
				PyEval_RestoreThread( _save ); // Python threading - end

				stopProfiling( PF_NICENESS );
			}
			eventLoop.processEvents();

			pollQueuedActions();

			//d->time = getNormalizedTime(); // Update our currenttime
			refreshTime(); // Update our currenttime

			try
			{
				Timing::instance()->poll();
				Console::instance()->poll();
			}
			catch ( wpException& e )
			{
				Console::instance()->log( LOG_PYTHON, e.error() + "\n" );
			}
		}

		dumpProfilingInfo();
	}
	catch ( wpException& exception )
	{
		Console::instance()->log( LOG_ERROR, exception.error() + "\n" );
	} /*catch (...) {
	  	error = true;
	  }*/

	setState( SHUTDOWN );
	Console::instance()->stop();
	ScriptManager::instance()->onServerStop(); // Notify python scripts about shutdown
	Network::instance()->broadcast( tr( "The server is shutting down." ) );
	unload();

	// Stop Python
	PythonEngine::instance()->unload();

	return;
}

void cServer::setupConsole()
{
	Console::instance()->setAttributes( true, false, true, 60, 140, 70, 12, FONT_NOSERIF );
	Console::instance()->send( QString( "\n%1 %2 %3\n\n" ).arg( productString(), productBeta(), productVersion() ) );
	Console::instance()->setAttributes( false, false, false, 0xAF, 0xAF, 0xAF, 0, FONT_FIXEDWIDTH );

	Console::instance()->send( "Copyright (C) 2000-2016 Wolfpack Development Team\n" );
	Console::instance()->send( QString( "Wolfpack Homepage: %1 \n" ).arg( DEFAULTWEBPAGE ));
	Console::instance()->send( tr( "By using this software you agree to the license accompanying this release.\n" ) );
	Console::instance()->send( tr( "Compiled on %1 %2\n" ).arg( __DATE__, __TIME__ ) );
	Console::instance()->send( tr( "Compiled for Qt %1 (Using: %2 %3)\n" ).arg( QT_VERSION_STR, qVersion(), qSharedBuild() ? " Shared" : " Static" ) );

	QStringList sqlDrivers = QSqlDatabase::drivers();
	Console::instance()->send( tr( "Available SQL Drivers: " ) );

	if (sqlDrivers.isEmpty()) {
		Console::instance()->send( tr("none") );
	} else {
		Console::instance()->send( sqlDrivers.join(" ") );
	}

	Console::instance()->send("\n");

	QString pythonBuild = cPythonEngine::version();
	pythonBuild = pythonBuild.left( pythonBuild.indexOf( ' ' ) );

#if defined(Py_ENABLE_SHARED)
	pythonBuild += " Shared";
#else
	pythonBuild += " Static";
#endif

#if defined(Py_UNICODE_WIDE)
	QString UnicodeType( "UCS-4" );
#else
	QString UnicodeType( "UCS-2" );
#endif
	Console::instance()->send( tr( "Compiled for Python %1 %2 (Using: %3)\n" ).arg( PY_VERSION, UnicodeType, pythonBuild ) );
	Console::instance()->send( "\n" );
	QString consoleTitle = QString( "%1 %2 %3" ).arg( productString(), productBeta(), productVersion() );
	Console::instance()->setConsoleTitle( consoleTitle );
}

void cServer::load()
{
	// NPC AI types
	Monster_Aggressive_L0::registerInFactory();
	Monster_Aggressive_L1::registerInFactory();
	Monster_Mage::registerInFactory();
	Commoner::registerInFactory();
	Monster_BladeSpirit::registerInFactory();
	Monster_EnergyVortex::registerInFactory();
	Monster_Berserk::registerInFactory();
	Human_Vendor::registerInFactory();
	Human_Stablemaster::registerInFactory();
	Human_Guard::registerInFactory();
	Human_Guard_Called::registerInFactory();
	Normal_Base::registerInFactory();
	Animal_Wild::registerInFactory();
	Animal_Domestic::registerInFactory();
	Animal_Predator::registerInFactory();

	// Script NPC AI types
	QStringList aiSections = Definitions::instance()->getSections( WPDT_AI );
	QStringList::const_iterator aiit( aiSections.begin() );
	for ( ; aiit != aiSections.end(); ++aiit )
	{
		ScriptAI::registerInFactory( *aiit );
	}

	QList<cComponent*>::iterator it( d->components.begin() );
	for ( ; it != d->components.end(); ++it )
	{
		cComponent* component = *it;
		load( component->getName() );
	}

	d->time = getNormalizedTime();
}

void cServer::unload()
{
	for ( int i = d->components.size() - 1; i >= 0; --i )
	{
		unload( d->components[i]->getName() );
	}
}

cComponent* cServer::findComponent( const QString& name )
{
	QList<cComponent*>::iterator it( d->components.begin() );
	for ( ; it != d->components.end(); ++it )
	{
		cComponent* component = *it;
		if ( component->getName() == name )
		{
			return component;
		}
	}

	return 0;
}

void cServer::registerComponent( cComponent* component, const QString& name, bool reloadable, bool silent, const QString& depends )
{
	component->setName( name );
	component->setRelodable( reloadable );
	component->setSilent( silent );
	component->setDepends( depends );

	if ( component->getName().isEmpty() )
	{
		throw wpException( "Trying to register a component without a name." );
	}

	if ( findComponent( component->getName() ) != 0 )
	{
		throw wpException( QString( "There already is a component with this name: %1" ).arg( component->getName() ) );
	}

	d->components.append( component );
}

void cServer::load( const QString& name )
{
	cComponent* component = findComponent( name );

	if ( !component )
	{
		throw wpException( QString( "Trying to load an unknown component: %1" ).arg( name ) );
	}

	if ( component->isLoaded() )
	{
		return;
	}

	if ( !component->isSilent() )
	{
		Console::instance()->sendProgress( tr( "Loading %1" ).arg( component->getName() ) );
	}

	component->load();

	if ( !component->isSilent() )
	{
		Console::instance()->sendDone();
	}

	// Find all components depending on this one and load them.
	QList<cComponent*>::iterator it( d->components.begin() );
	for ( ; it != d->components.end(); ++it )
	{
		cComponent* subcomponent = *it;
		if ( subcomponent->getDepends() == component->getName() )
		{
			load( subcomponent->getName() );
		}
	}
}

void cServer::unload( const QString& name )
{
	cComponent* component = findComponent( name );

	if ( !component )
	{
		throw wpException( QString( "Trying to unload an unknown component: %1" ).arg( name ) );
	}

	if ( !component->isLoaded() )
	{
		return;
	}

	// Find all components depending on this one and unload them.
	QList<cComponent*>::iterator it( d->components.begin() );
	for ( ; it != d->components.end(); ++it )
	{
		cComponent* subcomponent = *it;
		if ( subcomponent->getDepends() == component->getName() )
		{
			unload( subcomponent->getName() );
		}
	}

	if ( !component->isSilent() )
	{
		Console::instance()->sendProgress( tr( "Unloading %1" ).arg( component->getName() ) );
	}

	component->unload();

	if ( !component->isSilent() )
	{
		Console::instance()->sendDone();
	}
}

void cServer::reload( const QString& name )
{
	Server::instance()->setState( SCRIPTRELOAD );

	cComponent* component = findComponent( name );

	if ( !component )
	{
		throw wpException( QString( "Trying to reload an unknown component: %1" ).arg( name ) );
	}

	if ( component->isReloadable() )
	{
		if ( !component->isSilent() )
		{
			Console::instance()->sendProgress( tr( "Reloading %1" ).arg( tr( component->getName().toLatin1() ) ) );
		}

		component->reload();

		if ( !component->isSilent() )
		{
			Console::instance()->sendDone();
		}
	}

	// Find all components depending on this one and reload them.
	QList<cComponent*>::iterator it( d->components.begin() );
	for ( ; it != d->components.end(); ++it )
	{
		cComponent* subcomponent = *it;
		if ( !subcomponent->isReloadable() )
		{
			continue;
		}

		if ( subcomponent->getDepends() == component->getName() )
		{
			reload( subcomponent->getName() );
		}
	}

	Server::instance()->setState( RUNNING );
}

void cServer::setState( enServerState state )
{
	Console::instance()->notifyServerState( state );
	d->state = state;
}

unsigned int cServer::time()
{
	return d->time;
}

void cServer::refreshTime()
{
	d->time = getNormalizedTime();
}

enServerState cServer::getState()
{
	return d->state;
}
