
#include <qfile.h>
#include <qdatastream.h>

#include "exceptions.h"
#include "utilities.h"
#include "muls/hues.h"

void cHues::load() {
	// Set filenames
	QFile data(Utilities::getUoFilename("hues.mul"));

	// Open files
	if (!data.open(QIODevice::ReadOnly)) {
		throw Exception(tr("Unable to open hues data at %1.").arg(data.name()));
	}

    QDataStream dataStream(&data);
	dataStream.setByteOrder(QDataStream::LittleEndian);

	// Read 375 groups of hues
	unsigned int i = 0; // Current hue id
	unsigned int group;
	for (group = 0; group < 375; ++group) {
		unsigned int header;
		dataStream >> header; // Skip 4 byte header

		// Read 8 hues
		for (unsigned int j = 0; j < 8; ++j) {
			unsigned short color; // Temporary variable for 16-bit color values

			// Read 32 colors
			for (unsigned int k = 0; k < 32; ++k) {
                dataStream >> color;
				// Save the r,g,b components
				hues[i].colors[k].r = (color >> 7) & 0xF8;
				hues[i].colors[k].g = (color >> 2) & 0xF8;
				hues[i].colors[k].b = (color << 3) & 0xF8;
			}

			// Read Data Start and Data End
			dataStream >> color;
			hues[i].start.r = (color >> 7) & 0xF8;
			hues[i].start.g = (color >> 2) & 0xF8;
			hues[i].start.b = (color << 3) & 0xF8;

			dataStream >> color;
			hues[i].end.r = (color >> 7) & 0xF8;
			hues[i].end.g = (color >> 2) & 0xF8;
			hues[i].end.b = (color << 3) & 0xF8;

			char name[20] = {0, };
			dataStream.readRawBytes(&name[0], 20);

			++i;
		}
	}
}

void cHues::reload() {
	unload();
	load();
}

void cHues::unload() {
	// Nothing to do here... Really...
}

cHues *Hues = 0; // Global instance of hues
