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

#ifndef __MAKEMENUS_H__
#define __MAKEMENUS_H__

// Wolfpack includes
#include "typedefs.h"
#include "definable.h"
#include "gumps.h"
#include "singleton.h"

// Library includes
#include <qstringlist.h>
#include <qptrlist.h>
#include <qvaluevector.h>

// Forward declaration
class cMakeMenu;
class cMakeAction;
class cMakeSection;
class cMakeMenuGump;
class cItem;

class cMakeItem : public cDefinable
{
public:
	cMakeItem( const QString &name, const QString &section, UINT16 amount );
	cMakeItem( const cElement *Tag );

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	// Getters
	QString			name()		const	{ return name_; }
	QString			section()	const	{ return section_; }
	UINT16			amount()	const	{ return amount_; }

	// Setters
	void			setAmount( UINT16 amount )			{ amount_ = amount; }
	void			setSection( const QString& data )	{ section_ = data; }

private:
	QString			name_;
	QString			section_;
	UINT16			amount_;
};

class cUseItem : public cDefinable
{
public:
	cUseItem( const cElement *Tag );
	cUseItem( const QString& name, const QValueVector< UINT16 >& ids, const QValueVector< UINT16 >& colors, UINT16 amount );

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	// Getters
	QValueVector< UINT16 >	id()		const { return id_; }
	QString					name()		const { return name_; }
	QValueVector< UINT16 >	colors()	const { return colors_; }
	UINT16					amount()	const { return amount_; }

	bool		hasEnough( const cItem* pBackpack ) const;
private:
	QValueVector< UINT16 >	id_;
	QString		name_;
	QValueVector< UINT16 >	colors_;
	UINT16		amount_;
};

class cSkillCheck : public cDefinable
{
public:
	cSkillCheck( const cElement *Tag );

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	// Getters
	UINT8		skillid()		{ return skillid_; }
	UINT16		minimum()		{ return min_; }
	UINT16		maximum()		{ return max_; }

	bool		skilledEnough( P_CHAR pChar );
	void		applySkillMod( float skillmod );
private:
	UINT8		skillid_;
	UINT16		min_;
	UINT16		max_;
};

class cMakeSection : public cDefinable
{
public:
	cMakeSection( const QString &name, cMakeAction* baseaction = NULL );
	cMakeSection( const cElement *Tag, cMakeAction* baseaction = NULL );
	~cMakeSection();

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	// Getters
	QString						name()			const { return name_; }
	cMakeAction*				baseAction()	const { return baseaction_; }

	// Setters
	void		setName( const QString& data )			{ name_ = data; }

	virtual void	execute( cUOSocket* const socket );

protected:

	QString						name_;
	cMakeAction*				baseaction_;
};

class cMakeCustomSection : public cMakeSection
{
public:
	cMakeCustomSection( const QString &name, cMakeAction* baseaction = NULL );
	cMakeCustomSection( const cElement *Tag, cMakeAction* baseaction = NULL );
	~cMakeCustomSection();

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	QPtrList< cMakeItem >		makeitems()		const { return makeitems_; }
	QPtrList< cUseItem >		useitems()		const { return useitems_; }
	QPtrList< cSkillCheck >		skillchecks()	const { return skillchecks_; }

	virtual void	execute( cUOSocket* const socket );

	void		appendMakeItem( const cMakeItem* pmi )	{ makeitems_.append( pmi ); }
	void		appendUseItem( const cUseItem* pui )	{ useitems_.append( pui ); }

	// execute helper methods
	bool	hasEnough( cItem* pBackpack );
	void	useResources( cItem* pBackpack );
	// if any of foreach(skill) : skill < min => false
	bool	skilledEnough( P_CHAR pChar );
	// calcRank checks the skill and may raise it! (==0) => failed, (>0) => success
	UINT32	calcRank( P_CHAR pChar );

protected:
	QPtrList< cMakeItem >		makeitems_;
	QPtrList< cUseItem >		useitems_;
	QPtrList< cSkillCheck >		skillchecks_;
};

class cMakeNpcSection : public cMakeSection
{
public:
	cMakeNpcSection( const QString &name, cMakeAction* baseaction = NULL );
	cMakeNpcSection( const cElement *Tag, cMakeAction* baseaction = NULL );
	~cMakeNpcSection();

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	virtual void	execute( cUOSocket* const socket );

	struct makenpcprops_st
	{
		QString			name;
		QString			section;
	};

	makenpcprops_st				makenpc()		const { return makenpc_; }

	void	setNpcProperties( const QString &name, const QString &section )
	{
		makenpc_.name = name;
		makenpc_.section = section;
	}

private:
	makenpcprops_st				makenpc_;

};

class cMakeResourceSection : public cMakeCustomSection
{
public:
	cMakeResourceSection( const QString &name, cMakeAction* baseaction = NULL );
	cMakeResourceSection( const cElement *Tag, cMakeAction* baseaction = NULL );

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	virtual void	execute( cUOSocket* const socket );

	void	addMakeItemSectionPrefixes( const QString& prefix );
	void	applySkillMod( float skillmod );
};

class cMakeAmountSection : public cMakeCustomSection
{
public:
	cMakeAmountSection( const QString &name, cMakeAction* baseaction = NULL );
	cMakeAmountSection( const cElement *Tag, cMakeAction* baseaction = NULL );

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	virtual void	execute( cUOSocket* const socket );

	// action type specific methods
	void	setMakeItemAmounts( UINT16 amount );
};

class cDoCodeAction : public cMakeSection
{
public:
	cDoCodeAction( const QString &name, cMakeAction* baseaction = NULL );
	cDoCodeAction( const cElement *Tag, cMakeAction* baseaction = NULL );
	~cDoCodeAction();

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	virtual void	execute( cUOSocket* const socket );
	void setParams( const QString& a ) { params = a; }

protected:
	QString params;
};

class cDoScriptAction : public cMakeSection
{
public:
	cDoScriptAction( const QString &name, cMakeAction* baseaction = NULL );
	cDoScriptAction( const cElement *Tag, cMakeAction* baseaction = NULL );
	~cDoScriptAction();

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	virtual void	execute( cUOSocket* const socket );

protected:
	QString functionName;
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
	typedef QValueVector< cMakeSection* > SectionContainer;

	cMakeAction( const QString &name, UINT16 model, const QString &description, WPACTIONTYPE type, cMakeMenu* basemenu = NULL );
	cMakeAction( const cElement *Tag, cMakeMenu* basemenu = NULL );
	~cMakeAction()
	{
		SectionContainer::iterator it = makesections_.begin();
		SectionContainer::const_iterator end(makesections_.end());
		while( it != end )
		{
			delete (*it);
			++it;
		}
	}

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	// Getters
	QString			name()			const { return name_; }
	UINT16			model()			const { return model_; }
	QString			description()	const { return description_; }
	cMakeMenu*		baseMenu()		const { return basemenu_; }
	QString			failMsg()		const { return failmsg_; }
	QString			succMsg()		const { return succmsg_; }
	UINT8			charAction()	const { return charaction_; }
	UINT16			succSound()		const { return succsound_; }
	UINT16			failSound()		const { return failsound_; }
	WPACTIONTYPE	type()			const { return type_; }

	// Setters
	void		setName( const QString& data )		{ name_ = data; }
	void		setModel( UINT16 data )		{ model_ = data; }
	void		appendSection( cMakeSection* pms )	{ makesections_.push_back( pms ); }

	SectionContainer	makesections()	const { return makesections_; }

	void		execute( cUOSocket* socket, UINT32 makesection );

private:
	QString				name_;
	UINT16				model_;
	QString				description_;
	cMakeMenu*			basemenu_;
	WPACTIONTYPE		type_;

	SectionContainer	makesections_;
	QString				failmsg_;
	QString				succmsg_;
	UINT8				charaction_;
	UINT16				succsound_;
	UINT16				failsound_;

};

class cMakeMenu : public cDefinable
{
public:
	typedef QValueVector< cMakeMenu* > SubMenuContainer;
	typedef QValueVector< cMakeAction* > ActionContainer;
	
	cMakeMenu( const QString& name, cMakeMenu* previous = NULL );
	cMakeMenu( const cElement *Tag, cMakeMenu* previous = NULL );
	~cMakeMenu()
	{
		SubMenuContainer::iterator mit = submenus_.begin();
		while( mit != submenus_.end() )
		{
			delete (*mit);
			++mit;
		}

		ActionContainer::iterator ait = actions_.begin();
		while( ait != actions_.end() )
		{
			delete (*ait);
			++ait;
		}
	}

	// implements cDefinable
	virtual void processNode( const cElement *Tag );

	// Getters
	SubMenuContainer	subMenus() const		{ return submenus_; }
	ActionContainer		actions() const			{ return actions_; }
	QString				name() const			{ return name_; }
	QString				link() const			{ return link_; }
	cMakeMenu*			prevMenu() const		{ return prev_; }

	// Setters
	void	setName( const QString& data )							{ name_ = data; }

	bool	contains( const cMakeMenu* pMenu ) const
	{
		SubMenuContainer::const_iterator it = submenus_.begin();
		SubMenuContainer::const_iterator end( submenus_.end() );
		while( it != end )
		{
			if( (*it) == pMenu )
				return true;
			else
				if( (*it)->contains( pMenu ) )
					return true;
			++it;
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

	// more methods
	void	addSubMenu( cMakeMenu* pMenu )
	{
		submenus_.push_back( pMenu );
	}

	void	addAction( cMakeAction* pAction )
	{
		actions_.push_back( pAction );
	}

private:
	QString				name_;
	QString				link_;

	cMakeMenu*			prev_;
	SubMenuContainer	submenus_;
	ActionContainer		actions_;
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
	virtual void handleResponse( cUOSocket* socket, const gumpChoice_st& choice );
};

class cLastTenGump : public cGump
{
private:
	cMakeMenu* prev_;
	QPtrList< cMakeSection > sections_;
public:
	cLastTenGump( QPtrList< cMakeSection >, cMakeMenu* prev, QString notices = "" );

	// implements cGump
	virtual void handleResponse( cUOSocket* socket, const gumpChoice_st& choice );
};

class cAllMakeMenus
{
public:
	~cAllMakeMenus();

	void	load( void );
	void	reload( void );

	bool	contains( const cMakeMenu* pMenu ) const
	{
		std::map< QString, cMakeMenu* >::const_iterator it(menus_.begin());
		std::map< QString, cMakeMenu* >::const_iterator end(menus_.end());
		while( it != end )
		{
			if( it->second == pMenu )
				return true;
			else
				if( it->second->contains( pMenu ) )
					return true;
			++it;
		}
		return false;
	}

	cMakeMenu* getMenu( const QString& section ) const
	{
		std::map< QString, cMakeMenu* >::const_iterator it = menus_.find( section );
		if( it != menus_.end() )
			return it->second;
		else
			return 0;
	}

	void	callMakeMenu( cUOSocket* socket, const QString& section );

	// helper method
	UINT16 getModel( const cElement *Tag );

private:
	std::map< QString, cMakeMenu* >		menus_;
	void unload();
	void parseLocationNode( cMakeMenu* pGoMenu, const cElement* defSection );
};

typedef SingletonHolder<cAllMakeMenus> MakeMenus;

#endif




