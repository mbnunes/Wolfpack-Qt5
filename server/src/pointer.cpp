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

// - Tauriel experimental pointer array *crap*
//
// -- pointer.cpp		Item specific routines (add, delete change) in preperation for
//									going to a pointer based system (compiled under level 3 no errors!
//
#include "wolfpack.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "pointer.cpp"


/* Deprecated stuff
void setserial(int nChild, int nParent, int nType)
{ // Sets the serial #'s and adds to pointer arrays
  // types: 1-item container, 2-item spawn, 3-Item's owner 4-container is PC/NPC
  //        5-NPC's owner, 6-NPC spawned
	if (nChild == -1 || nParent == -1) return;
	switch(nType)
	{
	case 1:
//	    items[nChild].setContSerialOnly(items[nParent].serial);
//		setptr(&contsp[items[nChild].contserial%HASHMAX], nChild);
		clConsole.send("Warning: Item contserial being set by setserial!, this is WRONG - will do nothing.\n");
		break;
	case 2:				// Set the Item's Spawner
	    items[nChild].spawnserial=items[nParent].serial;
		setptr(&spawnsp[items[nChild].spawnserial%HASHMAX], nChild);
		break;
	case 3:				// Set the Item's Owner
//	    items[nChild].setOwnSerialOnly(chars[nParent].serial);
//		setptr(&ownsp[items[nChild].ownserial%HASHMAX], nChild);
		clConsole.send("Warning: Item ownserial being set by setserial!, this is WRONG - will do nothing.\n");
		break;
	case 4:
//	    items[nChild].setContSerialOnly(chars[nParent].serial);
//		setptr(&contsp[items[nChild].contserial%HASHMAX], nChild);
		clConsole.send("Warning: Char contserial being set by setserial!, this is WRONG - will do nothing.\n");
		break;
	case 5:				// Set the Character's Owner
		chars[nChild].setOwnSerialOnly(chars[nParent].serial);
		setptr(&cownsp[chars[nChild].ownserial%HASHMAX], nChild);
		//taken from 6904t2(5/10/99) - AntiChrist
		if( nChild != nParent )
			chars[nChild].tamed = true;
		else
			chars[nChild].tamed = false;
		break;
	case 6:				// Set the character's spawner
	    chars[nChild].spawnserial=items[nParent].serial;
		setptr(&cspawnsp[chars[nChild].spawnserial%HASHMAX], nChild);
		break;
	case 7:				// Set the Item in a multi
		clConsole.send("Warning: Item multi being set by setserial!, this is WRONG - will do nothing.\n");
//		items[nChild].multis=items[nParent].serial;
//		setptr(&imultisp[items[nChild].multis%HASHMAX], nChild);
		break;
	case 8:				//Set the CHARACTER in a multi
//		chars[nChild].multis=items[nParent].serial;
//		setptr(&cmultisp[chars[nChild].multis%HASHMAX], nChild);
		clConsole.send("Warning: Char multi being set by setserial!, this is WRONG - will do nothing.\n");
		break;
	default:
		clConsole.send("Warning: No handler for nType in setserial()");
		break;
	}
}
*/

///////////////////////////
// Name:	ContainerCountItems
// history:	by Duke, 26.03.2001
// Purpose:	searches the container given by serial AND the subcontainers
//			for items with the given id and (if given) color.
//			Returns the total number of items found
//
int ContainerCountItems(const int serial, short id, short color)
{
	unsigned int ci=0; 
	int total=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->free)			// just to be sure ;-)
			continue;
		if (pi->type==1)		// a subcontainer ?
		{
			total += ContainerCountItems(pi->serial, id, color);
			continue;
		}
		if (pi->id()==id &&
			(color==-1 || pi->color()==color))
			total += pi->amount;
	}
	return total;
}

/////////////////////
// Name:	FindItemBySerial
// Purpose:	creates an item pointer from the given serial, returns NULL if not found
// History:	by Duke, 5.11.2000
//
P_ITEM FindItemBySerial(int serial)
{
	if (!isItemSerial(serial))
		return NULL;
	cItemsManager::iterator iterItems = cItemsManager::getItemsManager().find( serial );
	if (iterItems == cItemsManager::getItemsManager().end()) 
		return NULL;
	else 
		return iterItems->second;
}

P_ITEM FindItemBySerPtr(unsigned char *p)
{
	int serial=LongFromCharPtr(p);
	if(serial == INVALID_SERIAL) return NULL;
	return FindItemBySerial(serial);
}

/////////////////////
// Name:	FindCharBySerial
// Purpose:	creates an chars pointer from the given serial, returns NULL if not found
// History:	by Duke, 11.11.2000
//			added BySerPtr Duke, 24.2.2001
//
P_CHAR FindCharBySerial(int serial)
{
	if (!isCharSerial(serial))
		return NULL;
	cCharsManager::iterator iterChars = cCharsManager::getCharsManager().find( serial );
	if ( iterChars == cCharsManager::getCharsManager().end())
		return NULL;
	else 
		return iterChars->second;
}

P_CHAR FindCharBySerPtr(unsigned char *p)
{
	int serial = LongFromCharPtr(p);
	if(serial == INVALID_SERIAL) return NULL;
	return FindCharBySerial(serial);
}

