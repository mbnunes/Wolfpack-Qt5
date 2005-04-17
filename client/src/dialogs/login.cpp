
#include <qapplication.h>

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
#include "mainwindow.h"

/* A custom cShardLabel class */
class cShardLabel : public cTextField {
private:
	cShardLabel *secondary;
	cShardLabel *tertiary;

public:
	cShardLabel(int x, int y, const QCString &text) : cTextField(x, y, 350, 25, 5, 0x34f, 0, true) {
		canHaveFocus_ = false; // These are only clickable
		setMouseOverHue(0x23);
		setText(text);
		secondary = 0;
		tertiary = 0;
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
			if (texture && x >= 0 && y >= 0 && x < texture->realWidth() && y < texture->realHeight()) {
				return this;
			}
		}

		return 0;
	}

	void onMouseDown(QMouseEvent *e) {
		if (e->button() == Qt::LeftButton) {
			// Select this shard
			Utilities::messageBox("Selected a shard entry.");
		}
	}
};

cLoginDialog::cLoginDialog() {
	container = 0;
	movieButton = 0;
	accountLoginGump = 0;
	shardSelectGump = 0;
	inpAccount = 0;
	inpPassword = 0;
	shardEntryOffset = 0;
	statusDialog = 0;
	statusLabel = 0;
	page = PAGE_LOGIN;
}

cLoginDialog::~cLoginDialog() {
}

void cLoginDialog::backClicked(cControl *sender) {
}

void cLoginDialog::nextClicked(cControl *sender) {
	switch (page) {
		// Initiate the login
		case PAGE_LOGIN:
			//UoSocket->connect(Config->loginHost(), Config->loginPort(), 0);
			//show(PAGE_CONNECTING);
			hide();
			WorldView->setVisible(true);
			break;

		case PAGE_SHARDLIST:
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
		cAsciiLabel *alabel = new cAsciiLabel("Ancient Realms", 9, 0x481);
		alabel->setPosition(243, 415);
		shardSelectGump->addControl(alabel);

		// World Button
		button = new cImageButton(0xa0, 0x190, 0x15e8, 0x15ea);
		button->setStateGump(BS_HOVER, 0x15e9);
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

		label = new cLabel(tr("Sort By:"), 1, 0x3e4, false);
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
		container->addControl(button);

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

		label = new cAsciiLabel(tr("UO Version 4.0.6a (Patch 3)").latin1(), 9, 0x34f, ALIGN_LEFT, true, true);
		label->setPosition(0xc8, 0x1a3);
		container->addControl(label);

		label = new cAsciiLabel(tr((char)0xa9 + QString(" 2004 Electronic Arts Inc.  All Rights Reserved.")).latin1(), 6, 0x481, ALIGN_LEFT, true, true);
		label->setPosition(0xc8, 0x1c5);
		container->addControl(label);

		label = new cAsciiLabel(tr("Account Name").latin1(), 2, 0x34f);
		label->setPosition(0xb4, 0x157);
		container->addControl(label);

		inpAccount = new cTextField(0x148, 0x157, 0xd2, 0x1e, 5, 0x34f, 3000, true);
		inpAccount->setMouseOverHue(0x2b8);
		inpAccount->setFocusHue(0x23);
		inpAccount->setMaxLength(16);
		container->addControl(inpAccount);

		label = new cAsciiLabel(tr("Password").latin1(), 2, 0x34f);
		label->setPosition(0xb4, 0x17f);
		container->addControl(label);

		inpPassword = new cTextField(0x148, 0x17f, 0xd2, 0x1e, 5, 0x34f, 3000, true);
		inpPassword->setPassword(true);
		inpPassword->setMaxLength(16);
		inpPassword->setMouseOverHue(0x2b8);
		inpPassword->setFocusHue(0x23);
		container->addControl(inpPassword);

		accountLoginGump = container;
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
	}
}

void cLoginDialog::show(enMenuPage page) {
	QWidget *mainWindow = App->mainWidget();
	if (mainWindow) {
		mainWindow->resize(640, 480);

        SetWindowPos( mainWindow->winId(), 0,
						0,       // x,
						0,       // y,
						640,       // w,
						480,       // h,
						SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOREPOSITION );

		/*mainWindow->setMaximumHeight(mainWindow->frameGeometry().height());
		mainWindow->setMinimumHeight(mainWindow->frameGeometry().height());
		mainWindow->setMaximumWidth(mainWindow->frameGeometry().width());
		mainWindow->setMinimumWidth(mainWindow->frameGeometry().width());*/
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
		connect(button, SIGNAL(onClick(cControl*)), this, SLOT(quitClicked(cControl*)));
		container->addControl(button);

		// MyUO Button
		button = new cImageButton(14, 146, 0x158f, 0x1591);
		button->setStateGump(BS_HOVER, 0x1590);
		connect(button, SIGNAL(onClick(cControl*)), this, SLOT(myUoClicked(cControl*)));
		container->addControl(button);

		// Account Button
		button = new cImageButton(14, 205, 0x1592, 0x1594);
		button->setStateGump(BS_HOVER, 0x1593);
		connect(button, SIGNAL(onClick(cControl*)), this, SLOT(accountClicked(cControl*)));
		container->addControl(button);

		// Movie Button
		movieButton = new cImageButton(14, 306, 0x1586, 0x1588);
		movieButton->setStateGump(BS_HOVER, 0x1587);
		connect(button, SIGNAL(onClick(cControl*)), this, SLOT(movieClicked(cControl*)));
		container->addControl(movieButton);

		// Credits Button
		button = new cImageButton(14, 330, 0x1583, 0x1585);
		button->setStateGump(BS_HOVER, 0x1584);
		connect(button, SIGNAL(onClick(cControl*)), this, SLOT(creditsClicked(cControl*)));
		container->addControl(button);

		// Help Button
		button = new cImageButton(14, 354, 0x1595, 0x1597);
		button->setStateGump(BS_HOVER, 0x1596);
		connect(button, SIGNAL(onClick(cControl*)), this, SLOT(helpClicked(cControl*)));
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

		// Back Button
		backButton = new cImageButton(586, 445, 0x15a1, 0x15a3);
		backButton->setStateGump(BS_HOVER, 0x15a2);
		connect(backButton, SIGNAL(onClick(cControl*)), this, SLOT(backClicked(cControl*)));
		container->addControl(backButton);

		// Next Button
		nextButton = new cImageButton(610, 445, 0x15a4, 0x15a6);
		nextButton->setStateGump(BS_HOVER, 0x15a5);
		connect(nextButton, SIGNAL(onClick(cControl*)), this, SLOT(nextClicked(cControl*)));		
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

		// Status Informations
		case PAGE_VERIFYING:
		case PAGE_CONNECTING:
			statusDialog->setVisible(false);
			nextButton->setVisible(false);
			backButton->setVisible(false);
			break;
	}

	// Which one to display?
	switch (page) {
		case PAGE_LOGIN:
			accountLoginGump->setVisible(true);
			movieButton->setVisible(true);
			nextButton->setVisible(true);
			break;
		case PAGE_SHARDLIST:
			shardSelectGump->setVisible(true);
			nextButton->setVisible(true);
			backButton->setVisible(true);
			break;
		case PAGE_CONNECTING:
			statusLabel->setText("Connecting...");
			statusDialog->setVisible(true);
			break;
		case PAGE_VERIFYING:
			statusLabel->setText("Verifying Account...");
			statusDialog->setVisible(true);
			break;
	}

	this->page = page;
}

void cLoginDialog::hide() {
	if (container->parent() == Gui) {
		Gui->removeControl(container);
		Gui->invalidate();
	}
	
	GLWidget->resize(Config->engineWidth(), Config->engineHeight());
	GLWidget->setMaximumHeight(32000);
	GLWidget->setMaximumWidth(32000);
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

void cLoginDialog::addShard(const stShardEntry &shard) {
	// add the shard entry and also add a control for the shard
	shards.append(shard);

	cShardLabel *shardLabel, *pingCount, *packetLoss;
	
	shardLabel = new cShardLabel(67, shardEntryOffset, shard.name);
	shardList->addControl(shardLabel);

	pingCount = new cShardLabel(67 + 186, shardEntryOffset, "-");
	shardList->addControl(pingCount);

	packetLoss = new cShardLabel(67 + 186 + 60, shardEntryOffset, "-");
	shardList->addControl(packetLoss);

	// Set Secondary + Tertiary
	shardLabel->setSecondary(pingCount);
	shardLabel->setTertiary(packetLoss);
	pingCount->setSecondary(shardLabel);
	pingCount->setTertiary(packetLoss);
	packetLoss->setSecondary(shardLabel);
	packetLoss->setTertiary(pingCount);
	
	shardEntryOffset += 25;
}

void cLoginDialog::onError(const QString &error) {
	statusLabel->setText(error.latin1());
}

void cLoginDialog::onDnsLookupComplete(const QHostAddress &address, unsigned short port) {
}

void cLoginDialog::onConnect() {
	show(PAGE_VERIFYING);

	QByteArray loginPacket(62);
	loginPacket.fill(0);
	loginPacket[0] = (unsigned char)0x80;
	if (!inpAccount->text().isNull()) {
		strcpy(loginPacket.data() + 1, inpAccount->text().left(30).data());
	}
	if (!inpPassword->text().isNull()) {
		strcpy(loginPacket.data() + 31, inpPassword->text().left(30).data());
	}
	UoSocket->send(loginPacket);
}

cLoginDialog *LoginDialog = 0;
