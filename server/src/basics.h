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

// basics.h: interface for the basics functions.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__BASICS_H__)
#define __BASICS_H__

// Platfrom specifics
#include "platform.h"

#include <functional>

// Forward definitions

class Coord_cl;
class QString;

//////////////////////////////////////////////////////////////////////
// name:	LongFromCharPtr, ShortFromCharPtr
// history:	init by Duke, 10.9.2000
// purpose:	calculates a long int from 4 subsequent bytes pointed to by 'p',
//			assuming 'normal' byte order (NOT intel style)
//			intended to replace calcserial() as follows
//			  serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
//			  serial=LongFromCharPtr(buffer[s]+7);

inline long LongFromCharPtr(const unsigned char *p){return (*p<<24) | (*(p+1)<<16) | (*(p+2)<<8) | *(p+3);}
inline SI16 ShortFromCharPtr(const unsigned char *p){return static_cast<SI16>((*p<<8) | *(p+1));}

//////////////////////////////////////////////////////////////////////
// Name:	LongToCharPtr, ShortToCharPtr
// Purpose: stores a long int into 4 subsequent bytes pointed to by 'p',
//			assuming 'normal' byte order (NOT intel style)
// History:	init Duke, 13.8.2000
inline void LongToCharPtr(const unsigned long i, unsigned char *p)
{
	*p = static_cast<UI08>(i>>24); *(p+1) = static_cast<UI08>(i>>16); *(p+2) = static_cast<UI08>(i>>8); *(p+3) = static_cast<UI08>(i);
}

inline void ShortToCharPtr(const unsigned short i, unsigned char *p)
{
	*p = static_cast<UI08>(i>>8); *(p+1) = static_cast<UI08>(i);	// no %256 for 2nd byte, truncation will occur anyway
}

// sereg : roll dices d&d style
int rollDice( const QString& dicePattern );

bool parseCoordinates( const QString &input, Coord_cl &coord );

int RandomNum(int nLowNum, int nHighNum);

QString hex2dec( const QString& value );

float RandomFloatNum(float nLowNum, float nHighNum);
unsigned int getNormalizedTime();

template<class T>
struct destroy : std::unary_function<T, void> {
	void operator()(T& d) const
	{ 
		delete d; 
		d = 0;
	}
};

#endif 

