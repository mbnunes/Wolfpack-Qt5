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

#if !defined(AFX_WPSCRIPTMANAGER_H__267F80C6_32E0_43C9_992F_9FAD5DECDC2C__INCLUDED_)
#define AFX_WPSCRIPTMANAGER_H__267F80C6_32E0_43C9_992F_9FAD5DECDC2C__INCLUDED_

#include "wpdefaultscript.h"
#include "wppythonscript.h"

#include <map>
#include <string>

//#include "wolfpack.h"

typedef std::map< std::string, WPDefaultScript* >::iterator ScriptIterator;

using namespace std;

enum WPScriptTypes
{
	WPST_DEFAULT	= 0,
	WPST_PYTHON		= 1,
	WPST_TRIGGER	= 2,
};

class WPScriptManager  
{
private:
	std::map< QString, WPDefaultScript* > Scripts;

public:
	WPScriptManager();
	virtual ~WPScriptManager();

	WPDefaultScript* FindScript( QString Name );
	void AddScript( QString Name, WPDefaultScript *Script );
	void RemoveScript( QString Name );

	void Load( void );
};

#endif // !defined(AFX_WPSCRIPTMANAGER_H__267F80C6_32E0_43C9_992F_9FAD5DECDC2C__INCLUDED_)
