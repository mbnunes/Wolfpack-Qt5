
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3CString>
#include <QDesktopWidget>

#include "config.h"
#include "dialogs/login.h"
#include "gui/gui.h"
#include "enums.h"
#include "uoclient.h"
#include "utilities.h"
#include "gui/tiledgumpimage.h"
#include "gui/gumpimage.h"
#include "gui/bordergump.h"
#include "gui/label.h"
#include "gui/asciilabel.h"
#include "gui/imagebutton.h"
#include "gui/textfield.h"
#include "gui/scrollbar.h"
#include "gui/worldview.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "mainwindow.h"
#include "version.h"

#include <QMenuBar>

/* A custom cShardLabel class */
class cShardLabel : public cTextField {
private:
	cShardLabel *secondary;
	cShardLabel *tertiary;
	int id;

public:
	cShardLabel(int x, int y, const Q3CString &text) : cTextField(x, y, 350, 25, 5, 0x34f, 0, true) {
		canHaveFocus_ = false; // These are only clickable
		setMouseOverHue(0x23);
		setText(text);
		secondary = 0;
		tertiary = 0;
		id = -1;
	}

	void setId(int data) {
		id = data;
	}

	// Forward MoseEnter + MouseLeave to the two other controls
	void onMouseEnter() {
		cTextField::onMouseEnter();
		if (secondary) {
			secondary->mouseOver_ = true;
		}
		if (tertiary) {
			tertiary->mouseOver_ = true;
		}
	}

	void onMouseLeave() {
		cTextField::onMouseLeave();
		if (secondary) {
			secondary->mouseOver_ = false;
		}
		if (tertiary) {
			tertiary->mouseOver_ = false;
		}
	}

	void setSecondary(cShardLabel *data) { secondary = data; }
	void setTertiary(cShardLabel *data) { tertiary = data; }

	// Do a hit test on the underlying text surface
	cControl *getControl(int x, int y) {
		cControl *result = cTextField::getControl(x, y);

		if (result) {
			cTexture *texture = surfaces[0];

			x -= 7;
			if (texture && x >= 0 && y >= 0 && (unsigned int)x < texture->realWidth() && (unsigned int)y < texture->realHeight()) {
				return this;
			}
		}

		return 0;
	}

	void onMouseDown(QMouseEvent *e) {
		// Select this shard
		if (e->button() == Qt::LeftButton) {
			LoginDialog->selectShard(id);
		}
	}
};

class cCharSelection : public cContainer {
protected:
	QStringList characters;
	Q3ValueVector<cAsciiLabel*> labels;
	int selectedCharacter_;
public:
	cCharSelection();
	void setCharacters(const QStringList &characters);
	void onMouseDown(QMouseEvent *e);
	cControl *getControl(int x, int y);
	int selectedCharacter() const;
};

cCharSelection::cCharSelection() {
	selectedCharacter_ = -1;
}

inline int cCharSelection::selectedCharacter() const {
	return selectedCharacter_;
}

void cCharSelection::setCharacters(const QStringList &characters) {
	selectedCharacter_ = -1;
	controls.clear();
	cContainer::clear(); // Clear all controls
	labels.clear();

	// Re-create the characterlist
	for (int i = 0; i < characters.size(); ++i) {
		cAsciiLabel *label = new cAsciiLabel(characters[i].toLatin1(), 5, 0x34f);
		label->setHueAll(true);
		label->update(); // Get width/height for it
		label->setPosition(64 + (287 - label->width()) / 2, 80 + 40 * i + 2);		
		labels.append(label);
		addControl(label);
	}
}

void cCharSelection::onMouseDown(QMouseEvent *e) {
	int selected = -1;

	// Which control is under the mouse
	QPoint local = mapFromGlobal(e->pos());
	if (local.x() > 64 && local.x() < 64 + 287) {
        int id = (local.y() - 80) / 40;
        int offset = (local.y() - 80) % 40;

		if (id >= 0 && id < labels.size() && offset < 31) {
			selected = id;
		}
	}

	// selection changed
	if (selected > -1) {
		for (int i = 0; i < labels.size(); ++i) {
			if (i == selected) {
				labels[i]->setHue(0x23);
			} else {
				labels[i]->setHue(0x34f);
			}
		}

		selectedCharacter_ = selected;
	}
}

cControl *cCharSelection::getControl(int x, int y) {
	cControl *control = 0;

	// Which control is under the mouse	
	if (x > 64 && x < 64 + 287) {
        int id = (y - 80) / 40;
        int offset = (y - 80) % 40;

		if (id >= 0 && id < labels.size() && offset < 31) {
			control = labels[id];
		}
	}
	if (control != 0 && control != this) {
		return this; // be greedy
	} else {
		return 0; // Don't hog
	}
}

cLoginDialog::cLoginDialog() {
	errorStatus = false;
	confirmDeleteText = 0;
	statusCancel = 0;
	statusOk = 0;
	charSelectWidget = 0;
	container = 0;
	confirmDeleteDialog = 0;
	movieButton = 0;
	accountLoginGump = 0;
	shardSelectGump = 0;
	inpAccount = 0;
	inpPassword = 0;
	shardEntryOffset = 0;
	statusDialog = 0;
	statusLabel = 0;
	selectCharDialog = 0;
	page = PAGE_LOGIN;

	// Connect to the UoSocket slots
	connect(UoSocket, SIGNAL(onConnect()), this, SLOT(socketConnect()));
	connect(UoSocket, SIGNAL(onHostFound()), this, SLOT(socketHostFound()));
	connect(UoSocket, SIGNAL(onError(const QString&)), this, SLOT(socketError(const QString&)));
}

cLoginDialog::~cLoginDialog() {
}

void cLoginDialog::backClicked(cControl *sender) {
	switch (page) {
		case PAGE_SHARDLIST:
		case PAGE_SELECTCHAR:
			UoSocket->disconnect();
			show(PAGE_LOGIN);
			break;
	}
}

void cLoginDialog::nextClicked(cControl *sender) {
	switch (page) {
		// Initiate the login
		case PAGE_LOGIN:
			// Save username in the configuration
			Config->setLastUsername(inpAccount->text());
			UoSocket->connect(Config->loginHost(), Config->loginPort(), 0);
			show(PAGE_CONNECTING);
			break;

		case PAGE_SHARDLIST:
			break;

		case PAGE_SELECTCHAR:
			if (charSelectWidget->selectedCharacter() != -1) {
				charSelected(sender);
			}
			break;
	}
}

static void shardlistScrolled(cVerticalScrollBar *scrollbar, int oldpos) {
	LoginDialog->onScrollShardList(oldpos, scrollbar->pos());
}

void cLoginDialog::onScrollShardList(int oldpos, int newpos) {
	int change = (newpos - oldpos) * 5;
	cContainer::Controls controls = shardList->getControls();
	for (cContainer::Iterator it = controls.begin(); it != controls.end(); ++it) {
		(*it)->setPosition((*it)->x(), (*it)->y() - change);
	}
}

void cLoginDialog::setStatusText(const QString &text) {
	statusLabel->setText(text.latin1());
}

void cLoginDialog::buildShardSelectGump() {
	if (!shardSelectGump) {
		shardSelectGump = new cContainer();
		shardSelectGump->setBounds(0, 0, 640, 480);

		cBorderGump *bordergump;
		cLabel *label;
		cImageButton *button;
		cGumpImage *image;

		// Select which shard to play on
		label = new cLabel(tr("Select which shard to play on:"), 1, 0x3e4, false);
		label->setPosition(0x9e, 0x48);
		shardSelectGump->addControl(label);

		// Latency:
		label = new cLabel(tr("Latency:"), 1, 0x3e4, false);
		label->setPosition(0x193, 0x48);
		shardSelectGump->addControl(label);

		// Packet Loss:
		label = new cLabel(tr("Packet Loss:"), 1, 0x3e4, false);
		label->setPosition(0x1d9, 0x48);
		shardSelectGump->addControl(label);

		// World "Background"
		image = new cGumpImage(0x589);
		image->setPosition(0x96, 0x186);
		shardSelectGump->addControl(image);

		// Old Shard?
		lastShardName = new cAsciiLabel("", 9, 0x481);
		lastShardName->setPosition(243, 415);
		shardSelectGump->addControl(lastShardName);

		// World Button
		button = new cImageButton(0xa0, 0x190, 0x15e8, 0x15ea);
		button->setStateGump(BS_HOVER, 0x15e9);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(selectLastShard()));
		shardSelectGump->addControl(button);

		cContainer *container = new cContainer(); // The shard selection scrollbox
		container->setBounds(0x96, 0x5a, 0x190, 0x130);
		shardSelectGump->addControl(container);

		// Background for the shard list
		bordergump = new cBorderGump(0xdac, 0);
		bordergump->setBounds(0, 0, 380, 271);
		container->addControl(bordergump);

		// The Scrollbar for the Shardlist
		cVerticalScrollBar *scrollbar = new cVerticalScrollBar(380, 1, 270);
		scrollbar->setRange(0, 9);
		scrollbar->setScrollCallback(shardlistScrolled);
		container->addControl(scrollbar);

		// This is unused at the moment, so don't show it
		/*label = new cLabel(tr("Sort By:"), 1, 0x3e4, false);
		label->setPosition(3, 278);
		container->addControl(label);

		// Timezone button
		button = new cImageButton(0x50, 276, 0x93b, 0x93c);
		button->setStateGump(BS_HOVER, 0x93d);
		container->addControl(button);

		// %Full button
		button = new cImageButton(0xbc, 276, 0x93e, 0x93f);
		button->setStateGump(BS_HOVER, 0x940);
		container->addControl(button);

		// Ping button
		button = new cImageButton(0x128, 276, 0x941, 0x942);
		button->setStateGump(BS_HOVER, 0x943);
		container->addControl(button);*/

		// The shardlist container
		shardList = new cContainer();
		shardList->setBounds(0, 10, 400, 284);
		container->addControl(shardList);
	}
}

void cLoginDialog::buildAccountLoginGump() {
	if (!accountLoginGump) {
		cContainer *container = new cContainer();
		container->setBounds(0, 0, 640, 480);

		// Account Background Gump
		cBorderGump *background = new cBorderGump(0x13be);
		background->setBounds(128, 288, 451, 157);
		container->addControl(background);

		// Castle Image
		cGumpImage *image;
		image = new cGumpImage(0x58a);
		image->setPosition(286, 45);
		container->addControl(image);

		cAsciiLabel *label;
		label = new cAsciiLabel(tr("Log in to Ultima Online").latin1(), 2, 0x34f);
		label->setPosition(0xfa, 0x12f);
		container->addControl(label);

		label = new cAsciiLabel(tr("Wolfpack UO Client Version " CLIENT_VERSION).latin1(), 9, 0x34f, ALIGN_LEFT, true, true);
		label->setPosition(0xc8, 0x1a3);
		container->addControl(label);

		label = new cAsciiLabel(tr((char)0xa9 + QString(" 2005 Wolfpack Development Team")).latin1(), 6, 0x481, ALIGN_LEFT, true, true);
		label->setPosition(0xc8, 0x1c5);
		container->addControl(label);

		label = new cAsciiLabel(tr("Account Name").latin1(), 2, 0x34f);
		label->setPosition(0xb4, 0x157);
		container->addControl(label);

		inpAccount = new cTextField(0x148, 0x157, 0xd2, 0x1e, 5, 0x34f, 3000, true);
		inpAccount->setMouseOverHue(0x2b8);
		inpAccount->setFocusHue(0x23);
		inpAccount->setMaxLength(16);
		inpAccount->setText(Config->lastUsername().toLatin1());
		container->addControl(inpAccount);
		connect(inpAccount, SIGNAL(enterPressed(cTextField*)), this, SLOT(enterPressed(cTextField*)));

		label = new cAsciiLabel(tr("Password").latin1(), 2, 0x34f);
		label->setPosition(0xb4, 0x17f);
		container->addControl(label);

		inpPassword = new cTextField(0x148, 0x17f, 0xd2, 0x1e, 5, 0x34f, 3000, true);
		inpPassword->setPassword(true);
		inpPassword->setMaxLength(16);
		inpPassword->setMouseOverHue(0x2b8);
		inpPassword->setFocusHue(0x23);
		container->addControl(inpPassword);
		connect(inpPassword, SIGNAL(enterPressed(cTextField*)), this, SLOT(enterPressed(cTextField*)));

		accountLoginGump = container;
	}
}

void cLoginDialog::enterPressed(cTextField *field) {
	// If we press enter in the account field, set focus to the password field
	if (field == inpAccount) {
		Gui->setInputFocus(inpPassword);
	} else if (field == inpPassword) {
		nextClicked(nextButton); 
	}
}

void cLoginDialog::buildStatusGump() {
	if (!statusDialog) {
		statusDialog = new cContainer();
		statusDialog->setBounds(0, 0, 640, 480);

		cBorderGump *background = new cBorderGump(0xa28);
		background->setBounds(116, 96, 408, 288);
		statusDialog->addControl(background);

		statusLabel = new cAsciiLabel("Status", 2, 0x34f, ALIGN_CENTER, false);
		statusLabel->setBounds(116, 150, 408, 200);
		statusDialog->addControl(statusLabel);

		statusCancel = new cImageButton(306, 296, 0x47e, 0x480);
		statusCancel->setStateGump(BS_HOVER, 0x47f);
		statusDialog->addControl(statusCancel);
		connect(statusCancel, SIGNAL(onButtonPress(cControl*)), this, SLOT(statusCancelClicked(cControl*)));

		statusOk = new cImageButton(306, 296, 0x481, 0x483);
		statusOk->setStateGump(BS_HOVER, 0x482);
		statusDialog->addControl(statusOk);
		connect(statusOk, SIGNAL(onButtonPress(cControl*)), this, SLOT(statusOkClicked(cControl*)));
	}
}

void cLoginDialog::show(enMenuPage page) {
	setErrorStatus(false); // Reset error status when changing pages

	MainWindow *mainWindow = (MainWindow*)qApp->mainWidget();
	if (mainWindow) {
		mainWindow->resizeGameWindow(640, 480, true);
	}

	if (!container) {
		container = new cWindow();
		container->setClosable(false);
		container->setMovable(false);
		container->setAlign(CA_CLIENT);

		cTiledGumpImage *tiledgump = new cTiledGumpImage(0xe14);
		tiledgump->setAlign(CA_CLIENT);
		container->addControl(tiledgump);

		cGumpImage *image; // Pointer to new gump images
		cImageButton *button; // Pointer for new buttons

		// BIG BORDER (UPPER LEFT)
		image = new cGumpImage(0x157c);
		container->addControl(image);

		// UO BANNER (Upper Left)
		image = new cGumpImage(0x15a0);
		image->setPosition(0, 4);
		container->addControl(image);

		// Quit Button
		button = new cImageButton(555, 4, 0x1589, 0x158B);
		button->setStateGump(BS_HOVER, 0x158A);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(quitClicked(cControl*)));
		container->addControl(button);

		// MyUO Button
		button = new cImageButton(14, 146, 0x158f, 0x1591);
		button->setStateGump(BS_HOVER, 0x1590);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(myUoClicked(cControl*)));
		container->addControl(button);

		// Account Button
		button = new cImageButton(14, 205, 0x1592, 0x1594);
		button->setStateGump(BS_HOVER, 0x1593);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(accountClicked(cControl*)));
		container->addControl(button);

		// Movie Button
		movieButton = new cImageButton(14, 306, 0x1586, 0x1588);
		movieButton->setStateGump(BS_HOVER, 0x1587);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(movieClicked(cControl*)));
		container->addControl(movieButton);

		// Credits Button
		button = new cImageButton(14, 330, 0x1583, 0x1585);
		button->setStateGump(BS_HOVER, 0x1584);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(creditsClicked(cControl*)));
		container->addControl(button);

		// Help Button
		button = new cImageButton(14, 354, 0x1595, 0x1597);
		button->setStateGump(BS_HOVER, 0x1596);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(helpClicked(cControl*)));
		container->addControl(button);

		// Teen Logo (only on english systems)
		/*image = new cGumpImage(0x2335);
		image->setPosition(0x32, 0x17f);
		container->addControl(image);*/
		if (!accountLoginGump) {
			buildAccountLoginGump();
			container->addControl(accountLoginGump);
			accountLoginGump->setVisible(false);
		}

		if (!shardSelectGump) {
			buildShardSelectGump();
			container->addControl(shardSelectGump);
			shardSelectGump->setVisible(false);
		}
	
		if (!statusDialog) {
			buildStatusGump();
			container->addControl(statusDialog);
			statusDialog->setVisible(false);
		}

		if (!selectCharDialog) {
			buildSelectCharGump();
			container->addControl(selectCharDialog);
			selectCharDialog->setVisible(false);
		}

		if (!confirmDeleteDialog) {
			buildConfirmDeleteGump();
			container->addControl(confirmDeleteDialog);
			confirmDeleteDialog->setVisible(false);
		}

		// Back Button
		backButton = new cImageButton(586, 445, 0x15a1, 0x15a3);
		backButton->setStateGump(BS_HOVER, 0x15a2);
		connect(backButton, SIGNAL(onButtonPress(cControl*)), this, SLOT(backClicked(cControl*)));
		container->addControl(backButton);

		// Next Button
		nextButton = new cImageButton(610, 445, 0x15a4, 0x15a6);
		nextButton->setStateGump(BS_HOVER, 0x15a5);
		connect(nextButton, SIGNAL(onButtonPress(cControl*)), this, SLOT(nextClicked(cControl*)));
		container->addControl(nextButton);
	}

	if (!container->parent()) {
		Gui->addControl(container);
		Gui->invalidate();
	}

	// First hide all others
	switch (this->page) {
		case PAGE_LOGIN:
			accountLoginGump->setVisible(false);
			movieButton->setVisible(false);
			nextButton->setVisible(false);
			backButton->setVisible(false);
			break;
		case PAGE_SHARDLIST:
			shardSelectGump->setVisible(false);
			nextButton->setVisible(false);
			backButton->setVisible(false);
			break;

		case PAGE_SELECTCHAR:
			selectCharDialog->setVisible(false);
			nextButton->setVisible(false);
			backButton->setVisible(false);
			break;

		// Status Informations
		case PAGE_VERIFYING:
		case PAGE_CONNECTING:
		case PAGE_DELETING:
		case PAGE_ENTERING:
			statusDialog->setVisible(false);
			nextButton->setVisible(false);
			backButton->setVisible(false);
			break;

		case PAGE_CONFIRMDELETE:
			confirmDeleteDialog->setVisible(false);
			break;
	}

	// Which one to display?
	switch (page) {
		case PAGE_LOGIN:
			accountLoginGump->setVisible(true);
			movieButton->setVisible(true);
			nextButton->setVisible(true);

			// Set Focus to the Account control if empty. Otherwise
			// to the password control
			if (inpAccount->text().isEmpty()) {
				Gui->setInputFocus(inpAccount);
			} else {
				Gui->setInputFocus(inpPassword);
			}

			break;
		case PAGE_SHARDLIST:
			shardSelectGump->setVisible(true);
			nextButton->setVisible(true);
			backButton->setVisible(true);
			break;
		case PAGE_CONNECTING:
			statusLabel->setText("Connecting...");
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(true);
			break;
		case PAGE_VERIFYING:
			statusLabel->setText("Verifying Account...");
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(true);
			break;
		case PAGE_SELECTCHAR:
			selectCharDialog->setVisible(true);
			nextButton->setVisible(true);
			backButton->setVisible(true);
			break;
		case PAGE_CONFIRMDELETE:
			confirmDeleteDialog->setVisible(true);
			nextButton->setVisible(false);
			backButton->setVisible(false);
			break;
		case PAGE_DELETING:
			statusLabel->setText(tr("Deleting character...").toLatin1());
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(false);
			break;
		case PAGE_ENTERING:
			statusLabel->setText(tr("Entering Britannia...").toLatin1());
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(false);
			break;
	}

	this->page = page;
}

void cLoginDialog::buildConfirmDeleteGump() {
	confirmDeleteDialog = new cContainer();
	confirmDeleteDialog->setBounds(0, 0, 640, 480);

	cBorderGump *background = new cBorderGump(0xa28);
	background->setBounds(166, 96, 308, 188);
	confirmDeleteDialog->addControl(background);

	confirmDeleteText = new cAsciiLabel("Status", 2, 0x34f, ALIGN_CENTER, false);
	confirmDeleteText->setBounds(116, 150, 408, 200);
	confirmDeleteDialog->addControl(confirmDeleteText);

	cImageButton *statusCancel = new cImageButton(356, 225, 0x47e, 0x480);
	statusCancel->setStateGump(BS_HOVER, 0x47f);
	confirmDeleteDialog->addControl(statusCancel);
	connect(statusCancel, SIGNAL(onButtonPress(cControl*)), this, SLOT(statusCancelClicked(cControl*)));

	cImageButton *statusOk = new cImageButton(256, 225, 0x481, 0x483);
	statusOk->setStateGump(BS_HOVER, 0x482);
	confirmDeleteDialog->addControl(statusOk);
	connect(statusOk, SIGNAL(onButtonPress(cControl*)), this, SLOT(statusOkClicked(cControl*)));
}

void cLoginDialog::buildSelectCharGump() {
	if (!selectCharDialog) {
		selectCharDialog = new cContainer();
		selectCharDialog->setBounds(160, 70, 408, 388);

		// Background
		cBorderGump *border = new cBorderGump(0xa28);
		border->setAlign(CA_CLIENT);
		selectCharDialog->addControl(border);

		// Top Label
		cAsciiLabel *topLabel = new cAsciiLabel(tr("Character Selection").latin1(), 2, 0x34f, ALIGN_CENTER, false);
		topLabel->setBounds(0, 40, 408, 200);
		selectCharDialog->addControl(topLabel);

		cImageButton *button;

		// New Character
        button = new cImageButton(64, 327, 0x159d, 0x159f);
		button->setStateGump(BS_HOVER, 0x159e);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(createCharClicked(cControl*)));
		selectCharDialog->addControl(button);

		// Delete Character
        button = new cImageButton(282, 327, 0x159a, 0x159c);
		button->setStateGump(BS_HOVER, 0x159b);
		connect(button, SIGNAL(onButtonPress(cControl*)), this, SLOT(deleteCharClicked(cControl*)));
		selectCharDialog->addControl(button);

		// add 6 char selection backgrounds
		for (int i = 0; i < 6; ++i) {
			selectCharBorder[i] = new cBorderGump(3000);
			selectCharBorder[i]->setBounds(64, 80 + 40 * i, 287, 31);
			selectCharDialog->addControl(selectCharBorder[i]);
		}

		// LAST: Char select widget
		charSelectWidget = new cCharSelection;
		charSelectWidget->setAlign(CA_CLIENT);
		selectCharDialog->addControl(charSelectWidget);
		connect(charSelectWidget, SIGNAL(onDoubleClick(cControl*)), this, SLOT(charSelected(cControl*)));
	}
}

void cLoginDialog::deleteCharClicked(cControl *sender) {
	int selected = charSelectWidget->selectedCharacter();
    
	if (selected != -1) {
		// TODO: Show the Confirm Dialog
		confirmDeleteText->setText(tr("Permanently delete\n%1?").arg(characterNames[selected]).toLatin1());
		show(PAGE_CONFIRMDELETE);
	}
}

void cLoginDialog::createCharClicked(cControl *sender) {
}

void cLoginDialog::hide() {
	setErrorStatus(false); // Reset error status when changing pages

	if (container->parent() == Gui) {
		Gui->removeControl(container);
		Gui->invalidate();
	}

	MainWindow *mainWindow = (MainWindow*)qApp->mainWidget();
	if (mainWindow) {
		// Resize the game window to the original size
		mainWindow->resizeGameWindow(Config->engineWidth(), Config->engineHeight(), false);

		// Is there a configuration setting for the window position?
		if (Config->engineWindowX() != -1 && Config->engineWindowY() != -1) {
			/*
				Check if the main window would still be visible if we use the given coordinates.
				Example: The config file contains the coordinates x=1024,y=768 for the window.
				It would be completely invisible at a 1024x768 resolution.
			*/	
			QRect geom = qApp->desktop()->screenGeometry(mainWindow);			
			if (Config->engineWindowX() < geom.width() && Config->engineWindowY() < geom.height()) {
				mainWindow->move(Config->engineWindowX(), Config->engineWindowY());
			}
		}

		// Maximize the game window if neccesary
		if (Config->engineMaximized()) {
			mainWindow->showMaximized();
		}
	}
}

void cLoginDialog::clearShardList() {
	shardEntryOffset = 0;
	shards.clear();
	// Clear out the shardlist container
	cContainer::Controls shardEntries = shardList->getControls();
	for (cContainer::Iterator it = shardEntries.begin(); it != shardEntries.end(); ++it) {
		shardList->removeControl(*it);
		delete *it;
	}
}

/*
	Open Browser for http://support.uo.com/
*/
void cLoginDialog::helpClicked(cControl *sender) {
	Utilities::launchBrowser("http://support.uo.com");
}

/*
	Open Browser for http://ultima-registration.com
*/
void cLoginDialog::accountClicked(cControl *sender) {
	Utilities::launchBrowser("http://ultima-registration.com");
}

void cLoginDialog::quitClicked(cControl *sender) {
	Client->quit();
}

/*
	Open Browser for http://my.uo.com/
*/
void cLoginDialog::myUoClicked(cControl *sender) {
	Utilities::launchBrowser("http://my.uo.com");
}

void cLoginDialog::creditsClicked(cControl *sender) {
}

void cLoginDialog::movieClicked(cControl *sender) {
	Utilities::messageBox("What should we do here...");
}

void cLoginDialog::selectLastShard() {
	for (int i = 0; i < shards.size(); ++i) {
		if (shards[i].id == Config->lastShardId()) {
			cRequestRelayPacket packet(shards[i].id);
			UoSocket->send(packet);
			break;
		}
	}
}

void cLoginDialog::addShard(const stShardEntry &shard) {
	// add the shard entry and also add a control for the shard
	shards.append(shard);

	cShardLabel *shardLabel, *pingCount, *packetLoss;

	shardLabel = new cShardLabel(67, shardEntryOffset, shard.name.toLatin1());
	shardList->addControl(shardLabel);

	pingCount = new cShardLabel(67 + 186, shardEntryOffset, "-");
	shardList->addControl(pingCount);

	packetLoss = new cShardLabel(67 + 186 + 60, shardEntryOffset, "-");
	shardList->addControl(packetLoss);

	// Set Secondary + Tertiary
	shardLabel->setSecondary(pingCount);
	shardLabel->setTertiary(packetLoss);
	shardLabel->setId(shards.size() - 1);
	pingCount->setSecondary(shardLabel);
	pingCount->setTertiary(packetLoss);
	pingCount->setId(shards.size() - 1);
	packetLoss->setSecondary(shardLabel);
	packetLoss->setTertiary(pingCount);
	packetLoss->setId(shards.size() - 1);

	if (shard.id == Config->lastShardId()) {
		lastShardName->setText(shard.name.toLatin1());
	}

	shardEntryOffset += 25;
}

void cLoginDialog::socketError(const QString &error) {
	statusLabel->setText(error.latin1());
}

void cLoginDialog::socketHostFound() {
}

void cLoginDialog::socketConnect() {
	// If we're connecting to a loginserver
	if (!UoSocket->isGameServer()) {
		// Send Login Packet
		show(PAGE_VERIFYING);

		cLoginPacket packet(inpAccount->text(), inpPassword->text());
		UoSocket->send(packet);
	} else {
		cGameLoginPacket packet(UoSocket->seed(), inpAccount->text(), inpPassword->text());
		UoSocket->send(packet);
	}
}

void cLoginDialog::selectShard(int id) {
	if (id >= 0 && id < (int)shards.size()) {
		cRequestRelayPacket packet(shards[id].id);
		UoSocket->send(packet);
		Config->setLastShardId(shards[id].id);
	}
}

void cLoginDialog::setCharacterList(const QStringList &characters) {
	charSelectWidget->setCharacters(characters);
	characterNames = characters;

	for (int i = 0; i < 6; ++i) {
		selectCharBorder[i]->setVisible(i < characters.size());
	}
}

void cLoginDialog::setErrorStatus(bool error) {
	switch (page) {
		case PAGE_VERIFYING:
		case PAGE_CONNECTING:
			if (statusCancel) {
				statusCancel->setVisible(!error);
			}
			if (statusOk) {
				statusOk->setVisible(error);
			}
			break;
	}
}

void cLoginDialog::statusCancelClicked(cControl *sender) {
	switch (page) {
		case PAGE_VERIFYING:
		case PAGE_CONNECTING:
			UoSocket->disconnect();
			show(PAGE_LOGIN);
			break;
		case PAGE_CONFIRMDELETE:
			show(PAGE_SELECTCHAR);
			break;
	}
}

void cLoginDialog::statusOkClicked(cControl *sender) {
	switch (page) {
		case PAGE_VERIFYING:
		case PAGE_CONNECTING:
			UoSocket->disconnect();
			show(PAGE_LOGIN);
			break;
		case PAGE_CONFIRMDELETE:
			{
				show(PAGE_DELETING);
				cDeleteCharacter delchar(charSelectWidget->selectedCharacter());
				UoSocket->send(delchar);				
			}
			break;
	}	
}

void cLoginDialog::charSelected(cControl *sender) {
	int selected = charSelectWidget->selectedCharacter();

	show(PAGE_ENTERING);
	cPlayMobilePacket packet(selected);
	UoSocket->send(packet);
}

cLoginDialog *LoginDialog = 0;
