
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

class cSpellTarget;
class cEndCasting;

// TARGET EDURES 30 sec (precasting)
// INVALID TARGET -> fizzle

/*!
	Returns the spell struct with the specified id
*/
stNewSpell *cNewMagic::findSpell( UINT8 id )
{
	if( id >= 64 )
		return 0;

	return &spells[id];
}

/*!
	Tries to find a spellstub based on the spell id
*/
stSpellStub *cNewMagic::findSpellStub( UINT8 id )
{
	UINT8 i = 0;
	while( spellStubs[i].stub )
	{
		if( i == id )
			return &spellStubs[i];
	}

	return 0;
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

/*!
	This class is used to loop the casting animation 
	while the spell delay is not over. The class 
	basically repeats the animation for the 
	passed destination serial and then readds itself 
	with the same parameters. The duration is used 
	to determine how long the tempeffect should 
	wait until it reanimates the object.
*/
class cAnimationRepeat: public cTempEffect
{
private:
	UINT32 anim,duration;
	SERIAL target;
public:
	/*!
		Constructs this class and sets the id of \a _anim 
		and the \a _duration of one animation cycle.
	*/
	cAnimationRepeat( SERIAL _dest, UINT8 _anim, UINT32 _duration, SERIAL _target = INVALID_SERIAL )
	{
		anim = _anim;
		duration = _duration;
		destSer = _dest;
		target = _target;
		serializable = false;
		dispellable = false;
		objectid = "animationrepeat";
		expiretime = uiCurrentTime + _duration; // + AnimationDuration
	}

	/*!
		Animates the object identified by \a source and 
        readds the TempEffect.
	*/
	virtual void Expire()
	{
		P_CHAR pChar = FindCharBySerial( destSer );

		if( pChar )
		{
			if( isItemSerial( target ) )
			{
				P_ITEM pItem = FindItemBySerial( target );
				if( pItem )
					pChar->turnTo( pItem );
			}
			else if( isCharSerial( target ) )
			{
				P_CHAR pTarget = FindCharBySerial( target );
				if( pTarget )
					pChar->turnTo( pTarget );
			}

			pChar->action( anim );
			cTempEffects::getInstance()->insert( new cAnimationRepeat( destSer, anim, duration, target ) );
		}
	}
};

/*!
	This TempEffect is used to end the casting procedure 
	after a given delay. It removes all instances of 
	\sa cAnimationRepeat assigned to the affected character.
*/
class cEndCasting: public cTempEffect
{
private:
	UINT8 spell;
	UINT16 model;
	SERIAL target;
	Coord_cl pos;
public:
	/*!
		Constructor of this class. 
		\a dest is the affected character, \a delay is the amount 
		of time to wait. \a _spell is the id of the spell being 
		casted.
	*/
	cEndCasting( SERIAL dest, UINT32 delay, UINT8 _spell, Coord_cl _pos, UINT16 _model, SERIAL _target )
	{
		dispellable = false;
        serializable = false;
		objectid = "endcasting";
		destSer = dest;
		spell = _spell;
		model = _model;
		target = _target;
		pos = _pos;
		expiretime = uiCurrentTime + delay;
	}

	virtual void Expire();
};

/*!
	Generic Targetting class for spells
*/
class cSpellTarget: public cTargetRequest
{
private:
	UINT8 spell;
public:
	/*!
		Constructs the target request
	*/
	cSpellTarget( UINT8 _spell )
	{
		spell = _spell;
	};

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		stNewSpell *sInfo = NewMagic->findSpell( spell );

		if( !sInfo )
		{
			// Precasting means that we already casted and so we have to fizzle
			if( SrvParams->precasting() )
				NewMagic->failSpell( socket->player() );
			return true;
		}

		Coord_cl pos = socket->player()->pos;
		pos.x = target->x();
		pos.y = target->y();
		pos.z = target->z();

		// Is our targetted creature correct?
		if( NewMagic->checkTarget( socket->player(), sInfo, target ) )
		{
			// We are precasting, now it's time for the spelleffect
			if( SrvParams->precasting() )
			{
				if( !socket->player()->casting() )
				{
					socket->sysMessage( tr( "You don't have a spell ready." ) );
					return true;
				}

				stSpellStub *sStub = NewMagic->findSpellStub( spell );

				if( sStub )
					sStub->stub( socket->player(), pos, target->model(), target->serial() );
			}
			// We are not precasting, so let us now begin casting
			else
			{
				if( !NewMagic->useMana( socket->player(), spell ) || !NewMagic->useReagents( socket->player(), spell ) )
					return true;

				socket->player()->action( sInfo->action ); // Directed Cast

				if( !NewMagic->checkSkill( socket->player(), spell ) )
					return true;

				// Only repeat the animation if walking disturbs casting
				if( SrvParams->walkDisturbsCast() )
					cTempEffects::getInstance()->insert( new cAnimationRepeat( socket->player()->serial, sInfo->action, sInfo->actiondelay ) );

				cTempEffects::getInstance()->insert( new cEndCasting( socket->player()->serial, sInfo->delay, spell, pos, target->model(), target->serial() ) );
			}
		}
		// Wrong target?? If Precasting -> Fizzle Spell
		else if( SrvParams->precasting() )
			NewMagic->failSpell( socket->player() );

		return true;
	}
};

/*!
	End the repetition of the animation and continue with 
	the casting.
*/
void cEndCasting::Expire()
{
	P_CHAR pChar = FindCharBySerial( destSer );
	pChar->setCasting( false );

	if( !pChar || !pChar->socket() )
		return;

	// Remove all repeating animations
	cTempEffects::getInstance()->dispel( pChar, "animationrepeat" );
		
	// If precasting we now show the target request and start the timeout (fizzle!) 
	// Tempeffect
	if( SrvParams->precasting() )
	{
		pChar->socket()->attachTarget( new cSpellTarget( spell ) );
	}
	// If we are not precasting continue casting with the spelleffect
	else
	{
		stSpellStub *sStub = NewMagic->findSpellStub( spell );
		if( !sStub )
			return;
		sStub->stub( pChar, pos, model, target );
	}
}

/*!
	This ends the casting of a specified character.
	If the second parameter is not false, it 
	displays the fizzle animation and the fizzle sound.
*/
void cNewMagic::failSpell( P_CHAR pMage, bool fizzle )
{
	pMage->setCasting( false );

	// Stop the repeating animation and the endspell thing
	cTempEffects::getInstance()->dispel( pMage, "animationrepeat" );
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
		if( pMage->socket() )
			pMage->socket()->sysMessage( tr( "You don't have enough mana to cast this spell." ) );
		return false;
	}

	pMage->setMn( pMage->mn() - sInfo->mana );
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
		failSpell( pMage );
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

	// If we are not precasting and the spell requires a target,
	// show the targetting cursor before casting
	if( !SrvParams->precasting() && sInfo->targets )
	{
		pMage->socket()->sysMessage( tr( sInfo->target ) );
		pMage->socket()->attachTarget( new cSpellTarget( spell ) );
	}
	// spells without a target or precasted spells dont need that
	else
	{
		if( !useMana( pMage, spell ) || !useReagents( pMage, spell ) )
			return;		

		cUOTxUnicodeSpeech speech;
		speech.setSource( pMage->serial );
		speech.setFont( 3 );
		speech.setLanguage( "ENG" ); // These are always ENG
		speech.setModel( pMage->id() );
		speech.setName( pMage->name.c_str() );
		speech.setText( sInfo->mantra );
		speech.setType( cUOTxUnicodeSpeech::Spell );

		// Send it to all Sockets in Range
		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
			if( mSock && mSock->player() && mSock->player()->inRange( pMage, mSock->player()->VisRange ) )
				mSock->send( &speech );

		pMage->action( sInfo->action ); // Non Directed Cast
		
		if( !checkSkill( pMage, spell ) )
			return;

		// This is the place where we start casting
		pMage->setCasting( true );

		// Only repeat the animation if walking disturbs casting
		if( SrvParams->walkDisturbsCast() )
			cTempEffects::getInstance()->insert( new cAnimationRepeat( pMage->serial, sInfo->action, sInfo->actiondelay ) );

		cTempEffects::getInstance()->insert( new cEndCasting( pMage->serial, sInfo->delay, spell, pMage->pos, pMage->id(), pMage->serial ) );
	}
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

stSpellStub cNewMagic::spellStubs[] =
{
	{ spellClumsy },	// 0 = Clumsy
	{ NULL } // Terminator
};

cNewMagic *NewMagic;

/*!
	Tempeffect for Bless
*/
class cBlessEffect: public cTempEffect
{
};

class cCurseEffect: public cTempEffect
{
};

class cDexModifier: public cTempEffect
{
private:
	INT8 modifier_;
public:
	cDexModifier()	
	{ 
		objectid = "dexmodifier";
		dispellable = true;
		serializable = true;
	}
	virtual ~cDexModifier() {;}

	void setModifier( INT8 data ) { modifier_ = data; }
	INT8 modifier() { return modifier_; }

	static cDexModifier *Create( P_CHAR pChar, INT8 modifier )
	{
		cDexModifier *dMod = new cDexModifier;
		dMod->setDest( pChar->serial );
		dMod->setModifier( modifier );

		// Apply the stuff
		pChar->setDex( pChar->effDex() + modifier );

		if( pChar->socket() )
			pChar->socket()->sendStatWindow();
		
		return dMod;
	}
	
	void Off( P_CHAR pc ) { Expire(); }

	// Reset the 
	virtual void Expire()
	{
		P_CHAR pChar = FindCharBySerial( getDest() );
		if( pChar )
		{
			pChar->setDex( pChar->effDex() - modifier_ );
			
			if( pChar->socket() )
				pChar->socket()->sendStatWindow();
		}
	}

	virtual void Serialize(ISerialization &archive)
	{
		// Save the modifier
		if( archive.isReading() )
			archive.read( "modifier", modifier_ );
		else if( archive.isWritting() )
			archive.write( "modifier", modifier_ );
	}
};

/*!
	Spell stub for Clumsy
*/
void cNewMagic::spellClumsy( P_CHAR pMage, Coord_cl tPos, UINT16 model, SERIAL tSerial, INT32 magery )
{
	P_CHAR pChar = FindCharBySerial( tSerial );

	if( !pChar )
		return;

	// Check if the character alread has a clumsyness effect on 
	vector< cTempEffect* > tEffects = cTempEffects::getInstance()->teffects.asVector();

	// Do not stack with curse effect
	std::vector< cTempEffect* >::iterator i;
	for( i = tEffects.begin(); i != tEffects.end(); ++i )
		if( i != NULL && (*i) != NULL && (*i)->dispellable && (*i)->getDest() == pChar->serial )
			if( (*i)->objectID() == "curse" )
				return;
			else if( (*i)->objectID() == "dexmodifier" )
			{
				(*i)->Expire();
				cTempEffects::getInstance()->teffects.erase( (*i) );
				break;
			}

	// Display an effect for the target
	pChar->soundEffect( 0x1E1 );
	pChar->effect( EFFECT_CURSE, 9 );

    // Insert and Create our little effect
	cDexModifier *dMod = cDexModifier::Create( pChar, -1  * RandomNum( 10, 20 ) );
	dMod->setExpiretime_s( RandomNum( 60, 120 ) );
	cTempEffects::getInstance()->insert( dMod );
}

