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

#if !defined(__ITEMSMGR_H__)
#define __ITEMSMGR_H__
// Platform specifics
#include "platform.h"

// Wolfpack specific
#include "exceptions.h"
#include "defines.h"
#include "typedefs.h"

// Forward declarations
class cItem;

// System Includes
#include <map>
#include <list>
#include "my_hash_map.h"

using namespace std;

struct IntHasher {

  // Since we are hashing based on integers we can just use the
  // GENERIC_HASH macro.  GENERIC_HASH is a macro which takes
  // an integer and scrambles it up.
  int operator()(const int & key)const {return GENERIC_HASH(key);}

  // SecondHashValue also takes an integer and scrambles it up.
  // However, for this to be useful we need SecondHashValue to
  // scramble the key up in a different way.  Therefore we multiply
  // key by 3 before calling GENERIC_HASH.  Also note that we have
  // set things up so SecondHashValue ALWAYS returns an odd number.
  // This is REQUIRED.  SecondHashValue must ALWAYS return an odd
  // number for the hash table to be properly searched.
  int SecondHashValue(const int & key)const {
    return 2*GENERIC_HASH(3*key) + 1;
  }

};

struct IntEqualCmp {
  bool operator()(const int x, const int y) const {return x == y;}
};

/*!
CLASS
    cItemsManager 

    This class storage management to Items ...


USAGE
    Singleton responsable for memory management and serial lookups
	for characters. To access the single instance, call the static member
	getInstance().
	\see cItem

*/
class cItemsManager: public std::map< SERIAL, P_ITEM >
{
protected:
	// Data members
	std::list<cItem*> deletedItems;
	SERIAL lastUsedSerial;
	my_hash_map<SERIAL, cItem*, IntHasher, IntEqualCmp> *hashMap;
protected:
	// Initialize our hashmap
	cItemsManager()
	{
		IntHasher intHasher;
		IntEqualCmp equalCmp;
		hashMap = new my_hash_map<SERIAL, cItem*, IntHasher, IntEqualCmp>( 1000, intHasher, equalCmp ); // Pre allocate 1000 items
	}
	cItemsManager(cItemsManager& _it) {} // Unallow copy constructor
	cItemsManager& operator=(cItemsManager& _it) { return *this; } // Unallow Assignment
public:
	~cItemsManager();
	void registerItem( cItem* ) throw(wp_exceptions::wpbad_ptr);
	void unregisterItem( cItem* ) throw (wp_exceptions::wpbad_ptr);
	void deleteItem ( cItem * ) throw (wp_exceptions::wpbad_ptr);
	void purge();
	SERIAL getUnusedSerial() const;

	static cItemsManager* getInstance()
	{
		static cItemsManager theItemsManager;
		return &theItemsManager; 
	}
};

class AllItemsIterator
{
protected:
	cItemsManager::iterator iterItems;
	
public:
	AllItemsIterator()							
	{ 
		iterItems = cItemsManager::getInstance()->begin(); 
	}

	virtual ~AllItemsIterator()					{ }
	P_ITEM GetData(void)						{ return iterItems->second; }
	P_ITEM First()								{ return cItemsManager::getInstance()->begin()->second; }
	P_ITEM Begin()								
	{
		iterItems = cItemsManager::getInstance()->begin();
		return GetData();
	}
	P_ITEM Next()
	{
		iterItems++;
		return iterItems->second;
	}

	bool atBegin()									{ return (iterItems == cItemsManager::getInstance()->begin()); }
	bool atEnd()									{ return (iterItems == cItemsManager::getInstance()->end()); }
	const AllItemsIterator operator++(int);
	const AllItemsIterator operator--(int);
	AllItemsIterator& operator++()								{ ++iterItems; return *this;				}
	AllItemsIterator& operator--()								{ --iterItems; return *this;				}
	
};

inline const AllItemsIterator AllItemsIterator::operator++(int)
{
	AllItemsIterator returnValue(*this);	// fetch
	++iterItems;							// increment
	return returnValue;						// return what was fetched
}

inline const AllItemsIterator AllItemsIterator::operator--(int)
{
	AllItemsIterator returnValue(*this);	// fetch
	--iterItems;							// increment
	return returnValue;						// return what was fetched
}

inline bool isItemSerial(long ser) 
{
	return (ser != INVALID_SERIAL && ser >= 0 && ser >= 0x40000000);
}


#endif // __ITEMSMGR_H__
