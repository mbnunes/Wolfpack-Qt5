
#include <qdatastream.h>
#include <qfile.h>

#include "exceptions.h"
#include "engine.h"
#include "utilities.h"
#include "config.h"
#include "muls/gumpart.h"
#include "muls/hues.h"
#include "muls/verdata.h"

class stGumpartPrivateImpl {
public:
	
	// NOTE: key & 0xFFFF == Gump ID, key & 0xEFFF0000 >> 16 == hue, 0x80000000 = partial flag
	// SurfaceCache *cache;
	QFile index;
	QFile data;
	QDataStream indexStream;
	QDataStream dataStream;
};

cGumpart::cGumpart() {
	// Should not be exposed to the public.
	// unsigned int cacheSize = Config->getNumber("Cache", "Gumpart Cache Size", 173);

	d = new stGumpartPrivateImpl;

	d->dataStream.setDevice(&d->data);
	d->indexStream.setDevice(&d->index);
	d->dataStream.setByteOrder(QDataStream::LittleEndian);
	d->indexStream.setByteOrder(QDataStream::LittleEndian);

	/*d->cache = new SurfaceCache(100, 173);
	d->cache->setAutoDelete(true);*/
}

cGumpart::~cGumpart() {
	//delete d->cache;
	delete d;
}

void cGumpart::load() {
	// Set filenames
	d->data.setName(Utilities::getUoFilename("gumpart.mul"));
	d->index.setName(Utilities::getUoFilename("gumpidx.mul"));

	// Open files
	if (!d->data.open(IO_ReadWrite)) {
		throw Exception(tr("Unable to open gump data at %1.").arg(d->data.name()));
	}

	if (!d->index.open(IO_ReadWrite)) {
		throw Exception(tr("Unable to open gump index at %1.").arg(d->index.name()));
	}
}

void cGumpart::unload() {
	d->data.close();
	d->index.close();
	//d->cache->clear();
}

void cGumpart::reload() {
	unload();
	load();
}

SDL_Surface *cGumpart::read(unsigned short id, unsigned short hueid, bool partialhue, bool texture, unsigned int *outWidth, unsigned int *outHeight) {
	int offset = -1; // Default to invalid
	int length = 0; // Length of data record
	short height, width; // Height and width of gump
	stHue *hue = Hues->get(hueid); // Retrieve the hue (for hue=0 its null)
	SDL_Surface *surface = 0; // The gump surface
	QDataStream dataStream = d->dataStream; // The stream to read the data from

	stVerdataRecord *patch = Verdata->getPatch(VERDATA_GUMPART, id);
	if (patch) {
		dataStream.setDevice(Verdata->getIoDevice());
		offset = patch->offset;
		length = patch->length;
		height = patch->height;
		width = patch->width;
	} else {
		d->indexStream.device()->at(12 * id);
		d->indexStream >> offset >> length >> height >> width; // Read index data
	}

	// Sanity checks
	if (offset >= 0 && length > 0 && width > 0 && height > 0) {
		if (outWidth) {
			*outWidth = width;
		}
		if (outHeight) {
			*outHeight = height;
		}

		surface = Engine->createSurface(width, height, true, false, texture);
		SDL_SetColorKey(surface, SDL_SRCCOLORKEY, 0);

		if (!surface) {
			throw Exception(tr("Unable to create new surface."));
		}
		
		SurfacePainter32 painter(surface);
		unsigned int transparent = painter.color(0, 255, 0, 0);

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
					
					pixel = painter.color(r, g, b); // Calculate the real pixel value
				} else {				
					// Calculate the real pixel value
					if (color) {
						pixel = painter.color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8);
					} else {
						pixel = transparent;
					}
				}

				// Set the pixels for our run
				for( xoffset = 0; xoffset < length; ++xoffset ) {
					painter.setPixel(x + xoffset, y, pixel);
				}

				x += xoffset;
			}
		}

		delete [] lookupTable;

		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
	
	return surface;
}

cTexture *cGumpart::readTexture(unsigned short id, unsigned short hue, bool partialHue) {
	unsigned int width, height;
	SDL_Surface *surface = read(id, hue, partialHue, true, &width, &height);
	cTexture *texture = new cTexture(surface, true);
	texture->setRealWidth(width);
	texture->setRealHeight(height);
	SDL_FreeSurface(surface);

	return texture;
}

cGumpart *Gumpart = 0; // Global cGumpart Instance
