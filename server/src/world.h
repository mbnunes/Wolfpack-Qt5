//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

/*
	Definition of the cWorld class which acts as a World Loader and Saver
	and keeps track of Items which should be deleted from the World 
	(for persistance) and on top of that keeps the registers for Serial->Object
	mapping.
*/

#if !defined( __WORLD_H__ )
#define __WORLD_H__

// Library Includes
#include "qstring.h"

// Wolfpack Includes
#include "typedefs.h"
#include "singleton.h"

inline bool isItemSerial( SERIAL serial ) { return ( serial > 0x40000000 ) && ( serial < 0xFFFFFFFF ); };
inline bool isCharSerial( SERIAL serial ) { return ( serial > 0x00000000 ) && ( serial < 0x40000000 ); };

class cCharIterator
{
private:
	struct stCharIteratorPrivate *p;
public:
	cCharIterator();
	virtual ~cCharIterator();

	P_CHAR first();
	P_CHAR next();
};

class cItemIterator
{
private:
	struct stItemIteratorPrivate *p;
public:
	cItemIterator();
	virtual ~cItemIterator();

	P_ITEM first();
	P_ITEM next();
};

class cWorld
{
friend class cCharIterator;
friend class cItemIterator;

private:
	// Everything that doesn't need to be accessed via a getter or setter
	// is implemented in this private structure for compile reasons.
	class cWorldPrivate *p;
	UINT32 _charCount;
	UINT32 _itemCount;
	UINT32 lastTooltip;
	SERIAL _lastCharSerial, _lastItemSerial;

	void loadFlatstore( const QString &prefix );
	void loadSql();

	void saveFlatstore( const QString &prefix );
	void saveSql();

public:
	// Constructor/Destructor
	cWorld();
	virtual ~cWorld();
	
	// WorldLoader interface
	void load( QString basepath = QString::null, QString prefix = QString::null, QString module = QString::null );
	void save( QString basepath = QString::null, QString prefix = QString::null, QString module = QString::null );

	// Book-keeping functions
	void registerObject( cUObject *object );
	void registerObject( SERIAL serial, cUObject *object );
	void unregisterObject( cUObject *object );
	void unregisterObject( SERIAL serial );
    
	// Register an object to be deleted with the next save
	void deleteObject( cUObject *object );
	void purge();

	// Lookup Functions
	P_ITEM findItem( SERIAL serial ) const;
	P_CHAR findChar( SERIAL serial ) const;
	cUObject *findObject( SERIAL serial ) const;

	SERIAL findCharSerial() const { return _lastCharSerial + 1; }
	SERIAL findItemSerial() const { return _lastItemSerial + 1; }

	UINT32 charCount() const { return _charCount; }
	UINT32 itemCount() const { return _itemCount; }

	UINT32 getUnusedTooltip() { return ++lastTooltip; }
};

typedef SingletonHolder< cWorld > World;

#define FindCharBySerial( serial ) World::instance()->findChar( serial )
#define FindItemBySerial( serial ) World::instance()->findItem( serial )

#endif

