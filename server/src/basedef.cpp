
#include "basedef.h"
#include "globals.h"
#include "wpdefmanager.h"
#include "prototypes.h"

#include <string.h>

cCharBaseDef *cBaseDefManager::getCharBaseDef( unsigned short id )
{
	if( id >= 0x400 )
		return 0;

	return chardefs[ id ];
}

void cBaseDefManager::load()
{
	for( unsigned int i = 0; i < 0x400; ++i )
	{
		// Check for <basechar id="">
		const cElement *element = DefManager->getDefinition( WPDT_CHARBASE, "0x" + QString::number( i, 16 ) );

		if( !element )
		{
			chardefs[i] = 0;
			continue;
		}

		chardefs[i] = new cCharBaseDef;
		chardefs[i]->basesound_ = hex2dec( element->getAttribute( "basesound", "0" ) ).toUShort();
		chardefs[i]->soundmode_ = hex2dec( element->getAttribute( "soundmode", "0" ) ).toUShort();
		chardefs[i]->shrinked_ = hex2dec( element->getAttribute( "shrinked", "0" ) ).toUShort();
		chardefs[i]->flags_ = hex2dec( element->getAttribute( "flags", "0" ) ).toUInt();	
		chardefs[i]->type_ = hex2dec( element->getAttribute( "type", "0" ) ).toUShort();
	}
}

void cBaseDefManager::unload()
{
	unsigned short i = 0;
	while( i < 0x400 )
		delete chardefs[i++];

	memset( &chardefs, 0, 0x400 * sizeof( cCharBaseDef* ) );
}
