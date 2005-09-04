
#include "client.h"
#include <QCursor>
#include <QMouseEvent>
#include "game/world.h"
#include "game/groundtile.h"
#include "game/statictile.h"
#include "game/mobile.h"
#include "game/dynamicitem.h"
#include "game/targetrequest.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "gui/gui.h"
#include "gui/worldview.h"
#include "muls/maps.h"
#include "sound.h"
#include "mainwindow.h"
#include <qgl.h>

cWorld::cWorld() : groundCache(2500) {
	x_ = 0;
	y_ = 0;
	z_ = 0;
	facet_ = TRAMMEL;
	roofCap_ = 1024;
	smoothMoveEnd_ = 0;
	drawxoffset = 0;
	drawyoffset = 0;
	cleaningUp = false;
	roofTimer.setSingleShot(true);
	connect(&roofTimer, SIGNAL(timeout()), this, SLOT(checkRoofs()));	
	belowMap_ = false;
	mouseOver_ = 0;
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

		// The player has to change facets too
		if (Player) {
			Player->setFacet(facet);
		}
	}
}

void cWorld::clearEntities() {
	cleaningUp = true;

	// Clear entities.
	Iterator it;
	for (it = entities.begin(); it != entities.end(); ++it) {
		Cell cell = it.value(); // Copy
		ConstCellIterator cit = cell.begin();
		while (cit != cell.end()) {
			if ((*cit) != Player) {
				Gui->removeOverheadText(*cit);
				(*cit)->setCellid(-1);
				(*cit)->decref();
			}
			++cit;
		}
	}

	cleaningUp = false;

	entities.clear();
}

void cWorld::cleanupEntities() {
	cleaningUp = true;

	QVector<uint> toremove; // Cells that are now empty

	for (Iterator it = entities.begin(); it != entities.end(); ++it) {
		ushort x, y;
		parseCellId(it.key(), x, y);
		int distance = Utilities::distance(x, y, x_, y_);

		if (distance > 18) {
			const Cell &cell = it.value();
			Cell::const_iterator cit = cell.begin();
			for (cit = cell.begin(); cit != cell.end(); ++cit) {
				if (distance > 18) {
					Gui->removeOverheadText(*cit);
					(*cit)->setCellid(-1);
					(*cit)->decref();
				}
			}
			toremove.append(it.key());
		}
	}

	// Remove empty cells
	for (int i = 0; i < toremove.size(); ++i) {
		entities.remove(toremove[i]);
	}

	cleaningUp = false;
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
			ground->setSortz(qMin<int>(leftz, rightz) + abs(leftz - rightz) / 2);
		} else {
			ground->setAveragez((leftz + rightz) / 2);
			ground->setSortz(qMin<int>(cell->z, bottomz) + abs(cell->z - bottomz) / 2);
		}

		ground->setId(cell->id);
		addEntity(ground);
	}

	// Iterate over the statics and add them
	int prioritySolver = 0;
	StaticBlock *block = Maps->getStaticBlock(facet_, x, y);
	if (block) {
		StaticBlock::iterator it;
		for (it = block->begin(); it != block->end(); ++it) {
			if (it->xoffset == x % 8 && it->yoffset == y % 8) {
				cStaticTile *tile = new cStaticTile(x, y, it->z, facet_);
				tile->setPrioritySolver(prioritySolver++);
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
	unsigned short left = qMax<int>(0, (short)x - 18);
	unsigned short top = qMax<int>(0, (short)y - 18);
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

	checkRoofs();
}

// Checks if the given entity should be sorted before the
// second entity
inline bool insertBefore(cEntity *entity, cEntity *next) {
	// Do in-cell sorting
	int priority = entity->priority() - next->priority();

	if (entity->priority() != next->priority()) {
		return entity->priority() < next->priority();
	}

	// If the priority is the same we have sorting based on tile type
	if (next->type() == MOBILE && entity->type() != MOBILE) {
		return true;
	}
	if (next->type() == ITEM && entity->type() != MOBILE && entity->type() != ITEM) {
		return true;
	}
	if (next->type() == STATIC && entity->type() == GROUND) {
		return true;
	}
	if (entity->priorityBonus() != next->priorityBonus()) {
		return entity->priorityBonus() < next->priorityBonus();
	}

	return entity->prioritySolver() < next->prioritySolver();
}

void cWorld::addEntity(cEntity *entity) {
	if (cleaningUp) {
		return;
	}

	// Always remove the entity from it's current cell before re-insertion
	if (entity->cellid() != -1) {
		Iterator it = entities.find(entity->cellid());
		if (it != entities.end()) {
			Cell &cell = it.value();
			cell.removeAll(entity);
			if (cell.isEmpty()) {
				entities.erase(it); // Clear the cell from the qmap if it's empty now
			}
		}
		entity->setCellid(-1); // Reset to not-in-world
	}

	if (entity->facet() != facet_) {
		return; // Don't add this entity, it's not on the same facet
	}

	// Calculate the cell id for the entity
	unsigned int cellid = getCellId(entity->x(), entity->y());
	
	cMobile *mobile = dynamic_cast<cMobile*>(entity);
	if (mobile && mobile->isMoving()) {
		// When we're leaving our tile to the left, we'll be overlapped
		if (mobile->direction() == 5) {
			cellid = getCellId(entity->x() + 1, entity->y() - 1);
		} else if (mobile->direction() == 6) {
			cellid = getCellId(entity->x() + 1, entity->y());
		} else if (mobile->direction() == 7) {
			cellid = getCellId(entity->x() + 1, entity->y() + 1);
		} else if (mobile->direction() == 0) {
			cellid = getCellId(entity->x(), entity->y() + 1);
		}
	}	

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
	if (cleaningUp) {
		return;
	}

	// Only remove the entity if it's in the grid.
	if (entity->cellid() != -1) {
		unsigned int cellid = (unsigned int)entity->cellid();
		Iterator it = entities.find(cellid);
		if (it != entities.end()) {
			it.value().removeAll(entity);
			if (it.value().isEmpty()) {
				entities.erase(it);
			}
		}
		entity->setCellid(-1);
	}
}

void cWorld::smoothMove(int x, int y, int z, uint duration) {
	// Cache old values
	int oldx = x_;
	int oldy = y_;
	int oldz = z_;

	// Move center of the world
	// TODO: Write an optimized loading technique for faster loading
	// and less distance checks
	moveCenter(oldx + x, oldy + y, oldz + z);

	// Get old drawing coordinates
	int diffx = oldx - x_;
	int diffy = oldy - y_;
	int diffz = oldz - z_;
	drawxoffset = (diffx - diffy) * 22;
	drawyoffset = (diffx + diffy) * 22 - diffz * 4;

	// Calculate new direction for the player
	int direction = Utilities::direction(oldx, oldy, x_, y_);

	// Move the player
	if (Player) {
		Player->smoothMove(drawxoffset, drawyoffset, duration);
		if (direction != Player->direction()) {
			Player->setDirection(direction);

			// Send the move request
			UoSocket->send(cMoveRequestPacket(direction, UoSocket->moveSequence()));
			UoSocket->pushSequence(UoSocket->moveSequence());
			if (UoSocket->moveSequence() == 255) {
				UoSocket->setMoveSequence(1);
			} else {
				UoSocket->setMoveSequence(UoSocket->moveSequence() + 1);
			}
		}
		Player->move(x_, y_, z_);
		Player->playMoveAnimation(duration, duration <= Player->getMoveDuration(true));

		// Send the move request
		UoSocket->send(cMoveRequestPacket(direction, UoSocket->moveSequence()));
		UoSocket->pushSequence(UoSocket->moveSequence());
		if (UoSocket->moveSequence() == 255) {
			UoSocket->setMoveSequence(1);
		} else {
			UoSocket->setMoveSequence(UoSocket->moveSequence() + 1);
		}
	}

	// Set the smooth move timeouts
	smoothMoveStart_ = Utilities::getTicks();
	smoothMoveEnd_ = smoothMoveStart_ + duration;

	// Start the roofcheck timer if we have a player (otherwise this is manual)
	if (Player) {
		roofTimer.setInterval(duration);
		roofTimer.setSingleShot(true);
		roofTimer.start();
	}
}

void cWorld::checkRoofs() {
	if (!Player) {
		return;
	}

	roofCap_ = 1024;
	belowMap_ = false;

	// Check for Roofs at x+1,y+1
	uint cellid = getCellId(Player->x() + 1, Player->y() + 1);
	
	if (entities.contains(cellid)) {
		cWorld::Cell cell = *entities.find(cellid);

		for (CellIterator it = cell.begin(); it != cell.end(); ++it) {
			cEntity *entity = (*it);
			if (entity->type() == STATIC || entity->type() == ITEM || entity->type() == GROUND) {
				if (entity->z() > Player->z() + 15 && entity->z() < roofCap_) {
					cStaticTile *staticTile = dynamic_cast<cStaticTile*>(*it);
					if (entity->type() == GROUND || staticTile && staticTile->tiledata()->isRoof()) {	
						if (entity->type() == GROUND) {
							belowMap_ = true;
						}
						roofCap_ = Player->z() + 15;
					}
				}
			}
		}
	}

	// Check for other items at x,y
	cellid = getCellId(Player->x(), Player->y());
	if (entities.contains(cellid)) {
		cWorld::Cell cell = *entities.find(cellid);

		for (CellIterator it = cell.begin(); it != cell.end(); ++it) {
			cStaticTile *staticTile = dynamic_cast<cStaticTile*>(*it);

			if (staticTile && staticTile->z() > Player->z() + 15) {
				if (staticTile->z() < roofCap_) {
					if (!staticTile->tiledata()->isRoof()) {
						roofCap_ = staticTile->z();
					}
				}
			}
		}
	}
}

void cWorld::draw(int x, int y, int width, int height) {
	if (Player && Player->isDead()) {
		GLWidget->enableGrayShader();
	}

	clearGroundCache(); // Clear unneeded cache items

	int diffx, diffy, diffz;
	int cellx, celly;
	int centerx = x + (width / 2);
	int centery = y + (height / 2);

	// Smooth move handling.
	if (smoothMoveEnd_ != 0) {
		uint currentTime = Utilities::getTicks();
		if (currentTime > smoothMoveEnd_) {
			smoothMoveEnd_ = 0;
			drawxoffset = 0;
			drawyoffset = 0;
			// The movement finished
		} else {
			uint duration = (smoothMoveEnd_ - smoothMoveStart_);
			double factor = (double)(smoothMoveEnd_ - currentTime) / (double)duration;

			centerx -= (int)(drawxoffset * factor);
			centery -= (int)(drawyoffset * factor);
		}
	}

	// Set Scissor Box
	glLoadIdentity();
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, GLWidget->height() - (y + height), width, height);

	// Set up the clipping variables
	int leftClip = x;
	int rightClip = x + width;
	int topClip = y;
	int bottomClip = y + height;

	// Get the entity below the mouse
	QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
	int mx = pos.x();
	int my = pos.y();

	cEntity *mouseEntity = 0;
	bool checkMouse = Gui->getControl(mx, my) == WorldView;

	// Draw all existing entites.
	for (int cx = qMax<int>(0, x_ - 18); cx <= x_ + 18; ++cx) {
		for (int cy = qMax<int>(0, y_ - 18); cy <= y_ + 18; ++cy) {
			unsigned int cellid = getCellId(cx, cy);
			Iterator it = entities.find(cellid);
			if (it != entities.end()) {
				Cell cell = it.value();
				for (ConstCellIterator cit = cell.begin(); cit != cell.end(); ++cit) {
					cEntity *entity = *cit;
					
					if (entity->z() >= roofCap_ && (belowMap_ || entity->type() != GROUND)) {
						continue;
					}

					if (entity->isInWorld()) {
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

	if (Player && Player->isDead()) {
		GLWidget->disableGrayShader();
	}
}

cEntity *cWorld::getEntity(int x, int y) {
	cEntity *found = 0; // The entity that was clicked on

	// Check reverse
	for (int cx = x_ + 18; !found && cx >= qMax<int>(0, x_ - 18); --cx) {
		for (int cy = y_ + 18; !found && cy >= qMax<int>(0, y_ - 18); --cy) {
			unsigned int cellid = getCellId(cx, cy);
			Iterator it = entities.find(cellid);
			if (it != entities.end()) {
				Cell &cell = it.value();
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
	if (e->button() == Qt::LeftButton) {
		cEntity *found = getEntity(e->x(), e->y());
	
		// Pass the event on to the entity
		if (found) {
			found->onDoubleClick(e);
		}
	}
}

void cWorld::onClick(QMouseEvent *e) {
	cEntity *found = getEntity(e->x(), e->y());

	// Pass the event on to the entity
	if (found) {
		if (e->button() == Qt::LeftButton) {
			// Targetting tages precedence...
			if (WorldView->targetRequest()) {
				cTargetRequest *request = WorldView->targetRequest();
				if (request->isValidTarget(found)) {
					WorldView->targetResponse(found);
				} else {
					Sound->playSound(0x3e8);
				}
				return;
			}

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
	stGroundInfo *ground = groundCache.object(cacheid);

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

		ground->splitLeftRight = abs(right - left) > abs(cell - bottom);
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

cDynamicItem *cWorld::findItem(unsigned int serial) const {
	cDynamicEntity *entity = findDynamic(serial);

	if (entity && entity->type() == ITEM) {
		return (cDynamicItem*)entity;
	} else {
		return 0;
	}
}

cMobile *cWorld::findMobile(unsigned int serial) const {
	cDynamicEntity *entity = findDynamic(serial);

	if (entity && entity->type() == MOBILE) {
		return (cMobile*)entity;
	} else {
		return 0;
	}
}

void cWorld::clearGroundCache() {
	groundCache.clear();
}

// The highest items will be @ the beginning
// While walking we always will try the highest first.
QVector<cBlockItem> cWorld::getBlockingItems(cMobile *mobile, ushort posx, ushort posy ) {	
	QVector<cBlockItem> blockList;

	// Process the map at that position
	cBlockItem mapBlock;
	mapBlock.maptile = true;
	mapBlock.z = Maps->getAverageHeight(mobile->facet(), posx, posy);
	mapBlock.height = 0;

	stMapCell *mapCell = Maps->getMapCell(mobile->facet(), posx, posy);
	cLandTileInfo *mapTile = Tiledata->getLandInfo(mapCell->id);

	// If it's not impassable it's automatically walkable
	if (!mapTile->isImpassable())
		mapBlock.walkable = true;
	else
		mapBlock.walkable = false; // TODO: Additional Checks for Seahorses etc.

	// Nodraw tiles don't count
	if (mapCell->id != 0x02) {
		blockList.append(mapBlock);
	}

	// Now for the static-items
	StaticBlock *block = Maps->getStaticBlock(mobile->facet(), posx, posy);
	
	const uchar cellx = posx % 8;
	const uchar celly = posy % 8;
	
	if (block) {
		for (StaticBlock::const_iterator cit = block->begin(); cit != block->end(); ++cit) {
			// Continue if it's not the tile we want
			if (cit->xoffset != cellx || cit->yoffset != celly)
				continue;

			cItemTileInfo *tTile = Tiledata->getItemInfo(cit->id);

			// Here is decided if the tile is needed
			// It's uninteresting if it's NOT blocking
			// And NOT a bridge/surface
			if ( !tTile->isBridge() && !tTile->isImpassable() && !tTile->isSurface() )
				continue;

			cBlockItem staticBlock;
			staticBlock.z = cit->z;

			// If we are a surface we can always walk here, otherwise check if
			// we are special
			if ( tTile->isSurface() && !tTile->isImpassable() )
				staticBlock.walkable = true;
			else
				staticBlock.walkable = false; // TODO: special walking checks

			// If we are a stair only the half height counts (round up)
			if ( tTile->isBridge() )
				staticBlock.height = (uchar)( ( tTile->height() ) / 2 );
			else
				staticBlock.height = tTile->height();

			blockList.append(staticBlock);
		}
	}

	// We are only interested in items at pos
	// todo: we could impliment blocking for items on the adjacent sides
	// during a diagonal move here, but this has yet to be decided.
	uint cellid = getCellId(posx, posy);
	Container::iterator it = entities.find(cellid);

	if (it != entities.end()) {
		for (CellIterator cit = it->begin(); cit != it->end(); ++cit) {
			/*if ( pChar && pChar->isDead() )
			{
				// Doors can be passed by ghosts
				if ( pItem->hasScript( "door" ) )
				{
					continue;
				}
			}*/
			cDynamicItem *item = dynamic_cast<cDynamicItem*>(*cit);

			if (!item) {
				continue;
			}		

			const cItemTileInfo *tTile = item->tiledata();

			// See above for what the flags mean
			if ( !tTile->isBridge() && !tTile->isImpassable() && !tTile->isSurface() )
				continue;

			cBlockItem blockItem;
			blockItem.height = ( tTile->isBridge() ) ? ( tTile->height() / 2 ) : tTile->height();
			blockItem.z = item->z();

			// Once again: see above for a description of this part
			if ( tTile->isSurface() && !tTile->isImpassable() )
				blockItem.walkable = true;
			else
				blockItem.walkable = false; // TODO: special walking checks

			blockList.append(blockItem);
		}
	}


	// deal with the multis now, or not.
	// 18 has been tested with castle sides and corners...
	/*MapMultisIterator iter = MapObjects::instance()->listMultisInCircle( pos, 18 );
	for ( cMulti*pMulti = iter.first(); pMulti; pMulti = iter.next() )
	{
		MultiDefinition* def = MultiCache::instance()->getMulti( pMulti->id() - 0x4000 );
		if ( !def )
			continue;

		QValueVector<multiItem_st> multi = def->getEntries();

		for ( unsigned int j = 0; j < multi.size(); ++j )
		{
			if ( multi[j].visible && ( pMulti->pos().x + multi[j].x == pos.x ) && ( pMulti->pos().y + multi[j].y == pos.y ) )
			{
				const tile_st &tTile = TileCache::instance()->getTile( multi[j].tile );
				if ( !( ( tTile.flag2 & 0x02 ) || ( tTile.flag1 & 0x40 ) || ( tTile.flag2 & 0x04 ) ) )
					continue;

				cBlockItem blockItem;
				blockItem.height = ( tTile.flag2 & 0x04 ) ? ( tTile.height / 2 ) : tTile.height;
				blockItem.z = pMulti->pos().z + multi[j].z;

				if ( ( tTile.flag2 & 0x02 ) && !( tTile.flag1 & 0x40 ) )
					blockItem.walkable = true;
				else
					blockItem.walkable = checkWalkable( pChar, pMulti->id() );

				blockList.push_back( blockItem );
				push_heap( blockList.begin(), blockList.end(), compareTiles() );
			}
		}
		continue;
	}*/

	qSort(blockList.begin(), blockList.end());

	return blockList;
};

#define P_M_MAX_Z_CLIMB		14
#define P_M_MAX_Z_FALL		20
#define P_M_MAX_Z_BLOCKS	14

bool cWorld::mayWalk(cMobile *mobile, ushort posx, ushort posy, signed char &posz) {
	// Go trough the array top-to-bottom and check
	// If we find a tile to walk on
	QVector<cBlockItem> blockList = getBlockingItems(mobile, posx, posy);
	bool found = false;
	uint i;
	bool priviledged = false;
	int oldz = posz;

	priviledged = false; // *cough cough*

	for ( i = 0; i < blockList.size(); ++i )
	{
		cBlockItem item = blockList[i];
		int itemTop = ( item.z + item.height );

		// If we found something to step on and the next tile
		// below would block us, use the good one instead
		if ( found && ( itemTop > posz - P_M_MAX_Z_BLOCKS || !item.walkable ) )
		{
			break;
		}

		// If we encounter any object with itemTop <= posz which is NOT walkable
		// Then we can as well just return false as while falling we would be
		// blocked by that object
		if ( !item.walkable && !priviledged && itemTop <= posz )
			return false;

		if ( item.walkable || priviledged )
		{
			// If the top of the item is within our max-climb reach
			// then the first check passed. in addition we need to
			// check if the "bottom" of the item is reachable
			// I would say 2 is a good "reach" value for the bottom
			// of any item
			if ( itemTop < posz + P_M_MAX_Z_CLIMB && itemTop >= posz - P_M_MAX_Z_FALL )
			{
				// We already found something to step on above.
				// See if it's easier to step down.
				if ( found && abs( oldz - posz ) < abs( oldz - itemTop ) )
				{
					break;
				}

				posz = itemTop;
				found = true;

				if ( item.height > 1 )
				{
					break;
				}

				// break; - We can't break here anymore since we have to check if the ground would be easier
				// to step on
				// Climbing maptiles is 5 tiles easier
			}
			else if ( item.maptile && itemTop < posz + P_M_MAX_Z_CLIMB + 5 && itemTop >= posz - P_M_MAX_Z_FALL )
			{
				// We already found something to step on above.
				// See if it's easier to step down.
				if ( found && abs( oldz - posz ) < abs( oldz - itemTop ) )
				{
					break;
				}

				posz = itemTop;
				found = true;
				//break;
			}
			else if ( itemTop < posz )
			{
				// We already found something to step on above.
				// See if it's easier to step down.
				if ( found && abs( oldz - posz ) < abs( oldz - itemTop ) )
				{
					break;
				}

				posz = itemTop;
				found = true;
				break;
			}
		}
	}

	if ( priviledged )
	{
		return true;
	}

	// If we're still at the same position
	// We didn't find anything to step on
	if ( !found )
		return false;

	// Another loop *IS* needed here (at least that's what i think)
	for ( i = 0; i < blockList.size(); ++i )
	{
		// So we know about the new Z position we are moving to
		// Lets check if there is enough space ABOVE that position (at least 15 z units)
		// If there is ANY impassable object between posz and posz + 15 we can't walk here
		cBlockItem item = blockList[i];
		signed char itemTop = ( item.z + item.height );

		// If the item is below what we step on, ignore it
		if ( itemTop <= posz )
		{
			continue;
		}

		// Does the top of the item looms into our space
		// Like before 15 is the assumed height of ourself
		// Use the new position here.
		if ( ( itemTop > posz ) && ( itemTop < posz + P_M_MAX_Z_BLOCKS ) )
			return false;

		// Or the bottom ?
		// note: the following test was commented out.  by putting the code back in,
		// npcs stop wandering through the walls of multis.  I am curious if this code
		// has other (negative) affects besides that.
		if ( ( item.z >= oldz ) && ( item.z < oldz + P_M_MAX_Z_BLOCKS / 2 ) )
			return false;

		// Or does it spread the whole range ?
		if ( ( item.z <= oldz ) && ( itemTop >= oldz + P_M_MAX_Z_BLOCKS / 2 ) )
			return false;

		// Is it at the new position ?
		if ( ( item.z >= posz ) && ( item.z < posz + P_M_MAX_Z_BLOCKS ) )
			return false;
	}

	// All Checks passed
	return true;
}

cWorld *World = 0;
