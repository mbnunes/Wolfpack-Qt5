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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
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

// Singleton Class to manage Items.
class cItemsManager : public std::map<SERIAL, cItem*>
{
private:
	cItemsManager() {} // Unallow anyone to instantiate.
public:
	void registerItem( cItem* ) throw(wp_exceptions::bad_ptr);
	void unregisterItem( cItem* ) throw (wp_exceptions::bad_ptr);
	SERIAL getUnusedSerial() const;

	static cItemsManager& getItemsManager()
	{
		static cItemsManager theItemsManager;
		return theItemsManager; 
	}
};

class AllItemsIterator
{
protected:
	cItemsManager::iterator iterItems;
	
public:
	AllItemsIterator()							
	{ 
		cItemsManager itemsManager = cItemsManager::getItemsManager();
		iterItems = itemsManager.begin(); 
	}

	virtual ~AllItemsIterator()					{ }
	P_ITEM GetData(void)						{ return iterItems->second; }
	P_ITEM First()								{ return cItemsManager::getItemsManager().begin()->second; }
	P_ITEM Begin()								
	{
		iterItems = cItemsManager::getItemsManager().begin();
		return GetData();
	}
	P_ITEM Next()
	{
		iterItems++;
	}
	bool atEnd()									{ return (iterItems == cItemsManager::getItemsManager().end()); }
	AllItemsIterator& operator++(int)		    { iterItems++; return *this; }
	
};

#endif // __ITEMSMGR_H__