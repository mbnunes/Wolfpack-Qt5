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

#if !defined(__SERVER_H__)
#define __SERVER_H__

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include "singleton.h"
#include "objectdef.h"

enum enServerState
{
	STARTUP			= 0,
	RUNNING,
	SCRIPTRELOAD,
	SHUTDOWN
};

class cAction;

enum enActionType
{
	RELOAD_SCRIPTS = 0,
	RELOAD_PYTHON,
	RELOAD_ACCOUNTS,
	RELOAD_CONFIGURATION,
	SAVE_WORLD,
	SAVE_ACCOUNTS
};

/*!
	\brief A server component.
*/
class WPEXPORT cComponent
{
private:
	QString depends;
	QString name;
	bool silent;
	bool reloadable;
	bool loaded;

public:
	cComponent();
	virtual ~cComponent();

	virtual void load();
	virtual void unload();
	virtual void reload();

	inline const QString& getName() const
	{
		return name;
	}

	inline void setName( const QString& name )
	{
		this->name = name;
	}

	inline const QString& getDepends() const
	{
		return depends;
	}

	inline void setDepends( const QString& depends )
	{
		this->depends = depends;
	}

	inline bool isReloadable() const
	{
		return reloadable;
	}

	inline void setRelodable( bool reloadable )
	{
		this->reloadable = reloadable;
	}

	inline bool isSilent() const
	{
		return silent;
	}

	inline void setSilent( bool silent )
	{
		this->silent = silent;
	}

	inline bool isLoaded() const
	{
		return loaded;
	}
};

class cServer
{
	OBJECTDEF(cServer)
private:
	class Private;
	Private* d;

	// Disallow copy constructor
	cServer( const cServer& )
	{
	}

	// Load a specific component
	void load( const QString& name );

	// Unload a specific component
	void unload( const QString& name );

	// Load all components
	void load();

	// Unload all components
	void unload();

	// Setup the console window
	void setupConsole();
	void pollQueuedActions();
public:
	cServer();
	~cServer();

	void queueAction( cAction *action );
	void queueAction( enActionType type );
	void setState( enServerState state );
	enServerState getState();

	void setSecure( bool secure );
	bool getSecure();

	bool isRunning();
	void cancel();

	// Return current server time
	unsigned int time();

	// Refresh current server time
	void refreshTime();

	// Returns false if an error occured
	bool run( int argc, char** argv );

	// Reload a specific component
	void reload( const QString& name );

	// Component registry
	cComponent* findComponent( const QString& name );
	void registerComponent( cComponent* component, const QString& name, bool reloadable = true, bool silent = false, const QString& depends = QString::null );
};

typedef SingletonHolder<cServer> Server;

#endif
