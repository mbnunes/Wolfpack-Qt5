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

#ifndef __CACHE_H
#define __CACHE_H


// System Includes

#include <cstdio>

class UOXFile;

#include "wolfpack.h"

// I am not above giving credit where it is due. This is the same cache that UOX currently uses.
// Thanks Zippy for doing the original designs on it, then to fur for fixing and making it run
// smoother. And to the UOX dev for doing some good work. 

class UOXFile
{
  private:

  int fmode, ok, bSize, bIndex;
  char *ioBuff;
  FILE *theFile;

  public:

  UOXFile(char *, char *);
  ~UOXFile();

  inline int qRefill(void) { return (bIndex >= bSize); };
  inline int ready(void) { return (ok); };
  void rewind(void);
  void seek(long, int);
  inline int eof(void) { return (feof(theFile)); };
  int wpgetch(void);
  void refill(void);
  char *gets(char *, int);
  int puts(char *);
  void getUChar(unsigned char *, unsigned int);
  void getUChar(unsigned char *c) { getUChar((unsigned char *) c, 1); }
  void getChar(signed char *, unsigned int);
  void getChar(signed char *c) { getChar(c, 1); }
  void getUShort(unsigned short *, unsigned int = 1);
  void getShort(signed short *, unsigned int = 1);
  void getULong(UI32 *, unsigned int = 1);
  void getLong(SI32 *, unsigned int = 1);
  void get_versionrecord(struct versionrecord *, unsigned int = 1);
  void get_st_multi(struct st_multi *, unsigned int = 1);
  void get_map_st(struct map_st *, unsigned int = 1);
  void get_st_multiidx(struct st_multiidx *, unsigned int = 1);
  void get_staticrecord(struct staticrecord *, unsigned int = 1);
};

#endif
