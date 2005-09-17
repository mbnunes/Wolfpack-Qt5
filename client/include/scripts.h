
#if !defined(__SCRIPTS_H__)
#define __SCRIPTS_H__

#include <QObject>
#include <QVariantList>

class cScripts : public QObject {
Q_OBJECT
protected:
	void initializeSearchPath();
public:
	cScripts();
	~cScripts();

	void load();
	void unload();

    QVariant callFunction(const QString &module, const QString &function, const QVariantList &arguments);
};

extern cScripts *Scripts;

#endif
