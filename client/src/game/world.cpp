
#include "uoclient.h"
#include <qcursor.h>
#include "game/world.h"
#include "game/groundtile.h"
#include "game/statictile.h"
#include "game/mobile.h"
#include "gui/gui.h"
#include "gui/worldview.h"
#include "muls/maps.h"
#include <qgl.h>

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
		Cell &cell = it.data();
		ConstCellIterator cit = cell.begin();
		while (cit != cell.end()) {
			(*cit)->decref();
			++cit;
		}		
	}

	entities.clear();
}

void cWorld::cleanupEntities() {
	QValueVector<unsigned int> toremove; // Cells that are now empty

	for (Iterator it = entities.begin(); it != entities.end(); ++it) {
		Cell &cell = it.data();
		CellIterator cit = cell.begin();		
		while (cit != cell.end()) {
			int distance = Utilities::distance((*cit)->x(), (*cit)->y(), x_, y_);			
			if (distance > 18) {
				(*cit)->decref();
				cit = cell.remove(cit);
			}
			++cit;
		}
		
		if (cell.isEmpty()) {
			toremove.append(it.key());
		}
	}

	// Remove empty cells
	for (QValueVector<unsigned int>::const_iterator it = toremove.begin(); it != toremove.end(); ++it) {
		entities.remove(*it);
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
	// the x,y axis checks are no longer neccesary since it's an in-cell check now
	/*// y-axis is the strongest sorter
	if (entity->y() != next->y()) {
		return entity->y() < next->y();
	} else if (entity->x() != next->x()) {
        return entity->x() < next->x();
	
	} else {*/
	// Do in-cell sorting
	int priority = entity->priority() - next->priority();

	if (priority > 0) {
		return false; // Our priority is higher than the next tile
	} else if (priority < 0) {
		return true; // Our priority is smaller, so sort before us.
	} else {
		return false; // If the priority is the same, tiles that come later, draw later
	}
	//}
}

void cWorld::addEntity(cEntity *entity) {
	// Always remove the entity from it's current cell before re-insertion
	if (entity->cellid() != -1) {
		Iterator it = entities.find(entity->cellid());
		if (it != entities.end()) {
			Cell &cell = it.data();
			cell.remove(entity);
			if (cell.isEmpty()) {
				entities.remove(it); // Clear the cell from the qmap if it's empty now
			}
		}
		entity->setCellid(-1); // Reset to not-in-world
	}

	if (entity->facet() != facet_) {
		return; // Don't add this entity, it's not on the same facet
	}

	// Calculate the cell id for the entity
	unsigned int cellid = getCellId(entity->x(), entity->y());
	entity->setCellid(cellid); // Save the current cellid with the entity

	// Update the tile priority of the new entity first
	entity->updatePriority();
	
	Cell &cell = entities[cellid];
	CellIterator it(cell.begin());
	// Search for an appropiate place in the list
	for (; it != cell.end(); ++it) {
		// See if the entity should be inserted before our current element
		if (insertBefore(entity, *it)) {
			cell.insert(it, entity); // Insert it before this entity
			return;
		}
	}

	cell.append(entity); // Simply append now
}

void cWorld::removeEntity(cEntity *entity) {
	// Only remove the entity if it's in the grid.
	if (entity->cellid() != -1) {
		unsigned int cellid = (unsigned int)entity->cellid();
		Iterator it = entities.find(cellid);
		if (it != entities.end()) {
			it.data().remove(entity);
			if (it.data().isEmpty()) {
				entities.remove(it);
			}
		}
	}
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
	smoothMoveEnd_ = Utilities::getTicks() + smoothMoveTime_;
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
		int moveProgress = smoothMoveTime_ - (smoothMoveEnd_ - Utilities::getTicks());
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
				//if (nextSmoothMoveUpdate < Utilities::getTicks()) {
					currentSmoothMoveFactor = 1.0f - (float)moveProgress / (float)smoothMoveTime_;
					//nextSmoothMoveUpdate = Utilities::getTicks() + (smoothMoveTime_;
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
	glScissor(x, App->mainWidget()->height() - (y + height), width, height);

	// Set up the clipping variables
	int leftClip = x;
	int rightClip = x + width;
	int topClip = y;
	int bottomClip = y + height;

	// Get the entity below the mouse
	QPoint pos = App->mainWidget()->mapFromGlobal(QCursor::pos());
	int mx = pos.x();
	int my = pos.y();

	cEntity *mouseEntity = 0;
	bool checkMouse = Gui->getControl(mx, my) == WorldView;

	// Draw all existing entites.
	for (int cx = QMAX(0, x_ - 18); cx <= x_ + 18; ++cx) {
		for (int cy = QMAX(0, y_ - 18); cy <= y_ + 18; ++cy) {
			unsigned int cellid = getCellId(cx, cy);
			Iterator it = entities.find(cellid);
			if (it != entities.end()) {
				Cell &cell = it.data();				
				for (ConstCellIterator cit = cell.begin(); cit != cell.end(); ++cit) {
					cEntity *entity = *cit;
					if (entity->isInWorld() && entity->z() < roofCap_) {
						// Calculate the difference on the x/y axis and get the offset from a 0,0 coordinate
						diffx = entity->x() - x_;
						diffy = entity->y() - y_;
						diffz = entity->z() - z_;

						cellx = centerx + (diffx - diffy) * 22;
						celly = centery + (diffx + diffy) * 22 - diffz * 4;

						entity->draw(cellx, celly, leftClip, topClip, rightClip, bottomClip);

						// Do a quick hit-test here with the mouse coordinate to speed up hit testing a lot
						// Possible optimization -> Do an inlined bounding box check here to remove the
						// performance hit by calling so many functions.
						if (checkMouse && entity->hitTest(mx - entity->drawx(), my - entity->drawy())) {
							mouseEntity = entity;
						}
					}
				}
			}
		}
	}
	
	// Save the mouse over entity
	mouseOver_ = mouseEntity;

	// Disable scissor box
	glDisable(GL_SCISSOR_TEST);
}

cEntity *cWorld::getEntity(int x, int y) {
	cEntity *found = 0; // The entity that was clicked on

	// Check reverse
	for (int cx = x_ + 18; !found && cx >= QMAX(0, x_ - 18); --cx) {
		for (int cy = y_ + 18; !found && cy >= QMAX(0, y_ - 18); --cy) {
			unsigned int cellid = getCellId(cx, cy);
			Iterator it = entities.find(cellid);
			if (it != entities.end()) {
				Cell &cell = it.data();
				if (!cell.isEmpty()) {
					ConstCellIterator cit = cell.end();
					do {
						--cit;
						cEntity *entity = *cit;
						if (entity->isInWorld() && entity->z() < roofCap_) {
							if (entity->hitTest(x - entity->drawx(), y - entity->drawy())) {
								found = entity;
								break;
							}
						}
					} while (cit != cell.begin());
				}
			}
		}
	}
	/*if (!entities.isEmpty()) {
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
	}*/

	return found;
}

void cWorld::onDoubleClick(QMouseEvent *e) {
}

void cWorld::onClick(QMouseEvent *e) {
	cEntity *found = getEntity(e->x(), e->y());

	// Pass the event on to the entity
	if (found) {
		if (e->button() == Qt::LeftButton) {
			found->onClick(e);
		} else if (e->button() == Qt::RightButton) {
			found->onRightClick(e);
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
