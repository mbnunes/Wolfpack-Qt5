
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "exceptions.h"
#include "engine.h"
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
	if (!data.open(IO_ReadWrite)) {
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

				SDL_Surface *surface = Engine->createSurface(width, height, true, false);
				SDL_PixelFormat *pf = surface->format;

				if (SDL_MUSTLOCK(surface)) {
					SDL_LockSurface(surface);
				}

				for (int y = 0; y < height; ++y) {
					for (int x = 0; x < width; ++x) {
						dataStream >> pixel;

						unsigned char *ptr = (unsigned char*)surface->pixels + (surface->pitch * y) + (x * pf->BytesPerPixel);
						switch (pf->BytesPerPixel) {
							case 4:
								*(unsigned int*)ptr = SDL_MapRGB(pf, (pixel >> 7) & 0xF8, (pixel >> 2) & 0xF8, (pixel << 3) & 0xF8);
								break;
							case 2:
								*(unsigned short*)ptr = SDL_MapRGB(pf, (pixel >> 7) & 0xF8, (pixel >> 2) & 0xF8, (pixel << 3) & 0xF8);
								break;
							default:
								throw Exception(tr("Invalid bytes per pixel value: %1").arg(pf->BytesPerPixel));
						}
					}
				}

				if (SDL_MUSTLOCK(surface)) {
					SDL_UnlockSurface(surface);
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
			if (characters[i][j]) {
				SDL_FreeSurface(characters[i][j]);
			}
			characters[i][j] = 0;
		}
	}
}

void cAsciiFonts::reload() {
	unload();
	load();
}

cTexture *cAsciiFonts::buildTextWrapped(unsigned char font, const QCString &text, unsigned short maxWidth, unsigned short hue, bool shaded, enTextAlign align) {
	// Insert Newslines if the word would exceed the maxWidth boundary
	unsigned int lineLength = 0;
	QCString wrapped;
	unsigned int spaceWidth = 0;

	SDL_Surface *sf = getCharacter(font, ' ');
	if (sf) {
		spaceWidth = sf->w;
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
			SDL_Surface *sf = getCharacter(font, word.at(i).latin1());
			if (sf) {
				wordWidth += sf->w;
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

cTexture *cAsciiFonts::buildText(unsigned char font, const QCString &text, unsigned short hueid, bool shaded, enTextAlign align, bool hueAll) {
	if (font > 9) {
		font = 3; // Default back to font 3 if the font is invalid
	}

	unsigned int width = 0; // Total width of the text	
	unsigned int height = this->height[font]; // Total height of the text
	unsigned int lineWidth = 0; // Length of the current line
	unsigned int lines = 1; // Number of lines
	QValueList<unsigned int> lineWidths; // Vector with the lengths of lines
	
	// Iterate over the string once to get the width of the string
	QCString::ConstIterator it;
	for (it = text.begin(); it != text.end(); ++it) {
		if (*it == '\n') {
			lines += 1;
			if (lineWidth > width) {
				width = lineWidth;
			}
			lineWidths.append(lineWidth);
			lineWidth = 0;
		} else {
			SDL_Surface *ch = getCharacter(font, *it);
	
			if (ch) {
				lineWidth += ch->w; // Increase the width of the text
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

	SDL_Surface *surface = 0; // The resulting text line
	unsigned int black;

	if (width > 0) {
		surface = Engine->createSurface(width, height, false, true, true);
		SDL_PixelFormat *pf = surface->format; // Cache the pixel format
		black = SDL_MapRGBA(pf, 0, 0, 0, 255);
		SDL_FillRect(surface, 0, black);

		// Start copying the characters over
		SDL_Rect dest;
		switch (align) {
			case ALIGN_LEFT:
				dest.x = 0; // Start on the left border
				break;
			case ALIGN_CENTER:
				dest.x = (width - lineWidths.front()) >> 1; // Take whats left of the total width and divide it into two
				break;
			case ALIGN_RIGHT:
				dest.x = (width - lineWidths.front()); // Take the right part and use it as an offset
				break;
		}
		lineWidths.pop_front();

		for (it = text.begin(); it != text.end(); ++it) {
			if (*it == '\n') {
				if (!lineWidths.isEmpty()) {
					switch (align) {
						case ALIGN_LEFT:
							dest.x = 0; // Start on the left border
							break;
						case ALIGN_CENTER:
							dest.x = (width - lineWidths.front()) >> 1; // Take whats left of the total width and divide it into two
							break;
						case ALIGN_RIGHT:
							dest.x = (width - lineWidths.front()); // Take the right part and use it as an offset
							break;
					}
					lineWidths.pop_front();
				}
				baseline += this->height[font];
			} else {
				SDL_Surface *ch = getCharacter(font, *it);
				if (ch) {
					dest.y = baseline - ch->h;
					SDL_BlitSurface(ch, 0, surface, &dest);
					dest.x += ch->w; // Increase for the next draw
				}
			}
		}
	}

	stHue *hue = Hues->get(hueid); // Cache the hue

	// Hueing the character is a per-pixel operation on the entire surface
	if (surface) {
		// Iterate over the surface
		unsigned int x = 0;
		unsigned int y = 0;
		unsigned int bpp = surface->format->BytesPerPixel;
		unsigned char r, g, b;
		unsigned int pixel;
		stColor color;
		SDL_PixelFormat *pf = surface->format;

		if (SDL_MUSTLOCK(surface)) {
			SDL_LockSurface(surface);
		}
		
		unsigned char *rowptr = (unsigned char*)surface->pixels; // Pointer used to iterate over rows
		unsigned char *ptr; // Pointer used within the row to iterate

		unsigned int swidth = surface->w;
		unsigned int sheight = surface->h;

		while (y++ < sheight) {
			ptr = rowptr;
			x = 0;
			while (x++ < swidth) {
				// Process Hue
				pixel = *((unsigned int*)ptr);
				if (pixel == black) {
					*((unsigned int*)ptr) = 0;
				} else if (hue && pixel) {
					SDL_GetRGB(pixel, pf, &r, &g, &b); // Transform RGB values

					// This is some strange special handling. The border is unhued.							
					if (hueAll || (r == g && g == b)) {
						color = hue->colors[r >> 3];
						*((unsigned int*)ptr) = SDL_MapRGB(pf, color.r, color.g, color.b);
					}
				}
				ptr += 4; // Jump to the next pixel in this row
			}
			rowptr += surface->pitch; // Jump to the next row
		}

		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}

	cTexture *result = new cTexture(surface);
	if (surface) {
		result->setRealWidth(width);
		result->setRealHeight(height);
		SDL_FreeSurface(surface);
	}

	return result;
}

cAsciiFonts *AsciiFonts = 0; // Global cAsciiFonts instance
