
#include "engine.h"
#include "game/world.h"
#include "game/groundtile.h"
#include "game/statictile.h"
#include "game/mobile.h"
#include "gui/gui.h"
#include "gui/worldview.h"
#include "muls/maps.h"

cWorld::cWorld() : groundCache(2500, 1999) {
	x_ = 0;
	y_ = 0;
	z_ = 0;
	facet_ = TRAMMEL;
	roofCap_ = 1024;
	groundCache.setAutoDelete(true);
	smoothMoveEnd_ = 0;
	drawxoffset = 0;
	drawyoffset = 0;
}

cWorld::~cWorld() {
	clearEntities();
}

void cWorld::changeFacet(enFacet facet) {
	if (facet != facet_) {
		clearEntities();
		x_ = 0;
		y_ = 0;
		z_ = 0;
		facet_ = facet;
		clearGroundCache();
	}
}

void cWorld::clearEntities() {
	// Clear entities.
	Iterator it;
	for (it = entities.begin(); it != entities.end(); ++it) {
		(*it)->decref();
	}

	entities.clear();
}

void cWorld::cleanupEntities() {
	Iterator it;
	for (it = entities.begin(); it != entities.end(); ++it) {
		if ((*it)->isInWorld()) {
			int distance = Utilities::distance((*it)->x(), (*it)->y(), x_, y_);
			if (distance > 18) {
				(*it)->decref();
				it = entities.remove(it);
			}
		}
	}
}

void cWorld::loadCell(unsigned short x, unsigned short y) {
	stMapCell *cell = Maps->getMapCell(facet_, x, y);

	// Invisible tiles
	if (cell->id != 2) {
		cGroundTile *ground = new cGroundTile(x, y, cell->z, facet_);

		// Get right/left/bottom cell and calculate average z value
		int rightz = Maps->getMapCell(facet_, x + 1, y)->z;
		int bottomz = Maps->getMapCell(facet_, x + 1, y + 1)->z;
		int leftz = Maps->getMapCell(facet_, x, y + 1)->z;
		
		if (abs(cell->z - bottomz) >= abs(leftz - rightz)) {
			ground->setAveragez((cell->z + bottomz) / 2);
		} else {
			ground->setAveragez((leftz + rightz) / 2);
		}

		ground->setId(cell->id);
		addEntity(ground);
	}

	// Iterate over the statics and add them
	StaticBlock *block = Maps->getStaticBlock(facet_, x, y);
	if (block) {
		StaticBlock::iterator it;
		for (it = block->begin(); it != block->end(); ++it) {
			if (it->xoffset == x % 8 && it->yoffset == y % 8) {
				cStaticTile *tile = new cStaticTile(x, y, it->z, facet_);
				tile->setId(it->id);
				tile->setHue(it->color);
				addEntity(tile);
			}
		}
	}
}

void cWorld::moveCenter(unsigned short x, unsigned short y, signed char z, bool fresh) {
	// Save the old coordinates for distance calculations (below)
	int oldx = x_;
	int oldy = y_;

	// Save the new world center
	x_ = x;
	y_ = y;
	z_ = z;

	// Calculate the distance between the old and new center
	int distance = Utilities::distance(oldx, oldy, x, y);

	// left, top, right, bottom define a rectangle on the uo map
	// we're going to use to load new tiles
	unsigned short left = QMAX(0, (short)x - 18);
	unsigned short top = QMAX(0, (short)y - 18);
	unsigned short right = x + 18;
	unsigned short bottom = y + 18;

	// If a fresh thingy was requested
	if (fresh || distance > 12) {
		// clear all old entities
		clearEntities();
	
		// iterate over the rectangle
		for (x = left; x <= right; ++x) {
			for (y = top; y <= bottom; ++y) {
				// Only add cells if they're within a 18 tile radius
				if (Utilities::distance(x_, y_, x, y) <= 18) {
					loadCell(x, y);
				}
			}
		}
	} else {
		// Remove unused entries first
		cleanupEntities();

		// Most likely left/top/etc. could be optimized so only changed areas are loaded
		for (x = left; x <= right; ++x) {
			for (y = top; y <= bottom; ++y) {
				// Compute old and new distance from the center to the tile
				int olddist = Utilities::distance(oldx, oldy, x, y);
				int newdist = Utilities::distance(x_, y_, x, y);

				// load the tile if it has recently come into view
				if (olddist > 18 && newdist <= 18) {
					loadCell(x, y);
				}
			}
		}
	}
}

// Checks if the given entity should be sorted before the
// second entity
inline bool insertBefore(cEntity *entity, cEntity *next) {
	// y-axis is the strongest sorter
	if (entity->y() != next->y()) {
		return entity->y() < next->y();
	} else if (entity->x() != next->x()) {
        return entity->x() < next->x();
	
	} else {
		// Do in-cell sorting
		int priority = entity->priority() - next->priority();
	
		if (priority > 0) {
			return false; // Our priority is higher than the next tile
		} else if (priority < 0) {
			return true; // Our priority is smaller, so sort before us.
		} else {
			return false; // If the priority is the same, tiles that come later, draw later
		}
	}
}

void cWorld::addEntity(cEntity *entity) {
	// Update the tile priority of the new entity first
	entity->updatePriority();
	Iterator it(entities.begin());

	// Search for an appropiate place in the list
	for (; it != entities.end(); ++it) {
		// See if the entity should be inserted before our current element
		if (insertBefore(entity, *it)) {
			entities.insert(it, entity); // Insert it before this entity
			return;
		}
	}

	entities.append(entity); // Simply append now
}

void cWorld::removeEntity(cEntity *entity) {
	entities.remove(entity);
}

void cWorld::smoothMove(int x, int y) {
	// Cache old values
	int oldx = x_;
	int oldy = y_;
	int oldz = z_;

	// TODO: Calculate correct new Z coordinate
	int newz = z_;
	newz = Maps->getMapCell(facet_, x_ + x, y_ + y)->z;

	// Move center of the world
	// TODO: Write an optimized loading technique for faster loading
	// and less distance checks
	moveCenter(oldx + x, oldy + y, newz);
	
	// Get old drawing coordinates
	int diffx = oldx - x_;
	int diffy = oldy - y_;
	int diffz = oldz - z_;
	drawxoffset = (diffx - diffy) * 22;
	drawyoffset = (diffx + diffy) * 22 - diffz * 4;

	// Calculate new direction for the player
	int direction = Utilities::direction(oldx, oldy, x_, y_); 

	xOffsetDecrease = drawxoffset / 370.0f;
	yOffsetDecrease = drawyoffset / 370.0f;

	// Move the player
	Player->smoothMove(drawxoffset, drawyoffset, 370);
	Player->setDirection(direction);
	Player->move(x_, y_, z_);
	Player->playAction(0, 370); // Play walk for the time of move

	// Set the smooth move timeouts, Take 125 ms
	smoothMoveTime_ = 370;
	smoothMoveEnd_ = SDL_GetTicks() + smoothMoveTime_;
	nextSmoothMoveUpdate = 0;
	currentSmoothMoveFactor = 0.0f;
}

void cWorld::draw(int x, int y, int width, int height) {
	clearGroundCache(); // Clear unneeded cache items

	int diffx, diffy, diffz;
	int cellx, celly;
	int centerx = x + (width / 2);
	int centery = y + (height / 2);

	// Smooth move handling. 
	if (smoothMoveEnd_ != 0) {
		int moveProgress = smoothMoveTime_ - (smoothMoveEnd_ - SDL_GetTicks());
		if (moveProgress < 0 || moveProgress >= (int)smoothMoveTime_) {
			smoothMoveEnd_ = 0;
			drawxoffset = 0;
			drawyoffset = 0;
		} else {
			centerx -= (drawxoffset - moveProgress * xOffsetDecrease);
			centery -= (drawyoffset - moveProgress * yOffsetDecrease);
			/*if (moveProgress <= 0.0f) {
				centerx -= drawxoffset;
				centery -= drawyoffset;
			} else {
				//if (nextSmoothMoveUpdate < SDL_GetTicks()) {
					currentSmoothMoveFactor = 1.0f - (float)moveProgress / (float)smoothMoveTime_;
					//nextSmoothMoveUpdate = SDL_GetTicks() + (smoothMoveTime_;
					// Limit ups for smooth moving??
				//}
				centerx -= (int)(currentSmoothMoveFactor * (float)drawxoffset);
				centery -= (int)(currentSmoothMoveFactor * (float)drawyoffset);
			}*/
		}
	}

	// Set Scissor Box
	glLoadIdentity();
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, Engine->height() - (y + height), width, height);

	// Set up the clipping variables
	int leftClip = x;
	int rightClip = x + width;
	int topClip = y;
	int bottomClip = y + height;

	// Draw all existing entites.
	QValueList<cEntity*>::const_iterator it;
	for (it = entities.begin(); it != entities.end(); ++it) {
		cEntity *entity = *it;
		if (entity->isInWorld() && entity->z() < roofCap_) {
			// Calculate the difference on the x/y axis and get the offset from a 0,0 coordinate
			diffx = entity->x() - x_;
			diffy = entity->y() - y_;
			diffz = entity->z() - z_;

			cellx = centerx + (diffx - diffy) * 22;
			celly = centery + (diffx + diffy) * 22 - diffz * 4;

			entity->draw(cellx, celly, leftClip, topClip, rightClip, bottomClip);
		}
	}

	// Get the entity below the mouse
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	// If there is a Window below the mouse, don't get a mouse over tile
	if (Gui->getControl(mx, my) == WorldView) {
		mouseOver_ = getEntity(mx, my);
	} else {
		mouseOver_ = 0;
	}

	// Disable scissor box
	glDisable(GL_SCISSOR_TEST);
}

cEntity *cWorld::getEntity(int x, int y) {
	cEntity *found = 0; // The entity that was clicked on

	// Get the entity at the given position
	if (!entities.isEmpty()) {
		Iterator it = entities.end();
		do {
			--it;
			cEntity *entity = *it; // Constant pointer to non const object
	
			if (!entity->isInWorld()) {
				continue;
			}
	
			// First check the bounding box of the entity
			if (entity->hitTest(x - entity->drawx(), y - entity->drawy())) {
					found = entity;
					break;
			}
		} while (it != entities.begin());
	}

	return found;
}

void cWorld::onClick(int x, int y, unsigned char button) {
	cEntity *found = getEntity(x, y);

	// Pass the event on to the entity
	if (found) {
		if (button == SDL_BUTTON_LEFT) {
			found->onClick();
		} else if (button == SDL_BUTTON_RIGHT) {
			found->onRightClick();
		}
	}
}

void cWorld::getGroundInfo(int x, int y, stGroundInfo *info) {
	// calculate a unique cache id for the ground id
	int cacheid = ((x << 16) & 0xFFFF0000) | (y & 0xFFFF);

	// Try to find the info in the cache
	stGroundInfo *ground = groundCache.find(cacheid);

	// Add in a radius of 5 tiles around the given center
	if (!ground) {
		ground = new stGroundInfo; // Create a new ground info item for the cache

		// Get the z offsets of the surrounding tiles (actually only three of the 
		// surrounding tiles are required)
		stMapCell *groundCell = Maps->getMapCell(facet_, x, y);
		int cell = groundCell->z;
		int left = Maps->getMapCell(facet_, x, y + 1)->z;
		int bottom = Maps->getMapCell(facet_, x + 1, y + 1)->z;
		int right = Maps->getMapCell(facet_, x + 1, y)->z;

		ground->z = cell; // Save the z offset for this tile

		// Water needs a special check
		cLandTileInfo *landInfo = Tiledata->getLandInfo(groundCell->id);
		
		// Water never gets stretched
		if (landInfo->isWet()) {
			ground->left = 0;
			ground->right = 0;
			ground->bottom = 0;
		} else {
			ground->left = - ((left - cell) << 2); // * 4
			ground->right = - ((right - cell) << 2); // * 4
			ground->bottom = - ((bottom - cell) << 2); // * 4
		}

		cVector u, v;
		if (ground->left != 0 || ground->right != 0 || ground->bottom != 0) {
			// Top Corner
			u.set(-22.0f, 22.0f, (cell - right) * 4);
			v.set(-22.0f, -22.0f, (left - cell) * 4);
			ground->normals[0] = (u * v).normalize();

			// Right Corner
			u.set(22.0f, 22.0f, (right - bottom) * 4);
			v.set(-22.0f, 22.0f, (cell - right) * 4);
			ground->normals[1] = (u * v).normalize();

			// Bottom Corner
			u.set(22.0f, -22.0f, (bottom - left) * 4);
			v.set(22.0f, 22.0f, (right - bottom) * 4);
			ground->normals[2] = (u * v).normalize();

			// Left Corner
			u.set(-22.0f, -22.0f, (left - cell) * 4);
			v.set(22.0f, -22.0f, (bottom - left) * 4);
			ground->normals[3] = (u * v).normalize();
		} else {
			ground->normals[0].set(0, 0, 1);
			ground->normals[1].set(0, 0, 1);
			ground->normals[2].set(0, 0, 1);
			ground->normals[3].set(0, 0, 1);
		}

		// Copy the ground info into the outgoing variable
		memcpy(info, ground, sizeof(stGroundInfo));

		// Try to insert it into the cache
		if (!groundCache.insert(cacheid, ground)) {
			delete ground; // Free the memory if the cache is full
		}
	} else {
		memcpy(info, ground, sizeof(stGroundInfo));
	}
}

void cWorld::clearGroundCache() {
	groundCache.clear();
}

cWorld *World = 0;