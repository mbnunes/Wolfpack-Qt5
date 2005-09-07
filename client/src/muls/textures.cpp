
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

		// Check the id in the texture cache
		QMap<uint, cTexture*>::iterator it = textureCache.find(id);
		if (it != textureCache.end()) {
			result = it.value();
			result->incref();
		}

		if (!result) {
			int size = largeTextures[id] ? 128 : 64;
			cSurface *surface = new cSurface(size, size);			

			// Seek to the start of the texture
			data.seek(offsets[id]);

			ushort *colors, *endPtr;
			if (size == 128) {
				colors = new ushort[128 * 128];
				endPtr = colors + 128 * 128;
				dataStream.readRawData((char*)colors, 128 * 128 * sizeof(ushort));
			} else {
				colors = new ushort[64 * 64];
				endPtr = colors + 64 * 64;
				dataStream.readRawData((char*)colors, 64 * 64 * sizeof(ushort));
			}

			// Read the texture
			unsigned int pixel;
			int x = 0, y = 0;
			ushort *current = colors;
			while (current != endPtr) {
				ushort color = *current;
				current++; // Advance to the next pixel

#if Q_uchar_ORDER != Q_LITTLE_ENDIAN
					color = ((color >> 8) & 0xFF) | ((color & 0xFF) << 8);
#endif

				pixel = cSurface::color(color); // Convert the color
				surface->setPixel(x, y, pixel); // Set the pixel

				// Advance to the next pixel
				if (++x == size) {
					x = 0;
					++y;
				}
			}
			
			delete [] colors;

			// Create the texture object
			result = new cTexture(surface, false);
			delete surface;

			uint *ident = (uint*)result->allocateIdentifier(sizeof(uint));
			*ident = id;
			result->setIdentifier(ident);
			result->setCache(this);
		}

		return result;
	}
}

void cTextures::registerTexture(cTexture *texture) {
	textureCache.insert(*(uint*)texture->identifier(), texture);
}

void cTextures::unregisterTexture(cTexture *texture) {
	textureCache.remove(*(uint*)texture->identifier());
}

cTextures *Textures = 0;
