
#include <qgl.h>

#include "mainwindow.h"
#include "exceptions.h"
#include "utilities.h"
#include "gui/container.h"

void cContainer::draw(int xoffset, int yoffset) {
	xoffset += x_;
	yoffset += y_;

	if (!disableScissorBox_) {
		// Save the old scissor test flag
		glPushAttrib(GL_SCISSOR_BIT|GL_SCISSOR_BIT);
	
		// And enable the scissor box for this container
		glEnable(GL_SCISSOR_TEST);
		
		// Set the scissor box to the coordinates of this container
		glScissor(xoffset, GLWidget->height() - (yoffset + height_), width_, height_);
	}

	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->isVisible()) {
			(*it)->draw(xoffset, yoffset);
		}
	}

	if (!disableScissorBox_) {
		// Pop the old scissor box/test attribs from the gl stack
		glPopAttrib();
	}
}

cContainer::cContainer() {
	disableScissorBox_ = false;
}

void cContainer::clear() {
	foreach( cControl* control, controls )
	{
		delete control;
	}
	controls.clear();
}

cContainer::~cContainer() {
	clear();
}

void cContainer::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (width_ != oldwidth || height_ != oldheight) {
		// Notify the child controls that we have resized
		for (Iterator it = controls.begin(); it != controls.end(); ++it) {
			(*it)->onParentResized(oldwidth, oldheight);
		}

		alignControl(0);
	}

	cControl::onChangeBounds(oldx, oldy, oldwidth, oldheight);
}

/*
		for (Iterator it = controls.begin(); it != controls.end(); ++it) {
			if (*it == control) {
				controls.erase(it);
				break;
			}
		}

		if (control->parent_ == this) {
			control->parent_ = 0;
		}
*/

void cContainer::addControl(cControl *control, bool back) {
	if (control == this) {
		throw Exception(tr("Trying to add a container control to itself."));
	}

	if (control->parent()) {
		control->parent()->removeControl(control);
	}

	if (!back) {
		controls.append(control);
	} else {
        controls.insert(controls.begin(), control);
	}

	if (control->canHaveFocus() || control->isContainer()) {
		control->setTabIndex(getHighestTabIndex() + 1); // Only bother for focusable controls
		tabControls.append(control);
		sortTabControls();
	}

	control->setParent(this);
	control->requestAlign();
}

void cContainer::removeControl(cControl *control) {
	if (control->parent() != this) {
		return;
	}

	for (int i = 0; i < controls.size(); ++i) {
		if (controls[i] == control) {
            controls.remove(i--);
		}
	}
	for (int i = 0; i < tabControls.size(); ++i) {
		if (tabControls[i] == control) {
			tabControls.remove(i--);
		}
	}

	control->setParent(0);
}

void cContainer::alignControl(cControl *control) {
	if (isDisableAlign()) {
		return; // This control is currently realigning its children
	}

	setDisableAlign(true);
	setNeedsRealign(true);

	QRect clientRect(0, 0, width_, height_); // This rectangle will be modified by the code "below"
	
	//
	// Check if we need a realignment
	//
	if (!control) {
		bool needRealign = false;
		for (Iterator it = controls.begin(); it != controls.end(); ++it) {
			if ((*it)->align() != CA_NONE || (*it)->hasRightAnchor() || (*it)->hasBottomAnchor()) {
				needRealign = true;
				break;
			}
		}

		if (!needRealign) {
			goto realign_end; // Jump to the end of the realignment process
		}
	}

	//
	// We do need a realignment
	//
	doAlignment(CA_TOP, control, clientRect);
	doAlignment(CA_BOTTOM, control, clientRect);
	doAlignment(CA_LEFT, control, clientRect);
	doAlignment(CA_RIGHT, control, clientRect);
	doAlignment(CA_CLIENT, control, clientRect);
	//doAlignment(CA_NONE, control, clientRect);

	// End of realignment
realign_end:
	setDisableAlign(false);
	setNeedsRealign(false);
}

void cContainer::doAlignment(enControlAlign align, cControl *control, QRect &clientRect) {
	QList<cControl*> alignlist; // List of Controls to be aligned
	QList<cControl*>::iterator lit; // List iterator

	/*
		The to-be-aligned control is included in the list if its non-null and if it
		shares the alignment with the alignment constant that is currently aligned.
		Note: For all alignments other than CA_NONE, the control is only regarded if visible.
	*/
	if (control && control->align() == align && (align == CA_NONE || control->isVisible())) {
		alignlist.append(control);
	}

	/*
		Now check all children of this container if they are affected by this alignment run.
	*/
	for (Iterator it = controls.begin(); it != controls.end(); ++it) {
		/*
			Here the same rules as above apply. Only realign the iterated control if it is affected
			by the current run and if its either visible or if the current run is for anchored controls.
		*/
		if (*it != control && (*it)->align() == align && (align == CA_NONE || (*it)->isVisible())) {
			/*
				At this point, we make sure, that the list of controls is sorted.
				The first control in the list comes first when we do the positioning work.
			*/
			lit = alignlist.begin();
			while (lit != alignlist.end() && !canAlignBefore(align, *it, *lit)) {
				++lit;
			}
			alignlist.insert(lit, *it); // Insert the current control into the alignment list
		}
	}

	/*
		Since we now have a sorted list of controls that should be aligned, we will process
		this list from beginning to end and position the controls in that list according to the
		current alignment.
	*/
	for (lit = alignlist.begin(); lit != alignlist.end(); ++lit) {
		doPositioning(align, *lit, clientRect);
	}
}

void cContainer::doPositioning(enControlAlign align, cControl *control, QRect &clientRect) {
	int newWidth = clientRect.width(); // By default we use up the entire remaining width
	if (align == CA_LEFT || align == CA_RIGHT)
		newWidth = control->width(); // For the left/right alignment, use the control width instead

	int newHeight = clientRect.height(); // By default we use up the entire remaining height
	if (align == CA_TOP || align == CA_BOTTOM)
		newHeight = control->height(); // For the bottom/top alignment, use the control height instead

	int newX = clientRect.x(); // By default, use the upper left position of the clientrect
	int newY = clientRect.y(); // By default, use the upper left position of the clientrect

	// Modify the remaining rect based on the alignment type
	switch (align) {
		case CA_TOP:
			clientRect.setY(clientRect.y() + newHeight); // Reduce the space at the top
			break;
		case CA_BOTTOM:
			clientRect.setHeight(clientRect.height() - newHeight); // Reduce the space at the bottom
			newY = clientRect.y() + clientRect.height(); // The top of the control will be the bottom of the new clientrect
			break;
		case CA_LEFT:
			clientRect.setX(clientRect.x() + newWidth); // Reduce the space at the left			
			break;
		case CA_RIGHT:
			clientRect.setWidth(clientRect.width() - newWidth); // Reduce the space at the right
			newX = clientRect.x() + clientRect.width(); // The top of the control will be the bottom of the new clientrect
			break;
	}

	control->setBounds(newX, newY, newWidth, newHeight);

   /*Control.FAnchorMove := True;
   try
     Control.SetBounds(NewLeft, NewTop, NewWidth, NewHeight);
   finally
     Control.FAnchorMove := False;
   end;*/
   /*if (Control.Width <> NewWidth) or (Control.Height <> NewHeight) with Rect do
        case AAlign of
          alTop: Dec(Top, NewHeight - Control.Height);
          alBottom: Inc(Bottom, NewHeight - Control.Height);
          alLeft: Dec(Left, NewWidth - Control.Width);
          alRight: Inc(Right, NewWidth - Control.Width);
          alClient:
            begin
              Inc(Right, NewWidth - Control.Width);
              Inc(Bottom, NewHeight - Control.Height);
            end;
        end;*/
}

bool cContainer::canAlignBefore(enControlAlign align, cControl *control1, cControl *control2) {
	switch (align) {
		case CA_TOP:
			return control1->y() < control2->y();
		case CA_BOTTOM:
			return control1->y() + control1->height() >= control2->y() + control2->height();
		case CA_LEFT:
			return control1->x() < control2->x();
		case CA_RIGHT:
			return control1->x() + control1->width() >= control2->x() + control2->width();
		default:
			return false; // By default we append to the list (CA_CLIENT, CA_NONE)
	}
}

cControl *cContainer::getControl(int x, int y) {
	cControl *result = 0; // The control at that position or 0

	// Only bother if the given coordinates are within this control
	if (visible_ && (disableScissorBox_ || (x >= 0 && y >= 0 && x < width_ && y < height_))) {
		if (!controls.isEmpty()) {
			Iterator it = controls.end();
			do {
				it--;
				if ((*it)->isVisible()) {
					result = (*it)->getControl(x - (*it)->x(), y - (*it)->y());
				}
				if (result) {
					break;
				}
			} while(it != controls.begin());
		}
		if (!result && isMoveHandle()) {
			result = this;
		}
	}
	return result;
}

unsigned int cContainer::getHighestTabIndex() {
	unsigned int result = 0;

	for (Iterator it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->canHaveFocus() && (*it)->tabIndex() > result) {
			result = (*it)->tabIndex();
		}
	}

	return result;
}

cControl *cContainer::getNextFocusControl(cControl *current) {
	if (!isVisibleOnScreen() || tabControls.isEmpty()) {
		return 0;
	}

	// Get the first one
	if (!current) {
		int i = 0;
		while (i < tabControls.size()) {
			if (tabControls[i]->isVisibleOnScreen()) {
				if (tabControls[i]->isContainer()) {
					cContainer *cont = dynamic_cast<cContainer*>(tabControls[i]);
					if (cont) {
						cControl *result = cont->getNextFocusControl(0);
						if (result) {
							return result;
						}
					}
				} else if (tabControls[i]->canHaveFocus()) {
					return tabControls[i];
				}
			}
			++i;
		}
		return 0; // Nothing found
	}

	cControl *inThisControl = current;

	while (inThisControl->parent() && inThisControl->parent() != this) {
		inThisControl = inThisControl->parent();
	}

	// Not contained in us at all
	if (!inThisControl->parent()) {
		return 0;
	}

	bool found = false;

	// Iterate over the controls and see what to do
	// NOTE: We do retur 0 when we reach the end. we do NOT wrap.
	for (int i = 0; i < tabControls.size(); ++i) {
		cControl *ctrl = tabControls[i];

		if (found && ctrl->isVisibleOnScreen()) {
			// Get the first control of the next ctrl if it's a container
			if (ctrl->isContainer()) {
				cContainer *cont = dynamic_cast<cContainer*>(ctrl);
				if (cont) {
					cControl *result = cont->getNextFocusControl(0);
					if (result) {
						return result;
					}
				}
			}

			// If that didn't work either, see if the ctrl can have the focus
			if (ctrl->canHaveFocus()) {
				return ctrl;
			}

			// Now simply check the next control until there is nothing left
		}

		// Check for the next control if neccesary
		if (ctrl == inThisControl) {
			found = true;

			// If this control is a container, see if we can continue within the container
			if (ctrl->isContainer()) {
				cContainer *cont = dynamic_cast<cContainer*>(ctrl);
				if (cont) {
					cControl *result = cont->getNextFocusControl(current);
					if (result) {
						return result;
					}
				}
			}
		}
	}

	return 0;
}

cControl *cContainer::getPreviousFocusControl(cControl *current) {
	if (!isVisibleOnScreen() || tabControls.isEmpty()) {
		return 0;
	}

	// Get the last one
	if (!current) {
		int i = tabControls.size() - 1;
		while (i >= 0) {
			if (tabControls[i]->isVisibleOnScreen()) {
				if (tabControls[i]->isContainer()) {
					cContainer *cont = dynamic_cast<cContainer*>(tabControls[i]);
					if (cont) {
						cControl *result = cont->getPreviousFocusControl(0);
						if (result) {
							return result;
						}
					}
				} else if (tabControls[i]->canHaveFocus()) {
					return tabControls[i];
				}
			}
			--i;
		}
		return 0; // Nothing found
	}

	cControl *inThisControl = current;

	while (inThisControl->parent() && inThisControl->parent() != this) {
		inThisControl = inThisControl->parent();
	}

	// Not contained in us at all
	if (!inThisControl->parent()) {
		return 0;
	}

	bool found = false;

	// Iterate over the controls and see what to do
	// NOTE: We do retur 0 when we reach the beginning. we do NOT wrap.
	for (int i = tabControls.size() - 1; i >= 0; --i) {
		cControl *ctrl = tabControls[i];

		if (found && ctrl->isVisibleOnScreen()) {
			// Get the first control of the next ctrl if it's a container
			if (ctrl->isContainer()) {
				cContainer *cont = dynamic_cast<cContainer*>(ctrl);
				if (cont) {
					cControl *result = cont->getPreviousFocusControl(0);
					if (result) {
						return result;
					}
				}
			}

			// If that didn't work either, see if the ctrl can have the focus
			if (ctrl->canHaveFocus()) {
				return ctrl;
			}

			// Now simply check the previous control until there is nothing left
		}

		// Check for the previous control if neccesary
		if (ctrl == inThisControl) {
			found = true;

			// If this control is a container, see if we can continue within the container
			if (ctrl->isContainer()) {
				cContainer *cont = dynamic_cast<cContainer*>(ctrl);
				if (cont) {
					cControl *result = cont->getPreviousFocusControl(current);
					if (result) {
						return result;
					}
				}
			}
		}
	}

	return 0;
}

bool cContainer::isContainer() const {
	return true;
}

void cContainer::getContainment(Controls &result) {
	for (Iterator it = controls.begin(); it != controls.end(); ++it) {
		if (!(*it)->isContainer()) {
			result.append(*it);
		} else {
			((cContainer*)*it)->getContainment(result);
		}
	}
}

static bool isTabLessThan(cControl *a, cControl *b) {
	return a->tabIndex() < b->tabIndex();
}

void cContainer::sortTabControls() {
	qStableSort(tabControls.begin(), tabControls.end(), isTabLessThan);
}
