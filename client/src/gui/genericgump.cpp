
#include "gui/genericgump.h"
#include "gui/bordergump.h"
#include "gui/checkertrans.h"
#include "gui/tiledgumpimage.h"
#include "gui/imagebutton.h"
#include "gui/gumpimage.h"
#include "gui/textfield.h"
#include "gui/itemimage.h"
#include "gui/checkbox.h"
#include "gui/label.h"
#include "network/outgoingpackets.h"
#include "network/uosocket.h"
#include "muls/localization.h"
#include "log.h"

#include <QMap>
#include <qvector.h>

QMap<uint, cGenericGump*> cGenericGump::instances;

cGenericGump::~cGenericGump() {
	instances.remove(gumpType_);
}

cGenericGump* cGenericGump::findByType(uint type) {
	if (instances.contains(type)) {
		return instances[type];
	} else {
		return 0;
	}
}

cGenericGump::cGenericGump(int x, int y, unsigned int serial, unsigned int gumpType) {
	noclose = false;
	x_ = x;
	y_ = y;
	disableScissorBox_ = true;
	movable_ = true;
	serial_ = serial;
	gumpType_ = gumpType;

	// If a gump of the same type already exists, get it's x,y coordinates and kill the old one
	if (gumpType != 0) {
		if (instances.contains(gumpType)) {
			cGenericGump *gump = instances[gumpType];
			x_ = gump->x();
			y_ = gump->y();
			Gui->removeControl(gump);
			delete gump;
		}
		instances.insert(gumpType, this);
	}
}

void cGenericGump::addControl(unsigned int page, cControl *control, bool back) {
	if (page == 0) {
		cContainer::addControl(control);
	} else {
		QMap<uint, cContainer*>::iterator it = pages.find(page);

		if (it == pages.end()) {
			cContainer *cont = new cContainer();
			cont->setBounds(0, 0, width_, height_);
			cont->setDisableScissorBox(true);
			if (page != 1) {
				cont->setVisible(false);
			}
			
			cContainer::addControl(cont);
			pages.insert(page, cont);
			
			cont->addControl(control, back);
		} else {
			it.value()->addControl(control, back);
		}
	}
}

void cGenericGump::processCommand(stLayoutContext &context, QString line, QStringList strings) {
	QStringList tokens = line.split(" ");
	
	if ( tokens.isEmpty() ) {
		return; // Invalid command
	}

	QString command = tokens[0].toLower(); // The first token is the command

	// page <id>
	if (command == "page" && tokens.size() >= 2) {
		unsigned int id = tokens[1].toUInt(); // Convert 1st parameter to int
		context.page = id;
	}
	// resizepic <x> <y> <id> <width> <height>
	else if (command == "resizepic" && tokens.size() >= 5) {		
		cBorderGump *gump = new cBorderGump(tokens[3].toUInt());
		gump->setBounds(tokens[1].toInt(), tokens[2].toInt(), tokens[4].toUInt(), tokens[5].toUInt());
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// gumppictiled <x> <y> <width> <height> <id> <hue=0>
	else if (command ==	"gumppictiled") {
		unsigned short hue = 0; // TODO: Parse arguments
		cTiledGumpImage *gump = new cTiledGumpImage(tokens[5].toUInt(), hue, false);
		gump->setBounds(tokens[1].toInt(), tokens[2].toInt(), tokens[3].toUInt(), tokens[4].toUInt());
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// checkertrans <x> <y> <width> <height>
	else if (command ==	"checkertrans") {
		if (context.page == 0) {
			cCheckerTrans *gump = new cCheckerTrans(true);
			gump->setBounds(tokens[1].toInt(), tokens[2].toInt(), tokens[3].toUInt(), tokens[4].toUInt());
			gump->setMoveHandle(true);
			enableStencil_ = true;
			cContainer::addControl(gump, true);

			gump = new cCheckerTrans(false);
			gump->setBounds(tokens[1].toInt(), tokens[2].toInt(), tokens[3].toUInt(), tokens[4].toUInt());
			gump->setMoveHandle(true);
			enableStencil_ = true;
			cContainer::addControl(gump);
		}
	}
	// text <x> <y> <hue> <id>
	else if (command == "text" && tokens.size() >= 5) {
		int textId = tokens[4].toUInt();
		unsigned short hue = tokens[3].toUInt() + 1; // 0 = 1 for this one ;)

		// Get the text from the string table
		QString text;
		if (textId < strings.size()) {
			text = strings[textId];
		} else {
			text = tr("ERR: Index out of Range (%1)").arg(textId);
		}

        cLabel *gump = new cLabel(text, 1, hue, hue != 1);
		gump->setPosition(tokens[1].toInt(), tokens[2].toInt());
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// croppedtext <x> <y> <width> <height> <hue> <id>
	else if (command == "croppedtext" && tokens.size() >= 7) {
		int textId = tokens[6].toUInt();
		ushort hue = tokens[5].toUInt() + 1; // 0 = 1 for this one ;)
		ushort width = tokens[3].toUInt();
		ushort height = tokens[4].toUInt();

		// Get the text from the string table
		QString text;
		if (textId < strings.size()) {
			text = strings[textId];
		} else {
			text = tr("ERR: Index out of Range (%1)").arg(textId);
		}

        cLabel *gump = new cLabel(text, 1, hue, hue != 1);
		gump->setPosition(tokens[1].toInt(), tokens[2].toInt());
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// button <x> <y> <off> <on> <close> <buttonid> <buttonid>
	else if (command == "button" && tokens.size() >= 8) {
		unsigned short off = tokens[3].toUShort();
		unsigned short on = tokens[4].toUShort();

		cImageButton *gump = new cImageButton(tokens[1].toInt(), tokens[2].toInt(), off, on);

		// Get interactivity properties for the button
		bool close = tokens[5] != "0"; // 1 = page switch, 0 = close button
		unsigned int buttonId;
		if (close) {
			buttonId = tokens[7].toUInt(); // Response code
			gump->setObjectName("closebutton");
		} else {
			buttonId = tokens[6].toUInt(); // Page id
			gump->setObjectName("pagebutton");
		}
		
		controlIds.insert(gump, buttonId); // Save control id
		connect(gump, SIGNAL(onButtonPress(cControl*)), this, SLOT(onButtonPress(cControl*)));
		
        addControl(context.page, gump);
	}
	// checkbox <x> <y> <off> <on> <checked> <id>
	else if (command == "checkbox" && tokens.size() >= 7) {
		unsigned short off = tokens[3].toUShort();
		unsigned short on = tokens[4].toUShort();

		cCheckbox *gump = new cCheckbox();
		gump->setPosition(tokens[1].toInt(), tokens[2].toInt());
		gump->setStateGump(cCheckbox::Unchecked, off);
		gump->setStateGump(cCheckbox::Hover, off);
		gump->setStateGump(cCheckbox::Pressed, on);
		gump->setStateGump(cCheckbox::Checked, on);
		gump->setStateGump(cCheckbox::CheckedHover, on);
		gump->setChecked(Utilities::stringToBool(tokens[5]));
		gump->setObjectName("checkbox");
			
		controlIds.insert(gump, tokens[6].toInt()); // Save control id
        addControl(context.page, gump);
	}
	// textentry <x> <y> <width> <height> <color> <id> <initialtext>
	else if (command == "textentry" && tokens.size() >= 8) {
		unsigned short hue = tokens[5].toUShort() + 1;

		// Get the text from the string table
		QString text;
		int textId = tokens[7].toUInt();
		if (textId < strings.size()) {
			text = strings[textId];
		} else {
			text = tr("ERR: Index out of Range (%1)").arg(textId);
		}

		//cTextField *gump = new cTextField(tokens[1].toInt(), tokens[2].toInt(), tokens[3].toInt(), tokens[4].toInt(), 3, hue, 0, true);
		cTextField *gump = new cTextField(tokens[1].toInt(), tokens[2].toInt(), tokens[3].toInt(), tokens[4].toInt(), 1, hue, 0, true, true);
		gump->setText(text);
		gump->setTextBorder(hue != 1);
		controlIds.insert(gump, tokens[6].toUInt());
		addControl(context.page, gump);		
	}
	// htmlgump <x> <y> <width> <height> <textid> <hasback> <scrollable>
	else if (command == "htmlgump" && tokens.size() >= 8) {
		int textId = tokens[5].toUInt();
		ushort width = tokens[3].toUInt();
		ushort height = tokens[4].toUInt();
		bool hasBackground = tokens[6].toUInt() != 0;
		bool scrollable = tokens[7].toUInt() != 0;

		// Get the text from the string table
		QString text;
		if (textId < strings.size()) {
			text = strings[textId];
		} else {
			text = tr("ERR: Index out of Range (%1)").arg(textId);
		}

        cLabel *gump = new cLabel(text, 1, 1, false, ALIGN_LEFT, false);
		gump->setHtmlMode(true);
		gump->setBounds(tokens[1].toInt(), tokens[2].toInt(), width, height);
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// xmfhtmlgump/xmfhtmlgumpcolor <x> <y> <width> <height> <textid> <hasback> <scrollable> [color]
	else if ((command == "xmfhtmlgump" && tokens.size() >= 8) || (command == "xmfhtmlgumpcolor" && tokens.size() >= 9)) {
		uint textId = tokens[5].toUInt();
		int x = tokens[1].toInt();
		int y = tokens[2].toInt();
		ushort width = tokens[3].toUInt();
		ushort height = tokens[4].toUInt();
		bool hasBackground = tokens[6].toUInt() != 0;
		bool scrollable = tokens[7].toUInt() != 0;

		// Add the 0xbb8 background with a 4 pixel margin
		if (hasBackground) {
			cBorderGump *gump = new cBorderGump(0xbb8);
			gump->setBounds(x, y, width, height);
			addControl(context.page, gump);
			x += 4;
			y += 4;
			width -= 8;
			height -= 8;
		}

		// Get the text from the string table
		QString text = Localization->get(textId);		

		if (command == "xmfhtmlgumpcolor") {
			text.prepend(QString("<basefont color=#%1>").arg(tokens[8].toUInt(), 0, 16));
		}

        cLabel *gump = new cLabel(text, 1, 1, false, ALIGN_LEFT, false);
		gump->setHtmlMode(true);
		gump->setBounds(x, y, width, height);
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// gumppic <x> <y> <gump>
	else if (command == "gumppic" && tokens.size() >= 4) {
		cGumpImage *gump = new cGumpImage(tokens[3].toUShort());
		gump->setPosition(tokens[1].toInt(), tokens[2].toInt());
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// tilepic <x> <y> <gump> <hue?>
	else if (command == "tilepic" && tokens.size() >= 4) {
		cItemImage *gump = new cItemImage(tokens[3].toUShort());
		gump->setPosition(tokens[1].toInt(), tokens[2].toInt());
		gump->setMoveHandle(true);
		addControl(context.page, gump);
	}
	// noclose
	else if (command == "noclose") {
		noclose = true;
	}
	// nomove
	else if (command == "nomove") {
		setMovable(false);
	}
	// any other command
	else {
		Log->print(LOG_WARNING, tr("Unknown command '%1' in generic gump received from server.\n").arg(line));
	}
	
}

void cGenericGump::parseLayout(QString layout, QStringList strings) {
	// Just walk trough it like a character array
	int commandStart = 0;
	bool inCommand = false;
	stLayoutContext context;
	context.page = 0;
	context.group = 0;
	
	// Treat the line as a string
	for (int i = 0; i < layout.length(); ++i) {
		// Command start symbol
		if (layout[i] == QChar('{')) {
			commandStart = i + 1;
			inCommand = true;
		// Command terminator
		} else if (inCommand && layout[i] == QChar('}')) {
			QString command = layout.mid(commandStart, i - commandStart).trimmed();			
			inCommand = false; // Until the next { we dont process commands
			processCommand(context, command, strings); // Process the parsed command
		}
	}
}

void cGenericGump::sendResponse(uint button) {
	QVector<uint> switches;
	QMap<uint, QString> strings;

	// Iterate over the controls in this gump and find all the textfields / switches
	cContainer::Controls content;
	getContainment(content);

	cContainer::Controls::iterator it;
	for (it = content.begin(); it != content.end(); ++it) {
		cTextField *textField = dynamic_cast<cTextField*>(*it);

		if (textField && controlIds.contains(textField)) {
			strings.insert(controlIds[textField], textField->text());
		}
	}

	UoSocket->send(cGenericGumpResponsePacket(serial_, gumpType_, button, switches, strings));
}

void cGenericGump::onButtonPress(cControl *sender) {
	// See if we know what "sender" is
	QMap<cControl*, uint>::iterator it = controlIds.find(sender);

	if (it == controlIds.end()) {
		return;
	}

	unsigned int controlId = it.value();

	// Switch to the given page
	if (sender->objectName() == "pagebutton") {
		if (controlId > 0) {
			QMap<uint, cContainer*>::iterator it;
			for (it = pages.begin(); it != pages.end(); ++it) {
				if (it.key() == controlId) {
					it.value()->setVisible(true);
				} else {
					it.value()->setVisible(false);
				}
			}
		}		
	}
	// Close the window and send a gump resonse packet
	else if (sender->objectName() == "closebutton") {
		sendResponse(controlId);
		Gui->queueDelete(this);
	}
}

/*
	We have to override this for the same reason we have to override draw()
*/
cControl *cGenericGump::getControl(int x, int y) {
	cControl *result = 0; // The control at that position or 0

	// Only bother if the given coordinates are within this control
	if (visible_) {
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
	}
	
	if (result && result != this && isMovable() && result->isMoveHandle()) {
		return this;
	}

	return result;
}

void cGenericGump::setGumpType(uint data) {
	if (gumpType_ != 0 && instances.contains(gumpType_)) {
		instances.remove(gumpType_);
	}
	gumpType_ = data;
	// If a gump of the same type already exists, get it's x,y coordinates and kill the old one
	if (gumpType_ != 0) {
		if (instances.contains(gumpType_)) {
			cGenericGump *gump = instances[gumpType_];
			x_ = gump->x();
			y_ = gump->y();
			Gui->removeControl(gump);
			delete gump;
		}	
		instances.insert(gumpType_, this);
	}
}

void cGenericGump::onClick(QMouseEvent *e) {
	if (!noclose && e->button() == Qt::RightButton) {
		sendResponse(0);
		Gui->queueDelete(this);
	} else {
		cWindow::onClick(e);
	}
}
