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

#if !defined( __BASEDEF_H__ )
#define __BASEDEF_H__

#include "definable.h"
#include "singleton.h"
#include <qmap.h>

class cElement;

class cCharBaseDef : public cDefinable
{
	friend class cCharBaseDefs;
protected:
	// Our id
	QCString id_;

	// Sounds
	unsigned short basesound_;
	unsigned char soundmode_;
	unsigned int flags_;
	unsigned char type_;
	unsigned short figurine_;
	unsigned short minDamage_;
	unsigned short maxDamage_;
	short minTaming_;
	QCString carve_;
	QCString lootPacks_;
	QCString bindmenu_;
	unsigned char controlSlots_;
	unsigned char criticalHealth_;

	// Misc Properties
	bool loaded;
	void load();
	void reset();
public:
	cCharBaseDef( const QCString& id );
	~cCharBaseDef();

	void processNode( const cElement* node );

	inline unsigned char controlSlots()
	{
		load();
		return controlSlots_;
	}

	inline unsigned char criticalHealth()
	{
		load();
		return criticalHealth_;
	}

	inline const QCString& id() const
	{
		return id_;
	}

	inline unsigned short basesound()
	{
		load();
		return basesound_;
	}

	inline unsigned char soundmode()
	{
		load();
		return soundmode_;
	}

	inline unsigned short figurine()
	{
		load();
		return figurine_;
	}

	inline unsigned int flags()
	{
		load();
		return flags_;
	}

	inline bool canFly()
	{
		load();
		return ( flags_ & 0x01 ) != 0;
	}

	inline bool antiBlink()
	{
		load();
		return ( flags_ & 0x02 ) != 0;
	}

	inline bool noCorpse()
	{
		load();
		return ( flags_ & 0x04 ) != 0;
	}

	inline unsigned short maxDamage()
	{
		load();
		return maxDamage_;
	}

	inline unsigned short minDamage()
	{
		load();
		return minDamage_;
	}

	inline short minTaming()
	{
		load();
		return minTaming_;
	}

	inline const QCString& carve()
	{
		load();
		return carve_;
	}

	inline const QCString& lootPacks()
	{
		load();
		return lootPacks_;
	}

	inline const QCString &bindmenu()
	{
		load();
		return bindmenu_;
	}
};

class cCharBaseDefs
{
protected:
	typedef QMap<QCString, cCharBaseDef*> Container;
	typedef Container::iterator Iterator;
	Container definitions;
public:
	cCharBaseDefs();
	~cCharBaseDefs();

	// Get a base definition
	// This is guaranteed to return a basedef. Even if uninitialized.
	cCharBaseDef* get( const QCString& id );

	// When reset is called, all loaded basedefs are unflagged.
	void reset();
};

typedef SingletonHolder<cCharBaseDefs> CharBaseDefs;

#endif
