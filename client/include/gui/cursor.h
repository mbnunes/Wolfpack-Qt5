
#if !defined(__CURSOR_H__)
#define __CURSOR_H__

#include "utilities.h"
#include "texture.h"
#include <QObject>

// This enumeration is only used here. Hence it's not exported to
// enums.h
enum enCursorType {
	CURSOR_DIR0 = 0,
	CURSOR_DIR1,
	CURSOR_DIR2,
	CURSOR_DIR3,
	CURSOR_DIR4,
	CURSOR_DIR5,
	CURSOR_DIR6,
	CURSOR_DIR7,
	CURSOR_GRAB,
	CURSOR_NORMAL,
	CURSOR_PICK,
	CURSOR_UNKNOWN, // What is this for?
	CURSOR_TARGET,
	CURSOR_HOURGLASS,
	CURSOR_WRITE,
	CURSOR_PIN,
	CURSOR_COUNT
};

// Since the art class is using this we cant make it private
struct stCursor {
	cTexture *surface;
	int xoffset;
	int yoffset;
};

/*
	This class manages the software cursor.
*/
class cCursor : public QObject {
Q_OBJECT

protected:
	stCursor cursors[CURSOR_COUNT];
	stCursor cursorsWar[CURSOR_COUNT];

	enCursorType cursor;
	bool warmode; // Warmode setting
	
	// Internal funtion for loading the given cursor
	void loadCursor(enCursorType cursor, bool warmode);
public:
	cCursor();

	void load();
	void unload();
	void reload();

	void setCursor(enCursorType cursor);
	int currentWidth() const;
	int currentHeight() const;
	int currentXOffset() const;
	int currentYOffset() const;

	// Changes the warmode setting for the cursor
	inline void setWarmode(bool warmode) {
		this->warmode = warmode;
	}

	// Draws the cursor on the fresh engine.
	void draw();
};

inline void cCursor::setCursor(enCursorType cursor) {
	this->cursor = cursor;
}

inline int cCursor::currentXOffset() const {
	return cursors[cursor].xoffset;
}

inline int cCursor::currentYOffset() const {
	return cursors[cursor].yoffset;
}

inline int cCursor::currentWidth() const {
	return cursors[cursor].surface->realWidth();
}

inline int cCursor::currentHeight() const {
	return cursors[cursor].surface->realHeight();
}

extern cCursor *Cursor;

#endif
