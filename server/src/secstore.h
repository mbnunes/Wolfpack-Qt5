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
#if !defined(__SECSTORE_H__)
#define __SECSTORE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <string>
#include <map>

using namespace std ;


// Third Party includes



//Forward class declaration

class Secstore_cl ;

//Wolfpack Includes


#include "wpconsole.h"
#include "basestore.h"
#include "section.h"

extern WPConsole_cl clConsole ;

//Class definitions
class Secstore_cl : public Basestore_cl
{
public:
	/// Constructor
	Secstore_cl()   ;
	/// Cosntructor of itself
	//Secstore_cl(const Secstore_cl& clData) ;
	/// Desctructor
	~Secstore_cl() ;

        // Retireve a section

	Section_cl get(string sLine) ;
	Section_cl get(char* szLine);
        // Operator functions
	//Secstore_cl&  operator=(const Secstore_cl& clData) ;

protected:
	/// Clear out any that we have

	void clear() ;

	void startSection(string& sLine) ;

	void addEntry(string& sLine) ;
	void endSection(string& sLine);

	void startFile(string sFile,SI32 siType) {}
	void endFile() {}
	void saveData(string sFile,SI32 siType){}

	void addBinaryEntry(UI32 uiKey,UI32 uiValue){}
	void addBinaryEntry(UI32 uiKey,UI16 uiValue){}
	void addBinaryEntry(UI32 uiKey,UI08 uiValue){}
	void addBinaryEntry(UI32 uiKey,string& sValue){}
	void addBinaryEntry(UI32 uiKey,SI32 siValue){}
	void addBinaryEntry(UI32 uiKey,SI16 siValue){}
	void addBinaryEntry(UI32 uiKey,SI08 siValue){}
	void addBinaryEntry(UI32 uiKey,RF32 rfValue){}
	void addBinaryEntry(UI32 uiKey,RF64 rfValue){}
	void addBinaryEntry(UI32 uiKey,bool bValue){}
	void endBinarySection() {}
	void startBinaryFile(string sFile,SI32 siType){}
	void endBinaryFile() {}
	void saveBinaryData(string sFile,SI32 siType) {}
	void startBinarySection(UI32 uiKey,SI32 siValue){}


private:

	map<string,Section_cl>				mapData ;
	map<string,Section_cl>::iterator	iterData ;
	Section_cl* ptrSection ;
	string sSectionName ;





};
//==========================================================================================

#endif



