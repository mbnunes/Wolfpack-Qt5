//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#if !defined(__ITEMSMGR_H__)
#define __ITEMSMGR_H__
// Platform specifics
#include "platform.h"

// Wolfpack specific
#include "exceptions.h"
#include "defines.h"
#include "typedefs.h"
#include "singleton.h"

// Forward declarations
class cItem;

// System Includes
#include <list>

// Important compile switch
#if defined(WP_DONT_USE_HASH_MAP)
#include <map>
typedef std::map< SERIAL, P_ITEM > cItemsManagerSuperClass;
#else
#include <hash_map>
typedef std::hash_map< SERIAL, P_ITEM > cItemsManagerSuperClass;
#endif

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
class cItemsManager: public cItemsManagerSuperClass
{
protected:
	// Data members
	std::list<cItem*> deletedItems;
	SERIAL lastUsedSerial;
	UINT32 lastTooltip;

public:
	cItemsManager() : lastUsedSerial(0), lastTooltip(0) {}
	~cItemsManager();
	void registerItem( cItem* ) throw(wp_exceptions::wpbad_ptr);
	void unregisterItem( cItem* ) throw (wp_exceptions::wpbad_ptr);
	void deleteItem ( cItem * ) throw (wp_exceptions::wpbad_ptr);
	void purge();
	UINT32 getUnusedTooltip() { return ++lastTooltip; }
	
	SERIAL getUnusedSerial() const;
};

typedef SingletonHolder<cItemsManager> ItemsManager;

class AllItemsIterator
{
protected:
	cItemsManager::iterator iterItems;
	
public:
	AllItemsIterator()							
	{ 
		iterItems = ItemsManager::instance()->begin(); 
	}

	virtual ~AllItemsIterator()					{ }
	P_ITEM GetData(void)						{ return iterItems->second; }
	P_ITEM First()								{ return ItemsManager::instance()->begin()->second; }
	P_ITEM Begin()								
	{
		iterItems = ItemsManager::instance()->begin();
		if ( ItemsManager::instance()->end() == iterItems )
			return 0;
		return GetData();
	}
	P_ITEM Next()
	{
		++iterItems;
		return iterItems->second;
	}

	bool atBegin()									{ return (iterItems == ItemsManager::instance()->begin()); }
	bool atEnd()									{ return (iterItems == ItemsManager::instance()->end()); }
	const AllItemsIterator operator++(int);
	AllItemsIterator& operator++()					{ ++iterItems; return *this; }
};

inline const AllItemsIterator AllItemsIterator::operator++(int)
{
	AllItemsIterator returnValue(*this);	// fetch
	++iterItems;							// increment
	return returnValue;						// return what was fetched
}

inline bool isItemSerial(long ser) 
{
	return (ser != INVALID_SERIAL && ser >= 0x40000000);
}


#endif // __ITEMSMGR_H__
