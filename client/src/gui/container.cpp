
#include <qvaluelist.h>

#include "engine.h"
#include "exceptions.h"
#include "utilities.h"
#include "gui/container.h"

void cContainer::draw(int xoffset, int yoffset) {
	xoffset += x_;
	yoffset += y_;

	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->isVisible()) {
			(*it)->draw(xoffset, yoffset);
		}
	}
}

cContainer::cContainer() {
}

void cContainer::clear() {
	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		delete *it;
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

	if (control->canHaveFocus()) {
		control->setTabIndex(getHighestTabIndex() + 1); // Only bother for focusable controls
	}

	control->setParent(this);
	control->requestAlign();
}

void cContainer::removeControl(cControl *control) {
	if (control->parent() != this) {
		return;
	}

	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		if (*it == control) {
			controls.erase(it);
			break;
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

	SDL_Rect clientRect; // This rectangle will be modified by the code "below"
	clientRect.x = 0;
	clientRect.y = 0;
	clientRect.w = width_;
	clientRect.h = height_;

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

void cContainer::doAlignment(enControlAlign align, cControl *control, SDL_Rect &clientRect) {
	QValueList<cControl*> alignlist; // List of Controls to be aligned
	QValueList<cControl*>::iterator lit; // List iterator

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

void cContainer::doPositioning(enControlAlign align, cControl *control, SDL_Rect &clientRect) {
	int newWidth = clientRect.w; // By default we use up the entire remaining width
	if (align == CA_LEFT || align == CA_RIGHT)
		newWidth = control->width(); // For the left/right alignment, use the control width instead

	int newHeight = clientRect.h; // By default we use up the entire remaining height
	if (align == CA_TOP || align == CA_BOTTOM)
		newHeight = control->height(); // For the bottom/top alignment, use the control height instead

	int newX = clientRect.x; // By default, use the upper left position of the clientrect
	int newY = clientRect.y; // By default, use the upper left position of the clientrect

	// Modify the remaining rect based on the alignment type
	switch (align) {
		case CA_TOP:
			clientRect.y += newHeight; // Reduce the space at the top
			clientRect.h -= newHeight;
			break;
		case CA_BOTTOM:
			clientRect.h -= newHeight; // Reduce the space at the bottom
			newY = clientRect.y + clientRect.h; // The top of the control will be the bottom of the new clientrect
			break;
		case CA_LEFT:
			clientRect.x += newWidth; // Reduce the space at the left
			clientRect.w -= newWidth;
			break;
		case CA_RIGHT:
			clientRect.w -= newWidth; // Reduce the space at the right
			newX = clientRect.x + clientRect.w; // The top of the control will be the bottom of the new clientrect
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
	if (visible_ && x >= 0 && y >= 0 && x < width_ && y < height_) {
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
		//if (!result) {
		//	result = this;
		//}
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
	cControl *result = 0;
	unsigned int tabindex = current ? current->tabIndex() : 0;
	for (Iterator it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->isVisible() && (*it)->canHaveFocus() && (*it)->tabIndex() > tabindex) {
			result = *it;
			break;
		}
	}

	if (!result && current) {
		return getNextFocusControl(0);
	}

	return result;
}

cControl *cContainer::getPreviousFocusControl(cControl *current) {
	// Try to find the one with the highest id that is smaller than the 
	// current focus control.
	cControl *result = 0;
	cControl *lastControl = current;
	unsigned int upperLimit = current ? current->tabIndex() : ~(unsigned int)0;	
	unsigned int currentIndex = 0;

	for (Iterator it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->isVisible() && (*it)->canHaveFocus()) {
			// Highest?
			if ((*it)->tabIndex() > (lastControl ? lastControl->tabIndex() : 0)) {
				lastControl = *it;
			}

			if ((*it)->tabIndex() > currentIndex && (*it)->tabIndex() < upperLimit) {
				result = *it;
				currentIndex = result->tabIndex();
			}
		}
	}

	if (!result) {
		result = lastControl;
	}

	return result;
}

bool cContainer::isContainer() const {
	return true;
}
