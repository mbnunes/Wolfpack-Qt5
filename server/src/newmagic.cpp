
// Library Includes
#include "qstringlist.h"
#include "qdom.h"

// Wolfpack Includes
#include "newmagic.h"
#include "chars.h"
#include "inlines.h"
#include "network/uosocket.h"
#include "srvparams.h"
#include "TmpEff.h"
#include "wpdefmanager.h"
#include "tilecache.h"

class cSpellTarget : public cTargetRequest
{
private:
	UINT8 spell;
	UINT8 type;
public:
	cSpellTarget( P_CHAR pMage, UINT8 _spell, UINT8 _type )
	{
		spell = _spell;
		type = _type;

		// We will just use the Mage for the sysmessage
		stNewSpell *sInfo = NewMagic->findSpell( spell );

		if( sInfo )
			pMage->socket()->sysMessage( sInfo->target );
		
		timeout_ = uiCurrentTime + ( 30 * MY_CLOCKS_PER_SEC );
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// Target information:
		socket->sysMessage( "Target information:" );
		socket->sysMessage( QString( "SERIAL: %1" ).arg( target->serial() ) );
		socket->sysMessage( QString( "Pos: %1,%2,%3" ).arg( target->x() ).arg( target->y() ).arg( target->z() ) );
		socket->sysMessage( QString( "Model: %1" ).arg( target->model() ) );

		return true;
	}

	virtual void timedout( cUOSocket *socket )
	{
		P_CHAR pChar = socket->player();

		// After the target timed out we cancel our spell
		if( pChar )
		{
			if( timeout_ > uiCurrentTime )
				socket->sysMessage( tr( "You loose your concentration after waiting for too long." ) );
			else
				socket->sysMessage( tr( "You cancel the spell." ) );

			NewMagic->disturb( pChar, false, -1 );
		}
	}
};

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
		QDomElement *elem = DefManager->getSection( WPDT_SPELL, sList[i] );

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
				spells[id].action = node.text().toInt();
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
			}
			else if( node.nodeName() == "mana" )
				spells[id].mana = node.text().toInt();
			else if( node.nodeName() == "scroll" )
				spells[id].scroll = hex2dec( node.text() ).toLong();

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

cEndCasting::cEndCasting( P_CHAR _mage, UINT8 _spell, UINT8 _type )
{
	mage = _mage->serial;
	spell = _spell;
	type = _type;
}

void cEndCasting::Expire()
{
	P_CHAR pMage = FindCharBySerial( mage );
	
	if( !pMage )
		return;

	pMage->stopRepeatedAction();

	if( !pMage->socket() )
		return;

	// Show a target cursor
	pMage->socket()->attachTarget( new cSpellTarget( pMage, spell, type ) );
}

/*!
	Calculates the Spell ID for a given Scroll Item id
*/
INT8 cNewMagic::calcSpellId( UINT16 scroll )
{
	tile_st tile = cTileCache::instance()->getTile( scroll );

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
		case 0:
			return 0x1F2E;
		case 1:
			return 0x1F2F;
		case 2:
			return 0x1F30;
		case 3:
			return 0x1F31;
		case 4:
			return 0x1F32;
		case 5:
			return 0x1F33;
		case 6:
			return 0x1F2D;
		case 7:
			return 0x1F34;
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
	pMage->setCasting( false );

	// Stop the repeating animation and the endspell thing
	pMage->stopRepeatedAction();
	cTempEffects::getInstance()->dispel( pMage, "endcasting" );
	
	if( fizzle )
	{
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
	return true;
}

/*!
	This function uses the reagents for the specified 
	spell. It displays a message with the missing 
	reagents if needed. The returnvalue is false if 
	the character does not have enough reagents.
*/
bool cNewMagic::useReagents( P_CHAR pMage, UINT8 spell )
{
	return true;
}

/*!
	Just like useReagents this function is checking 
	for required reagents on a characater to cast a 
	spell. But unlike useReagents it wont really 
	consume the reagents but just return false if 
	the required reagents are not present.
*/
bool cNewMagic::checkReagents( P_CHAR pMage, UINT8 spell )
{
	return true;
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

	// Check for required mana and required reagents, if not present: cancel casting
	if( !checkMana( pMage, spell ) || !checkReagents( pMage, spell ) )
		return;

	// Say the mantra
	// Type 0x0A : Spell
	pMage->talk( sInfo->mantra, pMage->saycolor(), 0x0A, false );

	// This is a very interesting move of OSI 
	// They send all action-packets the character has to perform in a row. 
	// But they use the action 0xE9 instead of 0x10, maybe it's a bitmask 
	// of 0xD9 but i am unsure. 
	// This will repeat the animation until
	// We are done casting or until we are being
	// disturbed.
	pMage->startRepeatedAction( 0xE9, 1250 ); // Repeat every 1250 ms

	// Now we have to do the following: 
	// We show the target cursor after a given amount of time (set in the scripts)
	// So what we are adding here is cEndCasting() supplying the Serial of our Mage 
	// And the ID of our Spell.

	// This is the place where we start casting
	pMage->setCasting( true );

	// Only repeat the animation if walking disturbs casting
	cTempEffects::getInstance()->insert( new cEndCasting( pMage, spell, CT_BOOK ) );
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
	Coord_cl pos = socket->player()->pos;
	if( pChar )
		pos = pChar->pos;
	else if( pItem )
		pos = pItem->pos;
	else
	{
		pos.x = target->x();
		pos.y = target->y();
		pos.z = target->z();
	}

	// Distance check (VisRange + 5 for macros)
	if( pos.distance( socket->player()->pos ) > ( socket->player()->VisRange + 5 ) )
	{
		socket->sysMessage( tr( "You can't see the target." ) );
		return false;
	}

	// Line of Sight check
	if( !lineOfSight( pos, socket->player()->pos, WALLS_CHIMNEYS|TREES_BUSHES|ROOFING_SLANTED|LAVA_WATER|DOORS ) )
	{
		socket->sysMessage( tr( "You can't see the target." ) );
		return false;
	}

	// Visibility check (Chars)
	if( pChar && pChar->isHidden() && !socket->player()->isGM() )
	{
		socket->sysMessage( tr( "You can't see this character." ) );
		return false;
	}

	// Visibility check (Items)
	if( pItem && ( ( pItem->visible == 1 && !socket->player()->Owns( pItem ) ) || pItem->visible == 2 ) && !socket->player()->isGM() )
	{
		socket->sysMessage( tr( "You can't see this item." ) );
		return false;
	}

	return true;
}

cNewMagic *NewMagic;

