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
#include "wolfpack.h"
#include "items.h"
#include "exceptions.h"

// System Includes
#include <map>
#include <list>

// Singleton Class to manage Items.
//##ModelId=3C5D92E30124
class cItemsManager : public std::map<SERIAL, cItem*>
{
protected:
	// Data members
	//##ModelId=3C5D92E302D4
	std::list<cItem*> deletedItems;
	//##ModelId=3C5D92E302F2
	SERIAL lastUsedSerial;
protected:
	//##ModelId=3C5D92E3030F
	cItemsManager() {} // Unallow anyone to instantiate.
	//##ModelId=3C5D92E30323
	cItemsManager(cItemsManager& _it) {} // Unallow copy constructor
	//##ModelId=3C5D92E30337
	cItemsManager& operator=(cItemsManager& _it) { return *this; } // Unallow Assignment
public:
	//##ModelId=3C5D92E3034B
	void registerItem( cItem* ) throw(wp_exceptions::bad_ptr);
	//##ModelId=3C5D92E30369
	void unregisterItem( cItem* ) throw (wp_exceptions::bad_ptr);
	//##ModelId=3C5D92E3037D
	void deleteItem ( cItem * ) throw (wp_exceptions::bad_ptr);
	//##ModelId=3C5D92E30391
	void purge();
	//##ModelId=3C5D92E3039B
	SERIAL getUnusedSerial() const;

	//##ModelId=3C5D92E303A5
	static cItemsManager* getInstance()
	{
		static cItemsManager theItemsManager;
		return &theItemsManager; 
	}
};

//##ModelId=3C5D92E40017
class AllItemsIterator
{
protected:
	//##ModelId=3C5D92E4004A
	cItemsManager::iterator iterItems;
	
public:
	//##ModelId=3C5D92E4005D
	AllItemsIterator()							
	{ 
		iterItems = cItemsManager::getInstance()->begin(); 
	}

	//##ModelId=3C5D92E4005E
	virtual ~AllItemsIterator()					{ }
	//##ModelId=3C5D92E40068
	P_ITEM GetData(void)						{ return iterItems->second; }
	//##ModelId=3C5D92E4007B
	P_ITEM First()								{ return cItemsManager::getInstance()->begin()->second; }
	//##ModelId=3C5D92E40085
	P_ITEM Begin()								
	{
		iterItems = cItemsManager::getInstance()->begin();
		return GetData();
	}
	//##ModelId=3C5D92E4008F
	P_ITEM Next()
	{
		iterItems++;
		return iterItems->second;
	}
	//##ModelId=3C5D92E40099
	bool atEnd()									{ return (iterItems == cItemsManager::getInstance()->end()); }
	//##ModelId=3C5D92E400A3
	const AllItemsIterator operator++(int);
	//##ModelId=3C5D92E400B7
	const AllItemsIterator operator--(int);
	//##ModelId=3C5D92E400CB
	AllItemsIterator& operator++()								{ ++iterItems; return *this;				}
	//##ModelId=3C5D92E400D5
	AllItemsIterator& operator--()								{ --iterItems; return *this;				}
	
};

//##ModelId=3C5D92E400A3
inline const AllItemsIterator AllItemsIterator::operator++(int)
{
	AllItemsIterator returnValue(*this);	// fetch
	++iterItems;							// increment
	return returnValue;						// return what was fetched
}

//##ModelId=3C5D92E400B7
inline const AllItemsIterator AllItemsIterator::operator--(int)
{
	AllItemsIterator returnValue(*this);	// fetch
	--iterItems;							// increment
	return returnValue;						// return what was fetched
}

#endif // __ITEMSMGR_H__
