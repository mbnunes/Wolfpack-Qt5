
#include <qdatastream.h>
#include <qfile.h>
#include <stdlib.h>

#include "exceptions.h"
#include "utilities.h"
#include "config.h"
#include "muls/maps.h"

cMaps *Maps = 0;

void cFacet::load() {
	// This is stupid. Map1 is actually using Map0, but only for certain areas.
	if (id_ == TRAMMEL) {
		mapData.setName(Utilities::getUoFilename("map0.mul"));
		staticData.setName(Utilities::getUoFilename("statics0.mul"));
		staticIndex.setName(Utilities::getUoFilename("staidx0.mul"));
	} else {
		mapData.setName(Utilities::getUoFilename(QString("map%1.mul").arg(id_)));
		staticData.setName(Utilities::getUoFilename(QString("statics%1.mul").arg(id_)));
		staticIndex.setName(Utilities::getUoFilename(QString("staidx%1.mul").arg(id_)));
	}

	if (!mapData.open(QIODevice::ReadOnly)) {
		if (optional_) {
			enabled_ = false; // Disable the map if it's optional
			return;
		} else {
			throw Exception(tr("Unable to open map data at %1.").arg(mapData.name()));
		}
	}

	if (!staticData.open(QIODevice::ReadOnly)) {
		if (optional_) {
			enabled_ = false; // Disable the map if it's optional
			return;
		} else {
			throw Exception(tr("Unable to open static data at %1.").arg(staticData.name()));
		}
	}

	if (!staticIndex.open(QIODevice::ReadOnly)) {
		if (optional_) {
			enabled_ = false; // Disable the map if it's optional
			return;
		} else {
			throw Exception(tr("Unable to open static index at %1.").arg(staticIndex.name()));
		}
	}

	// Connect to the data streams.
	mapDataStream.setDevice(&mapData);
	mapDataStream.setByteOrder(QDataStream::LittleEndian);
	staticDataStream.setDevice(&staticData);
	staticDataStream.setByteOrder(QDataStream::LittleEndian);
	staticIndexStream.setDevice(&staticIndex);
	staticIndexStream.setByteOrder(QDataStream::LittleEndian);
}

void cFacet::reload() {
	unload();
	load();
}

void cFacet::unload() {
	// Close the opened files
	mapData.close();
	staticData.close();
	staticIndex.close();

	// Clear the caches
	staticCache.clear();
	mapCache.clear();
}

cFacet::cFacet() : staticCache(100, 97), mapCache(100, 97) {
	enabled_ = true;
	optional_ = false; // Trigger an exception if missing
	emptyCell.id = 1;
	emptyCell.z = 0;
}

stMapCell *cFacet::getMapCell(unsigned short x, unsigned short y) {
	// if the coordinates are out of bounds, return an empty cell
	if (!enabled_) {
		return &emptyCell;
	}

	// Wrap around
	x %= width_;
	y %= height_;

	// Calculate the cache offset
	unsigned int blockid =  (x / 8) * (height_ / 8) + (y / 8);

	stMapCell *cells = mapCache.find(blockid);
	
	// Load the cell if its not loaded yet
	if (!cells) {
		// Every block is 196 bytes in size
		unsigned int offset = 196 * blockid;
		mapDataStream.device()->at(offset); // seek to the map block

		QDataStream &input = mapDataStream; // This could be used to make reading from verdata easier

		unsigned int header;
		input >> header; // Skip the block header

		cells = (stMapCell*)malloc(sizeof(stMapCell) * 64);

		for (int i = 0; i < 64; ++i) {
			input >> cells[i].id >> cells[i].z;
		}

		if (!mapCache.insert(blockid, cells)) {
			throw Exception("Unable to insert map block into cache. Memory will leak!");
		}
	}

	unsigned int cellid = (y % 8) * 8 + (x % 8);

	// Return the pointer to the referenced map id
	return cells + cellid;
}

signed char cFacet::getAverageHeight(ushort x, ushort y) {
	// first thing is to get the map where we are standing
	stMapCell *map1 = getMapCell(x, y);

	// if this appears to be a valid land id, <= 2 is invalid
	if (map1->id > 2 && -128 != map1->z) {
		stMapCell *map2 = getMapCell(x + 1, y);
		stMapCell *map3 = getMapCell(x, y + 1);
		stMapCell *map4 = getMapCell(x + 1, y + 1);

		// get three other nearby titles to decide on an average z?
		signed char map2z = map2 ? map2->z : map1->z;
		signed char map3z = map3 ? map3->z : map1->z;
		signed char map4z = map4 ? map4->z : map1->z;

		signed char testz = 0;
		if ( abs( map1->z - map4z ) <= abs( map2z - map3z ) )
		{
			if ( -128 == map4z )
				testz = map1->z;
			else // round down.
				testz = ( signed char ) ( floor( ( map1->z + map4z ) / 2.0 ) );
		}
		else
		{
			if ( -128 == map2z || -128 == map3z )
				testz = map1->z;
			else // round down
				testz = ( signed char ) ( floor( ( map2z + map3z ) / 2.0 ) );
		}
		return testz;
	}

	return map1->z;
}

StaticBlock *cFacet::getStaticBlock(unsigned short x, unsigned short y) {
	if (!enabled_) {
		return 0;
	}

	// Wrap around
	x %= width_;
	y %= height_;

	// Calculate the cache offset
	unsigned int blockid =  (x / 8) * (height_ / 8) + (y / 8);

	StaticBlock *tiles = staticCache.find(blockid);
	
	// Load the cell if its not loaded yet
	if (!tiles) {
		staticIndex.at(blockid * 12); // Seek the block in the index
		int offset, length;
		staticIndexStream >> offset >> length;
		
		if (offset != -1) {
			staticData.at(offset);
			
			// Read in the entire static block
			int count = length / 7;
			if (count > 0) {
				tiles = new StaticBlock(count);
				for (int i = 0; i < count; ++i) {
					stStaticItem &item = tiles->at(i);
					staticDataStream >> item.id >> item.xoffset >> item.yoffset >> item.z >> item.color;
				}
			}
		}

		if (!staticCache.insert(blockid, tiles)) {
			throw Exception("Unable to insert static block into cache. Memory will leak!");
		}
	}

	return tiles;
}

void cMaps::load() {
	// Initialize maps
	facets[FELUCCA].setName("Felucca");
	facets[FELUCCA].setWidth(768 * 8);
	facets[FELUCCA].setHeight(512 * 8);
	facets[FELUCCA].setId(FELUCCA);

	facets[TRAMMEL].setName("Trammel");
	facets[TRAMMEL].setWidth(768 * 8);
	facets[TRAMMEL].setHeight(512 * 8);
	facets[TRAMMEL].setId(TRAMMEL);

	facets[ILSHENAR].setName("Ilshenar");
	facets[ILSHENAR].setWidth(288 * 8);
	facets[ILSHENAR].setHeight(200 * 8);
	facets[ILSHENAR].setId(ILSHENAR);
	facets[ILSHENAR].setOptional(true);

	facets[MALAS].setName("Malas");
	facets[MALAS].setWidth(320 * 8);
	facets[MALAS].setHeight(256 * 8);
	facets[MALAS].setId(MALAS);
	facets[MALAS].setOptional(true);

	facets[TOKUNO].setName("Tokuno");
	facets[TOKUNO].setWidth(181 * 8);
	facets[TOKUNO].setHeight(181 * 8);
	facets[TOKUNO].setId(TOKUNO);
	facets[TOKUNO].setOptional(true);

	for (int i = 0; i < FACETCOUNT; ++i) {
		facets[i].load();
	}
}

void cMaps::unload() {
	for (int i = 0; i < FACETCOUNT; ++i) {
		facets[i].unload();
	}
}

void cMaps::reload() {
	for (int i = 0; i < FACETCOUNT; ++i) {
		facets[i].reload();
	}
}
