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

/*
 Cache script section locations by <erwin@andreasen.com>
 Reads through the contents of the file and saves location of each
 SECTION XYZ entry

 Calling Script::find() will then seek to that location directly rather
 than having to parse through all of the script
 */

#include "wolfpack.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
#include <sys/stat.h>
#include <errno.h>

#include "mstring.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "scriptc.cpp"

extern FILE* scpfile;

char get_modification_date(const char *filename, time_t* mod_time) 
{
    struct stat stat_buf;
    
    if ((stat(filename, &stat_buf)))
	{
        fprintf(stderr, "Cannot stat %s: %s", filename, strerror(errno));
        exit(1);
    }

    *mod_time = stat_buf.st_mtime;
    return 1;
}

bool Script::preload()	// load the complete script into memory (Duke,6.1.2001)
{
    FILE *fp;
    struct stat stat_buf;
    
    if (stat(filename, &stat_buf))
        return false;

    scpsize = stat_buf.st_size;

	if (flags & SCP_PRELOADED)
		delete img;
	img = new char[scpsize];
	curmempos=img;
    if (!(fp = fopen(filename, "rb")))
	{
        fprintf(stderr, "Cannot open %s: %s", filename, strerror(errno));
        exit(1);
	}
	int r=fread(img, sizeof( char ), scpsize, fp);	// read in the whole file
	if (r!=scpsize)
	{
		LogErrorVar("problem preloading %s",filename);
	}
	else
		flags |= SCP_PRELOADED;
	fclose(fp);
    return true;

}

void Script::reload()
{
	if(flags & SCP_PRELOADABLE)
	{
		preload();
		MakeIndexForMem();
	}
	else
		MakeIndexForFile();
}

#if 1
void Script::MakeIndexForFile()
{
	FILE *fp;
    char buf[1024], section_name[256];
    int count = 0;
    
    entries.clear();
    
    if (!(fp = fopen(filename, "r")))
	{
        fprintf(stderr, "Cannot open %s: %s", filename, strerror(errno));
        exit(1);
    }
    clConsole.send("Reloading %-15s: ", filename); fflush (stdout);

    // Snarf the part of SECTION... until EOL
	int loopexit=0;
    while( (fgets(buf, sizeof(buf), fp)) && (++loopexit < MAXLOOPS) )
        if (sscanf(buf, "SECTION %256[^\n]", section_name) == 1)
		{

			if (section_name[0] == 'x')
			{
				std::string sScript1 = section_name, sToken1;
				int sEnd = sScript1.rfind("x", 0), lastpos;
				while (sEnd != -1)
				{
					lastpos = sEnd;
					sEnd = sScript1.find("x", sEnd+1);
					sToken1 = sScript1.substr(lastpos, sEnd-lastpos);
					ScriptEntry section;
					section.name = sToken1;
					section.offset = ftell(fp);
					entries.insert(make_pair(section.name, section));
				}
			} else {
				ScriptEntry section;
				section.name = section_name;
				section.offset =  ftell(fp);
				entries.insert(make_pair(section.name, section));
			}
        }

    clConsole.send("%6d sections found.\n", entries.size());
    
    fclose(fp);
}

#elif
void Script::MakeIndexForFile()
{
	fstream myscpfile;
	mstring sInput;
	entries.clear();
	vector<mstring>	vecLine ;
	myscpfile.open(filename, ios::in);
	if (myscpfile.is_open())
	{
		clConsole.send("Reloading %-15s: ", filename); fflush (stdout);
		while(getline(myscpfile,sInput)) 
		{
			sInput.trim();
			// Ok, unfortunately, as we have no standard syntax for our SECTIONS, we just want the 
			// the first string
			mstring sFirst;
			SI32 siEnd = 0;
			siEnd = sInput.find(" ");
			if ((siEnd != string::npos)&&(siEnd!=0))
			{
				sFirst = sInput.substr(0,siEnd);
				sInput = sInput.substr(siEnd+1);
			}
			else
			{
				// No first word
				sFirst = sInput;
				sInput.erase();
			}
			if (!sFirst.empty())
			{

				if (sFirst == "SECTION")
				{
					// We are in a section, need to parse the section name
					
					
					if (sInput.find("x")==0)
					{
						vector<mstring> vecEntries ;
						mstring sEntries ;
						// strip off the leading x
						//sEntries = sInput.substr(1) ;
						vecEntries = sEntries.split("x") ;
						int i;
						for (i = 0; i< vecEntries.size(); i++)
						{
							
							ScriptEntry section;
							section.name = vecEntries[i] ;
							section.offset = myscpfile.tellg() ;
							entries.insert(make_pair(section.name,section)) ;
						}
						vecEntries.clear() ;
					}
					else
					{
						ScriptEntry section;
						section.name = sInput;
						section.offset = myscpfile.tellg();
						entries.insert(make_pair(section.name,section)) ;
					}
					vecLine.clear() ;
				}
			}
		}
		clConsole.send("%6d sections found.\n", entries.size()) ;
		myscpfile.close() ;
	}
	else
	{
		cerr<< "Cannot open " << filename << ": " << strerror(errno) <<endl;
		exit(1) ;
	}



/*
	FILE *fp;
    char buf[1024], section_name[256];
    int count = 0;
    
    entries.clear();
    
    if (!(fp = fopen(filename, "r")))
	{
        fprintf(stderr, "Cannot open %s: %s", filename, strerror(errno));
        exit(1);
    }
    clConsole.send("Reloading %-15s: ", filename); fflush (stdout);

    // Snarf the part of SECTION... until EOL
	int loopexit=0;
    while( (fgets(buf, sizeof(buf), fp)) && (++loopexit < MAXLOOPS) )
        if (sscanf(buf, "SECTION %256[^\n]", section_name) == 1)
		{

			if (section_name[0] == 'x')
			{
				mstring sScript1(section_name);
				sScript1.trim();
				vector<mstring> Tokens = sScript1.split("x");
				int i;
				for (i = 0; i < Tokens.size(); i++)
				{
					ScriptEntry section;
					section.name = Tokens[i];
					section.offset = ftell(fp);
					entries.insert(make_pair(section.name, section));
				}
			} else {
				ScriptEntry section;
				section.name = section_name;
				section.offset =  ftell(fp);
				entries.insert(make_pair(section.name, section));
			}
        }

    clConsole.send("%6d sections found.\n", entries.size());
    
    fclose(fp);
*/
}
#endif

void Script::MakeIndexForMem()
{
    char section_name[256];
    
    entries.clear();   // we need to figure out why this was crashing on Mingw32

    if (flags & SCP_PRELOADED)
	{
		int loopexit=0;
		while( NextLine() && (++loopexit < MAXLOOPS) )
		{
			if (sscanf((char*)temp, "SECTION %256[^\n]", section_name) == 1)
			{
				if (section_name[0] == 'x')
				{
					mstring sScript1(section_name);
					vector<mstring> Tokens = sScript1.split("x");
					unsigned int i;
					for (i = 0; i < Tokens.size(); i++)
					{
						ScriptEntry section;
						section.name = Tokens[i];
						section.offset = (unsigned long) curmempos;
						entries.insert(make_pair(section.name, section));
					}
				} 
				else 
				{
					ScriptEntry section;
					section.name = section_name;
					section.offset =  (unsigned long) curmempos;
					entries.insert(make_pair(section.name, section));
				}
			}
		}
	}
}

// Parse this script, caching section positions
Script::Script(const char *_filename, short _flags)
{
	flags = _flags;
	filename = NULL;
    filename = strdup(_filename);
    
    get_modification_date(filename, &last_modification);

    reload();
}

Script::~Script()
{
	if ( filename != NULL )
		delete filename;
}

// Look for that section in this previously parsed script file
char Script::find(const char *section) {
    time_t current;
    
    get_modification_date(filename, &current);

    if (current > last_modification)
	{
        reload();
        last_modification = current;
    }
	std::string dummy = section;
	std::map<std::string, ScriptEntry>::iterator iter_entry = entries.find(dummy);


    if (iter_entry == entries.end())
        return 0;

	if (flags & SCP_PRELOADED)
		curmempos=(char*)(iter_entry->second.offset);
	else
		fseek(scpfile, iter_entry->second.offset, SEEK_SET);

    return 1;
}

bool Script::Open()
{
	if (flags & SCP_PRELOADED)
	{
		curmempos=img;
	}
	else
		openscript(filename);
	return true;
}

Script* Script::Select(char* sect, short custom)
{
	Script * pScp;
	this->Open();
	if (this->find(sect))
		return this;

	this->Close();
	if (n_scripts[custom][0]!=0)	// do we have a name for the alternate script ?
	{
		pScp=i_scripts[custom];
		pScp->Open();
		if (pScp->find(sect))
			return pScp;
		
		pScp->Close();
	}
	return NULL;
}

scpMark Script::Suspend()
{
	scpMark pos;
	if (flags & SCP_PRELOADED)
		pos=(scpMark)curmempos;
	else
		pos=ftell(scpfile);
	this->Close();
	return pos;
}

void Script::Resume(scpMark mark)
{
	this->Open();
	if (flags & SCP_PRELOADED)
		curmempos=(char *)mark;
	else
		fseek(scpfile, mark, SEEK_SET);
}

bool Script::ReadMemLine()	// 
{
	char *p=curmempos;
	int i=0, valid=0;
	while(!valid)
	{
		i=0;
		while (p<img+scpsize)	// virtual EOF
		{
			if(*p==0x0A || *p==0x0D)
			{
				temp[i]=0;
				p++;
				break;
			}
			else
			{
				temp[i]=*p;
				p++;i++;
			}
		}
		curmempos=p;
		valid=1;
		if (temp[0]=='/' && temp[1]=='/') valid=0;
		if (temp[0]=='{') valid=0;
		if (temp[0]==0) valid=0;

		if (p>=img+scpsize && !valid)	// virtual EOF
			return false;
	}
	return true;
}

bool Script::NextLine()
{
	if (flags & SCP_PRELOADED)
	{
		if (!ReadMemLine())
			return false;
		strcpy((char*)script1,(char*)temp);
	}
	else
		read1();

	return true;
}

void Script::NextLineSplitted()
{
	if (flags & SCP_PRELOADED)
	{
		ReadMemLine();
		int i=0;
		script1[0]=0;
		script2[0]=0;
	
		while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=' && i<1024 )
		{
			i++;
		}
	
		strncpy((char*)script1,(char*) temp, i);
		script1[i]=0;
		if (script1[0]!='}' && temp[i]!=0)
			strcpy((char*)script2, (char*)(temp+i+1));
	}
	else
		read2();
}


// Look for that section in this previously parsed script file
char Script::isin(const char *section) {
    time_t current;
    
    get_modification_date(filename, &current);

    if (current > last_modification)
	{
        reload();
        last_modification = current;
    }

	std::map<std::string, ScriptEntry>::iterator iter_entry = entries.find(section);

	if (iter_entry == entries.end())
		return 0;
	else
	{
		fseek(scpfile, iter_entry->second.offset, SEEK_SET);
		return 1;
	}
}
