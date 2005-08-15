
#include <qdatastream.h>
#include <qfile.h>

#include "exceptions.h"
#include "utilities.h"
#include "config.h"
#include "surface.h"
#include "muls/gumpart.h"
#include "muls/hues.h"
#include "muls/verdata.h"

cGumpart::cGumpart() {
	dataStream.setDevice(&data);
	indexStream.setDevice(&index);
	dataStream.setByteOrder(QDataStream::LittleEndian);
	indexStream.setByteOrder(QDataStream::LittleEndian);

	tcache = new TextureCache(100, 173);
}

cGumpart::~cGumpart() {
	delete tcache;
}

void cGumpart::load() {
	// Set filenames
	data.setName(Utilities::getUoFilename("gumpart.mul"));
	index.setName(Utilities::getUoFilename("gumpidx.mul"));

	// Open files
	if (!data.open(QIODevice::ReadWrite)) {
		throw Exception(tr("Unable to open gump data at %1.").arg(data.name()));
	}

	if (!index.open(QIODevice::ReadWrite)) {
		throw Exception(tr("Unable to open gump index at %1.").arg(index.name()));
	}
}

void cGumpart::unload() {
	data.close();
	index.close();
	tcache->clear();
}

void cGumpart::reload() {
	unload();
	load();
}

cSurface *cGumpart::readSurface(unsigned short id, unsigned short hueid, bool partialhue, bool texture) {
	int offset = -1; // Default to invalid
	int length = 0; // Length of data record
	short height, width; // Height and width of gump
	stHue *hue = Hues->get(hueid); // Retrieve the hue (for hue=0 its null)
	cSurface *surface = 0; // The gump surface
	QDataStream &dataStream = this->dataStream; // The stream to read the data from

	stVerdataRecord *patch = Verdata->getPatch(VERDATA_GUMPART, id);
	if (patch) {
		dataStream.setDevice(Verdata->getIoDevice());
		offset = patch->offset;
		length = patch->length;
		height = patch->height;
		width = patch->width;
	} else {
		indexStream.device()->at(12 * id);
		indexStream >> offset >> length >> height >> width; // Read index data
	}

	// Sanity checks
	if (offset >= 0 && length > 0 && width > 0 && height > 0) {
		surface = new cSurface(width, height);
		surface->clear();

        dataStream.device()->at(offset);
	
		// Read the lookup table
		unsigned int *lookupTable = new unsigned int[height];
		int i;

		for (i = 0; i < height; ++i) {
			dataStream >> lookupTable[i];
		}

		// Read every line
		int x, y;
		unsigned short color, length;
		unsigned int xoffset;
		unsigned int pixel;
		for (y = 0; y < height; ++y) {
			dataStream.device()->at(offset + lookupTable[y] * 4);
		
			x = 0; // Read the RLE chunks
			while( x < width ) {				
				dataStream >> color >> length;
				
				// Process the color only if its not transparent
				if( hue && color != 0) {
					unsigned char index = (color >> 10) & 0x1F;
					unsigned char r = index << 3; // Multiply with 8
					unsigned char g = (color >> 2) & 0xF8;
					unsigned char b = (color << 3) & 0xF8;
						
					// Hue either everything or if partial hue is set only if 
					// its a gray pixel
					if (!partialhue || r == g && g == b) {
						r = hue->colors[index].r;
						g = hue->colors[index].g;
						b = hue->colors[index].b;
					}
					
					pixel = surface->color(r, g, b); // Calculate the real pixel value
				} else {				
					// Calculate the real pixel value
					if (color) {
						pixel = surface->color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8);
					} else {
						pixel = 0;
					}
				}

				// Set the pixels for our run
				for( xoffset = 0; xoffset < length; ++xoffset ) {
					surface->setPixel(x + xoffset, y, pixel);
				}

				x += xoffset;
			}
		}

		delete [] lookupTable;
	}
	
	return surface;
}

cTexture *cGumpart::readTexture(unsigned short id, unsigned short hue, bool partialHue) {
	// unsigned int cacheid = ((id & 0xFFF) << 16) | (hue & 0xFFFF) | (partialHue ? (1 << 31) : 0);
	cSurface *surface = readSurface(id, hue, partialHue, true);
	cTexture *texture = new cTexture(surface, true);
	delete surface;
	return texture;
}

cGumpart *Gumpart = 0; // Global cGumpart Instance
