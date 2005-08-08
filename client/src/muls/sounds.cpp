
#include "muls/sounds.h"
#include "log.h"
#include "utilities.h"

#include <qfile.h>
#include <qdatastream.h>

cSounds::cSounds() {
	memset(offsets, -1, sizeof(offsets));
	memset(lengths, 0, sizeof(lengths));
}

void cSounds::load() {
	QFile index(Utilities::getUoFilename("soundidx.mul"));

	if (!index.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open the sound index data at %2.").arg(index.fileName()));
	}

	QDataStream indexStream(&index);
	indexStream.setByteOrder(QDataStream::LittleEndian);

	unsigned int extra; // Dumpster
	for (int i = 0; i < 4069 && !indexStream.atEnd(); ++i) {
		indexStream >> offsets[i] >> lengths[i] >> extra;
	}

	index.close();

	data.setName(Utilities::getUoFilename("sound.mul"));

	if (!data.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open the sound data at %2.").arg(data.fileName()));
	}
}

void cSounds::unload() {
	data.close();
}

cSample *cSounds::readSound(unsigned short id) {
	if (id > 4069 || offsets[id] == -1 || lengths[id] == 0) {
		return 0;
	}
	
	cSample *result = new cSample();
	result->id = id;
	
	data.at(offsets[id] + 40);
	result->data = data.read(lengths[id] - 40);

	return result;
}

cSounds *Sounds = 0;
