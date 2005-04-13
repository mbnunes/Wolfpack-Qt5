
#if !defined(__UTILITIES_H__)
#define __UTILITIES_H__

#include <time.h>
#include <qstring.h>
#include <qintcache.h>

#include "exceptions.h"
#include "mersennetwister.h"
#include "texture.h"

// Define tr
inline QString tr(const QString &a) {
	return a;
}

// Useful mathematical definitions
const float PI = 3.14159265358979323846f;
const float PIHALF = PI / 2.0f;
const float PIFOURTH = PI / 4.0f;

// Utilities
namespace Utilities {
	void messageBox(QString message, QString caption = "Error", bool error = false);

	QString getUoFilename(const QString &filename);

	void launchBrowser(const QCString &url);

	inline unsigned int getTicks() {
		return (unsigned int)clock();
	}

	/*
		Return the direction from point a to point b.
	*/
	inline unsigned char direction(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
		unsigned int dir;
		short xdif, ydif;

		xdif = x2 - x1;
		ydif = y2 - y1;

		if ( ( xdif == 0 ) && ( ydif < 0 ) )
			dir = 0;
		else if ( ( xdif > 0 ) && ( ydif < 0 ) )
			dir = 1;
		else if ( ( xdif > 0 ) && ( ydif == 0 ) )
			dir = 2;
		else if ( ( xdif > 0 ) && ( ydif > 0 ) )
			dir = 3;
		else if ( ( xdif == 0 ) && ( ydif > 0 ) )
			dir = 4;
		else if ( ( xdif < 0 ) && ( ydif > 0 ) )
			dir = 5;
		else if ( ( xdif < 0 ) && ( ydif == 0 ) )
			dir = 6;
		else if ( ( xdif < 0 ) && ( ydif < 0 ) )
			dir = 7;
		else
			dir = 8;

		return (dir + 1) % 8;
	}

	/*
		Calculate the distance between two 2d uo coordinates
	*/
	inline unsigned int distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
		unsigned int uiResult = ~0U; // Initialize with *infinite*
		
		int xdiff = x1 - x2;
		int ydiff = y1 - y2;
		float rfResult = sqrt((float)(xdiff * xdiff + ydiff * ydiff));
		uiResult = (unsigned int)floor(rfResult); // truncate
		
		return uiResult;
	}
};

// Please note that this cache does steal reference counts
class TextureCache : public QIntCache< cTexture > {
public:
	TextureCache(int a, int b) : QIntCache< cTexture > (a, b) {
	}

protected:
	void deleteItem(Item d) {
		((cTexture*)d)->decref();
	}
};

// Define the Random object
extern MTRand *Random;

#endif
