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

#if !defined(__CHARSMGR_H__)
#define __CHARSMGR_H__
// Platform specifics
#include "platform.h"

// Wolfpack specific
//#include "wolfpack.h"
//#include "chars.h"
#include "exceptions.h"
#include "typedefs.h"

// System Includes
#include <map>
#include <list>

// Forward declarations
class cChar;


/*!
CLASS
    cCharsManager 

    This class storage management to Characters ...


USAGE
    Singleton responsable for memory management and serial lookups
	for characters. To access the single instance, call the static member
	getInstance().
*/
class cCharsManager : public std::map<SERIAL, cChar*>
{
protected:
	// Data Members
	std::list<cChar*> deletedChars;
	SERIAL	lastUsedSerial;
protected:
	cCharsManager() {} // Unallow anyone to instantiate.
	cCharsManager(cCharsManager& _it) {} // Unallow copy constructor
	cCharsManager& operator=(cCharsManager& _it) { return *this; } // Unallow Assignment
public:
	~cCharsManager();
	void registerChar( cChar* ) throw(wp_exceptions::wpbad_ptr);
	void unregisterChar( cChar* ) throw (wp_exceptions::wpbad_ptr);
	SERIAL getUnusedSerial() const;
	void deleteChar( cChar* ) throw(wp_exceptions::wpbad_ptr);
	void purge();

	static cCharsManager* getInstance()
	{
		static cCharsManager theCharsManager;
		return &theCharsManager; 
	}
};

/*!
CLASS
    

    This class provides an iterator service for the CharsManager \see cCharsManager

USAGE
	
	Iterators are classes that act like pointers to provide access to
	cCharsManager singleton's data.
*/
class AllCharsIterator
{
protected:
	cCharsManager::iterator iterChars;
	
public:
	AllCharsIterator()							
	{ 
		iterChars = cCharsManager::getInstance()->begin(); 
	}

	virtual ~AllCharsIterator()					{ }
	P_CHAR GetData(void)						{ return iterChars->second; }
	P_CHAR First()								{ return cCharsManager::getInstance()->begin()->second; }
	P_CHAR Begin()								
	{
		iterChars = cCharsManager::getInstance()->begin();
		return GetData();
	}
	P_CHAR Next()
	{
		iterChars++;
		return iterChars->second;
	}
	bool atEnd()										{ return (iterChars == cCharsManager::getInstance()->end()); }
	const AllCharsIterator operator++(int);
	const AllCharsIterator operator--(int);
	AllCharsIterator& operator++()						{ ++iterChars; return *this;	 }
	AllCharsIterator& operator--()						{ --iterChars; return *this;	 }
};

inline const AllCharsIterator AllCharsIterator::operator++(int)
{
	AllCharsIterator returnValue(*this);	// fetch
	++iterChars;							// increment
	return returnValue;						// return what was fetched
}

inline const AllCharsIterator AllCharsIterator::operator--(int)
{
	AllCharsIterator returnValue(*this);
	--iterChars;
	return returnValue;
}

#endif // __CHARSMGR_H__
