
#if !defined(__MAINWINDOW_H__)
#define __MAINWINDOW_H__

#include <qgl.h>
#include <qmainwindow.h>

class cControl;

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

	cControl *mouseCapture; // Control which got the last mousedown event
	cControl *lastMouseMovement; // Control that got the last movement event
	void createScreenshot(const QString &filename);
};

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
    MainWindow();
    ~MainWindow();
};

extern cGLWidget *GLWidget;

#endif
