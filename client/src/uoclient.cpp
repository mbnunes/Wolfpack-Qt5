
#include <qdatetime.h>
#include <qdir.h>
#include <q3vbox.h>
#include <qmessagebox.h>

#include "uoclient.h"
#include "utilities.h"
#include "config.h"
#include "log.h"
#include "exceptions.h"
#include "texture.h"
#include "sound.h"

#include "gui/cursor.h"
#include "gui/gui.h"
#include "gui/worldview.h"
#include "gui/contextmenu.h"

#include "game/mobile.h"
#include "game/world.h"

#include "muls/animations.h"
#include "muls/gumpart.h"
#include "muls/hues.h"
#include "muls/verdata.h"
#include "muls/art.h"
#include "muls/maps.h"
#include "muls/asciifonts.h"
#include "muls/unicodefonts.h"
#include "muls/sounds.h"
#include "muls/speech.h"
#include "muls/tiledata.h"
#include "muls/textures.h"
#include "muls/localization.h"

#include "dialogs/login.h"

#include "network/uosocket.h"

#include <QErrorMessage>

//#include <windows.h>

cUoClient::cUoClient() {
	running_ = true;

	UoSocket = new cUoSocket();

	Random = new MTRand();

	Config = new cConfig();
	Log = new cLog();
	Gui = new cGui();
	Cursor = new cCursor();

	// Mul Readers
	Animations = new cAnimations;
	Gumpart = new cGumpart;
	Hues = new cHues;
	Verdata = new cVerdata;
	Art = new cArt;
	AsciiFonts = new cAsciiFonts;
	UnicodeFonts = new cUnicodeFonts;
	Tiledata = new cTiledata;
	Maps = new cMaps;
	Textures = new cTextures;
	Sounds = new cSounds;
	Speech = new cSpeech;
	Sound = new cSound;
	Localization = new cLocalization;

	World = new cWorld;	

	ContextMenu = new cContextMenu;
	LoginDialog = new cLoginDialog;
}

cUoClient::~cUoClient() {
	delete UoSocket;

	delete World;

	delete LoginDialog;
	delete Cursor;

	delete Localization;
	delete Speech;
	delete Animations;
	delete Textures;
	delete Maps;
	delete Tiledata;
	delete UnicodeFonts;
	delete AsciiFonts;
	delete Art;
	delete Verdata;
	delete Hues;
	delete Gumpart;

	delete Gui;
	delete Log;
	delete Sound;
	delete Sounds;
	delete Config;

	delete Random;
}

void cUoClient::load() {
	Random->seed(); // Initialize Random Number Generator

	// Check UO installation
	if (Config->uoPath().isEmpty() || !QDir(Config->uoPath()).exists()) {
		errorMessage(tr("The client was unable to detect your Ultima Online installation.\nIf you are certain, that Ultima Online is installed on your computer, please check the file config.xml\n in your clients directory and set the Ultima Online Path value manually."), tr("Ultima Online Installation Is Missing"));
		throw SilentException();
	}

	Log->print(LOG_MESSAGE, tr("Using Ultima Online at: %1\n").arg(Config->uoPath()));

	// Load MulReaders
	Localization->load();
	Verdata->load();
	Gumpart->load();
	Hues->load();
	Art->load();
	AsciiFonts->load();
	UnicodeFonts->load();
	Tiledata->load();
	Maps->load();
	Textures->load();
	Animations->load();
	Sounds->load();
	Speech->load();
	Sound->load();

	Cursor->load(); // The cursor requires the mulreader classes
}

void cUoClient::unload() {
	Cursor->unload();

	// Unload MulReaders
	Sound->unload();
	Sounds->unload();
	Animations->unload();
	Textures->unload();
	Maps->unload();
	Tiledata->unload();
	UnicodeFonts->unload();
	AsciiFonts->unload();
	Art->unload();
	Speech->unload();
	Hues->unload();
	Gumpart->unload();
	Verdata->unload();
	Localization->unload();

	Config->save(); // Save Configuration

	Log->print("\n-----------------------------------------------------------------------------\n", false);
	Log->print(tr("Stopped Session (%1)\n").arg(QDateTime::currentDateTime().toString()), false);
	Log->print("-----------------------------------------------------------------------------\n\n", false);
}

void myMessageOutput( QtMsgType type, const char *msg )
{
    switch ( type ) {
        case QtDebugMsg:
			// This is crazy...
			// Log->print(LOG_DEBUG, tr("QT Debug: %1\n").arg(msg));
            break;
        case QtWarningMsg:
			Log->print(LOG_WARNING, tr("QT Warning: %1\n").arg(msg));
            break;
        case QtFatalMsg:
			Log->print(LOG_ERROR, tr("QT Fatal: %1\n").arg(msg));
			break;
    }
}

// Main Widget
#include "mainwindow.h"

void cUoClient::run()
{
	qInstallMsgHandler(myMessageOutput); // Install handler

	Config->setFile("config.xml"); // Default Config File
	Config->load(); // Load configuration (has to come first)
	
	Log->print("-----------------------------------------------------------------------------\n", false);
	Log->print(tr("Starting Session (%1)\n").arg(QDateTime::currentDateTime().toString()), false);
	Log->print("-----------------------------------------------------------------------------\n\n", false);

	// INITIALIZE WINDOW - OPENGL INTIAILZATION
	MainWindow *window = new MainWindow();
	window->setCaption("Ultima Online");
	qApp->setMainWidget(window);
	window->show();
	// END WINDOW INITIALIZATION

	try {
		load();
	} catch(const Exception &e) {
		Log->print(LOG_ERROR, e.message() + "\n");
		errorMessage(e.message());
		unload();
		return;
	} catch(const SilentException &) {
		unload();
		return;
	}
	
	LoginDialog->show(PAGE_LOGIN); // Set up the login screen

	//World->changeFacet(ILSHENAR);
	World->moveCenter(1245, 1758, 0, true);
	//World->moveCenter(1715, 301, -7, true);
	//Player = new cMobile(1715, 301, -7, ILSHENAR, 1);
	Player = new cMobile(1245, 1758, 0, TRAMMEL, 1);
	Player->incref(); // Always keep one reference
	World->addEntity(Player);

	// add the world view
	WorldView = new cWorldView(800, 600);
	WorldView->setVisible(false);
	Gui->addControl(WorldView);

	while (running()) {
		qApp->processEvents();
		UoSocket->poll(); // Poll network connection

		// Quit if the main window is hidden
		if (!window->isShown()) {
			running_ = false;
		}
	}

	try {
		unload();
	} catch(const Exception &e) {
		Log->print(LOG_ERROR, e.message() + "\n");
	}
}

void cUoClient::errorMessage(const QString &message, const QString &title) {
	QWidget *mainWindow = (MainWindow*)qApp->mainWidget();
	QMessageBox box(title, message, QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	box.setModal(true);
	box.show();
	while (box.isVisible()) {
		qApp->processEvents();
	}
}

cUoClient *Client = 0; // Global Client Instance
