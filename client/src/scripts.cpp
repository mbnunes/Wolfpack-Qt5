
#include "scripts.h"
#include "mainwindow.h"
#include "log.h"
#include "gui/gui.h"

#ifndef QSA_NO_IDE
#include <qsworkbench.h>
static QSWorkbench *uoclient_ide = 0;
#endif

#include <qsinputdialogfactory.h>
#include <qsinterpreter.h>
#include <qsobjectfactory.h>
#include <qswrapperfactory.h>
#include <QDir>

#include "mainwindow.h"
#include "network/uosocket.h"

#include "gui/asciilabel.h"
#include "gui/control.h"
#include "gui/container.h"
#include "gui/window.h"

class cObjectFactory : public QSObjectFactory {
public:
	cObjectFactory() {
		registerClass("cControl", &cControl::staticMetaObject);
		registerClass("cContainer", &cContainer::staticMetaObject);
		registerClass("cWindow", &cWindow::staticMetaObject);
		registerClass("cAsciiLabel", &cAsciiLabel::staticMetaObject);
	}

	virtual QObject *create(const QString &className, const QVariantList &arguments, QObject *context) {
		if (className == "cContainer") {
			return new cContainer();
		} else if (className == "cWindow") {
			return new cWindow();
		} else if (className == "cControl") {
			return new cControl();
		} else if (className == "cAsciiLabel") {
			return new cAsciiLabel("");
		} else {
			return 0;
		}
	}
};

cScripts::cScripts() {
	project = new QSProject(this, "qsproject");
	project->setStorageMode(QSProject::TextFiles);
}

cScripts::~cScripts() {
}

void cScripts::load() {
	// Try to create the directory if it does not exist
	QDir dir = QDir::current();	
	if (!dir.exists("dialogs")) {
		if (!dir.mkdir("dialogs")) {
			return;
		}
	}
	
	dir.cd("dialogs");
	QDir::setCurrent(dir.absolutePath());
	project->load("scripts.dat");
	dir.cdUp();
	QDir::setCurrent(dir.absolutePath());

	Log->print(LOG_MESSAGE, tr("Loaded %1 scripts from project file.\n").arg(project->scripts().count()));

	// Set up the namespace
	project->addObject(MainWindow);
	project->addObject(UoSocket);
	project->addObject(Gui);
	project->addObject(Log);

	qRegisterMetaType<cWindow*>("cWindow*");
	qRegisterMetaType<cControl*>("cControl*");
	qRegisterMetaType<cContainer*>("cContainer*");	
	qRegisterMetaType<cAsciiLabel*>("cAsciiLabel*");
	QSInterpreter::registerMetaObject(&cControl::staticMetaObject);
	QSInterpreter::registerMetaObject(&cContainer::staticMetaObject);
	QSInterpreter::registerMetaObject(&cWindow::staticMetaObject);
	QSInterpreter::registerMetaObject(&cGui::staticMetaObject);
	QSInterpreter::registerMetaObject(&cAsciiLabel::staticMetaObject);

	QSInterpreter *ip = project->interpreter();	
    ip->addObjectFactory(new QSInputDialogFactory);
	ip->addObjectFactory(new cObjectFactory);
}

void cScripts::unload() {
	// Try to create the directory if it does not exist
	QDir dir = QDir::current();	
	if (!dir.exists("dialogs")) {
		if (!dir.mkdir("dialogs")) {
			return;
		}
	}
	
	dir.cd("dialogs");
	QDir::setCurrent(dir.absolutePath());
	project->commitEditorContents();
	project->save();
	dir.cdUp();
	QDir::setCurrent(dir.absolutePath());	
}

void cScripts::showWorkbench() {
#ifndef QSA_NO_IDE
    // open the QSA Workbench
	if ( !uoclient_ide ) uoclient_ide = new QSWorkbench( project, MainWindow, "qside" );
    uoclient_ide->open();
#else
    QMessageBox::information( this, "Disabled feature",
			      "QSA Workbench has been disabled. Reconfigure to enable",
			      QMessageBox::Ok );
#endif
}

QVariant cScripts::callStaticMethod(QString className, QString methodName, QVariantList arguments) {
	return project->interpreter()->call(className + "." + methodName, arguments);
}

bool cScripts::classExists(QString className) {
	return project->interpreter()->hasClass(className);
}

cScripts *Scripts = 0;

