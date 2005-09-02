
#include "exceptions.h"
#include "muls/textures.h"
#include "surface.h"

static float BilinearTable[32][32][4];
bool BilinearTableCalculated = false;

void CalcBilinearTable() {
    float fracx, fracy;
    for (int y = 0; y < 32; y++)
        for (int x = 0; x < 32; x++)
          {
            fracx = (float) x / 32.0f;
            fracy = (float) y / 32.0f;
            BilinearTable[y][x][0] = fracx * fracy;
            BilinearTable[y][x][1] = (1.0f - fracx) * fracy;
            BilinearTable[y][x][2] = fracx * (1.0f - fracy);
            BilinearTable[y][x][3] = (1.0f - fracx) * (1.0f - fracy);
          }
}

cTextures::cTextures()  {
	dataStream.setDevice(&data);
	dataStream.setByteOrder(QDataStream::LittleEndian);
	voidTexture = 0;
}

cTextures::~cTextures() {
	if (voidTexture) {
		voidTexture->decref();
	}
}

void cTextures::load() {
	data.setFileName(Utilities::getUoFilename("texmaps.mul"));
	QFile index(Utilities::getUoFilename("texidx.mul"));

	// Open files
	if (!data.open(QIODevice::ReadOnly)) {
		throw Exception(tr("Unable to open texture data at %1.").arg(data.fileName()));
	}

	if (!index.open(QIODevice::ReadOnly)) {
		throw Exception(tr("Unable to open texture index at %1.").arg(index.fileName()));
	}

	// Slurp in the entire index data to make loading textures faster
	int length, extra;
	QDataStream indexStream(&index);
	indexStream.setByteOrder(QDataStream::LittleEndian);
	for (int i = 0; i < 4096; ++i) {
        indexStream >> offsets[i] >> length >> extra;
		largeTextures[i] = extra != 0;
	}

	index.close();
}

void cTextures::unload() {
	data.close();
}

void cTextures::reload() {
	unload();
	load();
}

cTexture *cTextures::readTexture(unsigned short id) {
	// Creature a dummy texture if there's nothing to see here...
	if (id > 4096 || offsets[id] < 0) {
		if (!voidTexture) {
			cSurface *result = new cSurface(64, 64);
			result->fill(result->color(255, 0, 255));
			voidTexture = new cTexture(result, false);
			delete result;
		}

		voidTexture->incref();
		return voidTexture;
	} else {
		cTexture *result = 0;

		if (!result) {
			int size = largeTextures[id] ? 128 : 64;
			cSurface *surface = new cSurface(size, size);			

			// Seek to the start of the texture
			data.seek(offsets[id]);

			// Read the texture
			unsigned short color;
			unsigned int pixel;
			for (int y = 0; y < size; ++y) {
				for (int x = 0; x < size; ++x) {
					dataStream >> color; // Read the pixel color (15bit)
					pixel = surface->color(color); // Convert the color
					surface->setPixel(x, y, pixel); // Set the pixel
				}
			}			

			// Create the texture object
			result = new cTexture(surface, false);
			delete surface;
		}

		return result;
	}
}

cTextures *Textures = 0;
