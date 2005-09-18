
#if !defined(__GENERICWRAPPER_H__)
#define __GENERICWRAPPER_H__

#include <Python.h>
#include <QObject>

extern PyTypeObject pyWrapperType; 

/*
	This class receives calls from the wrapped QObjects if they're deleted,
	so it can reset the pointer on it's parent.
*/
class cGenericWrapperHelper : public QObject {
Q_OBJECT
protected:
	PyObject *parent;
public:
    cGenericWrapperHelper(PyObject*, QObject*);
public slots:
	void parentDeleted(QObject *obj);
};

// This function generates a wrapper object for the given QObject instance
PyObject *generateWrapper(QObject *object);
bool isGenericWrapper(PyObject *obj);
QObject *getWrappedObject(PyObject *obj);

#endif
