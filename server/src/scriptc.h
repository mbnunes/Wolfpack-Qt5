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

#ifndef __SCRIPTC_H__
#define __SCRIPTC_H__

#include <map>
#include <string>

using namespace std ;
//##ModelId=3C5D92D10222
typedef unsigned long scpMark;

// One entry in the cache
//##ModelId=3C5D92D1025F
struct ScriptEntry {
	//##ModelId=3C5D92D10292
	string name;
	//##ModelId=3C5D92D102A5
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
//##ModelId=3C5D92D102C3
typedef long int _off_t ;
#endif


//##ModelId=3C5D92D103B3
class Script
{
public:
	//##ModelId=3C5D92D103D1
    Script(const char *_filename, short _flags=0);
	//##ModelId=3C5D92D103E5
	~Script();
	//##ModelId=3C5D92D20007
	bool Open();
	//##ModelId=3C5D92D20011
	void Close() {if (!(flags & SCP_PRELOADED)) closescript();}
	//##ModelId=3C5D92D20012
	Script* Select(char* sect, short custom);
	//##ModelId=3C5D92D20061
	scpMark Suspend();
	//##ModelId=3C5D92D20062
	void Resume(scpMark mark);
	//##ModelId=3C5D92D20075
	bool NextLine();
	//##ModelId=3C5D92D2007F
	void NextLineSplitted();
	//##ModelId=3C5D92D20089
	int  CmpTok1(char* comp) {return strcmp((char*)script1,comp);}

	//##ModelId=3C5D92D2009D
    char find(const char *section);
	//##ModelId=3C5D92D200B1
    char isin(const char *section);
        
    
private:
	//##ModelId=3C5D92D200BC
	bool preload();
	//##ModelId=3C5D92D200CF
    void reload();
	//##ModelId=3C5D92D200D9
	bool ReadMemLine();
	//##ModelId=3C5D92D200ED
	void MakeIndexForFile();
	//##ModelId=3C5D92D200F7
	void MakeIndexForMem();
    
	//##ModelId=3C5D92D20185
	map<string, ScriptEntry> entries;
	//##ModelId=3C5D92D201AC
    time_t last_modification;
	//##ModelId=3C5D92D201CA
    char *filename;
	//##ModelId=3C5D92D201E8
	char *img;
	//##ModelId=3C5D92D201FC
	_off_t scpsize;
	//##ModelId=3C5D92D2021A
	char *curmempos;	// current read position for preloaded files
	//##ModelId=3C5D92D20238
	short flags;
};

#endif
