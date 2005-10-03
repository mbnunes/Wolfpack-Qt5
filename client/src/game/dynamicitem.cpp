
#include "game/dynamicitem.h"
#include "game/world.h"
#include "game/mobile.h"
#include "game/tooltips.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "gui/gui.h"
#include "gui/containergump.h"
#include "gui/worldview.h"
#include "mainwindow.h"

cDynamicItem::cDynamicItem(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial) : cEntity(x, y, z, facet), cDynamicEntity(x, y, z, facet, serial) {
	alwaysMovable = false;
	invisible = false;
	type_ = ITEM;
	wearer_ = 0;
	container_ = 0;
	positionState_ = InWorld; // Contrusctor take care of movement	
	lastClickX_ = 0;
	lastClickY_ = 0;
	deleting = false;
	containerGump_ = 0;
	World->addEntity(this);
}

cDynamicItem::cDynamicItem(cDynamicItem *container, unsigned int serial) : cDynamicEntity(serial) {
	alwaysMovable = false;
	invisible = false;
	type_ = ITEM;
	wearer_ = 0;
	container_ = 0;
	positionState_ = InLimbo;	
	deleting = false;
	containerGump_ = 0;
	lastClickX_ = 0;
	lastClickY_ = 0;
	move(container);
}

cDynamicItem::cDynamicItem(cMobile *wearer, unsigned char layer, unsigned int serial) : cDynamicEntity(serial) {
	alwaysMovable = false;
	invisible = false;
	type_ = ITEM;
	wearer_ = 0;
	container_ = 0;
	positionState_ = InLimbo;
	deleting = false;
	containerGump_ = 0;
	lastClickX_ = 0;
	lastClickY_ = 0;
	move(wearer, layer);
}

cDynamicItem::~cDynamicItem() {
	deleting = true;
	cleanPosition();
	foreach (cDynamicItem *item, content_) {
		item->decref();
	}

	if (containerGump_) {
		containerGump_->setContainer(0); // Make sure the container gump doesn't try to unassign itself from us
		Gui->queueDelete(containerGump_);
	}
}

void cDynamicItem::move(unsigned short x, unsigned short y, signed char z) {
	cleanPosition();
	if (isInWorld()) {
		x_ = x;
		y_ = y;
		z_ = z;
		World->addEntity(this);
		positionState_ = InWorld;
	}
}

void cDynamicItem::move(cMobile *wearer, unsigned char layer) {
	cleanPosition();
	wearer_ = wearer;
	layer_ = (enLayer)layer;

    wearer->addEquipment(this);	
	positionState_ = OnMobile;
}

void cDynamicItem::move(cDynamicItem *container) {
	cleanPosition();
	
	container->removeItem(this);
	container->content_.append(this);

	if (container->containerGump()) {
		container->containerGump()->refreshContent();
	}
	
	container_ = container;
	positionState_ = InContainer;
}

void cDynamicItem::moveToLimbo() {
	cleanPosition();
	positionState_ = InLimbo;
}

void cDynamicItem::cleanPosition() {
	switch (positionState_) {
		case OnMobile:
			if (wearer_) {
				positionState_ = InLimbo;
				wearer_->removeEquipment(this);
				wearer_ = 0;
			}
			break;
		case InContainer:
			if (container_) {
                container_->removeItem(this);
			}
			break;
		case InWorld:
			World->removeEntity(this);
			break;
		default:
			break;
	}

	positionState_ = InLimbo;

	if (containerGump_) {
		containerGump_->setContainer(0); // Make sure the container gump doesn't try to unassign itself from us
		Gui->queueDelete(containerGump_);
		containerGump_ = 0;
	}
}

void cDynamicItem::setId(unsigned short data) {
	cStaticTile::setId(data);
}

void cDynamicItem::setHue(unsigned short data) {
	cStaticTile::setHue(data);
}

void cDynamicItem::onClick(QMouseEvent *e) {
	lastClickX_ = e->x() - drawx_;
	lastClickY_ = e->y() - drawy_;

	// Only send singleclick packets if tooltips are disabled
	if (!UoSocket->isTooltips()) {
		UoSocket->send(cSingleClickPacket(serial_));
	}

	// If ContextMenus are enabled, send a request packet for that
	if (UoSocket->isContextMenus()) {
		UoSocket->send(cRequestContextMenu(serial_));
	}
}

void cDynamicItem::onDoubleClick(QMouseEvent *e) {
	cDoubleClickPacket packet(serial_);
	UoSocket->send(packet);
}

void cDynamicItem::removeItem(cDynamicItem *item) {
	if (deleting) {
		return;
	}

	for (int i = 0; i < content_.size(); ++i) {
		if (content_[i] == item) {
			content_.remove(i);
			if (containerGump_) {
				containerGump_->refreshContent();
			}
			return;
		}
	}
}

void cDynamicItem::showContent(int x, int y, ushort gump) {
	if (containerGump_) {
		containerGump_->setContainer(0);
		Gui->queueDelete(containerGump_);		
	}

	containerGump_ = new cContainerGump(gump, hue_);
	containerGump_->setContainer(this);
	Gui->addControl(containerGump_);
}

void cDynamicItem::showContent(ushort gump) {
	int x = 50;
	int y = 50;

	if (containerGump_) {
		x = containerGump_->x();
		y = containerGump_->y();
	}

	showContent(x, y, gump);
}

void cDynamicItem::updatePriority() {
	priorityBonus_ = 0;

	if (tiledata_) {
		if (!tiledata_->isBackground()) {
			++priorityBonus_;
		}
		if (tiledata_->height() > 0) {
			++priorityBonus_;
		}
	}

	priority_ = z_ + priorityBonus_;
}

void cDynamicItem::setAmount(ushort amount) {
	amount_ = qMax<ushort>(1, amount);
	if (tiledata_ && tiledata_->isGeneric()) {
		if (amount_ == 1) {
			setDrawStacked(false);
		} else {
			setDrawStacked(true);
		}
	} else {
		setDrawStacked(false);
	}
}

bool cDynamicItem::canMove() const {
	// If it has weight 255 and not the appropiate flag,
	// return false
	if (tiledata_->weight() == 255 && !alwaysMovable) {
		return false;
	}

	return true;
}

float cDynamicItem::getAlpha() const {
	if (invisible) {
		return 0.6f;
	} else {
		return cStaticTile::getAlpha();
	}
}

void cDynamicItem::draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip) {
	if (invisible && !Player->isDead()) {
		GLWidget->enableGrayShader();
	}
	
	cStaticTile::draw(cellx, celly, leftClip, topClip, rightClip, bottomClip);

	if (invisible && !Player->isDead()) {
		GLWidget->disableGrayShader();
	}
}

void cDynamicItem::onMouseEnter() {
	if (canMove() && !highlight) {
		originalHue_ = hue_;
		highlight = true;
		hue_ = 0x36;
		if (texture) {
			texture->decref();
			texture = 0;
		}
		if (animation) {
			animation->decref();
			animation = 0;
		}
	}
}

void cDynamicItem::onMouseLeave() {
	if (highlight) {
		highlight = false;
		hue_ = originalHue_;
		if (texture) {
			texture->decref();
			texture = 0;
		}
		if (animation) {
			animation->decref();
			animation = 0;
		}
	}
}
