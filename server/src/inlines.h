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

#if !defined(_INLINES_H__)
#define _INLINES_H__

// library Includes
#include <algorithm>
#include <math.h>
#include "qapplication.h"
#include "qstring.h"

inline QString tr( const QString& text, const char* comment = 0, const char* context = "@default" )
{
	return qApp->translate( context, text.latin1(), comment );
}

inline bool isHair( Q_UINT16 model )
{
	return	( ( ( model >= 0x203B ) && ( model <= 0x203D ) ) || ( ( model >= 0x2044 ) && ( model <= 0x204A ) ) ) ? true : false;
}

inline bool isBeard( Q_UINT16 model )
{
	return	( ( ( model >= 0x203E ) && ( model <= 0x2041 ) ) || ( ( model >= 0x204B ) && ( model <= 0x204D ) ) ) ? true : false;
}

inline bool isNormalColor( Q_UINT16 color )
{
	return ( ( color >= 2 ) && ( color < 0x3ea ) ) ? true : false;
}

inline bool isSkinColor( Q_UINT16 color )
{
	return ( ( color >= 0x3EA ) && ( color <= 0x422 ) ) ? true : false;
}

inline bool isHairColor( Q_UINT16 color )
{
	return ( ( color >= 0x44E ) && ( color <= 0x47D ) ) ? true : false;
}

inline int round(double n) {
    double f = n - floor(n);
	if (f >= 0.50) {
		return (int)ceil(n);
	} else {
		return (int)floor(n);
	}
}

inline bool isBetween(double n, int lower, int higher, double tolerance = 0.5) {
	// Swap the bounds if they are out of order
	if (lower > higher) {
		std::swap(lower, higher);
	}

	return (n > lower - tolerance) && (n < higher + tolerance);
}

#endif
