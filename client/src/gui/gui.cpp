
#include "engine.h"
#include "gui/gui.h"
#include "gui/control.h"
#include "gui/window.h"
#include "muls/gumpart.h"

cGui *Gui = 0;

cGui::cGui() {
	inputFocus_ = 0;
	activeWindow_ = 0;
}

cGui::~cGui() {
}

// Redraw all controls
void cGui::draw() {
	Controls::const_iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		cWindow *window = (cWindow*)(*it);
		if (window->isVisible()) {
			// Draw the window
			window->draw(0, 0);
		}
	}
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
