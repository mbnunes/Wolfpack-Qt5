/****************************************************************************
** Meta object code from reading C++ file 'lineedithistory.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/gui/lineedithistory.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'lineedithistory.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_LineEditHistory_t {
    QByteArrayData data[10];
    char stringdata0[114];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LineEditHistory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LineEditHistory_t qt_meta_stringdata_LineEditHistory = {
    {
QT_MOC_LITERAL(0, 0, 15), // "LineEditHistory"
QT_MOC_LITERAL(1, 16, 10), // "searchNext"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 10), // "searchPrev"
QT_MOC_LITERAL(4, 39, 10), // "invalidate"
QT_MOC_LITERAL(5, 50, 21), // "requestVisibleSymbols"
QT_MOC_LITERAL(6, 72, 8), // "complete"
QT_MOC_LITERAL(7, 81, 10), // "itemChosen"
QT_MOC_LITERAL(8, 92, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(9, 109, 4) // "item"

    },
    "LineEditHistory\0searchNext\0\0searchPrev\0"
    "invalidate\0requestVisibleSymbols\0"
    "complete\0itemChosen\0QListWidgetItem*\0"
    "item"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LineEditHistory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,
       4,    0,   46,    2, 0x06 /* Public */,
       5,    0,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   48,    2, 0x08 /* Private */,
       7,    1,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,

       0        // eod
};

void LineEditHistory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LineEditHistory *_t = static_cast<LineEditHistory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->searchNext(); break;
        case 1: _t->searchPrev(); break;
        case 2: _t->invalidate(); break;
        case 3: _t->requestVisibleSymbols(); break;
        case 4: _t->complete(); break;
        case 5: _t->itemChosen((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (LineEditHistory::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LineEditHistory::searchNext)) {
                *result = 0;
            }
        }
        {
            typedef void (LineEditHistory::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LineEditHistory::searchPrev)) {
                *result = 1;
            }
        }
        {
            typedef void (LineEditHistory::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LineEditHistory::invalidate)) {
                *result = 2;
            }
        }
        {
            typedef void (LineEditHistory::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LineEditHistory::requestVisibleSymbols)) {
                *result = 3;
            }
        }
    }
}

const QMetaObject LineEditHistory::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_LineEditHistory.data,
      qt_meta_data_LineEditHistory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *LineEditHistory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LineEditHistory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_LineEditHistory.stringdata0))
        return static_cast<void*>(const_cast< LineEditHistory*>(this));
    return QLineEdit::qt_metacast(_clname);
}

int LineEditHistory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void LineEditHistory::searchNext()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void LineEditHistory::searchPrev()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void LineEditHistory::invalidate()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void LineEditHistory::requestVisibleSymbols()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
