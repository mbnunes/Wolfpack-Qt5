
/*!
	FlatStore Implementation file for cUObject
*/

#include "flatstore/flatstore.h"
#include "flatstore_keys.h" // Should be removed somehow (so external objects can register themself)
#include "uobject.h"
#include "world.h"

// Chunk Group: CHUNK_OBJECT
// Keep this Enum private!
enum eObjectKeys
{
	OBJECT_NAME = 0x01,		// String
	OBJECT_BINDMENU,		// String
	OBJECT_MULTI,			// 4 Byte Int
	OBJECT_POS_X,			// 2 Byte Short
	OBJECT_POS_Y,			// 2 Byte Short
	OBJECT_POS_Z,			// 1 Byte Char
	OBJECT_POS_MAP,			// 1 Byte Unsigned Char
	OBJECT_EVENTS,			// String
	OBJECT_DIRECTION
};

void cUObject::save( FlatStore::OutputFile *output, bool first ) throw()
{
	if( first )
		output->startObject( serial(),  CHUNK_UOBJECT );

    output->startChunkGroup( CHUNK_UOBJECT );

	if( !name().isEmpty() )
        output->chunkData( OBJECT_NAME, name().utf8().data() );

	output->chunkData( OBJECT_POS_X, (unsigned short)pos().x );
	output->chunkData( OBJECT_POS_Y, (unsigned short)pos().y );
	output->chunkData( OBJECT_POS_Z, (char)pos().z );
	output->chunkData( OBJECT_POS_MAP, (unsigned char)pos().map );

	if( !bindmenu().isEmpty() )
		output->chunkData( OBJECT_BINDMENU, bindmenu().utf8().data() );

	if( multis() != INVALID_SERIAL )
		output->chunkData( OBJECT_MULTI, (unsigned int)multis() );

	if( !eventList().isEmpty() )
		output->chunkData( OBJECT_EVENTS, eventList().utf8().data() );

	output->chunkData( OBJECT_DIRECTION, (unsigned char)direction() );

	output->finishChunkGroup();

	if( first )
		output->finishObject();
}

bool cUObject::load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile* input ) throw()
{
	// Unknown ChunkGroup
	if( chunkGroup != CHUNK_UOBJECT )
		return false;

	switch( chunkType )
	{
	case OBJECT_NAME:
		name_ = QString::fromUtf8( input->readString() );
		break;

	case OBJECT_POS_X:
		input->readUShort( pos_.x );
		break;

	case OBJECT_POS_Y:
		input->readUShort( pos_.y );
		break;

	case OBJECT_POS_Z:
		input->readChar( (char&)pos_.z );
		break;

	case OBJECT_POS_MAP:
		input->readUChar( pos_.map );
		break;

	case OBJECT_BINDMENU:
		bindmenu_ = QString::fromUtf8( input->readString() );
		break;

	case OBJECT_MULTI:
		input->readUInt( (unsigned int&)multis_ );
		break;

	case OBJECT_EVENTS:
		eventList_ = QString::fromUtf8( input->readString() );
		recreateEvents();
		break;

	case OBJECT_DIRECTION:
		{
			unsigned char temp;
			input->readUChar( temp );
			dir_ = temp;
			break;
		}

	default:
		return false;
	};
	
	return true;
}

bool cUObject::postload() throw()
{
	// Nothing to do here (yet).
	return true;
}
