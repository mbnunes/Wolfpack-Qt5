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
#include "wolfpack.h"
#include "chars.h"
#include "exceptions.h"

// System Includes
#include <map>
#include <list>

// Singleton Class to manage Items.
//##ModelId=3C5D931101CA
class cCharsManager : public std::map<SERIAL, cChar*>
{
protected:
	// Data Members
	//##ModelId=3C5D93110335
	std::list<cChar*> deletedChars;
	//##ModelId=3C5D93110366
	SERIAL	lastUsedSerial;
protected:
	//##ModelId=3C5D93110397
	cCharsManager() {} // Unallow anyone to instantiate.
	//##ModelId=3C5D931103AB
	cCharsManager(cCharsManager& _it) {} // Unallow copy constructor
	//##ModelId=3C5D931103C9
	cCharsManager& operator=(cCharsManager& _it) { return *this; } // Unallow Assignment
public:
	//##ModelId=3C5D931103DD
	void registerChar( cChar* ) throw(wp_exceptions::bad_ptr);
	//##ModelId=3C5D93120031
	void unregisterChar( cChar* ) throw (wp_exceptions::bad_ptr);
	//##ModelId=3C5D93120045
	SERIAL getUnusedSerial() const;
	//##ModelId=3C5D9312004F
	void deleteChar( cChar* ) throw(wp_exceptions::bad_ptr);
	//##ModelId=3C5D93120063
	void purge();

	//##ModelId=3C5D93120077
	static cCharsManager* getInstance()
	{
		static cCharsManager theCharsManager;
		return &theCharsManager; 
	}
};

//##ModelId=3C5D931200C7
class AllCharsIterator
{
protected:
	//##ModelId=3C5D931200FB
	cCharsManager::iterator iterChars;
	
public:
	//##ModelId=3C5D9312010E
	AllCharsIterator()							
	{ 
		iterChars = cCharsManager::getInstance()->begin(); 
	}

	//##ModelId=3C5D93120118
	virtual ~AllCharsIterator()					{ }
	//##ModelId=3C5D9312011A
	P_CHAR GetData(void)						{ return iterChars->second; }
	//##ModelId=3C5D9312012C
	P_CHAR First()								{ return cCharsManager::getInstance()->begin()->second; }
	//##ModelId=3C5D93120136
	P_CHAR Begin()								
	{
		iterChars = cCharsManager::getInstance()->begin();
		return GetData();
	}
	//##ModelId=3C5D931202DA
	P_CHAR Next()
	{
		iterChars++;
		return iterChars->second;
	}
	//##ModelId=3C5D931202E4
	bool atEnd()										{ return (iterChars == cCharsManager::getInstance()->end()); }
	//##ModelId=3C5D931202EE
	const AllCharsIterator operator++(int);
	//##ModelId=3C5D93120302
	const AllCharsIterator operator--(int);
	//##ModelId=3C5D93120316
	AllCharsIterator& operator++()						{ ++iterChars; return *this;	 }
	//##ModelId=3C5D93120320
	AllCharsIterator& operator--()						{ --iterChars; return *this;	 }
};

//##ModelId=3C5D931202EE
inline const AllCharsIterator AllCharsIterator::operator++(int)
{
	AllCharsIterator returnValue(*this);	// fetch
	++iterChars;							// increment
	return returnValue;						// return what was fetched
}

//##ModelId=3C5D93120302
inline const AllCharsIterator AllCharsIterator::operator--(int)
{
	AllCharsIterator returnValue(*this);
	--iterChars;
	return returnValue;
}

#endif // __CHARSMGR_H__
