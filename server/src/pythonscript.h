//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#ifndef __PYTHONSCRIPT_H__
#define __PYTHONSCRIPT_H__

// Wolfpack Includes
#include "python/engine.h"
#include "python/utilities.h"
#include "typedefs.h"

// Library Includes
#include <qstring.h>
#include <qregexp.h>
#include <qvaluevector.h>

// Script Based Events
enum ePythonEvent
{
	EVENT_USE = 0,
	EVENT_SINGLECLICK,
	EVENT_COLLIDE,
	EVENT_WALK,
	EVENT_CREATE,
	EVENT_TALK,
	EVENT_WARMODETOGGLE,
	EVENT_LOGIN,
	EVENT_LOGOUT,
	EVENT_HELP,
	EVENT_CHAT,
	EVENT_SKILLUSE,
	EVENT_SKILLGAIN,
	EVENT_STATGAIN,
	EVENT_SHOWPAPERDOLL,
	EVENT_SHOWSKILLGUMP,
	EVENT_DEATH,
	EVENT_SHOWPAPERDOLLNAME,
	EVENT_CONTEXTENTRY,
	EVENT_SHOWCONTEXTMENU,
	EVENT_SHOWTOOLTIP,
	EVENT_CHLEVELCHANGE,
	EVENT_SPEECH,
	EVENT_WEARITEM,
	EVENT_EQUIP,
	EVENT_UNEQUIP,
	EVENT_DROPONCHAR,
	EVENT_DROPONITEM,
	EVENT_DROPONGROUND,
	EVENT_PICKUP,
	EVENT_COMMAND,
	EVENT_BOOKUPDATEINFO,
	EVENT_BOOKREQUESTPAGE,
	EVENT_BOOKUPDATEPAGE,
	EVENT_DAMAGE,
	EVENT_CASTSPELL,
	EVENT_TRADE,
	EVENT_TRADESTART,
	EVENT_COUNT,
};

class cElement;
class cUOTxTooltipList;

class cPythonScript  
{
protected:
	QString name_; // Important!
	bool loaded;
	PyObject *codeModule; // This object stores the compiled Python Module
	PyObject *events[EVENT_COUNT];

public:
	cPythonScript();
	~cPythonScript();

	// We need an identification value for the scripts
	void setName( const QString &value ) { name_ = value; }
	QString name() const { return name_; }
	
	bool load( const cElement *element );
	void unload( void );
	bool isLoaded() const;

	static bool canChainHandleEvent( ePythonEvent event, cPythonScript **chain );
	static bool callChainedEventHandler( ePythonEvent, cPythonScript **chain, PyObject *args = 0 );
	static PyObject *callChainedEvent( ePythonEvent, cPythonScript **chain, PyObject *args = 0 );

	PyObject *callEvent( ePythonEvent, PyObject *args = 0, bool ignoreErrors = false );
	PyObject *callEvent( const QString &event, PyObject *args = 0, bool ignoreErrors = false );
	bool callEventHandler( ePythonEvent, PyObject *args = 0, bool ignoreErrors = false );
	bool callEventHandler( const QString &event, PyObject *args = 0, bool ignoreErrors = false );
	
	bool canHandleEvent( ePythonEvent event )
	{
		if( event >= EVENT_COUNT )
			return false;

		return events[event] != 0;
	}
};

#endif // __WPPYTHONSCRIPT_H__
