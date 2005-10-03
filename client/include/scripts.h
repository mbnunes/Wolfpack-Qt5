
#if !defined(__SCRIPTS_H__)
#define __SCRIPTS_H__

#include <QObject>
#include <QVariantList>
#include <QVector>

class cUniversalSlot;
typedef struct _object PyObject;

class cScripts : public QObject {
Q_OBJECT
protected:
	void initializeSearchPath();
	QVector<cUniversalSlot*> slotlist;
	QObject *sender_;
public:
	enum Error {
		RuntimeError,
		AttributeError,
		TypeError
	};

	cScripts();
	~cScripts();

	void load();
	void unload();

	void addSlot(cUniversalSlot *slot);
	void removeSlot(cUniversalSlot *slot);

	QObject *sender() const;
	void setSender(QObject *sender);

	QVector<cUniversalSlot*> getSlotlist() const;
    QVariant callFunction(const QString &module, const QString &function, const QVariantList &arguments);

	void setError(Error condition, QString message);

	/*
		Valid format keys and their meaning:
		Q: QString pointer
		T: bool pointer
	*/
	static bool parseArguments(PyObject *args, const char *format, ...);
};

inline QObject *cScripts::sender() const {
	return sender_;
}

inline void cScripts::setSender(QObject *sender) {
	sender_ = sender;
}

inline QVector<cUniversalSlot*> cScripts::getSlotlist() const {
	return slotlist;
}

extern cScripts *Scripts;

#endif
