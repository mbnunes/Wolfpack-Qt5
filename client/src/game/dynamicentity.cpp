
#include "game/dynamicentity.h"
#include "game/world.h"
#include "gui/label.h"

cDynamicEntity::cDynamicEntity(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial) : cEntity(x, y, z, facet) {
	serial_ = serial;
	World->registerDynamic(this);
}

cDynamicEntity::cDynamicEntity(unsigned int serial) : cEntity() {
	serial_ = serial;
	World->registerDynamic(this);
}

cDynamicEntity::~cDynamicEntity() {
	World->unregisterDynamic(this);	
}

void cDynamicEntity::move(unsigned short x, unsigned short y, signed char z) {
	World->removeEntity(this);
	if (isInWorld()) {
		x_ = x;
		y_ = y;
		z_ = z;
		World->addEntity(this);
	}
}

void cDynamicEntity::setFacet(enFacet facet) {
	World->removeEntity(this);
	facet_ = facet;
	World->addEntity(this);
}

void cDynamicEntity::addOverheadText(QString message, unsigned short color, unsigned char font) {
	// Calculate timeout...
	unsigned int timeout = 12000; // Just say 12 seconds for now

	cOverheadInfo info;
	info.timeout = QTime::currentTime().addMSecs(timeout);
	cLabel *label = new cLabel(message, font, color, true, ALIGN_LEFT, false);
	label->setWidth(240);
	label->update();
	info.control = label;
	overheadText.append(info);
}
