//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#if !defined (__CONTEXTMENU_H__)
#define __CONTEXTMENU_H__

#include "definable.h"
#include "globals.h"
#include "singleton.h"

// Qt Includes
#include <qvaluevector.h>
#include <qmap.h>
#include <qstringlist.h>

// Forward Definitions
class cUObject;
class cPythonScript;
class cElement;

class cContextMenuEntry
{
	ushort cliloc_;
	ushort flags_;
	ushort color_;
	ushort scriptTag_;
	bool checkvisible_;
	bool checkenabled_;

public:
	
	
	cContextMenuEntry( ushort cliloc, ushort scriptTag, ushort color = 0, bool checkvisible = false, bool checkenabled = false ) : 
	cliloc_( cliloc ), scriptTag_(scriptTag), flags_ ( 0 ), color_(0), checkvisible_(checkvisible_), checkenabled_(checkenabled)
	{
		flags_ |= ( color_ & 0xFFFF ) ? 32 : 0;
	}
	
	bool isEnabled() const	{ return !(flags_ & 0x0001); }
	bool setEnabled( bool enable )
	{
		flags_ = enable ? flags_ & ~0x0001 : flags_ | 0x0001;
	}

	ushort color() const {	return color_; }
	ushort flags() const {	return flags_; }
	ushort scriptTag() const { return scriptTag_; }
	ushort cliloc() const { return cliloc_;	}
	bool   checkVisible() const { return checkvisible_;	}
	bool   checkEnabled() const { return checkenabled_;	}
};

class cContextMenu : public cDefinable
{
public:
	typedef QValueVector< cContextMenuEntry* > Entries;
	typedef Entries::const_iterator const_iterator;
	typedef Entries::iterator iterator;

	const_iterator	begin() const	{ return entries_.begin(); }
	const_iterator  end()   const	{ return entries_.end();   }

	uint			count() const   { return entries_.count(); }
	void			processNode( const cElement *Tag );
	void			onContextEntry( cPlayer* from, cUObject* target, ushort entry );
	bool			onCheckVisible( cPlayer* from, cUObject* target, ushort entry );
	bool			onCheckEnabled( cPlayer* from, cUObject* target, ushort entry );
	void			recreateEvents();
	void			disposeEntries();

private:
	Entries	entries_;
	QPtrList<cPythonScript> scriptChain_;
	QString scripts_;
};

class cAllContextMenus
{
public:

	bool			menuExists( const QString& bindmenu ) const;
	void			load();
	void			unload();
	void			reload();

	cContextMenu*	getMenu( const QString& ) const;
private:
	typedef QMap< QString, cContextMenu* > Menus;
	typedef Menus::const_iterator const_iterator;
	Menus menus_;
};

typedef SingletonHolder<cAllContextMenus> ContextMenus;

#endif // __CONTEXTMENU_H__

