
#include "config.h"
#include "gui/cursor.h"
#include "muls/art.h"
#include "mainwindow.h"
#include <QCursor>
#include <qgl.h>

cCursor *Cursor = 0;

cCursor::cCursor() {
	cursor = CURSOR_NORMAL;
	warmode = false;

	memset(cursors, 0, sizeof(cursors));
	memset(cursorsWar, 0, sizeof(cursorsWar));
}

void cCursor::loadCursor(enCursorType cursor, bool warmode) {
	// These two values could be made configurable. On the other hand it
	// is trivial to patch the art ids.
	const unsigned short normalCursorOffset = 0x206a;
	const unsigned short warmodeCursorOffset = 0x2053;

	// Load the cursor surface, then process it.
	unsigned short tileid = (warmode ? warmodeCursorOffset : normalCursorOffset) + cursor;

	// Save the cursor
	if (warmode) {
		Art->readCursor(&cursorsWar[cursor], tileid, 0, false);
	} else {
		Art->readCursor(&cursors[cursor], tileid, 0, false);
	}
}

void cCursor::load() {
	for (int i = 0; i < CURSOR_COUNT; ++i) {
		loadCursor((enCursorType)i, false);
		loadCursor((enCursorType)i, true);
	}
}

void cCursor::unload() {
	for (int i = 0; i < CURSOR_COUNT; ++i) {
		if (cursors[i].surface) {
			cursors[i].surface->decref();			
			cursors[i].surface = 0;
		}
		if (cursorsWar[i].surface) {
			cursorsWar[i].surface->decref();
			cursorsWar[i].surface = 0;
		}
	}
}

void cCursor::reload() {
	unload();
	load();
}

void cCursor::draw() {
	stCursor &cursor = warmode ? cursorsWar[this->cursor] : cursors[this->cursor];
	
	// Fall back to normal cursor
	if (!cursor.surface) {
		return;
	}

	// Get the mouse position
	int x, y;
	QPoint pos = QCursor::pos();
	pos = GLWidget->mapFromGlobal(pos);
	x = pos.x();
	y = pos.y();

	// Modify based on x/y offset
	x -= cursor.xoffset;
	y -= cursor.yoffset;

	cursor.surface->bind();
	glLoadIdentity();

	glBegin(GL_QUADS);

	int right = x + cursor.surface->realWidth() - 2;
	int bottom = y + cursor.surface->realHeight() - 2;
	
	// We need to skip the first/last texel row/col
	float texLeft = 1.0 / cursor.surface->width(); 
	float texTop = 1.0 / cursor.surface->height();
	float texRight = (cursor.surface->realWidth() - 1) / (float)cursor.surface->width();
	float texBottom = (cursor.surface->realHeight() - 1) / (float)cursor.surface->height();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White. No Alpha.
	glTexCoord2f(texLeft, texTop); glVertex2f(x, y); // Upper left corner
	glTexCoord2f(texRight, texTop); glVertex2f(right, y); // Upper Right Corner
	glTexCoord2f(texRight, texBottom); glVertex2f(right, bottom); // Lower Right Corner
	glTexCoord2f(texLeft, texBottom); glVertex2f(x, bottom); // Lower Right Corner

	glEnd();
}
