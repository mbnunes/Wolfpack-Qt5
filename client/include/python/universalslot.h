
#if !defined(__UNIVERSALSLOT_H__)
#define __UNIVERSALSLOT_H__

#include <Python.h>

#include <QObject>
#include <QMetaMethod>

class cUniversalSlot : public QObject
{
public:
	cUniversalSlot(QObject *sender, const char *signal, PyObject *recipient);
	~cUniversalSlot();

    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    QT_TR_FUNCTIONS
    virtual int qt_metacall(QMetaObject::Call, int, void **);

	// The actual code-glue between qt->python
	void universalslot(void **arguments);
protected:
	PyObject *recipient;
	QMetaMethod slotSignature;
	bool valid;
};

#endif
