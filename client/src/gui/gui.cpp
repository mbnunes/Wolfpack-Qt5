
#include "gui/gui.h"
#include "gui/control.h"
#include "gui/window.h"
#include "gui/asciilabel.h"
#include "gui/label.h"
#include "gui/worldview.h"
#include "gui/genericgump.h"
#include "gui/containergump.h"
#include "gui/tooltip.h"
#include "muls/gumpart.h"
#include "game/entity.h"
#include "game/dynamicitem.h"
#include "game/mobile.h"
#include "game/world.h"
#include "game/tooltips.h"
#include "network/uosocket.h"
#include "mainwindow.h"
#include <qcursor.h>

cGui *Gui = 0;

cGui::cGui() {
	inputFocus_ = 0;
	activeWindow_ = 0;
	cleaningUpOverheadText = false;
}

cGui::~cGui() {
}

void cGui::queueDelete(cControl *ctrl) {
	if (this != ctrl && ctrl->parent() == this) {
		deleteQueue.append(ctrl);
	}
}

void cGui::removeOverheadText(cEntity *source) {
	if (cleaningUpOverheadText) {
		return;
	}

	for (int i = 0; i < overheadText.size(); ++i) {
		if (overheadText[i].entity == source) {
			delete overheadText[i].control;
			if (overheadText[i].entity) {
				cleaningUpOverheadText = true;
				overheadText[i].entity->decref();
				cleaningUpOverheadText = false;
			}
			overheadText.remove(i);
		}
	}
}

// Redraw all controls
void cGui::draw() {
	// Clear the queue	
	for (int i = 0; i < deleteQueue.size(); ++i) {
		removeControl(deleteQueue[i]);
		delete deleteQueue[i];
	}
	deleteQueue.clear();
	
	Controls::const_iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		cWindow *window = (cWindow*)(*it);
		if (window->isVisible()) {
			// Draw the window
			window->draw(0, 0);
		}
	}

	QTime current = QTime::currentTime();

	// Draw overhead texts
	for (int i = 0; i < overheadText.size(); ++i) {
		// Check timeout and set alpha accordingly
		int diff = current.msecsTo(overheadText[i].timeout);
		
		// Start fading out
		if (diff < 0) {
			// Completely timed out
			if (diff < -500) {
				delete overheadText[i].control;
				if (overheadText[i].entity) {
					cleaningUpOverheadText = true;
					overheadText[i].entity->decref();
					cleaningUpOverheadText = false;
				}				
				overheadText.remove(i--); // In the next iteration it's the same i again
				continue;
			}

			// Give it 1000 msec to time out
			float alpha = 1.0f - (- diff) / 500.0f;
			overheadText[i].control->setAlpha(alpha);
		}

		int xoffset = overheadText[i].centerx  - overheadText[i].control->width() / 2;
		int yoffset = overheadText[i].centery  - overheadText[i].control->height() / 2;

		bool dontClip = false;

		// Centerx/centery are in relation to drawx/drawy if the entity is present
		if (overheadText[i].entity) {
			// We always want the text to be ABOVE the mobile not in the center
			if (overheadText[i].entity->type() == MOBILE) {
				yoffset = overheadText[i].centery  - overheadText[i].control->height();
			}
			xoffset += overheadText[i].entity->drawx();
			yoffset += overheadText[i].entity->drawy();

			// The overlay text for contained items may not be clipped
			cDynamicItem *item = dynamic_cast<cDynamicItem*>(overheadText[i].entity);
			if (item && item->container()) {
				dontClip = true;
            }
		}

		if (WorldView && !dontClip) {
			int x, y, width, height;
			WorldView->getWorldRect(x, y, width, height);

			if (xoffset < x) {
				xoffset = x;
			}
			if (yoffset < y) {
				yoffset = y;
			}
			if (yoffset + overheadText[i].control->height() >= y + height) {
				yoffset = y + height - overheadText[i].control->height();
			}
			if (xoffset + overheadText[i].control->width() >= x + width) {
				xoffset = x + width - overheadText[i].control->width();
			}
		}

		overheadText[i].control->draw(xoffset, yoffset);
	}

	// Always draw the tooltip last
	cEntity *mEntity = World->mouseOver();

	// Check for ContainerGump controls if neccesary
	if (!mEntity && GLWidget->lastMouseMovement()) {
		cContainerItemImage *itemImg = dynamic_cast<cContainerItemImage*>(GLWidget->lastMouseMovement());
		if (itemImg) {
			mEntity = World->findDynamic(itemImg->serial());
		}
	}

	if (mEntity && mEntity->tooltipKey() != 0) {
		if (UoSocket && UoSocket->isTooltips()) {
			Tooltip->setEntity(mEntity);
			Tooltip->setTooltip(mEntity->tooltipKey());

			// Only draw if the tooltip is known
			if (Tooltips->contains(mEntity->tooltipKey())) {			
				QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
				int xpos = pos.x() + Cursor->currentWidth() - Cursor->currentXOffset();
				int ypos = pos.y();
				Tooltip->draw(xpos, ypos);
			}
		}
	}
}

void cGui::addItemNameText(int centerx, int centery, unsigned int timeout, QString message, unsigned short hue, unsigned char font, cEntity *source, bool ascii) {
	// Search for ovehead text from the same entity
	if (source) {
		for (int i = 0; i < overheadText.size(); ++i) {
			if (overheadText[i].entity == source && overheadText[i].itemName) {
				delete overheadText[i].control;
				if (overheadText[i].entity) {
					overheadText[i].entity->decref();
				}
				overheadText.remove(i);
				break;
			}
		}
	}

	cOverheadInfo info;
	info.timeout = QTime::currentTime().addMSecs(timeout);	
	info.centerx = centerx;
	info.centery = centery;
	info.entity = source;
	info.itemName = true;
	info.entity->incref(); // Keep an instance of the entity with this
	if (!ascii) {
		cLabel *label = new cLabel(message, font, hue);
		label->update();
		info.control = label;
	} else {
		cAsciiLabel *label = new cAsciiLabel(message, font, hue, ALIGN_LEFT, true, true);
		label->update();
		info.control = label;
	}
	overheadText.append(info);
}

void cGui::addOverheadText(int centerx, int centery, unsigned int timeout, QString message, unsigned short hue, unsigned char font, cEntity *source, bool ascii) {
	// Get current height of mobile
	if (!source) {
		return;
	}

	// Recalculate the centery value for mobiles
	cMobile *mobile = dynamic_cast<cMobile*>(source);
	if (mobile) {
		centery = - mobile->getCurrentHeight();
	}

	cOverheadInfo info;
	info.timeout = QTime::currentTime().addMSecs(timeout);	
	info.centerx = centerx;
	info.centery = centery;
	info.entity = source;
	info.itemName = false;
	info.entity->incref(); // Keep an instance of the entity with this

	if (!ascii) {
		cLabel *label;	
		if (source && source->type() == MOBILE) {
			label = new cLabel(message, font, hue, true, ALIGN_LEFT, false);
			label->setWidth(250);
		} else {
			label = new cLabel(message, font, hue);
		}
		label->update();
		label->setAutoSize(true); // This makes sure we have the right width/height
		label->setAutoSize(false);
		info.control = label;
	} else {
		cAsciiLabel *label;	
		if (source && source->type() == MOBILE) {
			label = new cAsciiLabel(message, font, hue, ALIGN_LEFT, false, true);
			label->setWidth(250);
		} else {
			label = new cAsciiLabel(message, font, hue, ALIGN_LEFT, true, true);
		}
		label->update();
		label->setAutoSize(true); // This makes sure we have the right width/height
		label->setAutoSize(false);
		info.control = label;
	}

	// Search trough overhead text for the entity and move it upwards. And increase 
	// our timeout value by the highest timeout we find
	QDateTime maxTimeout;
	for (int i = 0; i < overheadText.size(); ++i) {
		if (overheadText[i].entity == source && !overheadText[i].itemName) {
			overheadText[i].centery -= info.control->height();
			break;
		}
	}

	overheadText.append(info);
}

void cGui::invalidate() {
	// Gui cannot be invalidated (doesnt make sense)
}

void cGui::setInputFocus(cControl *focus) {
	// If there already is a focused control and that control 
	// differs from the new one, issue an onBlur() event.
	if (inputFocus_ && inputFocus_ != focus) {
		inputFocus_->onBlur(focus);
	}

	cControl *oldFocus = inputFocus_;
	inputFocus_ = focus;
	
	// Issue a onFocus event if the new control exists and gets the
	// input focus.
	if (inputFocus_) {
		inputFocus_->onFocus(oldFocus);
	}
}

void cGui::onDeleteControl(cControl *control) {
	if (inputFocus_ == control) {
		inputFocus_ = 0;
	}

	if (activeWindow_ == control) {
		activeWindow_ = 0;
	}
}

void cGui::addControl(cControl *control, bool back) {
	// Only Windows can be added to this control
	if (!control->isWindow()) {
		throw Exception(tr("Trying to add a control to the GUI which is not a top level window."));
	}

	cContainer::addControl(control, back);

	if (!activeWindow_) {
		activeWindow_ = (cWindow*)control;
	}

	QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
	
	cControl *motionControl = Gui->getControl(pos.x(), pos.y());
	if (motionControl != GLWidget->lastMouseMovement()) {
		if (GLWidget->lastMouseMovement()) {
			GLWidget->lastMouseMovement()->onMouseLeave();
		}
		GLWidget->setLastMouseMovement(motionControl);
		if (GLWidget->lastMouseMovement()) {
			GLWidget->lastMouseMovement()->onMouseEnter();
		}
	}
}

void cGui::closeAllGumps() {
	foreach(cControl *control, controls) {
		// Check for several types that should be closed when disconnected
		cGenericGump *generic = dynamic_cast<cGenericGump*>(control);
		if (generic) {
			queueDelete(generic);
			continue;
		}
		cContainerGump *gump = dynamic_cast<cContainerGump*>(control);
		if (gump) {
			queueDelete(gump);
			continue;
		}
	}
}

void cGui::setActiveWindow(cWindow *data) {
	// The WorldView is not affected by this
	if (data && data != WorldView) {
		for (int i = 0; i < controls.size(); ++i) {
			if (controls[i] == data) {
				controls.remove(i);
				break;
			}
		}
	
		controls.append(data);
	}

	activeWindow_ = data;
}
