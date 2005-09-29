
#include <qglobal.h>
#include <qclipboard.h>
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

#include "exceptions.h"
#include "client.h"
#include "gui/gui.h"
#include "gui/textfield.h"
#include "muls/asciifonts.h"
#include "muls/unicodefonts.h"

cTextField::cTextField() {
	font_ = 0;
	hue_ = 0x3b2;
	hueAll_ = false;
	canHaveFocus_ = true;
	mouseOver_ = false;
	mouseOverHue_ = -1;
	focusHue_ = -1;
	maxLength_ = 256;
	caret_ = 0;
	leftOffset_ = 0;
	caretXOffset_ = 0;
	selection_ = 0;
	password_ = false;
	dirty = true;
	unicodeMode_ = true;
	textBorder_ = true;
	
	for (int i = 0; i < 3; ++i) {
		surfaces[i] = 0;
	}

	backgroundId_ = 0;
	background_ = 0;
}

cTextField::cTextField(int x, int y, int width, int height, unsigned char font, unsigned short hue, unsigned short background, bool hueAll, bool unicodeMode) {
	font_ = font;
	hue_ = hue;
	x_ = x;
	y_ = y;
	width_ = width;
	height_ = height;
	hueAll_ = hueAll;
	canHaveFocus_ = true;
	mouseOver_ = false;
	mouseOverHue_ = -1;
	focusHue_ = -1;
	maxLength_ = 256;
	caret_ = 0;
	leftOffset_ = 0;
	caretXOffset_ = 0;
	selection_ = 0;
	password_ = false;
	dirty = true;
	unicodeMode_ = unicodeMode;
	
	for (int i = 0; i < 3; ++i) {
		surfaces[i] = 0;
	}

	backgroundId_ = background;

	if (backgroundId_ != 0) {
		background_ = new cBorderGump(backgroundId_);
		background_->setBounds(x_, y_, width_, height_);
	} else {
		background_ = 0;
	}
}

cTextField::~cTextField() {
	delete background_;

	for (int i = 0; i < 3; ++i) {
		if (surfaces[i]) {
			surfaces[i]->decref();
		}
	}
}

void cTextField::drawSelection(cSurface *surface) {
}

void cTextField::update() {
	// Extract the portion of the string we're going to work on
	QString substring = text_.right(text_.length() - leftOffset_);
	
	if (password_) {
		for (int i = 0; i < substring.length(); ++i) {
			substring[i] = '*';
		}
	}

	// XXXXX TODO: These surfaces in general are too long. 
	// Make sure the text is cropped after X pixels

	if (!surfaces[0]) {
		if (unicodeMode_) {		
			surfaces[0] = UnicodeFonts->buildText(font_, substring, hue_, false, textBorder_, ALIGN_LEFT);
		} else {
			surfaces[0] = AsciiFonts->buildText(font_, substring.toLatin1(), hue_, false, ALIGN_LEFT, hueAll_);
		}
	}

	if (!surfaces[1] && mouseOverHue_ != -1) {
		if (unicodeMode_) {		
			surfaces[1] = UnicodeFonts->buildText(font_, substring, mouseOverHue_, false, textBorder_, ALIGN_LEFT);
		} else {
			surfaces[1] = AsciiFonts->buildText(font_, substring.toLatin1(), mouseOverHue_, false, ALIGN_LEFT, hueAll_);
		}		
	}

	if (!surfaces[2] && focusHue_ != -1) {
		if (unicodeMode_) {
			surfaces[2] = UnicodeFonts->buildText(font_, substring, focusHue_, false, textBorder_, ALIGN_LEFT);
		} else {
			surfaces[2] = AsciiFonts->buildText(font_, substring.toLatin1(), focusHue_, false, ALIGN_LEFT, hueAll_);
		}
	}

	dirty = false;
}

void cTextField::draw(int xoffset, int yoffset) {
	int backgroundOffset = background_ ? 7 : 0;
	int backgroundMarginHeight = background_ ? 4 : 0;

	if (dirty) {
		update();
	}

	if (background_) {
		background_->draw(xoffset, yoffset);
	}

	cTexture *texture = surfaces[0];

	// Chose the surface that should be drawn based on the current control state
	if (Gui->inputFocus() == this && surfaces[2]) {
		texture = surfaces[2];
	} else {
		if (mouseOver_ && surfaces[1]) {
			texture = surfaces[1];
		}
	}

	// Draw the texture
	if (texture) {
		texture->draw(x_ + backgroundOffset + xoffset, y_ + yoffset);
	}

	bool drawSelection = false;
	int selectionLeft, selectionRight;

	if (selection_ < 0) {
		int selectionX = caretXOffset_;
		
		// Iterate over all characters in the selection and calculate the entire width of the selection
		for (int i = caret_ - 1; i >= (int)(caret_ + selection_); --i) {
			uchar charWidth;
			getCharacterWidth(i, charWidth);
            		selectionX -= charWidth;

			// We're leaving the visible area
			if (selectionX <= 0) {
				selectionX = 0;
				break;
			}
		}

		selectionLeft = selectionX;
		selectionRight = caretXOffset_;
		drawSelection = true;
	} else if (selection_ > 0) {
		int selectionX = caretXOffset_;
		
		// Iterate over all characters in the selection and calculate the entire width of the selection
		for (int i = caret_; i < (int)(caret_ + selection_); ++i) {
			uchar charWidth;
			getCharacterWidth(i, charWidth);
            selectionX += charWidth;

			// We're leaving the visible area
			if (selectionX >= width_) {
				selectionX = width_;
				break;
			}
		}

		selectionLeft = caretXOffset_;
		selectionRight = selectionX;
		drawSelection = true;
	}

	if (drawSelection) {
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		glVertex2i(xoffset + x_ + backgroundOffset + selectionLeft, y_ + yoffset + backgroundMarginHeight);
		glVertex2i(xoffset + x_ + backgroundOffset + selectionRight, y_ + yoffset + backgroundMarginHeight);
		glVertex2i(xoffset + x_ + backgroundOffset + selectionRight, y_ + yoffset + (height_ - backgroundMarginHeight));
		glVertex2i(xoffset + x_ + backgroundOffset + selectionLeft, y_ + yoffset + (height_ - backgroundMarginHeight));		
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}

	// Caret als zwei linien zeichnen
	if (Gui->inputFocus() == this) {
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glVertex2f(xoffset + x_ + backgroundOffset + caretXOffset_, y_ + yoffset + backgroundMarginHeight);
		glVertex2f(xoffset + x_ + backgroundOffset + caretXOffset_, y_ + yoffset + (height_ - backgroundMarginHeight));
		glVertex2f(xoffset + x_ + backgroundOffset + 1 + caretXOffset_, y_ + yoffset + backgroundMarginHeight);
		glVertex2f(xoffset + x_ + backgroundOffset + 1 + caretXOffset_, y_ + yoffset + (height_ - backgroundMarginHeight));
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
}

void cTextField::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (background_) {
		background_->setBounds(x_, y_, width_, height_);
	}

	if (width_ != oldwidth || height_ != oldheight) {
		invalidateText();
	}
}

void cTextField::onMouseLeave() {
	mouseOver_ = false;
}

void cTextField::onMouseEnter() {
	mouseOver_ = true;
}

void cTextField::onKeyDown(QKeyEvent *e) {
	int key = e->key();
	Qt::KeyboardModifiers state = e->modifiers();

	// Handle special chars
	if (key == Qt::Key_Backspace) {
		// Replace the selection with an empty string
		if (selection_ != 0) {
			replaceSelection("");
		} else if (caret_ > 0) {
			if ((state & Qt::ControlModifier) != 0) {
				do {
					text_.remove(caret_ - 1, 1);
					setCaret(caret_ - 1);
				} while (caret_ > 0 && QChar(text_.at(caret_ - 1)).isLetterOrNumber());
			} else {
				setCaret(caret_ - 1);
				text_.remove(caret_, 1);
			}
			invalidateText();
		}
	} else if (key == Qt::Key_Delete) {
		if (selection_ != 0) {
			replaceSelection("");
		} else if ((int)caret_ < text_.length()) {
			if ((state & Qt::ControlModifier) != 0) {
				do {
					text_.remove(caret_, 1);
				} while ((int)caret_ < text_.length() && QChar(text_.at(caret_)).isLetterOrNumber());
			} else {
				text_.remove(caret_, 1);
			}
			invalidateText();
		}
	} else if (key == Qt::Key_Left) {
		if (caret_ > 0) {
			setCaret(caret_ - 1);
			if ((state & Qt::ShiftModifier) != 0) {
				selection_++;
				invalidateText();
			} else {
				if (selection_ != 0) {
					selection_ = 0;
					invalidateText();
				}
			}
		}
	} else if (key == Qt::Key_Right) {
		if ((int)caret_ < text_.length()) {
			setCaret(caret_ + 1);
			if ((state & Qt::ShiftModifier) != 0) {
				selection_--;
				invalidateText();
			} else {
				if (selection_ != 0) {
					selection_ = 0;
					invalidateText();
				}
			}
		}
	} else if (key == Qt::Key_V && (state & Qt::ControlModifier) != 0) {
		QClipboard *clipboard = qApp->clipboard();
		QString text = clipboard->text();
		if (!text.isEmpty()) {
			replaceSelection(text);
		}
	} else if (key == Qt::Key_C && (state & Qt::ControlModifier) != 0) {
		QClipboard *clipboard = qApp->clipboard();
		QString text = getSelection();
		if (!text.isEmpty()) {
			clipboard->setText(text, QClipboard::Clipboard);
		}
	} else if (key == Qt::Key_X && (state & Qt::ControlModifier) != 0) {
		QClipboard *clipboard = qApp->clipboard();
		QString text = getSelection();
		if (!text.isEmpty()) {
			clipboard->setText(text, QClipboard::Clipboard);
			replaceSelection("");
		}
	} else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
		onEnter();
	} else if (key == Qt::Key_Home) {
		selection_ = 0;
		setCaret(0);
		invalidateText();
		/*if (key.mod & KMOD_SHIFT) {
			selection_--;
			invalidateText();
		} else {
			if (selection_ != 0) {
				selection_ = 0;
				invalidateText();
			}
		}*/
	} else if (key == Qt::Key_End) {
		setCaret(text_.length());
		selection_ = 0;		
		invalidateText(); 
	} else if (text_.length() < (int)maxLength_) {	
		QChar ch;
		if (e->text().length() > 0) {
			ch = e->text().at(0);
		} else {
			return;
		}

		// Check if the character is supported by the current font.
		if (!ch.isNull()) {
			bool supported = false;

			if (unicodeMode_) {
				supported = UnicodeFonts->isCharacterSupported(font_, ch.unicode());
			} else {
				supported = AsciiFonts->getCharacter(font_, ch.toLatin1()) != 0;
			}

			if (supported) {
				QString replacement(ch);
				replaceSelection(replacement);
			}
		}
	}
}

void cTextField::onKeyUp(QKeyEvent *e) {
}

/*
	This is rather complicated.
*/
void cTextField::setCaret(unsigned int pos) {
	int backgroundOffset = background_ ? 7 : 0;

	// Special Cases Sped up
	if (pos == 0) {
		leftOffset_ = 0;
		caretXOffset_ = 0;
		caret_ = 0;
		invalidateText();
		return;
	} else if ((int)pos == text_.length()) {
		// Measure the string length backwards
		int width = 0;
		int count = 0;
		for (int i = text_.length() - 1; i >= 0; --i) {
			bool found = false;
			int charWidth = 0;

			if (unicodeMode_) {
				QChar ch = translateChar(text_.at(i));
				charWidth = UnicodeFonts->getCharacterWidth(font_, text_, i, ch);
				found = charWidth > 0;
			} else {
				cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(i)).toLatin1());
				if (ch) {
					found = true;
					charWidth = ch->width();
				}
			}
			
			if (found) {
				if (width + charWidth > width_ - backgroundOffset * 2) {
					break;
				}
				++count;
				width += charWidth;
			}
		}
		caret_ = text_.length();
		caretXOffset_ = width;
		leftOffset_ = text_.length() - count;

		invalidateText();
		return;
	}

	if (caret_ != pos) {
		uint start = qMin<uint>(pos, caret_);
		uint end = qMax<uint>(pos, caret_);
		bool moveLeft = start < caret_;

		int change = 0;

		// Just *change* the xoffset. Don't recalculate.
		for (uint i = start; i < end; ++i) {
			int charWidth = 0;
			if (unicodeMode_) {
				QChar ch = translateChar(text_.at(i));
				charWidth = UnicodeFonts->getCharacterWidth(font_, text_, i, ch);
			} else {
				cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(i)).toLatin1());
				if (ch) {
					charWidth = ch->width();
				}
			}

			cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(i)).toLatin1());
			if (ch) {
				change += moveLeft ? (- charWidth) : charWidth;
			}
		}

		// Modify the change while its too small
		if ((int)caretXOffset_ + change < 0) {
			// Try to get 1/4 of the width_ into view
			while (leftOffset_ > 0 && (int)caretXOffset_ + change < width_ / 4) {
				int charWidth = 0;
				if (unicodeMode_) {
					QChar ch = translateChar(text_.at(leftOffset_));
					charWidth = UnicodeFonts->getCharacterWidth(font_, text_, leftOffset_, ch);
				} else {
					cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(leftOffset_)).toLatin1());
					if (ch) {
						charWidth = ch->width();
					}
				}
				
				change += charWidth;
				leftOffset_--;
			}
		}

		// Modify the change while its too big
		if ((int)caretXOffset_ + change > width_ - backgroundOffset * 2) {
			while ((int)leftOffset_ < text_.length() && (int)caretXOffset_ + change > (width_ - backgroundOffset * 2) - (width_ - backgroundOffset * 2) / 4) {
				int charWidth = 0;
				if (unicodeMode_) {
					QChar ch = translateChar(text_.at(leftOffset_));
					charWidth = UnicodeFonts->getCharacterWidth(font_, text_, leftOffset_, ch);
				} else {
					cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(leftOffset_)).toLatin1());
					if (ch) {
						charWidth = ch->width();
					}
				}

				change -= charWidth;
				leftOffset_++;

				// Which one got trough instead?
				int totalWidth = 0;
				for (int i = leftOffset_; totalWidth < width_ - backgroundOffset * 2 && i < (int)text_.length(); ++i) {
					int charWidth = 0;
					if (unicodeMode_) {
						QChar ch = translateChar(text_.at(i));
						charWidth = UnicodeFonts->getCharacterWidth(font_, text_, i, ch);
					} else {
						cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(i)).toLatin1());
						if (ch) {
							charWidth = ch->width();
						}
					}
					totalWidth += charWidth;
				}

				if (totalWidth < width_ - backgroundOffset * 2) {
					break; // Dont display less than possible
				}
			}
		}

		// Here we decide if it's even worth the effort
		caretXOffset_ += change;

		// Recalculate the left x-offset for the caret
		caret_ = pos;
	}
	invalidateText();
}

// Return the offset in the text_ string by using the given x
// value relative to this control
unsigned int cTextField::getOffset(int x) {
	if (background_) {
		x -= 7;
	}
	
	if (x < 0) {
		return leftOffset_;
	}

	int offset  = 0;
	int i;
	for (i = leftOffset_; i < (int)text_.length(); ++i) {
		int charWidth = 0;

		if (unicodeMode_) {
			charWidth = UnicodeFonts->getCharacterWidth(font_, text_, i, translateChar(text_.at(i)));
		} else {
            cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(i)).toLatin1());
			if (ch) {
				charWidth = ch->width();
			}
		}

		if (charWidth > 0) {
			// Is it in this character?
			if (x >= offset && x < offset + charWidth) {
				if (x - offset >= (charWidth / 2)) {
					// Insert caret before
					return i + 1;
				} else {
					// Insert caret after
					return i;
				}
			}
			offset += charWidth;
		}
	}

	return i;
}

void cTextField::onMouseDown(QMouseEvent *e) {
	cControl::onMouseDown(e);

	QPoint local = mapFromGlobal(e->pos());
	int index = getOffset(local.x());
	selection_ = 0;

	// XXXXXX TODO: Normal windows selection rules with mouse

	// Selection?
	Qt::KeyboardModifiers state = e->modifiers();
	if ((state & Qt::ShiftModifier) != 0) {
		// In which direction do we select?
		int diff = caret_ - index;

		// The offset would be the new selection value
		selection_ = diff;
	}

	setCaret(index);
}

void cTextField::onMouseUp(QMouseEvent *e) {
	cControl::onMouseUp(e);
}

void cTextField::replaceSelection(const QString &replacement) {
	// Delete the selection, reposition caret_, then reinsert
	if (selection_ != 0) {
		if (selection_ < 0) {
			setCaret(caret_ + selection_); // Place the caret at the start of the selection
			selection_ = - selection_;
		}
		for (int i = 0; i < selection_ && (int)caret_ < text_.length(); ++i) {
			text_.remove(caret_, 1);
		}
		selection_ = 0;
		invalidateText();
	}

	// Insert text at the caret
	int i;
	for (i = 0; i < (int)replacement.length() && text_.length() + 1 <= (int)maxLength_; ++i) {
		text_.insert(caret_ + i, replacement.at(i));
	}
	setCaret(caret_ + i);
}

QString cTextField::getSelection() {
	if (selection_ < 0) {
		return text_.mid(caret_ + selection_, - selection_);
	} else if (selection_ > 0) {
		return text_.mid(caret_, selection_);
	} else {
		return "";
	}
}

void cTextField::onEnter() {
	emit enterPressed(this);
}

bool cTextField::getCharacterWidth(uint i, uchar &charWidth) {
	if (unicodeMode_) {
		charWidth = UnicodeFonts->getCharacterWidth(font_, text_, i, translateChar(text_.at(i)));
		return true;
	} else {
        cSurface *ch = AsciiFonts->getCharacter(font_, translateChar(text_.at(i)).toLatin1());
		if (ch) {
			charWidth = ch->width();
			return true;
		}		
	}
	return false;
}

void cTextField::processDefinitionAttribute(QString name, QString value) {
	if (name == "hueall") {
		setHueAll(Utilities::stringToBool(value));
	} else if (name == "font") {
		setFont(Utilities::stringToUInt(value));
	} else if (name == "hue") {
		setFont(Utilities::stringToUInt(value));
	} else if (name == "text") {
		setText(value);
	} else if (name == "hoverhue") {
		setMouseOverHue(Utilities::stringToUInt(value));
	} else if (name == "focushue") {
		setFocusHue(Utilities::stringToUInt(value));
	} else if (name == "password") {
		setPassword(Utilities::stringToBool(value));
	} else if (name == "unicode") {
		setUnicodeMode(Utilities::stringToBool(value));
	} else if (name == "background") {
		setBackground(Utilities::stringToUInt(value));
	} else if (name == "maxlength") {
		setMaxLength(Utilities::stringToUInt(value));
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cTextField::processDefinitionElement(QDomElement element) {
	cControl::processDefinitionElement(element);
}

void cTextField::setBackground(ushort data) {
	if (backgroundId_ != data) {
		backgroundId_ = data;
		delete background_;
		if (data != 0) {
			background_ = new cBorderGump(data);
		}
	}
}
