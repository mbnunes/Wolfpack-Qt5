
#include "chars.h"
#include "flatstore/flatstore.h"
#include "flatstore_keys.h"

void cChar::save( FlatStore::OutputFile *output, bool first )
{
	if( first )
		output->startObject( serial(), CHUNK_CHAR );

	// Save Character specific data

	cUObject::save( output );

	if( first )
		output->finishObject();
}
