//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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


#if !defined(__MAPS_H__)
#define __MAPS_H__

#include "exceptions.h"
#include <qglobal.h>
#include <qstring.h>
#include <qmap.h>
#include <qvaluevector.h>
#include <iterator>

// Forward definitions
class MapsPrivate;
class Coord_cl;
class wpException;

// Structures
#pragma pack (1)
struct map_st {
	unsigned short id;
	signed char z;
};

struct staticrecord {
	Q_UINT16 itemid;
	Q_UINT8  xoff;
	Q_UINT8  yoff;
	Q_INT8   zoff;
};
#pragma pack()

class StaticsIterator
{
public:
    /**
     * Typedefs, STL conformance.
     */
    typedef std::bidirectional_iterator_tag  iterator_category;
    typedef ptrdiff_t  difference_type;

private:
	QValueVector<staticrecord> staticArray;
	uint baseX, baseY;
	uint pos;

private:
	void inc();
	void dec();
	void load( MapsPrivate*, ushort x, ushort y, bool exact );
	StaticsIterator();

public:
	StaticsIterator(ushort x, ushort y, MapsPrivate* d, bool exact = true);
	~StaticsIterator() { };

	bool atEnd() const;
	void reset();
	const staticrecord& data() const;

	// prefix Operators
    StaticsIterator& operator++();
    StaticsIterator& operator--();
	// postfix Operators
    StaticsIterator operator++(int);
    StaticsIterator operator--(int);
	const staticrecord*   operator->() const;
	const staticrecord&   operator*() const;

};

class Maps
{
	QMap<uint, MapsPrivate*> d;
	QString basePath;

	typedef QMap<uint, MapsPrivate*>::Iterator iterator;
	typedef QMap<uint, MapsPrivate*>::const_iterator const_iterator;

public:
	Maps( const QString& basePath );
	~Maps();

	bool registerMap( uint id, const QString& mapfile, uint mapwidth, uint mapheight, const QString& staticsfile, const QString& staticsidx );

	unsigned int mapPatches(unsigned int id);
	unsigned int staticPatches(unsigned int id);
	map_st seekMap( uint id, ushort x, ushort y ) const;
	map_st seekMap( const Coord_cl& ) const;
	bool hasMap( uint id ) const;
	signed char mapElevation( const Coord_cl& p ) const;
	signed char mapAverageElevation( const Coord_cl& p, int* top = 0, int* botton = 0 ) const;
	signed char dynamicElevation(const Coord_cl& pos) const;
	signed char height(const Coord_cl& pos);
	uint mapTileWidth( uint ) const;
	uint mapTileHeight( uint ) const;
	signed char staticTop(const Coord_cl& pos) const;
	bool canFit( int x, int y, int z, int map, int height ) const;
	StaticsIterator staticsIterator( uint id, ushort x, ushort y, bool exact = true ) const throw( wpException );
	StaticsIterator staticsIterator( const Coord_cl&, bool exact = true ) const throw( wpException );
};

// Inline member functions
inline const staticrecord& StaticsIterator::data() const
{
	return staticArray.at(pos);
}

inline bool StaticsIterator::atEnd() const
{
	return pos == staticArray.size();
}

inline void StaticsIterator::reset()
{
	pos = 0;
}

inline StaticsIterator& StaticsIterator::operator++() 
{
	inc();
	return *this;
}

inline StaticsIterator& StaticsIterator::operator--() 
{
	dec();
	return *this;
}

inline StaticsIterator StaticsIterator::operator--(int) 
{
	StaticsIterator tmp = *this;
	dec();
	return tmp;
}

inline StaticsIterator StaticsIterator::operator++(int) 
{
	StaticsIterator tmp = *this;
	inc();
	return tmp;
}

inline const staticrecord* StaticsIterator::operator->() const
{
	return &staticArray[pos];
}

inline const staticrecord& StaticsIterator::operator*() const
{
	return data();
}

#endif // __MAPS_H__

