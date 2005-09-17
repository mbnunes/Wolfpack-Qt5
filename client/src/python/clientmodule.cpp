
#include <Python.h>

#include "skills.h"
#include "gui/gui.h"
#include "muls/localization.h"
#include "python/genericwrapper.h"

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

	ADD_GLOBAL_OBJ(Gui);
	ADD_GLOBAL_OBJ(Skills);
	ADD_GLOBAL_OBJ(Localization);

	// Free our reference to the module
	Py_DECREF(module);
}
