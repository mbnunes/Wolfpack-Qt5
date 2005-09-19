
#include "python/genericwrapper.h"
#include "python/utilities.h"
#include "log.h"
#include <QMetaProperty>
#include <QVector>
#include <QStringList>

/*
	This object wraps a qobject method.
*/
struct pyMethodWrapperObject {
	PyObject_HEAD
	QObject *wrapped;
	uint wrappedMethodsCount;
	uint *wrappedMethods;
	cGenericWrapperHelper *helper;
};

/*
	Destructor for our generic QObject method wrapper
*/
static void methodwrapper_dealloc(pyMethodWrapperObject *obj) {
	delete obj->helper;
	delete [] obj->wrappedMethods;
	obj->wrapped = 0;
	obj->ob_type->tp_free(obj);
}

// There are at most 10 arguments so this speeds things up a lot
// for elemental datatypes
static int intArguments[11];
static uint uintArguments[11];
static ushort ushortArguments[11];
static short shortArguments[11];
static uchar ucharArguments[11];
static char charArguments[11];
static QString qstringArguments[11];
static bool boolArguments[11];
static void *ptrArguments[11];
static QStringList stringlistArguments[11];

/*
	Allocate the argument pointer space used by converted arguments.
*/
static void allocPythonToQt(void **ptr, uint index, QString qtType, PyObject *pyObj, QMetaType::Type *type) {
	// Special handling for void return types
	if (qtType == "" && !pyObj) {
		ptr[index] = 0;
		return;
	}

	// Strip "const " from the start if neccesary
	if (qtType.startsWith("const ")) {
		qtType = qtType.mid(6);
	}

	// Integer types
	if (qtType == "int") {
		ptr[index] = &intArguments[index];
		if (pyObj) {
			intArguments[index] = fromPythonToInt(pyObj);
		}
		if (type) {
			*type = QMetaType::Int;
		}
	} else if (qtType == "uint") {
		ptr[index] = &uintArguments[index];
		if (pyObj) {
			uintArguments[index] = fromPythonToUInt(pyObj);
		}
		if (type) {
			*type = QMetaType::UInt;
		}
	} else if (qtType == "unsigned short") {
		ptr[index] = &ushortArguments[index];
		if (pyObj) {
			ushortArguments[index] = fromPythonToUInt(pyObj);
		}
		if (type) {
			*type = QMetaType::UShort;
		}
	} else if (qtType == "short") {
		ptr[index] = &shortArguments[index];
		if (pyObj) {
			shortArguments[index] = fromPythonToInt(pyObj);
		}
		if (type) {
			*type = QMetaType::Short;
		}
	} else if (qtType == "unsigned char") {
		ptr[index] = &ucharArguments[index];
		if (pyObj) {
			ucharArguments[index] = fromPythonToUInt(pyObj);
		}
		if (type) {
			*type = QMetaType::UChar;
		}
	} else if (qtType == "char") {
		ptr[index] = &charArguments[index];
		if (pyObj) {
			charArguments[index] = fromPythonToInt(pyObj);
		}
		if (type) {
			*type = QMetaType::Char;
		}
	} else if (qtType == "QString") {
		ptr[index] = &qstringArguments[index];
		if (pyObj) {
			qstringArguments[index] = fromPythonToString(pyObj);
		}
		if (type) {
			*type = QMetaType::QString;
		}
	} else if (qtType == "bool") {
		ptr[index] = &boolArguments[index];
		if (pyObj) {
			boolArguments[index] = PyObject_IsTrue(pyObj);
		}
		if (type) {
			*type = QMetaType::Bool;
		}
	} else if (qtType == "QStringList") {
		ptr[index] = &stringlistArguments[index];
		stringlistArguments[index].clear();
		if (pyObj) {
			if (PyTuple_Check(pyObj)) {
				for (int i = 0; i < PyTuple_GET_SIZE(pyObj); ++i) {
					PyObject *str = PyObject_Str(PyTuple_GET_ITEM(pyObj, i));
					if (str) {
						stringlistArguments[index].append(fromPythonToString(str));
						Py_DECREF(str);
					} else {
						stringlistArguments[index].append(QString::null);
					}
				}
			} else if (PyList_Check(pyObj)) {
				for (int i = 0; i < PyList_GET_SIZE(pyObj); ++i) {
					PyObject *str = PyObject_Str(PyList_GET_ITEM(pyObj, i));
					if (str) {
						stringlistArguments[index].append(fromPythonToString(str));
						Py_DECREF(str);
					} else {
						stringlistArguments[index].append(QString::null);
					}
				}
			}
		}
		if (type) {
			*type = (QMetaType::Type)QVariant::StringList;
		}
	} else if (qtType.endsWith("*")) {
		if (isGenericWrapper(pyObj)) {
			// Pointer type, get the genericwrapper for our pyobj and see if we can cast, otherwise zero it out
			ptr[index] = &ptrArguments[index];
			if (pyObj) {
				ptrArguments[index] = getWrappedObject(pyObj);
			}
			if (type) {
				*type = QMetaType::QObjectStar;
			}
		} else if (!pyObj) {
			// Use the void* return type...
			ptr[index] = &ptrArguments[index];
			if (type) {
				*type = QMetaType::QObjectStar;
			}
		}
	} else {
		// ERROR!?
		ptr[index] = 0;
	}
}

/*
	Function managing the call ability of our method wrapper.
*/
static PyObject *methodwrapper_call(pyMethodWrapperObject *obj, PyObject *args, PyObject *kws) {
	QObject *wrapped = obj->wrapped;

	// Error checking for deleted objects
	if (!wrapped) {
		PyErr_Format(PyExc_RuntimeError, "Unable to call methods on an object that has already been deleted.");
		return 0;
	}

	// Iterate over the methods found in the wrapped object and find a suitable overload
	const QMetaObject *meta = wrapped->metaObject();

	// Save the number of passed arguments
	uint passedArguments = PyTuple_Size(args);

	// Limit the number of arguments for internal slots to 10.
	// This makes several things a LOT easier.
	if (passedArguments > 10) {
		PyErr_Format(PyExc_RuntimeError, "The maximum number of 10 arguments for an internal slot has been exceeded.");
		return 0;
	}

	// Iterate over the methods for this object
	for (uint i = 0; i < obj->wrappedMethodsCount; ++i) {
		QMetaMethod method = meta->method(obj->wrappedMethods[i]);
		QList<QByteArray> parameters = method.parameterTypes();

		// Skip this method if the number of arguments doesn't match
		if (parameters.count() != passedArguments) {
			continue;
		}

		// Check the argument types for compatibility
		uint pi = 0;
		bool compatible = true;
		foreach (QByteArray parameterType, parameters) {
			// Check if the input pyobject can be directly converted into the target type
			if (!canConvert(PyTuple_GET_ITEM(args, pi++), parameterType)) {
				compatible = false;
				break;
			}
		}

		if (!compatible) {
			continue; // One of the types doesn't match
		}

		QString returnType = method.typeName();
		if (returnType.startsWith("const ")) {
			returnType = returnType.mid(6);
		}

		// Create a list of pointers for the arguments
		void *paramList[11];
		QMetaType::Type returnValueType = QMetaType::Void;
		allocPythonToQt(paramList, 0, returnType, 0, &returnValueType); // Create return value

		pi = 0;
		foreach (QByteArray parameterType, parameters) {
			allocPythonToQt(paramList, pi+1, parameterType, PyTuple_GET_ITEM(args, pi), 0);
			++pi;
		}

		wrapped->qt_metacall(QMetaObject::InvokeMetaMethod, obj->wrappedMethods[i], paramList);

		return toPython(paramList[0], returnValueType);
	}

	PyErr_SetString(PyExc_RuntimeError, "Did not found a suitable overload for calling the method with the given arguments.");
	return 0;
}

/*
	The type information for our generic wrapper type.
*/
PyTypeObject pyMethodWrapperType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"genericmethodwrapper",
	sizeof(pyMethodWrapperObject),
	0,
	(destructor)methodwrapper_dealloc,	/* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_compare */
	0,								/* tp_repr */
	0,    							/* tp_as_number */
	0,								/* tp_as_sequence */
	0,								/* tp_as_mapping */
	0, 								/* tp_hash */
	(ternaryfunc)methodwrapper_call,	/* tp_call */
	0,								/* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	0,								/* tp_as_buffer */
	Py_TPFLAGS_BASETYPE,			/* tp_flags */
	0,								/* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	0,								/* tp_iter */
	0,								/* tp_iternext */
	0,								/* tp_methods */
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
	PyObject_Del,           		/* tp_free */
};

/*
	Create a wrapper object for the passed QObject and return it.
*/
PyObject *generateMethodWrapper(QObject *object, const QVector<uint> &methods) {
	pyMethodWrapperObject *wrapper = PyObject_New(pyMethodWrapperObject, &pyMethodWrapperType);
	wrapper->helper = new cGenericWrapperHelper((PyObject*)wrapper, object);
	wrapper->wrapped = object;
	wrapper->wrappedMethods = new uint[methods.size()];
	for (int i = 0; i < methods.size(); ++i) {
		wrapper->wrappedMethods[i] = methods[i];
	}
	wrapper->wrappedMethodsCount = methods.size();
	return (PyObject*)wrapper;
}

/*
	The Python object representing our wrapper.
*/
struct pyWrapperObject {
	PyObject_HEAD
	QObject *wrapped; // The object being wrapped by us
	cGenericWrapperHelper *helper; // The object receiving the destroy signal from the wrapped object
};

/*
	Destructor for our generic QObject wrapper
*/
static void wrapper_dealloc(pyWrapperObject *obj) {
	delete obj->helper;
	obj->wrapped = 0;
	obj->ob_type->tp_free(obj);
}

PyObject* wrapper_get(pyWrapperObject* self, char* name);
int wrapper_set(pyWrapperObject* self, char* name, PyObject* value);

/*
	The type information for our generic wrapper type.
*/
PyTypeObject pyWrapperType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"genericwrapper",
	sizeof(pyWrapperObject),
	0,
	(destructor)wrapper_dealloc,	/* tp_dealloc */
	0,								/* tp_print */
	(getattrfunc)wrapper_get,		/* tp_getattr */
	(setattrfunc)wrapper_set,		/* tp_setattr */
	0,								/* tp_compare */
	0,								/* tp_repr */
	0,    							/* tp_as_number */
	0,								/* tp_as_sequence */
	0,								/* tp_as_mapping */
	0, 								/* tp_hash */
	0,								/* tp_call */
	0,								/* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	0,								/* tp_as_buffer */
	Py_TPFLAGS_BASETYPE,			/* tp_flags */
	0,								/* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	0,								/* tp_iter */
	0,								/* tp_iternext */
	0,								/* tp_methods */
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
	PyObject_Del,           		/* tp_free */
};

/*
	Check if the given object is a generic wrapper.
*/
bool isGenericWrapper(PyObject *obj) {
	if (obj) {
		return obj->ob_type == &pyWrapperType;
	} else {
		return false;
	}
}

/*
	Return the QObject wrapped by the given py object.
*/
QObject *getWrappedObject(PyObject *obj) {
	return ((pyWrapperObject*)obj)->wrapped;
}

/*
	Create a wrapper object for the passed QObject and return it.
*/
PyObject *generateWrapper(QObject *object) {
	pyWrapperObject *wrapper = PyObject_New(pyWrapperObject, &pyWrapperType);
	wrapper->helper = new cGenericWrapperHelper((PyObject*)wrapper, object);
	wrapper->wrapped = object;
	return (PyObject*)wrapper;
}

/*
	Attribute getter for the generic wrapper class.
*/
static PyObject* wrapper_get(pyWrapperObject* self, char* name) {
	QObject *wrapped = self->wrapped;

	// Error checking for deleted objects
	if (!wrapped) {
		PyErr_Format(PyExc_AttributeError, "Unable to retrieve attribute %s since the wrapped object has already been deleted.", name);
		return 0;
	}

	// Check if the wrapped object has a property with the given name and return it
	const QMetaObject *meta = wrapped->metaObject();
	int propertyIndex = meta->indexOfProperty(name);
	
	// Found a property, now return it
	if (propertyIndex != -1) {
		// Get the property information
		QMetaProperty property = meta->property(propertyIndex);
		// Check if the property is readable
		if (property.isReadable()) {
			// Read the property from our wrapped object
			QVariant variant = property.read(wrapped);

			// Convert variant to PyObject*
			PyObject *result = toPython(variant);
			if (result) {
				return result;
			}			
		}
		PyErr_SetString(PyExc_AttributeError, name);
		return 0;
	}

	QVector<uint> methods; // Offsets for suitable methods
	// To speed up the search in the method wrapper we collect a list of suitable indices here
	for (int i = 0; i < meta->methodOffset() + meta->methodCount(); ++i) {
		QMetaMethod method = meta->method(i);
		if (method.access() == QMetaMethod::Public && method.methodType() == QMetaMethod::Slot) {
			// Check for name( in the signature
			if (QString(method.signature()).startsWith(name, Qt::CaseSensitive)) {
				// Check if there is a ( at the end of the name
				if (method.signature()[strlen(name)] == '(') {
					methods.append(i);
				}
			}
		}
	}

	// If we found suitable method ids, return a wrapper for them
	if (!methods.isEmpty()) {
		return generateMethodWrapper(wrapped, methods);
	}

	// Look for Enumerators on the class
	for (int i = 0; i < meta->enumeratorOffset() + meta->enumeratorCount(); ++i) {
		QMetaEnum enumerator = meta->enumerator(i);
		int result = enumerator.keyToValue(name);

		if (result != -1) {
			return PyInt_FromLong(result);
		}
	}

	PyErr_SetString(PyExc_AttributeError, name);
	return 0;
}

/*
	Attribute setter for the generic wrapper class.
*/
static int wrapper_set(pyWrapperObject* self, char* name, PyObject* value) {
	QObject *wrapped = self->wrapped;

	// Error checking for deleted objects
	if (!wrapped) {
		PyErr_Format(PyExc_AttributeError, "Unable to retrieve attribute %s since the wrapped object has already been deleted.", name);
		return 0;
	}

	// Check if the wrapped object has a property with the given name and return it
	const QMetaObject *meta = wrapped->metaObject();
	int propertyIndex = meta->indexOfProperty(name);
	
	// Found a property, now return it
	if (propertyIndex != -1) {
		// Get the property information
		QMetaProperty property = meta->property(propertyIndex);
		// Check if the property is writeable
		if (property.isWritable()) {
			QVariant variant;
		
			int type = QMetaType::type(property.typeName());
			if (type == QMetaType::Void) {
				type = QVariant::nameToType(property.typeName());
			}
			switch (type) {
				case QVariant::String:
					variant.setValue<QString>(fromPythonToString(value));
					break;
				case QVariant::Int:
				case QMetaType::Short:
				case QMetaType::Char:
					variant.setValue<int>(fromPythonToInt(value));
					break;
				case QVariant::UInt:
				case QMetaType::UShort:
				case QMetaType::UChar:
					variant.setValue<uint>(fromPythonToUInt(value));
					break;
				case QVariant::Bool:
					variant.setValue<bool>(PyObject_IsTrue(value) != 0);
					break;				
				default:
					PyErr_Format(PyExc_AttributeError, "Unsupported property type: %d (%s)", (int)property.type(), name);
					return -1;
			}

			// Read the property from our wrapped object
			if (property.write(wrapped, variant)) {
				return 0; // Success
			}
		}
		PyErr_SetString(PyExc_AttributeError, name);
		return -1;
	}

	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
}

/*
	Constructor for our generic wrapper helper.
*/
cGenericWrapperHelper::cGenericWrapperHelper(PyObject *pyobj, QObject *qobj) {
	parent = pyobj;
	connect(qobj, SIGNAL(destroyed(QObject*)), SLOT(parentDeleted(QObject*)));
}

/*
	Slot receiving the deleted signal of the wrapped object.
*/
void cGenericWrapperHelper::parentDeleted(QObject *obj) {
	((pyWrapperObject*)parent)->wrapped = 0;
}
