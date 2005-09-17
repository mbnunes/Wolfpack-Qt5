
#include <Python.h>

#include "scripts.h"
#include "mainwindow.h"
#include "log.h"
#include "gui/gui.h"
#include "muls/localization.h"

#include "mainwindow.h"
#include "skills.h"
#include "network/uosocket.h"

#include "gui/asciilabel.h"
#include "gui/control.h"
#include "gui/container.h"
#include "gui/window.h"
#include "gui/imagebutton.h"

#include "python/genericwrapper.h"
#include "python/loginterface.h"
#include "python/utilities.h"
#include "python/clientmodule.h"

#include <QDir>
#include <QApplication>
#include <QThread>

#include <cstdlib>

cScripts::cScripts() {
}

cScripts::~cScripts() {
}

/*
	Setup the search path for python modules
*/
void cScripts::initializeSearchPath() {
	putenv("PYTHONPATH=./scripts/;.");
}

void cScripts::load() {
	Log->print(LOG_NOTICE, tr("Starting the Python interpreter (Version: %1).\n").arg(Py_GetVersion()));

	Py_SetProgramName(QApplication::instance()->argv()[0]);
	Py_OptimizeFlag = 1; // Release only
	
	initializeSearchPath();

	Py_Initialize(); // Start the python interpreter
	PySys_SetArgv( QApplication::instance()->argc(), QApplication::instance()->argv() );
// Only install this is we're not being built for console (simulated by only enabling it in debug mode)
#if defined(NDEBUG)
	initializeLogInterface();
#endif
	initializeClientModule();
}

void cScripts::unload() {
	Log->print(LOG_NOTICE, tr("Stopping the Python interpreter.\n"));
	Py_Finalize();
}

static PyObject *buildArgumentTuple(const QVariantList &arguments) {
	PyObject *result = PyTuple_New(arguments.count());

	uint index = 0; // Current tuple index

	foreach (QVariant variant, arguments) {
		PyObject *value;

		switch (variant.type()) {
			case QVariant::String:
				value = toPython(variant.toString());
				break;

			case QMetaType::QObjectStar:				
				value = generateWrapper(qvariant_cast<QObject*>(variant));
				break;

			// If there is no possible translation, simply put None into the argument list
			default:
				Py_INCREF(Py_None);
				value = Py_None;
				break;
		}

		PyTuple_SET_ITEM(result, index++, value);
	}

	return result;
}

QVariant cScripts::callFunction(const QString &moduleName, const QString &functionName, const QVariantList &argumentList) {
	// Import the given module name
	PyObject *module = PyImport_ImportModule(moduleName.toLatin1().data());

	if (module) {
		// Get the function
		PyObject *function = PyObject_GetAttrString(module, functionName.toLatin1().data());

		// Check if the object with the given name really existed
		if (function) {
			// Check if the object is really callable and not a string or else
			if (PyCallable_Check(function)) {
				// Create the argument list
				PyObject *args = buildArgumentTuple(argumentList);

				// Call the function object
				PyObject *result = PyEval_CallObject(function, args);
				Py_XDECREF(result);

				Py_DECREF(args);
			}

			Py_DECREF(function);
		}

		Py_DECREF(module);
	}

	// Check for errors
	if (PyErr_Occurred()) {
		PyErr_Print();
	}

	return QVariant();
}

/*
void cScripts::load() {
	// Try to create the directory if it does not exist
	QDir dir = QDir::current();	
	if (!dir.exists("dialogs")) {
		if (!dir.mkdir("dialogs")) {
			return;
		}
	}

	// Set up the namespace
	project->addObject(MainWindow);
	project->addObject(UoSocket);
	project->addObject(Gui);
	project->addObject(Log);
	project->addObject(Localization);
	project->addObject(Skills);

	dir.cd("dialogs");
	QDir::setCurrent(dir.absolutePath());
	project->load("scripts.dat");
	dir.cdUp();
	QDir::setCurrent(dir.absolutePath());

	Log->print(LOG_MESSAGE, tr("Loaded %1 scripts from project file.\n").arg(project->scripts().count()));

	qRegisterMetaType<cWindow*>("cWindow*");
	qRegisterMetaType<cControl*>("cControl*");
	qRegisterMetaType<cContainer*>("cContainer*");	
	qRegisterMetaType<cAsciiLabel*>("cAsciiLabel*");
	QSInterpreter::registerMetaObject(&cControl::staticMetaObject);
	QSInterpreter::registerMetaObject(&cContainer::staticMetaObject);
	QSInterpreter::registerMetaObject(&cWindow::staticMetaObject);
	QSInterpreter::registerMetaObject(&cGui::staticMetaObject);
	QSInterpreter::registerMetaObject(&cAsciiLabel::staticMetaObject);
	QSInterpreter::registerMetaObject(&ButtonStatic::staticMetaObject);

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
*/
cScripts *Scripts = 0;
