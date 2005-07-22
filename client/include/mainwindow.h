
#if !defined(__MAINWINDOW_H__)
#define __MAINWINDOW_H__

#include <qgl.h>
#include <q3mainwindow.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

class cControl;
class QMenuBar;

class cGLWidget : public QGLWidget {
	Q_OBJECT

public:
	cGLWidget(QWidget *parent);
	~cGLWidget();

	// Do a screenshot the next time the buffer is flipped
	void screenshot(const QString &filename = QString::null);

protected:
	int lastMouseX, lastMouseY;

    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
	void mouseMoveEvent(QMouseEvent *e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);
	void keyPressEvent(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent * e);

	cControl *mouseCapture; // Control which got the last mousedown event
	cControl *lastMouseMovement; // Control that got the last movement event
	void createScreenshot(const QString &filename);
};

class MainWindow : public Q3MainWindow {
	Q_OBJECT

	QMenuBar *m_menuBar;
public:
    MainWindow();
    ~MainWindow();

	QMenuBar *menuBar() const;

	void resizeGameWindow(unsigned int width, unsigned int height, bool locked = false);
};

extern cGLWidget *GLWidget;

inline QMenuBar *MainWindow::menuBar() const {
	return m_menuBar;
}

#endif
