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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined(__MULTISCACHE_H__)
#define __MULTISCACHE_H__

#include <qvaluevector.h>
#include <qmap.h>
#include "../singleton.h"
#include "../server.h"

struct multiItem_st
{
	Q_INT16 tile;
	Q_INT16 x;
	Q_INT16 y;
	Q_INT8 z;
	bool visible;
};

class MultiDefinition
{
protected:
	QValueVector<multiItem_st> entries; // sorted list of items
	int left, right, top, bottom;
	unsigned int height, width;

	QValueVector< QValueVector<multiItem_st> > grid;
public:
	MultiDefinition();

	const QValueVector<multiItem_st> &itemsAt(int x, int y);

	unsigned int getHeight() {
		return height;
	}

	unsigned int getWidth() {
		return width;
	}

	int getLeft() {
		return left;
	}

	int getRight() {
		return right;
	}

	int getBottom() {
		return bottom;
	}

	int getTop() {
		return top;
	}

	void setItems( const QValueVector<multiItem_st>& items );
	bool inMulti( short x, short y );
	signed char multiHeight( short x, short y, short z ) const;
	QValueVector<multiItem_st> getEntries() const;
};

class cMultiCache : public cComponent
{
	QMap<ushort, MultiDefinition*> multis;
public:
	cMultiCache()
	{
	}
	virtual ~cMultiCache();

	void load();
	void unload();
	void reload();
	MultiDefinition* getMulti( ushort id );
};

typedef Singleton<cMultiCache> MultiCache;

#endif // __MULTISCACHE_H__
