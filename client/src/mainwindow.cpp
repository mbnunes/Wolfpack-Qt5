
#include "mainwindow.h"
#include "gui/gui.h"
#include "gui/cursor.h"
#include "gui/contextmenu.h"
#include "gui/worldview.h"
#include "game/world.h"
#include "config.h"
#include <qpixmap.h>
#include <qcursor.h>
#include <qimage.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qmenubar.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QErrorMessage>

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

#include "dialog_config.h"

MainWindow::MainWindow() {
	configDialog = new cConfigDialog(this);

	resize(640, 480); // Default size

	// Window Icon
	QPixmap pixmap((const char**)icon_xpm);
	setIcon(pixmap);

	// Create the File menu
	QMenu *file = new QMenu(this);
	file->addAction("E&xit", this, SLOT(close()));

	// Create a menu bar at the top of the window
	m_menuBar = new QMenuBar(this);
	m_menuBar->insertItem(tr("&File"), file);
	
	// Game Menu
	QMenu *game = m_menuBar->addMenu("&Game");
	game->addAction("Where", this, SLOT(menuWhere()));
	game->addSeparator();
	aHideStatics = game->addAction("Hide Statics");
	aHideStatics->setCheckable(true);
	aHideDynamics = game->addAction("Hide Dynamics");
	aHideDynamics->setCheckable(true);
	aHideMobiles = game->addAction("Hide Mobiles");
	aHideMobiles->setCheckable(true);
	aHideMap = game->addAction("Hide Map");	
	aHideMap->setCheckable(true);

	aHideMap->setChecked(Config->gameHideMap());
	aHideStatics->setChecked(Config->gameHideStatics());
	aHideDynamics->setChecked(Config->gameHideDynamics());
	aHideMobiles->setChecked(Config->gameHideMobiles());

	connect(game, SIGNAL(triggered(QAction*)), this, SLOT(menuGameClicked(QAction*)));

	setMenuBar(m_menuBar);

	GLWidget = new cGLWidget(this);
	setCentralWidget(GLWidget);
}

void MainWindow::menuGameClicked(QAction *action) {
	if (action == aHideStatics) {
		Config->setGameHideStatics(action->isChecked());
	} else if (action == aHideMap) {
		Config->setGameHideMap(action->isChecked());
	} else if (action == aHideDynamics) {
		Config->setGameHideDynamics(action->isChecked());
	} else if (action == aHideMobiles) {
		Config->setGameHideMobiles(action->isChecked());
	}
}

void MainWindow::menuWhere() {
	QString message = QString("%1,%2,%3,%4").arg(World->x()).arg(World->y()).arg(World->z()).arg(World->facet());

	// Show a window with the current location
	QErrorMessage *error = new QErrorMessage(this);
	error->message(message);
}

MainWindow::~MainWindow() {
}

cGLWidget::cGLWidget(QWidget *parent) : QGLWidget(parent) {
	// Cursor stuff
	setCursor(Qt::BlankCursor);

	lastMouseX = 0;
	lastMouseY = 0;

	setMouseTracking(true);

	mouseCapture = 0; // Control which got the last mousedown event
	lastMouseMovement = 0; // Control that got the last movement event

	setFocusPolicy(Qt::WheelFocus);
}

cGLWidget::~cGLWidget() {
}

void cGLWidget::initializeGL() {
	// Initialize OpenGL
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fill the screen with a black color
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

	cControl *motionControl = Gui->getControl(e->x(), e->y());
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
			movable->setPosition(movable->x() + xrel, movable->y() + yrel);
		}
	}

	if (!control) {
		control = Gui->getControl(e->x(), e->y());
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

void cGLWidget::keyPressEvent(QKeyEvent *e) {
	Qt::ButtonState state = e->state();
	int key = e->key();

	if (key == Qt::Key_Return && state & Qt::AltButton) {
		showMaximized();
		//Config->setEngineWindowed(!Config->engineWindowed());
		//Engine->reload();
		//Gui->invalidate();
	} else if (key == Qt::Key_Tab && (state & Qt::AltButton) == 0) {
		if (!Gui->inputFocus() || !Gui->inputFocus()->wantTabs()) {
			// Switch to the next Control
			if (Gui->activeWindow()) {
				cControl *nextControl;
				if ((state & Qt::ShiftButton) == Qt::ShiftButton) {
					nextControl = Gui->activeWindow()->getPreviousFocusControl(Gui->inputFocus());
				} else {
					nextControl = Gui->activeWindow()->getNextFocusControl(Gui->inputFocus());
				}
				Gui->setInputFocus(nextControl);
			}
		} else if (Gui->inputFocus()) {
			Gui->inputFocus()->onKeyDown(e);
		}
	} else {
		// Forward it to the control with the input focus. Otherwise go trough all controls.
		if (Gui->inputFocus()) {
			Gui->inputFocus()->onKeyDown(e);
		}
	}

	QWidget::keyPressEvent(e);
}

void cGLWidget::keyReleaseEvent(QKeyEvent *e) {
	// Forward it to the control with the input focus. Otherwise go trough all controls.
	if (Gui->inputFocus()) {
		Gui->inputFocus()->onKeyUp(e);
	}

	// Create a screenshot
	if (e->key() == Qt::Key_Print) {
		e->accept(); // Accept the event

		// Generate a nice filenamd and save the screenshot
		QDateTime current = QDateTime::currentDateTime();
		QString screenshotFilename = QString("screenshot-%1.jpg").arg(current.toString( "yyyyMMdd-hhmmss" ));
		createScreenshot(screenshotFilename);		
	}

	QWidget::keyReleaseEvent(e);
}

void cGLWidget::mousePressEvent(QMouseEvent *e) {
	if (mouseCapture) {
		mouseCapture->onMouseDown(e);
	} else {
		cControl *control = Gui->getControl(e->x(), e->y());
		if (control) {
			control->onMouseDown(e);
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
		if (control != ContextMenu && (!control || control->parent() != ContextMenu)) {
			ContextMenu->hide();
		}
	}

	QWidget::mousePressEvent(e);
}

void cGLWidget::mouseReleaseEvent(QMouseEvent *e) {
	cControl *control = mouseCapture;
	if (!control) {
		control = Gui->getControl(e->x(), e->y());
	}
	if (control) {
		control->onMouseUp(e);
		mouseCapture = 0; // Reset mouse capture
	}

	QWidget::mouseReleaseEvent(e);
}

void cGLWidget::mouseDoubleClickEvent(QMouseEvent * e) {
	cControl *control = control = Gui->getControl(e->x(), e->y());
	if (control) {
		control->processDoubleClick(e);
	}
	QWidget::mouseDoubleClickEvent(e);
}

void cGLWidget::createScreenshot(const QString &filename) {
	QImage image = grabFrameBuffer();
	
	// Save the image as JPG
	if (!image.save(filename, "JPEG")) {
		if (WorldView) {
			WorldView->addSysMessage(tr("Unable to save screenshot as %1.").arg(filename));
		}
	} else {
		if (WorldView) {
			WorldView->addSysMessage(tr("Screenshot saved as %1.").arg(filename));
		}
	}
}

void MainWindow::resizeGameWindow(unsigned int width, unsigned int height, bool locked)  {
	int mheight = m_menuBar->height();

	if (m_menuBar->isHidden()) {
		mheight = 0;
	}
       
	if (locked) {
		resize(width, height + mheight);
		layout()->setResizeMode( QLayout::FreeResize );
		setMaximumSize(size());
		setMinimumSize(size());
	} else {
		layout()->setResizeMode( QLayout::FreeResize );
		setMaximumSize(65535, 65535);
		setMinimumSize(100, 100);
		resize(width, height + mheight);
	}
}

void MainWindow::moveEvent(QMoveEvent *event) {
	// if the game view is visible, save the engine position
	if (WorldView && WorldView->isVisible()) {
		Config->setEngineWindowX(x());
		Config->setEngineWindowY(y());
	}

	QMainWindow::moveEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent * event) {
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

bool MainWindow::event(QEvent *e) {
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

void MainWindow::showEvent(QShowEvent *event) {
	QMainWindow::showEvent(event);
}

cConfigDialog::cConfigDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);
}

void cConfigDialog::show() {
	// Load the current configuration settings into this
	

	QDialog::show();
}

cGLWidget *GLWidget = 0;
