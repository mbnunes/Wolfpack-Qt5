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

cMakeAction::cMakeAction( const QDomElement &Tag )
{
	name_ = Tag.attribute( "id" );
	description_ = (char*)0;
	failmsg_ = (char*) 0;
	succmsg_ = (char*) 0;
	charaction_ = 0;
	sound_ = 0;
	if( Tag.hasAttribute( "inherit" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_MENU, Tag.attribute( "inherit" ) );
		applyDefinition( *DefSection );
	}
	applyDefinition( Tag );
}

void cMakeAction::processNode( const QDomElement &Tag )
{
	// CAUTION: the base tag attributes are evaluated in the constructor!!
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	if( TagName == "useitem" )
	{
		useitemprops_st useitem;
		useitem.id = hex2dec( Tag.attribute( "id" ) ).toUShort();
		useitem.amount = hex2dec( Tag.attribute( "amount" ) ).toUShort();
		useitem.name = Tag.attribute( "name" );
		if( Tag.attributes().contains( "color" ) )
		{
			QString color = Tag.attribute( "color" );
			if( color.contains( "-" ) )
			{
				QStringList colors = QStringList::split( "-", color );
				useitem.colormin = hex2dec( colors[0] ).toUShort();
				useitem.colormax = hex2dec( colors[1] ).toUShort();
			}
			else
				useitem.colormin = hex2dec( color ).toUShort();
		}

		if( useitem.amount == 0 )
			useitem.amount = 1;

		if( useitem.id > 0 )
			useitems_.push_back( useitem );
	}

	else if( TagName == "makeitem" )
	{
		makeitemprops_st makeitem;
		makeitem.name = Tag.attribute( "name" );
		makeitem.section = Tag.attribute( "id" );
		makeitem.model = hex2dec( Tag.attribute( "model" ) ).toUShort();
		makeitem.color = hex2dec( Tag.attribute( "color" ) ).toUShort();
		makeitem.amount = hex2dec( Tag.attribute( "amount" ) ).toUShort();
		
		if( Tag.hasChildNodes() )
		{
			QDomNode childNode = Tag.firstChild();
			while( !childNode.isNull() )
			{
				if( childNode.isElement() )
				{
					QDomElement childTag = childNode.toElement();
					QString chTagName = childTag.nodeName();
					QString chValue = getNodeValue( childTag );

					if( chTagName == "color" )
						makeitem.color = Value.toUShort();
					else if( chTagName == "amount" )
						makeitem.amount = Value.toUShort();
				}
				childNode = childNode.nextSibling();
			}
		}

		makeitems_.push_back( makeitem );
	}

	else if( TagName == "makenpc" )
	{
		makenpc_.name = Tag.attribute( "name" );
		makenpc_.section = Tag.attribute( "id" );
		if( Tag.attributes().contains( "model" ) )
			makenpc_.model = hex2dec( Tag.attribute( "model" ) ).toUShort();
		else
			makenpc_.model = creatures[ hex2dec( Tag.attribute( "body" ) ).toUShort() ].icon;
	}

	else if( TagName == "skill" )
	{
		skillprops_st skill;
		skill.skillid = hex2dec( Tag.attribute( "id" ) ).toUShort();
		skill.min = hex2dec( Tag.attribute( "min" ) ).toUShort();
		skill.max = hex2dec( Tag.attribute( "max" ) ).toUShort();

		skills_.push_back( skill );
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
}

void cMakeAction::execute( cUOSocket* socket )
{
	P_CHAR pChar = socket->player();
	P_ITEM pBackpack = FindItemBySerial( pChar->packitem );

	if( !socket || !pChar )
		return;

	// first check for necessary items
	bool hasEnough = true;
	std::vector< useitemprops_st >::iterator uiit = useitems_.begin();
	while( uiit != useitems_.end() && hasEnough )
	{
		// this one is in here because a GM doesnt need a backpack if he uses the add menu!
		if( !pBackpack )
			hasEnough = false;

		// the next loop will search for a the item in a range of colors.
		// it is a do-while, cause it shall run once through the loop if
		// colormin holds the one color and colormax == 0!
		UINT16 color = (*uiit).colormin;
		UINT16 amount = 0;
		do
		{
			amount += pBackpack->CountItems( (*uiit).id, color );
			++color;
		} while( color <= (*uiit).colormax );

		if( amount < (*uiit).amount )
			hasEnough = false;

		uiit++;
	}

	if( !hasEnough )
	{
		socket->sysMessage( tr("You do not have enough resources to create this!") );
		return;
	}

	// now we have to use the resources up!
	uiit = useitems_.begin();
	while( uiit != useitems_.end() && hasEnough )
	{
		P_ITEM pBackpack = FindItemBySerial( pChar->packitem );
		if( !pBackpack )
			hasEnough = false;

		// the next loop will use the items up in a range of colors.
		// it is a do-while, cause it shall run once through the loop if
		// colormin holds the one color and colormax == 0!
		UINT16 color = (*uiit).colormin;
		UINT16 amount = (*uiit).amount;
		UINT16 curramount = 0;
		do
		{
			// remove all available items or just the amount thats left
			curramount = pBackpack->CountItems( (*uiit).id, color );
			if( curramount > amount )
				curramount = amount;
			pBackpack->DeleteAmount( curramount, (*uiit).id, color );

			amount -= curramount;
			++color;
		} while( color <= (*uiit).colormax && amount > 0 );

		if( amount < (*uiit).amount )
			hasEnough = false;

		uiit++;
	}

	// then lets check the skill and calculate the item rank!
	bool hasSuccess = true;
	UINT32 ranksum = 0;
	std::vector< skillprops_st >::iterator skit = skills_.begin();
	while( skit != skills_.end() && hasSuccess )
	{
		if( !Skills->CheckSkill( pChar, (*skit).skillid, (*skit).min, (*skit).max ) )
			hasSuccess = false;
		else
		{
			UINT16 skill = pChar->skill( (*skit).skillid );
			if( skill > (*skit).max )
				skill = (*skit).max;
			if( skill < (*skit).min )
				skill = (*skit).min;

			ranksum += (UINT16)( ceil( (double)skill / (double)((*skit).max - (*skit).min) * 10.0f ) );
		}
		skit++;
	}

	if( !hasSuccess )
	{
		if( !failmsg_.isNull() )
		socket->sysMessage( failmsg_ );
		return;
	}
	else if( !succmsg_.isNull() )
		socket->sysMessage( succmsg_ );

	// lets calculate the rank for the item now
	UINT16 rank;
	if( skills_.size() == 0 )
		rank = 10;
	else
		rank = (UINT16)(ceil( ranksum / skills_.size() ));
	if( rank > 10 )
		rank = 10;
	if( rank < 1 )
		rank = 1;

	// finally lets create the items/npcs!

	// items:
	std::vector< makeitemprops_st >::iterator miit = makeitems_.begin();
	while( miit != makeitems_.end() )
	{
		P_ITEM pItem = Items->createScriptItem( (*miit).section );
		UINT16 minhp = 0;
		UINT16 maxhp = 0;
		if( pItem )
		{
			minhp = (UINT16)floor( (double)(rank-1) * 10.0f / 100.0f * (double)pItem->hp() );
			maxhp = (UINT16)floor( (double)(rank+1) * 10.0f / 100.0f * (double)pItem->hp() );
			if( pItem->isPileable() )
				pItem->setAmount( (*miit).amount );
			pItem->setColor( (*miit).color );
			pItem->setHp( RandomNum( minhp, maxhp ) );
			pBackpack->AddItem( pItem );
		}

		// if the item is not pileable create amount-1 items more
		if( pItem && !pItem->isPileable() )
		{
			for( UINT16 i = 1; i < (*miit).amount; i++ )
			{
				pItem = Items->createScriptItem( (*miit).section );
				if( pItem )
				{
					pItem->setColor( (*miit).color );
					pItem->setHp( RandomNum( minhp, maxhp ) );
					pBackpack->AddItem( pItem );
				}
			}
		}
		miit++;
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
	type_ = 0;
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

cMakeMenuGump::cMakeMenuGump( cMakeMenu* menu )
{
	menu_ = menu;
	setY( 40 );

	startPage();
	addBackground( 0xE10, 640, 480 ); //Background
	if( menu->subMenus().size() > 0 && menu->actions().size() > 0 ) // submenus and actions in one gump!
	{
		addResizeGump( 425, 15, 0xE10, 200, 405 );
		addCheckertrans( 15, 15, 410, 450 );
		addCheckertrans( 440, 30, 170, 375 );
		addCheckertrans( 425, 420, 200, 45 );
	}
	else
	{
		addCheckertrans( 15, 15, 610, 450 );
	}
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
		std::vector< cMakeMenu* > submenus = menu->subMenus();
		std::vector< cMakeMenu* >::iterator mit = submenus.begin();

		UINT32 i = 0;

		while( mit != submenus.end() )
		{
			addButton( 440, 30 + i * 22, 0xFA5, 0xFA7, i+1 );
			addText( 470, 30 + i * 22, (*mit)->name(), 0x530 );
			mit++;
			i++;
		}

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
			pagemod = 2;
			linemod = 180;
		}
		else
		{
			pagemod = 6;
			linemod = 40;
		}

		UINT32 page_;
		UINT32 pages = ((UINT32)ceil( (double)actions.size() / (double)pagemod ));

		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 right = page_ * pagemod - 1;
			UINT32 left = page_ * pagemod - pagemod;
			if( actions.size() <= right )
				right = actions.size()-1;

			i = left;
			while( i <= right )
			{
				addButton( 30, 80 + (menu_->type() == 1 ? 10 : 0) + (i-left) * linemod, 0xFA5, 0xFA7, submenus.size()+i+1 );
				if( actModels[i] > 0 )
					addTilePic( 70, 80 + (i-left) * linemod, actModels[i] );
				addText( 120, 80 + (i-left) * linemod, QString("%1").arg( actNames[i] ), 0x530 );
				addText( 120, 100 + (i-left) * linemod, QString("%1").arg( actDescr[i] ), 0x834 );

				if( menu_->type() == 0 )
				{
					addText(  30, 120 + (i-left) * linemod, tr("Resources"), 0x530 );
					addText( 230, 120 + (i-left) * linemod, tr("Crafting results"), 0x530 );

					QString content;
					std::vector< cMakeAction::useitemprops_st > useitems = actions[i]->useitems();
					std::vector< cMakeAction::useitemprops_st >::iterator uiit = useitems.begin();

					while( uiit != useitems.end() )
					{
						content += QString( "%1x %2<br>" ).arg( (*uiit).amount ).arg( (*uiit).name );
						uiit++;
					}
					addHtmlGump( 30, 140 + (i-left) * linemod, 180, 70, content, false, true );

					content = "";
					std::vector< cMakeAction::makeitemprops_st > makeitems = actions[i]->makeitems();
					std::vector< cMakeAction::makeitemprops_st >::iterator miit = makeitems.begin();

					while( miit != makeitems.end() )
					{
						content += QString( "%1x %2<br>" ).arg( (*miit).amount ).arg( (*miit).name );
						miit++;
					}
					if( !actions[i]->makenpc().name.isEmpty() )
						content += tr( "one %1" ).arg( actions[i]->makenpc().name );

					addHtmlGump( 230, 140 + (i-left) * linemod, 180, 70, content, false, true );

					std::vector< cMakeAction::skillprops_st > skills = actions[i]->skills();
					std::vector< cMakeAction::skillprops_st >::iterator skit = skills.begin();

					if( skills.size() == 0 )
						content="Checks no skill";
					else
						content="Checks skill";

					if( skills.size() > 1 )
						content += "s";
					if( skit != skills.end() )
					{
						content += QString(" %1").arg( skillname[ (*skit).skillid ] ).lower();
						skit++;
					}
					while( skit != skills.end() )
					{
						content += QString(", %1").arg( skillname[ (*skit).skillid ] ).lower();
						skit++;
					}
					content += ".";

					addCroppedText( 30, 210 + (i-left) * linemod, 410, 40, content, 0x834 );

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
		UINT32 pages = ((UINT32)floor( submenus.size() / 6 ))+1;
		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 i;
			UINT32 right = page_ * 6 - 1;
			UINT32 left = page_ * 6 - 6;
			if( submenus.size() < right )
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
			pagemod = 2;
			linemod = 180;
		}
		else
		{
			pagemod = 6;
			linemod = 40;
		}

		UINT32 i;
		UINT32 page_;
		UINT32 pages = ((UINT32)ceil( (double)actions.size() / (double)pagemod ));

		for( page_ = 1; page_ <= pages; page_++ )
		{
			startPage( page_ );

			UINT32 right = page_ * pagemod - 1;
			UINT32 left = page_ * pagemod - pagemod;
			if( actions.size() <= right )
				right = actions.size()-1;

			i = left;
			while( i <= right )
			{
				addButton( 30, 80 + (menu_->type() == 1 ? 10 : 0) + (i-left) * linemod, 0xFA5, 0xFA7, i+1 );
				if( actModels[i] > 0 )
					addTilePic( 70, 80 + (i-left) * linemod, actModels[i] );
				addText( 120, 80 + (i-left) * linemod, QString("%1").arg( actNames[i] ), 0x530 );
				addText( 120, 100 + (i-left) * linemod, QString("%1").arg( actDescr[i] ), 0x834 );

				if( menu_->type() == 0 )
				{
					addText(  30, 120 + (i-left) * linemod, tr("Resources"), 0x530 );
					addText( 330, 120 + (i-left) * linemod, tr("Crafting results"), 0x530 );
	
					QString content;
					std::vector< cMakeAction::useitemprops_st > useitems = actions[i]->useitems();
					std::vector< cMakeAction::useitemprops_st >::iterator uiit = useitems.begin();

					while( uiit != useitems.end() )
					{
						content += QString( "%1x %2<br>" ).arg( (*uiit).amount ).arg( (*uiit).name );
						uiit++;
					}
					addHtmlGump( 30, 140 + (i-left) * linemod, 280, 70, content, false, true );

					content = "";
					std::vector< cMakeAction::makeitemprops_st > makeitems = actions[i]->makeitems();
					std::vector< cMakeAction::makeitemprops_st >::iterator miit = makeitems.begin();

					while( miit != makeitems.end() )
					{
						content += QString( "%1x %2<br>" ).arg( (*miit).amount ).arg( (*miit).name );
						miit++;
					}
					if( !actions[i]->makenpc().name.isEmpty() )
						content += tr( "one %1" ).arg( actions[i]->makenpc().name );
	
					addHtmlGump( 330, 140 + (i-left) * linemod, 280, 70, content, false, true );
	
					std::vector< cMakeAction::skillprops_st > skills = actions[i]->skills();
					std::vector< cMakeAction::skillprops_st >::iterator skit = skills.begin();

					if( skills.size() == 0 )
						content="Checks no skill";
					else
						content="Checks skill";

					if( skills.size() > 1 )
						content += tr("s");
					if( skit != skills.end() )
					{
						content += QString(" %1").arg( skillname[ (*skit).skillid ] ).lower();
						skit++;
					}
					while( skit != skills.end() )
					{
						content += QString(", %1").arg( skillname[ (*skit).skillid ] ).lower();
						skit++;
					}
					content += ".";

					addCroppedText( 30, 210 + (i-left) * linemod, 610, 40, content, 0x834 );
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

void cMakeMenuGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 || !socket || !menu_ )
		return;

	if( !cAllMakeMenus::getInstance()->contains( menu_ ) )
	{
		socket->sysMessage( tr("Menu lost because of script reload. Try again please!") );
		return;
	}

	if( choice.button == -1 && menu_->prevMenu() && cAllMakeMenus::getInstance()->contains( menu_->prevMenu() ) )
		socket->send( new cMakeMenuGump( menu_->prevMenu() ) );

	std::vector< cMakeMenu* > submenus = menu_->subMenus();
	std::vector< cMakeAction* > actions = menu_->actions();

	if( (choice.button-1) < submenus.size() )
	{
		socket->send( new cMakeMenuGump( submenus[ choice.button-1 ] ) );
	}
	else if( (choice.button-1-submenus.size()) < actions.size() )
	{
		actions[ (choice.button-1-submenus.size()) ]->execute( socket );
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



