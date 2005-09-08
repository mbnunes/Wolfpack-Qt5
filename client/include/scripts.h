
#if !defined(__SCRIPTS_H__)
#define __SCRIPTS_H__

#include <QObject>
#include "qsproject.h"

class cScripts : public QObject {
Q_OBJECT
protected:
	QSProject *project;
public:
	cScripts();
	~cScripts();

	void load();
	void unload();

public slots:
	void showWorkbench();
};

extern cScripts *Scripts;

#endif
