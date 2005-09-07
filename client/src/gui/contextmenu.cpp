
#include "gui/checkertrans.h"
#include "gui/contextmenu.h"
#include "gui/gui.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "surface.h"
#include "mainwindow.h"
#include <QCursor>
#include <qgl.h>

// Our custom label
class cContextMenuEntry : public cLabel {
protected:
	int id_;

public:
	cContextMenuEntry(const QString &text, unsigned short hue, int id = -1) : cLabel(text, 1, hue, false) {
		id_ = id;
	}

	void onMouseUp(QMouseEvent *e) {
		if (Gui->getControl(e->x(), e->y()) == this) {
			emit ContextMenu->clicked(id_);			
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
	checkertransOff = 0;
	checkertransOn = 0;
	border = 0;
	serial_ = 0;
	connect(this, SIGNAL(clicked(ushort)), SLOT(sendResponse(ushort)));
}

cContextMenu::~cContextMenu() {
}

void cContextMenu::addEntry(const QString &name, unsigned short hue, int id) {
	cContextMenuEntry *entry = new cContextMenuEntry(name, hue);
	entry->update();
	entry->setId(id);
	entries.append(entry);
	addControl(entry);
}

void cContextMenu::show() {
	QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
	show(pos.x(), pos.y());
}

void cContextMenu::show(int x, int y) {
	if (!border) {
		checkertransOn = new cCheckerTrans(true);
		checkertransOn->setAlign(CA_CLIENT);
		addControl(checkertransOn);

		border = new cBorderGump(0xa3c);
		border->setAlign(CA_CLIENT);
		border->setMoveHandle(false);
		addControl(border, true);

		checkertransOff = new cCheckerTrans(false);
		checkertransOff->setAlign(CA_CLIENT);
		addControl(checkertransOff);
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
}

void cContextMenu::hide() {
	setVisible(false);
}

unsigned char stencils[64*64] = {1, };

void cContextMenu::draw(int xoffset, int yoffset) {
	xoffset += x_;
	yoffset += y_;

	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 0, 0xff);

	checkertransOn->draw(xoffset, yoffset);
	border->draw(xoffset, yoffset);
	checkertransOff->draw(xoffset, yoffset);

	// Get mouse position
	QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
	int x= pos.x();
	int y = pos.y();
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

	glDisable(GL_STENCIL_TEST);
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

void cContextMenu::sendResponse(ushort id) {
	UoSocket->send(cContextMenuResponsePacket(serial_, id));
	hide();
}

cContextMenu *ContextMenu = 0;
