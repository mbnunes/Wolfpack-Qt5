
#include "exceptions.h"
#include "engine.h"
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
	cache = new SurfaceCache(500, 463);
	cache->setAutoDelete(true);
	tcache = new TextureCache(500, 463);
	tcache->setAutoDelete(true);
	acache = new ArtAnimationCache(100, 17);
	acache->setAutoDelete(true);
}

cArt::~cArt() {
	tcache->clear();
	delete tcache;
	cache->clear();
	delete cache;
	acache->clear();
	delete acache;
}

cArtAnimation *cArt::readAnimation(unsigned short id, unsigned short hueid, bool partialhue) {
	unsigned int cacheid = getCacheId(id, hueid, partialhue, false, false);

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
			if (width < lineTreshold && maxwidth + width > lineTreshold) {
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
	SDL_Surface *surface = Engine->createSurface(totalwidth, totalheight, false, false, true);
	SDL_FillRect(surface, 0, 0); // Clear to transparent

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
	float textureWidth = (float)surface->w;
	float textureHeight = (float)surface->h;

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

		SurfacePainter32 painter(surface);
		painter.lock();
		for (int y = 0; y < height; ++y) {
			dataStream.device()->at(offset + lookupTable[y] * 2);
			int x = 0;

			while (x < width) {
				unsigned short xoffset, rlength;
				dataStream >> xoffset >> rlength;

				if( !xoffset && !rlength )
					break;

				unsigned int xend = x + xoffset + rlength;
				for( x = x + xoffset; x < xend; ++x ) {
					unsigned int pixel;
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
						
						unsigned int pixel = painter.color(r, g, b);
						painter.setPixel(currentx + x, currenty + y, pixel);
					} else {				
						// Calculate the real pixel value
						if (color != 0) {
							unsigned int pixel = painter.color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8);
							painter.setPixel(currentx + x, currenty + y, pixel);
						}
					}
				}
			}
		}
		painter.unlock();

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
	SDL_FreeSurface(surface);

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
	cache->clear(); // Clear the cache
	acache->clear();
	tcache->clear();

	data.close(); // Close the data file
	index.close(); // Close the index files
}

void cArt::reload() {
	unload();
	load();
}

SDL_Surface *cArt::getLandArt(unsigned short id, bool texture) {
	int offset = -1; // Default to invalid
	int length = 0; // Length of data record
	SDL_Surface *surface = 0; // The land surface
	QDataStream dataStream = this->dataStream; // The stream to read the data from

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
		surface = Engine->createSurface(44, 44, false, false, texture);

		if (!surface) {
			throw Exception(tr("Unable to create new surface."));
		}

		SurfacePainter32 painter(surface);
		SDL_FillRect(surface, 0, 0); // Clear transparency

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

		painter.lock();

		for( y = 0; y < 44; ++y ) {
			x = ( 44 - span ) / 2;

			for( i = 0; i < span; ++i ) {
				unsigned short color = colors[coffset++];
				unsigned int pixel;

				if (color == 0) {
					color = 8; // make sure land tiles aren't transparent
				}

				// Process the color only if its not transparent
				pixel = painter.color((color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8);

				painter.setPixel(x + i, y, pixel);
			}

			if( y < 21 )
				span += 2;
			if( y >= 22 )
				span -= 2;
		}

		painter.unlock();
	}

	return surface;
}

SDL_Surface *cArt::getItemArt(unsigned short id, unsigned short hueid, bool partialhue, bool stacked, bool texture, unsigned int *widthOut, unsigned int *heightOut) {
	int offset = -1; // Default to invalid
	int length = 0; // Length of data record
	stHue *hue = Hues->get(hueid); // Retrieve the hue (for hue=0 its null)
	SDL_Surface *surface = 0; // The land surface
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
		dataStream.device()->at(offset); // 
		
		unsigned int header;
		short width, height;
		dataStream >> header >> width >> height;
		if (widthOut) {
			*widthOut = width;
		}
		if (heightOut) {
			*heightOut = height;
		}
		
		if (width > 0 && height > 0) {
            unsigned short *lookupTable = new unsigned short[height];
			int i;

			for (i = 0; i < height; ++i) {
				dataStream >> lookupTable[i]; // Read in the offset table for every row
			}

			offset = dataStream.device()->at(); // Save offset

			// Create surface and set color key
			if (stacked) {
				surface = Engine->createSurface(width + 5, height + 5, false, true, texture);
			} else {
				surface = Engine->createSurface(width, height, false, true, texture);
			}

			if (!surface) {
				throw Exception(tr("Unable to create new surface."));
			}

			SDL_PixelFormat *pf = surface->format; // Cache pixel format

			unsigned int black;
			black = SDL_MapRGBA(pf, 0, 0, 0, 0);
			SDL_FillRect(surface, 0, black);

			SurfacePainter32 painter(surface);
			painter.lock();

			for (int y = 0; y < height; ++y) {
				dataStream.device()->at(offset + lookupTable[y] * 2);
				int x = 0;

				while (dataStream.device()->at() < offset + length) {
					unsigned short xoffset, rlength;
					dataStream >> xoffset >> rlength;

					if( !xoffset && !rlength )
						break;

					unsigned int xend = x + xoffset + rlength;

					for( x = x + xoffset; x < xend; ++x )
					{
						unsigned int pixel;
						unsigned short color;
						dataStream >> color;

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
							
							//pixel = painter.color(r, g, b); // Calculate the real pixel value
							pixel = SDL_MapRGBA(pf, r, g, b, 255);
						} else {				
							// Calculate the real pixel value
							if (color != 0) {
								//pixel = painter.color(, , );
								pixel = SDL_MapRGBA(pf, (color >> 7) & 0xF8, (color >> 2) & 0xF8, (color << 3) & 0xF8, 255);
							} else {
								pixel = black;
							}							
						}

						// XXX TODO: This is incorrect behaviour. Instead the lower right image should be drawn first
						// Solutions?
						if (stacked) {
							painter.setPixel(x + 5, y + 5, pixel); // Set the stacked portion
							
							// Set the pixels for our run
							// Only override if the pixel isn't painted yet
							if (painter.getPixel(x, y) != black) {
								painter.setPixel(x, y, pixel);
							}
						} else {
							painter.setPixel(x, y, pixel);
						}
					}
				}
			}

			painter.unlock();
			delete [] lookupTable;
		}
	}

	return surface;
}

SDL_Surface *cArt::readUncached(unsigned short id, unsigned short hue, bool partialhue, bool stacked, bool translucent, bool texture, unsigned int *widthOut, unsigned int *heightOut) {
	SDL_Surface *surface = 0;

	if (id >= 0x8000) {
		throw Exception(tr("Trying to read an invalid tile with id 0x%1.").arg(id, 0, 16));
	} else if (id >= 0x4000) {
		surface = getItemArt(id, hue, partialhue, stacked, texture, widthOut, heightOut);

		if (!surface && id != 0) {
			surface = getItemArt(0, hue, partialhue, stacked, texture, widthOut, heightOut);
		}
	} else {
		surface = getLandArt(id, texture);

		if (!surface && id != 0) {
			surface = getLandArt(0, texture);
		}

		if (surface) {
			if (widthOut) {
				*widthOut = 44;
			}
			if (heightOut) {
				*heightOut = 44;
			}
		}
	}

	if (translucent) {
		SDL_SetAlpha(surface, SDL_SRCALPHA, 127);
	}

	return surface;
}

cTexture *cArt::readLandTexture(unsigned short id, unsigned short hue) {
	unsigned int cacheid = getCacheId(id, hue, false, false, false);
	cTexture *result = tcache->find(cacheid);

	if (!result) {
		// Read a new art tile from the data files
		unsigned int width, height;
		SDL_Surface *surface = readUncached(id, hue, false, false, false, true, &width, &height);
		
		if (surface) {
			result = new cTexture(surface);
			result->setRealWidth(width);
			result->setRealHeight(height);
			SDL_FreeSurface(surface);
			if (tcache->insert(cacheid, result)) {
				result->incref();
			}
		}
	} else {
		result->incref();
	}

	return result;
}

cTexture *cArt::readItemTexture(unsigned short id, unsigned short hue, bool partialhue, bool stacked, bool translucent) {
	id += 0x4000;

	unsigned int cacheid = getCacheId(id, hue, partialhue, stacked, translucent);
	cTexture *result = tcache->find(cacheid);

	if (!result) {
		// Read a new art tile from the data files
		unsigned int width, height;
		SDL_Surface *surface = readUncached(id, hue, partialhue, stacked, translucent, true, &width, &height);
		
		if (surface) {
			result = new cTexture(surface);
			result->setRealWidth(width);
			result->setRealHeight(height);
			SDL_FreeSurface(surface);
			if (tcache->insert(cacheid, result)) {
				result->incref();
			}			
		}
	} else {
		result->incref();
	}

	return result;
}

/*
	Art reader
*/
SharedSurface *cArt::read(unsigned short id, unsigned short hue, bool partialhue, bool stacked, bool translucent) {
	unsigned int cacheid = getCacheId(id, hue, partialhue, stacked, translucent);
	SharedSurface *result = cache->find(cacheid);

	if (!result) {
		// Read a new art tile from the data files
		SDL_Surface *surface = readUncached(id, hue, partialhue, stacked, translucent);
		
		if (surface) {
			result = new SharedSurface(surface);
			cache->insert(cacheid, result);
		}
	}

	return result;
}

void cArt::readCursor(stCursor *cursor, unsigned short id, unsigned short hue, bool partialhue) {	
	unsigned int width, height;
	// Read the texture compatible surface
	SDL_Surface *surface = readUncached(0x4000 + id, hue, partialhue, false, false, true, &width, &height);

	// Search for the hotspot of the cursor (x/y offset)
	// Search for the x/y offset
	int xoffset = 0, yoffset = 0;
	int i;

	SurfacePainter32 painter(surface);
	painter.lock();

	/*
		The first row and column are used to determine the x and yoffset of the cursor
		a green dot is used on an otherwise blue background to indicate the x/y offset.
	*/

	// xoffset
	for (i = 1; i < width; ++i) {
		unsigned int color = painter.getPixel(i, 0);
        unsigned char g = painter.green(color);

		if (g != 0) {
			xoffset = i - 1;
			break;
		}
	}

	// yoffset
	for (i = 1; i < height; ++i) {
		unsigned int color = painter.getPixel(0, i);
        unsigned char g = painter.green(color);        

		if (g != 0) {
			yoffset = i - 1;
			break;
		}
	}

	painter.unlock();

	cTexture *texture = new cTexture(surface);
	texture->setRealHeight(height);
	texture->setRealWidth(width);
	SDL_FreeSurface(surface);

	// Write the given cursor structure
	cursor->surface = texture;
	cursor->xoffset = xoffset;
	cursor->yoffset = yoffset;
}

cArt *Art = 0; // Global cArt Instance
