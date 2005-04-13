
#include "game/entity.h"

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
}

cEntity::~cEntity() {
}

bool cEntity::isInWorld() const {
	return true;
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
