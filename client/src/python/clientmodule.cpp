
#include <Python.h>

#include "skills.h"
#include "utilities.h"
#include "scripts.h"
#include "dialogs/login.h"
#include "gui/gui.h"
#include "muls/localization.h"
#include "muls/gumpart.h"
#include "muls/tiledata.h"
#include "python/genericwrapper.h"
#include "python/utilities.h"
#include "python/universalslot.h"
#include "network/uosocket.h"
#include "network/network.h"
#include "game/mobile.h"
#include "game/world.h"

/*
	Connect a QObject Signal to a QObject Slot
	or
	Connect a QObject Signal to a Python Callable
*/
static PyObject *connect(PyObject *self, PyObject *args) {
	PyObject *sender, *recipient;
	PyObject *pySignal, *pySlot = 0;

	if (!PyArg_ParseTuple(args, "O!OO|O", &pyWrapperType, &sender, &pySignal, &recipient, &pySlot)) {
		return 0;
	}

	QString signal = fromPythonToString(pySignal);

	// If no slot is specified, reciepient has to be a callable
	if (!pySlot && !PyCallable_Check(recipient)) {
		PyErr_SetString(PyExc_TypeError, "client.connect(sender, signal, recipient): recipient has to be a callable python object.");
		return 0;
	}
	// If the slot is specified, recipient has to be a QObject
	if (pySlot && !isGenericWrapper(recipient)) {
		PyErr_SetString(PyExc_TypeError, "client.connect(sender, signal, recipient, slot): slot has to be a wrapped QObject.");
		return 0;
	}

	// Connect using the universal connector
	if (!pySlot) {
		QObject *qSender = getWrappedObject(sender);
		if (!qSender) {
			PyErr_SetString(PyExc_RuntimeError, "client.connect(sender, signal, recipient): sender has already been freed.");
			return 0;
		}

		cUniversalSlot *slot = new cUniversalSlot(qSender, signal.toLocal8Bit(), recipient);
		if (!slot->isValid()) {
			delete slot;
			PyErr_Format(PyExc_RuntimeError, "client.connect(sender, signal, recipient): Trying to connect to unknown signal: %s.", signal.toLocal8Bit().data());
			return 0;
		}
	}

	Py_RETURN_TRUE;
}

/*
	Disconnect a QObject Signal from a QObject Slot
	or
	Disconnect a QObject Signal from a Python Callable
*/
static PyObject *disconnect(PyObject *self, PyObject *args) {
	PyObject *sender, *recipient;

	if (!PyArg_ParseTuple(args, "O!O", &pyWrapperType, &sender, &recipient)) {
		return 0;
	}

	// If no slot is specified, reciepient has to be a callable
	if (!PyCallable_Check(recipient)) {
		PyErr_SetString(PyExc_TypeError, "client.disconnect(sender, signal, recipient): recipient has to be a callable python object.");
		return 0;
	}

	// Connect using the universal connector
	QObject *qSender = getWrappedObject(sender);
	if (!qSender) {
		PyErr_SetString(PyExc_RuntimeError, "client.disconnect(sender, signal, recipient): sender has already been freed.");
		return 0;
	}

	// Find the slot connected to the given object
	QVector<cUniversalSlot*> slotlist = Scripts->getSlotlist();

	foreach (cUniversalSlot *slot, slotlist) {
		if (slot->recipient() == recipient) {
			qSender->disconnect(slot);
			delete slot;
		}
	}
	Py_RETURN_TRUE;
}

static PyObject *random(PyObject *self, PyObject *args) {
	if (PyTuple_Size(args) == 2) {
		int mi, ma;
		if (!PyArg_ParseTuple(args, "ii", &mi, &ma)) {
			return 0;
		}

		return PyInt_FromLong(mi + Random->randInt(ma - mi));
	}

	Py_RETURN_NONE;
}

static PyObject *sender(PyObject *self, PyObject *args) {
	return generateWrapper(Scripts->sender());
}

static PyMethodDef clientMethods[] = {
	{"connect",		connect,	METH_VARARGS, 0},
	{"disconnect",	disconnect, METH_VARARGS, 0},
	{"random",		random,		METH_VARARGS, 0},
	{"sender",		sender,		METH_VARARGS, 0},
	{0, 0, 0, 0}
};

/*
	This is a specialized version of the Py_InitModule function.
*/
void addModuleMethods(PyObject *m) {
	PyObject *d = PyModule_GetDict(m);
	PyMethodDef *ml;

	PyObject *n = PyString_FromString("client");
	if (n == NULL)
		return;

	for (ml = &clientMethods[0]; ml->ml_name != NULL; ml++) {
		PyObject *v = PyCFunction_NewEx(ml, 0, n);
		if (v == NULL)
			return;
		if (PyDict_SetItemString(d, ml->ml_name, v) != 0) {
			Py_DECREF(v);
			return;
		}
		Py_DECREF(v);
	}
}

void initializeClientModule() {
	// Look for an "uoclient" module to modify or a new one to create
	PyObject *moduleName = PyString_FromString("client");
	PyObject *module = PyImport_Import(moduleName);
	Py_DECREF(moduleName);

	// If no module was found, create an empty one
	if (!module) {
		if (PyErr_Occurred()) {
			PyErr_Clear();
		}

		module = PyImport_AddModule("client");
		Py_INCREF(module);
	}

	// Add the global toplevel objects
	PyObject *obj;

#define ADD_GLOBAL_OBJ(object) obj = generateWrapper(object); \
	PyObject_SetAttrString(module, #object, generateWrapper(object)); \
	Py_DECREF(obj);

	// Add objects to the module
	ADD_GLOBAL_OBJ(Gui);
	ADD_GLOBAL_OBJ(Skills);
	ADD_GLOBAL_OBJ(Localization);
	ADD_GLOBAL_OBJ(Tiledata);
	ADD_GLOBAL_OBJ(Gumpart);
	ADD_GLOBAL_OBJ(LoginDialog);
	ADD_GLOBAL_OBJ(UoSocket);
	ADD_GLOBAL_OBJ(Network);
	ADD_GLOBAL_OBJ(Player);
	ADD_GLOBAL_OBJ(World);
	
	// Add Methods to the module
	addModuleMethods(module);

	// Free our reference to the module
	Py_DECREF(module);
}
