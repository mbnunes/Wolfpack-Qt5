//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// Wolfpack Includes
#include "accounts.h"
#include "junk.h"
#include "srvparams.h"
#include "pfactory.h"


// ===== AccountRecord Methods ===== //

AccountRecord::AccountRecord()
{
	acl_ = cCommands::instance()->getACL("");
}

void AccountRecord::Serialize( ISerialization& archive )
{
	if ( archive.isReading() )
	{
		archive.read("login", login_);
		archive.read("password", password_);
		archive.read("blocked", blocked_);
		QString temp;
		archive.read("acl", temp);
		acl_ = cCommands::instance()->getACL(temp);
		archive.read("lastlogin", temp);
		lastLogin_.fromString(temp, Qt::ISODate);
	}
	else // Writting
	{
		archive.write("login", login_);
		archive.write("password", password_);
		archive.write("blocked", blocked_);
		if ( cCommands::instance()->isValidACL(acl_) && !acl_.key().isNull() )
			archive.write("acl", acl_.key());
		else
			archive.write("acl", "");
		archive.write("lastlogin", lastLogin_.toString(Qt::ISODate));
	}
	cSerializable::Serialize( archive );
}

bool AccountRecord::isBlocked() const
{
	if ( blockUntil < QDateTime::currentDateTime() && !blocked_ )
		return true;
	else
		return false;
}

uint AccountRecord::secsToUnblock() const
{
	if ( blocked_ )
		return ~0;
	else if ( blockUntil < QDateTime::currentDateTime() )
		return QDateTime::currentDateTime().secsTo( blockUntil );
	return 0;
}

bool AccountRecord::addCharacter( cChar* d )
{
	if (qFind( characters_.begin(), characters_.end(), d ) != characters_.end())
	{
		characters_.push_back(d);
		return true;
	}
	return false;
}

bool AccountRecord::removeCharacter( cChar* d )
{
	QValueVector<cChar*>::iterator it = qFind( characters_.begin(), characters_.end(), d );
	if ( it != characters_.end() )
	{
		characters_.erase(it);
		return true;
	}
	return false;
}

bool AccountRecord::authorized( const QString& group, const QString& value ) const
{
	QMap<QString, QMap<QString, stACLcommand> >::const_iterator it = acl_.data().find(group);
	if ( it != acl_.data().end() )
	{
		QMap<QString, stACLcommand>::const_iterator it2 = it.data().find( value );
		if ( it2 != it.data().end() )
		{
			return it2.data().permit;
		}
		else
		{
			it2 = it.data().find("any");
			if ( it2 != it.data().end() )
			{
				return it2.data().permit;
			}
			else
				false; // any not found, then it's deny.
		}
	}
	
	// for now, group "any" not implemented.
	
	return false;
}

// ===== cAccounts ===== //

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

AccountRecord* cAccounts::authenticate(const QString& login, const QString& password, enErrorCode* error) const
{
	const_iterator it = accounts.find(login);
	*error = NoError;
	if ( it != accounts.end() )
	{
		// First we check for blocked account.
		if ( it.data()->isBlocked() )
		{	
			*error = Banned;
			return 0;
		}
		// Ok, let´s continue.
		if (it.data()->password() == password)
		{
			it.data()->resetLoginAttempts();		
			return it.data();
		}
		else
		{
			it.data()->loginAttemped();
			*error = BadPassword;
			// Now we check for the number of attempts;
			if ( it.data()->loginAttempts() > SrvParams->MaxLoginAttempts() )
			{
				it.data()->block(SrvParams->AccountBlockTime());
			}
			return 0;
		}
	}
	else
	{
		*error = LoginNotFound;
		return 0;
	}
}

void cAccounts::save()
{
	ISerialization* archive = cPluginFactory::serializationArchiver( SrvParams->accountsArchiver());
	archive->prepareWritting("accounts");
	// Now save accounts
	iterator it = accounts.begin();
	for (; it != accounts.end(); ++it )
	{
		archive->writeObject( it.data() );
	}
	archive->close();
}

void cAccounts::load()
{
	ISerialization* archive = cPluginFactory::serializationArchiver( SrvParams->accountsArchiver());
	archive->prepareReading("accounts");
	for (uint i = 0; i < archive->size(); ++i)
	{
		QString objectID;
		archive->readObjectID( objectID );
		if ( objectID == "ACCOUNT" )
		{
			AccountRecord* d = new AccountRecord;
			archive->readObject( d );
			accounts.insert( d->login(), d );
		}
		else
		{
			qFatal("Error parsing account records");
		}
	}
	archive->close();
}

void cAccounts::reload()
{
	clear();
	load();
}

AccountRecord* cAccounts::createAccount( const QString& login, const QString& password )
{
	AccountRecord* d = new AccountRecord;
	d->login_ = login;
	d->password_ = password;
	accounts.insert(d->login(), d);
	if ( accounts.count() == 1 ) // first account, it must be admin!
	{
		d->setACL( cCommands::instance()->getACL("admin") );
	}
	else
		d->setACL( cCommands::instance()->getACL("player") );
	save(); //make sure to save it.
	return d;
}

uint cAccounts::count()
{
	return accounts.count();
}

AccountRecord* cAccounts::getRecord( const QString& login )
{
	iterator it = accounts.find( login );
	if ( it == accounts.end() )
		return 0;
	else
		return it.data();
}

void AccountRecord::remove()
{
	Accounts->remove( this );
}

void cAccounts::remove( AccountRecord *record )
{
	if( accounts.contains( record->login() ) )
		accounts.remove( record->login() );
	delete record;
}
