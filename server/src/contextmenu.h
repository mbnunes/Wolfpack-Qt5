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
class WPDefaultScript;

class cConSingleOption
{
public:
	
	void		setOption( const QDomElement &Tag );
	Q_UINT16	getTag( void ) { return tag_; }
	Q_UINT16	getIntlocID(void) { return intlocid_; }
	Q_UINT16	getMsgID(void) { return msgid_; }
	
private:
	Q_UINT16	tag_;
	Q_UINT16	intlocid_;
	Q_UINT16	msgid_;
};

typedef QValueVector< cConSingleOption > vSingleOption;

class cConMenuOptions : public cDefinable
{
public:
	typedef QValueVector< cConSingleOption > vSingleOption;
	
	void			processNode( const QDomElement &Tag );
	vSingleOption	getOptions( void ) const { return options_; }
	void			addOption( const QDomElement &Tag );
	void			deleteAll( void ) {	options_.clear();	}
	
private: 
	vSingleOption	options_;
	
};

class cConMenu : public cDefinable
{
public:
							cConMenu() {};
							cConMenu( const QDomElement &Tag );
	void					processNode( const QDomElement &Tag );
	const cConMenuOptions*	getOptionsByAcl( const QString& acl ) const;
    void					recreateEvents( void );
	
	bool					onContextEntry( cChar *Caller, cUObject *Target, Q_UINT16 Tag ) const;

private:
	QMap< QString, cConMenuOptions >	options_;
	QStringList							eventList_;
	std::vector<WPDefaultScript*>		scriptChain;

};

class cAllConMenus
{
public:
	~cAllConMenus()	{ menus_.clear(); }

	bool	MenuExist( const QString& bindmenu ) const;
	void	load( void );
	void	reload( void );
	const cConMenuOptions* getMenuOptions( const QString& bindmenu, const QString& acl ) const;
	const cConMenu*		   getMenu( const QString& bindmenu, const QString& acl ) const;
	
private:
	QMap< QString, cConMenu >	menus_;
};

typedef SingletonHolder<cAllConMenus> ContextMenus;

#endif // __CONTEXTMENU_H__

