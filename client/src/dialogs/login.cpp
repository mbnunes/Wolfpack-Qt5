
#include <QApplication>
#include <QMouseEvent>
#include <QDesktopWidget>

#include "config.h"
#include "dialogs/login.h"
#include "gui/gui.h"
#include "enums.h"
#include "client.h"
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
	cShardLabel(int x, int y, const QString &text) : cTextField(x, y, 350, 25, 5, 0x34f, 0, true) {
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
	QVector<cAsciiLabel*> labels;
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
	newCharacter1 = 0;
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
		case PAGE_NEWCHARACTER1:
			show(PAGE_SELECTCHAR);
			break;
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

		case PAGE_NEWCHARACTER1:
			{
				cTextField *textfield = dynamic_cast<cTextField*>(newCharacter1->findByName("character_name"));
				if (textfield) {
					cCharacterCreationInfo info;
					info.name = textfield->text();
					UoSocket->send(cCharacterCreationPacket(info));
					show(PAGE_ENTERING);
				} else {
					show(PAGE_SELECTCHAR);
				}
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
	statusLabel->setText(text);
}

void cLoginDialog::buildShardSelectGump() {
}

void cLoginDialog::buildAccountLoginGump() {
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
}

void cLoginDialog::show(enMenuPage page) {
	setErrorStatus(false); // Reset error status when changing pages

	if (!container) {
		container = Gui->createDialog("LoginDialog");

		// Connect the buttons on the gump		
		connect(container->findByName("QuitButton"), SIGNAL(onButtonPress(cControl*)), MainWindow, SLOT(close()));
		connect(container->findByName("MyUOButton"), SIGNAL(onButtonPress(cControl*)), SLOT(myUoClicked(cControl*)));
		connect(container->findByName("AccountButton"), SIGNAL(onButtonPress(cControl*)), SLOT(accountClicked(cControl*)));
		movieButton = dynamic_cast<cImageButton*>(container->findByName("MovieButton"));
		connect(movieButton, SIGNAL(onButtonPress(cControl*)), SLOT(movieClicked(cControl*)));
		connect(container->findByName("CreditsButton"), SIGNAL(onButtonPress(cControl*)), SLOT(creditsClicked(cControl*)));
		connect(container->findByName("HelpButton"), SIGNAL(onButtonPress(cControl*)), SLOT(helpClicked(cControl*)));
		
		backButton = dynamic_cast<cImageButton*>(container->findByName("BackButton"));
		connect(backButton, SIGNAL(onButtonPress(cControl*)), SLOT(backClicked(cControl*)));
		nextButton = dynamic_cast<cImageButton*>(container->findByName("NextButton"));
		connect(nextButton, SIGNAL(onButtonPress(cControl*)), SLOT(nextClicked(cControl*)));
				
		accountLoginGump = dynamic_cast<cContainer*>(container->findByName("AccountLoginContainer"));

		// Show the current client version
		cAsciiLabel *clientVersion = dynamic_cast<cAsciiLabel*>(accountLoginGump->findByName("ClientVersionLabel"));
		if (clientVersion) {
			clientVersion->setText(tr("Wolfpack UO Client Version " CLIENT_VERSION));
		}

		// Set default text for the account input field
		inpAccount = dynamic_cast<cTextField*>(accountLoginGump->findByName("AccountTextfield"));
		inpAccount->setText(Config->lastUsername());
		connect(inpAccount, SIGNAL(enterPressed(cTextField*)), this, SLOT(enterPressed(cTextField*)));		

		// Connect password enter to login button
		inpPassword = dynamic_cast<cTextField*>(accountLoginGump->findByName("PasswordTextfield"));
		connect(inpPassword, SIGNAL(enterPressed(cTextField*)), this, SLOT(enterPressed(cTextField*)));

		// Shard selection gump
		shardSelectGump = dynamic_cast<cContainer*>(container->findByName("ShardSelectionContainer"));

		// Connect the last shard button and store the last shard label
		lastShardName = dynamic_cast<cAsciiLabel*>(shardSelectGump->findByName("LastShardLabel"));
		connect(shardSelectGump->findByName("LastShardButton"), SIGNAL(onButtonPress(cControl*)), this, SLOT(selectLastShard()));

		// Get the shardlist and set up the scrollbar
		cContainer *shardSelectionContainer = dynamic_cast<cContainer*>(shardSelectGump->findByName("ScrollableShardList"));
		cVerticalScrollBar *scrollbar = new cVerticalScrollBar(380, 1, 270);
		scrollbar->setRange(0, 9);
		scrollbar->setScrollCallback(shardlistScrolled);
		shardSelectionContainer->addControl(scrollbar);

		// Set up the list of shard entries
		shardList = dynamic_cast<cContainer*>(shardSelectGump->findByName("ShardListContainer"));

		// Set up the Status Dialog
		statusDialog = dynamic_cast<cContainer*>(container->findByName("StatusDialogContainer"));
		statusLabel = dynamic_cast<cAsciiLabel*>(statusDialog->findByName("StatusLabel"));
		statusOk = dynamic_cast<cImageButton*>(statusDialog->findByName("StatusOkButton"));
		statusCancel = dynamic_cast<cImageButton*>(statusDialog->findByName("StatusCancelButton"));
		connect(statusCancel, SIGNAL(onButtonPress(cControl*)), this, SLOT(statusCancelClicked(cControl*)));
		connect(statusOk, SIGNAL(onButtonPress(cControl*)), this, SLOT(statusOkClicked(cControl*)));

		// Set up the character selection dialog
		selectCharDialog = dynamic_cast<cContainer*>(container->findByName("CharacterSelectContainer"));
		connect(selectCharDialog->findByName("NewCharacterButton"), SIGNAL(onButtonPress(cControl*)), this, SLOT(createCharClicked(cControl*)));
		connect(selectCharDialog->findByName("DeleteCharacterButton"), SIGNAL(onButtonPress(cControl*)), this, SLOT(deleteCharClicked(cControl*)));

		for (int i = 0; i < 6; ++i) {
			selectCharBorder[i] = container->findByName(QString("CharacterNameBackground%1").arg(i));
		}

		charSelectWidget = new cCharSelection;
		charSelectWidget->setAlign(CA_CLIENT);
		selectCharDialog->addControl(charSelectWidget);
		connect(charSelectWidget, SIGNAL(onDoubleClick(cControl*)), this, SLOT(charSelected(cControl*)));
		/*
		if (!confirmDeleteDialog) {
			buildConfirmDeleteGump();
			container->addControl(confirmDeleteDialog);
			confirmDeleteDialog->setVisible(false);
		}*/
	}

	if (!container->parent()) {
		Gui->addControl(container);
	}

	if (MainWindow) {
		MainWindow->resizeGameWindow(container->width(), container->height(), true);
	}

	// First hide all others
	switch (this->page) {
		case PAGE_LOGIN:
			accountLoginGump->setVisible(false);
			if (movieButton) {
				movieButton->setVisible(false);
			}
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

		case PAGE_NEWCHARACTER1:
			newCharacter1->setVisible(false);
			backButton->setVisible(false);
			nextButton->setVisible(false);
			break;
	}

	// Which one to display?
	switch (page) {
		case PAGE_LOGIN:
			accountLoginGump->setVisible(true);
			if (movieButton) {
				movieButton->setVisible(true);
			}
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
		case PAGE_NEWCHARACTER1:
			if (!newCharacter1) {
				buildNewCharacter1();
			}
			newCharacter1->setVisible(true);
			backButton->setVisible(true);
			nextButton->setVisible(true);
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
	if (characterNames.size() < 5) {
		show(PAGE_NEWCHARACTER1);
	}
}

void cLoginDialog::hide() {
	setErrorStatus(false); // Reset error status when changing pages

	if (container->parent() == Gui) {
		Gui->removeControl(container);
	}

	if (MainWindow) {
		// Resize the game window to the original size
		MainWindow->resizeGameWindow(Config->engineWidth(), Config->engineHeight(), false);

		// Is there a configuration setting for the window position?
		if (Config->engineWindowX() != -1 && Config->engineWindowY() != -1) {
			/*
				Check if the main window would still be visible if we use the given coordinates.
				Example: The config file contains the coordinates x=1024,y=768 for the window.
				It would be completely invisible at a 1024x768 resolution.
			*/	
			QRect geom = qApp->desktop()->screenGeometry(MainWindow);			
			if (Config->engineWindowX() < geom.width() && Config->engineWindowY() < geom.height()) {
				MainWindow->move(Config->engineWindowX(), Config->engineWindowY());
			}
		}

		// Maximize the game window if neccesary
		if (Config->engineMaximized()) {
			MainWindow->showMaximized();
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
	statusLabel->setText(error);
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
    
	if (selected != -1) {
		show(PAGE_ENTERING);
		cPlayMobilePacket packet(selected);
		UoSocket->send(packet);
	}
}

void cLoginDialog::buildNewCharacter1() {
	if (!newCharacter1) {
		newCharacter1 = new cContainer();
		newCharacter1->setVisible(false);
		newCharacter1->setBounds(0, 0, 640, 480);

		cGumpImage *image;
		cBorderGump *background;
		cLabel *label;
		cTextField *textfield;

		background = new cBorderGump(0xa28);
		background->setBounds(0x64, 0x50, 0x1d6, 0x174);
		newCharacter1->addControl(background);

		image = new cGumpImage(0x58b);
		image->setPosition(0xd5, 0x39);
        newCharacter1->addControl(image);

		image = new cGumpImage(0x589);
		image->setPosition(0x122, 0x2c);
        newCharacter1->addControl(image);

		image = new cGumpImage(0x15aa);
		image->setPosition(0x12b, 0x35);
        newCharacter1->addControl(image);

		label = new cLabel(tr("Character name:"), 0);
		label->setPosition(0x80, 0x100);
		newCharacter1->addControl(label);

		textfield = new cTextField(0x150, 0x100, 150, 25, 3, 0x3b2, 3000, true, false);
		textfield->setObjectName("character_name");
		newCharacter1->addControl(textfield);
		
		container->addControl(newCharacter1);
	}
}

cLoginDialog *LoginDialog = 0;
