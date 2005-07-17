
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

MainWindow::MainWindow() {
	// Window Icon
	QPixmap pixmap((const char**)icon_xpm);
	setIcon(pixmap);

	// Create the File menu
	QMenu *file = new QMenu(this);
	file->addAction("E&xit", this, SLOT(close()));

	// Create a menu bar at the top of the window
	QMenuBar *menuBar = new QMenuBar(this);
	menuBar->insertItem(tr("&File"), file);

	GLWidget = new cGLWidget(this);
	setCentralWidget(GLWidget);
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

void cGLWidget::createScreenshot(const QString &filename) {
	// Read the framebuffer data
	//unsigned char *pixels = new unsigned char[4 * width() * height()];

	//glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	//QImage image(width(), height(), 32, 0);
	QImage image = grabFrameBuffer();
	
	/*unsigned char *ptr = pixels;

	// We have to flip the framebuffer data here
	for (int y = height() - 1; y >= 0; --y) {
		for (int x = 0; x < width(); ++x) {
			image.setPixel(x, y, qRgb(ptr[0], ptr[1], ptr[2]));
			ptr += 4;
		}
	}*/

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

	//delete [] pixels;
}

cGLWidget *GLWidget = 0;

