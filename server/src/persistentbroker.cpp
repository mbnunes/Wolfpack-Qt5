/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2016 by holders identified in AUTHORS.txt
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
#include "player.h"
#include "npc.h"
#include "corpse.h"
#include "skills.h"

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
	bool useTransaction;
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
	QString drivername = QString("q" + driver).toUpper();
	if ( !d->connection.isValid() || ( QString::compare( d->connection.driverName(), drivername) != 0 ) )
	{
		d->connection = QSqlDatabase::addDatabase( drivername );
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

bool cPersistentBroker::connect( const QString& host, const QString& db, const QString& username, const QString& password, int port, bool transaction )
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

	d->useTransaction = transaction;

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

void cPersistentBroker::lockTable( const QString& /*table*/ ) const
{
}

void cPersistentBroker::unlockTable( const QString& /*table*/ ) const
{
}

void cPersistentBroker::startTransaction()
{
	if ( d->useTransaction )
		d->connection.transaction();
}

void cPersistentBroker::commitTransaction()
{
	if ( d->useTransaction )
		d->connection.commit();
}

void cPersistentBroker::rollbackTransaction()
{
	if ( d->useTransaction )
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

void cPersistentBroker::truncateTable( const QString& table )
{
	QString query;
	if ( d->connection.driverName().toLower() == "qmysql" )
		query += "truncate ";
	else
		query += "delete from ";
	query += table;
	executeQuery( query );
}

void cPersistentBroker::prepareQueries() const
{
	// prepare all queries needed for one save only one time
	// this should be done here
	QSqlQuery * q;

	// Player Queries
	q = new QSqlQuery();
	q->prepare("insert into players values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
	cPlayer::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("update players set serial = ?, account = ?, additionalflags = ?, visualrange = ?, profile = ?, fixedlight = ?, strlock = ?, dexlock = ?, intlock = ?, maxcontrolslots = ? where serial = ?");
	cPlayer::setUpdateQuery(q);

	// Item Queries
	q = new QSqlQuery();
	q->prepare("insert into items values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
	cItem::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("update items set serial = ?, id = ?, color = ?, cont = ?, layer = ?, amount = ?, hp = ?, maxhp = ?, movable = ?, owner = ?, visible = ?, priv = ?, baseid = ? where serial = ?");
	cItem::setUpdateQuery(q);

	// UObject Queries
	q = new QSqlQuery();
	q->prepare("insert into uobject values ( ?, ?, ?, ?, ?, ?, ?, ?, ? )");
	cUObject::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("update uobject set name = ?, serial = ?, multis = ?, pos_x = ?, pos_y = ?, pos_z = ?, pos_map = ?, events = ?, havetags = ? where serial = ?");
	cUObject::setUpdateQuery(q);

	q = new QSqlQuery();
	q->prepare("insert into uobjectmap values ( ?, ? )");
	cUObject::setUObjectmapQuery(q);

	// CustomTag Queries
	q = new QSqlQuery();
	q->prepare("insert into tags values( ?, ?, ?, ?)");
	cCustomTags::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("DELETE FROM tags WHERE serial = ?");
	cCustomTags::setDeleteQuery(q);

	// Corpse Queries
	q = new QSqlQuery();
	q->prepare("insert into corpses values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
	cCorpse::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("update corpses set serial = ?, bodyid = ?, hairstyle = ?, haircolor = ?, beardstyle = ?, beardcolor = ?, direction = ?, charbaseid = ?, murderer = ?, murdertime = ? where serial = ?");
	cCorpse::setUpdateQuery(q);

	q = new QSqlQuery();
	q->prepare("INSERT INTO corpses_equipment VALUES ( ?, ?, ? )");
	cCorpse::setInsertEquipmentQuery(q);

	q = new QSqlQuery();
	q->prepare("DELETE FROM corpses_equipment WHERE serial = ?");
	cCorpse::setDeleteEquipmentQuery(q);

	// NPC Queries
	q = new QSqlQuery();
	q->prepare("insert into npcs values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
	cNPC::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("update npcs set serial = ?, summontime = ?, additionalflags = ?, owner = ?, stablemaster = ?, ai = ?, wandertype = ?, wanderx1 = ?, wanderx2 = ?, wandery1 = ?, wandery2 = ?, wanderradius = ? where serial = ?");
	cNPC::setUpdateQuery(q);

	// BaseChar Queries
	q = new QSqlQuery();
	q->prepare("insert into characters values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
	cBaseChar::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("update characters set serial = ?, name = ?, title = ?, creationdate = ?, body = ?, orgbody = ?, skin = ?, orgskin = ?, saycolor = ?, emotecolor = ?, strength = ?, strengthmod = ?, dexterity = ?, dexteritymod = ?, intelligence = ?, intelligencemod = ?, maxhitpoints = ?, hitpoints = ?, maxstamina = ?, stamina = ?, maxmana = ?, mana = ?, karma = ?, fame = ?, kills = ?, deaths = ?, hunger = ?, poison = ?, murderertime = ?, criminaltime = ?, gender = ?, propertyflags = ?, murderer = ?, guarding = ?, hitpointsbonus = ?, staminabonus = ?, manabonus = ?,  strcap = ?, dexcap = ?, intcap = ?, statcap = ?, baseid = ?, direction = ? where serial = ?");
	cBaseChar::setUpdateQuery(q);

	// Skills Queries
	q = new QSqlQuery();
	q->prepare("INSERT INTO skills VALUES( ?, ?, ?, ?, ?)");
	cSkills::setInsertQuery(q);

	q = new QSqlQuery();
	q->prepare("REPLACE INTO skills VALUES( ?, ?, ?, ?, ?)");
	cSkills::setUpdateQuery(q);

	q = NULL;
}

void cPersistentBroker::clearQueries() const
{
	// delete prepared queries after a save here
	QSqlQuery * q;

	// Player Queries
	q = cPlayer::getInsertQuery();
	cPlayer::setInsertQuery(NULL);
	delete q;

	q = cPlayer::getUpdateQuery();
	cPlayer::setUpdateQuery(NULL);
	delete q;

	// Item Queries
	q = cItem::getInsertQuery();
	cItem::setInsertQuery(NULL);
	delete q;

	q = cItem::getUpdateQuery();
	cItem::setUpdateQuery(NULL);
	delete q;

	// UObject Queries
	q = cUObject::getInsertQuery();
	cUObject::setInsertQuery(NULL);
	delete q;

	q = cUObject::getUpdateQuery();
	cUObject::setUpdateQuery(NULL);
	delete q;

	q = cUObject::getUObjectmapQuery();
	cUObject::setUObjectmapQuery(NULL);
	delete q;

	// CustomTag Queries
	q = cCustomTags::getInsertQuery();
	cCustomTags::setInsertQuery(NULL);
	delete q;

	q = cCustomTags::getDeleteQuery();
	cCustomTags::setDeleteQuery(NULL);
	delete q;

	// Corpse Queries
	q = cCorpse::getInsertQuery();
	cCorpse::setInsertQuery(NULL);
	delete q;

	q = cCorpse::getUpdateQuery();
	cCorpse::setUpdateQuery(NULL);
	delete q;

	q = cCorpse::getInsertEquipmentQuery();
	cCorpse::setInsertEquipmentQuery(NULL);
	delete q;

	q = cCorpse::getDeleteEquipmentQuery();
	cCorpse::setDeleteEquipmentQuery(NULL);
	delete q;

	// NPC Queries
	q = cNPC::getInsertQuery();
	cNPC::setInsertQuery(NULL);
	delete q;

	q = cNPC::getUpdateQuery();
	cNPC::setUpdateQuery(NULL);
	delete q;

	// BaseChar Queries
	q = cBaseChar::getInsertQuery();
	cBaseChar::setInsertQuery(NULL);
	delete q;

	q = cBaseChar::getUpdateQuery();
	cBaseChar::setUpdateQuery(NULL);
	delete q;

	// Skills Queries
	q = cSkills::getInsertQuery();
	cSkills::setInsertQuery(NULL);
	delete q;

	q = cSkills::getUpdateQuery();
	cSkills::setUpdateQuery(NULL);
	delete q;

}
