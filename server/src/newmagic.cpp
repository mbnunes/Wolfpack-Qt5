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

// Library Includes
#include <qstringlist.h>
#include <qdom.h>

// Wolfpack Includes
#include "newmagic.h"
#include "chars.h"
#include "inlines.h"
#include "network/uosocket.h"
#include "srvparams.h"
#include "TmpEff.h"
#include "spellbook.h"
#include "wpdefmanager.h"
#include "wpscriptmanager.h"
#include "wpdefaultscript.h"
#include "tilecache.h"
#include "targetrequests.h"

/*!
	Returns the spell struct with the specified id
*/
stNewSpell *cNewMagic::findSpell( UINT8 id )
{
	if( id >= 64 )
		return 0;

	return &spells[id];
}

void cNewMagic::load()
{
	// Get all spell definitions
	QStringList sList = DefManager->getSections( WPDT_SPELL );

	for( UINT8 i = 0; i < sList.count(); ++i )
	{
		const QDomElement *elem = DefManager->getSection( WPDT_SPELL, sList[i] );

		UINT8 id = elem->attribute( "id", "0" ).toUShort();

		if( !id || id > 64 )
			continue;

        id--;
		QDomElement node = elem->firstChild().toElement();

		while( !node.isNull() )
		{
			if( node.nodeName() == "name" )
				spells[id].name = node.text();
			else if( node.nodeName() == "mantra" )
				spells[id].mantra = node.text();
			else if( node.nodeName() == "target" )
				spells[id].target = node.text();
			else if( node.nodeName() == "booklow" )
				spells[id].booklow = node.text().toInt();
			else if( node.nodeName() == "bookhigh" )
				spells[id].bookhigh = node.text().toInt();
			else if( node.nodeName() == "scrolllow" )
				spells[id].scrolllow = node.text().toInt();
			else if( node.nodeName() == "scrollhigh" )
				spells[id].scrollhigh = node.text().toInt();
			else if( node.nodeName() == "actiondelay" )
				spells[id].actiondelay = node.text().toInt();
			else if( node.nodeName() == "delay" )
				spells[id].delay = node.text().toInt();
			else if( node.nodeName() == "action" )
				spells[id].action = hex2dec( node.text() ).toInt();
			else if( node.nodeName() == "targets" )
			{
				// process subnodes <char />, <item />, <ground />
				if( node.elementsByTagName( "char" ).count() >= 1 )
					spells[id].targets |= TARGET_CHAR;
				if( node.elementsByTagName( "item" ).count() >= 1 )
					spells[id].targets |= TARGET_ITEM;
				if( node.elementsByTagName( "ground" ).count() >= 1 )
					spells[id].targets |= TARGET_GROUND;
			}				
			else if( node.nodeName() == "flags" )
			{
				QDomNodeList nList = node.childNodes();

				for( INT32 i = 0; i < nList.count(); ++i )
				{
					QDomElement sNode = nList.item( i ).toElement();

					if( !sNode.isNull() )
					{
						if( sNode.nodeName() == "agressive" )
							spells[id].flags |= SPELL_AGRESSIVE;
						else if( sNode.nodeName() == "reflectable" )
							spells[id].flags |= SPELL_REFLECTABLE;
					}
				}
			}
			else if( node.nodeName() == "mana" )
				spells[id].mana = node.text().toInt();
			else if( node.nodeName() == "scroll" )
				spells[id].scroll = hex2dec( node.text() ).toLong();
			else if( node.nodeName() == "script" )
				spells[id].script = ScriptManager->find( node.text() );
			else if( node.nodeName() == "reagents" )
			{
				QDomNodeList nList = node.childNodes();

				for( INT32 i = 0; i < nList.count(); ++i )
				{
					QDomElement sNode = nList.item( i ).toElement();

					if( !sNode.isNull() )
					{
						if( sNode.nodeName() == "blackpearl" )
							spells[id].reagents.blackpearl = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "bloodmoss" )
							spells[id].reagents.bloodmoss = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "garlic" )
							spells[id].reagents.garlic = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "ginseng" )
							spells[id].reagents.ginseng = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "mandrake" )
							spells[id].reagents.mandrake = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "nightshade" )
							spells[id].reagents.nightshade = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "spidersilk" )
							spells[id].reagents.spidersilk = hex2dec( sNode.text() ).toInt();
						else if( sNode.nodeName() == "sulfurash" )
							spells[id].reagents.sulfurash = hex2dec( sNode.text() ).toInt();
					}
				}
			}

			QDomNode tmp = node.nextSibling();
			if( !tmp.isNull() )
				node = tmp.toElement();
			else
				break;
		}
	}
}

void cNewMagic::unload()
{
}

cEndCasting::cEndCasting( P_CHAR _mage, UINT8 _spell, UINT8 _type, UINT32 _delay )
{
	destSer = _mage->serial();
	spell = _spell;
	type = _type;
	serializable = false;
	expiretime = uiCurrentTime + _delay;
	objectid = "endcasting";
}

void cEndCasting::Expire()
{
	P_CHAR pMage = FindCharBySerial( destSer );
	
	if( !pMage )
		return;

	pMage->setPriv2( pMage->priv2() &~ 0x02 );
	//pMage->stopRepeatedAction();

	if( !pMage->socket() )
		return;

	// Show a target cursor *or* cast directly
	pMage->socket()->attachTarget( new cSpellTarget( pMage, spell, type ) );
}

/*!
	Calculates the Spell ID for a given Scroll Item id
*/
INT8 cNewMagic::calcSpellId( UINT16 scroll )
{
	tile_st tile = TileCache::instance()->getTile( scroll );

	if( tile.unknown1 == 0 )
		return -1;
	else
		return tile.unknown1 - 1;
}

/*!
	Calculates the Scroll Item ID for the specified 
	spell id. Take care, these values are basically 
	hardcoded.
*/
UINT16 cNewMagic::calcScrollId( UINT8 spell )
{
	// There is a small "glitch" in the tiledata for this
	// So only spells > 7 are normally ordered
	if( spell >= 8 )
		return 0x1F35 + ( spell - 8 );
	
	// Decided this would be fastest
	else switch( spell )
	{
		case 0:			return 0x1F2E;
		case 1:			return 0x1F2F;
		case 2:			return 0x1F30;
		case 3:			return 0x1F31;
		case 4:			return 0x1F32;
		case 5:			return 0x1F33;
		case 6:			return 0x1F2D;
		case 7:			return 0x1F34;
	}
	return 0;
}

/*!
This ends the casting of a specified character.
If the second parameter is not false, it 
displays the fizzle animation and the fizzle sound.
*/
void cNewMagic::disturb( P_CHAR pMage, bool fizzle, INT16 chance )
{
	if( !pMage->casting() )
		return;

	pMage->setCasting( false );
	pMage->setPriv2( pMage->priv2() & 0xEF ); // Unfreeze, NOTE: This could lead into problems with a normal "wanted" freeze

	// Stop the repeating animation and the endspell thing
	//pMage->stopRepeatedAction();

	cChar::Effects effects = pMage->effects();
	for( INT32 i = 0; i < effects.size(); ++i )
	{
		if( effects[i]->objectID() == "endcasting" )
		{
			// We found our effect
			TempEffects::instance()->teffects.erase( effects[i] );
			pMage->removeEffect( effects[i] );
			break;
		}
	}
	
	if( fizzle )
	{
		pMage->effect( 0x3735, 1, 30 );
		pMage->soundEffect( 0x005C );

		if( pMage->socket() )
			pMage->socket()->sysMessage( tr( "Your spell fizzles." ) );
	}
}

/*!
	This function is used to consume the mana for a 
	specific spell. If the character does not have
	enough mana, this function returns false.
*/
bool cNewMagic::useMana( P_CHAR pMage, UINT8 spell )
{
	// The character does not need any mana
	if( pMage->priv2() & 0x10 )
		return true;

	stNewSpell *sInfo = findSpell( spell );

	if( !sInfo )
		return false;

	if( pMage->mn() < sInfo->mana )
	{
		pMage->message( tr( "You don't have enough mana to cast this spell." ) );
		return false;
	}

	pMage->setMn( pMage->mn() - sInfo->mana );
	return true;
}

/*!
	This function is used to check for the required 
	mana to cast a specific spell. If the character
	does not have enough mana, a message is displayed
	above the characters head and it returns false.
*/
bool cNewMagic::checkMana( P_CHAR pMage, UINT8 spell )
{
	stNewSpell *sInfo = findSpell( spell );
	bool enoughMana = false;

	if( sInfo && pMage->mn() >= sInfo->mana )
		enoughMana = true;

	return enoughMana;
}

#define useReagent( name, itemid ) if( name > 0 && pItem->id() == itemid ) \
		{\
			if( name >= pItem->amount() ) \
			{ \
				name -= pItem->amount(); \
				Items->DeleItem( pItem ); \
			} \
			else if( name < pItem->amount() ) \
			{ \
				pItem->ReduceAmount( name ); \
				name = 0; \
			} \
		}\

/*!
	This function uses the reagents for the specified 
	spell. It displays a message with the missing 
	reagents if needed. The returnvalue is false if 
	the character does not have enough reagents.
*/
bool cNewMagic::useReagents( P_CHAR pMage, UINT8 spell )
{
	// Check for each reagent.
	// So we dont need to loop trough all items over and over again we'll use ONE loop (will be a bit less clean)
	P_ITEM pPack = pMage->getBackpack();

	stNewSpell *sInfo = findSpell( spell );
	UINT8 ginseng = sInfo->reagents.ginseng;
	UINT8 bloodmoss = sInfo->reagents.bloodmoss;
	UINT8 mandrake = sInfo->reagents.mandrake;
	UINT8 blackpearl = sInfo->reagents.blackpearl;
	UINT8 spidersilk = sInfo->reagents.spidersilk;
	UINT8 garlic = sInfo->reagents.garlic;
	UINT8 nightshade = sInfo->reagents.nightshade;
	UINT8 sulfurash = sInfo->reagents.sulfurash;

	QPtrList< cItem > content = pPack->getContainment();

	for( P_ITEM pItem = content.first(); pItem; pItem = content.next() )
	{
		if( blackpearl + bloodmoss + garlic + ginseng + mandrake + nightshade + sulfurash + spidersilk == 0 )
			break;

		useReagent( blackpearl, 0xF7A )
		else useReagent( bloodmoss, 0xF7B )
		else useReagent( garlic, 0xF84 )
		else useReagent( ginseng, 0xF85 )
		else useReagent( mandrake, 0xF86 )
		else useReagent( nightshade, 0xF88 )
		else useReagent( sulfurash, 0xF8C )
		else useReagent( spidersilk, 0xF8D )
	}

	QStringList missing;

	if( ginseng > 0 )
		missing.append( tr( "Ginseng" ) );
	if( bloodmoss > 0 )
		missing.append( tr( "Bloodmoss" ) );
	if( mandrake > 0 )
		missing.append( tr( "Mandrake" ) );
	if( blackpearl > 0 )
		missing.append( tr( "Black Pearls" ) );
	if( spidersilk > 0 )
		missing.append( tr( "Spider's Silk" ) );
	if( garlic > 0 )
		missing.append( tr( "Garlic" ) );
	if( nightshade > 0 )
		missing.append( tr( "Nightshade" ) );
	if( sulfurash > 0 )
		missing.append( tr( "Sulfurous Ash" ) );

	bool enoughReagents = true;

	if( missing.count() > 0 )
	{
		if( pMage->socket() )
		{
			pMage->message( tr( "You don't have enough reagents." ) );
			pMage->socket()->sysMessage( tr( "You lack the following reagents: %1" ).arg( missing.join( ", ") ) );
		}
		enoughReagents = false;
	}

	return enoughReagents;
}

#define checkReagent( name, itemid )		if( name > 0 && pItem->id() == itemid ) \
		{\
			if( pItem->amount() > name ) \
				name = 0; \
			else \
				name -= pItem->amount(); \
		}\

/*!
	Just like useReagents this function is checking 
	for required reagents on a characater to cast a 
	spell. But unlike useReagents it wont really 
	consume the reagents but just return false if 
	the required reagents are not present.
*/
bool cNewMagic::checkReagents( P_CHAR pMage, UINT8 spell )
{
	// Check for each reagent.
	// So we dont need to loop trough all items over and over again we'll use ONE loop (will be a bit less clean)
	P_ITEM pPack = pMage->getBackpack();

	stNewSpell *sInfo = findSpell( spell );
	UINT8 ginseng = sInfo->reagents.ginseng;
	UINT8 bloodmoss = sInfo->reagents.bloodmoss;
	UINT8 mandrake = sInfo->reagents.mandrake;
	UINT8 blackpearl = sInfo->reagents.blackpearl;
	UINT8 spidersilk = sInfo->reagents.spidersilk;
	UINT8 garlic = sInfo->reagents.garlic;
	UINT8 nightshade = sInfo->reagents.nightshade;
	UINT8 sulfurash = sInfo->reagents.sulfurash;

	QPtrList< cItem > content = pPack->getContainment();

	for( P_ITEM pItem = content.first(); pItem; pItem = content.next() )
	{
		checkReagent( blackpearl, 0xF7A )
		else checkReagent( bloodmoss, 0xF7B )
		else checkReagent( garlic, 0xF84 )
		else checkReagent( ginseng, 0xF85 )
		else checkReagent( mandrake, 0xF86 )
		else checkReagent( nightshade, 0xF88 )
		else checkReagent( sulfurash, 0xF8C )
		else checkReagent( spidersilk, 0xF8D )		
	}

	QStringList missing;

	if( ginseng > 0 )
		missing.append( tr( "Ginseng" ) );
	if( bloodmoss > 0 )
		missing.append( tr( "Bloodmoss" ) );
	if( mandrake > 0 )
		missing.append( tr( "Mandrake" ) );
	if( blackpearl > 0 )
		missing.append( tr( "Black Pearls" ) );
	if( spidersilk > 0 )
		missing.append( tr( "Spider's Silk" ) );
	if( garlic > 0 )
		missing.append( tr( "Garlic" ) );
	if( nightshade > 0 )
		missing.append( tr( "Nightshade" ) );
	if( sulfurash > 0 )
		missing.append( tr( "Sulfurous Ash" ) );

	bool enoughReagents = true;

	if( missing.count() > 0 )
	{
		if( pMage->socket() )
		{
			pMage->message( tr( "You don't have enough reagents." ) );
			pMage->socket()->sysMessage( tr( "You lack the following reagents: %1" ).arg( missing.join( ", ") ) );
		}
		enoughReagents = false;
	}

	return enoughReagents;
}

/*!
	This checks the needed skill to cast the specified 
	spell. If scroll is true, the spell is cast 
	from a scroll and if needed the scrolllow and scrollhigh 
	values are used instead of the book values. This 
	function returns true if the check was successful. 
*/
bool cNewMagic::checkSkill( P_CHAR pMage, UINT8 spell, bool scroll )
{
	UINT16 lowSkill, highSkill;

	stNewSpell *sInfo = findSpell( spell );

	if( !sInfo )
		return false;

	// Cut the requirements for scrolls if needed
	if( scroll && SrvParams->cutScrollReq() )
	{
		lowSkill = sInfo->scrolllow;
		highSkill = sInfo->scrollhigh;
	}
	else
	{
		lowSkill = sInfo->booklow;
		highSkill = sInfo->bookhigh;
	}

	// Do the skill check
	if( !pMage->checkSkill( MAGERY, lowSkill, highSkill ) )
	{
		disturb( pMage, true, -1 );
		return false;
	}

	// Skillcheck completed
	return true;
}

void cNewMagic::castSpell( P_CHAR pMage, UINT8 spell )
{
	if( !pMage || !pMage->socket() )
		return;

	stNewSpell *sInfo = findSpell( spell );

	if( !sInfo )
	{
		pMage->socket()->sysMessage( tr( "This spell is either not implemented or invalid" ) );
		return;
	}

	// Check if we can cast this spell
	if( !hasSpell( pMage, spell ) )
	{
		pMage->socket()->sysMessage( tr( "You don't know this spell." ) );
		return;
	}

	// Check for required mana and required reagents, if not present: cancel casting
	if( !checkMana( pMage, spell ) )
	{
		pMage->message( tr( "You don't have enough mana to cast this spell." ) );
		return;
	}
	
	if( !checkReagents( pMage, spell ) )
		return;

	if( pMage->casting() )
		disturb( pMage, true );

	// We start casting here
	pMage->setCasting( true );

	// We get frozen here too
	pMage->setPriv2( pMage->priv2() | 0x02 );	

	// Say the mantra
	// Type 0x0A : Spell
	pMage->talk( sInfo->mantra, pMage->saycolor() );

	// This is a very interesting move of OSI 
	// They send all action-packets the character has to perform in a row. 
	// But they use the action 0xE9 instead of 0x10, maybe it's a bitmask 
	// of 0xD9 but i am unsure. 
	// This will repeat the animation until
	// We are done casting or until we are being
	// disturbed.
	//pMage->startRepeatedAction( sInfo->action, sInfo->actiondelay ); // Repeat every 1250 ms
	// I *do* know that this is a drawback but for now a single animation is exactly what we need.
	pMage->action( sInfo->action );

	// Now we have to do the following: 
	// We show the target cursor after a given amount of time (set in the scripts)
	// So what we are adding here is cEndCasting() supplying the Serial of our Mage 
	// And the ID of our Spell.
	TempEffects::instance()->insert( new cEndCasting( pMage, spell, CT_BOOK, sInfo->delay ) );
}

void cNewMagic::useWand( P_CHAR pMage, P_ITEM pWand )
{
}

void cNewMagic::useScroll( P_CHAR pMage, P_ITEM pScroll )
{
}

bool cNewMagic::checkTarget( P_CHAR pCaster, stNewSpell *sInfo, cUORxTarget *target )
{
	cUOSocket *socket = pCaster->socket();

	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );

	if( sInfo->targets & (TARGET_CHAR|TARGET_ITEM) && !pItem && !pChar )
	{
		socket->sysMessage( tr( "Please target an object" ) );
		return false;
	}

	if( sInfo->targets & TARGET_CHAR && !pChar )
	{
		socket->sysMessage( tr( "Please target a living creature." ) );
		return false;
	}

	if( sInfo->targets & TARGET_ITEM && !pItem )
	{
		socket->sysMessage( tr( "Please target an item." ) );
		return false;
	}

	// Several checks (Get the correct position of the target and do NOT trust the position transmitted by the client)
	Coord_cl pos = socket->player()->pos();
	if( pChar )
		pos = pChar->pos();
	else if( pItem )
		pos = pItem->pos();
	else
	{
		pos.x = target->x();
		pos.y = target->y();
		pos.z = target->z();
	}


	if( pItem && !pItem->isInWorld() )
	{
		P_CHAR pOut = pItem->getOutmostChar();
		
		if( pOut == socket->player() )
			return true;
		else
		{
			socket->sysMessage( tr( "You can't reach this." ) );
			return false;
		}
	}
	else
	{
		// Distance check (VisRange + 5 for macros)
		if( pos.distance( socket->player()->pos() ) > ( socket->player()->VisRange() + 5 ) )
		{
			socket->sysMessage( tr( "You can't see the target." ) );
			return false;
		}
	
		// Line of Sight check
		if( !lineOfSight( pos, socket->player()->pos(), WALLS_CHIMNEYS|TREES_BUSHES|ROOFING_SLANTED|LAVA_WATER|DOORS ) )
		{
			socket->sysMessage( tr( "You can't see the target." ) );
			return false;
		}
	}
	
	// Visibility check (Chars)
	if( pChar && pChar->isHidden() && !socket->player()->isGM() )
	{
		socket->sysMessage( tr( "You can't see this character." ) );
		return false;
	}

	// Visibility check (Items)
	if( pItem && ( ( pItem->visible() == 1 && !socket->player()->Owns( pItem ) ) || pItem->visible() == 2 ) && !socket->player()->isGM() )
	{
		socket->sysMessage( tr( "You can't see this item." ) );
		return false;
	}

	return true;
}

static bool cont_has_spell( P_ITEM pCont, UINT8 spell )
{
	if( pCont->objectID() == "cSpellBook" )
	{
		cSpellBook *pBook = dynamic_cast< cSpellBook* >( pCont );

		if( pBook )
			return pBook->hasSpell( spell );

		return false;
	}
	else if( pCont->content().size() > 0 )
	{
		cItem::ContainerContent content = pCont->content();
		cItem::ContainerContent::const_iterator iter;
		cItem::ContainerContent::const_iterator end = content.end();
		for( iter = content.begin(); iter != end; ++iter )
			if( cont_has_spell( *iter, spell ) )
				return true;
	}

	return false;
}

bool cNewMagic::hasSpell( P_CHAR pMage, UINT8 spell )
{
	// Check for SpellBooks
	return cont_has_spell( pMage->getBackpack(), spell );
}

cNewMagic *NewMagic;
