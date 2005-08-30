
#include "gui/container.h"
#include "gui/itemimage.h"
#include "muls/art.h"

cItemImage::cItemImage(unsigned short id, unsigned short hue, bool partialhue, bool landtile) {
	item = 0;
	setItem(id, hue, partialhue, landtile);
}

cItemImage::~cItemImage() {
	if (item) {
		item->decref();
	}
}

void cItemImage::draw(int xoffset, int yoffset) {
	if (item) {
		item->draw(xoffset + x_, yoffset + y_);
	}
}

void cItemImage::setItem(unsigned short id, unsigned short hue, bool partialhue, bool landtile) {
	if (!this->item || this->id != id || this->hue != hue || this->partialhue != partialhue || this->landtile != landtile) {
		if (item) {
			item->decref();
		}
	
		this->id = id;
		this->hue = hue;
		this->partialhue = partialhue;
		this->landtile = landtile;

		if (landtile) {
			item = Art->readLandTexture(id);
		} else {
			item = Art->readItemTexture(id, hue, partialhue);
		}

		int oldwidth = width_;
		int oldheight = height_;
		if (item) {
			width_ = item->realWidth();
			height_ = item->realHeight();
			onChangeBounds(x_, y_, oldwidth, oldheight);
		}
	}
}

cControl *cItemImage::getControl(int x, int y) {
	if (item && item->hitTest(x, y)) {
		return this;
	} else {	
		return 0;
	}
}
