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

#if !defined(__TIMERS_H__)
#define __TIMERS_H__

//Platform specifics
#include "platform.h"

//System includes
#include <vector>
#include <functional>
#include <algorithm>

//Forward class declarations
class cTimer;
class cScriptTimer;
class cTimers;
class cBufferedReader;
class cBufferedWriter;

// Wolfpack includes
#include "typedefs.h"
#include "singleton.h"

#include <qstring.h>

class cDBResult;

class cTimer
{
protected:
	SERIAL sourSer;
	SERIAL destSer;
	QString objectid;
	bool serializable;

public:
	unsigned int expiretime;
	unsigned char dispellable;

	// Fib Heap Node variables
	cTimer* left;
	cTimer* right;
	cTimer* father;
	cTimer* son;
	unsigned int rank;
	bool marker;

	/*
	 * Provided for subclasses to save additional information.
	*/
	void saveFloat( unsigned int id, const QString& key, double value );
	void saveInt( unsigned int id, const QString& key, int value );
	void saveString( unsigned int id, const QString& key, const QString& value );
	void saveChar( unsigned int id, const QString& key, P_CHAR character );
	void saveItem( unsigned int id, const QString& key, P_ITEM item );
	bool loadFloat( unsigned int id, const QString& key, double& value );
	bool loadInt( unsigned int id, const QString& key, int& value );
	bool loadString( unsigned int id, const QString& key, QString& value );
	bool loadChar( unsigned int id, const QString& key, P_CHAR& character );
	bool loadItem( unsigned int id, const QString& key, P_ITEM& item );

	virtual void load(cBufferedReader &reader, unsigned int version);
	virtual void save(cBufferedWriter &writer, unsigned int version);

	//	cTimer() { serializable = true; }
	cTimer( cTimer* left_ = NULL, cTimer* right_ = NULL, cTimer* father_ = NULL, cTimer* son_ = NULL, int rank_ = 0, bool marker_ = false )
	{
		serializable = true;
		left = left_;
		right = right_;
		if ( !left )
			left = this;
		if ( !right )
			right = this;
		father = father_;
		son = son_;
		rank = rank_;
		marker = marker_;
		dispellable = false;	// Most Effects are NOT dispellable by default
	}

	virtual ~cTimer()
	{
	}

	bool operator<( const cTimer& a ) const
	{
		return expiretime < a.expiretime;
	}

	void setExpiretime_s( int seconds );
	void setExpiretime_ms( float milliseconds );
	void setDest( int ser );
	int getDest();
	void setSour( int ser );
	int getSour();
	void On( P_CHAR pc )
	{
		Q_UNUSED( pc );
	}
	void Off( P_CHAR pc )
	{
		Q_UNUSED( pc );
	}
	virtual void Expire() = 0;
	virtual void Dispel( P_CHAR pSource, bool silent = false )
	{
		Q_UNUSED( pSource ); Q_UNUSED( silent );
	}
	virtual QString objectID() const
	{
		return objectid;
	}
	bool isSerializable( void )
	{
		return serializable;
	}
	void setSerializable( bool data )
	{
		serializable = data;
	}

	virtual void load( unsigned int id, const char** result );
	virtual void save( unsigned int id );

	std::vector< cTimer* > asVector();
};

class cDelayedOnCreateCall : public cTimer
{
	SERIAL objSer_;
	const QString def_;
public:
	cDelayedOnCreateCall( cUObject*, const QString& definition );
	void Expire();
	QString objectID() const
	{
		return "cDelayedOnCreateCall";
	}
};

class cTimers
{
private:

	struct ComparePredicate : public std::binary_function<cTimer*, cTimer*, bool>
	{
		bool operator()( const cTimer* a, const cTimer* b )
		{
			return a->expiretime > b->expiretime;
		}
	};

public:
	cTimers();
	~cTimers();

	std::vector<cTimer*> teffects;

	void load();
	void save();
	void save(cBufferedWriter &writer);
	void load(cBufferedReader &reader);

	void check();
	void dispel( P_CHAR pc_dest, P_CHAR pSource, bool silent = false );
	void dispel( P_CHAR pc_dest, P_CHAR pSource, const QString& type, bool silent = false, bool onlyDispellable = true );

	void insert( cTimer* pT );
	void erase( cTimer* pT );

	int size()
	{
		return teffects.size();
	}

	int countSerializables();
};

typedef SingletonHolder<cTimers> Timers;

#endif
