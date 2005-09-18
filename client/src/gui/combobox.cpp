
#include "gui/combobox.h"
#include "gui/imagebutton.h"
#include "gui/bordergump.h"
#include "gui/stripeimage.h"
#include "gui/scrollbar.h"
#include "mainwindow.h"
#include "utilities.h"

#include <qgl.h>
#include <QCursor>

cCombolist::cCombolist() {
	combobox_ = 0;
}

cCombolist::~cCombolist() {
	if (combobox_) {
		combobox_->selectionList_ = 0;
	}
}

void cCombolist::scrolled(int pos) {
	if (!combobox_) {
		return;
	}

	int shift = - pos; // Pos is the pixel offset for the contents of the list

	QList<int> existingLabels; // Keep track of existing labels

	// Check all the labels in this combolist if they're still in sight etc.
	Controls controls = this->controls; // Make a copy
	foreach (cControl *control, controls) {
		cLabel *label = dynamic_cast<cLabel*>(control);
		if (!label) {
			continue;
		}

		// Retrieve the index for the given label
		QMap<cLabel*, uint>::iterator it = labelIds.find(label);
		if (it == labelIds.end()) {
			continue;
		}

		int index = *it;

		int labelTop = index * combobox_->itemHeight_ + shift;

		// Remove the label if it's out of sight
		if (labelTop + combobox_->itemHeight() < 0 || labelTop > height_) {
			labelIds.erase(it);
			delete label;
			continue;
		} else {
			label->setY(labelTop);
			existingLabels.append(index);
		}
	}

	// Calculate the indices that are currently visible
	int start = pos / combobox_->itemHeight();
	int end = start + (height_ + (combobox_->itemHeight() - 1)) / combobox_->itemHeight() + 1;
	for (int i = start; i < end && i < combobox_->items_.size(); ++i) {
		if (existingLabels.contains(i)) {
			continue;
		}

		int labelTop = i * combobox_->itemHeight_ + shift;

		// Don't add the label if it's out of sight
		if (labelTop + combobox_->itemHeight() < 0 || labelTop > height_) {			
			continue;
		}

		cLabel *label = new cLabel;
		label->setText(combobox_->items_[i]);
		label->setFont(1);
		label->setBorder(false);
		label->setHue(1);
		label->setPosition(3, labelTop);
		addControl(label);
		labelIds.insert(label, i);
	}
}

// See if we managed to select one
void cCombolist::onMouseUp(QMouseEvent *e) {
	QPoint mp = GLWidget->mapFromGlobal(QCursor::pos());
	mp.setX(mp.x() - x_);
	mp.setY(mp.y() - y_);
	cControl *scrollbar = findByName("scrollbar");

	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->isVisible()) {
			cLabel *label = dynamic_cast<cLabel*>((*it));
			
			// Draw a "selection box" if this control has mouse over focus
			if (label) {
				QMap<cLabel*, uint>::iterator it = labelIds.find(label);

				if (it == labelIds.end()) {
					continue;
				}

				bool mouseOver = true;

				// Check Height
				if (mp.y() < label->y() || mp.y() >= label->y() + (int)combobox_->itemHeight()) {
					mouseOver = false;
				}

				// Check Width
				if (mp.x() < label->x() || mp.x() > (scrollbar ? (scrollbar->x()) : width_)) {
					mouseOver = false;
				}

				if (mouseOver) {
					emit itemSelected(*it);
					Gui->setCurrentCombolist(0); // Remove us
					return;
				}
			}
		}
	}
}

cControl *cCombolist::getControl(int x, int y) {
	// This is a workaround so we get ourself even if the background is clicked
	moveHandle_ = true;
	movable_ = true;
	cControl *result = cWindow::getControl(x, y);
	movable_ = false;
	moveHandle_ = false;

	// We want to receive all events for the labels
	if (result && dynamic_cast<cLabel*>(result) != 0) {
		return this;
	}

	return result;
}

void cCombolist::draw(int xoffset, int yoffset) {
	if (!combobox_) {
		return;
	}

	QPoint mp = GLWidget->mapFromGlobal(QCursor::pos());
	mp.setX(mp.x() - x_);
	mp.setY(mp.y() - y_);
	cControl *scrollbar = findByName("scrollbar");

	xoffset += x_;
	yoffset += y_;

	// Save the old scissor test flag
	glPushAttrib(GL_SCISSOR_BIT|GL_SCISSOR_BIT);
	
	// And enable the scissor box for this container
	glEnable(GL_SCISSOR_TEST);
		
	// Set the scissor box to the coordinates of this container
	glScissor(xoffset, GLWidget->height() - (yoffset + height_), width_, height_);

	int rightMargin = 4;

	if (scrollbar) {
		rightMargin += scrollbar->width();
	}

	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		if ((*it)->isVisible()) {
			cLabel *label = dynamic_cast<cLabel*>((*it));
			
			// Draw a "selection box" if this control has mouse over focus
			if (label) {
				// The labels are always at the end of the control list, that means once
				// we reach the first label, we can modify the scissor box to cut the top/bottom of the labels
				// going in and out of view
				glScissor(xoffset, GLWidget->height() - (yoffset + height_ - 5), width_ - rightMargin, height_ - 10);

				// Here we check if the control is currently selected and point that out to the user
				QMap<cLabel*, uint>::iterator idit = labelIds.find(label);
				if (idit != labelIds.end() && *idit == combobox_->selectionIndex()) {
					// Draw a solid 25% alpha background (white)
					glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
					glBindTexture(GL_TEXTURE_2D, 0);
					glBegin(GL_QUADS);
					int topLeftX = xoffset + label->x() - 1;
					int topLeftY = yoffset + label->y() - 1;					
					glVertex2i(topLeftX, topLeftY + combobox_->itemHeight());
					glVertex2i(topLeftX + (scrollbar ? (scrollbar->x()) : width_) - 3, topLeftY + combobox_->itemHeight());
					glVertex2i(topLeftX + (scrollbar ? (scrollbar->x()) : width_) - 3, topLeftY);
					glVertex2i(topLeftX, topLeftY);
					glEnd();
				}

				// Here we check if the mouse is over the control and draw it differently
				bool mouseOver = true;

				// Check Height
				if (mp.y() < label->y() || mp.y() >= label->y() + (int)combobox_->itemHeight()) {
					mouseOver = false;
				}

				// Check Width
				if (mp.x() < label->x() || mp.x() > (scrollbar ? (scrollbar->x()) : width_)) {
					mouseOver = false;
				}

				if (mouseOver) {
					// Draw a solid 25% alpha background (white)
					glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
					glBindTexture(GL_TEXTURE_2D, 0);
					glBegin(GL_QUADS);
					int topLeftX = xoffset + label->x() - 1;
					int topLeftY = yoffset + label->y() - 1;					
					glVertex2i(topLeftX, topLeftY + combobox_->itemHeight());
					glVertex2i(topLeftX + (scrollbar ? (scrollbar->x()) : width_) - 3, topLeftY + combobox_->itemHeight());
					glVertex2i(topLeftX + (scrollbar ? (scrollbar->x()) : width_) - 3, topLeftY);
					glVertex2i(topLeftX, topLeftY);
					glEnd();
				}
			}

			(*it)->draw(xoffset, yoffset);
		}
	}

	// Pop the old scissor box/test attribs from the gl stack
	glPopAttrib();
}

cCombobox::cCombobox() {
	selection_ = 0;
	selectionIndex_ = -1;
	setStyle(Style1);
	selectionList_ = 0;
	maxListHeight_ = 100;
	itemHeight_ = 18;
}

cCombobox::~cCombobox() {
	if (selectionList_) {
		if (Gui->currentCombolist() == selectionList_) {
			Gui->setCurrentCombolist(0);
		} else {
			delete selectionList_;
		}
	}
}

void cCombobox::clearItems() {
	items_.clear();
	selectItem(-1);
}

void cCombobox::addItem(QString item) {
	items_.append(item);
}

void cCombobox::setStyle(cCombobox::Style style) {
	clear();

	style_ = style;

	// Build background
	cBorderGump *bordergump = new cBorderGump();
	bordergump->setObjectName("background");
	bordergump->setPosition(0, 0);
	bordergump->setSize(width_, height_);

	switch (style) {
		default:
		case Style1:
			height_ = 18;
			bordergump->setPosition(0, 1);
			bordergump->setSize(width_, height_ - 1);
			bordergump->setIds(9553, 9554, 9555, 9553, 9554, 9555, 9553, 9554, 9555);
			break;
		case Style2:
			height_ = 26;
			bordergump->setBounds(0, 0, width_, height_);
			bordergump->setId(3000);
			break;
	}

	addControl(bordergump);

	// Label representing the current selection (none?)
	selection_ = new cLabel;
	selection_->setFont(1);	
	selection_->setBorder(false);
	if (selectionIndex_ != -1 && selectionIndex_ < items_.size()) {
		selection_->setText(items_[selectionIndex_]);
	}	
	switch (style) {
		default:
		case Style1:
			selection_->setPosition(4, 0);
			selection_->setHue(0x8fd);
			break;
		case Style2:
			selection_->update();
			selection_->setPosition(4, 4);
			selection_->setHue(1);
			break;
	}
	addControl(selection_);

	// Toggle Button
	cImageButton *button = new cImageButton();
	button->setObjectName("toggle");
	switch (style) {
		default:
		case Style1:
			button->setStateGump(BS_UNPRESSED, 2706);
			button->setStateGump(BS_PRESSED, 2707);
			button->draw(0, 0);
			button->setPosition(width_ - button->width(), 0);
			break;
		case Style2:
			button->setStateGump(BS_UNPRESSED, 2706);
			button->setStateGump(BS_PRESSED, 2707);
			button->draw(0, 0);
			button->setPosition(width_ - button->width() - 4, (height_ - button->height()) / 2);
			break;
	}
	addControl(button);

	connect(button, SIGNAL(onButtonPress(cControl*)), SLOT(openSelectionList()));
}

void cCombobox::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	height_ = oldheight; // Do not allow to change height

	foreach (cControl *control, controls) {
		if (control->objectName() == "background") {
			control->setWidth((width_ - oldwidth) + control->width());
		} else if (control->objectName() == "toggle") {
			control->setX(control->x() + (width_ - oldwidth));
		}
	}

	cContainer::onChangeBounds(oldx, oldy, oldwidth, oldheight);
}

void cCombobox::openSelectionList() {
	if (items_.isEmpty()) {
		return;
	}

	delete selectionList_;

	// Get screen coordinates
	int x = x_;
	int y = y_;
	cContainer *parent = parent_;
	while (parent) {
		x += parent->x();
		y += parent->y();
		parent = parent->parent();
	}

	selectionList_ = new cCombolist;
	selectionList_->setBounds(x, y + height_, width_, qMin<uint>(maxListHeight_, items_.size() * itemHeight_));	

	cBorderGump *border = new cBorderGump();
	border->setAlign(CA_CLIENT);
	border->setMoveHandle(true);

	switch (style_) {
		default:
		case Style1:
			border->setIds(9553, 9554, 9555, 9553, 9554, 9555, 9556, 9557, 9558);
			break;
		case Style2:
			border->setId(3000);
			break;
	}

	selectionList_->addControl(border);

	int overlap = itemHeight_ * items_.size() - (selectionList_->height() - 5);

	// Only show the scrollbar if neccesary
	if (overlap > 0) {
		border = new cBorderGump(); // Scrollbar background
		border->setId(9200);
		
		cVerticalScrollbar *scrollbar = new cVerticalScrollbar(0, 5, selectionList_->height() - 10);
		scrollbar->setObjectName("scrollbar");
		scrollbar->setHandleId(9026);
		scrollbar->setBackgroundId(9204);
		scrollbar->setUpButtonIds(9900, 9902, 9901);
		scrollbar->setDownButtonIds(9906, 9908, 9907);
		scrollbar->setRange(0, overlap);
		scrollbar->setSmallIncrement(itemHeight_);
		
		scrollbar->setPosition(selectionList_->width() - 5 - scrollbar->width(), 5); // Make sure the border is kept
		border->setBounds(scrollbar->x(), scrollbar->y(), scrollbar->width(), scrollbar->height());
		selectionList_->addControl(border);
		selectionList_->addControl(scrollbar);		
		connect(scrollbar, SIGNAL(scrolled(int)), selectionList_, SLOT(scrolled(int)));
	}

	selectionList_->setCombobox(this);
	selectionList_->scrolled(0); // This displays the first bunch of entries
	connect(selectionList_, SIGNAL(itemSelected(int)), SLOT(selectItem(int)));
	Gui->setCurrentCombolist(selectionList_);
}

void cCombobox::selectItem(int index) {
	if (index < 0 || index >= items_.size()) {
		selectionIndex_ = -1;
		selection_->setText("");
	} else {
		selection_->setText(items_[index]);
		selectionIndex_ = index;
	}

	emit selectionChanged();
}

void cCombobox::processDefinitionAttribute(QString name, QString value) {
	if (name == "style") {
		setStyle(Style(Utilities::stringToUInt(value)));
	} else {
		return cContainer::processDefinitionAttribute(name, value);
	}
}
