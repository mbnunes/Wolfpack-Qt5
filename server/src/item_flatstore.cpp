
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

void cItem::save( FlatStore::OutputFile *output, bool first )
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
