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

#include "itemsmgr.h"
#include "basics.h"
#include "items.h"


#include <algorithm>

using namespace std;

cItemsManager::~cItemsManager()
{
	purge();
	const_iterator it = begin();
	const_iterator end(end());
	for (; it != end; ++it )
		delete it->second;
}

/*!
 * Registers an item into the ItemsManager Instance and enable lookups by serial.
 *
 * @param pc : Pointer to item
 *
 * @return void  : none
 */
void cItemsManager::registerItem(cItem* pi) throw(wp_exceptions::wpbad_ptr)
{
	if ( pi != NULL)
	{
		insert(make_pair(pi->serial()&0xBFFFFFFF, pi));
		lastUsedSerial = QMAX(lastUsedSerial, pi->serial());
	}
	else
	{
		throw wp_exceptions::wpbad_ptr("Invalid argument PI at cItemsManager::registerItem");
	}
}

/*!
 * Unregister from ItemsManager. Searches by serial will not find this item anymore.
 *
 * @param pc : Pointer to item. Item itself is unchanged.
 *
 * @return void  : none
 */
void cItemsManager::unregisterItem(cItem* pi) throw(wp_exceptions::wpbad_ptr)
{
	if ( pi != NULL)
		erase(pi->serial()&0xBFFFFFFF);
	else
		throw wp_exceptions::wpbad_ptr("Invalid argument PI at cItemsManager::unregisterItem");
}

/*!
 * Returns an unused, valid Serial number for Items
 *
 * @param none
 *
 * @return SERIAL  : Valid serial number
 */
SERIAL cItemsManager::getUnusedSerial() const
{
//	typedef maxKeyPred<SERIAL, cItem*> max_serialPred;
//	map<SERIAL, cItem*>::const_iterator temp = std::max_element(this->begin(), this->end(), max_serialPred());
	return QMAX(0x40000001, lastUsedSerial + 1);
}

/*!
 * Item is unregistered and queued for posterior delete. No more references to this
 * item should be made.
 *
 * @param pc : Pointer to item
 *
 * @return void  : none
 */
void cItemsManager::deleteItem(cItem* pi) throw(wp_exceptions::wpbad_ptr)
{
	if ( pi != NULL)
	{
		deletedItems.push_back(pi);
		unregisterItem(pi);
	}
	else
		throw wp_exceptions::wpbad_ptr("Invalid argument pi at cItemsManager::deleteItem");
}

/*!
  Free memory of items queued for delete.
 */
void cItemsManager::purge()
{
	list<cItem*>::const_iterator it;
	list<cItem*>::const_iterator end(deletedItems.end());
	for (it = deletedItems.begin(); it != end; ++it)
	{
		delete *it;
	}
	deletedItems.clear();
}

/////////////////////
// Name:	FindItemBySerial
// Purpose:	creates an item pointer from the given serial, returns NULL if not found
// History:	by Duke, 5.11.2000
//
P_ITEM FindItemBySerial(int serial)
{
	if (!isItemSerial(serial))
		return 0;
	cItemsManager::iterator iterItems = ItemsManager::instance()->find( serial&0xBFFFFFFF );
	if (iterItems == ItemsManager::instance()->end()) 
		return 0;
	else 
		return iterItems->second;
}
