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

#ifndef __GUMPSMGR_H__
#define __GUMPSMGR_H__

#include "defines.h"
#include "typedefs.h"

#include "qstringlist.h"
#include "exceptions.h"
#include <map>
#include <list>

// Forward Declarations
class cGump;
class cUOSocket;

class cGumpsManager : public std::map<SERIAL, cGump*>
{
protected:
	// Data members
	std::list<cGump*> deletedGumps;
	SERIAL lastUsedSerial;

	cGumpsManager() {} // Unallow anyone to instantiate.
	cGumpsManager(cGumpsManager& _it) {} // Unallow copy constructor
	cGumpsManager& operator=(cGumpsManager& _it) { return *this; } // Unallow Assignment

	void registerGump( cGump* ) throw(wp_exceptions::wpbad_ptr);
	void unregisterGump( cGump* ) throw (wp_exceptions::wpbad_ptr);
public:
	~cGumpsManager();
	
	// assigns a serial to the gump and sends it to the socket...
	void attachGump( cUOSocket* socket, cGump* gump );
	
	// finds the gump object by serial and type, passes choice to the
	// gump's handleResponse method and finally deletes the gump object.
	void handleResponse( cUOSocket* socket, SERIAL serial, UINT32 type, UINT32 choice );

	void purge();
	SERIAL getUnusedSerial() const;

	static cGumpsManager* getInstance()
	{
		static cGumpsManager theGumpsManager;
		return &theGumpsManager; 
	}
};

#endif