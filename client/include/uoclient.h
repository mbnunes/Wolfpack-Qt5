
#if !defined(__UOCLIENT_H__)
#define __UOCLIENT_H__

#include <qapplication.h>
#include <qstringlist.h>
#include <qmutex.h>

class cUoClient {
private:
	void load();
	void unload();
	bool running_; // Indicates whether the client is still running
	QMutex mutex; // Global client mutex (very dirty)
public:
	cUoClient();
	~cUoClient();

	void lock();
	void unlock();

	void run();

	bool running() { return running_; }
	void quit() { running_ = false; }

	void errorMessage(const QString &message, const QString &title = "Error");
};

inline void cUoClient::lock() {
	mutex.lock();
}

inline void cUoClient::unlock() {
	mutex.unlock();
}

extern cUoClient *Client;

#endif
