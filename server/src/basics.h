//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// basics.h: interface for the basics functions.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__BASICS_H__)
#define __BASICS_H__

// Platfrom specifics
#include "platform.h"
#include "qstring.h"

// System includes

#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <cstdio>

using namespace std ;

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


// LB, wrapping of the stdlib num-2-str functions
inline void numtostr(int i, char *ourstring) { sprintf(ourstring,"%d",i) ;}
//inline void numtostr(unsigned int i, char *string) { sprintf(string,"%u",i) ;}
inline void hextostr(int i, char *ourstring)       { sprintf (ourstring, "%x",i) ; }
//inline void hextostr(unsigned int i, char *string)      { sprintf (string, "%x",i) ; }
//#else 
//inline void numtostr(int i,char *ourstring)	{itoa(i,ourstring,10);}
//inline void hextostr(int i,char *ourstring)	{itoa(i,ourstring,16);}
//#endif

#define BASE_INARRAY -1
#define BASE_AUTO 0
#define BASE_BIN 2
#define BASE_OCT 8
#define BASE_DEC 10
#define BASE_HEX 16

// sereg : roll dices d&d style
int rollDice( QString dicePattern );

// Xan : conversion from sz to numbers
int str2num (char *sz, int base = BASE_AUTO);

int str2num (string sz,int base= BASE_AUTO) ;

// Xan : new style hexstring to number
inline int hex2num (char *sz)
{
	return str2num(sz, BASE_HEX);
}

inline int hex2num (string sz)
{
	return str2num(sz,BASE_HEX) ;
}

#endif 
