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
#if !defined(__TEXTSTORE_H__)
#define __TEXTSTORE_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <fstream>
#include <string>


using namespace std ;


// Third Party includes



//Forward class declaration

class Basestore_cl ;

//Wolfpack Includes

#include "whitespace.h"
#include "wpconsole.h"
#include "mstring.h"


extern WPConsole_cl clConsole;


// Defines we use

//Class definitions
/*!
CLASS
    

    This class provides the interface for script loading


USAGE
    This is the base class for script loading. All script 
	loading classes should inherate from this one.

*/
class Textstore_cl
{
public:
        /// Constructor
    Textstore_cl()   ;
        /// Desctructor
	~Textstore_cl() ;

	/// General processing methods, to invoke the process
	
	/// Text processing
        /// Process a list
    bool processList(string sFile, SI32 siType=0,bool bVerbose=false);
        /// Process a file (we allow overwrite but dont expect it to)
	/// siType is not used, but is passed along to the startFile method 
	/// that can be used by that method
    virtual bool processFile(string sFile, SI32 siType=0,bool bVerbose=false);
	/// Save data (we do nothing with this)
	virtual bool saveFile(string sFile, SI32 siType=0,bool bVerbose=false) ;

	

	/// Support methods
	
	// clear out anythign that has been stored
	virtual void clear() {}
	
	// Get the section count (if a list was used, it returns just the last file 
	// count processed!
	
	UI32 getSectionCount() ;



protected:

	//Virtual methods for text processing

	// Reading of files
	virtual void startSection(string& sLine){}
	virtual void addEntry(string& sLine){}
	virtual void endSection(string& sLine) {}
	virtual void startFile(string sFile,SI32 siType) {}
	virtual void endFile() {}

	// Writing of files
	virtual void saveData(string sFile, SI32 siType) {}
	void writeStartSection(string& sLine) ;
	void writeEndSection(string& sLine) ;
	void writeComment(string& sLine) ;
	void writeEntry(string& sKey, string& sLine) ;
	bool openFile(string sFile) ;
	bool closeFile() ;


private:
    string comment(string& sData) ;
    bool openSection(string& sLine) ;
    bool closeSection(string& sLine) ;
//	Data we need 

protected:
	bool bVerbose ;
	fstream fSavefile ;

private:
    bool bSectionPending ;

    bool bInSection;
	UI32	uiSectionOffset;

	UI32	uiSectionCount;
	
	string  sComment;
	string  sSectionKey;
	string  sSectionClose;
	string  sSectionStart;




};
//==========================================================================================

#endif



