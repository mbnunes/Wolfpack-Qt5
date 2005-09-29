
#include "gui/paperdoll.h"
#include "gui/worldview.h"
#include "gui/gui.h"
#include "game/mobile.h"
#include "game/dynamicitem.h"
#include "gui/worldview.h"
#include "gui/cursor.h"
#include "muls/gumpart.h"
#include "muls/tiledata.h"
#include "mainwindow.h"
#include "game/targetrequest.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "utilities.h"

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
	background = 0;
	leftMargin_ = 8;
	topMargin_ = 19;
	previewLayer = 0;

	connect(Gui, SIGNAL(itemDropped()), SLOT(checkPreview()));
}

cPaperdoll::~cPaperdoll() {
	delete background;

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
		connect(owner_, SIGNAL(equipmentChanged()), this, SLOT(checkPreview()));
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

	if (background) {
		background->draw(xoffset + x_, yoffset + y_);
	}

	xoffset += leftMargin_;
	yoffset += topMargin_;

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
			// Draw the preview layer at 75% alpha
			if (previewLayer == layer) {
				layers[layer]->draw(xoffset + x_, yoffset + y_, 0.75f);
			} else {
				layers[layer]->draw(xoffset + x_, yoffset + y_);
			}
		}
	}
}

ushort cPaperdoll::getBackground(bool &female) {
	ushort background = 0;
	female = false;

	// Layer 0 will be the pdoll background
	switch (owner_->body()) {
		case 123:
		case 124:
		case 125:
		case 126:
		case 141:
		case 175:
		case 183:
		case 185:
		case 750:
		case 764:
		case 766:
		case 767:
		case 768:
		case 769:
		case 771:
		case 777:
		case 744:
		case 0x190:
		case 0x3db:
		case 0x192:
			background = 0xC;
			break;
		case 184:
		case 186:
		case 751:
		case 765:
		case 770:
		case 773:
		case 774:
		case 745:
		case 0x191:
		case 0x193:
			background = 0xD;
			female = true;
			break;
		default:
			background = 0;
	}

	return background;
}

void cPaperdoll::update() {
	if (!dirty) {
		return;
	}

	clear();

	if (!owner_) {
		return;
	}

	bool female;
	ushort background = getBackground(female);

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
	// Most likely the background will be hit
	if (background && background->hitTest(x, y)) {
		return this;
	}

	x += leftMargin_;
	y += topMargin_;

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
		if (layers[layer] && layers[layer]->hitTest(x - leftMargin_, y - topMargin_)) {
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

		cDynamicItem *item = itemAtPos(mouseDownPos.x(), mouseDownPos.y());

		if (item) {
			// Check for target requests
			if (WorldView && WorldView->targetRequest()) {
				cTargetRequest *request = WorldView->targetRequest();

				if (item && request->isValidTarget(item)) {
					WorldView->targetResponse(item);
					return;
				}
			}

			// if the item is on the backpack layer, ignore it
			if (item->layer() == LAYER_BACKPACK) {
				return;
			}
		
			// Something was below the mouse otherwise this wouldn't have fired
			tracking = true;
		
			pickupTimer.start(1000);
			return;
		}
	}

	// No item was affected
	if (parent_) {
		parent_->onMouseDown(e);
		GLWidget->setMouseCapture(parent_);
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
	if (Gui->isDragging()) {
		return;
	}

	tracking = false;
	pickupTimer.stop();

	// Pickup the item below the mousedown position
	cDynamicItem *item = itemAtPos(mouseDownPos.x(), mouseDownPos.y());

	if (item) {
		Gui->dragItem(item);
		item->moveToLimbo();
		item->decref();

		// send a drag-request to the server
		UoSocket->send(cGrabItemPacket(item->serial()));
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

bool cPaperdoll::acceptsItemDrop(cDynamicItem *item) {
	if (!owner_) {
		return false;
	}
	return true;
}

void cPaperdoll::dropItem(cDynamicItem *item) {
	if (!owner_) {
		return;
	}

	QPoint pos = mapFromGlobal(GLWidget->mapFromGlobal(QCursor::pos()));

	// If the item was dropped on an equipped container, drop it into that container instead
	cDynamicItem *cont = itemAtPos(pos.x(), pos.y());
	
	if (cont && cont->tiledata()->isContainer()) {
		UoSocket->send(cDropItemPacket(item->serial(), ~0, ~0, ~0, cont->serial()));
	} else {
		UoSocket->send(cWearItemPacket(item->serial(), item->tiledata()->layer(), owner_->serial()));
	}
	Gui->dropItem();
}

void cPaperdoll::onMouseEnter() {
	if (WorldView->targetRequest()) {
		Cursor->setCursor(CURSOR_TARGET);
	} else {
		Cursor->setCursor(CURSOR_NORMAL);
	}

	checkPreview();
}

void cPaperdoll::onMouseLeave() {
	if (previewLayer != 0 && layers[previewLayer]) {
		layers[previewLayer]->decref();
		layers[previewLayer] = 0;
		previewLayer = 0;
	}

	Cursor->setCursor(CURSOR_NORMAL);
}

void cPaperdoll::setBackground(ushort gump, ushort hue, bool partialHue) {
	delete background;
	background = Gumpart->readTexture(gump, hue, partialHue);
	width_ = background->realWidth();
	height_ = background->realHeight();
}

void cPaperdoll::processDefinitionAttribute(QString name, QString value) {
	if (name == "background") {
		setBackground(Utilities::stringToUInt(value));
	} else if (name == "leftmargin") {
		leftMargin_ = Utilities::stringToInt(value);
	} else if (name == "topmargin") {
		topMargin_ = Utilities::stringToInt(value);
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cPaperdoll::checkPreview() {
	if (!Gui->isDragging() && previewLayer != 0) {
		layers[previewLayer]->decref();
		layers[previewLayer] = 0;
		previewLayer = 0;
	}
	if (Gui->isDragging() && previewLayer == 0) {
		// Only process if the mouse is currently above us
		if (GLWidget->lastMouseMovement() == this) {
			// Check if we should show a preview for the dragged item
			cDynamicItem *dragged = Gui->draggedItem();
			if (dragged) {
				enLayer layer = enLayer(dragged->tiledata()->layer());
				if (layer > 0 && layer < LAYER_VISIBLECOUNT) {
					update(); // Make sure that the layer is not occupied by an old item

					if (!layers[layer]) {
						bool female;
						getBackground(female);

						previewLayer = layer;
						ushort animation = dragged->tiledata()->animation();

						if (female && Gumpart->exists(animation + 60000)) {
							layers[previewLayer] = Gumpart->readTexture(animation + 60000, dragged->hue(), dragged->tiledata()->isPartialHue());
						} else {
							layers[previewLayer] = Gumpart->readTexture(animation + 50000, dragged->hue(), dragged->tiledata()->isPartialHue());
						}
					}
				}
			}
		}
	}
}
