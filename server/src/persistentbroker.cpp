/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
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

#include "persistentbroker.h"
#include "persistentobject.h"

#include "exceptions.h"
#include "dbdriver.h"
#include "console.h"

#include "log.h"

// Qt Includes
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <list>

struct stDeleteItem
{
	QString tables;
	QString conditions;
};

class PersistentBrokerPrivate
{
public:
	QSqlDatabase connection;
	bool sqlite;
	std::list<stDeleteItem> deleteQueue;
};

cPersistentBroker::cPersistentBroker() : d( new PersistentBrokerPrivate )
{
}

cPersistentBroker::~cPersistentBroker()
{
	delete d;
}

bool cPersistentBroker::openDriver( const QString& driver )
{
	if ( !d->connection.isValid() )
	{
		d->connection = QSqlDatabase::addDatabase( QString("q" + driver).toUpper() );
	}

	if ( driver.toLower() == "sqlite" )
		d->sqlite = true;

	if ( !d->connection.isValid() )
		return false;

	return true;
}

bool cPersistentBroker::isOpen() const
{
	return d->connection.isOpen();
}

bool cPersistentBroker::connect( const QString& host, const QString& db, const QString& username, const QString& password, int port )
{
	if ( !d->connection.isValid() )
		return false;

	// This does nothing but a little test-connection
	d->connection.setDatabaseName( db );
	d->connection.setUserName( username );
	d->connection.setPassword( password );
	d->connection.setHostName( host );
	if (port > 0) {
		d->connection.setPort( port );
	}

	if ( !d->connection.open() )
		return false;

	if ( d->sqlite )
	{
		d->connection.exec( "PRAGMA synchronous = OFF;" );
		d->connection.exec( "PRAGMA default_synchronous = OFF;" );
		d->connection.exec( "PRAGMA full_column_names = OFF;" );
		d->connection.exec( "PRAGMA show_datatypes = OFF;" );
		d->connection.exec( "PRAGMA parser_trace = OFF;" );
	}

	return true;
}

void cPersistentBroker::disconnect()
{
	d->connection.close();
}

bool cPersistentBroker::saveObject( PersistentObject* object )
{
	object->save();
	return true;
}

bool cPersistentBroker::deleteObject( PersistentObject* object )
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

bool cPersistentBroker::executeQuery( const QString& query )
{
	if ( !d->connection.isValid() )
		throw wpException( tr( "PersistentBroker not connected to database." ) );

	QSqlQuery q;
	if ( !q.exec( query ) )
	{
		Console::instance()->log( LOG_ERROR, q.lastError().text() );
		return false;
	}
	return true;
}

QSqlQuery cPersistentBroker::query( const QString& query )
{
	if ( !d->connection.isValid() )
		throw wpException( QString( "PersistentBroker not connected to database." ) );

	return d->connection.exec( query );
}

void cPersistentBroker::clearDeleteQueue()
{
	d->deleteQueue.clear();
}

void cPersistentBroker::flushDeleteQueue()
{
	std::list<stDeleteItem>::iterator iter;
	for ( iter = d->deleteQueue.begin(); iter != d->deleteQueue.end(); ++iter )
	{
		executeQuery( "DELETE FROM " + ( *iter ).tables + " WHERE " + ( *iter ).conditions );
	}

	d->deleteQueue.clear();
}

void cPersistentBroker::addToDeleteQueue( const QString& tables, const QString& conditions )
{
	stDeleteItem dItem;
	dItem.tables = tables;
	dItem.conditions = conditions;
	d->deleteQueue.push_back( dItem );
}

QString cPersistentBroker::lastError() const
{
	return d->connection.lastError().text();
}

void cPersistentBroker::lockTable( const QString& table ) const
{
}

void cPersistentBroker::unlockTable( const QString& table ) const
{
}

void cPersistentBroker::startTransaction()
{
	d->connection.transaction();
}

void cPersistentBroker::commitTransaction()
{
	d->connection.commit();
}

void cPersistentBroker::rollbackTransaction()
{
	d->connection.rollback();
}

bool cPersistentBroker::tableExists( const QString& table )
{
	if ( !d->connection.isOpen() )
	{
		throw wpException( QString( "Trying to query an existing table without a database connection." ) );
	}

	return d->connection.tables().contains( table, Qt::CaseInsensitive );
}

QString cPersistentBroker::quoteString( QString s )
{
	if ( s == QString::null )
		return QString( "" );

	if ( d->sqlite )
		return s.replace( "'", "''" ).toUtf8().data();
	else
		return s.replace( "'", "\\'" ).toUtf8().data();
}
