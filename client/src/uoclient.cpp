
#include <qdatetime.h>
#include <qdir.h>
#include <qvbox.h>

#include "uoclient.h"
#include "utilities.h"
#include "config.h"
#include "log.h"
#include "exceptions.h"
#include "texture.h"

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
#include "muls/tiledata.h"
#include "muls/textures.h"

#include "dialogs/login.h"

#include "network/uosocket.h"

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

	World = new cWorld;

	ContextMenu = new cContextMenu;
	LoginDialog = new cLoginDialog;
}

cUoClient::~cUoClient() {
	delete UoSocket;

	delete World;

	delete LoginDialog;
	delete Cursor;

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
	delete Config;

	delete Random;
}

void cUoClient::load() {
	Config->load(); // Load configuration (has to come first)

	Log->print("-----------------------------------------------------------------------------\n", false);
	Log->print(tr("Starting Session (%1)\n").arg(QDateTime::currentDateTime().toString()), false);
	Log->print("-----------------------------------------------------------------------------\n\n", false);

	Random->seed(); // Initialize Random Number Generator

	// Check UO installation
	if (Config->uoPath().isEmpty() || !QDir(Config->uoPath()).exists()) {
		Utilities::messageBox(tr("The client was unable to detect your Ultima Online installation.\nIf you are certain, that Ultima Online is installed on your computer, please check the file config.xml in your clients directory and set the Ultima Online Path value manually."), tr("Ultima Online Installation Is Missing"), true);
		throw Exception(tr("Unable to find Ultima Online installation."));
	}

	Log->print(LOG_MESSAGE, tr("Using Ultima Online at: %1\n").arg(Config->uoPath()));

	// Load MulReaders
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

	Cursor->load(); // The cursor requires the mulreader classes
}

void cUoClient::unload() {
	Cursor->unload();

	// Unload MulReaders
	Animations->unload();
	Textures->unload();
	Maps->unload();
	Tiledata->unload();
	UnicodeFonts->unload();
	AsciiFonts->unload();
	Art->unload();
	Hues->unload();
	Gumpart->unload();
	Verdata->unload();

	Config->save(); // Save Configuration

	Log->print("\n-----------------------------------------------------------------------------\n", false);
	Log->print(tr("Stopped Session (%1)\n").arg(QDateTime::currentDateTime().toString()), false);
	Log->print("-----------------------------------------------------------------------------\n\n", false);

	delete App;
}

void myMessageOutput( QtMsgType type, const char *msg )
{
    switch ( type ) {
        case QtDebugMsg:
			Log->print(LOG_DEBUG, tr("QT Debug: %1\n").arg(msg));
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

void cUoClient::run(const QStringList &arguments) {
	qInstallMsgHandler(myMessageOutput); // Install handler
	int argc = 0;
	App = new QApplication(argc, 0);

	Config->setFile("config.xml"); // Default Config File

	// INITIALIZE WINDOW - OPENGL INTIAILZATION
	MainWindow *window = new MainWindow();
	window->setCaption("Ultima Online");
	App->setMainWidget(window);	
	window->show();
	// END WINDOW INITIALIZATION

	try {
		load();
	} catch(const Exception &e) {
		Utilities::messageBox(e.message(), "Error", true);
		Log->print(LOG_ERROR, e.message() + "\n");
		unload();
		return;
	}
	
	LoginDialog->show(PAGE_SHARDLIST); // Set up the login screen

	//World->changeFacet(ILSHENAR);
	World->moveCenter(1245, 1758, 0, true);
	//World->moveCenter(1715, 301, -7, true);
	//Player = new cMobile(1715, 301, -7, ILSHENAR, 1);
	Player = new cMobile(1245, 1758, 0, TRAMMEL, 1);
	World->addEntity(Player);

	// add the world view
	WorldView = new cWorldView(800, 600);
	WorldView->setVisible(false);
	Gui->addControl(WorldView);

	while (running()) {
		App->processEvents();
		GLWidget->update();

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

cUoClient *Client = 0; // Global Client Instance
QApplication *App = 0; // Global QApplication Instance
