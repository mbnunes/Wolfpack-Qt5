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

#include "makemenus.h"
#include "wolfpack.h"
#include "network/uosocket.h"
#include "classes.h"
#include "targetrequests.h"
#include "wpdefmanager.h"
#include "charsmgr.h"

#include "debug.h"

#undef  DBGFILE
#define DBGFILE "makemenus.cpp"

cMakeItem::cMakeItem( const QDomElement &Tag )
{
	name_ = Tag.attribute( "name" );
	section_ = Tag.attribute( "section" );
	amount_ = hex2dec( Tag.attribute( "amount" ) ).toUShort();
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_MAKEITEM, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	applyDefinition( Tag );
}

void cMakeItem::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "name" )
		name_ = Value;

	else if( TagName == "section" )
		section_ = Value;

	else if( TagName == "amount" )
		amount_ = Value.toUShort();
}

cUseItem::cUseItem( const QDomElement &Tag )
{
	name_ = Tag.attribute( "name" );
	id_ = hex2dec( Tag.attribute( "itemid" ) ).toUShort();
	amount_ = hex2dec( Tag.attribute( "amount" ) ).toUShort();
	colormin_ = 0;
	colormax_ = 0;
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_USEITEM, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	applyDefinition( Tag );
}

void cUseItem::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "name" )
		name_ = Value;

	else if( TagName == "id" )
		id_ = Value.toUShort();

	else if( TagName == "amount" )
		amount_ = Value.toUShort();

	else if( TagName == "color" )
	{
		QString color = Tag.nodeValue();
		if( color.contains( "-" ) )
		{
			QStringList colors = QStringList::split( "-", color );
			colormin_ = hex2dec( colors[0] ).toUShort();
			colormax_ = hex2dec( colors[1] ).toUShort();
		}
		else
			colormin_ = Value.toUShort();
	}
}

cSkillCheck::cSkillCheck( const QDomElement &Tag )
{
	skillid_ = hex2dec( Tag.attribute( "skillid" ) ).toUShort();
	min_ = hex2dec( Tag.attribute( "min" ) ).toUShort();
	max_ = hex2dec( Tag.attribute( "max" ) ).toUShort();
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_SKILLCHECK, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	applyDefinition( Tag );
}

void cSkillCheck::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "skillid" )
		skillid_ = Value.toUShort();

	else if( TagName == "min" )
		min_ = Value.toUShort();

	else if( TagName == "max" )
		max_ = Value.toUShort();
}

cMakeSection::cMakeSection( const QDomElement &Tag, cMakeAction* baseaction )
{
	baseaction_ = baseaction;
	name_ = Tag.attribute( "name" );
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_MAKESECTION, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	applyDefinition( Tag );
}

void cMakeSection::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "makeitem" )
	{
		cMakeItem* pMakeItem = new cMakeItem( Tag );
		makeitems_.append( pMakeItem );
	}

	else if( TagName == "useitem" )
	{
		cUseItem* pUseItem = new cUseItem( Tag );
		useitems_.append( pUseItem );
	}

	else if( TagName == "skillcheck" )
	{
		cSkillCheck* pSkillCheck = new cSkillCheck( Tag );
		skillchecks_.append( pSkillCheck );
	}

	else if( TagName == "name" )
		name_ = Value;

	else if( TagName == "makenpc" )
	{
		makenpc_.name = Tag.attribute( "name" );
		makenpc_.section = Tag.attribute( "id" );
	}
}

bool	cMakeSection::hasEnough( cItem* pBackpack )
{
	bool hasEnough = true;
	QPtrListIterator< cUseItem > uiit( useitems_ );
	while( (uiit.current()) && hasEnough )
	{
		// this one is in here because a GM doesnt need a backpack if he uses the add menu!
		if( !pBackpack )
			hasEnough = false;

		// the next loop will search for a the item in a range of colors.
		// it is a do-while, cause it shall run once through the loop if
		// colormin holds the one color and colormax == 0!
		UINT16 color = uiit.current()->colormin();
		UINT16 amount = 0;
		do
		{
			amount += pBackpack->CountItems( uiit.current()->id(), color );
			++color;
		} while( color <= uiit.current()->colormax() );

		if( amount < uiit.current()->amount() )
			hasEnough = false;

		++uiit;
	}
	return hasEnough;
}

void	cMakeSection::useResources( cItem* pBackpack )
{
	QPtrListIterator< cUseItem > uiit( useitems_ );
	while( uiit.current() )
	{
		if( !pBackpack )
			return;

		// the next loop will use the items up in a range of colors.
		// it is a do-while, cause it shall run once through the loop if
		// colormin holds the one color and colormax == 0!
		UINT16 color = uiit.current()->colormin();
		UINT16 amount = uiit.current()->amount();
		UINT16 curramount = 0;
		do
		{
			// remove all available items or just the amount thats left
			curramount = pBackpack->CountItems( uiit.current()->id(), color );
			if( curramount > amount )
				curramount = amount;
			pBackpack->DeleteAmount( curramount, uiit.current()->id(), color );

			amount -= curramount;
			++color;
		} while( color <= uiit.current()->colormax() && amount > 0 );

		++uiit;
	}
}

// if any of foreach(skill) : skill < min => false
bool	cMakeSection::skilledEnough( cChar* pChar )
{
	bool skilledEnough = true;
	QPtrListIterator< cSkillCheck > skit( skillchecks_ );
	while( skit.current() && skilledEnough )
	{
		if( pChar->skill( skit.current()->skillid() ) < skit.current()->min() )
			skilledEnough = false;
		++skit;
	}
	return skilledEnough;
}

// calcRank checks the skill and may raise it! (==0) => failed, (>0) => success
UINT32	cMakeSection::calcRank( cChar* pChar )
{
	bool hasSuccess = true;
	UINT32 ranksum = 1;
	QPtrListIterator< cSkillCheck > skit( skillchecks_ );
	while( skit.current() && hasSuccess )
	{
		if( !Skills->CheckSkill( pChar, skit.current()->skillid(), skit.current()->min(), skit.current()->max() ) )
			hasSuccess = false;
		else
		{
			UINT16 skill = pChar->skill( skit.current()->skillid() );
			if( skill > skit.current()->max() )
				skill = skit.current()->max();
			if( skill < skit.current()->min() )
				skill = skit.current()->min();

			ranksum += (UINT16)( ceil( (double)skill / (double)(skit.current()->max() - skit.current()->min()) * 10.0f ) );
		}
		++skit;
	}
	if( !hasSuccess )
		return 0;
	else
		return ranksum;
}

void cMakeSection::execute( cUOSocket* socket )
{
	P_CHAR pChar = socket->player();
	P_ITEM pBackpack = FindItemBySerial( pChar->packitem );

	if( !socket || !pChar || !baseaction_ )
		return;

	// first check for necessary items
	if( !hasEnough( pBackpack ) )
	{
		socket->sysMessage( tr("You do not have enough resources to create this!") );
		return;
	}

	// now we have to use the resources up!
	useResources( pBackpack );

	// then lets check the skill and calculate the item rank!
	UINT32 ranksum = calcRank( pChar );
	if( ranksum == 0 )
	{
		if( !baseaction_->failMsg().isNull() )
		socket->sysMessage( baseaction_->failMsg() );
		return;
	}
	else if( !baseaction_->succMsg().isNull() )
		socket->sysMessage( baseaction_->succMsg() );

	// lets calculate the rank for the item now
	UINT16 rank;
	if( skillchecks_.count() == 0 )
		rank = 10;
	else
		rank = (UINT16)(ceil( ranksum / skillchecks_.count() ));
	if( rank > 10 )
		rank = 10;
	if( rank < 1 )
		rank = 1;

	// finally lets create the items/npcs!

	// items:
	QPtrList< cMakeItem > makeitems = makeitems_;
	QPtrListIterator< cMakeItem > miit( makeitems_ );
	while( miit.current() )
	{
		if( miit.current()->section().isNull() )
		{
			++miit;
			continue;
		}
		P_ITEM pItem = Items->createScriptItem( miit.current()->section() );
		UINT16 minhp = 0;
		UINT16 maxhp = 0;
		if( pItem )
		{
			minhp = (UINT16)floor( (double)(rank-1) * 10.0f / 100.0f * (double)pItem->hp() );
			maxhp = (UINT16)floor( (double)(rank+1) * 10.0f / 100.0f * (double)pItem->hp() );
			if( pItem->isPileable() )
				pItem->setAmount( miit.current()->amount() );
			pItem->setHp( RandomNum( minhp, maxhp ) );
			pBackpack->AddItem( pItem );
		}

		// if the item is not pileable create amount-1 items more
		if( pItem && !pItem->isPileable() )
		{
			for( UINT16 i = 1; i < miit.current()->amount(); i++ )
			{
				pItem = Items->createScriptItem( miit.current()->section() );
				if( pItem )
				{
					pItem->setHp( RandomNum( minhp, maxhp ) );
					pBackpack->AddItem( pItem );
				}
			}
		}
		++miit;
	}

	// npcs:
	if( !makenpc_.section.isEmpty() )
		socket->attachTarget( new cAddNpcTarget( makenpc_.section ) );

	if( baseaction_->charAction() > 0 )
		pChar->action( baseaction_->charAction() );

	if( baseaction_->sound() > 0 )
		pChar->soundEffect( baseaction_->sound() );

	// TODO rank messages
}

cMakeAction::cMakeAction( const QDomElement &Tag, cMakeMenu* basemenu )
{
	basemenu_ = basemenu;
	name_ = Tag.attribute( "id" );
	description_ = (char*)0;
	failmsg_ = (char*)0;
	succmsg_ = (char*)0;
	charaction_ = 0;
	sound_ = 0;
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_ACTION, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	applyDefinition( Tag );
}

void cMakeAction::processNode( const QDomElement &Tag )
{
	// CAUTION: the base tag attributes are evaluated in the constructor!!
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "make" )
	{
		cMakeSection* pMakeSection = new cMakeSection( Tag, this );
		if( pMakeSection )
			makesections_.push_back( pMakeSection );
	}

	else if( TagName == "fail" )
		failmsg_ = Value;

	else if( TagName == "success" )
		succmsg_ = Value;

	else if( TagName == "description" )
		description_ = Value;

	else if( TagName == "charaction" )
		charaction_ = Value.toUShort();

	else if( TagName == "name" )
		name_ = Value;

	else if( TagName == "sound" )
		sound_ = Value.toUShort();

	else if( TagName == "model" )
		model_ = Value.toUShort();

	else if( TagName == "bodymodel" )
		model_ = creatures[ Value.toUShort() ].icon;
}

void cMakeAction::execute( cUOSocket* socket, UINT32 makesection )
{
	if( makesection >= makesections_.size() )
		return;

	cMakeSection* pSection = makesections_[ makesection ];
	if( !pSection )
		return;

	pSection->execute( socket );
}

cMakeMenu::cMakeMenu( const QDomElement &Tag, cMakeMenu* previous )
{
	name_ = Tag.attribute( "id" );
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_MENU, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	prev_ = previous;
	applyDefinition( Tag );
}

void cMakeMenu::processNode( const QDomElement &Tag )
{
	// CAUTION: the base tag attributes are evaluated in the constructor!!
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "menu" )
	{
		cMakeMenu* pMakeMenu = new cMakeMenu( Tag, this );
		submenus_.push_back( pMakeMenu );
	}

	else if( TagName == "action" )
	{
		cMakeAction* pMakeAction = new cMakeAction( Tag, this );
		actions_.push_back( pMakeAction );
	}

	else if( TagName == "name" )
		name_ = Value;
}

cMakeMenuGump::cMakeMenuGump( cMakeMenu* menu, cUOSocket* socket, QString notices )
{
	QString htmlmask = "<body text=\"#FFFFFF\">%1</body>";
	QString htmlmaskcenter = "<body text=\"#FFFFFF\"><div align=\"center\">%1</div></body>";

	action_ = NULL;
	menu_ = menu;
	prev_ = menu->prevMenu();

	setX( 50 );
	setY( 50 );

	startPage();
	addResizeGump( 0, 0, 0x13BE, 530, 417 );

	if( socket->version().contains("3D") )
	{
		addTiledGump( 10, 32, 510, 5, 0xA40, -1 );
		addTiledGump( 10, 287, 510, 5, 0xA40, -1 );
		addTiledGump( 10, 337, 510, 5, 0xA40, -1 );
		addTiledGump( 160, 292, 5, 45, 0xA40, -1 );
		addTiledGump( 210, 37, 5, 250, 0xA40, -1 );
		addTiledGump( 10, 37, 5, 305, 0xA40, -1 );
		addTiledGump( 515, 37, 5, 305, 0xA40, -1 );
	}
	else
	{
		addTiledGump( 10, 10, 510, 22, 0xA40, -1 );
		addTiledGump( 10,292, 150, 45, 0xA40, -1 );
		addTiledGump( 165, 292, 355, 45, 0xA40, -1 );
		addTiledGump( 10, 342, 510, 65, 0xA40, -1 );
		addTiledGump( 10, 37, 200, 250, 0xA40, -1 );
		addTiledGump( 215, 37, 305, 250, 0xA40, -1 );
		addCheckertrans( 10, 10, 510, 397 );
	}

	addHtmlGump( 10, 12, 510, 20, htmlmaskcenter.arg( menu_->name() ) ); 
	addHtmlGump( 10, 39, 200, 20, htmlmaskcenter.arg( tr("CATEGORIES") ) ); 
	addHtmlGump( 215, 39, 305, 20, htmlmaskcenter.arg( tr("SELECTIONS") ) ); 
	addHtmlGump( 10, 294, 150, 20, htmlmaskcenter.arg( tr("NOTICES") ) );

	if( !notices.isEmpty() )
		addHtmlGump( 170, 292, 345, 41, htmlmask.arg( notices ) );

	// EXIT button , return value: 0
	addButton( 15, 382, 0xFB1, 0xFB3, 0 );
	addHtmlGump( 50, 385, 150, 18, htmlmask.arg( tr("EXIT") ) );

	// LAST TEN button , return value: 1
	addButton( 15, 60, 4011, 4012, 1 );
	addHtmlGump( 50, 63, 150, 18, htmlmask.arg( tr("LAST TEN") ) );

	// MARK ITEM switch , switch index: 1
	addCheckbox( 270, 362, 4005, 4007, 1, false );
	addHtmlGump( 305, 365, 150, 18, htmlmask.arg( tr("MARK ITEM") ) );

	// MAKE LAST button , return value: 2
	addButton( 270, 382, 4005, 4007, 2 );
	addHtmlGump( 305, 385, 150, 18, htmlmask.arg( tr("MAKE LAST") ) );

	// PREVIOUS MENU button, return value: -1
	if( menu_->prevMenu() )
	{
		addButton( 15, 362, 0xFAE, 0xFB0, -1 );
		addHtmlGump( 50, 365, 150, 18, htmlmask.arg( tr("PREVIOUS MENU") ) );
	}

	std::vector< cMakeMenu* > submenus = menu_->subMenus();
	std::vector< cMakeAction* > actions = menu_->actions();
	std::vector< cMakeMenu* >::iterator mit = submenus.begin();
	std::vector< cMakeAction* >::iterator ait = actions.begin();

	UINT32 page_;
	UINT32 menupages = ((UINT32)ceil( (double)submenus.size() / 9.0f ));
	UINT32 actionpages = ((UINT32)ceil( (double)actions.size() / 10.0f ));
	UINT32 pages = ( menupages > actionpages ) ? menupages : actionpages;

	for( page_ = 1; page_ <= pages; page_++ )
	{
		startPage( page_ );
		UINT32 yoffset = 80;
		UINT32 i = (page_-1) * 9;
		if( i  <= submenus.size() )
		{
			while( mit != submenus.end() && i <= submenus.size() && i < (page_ * 9) )
			{
				// category buttons, return values: 3 -> submenus.size()+3-1
				addButton( 15, yoffset, 4005, 4007, i+3 );
				addHtmlGump( 50, yoffset+3, 150, 18, htmlmask.arg( (*mit)->name() ) );
				yoffset += 20;
				i++;
				mit++;
			}
		}

		yoffset = 60;
		i = (pages-1) * 10;
		if( i  <= actions.size() )
		{
			while( ait != actions.end() && i <= actions.size() && i < (page_ * 10) )
			{
				// selection buttons, return values: submenus.size()+3 -> actions.size()+submenus.size()+3-1
				addButton( 220, yoffset, 4005, 4007, i + submenus.size() + 4 );
				addButton( 480, yoffset, 4011, 4012, i + submenus.size() + 4 + 1000 );
				addHtmlGump( 255, yoffset+3, 220, 18, htmlmask.arg( (*ait)->name() ) );
				yoffset += 20;
				i++;
				ait++;
			}
		}

		// page switch buttons
		if( page_ > 1 )
		{
			addPageButton( 15, 342, 0xFAE, 0xFB0, page_-1 );
			addHtmlGump( 50, 345, 150, 18, htmlmask.arg( tr("PREVIOUS PAGE") ) );
		}
		if( page_ < pages )
		{
			addPageButton( 270, 342, 4005, 4007, page_+1 );
			addHtmlGump( 305, 345, 150, 18, htmlmask.arg( tr("NEXT PAGE") ) );
		}

	}
}

cMakeMenuGump::cMakeMenuGump( cMakeAction* action, cUOSocket* socket )
{
	cChar* pChar = socket->player();
	action_ = action;
	menu_ = action->baseMenu();
	prev_ = action->baseMenu();

	setY( 50 );
	setX( 50 );

	QString htmlmask = "<body text=\"#FFFFFF\">%1</body>";
	QString htmlmaskcenter = "<body text=\"#FFFFFF\"><div align=\"center\">%1</div></body>";

	startPage();
	addResizeGump( 0, 0, 5054, 530, 417 );
	addTiledGump( 10, 10, 510, 22, 2624, -1 );
	addTiledGump( 165,	37, 355, 88, 2624, -1 );
	addTiledGump( 165, 130, 355, 80, 2624, -1 );
	addTiledGump( 165, 215, 355, 80, 2624, -1 );
	addTiledGump( 165, 300, 355, 80, 2624, -1 );
	addTiledGump( 10, 385, 510, 22, 2624, -1 );
	addTiledGump( 10,  37, 150, 88, 2624, -1 );
	addTiledGump( 10, 130, 150, 22, 2624, -1 );
	addTiledGump( 10, 215, 150, 22, 2624, -1 );
	addTiledGump( 10, 300, 150, 22, 2624, -1 );
	addCheckertrans( 10, 10, 510, 397 );

	addHtmlGump( 10, 12, 510, 20, htmlmaskcenter.arg( menu_->name() ) ); 
	if( action->model() > 0 )
		addTilePic( 15, 42, action->model() );
	addHtmlGump( 10, 132, 150, 20, htmlmaskcenter.arg( tr("SKILLS") ) );
	addHtmlGump( 10, 217, 150, 20, htmlmaskcenter.arg( tr("MATERIALS") ) );
	addHtmlGump( 10, 302, 150, 20, htmlmaskcenter.arg( tr("OTHER") ) );

	if( !action->description().isNull() )
		addHtmlGump( 170, 302, 345, 76, htmlmask.arg( action->description() ), false, true );

	addHtmlGump( 170, 39, 70, 20, htmlmask.arg( tr("ITEM") ) );

	addButton( 15, 387, 0xFAE, 0xFB0, -1 );
	addHtmlGump( 50, 389, 80, 18, htmlmask.arg( tr("BACK") ) );

	UINT32 page = 1;
	UINT32 button = 1;
	std::vector< cMakeSection* > sections = action->makesections();
	std::vector< cMakeSection* >::iterator next = sections.begin();
	next++;
	std::vector< cMakeSection* >::iterator it = sections.begin();
	cItem* pBackpack = NULL;
	if( pChar )
		 pBackpack = FindItemBySerial( pChar->packitem );

	while( it != sections.end() )
	{
		QString content;
		startPage( page );

		if( !pChar || !pBackpack || !(*it)->hasEnough( pBackpack ) || (*it)->skilledEnough( pChar ) )
		{
			it++;
			next++;
			button++;
			continue;
		}

		addHtmlGump( 245, 39, 270, 20, htmlmask.arg( QString("%1 %2").arg(action->name()).arg((*it)->name()) ) );

		if( sections.size() > 1 )
		{
			content = "";
			addPageButton( 135, 387, 4005, 4007, (page < sections.size()) ? (page+1) : 1 );
			if( next != sections.end() )
				content = (*next)->name();
			else
				content = (*(sections.begin()))->name();

			if( content.isEmpty() )
				content = tr("NEXT");
			addHtmlGump( 170, 389, 200, 18, htmlmask.arg( content.upper() ) );
		}

		addButton( 375, 387, 4005, 4007, button );
		addHtmlGump( 410, 389, 95, 18, htmlmask.arg( tr("MAKE NOW") ) );

		content = "";
		QPtrList< cSkillCheck > skillchecks = (*it)->skillchecks();
		QPtrListIterator< cSkillCheck > sit( skillchecks );
		while( sit.current() )
		{
			content += QString("%2% %1<br>").arg( skillname[ sit.current()->skillid() ] ).arg( QString::number( (double)sit.current()->min() / 10.0f, 'f', 1 ).lower() );
			++sit;
		}
		content = htmlmask.arg( content );
		addHtmlGump( 170, 132, 345, 76, content, false, (skillchecks.count() > 4) );

		content = "";
		QPtrList< cUseItem > useitems = (*it)->useitems();
		QPtrListIterator< cUseItem > uit( useitems );
		while( uit.current() )
		{
			content += QString("%2 %1<br>").arg( uit.current()->name() ).arg( uit.current()->amount() );
			++uit;
		}
		content = htmlmask.arg( content );
		addHtmlGump( 170, 217, 345, 76, content, false, (useitems.count() > 4) );


		page++;
		button++;
		it++;
		next++;
	}
	// not enough resources or not skilled enough
	if( page == 1 )
	{
		addHtmlGump( 245, 39, 270, 20, htmlmask.arg( QString("%1").arg(action->name()) ) );
	}
}

void cMakeMenuGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 || !socket || !menu_ )
		return;

	if( !cAllMakeMenus::getInstance()->contains( menu_ ) )
	{
		socket->sysMessage( tr("Menu lost because of script reload. Try again please!") );
		return;
	}

	std::vector< cMakeMenu* > submenus = menu_->subMenus();
	std::vector< cMakeAction* > actions = menu_->actions();

	if( choice.button == -1 && prev_ && cAllMakeMenus::getInstance()->contains( prev_ ) )
		socket->send( new cMakeMenuGump( prev_, socket ) );
	else if( action_ ) // we have response of a detail menu
	{
		std::vector< cMakeSection* > sections = action_->makesections();
		if( choice.button <= sections.size() )
		{
			cMakeSection* section = sections[ choice.button-1 ];
			cMakeMenu* basemenu = menu_->baseMenu();
			if( socket->player() )
				socket->player()->setLastSection( basemenu, section );
			section->execute( socket );
		}
	}
	else if( choice.button == 1 )
	{
		cChar* pChar = socket->player();
		if( !pChar )
			return;
		QPtrList< cMakeSection > sections = pChar->lastSelections( menu_->baseMenu() );
		socket->send( new cLastTenGump( sections, menu_ ) ); 
	}
	else if( choice.button == 2 )
	{
		cChar* pChar = socket->player();
		if( !pChar )
			return;
		if( pChar->lastSection( menu_->baseMenu() ) )
			pChar->lastSection( menu_->baseMenu() )->execute( socket );
	}
	else if( choice.button < submenus.size()+3 )
	{
		socket->send( new cMakeMenuGump( submenus[ choice.button-3 ], socket ) );
	}
	else if( choice.button < (actions.size()+submenus.size()+4) )
	{
		cChar* pChar = socket->player();
		if( !pChar )
			return;
		cItem* pBackpack = FindItemBySerial( pChar->packitem );
		std::vector< cMakeSection* > sections = actions[ choice.button - submenus.size() - 4 ]->makesections();
		if( sections[0]->hasEnough( pBackpack ) )
		{
			cMakeAction* action = actions[ choice.button - submenus.size() - 4 ];
			cMakeSection* section = action->makesections()[0];
			cMakeMenu* basemenu = menu_->baseMenu();
			pChar->setLastSection( basemenu, section );
			section->execute( socket );
		}
		else
			socket->send( new cMakeMenuGump( menu_, socket, tr("You do not have enough resources to make this item") ) );
	}
	else if( choice.button < (actions.size()+submenus.size()+4+1000) )
	{
		cChar* pChar = socket->player();
		if( !pChar )
			return;
		cMakeAction* action = actions[ choice.button - submenus.size() - 4 - 1000 ];
		std::vector< UINT32 > switches = choice.switches;
		std::vector< UINT32 >::iterator it = switches.begin();
		cMakeSection* section = action->makesections()[0];
		cMakeMenu* basemenu = menu_->baseMenu();
		while( it != switches.end() )
		{
			if( (*it) == 1 )
				pChar->setLastSection( basemenu, section );
			it++;
		}
		socket->send( new cMakeMenuGump( action, socket ) );
	}
}

cLastTenGump::cLastTenGump( QPtrList< cMakeSection > sections, cMakeMenu* prev, QString notices )
{
	QString htmlmask = "<body text=\"#FFFFFF\">%1</body>";
	QString htmlmaskcenter = "<body text=\"#FFFFFF\"><div align=\"center\">%1</div></body>";

	sections_ = sections;
	prev_ = prev;

	setX( 50 );
	setY( 50 );

	startPage();
	addResizeGump( 0, 0, 0x13BE, 530, 417 );
	addTiledGump( 10, 10, 510, 22, 0xA40, -1 );
	addTiledGump( 10,292, 150, 45, 0xA40, -1 );
	addTiledGump( 165, 292, 355, 45, 0xA40, -1 );
	addTiledGump( 10, 342, 510, 65, 0xA40, -1 );
	addTiledGump( 10, 37, 200, 250, 0xA40, -1 );
	addTiledGump( 215, 37, 305, 250, 0xA40, -1 );
	addCheckertrans( 10, 10, 510, 397 );

	addHtmlGump( 10, 12, 510, 20, htmlmaskcenter.arg( tr("LAST TEN") ) ); 
	addHtmlGump( 10, 39, 200, 20, htmlmaskcenter.arg( tr("CATEGORIES") ) ); 
	addHtmlGump( 215, 39, 305, 20, htmlmaskcenter.arg( tr("SELECTIONS") ) ); 
	addHtmlGump( 10, 294, 150, 20, htmlmaskcenter.arg( tr("NOTICES") ) );

	if( !notices.isEmpty() )
		addHtmlGump( 170, 292, 345, 41, htmlmask.arg( notices ) );

	// EXIT button , return value: 0
	addButton( 15, 382, 0xFB1, 0xFB3, 0 );
	addHtmlGump( 50, 385, 150, 18, htmlmask.arg( tr("EXIT") ) );

	// PREVIOUS MENU button, return value: -1
	if( prev_ )
	{
		addButton( 15, 362, 0xFAE, 0xFB0, -1 );
		addHtmlGump( 50, 365, 150, 18, htmlmask.arg( tr("PREVIOUS MENU") ) );
	}

	QPtrListIterator< cMakeSection > it( sections );

	startPage( 1 );
	UINT32 yoffset = 60;
	UINT32 i = 1;
	while( it.current() )
	{
		if( it.current()->baseAction() && it.current()->baseAction()->baseMenu() )
		{
			// category buttons, return values: 1 -> 10
			addButton( 15, yoffset, 4005, 4007, i );
			addHtmlGump( 50, yoffset+3, 150, 18, htmlmask.arg( it.current()->baseAction()->baseMenu()->name() ) );
		}
		// selection buttons, return values: 11 -> 20
		addButton( 220, yoffset, 4005, 4007, i+10 );
		// detail buttons, return values: 21 -> 30
		addButton( 480, yoffset, 4011, 4012, i+20 );
		addHtmlGump( 255, yoffset+3, 295, 18, htmlmask.arg( QString("%1 %2").arg(it.current()->baseAction()->name()).arg(it.current()->name()) ) );
		yoffset += 20;
		i++;
		++it;
	}
}

void cLastTenGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 || !socket )
		return;

	// script reload will lose any object, so it is enough to search for the prev menu.
	if( !cAllMakeMenus::getInstance()->contains( prev_ ) )
	{
		socket->sysMessage( tr("Menu lost because of script reload. Try again please!") );
		return;
	}

	if( choice.button == -1 && prev_ )
		socket->send( new cMakeMenuGump( prev_, socket ) );
	else if( choice.button > 0 && choice.button <= 10 )
	{
		cMakeSection* section = sections_.at( choice.button-1 );
		socket->send( new cMakeMenuGump( section->baseAction()->baseMenu(), socket ) );
	}
	else if( choice.button > 10 && choice.button <= 20 )
	{
		cMakeSection* section = sections_.at( choice.button-11 );
		cChar* pChar = socket->player();
		if( !pChar )
			return;
		cItem* pBackpack = FindItemBySerial( pChar->packitem );
		if( section && section->hasEnough( pBackpack ) )
		{
			section->execute( socket );
		}
		else
			socket->send( new cLastTenGump( sections_, prev_, tr("You do not have enough resources to make this item") ) );
	}
	else if( choice.button > 20 && choice.button <= 30 )
	{
		cMakeSection* section = sections_.at( choice.button-21 );
		cMakeAction* action = section->baseAction();
		socket->send( new cMakeMenuGump( action, socket ) ); 
	}
}

void cAllMakeMenus::load()
{
	QStringList sections = DefManager->getSections( WPDT_MENU );
	QStringList::const_iterator it = sections.begin();
	while( it != sections.end() )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_MENU, (*it) );
		if( !DefSection->isNull() )
		{
			cMakeMenu* pMakeMenu = new cMakeMenu( *DefSection );
			if( pMakeMenu )
			{
				menus_.insert( make_pair< QString, cMakeMenu* >( (*it), pMakeMenu ) );
			}
		}
		it++;
	}
}

void cAllMakeMenus::reload()
{
	std::map< QString, cMakeMenu* >::iterator iter = menus_.begin();
	while( iter != menus_.end() )
	{
		delete iter->second;
		iter++;
	}
	menus_.clear();

	AllCharsIterator iterChars;
	for (iterChars.Begin(); !iterChars.atEnd(); iterChars++)
	{
		P_CHAR pc = iterChars.GetData();
		if ( pc )
			pc->clearLastSelections();
	}
	load();
}



