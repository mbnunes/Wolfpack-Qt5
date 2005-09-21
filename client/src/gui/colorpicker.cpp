
#include "gui/colorpicker.h"
#include "muls/hues.h"
#include "surface.h"
#include "utilities.h"
#include "mainwindow.h"

#include <QCursor>

cColorPicker::cColorPicker() {
	startHue_ = 0;
	endHue_ = 0;
	columns_ = 10;
	rows_ = 10;
	dirty = true;
	texture = 0;
}

cColorPicker::~cColorPicker() {
	if (texture) {
		texture->decref();
	}
}

void cColorPicker::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (oldwidth != width_ || oldheight != height_) {
		dirty = true;
	}
	cControl::onChangeBounds(oldx, oldy, oldwidth, oldheight);
}

void cColorPicker::update() {
	if (texture) {
		texture->decref();
		texture = 0;
	}

	cSurface *surface = new cSurface(width_, height_, true);
	surface->clear();

	// We keep a 1 pixel border for the selection around the real color picker
	// So our real calculation-wise height and width is width_ - 2 and height_ - 2.
	
	uint widthPerColor = (width_ - 2) / columns_;
	uint heightPerColor = (height_ - 2) / rows_;

	ushort hueid = startHue_; // Initialize first hue and increment on draw
	for (uint y = 0; y < rows_; ++y) {
		for (uint x = 0; x < columns_ && hueid <= endHue_; ++x) {
			// Calc where we start drawing
			int startx = 1 + x * widthPerColor;
			int starty = 1 + y * heightPerColor;

			stHue *hue = Hues->get(hueid);
			// Index: 26 of hue
			uint color = cSurface::color(hue->colors[26].r, hue->colors[26].g, hue->colors[26].b);
			
			for (uint drawx = startx; drawx < startx + widthPerColor; ++drawx) {
				for (uint drawy = starty; drawy < starty + heightPerColor; ++drawy) {
					surface->setPixel(drawx, drawy, color);
				}
			}
			hueid++;
		}
	}

	texture = new cTexture(surface, false);
	delete surface;
}

void cColorPicker::draw(int xoffset, int yoffset) {
	if (dirty) {
		update();
	}

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}

	// Get the coordinates relative to us and see if we hover a color
	QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
	pos = mapFromGlobal(pos);
	
	uint widthPerColor = (width_ - 2) / columns_;
	uint heightPerColor = (height_ - 2) / rows_;

	uint xColorIndex = (pos.x() - 1) / widthPerColor;

	if (xColorIndex >= columns_) {
		return;
	}

	uint yColorIndex = (pos.y() - 1) / heightPerColor;

	if (yColorIndex >= rows_) {
		return;
	}

	ushort hueid = startHue_ + xColorIndex + yColorIndex * columns_;

	if (hueid <startHue_ || hueid > endHue_) {
		return;
	}

	stHue *hue = Hues->get(hueid);

	if (!hue) {
		return;
	}

	int startx = xoffset + x_ + xColorIndex * widthPerColor;
	int starty = yoffset + y_ + yColorIndex * heightPerColor;

	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glVertex2i(startx, starty);
	glVertex2i(startx + widthPerColor + 2, starty);	
	glVertex2i(startx + widthPerColor + 2, starty + heightPerColor + 2);
	glVertex2i(startx, starty + heightPerColor + 2);

	// Draw an "inner" rectangle with the original color to achieve some sort
	// of "outlining" the hovered color
	startx += 1;
	starty += 1;
	glColor4f(hue->colors[26].r / 255.0f, hue->colors[26].g / 255.0f, hue->colors[26].b / 255.0f, 255);
	glVertex2i(startx, starty);
	glVertex2i(startx + widthPerColor, starty );	
	glVertex2i(startx + widthPerColor, starty + heightPerColor);
	glVertex2i(startx, starty + heightPerColor);
	glEnd();
}

void cColorPicker::processDefinitionAttribute(QString name, QString value) {
	if (name == "starthue") {
		startHue_ = Utilities::stringToUInt(value);
	} else if (name == "endhue") {
		endHue_ = Utilities::stringToUInt(value);
	} else if (name == "columns") {
		columns_ = Utilities::stringToUInt(value);
	} else if (name == "rows") {
		rows_ = Utilities::stringToUInt(value);		
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cColorPicker::onMouseDown(QMouseEvent *e) {
	// Check which color has been selected
	// Get the coordinates relative to us and see if we hover a color
	QPoint pos = mapFromGlobal(e->pos());
	
	uint widthPerColor = (width_ - 2) / columns_;
	uint heightPerColor = (height_ - 2) / rows_;

	uint xColorIndex = (pos.x() - 1) / widthPerColor;

	if (xColorIndex >= columns_) {
		return;
	}

	uint yColorIndex = (pos.y() - 1) / heightPerColor;

	if (yColorIndex >= rows_) {
		return;
	}

	ushort hueid = startHue_ + xColorIndex + yColorIndex * columns_;

	if (hueid <startHue_ || hueid > endHue_) {
		return;
	}

	emit colorSelected(hueid);

	cControl::onMouseDown(e);
}
