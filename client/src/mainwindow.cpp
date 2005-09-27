
#include "mainwindow.h"
#include "gui/gui.h"
#include "gui/cursor.h"
#include "gui/contextmenu.h"
#include "gui/worldview.h"
#include "gui/combobox.h"
#include "game/world.h"
#include "game/dynamicitem.h"
#include "game/mobile.h"
#include "game/groundtile.h"
#include "network/uosocket.h"
#include "muls/localization.h"
#include "scripts.h"
#include "config.h"
#include "sound.h"
#include "log.h"
#include "profile.h"
#include "dialogs/cachestatistics.h"
#include "network/outgoingpackets.h"
#include "game/targetrequest.h"
#include <qpixmap.h>
#include <QCursor>
#include <qimage.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qmenubar.h>

#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QMessageBox>
#include <QDir>
#include <QApplication>
#include <QFileDialog>

// Shader Fun!
#if defined(Q_OS_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef GL_FRAGMENT_PROGRAM_ARB
#define GL_FRAGMENT_PROGRAM_ARB		0x8804
#endif
#ifndef GL_PROGRAM_FORMAT_ASCII_ARB
#define GL_PROGRAM_FORMAT_ASCII_ARB	0x8875
#endif

bool shadersAvailable = false;
QString grayShader_Program = "!!ARBfp1.0\n\
#vendor NVIDIA Corporation\n\
#version 1.0.02\n\
#profile arbfp1\n\
#program main\n\
#semantic main.texture : TEXUNIT0\n\
#var float4 IN.color : $vin.COLOR : COL0 : 0 : 1\n\
#var float3 IN.texcoord : $vin.TEXCOORD0 : TEX0 : 0 : 1\n\
#var sampler2DSHADOW texture : TEXUNIT0 : texunit 0 : 1 : 1\n\
#var float4 main.color : $vout.COLOR : COL : -1 : 1\n\
#const c[0] = 0.144 0.299 0.587\n\
PARAM c[1] = { { 0.14399999, 0.29899999, 0.58700001 } };\n\
TEMP R0;\n\
TEX R0, fragment.texcoord[0], texture[0], 2D;\n\
MUL R0.y, R0, c[0].z;\n\
MAD R0.x, R0, c[0].y, R0.y;\n\
MAD R0.x, R0.z, c[0], R0;\n\
MUL result.color.xyz, fragment.color.primary.x, R0.x;\n\
MOV result.color.w, R0;\n\
END\n\
# 6 instructions, 1 R-regs";

void (APIENTRYP my_glGenProgramsARB)(GLuint, GLuint *) = NULL;
void (APIENTRYP my_glBindProgramARB)(GLuint, GLuint) = NULL;
void (APIENTRYP my_glProgramStringARB)(GLuint, GLuint, GLint, const GLbyte *) = NULL;

void cGLWidget::getShaderPointers() {
	my_glGenProgramsARB = (void (APIENTRYP)(GLuint, GLuint *))context()->getProcAddress("glGenProgramsARB");
	my_glBindProgramARB = (void (APIENTRYP)(GLuint, GLuint))context()->getProcAddress("glBindProgramARB");
	my_glProgramStringARB = (void (APIENTRYP)(GLuint, GLuint, GLint, const GLbyte *))context()->getProcAddress("glProgramStringARB");

	if (my_glGenProgramsARB && my_glBindProgramARB && my_glProgramStringARB) {
		shadersAvailable = true;

		QFile file("gray_shader.fp");
		if (file.open(QIODevice::ReadOnly)) {
			grayShader_Program = file.readAll();
			file.close();
			Log->print(LOG_NOTICE, tr("Loading custom gray_shader shader program from %1.\n").arg(file.fileName()));
		}
	} else {
		Log->print(LOG_WARNING, tr("This OpenGL implementation doesn't seem to support ARB shaders.\n"));
		shadersAvailable = false;
	}
}

/* XPM */
static const char * const icon_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 16 1",
"  c #000000",
". c #444444",
"X c #4c4c4c",
"o c #555555",
"O c gray50",
"+ c gray60",
"@ c #b2b2b2",
"# c #86b2f8",
"$ c #95beff",
"% c #95d2ff",
"& c #b8cef1",
"* c #bfd8ff",
"= c #cbcbcb",
"- c #d4e5ff",
"; c white",
": c None",
/* pixels */
"::::::     :::::::::     :::::::",
"::::: =.--= ::::::: ;--== ::::::",
":::::: =OX ::::::::: XOO :::::::",
"::::: .+o ::::::::::: XO- ::::::",
":::: =oX= ::::::::::: =XX= :::::",
":::: %$$% ::::::::::: $%$% :::::",
":::: =XX= ::::::::::: =.X= :::::",
":::: @;OXO ::::::::: +.+-@ :::::",
"::::: -=+.O ::::::: +X+-. ::::::",
":::::: .=O.O  : :  O.+-. :::::::",
"::::::: ;-=oXO * +XX=-. ::::::::",
"::::::::  O-XX$=#O..O  :::::::::",
":::::::::  OX-#X=*XO  ::::::::::",
":::::::: =OXXX$=%XXX@- :::::::::",
"::::::: -O.=%$X*X%$#.O- ::::::::",
":::::: =+.=$   &   $=.+- :::::::",
":::::: ;X=% ::: ::: $=XO :::::::",
"::::: -OX% ::::::::: %XO= ::::::",
"::::: ;X=$ ::::::::: $=X+ ::::::",
":::: #XX%# ::::::::: #%Xo# :::::",
"::: %-$$$-# ::::::: #-$$%-% ::::",
":::: $XX%# ::::::::: #%XX# :::::",
"::::: =X=% ::::::::: %=X= ::::::",
"::::: =OX% ::::::::: %XO- ::::::",
"::::: +OO=# ::: ::: $=.+O ::::::",
":::::: .O.=$   #   $=.O. :::::::",
"::::::: .+X=#%#-#%##.+. ::::::::",
":::::::: .OoX=*$%=..O. :::::::::",
"::::::::: .=OXX$XXO+. ::::::::::",
"::::::::::  ;;X$X==  :::::::::::",
"::::::::::::  $*#  :::::::::::::",
":::::::::::::: * :::::::::::::::"
};

cMainWindow::cMainWindow() {
	setObjectName("MainWindow");

	QAction *action;

	// Window Icon
	QPixmap pixmap((const char**)icon_xpm);
	setWindowIcon(pixmap);

	// Create a menu bar at the top of the window
	m_menuBar = new QMenuBar(this);
	QMenu *file = m_menuBar->addMenu(tr("&File"));
	action = file->addAction(tr("Save profile as..."));	
	action->setObjectName("action_saveprofile");
	file->addSeparator();
	file->addAction("E&xit", this, SLOT(close()));		

	connect(file, SIGNAL(triggered(QAction*)), this, SLOT(menuGameClicked(QAction*)));
	
	// Game Menu
	QMenu *game = m_menuBar->addMenu("&Game");
	QMenu *hideMenu = game->addMenu("Hide entities");
	aHideStatics = hideMenu->addAction("Hide Statics");
	aHideStatics->setCheckable(true);
	aHideDynamics = hideMenu->addAction("Hide Dynamics");
	aHideDynamics->setCheckable(true);
	aHideMobiles = hideMenu->addAction("Hide Mobiles");
	aHideMobiles->setCheckable(true);
	aHideMap = hideMenu->addAction("Hide Map");	
	aHideMap->setCheckable(true);

	aHideMap->setChecked(Config->gameHideMap());
	aHideStatics->setChecked(Config->gameHideStatics());
	aHideDynamics->setChecked(Config->gameHideDynamics());
	aHideMobiles->setChecked(Config->gameHideMobiles());

	// Highlight entities menu
	QMenu *highlightMenu = game->addMenu(tr("Highlight"));

	// Highlight statics toggle
	action = highlightMenu->addAction(tr("Highlight static tiles"));
	action->setCheckable(true);
	action->setChecked(Config->gameHighlightStatics());
	action->setObjectName("action_highlight_statics");

	// Highlight map tiles toggle
	action = highlightMenu->addAction(tr("Highlight map tiles"));
	action->setChecked(Config->gameHighlightMap());
	action->setCheckable(true);
	action->setObjectName("action_highlight_map");

	game->addSeparator();

	action = game->addAction("Show current position");
	action->setObjectName("action_where");
	
    action = game->addAction("Resync with Server");
	action->setObjectName("action_resync");

    action = game->addAction("Open Backpack");
	action->setObjectName("action_openbackpack");

    action = game->addAction("Show Priority Information");
	action->setObjectName("action_showpriority");

	action = game->addAction("Export Localization");
	action->setObjectName("action_exportlocalization");

	connect(game, SIGNAL(triggered(QAction*)), this, SLOT(menuGameClicked(QAction*)));

	GLWidget = new cGLWidget(this);
	setCentralWidget(GLWidget);

	setMenuBar(m_menuBar);

	QMenu *helpMenu = m_menuBar->addMenu(tr("&Help"));
	action = helpMenu->addAction(tr("Statistics"));
	action->setObjectName("action_cachestatistics");
	action = helpMenu->addAction(tr("&About"));
	action->setObjectName("action_about");

	connect(helpMenu, SIGNAL(triggered(QAction*)), this, SLOT(menuGameClicked(QAction*)));	

	cacheStatistics = new cCacheStatistics(this);

	resize(640, 480); // Default size
}

void cMainWindow::menuGameClicked(QAction *action) {
	if (action == aHideStatics) {
		Config->setGameHideStatics(action->isChecked());
	} else if (action == aHideMap) {
		Config->setGameHideMap(action->isChecked());
	} else if (action == aHideDynamics) {
		Config->setGameHideDynamics(action->isChecked());
	} else if (action == aHideMobiles) {
		Config->setGameHideMobiles(action->isChecked());
	} else if (action->objectName() == "action_resync") {
		UoSocket->resync();
	} else if (action->objectName() == "action_highlight_statics") {
		Config->setGameHighlightStatics(action->isChecked());
	} else if (action->objectName() == "action_highlight_map") {
		Config->setGameHighlightMap(action->isChecked());
	} else if (action->objectName() == "action_where") {
		QString message = QString("You are currently at %1,%2,%3,%4.").arg(World->x()).arg(World->y()).arg(World->z()).arg(World->facet());

		// Show a window with the current location
		if (WorldView) {
			WorldView->addSysMessage(message);
		}
	} else if (action->objectName() == "action_about") {
	} else if (action->objectName() == "action_cachestatistics") {
		if (cacheStatistics) {
			cacheStatistics->show();			
			cacheStatistics->refresh();
		}
	} else if (action->objectName() == "action_openbackpack") {
		if (Player) {
			cDynamicItem *backpack = Player->getEquipment(LAYER_BACKPACK);
			if (backpack) {
				UoSocket->send(cDoubleClickPacket(backpack->serial()));
			}
		}
	} else if (action->objectName() == "action_showpriority") {
		if (WorldView && WorldView->isVisible()) {
			WorldView->cancelTarget();

			cGenericTargetRequest *request = new cGenericTargetRequest;
			connect(request, SIGNAL(targetted(cEntity*)), SLOT(showPriority(cEntity*)));
            WorldView->requestTarget(request);
		}
	} else if (action->objectName() == "action_saveprofile") {
		if (UoSocket->account().isEmpty() || !WorldView->isVisible()) {
			QMessageBox::critical(this, tr("Error"), tr("Unable to save profile when you're not logged in."), QMessageBox::Ok, QMessageBox::NoButton);
			return;
		}

		QString filename = QFileDialog::getSaveFileName(this, "Save profile as", QDir::current().absolutePath(), tr("XML Files (*.xml)"));

		if (!filename.isEmpty()) {
			Profile->saveToFile(filename, true);
		}		
	} else if (action->objectName() == "action_exportlocalization") {
		QString filename = QFileDialog::getSaveFileName(this, "Save localization as", QDir::current().absolutePath(), tr("Text Files (*.txt)"));

		if (!filename.isEmpty()) {
			Localization->export(filename, "ENU");
		}		
	}
}

void cMainWindow::showPriority(cEntity *entity) {
	if (!WorldView || !WorldView->isVisible()) {
		return;
	}

	QString message = tr("The entity has a drawing priority of %1 (%3), Z is %2.").arg(entity->priority()).arg(entity->z()).arg(entity->prioritySolver());
	cGroundTile *gt = dynamic_cast<cGroundTile*>(entity);
	if (gt) {
		message.append(tr(" The sort z value is %1.").arg(gt->sortz()));
	}
	entity->updatePriority();

	WorldView->addSysMessage(message);
}

cMainWindow::~cMainWindow() {
}

cGLWidget::cGLWidget(QWidget *parent) : QGLWidget(parent) {
	// Cursor stuff
	setCursor(Qt::BlankCursor);

	lastMouseX = 0;
	lastMouseY = 0;

	setMouseTracking(true);

	mouseCapture_ = 0; // Control which got the last mousedown event
	lastMouseMovement_ = 0; // Control that got the last movement event

	setFocusPolicy(Qt::WheelFocus);

	// Update Timer
	/*QTimer *timer = new QTimer(this);
	timer->setInterval(15); // 15ms redraw interval (max. 100fps)
	timer->setSingleShot(false);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start();*/

	singleClickTimer.setSingleShot(true);
	singleClickTimer.setInterval(QApplication::doubleClickInterval() + 20);
	singleClickTimer.stop();
	connect(&singleClickTimer, SIGNAL(timeout()), this, SLOT(singleClick()));

	ignoreMouseRelease = false;
	singleClickEvent = 0;
	lastDoubleClick = false;
}

cGLWidget::~cGLWidget() {
	delete singleClickEvent;
}

#if defined(Q_OS_WIN32)
typedef BOOL (CALLBACK *fpWglSwapIntervalEXT)(int interval);
fpWglSwapIntervalEXT wglSwapIntervalEXT = 0;
typedef int (CALLBACK *fpWglGetSwapIntervalEXT)(void);
fpWglGetSwapIntervalEXT wglGetSwapIntervalEXT = 0;
bool wglSwapIntervalLoaded = false;
#endif

void cGLWidget::initializeGL() {
	// Platform dependant crap
#if defined(Q_OS_WIN32)
	if (!wglSwapIntervalLoaded) {
		wglSwapIntervalEXT = (fpWglSwapIntervalEXT)wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT = (fpWglGetSwapIntervalEXT)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglSwapIntervalLoaded = true;
	}

	if (wglSwapIntervalEXT) {
		if (wglSwapIntervalEXT(0) == FALSE) {
			Log->print(LOG_ERROR, tr("Unable to set vsync to off.\n"));
		}
	}

	if (wglGetSwapIntervalEXT) {
		Log->print(LOG_NOTICE, tr("wglGetSwapIntervalEXT() returned %1.\n").arg(wglGetSwapIntervalEXT()));
	}
#endif

	// Initialize OpenGL
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fill the screen with a black color
	glClearStencil(1);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DITHER);
	// TODO: Disable mip mapping
	glEnable(GL_BLEND); // Enable aplha blending of textures
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH); // Go with flat shading for now
	glEnable(GL_NORMALIZE);

	// Set up texture parameters
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// Set up lighting
	glEnable(GL_LIGHT0);

	GLfloat light_position[] = { -1.0f, -1.0f, 0.5f, 0.0f };
	//GLfloat light_position[] = { 100.0f, -30.0f, 23.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	float specular[] = {2.0f, 2.0f, 2.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, specular);

	GLfloat lmodel_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	getShaderPointers(); // Initialize Pixelshader Pointers
	
	// Generate "Gray-Shader"
	if (shadersAvailable) {
		glEnable(GL_FRAGMENT_PROGRAM_ARB);		
		my_glGenProgramsARB(1, &grayShader);
		my_glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, grayShader);
		QByteArray data = grayShader_Program.toLatin1();
		my_glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, data.length(), (const GLbyte *)data.data());
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}

void cGLWidget::resizeGL( int w, int h ) {
	// Initialize OpenGL
	glViewport(0, 0, w, h); // Tell OpenGL the size of our window

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// We're not using real perspectiveb
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set a very simple projection matrix
	gluOrtho2D(0, w, h, 0);

	// Switch back to the model view matrix
	glMatrixMode(GL_MODELVIEW);

	// Notify the GUI of the change
	Gui->setBounds(0, 0, w, h); // Full size
}

void cGLWidget::paintGL() {
	// Check for current FPS
	static unsigned int nextMeasurement = 0;
	static unsigned int framesDrawn = 0;

	if (nextMeasurement < Utilities::getTicks()) {
		parentWidget()->setWindowTitle(tr("Ultima Online (%1 fps)").arg(framesDrawn));
		framesDrawn = 0;
		nextMeasurement = Utilities::getTicks() + 1000;
	}

	++framesDrawn;

	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	// Only bother drawing the world if it's visible
	if (WorldView && WorldView->isVisible()) {
		// Process the WorldView move tick
		WorldView->moveTick();

		// Draw the world first
		int x, y, width, height;
		WorldView->getWorldRect(x, y, width, height);
		World->draw(x, y, width, height);
	}

	Gui->draw(); // Draw the GUI controls
	Cursor->draw(); // Draw the Cursor Overlay
}

void cGLWidget::mouseMoveEvent(QMouseEvent *e) {
	// Calculate the x/y axis difference from the last time
	int xrel = e->x() - lastMouseX;
	int yrel = e->y() - lastMouseY;

	// Save the coordinates
	lastMouseX = e->x();
	lastMouseY = e->y();

	cControl *motionControl;
	if (Gui->currentCombolist()) {
		motionControl = Gui->currentCombolist()->getControl(e->x() - Gui->currentCombolist()->x(), e->y() - Gui->currentCombolist()->y());
	} else {
		motionControl = Gui->getControl(e->x(), e->y());
	}
	
	if (motionControl != lastMouseMovement_) {
		if (lastMouseMovement_) {
			lastMouseMovement_->onMouseLeave();
		}
		lastMouseMovement_ = motionControl;
		if (lastMouseMovement_) {
			lastMouseMovement_->onMouseEnter();
		}
	}

	cControl *control = mouseCapture_;
	if (control && control->isMoveHandle()) {
		cControl *movable = control->getMovableControl();
		if (movable) {
			movable->setPosition(movable->x() + xrel, movable->y() + yrel);
		}
	}

	if (!control) {
		if (Gui->currentCombolist()) {
			control = Gui->currentCombolist()->getControl(e->x() - Gui->currentCombolist()->x(), e->y() - Gui->currentCombolist()->y());
		} else {
			control = Gui->getControl(e->x(), e->y());
		}
	}

	if (control) {
		control->onMouseMotion(xrel, yrel, e);
	}

	QGLWidget::mouseMoveEvent(e);
}

void cGLWidget::enterEvent(QEvent *e) {
	QGLWidget::enterEvent(e);
}

void cGLWidget::leaveEvent(QEvent *e) {
	QGLWidget::leaveEvent(e);
}

void cGLWidget::checkInputFocus() {
	if (Gui->inputFocus() && !Gui->inputFocus()->isVisibleOnScreen()) {
		Gui->setInputFocus(0);
	}
}

void cGLWidget::keyPressEvent(QKeyEvent *e) {
	checkInputFocus();

	ignoreReturn = false;
	Qt::KeyboardModifiers state = e->modifiers();
	int key = e->key();

	if (key == Qt::Key_unknown) {
		return;
	}

	if (key == Qt::Key_Return && state & Qt::AltModifier) {
		showMaximized();
		//Config->setEngineWindowed(!Config->engineWindowed());
		//Engine->reload();
		//Gui->invalidate();
	} else if (key == Qt::Key_Tab) {
		if ((state & Qt::AltModifier) == 0) {
			if (Gui->inputFocus() && Gui->inputFocus()->wantTabs()) {
				Gui->inputFocus()->onKeyDown(e);
				ignoreReturn = true;
			}
		}
	} else {
		// The combo list supercedes _all_ other controls
		if (Gui->currentCombolist()) {
			if (e->key() == Qt::Key_Escape) {
				Gui->setCurrentCombolist(0);
			} else {
				Gui->currentCombolist()->onKeyDown(e);
			}
		} else {
			// Forward it to the control with the input focus. Otherwise go trough all controls.
			if (Gui->inputFocus()) {
				Gui->inputFocus()->onKeyDown(e);
				ignoreReturn = true;
			}
		}
	}

	QWidget::keyPressEvent(e);
}

void cGLWidget::keyReleaseEvent(QKeyEvent *e) {
	checkInputFocus();

	if (WorldView && WorldView->isTargetting() && e->key() == Qt::Key_Escape) {
		WorldView->cancelTarget();
		return;
	}

	// Forward it to the control with the input focus. Otherwise go trough all controls.
	if (Gui->currentCombolist()) {
		if (e->key() == Qt::Key_Escape) {
			Gui->setCurrentCombolist(0);
		} else {
			Gui->currentCombolist()->onKeyUp(e);
		}
	} else if (Gui->inputFocus()) {
		Gui->inputFocus()->onKeyUp(e);
	} else {
		if (!ignoreReturn && WorldView && WorldView->isVisible() && e->modifiers() == Qt::NoModifier && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
			WorldView->showInputLine();
		} else {
			uint key = e->key();
			if (e->modifiers() & Qt::AltModifier) {
				key |= Qt::ALT;
			}
			if (e->modifiers() & Qt::ShiftModifier) {
				key |= Qt::SHIFT;
			}
			if (e->modifiers() & Qt::ControlModifier) {
				key |= Qt::CTRL;
			}
			if (e->modifiers() & Qt::MetaModifier) {
				key |= Qt::META;
			}
			Profile->processShortcut(QKeySequence(key));
		}
	}

	QWidget::keyReleaseEvent(e);
}

void cGLWidget::mousePressEvent(QMouseEvent *e) {
	singleClickTimer.stop();

	if (mouseCapture_) {
		mouseCapture_->onMouseDown(e);
	} else {
		cControl *control;

		// If a contextmenu is currently visible and we're not clicking on something inside it,
		// eat the event and close the menu
		if (Gui->currentCombolist()) {
			control = Gui->currentCombolist()->getControl(e->x() - Gui->currentCombolist()->x(), e->y() - Gui->currentCombolist()->y());
			if (!control) {
				ignoreMouseRelease = true; // Ignore the next mouse release event
				Gui->setCurrentCombolist(0);
				return;
			}
		} else {
			control = Gui->getControl(e->x(), e->y());
		}

		if (control) {
			// Check if we can drop on this control
			if (Gui->isDragging()) {
				cDynamicItem *item = Gui->draggedItem();
				if (item && control->acceptsItemDrop(item)) {
					control->dropItem(item);
					return;
				}
			}

			mouseCapture_ = control;
			control->onMouseDown(e);			

			// If the new control wants to have the input focus, it gets it
			if (control->canHaveFocus()) {
				Gui->setInputFocus(control);
			}

			// Get the next control with the gui as its parent above it
			cContainer *parent = control->isContainer() ? (cContainer*)control : control->parent();
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
		if (control != ContextMenu && (!control || control->parent() != ContextMenu)) {
			ContextMenu->hide();
		}
	}

	QWidget::mousePressEvent(e);
}

void cGLWidget::mouseReleaseEvent(QMouseEvent *e) {
	if (ignoreMouseRelease) {
		ignoreMouseRelease = false;
		return;
	}

	if (e->button() == Qt::LeftButton && lastDoubleClick) {
		lastDoubleClick = false;
		return;
	}

	// Save this event and start the timer
	if (e->button() == Qt::LeftButton) {
		delete singleClickEvent;
		singleClickEvent = new QMouseEvent(*e);
		singleClickTimer.start();
	}
	
	cControl *control = mouseCapture_;
	mouseCapture_ = 0; // Reset mouse capture

	if (!control) {
		// If we don't have a mouse capture and aren't dragging anything.
		// return
		cDynamicItem *item = Gui->draggedItem();
		if (!item) {
			return;
		}

		// If a contextmenu is currently visible and we're not clicking on something inside it,
		// eat the event and close the menu
		if (Gui->currentCombolist()) {
			control = Gui->currentCombolist()->getControl(e->x() - Gui->currentCombolist()->x(), e->y() - Gui->currentCombolist()->y());
		} else {
			control = Gui->getControl(e->x(), e->y());
		}

		// Only check for drop events here
		if (control && control->acceptsItemDrop(item)) {
			control->dropItem(item);
		}
		return;
	}
	if (control) {
		if (control == WorldView && WorldView->targetRequest()) {
			if (e->button() == Qt::LeftButton) {
				singleClickTimer.stop();
				WorldView->onClick(e); // Directly translate to a singleclick event although it's not
			}
			return;
		}

		control->onMouseUp(e);

		if (e->button() != Qt::LeftButton) {
			control->onClick(e);
		}		
	}

	QWidget::mouseReleaseEvent(e);
}

void cGLWidget::singleClick() {
	if (singleClickEvent) {
		// Post the event to the gui system
		cControl *control = mouseCapture_;
		if (!control) {
			// If a contextmenu is currently visible and we're not clicking on something inside it,
			// eat the event and close the menu
			if (Gui->currentCombolist()) {
				control = Gui->currentCombolist()->getControl(singleClickEvent->x() - Gui->currentCombolist()->x(), singleClickEvent->y() - Gui->currentCombolist()->y());
			} else {
				control = Gui->getControl(singleClickEvent->x(), singleClickEvent->y());
			}			
		}
		if (control) {
			control->onClick(singleClickEvent);
			mouseCapture_ = 0; // Reset mouse capture
		}

		delete singleClickEvent;
		singleClickEvent = 0;
	}
}

void cGLWidget::mouseDoubleClickEvent(QMouseEvent * e) {
	lastDoubleClick = true;
	singleClickTimer.stop();

	cControl *control;

	// If a contextmenu is currently visible and we're not clicking on something inside it,
	// eat the event and close the menu
	if (Gui->currentCombolist()) {
		control = Gui->currentCombolist()->getControl(e->x() - Gui->currentCombolist()->x(), e->y() - Gui->currentCombolist()->y());
		if (!control) {
			Gui->setCurrentCombolist(0);
			return;
		}
	} else {
		control = Gui->getControl(e->x(), e->y());
	}

	if (control) {
		control->processDoubleClick(e);
	}
}

void cGLWidget::createScreenshot(const QString &filename) {
	QDir dir;

	// Make sure the Screenshots Path exists
	if (!dir.mkpath(Config->screenshotsPath())) {
		Log->print(LOG_ERROR, tr("Unable to create the screenshots path at '%1'.\n").arg(Config->screenshotsPath()));
		return;
	}

	QString realFilename = QDir(Config->screenshotsPath()).absoluteFilePath(filename);

	QImage image = grabFrameBuffer(false);
	
	// Save the image as PNG
	if (!image.save(realFilename, "PNG")) {
		if (WorldView) {
			WorldView->addSysMessage(tr("Unable to save screenshot as %1.").arg(realFilename));
		}
	} else {
		if (WorldView) {
			WorldView->addSysMessage(tr("Screenshot saved as %1.").arg(realFilename));
		}
	}
}

void cMainWindow::resizeGameWindow(unsigned int width, unsigned int height, bool locked)  {
	int mheight = m_menuBar->height();

	if (m_menuBar->isHidden()) {
		mheight = 0;
	}
       
	if (locked) {
		resize(width, height + mheight);
		setMaximumSize(size());
		setMinimumSize(size());
	} else {
		setMaximumSize(65535, 65535);
		setMinimumSize(100, 100);
		resize(width, height + mheight);
	}
}

void cMainWindow::moveEvent(QMoveEvent *event) {
	// if the game view is visible, save the engine position
	if (WorldView && WorldView->isVisible()) {
		Config->setEngineWindowX(x());
		Config->setEngineWindowY(y());
	}

	QMainWindow::moveEvent(event);
}

void cMainWindow::resizeEvent(QResizeEvent * event) {
	// if the game view is visible, change the engine size
	if (WorldView && WorldView->isVisible()) {
		int mheight = m_menuBar->height();

		if (m_menuBar->isHidden()) {
			mheight = 0;
		}

		Config->setEngineHeight(height() - mheight);
		Config->setEngineWidth(width());

		if (windowState() == Qt::WindowMaximized) {
			Config->setEngineMaximized(true);
		} else {
			Config->setEngineMaximized(false);
		}
	}	

	QMainWindow::resizeEvent(event);
}

bool cMainWindow::event(QEvent *e) {
	// if the game view is visible, change the engine size
	if (WorldView && WorldView->isVisible()) {
		int mheight = m_menuBar->height();

		if (m_menuBar->isHidden()) {
			mheight = 0;
		}

		Config->setEngineHeight(height() - mheight);
		Config->setEngineWidth(width());

		if (windowState() & Qt::WindowMaximized) {
			Config->setEngineMaximized(true);
		} else {
			Config->setEngineMaximized(false);
		}
	}

	return QMainWindow::event(e);
}

void cMainWindow::showEvent(QShowEvent *event) {
	QMainWindow::showEvent(event);
}

void cGLWidget::enableGrayShader() {
	if (shadersAvailable) {
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
	}
}

void cGLWidget::disableGrayShader() {
	if (shadersAvailable) {
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		glPopAttrib();
	}
}

bool cGLWidget::focusNextPrevChild(bool next) {
	if (!Gui->inputFocus() || !Gui->inputFocus()->wantTabs()) {
		// Switch to the next Control
		if (Gui->activeWindow()) {
			cControl *nextControl;
			if (!next) {
				nextControl = Gui->activeWindow()->getPreviousFocusControl(Gui->inputFocus());
			} else {
				nextControl = Gui->activeWindow()->getNextFocusControl(Gui->inputFocus());
			}
			// The search wrapped
			if (!nextControl) {
				if (!next) {
					nextControl = Gui->activeWindow()->getPreviousFocusControl(0);
				} else {
					nextControl = Gui->activeWindow()->getNextFocusControl(0);
				}
			}
			Gui->setInputFocus(nextControl);
		}
	}

	return true; // Always find a new tab control
}

void cMainWindow::closeEvent(QCloseEvent *e) {
	QApplication::instance()->quit();
	e->accept();
}

cMainWindow *MainWindow = 0;
cGLWidget *GLWidget = 0;
