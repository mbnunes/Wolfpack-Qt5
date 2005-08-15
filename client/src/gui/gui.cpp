
#include "gui/gui.h"
#include "gui/control.h"
#include "gui/window.h"
#include "gui/label.h"
#include "gui/worldview.h"
#include "muls/gumpart.h"
#include "game/entity.h"

cGui *Gui = 0;

cGui::cGui() {
	inputFocus_ = 0;
	activeWindow_ = 0;
}

cGui::~cGui() {
}

void cGui::queueDelete(cControl *ctrl) {
	if (this != ctrl && ctrl->parent() == this) {
		deleteQueue.append(ctrl);
	}
}

void cGui::removeOverheadText(cEntity *source) {
	for (int i = 0; i < overheadText.size(); ++i) {
		if (overheadText[i].entity == source) {
			delete overheadText[i].control;
			if (overheadText[i].entity) {
				overheadText[i].entity->decref();
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
					overheadText[i].entity->decref();
				}
				overheadText.remove(i);
				continue;
			}

			// Give it 1000 msec to time out
			float alpha = 1.0f - (- diff) / 500.0f;
			overheadText[i].control->setAlpha(alpha);
		}

		int xoffset = overheadText[i].centerx  - overheadText[i].control->width() / 2;
		int yoffset = overheadText[i].centery  - overheadText[i].control->height() / 2;

		// Centerx/centery are in relation to drawx/drawy if the entity is present
		if (overheadText[i].entity) {
			xoffset += overheadText[i].entity->drawx();
			yoffset += overheadText[i].entity->drawy();
		}

		if (WorldView) {
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
}

void cGui::addOverheadText(int centerx, int centery, unsigned int timeout, QString message, unsigned short hue, unsigned char font, cEntity *source) {
	// Search for ovehead text from the same entity
	if (source) {
		for (int i = 0; i < overheadText.size(); ++i) {
			if (overheadText[i].entity == source) {
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
	info.entity->incref(); // Keep an instance of the entity with this
	cLabel *label = new cLabel(message, font, hue);
	label->update();
	info.control = label;
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

	if (!activeWindow_ && control->isContainer()) {
		activeWindow_ = (cWindow*)control;
	}
}
