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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "accounts.h"
#include "ai/ai.h"
#include "ai/ai_mage.h"
#include "basedef.h"
#include "basics.h"
#include "commands.h"
#include "console.h"
#include "getopts.h"
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
#include "sqlite/sqlite.h"

// Qt Includes
#include <qwaitcondition.h>
#include <qptrvector.h>
#include <qapplication.h>
#include <qtextcodec.h>

#if defined(MYSQL_DRIVER)
#if defined(Q_OS_WIN32)
#include <winsock.h>
#endif
#include <mysql.h>
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
	QPtrList<cComponent> components;
	bool running;
	enServerState state;
	bool secure;
	QMutex actionMutex;
	unsigned int time;
	QValueVector<cAction*> actionQueue;
	QApplication *app;

	Private() : running( true ), state( STARTUP ), secure( true ), time( 0 )
	{
	}
};

void cServer::queueAction( enActionType type ) {
	switch (type) {
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

void cServer::queueAction( cAction *action )
{
	if ( d->state == RUNNING )
	{
		QMutexLocker lock( &d->actionMutex );
		d->actionQueue.push_back( action );
	} else {
		delete action; // Delete it right away
	}
}

void cServer::pollQueuedActions()
{
	// Perform Threadsafe Actions
	if ( !d->actionQueue.empty() )
	{
		d->actionMutex.lock();
		cAction *action = *(d->actionQueue.begin());
		d->actionQueue.erase(d->actionQueue.begin());
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
	d->app = 0;

	// Register Components
	registerComponent( Config::instance(), QT_TR_NOOP("configuration"), true, false );

	// We want to start this independently
	//registerComponent(PythonEngine::instance(), "python", false, true, "configuration");

	registerComponent( Maps::instance(), QT_TR_NOOP("maps"), true, false, "configuration" );
	registerComponent( MapObjects::instance(), QT_TR_NOOP("sectormaps"), false, true, "maps" );
	registerComponent( TileCache::instance(), QT_TR_NOOP("tiledata"), true, false, "configuration" );
	registerComponent( MultiCache::instance(), QT_TR_NOOP("multis"), true, false, "configuration" );

	registerComponent( Definitions::instance(), QT_TR_NOOP("definitions"), true, false, "configuration" );
	registerComponent( ScriptManager::instance(), QT_TR_NOOP("scripts"), true, false, "definitions" );
	registerComponent( ContextMenus::instance(), QT_TR_NOOP("contextmenus"), true, false, "scripts" );
	registerComponent( SpawnRegions::instance(), QT_TR_NOOP("spawnregions"), true, false, "definitions" );
	registerComponent( Territories::instance(), QT_TR_NOOP("territories"), true, false, "definitions" );

	// Accounts come before world
	registerComponent( Accounts::instance(), QT_TR_NOOP("accounts"), true, false );
	registerComponent( World::instance(), QT_TR_NOOP("world"), false, true );

	registerComponent( Network::instance(), QT_TR_NOOP("network"), true, false );
}

cServer::~cServer()
{
	delete d->app;
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

bool cServer::run( int argc, char** argv )
{
	// If have no idea where i should put this otherwise
#if defined(Q_OS_UNIX)
	signal(SIGPIPE, SIG_IGN);
#endif

	bool error = false;

	setState( STARTUP );

	d->app = new QApplication ( argc, argv, false );

/*	cItem *item1 = (cItem*)0;
	cItem *item2 = (cItem*)1;
	cItem *item3 = (cItem*)2;

	ContainerContent content;
	for (int i = 0; i < 18; ++i) {
		content.add((cItem*)i);
	}

	content.remove((cItem*)16);
	content.remove((cItem*)15);
	content.remove((cItem*)16);
	content.remove((cItem*)17);

	Console::instance()->send(content.dump());

	ContainerIterator it(content);
	while (!it.atEnd()) {
		Console::instance()->send(QString::number(reinterpret_cast<size_t>(*it)) + "\n");
		if ((*it) == item3) {
			content.remove(item3);
		}
		++it;
	}

	return false;*/

	// Load wolfpack.xml
	Config::instance()->load();
	if ( !Config::instance()->fileState() )
	{
		Console::instance()->log( LOG_WARNING, "Your configuration file [wolfpack.xml] have just been created with default settings.\n");
		Console::instance()->log( LOG_WARNING, "You might have to change it accordingly before running again\n");
	}

#if !defined( QT_NO_TRANSLATION )
	// Start the QT translator
	QString languageFile = Config::instance()->getString( "General", "Language File", QString("wolfpack_") + QTextCodec::locale(), true );
	if ( !languageFile.isEmpty() )
	{
		QTranslator* translator = new QTranslator( qApp );
		if ( !translator->load( languageFile, "." ) )
		{
			Console::instance()->log( LOG_WARNING, "Couldn't load translator.\n" );
			// return false;
		}
		else
		{
			qApp->installTranslator( translator );
		}
	}
#endif

	// Start Python
	PythonEngine::instance()->load();

	// Parse the parameters.
	Getopts::instance()->parse_options( argc, argv );

	// Print a header and useful version informations
	setupConsole();

	// Load all subcomponents
	try
	{
		load();
	}
	catch(wpException &e)
	{
		Console::instance()->log(LOG_ERROR, e.error() + "\n");
		return false;
	}


	try {
		// Open the Worldsave and Account Database drivers.
		if ( Config::instance()->databaseDriver() != "binary" && !PersistentBroker::instance()->openDriver( Config::instance()->databaseDriver() ) )
		{
			Console::instance()->log( LOG_ERROR, tr( "Unknown Worldsave Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->databaseDriver() ) );
			return 1;
		}

		if ( !PersistentBroker::instance()->openDriver( Config::instance()->accountsDriver() ) )
		{
			Console::instance()->log( LOG_ERROR, tr( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
			return 1;
		}

		setState( RUNNING );

		ScriptManager::instance()->onServerStart();	// Notify all scripts about server startup
		Console::instance()->start(); // Notify the console about the server startup

		PyThreadState* _save;
		QWaitCondition niceLevel;
		unsigned char cycles = 0;

		clearProfilingInfo();

		while ( isRunning() ) {
			// Every 10th cycle we sleep for a while and give other threads processing time.
			if ( ++cycles == 10 )
			{
				startProfiling(PF_NICENESS);

				cycles = 0;
				_save = PyEval_SaveThread(); // Python threading - start
				switch ( Config::instance()->niceLevel() )
				{
				case 0:
					break;	// very unnice - hog all cpu time
				case 1:
					if ( Network::instance()->count() != 0 )
						niceLevel.wait( 10 );
					else
						niceLevel.wait( 100 ); break;
				case 2:
					niceLevel.wait( 10 ); break;
				case 3:
					niceLevel.wait( 40 ); break;// very nice
				case 4:
					if ( Network::instance()->count() != 0 )
						niceLevel.wait( 10 );
					else
						niceLevel.wait( 4000 ); break; // anti busy waiting
				case 5:
					if ( Network::instance()->count() != 0 )
						niceLevel.wait( 40 );
					else
						niceLevel.wait( 5000 ); break;
				default:
					niceLevel.wait( 10 ); break;
				}
				qApp->processEvents( 40 );
				PyEval_RestoreThread( _save ); // Python threading - end

				stopProfiling(PF_NICENESS);
			}

			pollQueuedActions();

			d->time = getNormalizedTime(); // Update our currenttime

			try
			{
				Network::instance()->poll();
				Timing::instance()->poll();
				Console::instance()->poll();
			}
			catch ( wpException& e )
			{
				Console::instance()->log( LOG_PYTHON, e.error() + "\n" );
			}
		}

		dumpProfilingInfo();

	} catch (wpException &exception) {
		Console::instance()->log(LOG_ERROR, exception.error() + "\n" );
		error = true;
	} /*catch (...) {
		error = true;
	}*/

	setState( SHUTDOWN );
	Console::instance()->stop();
	ScriptManager::instance()->onServerStop(); // Notify python scripts about shutdown
	Network::instance()->broadcast( tr("The server is shutting down.") );
	unload();

	// Stop Python
	PythonEngine::instance()->unload();

	return !error;
}

void cServer::setupConsole()
{
	Console::instance()->setAttributes( true, false, true, 60, 140, 70, 12, FONT_NOSERIF );
	Console::instance()->send( QString( "\n%1 %2 %3\n\n" ).arg( productString(), productBeta(), productVersion() ) );
	Console::instance()->setAttributes( false, false, false, 0xAF, 0xAF, 0xAF, 0, FONT_FIXEDWIDTH );

	Console::instance()->send( "Copyright (C) 2000-2004 Wolfpack Development Team\n" );
	Console::instance()->send( "Wolfpack Homepage: http://www.wpdev.org/\n" );
	Console::instance()->send( tr("By using this software you agree to the license accompanying this release.\n") );
	Console::instance()->send( tr("Compiled on %1 %2\n").arg( __DATE__ , __TIME__ ) );
	Console::instance()->send( tr("Compiled for Qt %1 (Using: %2 %3)\n").arg(QT_VERSION_STR, qVersion(), qSharedBuild() ? " Shared" : " Static" ) );
	QString pythonBuild = Py_GetVersion();
	pythonBuild = pythonBuild.left( pythonBuild.find( ' ' ) );

#if defined(Py_ENABLE_SHARED)
	pythonBuild += " Shared";
#else
	pythonBuild += " Static";
#endif

#if defined(Py_UNICODE_WIDE)
	QString UnicodeType("UCS-4");
#else
	QString UnicodeType("UCS-2");
#endif
	Console::instance()->send( tr("Compiled for Python %1 %2 (Using: %3)\n").arg(PY_VERSION, UnicodeType, pythonBuild) );
	Console::instance()->send( tr("Compiled with SQLite %1\n" ).arg( SQLITE_VERSION ) );
#if defined (MYSQL_DRIVER)
	Console::instance()->send( tr( "Compiled for MySQL %1 (Using: %2)\n" ).arg( MYSQL_SERVER_VERSION, mysql_get_client_info() ) );
#else
	Console::instance()->send( tr("MySQL Support: disabled\n") );
#endif
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
	Monster_Berserk::registerInFactory();
	Human_Vendor::registerInFactory();
	Human_Stablemaster::registerInFactory();
	Human_Guard::registerInFactory();
	Human_Guard_Called::registerInFactory();
	Normal_Base::registerInFactory();
	Animal_Wild::registerInFactory();
	Animal_Domestic::registerInFactory();

	// Script NPC AI types
	QStringList aiSections = Definitions::instance()->getSections( WPDT_AI );
	QStringList::const_iterator aiit( aiSections.begin() );
	for ( ; aiit != aiSections.end(); ++aiit )
	{
		ScriptAI::registerInFactory( *aiit );
	}

	QPtrList<cComponent>::iterator it( d->components.begin() );
	for ( ; it != d->components.end(); ++it )
	{
		cComponent* component = *it;
		load( component->getName() );
	}

	d->time = getNormalizedTime();
}

void cServer::unload()
{
	QPtrVector<cComponent> vector;
	d->components.toVector( &vector );
	int i;

	for ( i = vector.size() - 1; i >= 0; --i )
	{
		unload( vector[i]->getName() );
	}
}

cComponent* cServer::findComponent( const QString& name )
{
	QPtrList<cComponent>::iterator it( d->components.begin() );
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
	QPtrList<cComponent>::iterator it( d->components.begin() );
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
	QPtrList<cComponent>::iterator it( d->components.begin() );
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
			Console::instance()->sendProgress( tr( "Reloading %1" ).arg( tr(component->getName()) ) );
		}

		component->reload();

		if ( !component->isSilent() )
		{
			Console::instance()->sendDone();
		}
	}

	// Find all components depending on this one and reload them.
	QPtrList<cComponent>::iterator it( d->components.begin() );
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
