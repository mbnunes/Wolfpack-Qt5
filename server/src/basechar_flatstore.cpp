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

#include "accounts.h"
#include "globals.h"
#include "srvparams.h"
#include "flatstore/flatstore.h"
#include "flatstore_keys.h"
#include "territories.h"
#include "sectors.h"
#include "basechar.h"

// Object-Type: 0x0002
// Character Chunk Keys, Maximum: 256
enum eBaseCharKeys
{
	CHAR_ORGNAME = 0x01,
	CHAR_TITLE,
	CHAR_DIR,
	CHAR_BODY,
	CHAR_ORGBODY,
	CHAR_SKIN,
	CHAR_ORGSKIN,
	CHAR_SAY,
	CHAR_EMOTE,
	CHAR_STRENGTH,
	CHAR_STRENGTHMOD,
	CHAR_DEXTERITY,
	CHAR_DEXTERITYMOD,
	CHAR_INTELLIGENCE,
	CHAR_INTELLIGENCEMOD,
	CHAR_MAXHITPOINTS,
	CHAR_HITPOINTS,
	CHAR_MAXSTAMINA,
	CHAR_STAMINA,
	CHAR_MAXMANA,
	CHAR_MANA,
	CHAR_KARMA,
	CHAR_FAME,
	CHAR_KILLS,
	CHAR_DEATHS,
	CHAR_DEFENSE,
	CHAR_HUNGER,
	CHAR_POISON,
	CHAR_POISONED,
	CHAR_MURDERERTIME,
	CHAR_CRIMINALTIME,
	CHAR_NUTRIMENT,
	CHAR_GENDER,
	CHAR_PROPFLAGS
};

//			addField( "value", (*it).value );
//			addField( "locktype", (*it).lock );
//			addField( "cap", (*it).cap );

void cBaseChar::save( FlatStore::OutputFile *output, bool first ) throw()
{
	if( first )
		output->startObject( serial(), CHUNK_CHAR );

	cUObject::save( output );

	// Save Character specific data
	output->startChunkGroup( CHUNK_CHAR );

	if( isIncognito() )
		output->chunkData( CHAR_ORGNAME, (const char*)orgName_.utf8().data() );

	if( !title_.isEmpty() && !title_.isNull() )
		output->chunkData( CHAR_TITLE, (const char*)title_.utf8().data() );

	if( direction() != 0 )
		output->chunkData( CHAR_DIR, (unsigned char)direction() );

	output->chunkData( CHAR_BODY, (unsigned short)bodyID() );
	output->chunkData( CHAR_ORGBODY, (unsigned short)orgBodyID() );

	if( skin() != 0 )
		output->chunkData( CHAR_SKIN, (unsigned short)skin() );

	if( orgSkin() != 0 )
		output->chunkData( CHAR_ORGSKIN, (unsigned short)orgSkin() );

	if( saycolor() != 0x1700 )
		output->chunkData( CHAR_SAY, (unsigned short)saycolor() );

	if( emoteColor() != 0x23 )
		output->chunkData( CHAR_EMOTE, (unsigned short)emoteColor() );

	if( strength() != 50 )
		output->chunkData( CHAR_STRENGTH, (short)strength() );

	if( strengthMod() )
		output->chunkData( CHAR_STRENGTHMOD, (short)strengthMod() );

	if( dexterity() != 50 )
		output->chunkData( CHAR_DEXTERITY, (short)dexterity() );

	if( dexterityMod() )
		output->chunkData( CHAR_DEXTERITYMOD, (short)dexterityMod() );

	if( intelligence() != 50 )
		output->chunkData( CHAR_INTELLIGENCE, (short)intelligence() );

	if( intelligenceMod() )
		output->chunkData( CHAR_INTELLIGENCEMOD, (short)intelligenceMod() );

	if( maxHitpoints() != 50 )
		output->chunkData( CHAR_MAXHITPOINTS, (short)maxHitpoints() );

	if( hitpoints() != maxHitpoints() )
		output->chunkData( CHAR_HITPOINTS, (short)hitpoints() );

	if( maxStamina() != 50 )
		output->chunkData( CHAR_MAXSTAMINA, (short)maxStamina() );

	if( stamina() != maxStamina() )
		output->chunkData( CHAR_STAMINA, (short)stamina() );

	if( maxMana() != 50 )
		output->chunkData( CHAR_MAXMANA, (short)maxMana() );

	if( mana() != maxMana() )
		output->chunkData( CHAR_MANA, (short)mana() );

	if( karma() )
		output->chunkData( CHAR_KARMA, (int)karma() );

	if( fame() )
		output->chunkData( CHAR_FAME, (int)fame() );

	if( kills() )
		output->chunkData( CHAR_KILLS, (unsigned int)kills() );

	if( deaths() )
		output->chunkData( CHAR_DEATHS, (unsigned int)deaths() );
	
	if( bodyArmor() )
		output->chunkData( CHAR_DEFENSE, (unsigned short )bodyArmor() );

	if( hunger() != 6 )
		output->chunkData( CHAR_HUNGER, (int)hunger() );

	if( poison() )
		output->chunkData( CHAR_POISON, (int)poison() );

	if( poisoned() )
		output->chunkData( CHAR_POISONED, (unsigned int)poisoned() );

	if( murdererTime() )
		output->chunkData( CHAR_MURDERERTIME, (unsigned int)( murdererTime() - uiCurrentTime ) );

	if( criminalTime() )
		output->chunkData( CHAR_CRIMINALTIME, (unsigned int)( criminalTime() - uiCurrentTime ) );

	if( nutriment() )
		output->chunkData( CHAR_NUTRIMENT, (unsigned int)nutriment() );

	if( propertyFlags_ )
		output->chunkData( CHAR_PROPFLAGS, (unsigned int)propertyFlags() );

	if( gender_ )
		output->startChunk( CHAR_GENDER );

	output->finishChunkGroup();

	output->startChunkGroup( CHUNK_SKILLS );
	output->finishChunkGroup();

	if( first )
		output->finishObject();
}

bool cBaseChar::load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile *input ) throw()
{
	if( chunkGroup == CHUNK_SKILLS )
	{
		// Skills
		return true;
	}

	unsigned int temp;
	
	if( chunkGroup != CHUNK_CHAR )
		return cUObject::load( chunkGroup, chunkType, input );

	switch( chunkType )
	{
	case CHAR_ORGNAME:
		orgName_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_TITLE:
		title_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_BODY:
		input->readUShort( bodyID_ );
		break;

	case CHAR_ORGBODY:
		input->readUShort( orgBodyID_ );
		break;

	case CHAR_SKIN:
		input->readUShort( skin_ );
		break;

	case CHAR_ORGSKIN:
		input->readUShort( orgSkin_ );
		break;

	case CHAR_SAY:
		input->readUShort( saycolor_ );
		break;

	case CHAR_EMOTE:
		input->readUShort( emoteColor_ );
		break;

	case CHAR_STRENGTH:
		input->readShort( strength_ );
		break;

	case CHAR_STRENGTHMOD:
		input->readShort( strengthMod_ );
		break;

	case CHAR_DEXTERITY:
		input->readShort( dexterity_ );
		break;

	case CHAR_DEXTERITYMOD:
		input->readShort( dexterityMod_ );
		break;

	case CHAR_INTELLIGENCE:
		input->readShort( intelligence_ );
		break;

	case CHAR_INTELLIGENCEMOD:
		input->readShort( intelligenceMod_ );
		break;

	case CHAR_MAXHITPOINTS:
		input->readUShort( maxHitpoints_ );
		break;

	case CHAR_HITPOINTS:
		input->readShort( hitpoints_ );
		break;

	case CHAR_MAXSTAMINA:
		input->readUShort( maxStamina_ );
		break;

	case CHAR_STAMINA:
		input->readShort( stamina_ );
		break;

	case CHAR_MAXMANA:
		input->readUShort( maxMana_ );
		break;

	case CHAR_MANA:
		input->readShort( mana_ );
		break;

	case CHAR_KARMA:
		input->readShort( karma_ );
		break;

	case CHAR_FAME:
		input->readShort( fame_ );
		break;

	case CHAR_KILLS:
		input->readUShort( kills_ );
		break;

	case CHAR_DEATHS:
		input->readUShort( deaths_ );
		break;

	case CHAR_DEFENSE:
		input->readUShort( bodyArmor_ );
		break;

	case CHAR_HUNGER:
		input->readInt( hunger_ );
		break;

	case CHAR_POISON:
		input->readInt( poison_ );
		break;

	case CHAR_POISONED:
		input->readUInt( poisoned_ );
		break;

	case CHAR_MURDERERTIME:
		input->readUInt( murdererTime_ );
		break;

	case CHAR_CRIMINALTIME:
		input->readUInt( criminalTime_ );
		break;

	case CHAR_NUTRIMENT:
		input->readUInt( nutriment_ );
		break;

	case CHAR_PROPFLAGS:
		input->readUInt( propertyFlags_ );
		break;

	case CHAR_GENDER:
		gender_ = true;
		break;

	default:
		return false;
	}

	return true;
}


bool cBaseChar::postload() throw()
{
	// if we are not bound to a stablemaster, register us with the map objects
	cTerritory *region = AllTerritories::instance()->region( pos_.x, pos_.y, pos_.map );
	setRegion( region );	

	MapObjects::instance()->add( this );

	return cUObject::postload();
}
