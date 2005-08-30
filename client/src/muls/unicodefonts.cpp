
#include <q3valuelist.h>

#include "surface.h"
#include "log.h"
#include "exceptions.h"
#include "utilities.h"
#include "muls/unicodefonts.h"
#include "muls/hues.h"
#include <QStringList>

cUnicodeFonts::cUnicodeFonts() {
}

cUnicodeFonts::~cUnicodeFonts() {
}

void cUnicodeFonts::load() {
	QString filename[3];
	filename[0] = Utilities::getUoFilename("unifont.mul");
	filename[1] = Utilities::getUoFilename("unifont1.mul");
	filename[2] = Utilities::getUoFilename("unifont2.mul");

	// Initialize Data Streams
	unsigned int f;
	for (f = 0; f < 3; ++f) {
		data[f].setName(filename[f]); // Try to open the unicode font file
		if (!data[f].open(QIODevice::ReadOnly)) {
			throw Exception(tr("Unable to open the unicode font %1 at %2.").arg(f).arg(filename[f]));
		}

		QDataStream &dataStream = this->dataStream[f]; // Get a reference to the data stream
		dataStream.setDevice(&data[f]); // Set it to the current font input file
		dataStream.setByteOrder(QDataStream::LittleEndian); // Use little endian input
		
		// Read in lookup table
		for (unsigned int i = 0; i < 0x10000; ++i) {
			dataStream >> (unsigned int&)seekOffsets[f][i]; // Read in a seek offset for one of the characters
		}

		Log->print(LOG_MESSAGE, tr("Finished caching the lookup data for unicode font %1.\n").arg(f));
	}
}

void cUnicodeFonts::unload() {
	data[0].close();
	data[1].close();
	data[2].close();
}

void cUnicodeFonts::reload() {
	unload();
	load();
}

cTexture *cUnicodeFonts::buildTextWrapped(unsigned char font, const QString &text, unsigned short maxWidth, unsigned short hue, bool shaded, bool border, enTextAlign align) {
	// Insert Newslines if the word would exceed the maxWidth boundary
	unsigned int lineLength = 0;
	QString wrapped;
	unsigned int spaceWidth = 7;

	// Split into words using the spaces
	QStringList parts = QStringList::split(" ", text);
	QStringList::const_iterator it;
	for (it = parts.begin(); it != parts.end(); ++it) {
		QString word = *it;
		bool first = (it == parts.begin());
		unsigned int wordWidth = 0;
		unsigned int i;
		bool firstInLine = lineLength == 0;
		for (i = 0; i < word.length(); ++i) {
			wordWidth += getCharacterWidth(font, word.at(i), firstInLine);
		}

		if (!first) {
			wordWidth += spaceWidth;
		}

		// Check if the word still fits
		if (lineLength > 0 && lineLength + wordWidth > maxWidth) {
			wrapped += "\n";
			if (!first) {
				wordWidth -= spaceWidth; // We're not prepending the space anyway
			}
			lineLength = wordWidth;
		} else {
			if (!first) {
				wrapped += " ";				
			}
			lineLength += wordWidth;
		}

		wrapped += word;
	}

	return buildText(font, wrapped, hue, shaded, border, align);
}

/*
	This will be a very very long function but for speeds sake i can't split it up.

*/
cTexture *cUnicodeFonts::buildText(unsigned char font, const QString &text, unsigned short hueid, bool shaded, bool border, enTextAlign align) {
	font %= 3; // Wrap the font
	stHue *hue = Hues->get(hueid); // Cache the hue

	unsigned int width = 0; // Total width of the text
	unsigned int height = 0; // Total height of the text
	unsigned int lineWidth = 0; // Length of the current line
	unsigned int lineHeight = 0; // Height of the current line
	unsigned int lines = 1; // Number of lines
	unsigned int i;
	Q3ValueList<unsigned int> lineWidths; // Vector with the lengths of lines
	Q3ValueList<unsigned int> lineHeights; // List with the heights of every line
	QDataStream &dataStream = this->dataStream[font];

	/*
		DEBUGGING: print xoffset values of first 4096 chars if they differ from 0
		for (unsigned int j = 0; j < 0x1000; ++j) {
		signed char xoffset, yoffset, width, height;
		dataStream.device()->at(seekOffsets[font][j]);
		dataStream >> xoffset >> yoffset >> width >> height;

		if (xoffset != 0 && width > 0 && height > 0) {
			Log->print(tr("Character 0x%1 has an xoffset value: %2.\n").arg(j, 0, 16).arg(xoffset));
		}
	}*/

	// Iterate over the string once to get the width of the string	
	for (i = 0; i < text.length(); ++i) {
		const QChar ch = text.at(i);
		if (ch.latin1() == '\n') {
			lines += 1;
			if (lineWidth > width) {
				width = lineWidth;
			}
			lineHeight += 2; // Increase Lineheight if we encounter a linebreak
			height += lineHeight;
			lineWidths.append(lineWidth);
			lineHeights.append(lineHeight);
			lineHeight = 0;
			lineWidth = 0;
		} else if (ch.isSpace()) {
			lineWidth += 7; // Space Width

			if (i + 1 < text.length() && (text.at(i+1).latin1() != '\n')) {
				lineWidth += 1;
			}
		} else {
			signed char xoffset, yoffset, width, height;
			dataStream.device()->at(seekOffsets[font][ch.unicode()]);
			dataStream >> xoffset >> yoffset >> width >> height;

			height += abs(yoffset);
			// Only process this if it's NOT the first character
			if (i && width > - xoffset) {
				width += xoffset; // add to the width (there are also negative xoffsets!!)
			}

			if (lineWidth == 0 && border) {
				// add another pixel if its the first character IN A LINE and a border should be drawn
				lineWidth += 1;
			}

			if (width > 0 && height > 0) {
				lineWidth += width; // Increase the width of the text
				if ((unsigned char)height > lineHeight) {
					lineHeight = (unsigned char)height;
				}
			}

			if ((i+1 < text.length() && (text.at(i+1).latin1() != '\n')) || border) {
				// add another pixel if its not the last character
				// if its the last character and we want to draw a border, add one pixel for the border
				lineWidth += 1;
			}
		}
	}

	if (lineWidth > 0) {
		lineWidths.append(lineWidth);
		if (lineWidth > width) {
			width = lineWidth;
		}
	}

	if (lineHeight > 0) {
		lineHeights.append(lineHeight);
		height += lineHeight;
	}

	// If a border is about to be drawn and there are lines, 
	// increase the height by 2
	if (border && !lineHeights.isEmpty()) {
		height += 2;
	}

	//Log->print(LOG_MESSAGE, tr("String %1 has total height of %2 pixels and total width of %3 pixels.\n").arg(text).arg(height).arg(width));

	//unsigned int baseline = lineHeights.front(); // base of current line
	//lineHeights.pop_front();
	unsigned int baseline = border ? 1 : 0;

	cSurface *surface = 0; // The resulting text line

	if (width > 0 && height > 0) {
		int xoffset; // current offset in line

		surface = new cSurface(width, height);

		unsigned int background = 0; // cache the background color
		unsigned int foreground; // cache the foreground color
		if (!hue) {
			foreground = surface->color(0xDD, 0xDD, 0xDD);
		} else {
			foreground = surface->color(hue->colors[31].r, hue->colors[31].g, hue->colors[31].b);
		}
		unsigned int bordercolor = surface->color(0, 0, 8); // cache the border color

		surface->clear(); // Make the image transparent first

		// Start copying the characters over
		switch (align) {
			case ALIGN_LEFT:
				xoffset = 0; // Start on the left border
				break;
			case ALIGN_CENTER:
				xoffset = (width - lineWidths.front()) >> 1; // Take whats left of the total width and divide it into two
				break;
			case ALIGN_RIGHT:
				xoffset = (width - lineWidths.front()); // Take the right part and use it as an offset
				break;
		}
		lineWidths.pop_front();

		if (border) {
			++xoffset; // Increase the xoffset if there is a border to make place for it
		}

		for (unsigned int i = 0; i < text.length(); ++i) {
			QChar ch = text.at(i);			
			if (ch.latin1() == '\n') {
				if (!lineWidths.isEmpty() && !lineHeights.isEmpty()) {
					switch (align) {
						case ALIGN_LEFT:
							xoffset = 0; // Start on the left border
							break;
						case ALIGN_CENTER:
							xoffset = (width - lineWidths.front()) >> 1; // Take whats left of the total width and divide it into two
							break;
						case ALIGN_RIGHT:
							xoffset = (width - lineWidths.front()); // Take the right part and use it as an offset
							break;
					}
					lineWidths.pop_front();
					baseline += lineHeights.front();
					lineHeights.pop_front();

					if (border) {
						++xoffset; // Increase the xoffset if there is a border to make place for it
					}
				}
			} else if (ch.isSpace()) {
				xoffset += 7; // Just insert some xoffset

				if (i+1 < text.length()) {
					xoffset += 1; // There is one additional pixel between characters
				}
			} else {
				// Seek to the character in question, decode it and blit it
				dataStream.device()->at(seekOffsets[font][ch.unicode()]);
				signed char pxoffset, pyoffset, pwidth, pheight;
				dataStream >> pxoffset >> pyoffset >> pwidth >> pheight;

				//Log->print(LOG_MESSAGE, tr("Character '%1' has xoffset %2 and yoffset %3.\n").arg(ch.latin1()).arg(pxoffset).arg(pyoffset));

				unsigned char scanline[32]; // Maximum length of a line is 32 byte (8 bit, 256 pixels) so we just use a static array
				unsigned char pslwidth = (pwidth + 7) / 8;
                unsigned char psloffset = 0; // Offset within the scanline array
				unsigned char pslpadding = 0; // Offset within a single byte
				bool drawpixel = false; // The pixel value

				// If it's the first character, ignore the characters xoffset (Doesnt make much sense)
				if (i == 0) {
					pxoffset = 0;
				}

				// Start reading scalines
				for (signed char y = 0; y < pheight; ++y) {
					dataStream.readRawBytes((char*)scanline, pslwidth); // Read in the entire scanline at once
					unsigned char *ptr = surface->scanline(baseline + pyoffset + y) + (xoffset + pxoffset) * 4;
					unsigned char *endptr = ptr + pwidth * 4; // Calculate the end of the row so an easy pointer comparison suffices
					pslpadding = 0;
					psloffset = 0;

					while (ptr != endptr) { // Process every pixel of the scanline
						drawpixel = (scanline[psloffset] & (0x80 >> pslpadding++)) != 0;
						pslpadding %= 8;
						if (!pslpadding) {
							++psloffset;
						}

						if (drawpixel) {
							*((unsigned int*)ptr) = foreground;
							if (border) {
								for (int bordery = -1; bordery < 2; ++bordery) {
									unsigned char *borderptr = (ptr + bordery * (surface->width() * 4) - 4);
									unsigned char *borderendptr = borderptr + 12;
									while (borderptr != borderendptr) {
										if (*((unsigned int*)borderptr) != foreground) {
											*((unsigned int*)borderptr) = bordercolor;
										}
										borderptr += 4;
									}
								}
							}
						}
						ptr += 4;
					}
				}

				xoffset += pwidth + pxoffset;

				if (i+1 < text.length()) {
					xoffset += 1; // There is one additional pixel between characters
				}
			}
		}
	}

	cTexture *result = new cTexture(surface);
	delete surface;	
	return result;
}
/*
SDL_Surface *cUnicodeFonts::getCharacter(unsigned char font, const QChar &ch, unsigned short hue, bool border) {
	SDL_Surface *surface = 0;

	font %= 3; // Wrap the font

	// Seek to the cached offset
	QDataStream &dataStream = this->dataStream[font];
	dataStream.device()->at(seekOffsets[font][ch.unicode()]);
    
	if (!dataStream.atEnd()) {		
		signed char xoffset, yoffset, width, height; // Character properties
        
		dataStream >> xoffset >> yoffset >> width >> height;

		if (width > 0 && height > 0) {
			surface = Engine->createSurface(width, height, true, false, false);
			SDL_PixelFormat *pf = surface->format;

			unsigned int y = 0;
			unsigned int sloffset; // Position within the scanline
			unsigned int padding; // Position within a single byte
			unsigned int slwidth = (width + 7) / 8; // Count of bytes in scanline

			// Read in entire lines at once
			unsigned char *scanline = new unsigned char[slwidth];

			if (SDL_MUSTLOCK(surface)) {
				SDL_LockSurface(surface);
			}

			while(y < height) {
				dataStream.readRawBytes((char*)scanline, slwidth);
				sloffset = 0;
				padding = 0;

				while (sloffset < slwidth) {
					for (int i = 0; i < 8; ++i) {						
						bool on = (scanline[sloffset] & (0x80 >> padding)) != 0;
						unsigned int x = (sloffset << 3) + padding;
						padding = (padding + 1) % 8; // Jump to the next bit
						if (on && x < width) {
							unsigned char *ptr = (unsigned char*)surface->pixels + (surface->pitch * y) + (x * pf->BytesPerPixel);

							switch (pf->BytesPerPixel) {
								case 4:
									*(unsigned int*)ptr = SDL_MapRGB(pf, 255, 255, 255);
									break;
								case 2:
									*(unsigned short*)ptr = SDL_MapRGB(pf, 255, 255, 255);
									break;
								default:
									throw Exception(tr("Invalid bytes per pixel value: %1").arg(pf->BytesPerPixel));
							}
						}
					}

					sloffset++; // Next byte in scanline
				}
				y++;
			}

			if (SDL_MUSTLOCK(surface)) {
				SDL_UnlockSurface(surface);
			}

			delete [] scanline;
		}
	}

	return surface;	
}*/

stUnicodeCharInfo cUnicodeFonts::getCharacterInfo(uchar font, ushort ch) {
	font %= 3;

	QDataStream &dataStream = this->dataStream[font];

	stUnicodeCharInfo result;
	dataStream.device()->at(seekOffsets[font][ch]);
	dataStream >> result.xoffset >> result.yoffset >> result.width >> result.height;
	return result;
}

int cUnicodeFonts::getCharacterWidth(uchar font, const QString &text, uint i, QChar ch) {
	font %= 3;

	QDataStream &dataStream = this->dataStream[font];
	int result = 0;

	if (ch.latin1() == '\n') {
		return 0;
	} else if (ch.isSpace()) {
		result += 7; // Space Width

		if (i + 1 < text.length() && (text.at(i+1).latin1() != '\n')) {
			result += 1;
		}
	} else {
		signed char xoffset, yoffset, width, height;
		dataStream.device()->at(seekOffsets[font][ch.unicode()]);
		dataStream >> xoffset >> yoffset >> width >> height;

		// Only process this if it's NOT the first character
		if (i && width > - xoffset) {
			width += xoffset; // add to the width (there are also negative xoffsets!!)
		}

		if (i == 0 || text.at(i-1).latin1() == '\n') {
			// add another pixel if its the first character IN A LINE and a border should be drawn
			width += 1;
		}

		width += 1;
		result = width;
	}

	return result;
}

int cUnicodeFonts::getCharacterWidth(uchar font, QChar ch, bool first) {
	font %= 3;

	QDataStream &dataStream = this->dataStream[font];
	int result = 0;

	if (ch.latin1() == '\n') {
		return 0;
	} else if (ch.isSpace()) {
		result += 8; // Space Width (Including 1 xoffset)
	} else {
		signed char xoffset, yoffset, width, height;
		dataStream.device()->at(seekOffsets[font][ch.unicode()]);
		dataStream >> xoffset >> yoffset >> width >> height;

		// Only process this if it's NOT the first character
		if (!first && width > - xoffset) {
			width += xoffset; // add to the width (there are also negative xoffsets!!)
		}

		if (first) {
			// add another pixel if its the first character IN A LINE and a border should be drawn
			width += 1;
		}

		width += 1;
		result = width;
	}

	return result;
}

cUnicodeFonts *UnicodeFonts = 0; // Global cUnicodeFonts instance
