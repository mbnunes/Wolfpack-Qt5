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

#ifndef __MAKEMENUS_H__
#define __MAKEMENUS_H__

// Wolfpack includes
#include "typedefs.h"
#include "definable.h"
#include "gumps.h"
#include "prototypes.h"

// Library includes
#include "qstringlist.h"
#include "qdom.h"

// Forward declaration
class cMakeMenuGump;

class cMakeAction : public cDefinable
{
public:
	cMakeAction( const QDomElement &Tag );
	~cMakeAction() {}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	QString		name()			{ return name_; }
	UINT16		model()			{ return model_; }
	QString		description()	{ return description_; }

	// Setters
	void		setName( QString data )		{ name_ = data; }
	void		setModel( UINT16 data )		{ model_ = data; }

	struct useitemprops_st
	{
		UINT16			id;
		QString			name;
		UINT16			colormin;
		UINT16			colormax;
		UINT16			amount;
	};

	struct makeitemprops_st
	{
		UINT16			model;
		QString			name;
		QString			section;
		UINT16			color;
		UINT16			amount;
	};

	struct makenpcprops_st
	{
		UINT16			model;
		QString			name;
		QString			section;
	};

	struct skillprops_st
	{
		UINT8 skillid;
		UINT16 min;
		UINT16 max;
	};

	std::vector< useitemprops_st >		useitems()	const { return useitems_; }
	std::vector< makeitemprops_st >		makeitems()	const { return makeitems_; }
	makenpcprops_st						makenpc()	const { return makenpc_; }
	std::vector< skillprops_st >		skills()	const { return skills_; }

	void		execute( cUOSocket* socket );

private:
	QString							name_;
	UINT16							model_;
	QString							description_;

	std::vector< useitemprops_st >		useitems_;
	std::vector< makeitemprops_st >		makeitems_;
	makenpcprops_st						makenpc_;
	std::vector< skillprops_st >		skills_;
	QString								failmsg_;
	QString								succmsg_;
	UINT8								charaction_;
	UINT16								sound_;
};

class cMakeMenu : public cDefinable
{
public:
	cMakeMenu::cMakeMenu( const QDomElement &Tag, cMakeMenu* previous = NULL );
	~cMakeMenu()
	{
		std::vector< cMakeMenu* >::iterator mit = submenus_.begin();
		while( mit != submenus_.end() )
		{
			delete (*mit);
			mit++;
		}

		std::vector< cMakeAction* >::iterator ait = actions_.begin();
		while( ait != actions_.end() )
		{
			delete (*ait);
			ait++;
		}
	}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	std::vector< cMakeMenu* >	subMenus()		{ return submenus_; }
	std::vector< cMakeAction* > actions()		{ return actions_; }
	QString						name()			{ return name_; }
	UINT16						model()			{ return model_; }
	QString						description()	{ return description_; }
	cMakeMenu*					prevMenu()		{ return prev_; }
	UINT8						type()			{ return type_; }

	// Setters
	void	setName( QString data )							{ name_ = data; }
	void	setModel( UINT16 data )							{ model_ = data; }

	bool	contains( cMakeMenu* pMenu )
	{
		std::vector< cMakeMenu* >::iterator it = submenus_.begin();
		while( it != submenus_.end() )
		{
			if( (*it) == pMenu )
				return true;
			else
				if( (*it)->contains( pMenu ) )
					return true;
			it++;
		}
		return false;
	}

private:
	QString						name_;
	UINT16						model_;
	QString						description_;
	UINT8						type_;

	cMakeMenu*					prev_;
	std::vector< cMakeMenu* >	submenus_;
	std::vector< cMakeAction* > actions_;
};

class cMakeMenuGump : public cGump
{
private:
	cMakeMenu* menu_;
	cMakeMenu* prev_;
public:
	cMakeMenuGump( cMakeMenu* menu );
	
	// implements cGump
	virtual void handleResponse( cUOSocket* socket, gumpChoice_st choice );
};

class cAllMakeMenus
{
protected:
	cAllMakeMenus() {};
public:
	~cAllMakeMenus()
	{
		std::map< QString, cMakeMenu* >::iterator iter = menus_.begin();
		while( iter != menus_.end() )
		{
			delete iter->second;
			iter++;
		}
		menus_.clear();
	}

	static cAllMakeMenus* getInstance()
	{
		static cAllMakeMenus theMakeMenusManager;
		return &theMakeMenusManager; 
	}

	void	load( void );

	void	reload( void );

	bool	contains( cMakeMenu* pMenu )
	{
		std::map< QString, cMakeMenu* >::iterator it = menus_.begin();
		while( it != menus_.end() )
		{
			if( it->second == pMenu )
				return true;
			else
				if( it->second->contains( pMenu ) )
					return true;
			it++;
		}
		return false;
	}

	cMakeMenu* getMenu( QString section )
	{
		std::map< QString, cMakeMenu* >::iterator it = menus_.find( section );
		cMakeMenu* pMakeMenu = NULL;
		if( it != menus_.end() )
			pMakeMenu = it->second;
		return pMakeMenu;
	}

private:
	std::map< QString, cMakeMenu* >		menus_;
};


#endif




