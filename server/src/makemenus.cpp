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

cMakeSection::cMakeSection( const QDomElement &Tag )
{
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

cMakeAction::cMakeAction( const QDomElement &Tag )
{
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
		cMakeSection* pMakeSection = new cMakeSection( Tag );
		if( pMakeSection )
			makesections_.push_back( pMakeSection );
	}

	else if( TagName == "makenpc" )
	{
		makenpc_.name = Tag.attribute( "name" );
		makenpc_.section = Tag.attribute( "id" );
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

	P_CHAR pChar = socket->player();
	P_ITEM pBackpack = FindItemBySerial( pChar->packitem );

	if( !socket || !pChar )
		return;

	// first check for necessary items
	if( !pSection->hasEnough( pBackpack ) )
	{
		socket->sysMessage( tr("You do not have enough resources to create this!") );
		return;
	}

	// now we have to use the resources up!
	pSection->useResources( pBackpack );

	// then lets check the skill and calculate the item rank!
	UINT32 ranksum = pSection->calcRank( pChar );
	if( ranksum == 0 )
	{
		if( !failmsg_.isNull() )
		socket->sysMessage( failmsg_ );
		return;
	}
	else if( !succmsg_.isNull() )
		socket->sysMessage( succmsg_ );

	// lets calculate the rank for the item now
	UINT16 rank;
	if( pSection->skillchecks().count() == 0 )
		rank = 10;
	else
		rank = (UINT16)(ceil( ranksum / pSection->skillchecks().count() ));
	if( rank > 10 )
		rank = 10;
	if( rank < 1 )
		rank = 1;

	// finally lets create the items/npcs!

	// items:
	QPtrList< cMakeItem > makeitems = pSection->makeitems();
	QPtrListIterator< cMakeItem > miit( makeitems );
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

	if( charaction_ > 0 )
		pChar->action( charaction_ );

	if( sound_ > 0 )
		pChar->soundEffect( sound_ );

	// TODO rank messages
}

cMakeMenu::cMakeMenu( const QDomElement &Tag, cMakeMenu* previous )
{
	name_ = Tag.attribute( "id" );
	description_ = (char*)0;
	type_ = 1; // defaults to compact
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
		cMakeAction* pMakeAction = new cMakeAction( Tag );
		actions_.push_back( pMakeAction );
	}

	else if( TagName == "description" )
	{
		description_ = Value;
	}

	else if( TagName == "name" )
		name_ = Value;

	else if( TagName == "model" )
		model_ = Value.toUShort();

	else if( TagName == "type" )
	{
		if( Value == "detail" )
			type_ = 0;
		else if( Value == "compact" )
			type_ = 1;
		else
			type_ = Value.toUShort();
	}
}

void cMakeMenuGump::buildSection( UINT32 x, UINT32 y, UINT32 width, cMakeSection* makesection )
{
	if( !makesection )
		return;
	QString content;

	QPtrList< cUseItem > useitems = makesection->useitems();
	QPtrListIterator< cUseItem > uiit( useitems );

	if( useitems.count() == 0 )
		content="use no resources";
	else
		content="use";

	if( uiit.current() )
	{
		content += QString( " %1x %2" ).arg( uiit.current()->amount() ).arg( uiit.current()->name() );
		++uiit;
	}
	while( uiit.current() )
	{
		content += QString( ", %1x %2" ).arg( uiit.current()->amount() ).arg( uiit.current()->name() );
		++uiit;
	}
	
	QPtrList< cMakeItem > makeitems = makesection->makeitems();
	QPtrListIterator< cMakeItem > miit( makeitems );

	if( makeitems.count() == 0 )
		content+=" to craft nothing";
	else
		content+=" to craft";

	if( miit.current() )
	{
		content += QString( " %1x %2" ).arg( miit.current()->amount() ).arg( miit.current()->name() );
		++miit;
	}
	while( miit.current() )
	{
		content += QString( ", %1x %2" ).arg( miit.current()->amount() ).arg( miit.current()->name() );
		++miit;
	}
	content += ".";
	addText( x, y, content, 0x530 );

	content = "";
	QPtrList< cSkillCheck > skills = makesection->skillchecks();
	QPtrListIterator< cSkillCheck > skit( skills );

	if( skills.count() == 0 )
		content="checks no skill";
	else
		content="checks skill";
	if( skills.count() > 1 )
		content += "s";

	if( skit.current() )
	{
		content += QString(" %1(%2)").arg( skillname[ skit.current()->skillid() ] ).arg( QString::number( (double)skit.current()->min()/10.0f,'f',1 ) ).lower();
		++skit;
	}
	while( skit.current() )
	{
		content += QString(", %1(%2)").arg( skillname[ skit.current()->skillid() ] ).arg( QString::number( (double)skit.current()->min()/10.0f,'f',1 ) ).lower();
		++skit;
	}
	content += ".";
	addText( x, y+20, content, 0x834 );
}

void cMakeMenuGump::buildSubMenus( UINT32 x, UINT32 y, UINT32 width, UINT32 height )
{
	addResizeGump( x, y, 0xE10, width, height );
	addCheckertrans( x+15, y+15, width-30, height-30 );

	addText( x+25, y+15, tr( "Categories:" ), 0x834 );
	std::vector< cMakeMenu* > submenus = menu_->subMenus();
	std::vector< cMakeMenu* >::iterator mit = submenus.begin();

	UINT32 i = 0;

	while( mit != submenus.end() )
	{
		addButton( x+15, y+35 + i * 22, 0xFA5, 0xFA7, i+1 );
		addText( x+55, y+35 + i * 22, (*mit)->name(), 0x530 );
		mit++;
		i++;
	}
}

cMakeMenuGump::cMakeMenuGump( cMakeMenu* menu )
{
	menu_ = menu;
	prev_ = menu->prevMenu();
	setY( 40 );

	startPage();
	addBackground( 0xE10, 640, 480 ); //Background
	addCheckertrans( 15, 15, 610, 450 );

	if( menu->model() > 0 )
	{
		addTilePic( 20, 20, menu->model() );
		addText( 70, 20, (menu->name().isEmpty() ? tr("Make Menu") : menu->name()), 0x530 );
		addText( 70, 40, menu->description(), 0x834 );
	}
	else
	{
		addText( 20, 20, (menu->name().isEmpty() ? tr("Make Menu") : menu->name()), 0x530 );
		addText( 20, 40, menu->description(), 0x834 );
	}
	
	// X button
	addText( 70, 440, "Close", 0x834 );
	addButton( 30, 440, 0xFB1, 0xFB3, 0 ); 
	if( menu_->prevMenu() )
	{
		addText( 220, 440, "Previous", 0x834 );
		addButton( 180, 440, 0xFAE, 0xFB0, -1 ); 
	}

	if( menu->subMenus().size() > 0 && menu->actions().size() > 0 ) // submenus and actions in one gump!
	{
		buildSubMenus( 425, 15, 200, 405 );

		QStringList actNames;
		QStringList actDescr;
		std::vector< UINT16 > actModels;
		std::vector< cMakeAction* > actions = menu->actions();
		std::vector< cMakeAction* >::iterator it = actions.begin();

		while( it != actions.end() )
		{
			actNames.push_back( (*it)->name() );
			actDescr.push_back( (*it)->description() );
			actModels.push_back( (*it)->model() );
			it++;
		}
		
		UINT32 pagemod, linemod;
		if( menu_->type() == 0 )
		{
			pagemod = 1;
			linemod = 50;
		}
		else
		{
			pagemod = 6;
			linemod = 40;
		}

		UINT32 page_;
		UINT32 pages = ((UINT32)ceil( (double)actions.size() / (double)pagemod ));
		UINT32 boffset = 0;

		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 right = page_ * pagemod - 1;
			UINT32 left = page_ * pagemod - pagemod;
			if( actions.size() <= right )
				right = actions.size()-1;

			std::vector< cMakeMenu* > submenus = menu_->subMenus();
			UINT32 i = left;
			while( i <= right )
			{
				if( menu_->type() == 1 )
				{
					addButton( 30, 80 + (i-left) * linemod, 0xFA5, 0xFA7, submenus.size()+i*2+1 );
					addButton( 30, 100 + (i-left) * linemod, 0xFAB, 0xFAD, submenus.size()+i*2+2 ); 
				}
				if( actModels[i] > 0 )
					addTilePic( 70, 80 + (i-left) * linemod, actModels[i] );
				addText( 120, 80 + (i-left) * linemod, QString("%1").arg( actNames[i] ), 0x530 );
				addText( 120, 100 + (i-left) * linemod, QString("%1").arg( actDescr[i] ), 0x834 );

				if( menu_->type() == 0 )
				{
					UINT32 yoffset = 130;
					std::vector< cMakeSection* > sections = actions[i]->makesections();
					std::vector< cMakeSection* >::iterator sit = sections.begin();
					while( sit != sections.end() )
					{
						addButton( 30, yoffset+10, 0xFA5, 0xFA7, submenus.size()+1+boffset );
						buildSection( 70, yoffset, 490, (*sit) );
						sit++;
						boffset++;
						yoffset += linemod;
					}
				}
				i++;
			}


			addText( 440, 440, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
			if( page_ > 1 ) // previous page
				addPageButton( 400, 440, 0x0FC, 0x0FC, page_-1 );

			if( page_ < pages ) // next page
				addPageButton( 420, 440, 0x0FA, 0x0FA, page_+1 );
		}
	}
	else if( menu->subMenus().size() > 0 ) // only submenus in the menu!
	{
		addText( 30, 70, tr( "Categories:" ), 0x834 );

		QStringList subNames;
		QStringList subDescr;
		std::vector< UINT16 > subModels;
		std::vector< cMakeMenu* > submenus = menu->subMenus();
		std::vector< cMakeMenu* >::iterator it = submenus.begin();

		while( it != submenus.end() )
		{
			subNames.push_back( (*it)->name() );
			subDescr.push_back( (*it)->description() );
			subModels.push_back( (*it)->model() );
			it++;
		}
		
		UINT32 page_;
		UINT32 pages = ((UINT32)ceil( (double)submenus.size() / 6.0f ));
		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 i;
			UINT32 right = page_ * 6 - 1;
			UINT32 left = page_ * 6 - 6;
			if( submenus.size() <= right )
				right = submenus.size()-1;

			i = left;
			while( i <= right )
			{
				addButton( 30, 110 + (i-left) * 50, 0xFA5, 0xFA7, i+1 );
				if( subModels[i] > 0 )
					addTilePic( 70, 100 + (i-left) * 50, subModels[i] );
				addText( 120, 100 + (i-left) * 50, QString("%1").arg( subNames[i] ), 0x530 );
				addText( 120, 120 + (i-left) * 50, QString("%1").arg( subDescr[i] ), 0x834 );
				i++;
			}


			addText( 440, 440, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
			if( page_ > 1 ) // previous page
				addPageButton( 400, 440, 0x0FC, 0x0FC, page_-1 );

			if( page_ < pages ) // next page
				addPageButton( 420, 440, 0x0FA, 0x0FA, page_+1 );
		}
	}
	else if( menu->actions().size() > 0 ) // only actions in the menu!
	{
		QStringList actNames;
		QStringList actDescr;
		std::vector< UINT16 > actModels;
		std::vector< cMakeAction* > actions = menu->actions();
		std::vector< cMakeAction* >::iterator it = actions.begin();

		while( it != actions.end() )
		{
			actNames.push_back( (*it)->name() );
			actDescr.push_back( (*it)->description() );
			actModels.push_back( (*it)->model() );
			it++;
		}
		
		UINT32 pagemod, linemod;
		if( menu_->type() == 0 )
		{
			pagemod = 1;
			linemod = 50;
		}
		else
		{
			pagemod = 6;
			linemod = 40;
		}

		UINT32 i;
		UINT32 page_;
		UINT32 pages = ((UINT32)ceil( (double)actions.size() / (double)pagemod ));
		UINT32 boffset = 0;

		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			std::vector< cMakeMenu* > submenus = menu_->subMenus();
			UINT32 right = page_ * pagemod - 1;
			UINT32 left = page_ * pagemod - pagemod;
			if( actions.size() <= right )
				right = actions.size()-1;

			i = left;
			while( i <= right )
			{
				if( menu_->type() == 1 )
				{
					addButton( 30, 80 + (i-left) * linemod, 0xFA5, 0xFA7, submenus.size()+i*2+1 );
					addButton( 30, 100 + (i-left) * linemod, 0xFAB, 0xFAD, submenus.size()+i*2+2 ); 
				}
				if( actModels[i] > 0 )
					addTilePic( 70, 80 + (i-left) * linemod, actModels[i] );
				addText( 120, 80 + (i-left) * linemod, QString("%1").arg( actNames[i] ), 0x530 );
				addText( 120, 100 + (i-left) * linemod, QString("%1").arg( actDescr[i] ), 0x834 );

				if( menu_->type() == 0 )
				{
					UINT32 yoffset = 120;
					std::vector< cMakeSection* > sections = actions[i]->makesections();
					std::vector< cMakeSection* >::iterator sit = sections.begin();
					while( sit != sections.end() )
					{
						addButton( 30, yoffset+10, 0xFA5, 0xFA7, submenus.size()+1+boffset );
						buildSection( 70, yoffset, 580, (*sit) );
						sit++;
						boffset++;
						yoffset += linemod;
					}
				}
				i++;
			}


			addText( 440, 440, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
			if( page_ > 1 ) // previous page
				addPageButton( 400, 440, 0x0FC, 0x0FC, page_-1 );

			if( page_ < pages ) // next page
				addPageButton( 420, 440, 0x0FA, 0x0FA, page_+1 );
		}
	}
}

cMakeMenuGump::cMakeMenuGump( cMakeAction* action, cMakeMenu* previous )
{
	menu_ = previous;
	prev_ = previous;
	setY( 40 );

	startPage();
	addBackground( 0xE10, 640, 480 ); //Background
	addCheckertrans( 15, 15, 610, 450 );

	if( menu_->model() > 0 )
	{
		addTilePic( 20, 20, menu_->model() );
		addText( 70, 20, (menu_->name().isEmpty() ? tr("Make Menu") : menu_->name()), 0x530 );
		addText( 70, 40, menu_->description(), 0x834 );
	}
	else
	{
		addText( 20, 20, (menu_->name().isEmpty() ? tr("Make Menu") : menu_->name()), 0x530 );
		addText( 20, 40, menu_->description(), 0x834 );
	}
	
	// X button
	addText( 70, 440, "Close", 0x834 );
	addButton( 30, 440, 0xFB1, 0xFB3, 0 ); 
	if( prev_ )
	{
		addText( 220, 440, "Previous", 0x834 );
		addButton( 180, 440, 0xFAE, 0xFB0, -1 ); 
	}
		
	UINT32 pagemod, linemod;
	pagemod = 1;
	linemod = 50;

	UINT32 i;
	UINT32 boffset = 0;

	std::vector< cMakeAction* > actions = menu_->actions();
	std::vector< cMakeAction* >::iterator ait = actions.begin();
	while( ait != actions.end() && (*ait) != action )
	{
		boffset += (*ait)->makesections().size();
		ait++;
	}

	startPage( 1 );
			
	if( action->model() > 0 )
		addTilePic( 30, 80, action->model() );
	addText( 80, 80, QString("%1").arg( action->name() ), 0x530 );
	addText( 80, 100, QString("%1").arg( action->description() ), 0x834 );

	UINT32 yoffset = 130;
	std::vector< cMakeSection* > sections = action->makesections();
	std::vector< cMakeSection* >::iterator sit = sections.begin();
	while( sit != sections.end() )
	{
		addButton( 30, yoffset+10, 0xFA5, 0xFA7, menu_->subMenus().size()+1+boffset );
		buildSection( 70, yoffset, 490, (*sit) );
		sit++;
		boffset++;
		yoffset += linemod;
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
		socket->send( new cMakeMenuGump( prev_ ) );
	else if( (choice.button-1) < submenus.size() )
	{
		socket->send( new cMakeMenuGump( submenus[ choice.button-1 ] ) );
	}
	else if( menu_->type() == 1 && (choice.button-1-submenus.size()) < (actions.size()*2) && menu_ != prev_ ) // menu_ == prev means we have a action detail menu
	{
		if( ((choice.button-1-submenus.size())%2) == 0 ) // if its even
			actions[ (UINT32)floor( (double)(choice.button-1-submenus.size())/2.0f ) ]->execute( socket, 0 );
		else
			socket->send( new cMakeMenuGump( actions[ (UINT32)floor( (double)(choice.button-1-submenus.size())/2.0f ) ], menu_ ) );
	}
	else if( menu_->type() == 0 || menu_ == prev_ )
	{
		std::vector< cMakeAction* > actions = menu_->actions();
		std::vector< cMakeAction* >::iterator it = actions.begin();
		UINT32 boffset = 0;
		while( it != actions.end() )
		{
			std::vector< cMakeSection* > sections = (*it)->makesections();
			if( (choice.button-1-submenus.size()-boffset) < sections.size() )
				(*it)->execute( socket, (choice.button-1-submenus.size()-boffset) );
			else
				boffset += sections.size();
			it++;
		}
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

	load();
}



