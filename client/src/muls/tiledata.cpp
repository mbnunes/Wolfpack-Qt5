
#include <qfile.h>
#include <qdatastream.h>

#include "exceptions.h"
#include "utilities.h"
#include "log.h"
#include "muls/tiledata.h"

cTileInfo::cTileInfo() {
	refcount = 1;
	memset(items, 0, sizeof(items));
	memset(land, 0, sizeof(land));
}

cTileInfo::~cTileInfo() {
}

void cTiledata::load() {
	// Set filenames
	QFile data(Utilities::getUoFilename("tiledata.mul"));

	// Open files
	if (!data.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open tile data at %1.").arg(data.name()));
	}

    QDataStream dataStream(&data);
	dataStream.setByteOrder(QDataStream::LittleEndian);

	// Read 512 land groups
	unsigned int i = 0; // Current tile id
	unsigned int group;
	for (group = 0; group < 512; ++group) {
		unsigned int header;
		dataStream >> header; // Skip 4 byte header

		// Read 32 tiles
		for (unsigned int j = 0; j < 32; ++j) {
			cLandTileInfo *info = new cLandTileInfo; // Create new tile info instance

			dataStream >> info->flags_ >> info->texture_;

			char name[20] = {0, };
			dataStream.readRawBytes(&name[0], 20);
			info->name_ = name;

            land[i++] = info;
		}
	}

	Log->print(LOG_TRACE, tr("Finished loading the land tile information.\n"));

	i = 0; // Start over at index 0
	for (group = 0; group < 512; ++group) {
		unsigned int header;
		dataStream >> header; // Skip 4 byte header

		// Read 32 tiles
		for (unsigned int j = 0; j < 32; ++j) {
			cItemTileInfo *info = new cItemTileInfo; // Create new tile info instance

			// Read the tile information
			dataStream >> info->flags_ >> info->weight_ >> info->quality_ >> info->unknown1_ >> info->unknown2_
				>> info->quantity_ >> info->animation_ >> info->unknown3_ >> info->hue_ >> info->unknown4_
				>> info->unknown5_ >> info->height_;

			char name[20] = {0, };
			dataStream.readRawBytes(&name[0], 20);
			info->name_ = name;

            items[i++] = info;
		}
	}

	Log->print(LOG_TRACE, tr("Finished loading the item tile information.\n"));
}

void cTiledata::unload() {
	for (int i = 0; i < 0x4000; ++i) {
		land[i]->decref();
		items[i]->decref();
	}
}

void cTiledata::reload() {
	unload();
	load();
}

cTiledata *Tiledata = 0;
