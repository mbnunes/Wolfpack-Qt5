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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

// basics.h: interface for the basics functions.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__BASICS_H__)
#define __BASICS_H__

// Platfrom specifics
#include "platform.h"

// System includes

#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <cstdio>

using namespace std ;
/*
//#if defined(_MSC_VER) 
//#pragma warning (push)
//#pragma warning (disable : 4244) // lets ger rid of the warnings! they are ok here
//#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include <cstdlib>
//#include <cctype>
//#include <string>
//#ifndef _MSC_VER
//#include <cstdio>
//#endif

using namespace std ;
*/
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

int makenum2(char *s);

#if 1
#define str2num atoi
#else
inline int str2num(char *s) // Convert string to integer
{
	unsigned int i;
	int n=0;
	int neg=0;
	unsigned int length=strlen((char*)s);
	for(i=0;i<length;i++)
	{
		if (s[i]==' ') continue;
		if (s[i]=='-') neg=1;
		n*=10; // Multiply by 10
		if (isdigit(s[i]))
			n=n+(s[i])-48; // Convert char to number from 0 to 9
	}
	if (neg) n=-n;
	return n;
}
#endif

inline int hstr2num(char *s) // Convert hex string to integer
{
	unsigned int i;
	int n=0;

	for (i=0;i<strlen((char*)s);i++)
	{
		n*=16;
		if (isdigit(s[i]))
			n=n+(s[i])-48; // Convert char to number from 0 to 9
		if ((s[i]>=65) && (s[i]<=70)) // Uppercase A-F
			n=n+(s[i])-65+10;
		if ((s[i]>=97) && (s[i]<=102)) // Lowercase A-F
			n=n+(s[i])-97+10;
	}
	if (s[0]=='-') n=-n;
	return n;
}



#endif 
