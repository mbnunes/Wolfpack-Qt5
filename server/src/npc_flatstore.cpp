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
#include "mapobjects.h"
#include "npc.h"
#include "player.h"
#include "world.h"

static cUObject* productCreator()
{
	return new cNPC;
}

void cNPC::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT /*! STRAIGHT_JOIN SQL_SMALL_RESULT */ uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cNPC' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType( "cNPC", productCreator );
	UObjectFactory::instance()->registerType( QString::number( CHUNK_NPC ), productCreator );
	UObjectFactory::instance()->registerSqlQuery( "cNPC", sqlString );
}

// Object-Type: 0x0002
// Character Chunk Keys, Maximum: 256
enum eNPCKeys
{
	NPC_STABLEMASTER,
	NPC_SPAWNREGION,
	NPC_OWNER,
	NPC_MINDAMAGE,
	NPC_MAXDAMAGE,
	NPC_WANDERTYPE,
	NPC_WANDERX1,
	NPC_WANDERY1,
	NPC_WANDERX2,
	NPC_WANDERY2,
	NPC_WANDERRADIUS,
	NPC_WANDERFOLLOWTARGET,
	NPC_WANDERDEST_X,
	NPC_WANDERDEST_Y,
	NPC_WANDERDEST_Z,
	NPC_WANDERDEST_MAP,
	NPC_CARVE,
	NPC_TAMING_MINSKILL,
	NPC_SUMMONTIME,
	NPC_LOOTLIST,
	NPC_ADDFLAGS
};

//			addField( "value", (*it).value );
//			addField( "locktype", (*it).lock );
//			addField( "cap", (*it).cap );

void cNPC::save( FlatStore::OutputFile *output, bool first ) throw()
{
	if( first )
		output->startObject( serial(), CHUNK_NPC );

	cBaseChar::save( output );

	// Save Character specific data
	output->startChunkGroup( CHUNK_NPC );

	if( stablemasterSerial() != INVALID_SERIAL )
		output->chunkData( NPC_STABLEMASTER, (unsigned int)stablemasterSerial() );

	if( !spawnregion().isNull() && !spawnregion().isEmpty() )
		output->chunkData( NPC_SPAWNREGION, (const char*)spawnregion().utf8().data() );

	if( owner() )
		output->chunkData( NPC_OWNER, (unsigned int)owner()->serial() );

	if( minDamage() )
		output->chunkData( NPC_MINDAMAGE, (int)minDamage() );

	if( maxDamage() )
		output->chunkData( NPC_MAXDAMAGE, (int)maxDamage() );
	
	if( wanderType() == enFreely )
		output->chunkData( NPC_WANDERTYPE, (unsigned char)wanderType() );
	else if( wanderType() == enRectangle )
	{
		output->chunkData( NPC_WANDERTYPE, (unsigned char)wanderType() );
		output->chunkData( NPC_WANDERX1, wanderX1() );
		output->chunkData( NPC_WANDERY1, wanderY1() );
		output->chunkData( NPC_WANDERX2, wanderX2() );
		output->chunkData( NPC_WANDERY2, wanderY2() );
	}
	else if( wanderType() == enCircle )
	{
		output->chunkData( NPC_WANDERTYPE, (unsigned char)wanderType() );
		output->chunkData( NPC_WANDERX1, wanderX1() );
		output->chunkData( NPC_WANDERY1, wanderY1() );
		output->chunkData( NPC_WANDERRADIUS, wanderRadius() );
	}
	else if( wanderType() == enFollowTarget )
	{
		output->chunkData( NPC_WANDERTYPE, (unsigned char)wanderType() );
		output->chunkData( NPC_WANDERFOLLOWTARGET, wanderFollowTarget() );
	}
	else if( wanderType() == enGoToPosition )
	{
		output->chunkData( NPC_WANDERTYPE, (unsigned char)wanderType() );
		output->chunkData( NPC_WANDERDEST_X, wanderDestination().x );
		output->chunkData( NPC_WANDERDEST_Y, wanderDestination().y );
		output->chunkData( NPC_WANDERDEST_Z, wanderDestination().z );
		output->chunkData( NPC_WANDERDEST_MAP, wanderDestination().map );
	}

	if( !carve().isNull() && !carve().isEmpty() )
		output->chunkData( NPC_CARVE, (const char*)carve().utf8().data() );

	if( tamingMinSkill() )
		output->chunkData( NPC_TAMING_MINSKILL, (int)tamingMinSkill() );

	if( summonTime() )
		output->chunkData( NPC_SUMMONTIME, (unsigned int)( summonTime() - uiCurrentTime ) );

	if( !lootList().isNull() && !lootList().isEmpty() )
		output->chunkData( NPC_LOOTLIST, (const char*)lootList().utf8().data() );

	if( additionalFlags() )
		output->chunkData( NPC_ADDFLAGS, additionalFlags() );

	output->finishChunkGroup();

	output->startChunkGroup( CHUNK_SKILLS );
	output->finishChunkGroup();

	if( first )
		output->finishObject();
}

bool cNPC::load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile *input ) throw()
{
	if( chunkGroup == CHUNK_SKILLS )
	{
		// Skills
		return true;
	}

	unsigned int temp;
	
	if( chunkGroup != CHUNK_NPC )
		return cBaseChar::load( chunkGroup, chunkType, input );

	switch( chunkType )
	{
	case NPC_STABLEMASTER:
		input->readUInt( (unsigned int&)stablemasterSerial_ );
		break;

	case NPC_SPAWNREGION:
		spawnregion_ = QString::fromUtf8( input->readString() );
		break;

	case NPC_OWNER:
		input->readUInt( temp );
		owner_ = dynamic_cast<P_PLAYER>(FindCharBySerial( temp ));
		break;

	case NPC_MINDAMAGE:
		input->readUShort( minDamage_ );
		break;

	case NPC_MAXDAMAGE:
		input->readUShort( maxDamage_ );
		break;

	case NPC_WANDERTYPE:
		{
			unsigned char tmp;
			input->readUChar( tmp );
			setWanderType( (enWanderTypes)tmp );
			break;
		}

	case NPC_WANDERX1:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			setWanderX1( tmp );
			break;
		}

	case NPC_WANDERX2:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			setWanderX2( tmp );
			break;
		}

	case NPC_WANDERY1:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			setWanderY1( tmp );
			break;
		}

	case NPC_WANDERY2:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			setWanderY2( tmp );
			break;
		}

	case NPC_WANDERRADIUS:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			setWanderRadius( tmp );
			break;
		}

	case NPC_WANDERFOLLOWTARGET:
		{
			unsigned int tmp;
			input->readUInt( tmp );
			setWanderFollowTarget( tmp );
			break;
		}

	case NPC_WANDERDEST_X:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			Coord_cl tmppos = wanderDestination();
			tmppos.x = tmp;
			setWanderDestination( tmppos );
			break;
		}

	case NPC_WANDERDEST_Y:
		{
			unsigned short tmp;
			input->readUShort( tmp );
			Coord_cl tmppos = wanderDestination();
			tmppos.y = tmp;
			setWanderDestination( tmppos );
			break;
		}

	case NPC_WANDERDEST_Z:
		{
			char tmp;
			input->readChar( tmp );
			Coord_cl tmppos = wanderDestination();
			tmppos.z = tmp;
			setWanderDestination( tmppos );
			break;
		}

	case NPC_WANDERDEST_MAP:
		{
			unsigned char tmp;
			input->readUChar( tmp );
			Coord_cl tmppos = wanderDestination();
			tmppos.map = tmp;
			setWanderDestination( tmppos );
			break;
		}

	case NPC_CARVE:
		carve_ = QString::fromUtf8( input->readString() );
		break;

	case NPC_TAMING_MINSKILL:
		input->readShort( tamingMinSkill_ );
		break;

	case NPC_SUMMONTIME:
		input->readUInt( summonTime_ );
		summonTime_ += uiCurrentTime;
		break;

	case NPC_LOOTLIST:
		lootList_ = QString::fromUtf8( input->readString() );
		break;

	case NPC_ADDFLAGS:
		input->readUInt( additionalFlags_ );
		break;

	default:
		return false;
	}

	return true;
}


bool cNPC::postload() throw()
{
	return cBaseChar::postload();
}
