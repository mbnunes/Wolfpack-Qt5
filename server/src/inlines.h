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

#if !defined(_INLINES_H__)
#define _INLINES_H__

#undef  DBGFILE
#define DBGFILE "inlines.h"

inline int calcCharFromPtr(unsigned char *p)
{
	int serial;
	if((serial=LongFromCharPtr(p)) == INVALID_SERIAL) return -1;
	return findbyserial(charsp, serial, 1);
}

inline UOXSOCKET calcSocketFromChar(P_CHAR pc)
{
	int j;
	if (pc == NULL || pc->npc)
		return -1;
	for (j = 0; j < now; j++)
		if (currchar[j] == pc && perm[j]) return j;

	return -1;
}

inline UOXSOCKET calcSocketFromChar(CHARACTER i)
{
	return calcSocketFromChar(MAKE_CHAR_REF(i));
}

inline int calcCharFromSer(unsigned char ser1, unsigned char ser2, unsigned char ser3, unsigned char ser4)
{
	int serial;

	serial = calcserial(ser1, ser2, ser3, ser4);
	return(findbyserial(charsp, serial, 1));
}

inline int calcCharFromSer(int serial)
{
	return(findbyserial(charsp, serial, 1));
}

inline void SetTimerSec(unsigned long *timer, const short seconds)
{
	*timer=seconds * MY_CLOCKS_PER_SEC + uiCurrentTime;
}

inline bool isCharSerial(long ser) {return (ser != INVALID_SERIAL && ser >= 0 && ser <  0x40000000);}
inline bool isItemSerial(long ser) {return (ser != INVALID_SERIAL && ser >= 0 && ser >= 0x40000000);}



#endif

