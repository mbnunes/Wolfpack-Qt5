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

#include "charsmgr.h"
#include "chars.h"
#include "basics.h"

#include <algorithm>

using namespace std;
/*
template<class _T, class _P>
class maxKeyPred : public binary_function<pair<_T, _P>, pair<_T, _P>, bool>
{
public:
	bool operator()(pair<_T, _P> a, pair<_T, _P> b)
	{
		return a.first < b.first;
	}
};


struct max_serialPred : binary_function<pair<SERIAL, cChar*>, pair<SERIAL, cChar*>, bool>
{
	bool operator()(pair<SERIAL,cChar*> a, pair<SERIAL,cChar*> b)
	{
		return a.first < b.first;
	}
};
*/

cCharsManager::~cCharsManager()
{
	iterator it = begin();
	for (; it != end(); ++it )
		delete it->second;
}

/*!
 * Registers a character into the CharsManager Instance and enable lookups by serial.
 *
 * @param pc : Pointer to character
 *
 * @return void  : none
 */
void cCharsManager::registerChar(cChar* pc) throw(wp_exceptions::wpbad_ptr)
{
	if ( pc != NULL)
	{
		insert(make_pair(pc->serial(), pc));
		lastUsedSerial = QMAX(lastUsedSerial, pc->serial());
	}
	else
	{
		throw wp_exceptions::wpbad_ptr("Invalid argument pc at cCharsManager::registerChar");
	}
}


/*!
 * Unregister from CharsManager. Searches by serial will not find this char anymore.
 *
 * @param pc : Pointer to character. Character itself is unchanged.
 *
 * @return void  : none
 */
void cCharsManager::unregisterChar(cChar* pc) throw(wp_exceptions::wpbad_ptr)
{
	if ( pc != NULL)
		erase(pc->serial());
	else
		throw wp_exceptions::wpbad_ptr("Invalid argument pc at cCharsManager::unregisterChar");
}


/*!
 * Returns an unused, valid Serial number for Characters
 *
 * @param none
 *
 * @return SERIAL  : Valid serial number
 */
SERIAL cCharsManager::getUnusedSerial() const
{
//	typedef maxKeyPred<SERIAL, cChar*> max_serialPred;
//	map<SERIAL, cChar*>::const_iterator temp = std::max_element(this->begin(), this->end(), max_serialPred());
	return QMAX(1, lastUsedSerial+1); // no serial 0
}


/*!
 * Character is unregistered and queued for posterior delete. No more references to this
 * character should be made.
 *
 * @param pc : Pointer to character
 *
 * @return void  : none
 */

void cCharsManager::deleteChar(cChar* pc) throw(wp_exceptions::wpbad_ptr)
{
	if ( pc != NULL)
	{
		deletedChars.push_back(pc);
		unregisterChar(pc);
	}
	else
		throw wp_exceptions::wpbad_ptr("Invalid argument pc at cCharsManager::deleteChar");
}


/*!
 * Free memory of chars queued for delete.
 *
 * @param none
 *
 * @return void  : none
 */

void cCharsManager::purge()
{
	list<cChar*>::iterator it;
	for (it = deletedChars.begin(); it != deletedChars.end(); it++)
	{
		delete *it;
	}
	deletedChars.clear();
}

/////////////////////
// Name:	FindCharBySerial
// Purpose:	creates an chars pointer from the given serial, returns NULL if not found
// History:	by Duke, 11.11.2000
//			added BySerPtr Duke, 24.2.2001
// NOTE: Eventually we should check if the char has been freed
P_CHAR FindCharBySerial(int serial)
{
	if (!isCharSerial(serial))
		return NULL;
	cCharsManager::iterator iterChars = CharsManager::instance()->find( serial );
	if ( iterChars == CharsManager::instance()->end())
		return NULL;
	else 
		return iterChars->second;
}

