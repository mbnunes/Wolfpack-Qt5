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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

#ifndef __SCRIPTC_H__
#define __SCRIPTC_H__

#include <map>
#include <string>

using namespace std ;
typedef unsigned long scpMark;

// One entry in the cache
struct ScriptEntry {
	string name;
    unsigned long offset;
};


bool openscript (char *name, bool shutdownonfail = true);
void closescript ();
void read1 ();
void read2 ();
extern char script1[512];

#define SCP_PRELOADABLE	0x0008
#define SCP_PRELOADED	0x0004

#ifndef _MSC_VER
typedef long int _off_t ;
#endif


class Script
{
public:
    Script(const char *_filename, short _flags=0);
	~Script();
	bool Open();
	void Close() {if (!(flags & SCP_PRELOADED)) closescript();}
	Script* Select(char* sect, short custom);
	scpMark Suspend();
	void Resume(scpMark mark);
	bool NextLine();
	void NextLineSplitted();
	int  CmpTok1(char* comp) {return strcmp((char*)script1,comp);}

    char find(const char *section);
    char isin(const char *section);
        
    
private:
	bool preload();
    void reload();
	bool ReadMemLine();
	void MakeIndexForFile();
	void MakeIndexForMem();
    
	map<string, ScriptEntry> entries;
    time_t last_modification;
    char *filename;
	char *img;
	_off_t scpsize;
	char *curmempos;	// current read position for preloaded files
	short flags;
};

#endif
