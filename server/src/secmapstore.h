//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//      Copyright 2001 by holders identified in authors.txt
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
#if !defined(__SECMAPSTORE_H__)
#define __SECMAPSTORE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <string>
#include <map>

using namespace std ;


// Third Party includes



//Forward class declaration

class Secmapstore_cl;

//Wolfpack Includes


#include "wpconsole.h"
#include "basestore.h"
#include "sectionmap.h"
#include "mstring.h"

extern WPConsole_cl clConsole ;

// we typedef because later we do a map of a map, gcc doesnt like the syntax sometimes

typedef  map<string,Sectionmap_cl>   wpMapSection ;

//Class definitions
class Secmapstore_cl : public Basestore_cl
{
public:
	/// Constructor
    Secmapstore_cl()   ;
    // Cosntructor of itself
    //Secmapstore_cl(const Secmapstore_cl& clData) ;
    /// Desctructor
    ~Secmapstore_cl() ;

        /// Clear out any that we have

	void clear(string sSection) ;
	void clear(char* szSection) ;

	void clear(string sSection,string sType) ;
	void clear(char* szSection,char* szType) ;
	void clear() ;




    // Retrieve a section
    Sectionmap_cl get(char* szSection, char* szType) ;

    Sectionmap_cl get(string sSection, string sType) ;

    // Operator functions
    //Secmapstore_cl&  operator=(const Secmapstore_cl& clData) ;
protected:

	void startSection(string& sLine) ;

    void addEntry(string& sLine) ;
    void endSection(string& sLine);

    void startFile(string sFile,SI32 siType) {}
    void endFile() {}
	void saveData(string sLine,SI32 siType) {}

	void addBinaryEntry(UI32 uiKey,UI32 uiValue){}
	void addBinaryEntry(UI32 uiKey,UI16 uiValue){}
	void addBinaryEntry(UI32 uiKey,UI08 uiValue){}
	void addBinaryEntry(UI32 uiKey,string& sValue){}
	void addBinaryEntry(UI32 uiKey,SI32 siValue){}
	void addBinaryEntry(UI32 uiKey,SI16 siValue){}
	void addBinaryEntry(UI32 uiKey,SI08 siValue){}
	void addBinaryEntry(UI32 uiKey,bool bValue){}
	void addBinaryEntry(UI32 uiKey,RF32 rfValue){}
	void addBinaryEntry(UI32 uiKey,RF64 rfValue){}
	void endBinarySection() {}
	void startBinaryFile(string sFile,SI32 siType){}
	void endBinaryFile() {}
	void saveBinaryData(string sFile,SI32 siType) {}
	void startBinarySection(UI32 uiKey,SI32 siValue){}


private:

	map<string, wpMapSection> mapSection ;

	map<string, wpMapSection>::iterator iterSection ;

//        wpMapSection     mapData ;

    wpMapSection::iterator   iterData ;

    Sectionmap_cl* ptrSectionmap ;

    string sSectionName ;
	string sSectionType ;


};
//==========================================================================================

#endif



