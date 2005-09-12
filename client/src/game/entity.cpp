
#include "game/entity.h"
//Added by qt3to4:
#include <QMouseEvent>

#include "log.h"
#include "gui/label.h"
#include "gui/gui.h"
#include "gui/tooltip.h"
#include "game/world.h"

/*
	This private class manages overhead text for Entities.
	(All entities can have overhead text. Static items when clicked
	on etc.)
*/
class cOverheadText {
protected:
	cEntity *entity_;
public:
	cEntity *entity() const;
	void setEntity(cEntity *entity);

	virtual ~cOverheadText();
};

inline void cOverheadText::setEntity(cEntity *entity) {
	entity_ = entity;
}

inline cEntity *cOverheadText::entity() const {
	return entity_;
}

cOverheadText::~cOverheadText() {
}

class cOverheadTextUnicode : public cLabel, public cOverheadText {
public:
	cOverheadTextUnicode(QString message, unsigned short color, unsigned char font, cEntity *source);
};

cOverheadTextUnicode::cOverheadTextUnicode(QString message, unsigned short color, unsigned char font, cEntity *source) : cLabel(message, font, color, true, ALIGN_LEFT, false) {
	setWidth(240);
    update();
	entity_ = source;
}

void cEntity::removeOverheadText(cControl *overhead) {
	for (int i = 0; i < overheadTexts.size(); ++i) {
		if (overheadTexts[i] == overhead) {
			overheadTexts.remove(i);
			return;
		}
	}
}

void cEntity::addOverheadText(QString message, unsigned short color, unsigned char font) {
	cOverheadTextUnicode *obj = new cOverheadTextUnicode(message, color, font, this);
	overheadTexts.append(obj);
}

cEntity::cEntity() {
	refcount = 1;
	x_ = 0;
	y_ = 0;
	z_ = 0;
	facet_ = INTERNAL;
	drawx_ = -9999;
	drawy_ = -9999;
	width_ = 0;
	height_ = 0;
	type_ = UNKNOWN;
	priority_ = 0;
	cellid_ = -1;
	prioritySolver_ = 0;
	priorityBonus_ = 0;
	tooltipKey_ = 0;
}

cEntity::cEntity(unsigned short x, unsigned short y, signed char z, enFacet facet) {
	refcount = 1;
	x_ = x;
	y_ = y;
	z_ = z;
	facet_ = facet;
	drawx_ = -9999;
	drawy_ = -9999;
	width_ = 0;
	height_ = 0;
	type_ = UNKNOWN;
	priority_ = 0;
	cellid_ = -1;
	prioritySolver_ = 0;
	priorityBonus_ = 0;
	tooltipKey_ = 0;
}

cEntity::~cEntity() {
	World->removeEntity(this);
	Gui->removeOverheadText(this);
	if (Tooltip->entity() == this) {
		Tooltip->setEntity(0);
		Tooltip->setTooltip(0);
		Tooltip->setVisible(false);
	}
}

bool cEntity::isInWorld() const {
	return facet_ != INTERNAL;
}

void cEntity::draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip) {
}

bool cEntity::hitTest(int x, int y) {
	return true;
}

void cEntity::onClick(QMouseEvent *e) {
}

void cEntity::onRightClick(QMouseEvent *e) {
}

void cEntity::updatePriority() {
}

void cEntity::onDoubleClick(QMouseEvent *e) {
}
