
#if !defined(__UTILITIES_H__)
#define __UTILITIES_H__

#include <time.h>
#include <QString>

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

	void launchBrowser(const QString &url);

	uint getTicks();


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

		return dir % 8;
	}

	QString parseArguments(QString localized, QStringList arguments);

	/*
		Calculate the distance between two 2d uo coordinates
	*/
	inline unsigned int distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
		unsigned int uiResult = ~0U; // Initialize with *infinite*
		
		int xdiff = x1 - x2;
		int ydiff = y1 - y2;
		float rfResult = sqrt((float)(xdiff * xdiff + ydiff * ydiff));
		uiResult = (unsigned int)rfResult; // truncate
		
		return uiResult;
	}

	/*
		Calculate the distance between two 2d uo coordinates the SIMPLE way
	*/
	inline unsigned int simpleDistance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
		unsigned int uiResult = ~0U; // Initialize with *infinite*
		
		int xdiff = qAbs<int>(x1 - x2);
		int ydiff = qAbs<int>(y1 - y2);
		
		return qMax<int>(xdiff, ydiff);
	}

	QString localLanguage();

	QString dumpData(const QByteArray &data);

	inline bool isMobileSerial(uint serial) {
		return serial > 0 && serial < 0x40000000;
	}

	inline bool isItemSerial(uint serial) {
		return serial > 0x40000000 && serial < 0x80000000;
	}

	inline bool isValidSerial(uint serial) {
		return isItemSerial(serial) || isMobileSerial(serial);
	}

	inline bool isInvalidSerial(uint serial) {
		return !isItemSerial(serial) && !isMobileSerial(serial);
	}

	// Translate an equipment item id into a valid body id for a mount
	ushort getMountBody(ushort id);

	inline void calcXYFromDirection(unsigned char direction, int &xdiff, int &ydiff) {
		direction &= 0xf;
		switch (direction) {
			// Up
			case 0:
				xdiff = -1;
				ydiff = -1;
				break;
			// Up-Right
			case 1:
				xdiff = 0;
				ydiff = -1;
				break;
			// Right
			case 2:
				xdiff = 1;
				ydiff = -1;
				break;
			// Down-Right
			case 3:
				xdiff = 1;
				ydiff = 0;
				break;
			// Down
			case 4:
				xdiff = 1;
				ydiff = 1;
				break;
			// Down-Left
			case 5:
				xdiff = 0;
				ydiff = 1;
				break;
			// Left
			case 6:
				xdiff = -1;
				ydiff = 1;
				break;
			// Up-Left
			case 7:
				xdiff = -1;
				ydiff = 0;
				break;
			// Default
			default:
				return; // Do nothing
		};
	}

	inline uint stringToUInt(QString value) {
		bool hex = value.startsWith("0x", Qt::CaseInsensitive);
		if (hex) {
			return value.right(value.length() - 2).toUInt(0, 16);
		} else {
			return value.toUInt(0, 10);
		}
	}

	inline int stringToInt(QString value) {
		bool hex = value.startsWith("0x", Qt::CaseInsensitive);
		if (hex) {
			return value.right(value.length() - 2).toInt(0, 16);
		} else {
			return value.toInt(0, 10);
		}
	}

	inline bool stringToBool(QString value) {
		if (value == "0" || value.isEmpty() || value == "false") {
			return false;
		} else {
			return true;
		}                
	}
};

// Define the Random object
extern MTRand *Random;

#endif
