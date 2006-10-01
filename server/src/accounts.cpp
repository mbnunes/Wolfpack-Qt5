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

// Wolfpack Includes
#include "accounts.h"
#include "exceptions.h"
#include "serverconfig.h"
#include "dbdriver.h"
#include "console.h"
#include "commands.h"
#include "player.h"
#include "persistentbroker.h"
#include "world.h"
#include "md5.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "network/network.h"
#include "network/uosocket.h"

#include <QtGlobal>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>

#define ACCT_DATABASE_VERSION 1

// DB AutoCreation
const char* createSql = "CREATE TABLE accounts (\
login varchar(16) NOT NULL default '',\
password varchar(32) NOT NULL default '',\
flags int NOT NULL default '0',\
acl varchar(255) NOT NULL default 'player',\
lastlogin int NOT NULL default '0',\
blockuntil int NOT NULL default '0',\
email varchar(255) NOT NULL default '',\
creationdate varchar(19) default NULL,\
totalgametime int NOT NULL default '0',\
slots smallint(5) NOT NULL default '1',\
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

cAccount::cAccount() : acl_( 0 ), flags_( 0 ), attempts_( 0 ), inUse_( false )
{
}

void cAccount::setPassword( const QString& password )
{
	if ( Config::instance()->hashAccountPasswords() )
	{
		password_ = cMd5::fastDigest( password );
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
	if ( qFind( characters_.begin(), characters_.end(), d ) == characters_.end() )
	{
		characters_.push_back( d );
		return true;
	}
	return false;
}

bool cAccount::removeCharacter( P_PLAYER d )
{
	QList<P_PLAYER>::iterator it = qFind( characters_.begin(), characters_.end(), d );
	if ( it != characters_.end() )
	{
		characters_.erase( it );
		return true;
	}
	return false;
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
			authorized = it.value()->password() == cMd5::fastDigest( password );
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

	bool transaction;

	try
	{
		if ( !db.isOpen() )
		{
			db.setHostName( Config::instance()->accountsHost() );
			db.setDatabaseName( Config::instance()->accountsName() );
			db.setUserName( Config::instance()->accountsUsername() );
			db.setPassword( Config::instance()->accountsPassword() );
			db.setPort( Config::instance()->accountsPort() );
			if ( !db.open() )
				throw wpException( db.lastError().text() );
			db.exec( "PRAGMA synchronous = OFF;" );
			db.exec( "PRAGMA default_synchronous = OFF;" );
			db.exec( "PRAGMA full_column_names = OFF;" );
			db.exec( "PRAGMA show_datatypes = OFF;" );
			db.exec( "PRAGMA parser_trace = OFF;" );
		}

		if ( !db.tables().contains( "accounts", Qt::CaseInsensitive ) )
		{
			Console::instance()->send( tr( "Accounts database didn't exist! Creating one\n" ) );
			db.exec( createSql );
			if (!getRecord("admin")) {
				cAccount* account = createAccount( "admin", "admin" );
				Console::instance()->send( tr( "Created default admin account: Login = admin, Password = admin\n" ) );
			}
		}

		transaction = db.transaction();

		// Lock the table
		QSqlQuery query( db );
		if (!query.exec( "TRUNCATE accounts" )) {
			query.exec("DELETE FROM accounts");
		}
		query.prepare( "insert into accounts values( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )" );
		iterator it = accounts.begin();
		for ( ; it != accounts.end(); ++it )
		{
			// INSERT
			cAccount* account = it.value();

			query.addBindValue( account->login_ );
			query.addBindValue( account->password_ );
			query.addBindValue( QVariant::fromValue<uint>( account->flags_ ) );
			query.addBindValue( account->aclName_.isEmpty() ? "" : account->aclName_ );
			query.addBindValue( QVariant::fromValue<uint>( !account->lastLogin_.isNull() ? account->lastLogin_.toTime_t() : 0 ) );
			query.addBindValue( QVariant::fromValue<uint>( !account->blockUntil.isNull() ? account->blockUntil.toTime_t() : 0 ) );
			query.addBindValue( QString(account->email_) );
			query.addBindValue( QString(account->creationdate_) );
			query.addBindValue( account->totalgametime_ );
			query.addBindValue( account->charslots_ );

			if (!query.exec()) {
				Console::instance()->log(LOG_ERROR, tr("Unable to save account '%1' because of the following error: %2").arg( account->login_ ).arg(query.lastError().text()));
			}
		}

		db.commit();		
	}
	catch ( wpException& error )
	{
		db.rollback();
		Console::instance()->log( LOG_ERROR, tr( "Error while saving Accounts: %1." ).arg( error.error() ) );
	}
	catch ( ... )
	{
		db.rollback();
		Console::instance()->log( LOG_ERROR, tr( "Unknown error while saving Accounts." ) );
	}

	db.close();
}

void cAccounts::load()
{
	//
	// Check and Update Database Section
	//

	// Opening Persistent Broker
	if ( !PersistentBroker::instance()->openDriver( Config::instance()->accountsDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
		return;
	}

	if ( !PersistentBroker::instance()->connect( Config::instance()->accountsHost(), Config::instance()->accountsName(), Config::instance()->accountsUsername(), Config::instance()->accountsPassword(), Config::instance()->accountsPort() ) )
	{
		throw wpException( tr( "Unable to open the account database: %1." ).arg(PersistentBroker::instance()->lastError()) );
	}

	QSqlQuery acctquery;

	// Mounting table if it not exists
	if ( !PersistentBroker::instance()->tableExists( "settings" ) )
	{
		Console::instance()->send( tr( "Account Settings database didn't exist! Creating one\n" ) );
		if ( Config::instance()->accountsDriver() == "mysql" )
		{
			acctquery.exec( createMySqlSettings );
			acctquery.exec( "insert into `settings` (`option`, `value`) values ('db_version',0);" );
		}
		else { 
			acctquery.exec( createSqlSettings );
			acctquery.exec( "insert into settings (option, value) values ('db_version',0);" );
		}		
	}

	// Load Options
	QString settingsSql = "SELECT value FROM settings WHERE option = 'db_version';";
	if ( Config::instance()->accountsDriver() == "mysql" )
	{
		settingsSql = "SELECT `value` FROM `settings` WHERE `option` = 'db_version';";
	}
	acctquery.exec( settingsSql );
	acctquery.next();
	
	// Get Database Version
	QString db_version = acctquery.value( 0 ).toString();

	acctquery.clear(); // Lets make this table free

	if ( db_version.toInt() != ACCT_DATABASE_VERSION )
	{
		// Call Event
		cPythonScript *script = ScriptManager::instance()->getGlobalHook( EVENT_UPDATEACCTDATABASE );
		if ( !script || !script->canHandleEvent( EVENT_UPDATEACCTDATABASE ) )
		{
			throw wpException( tr( "Unable to load account database. Version mismatch: %1 != %2." ).arg( db_version.toInt() ).arg( ACCT_DATABASE_VERSION ) );
		}

		PyObject *args = Py_BuildValue( "(ii)", ACCT_DATABASE_VERSION, db_version.toInt() );
		bool result = script->callEventHandler( EVENT_UPDATEACCTDATABASE, args );
		Py_DECREF( args );

		if ( !result )
		{
			throw wpException( tr( "Unable to load account database. Version mismatch: %1 != %2." ).arg( db_version.toInt() ).arg( ACCT_DATABASE_VERSION ) );
		}
	}

	// Disconnect (For script purpouse?)
	PersistentBroker::instance()->disconnect();

	//
	// Now the normal Proccess for Account load
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
	
	// Load all Accounts
	try
	{
		if ( !db.isOpen() )
		{
			db.setHostName( Config::instance()->accountsHost() );
			db.setDatabaseName( Config::instance()->accountsName() );
			db.setUserName( Config::instance()->accountsUsername() );
			db.setPassword( Config::instance()->accountsPassword() );
			db.setPort( Config::instance()->accountsPort() );
			if ( !db.open() )
				throw wpException( db.lastError().text() );
			db.exec( "PRAGMA synchronous = OFF;" );
			db.exec( "PRAGMA default_synchronous = OFF;" );
			db.exec( "PRAGMA full_column_names = OFF;" );
			db.exec( "PRAGMA show_datatypes = OFF;" );
			db.exec( "PRAGMA parser_trace = OFF;" );
		}

		// Initializing Query
		QSqlQuery query ( db );
		query.setForwardOnly( true );

		// Checking for Account Database
		if ( !db.tables().contains( "accounts", Qt::CaseInsensitive ) )
		{
			Console::instance()->send( tr( "Accounts database didn't exist! Creating one\n" ) );
			db.exec( createSql );
			cAccount* account = createAccount( "admin", "admin" );
			Console::instance()->send( tr( "Created default admin account: Login = admin, Password = admin\n" ) );
		}

		// Selecting things to Query
		query.exec( "SELECT login,password,flags,acl,lastlogin,blockuntil,email,creationdate,totalgametime,slots FROM accounts" );

		// Clear Accounts HERE
		// Here we can be pretty sure that we have a valid datasource for accounts
		unload();

		while ( query.next() )
		{
			cAccount* account = new cAccount;
			account->login_ = query.value( 0 ).toString().toLower();
			account->password_ = query.value( 1 ).toString();
			account->flags_ = query.value( 2 ).toInt();
			account->aclName_ = query.value( 3 ).toString();
			account->refreshAcl();
			if ( query.value( 4 ).toInt() != 0 )
				account->lastLogin_.setTime_t( query.value( 4 ).toInt() );

			if ( query.value( 5 ).toInt() != 0 )
				account->blockUntil.setTime_t( query.value( 5 ).toInt() );

			account->email_ = query.value( 6 ).toByteArray();
			account->creationdate_ = query.value( 7 ).toString();
			account->totalgametime_ = query.value( 8 ).toInt();
			account->charslots_ = query.value( 9 ).toInt();

			// See if the password can and should be hashed,
			// Md5 hashes are 32 characters long.
			if ( Config::instance()->hashAccountPasswords() && account->password_.length() != 32 )
			{
				if ( Config::instance()->convertUnhashedPasswords() )
				{
					account->password_ = cMd5::fastDigest( account->password_ );
					Console::instance()->log( LOG_NOTICE, tr( "Hashed account password for '%1'.\n" ).arg( account->login_ ) );
				}
				else
				{
					Console::instance()->log( LOG_NOTICE, tr( "Account '%1' has an unhashed password.\n" ).arg( account->login_ ) );
				}
			}

			accounts.insert( account->login_, account );
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
	d->setCreationDate( (QDateTime::currentDateTime()).toString() );
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
	if ( accounts.contains( record->login() ) )
		accounts.remove( record->login() );
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
	{
		return true;
	}
	else
	{
		return cPythonScriptable::implements( name );
	}
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
	/*
		\rproperty account.creationdate The Creation date for this account.
	*/
	PY_PROPERTY( "creationdate", creationdate() );
	PY_PROPERTY( "totalgametime", totalgametime() );
	PY_PROPERTY( "charslots", charslots() );
	/*
		\rproperty account.characters A tuple of <object id="CHAR">char</object> objects.
		This tuple contains all characters assigned to this account.
	*/
	if ( name == "characters" )
	{
		PyObject* tuple = PyTuple_New( characters_.size() );
		for ( uint i = 0; i < characters_.size(); ++i ) {
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
		if( text == QString::null )	
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
		if ( value.toInt() > 6 )
            setCharSlots( 6 );
		else if ( value.toInt() < 1 )
			setCharSlots( 1 );
		else
			setCharSlots( value.toInt() );
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
