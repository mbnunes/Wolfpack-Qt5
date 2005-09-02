
#if !defined(__MAINWINDOW_H__)
#define __MAINWINDOW_H__

#include <qgl.h>
#include <qmainwindow.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDialog>
#include <QTimer>

class cControl;
class QMenuBar;
class QAction;
class cCacheStatistics;

class cGLWidget : public QGLWidget {
	Q_OBJECT

public:
	cGLWidget(QWidget *parent);
	~cGLWidget();

	// Do a screenshot the next time the buffer is flipped
	void screenshot(const QString &filename = QString::null);

	cControl *mouseCapture() const;
	cControl *lastMouseMovement() const;
	void setMouseCapture(cControl *control);
	void setLastMouseMovement(cControl *control);

	void enableGrayShader();
	void disableGrayShader();
public slots:
	void singleClick();

protected:
	void getShaderPointers();
	uint grayShader;
	bool lastDoubleClick;
	QTimer singleClickTimer;
	QMouseEvent *singleClickEvent;
	bool ignoreReturn;

	int lastMouseX, lastMouseY;

	void checkInputFocus();
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
	bool focusNextPrevChild(bool next);

	cControl *mouseCapture_; // Control which got the last mousedown event
	cControl *lastMouseMovement_; // Control that got the last movement event
	void createScreenshot(const QString &filename);
};

class cMainWindow : public QMainWindow {
	Q_OBJECT

	QMenuBar *m_menuBar;
public:
    cMainWindow();
    ~cMainWindow();

	QMenuBar *menuBar() const;

	void resizeGameWindow(unsigned int width, unsigned int height, bool locked = false);

protected:
	void resizeEvent(QResizeEvent * event);
	void moveEvent(QMoveEvent *event);
	void showEvent(QShowEvent *event);
	bool event(QEvent *e);

	QAction *aHideStatics;
	QAction *aHideMap;
	QAction *aHideMobiles;
	QAction *aHideDynamics;

	cCacheStatistics *cacheStatistics;

public slots:
	void menuGameClicked(QAction *action);
};

extern cMainWindow *MainWindow;
extern cGLWidget *GLWidget;

inline QMenuBar *cMainWindow::menuBar() const {
	return m_menuBar;
}

inline cControl *cGLWidget::mouseCapture() const {
	return mouseCapture_;
}

inline cControl *cGLWidget::lastMouseMovement() const {
	return lastMouseMovement_;
}

inline void cGLWidget::setMouseCapture(cControl *control) {
	mouseCapture_ = control;
}

inline void cGLWidget::setLastMouseMovement(cControl *control) {
	lastMouseMovement_ = control;
}

#endif
