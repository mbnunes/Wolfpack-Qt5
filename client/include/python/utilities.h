
#if !defined(__PYTHON_UTILITIES_H__)
#define __PYTHON_UTILITIES_H__

#include <QString>
#include <QByteArray>
#include <QVariant>

#include "python/genericwrapper.h"

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
	if (to == "QStringList") {
		return PyTuple_Check(from) || PyList_Check(from);
	} else if (to == "PyObject*") {
		return true;
	}

	QMetaType::Type type = (QMetaType::Type)QMetaType::type(to.toLocal8Bit());

	if (type == QMetaType::QObjectStar || type >= QMetaType::User) {
			// Check if it's a pointer type and see if from is a genericwrapper capable of
			// casting to the required type
			if (isGenericWrapper(from)) {
				QObject *wrapped = getWrappedObject(from);
				if (wrapped->inherits(to.left(to.length() - 1).toLocal8Bit())) {
					return true; // We can convert it. yay!
				}
			}
	}

	switch (type) {
		case QMetaType::QString:
			return PyString_Check(from) || PyUnicode_Check(from);
		case QMetaType::UInt:
		case QMetaType::Int:
		case QMetaType::UShort:
		case QMetaType::Short:
		case QMetaType::Char:
		case QMetaType::UChar:
		case QMetaType::ULong:
		case QMetaType::Long:
		case QMetaType::Float:
		case QMetaType::Double:
			return PyInt_Check(from) || PyLong_Check(from) || PyFloat_Check(from);
			break;
		case QMetaType::Bool:
			return true;
		default:
			return false;
	};
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
		case QMetaType::Short:
			return toPython(variant.value<short>());
		case QMetaType::Char:
			return toPython(variant.value<char>());
		case QVariant::UInt:
			return toPython(variant.value<uint>());
		case QMetaType::UShort:
			return toPython(variant.value<ushort>());
		case QMetaType::UChar:
			return toPython(variant.value<uchar>());
		case QVariant::Bool:
			return toPython(variant.toBool());
		case QVariant::ULongLong:
			return toPython(variant.toULongLong());
		case QMetaType::QObjectStar:
			return toPython(qvariant_cast<QObject*>(variant));
		default:
			return 0;
	}
}

inline PyObject *toPython(void *ptr, QMetaType::Type type) {
	if (ptr == 0) {
		Py_RETURN_NONE;
	}

	if (type >= QMetaType::User) {
		type = QMetaType::QObjectStar;
	}

	switch (type) {
		// All Integer types go into the same function anyway
		case QMetaType::Int:
			return toPython(*(int*)ptr);
		case QMetaType::UInt:
			return toPython(*(uint*)ptr);
		case QMetaType::UShort:
			return toPython(*(ushort*)ptr);
		case QMetaType::Short:
			return toPython(*(short*)ptr);
		case QMetaType::Char:
			return toPython(*(char*)ptr);
		case QMetaType::UChar:
			return toPython(*(uchar*)ptr);
		case QMetaType::QString:
			return toPython(*(QString*)ptr);
		case QMetaType::Bool:
			return toPython(*(bool*)ptr);
		case QMetaType::QObjectStar:
			if (*(QObject**)ptr == 0) {
				Py_RETURN_NONE;
			} else {
				return toPython(*(QObject**)ptr);
			}

		// Default return type is None
		default:
		case QMetaType::Void:
			Py_RETURN_NONE;
	}
}

#endif
