
#include <qmap.h>
#include <qstringlist.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <Q3CString>

#include "exceptions.h"
#include "log.h"
#include "utilities.h"
#include "muls/asciifonts.h"
#include "muls/hues.h"

cAsciiFonts::cAsciiFonts() {
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 224; ++j) {
			characters[i][j] = 0;
		}
		height[i] = 0;
	}
}

cAsciiFonts::~cAsciiFonts() {
}

void cAsciiFonts::load() {
	// Set filenames
	QFile data(Utilities::getUoFilename("fonts.mul"));

	// Open files
	if (!data.open(QIODevice::ReadWrite)) {
		throw Exception(tr("Unable to open ascii font data at %1.").arg(data.name()));
	}

	// Set the devices for the streams
	QDataStream dataStream(&data);
	dataStream.setByteOrder(QDataStream::LittleEndian);

	// Load the font data
	unsigned char header;
	unsigned short pixel;
	unsigned char height = 0; // Height of the entire font (max. height of all chars)
	for (unsigned char f = 0; f < 10; ++f) {
		dataStream >> header;
		this->height[f] = 0;

		for (int c = 0; c < 224; ++c) {
			// Read in 224 characters
			signed char width, height, unknown;
			dataStream >> width >> height >> unknown;
			
			if (width > 0 && height > 0) {
				if (height > this->height[f]) {
					this->height[f] = height;
				}

				// Create a non texture surface for caching the character pixel data
				cSurface *surface = new cSurface(width, height, false);
				surface->clear();
			
				for (int y = 0; y < height; ++y) {
					for (int x = 0; x < width; ++x) {
						dataStream >> pixel;
						if (pixel != 0) {
							surface->setPixel(x, y, surface->color(pixel));
						}
					}
				}

				characters[f][c] = surface;
			}
		}
		Log->print(LOG_MESSAGE, tr("Loaded ascii font %1 (Height: %2).\n").arg(f).arg(this->height[f]));
	}

	data.close();
}

void cAsciiFonts::unload() {
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 224; ++j) {
			delete characters[i][j];
			characters[i][j] = 0;
		}
	}
}

void cAsciiFonts::reload() {
	unload();
	load();
}

cTexture *cAsciiFonts::buildTextWrapped(unsigned char font, const Q3CString &text, unsigned short maxWidth, unsigned short hue, bool shaded, enTextAlign align) {
	// Insert Newslines if the word would exceed the maxWidth boundary
	unsigned int lineLength = 0;
	Q3CString wrapped;
	unsigned int spaceWidth = 0;

	cSurface *sf = getCharacter(font, ' ');
	if (sf) {
		spaceWidth = sf->width();
	}

	// Split into words using the spaces
	QStringList parts = QStringList::split(" ", text.data());
	QStringList::const_iterator it;
	for (it = parts.begin(); it != parts.end(); ++it) {
		QString word = *it;
		bool first = (it == parts.begin());
		unsigned int wordWidth = 0;
		unsigned int i;
		for (i = 0; i < word.length(); ++i) {
			cSurface *sf = getCharacter(font, word.at(i).latin1());
			if (sf) {
				wordWidth += sf->width();
			}
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

		wrapped += word.latin1();
	}

	return buildText(font, wrapped, hue, shaded, align);
}

cTexture *cAsciiFonts::buildText(unsigned char font, const Q3CString &text, unsigned short hueid, bool shaded, enTextAlign align, bool hueAll) {
	if (font > 9) {
		font = 3; // Default back to font 3 if the font is invalid
	}

	unsigned int width = 0; // Total width of the text	
	unsigned int height = this->height[font]; // Total height of the text
	unsigned int lineWidth = 0; // Length of the current line
	unsigned int lines = 1; // Number of lines
	Q3ValueList<unsigned int> lineWidths; // Vector with the lengths of lines
	
	// Iterate over the string once to get the width of the string
	Q3CString::ConstIterator it;
	for (it = text.begin(); it != text.end(); ++it) {
		if (*it == '\n') {
			lines += 1;
			if (lineWidth > width) {
				width = lineWidth;
			}
			lineWidths.append(lineWidth);
			lineWidth = 0;
		} else {
			cSurface *ch = getCharacter(font, *it);
	
			if (ch) {
				lineWidth += ch->width(); // Increase the width of the text
			}
		}
	}

	if (lineWidth > 0) {
		lineWidths.append(lineWidth);

		if (lineWidth > width) {
			width = lineWidth;
		}
	}

	unsigned int baseline = height; // Store the baseline
	height = lines * height; // Increase the height of the text	

	cSurface *surface = 0; // The resulting text line

	if (width > 0) {
		surface = new cSurface(width, height);
		surface->clear(); // Clear the background of the surface

		// Start copying the characters over
		int destx = 0;
		int desty = 0;
		switch (align) {
			case ALIGN_LEFT:
				destx = 0; // Start on the left border
				break;
			case ALIGN_CENTER:
				destx = (width - lineWidths.front()) >> 1; // Take whats left of the total width and divide it into two
				break;
			case ALIGN_RIGHT:
				destx = (width - lineWidths.front()); // Take the right part and use it as an offset
				break;
		}
		lineWidths.pop_front();

		for (it = text.begin(); it != text.end(); ++it) {
			if (*it == '\n') {
				if (!lineWidths.isEmpty()) {
					switch (align) {
						case ALIGN_LEFT:
							destx = 0; // Start on the left border
							break;
						case ALIGN_CENTER:
							destx = (width - lineWidths.front()) >> 1; // Take whats left of the total width and divide it into two
							break;
						case ALIGN_RIGHT:
							destx = (width - lineWidths.front()); // Take the right part and use it as an offset
							break;
					}
					lineWidths.pop_front();
				}
				baseline += this->height[font];
			} else {
				cSurface *ch = getCharacter(font, *it);
				if (ch) {
					desty = baseline - ch->height();
					// MemCpy Line by Line
					for (int yl = 0; yl < ch->height(); ++yl) {
						unsigned char *src = ch->scanline(yl);
						unsigned char *dest = surface->scanline(yl + desty) + destx * 4;
						memcpy(dest, src, ch->width() * 4);
					}
					destx += ch->width(); // Increase for the next draw
				}
			}
		}
	}

	stHue *hue = Hues->get(hueid); // Cache the hue

	// Hueing the character is a per-pixel operation on the entire surface
	if (surface && hue) {
		// Iterate over the surface
		unsigned char r, g, b;
		unsigned int pixel;
		stColor color;
	
		unsigned int swidth = surface->realWidth();
		unsigned int sheight = surface->realHeight();

		for (int x = 0; x < surface->realWidth(); ++x) {
			for (int y = 0; y < surface->realHeight(); ++y) {
				pixel = surface->getPixel(x, y);
				if (pixel != 0) {
					r = surface->red(pixel);
					b = surface->blue(pixel);
					g = surface->green(pixel);
					if (hueAll || (r == g && g == b)) {
						color = hue->colors[r >> 3];
						surface->setPixel(x, y, surface->color(color.r, color.b, color.g));
					}
				}
			}
		}
	}

	cTexture *result = new cTexture(surface);
	delete surface;

	return result;
}

cAsciiFonts *AsciiFonts = 0; // Global cAsciiFonts instance
