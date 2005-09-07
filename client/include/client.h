
#if !defined(__UOCLIENT_H__)
#define __UOCLIENT_H__

#include <qapplication.h>
#include <qstringlist.h>
#include <qmutex.h>
#include <QObject>

class cUoClient : public QObject {
Q_OBJECT
private:
	void load();
	void unload();
public:
	cUoClient();
	~cUoClient();

	void lock();
	void unlock();

	void run();

	void errorMessage(const QString &message, const QString &title = "Error");

public slots:
	void quit();
};

extern cUoClient *Client;

#endif
