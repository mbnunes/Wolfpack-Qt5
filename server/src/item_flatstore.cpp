
#include "items.h"
#include "chars.h"
#include "flatstore/flatstore.h"
#include "flatstore_keys.h"
#include "globals.h"

// Object-Type: 0x0001
// Item Chunk Keys, Maximum: 256
enum eItemKeys
{
	ITEM_ID = 0x00,	// USHORT
	ITEM_CREATOR,	// String
	ITEM_COLOR,		// USHORT
	ITEM_CONTAINER,	// UINT
	ITEM_LAYER,		// UCHAR (only for containers)
	ITEM_TYPE,		// UINT
	ITEM_TYPE2,		// UINT
	ITEM_MORE1,
	ITEM_MORE2,
	ITEM_MORE3,
	ITEM_MORE4,
	ITEM_MOREB1,
	ITEM_MOREB2,
	ITEM_MOREB3,
	ITEM_MOREB4,
	ITEM_MOREX,
	ITEM_MOREY,
	ITEM_MOREZ,
	ITEM_AMOUNT,
	ITEM_DOORDIR,
	ITEM_DYE,
	ITEM_DECAYTIME,
	ITEM_ATTACK,
	ITEM_DEFENSE,
	ITEM_HIDAMAGE,
	ITEM_LODAMAGE,
	ITEM_STRENGTH,
	ITEM_TIMEUNUSED,
	ITEM_WEIGHT,
	ITEM_HEALTH,
	ITEM_MAXHEALTH,
	ITEM_RANK,
	ITEM_STRENGTH2,
	ITEM_DEXTERITY,
	ITEM_DEXTERITY2,
	ITEM_INTELLIGENCE,
	ITEM_INTELLIGENCE2,
	ITEM_SPEED,
	ITEM_POISONED,
	ITEM_MAGIC,
	ITEM_OWNER,
	ITEM_VISIBLE,
	ITEM_SPAWN,
	ITEM_DIRECTION,
	ITEM_PRIV,
	ITEM_SELLPRICE,
	ITEM_BUYPRICE,
	ITEM_RESTOCK,
	ITEM_DISABLED,
	ITEM_SPAWNREGION,
	ITEM_GOOD,
	ITEM_DESCRIPTION,
	ITEM_CARVE,
	ITEM_ACCURACY
};

void cItem::save( FlatStore::OutputFile *output, bool first ) throw()
{
	if( first )
		output->startObject( serial(), CHUNK_ITEM );

	cUObject::save( output );

	output->startChunkGroup( CHUNK_ITEM );

	output->chunkData( ITEM_ID, (unsigned short)id() );

	// Save Item Specific Data
	if( !creator().isEmpty() && !creator().isNull() )
		output->chunkData( ITEM_CREATOR, creator().utf8().data() );

	if( color() )
		output->chunkData( ITEM_COLOR, (unsigned short)color() );

	if( container_ )
	{
		output->chunkData( ITEM_CONTAINER, (unsigned int)container_->serial() );

		if( container_->isChar() )
			output->chunkData( ITEM_LAYER, (unsigned char)layer() );
	}

	if( type() )
		output->chunkData( ITEM_TYPE, (unsigned int)type() );

	if( type2() )
		output->chunkData( ITEM_TYPE2, (unsigned int)type2() );

	if( more1() )
		output->chunkData( ITEM_MORE1, (unsigned char)more1() );

	if( more2() )
		output->chunkData( ITEM_MORE2, (unsigned char)more2() );

	if( more3() )
		output->chunkData( ITEM_MORE3, (unsigned char)more3() );

	if( more4() )
		output->chunkData( ITEM_MORE4, (unsigned char)more4() );

	if( moreb1() )
		output->chunkData( ITEM_MOREB1, (unsigned char)moreb1() );

	if( moreb2() )
		output->chunkData( ITEM_MOREB2, (unsigned char)moreb2() );

	if( moreb3() )
		output->chunkData( ITEM_MOREB3, (unsigned char)moreb3() );

	if( moreb4() )
		output->chunkData( ITEM_MOREB4, (unsigned char)moreb4() );

	if( morex() )
		output->chunkData( ITEM_MOREX, (unsigned int)morex() );

	if( morey() )
		output->chunkData( ITEM_MOREY, (unsigned int)morey() );

	if( morez() )
		output->chunkData( ITEM_MOREZ, (unsigned int)morez() );

	if( amount() > 1 )
		output->chunkData( ITEM_AMOUNT, (unsigned short)amount() );

	if( doordir() )
		output->chunkData( ITEM_DOORDIR, (unsigned char)doordir() );

	if( dye() )
		output->chunkData( ITEM_DYE, (unsigned char)dye() );

	if( decaytime() )
		output->chunkData( ITEM_DECAYTIME, (unsigned int)( decaytime() - uiCurrentTime ) );

	if( att() )
		output->chunkData( ITEM_ATTACK, (unsigned int)att() );

	if( def() )
		output->chunkData( ITEM_DEFENSE, (unsigned int)def() );

	if( hidamage() )
		output->chunkData( ITEM_HIDAMAGE, (short)hidamage() );

	if( lodamage() )
		output->chunkData( ITEM_LODAMAGE, (short)lodamage() );

	if( st() )
		output->chunkData( ITEM_STRENGTH, (short)st() );
	
	if( time_unused )
		output->chunkData( ITEM_TIMEUNUSED, (unsigned int)time_unused );

	if( weight_ )
		output->chunkData( ITEM_WEIGHT, (short)weight_ );

	if( hp() )
		output->chunkData( ITEM_HEALTH, (short)hp() );

	if( maxhp() )
		output->chunkData( ITEM_MAXHEALTH, (short)maxhp() );

	if( rank() )
		output->chunkData( ITEM_RANK, (int)rank() );

	if( st2() )
		output->chunkData( ITEM_STRENGTH2, (short)st2() );

	if( dx() )
		output->chunkData( ITEM_DEXTERITY, (short)dx() );
	
	if( dx2() )
		output->chunkData( ITEM_DEXTERITY2, (short)dx2() );

	if( in() )
		output->chunkData( ITEM_INTELLIGENCE, (short)in() );

	if( in2() )
		output->chunkData( ITEM_INTELLIGENCE2, (short)in2() );

	if( speed() )
		output->chunkData( ITEM_SPEED, (short)speed() );

	if( poisoned() )
		output->chunkData( ITEM_POISONED, (unsigned int)poisoned() );

	if( magic() )
		output->chunkData( ITEM_MAGIC, (unsigned char)magic() );

	if( owner() )
		output->chunkData( ITEM_OWNER, (unsigned int)(owner()->serial()) );

	if( visible() )
		output->chunkData( ITEM_VISIBLE, (unsigned char)visible() );

	if( spawnserial != INVALID_SERIAL )
		output->chunkData( ITEM_SPAWN, (unsigned int)spawnserial );

	if( direction() )
		output->chunkData( ITEM_DIRECTION, (unsigned int)direction() );

	if( priv() != 1 )
		output->chunkData( ITEM_PRIV, (unsigned char)priv() );

	if( sellprice() )
		output->chunkData( ITEM_SELLPRICE, (int)sellprice() );

	if( buyprice() )
		output->chunkData( ITEM_BUYPRICE, (int)buyprice() );

	if( restock() != 1 )
		output->chunkData( ITEM_RESTOCK, (short)restock() );

	if( disabled() )
		output->chunkData( ITEM_DISABLED, (int)disabled() );

	if( !spawnregion().isNull() && !spawnregion().isEmpty() )
		output->chunkData( ITEM_SPAWNREGION, spawnregion().utf8().data() );

	if( good() != -1 )
		output->chunkData( ITEM_GOOD, (int)good() );

	if( !description().isNull() && !description().isEmpty() )
		output->chunkData( ITEM_DESCRIPTION, description().utf8().data() );

	if( !carve().isNull() && !carve().isEmpty() )
		output->chunkData( ITEM_CARVE, carve().utf8().data() );
	
	if( accuracy() != 100 )
		output->chunkData( ITEM_ACCURACY, (unsigned short)accuracy() );

	output->finishChunkGroup();

	if( first )
		output->finishObject();
}

bool cItem::load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile *input )
{
	if( chunkGroup != CHUNK_ITEM )
		return cUObject::load( chunkGroup, chunkType, input ); 

	unsigned int temp;

	switch( chunkType )
	{
	case ITEM_ID:
		input->readUShort( id_ );
		break;

	case ITEM_CREATOR:
		creator_ = QString::fromUtf8( input->readString() );
		break;

	case ITEM_COLOR:
		input->readUShort( color_ );
		break;

	case ITEM_CONTAINER:
		/*
		WARNING

		We are temporarily caching the container serial in a 32-bit pointer.
		*/
		input->readUInt( temp );
		container_ = reinterpret_cast< cUObject* >( temp );

		break;

	case ITEM_LAYER:
		input->readUChar( layer_ );
		break;

	case ITEM_TYPE:
		input->readUShort( type_ );
		break;

	case ITEM_TYPE2:
		input->readUShort( type2_ );
		break;

	case ITEM_MORE1:
		input->readUChar( more1_ );
		break;

	case ITEM_MORE2:
		input->readUChar( more2_ );
		break;

	case ITEM_MORE3:
		input->readUChar( more3_ );
		break;

	case ITEM_MORE4:
		input->readUChar( more4_ );
		break;

	case ITEM_MOREB1:
		input->readUChar( moreb1_ );
		break;

	case ITEM_MOREB2:
		input->readUChar( moreb2_ );
		break;

	case ITEM_MOREB3:
		input->readUChar( moreb3_ );
		break;

	case ITEM_MOREB4:
		input->readUChar( moreb4_ );
		break;

	case ITEM_MOREX:
		input->readUInt( morex_ );
		break;

	case ITEM_MOREY:
		input->readUInt( morey_ );
		break;

	case ITEM_MOREZ:
		input->readUInt( morez_ );
		break;

	case ITEM_AMOUNT:
		input->readUShort( amount_ );
		break;

	case ITEM_DOORDIR:
		input->readUChar( doordir_ );
		break;

	case ITEM_DYE:
		input->readUChar( dye_ );
		break;

	case ITEM_DECAYTIME:
		input->readUInt( decaytime_ );
		decaytime_ += uiCurrentTime;
		break;

	case ITEM_ATTACK:
		input->readUInt( att_ );
		break;

	case ITEM_DEFENSE:
		input->readUInt( def_ );
		break;

	case ITEM_HIDAMAGE:
		input->readShort( hidamage_ );
		break;

	case ITEM_LODAMAGE:
		input->readShort( lodamage_ );
		break;

	case ITEM_STRENGTH:
		input->readShort( st_ );
		break;

	case ITEM_TIMEUNUSED:
		input->readUInt( time_unused );
		break;

	case ITEM_WEIGHT:
		input->readShort( weight_ );
		break;

	case ITEM_HEALTH:
		input->readShort( hp_ );
		break;

	case ITEM_MAXHEALTH:
		input->readShort( maxhp_ );
		break;

	case ITEM_RANK:
		input->readInt( rank_ );
		break;

	case ITEM_STRENGTH2:
		input->readShort( st2_ );
		break;

	case ITEM_DEXTERITY:
		input->readShort( dx_ );
		break;

	case ITEM_DEXTERITY2:
		input->readShort( dx2_ );
		break;

	case ITEM_INTELLIGENCE:
		input->readShort( in_ );
		break;

	case ITEM_INTELLIGENCE2:
		input->readShort( in2_ );
		break;

	case ITEM_SPEED:
		input->readShort( speed_ );
		break;

	case ITEM_POISONED:
		input->readUInt( poisoned_ );
		break;

	case ITEM_MAGIC:
		input->readUChar( magic_ );
		break;

	case ITEM_OWNER:
		input->readUInt( (unsigned int&)ownserial_ );
		break;

	case ITEM_VISIBLE:
		input->readUChar( visible_ );
		break;

	case ITEM_SPAWN:
		input->readUInt( (unsigned int&)spawnserial );
		break;

	case ITEM_DIRECTION:
		input->readUChar( dir_ );
		break;

	case ITEM_PRIV:
		input->readUChar( priv_ );
		break;

	case ITEM_SELLPRICE:
		input->readInt( sellprice_ );
		break;

	case ITEM_BUYPRICE:
		input->readInt( buyprice_ );
		break;

	case ITEM_RESTOCK:
		input->readUShort( restock_ );
		break;

	case ITEM_DISABLED:
		input->readUInt( disabled_ );
		break;

	case ITEM_SPAWNREGION:
		spawnregion_ = QString::fromUtf8( input->readString() );
		break;

	case ITEM_GOOD:
		input->readInt( good_ );
		break;

	case ITEM_DESCRIPTION:
		desc_ = QString::fromUtf8( input->readString() );
		break;

	case ITEM_CARVE:
		carve_ = QString::fromUtf8( input->readString() );
		break;

	case ITEM_ACCURACY:
		input->readUShort( accuracy_ );
		break;

	default: 
		return false;
	}

	return true;
}
