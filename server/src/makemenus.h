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
#include "qptrlist.h"
#include "qvaluevector.h"

// Forward declaration
class cMakeMenu;
class cMakeAction;
class cMakeSection;
class cMakeMenuGump;
class cItem;

class cMakeItem : public cDefinable
{
public:
	cMakeItem( const QDomElement &Tag );
	~cMakeItem() {}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	QString			name()		const	{ return name_; }
	QString			section()	const	{ return section_; }
	UINT16			amount()	const	{ return amount_; }

	// Setters
	void			setAmount( UINT16 amount ) { amount_ = amount; }
	void			setSection( QString data ) { section_ = data; }

private:
	QString			name_;
	QString			section_;
	UINT16			amount_;
};

class cUseItem : public cDefinable
{
public:
	cUseItem( const QDomElement &Tag );
	cUseItem( QString name, QValueVector< UINT16 > ids, QValueVector< UINT16 > colors, UINT16 amount );
	~cUseItem() {}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	QValueVector< UINT16 >	id()		{ return id_; }
	QString		name()		{ return name_; }
	QValueVector< UINT16 >	colors()	{ return colors_; }
	UINT16		amount()	{ return amount_; }

	bool		hasEnough( cItem* pBackpack );
private:
	QValueVector< UINT16 >	id_;
	QString		name_;
	QValueVector< UINT16 >	colors_;
	UINT16		amount_;
};

class cSkillCheck : public cDefinable
{
public:
	cSkillCheck( const QDomElement &Tag );
	~cSkillCheck() {}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	UINT8		skillid()		{ return skillid_; }
	UINT16		minimum()		{ return min_; }
	UINT16		maximum()		{ return max_; }

	bool		skilledEnough( cChar* pChar );
	void		applySkillMod( float skillmod );
private:
	UINT8		skillid_;
	UINT16		min_;
	UINT16		max_;
};

class cMakeSection : public cDefinable
{
public:
	cMakeSection( const QDomElement &Tag, cMakeAction* baseaction = NULL );
	~cMakeSection() 
	{
		makeitems_.setAutoDelete( true );
		makeitems_.clear();

		useitems_.setAutoDelete( true );
		useitems_.clear();

		skillchecks_.setAutoDelete( true );
		skillchecks_.clear();

	}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	struct makenpcprops_st
	{
		QString			name;
		QString			section;
	};

	// Getters
	QPtrList< cMakeItem >		makeitems()		const { return makeitems_; }
	QPtrList< cUseItem >		useitems()		const { return useitems_; }
	QPtrList< cSkillCheck >		skillchecks()	const { return skillchecks_; }
	QString						name()			const { return name_; }
	cMakeAction*				baseAction()	const { return baseaction_; }
	makenpcprops_st				makenpc()		const { return makenpc_; }

	// Setters
	void		setName( QString data )		{ name_ = data; }
	void		appendUseItem( cUseItem* pui ) { useitems_.append( pui ); }

	void		execute( cUOSocket* socket );

	// execute helper methods
	bool	hasEnough( cItem* pBackpack );
	void	useResources( cItem* pBackpack );
	// if any of foreach(skill) : skill < min => false
	bool	skilledEnough( cChar* pChar );
	// calcRank checks the skill and may raise it! (==0) => failed, (>0) => success
	UINT32	calcRank( cChar* pChar );

	// action type specific methods
	void	setMakeItemAmounts( UINT16 amount );
	void	addMakeItemSectionPrefixes( QString prefix );
	void	applySkillMod( float skillmod );
private:
	QPtrList< cMakeItem >		makeitems_;
	QPtrList< cUseItem >		useitems_;
	QPtrList< cSkillCheck >		skillchecks_;
	makenpcprops_st				makenpc_;

	QString						name_;
	cMakeAction*				baseaction_;
};

class cMakeAction : public cDefinable
{
public:
	enum WPACTIONTYPE
	{
		CUSTOM_SECTIONS = 0,// define makesections with <make> tag
		RESOURCE_SECTIONS,	// define makesections by a resource's types (ores, woods)
		AMOUNT_SECTIONS,	// define makesections by a list of amounts
		NPC_SECTION,		// define a makesection by an npc section
		DELAYED_SECTIONS,	// define a makesection by several actions to be executed in a delayed action queue (alchemy)
		CODE_ACTION,		// run a code segment
		SCRIPT_ACTION		// run a script
	};

	cMakeAction( const QDomElement &Tag, cMakeMenu* basemenu = NULL );
	~cMakeAction() 
	{
		std::vector< cMakeSection* >::iterator it = makesections_.begin();
		while( it != makesections_.end() )
		{
			delete (*it);
			it++;
		}
	}

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	QString			name()			{ return name_; }
	UINT16			model()			{ return model_; }
	QString			description()	{ return description_; }
	cMakeMenu*		baseMenu()		{ return basemenu_; }
	QString			failMsg()		const { return failmsg_; }
	QString			succMsg()		const { return succmsg_; }
	UINT8			charAction()	const { return charaction_; }
	UINT16			succSound()		const { return succsound_; }
	UINT16			failSound()		const { return failsound_; }
	WPACTIONTYPE	type()			const { return type_; }

	// Setters
	void		setName( QString data )		{ name_ = data; }
	void		setModel( UINT16 data )		{ model_ = data; }

	std::vector< cMakeSection* >		makesections()	const { return makesections_; }

	void		execute( cUOSocket* socket, UINT32 makesection );

private:
	QString							name_;
	UINT16							model_;
	QString							description_;
	cMakeMenu*						basemenu_;
	WPACTIONTYPE					type_;

	std::vector< cMakeSection* >		makesections_;
	QString								failmsg_;
	QString								succmsg_;
	UINT8								charaction_;
	UINT16								succsound_;
	UINT16								failsound_;

};

class cMakeMenu : public cDefinable
{
public:
	cMakeMenu( const QDomElement &Tag, cMakeMenu* previous = NULL );
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
	QString						link()			{ return link_; }
	cMakeMenu*					prevMenu()		{ return prev_; }

	// Setters
	void	setName( QString data )							{ name_ = data; }

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

	cMakeMenu* baseMenu()
	{
		if( prev_ )
			return prev_->baseMenu();
		else
			return this;
	}

private:
	QString						name_;
	QString						link_;

	cMakeMenu*					prev_;
	std::vector< cMakeMenu* >	submenus_;
	std::vector< cMakeAction* > actions_;
};

class cMakeMenuGump : public cGump
{
private:
	cMakeMenu* menu_;
	cMakeMenu* prev_;
	cMakeAction* action_;
public:
	cMakeMenuGump( cMakeMenu* menu, cUOSocket* socket, QString notices = "" );
	cMakeMenuGump( cMakeAction* action, cUOSocket* socket );
	
	// implements cGump
	virtual void handleResponse( cUOSocket* socket, gumpChoice_st choice );
};

class cLastTenGump : public cGump
{
private:
	cMakeMenu* prev_;
	QPtrList< cMakeSection > sections_;
public:
	cLastTenGump( QPtrList< cMakeSection >, cMakeMenu* prev, QString notices = "" );

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

	void	callMakeMenu( cUOSocket* socket, QString section );

private:
	std::map< QString, cMakeMenu* >		menus_;
};


#endif




