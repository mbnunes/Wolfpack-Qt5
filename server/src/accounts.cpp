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

// Wolfpack Includes
#include "accounts.h"
#include "exceptions.h"
#include "serverconfig.h"
#include "dbdriver.h"
#include "console.h"
#include "commands.h"
#include "player.h"
#include "world.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "network/network.h"
#include "network/uosocket.h"

#include <QtGlobal>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

#define ACCT_DATABASE_VERSION 2

// DB AutoCreation
const char* createSql = "CREATE TABLE accounts (\
login varchar(16) NOT NULL default '',\
password varchar(128) NOT NULL default '',\
flags int NOT NULL default '0',\
acl varchar(255) NOT NULL default 'player',\
lastlogin int NOT NULL default '0',\
blockuntil int NOT NULL default '0',\
email varchar(255) NOT NULL default '',\
creationdate varchar(19) default NULL,\
totalgametime int NOT NULL default '0',\
slots smallint(5) NOT NULL default '1',\
chatname varchar(255) NOT NULL default '',\
ignorelist longtext,\
PRIMARY KEY (login)\
);";

const char* createSqlSettings = "CREATE TABLE settings (\
option varchar(255) NOT NULL default '', \
value varchar(255) NOT NULL default '', \
PRIMARY KEY (option) \
);";

const char* createMySqlSettings = "CREATE TABLE `settings` (\
`option` varchar(255) NOT NULL default '', \
`value` varchar(255) NOT NULL default '', \
PRIMARY KEY (`option`) \
);";

/*****************************************************************************
  cAccount member functions
 *****************************************************************************/

cAccount::cAccount() : acl_( 0 ), flags_( 0 ), attempts_( 0 ), inUse_( false ), chatname_( "" ), ignorelist_( "" )
{
}

void cAccount::setPassword( const QString& password )
{
	if ( Config::instance()->hashAccountPasswords() )
	{
		// Avoid double hashing if already a 128-char SHA-512 hex string
		if ( password.length() == 128 )
		{
			password_ = password;
		}
		else
		{
			password_ = QString::fromLatin1( QCryptographicHash::hash( password.toUtf8(), QCryptographicHash::Sha512 ).toHex() );
		}
	}
	else
	{
		password_ = password;
	}
}

bool cAccount::isBlocked() const
{
	if ( ( blockUntil.isValid() && blockUntil < QDateTime::currentDateTime() ) || flags_ & 0x00000001 )
		return true;
	else
		return false;
}

uint cAccount::secsToUnblock() const
{
	if ( isBlocked() )
		return static_cast<uint>( ~0 );
	else if ( blockUntil.isValid() && blockUntil < QDateTime::currentDateTime() )
		return QDateTime::currentDateTime().secsTo( blockUntil );
	return 0;
}

bool cAccount::addCharacter( P_PLAYER d )
{
	if ( !characters_.contains( d ) )
	{
		characters_.push_back( d );
		return true;
	}
	return false;
}

bool cAccount::removeCharacter( P_PLAYER d )
{
	return characters_.removeOne( d );
}

/*!
	Checks if the account is autorized to perform the action catagorized
	into \a group and \a value pair, found in the ACL
*/
bool cAccount::authorized( const QString& group, const QString& value ) const
{
	// No Valid ACL specified
	if ( !acl_ )
		return false; // Since refreshAcl have already tried to get one, just give up.

	// No group? No Access!
	QMap<QString, QMap<QString, bool> >::const_iterator groupIter = acl_->groups.find( group );
	if ( groupIter == acl_->groups.end() )
		return false;

	// Group
	QMap<QString, bool> aGroup = groupIter.value();

	// Check if we have a rule for the specified command, if not check for any
	if ( aGroup.find( value ) != aGroup.end() )
		return aGroup[value];

	if ( aGroup.find( "any" ) != aGroup.end() )
		return aGroup["any"];

	return false;
}

void cAccount::remove()
{
	QList<P_PLAYER>::iterator it;
	for ( it = characters_.begin(); it != characters_.end(); ++it )
	{
		( *it )->setAccount( 0, false );
		if ( ( *it )->socket() )
		{
			( *it )->socket()->setAccount( 0 );
		}
		( *it )->remove();
	}
	characters_.clear();

	Accounts::instance()->remove( this );
}

void cAccount::refreshAcl()
{
	acl_ = Commands::instance()->getACL( aclName_ );

	// No Valid ACL specified, will set as "player"
	if ( !acl_ )
	{
		acl_ = Commands::instance()->getACL( "player" );
	}
}

bool cAccount::isAllMove() const
{
	return flags_ & 0x00000002;
}

bool cAccount::isAllShow() const
{
	return flags_ & 0x00000004;
}

bool cAccount::isMultiGems() const
{
	return flags_ & 0x00000040;
}

void cAccount::setMultiGems( bool data )
{
	if ( data )
	{
		flags_ |= 0x40;
	}
	else
	{
		flags_ &= ~0x40;
	}
}

bool cAccount::isShowSerials() const
{
	return flags_ & 0x00000008;
}

bool cAccount::isPageNotify() const
{
	return flags_ & 0x00000010;
}

bool cAccount::isStaff() const
{
	return flags_ & 0x00000020;
}

bool cAccount::isJailed() const
{
	return flags_ & 0x00000080;
}

bool cAccount::isYoung() const
{
	return flags_ & 0x00000100;
}

void cAccount::setBlocked( bool data )
{
	if ( data )
		flags_ |= 0x00000001;
	else
		flags_ &= 0xFFFFFFFE;
}

void cAccount::setAllMove( bool data )
{
	if ( data )
		flags_ |= 0x00000002;
	else
		flags_ &= 0xFFFFFFFD;
}

void cAccount::setAllShow( bool data )
{
	if ( data )
		flags_ |= 0x00000004;
	else
		flags_ &= 0xFFFFFFFB;
}

void cAccount::setShowSerials( bool data )
{
	if ( data )
		flags_ |= 0x00000008;
	else
		flags_ &= 0xFFFFFFF7;
}

void cAccount::setPageNotify( bool data )
{
	if ( data )
		flags_ |= 0x00000010;
	else
		flags_ &= 0xFFFFFFEF;
}

void cAccount::setStaff( bool data )
{
	if ( data )
		flags_ |= 0x00000020;
	else
		flags_ &= 0xFFFFFFDF;
}

void cAccount::setJailed( bool data )
{
	if ( data )
		flags_ |= 0x00000080;
	else
		flags_ &= 0xFFFFFF7F;
}

void cAccount::setYoung( bool data )
{
	if ( data )
		flags_ |= 0x00000100;
	else
		flags_ &= 0xFFFFFEFF;
}

unsigned int cAccount::rank() const
{
	if ( acl_ )
		return acl_->rank;
	else
		return 1;
}

/*****************************************************************************
  cAccounts member functions
 *****************************************************************************/

cAccounts::~cAccounts()
{
}

void cAccounts::unload()
{
	qDeleteAll( accounts );
	accounts.clear();
	deletedAccounts_.clear();

	QSqlDatabase db = QSqlDatabase::database( "accounts", false );
	if ( db.isValid() && db.isOpen() )
	{
		db.close();
	}

	cComponent::unload();
}

cAccount* cAccounts::authenticate( const QString& login, const QString& password, enErrorCode* error ) const
{
	const_iterator it = accounts.find( login.toLower() ); // make sure it's case insensitive
	if ( error )
		*error = NoError;
	if ( it != accounts.end() )
	{
		// First we check for blocked account.
		if ( it.value()->isBlocked() )
		{
			if ( error )
				*error = Banned;
			return 0;
		}

		if ( it.value()->inUse() )
		{
			if ( error )
				*error = AlreadyInUse;
			return 0;
		}

		bool authorized = false;

		// Regard hashed passwords
		if ( Config::instance()->hashAccountPasswords() )
		{
			QString sha512Hash = QString::fromLatin1( QCryptographicHash::hash( password.toUtf8(), QCryptographicHash::Sha512 ).toHex() );
			QString md5Hash = QString::fromLatin1( QCryptographicHash::hash( password.toLocal8Bit(), QCryptographicHash::Md5 ).toHex() );

			if ( it.value()->password() == sha512Hash )
			{
				authorized = true;
			}
			else if ( it.value()->password().length() == 32 && it.value()->password() == md5Hash )
			{
				// Legacy MD5 match: automatically upgrade to SHA-512!
				authorized = true;
				it.value()->setPassword( password );
				const_cast<cAccounts*>(this)->saveAccount( it.value() );
				Console::instance()->log( LOG_NOTICE, tr( "Upgraded account '%1' password from MD5 to SHA-512.\n" ).arg( it.value()->login() ) );
			}
			else if ( it.value()->password() == password )
			{
				// Legacy plaintext match: automatically upgrade to SHA-512!
				authorized = true;
				it.value()->setPassword( password );
				const_cast<cAccounts*>(this)->saveAccount( it.value() );
				Console::instance()->log( LOG_NOTICE, tr( "Upgraded account '%1' password from plaintext to SHA-512.\n" ).arg( it.value()->login() ) );
			}
		}
		else
		{
			authorized = it.value()->password() == password;
		}

		// Ok, lets continue.
		if ( authorized )
		{
			it.value()->setLastLogin( QDateTime::currentDateTime() );
			it.value()->resetLoginAttempts();
			return it.value();
		}
		else
		{
			if ( error )
				*error = BadPassword;

			return 0;
		}
	}
	else
	{
		if ( error )
		{
			*error = LoginNotFound;
		}

		return 0;
	}
}

// Opens the accounts connection and applies per-driver settings
static void openAccountsDb( QSqlDatabase& db )
{
	const QString driver = Config::instance()->accountsDriver().toLower();
	if ( driver == "mysql" )
	{
		db.setConnectOptions( "MYSQL_OPT_RECONNECT=1" );
	}

	db.setHostName( Config::instance()->accountsHost() );
	db.setDatabaseName( Config::instance()->accountsName() );
	db.setUserName( Config::instance()->accountsUsername() );
	db.setPassword( Config::instance()->accountsPassword() );
	db.setPort( Config::instance()->accountsPort() );
	if ( !db.open() )
		throw wpException( db.lastError().text() );

	QSqlQuery q( db );
	if ( driver == "sqlite" )
	{
		// WAL gives much better write performance and avoids blocking readers
		q.exec( "PRAGMA journal_mode = WAL;" );
		q.exec( "PRAGMA synchronous = NORMAL;" );
		q.exec( "PRAGMA foreign_keys = ON;" );
		q.exec( "PRAGMA busy_timeout = 5000;" );
		q.exec( "PRAGMA temp_store = MEMORY;" );
	}
	else if ( driver == "mysql" )
	{
		q.exec( "SET NAMES utf8mb4" );
	}
}

// Executes a statement on the accounts connection and logs failures
static bool execAccountsSql( QSqlDatabase& db, const QString& sql )
{
	QSqlQuery q( db );
	if ( !q.exec( sql ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Accounts database query failed: %1 (%2)" ).arg( q.lastError().text(), sql.left( 80 ) ) );
		return false;
	}
	return true;
}

// Returns the CREATE TABLE statement for the accounts table, adapted for the driver
static QString accountsCreateSql()
{
	QString sql = QString::fromLatin1( createSql );
	if ( Config::instance()->accountsDriver().toLower() == "mysql" )
	{
		sql.chop( 1 ); // trailing ';'
		sql += " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	}
	return sql;
}

// Automatically imports accounts from a legacy accounts.db file into the new/unified database
static bool importLegacyAccounts( QSqlDatabase& targetDb )
{
	QFileInfo legacyFile( "accounts.db" );
	if ( !legacyFile.exists() || legacyFile.size() == 0 )
		return false;

	const QString connName = "legacy_accounts_import";
	int count = 0;
	{
		QSqlDatabase legacyDb = QSqlDatabase::addDatabase( "QSQLITE", connName );
		legacyDb.setDatabaseName( legacyFile.filePath() );
		if ( !legacyDb.open() )
		{
			QSqlDatabase::removeDatabase( connName );
			return false;
		}

		if ( !legacyDb.tables().contains( "accounts", Qt::CaseInsensitive ) )
		{
			legacyDb.close();
			QSqlDatabase::removeDatabase( connName );
			return false;
		}

		QSqlQuery readQuery( legacyDb );
		if ( !readQuery.exec( "SELECT login,password,flags,acl,lastlogin,blockuntil,email,creationdate,totalgametime,slots,chatname,ignorelist FROM accounts" ) )
		{
			legacyDb.close();
			QSqlDatabase::removeDatabase( connName );
			return false;
		}

		QSqlQuery writeQuery( targetDb );
		writeQuery.prepare( "REPLACE INTO accounts (login,password,flags,acl,lastlogin,blockuntil,email,creationdate,totalgametime,slots,chatname,ignorelist) values( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )" );

		while ( readQuery.next() )
		{
			for ( int i = 0; i < 12; ++i )
			{
				QVariant val = readQuery.value( i );
				if ( ( i == 6 || i == 10 || i == 11 ) && val.isNull() ) // email, chatname, ignorelist
				{
					val = QString( "" );
				}
				writeQuery.addBindValue( val );
			}
			if ( writeQuery.exec() )
				count++;
		}

		legacyDb.close();
	}
	QSqlDatabase::removeDatabase( connName );

	if ( count > 0 )
	{
		Console::instance()->send( QString( "Successfully migrated %1 accounts from legacy accounts.db into the database!\n" ).arg( count ) );
		QFile::rename( "accounts.db", "accounts.db.imported" );
		return true;
	}
	return false;
}

void cAccounts::save()
{
	// Open the Account Driver
	QSqlDatabase db = QSqlDatabase::database("accounts");
	if ( !db.isValid() )
	{
		db = QSqlDatabase::addDatabase( QString("q" + Config::instance()->accountsDriver()).toUpper(), "accounts" );
		if ( !db.isValid() )
		{
			throw wpException( tr( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
		}
	}

	bool transaction = false;

	try
	{
		if ( !db.isOpen() )
			openAccountsDb( db );

		if ( !db.tables().contains( "accounts", Qt::CaseInsensitive ) )
		{
			Console::instance()->send( tr( "Accounts database didn't exist! Creating one\n" ) );
			execAccountsSql( db, accountsCreateSql() );
			if (!getRecord("admin")) {
				cAccount* account = createAccount( "admin", "admin" );
				Console::instance()->send( tr( "Created default admin account: Login = admin, Password = admin\n" ) );
			}
		}

		transaction = db.transaction();
		if ( !transaction )
			Console::instance()->log( LOG_WARNING, tr( "Accounts database: unable to start a transaction: %1" ).arg( db.lastError().text() ) );

		// Delete accounts queued for removal
		if ( !deletedAccounts_.isEmpty() )
		{
			QSqlQuery delQuery( db );
			delQuery.prepare( "DELETE FROM accounts WHERE login = ?" );
			foreach ( const QString& delLogin, deletedAccounts_ )
			{
				delQuery.addBindValue( delLogin );
				if ( !delQuery.exec() )
				{
					Console::instance()->log( LOG_ERROR, tr( "Unable to delete account '%1': %2" ).arg( delLogin ).arg( delQuery.lastError().text() ) );
				}
			}
			deletedAccounts_.clear();
		}

		// Incremental upsert: REPLACE INTO updates existing records and inserts new ones
		QSqlQuery query( db );
		query.prepare( "REPLACE INTO accounts (login,password,flags,acl,lastlogin,blockuntil,email,creationdate,totalgametime,slots,chatname,ignorelist) values( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )" );
		iterator it = accounts.begin();
		for ( ; it != accounts.end(); ++it )
		{
			cAccount* account = it.value();

			query.addBindValue( account->login_ );
			query.addBindValue( account->password_ );
			query.addBindValue( QVariant::fromValue<uint>( account->flags_ ) );
			query.addBindValue( account->aclName_.isEmpty() ? "" : account->aclName_ );
			query.addBindValue( QVariant::fromValue<uint>( !account->lastLogin_.isNull() ? account->lastLogin_.toSecsSinceEpoch() : 0 ) );
			query.addBindValue( QVariant::fromValue<uint>( !account->blockUntil.isNull() ? account->blockUntil.toSecsSinceEpoch() : 0 ) );
			query.addBindValue( QString::fromLatin1( account->email_.constData() ) ); // never NULL
			query.addBindValue( QString(account->creationdate_) );
			query.addBindValue( account->totalgametime_ );
			query.addBindValue( account->charslots_ );
			query.addBindValue( account->chatname_.isNull() ? QString( "" ) : account->chatname_ );
			query.addBindValue( account->ignorelist_.isNull() ? QString( "" ) : account->ignorelist_ );

			if (!query.exec()) {
				Console::instance()->log(LOG_ERROR, tr("Unable to save account '%1' because of the following error: %2").arg( account->login_ ).arg(query.lastError().text()));
			}
		}

		if ( transaction && !db.commit() )
			throw wpException( db.lastError().text() );
	}
	catch ( wpException& error )
	{
		if ( transaction )
			db.rollback();
		Console::instance()->log( LOG_ERROR, tr( "Error while saving Accounts: %1." ).arg( error.error() ) );
	}
	catch ( ... )
	{
		if ( transaction )
			db.rollback();
		Console::instance()->log( LOG_ERROR, tr( "Unknown error while saving Accounts." ) );
	}

	// Keep db open for connection reuse; closed in unload()
}

void cAccounts::saveAccount( cAccount* account )
{
	if ( !account )
		return;

	QSqlDatabase db = QSqlDatabase::database( "accounts" );
	if ( !db.isValid() )
		return;

	if ( !db.isOpen() )
	{
		try
		{
			openAccountsDb( db );
		}
		catch ( ... )
		{
			return;
		}
	}

	QSqlQuery query( db );
	query.prepare( "REPLACE INTO accounts (login,password,flags,acl,lastlogin,blockuntil,email,creationdate,totalgametime,slots,chatname,ignorelist) values( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )" );
	query.addBindValue( account->login_ );
	query.addBindValue( account->password_ );
	query.addBindValue( QVariant::fromValue<uint>( account->flags_ ) );
	query.addBindValue( account->aclName_.isEmpty() ? "" : account->aclName_ );
	query.addBindValue( QVariant::fromValue<uint>( !account->lastLogin_.isNull() ? account->lastLogin_.toSecsSinceEpoch() : 0 ) );
	query.addBindValue( QVariant::fromValue<uint>( !account->blockUntil.isNull() ? account->blockUntil.toSecsSinceEpoch() : 0 ) );
	query.addBindValue( QString::fromLatin1( account->email_.constData() ) );
	query.addBindValue( QString( account->creationdate_ ) );
	query.addBindValue( account->totalgametime_ );
	query.addBindValue( account->charslots_ );
	query.addBindValue( account->chatname_.isNull() ? QString( "" ) : account->chatname_ );
	query.addBindValue( account->ignorelist_.isNull() ? QString( "" ) : account->ignorelist_ );

	if ( !query.exec() )
	{
		Console::instance()->log( LOG_ERROR, tr( "Unable to save account '%1': %2" ).arg( account->login_ ).arg( query.lastError().text() ) );
	}
}

void cAccounts::load()
{
	try
	{
		//
		// Check and Update Database Section
		//

		// Open the Account Driver
		QSqlDatabase db = QSqlDatabase::database("accounts");
		if ( !db.isValid() )
		{
			db = QSqlDatabase::addDatabase( QString("q" + Config::instance()->accountsDriver()).toUpper(), "accounts" );
			if ( !db.isValid() )
			{
				throw wpException( tr( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
			}
		}

		// Open the Database
		if ( !db.isOpen() )
			openAccountsDb( db );

		// Checking for Account Database
		cAccount* adminAccount = NULL;
		int db_version = 0;
		if ( !db.tables().contains( "accounts", Qt::CaseInsensitive ) )
		{
			Console::instance()->send( tr( "Accounts database didn't exist! Creating one\n" ) );
			execAccountsSql( db, accountsCreateSql() );
			db_version = ACCT_DATABASE_VERSION;

			// Check if we can migrate from an existing legacy accounts.db
			bool imported = false;
			if ( Config::instance()->accountsName() != "accounts.db" || Config::instance()->accountsDriver().toLower() != "sqlite" )
			{
				imported = importLegacyAccounts( db );
			}

			if ( !imported )
			{
				unload();
				adminAccount = createAccount( "admin", "admin" );
				Console::instance()->send( tr( "Created default admin account: Login = admin, Password = admin\n" ) );
			}
			else
			{
				// If legacy accounts were imported, make sure there is at least an admin account
				if ( !getRecord( "admin" ) )
				{
					adminAccount = createAccount( "admin", "admin" );
					Console::instance()->send( tr( "Created default admin account: Login = admin, Password = admin\n" ) );
				}
			}

			// reopen the Database, if needed
			if ( !db.isOpen() )
				openAccountsDb( db );
		}

		// Ensure password column is varchar(128) to store SHA-512 hashes
		if ( Config::instance()->accountsDriver().toLower() == "mysql" )
		{
			QSqlQuery alterQuery( db );
			alterQuery.exec( "ALTER TABLE accounts MODIFY password varchar(128) NOT NULL default ''" );
		}

		// Creating settings table if it doesn't exist
		if ( !db.tables().contains( "settings", Qt::CaseInsensitive ) )
		{
			if ( Config::instance()->accountsDriver() == "mysql" )
			{
				execAccountsSql( db, createMySqlSettings );
				execAccountsSql( db, QString("insert into `settings` (`option`, `value`) values ('acct_db_version',%1);").arg( ACCT_DATABASE_VERSION ) );
			}
			else
			{
				execAccountsSql( db, createSqlSettings );
				execAccountsSql( db, QString("insert into settings (option, value) values ('acct_db_version',%1);").arg( ACCT_DATABASE_VERSION ) );
			}
			db_version = ACCT_DATABASE_VERSION;
		}
		else
		{
			// Load DB Version: check for 'acct_db_version' first to support shared database with world
			QString settingsSql;
			if ( Config::instance()->accountsDriver() == "mysql" )
			{
				settingsSql = "SELECT `value` FROM `settings` WHERE `option` = 'acct_db_version';";
			}
			else
			{
				settingsSql = "SELECT value FROM settings WHERE option = 'acct_db_version';";
			}
			QSqlQuery acctquery ( db );
			acctquery.exec( settingsSql );
			if ( acctquery.first() )
			{
				db_version = acctquery.value( 0 ).toInt();
			}
			else
			{
				// Fallback: check if legacy 'db_version' exists and is for accounts (<= ACCT_DATABASE_VERSION)
				if ( Config::instance()->accountsDriver() == "mysql" )
				{
					settingsSql = "SELECT `value` FROM `settings` WHERE `option` = 'db_version';";
				}
				else
				{
					settingsSql = "SELECT value FROM settings WHERE option = 'db_version';";
				}
				acctquery.exec( settingsSql );
				if ( acctquery.first() && acctquery.value( 0 ).toInt() <= ACCT_DATABASE_VERSION )
				{
					db_version = acctquery.value( 0 ).toInt();
				}
				else
				{
					db_version = ACCT_DATABASE_VERSION;
				}
				// Save as acct_db_version so it doesn't collide with world db_version
				if ( Config::instance()->accountsDriver() == "mysql" )
				{
					execAccountsSql( db, QString("REPLACE INTO `settings` (`option`, `value`) VALUES ('acct_db_version', '%1');").arg( db_version ) );
				}
				else
				{
					execAccountsSql( db, QString("REPLACE INTO settings (option, value) VALUES ('acct_db_version', '%1');").arg( db_version ) );
				}
			}
		}

		if ( db_version != ACCT_DATABASE_VERSION )
		{
			// Call Event
			cPythonScript *script = ScriptManager::instance()->getGlobalHook( EVENT_UPDATEACCTDATABASE );
			if ( !script || !script->canHandleEvent( EVENT_UPDATEACCTDATABASE ) )
			{
				throw wpException( tr( "Unable to load account database. Version mismatch: %1 != %2." ).arg( db_version ).arg( ACCT_DATABASE_VERSION ) );
			}

			PyObject *args = Py_BuildValue( "(ii)", ACCT_DATABASE_VERSION, db_version );
			bool result = script->callEventHandler( EVENT_UPDATEACCTDATABASE, args );
			Py_DECREF( args );

			if ( !result )
			{
				throw wpException( tr( "Unable to load account database. Version mismatch: %1 != %2." ).arg( db_version ).arg( ACCT_DATABASE_VERSION ) );
			}
		}

		// if we have the new created admin account, there aren't any other accounts to load
		if ( adminAccount == NULL )
		{
			// Initializing Query
			QSqlQuery query ( db );
			query.setForwardOnly( true );

			// Selecting things to Query
			query.exec( "SELECT login,password,flags,acl,lastlogin,blockuntil,email,creationdate,totalgametime,slots,chatname,ignorelist FROM accounts" );

			// Clear Accounts HERE
			// Here we can be pretty sure that we have a valid datasource for accounts
			unload();
			bool convertedAny = false;
			while ( query.next() )
			{
				cAccount* account = new cAccount;
				account->login_ = query.value( 0 ).toString().toLower();
				account->password_ = query.value( 1 ).toString();
				account->flags_ = query.value( 2 ).toInt();
				account->aclName_ = query.value( 3 ).toString();
				account->refreshAcl();
				if ( query.value( 4 ).toInt() != 0 )
					account->lastLogin_.setSecsSinceEpoch( query.value( 4 ).toInt() );

				if ( query.value( 5 ).toInt() != 0 )
					account->blockUntil.setSecsSinceEpoch( query.value( 5 ).toInt() );

				account->email_ = query.value( 6 ).toByteArray();
				account->creationdate_ = query.value( 7 ).toString();
				account->totalgametime_ = query.value( 8 ).toInt();
				account->charslots_ = query.value( 9 ).toInt();
				account->chatname_ = query.value( 10 ).toString();
				account->ignorelist_ = query.value( 11 ).toString();

				// See if the password can and should be hashed with SHA-512.
				// SHA-512 hashes are 128 characters long.
				// Legacy MD5 hashes are 32 characters long and will be upgraded on first login.
				// Plaintext passwords (length != 32 && != 128) can be hashed directly if convertUnhashedPasswords is enabled.
				if ( Config::instance()->hashAccountPasswords() && account->password_.length() != 128 )
				{
					if ( account->password_.length() != 32 && Config::instance()->convertUnhashedPasswords() )
					{
						account->setPassword( account->password_ );
						convertedAny = true;
						Console::instance()->log( LOG_NOTICE, tr( "Hashed plaintext account password to SHA-512 for '%1'.\n" ).arg( account->login_ ) );
					}
					else if ( account->password_.length() != 32 )
					{
						Console::instance()->log( LOG_NOTICE, tr( "Account '%1' has an unhashed password.\n" ).arg( account->login_ ) );
					}
				}

				accounts.insert( account->login_, account );
			}

			if ( convertedAny )
			{
				save();
			}
		}
	}
	catch ( wpException& error )
	{
		throw wpException( tr( "Error while loading Accounts: %1" ).arg( error.error() ) );
	}
	catch ( ... )
	{
		throw wpException( tr( "Unknown error while loading Accounts" ) );
	}

	cComponent::load();
}

/*!
	Reloads all accounts
*/
void cAccounts::reload()
{
	QMap<SERIAL, QString> characcnames;
	QStringList sockaccnames;

	cCharIterator iterChars;
	P_CHAR pc;
	for ( pc = iterChars.first(); pc; pc = iterChars.next() )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>( pc );
		if ( pp && pp->account() )
		{
			characcnames.insert( pp->serial(), pp->account()->login() );
		}
	}

	cUOSocket* mSock = NULL;
	QList<cUOSocket*> sockets = Network::instance()->sockets();
	foreach ( mSock, sockets )
	{
		if ( mSock->account() )
			sockaccnames.push_back( mSock->account()->login() );
		else
			sockaccnames.push_back( QString() );
	}

	load();

	QMap<SERIAL, QString>::Iterator it = characcnames.begin();
	while ( it != characcnames.end() )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>( FindCharBySerial( it.key() ) );
		if ( pp )
			pp->setAccount( getRecord( it.value() ), false );
		++it;
	}

	QStringList::iterator sit = sockaccnames.begin();
	foreach ( mSock, sockets )
	{
		if ( !( *sit ).isNull() )
			mSock->setAccount( getRecord( ( *sit ) ) );
		++sit;
	}
}

/*!
	Creates an account with \a login and \a password. If this is the first account
	on the system, it will be set with "admin" acl, otherwise, it will default to
	"player" acl.
*/
cAccount* cAccounts::createAccount( const QString& login, const QString& password )
{
	cAccount* d = new cAccount;
	d->login_ = login.toLower();
	d->setPassword( password );
	d->setCreationDate( QDateTime::currentDateTime().toString( "yyyy-MM-dd HH:mm:ss" ) ); // 19 chars, fits varchar(19)
	d->setTotalGameTime( 0 );
	d->setCharSlots( Config::instance()->maxCharsPerAccount() );
	accounts.insert( d->login(), d );
	if ( accounts.count() == 1 ) // first account, it must be admin!
	{
		d->setAcl( "admin" );
		d->refreshAcl();
		save(); // Make sure to save it.
		//reload(); // Reloads
		return d;
	}
	else
	{
		d->refreshAcl();
		save(); // Make sure to save it.
		return d;
	}
}

/*!
	Removes \a record account from the system
*/
void cAccounts::remove( cAccount* record )
{
	if ( !record )
		return;

	QString login = record->login();
	if ( accounts.contains( login ) )
		accounts.remove( login );

	deletedAccounts_.append( login );
	delete record;
}

/*!
	Returns the number of loaded accounts
*/
uint cAccounts::count()
{
	return accounts.count();
}

/*!
	Retrieves the account matching \a login or 0 if no such
	account can be found.
*/
cAccount* cAccounts::getRecord( const QString& login )
{
	iterator it = accounts.find( login );
	if ( it == accounts.end() )
		return 0;
	else
		return it.value();
}

void cAccounts::clearAcls()
{
	iterator it = accounts.begin();

	while ( it != accounts.end() )
	{
		it.value()->refreshAcl();
		++it;
	}
}

const char* cAccount::className() const
{
	return "account";
}

bool cAccount::implements( const QString& name ) const
{
	if ( name == "account" )
		return true;
	else
		return cPythonScriptable::implements( name );
}

PyObject* cAccount::getPyObject()
{
	return createPyObject( this );
}

PyObject* cAccount::getProperty( const QString& name, uint hash )
{
	PY_PROPERTY( "acl", acl() );
	PY_PROPERTY( "email", email() );
	// \rproperty account.name The name of this account.
	PY_PROPERTY( "name", login() );
	PY_PROPERTY( "multigems", isMultiGems() );
	PY_PROPERTY( "password", password() );
	PY_PROPERTY( "rawpassword", password() );
	PY_PROPERTY( "flags", flags() );
	// \rproperty account.creationdate The Creation date for this account.
	PY_PROPERTY( "creationdate", creationdate() );
	PY_PROPERTY( "totalgametime", totalgametime() );
	PY_PROPERTY( "charslots", charslots() );
	PY_PROPERTY( "chatname", chatname() );
	PY_PROPERTY( "ignorelist", ignorelist() );
	/*
		\rproperty account.characters A tuple of <object id="CHAR">char</object> objects.
		This tuple contains all characters assigned to this account.
	*/
	if ( name == "characters" )
	{
		PyObject* tuple = PyTuple_New( characters_.size() );
		for ( int i = 0; i < characters_.size(); ++i ) {
			PyTuple_SetItem( tuple, i, characters_[i]->getPyObject() );
		}
		return tuple;
	}
	/*
		\rproperty account.lastlogin The last login date of this account or
		an empty string if it's unknown.
	*/
	PY_PROPERTY( "lastlogin", lastLogin().toString() );
	PY_PROPERTY( "blockuntil", blockUntil.toString() );
	// \rproperty account.inuse Indicates whether this account is currently in use.
	PY_PROPERTY( "inuse", inUse() );
	// \rproperty account.rank Returns the integer rank of this account. This is inherited by the ACL of this account.
	PY_PROPERTY( "rank", rank() );
	// \rproperty account.young Returns the Young Status for this account
	PY_PROPERTY( "young", isYoung() );

	return cPythonScriptable::getProperty( name, hash );
}

stError* cAccount::setProperty( const QString& name, const cVariant& value )
{
	/*
		\property account.acl The name of the ACL used to check the permissions of this account.
	*/
	if ( name == "acl" )
	{
		setAcl( value.toString() );
		return 0;
	}
	/*
		\property account.email The E-Mail address associated with this account.
	*/
	else if ( name == "email" )
	{
		QString text = value.toString();
		if( text == QString() )
			PROPERTY_ERROR( -2, "String expected" );
		email_ = text.toLatin1();
		return 0;
	}
	/*
		\property account.multigems Indicates whether Multis should be sent as Worldgems to this account.
	*/
	else if ( name == "multigems" )
	{
		setMultiGems( value.toInt() != 0 );
		return 0;
	}
	/*
		\property account.password The password of this account. Please note that if MD5 hashing is activated,
		this property will only return the hashed password. But when setting this property you don't need to
		specify the MD5 hashed password as it will be automatically converted.
	*/
	else if ( name == "password" )
	{
		setPassword( value.toString() );
		return 0;
	}
	/*
		\property account.rawpassword If you use MD5 hashing this property is the hashed password and no
		conversions will be done automatically if you set this property. If you don't use MD5 hashing,
		this property is equivalent to the password property.
	*/
	else if ( name == "rawpassword" )
	{
		password_ = value.toString();
		return 0;
	}
	/*
		\property account.flags This property provides direct access to the flags of this account. Possible flags
		are:
		<code>0x00000001 blocked
		0x00000002 allmove
		0x00000004 allshow
		0x00000008 showserials
		0x00000010 pagenotify
		0x00000020 staff - gm mode on/off
		0x00000040 multigems on/off
		0x00000080 jailed
		0x00000100 young</code>
	*/
	else if ( name == "flags" )
	{
		SET_INT_PROPERTY( "flags", flags_ );
		return 0;
	}
	/*
	\property account.blockuntil This is the date and time when this account will be unblocked.
	The following format for the date and time is used (from the QT documentation):
	<code>
	Qt::ISODate - ISO 8601 extended format (YYYY-MM-DD, or with time, YYYY-MM-DDTHH:MM:SS)
	</code>
	*/
	else if ( name == "blockuntil" )
	{
		QDateTime datetime = QDateTime::fromString( value.toString(), Qt::ISODate );
		setBlockUntil( datetime );
		return 0;
	}
	/*
		\rproperty account.charslots How many Slots for chars this account have.
	*/
	else if ( name == "charslots" )
	{
		if (value.toInt() > 6)
			setCharSlots(7);
		else if ( value.toInt() == 6 )
            setCharSlots( 6 );
		else if ( value.toInt() < 1 )
			setCharSlots( 1 );
		else
			setCharSlots( value.toInt() );
		return 0;
	}
	/*
		\rproperty account.chatname The Chat name to this account
	*/
	else if ( name == "chatname" )
	{
		setChatName( value.toString() );
		return 0;
	}
	/*
		\rproperty account.ignorelist The Chat's ignore list of this account
	*/
	else if ( name == "ignorelist" )
	{
		setIgnoreList( value.toString() );
		return 0;
	}
	/*
		\rproperty account.totalgametime How many online minutes this account have since creation.
	*/
	else if ( name == "totalgametime" )
	{
		setTotalGameTime( value.toInt() );
		return 0;
	}

	return cPythonScriptable::setProperty( name, value );
}

void cAccount::setAcl( const QString& d )
{
	aclName_ = d;
	refreshAcl();
}
