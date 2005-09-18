
#if !defined(__SCRIPTS_H__)
#define __SCRIPTS_H__

#include <QObject>
#include <QVariantList>
#include <QVector>

class cUniversalSlot;

class cScripts : public QObject {
Q_OBJECT
protected:
	void initializeSearchPath();
	QVector<cUniversalSlot*> slotlist;
public:
	cScripts();
	~cScripts();

	void load();
	void unload();

	void addSlot(cUniversalSlot *slot);
	void removeSlot(cUniversalSlot *slot);

    QVariant callFunction(const QString &module, const QString &function, const QVariantList &arguments);
};

extern cScripts *Scripts;

#endif
