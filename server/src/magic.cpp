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

#include "magic.h"
#include "chars.h"
#include "items.h"
#include "multis.h"
#include "SndPkg.h"
#include "itemid.h"
#include "tilecache.h"
#include "debug.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "globals.h"
#include "wpdefmanager.h"
#include "classes.h"
#include "maps.h"
#include "network.h"
#include "TmpEff.h"
#include "territories.h"
#include "skills.h"
#include "tilecache.h"
#include "walking.h"

#undef DBGFILE
#define DBGFILE "magic.cpp"
#define NOTUSED 0

/////////////////////////////////////////////////////////////////
/// INDEX:
//		- misc magic functions
//		- NPCs casting spells related functions
//		- ITEMs magic powers related funcions
//		- PCs casting spells related functions
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////// MISC MAGIC FUNCTIONS ////////////////////////////////////
/////////////////////////////////////////////////////////////////

// NEW IMPLEMENTATION BY DARKSTORM
cSpell::cSpell( void ):
bitfield_( 1 ), mana_( 0 ), stamina_( 0 ), health_( 0 ),
reqGinseng_( 0 ), reqMoss_( 0 ), reqDrake_( 0 ), 
reqPearl_( 0 ), reqSilk_( 0 ), reqAsh_( 0 ),
reqShade_( 0 ), reqGarlic_( 0 ), lowSkill_( 0 ),
highSkill_( 1000 ), scrollLowSkill_( 0 ),
scrollHighSkill_( 1000 ), action_( 0x10 ), delay_( 0 )
{
}

void cSpell::load( QDomElement &node )
{
	id_ = node.attribute( "id", "1" ).toInt();

	// Read the spell properties
	QDomNodeList nodes = node.childNodes();

	for( UI16 i = 0; i < nodes.count(); i++ )
	{
		if( !nodes.item( 1 ).isElement() )
			continue;

		QDomElement property = nodes.item( i ).toElement();

		if( property.nodeName() == "mantra" )
			mantra_ = property.toElement().text();
		else if( property.nodeName() == "circle" )
			circle_ = property.toElement().text().toInt();
		else if( property.nodeName() == "lowskill" )
			lowSkill_ = property.toElement().text().toInt();
		else if( property.nodeName() == "highskill" )
			highSkill_ = property.toElement().text().toInt();
		else if( property.nodeName() == "scrolllowskill" )
			scrollLowSkill_ = property.toElement().text().toInt();
		else if( property.nodeName() == "scrollhighskill" )
			scrollHighSkill_ = property.toElement().text().toInt();
		else if( property.nodeName() == "action" )
			action_ = property.toElement().text().toInt( 0, 16 );
		else if( property.nodeName() == "target" )
			target_ = property.toElement().text();
		
		// <reflect />, <noreflect />
		else if( property.nodeName() == "reflect" )
			setReflect( true );
		else if( property.nodeName() == "noreflect" )
			setReflect( false );

		// <agressive />, <notagressive />
		else if( property.nodeName() == "agressive" )
			setAgressive( true );
		else if( property.nodeName() == "notagressive" )
			setAgressive( false );

		// <enabled />, <disabled />
		else if( property.nodeName() == "enabled" )
			setEnabled( true );
		else if( property.nodeName() == "disabled" )
			setEnabled( false );

		// <runic />, <notrunic />
		else if( property.nodeName() == "runic" )
			setRunic( true );
		else if( property.nodeName() == "notrunic" )
			setRunic( false );

		// Parse requirements:
		else if( property.nodeName() == "requirements" )
		{
			QDomNodeList requirements = property.childNodes();

			for( UI16 j = 0; j < requirements.count(); j++ )
			{
				if( !requirements.item( j ).isElement() )
					continue;

				QDomElement requirement = requirements.item( j ).toElement();
				
				// <health>, <mana>, <stamina>
				if( requirement.nodeName() == "mana" )
					mana_ = requirement.text().toInt();
				else if( requirement.nodeName() == "stamina" )
					stamina_ = requirement.text().toInt();
				else if( requirement.nodeName() == "health" )
					health_ = requirement.text().toInt();

				// Reagents
				else if( requirement.nodeName() == "ginseng" )
					reqGinseng_ = requirement.text().toInt();
				else if( requirement.nodeName() == "moss" )
					reqMoss_ = requirement.text().toInt();
				else if( requirement.nodeName() == "drake" )
					reqDrake_ = requirement.text().toInt();
				else if( requirement.nodeName() == "pearl" )
					reqPearl_ = requirement.text().toInt();
				else if( requirement.nodeName() == "silk" )
					reqSilk_ = requirement.text().toInt();
				else if( requirement.nodeName() == "ash" )
					reqAsh_ = requirement.text().toInt();
				else if( requirement.nodeName() == "shade" )
					reqShade_ = requirement.text().toInt();
				else if( requirement.nodeName() == "garlic" )
					reqGarlic_ = requirement.text().toInt();
			}
		}
	}
}

bool cSpell::prepare( P_CHAR character, UI08 source )
{
	if( source == 1 )
		return true;
	else
		return false;
}

void cSpell::cast( P_CHAR character, UI08 source )
{
}

// Magic "container" class
void cMagic::load( void )
{
	clConsole.PrepareProgress( "Loading spells" );

	QStringList sections = DefManager->getSections( WPDT_SPELL );

	for( UI08 i = 0; i < sections.count(); i++ )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_SPELL, sections[ i ] );
		// Check if it's a valid spell-id
		bool ok = true;
		UI32 spellID = sections[ i ].toInt( &ok );

		if( !ok || spellID > 255 )
			continue;

		loadedSpells[ spellID ] = new cSpell;
		loadedSpells[ spellID ]->load( *DefSection );
	}

	clConsole.ProgressDone();
}

void cMagic::reload( void )
{
	unload();
	load();
}

void cMagic::unload( void )
{
	map< UI08, cSpell* >::iterator iterator;

	for( iterator = loadedSpells.begin(); iterator != loadedSpells.end(); ++iterator )
		delete iterator->second;
}

// Checks for sufficient Mana/Stamina/Health
bool cMagic::checkStats( P_CHAR caster, cSpell *spell )
{
	if( caster->isGM() )
		return true;

	if( ( caster->mn() < spell->mana() ) && !( caster->priv2() & 0x10 ) )
	{
		sysmessage( calcSocketFromChar( caster ), "You have insufficient mana to cast that spell.");
		return false;
	}

	if( caster->stm() < spell->stamina() )
	{
		sysmessage( calcSocketFromChar( caster ), "You have insufficient stamina to cast that spell.");
		return false;
	}

	if( caster->hp() < spell->health() )
	{
		sysmessage( calcSocketFromChar( caster ), "You have insufficient health to cast that spell.");
		return false;
	}

	return true;
}

void cMagic::speakMantra( P_CHAR caster, cSpell *spell )
{
	if( spell->runic() || caster->skill( MAGERY ) >= 1000 )
		npctalkall_runic( caster, spell->mantra().upper().ascii(), 0 );
	else
		caster->talk( spell->mantra(), -1, 0 );
}

cSpell *cMagic::getSpell( UI08 spellId )
{
	if( loadedSpells.find( spellId ) != loadedSpells.end() )
		return loadedSpells[ spellId ];

	return NULL;
}
	
void cMagic::openSpellBook( P_CHAR mage, P_ITEM spellbook )
{
/*

	Since DS is rewritting this stuff, I think it's safe to assume he have
	already done this conversion... taking out those cPacket stuff, we use cUOPacket


	UI16 i;

	// Sends a spellbook to the client
	UOXSOCKET socket = calcSocketFromChar( mage );

	// We *need* to send the spellbook to a connected client
	if( socket == -1 )
		return;

	// If no SpellBook is passed ( Third Dawn only ! ) find one
	if( spellbook == NULL )
	{
		spellbook = findSpellBook( mage );
		
		if( spellbook == NULL )
		{
			sysmessage( socket, "You do not posess a spellbook" );
			return;
		}
	}

	// Draw Container (GumpID: 0xFFFF, Serial: SpellBook );
	cPDrawContainer drawContainer( 0xFFFF, spellbook->serial );
	drawContainer.send( socket );
	
	// Get the spells
	vector< UI08 > spells;

	vector< SERIAL > scrolls = contsp.getData( spellbook->serial );

	for( i = 0; i < scrolls.size(); i++ )
	{
		P_ITEM scroll = FindItemBySerial( scrolls[ i ] );
		SI16 spellId = calcSpellId( scroll->id() );
		
		if( spellId > 0 )
			spells.push_back( spellId );
	}

	// No spells to send so quit
	if( spells.size() == 0 )
		return;

	cPContainerItems containerItems;

	// Send the spells
	// Amount: spellId
	// Serial: Unique id per spell (just use the default)
	// Hue: Irrelevant
	for( i = 0; i < spells.size(); i++ )
		containerItems.addItem( 0x41000000 + spells[ i ], 0, spells[ i ], 0, 0, spellbook->serial, 0x450 );

	containerItems.send( socket );
	*/
}

// This should get one spellbook
P_ITEM cMagic::findSpellBook( P_CHAR mage )
{
	if( !mage )
		return NULL;

	// We'll only handle Spellbooks in the mainpack or on the characters body
	vector< SERIAL > equipment = contsp.getData( mage->serial );

	UI16 i;
	P_ITEM backpack;

	for( i = 0; i < equipment.size(); i++ )
	{
		P_ITEM item = FindItemBySerial( equipment[ i ] );
		
		if( ( item->layer() == 1 ) && ( item->type() == 9 ) )
			return item;

		// While we're at it get the packitem
		if( item->layer() == 0x15 ) 
			backpack = item;
	}

	// The character doesn't have any spellbooks equipped so check his backpack
	if( backpack != NULL )
	{
		vector< SERIAL > content = contsp.getData( backpack->serial );

		for( i = 0; i < content.size(); i++ )
		{
			P_ITEM item = FindItemBySerial( content[ i ] );

			if( item->type() == 9 )
				return item;
		}
	}

	return NULL;
}

// TO DO: Check for spells in the book if sourceType == 0
bool cMagic::prepare( P_CHAR caster, UI08 spellId, UI08 sourceType, P_ITEM sourceItem )
{
	// Prepares to cast a spell
	if( caster == NULL )
		return false;

	if( loadedSpells.find( spellId ) == loadedSpells.end() )
	{
		sysmessage( calcSocketFromChar( caster ), QString( "There is no such spell (%1)" ).arg( spellId ) );
		return false;
	}

	// Get the spell
	cSpell *spell = loadedSpells.find( spellId )->second;

	// Casting in jail is not allowed
	if( caster->cell() != 0 && !caster->isGM() )
	{
		sysmessage( calcSocketFromChar( caster ), "You may not cast spells while you are in jail." );
		return false;
	}

	// Casting agressive spells is not allowed in guarded towns
	if( spell->agressive() && !caster->isGM() )
	{
		cTerritory* Region = caster->region;
		if( Region && Region->allowsMagic() )
		{
			sysmessage( calcSocketFromChar( caster ), "You may not cast agressive spells in a guarded area." );
			return false;
		}
	}

	if( !spell->enabled() )
	{
		sysmessage( calcSocketFromChar( caster ), "This spell is currently disabled." );
		return false;
	}

	// Casting with Weapons or Shields equipped is not allowed for players
	// For Wands it's ignored
	if( ( !caster->isGMorCounselor() ) && ( sourceType != 2 ) )
	{
		P_ITEM item;
		vector< SERIAL > equipment = contsp.getData( caster->serial );
		
		for( UI32 i = 0; i < equipment.size(); i++ )
		{
			item = FindItemBySerial( equipment[ i ] );

			if( item == NULL )
				continue;

			// If it's on layer 1 it has to be a spellbook
			if( ( ( item->layer() == 1 ) && ( item->type() == 9 ) ) || ( item->layer() == 2 ) )
			{
				// Allowed are: 
				// Gnarled staffs (0x13F9), 
				// Black Staffs and Wands (0x0DF0 - 0x0DF5), 
				// Quarter Staffs ( 0xE89+0xE8A ),
				// Order+Chaos Shields

				UI16 id = item->id();

				if( ( id == 0x13f9 ) || ( id >= 0xDF0 && id <= 0xDF5 ) || ( id == 0xE89 ) || ( id == 0xE8A ) )
					continue;

				sysmessage( calcSocketFromChar( caster ), "You cannot cast without free hands." );
				return false;
			}
		}
	}

	// Unhide the caster for agressive spells only? (later)
	caster->unhide();
	
	// We can't meditade while we're casting
	caster->disturbMed();

	// Check for sufficient mana if we're not casting from a wand
	if( sourceType != 2 )
		if( !checkStats( caster, spell ) )
			return false;

	// If we're casting out of a spellbook we'll consume reagents
	if( sourceType == 0 && !( caster->priv2() & 0x80 ) )
		if( !checkReagents( caster, spell ) )
			return false;

	// Display the mantra *only* if we're not casting from a wand
	if( sourceType != 2 )
		speakMantra( caster, spell );
	
	//caster->setSpell( spellId ); // Remember our current spell-id - should rather be a pointer to a cSpell
	//caster->setCasting( true ); // we're casting ! :o)
	
	// "Enduring" animation
	//caster->setSpellaction( spell->action() );
	//caster->setNextact( 75 );

	// Measure the delay until our targetting cursor is shown
	// Spellbooks have the full delay, i'll half it for scrolls
	//caster->setSpelltime( 0 );

	if( !caster->isGM() )
	{
		UI32 delay = ( spell->delay() * MY_CLOCKS_PER_SEC );

		if( sourceType == 1 )
			delay /= 2;

		//caster->setSpelltime( uiCurrentTime + delay );
	}

	// If we're using a wand don't display effects
	if( sourceType == 2 )
		return false;

	// Start our casting action
	// AllTmpEff->Insert( new cTimedAction( caster, spell->action(), spell->delay() ) );
	cTempEffects::getInstance()->insert( new cTimedSpellAction( caster->serial, spell->action() ) );
	cMagic::preParticles( spellId, caster );

	return spell->prepare( caster, sourceType );

	/*if (!SrvParams->cutScrollReq())
	{
		if (type==1 && !(pc_currchar->isGM()) && !Skills->CheckSkill(pc_currchar, MAGERY, loskill, hiskill))
		{
				SpellFail(s);
				pc_currchar->setSpell( 0 );
				pc_currchar->setCasting(false);
				return false;
		}
	}*/
}

///////////////////
// Name:	checkReagents
// History:	DarkStorm, 30. May 2002
// Purpose:	Check for needed Reagents and display a message
//
bool cMagic::checkReagents( P_CHAR caster, cSpell *spell )
{
		QStringList missing;

		// Check for every reagent
		if( spell->reqAsh() && ( getamount( caster, 0x0F8C ) < spell->reqAsh() ) )
			missing.push_back( "Sa" );
	
		if( spell->reqDrake() && ( getamount( caster, 0xF86 ) < spell->reqDrake() ) )
			missing.push_back( "Mr" );

		if( spell->reqGarlic() && ( getamount( caster, 0xF84 ) < spell->reqGarlic() ) )
			missing.push_back( "Ga" );

		if( spell->reqGinseng() && ( getamount( caster, 0xF85 ) < spell->reqGinseng() ) )
			missing.push_back( "Gi" );

		if( spell->reqMoss() && ( getamount( caster, 0xF7B ) < spell->reqMoss() ) )
			missing.push_back( "Bm" );

		if( spell->reqPearl() && ( getamount( caster, 0xF7A ) < spell->reqPearl() ) )
			missing.push_back( "Bp" );
		
		if( spell->reqShade() && ( getamount( caster, 0xF88 ) < spell->reqShade() ) )
			missing.push_back( "Ns" );

		if( spell->reqSilk() && ( getamount( caster, 0xF8D ) < spell->reqSilk() ) )
			missing.push_back( "Ss" );

		if( missing.count() > 0 )
		{
			sysmessage( calcSocketFromChar( caster ), QString( "You don't have enough reagents to cast that spell [%1]" ).arg( missing.join( "," ) ) );
			return false;
		}
		else		
			return true;
}

short cMagic::calcSpellId( UI16 model )
{
	tile_st tile = cTileCache::instance()->getTile( model );
	
	if( tile.unknown1 == 0 )
		return -1;
	else
		return tile.unknown1;
}

bool cMagic::hasSpell( P_CHAR mage, UI08 spellId )
{
	return hasSpell( findSpellBook( mage ), spellId );
}

bool cMagic::hasSpell( P_ITEM spellbook, UI08 spellId )
{
	// No spellbook no spells!
	if( spellbook == NULL )
		return false;

	// If a certain attribute of the spellbook is set i think 
	// we should always tell the server: "yes we got the spell"

	// Check for the spell-Scroll
	vector< SERIAL > content = contsp.getData( spellbook->serial );
	vector< SERIAL >::iterator myIter;
	
	for( myIter = content.begin(); myIter != content.end(); ++myIter )
	{
		P_ITEM scroll = FindItemBySerial( (*myIter) );

		if( calcSpellId( scroll->id() ) == spellId )
			return true;
	}

	return false;
}

///////////////////
// Name:	InitSpells
// History:	AntiChrist, 11 September 1999
// Purpose:	Initialize magery system. Parse spells.scp and store spells data.
//
struct spell_st
{
	int enabled;		// spell enabled?
	int circle;			// circle number
	int mana;			// mana requirements
	int loskill;		// low magery skill req.
	int hiskill;		// high magery skill req.
	int sclo;			// low magery skill req. if using scroll
	int schi;			// high magery skill req. if using scroll
	char mantra[25];	// words of power
	int action;			// character action
	int delay;			// spell delay
	reag_st reagents;	// reagents req.
	char strToSay[100];	// string visualized with targ. system
	int reflect;		// 1=spell reflectable, 0=spell not reflectable
	unsigned char runic;
};

spell_st spells[100];

/*int cMagic::InitSpells(void)
{
	int curspell = 0; // current spell
	
	openscript("spells.scp");
	// if (!i_scripts[spells_script]->find("SPELLA"))
	//{
	//	closescript();
	//	clConsole.send("Cannot find SPELLS SETTINGS string in spells.scp!");
	//	return false;
	//}
	
	unsigned long loopexit = 0;
	do
	{
		read2();
		if (!strcmp("SPELL", (char*)script1))			curspell = str2num(script2);
		else if (!strcmp("ENABLE", (char*)script1))		spells[curspell].enabled = str2num(script2);
		else if (!strcmp("CIRCLE", (char*)script1))		spells[curspell].circle = str2num(script2);
		else if (!strcmp("MANA", (char*)script1))		spells[curspell].mana = str2num(script2);
		else if (!strcmp("LOSKILL", (char*)script1))	spells[curspell].loskill = str2num(script2);
		else if (!strcmp("HISKILL", (char*)script1))	spells[curspell].hiskill = str2num(script2);
		else if (!strcmp("SCLO", (char*)script1))		spells[curspell].sclo = str2num(script2);
		else if (!strcmp("SCHI", (char*)script1))		spells[curspell].schi = str2num(script2);
		else if (!strcmp("MANTRA", (char*)script1))		strcpy(spells[curspell].mantra, (char*)script2);
		else if (!strcmp("ACTION", (char*)script1))		spells[curspell].action = hex2num(script2);
		else if (!strcmp("DELAY", (char*)script1))		spells[curspell].delay = str2num(script2);
		else if (!strcmp("ASH", (char*)script1))		spells[curspell].reagents.ash = str2num(script2);
		else if (!strcmp("DRAKE", (char*)script1))		spells[curspell].reagents.drake = str2num(script2);
		else if (!strcmp("GARLIC", (char*)script1))		spells[curspell].reagents.garlic = str2num(script2);
		else if (!strcmp("GINSING", (char*)script1))	spells[curspell].reagents.ginseng = str2num(script2);
		else if (!strcmp("MOSS", (char*)script1))		spells[curspell].reagents.moss = str2num(script2);
		else if (!strcmp("PEARL", (char*)script1))		spells[curspell].reagents.pearl = str2num(script2);
		else if (!strcmp("SHADE", (char*)script1))		spells[curspell].reagents.shade = str2num(script2);
		else if (!strcmp("SILK", (char*)script1))		spells[curspell].reagents.silk = str2num(script2);
		else if (!strcmp("TARG", (char*)script1))		strcpy(spells[curspell].strToSay, (char*)script2);
		else if (!strcmp("REFLECT", (char*)script1))	spells[curspell].reflect = str2num(script2);
		else if (!strcmp("RUNIC", (char*)script1))		spells[curspell].runic = str2num(script2);
	}
	while ((strcmp((char*)script1, "EOF")) &&(++loopexit < MAXLOOPS));
	closescript();
	return true;
}*/

// Spellbook
// Purpose:	Sends the spellbook item (with all the
//			memorized spells) to player when doubleclicked.

void cMagic::SpellBook(UOXSOCKET s)
{
	SpellBook(s,FindItemBySerPtr(buffer[s])) ;
}
void cMagic::SpellBook(UOXSOCKET s, P_ITEM pi)
{
	//if (pi == NULL)
		//return;
	
//	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	if (!pi && pc_currchar->packitem() != INVALID_SERIAL)
	{
		unsigned int ci=0;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pc_currchar->packitem());
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);

			if (pj->type()==9)
			{
				pi=pj;
				break;
			}
		}
	}
	if (!pi)
	{
		unsigned int ci=0;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pc_currchar->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);
			if ( pj->type() == 9 && pj->layer() == 1 )
			{
				pi=pj;
				break;
			}
		}
	}
	// LB remark: If you want more than one spellbook per player working predictable
	// quite a lot of that function needs to be rewritten !
	// reason: just have a look at the loop above ...

	if (!pi ||	// no book at all
		(pc_currchar->packitem() != INVALID_SERIAL && pi->contserial != pc_currchar->packitem() &&	// not in primary pack
				!pc_currchar->Wears(pi)))		// not equipped
	{
		sysmessage(s, "In order to open spellbook, it must be equipped in your hand or in the first layer of your backpack.");
		return;
	}

	if( pi->layer() != 1 ) senditem(s,pi); // prevents crash if pack not open

	unsigned char sbookstart[8]="\x24\x40\x01\x02\x03\xFF\xFF";
	LongToCharPtr(pi->serial, &sbookstart[1]);
	Xsend(s, sbookstart, 7);

	int spells[65] = {0,};
	int i, scount=0;

	unsigned int ci=0;
	P_ITEM pj;
	vector<SERIAL> vecContainer = contsp.getData(pi->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pj = FindItemBySerial(vecContainer[ci]);
		if (IsSpellScroll72(pj->id()))
		{
			spells[pj->id()-0x1F2D]=1;
		}
	}

	// Fix for Reactive Armor/Bird's Eye dumbness. :)
	i=spells[0];
	spells[0]=spells[1];
	spells[1]=spells[2];
	spells[2]=spells[3];
	spells[3]=spells[4];
	spells[4]=spells[5];
	spells[5]=spells[6]; // Morac is right! :)
	spells[6]=i;
	// End fix.

	if (spells[64])
	{
		for (i=0;i<64;i++)
			spells[i]=1;
		spells[64]=0;
	}
	//spells[64]=spells[65];
	//spells[65]=0;

	for ( i = 0; i < 64; ++i )
	{
		if (spells[i]) 
			++scount;
	}
	char sbookinit[6]="\x3C\x00\x3E\x00\x03";
	sbookinit[1]=((scount*19)+5)>>8;
	sbookinit[2]=((scount*19)+5)%256;
	sbookinit[3]=scount>>8;
	sbookinit[4]=scount%256;
	if (scount>0) Xsend(s, sbookinit, 5);

	unsigned char sbookspell[20]="\x40\x01\x02\x03\x1F\x2E\x00\x00\x01\x00\x48\x00\x7D\x40\x01\x02\x03\x00\x00";
	for (i=0;i<64;i++)
	{
		if (spells[i])
		{
			sbookspell[0]=0x41;
			sbookspell[1]=0x00;
			sbookspell[2]=0x00;
			sbookspell[3]=i+1;
			sbookspell[8]=i+1;
			LongToCharPtr(pi->serial, &sbookspell[13]);
			Xsend(s, sbookspell, 19);
		}
	}
}


///////////////////
// Name:	GateCollision
// History:	Unknown, Modified by AntiChrist - collision dir
// Purpose:	Used when a PLAYER passes throu a gate. Takes the player
//			to the other side of the gate-link.
//
char cMagic::GateCollision(P_CHAR pc_player)
{
	unsigned int n;
//	extern cRegion *cMapObjects::getInstance();

	// Check to make sure that this isn't a NPC (they shouldn't go throught gates)
	if( pc_player->isNpc() )
	return 0;

	// Now check whether the PC (player character) has moved or simply turned
	// If they have only turned, then ignore checking for a gate collision since it would
	// have happened the previous time
	if( pc_player->pos == pc_player->prevPos() )
		return 0;

	RegionIterator4Items ri( pc_player->pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if (mapitem->type() == 51 || (mapitem->type()==52))
			{
				if (mapitem->type()==51) n=1;
				else n=0;
				if ((pc_player->pos.x==mapitem->pos.x)&&
					(pc_player->pos.y==mapitem->pos.y)&&
					(pc_player->pos.z>=mapitem->pos.z))
				{
			// Dupois - Check for any NPC's that are following this player
			//			There has to be a better way than this...
			//			Think about it some more and change this.
			// If this is a player character
					if ( pc_player->isPlayer() )
					{
						// Look for an NPC
						RegionIterator4Chars rg(pc_player->pos);
						for ( rg.Begin(); !rg.atEnd(); rg++ )
						{
							P_CHAR pc = rg.GetData();
							// That is following this player character
							if ( (pc->isNpc()) && (pc->ftarg() == pc_player->serial) )
							{
								// If the NPC that is following this player character is within 5 paces
								if ( chardist(pc_player, pc) <= 4 )
								{
									// Teleport the NPC along with the player
									pc->MoveTo(gatex[mapitem->gatenumber][n], gatey[mapitem->gatenumber][n], gatez[mapitem->gatenumber][n]);
									teleport(pc);
								}
							}
						}
					}
					// Set the characters destination
					pc_player->MoveTo(gatex[mapitem->gatenumber][n], gatey[mapitem->gatenumber][n]+ 1, gatez[mapitem->gatenumber][n]);
					teleport(pc_player);
					soundeffect( calcSocketFromChar( pc_player ), 0x01, 0xFE );
					staticeffect( pc_player, 0x37, 0x2A, 0x09, 0x06 );
				}
			}
		}
	}

	// Since the character has moved a step update the prevXYZ values
	// to prevent the "bounce back" effect of the GateCollision check
	pc_player->setPrevPos(pc_player->pos);

	return 1;
}

///////////////////
// Name:	SummonMonster
// History:	Unknown
// Purpose:	Summon a monster (dispellable with DISPEL).
//
void cMagic::SummonMonster(UOXSOCKET s, unsigned char id1, unsigned char id2, char * monstername, unsigned char color1, unsigned char color2, int x, int y, int z, int spellnum)
{
	//CHARACTER c;
//	CHARACTER cc = currchar[s];
	P_CHAR pc_currchar = currchar[s];
	P_CHAR pc_monster = NULL;
	int id=(id1<<8)+id2;
	UI16 color = (color1 << 8) + color2;
	switch(id)
	{
	case 0x0000:	// summon monster
 		soundeffect( s, 0x02, 0x15 );
		pc_monster = cCharStuff::createScriptNpc( s, NULL, DefManager->getRandomListEntry( "10000" ));
 		if( pc_monster == NULL )
 		{
 			sysmessage( s, "Contact your shard op to setup the summon list!" );
 			return;
 		}
		pc_monster->SetOwnSerial(pc_currchar->serial);
		pc_monster->MoveTo(pc_currchar->pos.x+rand()%2, pc_currchar->pos.y+rand()%2, pc_currchar->pos.z);
 		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
 		updatechar(pc_monster);
 		npcaction(pc_monster, 0x0C);
		doStaticEffect(pc_monster, spellnum);
 		return;

	case 0x000D: // Energy Vortex & Air elemental
		if (color1==0x00 && color2==0x75)
		{
			soundeffect(s, 0x02, 0x12); // EV
			pc_monster = cCharStuff::createScriptNpc(s,NULL,"295",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			if( pc_monster == NULL )
				return;
            pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
			pc_monster->setNpcAIType(50);
			pc_monster->setTamed(false);			
		}
		else
		{
			soundeffect(s, 0x02, 0x17); // AE
			pc_monster = cCharStuff::createScriptNpc(s,NULL,"291",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			if( pc_monster == NULL )
				return;
			pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
			pc_monster->setTamed(true);
		}
		break;
	case 0x000A: // Daemon
		soundeffect(s, 0x02, 0x16);
		pc_monster = cCharStuff::createScriptNpc(s,NULL,"290",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		if( pc_monster == NULL )
			return;
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->setTamed(true);
		break;
	case 0x000E: //Earth
		soundeffect(s, 0x02, 0x17);
		pc_monster = cCharStuff::createScriptNpc(s,NULL,"292",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		if( pc_monster == NULL )
			return;
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->setTamed(true);
		break;
	case 0x000F: //Fire
		soundeffect(s, 0x02, 0x17);
		pc_monster = cCharStuff::createScriptNpc(s,NULL,"293",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		if( pc_monster == NULL )
			return;
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->setTamed(true);
		break;
	case 0x0010: //Water
		soundeffect(s, 0x02, 0x17);
		pc_monster = cCharStuff::createScriptNpc(s,NULL,"294",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		if( pc_monster == NULL )
			return;
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->setTamed(true);
		break;
	case 0x023E: //Blade Spirits
		soundeffect(s, 0x02, 0x12); // I don't know if this is the right effect...	
		pc_monster = cCharStuff::createScriptNpc(s,NULL,"296",pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		if( pc_monster == NULL )
			return;
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->setNpcAIType(50);
		pc_monster->setTamed(false);
		break;
	case 0x03e2: // Dupre The Hero
		soundeffect(s, 0x02, 0x46);
		pc_monster = new cChar;
		pc_monster->Init();
		pc_monster->setDef(50);
		pc_monster->setLoDamage(50);
		pc_monster->setHiDamage(100);
		pc_monster->spattack=7552; // 1-7 level spells
		pc_monster->setBaseSkill(MAGERY, 900); // 90 magery
		pc_monster->setBaseSkill(TACTICS, 1000); // 70 tactics
		pc_monster->setBaseSkill(WRESTLING, 900);
		pc_monster->setBaseSkill(SWORDSMANSHIP, 1000);
		pc_monster->setBaseSkill(PARRYING, 1000);
		pc_monster->setSkill(MAGICRESISTANCE, 650);
		pc_monster->setHp(600);
		pc_monster->setSt( pc_monster->hp() );
		pc_monster->setDex(70);
		pc_monster->setStm(70);
		pc_monster->setMn(100);
		pc_monster->setIn( pc_monster->mn() );
		pc_monster->setFame(10000);
		pc_monster->setKarma(10000);
		break;
	case 0x000B: // Black Night
		soundeffect(s, 0x02, 0x16);
		pc_monster = new cChar;
		pc_monster->Init();
		pc_monster->setDef(50);
		pc_monster->setLoDamage(50);
		pc_monster->setHiDamage(100);
		pc_monster->spattack=7552; // 1-7 level spells
		pc_monster->setBaseSkill(MAGERY, 1000); // 100 magery
		pc_monster->setBaseSkill(TACTICS, 1000);// 100 tactics
		pc_monster->setBaseSkill(WRESTLING, 900);
		pc_monster->setBaseSkill(SWORDSMANSHIP, 1000);
		pc_monster->setBaseSkill(PARRYING, 1000);
		pc_monster->setSkill(MAGICRESISTANCE, 1000);
		pc_monster->setHp(600);
		pc_monster->setSt( pc_monster->hp() );
		pc_monster->setDex(70);
		pc_monster->setStm(70);
		pc_monster->setMn(100);
		pc_monster->setIn(pc_monster->mn() );
		break;
	case 0x0190: // Death Knight
		soundeffect(s, 0x02, 0x46);
		pc_monster = new cChar;
		pc_monster->Init();
		pc_monster->setDef(20);
		pc_monster->setLoDamage(10);
		pc_monster->setHiDamage(45);
		pc_monster->spattack=4095; // 1-7 level spells
		pc_monster->setBaseSkill(MAGERY, 500); // 90 magery
		pc_monster->setBaseSkill(TACTICS, 1000); // 70 tactics
		pc_monster->setBaseSkill(WRESTLING, 900);
		pc_monster->setBaseSkill(SWORDSMANSHIP, 1000);
		pc_monster->setBaseSkill(PARRYING, 1000);
		pc_monster->setSkill(MAGICRESISTANCE, 650);
		pc_monster->setHp(600);
		pc_monster->setSt( pc_monster->hp() );
		pc_monster->setDex(70);
		pc_monster->setStm(70);
		pc_monster->setMn(100);
		pc_monster->setIn( pc_monster->mn() );
		pc_monster->setFame(-10000);
		pc_monster->setKarma(-10000);
		break;
	default:
		soundeffect(s, 0x02, 0x15);
		return;
	}

	pc_monster->name = monstername;
	pc_monster->setId(id);
	pc_monster->setXid(id);
	pc_monster->setSkin(color);
	pc_monster->setXSkin(color);
	pc_monster->setPriv2(0x20);
	pc_monster->setNpc(1);

	if (id!=0x023E && !(id==0x000d && color1==0 && color2==0x75)) // don't own BS or EV.
		pc_monster->SetOwnSerial(pc_currchar->serial);

	if (x==0)
	{
		pc_monster->MoveTo(pc_currchar->pos.x-1, pc_currchar->pos.y, pc_currchar->pos.z);
	}
	else
	{
		pc_monster->MoveTo(x, y, z);
	}

	pc_monster->spadelay=10;
	pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill(MAGERY)/10)*(MY_CLOCKS_PER_SEC*2)));
	updatechar(pc_monster);
	npcaction(pc_monster, 0x0C);
	doStaticEffect(pc_monster, spellnum);

	// AntiChrist (9/99) - added the chance to make the monster attack
	// the person you targeted ( if you targeted a char, naturally :) )
	int serial=LongFromCharPtr(buffer[s]+7);
	if (serial==-1) return;
	P_CHAR pc_i = FindCharBySerial( serial );
	if(pc_i == NULL) return;

	pc_i->attackTarget( pc_monster );
}

///////////////////
// Name:	CheckBook
// History:	Unknown
// Purpose:	Check if the spell is memorized into the spellbook.
//
bool cMagic::CheckBook(int circle, int spell, P_ITEM pi)
{
	bool raflag = false;

	int spellnum = spell+(circle-1)*8;
	// Fix for OSI stupidity. :)
	if (spellnum==6) raflag = true;
	if (spellnum>=0 && spellnum<6) spellnum++;
	if (raflag) spellnum=0;

	unsigned int ci=0;
	P_ITEM pj;
	vector<SERIAL> vecContainer = contsp.getData(pi->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pj = FindItemBySerial(vecContainer[ci]);
		if((pj->id()==(0x1F2D+spellnum) || pj->id()==0x1F6D))
		{
			return false;
		}
	}
	return true;
}

int cMagic::SpellsInBook(P_ITEM pi)
{
	int spellcount = 0;
	P_ITEM pj;
	if (pi == NULL)
		return -1;
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(pi->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pj = FindItemBySerial(vecContainer[ci]);
		if (pj->id() == 0x1F6D)
			spellcount = 64;
		else
			spellcount++;
	}
	if (spellcount >= 64)
		spellcount = 64;
	return spellcount;
}


///////////////////
// Name:	SbOpenContainer
// History:	Unknown
// Purpose:	Open player's spellbook as a container.
//			(used for SBOPEN command)
//
void cMagic::SbOpenContainer(UOXSOCKET s)
{
/*	P_ITEM pi = FindItemBySerPtr(buffer[s]+7);

	if (pi)
	{
		if ((pi->type()==9))
			backpack(s, pi->serial);
		else
			sysmessage(s,"That is not a spellbook.");
	}*/
}



///////////////////
// Name:	CheckMana
// History:	Unknown, Modified by AntiChrist to use spells[] array.
// Purpose:	Check if character has enought mana to cast a spell of that circle.
//
char cMagic::CheckMana(P_CHAR pc, int num)
{

	if (pc->priv2()&0x10)
		return 1;

	if (pc->mn() >= spells[num].mana)
		return 1;
	else
	{
		UOXSOCKET p = calcSocketFromChar(pc);
		if (p != -1)
			sysmessage(p, "You have insufficient mana to cast that spell.");
		return 0;
	}
}



///////////////////
// Name:	SubtractMana
// History:	Unknown
// Purpose:	Substract the required mana from character's mana reserve.
//
char cMagic::SubtractMana4Spell(P_CHAR pc, int num)
{
	return Magic->SubtractMana(pc, spells[num].mana);
}
char cMagic::SubtractMana(P_CHAR pc, int mana)
{
	if (pc->priv2()&0x10)
		return 1;

	if (pc->mn() >= mana)
		pc->setMn( pc->mn() - mana );
	else
		pc->setMn(0);

	updatestats((pc), 1);//AntiChrist - bugfix
	return 1;
}

///////////////////
// Name:	CheckMagicReflect
// History:	Unknown
// Purpose:	Check if character is protected by MagicReflect;
//			if yes, remove the protection and do visual effect.
//
bool cMagic::CheckMagicReflect(P_CHAR pc)
{
	if (pc->priv2()&0x40)
	{
//		pc->priv2 &= 0xBF;
		pc->setPriv2(pc->priv2() & 0xBF);
		staticeffect(pc, 0x37, 0x3A, 0, 15);
		return true;
	}
	return false;
}

P_CHAR cMagic::CheckMagicReflect(P_CHAR &attacker, P_CHAR &defender)
{
	if (defender->priv2()&0x40)
	{
//		defender->priv2 &= 0xBF;
		defender->setPriv2(defender->priv2() & 0xBF);
		staticeffect(defender, 0x37, 0x3A, 0, 15);
		return attacker;
	}
	return defender;
}


///////////////////
// Name:	CheckResist
// History:	Unknown, Modified by AntiChrist to add EV.INT. check
// Purpose:	Check character's magic resistance.
//
char cMagic::CheckResist(P_CHAR pc_attacker, P_CHAR pc_defender, int circle)
{
	char i=pc_defender->checkSkill( MAGICRESISTANCE, 80*circle, 800+(80*circle));

	if (i)
	{
		//AntiChrist - 7/10/99
		//A higher Eval.Int. can disable opponent's resistspell!
		if(pc_attacker != NULL) //NOTE: only do the EV.INT. check if attacker >= 0
		{
			if(pc_attacker->skill(EVALUATINGINTEL) < pc_defender->skill(MAGICRESISTANCE) && (rand()%3)==0)
			{
				UOXSOCKET s = calcSocketFromChar(pc_defender);
				if (s!=-1)
					sysmessage(s, "You feel yourself resisting magical energy!");
			} else
				return 0;
		} else {//attacker=-1: used for checking field effects
			UOXSOCKET s = calcSocketFromChar(pc_defender);
			if (s!=-1)
				sysmessage(s, "You feel yourself resisting magical energy!");
		}
	}
	return i;
}



///////////////////
// Name:	MagicDamage
// History:	Unknown
// Purpose:	Calculate and inflict magic damage.
//
void cMagic::MagicDamage(P_CHAR pc, int amount)
{
	if( pc == NULL )
		return;

	if ( pc->priv2()&0x02  &&  pc->effDex() > 0 )
	{
//		pc->priv2 &= 0xFD; // unfreeze
		pc->setPriv2(pc->priv2() & 0xFD);
		UOXSOCKET s = calcSocketFromChar(pc);
		if (s != -1) sysmessage(s, "You are no longer frozen.");
	}

	cTerritory* Region = pc->region;
	
	if ( Region != NULL && !pc->isInvul() && Region->allowsMagicDamage() ) // LB magic-region change
	{
		if (pc->isNpc()) amount *= 2;			// double damage against non-players
//		pc->hp = QMAX(0, pc->hp-amount);
		pc->setHp( QMAX(0, pc->hp() - amount ) );
		updatestats((pc), 0);
		if (pc->hp() <= 0)
		{
			pc->kill();
		}
	}
}


///////////////////
// Name:	PoisonDamage
// History:	Unknown
// Purpose:	Apply the poison to the character.
//
void cMagic::PoisonDamage(P_CHAR pc, int poison) // new functionality, lb !!!
{
	if( pc == NULL )
		return;

	UOXSOCKET s = calcSocketFromChar(pc);

	if (pc->priv2()&0x02)
	{
//		pc->priv2 &= 0xFD;
		pc->setPriv2(pc->priv2() & 0xFD);
		if (s!=-1)
			sysmessage(s, "You are no longer frozen.");
	}

	cTerritory* Region = pc->region;
	
	if ( Region != NULL && !pc->isInvul() && Region->allowsMagicDamage() ) // LB magic-region change
	{
		if (poison>5) poison = 5;
		else if (poison<0) poison = 1;
		pc->setPoisoned(poison);
		pc->setPoisonwearofftime(uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()));
		if (s != -1)
			impowncreate(s, pc, 1); //Lb, sends the green bar !
	}
}



///////////////////
// Name:	CheckFieldEffects2
// History:	Unknown
// Purpose:	Check if character stands on a magic-field, and apply effects.
//
// timecheck: 0: always executed no matter of the nextfieldspelltime value
// timecheck: 1: only executed if the time is right for next fieldeffect check
// we need this cauz its called from npccheck and pc-check
// npc-check already has its own timer, pc check not.
// thus in npccheck its called with 0, in pc check with 1
// we could add the fieldeffect check time the server.scp but i think this solution is better.
// LB October 99
//
void cMagic::CheckFieldEffects2(unsigned int currenttime, P_CHAR pc, char timecheck)//c=character (Not socket) //Lag fix -- Zippy

{
	// - Tauriel's region stuff 3/6/99

	int j;

	if (timecheck)
	{
		if (nextfieldeffecttime<=currenttime) j=1; else j=0;
	} else j=1;

	if (j)
	{
		RegionIterator4Items ri(pc->pos);
		for ( ri.Begin(); !ri.atEnd(); ri++)
		{
			P_ITEM mapitem = ri.GetData();
			if (mapitem != NULL)
			{
				//clConsole.send("itemname: %s\n",items[mapitem].name);// perfect for mapregion debugging, LB
				if ((mapitem->pos.x==pc->pos.x)&&(mapitem->pos.y==pc->pos.y))	// lb
				{
					if (mapitem->id()==0x3996 || mapitem->id()==0x398C)
					{
						if (!CheckResist(NULL, pc, 4))
							MagicDamage(pc, mapitem->morex/100);
						else
							MagicDamage(pc, mapitem->morex/200);
						pc->soundEffect( 0x0208 );
						return; //Ripper
					} else if (mapitem->id()==0x3915 || mapitem->id()==0x3920)
					{//Poison Field
						if (!CheckResist(NULL, pc, 5))
						{
							if ((mapitem->morex<997))
								PoisonDamage(pc,2);
							else
								PoisonDamage(pc,3); // gm mages can cast greater poison field, LB
						} else PoisonDamage(pc,1); // cant be completly resited
						
						pc->soundEffect( 0x0208 );
						return; //Ripper
					}
					else if (mapitem->id()==0x3979 || mapitem->id()==0x3967)
					{//Para Field
						if (!CheckResist(NULL, pc, 6))
							tempeffect(pc, pc, 1, 0, 0, 0);
						pc->soundEffect( 0x0204 );
						return; //Ripper
					}
					break;
				}
			}
		}
	}
}

///////////////////
// Name:	BoxSpell
// History:	Unknown
// Purpose:	Calculate the spell box effect, depending on character's magery skill.
//
void cMagic::BoxSpell(UOXSOCKET s, int& x1, int& x2, int& y1, int& y2, int& z1, int& z2)
{
	int x, y, z, lenght;

	x=(buffer[s][11]<<8)+buffer[s][12];
	y=(buffer[s][13]<<8)+buffer[s][14];
	z=buffer[s][16];

	lenght=currchar[s]->skill(MAGERY)/170; // increased max-range, LB

	x1=x-lenght;
	x2=x+lenght;
	y1=y-lenght;
	y2=y+lenght;
	z1=z;
	z2=z+3;
}

///////////////////
// Name:	MagicTrap
// History:	Unknown
// Purpose:	Do the visual effect and apply magic damage when a player opens a trapped container.
//
void cMagic::MagicTrap(P_CHAR pc, P_ITEM pTrap)
{
	if (!pTrap) return;
	staticeffect(pc, 0x36, 0xB0, 0x09, 0x09);
	pc->soundEffect( 0x0207 );
	if(CheckResist(NULL, pc, 4))
		MagicDamage(pc,pTrap->moreb2());
	else
		MagicDamage(pc,pTrap->moreb2()/2);
	pTrap->setMoreb1( 0 );
	pTrap->setMoreb2( 0 );
	pTrap->setMoreb3( 0 );
}

///////////////////
// Name:	CheckReagents
// History:	Unknown, Modified by AntiChrist to use reag_st
// Purpose:	Check for required reagents in player's backpack.
//
/*char cMagic::CheckReagents(P_CHAR pc, int num)
{
	reag_st reagents = spells[num].reagents;
	reag_st failmsg = {0,};

	if (pc->priv2&0x80) return 1;

	//memset(&failmsg,0,sizeof(reag_st)); // set all members to 0

	if (reagents.ash!=0 && getamount(pc, 0x0F8C)<reagents.ash)
		failmsg.ash=1;
	if (reagents.drake!=0 && getamount(pc, 0x0F86)<reagents.drake)
		failmsg.drake=1;
	if (reagents.garlic!=0 && getamount(pc, 0x0F84)<reagents.garlic)
		failmsg.garlic=1;
	if (reagents.ginseng!=0 && getamount(pc, 0x0F85)<reagents.ginseng)
		failmsg.ginseng=1;
	if (reagents.moss!=0 && getamount(pc, 0x0F7B)<reagents.moss)
		failmsg.moss=1;
	if (reagents.pearl!=0 && getamount(pc, 0x0F7A)<reagents.pearl)
		failmsg.pearl=1;
	if (reagents.shade!=0 && getamount(pc, 0x0F88)<reagents.shade)
		failmsg.shade=1;
	if (reagents.silk!=0 && getamount(pc, 0x0F8D)<reagents.silk)
		failmsg.silk=1;

	int fail = RegMsg(pc,failmsg);

	return fail;
}

///////////////////
// Name:	RegMsg
// History:	Unknown, Reprogrammed by AntiChrist to display missing reagents types.
// Purpose:	Display an error message if character has no enougth resgs.
//
int cMagic::RegMsg(P_CHAR pc, reag_st failmsg)
{
	bool display = false;
	char message[100] = {0,};  //just to make sure we end with '\0'

	strcpy(message, "You do not have enough reagents to cast that spell.[");

	if (failmsg.ash)	{ display = true; strcat(message,"Sa,"); }
	if (failmsg.drake)	{ display = true; strcat(message,"Mr,"); }
	if (failmsg.garlic)	{ display = true; strcat(message,"Ga,"); }
	if (failmsg.ginseng){ display = true; strcat(message,"Gi,"); }
	if (failmsg.moss)	{ display = true; strcat(message,"Bm,"); }
	if (failmsg.pearl)	{ display = true; strcat(message,"Bp,"); }
	if (failmsg.shade)	{ display = true; strcat(message,"Ns,"); }
	if (failmsg.silk)	{ display = true; strcat(message,"Ss,"); }

	message[strlen(message)-1]=']';

	if (display)
	{
		UOXSOCKET i = calcSocketFromChar(pc);
		if (i != -1)
			sysmessage(i, message);
		return 0;
	}

	return 1;
}*/


///////////////////
// Name:	PFireballTarget
// History:	Unknown
// Purpose:	Calculate and inflict a magic damage.
//
void cMagic::PFireballTarget(P_CHAR pc_i, P_CHAR pc, int j) //j = % dammage
{
	int dmg;
	movingeffect(pc_i, pc, 0x36, 0xD5, 0x05, 0x00, 0x01);
	pc_i->soundEffect( 0x015E );
	// do we have to calculate attacker hp percentage,
	// or defender hp percentage?!?!?!
	dmg=(int)(((float)pc->hp()/100) * j);
	MagicDamage(pc, dmg);
}

///////////////////
// Name:	SpellFail
// History:	Unknown
// Purpose:	Do visual and sound effects when a player fails to cast a spell.
//
void cMagic::SpellFail(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	//Use Reagents on failure ( if casting from spellbook )
	if (currentSpellType[s]==0)
		DelReagents( pc_currchar, pc_currchar->spell() );

	//npcaction(cc, 128); // whaaaaaaaaaaaaaat ?
	//orders the PG to move a step on, but the pg doesn't really move
	//disappearing from the other clients. solarin
	
	if ( rand()%5==2 )
		doStaticEffect(pc_currchar, 99);
	else
		staticeffect(pc_currchar, 0x37, 0x35, 0, 30);
	pc_currchar->soundEffect( 0x005C );
	npcemote(s, pc_currchar, "The spell fizzles.",0);
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////// NPCs CASTING SPELLS RELATED FUNCTIONS ///////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


///////////////////
// Name:	NPCLightningTarget
// History:	Unknown
// Purpose:	Used for NPC; cast a lightning spell.
//
void cMagic::LightningSpell(P_CHAR pc_Attacker, P_CHAR pc_Defender, bool usemana)
{
	if ( pc_Attacker == NULL || pc_Defender == NULL)
		return;
	P_CHAR pc_trg = CheckMagicReflect(pc_Attacker, pc_Defender);
	
	if (usemana)
		SubtractMana(pc_Attacker, 11);
	bolteffect(pc_trg, true);
	pc_trg->soundEffect( 0x0029 );

	if (CheckResist(pc_Attacker, pc_trg, 4))
	{
		MagicDamage(pc_trg, pc_Attacker->skill(MAGERY)/180+RandomNum(1,2));
		//MagicDamage(t, (2+(rand()%3)+1)*(pc_currchar->skill(MAGERY)/1000+1));
	}
	else
	{
		MagicDamage(pc_trg, (pc_Attacker->skill(MAGERY)+1*(pc_Attacker->skill(EVALUATINGINTEL)/3))/(89+1*(pc_trg->skill(MAGICRESISTANCE)/30))+RandomNum(1,5));
		//MagicDamage(t, (4+(rand()%5)+2)*(pc_currchar->skill(MAGERY)/750+1));
	}
	return;
}

void cMagic::NPCHeal(P_CHAR pc)
{
    int loskill=spells[10].loskill;
    int hiskill=spells[10].hiskill;
	if ( pc == NULL ) return;

	if (!pc->checkSkill( MAGERY, loskill, hiskill))
	{
		UOXSOCKET ss=calcSocketFromChar(pc);
		if (ss>-1)
		{
			SpellFail(ss);
		}
		return;
	}
	if (CheckMana(pc,10))
	{
		SubtractMana(pc, 10);
		int j=pc->hp()+(pc->skill(MAGERY)/30+RandomNum(1,12));
		pc->setHp( QMIN(pc->st(), static_cast<signed short>(j)) );
		doStaticEffect(pc, 4);
		updatestats(pc, 0);
	}
}

void cMagic::NPCCure(P_CHAR pc)
{
    int loskill=spells[11].loskill;
    int hiskill=spells[11].hiskill;

	if ( pc == NULL ) return;

	if (!pc->checkSkill( MAGERY, loskill, hiskill))
	{
		UOXSOCKET ss=calcSocketFromChar(pc);
		if (ss>-1)
		{
			SpellFail(ss);
		}
		return;
	}
	if (CheckMana(pc,11))
	{
		doStaticEffect(pc, 11);
		SubtractMana(pc,5);
		pc->setPoisoned(0);
		pc->setPoisonwearofftime(uiCurrentTime);
		pc->emote("Laughs at the poison attempt");
	}

}

void cMagic::NPCDispel(P_CHAR pc_s, P_CHAR pc_i)
{
	int loskill=spells[41].loskill;
	int hiskill=spells[41].hiskill;
	if ( pc_s == NULL)
		return;
	if (!pc_s->checkSkill( MAGERY, loskill, hiskill))
	{
		UOXSOCKET ss = calcSocketFromChar(pc_s);
		if (ss>-1)
		{
			SpellFail(ss);
		}
		return;
	}
	if (CheckMana(pc_s, 41))
	{
		if ( pc_i == NULL)
			return;
		if (pc_i->priv2()&0x20)
		{
			SubtractMana(pc_s,20);
			tileeffect(pc_i->pos.x,pc_i->pos.y,pc_i->pos.z, 0x37, 0x2A, 0x00, 0x00);
			if (pc_i->isNpc())
				cCharStuff::DeleteChar(pc_i);
			else pc_i->kill();
		}
	}
}

///////////////////
// Name:	NPCEBoltTarget
// History:	Unknown
// Purpose:	Used for NPC; cast a energy bolt spell.
//
void cMagic::EnergyBoltSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 20);
	doMoveEffect(42, pc_target, pc_defender);
	pc_target->soundEffect( 0x020A );

	if (CheckResist(pc_attacker, pc_defender, 6))
	{
		MagicDamage(pc_target, pc_attacker->skill(MAGERY)/120);
		//MagicDamage(t, (4+(rand()%4)+2)*(pc_currchar->skill(MAGERY)/1000+1));
	}
	else
	{
		MagicDamage(pc_target, (pc_attacker->skill(MAGERY)+1*(pc_attacker->skill(EVALUATINGINTEL)/3))/(34+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,10));
		//MagicDamage(t, (8+(rand()%5)+4)*(pc_currchar->skill(MAGERY)/750+1));
	}
	return;
}

///////////////////
// Name:	CheckParry
// History:	Unknown
// Purpose:	Check player's parrying skill (for cannonblast).
//
char cMagic::CheckParry(P_CHAR pc_player, int circle)
{
	char i=pc_player->checkSkill( PARRYING, 80*circle, 800+(80*circle));
	if(i)
	{
		UOXSOCKET s = calcSocketFromChar(pc_player);
		if (s!=-1)
		{
			sysmessage(s, "You have dodged the cannon blast, and have taken less damage.");
		}
	}
	return i;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////// ITEMs MAGIC POWER RELATED FUNCTIONS /////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


///////////////////
// Name:	MagicArrowSpellItem
// History:	Modifyed by Correa to be used by both Items and Players
// Purpose:	Cast magic arrow
//
void cMagic::MagicArrow(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	doMoveEffect(5, pc_target, pc_defender);
	pc_target->soundEffect( 0x01E5 );
	if (usemana)
		SubtractMana(pc_attacker, 4);
	if (CheckResist(pc_attacker, pc_target, 1))
	{
		MagicDamage(pc_target, (1+(rand()%1)+1)*(pc_attacker->skill(MAGERY)/2000+1));
	}
	else
	{
		MagicDamage(pc_target, (1+(rand()%1)+2)*(pc_attacker->skill(MAGERY)/1500+1));
	}
	return;
}

///////////////////
// Name:	ClumsySpell
// History:	Modifyed by Correa to be used by both Items and Chars
// Purpose:	cast a clumsy spell.
//
void cMagic::ClumsySpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;

	CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 4);

	doStaticEffect(pc_defender, 1);
	pc_defender->soundEffect( 0x01DF );
	if (CheckResist(pc_attacker, pc_defender, 1)) return;
	tempeffect(pc_attacker, pc_defender, 3, pc_attacker->skill(MAGERY)/100, 0, 0);
	return;
}

///////////////////
// Name:	FeebleMindSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a feeblemind spell.
//
void cMagic::FeebleMindSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{

	if ( pc_attacker == NULL || pc_defender == NULL)
		return;

	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 4);

	doStaticEffect(pc_target, 3);
	pc_target->soundEffect( 0x01E4 );
	if (CheckResist(pc_attacker, pc_target, 1)) return;
	tempeffect(pc_attacker, pc_target, 4, pc_attacker->skill(MAGERY)/100, 0, 0);
	return;
}

///////////////////
// Name:	WeakenSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a weaken spell.
//
void cMagic::WeakenSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 4);
	
	doStaticEffect(pc_target, 8);
	pc_target->soundEffect( 0x01E6 );
	if(CheckResist(pc_attacker, pc_target, 1)) return;
	tempeffect(pc_attacker, pc_target, 5, pc_attacker->skill(MAGERY)/100, 0, 0);
	return;
}

///////////////////
// Name:	HarmSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a harm spell.
//
void cMagic::HarmSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 6);

	doStaticEffect(pc_target, 12);
	pc_target->soundEffect( 0x01F1 );
	if (CheckResist(pc_attacker, pc_target, 2))
	{
		MagicDamage(pc_defender, pc_attacker->skill(MAGERY)/500+1);
	}
	else
	{
		MagicDamage(pc_defender, pc_attacker->skill(MAGERY)/250+RandomNum(1,2));
	}
	return;
}


///////////////////
// Name:	FireballSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a fireball spell.
//
void cMagic::FireballSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 9);

	doMoveEffect(18, pc_attacker, pc_target);
	pc_attacker->soundEffect( 0x015E );
	if (CheckResist(pc_attacker, pc_target, 3))
		MagicDamage(pc_target, pc_attacker->skill(MAGERY)/280+1);
	else
		MagicDamage(pc_target, (pc_attacker->skill(MAGERY)+1*(pc_attacker->skill(EVALUATINGINTEL)/3))/(139+1*(pc_target->skill(MAGICRESISTANCE)/30))+RandomNum(1,4));
	return;
}

///////////////////
// Name:	CurseSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a curse spell.
//
void cMagic::CurseSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	int j;

	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 11);

	doStaticEffect(pc_target, 27);
	pc_target->soundEffect( 0x01E1 );
	if(CheckResist(pc_attacker, pc_target, 1)) return;
	j = pc_attacker->skill(MAGERY)/100;
	tempeffect(pc_attacker, pc_target, 12, j, j, j);
	return;
}


///////////////////
// Name:	MindBlastSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a mindblast spell.
//
void cMagic::MindBlastSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 14);

	doStaticEffect(pc_target, 37);
	pc_target->soundEffect( 0x0213 );
	if (pc_attacker->in()>pc_target->in())
	{
		if (CheckResist(pc_attacker, pc_target, 5))
		{
			MagicDamage(pc_target, (pc_attacker->in()-pc_target->in())/4);
		}
		else
		{
			MagicDamage(pc_target, (pc_attacker->in()-pc_target->in())/2);
		}
	}
	else
	{
		if (CheckResist(pc_defender, pc_target, 5))
		{
			MagicDamage(pc_attacker, (pc_target->in()-pc_attacker->in())/4);
		}
		else
		{
			MagicDamage(pc_attacker, (pc_target->in()-pc_attacker->in())/2);
		}
	}
	return;
}


///////////////////
// Name:	ParalyzeSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a paralyze spell.
//
void cMagic::ParalyzeSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 14);

	if (CheckResist(pc_attacker, pc_target, 5)) return;
	doStaticEffect(pc_target, 38);
	pc_target->soundEffect( 0x0204 );
	tempeffect(pc_attacker, pc_target, 1, 0, 0, 0);
	return;
}


///////////////////
// Name:	ExplosionSpell
// History:	Modified by Correa to be used by both Items and Chars
// Purpose:	Used for ITEMS; cast a explosion spell.
//
void cMagic::ExplosionSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 20);

	
	doStaticEffect(pc_target, 43);
	pc_target->soundEffect( 0x0207 );
	if (CheckResist(pc_attacker, pc_target, 6))
	{
		MagicDamage(pc_target, pc_attacker->skill(MAGERY)/120+RandomNum(1,5));
	}
	else
	{
		MagicDamage(pc_target, pc_attacker->skill(MAGERY)/40+RandomNum(1,10));
	}
	return;
}

///////////////////
// Name:	FlameStrikeSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a flamestrike spell.
//
void cMagic::FlameStrikeSpell(P_CHAR pc_attacker, P_CHAR pc_defender, bool usemana)
{
	if ( pc_attacker == NULL || pc_defender == NULL)
		return;
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	if (usemana)
		SubtractMana(pc_attacker, 40);
	doStaticEffect(pc_target, 51);
	pc_target->soundEffect( 0x0208 );

	if (CheckResist(pc_attacker, pc_defender, 7))
	{
		MagicDamage(pc_target, pc_attacker->skill(MAGERY)/80);
		//MagicDamage(t, (3+(rand()%11)+1)*(chars[cc].skill[MAGERY]/1000+1));
	}
	else
	{
		MagicDamage(pc_target, (pc_attacker->skill(MAGERY)+1*(pc_attacker->skill(EVALUATINGINTEL)/3))/(34+1*(pc_target->skill(MAGICRESISTANCE)/30))+RandomNum(1,25));
		//MagicDamage(t, (10+(rand()%12)+1)*(chars[cc].skill[MAGERY]/750+1));
	}
	return;
}


/////////////////////////////////////////////////////////////////
/////// PCs CASTING SPELLS RELATED FUNCTIONS ////////////////////
/////////////////////////////////////////////////////////////////

void cMagic::preParticles(int num, P_CHAR pc)
{
    stat_st t;
	t = cMagic::getStatEffects_pre(num);
	if( t.effect[4] != -1 && t.effect[5] != -1 && t.effect[6] != -1 && t.effect[7] != -1 )
	staticeffect(pc, NOTUSED, NOTUSED, NOTUSED, NOTUSED, true, &t, true); 			 			  		
}

void cMagic::afterParticles(int num, P_CHAR pc)
{
    stat_st t;
	t = cMagic::getStatEffects_after(num);
	if( t.effect[4] != -1 && t.effect[5] != -1 && t.effect[6] != -1 && t.effect[7] != -1 )
	staticeffect(pc, NOTUSED, NOTUSED, NOTUSED, NOTUSED, true, &t, true);
}

void cMagic::itemParticles(int num, P_ITEM pi)
{
    stat_st t;
	t = cMagic::getStatEffects_item(num);
	if( t.effect[4] != -1 && t.effect[5] != -1 && t.effect[6] != -1 && t.effect[7] != -1 )
	staticeffect2(pi, NOTUSED, NOTUSED, NOTUSED, NOTUSED, NOTUSED, true, &t, true);
}

void cMagic::NewCastSpell( UOXSOCKET s )
{
	// for LocationTarget spell like ArchCure, ArchProtection etc...
	int	j, x1, x2, y1, y2, z1, z2, range, distx, disty;
	int	StartGrid;
	int	getcell;
	int dmg, dmgmod;
	
	unsigned int increment;
	unsigned int checkgrid;
	
	int loskill, hiskill;//AntiChrist moved here
	P_CHAR pc_currchar = currchar[s];
	int curSpell = pc_currchar->spell();
	short xo,yo;
	signed char zo;
	
	int calcreg;
	int defender;
	unsigned long loopexit = 0;
	int a;
	double d;
	bool recalled;
	int n;
	int dx, dy, dz;
	int fx[5], fy[5]; // bugfix LB, was fx[4] ...
	unsigned char id1, id2;	
	int snr;
	bool char_selected, item_selected, terrain_selected;
	
	cTerritory* Region = cAllTerritories::getInstance()->region( pc_currchar->pos.x, pc_currchar->pos.y );

	if (pc_currchar->dead())
		return;
	if (currentSpellType[s]==0)
	{
		loskill=spells[curSpell].loskill;
		hiskill=spells[curSpell].hiskill;
		if (!(pc_currchar->isGM()) && !pc_currchar->checkSkill( MAGERY, loskill, hiskill))
		{
			SpellFail(s);
			pc_currchar->setSpell( 0 );
			pc_currchar->setCasting(false);
			return;
		}
	}
	if (currentSpellType[s]!=2) SubtractMana(pc_currchar, spells[curSpell].mana);
	
	if (currentSpellType[s]==0) DelReagents(pc_currchar, curSpell );
	
	if( requireTarget( curSpell ) )					// target spells if true
	{
		if( travelSpell( curSpell )	)				// travel spells.... mark, recall and gate
		{
			// mark, recall and gate go here
			P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
			cMulti* pi_multi = cMulti::findMulti( pc_currchar->pos );		
			
			if(pi)
			{
				if( !pi->isInWorld() || lineOfSight( pc_currchar->pos, pi->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) || pc_currchar->isGM() ) // bugfix LB
				{
					if ((pi->type()==50))
					{
						playSound( pc_currchar, curSpell );
						doStaticEffect( pc_currchar, curSpell );
						switch( curSpell )
						{
							//////////// (32) RECALL ////////////////
						case 32:
							if ( pi->morex<=200 && pi->morey<=200 )
							{
								sysmessage(s,"That rune has not been marked yet!");
								recalled = false;
							}
							else if (!pc_currchar->isGM()) //Morrolan no recall if too heavy, GM's excempt
							{
								/*sysmessage(s, "You are too heavy to do that!");
								sysmessage(s, "You feel drained from the attempt.");
								statwindow(s, pc_currchar);*/
								recalled = false;
							}
							else
							{										
								xo=pc_currchar->pos.x;
								yo=pc_currchar->pos.y;
								zo=pc_currchar->pos.z;											
								
								cMagic::invisibleItemParticles(pc_currchar, curSpell, xo, yo, zo);
								
								pc_currchar->MoveTo(pi->morex,pi->morey,pi->morez); //LB
								teleport((pc_currchar));
								doStaticEffect( pc_currchar, curSpell );
								sysmessage(s,"You have recalled from the rune.");
								recalled = true;
							}
							break;
							//////////// (45) MARK //////////////////
						case 45:
							if (pi_multi != NULL)
							{	
				                if (IsHouse(pi_multi->id()))
								{
					                sysmessage(s, "You can`t mark a rune in a house!");
				                    return;
								}
							}
							pi->morex=pc_currchar->pos.x;
							pi->morey=pc_currchar->pos.y;
							pi->morez=pc_currchar->pos.z;
							sysmessage(s,"Recall rune marked.");
							//antichrist
							if( Region != NULL )
								pi->setName( QString( "Rune to: %1" ).arg( Region->name() ) );
							
							cMagic::invisibleItemParticles(pc_currchar, curSpell, pc_currchar->pos.x, pc_currchar->pos.y, pc_currchar->pos.z);						
							
							break;
							//////////// (52) GATE //////////////////
						case 52:
							if ( pi->morex<=200 && pi->morey<=200 )
							{
								sysmessage(s,"That rune has not been marked yet!");
								recalled=false;
							}
							else
							{
								gatex[gatecount][0]=pc_currchar->pos.x;	//create gate a player location
								gatey[gatecount][0]=pc_currchar->pos.y;
								gatez[gatecount][0]=pc_currchar->pos.z;
								gatex[gatecount][1]=pi->morex; //create gate at marked location
								gatey[gatecount][1]=pi->morey;
								gatez[gatecount][1]=pi->morez;
								recalled=true;
								
								for (n=0;n<2;n++)
								{
									strcpy((char*)temp,"a blue moongate");
									P_ITEM pi_c = Items->SpawnItem(-1, pc_currchar, 1,"#",0,0x0f,0x6c,0,0,0);
									if(pi_c != NULL)	//AntiChrist - to prevent crashes
									{
										pi_c->setType( 51+n );
										pi_c->pos.x=gatex[gatecount][n];
										pi_c->pos.y=gatey[gatecount][n];
										pi_c->pos.z=gatez[gatecount][n];
										pi_c->gatetime = static_cast<unsigned int>(uiCurrentTime+(SrvParams->gateTimer()*MY_CLOCKS_PER_SEC));
										//clConsole.send("GATETIME:%i UICURRENTTIME:%d GETCLOCK:%d\n",SrvParms->gatetimer,uiCurrentTime,getclock());
										pi_c->gatenumber=gatecount;
										pi_c->dir=1;
										
										cMapObjects::getInstance()->add(pi_c);	//add gate to list of items in the region
										pi_c->update();//AntiChrist
									}
									if (n==1)
									{
										gatecount++;
										if (gatecount>MAXGATES) gatecount=0;
									}
									addid1[s]=0;
									addid2[s]=0;
								}
							}
							break;
						default:
							clConsole.send("MAGIC-ERROR: Unknown Travel spell %i, magic.cpp\n", curSpell );
							break;
						}
					}
					else
						sysmessage( s, "That item is not a recall rune." );
				}
			}
			else
				sysmessage( s, "Not a valid target on item!" );
			pc_currchar->setSpell( 0 );
			return;
		}
		
		/////////////////////////////////////////////////////////
		// spell code goes starts for targetted spells
		/////////////////////////////////////////////////////////
		
		if( reqCharTarget( curSpell ) )
		{
			// TARGET CALC HERE
			defender=LongFromCharPtr(buffer[s]+7);	// character we are attacking is HERE
			P_CHAR pc_defender = FindCharBySerial( defender );
			// IF TARGET VALID
			if (pc_defender != NULL)				// we have to have targetted a person to kill them :)
			{
				if( chardist( pc_defender, pc_currchar ) > SrvParams->attack_distance() )
				{
					sysmessage( s, "You can't cast on someone that far away!" );
					return;
				}
				if ((lineOfSight( pc_currchar->pos, pc_defender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
					(pc_currchar->isGM())))
				{
					if( aggressiveSpell( curSpell ) )
					{
						if ((pc_defender->isInnocent()) && (pc_defender != pc_currchar)&& !pc_currchar->Owns(pc_defender)&&(!GuildCompare(pc_defender, pc_currchar)) )
						{
							criminal(pc_currchar);
						}
						if (pc_defender->npcaitype()==17) // Ripper 11-14-99
						{
							sysmessage(s," They are Invulnerable merchants!");
							return;
						}
						pc_defender->attackTarget( pc_currchar );
					}
					if( spellReflectable( curSpell ) )
					{
						//clConsole.send("spell-reflectable\n");
						pc_defender = CheckMagicReflect(pc_currchar, pc_defender);
					}
					
					playSound( pc_currchar, curSpell );
					doMoveEffect( curSpell, pc_defender, pc_currchar );
					if (curSpell !=30 && curSpell !=58) doStaticEffect( pc_defender, curSpell );
					
					switch( curSpell )
					{
						//////////// (1) CLUMSY /////////////////
					case 1:
						tempeffect( pc_currchar, pc_defender, 3, pc_currchar->skill(MAGERY)/100, 0, 0);
						break;
						//////////// (3) FEEBLEMIND /////////////
					case 3:
						tempeffect( pc_currchar, pc_defender, 4, pc_currchar->skill(MAGERY)/100, 0, 0);
						break;
						//////////// (4) HEAL ///////////////////
					case 4:
						int m,h;
						m=pc_currchar->skill(MAGERY);
						if ((pc_currchar->isInnocent()) &&(pc_currchar->serial != pc_defender->serial))
						{
							if ((pc_defender->crimflag()>0) ||(pc_defender->isMurderer()))
							{
								criminal(pc_currchar);
							}
						}
						
						if (m<100) h=1; else
							if (m>=100 && m<200) h=2; else
								if (m>=200 && m<300) h=4; else
									if (m>=300 && m<400) h=5; else
										if (m>=400) h=6;
										if (h>2) h+=rand()%3;
										
										pc_defender->setHp(pc_defender->hp()+h);
										updatestats((pc_defender), 0);
										
										break;
										//////////// (5) MAGIC ARROW ////////////
					case 5:
						MagicDamage(pc_defender, (1+(rand()%1)+1)*(pc_currchar->skill(MAGERY)/2000+1));
						break;
						//////////// (6) NIGHT SIGHT ////////////
					case 6:
						tempeffect( pc_currchar, pc_defender, 2, 0, 0, 0);
						break;
						//////////// (7) REACTIVE ARMOR /////////
					case 7:
						if (Skills->GetAntiMagicalArmorDefence(pc_currchar)>10)
						{
							sysmessage(s,"Spell fails due to the armor on target!");
							break;
						}
						tempeffect( pc_currchar, pc_defender, 15, pc_currchar->skill(MAGERY)/100, 0, 0 );
						pc_defender->setRa(1);
						break;
						//////////// (8) WEAKEN /////////////////
					case 8:
						tempeffect( pc_currchar, pc_defender, 5, pc_currchar->skill(MAGERY)/100, 0, 0);
						break;
						//////////// (9) AGILITY ////////////////
					case 9:
						tempeffect( pc_currchar, pc_defender, 6, pc_currchar->skill(MAGERY)/100, 0, 0);
						break;
						//////////// (10) CUNNING ///////////////
					case 10:
						tempeffect( pc_currchar, pc_defender, 7, pc_currchar->skill(MAGERY)/100, 0, 0);
						break;
						//////////// (11) CURE //////////////////
					case 11:
						pc_defender->setPoisoned(0);
						pc_defender->setPoisonwearofftime(uiCurrentTime);
						impowncreate(s, pc_defender, 1); // updating to blue bar
						break;
						//////////// (12) HARM //////////////////
					case 12:
						if (CheckResist(pc_currchar, pc_defender, 2))
							MagicDamage(pc_defender, pc_currchar->skill(MAGERY)/500+1);
						else
							MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(249+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,2));
						break;
						//////////// (15) PROTECTION ////////////
					case 15:
						if (Skills->GetAntiMagicalArmorDefence(pc_currchar)>10)
						{
							sysmessage(s,"Spell fails due to the armor on target!");
							break;
						}
						tempeffect( pc_currchar, pc_defender, 21, pc_currchar->skill(MAGERY)/100, 0, 0 );
						break;
						//////////// (16) STRENGTH //////////////
					case 16:
						tempeffect( pc_currchar, pc_defender, 8, pc_currchar->skill(MAGERY)/100, 0, 0);
						break;
						//////////// (17) BLESS /////////////////
					case 17:
						j=pc_currchar->skill(MAGERY)/100;
						tempeffect( pc_currchar, pc_defender, 11, j, j, j);
						break;
						//////////// (18) FIREBALL //////////////
					case 18:
						if (CheckResist(pc_currchar, pc_defender, 3))
							MagicDamage(pc_defender, pc_currchar->skill(MAGERY)/280+1);
						else
							MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(139+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,4));
						break;
						//////////// (20) POISON ////////////////
					case 20:
						if(CheckResist(pc_currchar, pc_defender, 1)) return;
						{
							pc_defender->setPoisoned(2);
							pc_defender->setPoisonwearofftime(uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer())); // LB
							impowncreate( s, pc_defender, 1); //Lb, sends the green bar !
						}
						break;
						//////////// (27) CURSE /////////////////
					case 27:
						if(!CheckResist(pc_currchar, pc_defender, 1))
						{
							j=pc_currchar->skill(MAGERY)/100;
							tempeffect(pc_currchar, pc_defender, 12, j, j, j);
						}
						break;
						//////////// (29) GREATER HEAL //////////
					case 29:
						if ((pc_currchar->isInnocent()) &&(pc_currchar->serial != pc_defender->serial))
						{
							if ((pc_defender->crimflag()>0) ||(pc_defender->isMurderer()))
							{
								criminal(pc_currchar);
							}
						}
						j=pc_defender->hp()+(pc_currchar->skill(MAGERY)/30+RandomNum(1,12));
						pc_defender->setHp(QMIN(pc_defender->st(), static_cast<signed short>(j)) );
						updatestats((pc_defender), 0);
						break;
						//////////// (30) LIGHTNING /////////////
					case 30:
						bolteffect(pc_defender, true);
						if (CheckResist(pc_currchar, pc_defender, 4))
							MagicDamage(pc_defender, pc_currchar->skill(MAGERY)/180+RandomNum(1,2));
						else
							MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(89+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,5));
						break;
						//////////// (31) MANA DRAIN ////////////
					case 31:
						if(!CheckResist(pc_currchar, pc_defender, 4))
						{
//							pc_defender->mn-=pc_currchar->skill(MAGERY)/35;
							pc_defender->setMn(pc_defender->mn() - pc_currchar->skill(MAGERY)/35);
							if (pc_defender->mn()<0) pc_defender->setMn(0);
							updatestats((pc_defender), 1);
						}
						break;
						//////////// (37) MIND BLAST ////////////
					case 37:
						if (pc_currchar->in()>pc_defender->in())
						{
							if (CheckResist(pc_currchar, pc_defender, 5))
								MagicDamage(pc_defender, (pc_currchar->in()-pc_defender->in())/4);
							else
								MagicDamage(pc_defender, (pc_currchar->in()-pc_defender->in())/2);
						}
						else
						{
							if (CheckResist(pc_currchar, pc_currchar, 5))
								MagicDamage(pc_currchar, (pc_defender->in()-pc_currchar->in())/4);
							else
								MagicDamage(pc_currchar, (pc_defender->in()-pc_currchar->in())/2);
						}
						cMagic::afterParticles(37, pc_currchar);
						break;
						//////////// (38) PARALYZE //////////////
					case 38:
						if (!CheckResist(pc_currchar, pc_defender, 7))
							tempeffect(pc_currchar, pc_defender, 1, 0, 0, 0);
						break;
						//////////// (41) DISPEL ////////////////
					case 41:
						if (pc_defender->priv2()&0x20)
						{
							tileeffect(pc_defender->pos.x,pc_defender->pos.y,pc_defender->pos.z, 0x37, 0x2A, 0x00, 0x00);
							if (pc_defender->isNpc())
								cCharStuff::DeleteChar(pc_defender);
							else
								pc_defender->kill();
						}
						break;
						//////////// (42) ENERGY BOLT ///////////
					case 42:
						if (CheckResist(pc_currchar, pc_defender, 6))
							MagicDamage(pc_defender, pc_currchar->skill(MAGERY)/120);
						else
							MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(34+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,10));						
						
						break;
						//////////// (43) EXPLOSION /////////////
					case 43:
						if (CheckResist(pc_currchar, pc_defender, 6))
							MagicDamage(pc_defender, pc_currchar->skill(MAGERY)/120+RandomNum(1,5));
						else
							MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(39+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,10));
						break;
						//////////// (44) INVISIBILITY //////////
					case 44:						
						
						cMagic::invisibleItemParticles(pc_defender, curSpell, pc_defender->pos.x, pc_defender->pos.y, pc_defender->pos.z+12);
						
						pc_defender->setHidden( 2 );
						updatechar(pc_defender);
						pc_defender->setInvisTimeout( uiCurrentTime+(SrvParams->invisTimer()*MY_CLOCKS_PER_SEC) );
						break;
						//////////// (51) FLAMESTRIKE ///////////
					case 51:
						if (CheckResist(pc_currchar, pc_defender, 7))
							MagicDamage(pc_defender, pc_currchar->skill(MAGERY)/80);
						else
							MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(34+1*(pc_defender->skill(MAGICRESISTANCE)/30))+RandomNum(1,25));
						break;
						//////////// (53) MANA VAMPIRE //////////
					case 53:
						if(!CheckResist(pc_currchar, pc_defender, 7))
						{
							if (pc_defender->mn()<40)
							{
//								pc_currchar->mn += pc_defender->mn;
								pc_currchar->setMn(pc_currchar->mn() + pc_defender->mn());
								pc_defender->setMn(0);
							}
							else
							{
//								pc_defender->mn-=40;
								pc_defender->setMn(pc_defender->mn() - 40);
//								pc_currchar->mn+=40;
								pc_currchar->setMn(pc_currchar->mn() + 40);
							}
							updatestats(pc_defender, 1);
							updatestats(pc_currchar, 1);
						}
						break;
						//////////// (59) RESURRECTION //////////
					case 59:
						if (pc_defender->dead() && online(pc_defender))
						{
							cMagic::doStaticEffect(pc_defender, curSpell);
							Targ->NpcResurrectTarget(pc_defender);		
							cMagic::invisibleItemParticles(pc_defender, curSpell, pc_defender->pos.x, pc_defender->pos.y, pc_defender->pos.z);
							return;
						}
						else if (!pc_defender->dead()) sysmessage(s,"That player isn't dead!");
						else sysmessage(s,"That player isn't online!");
						break;
						//////////// (66) CANNON FIRING /////////
					case 66:
						if (CheckParry(pc_defender, 6))
							MagicDamage(pc_defender, pc_currchar->skill(TACTICS)/50);
						else
							MagicDamage(pc_defender, pc_currchar->skill(TACTICS)/25);
						break;
					default:
						clConsole.send("MAGIC-ERROR: Unknown CharacterTarget spell %i, magic.cpp\n", curSpell );
						break;
					}
				}
				else
				{
					sysmessage(s,"There seems to be something in the way.");
				}
			}
			else
				sysmessage(s,"That is not a person.");
		}
		else if( reqLocTarget( curSpell ) )
		{
			// field spells mostly go here
			Coord_cl targetLocation((buffer[s][11]<<8)+buffer[s][12], (buffer[s][13]<<8)+buffer[s][14], buffer[s][16]+cTileCache::instance()->tileHeight((buffer[s][17]<<8)+buffer[s][18]), currchar[s]->pos.map); 
			int x=fx[0]=(buffer[s][11]<<8)+buffer[s][12];
			int y=fy[0]=(buffer[s][13]<<8)+buffer[s][14];
			//z=buffer[s][16];
			int z=buffer[s][16]+cTileCache::instance()->tileHeight((buffer[s][17]<<8)+buffer[s][18]); // bugfix, LB
			
			defender=LongFromCharPtr(buffer[s]+7);
			P_CHAR pc_i = FindCharBySerial( defender );
			P_ITEM pi_l = FindItemBySerial( defender );
			
			// ..[1] 0-> object or item selected, 1-> terrain selected
			char_selected=item_selected=terrain_selected=false;
			if      (pc_i != NULL && buffer[s][1]==0)      char_selected=true;
            else if (pi_l != NULL && buffer[s][1]==0)      item_selected=true;
			else						                   terrain_selected=true;
			
			//AntiChrist - location check
			if (!(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF))
			{
				//Coord_cl clTemp1(x,y,z);
				
				if ((lineOfSight( pc_currchar->pos, targetLocation,WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
					(pc_currchar->isGM())))
				{
					if( fieldSpell( curSpell ) )
					{
						//AntiChrist - fixed the fields missing middle piece - 9/99
						//(changed the order of fy and fy also!)
						j = fielddir(pc_currchar, targetLocation.x, targetLocation.y, targetLocation.z); // lb bugfix, socket char# confusion
						if (j)
						{
							fx[0]=fx[1]=fx[2]=fx[3]=fx[4]=x;
							fy[0]=y;
							fy[1]=y+1;
							fy[2]=y-1;
							fy[3]=y+2;
							fy[4]=y-2;
						}
						else
						{
							fy[0]=fy[1]=fy[2]=fy[3]=fy[4]=y;
							fx[0]=x;
							fx[1]=x+1;
							fx[2]=x-1;
							fx[3]=x+2;
							fx[4]=x-2;
						}	// end else
					}	// end if
					
					playSound( pc_currchar, curSpell );
					if (curSpell != 33 && curSpell !=58) doStaticEffect( pc_currchar, curSpell );
					
					RegionIterator4Chars ri(pc_currchar->pos);
					Coord_cl clTemp2(x,y,z) ;
					
					switch( curSpell )
					{
						//////////// (22) TELEPORT //////////////////
					case 22:
						{
							
							if (char_selected) { sysmessage(s, "You can't use that spell on characters");  return; }
							if (item_selected) { sysmessage(s, "You can't teleport there.");  return; }
							
							cMulti* pi_multi = cMulti::findMulti( Coord_cl(x, y, z) );
							if( !pi_multi )
							{
								sysmessage( s, "You can't teleport here!" );
								return;
							}
							tile_st tile = cTileCache::instance()->getTile( ((buffer[s][0x11]<<8)+buffer[s][0x12]) );
							if( (!strcmp((char *)tile.name, "water")) || (tile.flag1&0x80) )
							{
								sysmessage(s,"Give up wanabe Jesus !");
								return;
							}
							if (!Movement->canLandMonsterMoveHere(targetLocation))
							{
								 switch(RandomNum(0,4))
								 {
								 case 0: sysmessage(s,"Thats a no teleport zone!"); break;
								 case 1: sysmessage(s,"Thats not possible!"); break;
								 case 2: sysmessage(s,"You can't teleport here!"); break;
								 default:
									 break;
								 }
							     return;                            
							}
							
							cMapObjects::getInstance()->remove(pc_currchar); //LB
							
							xo=pc_currchar->pos.x;
							yo=pc_currchar->pos.y;
							zo=pc_currchar->pos.z;
							
							pc_currchar->MoveTo(x,y,z);
							teleport( pc_currchar );
							doStaticEffect( pc_currchar, curSpell );
							cMagic::invisibleItemParticles(pc_currchar, curSpell, xo, yo, zo);
						}
						break;
						//////////// (24) WALL OF STONE /////////////
					case 24:
						
						if (char_selected) { sysmessage(s, "You can't use that spell on characters");  return; }
						id1 = 0x00;
						id2 = 0x80;
						snr = 24;
						break;
						//////////// (25) ARCH CURE /////////////////
					case 25:
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//Char cMapObjects::getInstance()
						ri = pc_currchar->pos;
						for (ri.Begin(); !ri.atEnd(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(mapchar) || (mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(chars[ii].pos.z>=z1&&chars[ii].pos.z<=z2)*/)
								{
									if ((lineOfSight( pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										cMagic::doStaticEffect(mapchar, 25);										
										// tempeffect(DEREF_P_CHAR(pc_currchar), ii, 2, 0, 0, 0); // lb bugfix ?? why does this cll night-sight effect
										mapchar->soundEffect( 0x01E9 );
										mapchar->setPoisoned(0);
									}
									else
									{
										sprintf((char*)temp,"There seems to be something in the way between you and %s.", mapchar->name.c_str());
										sysmessage(s, (char*)temp);
									}
								}
							}//if mapitem
						}
						break;
						////////// (26) ARCH PROTECTION ///////////////
					case 26:
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						x1 = x2 = y1 = y2 = z1 = z2 = 0;
						BoxSpell( s, x1, x2, y1, y2, z1, z2 );
						for (ri.Begin(); !ri.atEnd(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if( mapchar != NULL )
							{
								if (( online( mapchar ) || ( mapchar->isNpc() )) && ( mapchar->pos.x >= x1 && mapchar->pos.x <= x2 ) &&
								( mapchar->pos.y >= y1 && mapchar->pos.y <= y2 ) /*&&
								( chars[ii].pos.z >= z1 && chars[ii].pos.z <= z2 )*/)
								{
									if(( lineOfSight( pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										( pc_currchar->isGM() )))
									{
										playSound( mapchar, curSpell );
										doStaticEffect( mapchar, 26 );	// protection
										tempeffect( pc_currchar, mapchar, 21, pc_currchar->skill(MAGERY)/100, 0, 0 );
									}
									else
									{
										sprintf( (char*)temp, "There seems to be something in the way between you and %s.", mapchar->name.c_str() );
										sysmessage( s, (char*)temp );
									}
								}
							} // if mapitem
						}
						break;
						//////////// (28) FIRE FIELD ////////////////
					case 28:
						
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						
						if (j)
						{
							id1=0x39;
							id2=0x96;
						}		// end if
						else
						{
							id1=0x39;
							id2=0x8C;
						}		// end else
						snr = 28;
						break;
						//////////// (33) BLADE SPIRITS /////////////
					case 33:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						SummonMonster( s, 0x02, 0x3E, "a blade spirit", 0x00, 0x00, x, y, z, curSpell );
						break;
						//////////// (34) DISPEL FIELD //////////////
					case 34:
						{
							if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }							
							P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
							if (pi)
							{
								if ((lineOfSight( pc_currchar->pos, pi->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
									(pc_currchar->isGM())))
								{
									pi->soundEffect( 0x201 );
									if (pi->priv&5 || pi->priv&4) Items->DeleItem( pi );
								}
								else
								{
									sysmessage(s, "There seems to be something in the way.");
								}
							}
						}
						break;
						//////////// (39) POISON FIELD //////////////
					case 39:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						if (j)
						{
							id1=0x39;
							id2=0x20;
						}
						else
						{
							id1=0x39;
							id2=0x15;
						}
						snr = 39;
						break;
						//////////// (46) MASS CURSE ////////////////
					case 46:
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//Char cMapObjects::getInstance()
						ri = pc_currchar->pos;
						for (ri.Begin(); !ri.atEnd(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(mapchar)||(mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(chars[ii].pos.z>=z1&&chars[ii].pos.z<=z2)*/)
								{
									if ((lineOfSight( pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if (mapchar->npcaitype() == 17) // Ripper 11-14-99
										{
											sysmessage(s," They are Invulnerable merchants!");
											return;
										}
										
										if (mapchar->isNpc())
											mapchar->attackTarget( pc_currchar );
										doStaticEffect(mapchar, curSpell);
										mapchar->soundEffect( 0x01FB );
										if(CheckResist(pc_currchar, mapchar, 6)) j=pc_currchar->skill(MAGERY)/200;
										else j=pc_currchar->skill(MAGERY)/75;
										tempeffect(pc_currchar, mapchar, 12, j, j, j);
									}
									else
									{
										sprintf((char*)temp,"Try as you might, but you cannot see %s well enough to cover.", mapchar->name.c_str());
										sysmessage(s, (char*)temp);
									}
								}
							}//if	mapitem
						}
						break;
						//////////// (47) PARALYZE FIELD ////////////
					case 47:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						if (j)
						{
							id1=0x39;
							id2=0x79;
						}
						else
						{
							id1=0x39;
							id2=0x67;
						}
						snr = 47;
						break;
						//////////// (48) REVEAL ////////////////////
					case 48:
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						if ((lineOfSight( pc_currchar->pos, clTemp2, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
							(pc_currchar->isGM())))
						{
							j=pc_currchar->skill(MAGERY);
							range=(((j-261)*(15))/739)+5;
							//If the caster has a Magery of 26.1 (min to cast reveal w/ scroll), range radius is
							//5 tiles, if magery is maxed out at 100.0 (except for gms I suppose), range is 20
							
							increment = 0;
							RegionIterator4Chars ri(pc_currchar->pos);
							for (ri.Begin(); !ri.atEnd(); ri++)
							{
								P_CHAR mapchar = ri.GetData();
								if (mapchar != NULL)
								{
									if (mapchar->isHidden()) //does reveal hidden people as well now :)
									{
										dx=abs(mapchar->pos.x-x);
										dy=abs(mapchar->pos.y-y);
										dz=abs(mapchar->pos.z-z); // new--difference in z coords
										d=hypot(dx, dy);
										if ((d<=range)&&(dz<=15)) //char to reveal is within radius or range and no more than 15 z coords away
										{
											mapchar->unhide();
										}
									}
								}
							}
							soundeffect(s,0x01,0xFD);
						}
						else
						{
							sysmessage(s, "You would like to see if anything was there, but there is too much stuff in the way.");
						}
						break;
						
					case 49:///////// Chain Lightning ///////////
						///////// Forgotten, added by LB ////
						
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						//Char cMapObjects::getInstance()
						ri = pc_currchar->pos;
						for (ri.Begin(); !ri.atEnd(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(mapchar)||(mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(chars[ii].pos.z>=z1&&chars[ii].pos.z<=z2)*/)
								{
									if ((lineOfSight( pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if (mapchar->npcaitype() == 17) // Ripper 11-14-99
										{
											sysmessage(s," They are Invulnerable merchants!");
											return;
										}
										if (mapchar->isNpc())
											pc_currchar->attackTarget( mapchar );
										bolteffect(mapchar, true);
										pc_currchar->soundEffect( 0x0029 ); //Homey fix for chainlightning sound
										
										P_CHAR pc_defender = NULL, pc_attacker = NULL, pc_ii = NULL;
										if(CheckMagicReflect(mapchar))//AntiChrist
										{
											pc_defender = pc_currchar;
											pc_attacker = pc_ii = mapchar;
										} else
										{
											pc_defender = pc_ii = mapchar;
											pc_attacker = pc_currchar;
										}
										if (CheckResist(pc_attacker, pc_defender, 7))
										{
											MagicDamage(pc_defender, pc_attacker->skill(MAGERY)/70);
										}
										else
										{
											MagicDamage(pc_defender, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(49+1*(pc_ii->skill(MAGICRESISTANCE)/30)));
										}
									}
									else
									{
										sysmessage(s, "You cannot see the target well.");
									}
								}
							}//if Mapitem
						}
						break;
						
						//////////// (50) ENERGY FIELD //////////////
					case 50:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						if (j)
						{
							id1=0x39;
							id2=0x56;
						}
						else
						{
							id1=0x39;
							id2=0x46;
						}
						snr = 50;
						break;
						//////////// (54) MASS DISPEL ///////////////
					case 54:
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						
						//Char cMapObjects::getInstance()
						ri = pc_currchar->pos;
						for (ri.Begin(); !ri.atEnd(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(mapchar)||(mapchar->isNpc())) && (mapchar->priv2()&0x20)&&
									(mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
									(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
									(mapchar->pos.z>=z1&&mapchar->pos.z<=z2)*/)
								{
									if ((lineOfSight( pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if(CheckResist(pc_currchar, mapchar, 7) && rand()%2==0 ) // cant be 100% resisted , LB, osi
										{
											if (mapchar->isNpc()) 
												mapchar->attackTarget( pc_currchar );
											return;
										}
										if (mapchar->isNpc()) mapchar->kill(); // LB !!!!
										mapchar->soundEffect( 0x0204 );
										doStaticEffect(mapchar, curSpell);
									}
								}
							}//if mapitem
						}
						break;
						//////////// (55) METEOR SWARM //////////////
					case 55:
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//Char cMapObjects::getInstance()
						ri = pc_currchar->pos;
						for (ri.Begin(); !ri.atEnd(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(mapchar)||(mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(mapchar->pos.z>=z1&&mapchar->pos.z<=z2)*/)
								{
									if ((lineOfSight( pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if (mapchar->npcaitype() == 17) // Ripper 11-14-99
										{
											sysmessage(s," They are Invulnerable merchants!");
											return;
										}
										if (mapchar->isNpc()) 
											pc_currchar->attackTarget( mapchar );
										mapchar->soundEffect( 0x0160 ); //sound fix for swarm--Homey
										cMagic::doMoveEffect(curSpell, pc_currchar, mapchar );
										cMagic::doStaticEffect(mapchar, curSpell);
										if (CheckResist(pc_currchar, mapchar, 7))
										{
											MagicDamage(mapchar, pc_currchar->skill(MAGERY)/80);
										}
										else
										{
											MagicDamage(mapchar, (pc_currchar->skill(MAGERY)+1*(pc_currchar->skill(EVALUATINGINTEL)/3))/(39+1*(pc_i->skill(MAGICRESISTANCE)/30)));
										}
									}
									else
									{
										sysmessage(s, "You cannot see the target well.");
									}
								}
							}//if Mapitem
						}
						break;
						//////////// (58) ENERGY VORTEX /////////////
					case 58:
						SummonMonster( s, 0x00, 0x0d, "an energy vortex", 0x00, 0x75, x, y, z, curSpell );
						break;
					default:
						clConsole.send("MAGIC-ERROR: Unknown LocationTarget spell %i, magic.cpp\n", curSpell );
						break;
				}
				
				//AntiChrist - bugfix -
				//wall of stone has to be 3 tiles of length, like OSI
				int j2;
				if(id1==0x00 &&	id2==0x80)
				{//if a wall of stone
					j2=2;
				} else
				{
					j2=4;
				}
				if( fieldSpell( curSpell ) )
				{
					cTerritory* Region = cAllTerritories::getInstance()->region( x, y );
					if( Region != NULL && !Region->isGuarded() ) // Ripper 11-14-99
						if( !Region->allowsMagic() || !Region->allowsMagicDamage() ) // LB magic region changes !
						{
							sysmessage(s, tr(" You cant cast in town!"));
							return;
						}
						for( j=0; j<=j2; j++ )
						{
							P_ITEM pi=Items->SpawnItem(currchar[s],1,"#",0,(id1<<8)+id2,0,0);
							if(pi)
							{
								pi->priv |= 0x05;
								pi->decaytime=uiCurrentTime+((pc_currchar->skill(MAGERY)/15)*MY_CLOCKS_PER_SEC);
								pi->morex=pc_currchar->skill(MAGERY); // remember casters magery skill for damage, LB
								pi->MoveTo(fx[j], fy[j], Map->height( Coord_cl( fx[j], fy[j], z, pi->pos.map )));
								pi->dir=29;
								pi->magic=2;
								pi->update();
								cMagic::itemParticles(snr,pi);
							}
						}
				}
			} // if los
			
			} else { // location check
				sysmessage(s,"That is not a valid location.");
			}
		}
		else if( reqItemTarget( curSpell ) )
		{
			// ITEM TARGET
			P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
			if(pi)
			{
				if ((lineOfSight( pc_currchar->pos, pi->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
					(pc_currchar->isGM())))
				{
					playSound( pc_currchar, curSpell );
					switch( curSpell )
					{
						//////////// (13) MAGIC TRAP ////////////
					case 13:
						if((pi->type()==1 || pi->type()==63 ||
							pi->type()==8 || pi->type()==64)
							&& pi->id()!=0x0E75)
						{
							pi->setMoreb1( 1 );
							pi->setMoreb2( pc_currchar->skill( MAGERY ) / 20 );
							pi->setMoreb3( pc_currchar->skill( MAGERY ) / 10 );
							pi->soundEffect( 0x1F0 );
							cMagic::itemParticles(13,pi);
							sysmessage(s,"It's trapped!");
						}
						else
							sysmessage(s, "You cannot trap this!!!");
						break;
						//////////// (14) MAGIC UNTRAP //////////
					case 14:
						if((pi->type()==1 || pi->type()==63 ||
							pi->type()==8 || pi->type()==64))
						{
							if( pi->moreb1() == 1 )
							{
								if( rand() % 100 <= 50 + ( pc_currchar->skill( MAGERY ) / 10 ) - pi->moreb3() )
								{
									pi->setMoreb1( 0 );
									pi->setMoreb2( 0 );
									pi->setMoreb3( 0 );
									pi->soundEffect( 0x1F1 );
									cMagic::itemParticles( 14, pi );
									sysmessage( s, tr("You successfully untrap this item!") );
								}
								else 
									sysmessage(s, tr("You miss to untrap this item!"));
							}
							else sysmessage(s, tr("This item doesn't seem to be trapped!"));
						}
						else sysmessage(s,tr("This item cannot be trapped!"));
						break;
						//////////// (19) MAGIC LOCK ////////////
					case 19:
						if((pi->type()==1 || pi->type()==63)
							&& pi->id()!=0x0E75 )
						{
							switch(pi->type())
							{
							case 1:	pi->setType( 8 ); break;
							case 63: pi->setType( 64 ); break;
							default:
								clConsole.send("ERROR: Fallout of switch statement without default. magic.cpp, magiclocktarget()/n"); //Morrolan
								break;
							}
							pi->soundEffect( 0x200 );
							cMagic::itemParticles(19,pi);
							sysmessage(s,"It's locked!");
						}
						else
							sysmessage(s, tr("You cannot lock this!!!"));
						break;
						
						//////////// (23) MAGIC UNLOCK //////////
						
					case 23:
						if( pi->type() == 8 || pi->type() == 64 )
						{
							if(pi->more1==0 && pi->more2==0 && pi->more3==0 && pi->more4==0)
							{ //Make sure it isn't an item that has a key (i.e. player house, chest..etc)
								switch(pi->type())
								{
								case 8: pi->setType( 1 ); break;
								case 64: pi->setType( 63 ); break;
								default:
									//LogError("switch reached default");
									return;
								}
								pi->soundEffect( 0x1FF );
								cMagic::itemParticles(23,pi);
								sysmessage(s, "You manage to pick the lock.");
							}
						} else
						{
							if ( pi->type() == 1 || pi->type() == 63 || pi->type() == 65 || pi->type() == 87 )
							{
								sysmessage( s, tr("That is not locked." ));
							}
							else
							{
								sysmessage( s, tr("That does not have a lock."));
							}
						}
						break;
						//////////// (41) DISPEL ////////////////
						//case 41:
						//	if (pi->isInvul()) Items->DeleItem(i);
						//	break;
						// Uhmm.....this already is in CharacterTarget spells
						// don't think if it should be here......
					default:
						clConsole.send("MAGIC-ERROR: Unknown ItemTarget spell %i, magic.cpp\n", curSpell );
						break;
					}
				}
				else
					sysmessage( s, tr("There is something in the way!") );
			}
			else
				sysmessage( s, tr("That is not a valid item") );
		}
		else
			sysmessage( s, tr("Can't cope with this spell, requires a target but it doesn't specify what type" ));
		pc_currchar->setSpell( 0 );
		return;
	}
	else
	{
		// non targetted spells
		playSound( pc_currchar, curSpell );
		if ( curSpell != 35 && curSpell !=40) doStaticEffect( pc_currchar, curSpell );
		
		switch( curSpell )
		{
			//////////// (2) CREATE FOOD ////////////////
        case 2: // Fallen/Ripper - Now random food from scripts (ITEMLIST 75)
			{
	           P_ITEM pj = Items->SpawnItemBackpack2(s,DefManager->getRandomListEntry("75"),1);
			   {
	              if (pj != NULL) // Ripper
				  {
	                 pj->setType( 14 );
					 sprintf((char*)temp,"You magically create food in your backpack: %s",pj->name().ascii());
		             sysmessage(s,(char*)temp);
	                 pj->update();
				  }
			   }
			}
	    break;
			//////////// (3) SUMMON MONSTER ////////////
		case 40: //33:
			SummonMonster( s, 0, 0, "#", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (35) INCOGNITO ////////////
		case 35:
			tempeffect(pc_currchar, pc_currchar,19,0,0,0);//Incognito temp effect
			cMagic::doStaticEffect( pc_currchar, curSpell);
			break;
			//////////// (36) MAGIC REFLECTION /////////
		case 36:
//			pc_currchar->priv2=pc_currchar->priv2|0x40;
			pc_currchar->setPriv2(pc_currchar->priv2()|0x40);
			break;
			//////////// (56) POLYMORPH /////////////////
		case 56:
//			PolymorphMenu( s, POLYMORPHMENUOFFSET ); // Antichrist's Polymorph
			break;
			//////////// (57) EARTHQUAKE ///////////////
		case 57:
		{
			dmg=(pc_currchar->skill(MAGERY)/40)+(rand()%20-10);
			dmgmod = 0;
			
			loopexit=0;
			RegionIterator4Chars ri( pc_currchar->pos );
			for( ri.Begin(); !ri.atEnd(); ri++ )
			{
				P_CHAR pc = ri.GetData();
				if (pc != NULL)
				{
					if (pc->isInvul() || pc->npcaitype()==17)		// don't affect vendors
						continue;
					if (pc->isSameAs(pc_currchar))				// nor the caster
						continue;
					distx=abs(pc->pos.x - pc_currchar->pos.x);
					disty=abs(pc->pos.y - pc_currchar->pos.y);
					if(distx<=15 && disty<=15 && (pc->isNpc() || online(pc)))
					{
						if(pc->isInnocent()) criminal(currchar[s]);
						
						if (!pc->isGM() && pc->account()!=0)
							dmgmod = QMIN(distx,disty);
						dmgmod = -(dmgmod - 7);
//								pc->hp -=  dmg+dmgmod;
						pc->setHp(pc->hp() - dmg+dmgmod);
//								pc->stm -= rand()%10+5;
						pc->setStm(pc->stm() - rand()%10+5);									
						if(pc->stm()<0)  pc->setStm(0);
						if(pc->hp()<0) pc->setHp(0);						 							
						
						if (pc->isPlayer() && online(pc))
						{
							if(rand()%2) npcaction(pc, 0x15); else npcaction(pc, 0x16);
							if((pc->isNpc() || online(pc)) && pc->hp() == 0)
							{
								pc->kill();
							}
						}	
						else
						{
							if (pc->hp()<=0)
								pc->kill();
							else
							{
								if (pc->isNpc())
								{
									npcaction(pc, 0x2);
									currchar[s]->attackTarget(pc);
								}
							}
						}	
					} //if Distance
				}
			}
		}
			break;			//////////// (60) SUMMON AIR ELEMENTAL /////
		case 60:
			SummonMonster( s, 0x00, 0x0d, "Air", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (61) SUMMON DAEMON ////////////
		case 61:
			SummonMonster( s, 0x00, 0x0A, "Daemon", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (62) SUMMON EARTH ELEMENTAL ///
		case 62:
			SummonMonster( s, 0x00, 0x0E, "Earth", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (63) SUMMON FIRE ELEMENTAL ////
		case 63:
			SummonMonster( s, 0x00, 0x0F, "Fire", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (64) SUMMON WATER ELEMENTAL ///
		case 64:
			SummonMonster( s, 0x00, 0x10, "Water", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (65) SUMMON HERO //////////////
		case 65:
			SummonMonster( s, 0x03, 0xE2, "Dupre the Hero", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (67) SUMMON BLACK NIGHT ///////
		case 67:
			SummonMonster( s, 0x00, 0x0A, "Black Night", 5000>>8, 5000%256, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
		default:
			clConsole.send("MAGIC-ERROR: Unknown NonTarget spell %i, magic.cpp\n", curSpell );
			break;
		}
		//sysmessage( s, "Spells like meteor swarm will go here, eventually" );
		pc_currchar->setSpell( 0 );
		return;
	}
	
}

/*bool cMagic::townTarget( unsigned char num )
{
	switch( num )
	{
		case 2: // create food
		case 4: // heal
		case 6: // nightsight
		case 7: // reactive armor
		case 9: // agility
		case 11: // cure
		case 15: // protection
		case 16: // strength
		case 17: // bless
		case 22: // teleport
		case 25: // arch cure
		case 26: // arch protection
		case 29: // greater heal
		case 32: // recall
		case 40: // summoncreature
		case 44: // invisibility
		case 45: // mark
		case 48: // reveal
		case 52: // gate travel
		case 59: // resurrection
		case 36: // magic reflection
		case 56: // polymorph
		case 60: // air elemental
		case 61: // daemon
		case 62: // earth elemental
		case 63: // fire elemental
		case 64: // water elemental
			return true;
		default:
			return false;
	}
	return true;
}*/

bool cMagic::requireTarget( unsigned char num )
{

	switch( num )
	{
	case 1: // Clumsy
	case 3: // Feeblemind
	case 4: // Heal
	case 5: // Magic Arrow
	case 6: // Night Sight
	case 7:	// Reactive Armour
	case 8: // Weaken
	case 9: // Agility
	case 10:// Cunning
	case 11:// Cure
	case 12:// Harm
	case 13:// Magic Trap
	case 14:// Magic Untrap
	case 15:// Protection
	case 16:// Strength
	case 17:// Bless
	case 18:// Fireball
	case 19:// Magic Lock
	case 20:// Poison
	case 22:// Teleport
	case 23:// Unlock
	case 24:// Wall of Stone
	case 25:// Arch Cure
	case 26:// Arch protection
	case 27:// Curse
	case 28:// Fire Field
	case 29:// Greater Heal
	case 30:// Lightning
	case 31:// Mana drain
	case 32:// Recall
	case 33:// Blade Spirits
	case 34:// Dispel Field
	case 37:// Mind Blast
	case 38:// Paralyze
	case 39:// Poison Field
	case 41:// Dispel
	case 42:// Energy Bolt
	case 43:// Explosion
	case 44:// Invisibility
	case 45:// Mark
	case 46:// Mass curse
	case 47:// Paralyze Field
	case 48:// Reveal
	case 49:// Chain Lightning
	case 50:// Energy Field
	case 51:// Flamestrike
	case 52:// Gate Travel
	case 53:// Mana Vampire
	case 54:// Mass Dispel
	case 55:// Meteor Swarm
	case 58:// Energy Vortex
	case 59:// Resurrection
	case 66:// Cannon Firing
			return true;


	case 36:// Magic Reflection
	case 57:// Earthquake
	case 56:// Polymorph
	case 60:// Summon Air Elemental
	case 61:// Summon Daemon
	case 62:// Summon Earth Elemental
	case 63:// Summon Fire Elemental
	case 64:// Summon Water Elemental
	case 65:// Summon Hero
	case 67:// Summon Black Night
	default:
			return false;
	}
	return false;
}

void cMagic::DelReagents( P_CHAR pc, int num )
{
	if (!pc) return;
	if (pc->priv2()&0x80) return;
	reag_st& R = spells[num].reagents;
	delequan(pc, 0x0F7A, R.pearl);
	delequan(pc, 0x0F7B, R.moss);
	delequan(pc, 0x0F84, R.garlic);
	delequan(pc, 0x0F85, R.ginseng);
	delequan(pc, 0x0F86, R.drake);
	delequan(pc, 0x0F88, R.shade);
	delequan(pc, 0x0F8C, R.ash);
	delequan(pc, 0x0F8D, R.silk);
}

bool cMagic::spellReflectable( int num )
{
	//AntiChrist - customizable in spells.scp
	if(spells[num].reflect)
		return true;
	else return false;
}

bool cMagic::travelSpell( int num )
{
	switch( num )
	{
		case 52:
		case 32:
		case 45:
			return true;
		default:
			return false;
	}
	return false;
}

bool cMagic::reqCharTarget( int num )
{
	switch( num )
	{
		case 1:	// Clumsy
		case 3:	// Feeblemind
		case 4:	// Heal
		case 5:	// Magic Arrow
		case 6:	// Night Sight
		case 7:	// Reactive armour
		case 8:	// Weaken
		case 9:	// Agility
		case 10: // Cunning
		case 11: // Cure
		case 12: // Harm
		case 15: // Protection
		case 16: // Strength
		case 17: // Bless
		case 18: // Fireball
		case 20: // Poison
		case 27: // Curse
		case 29: // Greater Heal
		case 30: // Lightning
		case 31: // Mana drain
		case 37: // Mind Blast
		case 38: // Paralyze
		case 41: // Dispel
		case 42: // Energy Bolt
		case 43: // Explosion
		case 44: // Invisibility
		case 51: // Flamestrike
		case 53: // Mana Vampire
		case 56: // Polymorph
		case 59: // Resurrection
		case 66: // Cannon Firing
			return true;
		default:
			return false;
	}
	return false;
}

bool cMagic::reqLocTarget( int num )
{
	switch( num )
	{
		case 22:// Teleport
		case 24:// Wall of Stone
		case 25:// Arch Cure
		case 26:// Arch protection
		case 28:// Fire Field
		case 33:// Blade Spirits
		case 34:// Dispel Field
		case 39:// Poison Field
		case 46:// Mass curse
		case 47:// Paralyze Field
		case 48:// Reveal
		case 49:// Chainlightning
		case 50:// Energy Field
		case 54:// Mass Dispel
		case 55:// Meteor Swarm
		case 58:// Energy Vortex
			return true;
		default:
			return false;
	}
	return false;
}

bool cMagic::reqItemTarget( int num )
{
	switch( num )
	{
		case 13:// Magic Trap
		case 14:// Magic Untrap
		case 19:// Magic Lock
		case 23:// Unlock
			return true;
		default:
			return false;
	}
	return false;
}

move_st cMagic::getMoveEffects( int num )
{
	move_st temp;
	int te0,te1,te2,te3,te4;                // 2d move efefcts
	int te5,te6,te7,te8,te9,te10,te11,te12,te13, te14, te15, te16, te17; // 3d moving effects
	switch( num )
	{
	case 5:		te0=0x36; te1=0xE4; te2=0x05; te3=0x00; te4=0x01;
		        te5=0x36; te6=0xe4; te7=0x07; te8=0x00; te9=0x0b; te10=0xbe; te11=0x0f; te12=0xa6; te13=0x00; te14=0x00; te15=0; te16=0; te17=0;
				break;

	case 18:	te0=0x36; te1=0xD5; te2=0x07; te3=0x00; te4=0x01;
		        te5=0x36; te6=0xd4; te7=0x07; te8=0x00; te9=0x0b; te10=0xcb; te11=0x0f; te12=0xb3; te13=0x1; te14=0x60;te15=0; te16=1; te17=0;
		        break;

	case 42:	te0=0x37; te1=0x9F; te2=0x07; te3=0x00; te4=0x01;
		        te5=0x37; te6=0x9f; te7=0x07; te8=0x00; te9=0x0b; te10=0xe3; te11=0x0f; te12=0xcb; te13=0x2; te14=0x11; te15=0; te16=1; te17=0;
		        break;

	case 55:	te0=0x36; te1=0xD5; te2=0x07; te3=0x00; te4=0x01;
		        te5=0x36; te6=0xd4; te7=0x0a; te8=0x07; te9=0x25; te10=0x1d; te11=0x00; te12=0x01; te13=0x0; te14=0x0; te15=0; te16=1; te17=1;
		        break;
						

	default:	te0=-1;	  te1=-1;   te2=-1;   te3=-1;   te4=-1;	  break;
	}

	temp.effect[0]=te0;
	temp.effect[1]=te1;
	temp.effect[2]=te2;
	temp.effect[3]=te3;
	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	temp.effect[13]=te13;
	temp.effect[14]=te14;
	temp.effect[15]=te15;
	temp.effect[16]=te16;
	temp.effect[17]=te17;

	return temp;
}

stat_st cMagic::getStatEffects( int num )
{
	stat_st temp;
	int te0,te1,te2,te3,te4;      // 2d effects
	int te5,te6,te7,te8,te9,te10,te11,te12; // 3d particles

	//te5=te6=te7=te8=te9=te10=0;
    te11=0; te12=1;
	switch( num )
	{
	case 1:		te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x8a; te10=0xffffff00;			
				break;
	case 3:		te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x79; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x8c; te10=0xffffff00;
		        break;
	case 4:		te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x6A; te6=0x09; te7=0x20;
				te8=0x13; te9=0x8d; te10=0xffffff00;
		        break;
	case 7:		te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x6A; te6=0x09; te7=0x20;
				te8=0x13; te9=0x90; te10=0xffffff03;
		        break;
	case 8:		te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x79; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x91; te10=0xffffff00;
				break;
	case 9:		te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x92; te10=0xffffff03;
				break;
	case 10:	te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x93; te10=0xffffff00;
				break;
	case 11:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x94; te10=0xffffff03;
				break;
	case 12:	te0=0x37; te1=0x4A; te2=0x09; te3=0x07;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x95; te10=0xffffff03;
		        break;
	case 15:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x5a; te6=0x09; te7=0x14;
				te8=0x13; te9=0x98; te10=0xffffff03;
		        break;
	case 16:	te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x99; te10=0xffffff03;
		        break;
	case 17:	te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x9a; te10=0xffffff02;
		        break;
	case 20:	te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x9d; te10=0xffffff03;
		        break;
	case 22:	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x00;
				te8=0x13; te9=0x9f; te10=0xffffff00;
		        break;
	case 25:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x6a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x94; te10=0xffffff03;
		        break;
	case 26:    te0=0x37; te1=0x5A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x5a; te6=0x09; te7=0x14;
				te8=0x13; te9=0xa3; te10=0xffffff03;
				break;
	case 27:	te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xa4; te10=0xffffff03;
				break;
	case 29:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
                te4=0x37; te5=0x6a; te6=0x09; te7=0x20;
				te8=0x13; te9=0xa6; te10=0xffffff03;
		        break;
	case 49:
	case 30:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te8=0x13; te9=0xa7; te10=0xffffff03;
		        break;
	case 31:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xa8; te10=0xffffff00;
		        break;
	case 32:  	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te8=0x13; te9=0xa9; te10=0xffffff03;
		        break;
	case 33:  	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x00;
				te8=0x13; te9=0xaa; te10=0xffffff00;
		        break;
	case 35:  	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xac; te10=0xffffff00;
		        break;
	case 36:	te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xad; te10=0xffffff03;				
		        break;
	case 37:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 	
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
				te8=0x07; te9=0xf6; te10=0xffffff00;	
		        break;
	case 38:	te0=0x37; te1=0x35; te2=0x00; te3=30;
		        te4=0x37; te5=0x6a; te6=0x06; te7=0x01;
				te8=0x13; te9=0xaf; te10=0xffffff05;
				break;
	case 40:
	case 48:    te0=0x37; te1=0x35; te2=0x00; te3=30;
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x0a;
				te8=0x13; te9=0xb1; te10=0xffffff00;
				break;
	case 43:	te0=0x36; te1=0xB0; te2=0x09; te3=0x09;
		        te4=0x36; te5=0xbd; te6=0x0a; te7=0x0a;
				te8=0x13; te9=0xb4; te10=0xffffff00;
		        break;
	case 46:	te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x95; te10=0xffffff03;		
		        break;		
	case 51:	te0=0x37; te1=0x09; te2=0x09; te3=0x19; 			
		        te4=0x37; te5=0x09; te6=0x0a; te7=0x1e;
				te8=0x13; te9=0xbc; te10=0xffffff05;
		        break;
	case 53:	te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xbe; te10=0xffffff00;		
		        break;
	case 54:	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x0a;
				te8=0x13; te9=0x9f; te10=0xffffff00;		
		        break;
	case 55:	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te10=rand()%3;
				switch (te10)
				{
				  case 0: te8=0x13; te9=0xc0; break;
				  case 1: te8=0x17; te9=0xa8; break;
				  case 2: te9=0x1b; te9=0x90; break;
				  default: te8=0x13; te9=0xc0;
				}
				te10=0xffffff00;		
		        break;
	case 58: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te8=0x13; te9=0xc3; te10=0xffffff00;
				break;
	case 59: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x36; te5=0x6a; te6=0x09; te7=0x20;
			    te8=0x25; te9=0x1d; te10=0xffffff03;			
				break;
	case 60: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
			    te8=0x13; te9=0xc5; te10=0xffffff03;			
				//te11=27; te12=0xf;
				break;
	case 61: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x00;
			    te8=0x13; te9=0xc6; te10=0xffffff03;			
				break;
	case 62: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
			    te8=0x13; te9=0xc7; te10=0xffffff03;			
				break;
	case 63: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
			    te8=0x13; te9=0xc8; te10=0xffffff03;			
				break;
	case 64: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
		        te4=0x00; te5=0x00; te6=0x09; te7=0x00;
			    te8=0x13; te9=0xc9; te10=0xffffff03;			
				break;

	case 66:	te0=0x36; te1=0xB0; te2=0x09; te3=0x09;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x9e; te10=0xffffff00;	
		        break;

	case 99:	te0=0x37; te1=0x35; te2=0x00; te3=0x30;  // fizzle
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
				te8=0x0f; te9=0xcb; te10=0xffffff01;	
		        break;			

	default:	te0=-1;	te1=-1;	te2=-1;	te3=-1; break;
	}

	temp.effect[0]=te0;
	temp.effect[1]=te1;
	temp.effect[2]=te2;
	temp.effect[3]=te3;
	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	
	return temp;
}

stat_st cMagic::getStatEffects_pre( int num)
{
    stat_st temp = {-1,};

	switch( num )
	{
	case 1:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 2:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x33; temp.effect[10]=0xffff0102; break;
	case 3:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;		
	case 4:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;		
	case 5:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x51; temp.effect[10]=0xffff0102; break;
	case 6:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 7:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x33; temp.effect[10]=0xffff0102; break;		
	case 8:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;

	case 9:	   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;
	case 10:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;
	case 11:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;
	case 12:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x51; temp.effect[10]=0xffff0102; break;
	case 13:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x29; temp.effect[10]=0xffff0102; break;
	case 14:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x29; temp.effect[10]=0xffff0102; break;
	case 15:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x33; temp.effect[10]=0xffff0102; break;
	case 16:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;

	case 17:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x51; temp.effect[10]=0xffff0102; break;
	case 18:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x51; temp.effect[10]=0xffff0102; break;
	case 19:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x29; temp.effect[10]=0xffff0102; break;
	case 20:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x5b; temp.effect[10]=0xffff0102; break;
	case 21:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 22:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 23:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x29; temp.effect[10]=0xffff0102; break;
	case 24:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x33; temp.effect[10]=0xffff0102; break;
	case 25:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;

	case 26:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x33; temp.effect[10]=0xffff0102; break;
	case 27:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 28:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x51; temp.effect[10]=0xffff0102; break;
	case 29:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x65; temp.effect[10]=0xffff0102; break;
	case 30:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x3d; temp.effect[10]=0xffff0102; break;
	case 31:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 32:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;

	case 33:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x50; temp.effect[10]=0x02010102; break;
	case 34:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;
	case 35:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;
	case 36:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x34; temp.effect[10]=0xffff0102; break;
	case 37:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x48; temp.effect[10]=0xffff0102; break;
	case 38:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x34; temp.effect[10]=0xffff0102; break;
	case 39:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x5c; temp.effect[10]=0xffff0102; break;
	case 40:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x50; temp.effect[10]=0xffff0102; break;

	case 41:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;
	case 42:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x3e; temp.effect[10]=0xffff0102; break;
	case 43:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x51; temp.effect[10]=0xffff0102; break;
	case 44:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;
	case 45:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;
	case 46:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x47; temp.effect[10]=0xffff0102; break;
	case 47:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x34; temp.effect[10]=0xffff0102; break;
	case 48:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;

	case 49:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x3e; temp.effect[10]=0xffff0102; break;
	case 50:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x3e; temp.effect[10]=0xffff0102; break;
	case 51:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x52; temp.effect[10]=0xffff0102; break;
	case 52:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x48; temp.effect[10]=0xffff0102; break;
	case 53:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x48; temp.effect[10]=0xffff0102; break;
	case 54:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;
	case 55:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x52; temp.effect[10]=0xffff0102; break;
	case 56:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x2a; temp.effect[10]=0xffff0102; break;

	case 57:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x34; temp.effect[10]=0xffff0102; break;
	case 58:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x48; temp.effect[10]=0xffff0102; break;
	case 59:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x66; temp.effect[10]=0xffff0102; break;
	case 60:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x32; temp.effect[10]=0xffff0102; break;
	case 61:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x5a; temp.effect[10]=0xffff0102; break;
	case 62:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x3c; temp.effect[10]=0xffff0102; break;
	case 63:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x5a; temp.effect[10]=0xffff0102; break;
	case 64:   temp.effect[4]=0x00; temp.effect[5]=0x00; temp.effect[6]=0x0a; temp.effect[7]=0x05; temp.effect[8]=0x23; temp.effect[9]=0x6e; temp.effect[10]=0xffff0102; break;

	default:   temp.effect[4] = -1; temp.effect[5] = -1; temp.effect[6] = -1; temp.effect[7]=-1; break;
	}

	temp.effect[11]=0;
	temp.effect[12]=1;
	
	return temp;
}

stat_st cMagic::getStatEffects_after( int num)
{
    stat_st temp;
    memset( &temp, -1, sizeof ( stat_st ) );    	
	int te4,te5,te6,te7,te8,te9,te10, te11,te12;

	te11=0; te12=1;
	switch( num )
	{
	   case 6:	  te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x8f; te10=0xffff0003; break;	  	
	   case 37:   te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f; te8=0x13; te9=0xae; te10=0xffff0000; break;
	   case 38:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x13; te9=0xaf; te10=0xffffff05; te11=0x27; te12=0x0f; break;
	   default:	te4=-1;	te5=-1;	te6=-1;	te7=-1; break;
	}

	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	
	return temp;
}

stat_st cMagic::getStatEffects_item( int num)
{
    stat_st temp;
    memset( &temp, -1, sizeof ( stat_st ) );    	
	int te4,te5,te6,te7,te8,te9,te10, te11;

	switch( num )
	{	
	   case 13:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0x96; te10=0xffff0000; te11=0; break;
	   case 14:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x8f; te10=0xffff0003; te11=0; break;
	   case 19:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x9c; te10=0xffff0002; te11=0; break;
	   case 22:   te4=0x37; te5=0x28; te6=0x0a; te7=0x00; te8=0x07; te9=0xe7; te10=0xffff0002; te11=0; break;
	   case 23:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0xa0; te10=0xffff0003; te11=0; break;
	   case 24:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xa1; te10=0xffff0003; te11=0; break;
	   case 28:   te4=0x37; te5=0x6a; te6=0x0f; te7=0x0a; te8=0x13; te9=0xa5; te10=0xffff0000; te11=0; break;
	   case 32:   te4=0x00; te5=0x00; te6=0x00; te7=0x00; te8=0x13; te9=0xa9; te10=0xffff0003; te11=0; break;
	   case 39:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xb0; te10=0xffff0000; te11=0; break;
	   case 44:   te4=0x37; te5=0x6a; te6=0x0a; te7=0x0f; te8=0x13; te9=0xb5; te10=0xffff0003; te11=0; break;
	   case 45:   te4=0x37; te5=0x79; te6=0x0a; te7=0x0f; te8=0x13; te9=0xb6; te10=0xffff0003; te11=0; break;
	   case 47:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xb8; te10=0xffff0003; te11=0; break;
	   case 50:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xbb; te10=0xffff0003; te11=0; break;
	   case 59:   te4=0x00; te5=0x00; te6=0x00; te7=0x00; te8=0x13; te9=0xc4; te10=0xffff0003; te11=0; break;

	   default:	te4=-1;	te5=-1;	te6=-1;	te7=-1; break;
	}

	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	
	return temp;
}

void cMagic::invisibleItemParticles(P_CHAR pc, int spellNum, short x, short y, signed char z)
{
	P_ITEM it;

    // create a dummy item for the effect on old location
    it = Items->SpawnItem(pc, 1, "bugalert, plz let the devteam know", 0, 0x1, 0x00, 0);					
	it->MoveTo( x, y, z );
	it->update();
	itemParticles(spellNum, it );
	// this is rather tricky, deleitem can't be applied there
	// because the client has to tihnk its still there. np because it's an invisible item anyway
	// but we have to tell the memory manger to delete it
	cItemsManager::getInstance()->unregisterItem(it);
	delete it;
}

sound_st cMagic::getSoundEffects( int num )
{
	sound_st temp;
	int te0,te1;

	switch( num )	// The reason why I deleted some is because they already have
	{				// sounds in other parts of the code, keep them in if you want,
					// even the deleted numbers have been corrected - Paul77
	case 1:		te0=0x01; te1=0xDF; break;
	case 2:		te0=0x01; te1=0xE2; break;
	case 3:		te0=0x01; te1=0xE4; break;
	case 4:		te0=0x01; te1=0xF2; break;
	case 5:		te0=0x01; te1=0xE5; break;
	case 6:		te0=0x01; te1=0xE3; break;
	case 7:		te0=0x01; te1=0xEE; break;
	case 8:		te0=0x01; te1=0xE6; break;
	case 9:		te0=0x01; te1=0xE7; break;
	case 10:	te0=0x01; te1=0xEB; break;
	case 11:	te0=0x01; te1=0xE0; break; // Changed by Paul77
	case 12:	te0=0x01; te1=0xF1; break;
	case 13:	te0=0x01; te1=0xEF; break; // Added by Paul77
	case 14:	te0=0x01; te1=0xF0; break; // Added by Paul77
	case 15:	te0=0x01; te1=0xED; break; // Changed by Paul77
	case 16:	te0=0x01; te1=0xEE; break;
	case 17:	te0=0x01; te1=0xEA; break;
	case 18:	te0=0x01; te1=0x5E; break;
	case 19:	te0=0x01; te1=0xF4; break; // Added by Paul77
	case 20:	te0=0x02; te1=0x05; break; // Changed by Paul77
	case 22:	te0=0x01; te1=0xFE; break;
	case 23:	te0=0x01; te1=0xFF; break; // Added by Paul77
	case 24:	te0=0x01; te1=0xF6; break;
//	case 25:	te0=0x01; te1=0xE9; break; Deleted by Paul77
	case 26:	te0=0x01; te1=0xF7; break;
	case 27:	te0=0x01; te1=0xE1; break;
	case 28:	te0=0x02; te1=0x0C; break;
	case 29:	te0=0x02; te1=0x02; break;
	case 30:	te0=0x00; te1=0x29; break;
	case 31:	te0=0x01; te1=0xF9; break;
	case 32:	te0=0x01; te1=0xFC; break;
//	case 33:	te0=0x02; te1=0x12; break; Deleted by Paul77
	case 34:	te0=0x02; te1=0x01; break;
//	case 35:	te0=0x02; te1=0x03; break; Deleted by Paul77 - Should not have a sound for Incognito
	case 36:	te0=0x01; te1=0xE9; break;
	case 37:	te0=0x02; te1=0x13; break;
	case 38:	te0=0x02; te1=0x04; break;
	case 39:	te0=0x02; te1=0x0C; break;
	case 41:	te0=0x02; te1=0x01; break; // Changed by Paul77
	case 42:	te0=0x02; te1=0x0A; break;
	case 43:	te0=0x02; te1=0x07; break;
	case 44:	te0=0x02; te1=0x03; break;
	case 45:	te0=0x01; te1=0xFA; break;
//	case 46:	te0=0x01; te1=0xFB; break; Changed and Deleted by Paul77
	case 47:	te0=0x02; te1=0x0B; break;
	case 48:	te0=0x01; te1=0xFD; break; // Sound happens twice (probably justified so kept it) - Paul77
//	case 49:	te0=0x00; te1=0x29; break; Deleted by Paul77
	case 50:	te0=0x02; te1=0x0B; break; // Changed by Paul77
	case 51:	te0=0x02; te1=0x08; break;
	case 52:	te0=0x02; te1=0x0E; break;
	case 53:	te0=0x01; te1=0xF9; break; // Changed by Paul77
	case 54:	te0=0x02; te1=0x09; break; // Changed by Paul77
	case 55:	te0=0x01; te1=0x60; break; // Sound happens twice (probably justified so kept it) - Paul77
//	case 58:	te0=0x02; te1=0x12; break; Changed and Deleted by Paul77
//	case 60:	te0=0x02; te1=0x17; break; Deleted by Paul77
//	case 61:	te0=0x02; te1=0x16; break; Deleted by Paul77
//	case 62:	te0=0x02; te1=0x17; break; Deleted by Paul77
//	case 63:	te0=0x02; te1=0x17; break; Deleted by Paul77
//	case 64:	te0=0x02; te1=0x17; break; Deleted by Paul77
	case 65:	te0=0x02; te1=0x46; break;
	case 66:	te0=0x02; te1=0x07; break;

	default:	te0=-1;	te1= -1; break;
	}
	temp.effect[0]=te0;
	temp.effect[1]=te1;
	return temp;
}

void cMagic::playSound( P_CHAR pc_source, int num )
{
	sound_st temp;
	temp.effect[0] = -1;
	temp.effect[1] = -1;

	temp = getSoundEffects( num );
	if( temp.effect[0] != -1 && temp.effect[1] != -1 )
		pc_source->soundEffect( (short)( temp.effect[0] << 8 | temp.effect[1] ) );
}

void cMagic::doStaticEffect( P_CHAR source, int num )
{
	stat_st temp;
	memset( &temp , -1, sizeof(stat_st) );	
	temp = getStatEffects( num );

	if( temp.effect[0] != -1 && temp.effect[1] != -1 && temp.effect[2] != -1 && temp.effect[3] != -1 )
	{
		staticeffect( source, temp.effect[0], temp.effect[1], temp.effect[2], temp.effect[3], true,  &temp);
		// looks stupid to pass a pointer to a struct variable and elements of the *same* struct variable
		// actually it's very tricky but ok. (via pointer #4..15 is accessed, saved work to change a few 1000 LOC's)
		// please don't touch. thx, LB
	}
}

void cMagic::doMoveEffect( int num, P_CHAR target, P_CHAR source )
{
	move_st temp;
	memset( &temp , -1, sizeof( move_st) );	

	temp = getMoveEffects( num );

	if( temp.effect[0] != -1 && temp.effect[1] != -1 && temp.effect[2] != -1 && temp.effect[3] != -1 && temp.effect[4] != -1 )
		movingeffect(source, target, temp.effect[0], temp.effect[1], temp.effect[2], temp.effect[3], temp.effect[4], true, &temp );
}

bool cMagic::aggressiveSpell( int num )
{
	switch( num )
	{
	case 1: // Clumsy
	case 3: // Feeblemind
	case 5: // Magic Arrow
	case 8: // Weaken
	case 12:// Harm
	case 18:// Fireball
	case 20:// Poison
	case 27:// Curse
	case 30:// Lightning
	case 31:// Mana drain
	case 37:// Mind Blast
	case 38:// Paralyze
	case 42:// Energy Bolt
	case 43:// Explosion
	case 46:// Mass curse
	case 49:// Chain Lightning
	case 51:// Flamestrike
	case 53:// Mana Vampire
	case 55:// Meteor Swarm
	case 66:// Cannon Firing
			return true;
	default:
			return false;
	}
	return false;
}

bool cMagic::fieldSpell( int num )
{
	switch( num )
	{
		case 24:// Wall of Stone
		case 28:// Fire Field
		case 39:// Poison Field
		case 47:// Paralyze Field
		case 50:// Energy Field
			return true;
		default:
			return false;
	}
}

// added by AntiChrist (9/99)
void cMagic::PolymorphMenu(int s,int gmindex)
{
/*	int total, i, loopexit=0;
	char lentext;
	char sect[512];
	static char gmtext[30][257];
	int gmid[30];
	int gmnumber=0,dummy=0;

	openscript("polymorph.scp");
	sprintf(sect, "POLYMORPHMENU %i",gmindex);
	if (!i_scripts[polymorph_script]->find(sect))
	{
		closescript();
		return;
	}
	read1();
	lentext=sprintf(gmtext[0],(char*)script1);
	do
	{
		read2();
		if (script1[0]!='}' && strcmp("POLYMORPHID",(char*)script1))
		{
			gmnumber++;
			gmid[gmnumber]=hex2num(script1);
			strcpy(gmtext[gmnumber],(char*)script2);
			read1();
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	// reading polymorph duration time ...
	sprintf(sect, "POLYMORPHDURATION %i",dummy);
	if (!i_scripts[polymorph_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read2();
		if (script1[0]!='}')
		{
			polyduration=str2num(script1);
			read1();
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();

//	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	total=9+1+lentext+1;
	for (i=1;i<=gmnumber;i++) total+=4+1+strlen(gmtext[i]);
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;
	LongToCharPtr(pc_currchar->serial, &gmprefix[3]);
	gmprefix[7]=gmindex>>8;
	gmprefix[8]=gmindex%256;
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, gmtext[0], lentext);
	lentext=gmnumber;
	Xsend(s, &lentext, 1);
	for (i=1;i<=gmnumber;i++)
	{
		gmmiddle[0]=gmid[i]>>8;
		gmmiddle[1]=gmid[i]%256;
		Xsend(s, gmmiddle, 4);
		lentext=strlen(gmtext[i]);
		Xsend(s, &lentext, 1);
		Xsend(s, gmtext[i], lentext);
	}
*/
}

// added by AntiChrist (9/99)
void cMagic::Polymorph(int s, int gmindex, int creaturenumber)
{
/*	int i,k,loopexit=0;
	int id1,id2;

	char sect[512];
	openscript("polymorph.scp");
	sprintf(sect, "POLYMORPHMENU %i",gmindex);
	if (!i_scripts[polymorph_script]->find(sect))
	{
		closescript();
		return;
	}
	read1();
	i=0,k=0;

	do
	{
		read2();
		if (script1[0]!='}' && strcmp("POLYMORPHID",(char*)script1))
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (i<creaturenumber) && (++loopexit < MAXLOOPS) );

	read2();
	k=hex2num(script2);

	closescript();

	id1=k>>8;
	id2=k%256;
//	int cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	tempeffect(pc_currchar, pc_currchar, 18, id1, id2, 0);

	teleport((pc_currchar));

  */
}

// only used for the /heal command
// LB
void cMagic::Heal(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	SERIAL defender=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc_defender = FindCharBySerial( defender );
	if (pc_defender != NULL)
	{
		playSound( pc_currchar, 4);
		doStaticEffect(pc_defender, 4);
		pc_defender->setHp( pc_defender->st() );
		updatestats((pc_defender), 0);
	} else
		sysmessage(s,"Not a valid heal target");

}

// only used for the /recall command
// AntiChrist
void cMagic::Recall(UOXSOCKET s)
{
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);	//Targeted item
//	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	if (pi)
	{
		if ( pi->morex<=200 && pi->morey<=200 )
		{
			sysmessage(s,"That rune has not been marked yet!");
		}
		else
		{
			pc_currchar->MoveTo(pi->morex,pi->morey,pi->morez); //LB
			teleport((pc_currchar));
			sysmessage(s,"You have recalled from the rune.");
		}
	} else sysmessage(s,"Not a valid recall target");//AntiChrist
}

// only used for the /mark command
// AntiChrist
void cMagic::Mark(UOXSOCKET s)
{
	//Targeted item
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi)
	{
		pi->morex=pc_currchar->pos.x;
		pi->morey=pc_currchar->pos.y;
		pi->morez=pc_currchar->pos.z;
		sysmessage(s,"Recall rune marked.");
	} else sysmessage(s,"Not a valid mark target");//AntiChrist
}
/*
void cMagic::CannonTarget(int s)
{
	int i, j, success;
	for (i=0;i<charcount;i++)
	{
		if ((chars[i].ser1==buffer[s][7])&&(chars[i].ser2==buffer[s][8])&&
			(chars[i].ser3==buffer[s][9])&&(chars[i].ser4==buffer[s][10]))
		{
			
			if (CheckMagicReflect(i)) i=cc;
			staticeffect(i, 0x36, 0xB0, 0x09, 0x09);
			soundeffect2(i, 0x02, 0x07);
			for (j=0;j<charcount;j++)
			{
				if (chars[j].x==chars[i].x && chars[j].y==chars[i].y && chars[j].z==chars[i].z)
				{
					npcattacktarget(j, cc);
					if (CheckParry(j, 6))
					{
						MagicDamage(j, pc_currchar->skill[TACTICS]/50);
					}
					else
					{
						MagicDamage(j, pc_currchar->skill[TACTICS]/25);
					}
				}
			}
			return;
		}
	}
}
*/

// only used for the /gate command
// AntiChrist
void cMagic::Gate(UOXSOCKET s)
{
	int n;

	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);	//Targeted item
	if (pi)
	{
		if ( pi->morex<=200 && pi->morey<=200 )
		{
			sysmessage(s,"That rune has not been marked yet!");
		}
		else
		{
//			CHARACTER cc = currchar[s];
			P_CHAR pc_currchar = currchar[s];
			gatex[gatecount][0]=pc_currchar->pos.x;	//create gate a player location
			gatey[gatecount][0]=pc_currchar->pos.y;
			gatez[gatecount][0]=pc_currchar->pos.z;
			gatex[gatecount][1]=pi->morex; //create gate at marked location
			gatey[gatecount][1]=pi->morey;
			gatez[gatecount][1]=pi->morez;

			for (n=0;n<2;n++)
			{
				strcpy((char*)temp,"a blue moongate");
				P_ITEM pi_c = Items->SpawnItem(-1,currchar[s],1,"#",0,0x0f,0x6c,0,0,0);
				if(pi_c != NULL)//AntiChrist - to prevent crashes
				{
					pi_c->setType( 51 + n );
					pi_c->pos.x=gatex[gatecount][n];
					pi_c->pos.y=gatey[gatecount][n];
					pi_c->pos.z=gatez[gatecount][n];
					pi_c->gatetime = static_cast<unsigned int>(uiCurrentTime+(SrvParams->gateTimer()*MY_CLOCKS_PER_SEC));
					//clConsole.send("GATETIME:%i UICURRENTTIME:%d GETCLOCK:%d\n",SrvParms->gatetimer,uiCurrentTime,getclock());
					pi_c->gatenumber=gatecount;
					pi_c->dir=1;
					cMapObjects::getInstance()->add(pi_c);	//add gate to list of items in the region
					pi_c->update();//AntiChrist
				}
				if (n==1)
				{
					gatecount++;
					if (gatecount>MAXGATES) gatecount=0;
				}
				addid1[s]=0;
				addid2[s]=0;
			}
		}
	} else sysmessage(s,"Not a valid gate target");//AntiChrist
}

void cMagic::Action4Spell( UOXSOCKET s, int num )
{
	/*if (spells[num].action)
		impaction(s, spells[num].action);*/
}

void cMagic::AfterSpellDelay(UOXSOCKET s, P_CHAR pc)
{
	if( Magic->requireTarget( pc->spell() ) )
	{
//		target(s,0,1,0,100, spells[pc->spell()].strToSay );
	}
	else
	{
		Magic->NewCastSpell( s );
	}
	pc->setCasting(false);
	pc->setSpelltime(0);
//	pc->priv2 &= 0xfd; // unfreeze, bugfix LB
	pc->setPriv2(pc->priv2() & 0xfd);
}
