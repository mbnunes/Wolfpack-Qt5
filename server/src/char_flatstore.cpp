
#include "accounts.h"
#include "chars.h"
#include "flatstore/flatstore.h"
#include "flatstore_keys.h"

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
	CHAR_SPEECH,
	CHAR_DISABLEDMSG,
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
	CHAR_GUILDFEATLY,
	CHAR_MURDERRATE,
	CHAR_MENUPRIV,
	CHAR_LOOTLIST,
	CHAR_FOOD,
	CHAR_PROFILE,
	CHAR_DESTINATION,
	CHAR_SEX
};

//			addField( "value", (*it).value );
//			addField( "locktype", (*it).lock );
//			addField( "cap", (*it).cap );

void cChar::save( FlatStore::OutputFile *output, bool first )
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

	if( npc_type() )
		output->chunkData( CHAR_NPCTYPE, (unsigned char)npc_type() );

	// Crappy Value, replace asap
	output->chunkData( CHAR_TIMEUNUSED, (unsigned int)time_unused() );

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
/*
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
	CHAR_SPEECH,
	CHAR_DISABLEDMSG,
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
	CHAR_GUILDFEATLY,
	CHAR_MURDERRATE,
	CHAR_MENUPRIV,
	CHAR_LOOTLIST,
	CHAR_FOOD,
	CHAR_PROFILE,
	CHAR_DESTINATION,
	CHAR_SEX
*/
	output->finishChunkGroup();

	output->startChunkGroup( CHUNK_SKILLS );
	output->finishChunkGroup();

	if( first )
		output->finishObject();
}
