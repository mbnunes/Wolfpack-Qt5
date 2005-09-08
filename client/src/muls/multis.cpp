
#include "muls/multis.h"
#include "utilities.h"

#include <QDataStream>

cMultis::cMultis() {
	cache.setMaxCost(10000); // Maximum of 10 000 items in cache (about 120kb)
	memset(offsets, -1, sizeof(offsets));
	memset(lengths, 0, sizeof(lengths));
}

void cMultis::load() {
	QFile index(Utilities::getUoFilename("multi.idx"));

	if (!index.open(QFile::ReadOnly)) {
		throw Exception(tr("Unable to open the multi index data at %2.").arg(index.fileName()));
	}

	QDataStream indexStream(&index);
	indexStream.setByteOrder(QDataStream::LittleEndian);

	unsigned int extra; // Dumpster
	for (int i = 0; i < multiCount && !indexStream.atEnd(); ++i) {
		indexStream >> offsets[i] >> lengths[i] >> extra;
	}

	index.close();

	data.setFileName(Utilities::getUoFilename("multi.mul"));

	if (!data.open(QFile::ReadOnly)) {
		throw Exception(tr("Unable to open the multi data at %2.").arg(data.fileName()));
	}
}

void cMultis::unload() {
	data.close();
	cache.clear();
}

MultiData cMultis::readMulti(ushort id) {
	id -= 0x4000;
	
	MultiData result;

	if (id >= multiCount || offsets[id] == -1) {
		return result;
	}

	// Check cache first
	MultiData *cacheResult = cache.object(id);
	if (cacheResult) {
		return *cacheResult;
	}
    
	QDataStream input(&data);
	input.setByteOrder(QDataStream::LittleEndian);
	data.seek(offsets[id]);

	ushort itemCount = lengths[id] / 12;

	stMultiItem item;
	uint showItem;
	for (int i = 0; i < itemCount; ++i) {
        input >> item.id >> item.x >> item.y >> item.z >> showItem;
		if (showItem || i == 0) {
			result.append(item);
		}
	}

	cache.insert(id, new MultiData(result), itemCount);

	return result;
}

cMultis *Multis = 0;
