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

//##ModelId=3C5D92F2005D
class UOXFile
{
  private:

  int fmode, ok, bSize, bIndex;
	//##ModelId=3C5D92F2007B
  char *ioBuff;
	//##ModelId=3C5D92F20099
  FILE *theFile;

  public:

	//##ModelId=3C5D92F200AD
  UOXFile(char *, char *);
	//##ModelId=3C5D92F200C2
  ~UOXFile();

	//##ModelId=3C5D92F200CB
  inline int qRefill(void) { return (bIndex >= bSize); };
	//##ModelId=3C5D92F200DF
  inline int ready(void) { return (ok); };
	//##ModelId=3C5D92F200F3
  void rewind(void);
	//##ModelId=3C5D92F20108
  void seek(long, int);
	//##ModelId=3C5D92F2011C
  inline int eof(void) { return (feof(theFile)); };
	//##ModelId=3C5D92F20130
  int wpgetch(void);
	//##ModelId=3C5D92F2013B
  void refill(void);
	//##ModelId=3C5D92F2014E
  char *gets(char *, int);
	//##ModelId=3C5D92F2016C
  int puts(char *);
	//##ModelId=3C5D92F20194
  void getUChar(unsigned char *, unsigned int);
	//##ModelId=3C5D92F201A8
  void getUChar(unsigned char *c) { getUChar((unsigned char *) c, 1); }

	//##ModelId=3C5D92F201BC
  void getChar(signed char *, unsigned int);
	//##ModelId=3C5D92F201DA
  void getChar(signed char *c) { getChar(c, 1); }

	//##ModelId=3C5D92F201E4
  void getUShort(unsigned short *, unsigned int = 1);
	//##ModelId=3C5D92F20202
  void getShort(signed short *, unsigned int = 1);
	//##ModelId=3C5D92F20216
  void getULong(UI32 *, unsigned int = 1);
	//##ModelId=3C5D92F20234
  void getLong(SI32 *, unsigned int = 1);  

	//##ModelId=3C5D92F20248
  void get_versionrecord(struct versionrecord *, unsigned int = 1);
	//##ModelId=3C5D92F20266
  void get_st_multi(struct st_multi *, unsigned int = 1);
	//##ModelId=3C5D92F2027B
  void get_land_st(struct land_st *, unsigned int = 1);
	//##ModelId=3C5D92F20298
  void get_tile_st(struct tile_st *, unsigned int = 1);
	//##ModelId=3C5D92F202AC
  void get_map_st(struct map_st *, unsigned int = 1);
	//##ModelId=3C5D92F202CA
  void get_st_multiidx(struct st_multiidx *, unsigned int = 1);
	//##ModelId=3C5D92F202DE
  void get_staticrecord(struct staticrecord *, unsigned int = 1);
};

#endif
