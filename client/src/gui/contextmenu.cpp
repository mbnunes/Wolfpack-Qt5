
#include "gui/contextmenu.h"
#include "gui/gui.h"
#include "engine.h"

// Our custom label
class cContextMenuEntry : public cLabel {
protected:
	int id_;

public:
	cContextMenuEntry(const QString &text, unsigned short hue, int id = -1) : cLabel(text, 1, hue, false) {
		id_ = id;
	}

	void onMouseUp(int x, int y, unsigned char button, bool pressed) {
		if (Gui->getControl(x, y) == this) {
			if (ContextMenu->callback) {
				ContextMenu->callback(ContextMenu, id_, ContextMenu->customData_);
				ContextMenu->callback = 0;
				ContextMenu->customData_ = 0;
			}
			ContextMenu->hide();
		}
	}

	int id() const {
		return id_;
	}

	void setId(int data) {
		id_ = data;
	}
};

cContextMenu::cContextMenu() {
	border = 0;
	checkerboard = 0;
	callback = 0;
	customData_ = 0;
}

cContextMenu::~cContextMenu() {
	if (checkerboard) {
		checkerboard->decref();
	}
}

void cContextMenu::addEntry(const QString &name, unsigned short hue, int id) {
	cContextMenuEntry *entry = new cContextMenuEntry(name, hue);
	entry->refreshSurface();
	entry->setId(id);
	entries.append(entry);
	addControl(entry);
}

void cContextMenu::show() {
	int x, y;
	SDL_GetMouseState(&x, &y);
	show(x, y);
}

void cContextMenu::show(int x, int y) {
	if (!border) {
		border = new cBorderGump(0xa3c);
		border->setAlign(CA_CLIENT);
		border->setMoveHandle(false);
		addControl(border, true);
	}

	if (!checkerboard) {
		SDL_Surface *surface = Engine->createSurface(64, 64, false, false, true);
		SurfacePainter32 painter(surface);
		int black = painter.color(0, 0, 0, 0);
		int white = painter.color(255, 255, 255, 255);
		for (int x = 0; x < 64; ++x) {
			for (int y = 0; y < 64; ++y) {
				if (y % 2 == 0) {
					if (x % 2 == 0) {
						painter.setPixel(x, y, white);
					} else {
						painter.setPixel(x, y, black);
					}
				} else {
					if (x % 2 == 0) {
						painter.setPixel(x, y, black);
					} else {
						painter.setPixel(x, y, white);
					}
				}				
			}
		}
		checkerboard = new cTexture(surface, false);
		SDL_FreeSurface(surface);
	}

	if (!parent_) {
		Gui->addControl(this);
	}

	int totalheight = 0;
	int maxwidth = 0;

	// Iterate over all known controls and see how high this thing is
	Iterator it;
	for (it = entries.begin(); it != entries.end(); ++it) {
		(*it)->setPosition(10, 10 + totalheight); // Reposition the entry
		totalheight += (*it)->height();
		if ((*it)->width() > maxwidth) {
			maxwidth = (*it)->width();
		}
	}
    
	// Resize the contextmenu
	setSize(20 + maxwidth, 22 + totalheight);

	setPosition(x, y);
	setVisible(true);
}

void cContextMenu::clear() {
	// Iterate over all known controls and see how high this thing is
	Iterator it;
	for (it = entries.begin(); it != entries.end(); ++it) {
		removeControl((*it));
		delete *it;
	}
	entries.clear();

	setCallback(0, 0); // Reset callback
}

void cContextMenu::hide() {
	setVisible(false);
	setCallback(0, 0);
}

unsigned char stencils[64*64] = {1, };

void cContextMenu::draw(int xoffset, int yoffset) {
	xoffset += x_;
	yoffset += y_;

	// Enable stencil buffer
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NEVER, 1, 1);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	// Simply write to the stencil buffer
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_EQUAL, 1.0f);

	float rightTexCoord = (float)width_ / 64.0f;
	float downTexCoord = (float)height_ / 64.0f;

	checkerboard->bind();
	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(xoffset, yoffset);
	glTexCoord2f(rightTexCoord, 0.0f); glVertex2i(xoffset + width_, yoffset);
	glTexCoord2f(rightTexCoord, downTexCoord); glVertex2i(xoffset + width_, yoffset + height_);
	glTexCoord2f(0.0f, downTexCoord); glVertex2i(xoffset, yoffset + height_);
	glEnd();

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 0, 0xff);
	border->draw(xoffset, yoffset);

	glDisable(GL_STENCIL_TEST);

	// Get mouse position
	int x, y;
	SDL_GetMouseState(&x, &y);

	x -= xoffset; // Modify mouse coordinates
	y -= yoffset; // Modify mouse coordinates

	Iterator it;
	for (it = entries.begin(); it != entries.end(); ++it) {		
		if (x >= 7 && x <= width_ - 7 && y >= (*it)->y() && y < (*it)->y() + (*it)->height()) {
			glDisable(GL_TEXTURE_2D); // Disable texturing.
			glBegin(GL_QUADS);
			glColor4f(0.5f, 0.5f, 0.5f, 1.0f); // Should be a shade of gray
			int left = xoffset + 7;
			int top = yoffset + (*it)->y() + 2;
			glVertex2i(left, top);
			glVertex2i(left + width_ - 12, top);
			glVertex2i(left + width_ - 12, top + (*it)->height());
			glVertex2i(left, top + (*it)->height());
			glEnd();
			glEnable(GL_TEXTURE_2D); // Re-Enable Texturing
		}
		(*it)->draw(xoffset, yoffset);
	}
}

cControl *cContextMenu::getControl(int x, int y) {
	cControl *result = 0;

	// Bounding box check for us
	if (x >= 0 && y >= 0 && x < width_ && y < height_) {
		result = this;

		// Only height checks for the children
		if (x >= 7 && x < width_ - 7) {
			Iterator it;
			for (it = entries.begin(); it != entries.end(); ++it) {
				if (y >= (*it)->y() && y < (*it)->y() + (*it)->height()) {
					result = (*it);
					break;
				}
			}
		}
	}

	return result;
}

void cContextMenu::setCallback(fnContextMenuCallback callback, void *customData) {
	if (this->callback) {
		this->callback(this, -1, customData_);
	}

	this->callback = callback;
	customData_ = customData;
}

cContextMenu *ContextMenu = 0;
