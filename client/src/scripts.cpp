
#include <Python.h>

#include "scripts.h"
#include "mainwindow.h"
#include "log.h"
#include "gui/gui.h"
#include "muls/localization.h"

#include "mainwindow.h"
#include "skills.h"
#include "network/uosocket.h"

#include "game/dynamicentity.h"
#include "game/dynamicitem.h"
#include "game/entity.h"
#include "game/groundtile.h"
#include "game/mobile.h"
#include "game/multi.h"
#include "game/statictile.h"

#include "gui/asciilabel.h"
#include "gui/control.h"
#include "gui/container.h"
#include "gui/window.h"
#include "gui/imagebutton.h"

#include "python/genericwrapper.h"
#include "python/loginterface.h"
#include "python/utilities.h"
#include "python/clientmodule.h"
#include "python/universalslot.h"
#include "python/argparser.h"

#include <QDir>
#include <QApplication>
#include <QThread>

#include <cstdlib>

cScripts::cScripts() {
	sender_ = 0;
}

cScripts::~cScripts() {
}

/*
	Setup the search path for python modules
*/
void cScripts::initializeSearchPath() {
	// Modify our search-path
	PyObject* searchpath = PySys_GetObject( "path" );
	
	QStringList elements;
	elements << "./scripts/" << "./";
	
	// Prepend our items to the searchpath
	for (int i = elements.count() - 1; i >= 0; --i) {
		PyList_Insert( searchpath, 0, PyString_FromString( elements[i].toLatin1() ) );
	}
	
	// Import site now
	PyObject* m = PyImport_ImportModule("site");
	Py_XDECREF( m );
}

void cScripts::load() {
	// Every QObject subclass has to be registered so we know it's a supported type
	qRegisterMetaType<cWindow*>("cWindow*");
	qRegisterMetaType<cControl*>("cControl*");
	qRegisterMetaType<cContainer*>("cContainer*");	
	qRegisterMetaType<cAsciiLabel*>("cAsciiLabel*");

	qRegisterMetaType<cDynamicItem*>("cDynamicItem*");
	qRegisterMetaType<cDynamicEntity*>("cDynamicEntity*");
	qRegisterMetaType<cEntity*>("cEntity*");
	qRegisterMetaType<cGroundTile*>("cGroundTile*");
	qRegisterMetaType<cMobile*>("cMobile*");
	qRegisterMetaType<cMulti*>("cMulti*");
	qRegisterMetaType<cMultiItem*>("cMultiItem*");
	qRegisterMetaType<cStaticTile*>("cStaticTile*");

	Log->print(LOG_NOTICE, tr("Starting the Python interpreter (Version: %1).\n").arg(Py_GetVersion()));

	Py_SetProgramName(QApplication::instance()->argv()[0]);
	Py_OptimizeFlag = 1; // Release only
	Py_NoSiteFlag = 1; // No import because we need to set the search path first	

	Py_Initialize(); // Start the python interpreter
	PySys_SetArgv( QApplication::instance()->argc(), QApplication::instance()->argv() );

	initializeSearchPath();

// Only install this is we're not being built for console (simulated by only enabling it in debug mode)
#if defined(NDEBUG)
	initializeLogInterface();
#endif
	initializeClientModule();
}

void cScripts::unload() {
	Log->print(LOG_NOTICE, tr("Stopping the Python interpreter.\n"));

	// Remove pending slots
	QVector<cUniversalSlot*> slotcopy = slotlist;
	slotlist.clear();

	foreach (cUniversalSlot *slot, slotcopy) {
		delete slot;
	}

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

void cScripts::addSlot(cUniversalSlot *slot) {
	slotlist.append(slot);
}

void cScripts::removeSlot(cUniversalSlot *slot) {
	for (int i = 0; i < slotlist.size(); ++i) {
		if (slotlist[i] == slot) {
			slotlist.remove(i);
			return;
		}
	}
}

void cScripts::setError(Error condition, QString message) {
	PyObject *exception;
	switch (condition) {
		default:
		case RuntimeError:
			exception = PyExc_RuntimeError;
			break;
		case AttributeError:
			exception = PyExc_AttributeError;
			break;
		case TypeError:
			exception = PyExc_TypeError;
			break;
	}

	PyErr_SetString(exception, message.toLocal8Bit());
}

bool cScripts::parseArguments(PyObject *args, const char *format, ...) {	
	bool retval;
	va_list va;
	
	va_start(va, format);
	retval = ::parseArguments(args, format, &va, 0);
	va_end(va);
	return retval;
}

cScripts *Scripts = 0;
