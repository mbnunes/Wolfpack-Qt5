
#if !defined(__UNICODEFONTS_H__)
#define __UNICODEFONTS_H__

#include <QFile>
#include <QDataStream>
#include <QString>

#include "enums.h"
#include "utilities.h"

struct stUnicodeCharInfo {
	uchar width;
	uchar height;
	signed char xoffset;
	signed char yoffset;
};

class cUnicodeFonts {
private:
	// Since the unicode fonts are too big, we'll only cache the indexers into the file
	// Since we have 0x10000 characters and every indexer is 4 uchar, we have about
	// 256 kb of index data per file. I think that is acceptable.
	qint32 seekOffsets[3][0x10000];
	QFile data[3];
	QDataStream dataStream[3];

	int getCharacterWidth(uchar font, QChar ch, bool first);
public:
	cUnicodeFonts();
	~cUnicodeFonts();

	void load();
	void unload();
	void reload();

	// Build a text string
	bool isCharacterSupported(uchar font, ushort ch) const;
	cTexture *buildTextWrapped(unsigned char font, const QString &text, unsigned short maxWidth, unsigned short hue = 0, bool shaded = false, bool border = false, enTextAlign align = ALIGN_LEFT, bool processHtml = false);
	cTexture *buildText(unsigned char font, QString	text, unsigned short hue = 0, bool shaded = false, bool border = false, enTextAlign align = ALIGN_LEFT, bool processHtml = false);
    stUnicodeCharInfo getCharacterInfo(uchar font, ushort ch);
	int getCharacterWidth(uchar font, const QString &text, uint pos, QChar ch);
};

inline bool cUnicodeFonts::isCharacterSupported(uchar font, ushort ch) const {
	font %= 3;

	if (seekOffsets[font][ch] != ~0) {
		return true;
	} else {
		return false;
	}
}

extern cUnicodeFonts *UnicodeFonts;

#endif
