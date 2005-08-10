
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
#include "dialog_config.h"

class cControl;
class QMenuBar;
class QAction;

class cConfigDialog : public QDialog {
private:
	Ui::configDialog ui;
public:
	cConfigDialog(QWidget *parent);
	void show();
};

class cGLWidget : public QGLWidget {
	Q_OBJECT

public:
	cGLWidget(QWidget *parent);
	~cGLWidget();

	// Do a screenshot the next time the buffer is flipped
	void screenshot(const QString &filename = QString::null);

public slots:
	void singleClick();

protected:
	bool lastDoubleClick;
	QTimer singleClickTimer;
	QMouseEvent *singleClickEvent;
	bool ignoreReturn;

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

class MainWindow : public QMainWindow {
	Q_OBJECT

	QMenuBar *m_menuBar;
public:
    MainWindow();
    ~MainWindow();

	QMenuBar *menuBar() const;

	void resizeGameWindow(unsigned int width, unsigned int height, bool locked = false);

protected:
	void resizeEvent(QResizeEvent * event);
	void moveEvent(QMoveEvent *event);
	void showEvent(QShowEvent *event);
	bool event(QEvent *e);

	cConfigDialog *configDialog;

	QAction *aHideStatics;
	QAction *aHideMap;
	QAction *aHideMobiles;
	QAction *aHideDynamics;

public slots:
	void menuWhere();
	void menuGameClicked(QAction *action);
};

extern cGLWidget *GLWidget;

inline QMenuBar *MainWindow::menuBar() const {
	return m_menuBar;
}

#endif
