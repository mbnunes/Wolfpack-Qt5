
#include "accounts.h"
#include "globals.h"
#include "chars.h"
#include "srvparams.h"
#include "flatstore/flatstore.h"
#include "flatstore_keys.h"
#include "territories.h"

static cUObject* productCreator()
{
	return new cChar;
}

void cChar::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT /*! STRAIGHT_JOIN SQL_SMALL_RESULT */ uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cChar' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType( "cChar", productCreator );
	UObjectFactory::instance()->registerType( QString::number( CHUNK_CHAR ), productCreator );
	UObjectFactory::instance()->registerSqlQuery( "cChar", sqlString );
}

// Object-Type: 0x0002
// Character Chunk Keys, Maximum: 256
enum eCharKeys
{
	CHAR_ORGNAME = 0x01,
	CHAR_TITLE,
	CHAR_ACCOUNT,
	CHAR_GUILDTYPE,
	CHAR_GUILDTRAITOR,
	CHAR_CELL,
	CHAR_DIR,
	CHAR_BODY,
	CHAR_XBODY,
	CHAR_SKIN,
	CHAR_XSKIN,
	CHAR_PRIV,
	CHAR_PRIV2,
	CHAR_STABLEMASTER,
	CHAR_NPCTYPE,
	CHAR_TIMEUNUSED,
	CHAR_FONT,
	CHAR_SAY,
	CHAR_EMOTE,
	CHAR_STRENGTH,
	CHAR_STRENGTH2,
	CHAR_DEXTERITY,
	CHAR_DEXTERITY2,
	CHAR_INTELLIGENCE,
	CHAR_INTELLIGENCE2,
	CHAR_HITPOINTS,
	CHAR_SPAWNREGION,
	CHAR_STAMINA,
	CHAR_MANA,
	CHAR_HOLDGOLD,
	CHAR_SHOP,
	CHAR_OWNER,
	CHAR_KARMA,
	CHAR_FAME,
	CHAR_KILLS,
	CHAR_DEATHS,
	CHAR_DEAD,
	CHAR_FIXEDLIGHT,
	CHAR_CANTRAIN,
	CHAR_DEFENSE,
	CHAR_LODAMAGE,
	CHAR_HIDAMAGE,
	CHAR_WAR,
	CHAR_NPCWANDER,
	CHAR_OLDNPCWANDER,
	CHAR_CARVE,
	CHAR_FX1,
	CHAR_FY1,
	CHAR_FX2,
	CHAR_FY2,
	CHAR_FZ1,
	CHAR_SPAWN,
	CHAR_HIDDEN,
	CHAR_HUNGER,
	CHAR_NPCAITYPE,
	CHAR_TAMING,
	CHAR_UNSUMMONTIMER,
	CHAR_POISON,
	CHAR_POISONED,
	CHAR_FLEEAT,
	CHAR_REATTACKAT,
	CHAR_SPLIT,
	CHAR_SPLITCHANCE,
	CHAR_GUILDTOGGLE,
	CHAR_GUILDSTONE,
	CHAR_GUILDTITLE,
	CHAR_GUILDFEALTY,
	CHAR_MURDERRATE,
	CHAR_LOOTLIST,
	CHAR_FOOD,
	CHAR_PROFILE,
	CHAR_DESTINATION,
	CHAR_SEX
};

//			addField( "value", (*it).value );
//			addField( "locktype", (*it).lock );
//			addField( "cap", (*it).cap );

void cChar::save( FlatStore::OutputFile *output, bool first ) throw()
{
	if( first )
		output->startObject( serial(), CHUNK_CHAR );

	cUObject::save( output );

	// Save Character specific data
	output->startChunkGroup( CHUNK_CHAR );

	if( incognito() )
		output->chunkData( CHAR_ORGNAME, (const char*)orgname_.utf8().data() );

	if( !title_.isEmpty() && !title_.isNull() )
		output->chunkData( CHAR_TITLE, (const char*)title_.utf8().data() );

	if( account_ )
		output->chunkData( CHAR_ACCOUNT, (const char*)account_->login().utf8().data() );
	
	if( guildType() )
		output->chunkData( CHAR_GUILDTYPE, (short)guildType() );

	if( guildTraitor() )
		output->startChunk( CHAR_GUILDTRAITOR );

	if( cell() != 0 )
		output->chunkData( CHAR_CELL, (unsigned char)cell() );

	if( dir() != 0 )
		output->chunkData( CHAR_DIR, (unsigned char)dir() );

	output->chunkData( CHAR_BODY, (unsigned short)id() );
	output->chunkData( CHAR_XBODY, (unsigned short)xid() );

	if( skin() != 0 )
		output->chunkData( CHAR_SKIN, (unsigned short)skin() );

	if( xskin() != 0 )
		output->chunkData( CHAR_XSKIN, (unsigned short)xskin() );

	if( priv )
		output->chunkData( CHAR_PRIV, (unsigned char)priv );

	if( priv2() )
		output->chunkData( CHAR_PRIV, (unsigned char)priv2() );

	if( stablemaster_serial() != INVALID_SERIAL )
		output->chunkData( CHAR_STABLEMASTER, (unsigned int)stablemaster_serial() );

	if( fonttype() != 3 )
		output->chunkData( CHAR_FONT, (unsigned char)fonttype() );

	if( saycolor() != 0x1700 )
		output->chunkData( CHAR_SAY, (unsigned short)saycolor() );

	if( emotecolor() != 0x23 )
		output->chunkData( CHAR_EMOTE, (unsigned short)emotecolor() );

	if( st() != 50 )
		output->chunkData( CHAR_STRENGTH, (short)st() );

	if( st2() )
		output->chunkData( CHAR_STRENGTH2, (short)st2() );

	if( dx != 50 )
		output->chunkData( CHAR_DEXTERITY, (short)dx );

	if( dx2 )
		output->chunkData( CHAR_DEXTERITY2, (short)dx2 );

	if( in() != 50 )
		output->chunkData( CHAR_INTELLIGENCE, (short)in() );

	if( in2() )
		output->chunkData( CHAR_INTELLIGENCE2, (short)in2() );

	if( hp() != 50 )
		output->chunkData( CHAR_HITPOINTS, (short)hp() );

	if( !spawnregion().isNull() && !spawnregion().isEmpty() )
		output->chunkData( CHAR_SPAWNREGION, (const char*)spawnregion().utf8().data() );

	if( stm() != 50 )
		output->chunkData( CHAR_STAMINA, (short)stm() );

	if( mn() != 50 )
		output->chunkData( CHAR_MANA, (short)mn() );

	if( shop() )
		output->startChunk( CHAR_SHOP );

	if( owner() )
		output->chunkData( CHAR_OWNER, (unsigned int)owner()->serial() );

	if( karma() )
		output->chunkData( CHAR_KARMA, (int)karma() );

	if( fame() )
		output->chunkData( CHAR_FAME, (int)fame() );

	if( kills() )
		output->chunkData( CHAR_KILLS, (unsigned int)kills() );

	if( deaths() )
		output->chunkData( CHAR_DEATHS, (unsigned int)deaths() );
	
	if( dead() )
		output->startChunk( CHAR_DEAD );

	if( fixedlight() )
		output->chunkData( CHAR_FIXEDLIGHT, (unsigned char)fixedlight() );

	if( cantrain() )
		output->startChunk( CHAR_CANTRAIN );

	if( def() )
		output->chunkData( CHAR_DEFENSE, (unsigned int)def() );

	if( lodamage() )
		output->chunkData( CHAR_LODAMAGE, (int)lodamage() );

	if( hidamage() )
		output->chunkData( CHAR_HIDAMAGE, (int)hidamage() );
	
	if( war() )
		output->startChunk( CHAR_WAR );

	if( npcWander() )
		output->chunkData( CHAR_NPCWANDER, (unsigned char)npcWander() );

	if( oldnpcWander() )
		output->chunkData( CHAR_OLDNPCWANDER, (unsigned char)oldnpcWander() );

	if( !carve().isNull() && !carve().isEmpty() )
		output->chunkData( CHAR_CARVE, (const char*)carve().utf8().data() );

	if( fx1() )
		output->chunkData( CHAR_FX1, (int)fx1() );

	if( fx2() )
		output->chunkData( CHAR_FX2, (int)fx2() );

	if( fy1() )
		output->chunkData( CHAR_FY1, (int)fy1() );

	if( fy2() )
		output->chunkData( CHAR_FY2, (int)fy2() );

	if( fz1() )
		output->chunkData( CHAR_FZ1, (char)fz1() );

	if( spawnSerial() != INVALID_SERIAL )
		output->chunkData( CHAR_SPAWN, (unsigned int)spawnSerial() );

	if( hidden() )
		output->chunkData( CHAR_HIDDEN, (unsigned char)hidden() );

	if( hunger() != 6 )
		output->chunkData( CHAR_HUNGER, (int)hunger() );

	if( npcaitype() )
		output->chunkData( CHAR_NPCAITYPE, (int)npcaitype() );

	if( taming() )
		output->chunkData( CHAR_TAMING, (int)taming() );

	if( summontimer() )
		output->chunkData( CHAR_UNSUMMONTIMER, (unsigned int)( summontimer() - uiCurrentTime ) );

	if( poison() )
		output->chunkData( CHAR_POISON, (int)poison() );

	if( poisoned() )
		output->chunkData( CHAR_POISONED, (unsigned int)poisoned() );

	if( fleeat() != SrvParams->npc_base_fleeat() )
		output->chunkData( CHAR_FLEEAT, (short)fleeat() );

	if( reattackat() != SrvParams->npc_base_reattackat() )
		output->chunkData( CHAR_REATTACKAT, (short)reattackat() );

	if( split() )
		output->chunkData( CHAR_SPLIT, (unsigned char)split() );

	if( splitchnc() )
		output->chunkData( CHAR_SPLITCHANCE, (unsigned char)splitchnc() );

	if( guildtoggle() )
		output->startChunk( CHAR_GUILDTOGGLE );

	if( guildstone() != INVALID_SERIAL )
		output->chunkData( CHAR_GUILDSTONE, (unsigned int)guildstone() );

	if( !guildtitle().isNull() && !guildtitle().isEmpty() )
		output->chunkData( CHAR_GUILDTITLE, (const char*)guildtitle().utf8().data() );

	if( guildfealty() != INVALID_SERIAL )
		output->chunkData( CHAR_GUILDFEALTY, (unsigned int)guildfealty() );

	if( murderrate() )
		output->chunkData( CHAR_MURDERRATE, (unsigned int)( murderrate() - uiCurrentTime ) );

	if( !lootList().isNull() && !lootList().isEmpty() )
		output->chunkData( CHAR_LOOTLIST, (const char*)lootList().utf8().data() );

	if( food() )
		output->chunkData( CHAR_FOOD, (unsigned int)food() );

	if( !profile().isNull() && !profile().isEmpty() )
		output->chunkData( CHAR_PROFILE, (const char*)profile().utf8().data() );

	if( ptarg().x || ptarg().y || ptarg().z || ptarg().map )
	{
		output->startChunk( CHAR_DESTINATION );
		output->writeUShort( ptarg().x );
		output->writeUShort( ptarg().y );
		output->writeChar( ptarg().z );
		output->writeUChar( ptarg().map );
	}

	if( sex() )
		output->startChunk( CHAR_SEX );

	output->finishChunkGroup();

	output->startChunkGroup( CHUNK_SKILLS );
	output->finishChunkGroup();

	if( first )
		output->finishObject();
}

bool cChar::load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile *input ) throw()
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
		orgname_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_TITLE:
		title_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_ACCOUNT:
		account_ = Accounts::instance()->getRecord( QString::fromUtf8( input->readString() ) );

		if( account_ )
			account_->addCharacter( this );
		break;

	case CHAR_GUILDTYPE:
		input->readShort( GuildType );
		break;

	case CHAR_GUILDTRAITOR:
		GuildTraitor = true;
		break;

	case CHAR_CELL:
		input->readUChar( cell_ );
		break;

	case CHAR_DIR:
		input->readUChar( dir_ );
		break;

	case CHAR_BODY:
		input->readUShort( id_ );
		break;

	case CHAR_XBODY:
		input->readUShort( xid_ );
		break;

	case CHAR_SKIN:
		input->readUShort( skin_ );
		break;

	case CHAR_XSKIN:
		input->readUShort( xskin_ );
		break;

	case CHAR_PRIV:
		input->readUChar( priv );
		break;

	case CHAR_PRIV2:
		input->readUChar( priv2_ );
		break;

	case CHAR_STABLEMASTER:
		input->readUInt( (unsigned int&)stablemaster_serial_ );
		break;

	case CHAR_FONT:
		input->readUChar( fonttype_ );
		break;

	case CHAR_SAY:
		input->readUShort( saycolor_ );
		break;

	case CHAR_EMOTE:
		input->readUShort( emotecolor_ );
		break;

	case CHAR_STRENGTH:
		input->readShort( st_ );
		break;

	case CHAR_STRENGTH2:
		input->readShort( st2_ );
		break;

	case CHAR_DEXTERITY:
		input->readShort( dx );
		break;

	case CHAR_DEXTERITY2:
		input->readShort( dx2 );
		break;

	case CHAR_INTELLIGENCE:
		input->readShort( in_ );
		break;

	case CHAR_INTELLIGENCE2:
		input->readShort( in2_ );
		break;

	case CHAR_HITPOINTS:
		input->readShort( hp_ );
		break;

	case CHAR_SPAWNREGION:
		spawnregion_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_STAMINA:
		input->readShort( stm_ );
		break;

	case CHAR_MANA:
		input->readShort( mn_ );
		break;

	case CHAR_SHOP:
		shop_ = true;
		break;

	case CHAR_OWNER:
		/*
			WARNING
			We are using a Pointer to store an unsigned
			integer here. That assumes the pointer is 
			at least 32 bit. So this software won't work
			in 16 bit environments anymore. It will work
			in 64 bit environments though.
		*/
		input->readUInt( temp );
		owner_ = reinterpret_cast< cChar* >( temp );
		break;

	case CHAR_KARMA:
		input->readInt( karma_ );
		break;

	case CHAR_FAME:
		input->readInt( fame_ );
		break;

	case CHAR_KILLS:
		input->readUInt( kills_ );
		break;

	case CHAR_DEATHS:
		input->readUInt( deaths_ );
		break;

	case CHAR_DEAD:
		dead_= true;
		break;

	case CHAR_FIXEDLIGHT:
		input->readUChar( fixedlight_ );
		break;

	case CHAR_CANTRAIN:
		cantrain_ = true;
		break;

	case CHAR_DEFENSE:
		input->readUInt( def_ );
		break;

	case CHAR_LODAMAGE:
		input->readInt( lodamage_ );
		break;

	case CHAR_HIDAMAGE:
		input->readInt( hidamage_ );
		break;

	case CHAR_WAR:
		war_ = true;
		break;

	case CHAR_NPCWANDER:
		input->readUChar( npcWander_ );
		break;

	case CHAR_OLDNPCWANDER:
		input->readUChar( oldnpcWander_ );
		break;

	case CHAR_CARVE:
		carve_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_FX1:
		input->readInt( fx1_ );
		break;

	case CHAR_FY1:
		input->readInt( fy1_ );
		break;

	case CHAR_FX2:
		input->readInt( fx2_ );
		break;

	case CHAR_FY2:
		input->readInt( fy2_ );
		break;

	case CHAR_FZ1:
		input->readChar( (char&)fz1_ );
		break;

	case CHAR_SPAWN:
		input->readUInt( (unsigned int&)spawnserial_ );
		break;

	case CHAR_HIDDEN:
		input->readUChar( hidden_ );
		break;

	case CHAR_HUNGER:
		input->readInt( hunger_ );
		break;

	case CHAR_NPCAITYPE:
		input->readInt( npcaitype_ );
		break;

	case CHAR_TAMING:
		input->readInt( taming_ );
		break;

	case CHAR_UNSUMMONTIMER:
		input->readUInt( summontimer_ );
		summontimer_ += uiCurrentTime;
		break;

	case CHAR_POISON:
		input->readInt( poison_ );
		break;

	case CHAR_POISONED:
		input->readUInt( poisoned_ );
		break;

	case CHAR_FLEEAT:
		input->readShort( fleeat_ );
		break;

	case CHAR_REATTACKAT:
		input->readShort( reattackat_ );
		break;

	case CHAR_SPLIT:
		input->readUChar( split_ );
		break;

	case CHAR_SPLITCHANCE:
		input->readUChar( splitchnc_ );
		break;

	case CHAR_GUILDTOGGLE:
		guildtoggle_ = true;
		break;

	case CHAR_GUILDSTONE:
		input->readUInt( (unsigned int&)guildstone_ );
		break;

	case CHAR_GUILDTITLE:
		guildtitle_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_GUILDFEALTY:
		input->readUInt( (unsigned int&)guildfealty_ );
		break;

	case CHAR_MURDERRATE:
		input->readUInt( murderrate_ );
		break;

	case CHAR_LOOTLIST:
		loot_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_FOOD:
		input->readUInt( food_ );
		break;

	case CHAR_PROFILE:
		profile_ = QString::fromUtf8( input->readString() );
		break;

	case CHAR_DESTINATION:
		input->readUShort( ptarg_.x );
		input->readUShort( ptarg_.y );
		input->readChar( (char&)ptarg_.z );
		input->readUChar( ptarg_.map );
		break;

	case CHAR_SEX:
		sex_ = true;
		break;

	default:
		return false;
	}

	return true;
}


bool cChar::postload() throw()
{
	// if we are not bound to a stablemaster, register us with the map objects
	cTerritory *region = cAllTerritories::getInstance()->region( pos_.x, pos_.y, pos_.map );
	setRegion( region );	

	return cUObject::postload();
}
