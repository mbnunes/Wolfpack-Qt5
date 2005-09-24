
#include "utilities.h"
#include "config.h"
#include "log.h"
#include "muls/verdata.h"

#include <QtCore>

cVerdata *Verdata = 0; // Global cVerdata Instance

cVerdata::cVerdata() {
	enabled = false; // Disable by default
}

cVerdata::~cVerdata() {
}

void cVerdata::load() {	
	QString filename = Utilities::getUoFilename("verdata.mul"); // Make sure that the filename translation is in the config

	if (!Config->useVerdata()) {
		Log->print(LOG_DEBUG, tr("Ignoring verdata.mul because of configuration settings.\n"));
		return; // The config says we shouldn't use the verdata
	}

	// Read the verdata data.
	data.setFileName(filename);

	if (!data.open(QIODevice::ReadOnly)) {
		Log->print(LOG_DEBUG, tr("Ignoring verdata.mul because it doesn't exist at %1.\n").arg(filename));
		return;
	}

	QDataStream dataStream(&data);
	dataStream.setByteOrder(QDataStream::LittleEndian);
	
	unsigned int count;
	dataStream >> count; // Number of records in file

	// Read Records
	unsigned int realCount = 0;
	for (unsigned int i = 0; i < count; ++i) {
		unsigned int fileid, blockid;
		stVerdataRecord record;
		dataStream >> fileid >> blockid >> record.offset >> record.length >> record.height >> record.width;

		if (fileid < VERDATA_COUNT && (record.offset < 0 || (uint)record.offset < data.size())) {
			records[fileid].insert(blockid, record); // Save the record
			//Log->print(LOG_DEBUG, tr("Loaded patched block %1 for file 0x%2 with length %3 from verdata.\n").arg(blockid).arg(fileid, 0, 16).arg(record.length));
			++realCount;
		}
	}

	Log->print(LOG_DEBUG, tr("Loaded %1 patched blocks from verdata.mul which has originally %2 blocks.\n").arg(realCount).arg(count));
	enabled = true;
}

void cVerdata::unload() {
	// Clear all verdata caches
}

void cVerdata::reload() {
	unload();
	load();
}
