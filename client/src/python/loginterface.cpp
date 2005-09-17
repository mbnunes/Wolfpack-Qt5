
#include "python/loginterface.h"
#include "python/utilities.h"
#include "log.h"
#include <QString>

PyMethodDef methods[]; // Forward declaration (?)

static void logInterfaceDealloc(PyObject *v) {
	PyObject_DEL(v);
}

// The type object for this class
static PyTypeObject logInterfaceType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"loginterface",
	sizeof(PyObject),
	0,
	logInterfaceDealloc,			/* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_compare */
	0,								/* tp_repr */
	0,								/* tp_as_number */
	0,								/* tp_as_sequence */
	0,								/* tp_as_mapping */
	0,								/* tp_hash */
    0,								/* tp_call */
    0,								/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,								/* tp_setattro */
	0,								/* tp_as_buffer */
	0,								/* tp_flags */
	0,								/* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	0,								/* tp_iter */
	0,								/* tp_iternext */
	methods,						/* tp_methods */
	0,								/* tp_members */
	0,								/* tp_getset */
	0,								/* tp_base */
	0,								/* tp_dict */
	0,								/* tp_descr_get */
	0,								/* tp_descr_set */
	0,								/* tp_dictoffset */
	0,								/* tp_init */
	0,								/* tp_alloc */
	0,								/* tp_new */
	0,           					/* tp_free */
};

// Create an object of our log interface class here
void initializeLogInterface() {
	PyObject *logInterface = PyObject_New(PyObject, &logInterfaceType);
	PySys_SetObject("stderr", logInterface);
	PySys_SetObject("stdout", logInterface);
	PySys_SetObject("__stderr__", logInterface);
	PySys_SetObject("__stdout__", logInterface);
	Py_DECREF(logInterface);
}

static PyObject* logInterface_write(PyObject* self, PyObject* args) {
	PyObject *line;
	if (!PyArg_ParseTuple(args, "O", &line)) {
		return 0;
	}

	Log->print(LOG_MESSAGE, fromPythonToString(line), false);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef methods[] = {
	{"write", logInterface_write, METH_VARARGS, 0},
	{0, 0, 0, 0} // Terminator
};
