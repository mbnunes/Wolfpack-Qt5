
#include "pyaction.h"

cActionPythonCode::cActionPythonCode(PyObject *code, PyObject *args) {
	this->code = code;
	this->args = args;

	// Keep references around of both objects
	Py_INCREF(this->code);
	Py_INCREF(this->args);
}

cActionPythonCode::~cActionPythonCode() {
	// Free both objects.
	Py_DECREF(this->code);
	Py_DECREF(this->args);
}

void cActionPythonCode::execute() {
	// Execute the code object
	if (PyCallable_Check(code)) {
		PyObject *result = PyObject_CallObject(code, args);
		Py_XDECREF(result);
		reportPythonError();
	}
}
