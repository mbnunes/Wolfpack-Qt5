
#if !defined(__PYTHON_UTILITIES_H__)
#define __PYTHON_UTILITIES_H__

#include <QString>
#include <QByteArray>
#include <QVariant>

#include "python/genericwrapper.h"

// Several Python Utility Functions

/*
	Convert a python object into a QString.
*/
inline QString fromPythonToString(PyObject* object) {
	if (!object) {
		return QString::null;
	} else if ( PyUnicode_Check( object ) ) {
#if defined(Py_UNICODE_WIDE)
		PyObject *utf8 = PyUnicode_AsUTF8String( object );
		return QString::fromUtf8( PyString_AsString( utf8 ) );
		Py_DECREF(utf8);
#else
		return QString::fromUtf16( ( ushort * ) PyUnicode_AS_UNICODE( object ) );
#endif
	} else if ( PyString_Check( object ) ) {
		return QString::fromLocal8Bit( PyString_AsString( object ) );
	} else if ( PyInt_Check( object ) ) {
		return QString::number( PyInt_AsLong( object ) );
	} else if ( PyFloat_Check( object ) ) {
		return QString::number( PyFloat_AsDouble( object ) );
	} else {
		return QString::null;
	}
}

/*
	Convert a python object into a int.
*/
inline int fromPythonToInt(PyObject* object) {
	if (!object) {
		return 0;
	} else if ( PyInt_Check( object ) ) {
		return PyInt_AS_LONG(object);
	} else if ( PyLong_Check( object ) ) {
		return PyLong_AsLong(object);
	} else if ( PyFloat_Check( object ) ) {
		return (int)PyFloat_AsDouble( object );
	} else {
		return 0;
	}
}

/*
	Convert a python object into a uint.
*/
inline int fromPythonToUInt(PyObject* object) {
	if (!object) {
		return 0;
	} else if ( PyInt_Check( object ) ) {
		return PyInt_AS_LONG(object);
	} else if ( PyLong_Check( object ) ) {
		return PyLong_AsUnsignedLong(object);
	} else if ( PyFloat_Check( object ) ) {
		return (int)PyFloat_AsDouble( object );
	} else {
		return 0;
	}
}

/*
Check if we can convert the given PyObject to the target type.
*/
inline bool canConvert(PyObject *from, const QString &to) {
	if (to == "QString") {
		return PyString_Check(from) || PyUnicode_Check(from);
	} else if (to == "int" || to == "uint" || to == "unsigned short" || to == "short" || to == "unsigned char" || to == "char") {
		return PyInt_Check(from) || PyLong_Check(from) || PyFloat_Check(from);
	} else if (to == "bool") {
		return true; // Always convertable
	} else {
		// Check if it's a pointer type and see if from is a genericwrapper capable of
		// casting to the required type
		if (isGenericWrapper(from)) {
			QObject *wrapped = getWrappedObject(from);
			if (wrapped->inherits(to.left(to.length() - 1).toLocal8Bit())) {
				return true; // We can convert it. yay!
			}
		}		

		return false;
	}
}

/*
Convert a QString into a python object.
*/
inline PyObject *toPython(const QString &str) {
	if (str.isEmpty()) {
		return PyUnicode_FromWideChar((ushort*)"\x00\x00", 0);
	} else {
#if defined(Py_UNICODE_WIDE)
		QByteArray utf = str.toUtf8();
		return PyUnicode_DecodeUTF8(utf.data(), utf.length(), "");
#else
		return PyUnicode_FromUnicode((Py_UNICODE*) str.utf16(), str.length());
#endif
	}
}

/*
	Integer conversion routines
*/
inline PyObject *toPython(int data) {
	return PyInt_FromLong(data);
}

inline PyObject *toPython(uint data) {
	return PyInt_FromLong(data);
}

inline PyObject *toPython(qulonglong data) {
	return PyLong_FromUnsignedLongLong(data);
}

inline PyObject *toPython(qlonglong data) {
	return PyLong_FromLongLong(data);
}

inline PyObject *toPython(bool data) {
	if (data) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

inline PyObject *toPython(QObject *obj) {
	return generateWrapper(obj);
}

/*
Convert a Variant to a python object
*/
inline PyObject *toPython(const QVariant &variant) {
	switch (variant.type()) {
		case QVariant::String:
			return toPython(variant.toString());
		case QVariant::Int:
			return toPython(variant.toInt());
		case QVariant::UInt:
			return toPython(variant.toUInt());
		case QVariant::ULongLong:
			return toPython(variant.toULongLong());
		case QMetaType::QObjectStar:
			return toPython(qvariant_cast<QObject*>(variant));
		default:
			return 0;
	}
}

#endif
