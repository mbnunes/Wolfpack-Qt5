
#include "game/multi.h"
#include "game/world.h"
#include "muls/multis.h"
#include "utilities.h"

cMulti::cMulti(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial) : cEntity(x, y, z, facet), cDynamicItem(x, y, z, facet, serial) {
	World->removeEntity(this); // cDynamicItem adds us
	type_ = MULTI;
}

cMulti::~cMulti() {
	foreach (cMultiItem *tile, tiles_) {
		World->removeEntity(tile);
		tile->setMulti(0);
		tile->decref();
	}
}

void cMulti::draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip) {
	// Don't draw anything...
}

void cMulti::setId(ushort data) {
	if (data == id_ || data < 0x4000) {
		return;
	}

	id_ = data;
	refreshTiles();
}

void cMulti::refreshTiles() {
	// Clear all current tiles and re-create them
	foreach (cMultiItem *item, tiles_) {
		World->removeEntity(item);
		item->setMulti(0);
		item->decref();
	}
	tiles_.clear();

    // Read the multi information from file and repopulate all the 
	// tiles that are currently in view
	MultiData items = Multis->readMulti(id_);
	foreach (stMultiItem item, items) {
		int tileX = x_ + item.x;
		int tileY = y_ + item.y;

		if (Utilities::simpleDistance(tileX, tileY, World->x(), World->y()) <= 18) {
			cMultiItem *tile = new cMultiItem(tileX, tileY, z_ + item.z, World->facet());
			tile->setMulti(this);
			tile->setId(item.id);
			World->addEntity(tile);
			tiles_.append(tile);
		}
	}
}

void cMulti::refreshTiles(ushort x, ushort y) {
    // Read the multi information from file and repopulate all the 
	// tiles that are currently in view
	MultiData items = Multis->readMulti(id_);
	foreach (stMultiItem item, items) {
		int tileX = x_ + item.x;
		int tileY = y_ + item.y;

		if (tileX == x && tileY == y) {
			cMultiItem *tile = new cMultiItem(tileX, tileY, z_ + item.z, World->facet());
			tile->setMulti(this);
			tile->setId(item.id);
			World->addEntity(tile);
			tiles_.append(tile);
		}
	}
}

void cMulti::removeTile(cMultiItem *tile) {
	for (int i = 0; i < tiles_.size(); ++i) {
		if (tiles_[i] == tile) {
			tiles_.remove(i);
			return;
		}
	}
}

cMultiItem::cMultiItem(ushort x, ushort y, signed char z, enFacet facet) : cEntity(x, y, z, facet), cStaticTile(x, y, z, facet) {
}

cMultiItem::~cMultiItem() {
	if (multi_) {
		multi_->removeTile(this);
	}
}


void cMulti::move(ushort x, ushort y, signed char z) {
	refreshTiles();
}
