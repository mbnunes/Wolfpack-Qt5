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

#include "persistentbroker.h"
#include "persistentobject.h"

#include "dbdriver.h"
#include "console.h"
#include "globals.h"
#include "log.h"

// Qt Includes
#include <qstring.h>

#include <list>

struct stDeleteItem
{
	QString tables;
	QString conditions;
};

class PersistentBrokerPrivate
{
public:
	PersistentBrokerPrivate() : connection(0) {}
	~PersistentBrokerPrivate() { delete connection; }

	cDBDriver* connection;
	bool sqlite;
	std::list< stDeleteItem > deleteQueue;
};

PersistentBroker::PersistentBroker() : d( new PersistentBrokerPrivate )
{
}

PersistentBroker::~PersistentBroker()
{
	delete d;
}

bool PersistentBroker::openDriver( const QString& driver )
{
	if( d->connection != 0 )
	{
		d->connection->close();
		delete d->connection;
		d->connection = 0;
	}

	if( driver == "sqlite" )
	{
		d->connection = new cSQLiteDriver();
		d->sqlite = true;
	}
	else if( driver == "mysql" )
	{
#ifdef MYSQL_DRIVER
		d->connection = new cMySQLDriver;
		d->sqlite = false;
#else
		throw QString( "Sorry, you have to define MYSQL_DRIVER to make wolfpack work with MySQL.\n" );
#endif
	}

	if ( !d->connection )
		return false;

	return true;
}

bool PersistentBroker::connect( const QString& host, const QString& db, const QString& username, const QString& password )
{
	if (!d->connection)
		return false;

	// This does nothing but a little test-connection
	d->connection->setDatabaseName( db );
	d->connection->setUserName( username );
	d->connection->setPassword( password );
	d->connection->setHostName( host );
	
	if( !d->connection->open() )
		return false;

	return true;
}

void PersistentBroker::disconnect()
{
	if( d->connection )
		d->connection->close();
}

bool PersistentBroker::saveObject( PersistentObject* object )
{
	object->save();
	return true;
}

bool PersistentBroker::deleteObject( PersistentObject* object )
{
	return object->del();

/*	static const bool hasTransaction = connection->driver()->hasFeature(QSqlDriver::Transactions);
	if ( hasTransaction )
		connection->transaction();
	if ( object->del() )
	{
		if ( hasTransaction )
			connection->commit();
		return true;
	}
	else
	{
		if ( hasTransaction )
			connection->rollback();
		return false;
	}
	*/
}

bool PersistentBroker::executeQuery( const QString& query )
{
	if( !d->connection )
		throw QString( "PersistentBroker not connected to database." );

	//qWarning( query );
	bool result = d->connection->exec(query);
	if( !result )
	{
		Console::instance()->log( LOG_ERROR, d->connection->error() );
	}
	return result;
}

cDBDriver* PersistentBroker::driver() const
{
	return d->connection;
}

cDBResult PersistentBroker::query( const QString& query )
{
	if( !d->connection )
		throw QString( "PersistentBroker not connected to database." );

	return d->connection->query( query );
}

void PersistentBroker::clearDeleteQueue()
{
	d->deleteQueue.clear();
}

void PersistentBroker::flushDeleteQueue()
{
	std::list< stDeleteItem >::iterator iter;
	for( iter = d->deleteQueue.begin(); iter != d->deleteQueue.end(); ++iter )
	{
		executeQuery( "DELETE FROM " + (*iter).tables + " WHERE " + (*iter).conditions );
	}

	d->deleteQueue.clear();
}

void PersistentBroker::addToDeleteQueue( const QString &tables, const QString &conditions )
{
	stDeleteItem dItem;
	dItem.tables = tables;
	dItem.conditions = conditions;
	d->deleteQueue.push_back( dItem );
}

QString PersistentBroker::lastError() const
{
	return d->connection->error();
}

void PersistentBroker::lockTable( const QString& table ) const
{
	d->connection->lockTable( table );
}

void PersistentBroker::unlockTable( const QString& table ) const
{
	d->connection->unlockTable( table );
}

void PersistentBroker::startTransaction()
{
	executeQuery( "BEGIN;" );
}

void PersistentBroker::commitTransaction()
{
	executeQuery( "COMMIT;" );
}

void PersistentBroker::rollbackTransaction()
{
	executeQuery( "ROLLBACK;" );
}

bool PersistentBroker::tableExists( const QString &table )
{
	return d->connection->tableExists( table );
}

QString PersistentBroker::quoteString( QString s )
{
	if( d->sqlite )
		return s.replace( QRegExp("'"), "''" );
	else
		return s.replace( QRegExp("'"), "\\'" );
}
