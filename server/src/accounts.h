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



#if !defined(__ACCOUNTS_H__)
#define __ACCOUNTS_H__


#include "iserialization.h"
#include "typedefs.h"

// Library Includes
#include "qstring.h"
#include "qstringlist.h"
#include "qdatetime.h"
#include "qvaluevector.h"
#include "qmap.h"


// Forward Class declarations

class cChar;

class AccountRecord : public cSerializable
{
	friend class cAccounts; // my manager
private:
	QString login_;
	QString password_;
	QStringList groups_;
	QValueVector<cChar*> characters_;
	QDateTime lastLogin;
	QDateTime blockUntil;
	bool blocked_;
	int attempts_;

public:
	
	AccountRecord();

	QString login() const;

	QString password() const;
	void setPassword( const QString& );
	QValueVector<cChar*> caracterList() const;
	bool authorized( const QString& action, const QString& value ) const;
	bool addCharacter( cChar* );
	bool removeCharacter( cChar* );
	
	bool isBlocked() const;
	void resetLoginAttempts() { attempts_ = 0; };
	void loginAttemped() { ++attempts_; }
	int loginAttempts() { return attempts_; }
	void block( int seconds ) 
	{
		blockUntil = QDateTime::currentDateTime().addSecs( seconds );
	}

	void block()
	{
		blocked_ = true;
	}

	void Serialize( ISerialization& );
	QString	objectID( void ) const;
};


class cAccounts
{
private:
	QMap<QString, AccountRecord*> accounts;
	typedef QMap<QString, AccountRecord*>::iterator iterator;
	typedef QMap<QString, AccountRecord*>::const_iterator const_iterator;

public:
	enum enErrorCode {LoginNotFound, BadPassword, Banned, Wipped, NoError};

public:
	~cAccounts();
	AccountRecord* authenticate(const QString& login, const QString& password, enErrorCode* = 0) const; 
	AccountRecord* getRecord( const QString& );
	AccountRecord* createAccount( const QString& login, const QString& password );

	uint count();

	void save();
	void load();
	void reload();
	void clear();
};

// inline members
inline QString AccountRecord::login() const
{
	return login_;
}

inline QString AccountRecord::password() const
{
	return password_;
}

inline void AccountRecord::setPassword( const QString& data )
{
	password_ = data;
}

inline QValueVector<cChar*> AccountRecord::caracterList() const
{
	return characters_;
}

inline QString AccountRecord::objectID( void ) const
{
	return "ACCOUNT";
}

#endif // __ACCOUNTS_H__

