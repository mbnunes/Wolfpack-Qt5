
#include "exceptions.h"
#include "muls/art.h"
#include "muls/hues.h"
#include "muls/verdata.h"

cArtAnimation::cArtAnimation() {
	frames_ = 0;
	texture_ = 0;
	refcount = 1;
}

cArtAnimation::~cArtAnimation() {
	delete [] frames_;
	if (texture_) {
		texture_->decref();
	}
}

cArt::cArt() {
	tcache = new TextureCache(500, 463);
	tcache->setAutoDelete(true);
	acache = new ArtAnimationCache(100, 17);
	acache->setAutoDelete(true);
}

cArt::~cArt() {
	tcache->clear();
	delete tcache;
	acache->clear();
	delete acache;
}

cArtAnimation *cArt::readAnimation(unsigned short id, unsigned short hueid, bool partialhue) {
	unsigned int cacheid = getCacheId(id, hueid, partialhue);

	// Cache lookup first
	cArtAnimation *result = acache->find(cacheid);
	if (result) {
		result->incref();
		return result;
	}

	// Cache look up failed, so rebuild it.
	QMap<unsigned short, stAnimdata>::const_iterator it;

	// Check if there is an animation record for the given tile
	it = animdata.find(id);

	if (it == animdata.end()) {
		return 0; // There is no animation data for this tile
	}

	// Keep a reference to the animdata
    const stAnimdata &info = it.data();

	// Create frame information structure
	stArtFrame *frames = new stArtFrame[info.count];

	const unsigned int lineTreshold = 512; // Try to stay below 512 pixel

	// Measure the total width and height of the resulting texture
	int totalwidth = 0, totalheight = 0;
	int maxwidth = 0, maxheight = 0;
	int totalmaxheight = 0; // Maximum frameheight

	// Save the offsets so we dont need to read them again (fileio: bad)
	int *offsets = new int[info.count];

	// Prepare the data stream for reading
	QDataStream &dataStream = this->dataStream;

	for (int i = 0; i < info.count; ++i) {
		unsigned short frameid = 0x4000 + info.frames[i] + id; // Calculate offset
		unsigned short width, height;

		// Read the offset from artidx.mul
		stVerdataRecord *patch = Verdata->getPatch(VERDATA_ART, id);
		if (patch) {
			dataStream.setDevice(Verdata->getIoDevice());
			offsets[i] = patch->offset;
		} else {
			indexStream.device()->at(12 * frameid);
			indexStream >> offsets[i]; // Read index data
		}

		// Read the width/height
		if (offsets[i] != -1) {
			dataStream.device()->at(offsets[i] + 4);
			dataStream >> width >> height;

			// Calculate the maximum frame height
			if (height > totalmaxheight) {
				totalmaxheight = height;
			}

			// If we would hit the line boundary, wrap around
			if (((unsigned int)width < lineTreshold) && ((unsigned int)(maxwidth + width) > lineTreshold)) {
				if (maxwidth > totalwidth) {
					totalwidth = maxwidth;
				}
				maxwidth = width;
				totalheight += maxheight; // Save the height of the last line
				maxheight = height; // We're the only element of this line for now
			} else {
				// Modify width/height
				if (height > maxheight) {
					maxheight = height;
				}

				maxwidth += width; // Width of this line
			}
		}
	}

	// If there was only one line, we need to fix that here
	if (maxwidth > totalwidth) {
		totalwidth = maxwidth;
	}
	totalheight += maxheight; // Don't forget the height of the current line

	// Not a valid animation
	if (totalwidth == 0 && totalheight == 0) {
		return 0;
	}

	// Create the surface now.
	cSurface *surface = new cSurface(totalwidth, totalheight, true);
	surface->clear();

	// Pre-allocate the lookup table to save time
	unsigned short *lookupTable = new unsigned short[totalmaxheight];

	// Now read the art tiles into the existing surface
	int currentx = 0, currenty = 0; // Current draw offset within the texture
	maxheight = 0; // We need this.

	// Get the used hue
	stHue *hue = 0;
	if (hueid != 0) {
		hue = Hues->get(hueid);
	}

	// Save float width/height of the texture to prevent unneeded conversions
	float textureWidth = (float)surface->width();
	float textureHeight = (float)surface->height();

	for (int i = 0; i < info.count; ++i) {
		unsigned short width, height;
		dataStream.device()->at(offsets[i] + 4);
		dataStream >> width >> height; // Read width / height

		// Skip bogus image
		if (width < 1 || height < 1) {
			continue;
		}

		for (int j = 0; j < height; ++j) {
			dataStream >> lookupTable[j]; // Read in the offset table for every row
		}

		int offset = dataStream.device()->at(); // Save offset

		for (unsigned int y = 0; y < height; ++y) {
			dataStream.device()->at(offset + lookupTable[y] * 2);
			unsigned int x = 0;

			while (x < width) {
				unsigned short xoffset, rlength;
				dataStream >> xoffset >> rlength;

				if( !xoffset && !rlength )
					break;

				unsigned int xend = x + xoffset + rlength;
				for( x = x + xoffset; x < xend; ++x ) {
					unsigned short color;
					dataStream >> color;

					// Process the color only if its not transparent
					if (hue && color != 0) {
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

						unsigned int pixel = surface->color(r, g, b);
						surface->setPixel(currentx + x, currenty + y, pixel);
					} else {
						// Calculate the real pixel value
						if (color != 0) {
							unsigned int pixel = surface->color(color);
							surface->setPixel(currentx + x, currenty + y, pixel);
						}
					}
				}
			}
		}

		// Fill the frame structure
		frames[i].width = width;
		frames[i].height = height;
		frames[i].x = currentx;
		frames[i].y = currenty;
		frames[i].id = id + info.frames[i];
		frames[i].texelLeft = currentx / textureWidth;
		frames[i].texelTop = currenty / textureHeight;
		frames[i].texelRight = (currentx + width) / textureWidth;
		frames[i].texelBottom = (currenty + height) / textureHeight;

		// we need to maintain this for correct lines
		if (height > maxheight) {
			maxheight = height;
		}

		// Advance currentx/currenty
		currentx += width;
		if (currentx >= totalwidth) {
            currenty += maxheight;
			maxheight = 0;
			currentx = 0;
		}
	}

	// Create the result
	result = new cArtAnimation;
	result->setFrameCount(info.count);
	result->setFrameDelay(info.interval);
	result->setStartDelay(info.startdelay);
	result->setFrames(frames); // Frames is now owned by cArtAnimation
	result->setTexture(new cTexture(surface, true));

	// Free the temporary surface
	delete surface;

	delete [] lookupTable;
	delete [] offsets;

	// Try to insert it into the cache
	if (acache->insert(cacheid, result)) {
		result->incref();
	}

	return result;
}

void cArt::load() {
	// Set filenames
	data.setName(Utilities::getUoFilename("art.mul"));
	index.setName(Utilities::getUoFilename("artidx.mul"));

	// Open files
	if (!data.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open art data at %1.").arg(data.name()));
	}

	if (!index.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open the art index at %1.").arg(index.name()));
	}

	// Set the devices for the streams
	dataStream.setDevice(&data);
	dataStream.setByteOrder(QDataStream::LittleEndian);
	indexStream.setDevice(&index);
	indexStream.setByteOrder(QDataStream::LittleEndian);

	// Read animdata.mul
	QFile animdata(Utilities::getUoFilename("animdata.mul"));

	if (!animdata.open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open art animation information at %1.").arg(animdata.name()));
	}

	// Create input stream
	QDataStream input(&animdata);
	input.setByteOrder(QDataStream::LittleEndian);

	stAnimdata info; // Structure we read into
	unsigned char unknown, delay, interval;

	// Read 2048 blocks
	for (int i = 0; i < 2048; ++i) {
		int header;
		input >> header; // Skip header

		// Read 8 tiles
		for (int j = 0; j < 8; ++j) {
			// byte order is irrelevant here
			input.readRawBytes((char*)info.frames, sizeof(info.frames));
			input >> unknown >> info.count >> interval >> delay;

			// Only bother with existing information data
			if (info.count > 0) {
				info.interval = interval * 100;
				info.startdelay = delay * 100;
				this->animdata.insert(i * 8 + j, info);
			}
		}
	}
}

void cArt::unload() {
	acache->clear();
	tcache->clear();

	data.close(); // Close the data file
	index.close(); // Close the index files
}

void cArt::reload() {
	unload();
	load();
}

/*
	NOTE: Land art tiles should be cramped into larger textures.
*/
cSurface *cArt::readLandSurface(unsigned short id, bool texture) {
	int offset = -1; // Default to invalid
	int length = 0; // Length of data record
	cSurface *surface = 0; // The land surface
	QDataStream &dataStream = this->dataStream; // The stream to read the data from

	stVerdataRecord *patch = Verdata->getPatch(VERDATA_ART, id);
	if (patch) {
		dataStream.setDevice(Verdata->getIoDevice());
		offset = patch->offset;
		length = patch->length;
	} else {
		indexStream.device()->at(12 * id);
		indexStream >> offset >> length; // Read index data
	}

	dataStream.setByteOrder(QDataStream::LittleEndian);

	// Sanity checks
	if (offset >= 0 && length > 0) {
		// Create surface and set color key
		surface = new cSurface(44, 44, texture);
		surface->clear();

        dataStream.device()->at(offset);

		// Start Reading the Landtile
		unsigned short colors[1024];
		int i;
		for (i = 0; i < 1024; ++i) {
			dataStream >> colors[i];
		}

		unsigned int coffset = 0;
		int x = 0, y = 0;
		int span = 2;

		for( y = 0; y < 44; ++y ) {
			x = ( 44 - span ) / 2;

			for( i = 0; i < span; ++i ) {
				unsigned short color = colors[coffset++];
				unsigned int pixel;

				if (color == 0) {
					color = 8; // make sure land tiles aren't transparent
				}

				// Process the color only if its not transparent
				pixel = surface->color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8);

				surface->setPixel(x + i, y, pixel);
			}

			if( y < 21 )
				span += 2;
			if( y >= 22 )
				span -= 2;
		}
	}

	return surface;
}

cSurface *cArt::readItemSurface(unsigned short id, unsigned short hueid, bool partialhue, bool texture) {
	int offset = -1; // Default to invalid
	int length = 0; // Length of data record
	stHue *hue = Hues->get(hueid); // Retrieve the hue (for hue=0 its null)
	cSurface *surface = 0; // The land surface
	QDataStream &dataStream = this->dataStream; // The stream to read the data from

	id += 0x4000; // Static items start at id 0x4000

	stVerdataRecord *patch = Verdata->getPatch(VERDATA_ART, id);
	if (patch) {
		dataStream.setDevice(Verdata->getIoDevice());
		offset = patch->offset;
		length = patch->length;
	} else {
		indexStream.device()->at(12 * id);
		indexStream >> offset >> length; // Read index data
	}

	dataStream.setByteOrder(QDataStream::LittleEndian);

	// Sanity checks
	if (offset >= 0 && length > 0) {
		dataStream.device()->at(offset); //

		unsigned int header;
		short width, height;
		dataStream >> header >> width >> height;

		if (width > 0 && height > 0) {
            unsigned short *lookupTable = new unsigned short[height];
			int i;

			for (i = 0; i < height; ++i) {
				dataStream >> lookupTable[i]; // Read in the offset table for every row
			}

			offset = dataStream.device()->at(); // Save offset

			// Create surface and set color key
			surface = new cSurface(width, height, texture);
			surface->clear();

			for (int y = 0; y < height; ++y) {
				dataStream.device()->at(offset + lookupTable[y] * 2);
				unsigned int x = 0;

				while ((int)dataStream.device()->at() < offset + length) {
					unsigned short xoffset, rlength;
					dataStream >> xoffset >> rlength;

					if( !xoffset && !rlength )
						break;

					unsigned int xend = x + xoffset + rlength;

					for( x = x + xoffset; x < xend; ++x ) {
						unsigned int pixel;
						unsigned short color;
						dataStream >> color;

						if ( color == 0 ) {
							continue; // Skip transparent pixels
						}

						// Process the color only if its not transparent
						if( hue ) {
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

							pixel = surface->color(r, g, b);
						} else {
							pixel = surface->color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8);
							surface->setPixel(x, y, pixel);
						}
					}
				}
			}

			delete [] lookupTable;
		}
	}

	return surface;
}

cTexture *cArt::readLandTexture(unsigned short id) {
	unsigned int cacheid = getCacheId(id, 0, false);
	cTexture *result = tcache->find(cacheid);

	if (!result) {
		// Read a new art tile from the data files
		cSurface *surface = readLandSurface(id, true);

		if (surface) {
			result = new cTexture(surface);
			delete surface;
			if (tcache->insert(cacheid, result)) {
				result->incref();
			}
		}
	} else {
		result->incref();
	}

	return result;
}

cTexture *cArt::readItemTexture(unsigned short id, unsigned short hue, bool partialhue) {
	unsigned int cacheid = getCacheId(id + 0x4000, hue, partialhue);
	cTexture *result = tcache->find(cacheid);

	if (!result) {
		// Read a new art tile from the data files
		cSurface *surface = readItemSurface(id, hue, partialhue, true);

		if (surface) {
			result = new cTexture(surface);
			delete surface;
			if (tcache->insert(cacheid, result)) {
				result->incref();
			}
		}
	} else {
		result->incref();
	}

	return result;
}

void cArt::readCursor(stCursor *cursor, unsigned short id, unsigned short hue, bool partialhue) {
	// Read the texture compatible surface
	cSurface *surface = readItemSurface(id, hue, partialhue, true);

	// Search for the hotspot of the cursor (x/y offset)
	// Search for the x/y offset
	int xoffset = 0, yoffset = 0;
	int i;

	/*
		The first row and column are used to determine the x and yoffset of the cursor
		a green dot is used on an otherwise blue background to indicate the x/y offset.
	*/

	// xoffset
	for (i = 1; i < surface->realWidth(); ++i) {
		unsigned int color = surface->getPixel(i, 0);
        unsigned char g = surface->green(color);

		if (g != 0) {
			xoffset = i - 1;
			break;
		}
	}

	// yoffset
	for (i = 1; i < surface->realHeight(); ++i) {
		unsigned int color = surface->getPixel(0, i);
        unsigned char g = surface->green(color);

		if (g != 0) {
			yoffset = i - 1;
			break;
		}
	}

	cTexture *texture = new cTexture(surface);
	delete surface;

	// Write the given cursor structure
	cursor->surface = texture;
	cursor->xoffset = xoffset;
	cursor->yoffset = yoffset;
}

cArt *Art = 0; // Global cArt Instance
