
#include "config.h"
#include "game/mobile.h"
#include "game/world.h"
#include "game/dynamicitem.h"
#include "muls/tiledata.h"
#include "mainwindow.h"
#include "gui/worldview.h"
#include "network/outgoingpackets.h"
#include "network/uosocket.h"
#include "log.h"

// Draw order for layers dependant on direction facing
// -1 terminates
static const int drawOrder[8][25] = {
	{5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 20, 21, -1},
	{5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 20, 21, -1},
	{5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 20, 21, -1},
	{20, 5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 21, -1},
	{20, 5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 21, -1},
	{20, 5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 21, -1},
	{5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 20, 21, -1},
	{5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 20, 21, -1}
};

cMobile::cMobile(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial) : cEntity(x, y, z, facet), cDynamicEntity(x, y, z, facet, serial) {
	body_ = 1;
	hue_ = 0;
	direction_ = 0;
	partialHue_ = false;
	dead = false;
	warmode = false;
	type_ = MOBILE;
	sequence_ = 0;
	currentAction_ = getIdleAction();
	currentActionEnd_ = 0;
	nextFrame = 0;
	frame = 0;
	nextMountFrame = 0;
	mountFrame = 0;
	smoothMoveEnd = 0;
	hidden = false;
	deleting = false;

	for (int i = 0; i < LAYER_COUNT; ++i) {
		equipmentSequences[i] = 0;		
		equipment[i] = 0;
	}
}

void cMobile::setSerial(unsigned int serial) {
	World->unregisterDynamic(this);
	serial_ = serial;
	World->registerDynamic(this);
}

void cMobile::smoothMove(int xoffset, int yoffset, unsigned int duration) {
	drawxoffset = xoffset;
	drawyoffset = yoffset;
	smoothMoveTime = duration;
	smoothMoveEnd = Utilities::getTicks() + duration;
}

cMobile::~cMobile() {
	deleting = true;
	freeSequence();

	if (this == Player) {
		Player = 0; // Reset player to null -> important
	}

	// Dec-reference Equipment
	for (int i = 0; i < LAYER_COUNT; ++i) {
		if (equipmentSequences[i]) {
			equipmentSequences[i]->decref();
		}
		if (equipment[i]) {
			equipment[i]->moveToLimbo();
			equipment[i]->decref();
		}
	}
	
	QVector<cDynamicItem*> equipmentCopy = invisibleEquipment;
	QVector<cDynamicItem*>::iterator it;
	for (it = equipmentCopy.begin(); it != equipmentCopy.end(); ++it) {
		(*it)->moveToLimbo();
		(*it)->decref();
	}

	Log->print(LOG_NOTICE, QString("Freeing mobile @ 0x%1, serial 0x%2.\n").arg((uint)this, 0, 16).arg(serial_, 0, 16));
}

void cMobile::removeEquipment(cDynamicItem *item) {
	if (!deleting) {
		// Dec-reference Equipment
		for (int i = 0; i < LAYER_COUNT; ++i) {
			if (equipment[i] == item) {
				equipment[i]->moveToLimbo();
				equipment[i] = 0;
				if (equipmentSequences[i]) {
					equipmentSequences[i]->decref();
					equipmentSequences[i] = 0;
				}

				if (i == LAYER_MOUNT) {
					currentAction_ = getIdleAction();
				}

				return;
			}
		}
	}
}

void cMobile::playAction(unsigned char action, unsigned int duration) {
	if (currentAction_ != action) {
		freeSequence(); // Free old sequence if the actions dont match
	}

	// Set the action and the duration
	currentAction_ = action;
	currentActionEnd_ = Utilities::getTicks() + duration;
}

void cMobile::freeSequence() {
	if (sequence_) {
		sequence_->decref();
		sequence_ = 0;
	}

	for (int i = 0; i < LAYER_VISIBLECOUNT; ++i) {
		if (equipmentSequences[i]) {
			equipmentSequences[i]->decref();
			equipmentSequences[i] = 0;
		}
	}
}

void cMobile::refreshSequence() {
	if (sequence_) {
		sequence_->decref();
	}

	sequence_ = Animations->readSequence(body_, currentAction_, direction_, hue_, partialHue_);	
	
	// Try to maintain the flow of the animation
	if (sequence_ && frame >= sequence_->frameCount()) {
		frame = 0;
		nextFrame = Utilities::getTicks() + getFrameDelay();
	}

	// Refresh for visible equipment only
	for (int i = 0; i < LAYER_VISIBLECOUNT; ++i) {
		if (equipmentSequences[i]) {
			equipmentSequences[i]->decref();
			equipmentSequences[i] = 0;
		}

		if (equipment[i]) {
			cItemTileInfo *tinfo = Tiledata->getItemInfo(equipment[i]->id());

			if (tinfo && tinfo->animation()) {
				unsigned short model = tinfo->animation();
				bool partialHue = tinfo->isPartialHue();
				equipmentSequences[i] = Animations->readSequence(model, currentAction_, direction_, equipment[i]->hue(), partialHue);
			}
		}
	}

	// Check for the mount separately
	if (equipmentSequences[LAYER_MOUNT]) {
		equipmentSequences[LAYER_MOUNT]->decref();
		equipmentSequences[LAYER_MOUNT] = 0;
	}

	if (equipment[LAYER_MOUNT]) {
		unsigned short model = Utilities::getMountBody(equipment[LAYER_MOUNT]->id());
		equipmentSequences[LAYER_MOUNT] = Animations->readSequence(model, currentMountAction(), direction_, equipment[LAYER_MOUNT]->hue(), false);

		if (equipmentSequences[LAYER_MOUNT] && mountFrame >= equipmentSequences[LAYER_MOUNT]->frameCount()) {
			mountFrame = 0;
			nextMountFrame = Utilities::getTicks() + getMountFrameDelay();
		}
	}
}

ushort cMobile::currentMountAction() const {
	// Try to translate the current action
	// We assume human bodytype for this
	if (bodyType() != HUMAN && bodyType() != EQUIPMENT) {
		return 0;
	}

	if (currentAction_ == 24) {
		return 1; // Running horse
	} else if (currentAction_ == 23) {
        return 0; // Walking horse
	} else {
		return 2; // Standing horse
	}
}

unsigned int cMobile::getFrameDelay() {
	return 100;
}

unsigned int cMobile::getMountFrameDelay() {
	return 100;
}

enBodyType cMobile::bodyType() const {
	return Animations->getBodyType(body_);
}

unsigned char cMobile::getIdleAction() {
	switch (bodyType()) {
		case ANIMAL:
			return 2; // Animal animation for standing
		case MONSTER:
			return 1; // High detail critter animation for standing		
		case HUMAN:
		case EQUIPMENT:
			if (equipment[LAYER_MOUNT]) {
				return 25;
			} else if (warmode) {				
				if (!equipment[LAYER_RIGHTHAND] && equipment[LAYER_LEFTHAND]) {
					return 8; // Two handed attack stand
				} else {
					return 7; // One handed attack stand
				}
			}
		default:
			return 4; // Human animation for standing
	}
}

void cMobile::draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip) {
	if (Config->gameHideMobiles()) {
		return;
	}

	// Test for smoother player movement
	if (this == Player) {
		cellx = WorldView->x() + WorldView->width() / 2;
		celly = WorldView->y() + WorldView->height() / 2;
	}

	// See if the current action expired
	if (currentActionEnd_ != 0 && currentActionEnd_ < Utilities::getTicks()) {
		freeSequence(); // Free current surface
		currentActionEnd_ = 0; // Reset end time
		currentAction_ = getIdleAction();
	}

	// Refresh the sequence
	if (!sequence_) {
		refreshSequence();
	}

	float alpha = 1.0f;

	// Modify cellx/celly based on the smooth move settings
	// Smooth move handling.
	if (smoothMoveEnd != 0) {
		int moveProgress = smoothMoveTime - (smoothMoveEnd - Utilities::getTicks());
		if (moveProgress < 0 || moveProgress >= (int)smoothMoveTime) {
			smoothMoveEnd = 0;
			World->removeEntity(this);
			World->addEntity(this);
		} else if (this != Player) {
			if (moveProgress <= 0) {
				cellx += drawxoffset;
				celly += drawyoffset;
			} else {
				float factor = 1.0f - (float)moveProgress / (float)smoothMoveTime;
				cellx += (int)(factor * (float)drawxoffset);
				celly += (int)(factor * (float)drawyoffset);
			}
		}
	}

	if (isHidden()) {
		GLWidget->enableGrayShader();
	}

	// Draw
	if (sequence_) {
		// Skip to next frame
		if (nextFrame < Utilities::getTicks()) {
			if (++frame >= sequence_->frameCount()) {
				frame = 0;
			}
			nextFrame = Utilities::getTicks() + getFrameDelay();
		}
		
		// The anims facing right are generated by flipping the ones facing left
		bool flip = (direction_ >= 0 && direction_ < 4);

		// Mounts come always first
		if ((bodyType() == HUMAN || bodyType() == EQUIPMENT) && equipment[LAYER_MOUNT] && equipmentSequences[LAYER_MOUNT]) {
			// Skip to next frame
			if (nextMountFrame < Utilities::getTicks()) {
				if (++mountFrame >= equipmentSequences[LAYER_MOUNT]->frameCount()) {
					mountFrame = 0;
				}
				nextMountFrame = Utilities::getTicks() + getMountFrameDelay();
			}
			
			mountFrame = frame; // Until something better is found
            equipmentSequences[LAYER_MOUNT]->draw(mountFrame, cellx, celly, flip, alpha);            
		}

		sequence_->draw(frame, cellx, celly, flip, alpha);

		// Draw the equipment
		if (bodyType() == HUMAN || bodyType() == EQUIPMENT) {
			const int *order = drawOrder[direction_ % 8];
			while (*order != -1) {
				enLayer layer = (enLayer)*order;
	
				if (layer < LAYER_VISIBLECOUNT && equipmentSequences[layer]) {
					// Oh great OSI... Another exception from the rule *sigh*
					// Don't draw Hair if we're wearing a gm robe
					if (layer != LAYER_HAIR || !equipmentSequences[LAYER_OUTERTORSO] || equipmentSequences[LAYER_OUTERTORSO]->body() != 0x3db) {					
						equipmentSequences[layer]->draw(frame, cellx, celly, flip, alpha);
					}				
				}
	
				++order; // Next layer
			}
		}
	}

	if (isHidden()) {
		GLWidget->disableGrayShader();
	}

	drawx_ = cellx;
	drawy_ = celly;
}

bool cMobile::hitTest(int x, int y) {
	if (Config->gameHideMobiles()) {
		return false;
	}

	bool flip = (direction_ >= 0 && direction_ < 4);

	// Check the sequence and all equipment (sucks like hell)
	if (sequence_ && sequence_->hitTest(frame, x, y, flip)) {
        return true;
	}

	if (equipmentSequences[LAYER_MOUNT] && equipmentSequences[LAYER_MOUNT]->hitTest(mountFrame, x, y, flip)) {
        return true;
	}

	return false;
}

void cMobile::updatePriority() {
	priorityBonus_ = 2;
	priority_ = z_ + priorityBonus_;
}

/*void cMobile::addEquipment(unsigned int serial, unsigned short id, unsigned short hue, enLayer layer) {
	stEquipInfo info;
	info.serial = serial;
	info.hue = hue;
	info.id = id;
	info.layer = layer;
	equipment.append(info);

	// Load Sequence for this
	if (equipmentSequences[layer]) {
		equipmentSequences[layer]->decref();
	}

	cItemTileInfo *tinfo = Tiledata->getItemInfo(id);

	if (tinfo && tinfo->animation()) {
		unsigned short model = tinfo->animation();
		bool partialHue = tinfo->isPartialHue();
		equipmentSequences[layer] = Animations->readSequence(model, currentAction_, direction_, hue, partialHue);
	} else {
		equipmentSequences[layer] = 0;
	}
}*/

void cMobile::addEquipment(cDynamicItem *item) {
	enLayer layer = item->layer();

	// decide whether it's visible or not
	if (layer < LAYER_VISIBLECOUNT) {
		if (equipment[layer]) {
			equipment[layer]->moveToLimbo();
			equipment[layer]->decref(); // If this happens it's a double equip...
		}
		equipment[layer] = item;

		refreshEquipment(layer);
	} else if (layer == LAYER_MOUNT) {
		if (equipment[layer]) {
			equipment[layer]->moveToLimbo();
			equipment[layer]->decref(); // If this happens it's a double equip...
		}
		equipment[layer] = item;

		refreshEquipment(layer);
		currentAction_ = getIdleAction();
	} else {
        invisibleEquipment.append(item);
	}
}

void cMobile::refreshEquipment(enLayer layer) {
	if (layer == LAYER_MOUNT) {
		if (!equipment[layer]) {
			if (equipmentSequences[layer]) {
				equipmentSequences[layer]->decref();
				equipmentSequences[layer] = 0;
			}
			return;
		}

		if (equipment[LAYER_MOUNT]) {
			cSequence *sequence = equipmentSequences[layer];
			ushort model = Utilities::getMountBody(equipment[layer]->id());
			uchar action = currentMountAction();
			bool partialHue = false;
			ushort hue = equipment[LAYER_MOUNT]->hue();
	
			if (!sequence || sequence->body() != model || sequence->action() != action || sequence->direction() != direction_ || sequence->hue() != hue || sequence->partialHue() != partialHue) {	
				if (sequence) {
					sequence->decref();
				}
				equipmentSequences[layer] = Animations->readSequence(model, action, direction_, hue, partialHue);
			} else if (sequence) {
				sequence->decref();
				equipmentSequences[layer] = 0;
			}
		}
	}

	if (layer >= LAYER_VISIBLECOUNT) {
		return;
	}

	if (!equipment[layer]) {
		if (equipmentSequences[layer]) {
			equipmentSequences[layer]->decref();
			equipmentSequences[layer] = 0;
		}
		return;
	}

	cDynamicItem *item = equipment[layer];

	// Only reload if neccesary
	cItemTileInfo *tinfo = Tiledata->getItemInfo(item->id());

	if (tinfo && tinfo->animation()) {
		unsigned short model = tinfo->animation();

		if (model < 0x400) {
			bool partialHue = tinfo->isPartialHue();
	
			cSequence *sequence = equipmentSequences[layer];
	
			if (!sequence || sequence->body() != model || sequence->action() != currentAction_ || sequence->direction() != direction_ || sequence->hue() != item->hue() || sequence->partialHue() != partialHue) {	
				if (sequence) {
				sequence->decref();
				}
				equipmentSequences[layer] = Animations->readSequence(model, currentAction_, direction_, item->hue(), partialHue);
			} else if (sequence) {
				sequence->decref();
				equipmentSequences[layer] = 0;
			}
		}
	} else {
		if (equipmentSequences[layer]) {
			equipmentSequences[layer]->decref();
		}
		equipmentSequences[layer] = 0;
	}
}

uint cMobile::getMoveDuration(bool running) const {	
	int result = 0;
	if (!running) {
		result = 370;
	} else {
		result = 370 / 2;
	}

	if (equipment[LAYER_MOUNT]) {
		result /= 2;
	}

	return result;
}

void cMobile::playMoveAnimation(uint duration, bool running) {
	uchar action = 0;

	if (bodyType() == HUMAN) {
		// Armed movement position
		if (equipment[LAYER_RIGHTHAND]) {
			action = running ? 3 : 1;
		} else if (running) {
			action = 2;
		}

		// Warmode only got a walking animation
		if (warmode && !running) {
			action = 15;
		}
	}

	// Mounted movement
	if (bodyType() == HUMAN && equipment[LAYER_MOUNT]) {
		action = running ? 24 : 23;
	}

	playAction(action, duration);
}

void cMobile::processFlags(uchar flags) {
	hidden = (flags & 0x80) != 0;
	warmode = (flags & 0x40) != 0;
}

uint cMobile::getCurrentHeight() {
	if (!sequence_) {
		return 0;
	} else {
		return abs(sequence_->getFrameTop(frame));
	}
}

void cMobile::onDoubleClick(QMouseEvent *e) {
	UoSocket->send(cDoubleClickPacket(serial_));
}

cMobile *Player = 0;