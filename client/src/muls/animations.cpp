
#include "muls/animations.h"
#include "log.h"
#include "engine.h"

cSequence::cSequence(unsigned short body, unsigned char action, unsigned char direction) {
	refcount = 0;
	body_ = body;
	action_ = action;
	direction_ = direction;
	texture_ = 0;
}

cSequence::~cSequence() {
	if (texture_) {
		texture_->decref();
	}
}

void cSequence::draw(int frame, int cellx, int celly, bool flip) {
	if (frame < 0 || frame >= frameCount_) {
		return; // Invalid frame id
	}

	stFrame &info = frames[frame];

	int drawx; // x draw position
	if (flip) {
		// If the image is flipped, centerx needs to be flipped too
		drawx = cellx - (info.width - info.centerx);
	} else {
		drawx = cellx - info.centerx;
	}

	// Don't ask me why this works
	int drawy = celly - info.height - info.centery;

	texture_->bind();
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	if (flip) {
		glTexCoord2f(info.texelRight, info.texelTop); glVertex2i(drawx, drawy);
		glTexCoord2f(info.texelLeft, info.texelTop); glVertex2i(drawx + info.width, drawy);
		glTexCoord2f(info.texelLeft, info.texelBottom); glVertex2i(drawx + info.width, drawy + info.height);
		glTexCoord2f(info.texelRight, info.texelBottom); glVertex2i(drawx, drawy + info.height);
	} else {
		glTexCoord2f(info.texelLeft, info.texelTop); glVertex2i(drawx, drawy);
		glTexCoord2f(info.texelRight, info.texelTop); glVertex2i(drawx + info.width, drawy);
		glTexCoord2f(info.texelRight, info.texelBottom); glVertex2i(drawx + info.width, drawy + info.height);
		glTexCoord2f(info.texelLeft, info.texelBottom); glVertex2i(drawx, drawy + info.height);
	}
	glEnd();
}

void cSequence::load(QDataStream &input) {	
	unsigned int palette[256];
	unsigned short pixel;

    // Read the 265 color palette first
	for (int i = 0; i < 256; ++i) {
		input >> pixel; // Read color value		
		if (pixel == 0) {
			palette[i] = 0; // 0 is the color key
		} else {			
			palette[i] = SurfacePainter32::color(pixel); // Convert to 32-bit for improved speed
		}
	}

	// Save the seek offset
	int seekOffset = input.device()->at();

	// Read frame count
	int frameCount;
	input >> frameCount;
	frameCount_ = frameCount; // Converts to short

	// Resize the frame information array
	frames.resize(frameCount);

	// Create the frame lookup table
	int *frameLookup = new int[frameCount];

	// Totalwidth and Maxheight of the resulting texture
	int maxheight = 0;
	int totalwidth = 0;

	// Read the lookup table for all frames
	// Measure the maximum height and total width of all frames while we're at it
	for (int i = 0; i < frameCount_; ++i) {
		// Seek to the entry in the lookup table
		input.device()->at(seekOffset + 4 + i * 4);

		input >> frameLookup[i]; // Read the seek offset
		
		input.device()->at(seekOffset + frameLookup[i]); // Seek to the frame data

        // Read the frame information header
		stFrame &frame = frames[i];
		input >> frame.centerx >> frame.centery >> frame.width >> frame.height;

		// Calculate max height / total width
		totalwidth += frames[i].width;
		if (frames[i].height > maxheight) {
			maxheight = frames[i].height;
		}
	}

	// Create the surface and read the animation
	SDL_Surface *surface = Engine->createSurface(totalwidth, maxheight, false, false, true);
	SDL_FillRect(surface, 0, 0); // Make Transparent
	SurfacePainter32 painter(surface);

	// Used to speed up texel calculation
	float twidth = (float)surface->w;
	float theight = (float)surface->h;

	painter.lock();
	int drawx = 0, drawy = 0;
	for (int i = 0; i < frameCount_; ++i) {
		input.device()->at(seekOffset + frameLookup[i] + 8); // Seek to the frame data
        stFrame &frame = frames[i]; // Keep a reference to the frame header

		// Start reading the frame runs
		int header = 0;
		short xoffset, yoffset, length; // Chunk properties
		do {
			input >> header; // Read the frame header

			// This is the end marker
			if (header == 0x7FFF7FFF) {
				break;
			}

			// Unpack the header
			xoffset = (header >> 22) & 0x3FF;
			yoffset = (header >> 12) & 0x3FF;
			length = header & 0xFFF;

			// Respect the sign of xoffset
			if (xoffset & 0x200) {
				xoffset |= 0xFC00; // This is suboptimal
			}
			if (yoffset & 0x200) {
				yoffset |= 0xFC00; // This is suboptimal
			}

			// No idea what this is for
			yoffset += frame.height;

			unsigned char pixel;
			for (int i = 0; i < length; ++i) {
				input >> pixel;
				painter.setPixel(drawx + frame.centerx + xoffset + i, frame.centery + yoffset, palette[pixel]);
			}
		} while (true);

		// Conveniently precalculate the texture coordinates
		frame.texelTop = drawy / theight;
		frame.texelLeft = drawx / twidth;
		frame.texelRight = (drawx + frame.width) / twidth;
		frame.texelBottom = (drawy + frame.height) / theight;

		drawx += frame.width;
	}
	painter.unlock();

	texture_ = new cTexture(surface);
	SDL_FreeSurface(surface);

	delete [] frameLookup;
}

signed char cAnimations::getFileId(unsigned short body) const {
	// Is there some sort of override in place?
	FileMap::const_iterator it = fileMapping.find(body);
	if (it == fileMapping.end()) {
		return 0; // No special mapping exists
	}
    
	// Only return the file id if it's valid.
	if (it.data() < ANIMATION_FILES) {
		signed char result = it.data();

		// If the file isn't available, return -1
		if (!indexFile[result].isOpen() || !dataFile[result].isOpen()) {
			result = -1;
		}

		return result;
	} else {
		return -1; // Invalid file id
	}
}

cAnimations::cAnimations() : cache(100, 109) {
	// Clear the body types
	memset(bodyTypes, 0, sizeof(bodyTypes));

	for (int i = 0; i < ANIMATION_FILES; ++i) {
		indexStream[i].setDevice(&indexFile[i]);
		indexStream[i].setByteOrder(QDataStream::LittleEndian);
		dataStream[i].setDevice(&dataFile[i]);
		dataStream[i].setByteOrder(QDataStream::LittleEndian);
	}
}

cAnimations::~cAnimations() {
}

void cAnimations::load() {
	// Load essential files
	dataFile[0].setName(Utilities::getUoFilename("anim.mul"));
	indexFile[0].setName(Utilities::getUoFilename("anim.idx"));

	if (!dataFile[0].open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open animation data from %1.").arg(dataFile[0].name()));
	}

	if (!indexFile[0].open(IO_ReadOnly)) {
		throw Exception(tr("Unable to open animation index data from %1.").arg(indexFile[0].name()));
	}

	// Load supplemental files
	loadMobTypesTxt();
	loadBodyConvDef();
	loadBodyDef();

	// Load secondary animation files
	for (int i = 1; i < ANIMATION_FILES; ++i) {
		dataFile[i].setName(Utilities::getUoFilename(QString("anim%1.mul").arg(i+1)));
		indexFile[i].setName(Utilities::getUoFilename(QString("anim%1.idx").arg(i+1)));

		if (!dataFile[i].open(IO_ReadOnly)) {
			Log->print(LOG_WARNING, tr("Unable to find %1. Skipping.\n").arg(dataFile[i].name()));
			continue;
		}

		if (!indexFile[i].open(IO_ReadOnly)) {
			dataFile[i].close();
			Log->print(LOG_WARNING, tr("Unable to find %1. Skipping.\n").arg(indexFile[i].name()));
			continue;
		}

		Log->print(LOG_WARNING, tr("Successfully loaded animation file %1.\n").arg(i+1));
	}
}

void cAnimations::unload() {
	for (int i = 0; i < ANIMATION_FILES; ++i) {
		dataFile[i].close();
		indexFile[i].close();
	}

	cache.clear();
	fallback.clear();
	fileMapping.clear();
}

bool cAnimations::getFallback(signed char &file, unsigned short &body, unsigned short &hue) const {
	FallbackMap::const_iterator it = fallback.find(body);

	// See if a fallback animation for this exists.
	if (it == fallback.end()) {
		return false; // Invalid animation
	} else {
		body = it.data().body; // We did find a fallback body value
		if (it.data().hue != 0) {
			hue = it.data().hue;
		}

		// Now we have to check if the fallback body value is in a file we have
		file = getFileId(body);

		// A file id of -1 means the file doesn't exist
		return file != -1;
	}
}

unsigned int cAnimations::getSeekOffset(signed char file, unsigned short body, unsigned char action, unsigned char direction) const {
	// Invalid body id
	if (body >= 2048) {
		return 0;
	}

	unsigned int result = 0;

	switch (file) {
		case 0:
			if (body < 200) {
				result = body * 110;
			} else if (body < 400) {
				result = (body - 200) * 65 + 22000;
			} else {
				result = (body - 400) * 175 + 35000;
			}
			break;

		case 1:
			if (body < 200) {
				result = body * 110;
			} else {
				result = (body - 200) * 65 + 22000;
			}
			break;

		case 2:
			if (body < 300) {
				result = body * 65;
			} else if (body < 400) {
				result = (body - 300) * 110 + 33000;
			} else {
				result = (body - 400) * 175 + 35000;
			}
			break;

		case 3:
			result = 0;
			break;

		default:
			result = 0;
			break;
	};

	result += action * 5 + direction;

	return result;
}

void cAnimations::loadMobTypesTxt() {
}

void cAnimations::loadBodyConvDef() {
}

void cAnimations::loadBodyDef() {
}

cSequence *cAnimations::readSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue, bool partialhue) {
	// Directions can't be translated directly.
	direction &= 7; // Only the lower 3 bit are valid
	static const unsigned char directionmap[8] = {4, 3, 2, 1, 0, 1, 2, 3};
	direction = directionmap[direction];	

	// Calculate the cache id for the given parameters
	unsigned int cacheid = getCacheId(body, action, direction, hue, partialhue);

	// Try to find the sequence in the cache
	cSequence *result = 0; //cache.find(cacheid);

	// No cached sequence was found. Create a new one.
	if (!result) {
		// Translate the body value to a file first
		signed char file = getFileId(body);

		// Try to use the fallback table if the file couldn't be found
		if (file == -1) {
			if (!getFallback(file, body, hue)) {
				return 0; // Either the fallback was invalid too or there wasn't a fallback
			}
		}

		// If we do have a valid file id now, try to find the index record for it
		int offset, length;
		QDataStream &index = indexStream[file];
		index.device()->at(getSeekOffset(file, body, action, direction) * 12);
		index >> offset >> length;

		// If the file doesn't have the requested animation, try to fall back
		if (offset == -1 || length == 0) {
			if (!getFallback(file, body, hue)) {
				return 0; // No fallback could be found for this
			}

			// Only try the lookup process once again. if it fails, return 0.
			QDataStream &index = indexStream[file];
			index.device()->at(getSeekOffset(file, body, action, direction) * 12);
			index >> offset >> length;
			
			// The lookup failed
			if (offset == -1 || length == 0) {
				return 0; // Invalid animation
			}
		}

		// Now that we have the valid offset and length, seek to the data position
		// and return a new animation
		result = new cSequence(body, action, direction);
		dataFile[file].at(offset);
		result->load(dataStream[file]);
	}

	return result;
}

cAnimations *Animations = 0;
