
#include "game/dynamicitem.h"
#include "game/world.h"
#include "game/mobile.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"

cDynamicItem::cDynamicItem(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial) : cEntity(x, y, z, facet), cDynamicEntity(x, y, z, facet, serial) {
	type_ = ITEM;
	wearer_ = 0;
	container_ = 0;
	positionState_ = InWorld; // Contrusctor take care of movement
	World->addEntity(this);
}

cDynamicItem::cDynamicItem(cDynamicItem *container, unsigned int serial) : cDynamicEntity(serial) {
	type_ = ITEM;
	wearer_ = 0;
	container_ = 0;
	positionState_ = InLimbo;
	move(container);
}

cDynamicItem::cDynamicItem(cMobile *wearer, unsigned char layer, unsigned int serial) : cDynamicEntity(serial) {
	type_ = ITEM;
	wearer_ = 0;
	container_ = 0;
	positionState_ = InLimbo;
	move(wearer, layer);
}

cDynamicItem::~cDynamicItem() {
	cleanPosition();
}

void cDynamicItem::move(unsigned short x, unsigned short y, signed char z) {
	cleanPosition();
	positionState_ = InWorld;
	cDynamicEntity::move(x, y, z);
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
	
	// Add us to the container

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
			}
			break;
		case InContainer:
			if (container_) {
			}
			break;
		case InWorld:
			World->removeEntity(this);
			break;
	}
}

void cDynamicItem::setId(unsigned short data) {
	cStaticTile::setId(data);

}

void cDynamicItem::setHue(unsigned short data) {
	cStaticTile::setHue(data);
}

void cDynamicItem::onDoubleClick(QMouseEvent *e) {
	cDoubleClickPacket packet(serial_);
	UoSocket->send(packet);
}
