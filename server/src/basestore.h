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
#if !defined(__BASESTORE_H__)
#define __BASESTORE_H__

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
#define BBINARY_VERSION 0x2
#define BSECTION_START 0xFFFFFFFE 
#define BSECTION_END 0xFFFFFFFD
#define BTYPE_SI32 0 
#define BTYPE_UI32 1
#define BTYPE_SI16 2
#define BTYPE_UI16 3
#define BTYPE_SI08 4
#define BTYPE_UI08 5 
#define BTYPE_STRING 6 
#define BTYPE_BOOL 7 
#define BTYPE_RF32 8
#define BTYPE_RF64 9
//Class definitions
/*!
CLASS
    

    This class provides the interface for script loading


USAGE
    This is the base class for script loading. All script 
	loading classes should inherate from this one.

*/
class Basestore_cl
{
public:
        /// Constructor
	Basestore_cl()   ;
	/// Desctructor
	~Basestore_cl() ;

	/// General processing methods, to invoke the process
	

	/// Binary processing

	/// Process a file
    virtual bool processFile(string sFile, SI32 siType=0,bool bVerbose=false);
	/// Save data
	virtual bool saveFile(string sFile,SI32 siType=0, bool bVerbose=false) ;

	/// Support methods
	
	// clear out anythign that has been stored
    virtual void clear(){}
	
	// Get the section count (if a list was used, it returns just the last file 
	// count processed!
	
	UI32 getSectionCount() ;


protected:



	//Virtual methods for binary processing
	// Reading of files
	virtual void startSection(UI32 uiKey, SI32 siValue){}
	virtual void addEntry(UI32 uiKey,UI32 uiValue) {}
	virtual void addEntry(UI32 uiKey,string& sValue){}
	virtual void addEntry(UI32 uiKey,SI32 siValue){}
	virtual void addEntry(UI32 uiKey,UI16 uiValue){}
	virtual void addEntry(UI32 uiKey,SI16 siValue){}
	virtual void addEntry(UI32 uiKey,UI08 uiValue){}
	virtual void addEntry(UI32 uiKey,SI08 siValue){}
	virtual void addEntry(UI32 uiKey,RF64 rfValue){}
	virtual void addEntry(UI32 uiKey,bool bValue){}
	virtual void addEntry(UI32 uiKey,RF32 rfValue){}
	virtual void endSection() {}
	virtual void startFile(string sFile,SI32 siType) {}
    virtual void endFile() {}
	// Writing of files 
	virtual void saveBinaryData(string sFile,SI32 siType) {}
	bool	writeEntry(fstream& fFile,UI32 uiKey,UI32 uiValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,SI32 siValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,RF32 rfValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,string& sValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,bool bValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,UI16 uiValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,SI16 siValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,UI08 uiValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,SI08 siValue) ;
	bool	writeEntry(fstream& fFile,UI32 uiKey,RF64 rfValue) ;
	bool	writeStartSection(fstream& fFile,UI32 uiType, SI32 siKey) ;
	bool	writeEndSection(fstream& fFile) ;
	bool	abortSection(fstream& fFile) ;
	bool	closeFile(fstream& fFile) ;
	bool	openFile(string sFile) ;

private:

//	Data we need 

protected:
	bool bVerbose ;
	fstream fSavefile ;

private:
    bool bSectionPending ;

    bool bInSection;
	UI32	uiSectionOffset;

	UI32	uiSectionCount;
	





};
//==========================================================================================

#endif



