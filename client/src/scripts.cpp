
#include "scripts.h"
#include "mainwindow.h"
#include "log.h"

#ifndef QSA_NO_IDE
#include <qsworkbench.h>
static QSWorkbench *uoclient_ide = 0;
#endif

#include <qsinputdialogfactory.h>
#include <qsinterpreter.h>
#include <qsobjectfactory.h>
#include <QDir>

#include "mainwindow.h"
#include "network/uosocket.h"

cScripts::cScripts() {
	project = new QSProject(this, "qsproject");
	project->setStorageMode(QSProject::Bundle);
}

cScripts::~cScripts() {
}

void cScripts::load() {
	project->load("scripts.dat");
	Log->print(LOG_MESSAGE, tr("Loaded %1 scripts from project file.\n").arg(project->scripts().count()));

	// Set up the namespace
	project->addObject(MainWindow);
	project->addObject(UoSocket);

    QSInterpreter *ip = project->interpreter();
    ip->addObjectFactory(new QSInputDialogFactory);
}

void cScripts::unload() {
	project->save();
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

cScripts *Scripts = 0;

