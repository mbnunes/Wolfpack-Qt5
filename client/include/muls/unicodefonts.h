
#if !defined(__UNICODEFONTS_H__)
#define __UNICODEFONTS_H__

#include <qfile.h>
#include <qdatastream.h>
#include <q3cstring.h>

#include "enums.h"
#include "utilities.h"

class cUnicodeFonts {
private:
	// Since the unicode fonts are too big, we'll only cache the indexers into the file
	// Since we have 0x10000 characters and every indexer is 4 byte, we have about
	// 256 kb of index data per file. I think that is acceptable.
	QFile::Offset seekOffsets[3][0x10000];
	QFile data[3];
	QDataStream dataStream[3];
public:
	cUnicodeFonts();
	~cUnicodeFonts();

	void load();
	void unload();
	void reload();

	// Build a text string
	cTexture *buildText(unsigned char font, const QString &text, unsigned short hue = 0, bool shaded = false, bool border = false, enTextAlign align = ALIGN_LEFT);
	//SDL_Surface *getCharacter(unsigned char font, const QChar &ch, unsigned short hue = 0, bool border = false);
	//SDL_Surface *buildText(unsigned char font, const QCString &text, unsigned short hue = 0, bool shaded = false, enTextAlign align = ALIGN_LEFT);
	//SDL_Surface *buildTextWrapped(unsigned char font, const QCString &text, unsigned short maxWidth, unsigned short hue = 0, bool shaded = false, enTextAlign align = ALIGN_LEFT);
};

extern cUnicodeFonts *UnicodeFonts;

#endif
