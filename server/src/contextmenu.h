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

#ifndef __CONTEXTMENU_H
#define __CONTEXTMENU_H


#include <vector>
#include <map>
#include "definable.h"
#include "wpdefmanager.h"
#include "globals.h"


class cConSingleOption
{
public:
	
	void		setOption( const QDomElement &Tag );
	Q_UINT16	getTag( void ) { return tag_; }
	Q_UINT16	getIntlocID(void) { return intlocid_; }
	Q_UINT16	getMsgID(void) { return msgid_; }
	QString		getScriptName(void) { return scriptname_; }
	
private:
	Q_UINT16	tag_;
	Q_UINT16	intlocid_;
	Q_UINT16	msgid_;
	QString		scriptname_;
};

typedef std::vector< cConSingleOption > vSingleOption;

class cConMenuOptions : public cDefinable
{
public:
	typedef std::vector< cConSingleOption > vSingleOption;
	
	void			processNode( const QDomElement &Tag );
	vSingleOption	getOptions( void ) { return options_; }
	void			addOption( const QDomElement &Tag );
	void			deleteAll( void ) {	options_.erase( options_.begin(), options_.end() );	}
	
private: 
	vSingleOption	options_;
	
};

class cConMenu : public cDefinable
{
public:
						cConMenu( const QDomElement &Tag );
	void				processNode( const QDomElement &Tag );
	cConMenuOptions*	getOptionsByAcl( QString acl );
	
private:
	std::map< QString, cConMenuOptions >	options_;
};

class cAllConMenus
{
protected:
	cAllConMenus() {};
	
public:
	~cAllConMenus()	{ menus_.clear(); }
	static cAllConMenus* cAllConMenus::getInstance()
	{
		static cAllConMenus theConMenusManager;
		return &theConMenusManager;
	}
	bool	MenuExist( QString bindmenu );
	void	load( void );
	void	reload( void );
	cConMenuOptions* getMenu( QString bindmenu, QString acl );
	
private:
	std::map< QString, cConMenu >	menus_;
	
};

#endif

