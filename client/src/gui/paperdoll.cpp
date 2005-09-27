
#include "gui/paperdoll.h"
#include "gui/worldview.h"
#include "gui/gui.h"
#include "game/mobile.h"
#include "game/dynamicitem.h"
#include "muls/gumpart.h"
#include "muls/tiledata.h"
#include "mainwindow.h"

// Draw order for gumps. Zero comes always first though
static int drawOrder[25] = {20, 5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 21, -1};
static uint drawOrderSize = 22;

cPaperdoll::cPaperdoll() {
	dirty = false;
	memset(layers, 0, sizeof(layers));
	owner_ = 0;
	changeable_ = false;
	pickupTimer.setSingleShot(true);
	connect(&pickupTimer, SIGNAL(timeout()), SLOT(pickupItem()));
}

cPaperdoll::~cPaperdoll() {
	// Disconnect from the owner
	if (owner_) {
		owner_->disconnect(this, SLOT(ownerDeleted()));
		owner_ = 0;
	}

	clear();
}

void cPaperdoll::clear() {
	for (int i = 0; i < LAYER_VISIBLECOUNT+1; ++i) {
		if (layers[i]) {
			layers[i]->decref();
			layers[i] = 0;
		}
  	}
}

void cPaperdoll::setOwner(cMobile *owner) {
	// Disconnect from the owner
	if (owner_) {
		owner_->disconnect(this, SLOT(ownerDeleted()));
		owner_->disconnect(this, SLOT(invalidate()));
		owner_ = 0;
	}

	owner_ = owner;
	
	// Disconnect from the owner
	if (owner_) {
		connect(owner_, SIGNAL(destroyed(QObject*)), this, SLOT(ownerDeleted()));
		connect(owner_, SIGNAL(bodyChanged()), this, SLOT(invalidate()));
		connect(owner_, SIGNAL(equipmentChanged()), this, SLOT(invalidate()));
	}

	dirty = true;
}

void cPaperdoll::ownerDeleted() {
	owner_ = 0;
	dirty = true;
}

void cPaperdoll::draw(int xoffset, int yoffset) {
	if (dirty) {
		update();
	}

	if (!owner_) {
		return;
	}

	if (layers[0]) {
		layers[0]->draw(xoffset + x_, yoffset + y_);
	}

	uint i = 0;
	forever {
		// Get the next layer to draw
		int layer = drawOrder[i++];

		// List terminator is -1
		if (layer == -1) {
			break;
		}

		// Draw the layer
		if (layers[layer]) {
			layers[layer]->draw(xoffset + x_, yoffset + y_);
		}
	}
}

void cPaperdoll::update() {
	if (!dirty) {
		return;
	}

	clear();

	if (!owner_) {
		return;
	}

	ushort background = 0;
	bool female = false;

	// Layer 0 will be the pdoll background
	switch (owner_->body()) {
		case 0x190:
		case 0x3db:
		case 0x192:
			background = 0xC;
			break;
		case 0x191:
		case 0x193:
			background = 0xD;
			female = true;
			break;
		default:
			background = 0;
	}

	// No paperdoll for non-humans
	if (background == 0) {
		return;
	}

	layers[0] = Gumpart->readTexture(background, owner_->hue(), owner_->partialHue());

	for (uint i = LAYER_RIGHTHAND; i < LAYER_VISIBLECOUNT+1; ++i) {
		cDynamicItem *item = owner_->getEquipment(enLayer(i));

		// Get the texture for the item (animation + 50000 or animation + 60000)
		if (item) {
			ushort animation = item->tiledata()->animation();

			if (female && Gumpart->exists(animation + 60000)) {
                layers[i] = Gumpart->readTexture(animation + 60000, item->hue(), item->tiledata()->isPartialHue());
			} else {
				layers[i] = Gumpart->readTexture(animation + 50000, item->hue(), item->tiledata()->isPartialHue());
			}
		}
	}

	dirty = false;
}

cControl *cPaperdoll::getControl(int x, int y) {
	if (x >= 0 && y >= 0 && x < width_ && y < height_) {
		// Check the layers _above_ the background if they're hit.
		// otherwise act as transparent
		for (int i = LAYER_RIGHTHAND; i < LAYER_VISIBLECOUNT+1; ++i) {
			if (layers[i] && layers[i]->hitTest(x, y)) {
                return this;				
			}
		}
	}
	return 0;
}

cDynamicItem *cPaperdoll::itemAtPos(int x, int y) {
	if (!owner_ || x < 0 || y < 0 || x >= width_ || y >= height_) {
		return 0; // No owner or not clicked on us
	}

	// Iterate over the reverse draw order and check if we hit something
	for (int i = drawOrderSize - 1; i >= 0; --i) {
		int layer = drawOrder[i];
		if (layers[layer] && layers[layer]->hitTest(x, y)) {
			return owner_->getEquipment(enLayer(layer));
		}
	}
	
	// Didn't find anything
	return 0;
}

void cPaperdoll::onClick(QMouseEvent *e) {
	QPoint pos = mapFromGlobal(e->pos());
	cDynamicItem *item = itemAtPos(pos.x(), pos.y());
	if (item) {
		item->onClick(e);
	}
}

void cPaperdoll::processDoubleClick(QMouseEvent *e) {
	QPoint pos = mapFromGlobal(e->pos());
	cDynamicItem *item = itemAtPos(pos.x(), pos.y());
	if (item) {
		item->onDoubleClick(e);
	}
}

void cPaperdoll::onMouseDown(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseDownPos = mapFromGlobal(e->pos());
	
		// Something was below the mouse otherwise this wouldn't have fired
		tracking = true;
	
		pickupTimer.start(1000);
	}
}

void cPaperdoll::onMouseMotion(int xrel, int yrel, QMouseEvent *e) {
	// If we're tracking, that means we'll pickup an item
	if (tracking) {
		pickupItem();
	}
}

/*
	Pickup the item below the mouse location stored in mouseDownPos
*/
void cPaperdoll::pickupItem() {	
	tracking = false;
	pickupTimer.stop();

	// Pickup the item below the mousedown position
	cDynamicItem *item = itemAtPos(mouseDownPos.x(), mouseDownPos.y());

	if (item) {
		WorldView->addSysMessage(QString("picking up item 0x%1").arg(item->serial(), 0, 16));
	}	
}

void cPaperdoll::onMouseUp(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		// Stop the pickup timer when we release the button early enough
		pickupTimer.stop();
		tracking = false;
	} else  if (e->button() == Qt::RightButton) {
		// Close the window when we're rightclicking equipment
		QPoint pos = mapFromGlobal(e->pos());
		if (getControl(pos.x(), pos.y()) == this) {
			if (parent_->isWindow() && ((cWindow*)parent_)->isClosable()) {
				Gui->queueDelete(parent_);
			}
		}
	}
}
