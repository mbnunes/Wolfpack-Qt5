
#if !defined(__UOCLIENT_H__)
#define __UOCLIENT_H__

#include <QApplication>
#include <QStringList>
#include <QThread>
#include <QObject>

class cUoClient : public QThread {
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
