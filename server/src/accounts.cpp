//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

// Wolfpack Includes
#include "accounts.h"
#include "srvparams.h"
#include "network/uosocket.h"
#include "dbdriver.h"
#include "console.h"
#include "commands.h"
#include "player.h"
#include "network.h"
#include "globals.h"
#include "persistentbroker.h"
#include "world.h"

/*****************************************************************************
  cAccount member functions
 *****************************************************************************/

cAccount::cAccount()
: acl_(0), inUse_(false), flags_(0)
{
}

/*void cAccount::Serialize( ISerialization& archive )
{
	if ( archive.isReading() )
	{
		archive.read("login", login_);
		archive.read("password", password_);
		archive.read("flags", flags_);
		archive.read("acl", aclName_);
		QString temp;
		archive.read("lastlogin", temp);
		if( !temp.isNull() && !temp.isEmpty() && temp != "0" )
			lastLogin_ = QDateTime::fromString( temp, Qt::ISODate );

		archive.read("blockuntil", temp);
		if( !temp.isNull() && !temp.isEmpty() && temp != "0" )
			blockUntil = QDateTime::fromString( temp, Qt::ISODate );

		refreshAcl(); // Reload our ACL
	}
	else // Writting
	{
		archive.write( "login", login_ );
		archive.write( "password", password_ );
		archive.write( "flags", flags_ );
		archive.write( "acl", aclName_ );

		if( lastLogin_.isValid() )
			archive.write( "lastlogin", lastLogin_.toString( Qt::ISODate ) );
		else
			archive.write( "lastlogin", QString( "0" ) );
		if( blockUntil.isValid() )
			archive.write( "blockuntil", blockUntil.toString( Qt::ISODate ) );
		else
			archive.write( "blockuntil", QString( "0" ) );
	}
	cSerializable::Serialize( archive );
}*/

bool cAccount::isBlocked() const
{
	if ( (blockUntil.isValid() && blockUntil < QDateTime::currentDateTime()) || flags_&0x00000001 )
		return true;
	else
		return false;
}

uint cAccount::secsToUnblock() const
{
	if ( isBlocked() )
		return ~0;
	else if ( blockUntil.isValid() && blockUntil < QDateTime::currentDateTime() )
		return QDateTime::currentDateTime().secsTo( blockUntil );
	return 0;
}

bool cAccount::addCharacter( P_PLAYER d )
{
	if( qFind( characters_.begin(), characters_.end(), d ) == characters_.end() )
	{
		characters_.push_back(d);
		return true;
	}
	return false;
}

bool cAccount::removeCharacter( P_PLAYER d )
{
	QValueVector<P_PLAYER>::iterator it = qFind( characters_.begin(), characters_.end(), d );
	if ( it != characters_.end() )
	{
		characters_.erase(it);
		return true;
	}
	return false;
}

bool cAccount::authorized( const QString& group, const QString& value ) const
{
	// No Valid ACL specified
	if( !acl_ )
	{// Let's try harder get one.
		acl_ = cCommands::instance()->getACL( aclName_ ); // loads if there was any specified.
		if ( !acl_ )
		{
			acl_ = cCommands::instance()->getACL( "player" );
			if ( acl_ )
				aclName_ = "player";
			else
				return false;
		}
	}
	// No group? No Access!
	QMap< QString, QMap< QString, bool > >::iterator groupIter = acl_->groups.find( group );
	if( groupIter == acl_->groups.end() )
		return false;

	// Group
	QMap< QString, bool > aGroup = groupIter.data();

	// Check if we have a rule for the specified command, if not check for any
	if(	aGroup.find( value ) != aGroup.end() )
		return aGroup[ value ];

	if( aGroup.find( "any" ) != aGroup.end() )
		return aGroup[ "any" ];

	return false;
}

void cAccount::remove()
{
	Accounts::instance()->remove( this );
}

void cAccounts::remove( cAccount *record )
{
	if( accounts.contains( record->login() ) )
		accounts.remove( record->login() );
	delete record;
}

void cAccount::refreshAcl()
{
	acl_ = cCommands::instance()->getACL( aclName_ ); 
}

bool cAccount::isAllMove() const
{
	return flags_&0x00000002;
}

bool cAccount::isAllShow() const
{
	return flags_&0x00000004;
}

bool cAccount::isShowSerials() const
{
	return flags_&0x00000008;
}

bool cAccount::isPageNotify() const
{
	return flags_&0x00000010;
}

bool cAccount::isStaff() const
{
	return flags_&0x00000020;
}

void cAccount::setBlocked( bool data )
{
	if( data )
		flags_ |= 0x00000001;
	else
		flags_ &= 0xFFFFFFFE;
}

void cAccount::setAllMove( bool data )
{
	if( data )
		flags_ |= 0x00000002;
	else
		flags_ &= 0xFFFFFFFD;
}

void cAccount::setAllShow( bool data )
{
	if( data )
		flags_ |= 0x00000004;
	else
		flags_ &= 0xFFFFFFFB;
}

void cAccount::setShowSerials( bool data )
{
	if( data )
		flags_ |= 0x00000008;
	else
		flags_ &= 0xFFFFFFF7;
}

void cAccount::setPageNotify( bool data )
{
	if( data )
		flags_ |= 0x00000010;
	else
		flags_ &= 0xFFFFFFEF;
}

void cAccount::setStaff( bool data )
{
	if( data )
		flags_ |= 0x00000020;
	else
		flags_ &= 0xFFFFFFDF;
}

/*****************************************************************************
  cAccounts member functions
 *****************************************************************************/

cAccounts::~cAccounts()
{
	clear();
}

void cAccounts::clear()
{
	iterator it = accounts.begin();
	for (; it != accounts.end(); ++it)
		delete it.data();	
}

cAccount* cAccounts::authenticate(const QString& login, const QString& password, enErrorCode* error) const
{
	const_iterator it = accounts.find(login);
	if( error )
		*error = NoError;
	if ( it != accounts.end() )
	{
		// First we check for blocked account.
		if ( it.data()->isBlocked() )
		{	
			if( error )
				*error = Banned;
			return 0;
		}

		if( it.data()->inUse() )
		{
			if( error )
				*error = AlreadyInUse;
			return 0;
		}

		// Ok, let´s continue.
		if (it.data()->password() == password)
		{
			it.data()->setLastLogin( QDateTime::currentDateTime() );
			it.data()->resetLoginAttempts();
			return it.data();
		}
		else
		{
			if( error )
				*error = BadPassword;

			return 0;
		}
	}
	else
	{
		if( error )
			*error = LoginNotFound;
		return 0;
	}
}

void cAccounts::save()
{
	// Open the Account Driver
	if( !persistentBroker->openDriver( SrvParams->accountsDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml").arg( SrvParams->accountsDriver() ) );
		return;
	}

	bool connected = false;

	try
	{
		persistentBroker->connect( SrvParams->accountsHost(), SrvParams->accountsName(), SrvParams->accountsUsername(), SrvParams->accountsPassword() );
		connected = true;

		persistentBroker->executeQuery( "BEGIN;" );

		persistentBroker->executeQuery( "DELETE FROM accounts;" );

		iterator it = accounts.begin();
		for (; it != accounts.end(); ++it)
		{
			// INSERT 
			cAccount *account = it.data();
	
			QString sql( "INSERT INTO accounts VALUES( '%1', '%2', %3, '%4', %5, %6 );" );

			sql = sql.arg( account->login_ ).arg( account->password_ ).arg( account->flags_ ).arg( account->aclName_ ).arg( !account->lastLogin_.isNull() ? account->lastLogin_.toTime_t() : 0 ).arg( !account->blockUntil.isNull() ? account->blockUntil.toTime_t() : 0 );

			persistentBroker->executeQuery( sql );
		}

		persistentBroker->executeQuery( "COMMIT;" );
	}
	catch( QString &error )
	{
		if( connected )
			persistentBroker->executeQuery( "ROLLBACK;" );
		Console::instance()->log( LOG_ERROR, QString( "Error while saving Accounts: %1." ).arg( error ) );
	}
	catch( ... )
	{
		if( connected )
			persistentBroker->executeQuery( "ROLLBACK;" );
		Console::instance()->log( LOG_ERROR, "Unknown error while saving Accounts." );
	}	
}

void cAccounts::load()
{
	// Open the Account Driver
	if( !persistentBroker->openDriver( SrvParams->accountsDriver() ) )
	{
		Console::instance()->log( LOG_ERROR, QString( "Unknown Account Database Driver '%1', check your wolfpack.xml").arg( SrvParams->accountsDriver() ) );
		return;
	}

	// Load all Accounts
	try
	{
		persistentBroker->connect( SrvParams->accountsHost(), SrvParams->accountsName(), SrvParams->accountsUsername(), SrvParams->accountsPassword() );

		cDBResult result = persistentBroker->query( "SELECT accounts.login,accounts.password,accounts.flags,accounts.acl,accounts.lastlogin,accounts.blockuntil FROM accounts;" );

		// Clear Accounts HERE
		// Here we can be pretty sure that we have a valid datasource for accounts
		clear();

		while( result.fetchrow() )
		{
			cAccount *account = new cAccount;
			account->login_ = result.getString( 0 );
			account->password_ = result.getString( 1 );
			account->flags_ = result.getInt( 2 );
			account->aclName_ = result.getString( 3 );
			account->refreshAcl();
			if( result.getInt( 4 ) != 0 )
				account->lastLogin_.setTime_t( result.getInt( 4 ) );

			if( result.getInt( 5 ) != 0 )
				account->blockUntil.setTime_t( result.getInt( 5  ) );

			accounts.insert( account->login_, account );
		}
	}
	catch( QString &error )
	{
		Console::instance()->log( LOG_ERROR, QString( "Error while loading Accounts: %1" ).arg( error ) );
	}
	catch( ... )
	{
		Console::instance()->log( LOG_ERROR, "Unknown error while loading Accounts" );
	}	
}

void cAccounts::reload()
{
	QMap< SERIAL, QString > characcnames;
	QStringList sockaccnames;

	cCharIterator iterChars;
	P_CHAR pc;
	for( pc = iterChars.first(); pc; pc = iterChars.next() )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pc);
		if( pp && pp->account() )
		{
			characcnames.insert( pp->serial(), pp->account()->login() );
		}
	}

	cUOSocket* mSock = NULL;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->account() )
			sockaccnames.push_back( mSock->account()->login() );
		else
			sockaccnames.push_back( QString() );
	}

	load();

	QMap< SERIAL, QString >::Iterator it = characcnames.begin();
	while( it != characcnames.end() )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(FindCharBySerial( it.key() ));
		if( pp )
			pp->setAccount( getRecord( it.data() ), false );
		++it;
	}

	QStringList::iterator sit = sockaccnames.begin();
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( !(*sit).isNull() )
			mSock->setAccount( getRecord( (*sit) ) );
		++sit;
	}
}

cAccount* cAccounts::createAccount( const QString& login, const QString& password )
{
	cAccount* d = new cAccount;
	d->login_ = login;
	d->password_ = password;
	accounts.insert(d->login(), d);
	if ( accounts.count() == 1 ) // first account, it must be admin!
		d->setAcl( "admin" );
	else
		d->setAcl( "player" );
	d->refreshAcl();
	save(); //make sure to save it.
	return d;
}

uint cAccounts::count()
{
	return accounts.count();
}

cAccount* cAccounts::getRecord( const QString& login )
{
	iterator it = accounts.find( login );
	if ( it == accounts.end() )
		return 0;
	else
		return it.data();
}

