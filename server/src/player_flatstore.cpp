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
#include "player.h"

static cUObject* productCreator()
{
	return new cPlayer;
}

void cPlayer::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT /*! STRAIGHT_JOIN SQL_SMALL_RESULT */ uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cPlayer' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType( "cPlayer", productCreator );
	UObjectFactory::instance()->registerType( QString::number( CHUNK_PLAYER ), productCreator );
	UObjectFactory::instance()->registerSqlQuery( "cPlayer", sqlString );
}

// Object-Type: 0x0002
// Character Chunk Keys, Maximum: 256
enum ePlayerKeys
{
	CHAR_ACCOUNT,
	CHAR_FIXEDLIGHT,
	CHAR_PROFILE,
	CHAR_VISRANGE,
	CHAR_ADDFLAGS
};

//			addField( "value", (*it).value );
//			addField( "locktype", (*it).lock );
//			addField( "cap", (*it).cap );

void cPlayer::save( FlatStore::OutputFile *output, bool first ) throw()
{
	if( first )
		output->startObject( serial(), CHUNK_PLAYER );

	cBaseChar::save( output );

	// Save Character specific data
	output->startChunkGroup( CHUNK_PLAYER );

	if( account_ )
		output->chunkData( CHAR_ACCOUNT, (const char*)account_->login().utf8().data() );
	
	if( fixedLightLevel() )
		output->chunkData( CHAR_FIXEDLIGHT, (unsigned char)fixedLightLevel() );

	if( !profile().isNull() && !profile().isEmpty() )
		output->chunkData( CHAR_PROFILE, (const char*)profile().utf8().data() );

	if( visualRange() )
		output->chunkData( CHAR_VISRANGE, (unsigned char)visualRange() );

	if( additionalFlags() )
		output->chunkData( CHAR_ADDFLAGS, (unsigned int)additionalFlags() );

	output->finishChunkGroup();

	output->startChunkGroup( CHUNK_SKILLS );
	output->finishChunkGroup();

	if( first )
		output->finishObject();
}

bool cPlayer::load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile *input ) throw()
{
	if( chunkGroup == CHUNK_SKILLS )
	{
		// Skills
		return true;
	}

	unsigned int temp;
	
	if( chunkGroup != CHUNK_PLAYER )
		return cBaseChar::load( chunkGroup, chunkType, input );

	switch( chunkType )
	{
	case CHAR_ACCOUNT:
		account_ = Accounts::instance()->getRecord( QString::fromUtf8( input->readString() ) );

		if( account_ )
			account_->addCharacter( this );
		break;

	case CHAR_FIXEDLIGHT:
		input->readUChar( fixedLightLevel_ );
		break;

	case CHAR_PROFILE:
		profile_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_VISRANGE:
		input->readUChar( visualRange_ );
		break;

	case CHAR_ADDFLAGS:
		input->readUInt( additionalFlags_ );
		break;

	default:
		return false;
	}

	return true;
}


bool cPlayer::postload() throw()
{
	return cBaseChar::postload();
}
