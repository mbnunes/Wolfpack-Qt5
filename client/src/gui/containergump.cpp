
#include <QMouseEvent>

#include "gui/containergump.h"
#include "gui/gumpimage.h"
#include "gui/itemimage.h"
#include "gui/gui.h"
#include "gui/worldview.h"
#include "game/world.h"
#include "game/targetrequest.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"

cContainerItemImage::cContainerItemImage(cDynamicItem *item) : cItemImage(item->id(), item->hue(), false, false) {
	originalHue = item->hue();
	serial_ = item->serial();
}

void cContainerItemImage::onMouseEnter() {
	setItem(id, 0x36, false, landtile);
}

void cContainerItemImage::onMouseDown(QMouseEvent *e) {
	// Check for target requests
	if (e->button() == Qt::LeftButton) {		
		if (WorldView && WorldView->targetRequest()) {
            cTargetRequest *request = WorldView->targetRequest();
			cDynamicItem *item = World->findItem(serial_);

			if (item && request->isValidTarget(item)) {
				WorldView->targetResponse(item);
				return;
			}
		}
	}

	return cItemImage::onMouseDown(e);
}

void cContainerItemImage::onClick(QMouseEvent *e) {
	// Let the item know where it has been drawn
	cDynamicItem *item = World->findItem(serial_);
	if (item) {
		item->setLastClickX(item->drawx() - e->x());
		item->setLastClickY(item->drawy() - e->y());
	}

	UoSocket->send(cSingleClickPacket(serial_));
}

void cContainerItemImage::processDoubleClick(QMouseEvent *e) {
	// Let the item know where it has been drawn
	cDynamicItem *item = World->findItem(serial_);
	if (item) {
		item->setLastClickX(item->drawx() - e->x());
		item->setLastClickY(item->drawy() - e->y());
	}

	UoSocket->send(cDoubleClickPacket(serial_));
}

void cContainerItemImage::draw(int xoffset, int yoffset) {
	// Let the item know where it has been drawn
	cDynamicItem *item = World->findItem(serial_);
	if (item) {
		item->setDrawx(xoffset + x_ + width_ / 2);
		item->setDrawy(yoffset + y_ + height_ / 2);
	}

	cItemImage::draw(xoffset, yoffset);
}

void cContainerItemImage::onMouseLeave() {	
	setItem(id, originalHue, false, landtile);
}

cContainerGump::cContainerGump(ushort id, ushort hue) {
	// TODO: Configuration option
	hue = 0;

	// Create the backdrop gump and add it to the window (and resize accordingly)
	background = new cGumpImage(id, hue, false, true); 
	background->update(); // get the real size

    setSize(background->width(), background->height());
	background->setPosition(0, 0);
	background->setMoveHandle(true);	
	addControl(background); // Add the background control

	setMovable(true); // Containers are movable

	container_ = 0; // Initialize to 0
	contentChanged = false;
}

cContainerGump::~cContainerGump() {
	if (container_ && container_->containerGump_ == this) {
        container_->containerGump_ = 0;		
	}
}

void cContainerGump::onClick(QMouseEvent *e) {
	// Right mouse button closes the container
	if (e->button() == Qt::RightButton) {
		Gui->queueDelete(this);
	}
}

void cContainerGump::setContainer(cDynamicItem *container) {
	if (container_ != container) {
		flagContentChanged();
	}
	container_ = container;
}

void cContainerGump::draw(int xoffset, int yoffset) {
	if (contentChanged) {
		refreshContent();
		contentChanged = false;
	}
	cWindow::draw(xoffset, yoffset);
}

void cContainerGump::refreshContent() {
	foreach(cControl *ctrl, controls) {
		// Delete everything except the background
		if (ctrl != background) {
			delete ctrl;
		}
	}

    // Clear controls, re-add the background
	controls.clear();
	controls.append(background);

	// Nothing else to do
	if (!container_) {
		return;
	}

	// Iterate over the container content and add tilepics for it
	foreach (cDynamicItem *item, container_->content()) {
		bool partialHue = false;
		if (item->tiledata()) {
			item->tiledata()->isPartialHue();
		}

		//cItemImage *gump = new cItemImage(item->id(), item->hue(), partialHue, false);
		cContainerItemImage *gump = new cContainerItemImage(item);
		gump->setPosition(item->containerX(), item->containerY());
		addControl(gump);
	}
}
