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

// Wolfpack Includes
#include "accounts.h"
#include "serverconfig.h"
#include "network/uosocket.h"
#include "dbdriver.h"
#include "console.h"
#include "commands.h"
#include "player.h"
#include "network/network.h"
#include "persistentbroker.h"
#include "world.h"
#include "md5.h"

// DB AutoCreation
const char* createSql = "CREATE TABLE accounts (\
login varchar(16) NOT NULL default '',\
password varchar(32) NOT NULL default '',\
flags int NOT NULL default '0',\
acl varchar(255) NOT NULL default 'player',\
lastlogin int NOT NULL default '',\
blockuntil int NOT NULL default '',\
email varchar(255) NOT NULL default '',\
PRIMARY KEY (login)\
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
	QValueVector<P_PLAYER>::iterator it = qFind( characters_.begin(), characters_.end(), d );
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
	QMap<QString, bool> aGroup = groupIter.data();

	// Check if we have a rule for the specified command, if not check for any
	if ( aGroup.find( value ) != aGroup.end() )
		return aGroup[value];

	if ( aGroup.find( "any" ) != aGroup.end() )
		return aGroup["any"];

	return false;
}

void cAccount::remove()
{
	QValueVector<P_PLAYER>::iterator it;
	for (it = characters_.begin(); it != characters_.end(); ++it) {
		(*it)->setAccount(0, false);
		if ((*it)->socket()) {
			(*it)->socket()->setAccount(0);
		}
		(*it)->remove();
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
	iterator it = accounts.begin();
	for ( ; it != accounts.end(); ++it )
		delete it.data();
	accounts.clear();

	cComponent::unload();
}

cAccount* cAccounts::authenticate( const QString& login, const QString& password, enErrorCode* error ) const
{
	const_iterator it = accounts.find( login.lower() ); // make sure it's case insensitive
	if ( error )
		*error = NoError;
	if ( it != accounts.end() )
	{
		// First we check for blocked account.
		if ( it.data()->isBlocked() )
		{
			if ( error )
				*error = Banned;
			return 0;
		}

		if ( it.data()->inUse() )
		{
			if ( error )
				*error = AlreadyInUse;
			return 0;
		}

		bool authorized = false;

		// Regard hashed passwords
		if ( Config::instance()->hashAccountPasswords() )
		{
			authorized = it.data()->password() == cMd5::fastDigest( password );
		}
		else
		{
			authorized = it.data()->password() == password;
		}

		// Ok, lets continue.
		if ( authorized )
		{
			it.data()->setLastLogin( QDateTime::currentDateTime() );
			it.data()->resetLoginAttempts();
			return it.data();
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
	if ( !PersistentBroker::instance()->openDriver( Config::instance()->accountsDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
		return;
	}

	bool connected = false;

	try
	{
		PersistentBroker::instance()->connect( Config::instance()->accountsHost(), Config::instance()->accountsName(), Config::instance()->accountsUsername(), Config::instance()->accountsPassword() );
		connected = true;

		if ( !PersistentBroker::instance()->tableExists( "accounts" ) )
		{
			Console::instance()->send( "Accounts database didn't exist! Creating one\n" );
			PersistentBroker::instance()->executeQuery( createSql );
			cAccount* account = createAccount( "admin", "admin" );
			account->setAcl( "admin" );
			Console::instance()->send( "Created default admin account: Login = admin, Password = admin\n" );
		}

		// Lock the table
		PersistentBroker::instance()->lockTable( "accounts" );
		PersistentBroker::instance()->executeQuery( "BEGIN;" );
		PersistentBroker::instance()->executeQuery( "DELETE FROM accounts;" );

		iterator it = accounts.begin();
		for ( ; it != accounts.end(); ++it )
		{
			// INSERT
			cAccount* account = it.data();

			QString sql( "REPLACE INTO accounts VALUES( '%1', '%2', %3, '%4', %5, %6, '%7' );" );

			sql = sql.arg( PersistentBroker::instance()->quoteString(account->login_) )
				.arg( PersistentBroker::instance()->quoteString(account->password_) )
				.arg( account->flags_ )
				.arg( PersistentBroker::instance()->quoteString(account->aclName_) )
				.arg( !account->lastLogin_.isNull() ? account->lastLogin_.toTime_t() : 0 )
				.arg( !account->blockUntil.isNull() ? account->blockUntil.toTime_t() : 0 )
				.arg( PersistentBroker::instance()->quoteString(account->email_) );

			PersistentBroker::instance()->executeQuery( sql );
		}

		PersistentBroker::instance()->executeQuery( "COMMIT;" );
		PersistentBroker::instance()->unlockTable( "accounts" );
	}
	catch ( QString& error )
	{
		if ( connected )
			PersistentBroker::instance()->executeQuery( "ROLLBACK;" );
		Console::instance()->log( LOG_ERROR, QString( "Error while saving Accounts: %1." ).arg( error ) );
	}
	catch ( ... )
	{
		if ( connected )
			PersistentBroker::instance()->executeQuery( "ROLLBACK;" );
		Console::instance()->log( LOG_ERROR, "Unknown error while saving Accounts." );
	}
}

void cAccounts::load()
{
	// Open the Account Driver
	if ( !PersistentBroker::instance()->openDriver( Config::instance()->accountsDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml" ).arg( Config::instance()->accountsDriver() ) );
		return;
	}

	// Load all Accounts
	try
	{
		PersistentBroker::instance()->connect( Config::instance()->accountsHost(), Config::instance()->accountsName(), Config::instance()->accountsUsername(), Config::instance()->accountsPassword() );

		if ( !PersistentBroker::instance()->tableExists( "accounts" ) )
		{
			Console::instance()->send( "Accounts database didn't exist! Creating one\n" );
			PersistentBroker::instance()->executeQuery( createSql );
			cAccount* account = createAccount( "admin", "admin" );
			account->setAcl( "admin" );
			Console::instance()->send( "Created default admin account: Login = admin, Password = admin\n" );
		}

		PersistentBroker::instance()->lockTable( "accounts" );
		cDBResult result = PersistentBroker::instance()->query( "SELECT login,password,flags,acl,lastlogin,blockuntil,email FROM accounts;" );

		// Clear Accounts HERE
		// Here we can be pretty sure that we have a valid datasource for accounts
		unload();

		while ( result.fetchrow() )
		{
			cAccount* account = new cAccount;
			account->login_ = result.getString( 0 ).lower();
			account->password_ = result.getString( 1 );
			account->flags_ = result.getInt( 2 );
			account->aclName_ = result.getString( 3 );
			account->refreshAcl();
			if ( result.getInt( 4 ) != 0 )
				account->lastLogin_.setTime_t( result.getInt( 4 ) );

			if ( result.getInt( 5 ) != 0 )
				account->blockUntil.setTime_t( result.getInt( 5 ) );

			account->email_ = result.getString( 6 );

			// See if the password can and should be hashed,
			// Md5 hashes are 32 characters long.
			if ( Config::instance()->hashAccountPasswords() && account->password_.length() != 32 )
			{
				if ( Config::instance()->convertUnhashedPasswords() )
				{
					account->password_ = cMd5::fastDigest( account->password_ );
					Console::instance()->log( LOG_NOTICE, QString( "Hashed account password for '%1'.\n" ).arg( account->login_ ) );
				}
				else
				{
					Console::instance()->log( LOG_NOTICE, QString( "Account '%1' has an unhashed password.\n" ).arg( account->login_ ) );
				}
			}

			accounts.insert( account->login_.lower(), account );
		}

		result.free();
		PersistentBroker::instance()->unlockTable( "accounts" );
	}
	catch ( QString& error )
	{
		Console::instance()->log( LOG_ERROR, QString( "Error while loading Accounts: %1" ).arg( error ) );
	}
	catch ( ... )
	{
		Console::instance()->log( LOG_ERROR, "Unknown error while loading Accounts" );
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
	for ( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
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
			pp->setAccount( getRecord( it.data() ), false );
		++it;
	}

	QStringList::iterator sit = sockaccnames.begin();
	for ( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
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
	d->login_ = login.lower();
	d->setPassword( password );
	accounts.insert( d->login(), d );
	if ( accounts.count() == 1 ) // first account, it must be admin!
		d->setAcl( "admin" );
	d->refreshAcl();
	save(); //make sure to save it.
	return d;
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
		return it.data();
}

void cAccounts::clearAcls()
{
	iterator it = accounts.begin();

	while ( it != accounts.end() )
	{
		it.data()->refreshAcl();
		++it;
	}
}

const char* cAccount::className() const {
	return "account";
}

bool cAccount::implements( const QString& name ) const {
	if (name == "account") {
		return true;
	} else {
		return cPythonScriptable::implements(name);
	}
}

PyObject* cAccount::getPyObject() {
	return createPyObject(this);
}

PyObject* cAccount::getProperty( const QString& name ) {
	PY_PROPERTY("acl", acl());
	PY_PROPERTY("email", email());
	// \rproperty account.name The name of this account.
	PY_PROPERTY("name", login());
	PY_PROPERTY("multigems", isMultiGems() );
	PY_PROPERTY("password", password());
	PY_PROPERTY("rawpassword", password());
	PY_PROPERTY("flags", flags());
	/*
		\rproperty account.characters A list of <object id="CHAR">char</object> objects.
		This list contains all characters assigned to this account.
	*/
	if (name == "characters") {
		PyObject* list = PyList_New(characters_.size());
		for ( uint i = 0; i < characters_.size(); ++i )
			PyList_SetItem( list, i, PyGetCharObject( characters_[i] ) );
		return list;
	}
	/*
		\rproperty account.lastlogin The last login date of this account or
		an empty string if it's unknown.
	*/
	PY_PROPERTY("lastlogin", lastLogin().toString());
	PY_PROPERTY("blockuntil", blockUntil.toString());
	// \rproperty account.inuse Indicates whether this account is currently in use.
	PY_PROPERTY("inuse", inUse());
	// \rproperty account.rank Returns the integer rank of this account. This is inherited by the ACL of this account.
	PY_PROPERTY("rank", rank());

	return cPythonScriptable::getProperty(name);
}

stError* cAccount::setProperty( const QString& name, const cVariant& value ) {
	// \property account.acl The name of the ACL used to check the permissions of this account.
	if (name == "acl") {
		setAcl(value.toString());
		return 0;
	}
	// \property account.email The E-Mail address associated with this account.
	else SET_STR_PROPERTY("email", email_)
	// \property account.multigems Indicates whether Multis should be sent as Worldgems to this account.
	else if (name == "multigems") {
		setMultiGems(value.toInt() != 0);
		return 0;
	}
	/*
		\property account.password The password of this account. Please note that if MD5 hashing is activated,
		this property will only return the hashed password. But when setting this property you don't need to
		specify the MD5 hashed password as it will be automatically converted.
	*/
	else if (name == "password") {
		setPassword(value.toString());
		return 0;
	}
	/*
		\property account.rawpassword If you use MD5 hashing this property is the hashed password and no 
		conversions will be done automatically if you set this property. If you don't use MD5 hashing, 
		this property is equivalent to the password property.
	*/
	else if (name == "rawpassword") {
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
		0x00000040 multigems on/off</code>
	*/
	SET_INT_PROPERTY("flags", flags_)
	/*
		\property account.blockuntil This is the date and time when this account will be unblocked.
		The following format for the date and time is used (from the QT documentation): 
		<code>
		Qt::ISODate - ISO 8601 extended format (YYYY-MM-DD, or with time, YYYY-MM-DDTHH:MM:SS)
		</code>
	*/
	else if (name == "blockuntil") {
		QDateTime datetime = QDateTime::fromString(value.toString(), Qt::ISODate);
		setBlockUntil( datetime );
		return 0;
	}

	return cPythonScriptable::setProperty( name, value );
}

void cAccount::setAcl( const QString& d )
{
	aclName_ = d;
	refreshAcl();
}
