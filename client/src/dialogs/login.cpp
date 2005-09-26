
#include <Python.h>

#include <QApplication>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QMessageBox>

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
#include "muls/localization.h"
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
		case PAGE_NEWCHARACTER:
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
	}
}

void cLoginDialog::shardlistScrolled(int oldpos) {
	cVerticalScrollbar *scrollbar = dynamic_cast<cVerticalScrollbar*>(container->findByName("ShardlistScrollbar"));
	if (scrollbar) {
		int change = (scrollbar->pos() - oldpos) * 5;
		cContainer::Controls controls = shardList->getControls();
		for (cContainer::Iterator it = controls.begin(); it != controls.end(); ++it) {
			(*it)->setPosition((*it)->x(), (*it)->y() - change);
		}
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

		if (!container) {
			QMessageBox::critical(MainWindow, "Error", "Unable to create login dialog. Check dialogs/login.xml for errors.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			MainWindow->close();
			return;
		}

		// Connect the buttons on the gump		
		connect(container->findByName("QuitButton"), SIGNAL(onButtonPress(cControl*)), MainWindow, SLOT(close()));
		connect(container->findByName("MyUOButton"), SIGNAL(onButtonPress(cControl*)), SLOT(myUoClicked(cControl*)));
		connect(container->findByName("AccountButton"), SIGNAL(onButtonPress(cControl*)), SLOT(accountClicked(cControl*)));
		movieButton = dynamic_cast<cImageButton*>(container->findByName("MovieButton"));
		connect(movieButton, SIGNAL(onButtonPress(cControl*)), SLOT(movieClicked(cControl*)));
		connect(container->findByName("CreditsButton"), SIGNAL(onButtonPress(cControl*)), SLOT(creditsClicked(cControl*)));
		connect(container->findByName("HelpButton"), SIGNAL(onButtonPress(cControl*)), SLOT(helpClicked(cControl*)));
		
		backButton = dynamic_cast<cImageButton*>(container->findByName("BackButton", false));
		connect(backButton, SIGNAL(onButtonPress(cControl*)), SLOT(backClicked(cControl*)));
		nextButton = dynamic_cast<cImageButton*>(container->findByName("NextButton", false));
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
		cVerticalScrollbar *scrollbar = new cVerticalScrollbar(380, 1, 270);
		scrollbar->setObjectName("ShardlistScrollbar");
		scrollbar->setRange(0, 9);
		connect(scrollbar, SIGNAL(scrolled(int)), SLOT(shardlistScrolled(int)));
		shardSelectionContainer->addControl(scrollbar);

		// Set up the list of shard entries
		shardList = dynamic_cast<cContainer*>(shardSelectGump->findByName("ShardListContainer"));

		// Set up the Status Dialog
		statusDialog = dynamic_cast<cContainer*>(container->findByName("StatusDialogContainer"));
		statusLabel = dynamic_cast<cLabel*>(statusDialog->findByName("StatusLabel"));
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
		
		// Build delete confirmation dialog
		confirmDeleteDialog = dynamic_cast<cContainer*>(container->findByName("ConfirmDeleteContainer"));
		confirmDeleteText = dynamic_cast<cAsciiLabel*>(container->findByName("ConfirmationLabel"));
		connect(confirmDeleteDialog->findByName("DeleteCancelButton"), SIGNAL(onButtonPress(cControl*)), this, SLOT(statusCancelClicked(cControl*)));
		connect(confirmDeleteDialog->findByName("DeleteOkButton"), SIGNAL(onButtonPress(cControl*)), this, SLOT(statusOkClicked(cControl*)));
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

		case PAGE_NEWCHARACTER:
			emit hideCharacterCreation();	
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
			statusLabel->setText(Localization->get(3000002));
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(true);
			break;
		case PAGE_VERIFYING:
			statusLabel->setText(Localization->get(3000003));
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
			statusLabel->setText(tr("Deleting character..."));
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(false);
			break;
		case PAGE_ENTERING:
			statusLabel->setText(Localization->get(3000001));
			statusDialog->setVisible(true);
			statusOk->setVisible(false);
			statusCancel->setVisible(false);
			break;
		case PAGE_NEWCHARACTER:
			backButton->setVisible(false);
			nextButton->setVisible(false);
			emit showCharacterCreation();
			break;
	}

	this->page = page;
}

void cLoginDialog::buildConfirmDeleteGump() {
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
		show(PAGE_NEWCHARACTER);
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
		UoSocket->setAccount(inpAccount->text());
	} else {
		cGameLoginPacket packet(UoSocket->seed(), inpAccount->text(), inpPassword->text());
		UoSocket->send(packet);
		UoSocket->setAccount(inpAccount->text());
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

cLoginDialog *LoginDialog = 0;
