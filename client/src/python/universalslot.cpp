
#include "python/universalslot.h"
#include "python/utilities.h"
#include "scripts.h"

#include <QMetaObject>

static const uint qt_meta_data_cUniversalSlot[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cUniversalSlot[] = {
    "cUniversalSlot\0\0universalslot()\0"
};

const QMetaObject cUniversalSlot::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cUniversalSlot,
      qt_meta_data_cUniversalSlot, 0 }
};

const QMetaObject *cUniversalSlot::metaObject() const
{
    return &staticMetaObject;
}

void *cUniversalSlot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cUniversalSlot))
        return static_cast<void*>(const_cast<cUniversalSlot*>(this));
    return QObject::qt_metacast(_clname);
}

int cUniversalSlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: universalslot(_a); break;
        }
        _id -= 1;
    }
    return _id;
}

void cUniversalSlot::universalslot(void **arguments) {
	if (!recipient) {
		return;
	}

	QList<QByteArray> paramTypes = slotSignature.parameterTypes();

	// Generate the argument list (the worst part of it all...)
	PyObject *args = PyTuple_New(paramTypes.count());
	uint index = 0;
	foreach (QByteArray paramType, paramTypes) {
		PyObject *object = 0;

		QMetaType::Type type = (QMetaType::Type)QMetaType::type(paramType);
		object = toPython(arguments[index+1], type);

		PyTuple_SET_ITEM(args, index++, object);
	}

	// Call the object, decref arguments afterwards and dump the return value
	PyObject *result = PyEval_CallObject(recipient, args);
	Py_DECREF(args);

	if (!result) {
		if (PyErr_Occurred()) {
			PyErr_Print();
		}
	} else {
		Py_DECREF(result);
	}
}

cUniversalSlot::cUniversalSlot(QObject *sender, const char *signal, PyObject *recipient) : QObject(sender) {	
	// Look for the method definition in sender and store it
	const QMetaObject *meta = sender->metaObject();
	QByteArray normalizedSignal = meta->normalizedSignature(signal);

	int offset = meta->indexOfSignal(signal);
	// Signal not found
	if (offset == -1) {
		valid = false;
		this->recipient = 0;
		Scripts->addSlot(this);
		return;
	}

	slotSignature = meta->method(offset);
	this->recipient = recipient;
	Py_XINCREF(this->recipient);

	// Establish the connection
	QObject::connect(sender, QString("2%1").arg(signal).toLocal8Bit(), SLOT(universalslot()));
	Scripts->addSlot(this);
}

cUniversalSlot::~cUniversalSlot() {
	Scripts->removeSlot(this);
	Py_XDECREF(recipient);
}
