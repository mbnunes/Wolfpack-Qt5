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
#include "singleton.h"

// Library Includes
#include <qstring.h>
#include <qdatetime.h>
#include <qvaluevector.h>
#include <qmap.h>

// Wolfpack includes
//#include "commands.h"

// Forward Class declarations

class cChar;
class cAcl;

class AccountRecord : public cSerializable
{
	friend class cAccounts; // my manager
private:
	QString login_;
	QString password_;
	cAcl *acl_;
	QString aclName_;
	QValueVector<cChar*> characters_;
	QDateTime lastLogin_;
	QDateTime blockUntil;

	// Flags for this Account
	// 0x00000001 blocked
	// 0x00000002 allmove
	// 0x00000004 allshow
	// 0x00000008 showserials
	// 0x00000010 pagenotify
	UINT32 flags_;
	int attempts_;
	bool inUse_;

public:	
	AccountRecord();

	QString login() const;

	QString password() const;
	void remove();
	void setPassword( const QString& );
	QValueVector<cChar*> caracterList() const;
	bool authorized( const QString& action, const QString& value ) const;
	bool addCharacter( cChar* );
	bool removeCharacter( cChar* );
	bool inUse() const;
	void resetLoginAttempts() { attempts_ = 0; };
	void loginAttemped() { ++attempts_; }
	int loginAttempts() { return attempts_; }
	void block( int seconds );
	uint secsToUnblock() const;
	void setAcl( const QString &nAcl );
	QString acl() const;
	QDateTime lastLogin() const;
	void setLastLogin( const QDateTime& );
	void refreshAcl();
	void setInUse( bool data );
	void setFlags( UINT32 data );
	UINT32 flags() const;
		
	void Serialize( ISerialization& );
	QString	objectID( void ) const;

	// Flag Setters/Getters
	bool isBlocked() const;
	bool isAllMove() const;
	bool isAllShow() const;
	bool isShowSerials() const;
	bool isPageNotify() const;

	void setBlocked( bool data );
	void setAllMove( bool data );
	void setAllShow( bool data );
	void setShowSerials( bool data );
	void setPageNotify( bool data );
};


class cAccounts
{
private:
	QMap<QString, AccountRecord*> accounts;
	typedef QMap<QString, AccountRecord*>::iterator iterator;
public:
	enum enErrorCode {LoginNotFound, BadPassword, Banned, Wipped, AlreadyInUse, NoError};

public:
	~cAccounts();
	AccountRecord* authenticate(const QString& login, const QString& password, enErrorCode* = 0) const; 
	AccountRecord* getRecord( const QString& );
	AccountRecord* createAccount( const QString& login, const QString& password );

	uint count();
	void remove( AccountRecord *record );

	void save();
	void load();
	void reload();
	void clear();

	typedef QMap<QString, AccountRecord*>::const_iterator const_iterator;
	const_iterator begin() const { return accounts.begin(); }
	const_iterator end() const { return accounts.end(); }
};

// inline members
inline QString AccountRecord::acl() const
{
	return aclName_;
}

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

inline void AccountRecord::setAcl( const QString &d )
{
	aclName_ = d;
}

inline void AccountRecord::block( int seconds )
{
	blockUntil = QDateTime::currentDateTime().addSecs( seconds );
}

inline QDateTime AccountRecord::lastLogin() const
{
	return lastLogin_;
}

inline UINT32 AccountRecord::flags() const
{
	return flags_;
}

inline void AccountRecord::setLastLogin( const QDateTime& d )
{
	lastLogin_ = d;
}

inline bool AccountRecord::inUse() const
{
	return inUse_;
}

inline void AccountRecord::setInUse( bool data )
{
	inUse_ = data;
}

inline void AccountRecord::setFlags( UINT32 data )
{
	flags_ = data;
}

typedef SingletonHolder<cAccounts> Accounts;

#endif // __ACCOUNTS_H__

