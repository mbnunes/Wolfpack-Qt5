
#include "engine.h"
#include "exceptions.h"
#include "muls/textures.h"

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

cTextures::cTextures() : cache(300, 463) {
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
	data.setName(Utilities::getUoFilename("texmaps.mul"));
	QFile index(Utilities::getUoFilename("texidx.mul"));

	// Open files
	if (!data.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open texture data at %1.").arg(data.name()));
	}

	if (!index.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open texture index at %1.").arg(index.name()));
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
	cache.clear();
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
			SDL_Surface *result = Engine->createSurface(64, 64, false, false);
			SDL_PixelFormat *pf = result->format;
			SDL_FillRect(result, 0, SDL_MapRGB(pf, 255, 0, 255));
			voidTexture = new cTexture(result, false);
			SDL_FreeSurface(result);
		}

		voidTexture->incref();
		return voidTexture;
	} else {
		cTexture *result = cache.find(id);

		if (!result) {
			int size = largeTextures[id] ? 128 : 64;
			SDL_Surface *surface = Engine->createSurface(size, size, false, false, true);
			SurfacePainter32 painter(surface);

			// Seek to the start of the texture
			data.at(offsets[id]);

			// Read the texture
			unsigned short color;
			unsigned int pixel;
			for (int y = 0; y < size; ++y) {
				for (int x = 0; x < size; ++x) {
					dataStream >> color; // Read the pixel color (15bit)
					pixel = painter.color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8); // Convert to our surfaces format
					painter.setPixel(x, y, pixel);
				}
			}			

			// Create the texture object
			result = new cTexture(surface, false);
			SDL_FreeSurface(surface);

			if (cache.insert(id, result)) {
				result->incref();
			}
		} else {
			result->incref();
		}

		return result;
	}
}

cTextures *Textures = 0;
