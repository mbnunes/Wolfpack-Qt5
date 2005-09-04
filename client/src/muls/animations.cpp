
#include "muls/animations.h"
#include "muls/hues.h"
#include "log.h"
#include "surface.h"
#include <qgl.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qstring.h>

cSequence::cSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue, bool partialHue) {
	refcount = 1;
	body_ = body;
	action_ = action;
	direction_ = direction;
	texture_ = 0;
	hue_ = hue;
	partialHue_ = partialHue;
}

cSequence::~cSequence() {
	Animations->totalSequenceSize_ -= getMemorySize();

	if (texture_) {
		texture_->decref();
	}

	QMap<stSequenceIdent, cSequence*>::iterator it = Animations->SequenceCache.find(ident);
	if (it != Animations->SequenceCache.end()) {
		Animations->SequenceCache.erase(it);
	}
}

// Decrement the internal reference count
void cSequence::decref() {
	if (--refcount == 0) {
		Animations->beforeSequenceDeletion(this);
		if (refcount == 0) {
			delete this;
		}
	}
}

// Increment the internal reference count
void cSequence::incref() {
	++refcount;
}

bool cSequence::hitTest(int frame, int x, int y, bool flip) {
	if (!texture_) {
		return false;
	}

	if (frame < 0 || frame >= frameCount_) {
		return false; // Invalid frame id
	}

	stFrame &info = frames[frame];

	int drawx; // x draw position
	if (flip) {
		// If the image is flipped, centerx needs to be flipped too
		drawx = - (info.width - info.centerx);
	} else {
		drawx = - info.centerx;
	}

	// Don't ask me why this works
	int drawy = - info.height - info.centery;

	if (x < drawx || y < drawy || x >= drawx + info.width || y >= drawy + info.height) {
		return false;
	}

	return true;
}

int cSequence::getFrameTop(int frame) {
	if (!texture_) {
		return 0;
	}

	if (frame < 0 || frame >= frameCount_) {
		return 0; // Invalid frame id
	}

	stFrame &info = frames[frame];
	return - info.height - info.centery;
}

void cSequence::draw(int frame, int cellx, int celly, bool flip, float alpha) {
	if (!texture_) {
		return;
	}

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
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
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

	stHue *hue = 0;

	if (hue_ != 0) {
		hue = Hues->get(hue_);
	}

    // Read the 265 color palette first
	for (int i = 0; i < 256; ++i) {
		input >> pixel; // Read color value		
		if (pixel == 0) {
			palette[i] = 0; // 0 is the color key
		} else {
			// Process the color only if its not transparent
			if (hue && pixel != 0) {
				unsigned char index = (pixel >> 10) & 0x1F;
				unsigned char r = index << 3; // Multiply with 8
				unsigned char g = (pixel >> 2) & 0xF8;
				unsigned char b = (pixel << 3) & 0xF8;

				// Hue either everything or if partial hue is set only if
				// its a gray pixel
				if (!partialHue_ || r == g && g == b) {
					r = hue->colors[index].r;
					g = hue->colors[index].g;
					b = hue->colors[index].b;
				}

				palette[i] = cSurface::color(r, g, b);
			} else {
				palette[i] = cSurface::color(pixel);
			}
		}
	}

	// Save the seek offset
	uint seekOffset = input.device()->pos();

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
		input.device()->seek(seekOffset + 4 + i * 4);

		input >> frameLookup[i]; // Read the seek offset
		
		input.device()->seek(seekOffset + frameLookup[i]); // Seek to the frame data

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
	cSurface *surface = new cSurface(totalwidth, maxheight);
	surface->clear();

	// Used to speed up texel calculation
	float twidth = (float)surface->width();
	float theight = (float)surface->height();

	int drawx = 0, drawy = 0;
	for (int i = 0; i < frameCount_; ++i) {
		input.device()->seek(seekOffset + frameLookup[i] + 8); // Seek to the frame data
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
				surface->setPixel(drawx + frame.centerx + xoffset + i, frame.centery + yoffset, palette[pixel]);
			}
		} while (true);

		// Conveniently precalculate the texture coordinates
		frame.texelTop = drawy / theight;
		frame.texelLeft = drawx / twidth;
		frame.texelRight = (drawx + frame.width) / twidth;
		frame.texelBottom = (drawy + frame.height) / theight;

		drawx += frame.width;
	}

	texture_ = new cTexture(surface);
	delete surface;

	delete [] frameLookup;

	Animations->totalSequenceSize_ += getMemorySize();
}

signed char cAnimations::getFileId(unsigned short &body) const {
	// Is there some sort of override in place?
	FileMap::const_iterator it = fileMapping.find(body);
	if (it == fileMapping.end()) {
		return 0; // No special mapping exists
	}
    
	stFileInfo info = it.value();

	// Only return the file id if it's valid.
	if (info.file < ANIMATION_FILES) {
		// If the file isn't available, return -1
		if (!indexFile[info.file].isOpen() || !dataFile[info.file].isOpen()) {
			return -1;
		}

		body = info.body;
		return info.file;
	} else {
		return -1; // Invalid file id
	}
}

cAnimations::cAnimations() {
	// Clear the body types
	memset(bodyTypes, 0, sizeof(bodyTypes));
	memset(flags, 0, sizeof(flags));

	for (int i = 0; i < ANIMATION_FILES; ++i) {
		indexStream[i].setDevice(&indexFile[i]);
		indexStream[i].setByteOrder(QDataStream::LittleEndian);
		dataStream[i].setDevice(&dataFile[i]);
		dataStream[i].setByteOrder(QDataStream::LittleEndian);
	}

	totalSequenceSize_ = 0;
}

cAnimations::~cAnimations() {
}

void cAnimations::load() {
	// Load essential files
	dataFile[0].setFileName(Utilities::getUoFilename("anim.mul"));
	indexFile[0].setFileName(Utilities::getUoFilename("anim.idx"));

	if (!dataFile[0].open(QIODevice::ReadOnly)) {
		throw Exception(tr("Unable to open animation data from %1.").arg(dataFile[0].fileName()));
	}

	if (!indexFile[0].open(QIODevice::ReadOnly)) {
		throw Exception(tr("Unable to open animation index data from %1.").arg(indexFile[0].fileName()));
	}

	// Load supplemental files
	loadMobTypesTxt();
	loadBodyConvDef();
	loadBodyDef();

	// Load secondary animation files
	for (int i = 1; i < ANIMATION_FILES; ++i) {
		dataFile[i].setFileName(Utilities::getUoFilename(QString("anim%1.mul").arg(i+1)));
		indexFile[i].setFileName(Utilities::getUoFilename(QString("anim%1.idx").arg(i+1)));

		if (!dataFile[i].open(QIODevice::ReadOnly)) {
			Log->print(LOG_WARNING, tr("Unable to find %1. Skipping.\n").arg(dataFile[i].fileName()));
			continue;
		}

		if (!indexFile[i].open(QIODevice::ReadOnly)) {
			dataFile[i].close();
			Log->print(LOG_WARNING, tr("Unable to find %1. Skipping.\n").arg(indexFile[i].fileName()));
			continue;
		}

		Log->print(LOG_NOTICE, tr("Successfully loaded animation file %1.\n").arg(i+1));
	}
}

void cAnimations::unload() {
	foreach(cSequence *sequence, ownedSequences_) {
		sequence->decref();
	}
	ownedSequences_.clear();

	for (int i = 0; i < ANIMATION_FILES; ++i) {
		dataFile[i].close();
		indexFile[i].close();
	}

	fallback.clear();
	fileMapping.clear();
}

bool cAnimations::getFallback(signed char &file, unsigned short &body, unsigned short &hue) const {
	FallbackMap::const_iterator it = fallback.find(body);

	// See if a fallback animation for this exists.
	if (it == fallback.end()) {
		return false; // Invalid animation
	} else {
		body = it.value().body; // We did find a fallback body value
		if (it.value().hue != 0) {
			hue = it.value().hue;
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
			if (body < 200) {
				result = body * 65;
			} else if (body < 400) {
				result = (body - 200) * 110 + 13000;
			} else {
				result = (body - 400) * 175 + 35000;
			}
			break;

		default:
			result = 0;
			break;
	};

	result += action * 5 + direction;

	return result;
}

void cAnimations::loadMobTypesTxt() {
	QFile file(Utilities::getUoFilename("mobtypes.txt"));

	if (!file.open(QFile::ReadOnly)) {
		throw Exception(tr("Unable to open mobile type data from %1.").arg(file.fileName()));
	}

	// Format is: 
	// body type(string) flags
	QTextStream stream(&file);
	while (!stream.atEnd()) {
		QString line = stream.readLine().trimmed();
		
		if (!line.startsWith("#")) {
			QStringList parts = line.split(QRegExp("\\s+"));
			
			// 3 parts required
			if (parts.size() == 3) {
				ushort body = parts[0].toUShort();

				// Check for bogus body value
				if (body >= 4096) {
					Log->print(LOG_WARNING, tr("Body value %1 in mobtypes.txt is too high (Maximum is 4095).\n").arg(body));
					continue;
				}

				flags[body] = parts[2].toUShort(); // Parse flags

				// Parse the body type
                enBodyType type;
				if (parts[1] == "HUMAN") {
					type = HUMAN;
				} else if (parts[1] == "ANIMAL") {
					type = ANIMAL;
				} else if (parts[1] == "MONSTER") {
					type = MONSTER;
				} else if (parts[1] == "EQUIPMENT") {
					type = EQUIPMENT;
				} else {
					Log->print(LOG_WARNING, tr("Unknown body type value in mobtypes.txt for body %1: %2.\n").arg(body).arg(parts[1]));
					continue;
				}

                bodyTypes[body] = type;
			}
		}
	}
}

void cAnimations::loadBodyConvDef() {
	QFile file(Utilities::getUoFilename("bodyconv.def"));

	if (!file.open(QFile::ReadOnly)) {
		throw Exception(tr("Unable to open body conversion data from %1.").arg(file.fileName()));
	}

	// Format is: 
	// body anim2 anim3 anim4	
	QTextStream stream(&file);
	while (!stream.atEnd()) {
		QString line = stream.readLine().trimmed();
		
		if (!line.startsWith("#")) {
			QStringList parts = line.split(QRegExp("\\s+"));
			
			// 4 parts required
			if (parts.size() == 4) {
				unsigned short body = parts[0].toUShort();
				stFileInfo info;
				for (int i = 1; i < 5; ++i) {					
					int newbody = parts[i].toShort();
					if (newbody > 0) {
						info.body = (unsigned short)newbody;
						info.file = i;
						fileMapping.insert(body, info);
						break;
					}
				}
			}
		}
	}
}

void cAnimations::loadBodyDef() {
	QFile file(Utilities::getUoFilename("body.def"));

	if (!file.open(QFile::ReadOnly)) {
		throw Exception(tr("Unable to open body fallback data from %1.").arg(file.fileName()));
	}

	// Format is: 
	// oldbody {newbody, newbody, newbody} newhue
	QRegExp pattern("^\\s*(\\d+)\\s+\\{([^\\}]+)\\}\\s+(\\d+)\\s*$");
	QRegExp splitPattern("\\s*,\\s*");

	QTextStream stream(&file);	
	while (!stream.atEnd()) {
		QString line = stream.readLine();
		
		if (line.contains(pattern)) {
			// Log->print(LOG_NOTICE, tr("Found body translation: %1 to %2 with hue %3.\n").arg(pattern.cap(1)).arg(pattern.cap(2)).arg(pattern.cap(3)));
			// Parse the list of new ids
			QStringList newIds = pattern.cap(2).split(splitPattern);
			unsigned short oldBody = pattern.cap(1).toUShort();
			stFallback entry;
			entry.hue = pattern.cap(3).toUShort();
			if (newIds.size() > 0) {
				entry.body = newIds[0].toUShort();
                fallback.insert(oldBody, entry);		
			}
		}
	}
}

cSequence *cAnimations::readSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue, bool partialhue) {
	// Directions can't be translated directly.
	direction &= 7; // Only the lower 3 bit are valid
	static const unsigned char directionmap[8] = {3, 2, 1, 0, 1, 2, 3, 4};
	direction = directionmap[direction];	

	stSequenceIdent ident;
	ident.body = body;
	ident.action = action;
	ident.direction = direction;
	ident.hue = hue;
	ident.partialhue = partialhue;

	// Try to find the sequence in the cache
	cSequence *result = 0;

	QMap<stSequenceIdent, cSequence*>::iterator it = SequenceCache.find(ident);

	if (it != SequenceCache.end()) {
		result = it.value();
		result->incref();
		return result;
	}

	// No cached sequence was found. Create a new one.
	if (!result) {
		// Translate the body value to a file first
		signed char file = getFileId(body);
		bool load = true;

		// Try to use the fallback table if the file couldn't be found
		if (file == -1) {
			if (!getFallback(file, body, hue)) {
				Log->print(LOG_WARNING, tr("Trying to read invalid animation sequence from file %4 (not found). Body 0x%1, Action 0x%2, Direction 0x%3.\n").arg(body).arg(action).arg(direction).arg(file));
				load = false; // Either the fallback was invalid too or there wasn't a fallback
			}
		}

		int offset, length;
		if (load) {
			// If we do have a valid file id now, try to find the index record for it			
			QDataStream &index = indexStream[file];
			index.device()->seek(getSeekOffset(file, body, action, direction) * 12);
			index >> offset >> length;

			// If the file doesn't have the requested animation, try to fall back
			if (offset == -1 || length == 0) {
				if (!getFallback(file, body, hue)) {
					//Log->print(LOG_WARNING, tr("Trying to read invalid animation sequence. Body 0x%1, Action 0x%2, Direction 0x%3.\n").arg(body).arg(action).arg(direction));
					return 0;
				}

				if (load) {
					// Only try the lookup process once again. if it fails, return 0.
					QDataStream &index = indexStream[file];
					index.device()->seek(getSeekOffset(file, body, action, direction) * 12);
					index >> offset >> length;
					
					// The lookup failed
					if (offset == -1 || length == 0) {
						Log->print(LOG_WARNING, tr("Trying to read invalid animation sequence. Fallback. Body 0x%1, Action 0x%2, Direction 0x%3.\n").arg(body).arg(action).arg(direction));
						load = false; // Invalid animation
					}
				}
			}
		}

		// Now that we have the valid offset and length, seek to the data position
		// and return a new animation
		result = new cSequence(body, action, direction, hue, partialhue);
		result->setIdent(ident);
		if (load) {
			dataFile[file].seek(offset);
			result->load(dataStream[file]);
		}

		SequenceCache.insert(ident, result);

		// Clean up the cache if neccesary
		while (totalSequenceSize_ >= maximumSequenceSize() && !ownedSequences_.isEmpty()) {
			cSequence *sequence = ownedSequences_.front();
			sequence->decref();
			ownedSequences_.pop_front();
		}
	}

	return result;
}

uint cAnimations::cacheSize() const {
	return SequenceCache.size();
}

void cAnimations::beforeSequenceDeletion(cSequence *sequence) {
	if (totalSequenceSize_ >= maximumSequenceSize() || ownedSequences_.contains(sequence)) {
		return; // Let the sequence be deleted
	}

	// Keep a reference to the sequence
	sequence->incref();
	ownedSequences_.append(sequence);
}

cAnimations *Animations = 0;
