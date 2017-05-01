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

#if !defined(__ACCOUNTS_H__)
#define __ACCOUNTS_H__

// Library Includes
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QMap>

// Wolfpack includes
#include "server.h"
#include "typedefs.h"
#include "singleton.h"
#include "pythonscript.h"

// Forward Class declarations

class cPlayer;
class cAcl;

class cAccount : public cPythonScriptable
{
	friend class cAccounts; // my manager
private:
	QString login_;
	QString password_;
	cAcl* acl_;
	QString aclName_;
	QList<P_PLAYER> characters_;
	QDateTime lastLogin_;
	QDateTime blockUntil;
	QByteArray email_;
	QString creationdate_;
	int totalgametime_;
	int charslots_;
	QString chatname_;
	QString ignorelist_;

	// Flags for this Account
	// 0x00000001 blocked
	// 0x00000002 allmove
	// 0x00000004 allshow
	// 0x00000008 showserials
	// 0x00000010 pagenotify
	// 0x00000020 staff - gm mode on/off
	// 0x00000040 multigems on/off
	// 0x00000080 jailed
	// 0x00000100 young
	quint32 flags_;
	int attempts_;
	bool inUse_;

public:
	cAccount();

	QString login() const;
	QString password() const;
	const QByteArray& email() const;
	const QString creationdate() const;
	const int totalgametime() const;
	const int charslots() const;
	const QString chatname() const;
	const QString ignorelist() const;
	unsigned int rank() const;
	void remove();
	QList<P_PLAYER> caracterList() const;
	bool authorized( const QString& action, const QString& value ) const;
	bool addCharacter( P_PLAYER );
	bool removeCharacter( P_PLAYER );
	bool inUse() const;
	void resetLoginAttempts()
	{
		attempts_ = 0;
	};
	void loginAttemped()
	{
		++attempts_;
	}
	int loginAttempts()
	{
		return attempts_;
	}
	void block( int seconds );
	uint secsToUnblock() const;
	void setAcl( const QString& nAcl );
	QString acl() const;
	QDateTime lastLogin() const;
	void setLastLogin( const QDateTime& );
	void setEmail( const QByteArray& email );
	void setBlockUntil( const QDateTime& d );
	void refreshAcl();
	void setInUse( bool data );
	void setFlags( quint32 data );
	void setPassword( const QString& );
	void setCreationDate( const QString& creationdate );
	void setTotalGameTime( const int totalgametime );
	void setCharSlots( const int charslots );
	void setChatName( const QString& chatname );
	void setIgnoreList( const QString& ignorelist );

	quint32 flags() const;
	QDateTime blockedUntil() const
	{
		return blockUntil;
	}

	// Flag Setters/Getters
	bool isBlocked() const;
	bool isAllMove() const;
	bool isAllShow() const;
	bool isShowSerials() const;
	bool isPageNotify() const;
	bool isStaff() const;
	bool isMultiGems() const;
	bool isJailed() const;
	bool isYoung() const;

	void setBlocked( bool data );
	void setAllMove( bool data );
	void setAllShow( bool data );
	void setShowSerials( bool data );
	void setPageNotify( bool data );
	void setStaff( bool data );
	void setMultiGems( bool data );
	void setJailed( bool data );
	void setYoung( bool data );

	// Python Scriptable Interface
	const char* className() const;
	bool implements( const QString& name ) const;
	PyObject* getPyObject();
	PyObject* getProperty( const QString& name, uint hash = 0 );
	stError* setProperty( const QString& name, const cVariant& value );
};


class cAccounts : public cComponent
{
private:
	QMap<QString, cAccount*> accounts;
	typedef QMap<QString, cAccount*>::iterator iterator;
public:
	enum enErrorCode
	{
		LoginNotFound,
		BadPassword,
		Banned,
		Wipped,
		AlreadyInUse,
		NoError
	};

public:
	~cAccounts();
	cAccount* authenticate( const QString& login, const QString& password, enErrorCode* = 0 ) const;
	cAccount* getRecord( const QString& );
	cAccount* createAccount( const QString& login, const QString& password );

	uint count();
	void remove( cAccount* record );

	void save();
	void load();
	void unload();
	void reload();
	void clearAcls();

	typedef QMap<QString, cAccount*>::const_iterator const_iterator;
	const_iterator begin() const
	{
		return accounts.begin();
	}
	const_iterator end() const
	{
		return accounts.end();
	}
};

// inline members
inline QString cAccount::acl() const
{
	return aclName_;
}

inline QString cAccount::login() const
{
	return login_;
}

inline QString cAccount::password() const
{
	return password_;
}

inline QList<P_PLAYER> cAccount::caracterList() const
{
	return characters_;
}

inline void cAccount::block( int seconds )
{
	blockUntil = QDateTime::currentDateTime().addSecs( seconds );
}

inline QDateTime cAccount::lastLogin() const
{
	return lastLogin_;
}

inline quint32 cAccount::flags() const
{
	return flags_;
}

inline const QByteArray& cAccount::email() const
{
	return email_;
}

inline const QString cAccount::creationdate() const
{
	return creationdate_;
}

inline const int cAccount::totalgametime() const
{
	return totalgametime_;
}

inline const int cAccount::charslots() const
{
	return charslots_;
}

inline const QString cAccount::chatname() const
{
	return chatname_;
}

inline const QString cAccount::ignorelist() const
{
	return ignorelist_;
}

inline void cAccount::setEmail( const QByteArray& email )
{
	email_ = email;
}

inline void cAccount::setCreationDate( const QString& creationdate )
{
	creationdate_ = creationdate;
}

inline void cAccount::setTotalGameTime( const int totalgametime )
{
	totalgametime_ = totalgametime;
}

inline void cAccount::setCharSlots( const int charslots )
{
	charslots_ = charslots;
}

inline void cAccount::setChatName( const QString& chatname )
{
	chatname_ = chatname;
}

inline void cAccount::setIgnoreList( const QString& ignorelist )
{
	ignorelist_ = ignorelist;
}

inline void cAccount::setLastLogin( const QDateTime& d )
{
	lastLogin_ = d;
}

inline bool cAccount::inUse() const
{
	return inUse_;
}

inline void cAccount::setInUse( bool data )
{
	inUse_ = data;
}

inline void cAccount::setBlockUntil( const QDateTime& d )
{
	blockUntil = d;
}

inline void cAccount::setFlags( quint32 data )
{
	flags_ = data;
}

typedef Singleton<cAccounts> Accounts;

#endif // __ACCOUNTS_H__

