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

#if !defined(__COORD_H__)
#define __COORD_H__

// Platform specifics
#include "platform.h"
#include "typedefs.h"

class cUORxTarget;

class WPEXPORT Coord
{
public:
	Coord( void );
	Coord( const Coord& clCoord );
	Coord( UI16, UI16, SI08 = 0, UI08 uiMap = 0, UI08 uiPlane = 0 );
	// Operators
	Coord& operator=( const Coord& clCoord );
	bool operator==( const Coord& ) const;
	bool operator!=( const Coord& ) const;
	Coord operator+( const Coord& src ) const;
	Coord operator-( const Coord& src ) const;
	unsigned int distance( const Coord& src ) const;
	unsigned int direction( const Coord& dest ) const;
	bool lineOfSight(const Coord &dest, bool debug = false) const;

	// Utility functions for getting correct LOS points
	Coord losCharPoint(bool eye = false) const;
	Coord losItemPoint(unsigned short id) const;
	Coord losMapPoint() const;
	static Coord losTargetPoint(cUORxTarget *target, unsigned char map);

	static UI32 distance( const Coord& a, const Coord& b );

	/*!
		Displays an effect staying at this position.
	*/
	void effect( UI16 id, UI08 speed = 10, UI08 duration = 5, UI16 hue = 0, UI16 renderMode = 0 ) const;

	static Coord null;

	inline bool isInternalMap() const {
		return map == 0xFF;
	}

	inline void setInternalMap() {
		map = 0xFF;
	}

public:
	UI16 x;
	UI16 y;
	SI08 z;
	UI08 map;
};

#endif // __COORD_H__
