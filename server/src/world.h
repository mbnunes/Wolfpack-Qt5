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

#if !defined( __WORLD_H__ )
#define __WORLD_H__

// Wolfpack Includes
#include "typedefs.h"
#include "singleton.h"

// Library Includes
#include <qvaluevector.h>
#include "server.h"

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

class cWorld : public cComponent
{
friend class cCharIterator;
friend class cItemIterator;

private:
	// Everything that doesn't need to be accessed via a getter or setter
	// is implemented in this private structure for compile reasons.
	class cWorldPrivate *p;
	unsigned int _charCount;
	unsigned int _itemCount;
	unsigned int lastTooltip;
	SERIAL _lastCharSerial, _lastItemSerial;

public:
	// Constructor/Destructor
	cWorld();
	virtual ~cWorld();

	// WorldLoader interface
	void load();
	void unload();
	void save();

	// For the "settings" table
	void getOption( const QString name, QString &value, const QString fallback, bool newconnection = true );
	void setOption( const QString name, const QString value, bool newconnection = true );

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

	unsigned int charCount() const { return _charCount; }
	unsigned int itemCount() const { return _itemCount; }

	unsigned int getUnusedTooltip() { return ++lastTooltip; }
};

typedef SingletonHolder< cWorld > World;

#define FindCharBySerial( serial ) World::instance()->findChar( serial )
#define FindItemBySerial( serial ) World::instance()->findItem( serial )
inline bool isItemSerial( SERIAL serial ) { return ( serial > 0x40000000 ) && ( serial < 0xFFFFFFFF ); };
inline bool isCharSerial( SERIAL serial ) { return ( serial > 0x00000000 ) && ( serial < 0x40000000 ); };


#endif

