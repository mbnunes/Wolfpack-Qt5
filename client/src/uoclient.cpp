
#include <qdatetime.h>
#include <qdir.h>

#include "uoclient.h"
#include "utilities.h"
#include "engine.h"
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

#include <windows.h>

cUoClient::cUoClient() {
	UoSocket = new cUoSocket();

	Random = new MTRand();

	Config = new cConfig();
	Log = new cLog();
	Engine = new cEngine();
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
	delete Engine;
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

	// Initialize SDL
	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
		throw Exception(tr("Unable to initialize SDL: %1").arg(SDL_GetError()));
    }

	SDL_EnableUNICODE(1); // Enable UNICODE event handling
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	Engine->load(); // Initialize Engine, Open Window

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

	Engine->unload(); // Close Window, Shutdown Engine

	SDL_Quit(); // Quit SDL

	Config->save(); // Save Configuration

	Log->print("\n-----------------------------------------------------------------------------\n", false);	
	Log->print(tr("Stopped Session (%1)\n").arg(QDateTime::currentDateTime().toString()), false);
	Log->print("-----------------------------------------------------------------------------\n\n", false);

	delete App;
}

void cUoClient::processSdlEvent(const SDL_Event &event) {
	static cControl *mouseCapture = 0; // Control which got the last mousedown event
	static cControl *lastMouseMovement = 0; // Control that got the last movement event

	switch (event.type) {
		case SDL_QUIT:
			quit(); // Signal the client to quit
			break;

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				quit(); // Signal the client to quit
			} else if (event.key.keysym.sym == SDLK_F4) {
				if (event.key.keysym.mod & KMOD_ALT) {
					quit();
				}
			} else if (event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod & KMOD_ALT) {
				Config->setEngineWindowed(!Config->engineWindowed());
				Engine->reload();
				Gui->invalidate();
			} else if (event.key.keysym.sym == SDLK_TAB) {
				if (!Gui->inputFocus() || !Gui->inputFocus()->wantTabs()) {
					// Switch to the next Control
					if (Gui->activeWindow()) {
						cControl *nextControl;
						if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
							nextControl = Gui->activeWindow()->getPreviousFocusControl(Gui->inputFocus());
						} else {
							nextControl = Gui->activeWindow()->getNextFocusControl(Gui->inputFocus());
						}
						Gui->setInputFocus(nextControl);
					}
				} else if (Gui->inputFocus()) {
					Gui->inputFocus()->onKeyDown(event.key.keysym);
				}
			} else {
				// Forward it to the control with the input focus. Otherwise go trough all controls.
				if (Gui->inputFocus()) {
					Gui->inputFocus()->onKeyDown(event.key.keysym);
				}
			}
			break;

		case SDL_KEYUP:            			
			// Forward it to the control with the input focus. Otherwise go trough all controls.
			if (Gui->inputFocus()) {
				Gui->inputFocus()->onKeyUp(event.key.keysym);
			}

			if (event.key.keysym.sym == SDLK_F12) {
				Engine->screenshot();
			}

			break;

		case SDL_VIDEORESIZE:
			if (Engine->lockSize()) {
				Engine->resize(Engine->width(), Engine->height()); // Resize Engine
			} else {
				Engine->resize(event.resize.w, event.resize.h); // Resize Engine
				Gui->invalidate();
			}
			break;

		// TODO:
		// a) Generate onMouseEnter + onMouseLeave events for hover buttons
		// b) Generate real onMouseMove events for the controls the mouse is currently being moved over
		// c) Movement of controls has to be improved so you can't move controls out of the boundaries of their
		//    containers.

		case SDL_MOUSEBUTTONDOWN:
			{
				if (mouseCapture) {
					mouseCapture->onMouseDown(event.button.x, event.button.y, event.button.button, event.button.state == SDL_PRESSED);
				} else {
					cControl *control = Gui->getControl(event.button.x, event.button.y);
					if (control) {
						control->onMouseDown(event.button.x, event.button.y, event.button.button, event.button.state == SDL_PRESSED);
						mouseCapture = control;

						// If the new control wants to have the input focus, it gets it
						if (control->canHaveFocus()) {
							Gui->setInputFocus(control);
						}

						// Get the next control with the gui as its parent above it
						cContainer *parent = control->parent();
						while (parent && parent->parent() != Gui) {
							parent = parent->parent();
						}

						if (parent && parent->isWindow() && parent != Gui) {
							Gui->setActiveWindow((cWindow*)parent);
						} else {
							Gui->setActiveWindow(0);
						}
					}

					// If a contextmenu is open and we click anywhere else -> close the menu
					if (control != ContextMenu && control->parent() != ContextMenu) {
						ContextMenu->hide();
					}
				}
			}
			break;

		case SDL_MOUSEMOTION:
			{
				cControl *motionControl = Gui->getControl(event.motion.x, event.motion.y);
				if (motionControl != lastMouseMovement) {
					if (lastMouseMovement) {
						lastMouseMovement->onMouseLeave();
					}
					lastMouseMovement = motionControl;
					if (lastMouseMovement) {
						lastMouseMovement->onMouseEnter();
					}
				}				
				
				cControl *control = mouseCapture;
				if (control && control->isMoveHandle()) {
					cControl *movable = control->getMovableControl();
					if (movable) {
						movable->setPosition(movable->x() + event.motion.xrel, movable->y() + event.motion.yrel);
					}
				}

				if (!control) {
					control = Gui->getControl(event.motion.x, event.motion.y);
				}

				if (control) {
					control->onMouseMotion(event.motion.xrel, event.motion.yrel, event.motion.state);
				}
			}
			break;

		case SDL_MOUSEBUTTONUP:
			{
				cControl *control = mouseCapture;
				if (!control) {
					control = Gui->getControl(event.button.x, event.button.y);
				}
				if (control) {
					control->onMouseUp(event.button.x, event.button.y, event.button.button, event.button.state == SDL_PRESSED);
					mouseCapture = 0; // Reset mouse capture
				}
			}
			break;

		default:
			break; // Do Nothing
	}
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

BOOL (__stdcall *wglSwapIntervalEXT)(int interval);

void cUoClient::run(const QStringList &arguments) {
	qInstallMsgHandler(myMessageOutput); // Install handler
	int argc = 0;		
	App = new QApplication(argc, 0);
	
	Config->setFile("config.xml"); // Default Config File

	try {
		load();
	} catch(const Exception &e) {
		Utilities::messageBox(e.message(), "Error", true);
		Log->print(LOG_ERROR, e.message() + "\n");
		unload();
		return;
	}

	// This is windows specific
	wglSwapIntervalEXT = (BOOL (__stdcall *)(int))wglGetProcAddress("wglSwapIntervalEXT");

	//LoginDialog->show(PAGE_LOGIN); // Set up the login screen
	//World->changeFacet(ILSHENAR);
	World->moveCenter(1245, 1758, 0, true);
	//World->moveCenter(1715, 301, -7, true);
	//Player = new cMobile(1715, 301, -7, ILSHENAR, 1);
	Player = new cMobile(1245, 1758, 0, TRAMMEL, 1);
	World->addEntity(Player);

	// add the world view
	WorldView = new cWorldView(800, 600);
	Gui->addControl(WorldView);

	while (running()) {
		UoSocket->poll();
		
		SDL_Event event; // An SDL Event
		while (SDL_PollEvent(&event)) {
			processSdlEvent(event);
		}

		glClear(GL_COLOR_BUFFER_BIT); // Clear both. Background and Z Buffer.
		glLoadIdentity(); // Return to the identity matrix
		
		// Process the WorldView move tick
		WorldView->moveTick();

		// Draw the world first
		int x, y, width, height;
		WorldView->getWorldRect(x, y, width, height);

		World->draw(x, y, width, height);

		Gui->draw(); // Draw the GUI controls
		Cursor->draw(); // Draw the cursor overlay

		wglSwapIntervalEXT(4);
		Engine->poll(); // Swap
		//SDL_Delay(5);
	}

	try {
		unload();
	} catch(const Exception &e) {
		Log->print(LOG_ERROR, e.message() + "\n");
	}
}

cUoClient *Client = 0; // Global Client Instance
QApplication *App = 0; // Global QApplication Instance
