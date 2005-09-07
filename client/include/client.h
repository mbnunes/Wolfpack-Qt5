
#if !defined(__UOCLIENT_H__)
#define __UOCLIENT_H__

#include <QString>

class cUoClient
{
private:
	void load();
	void unload();
public:
	cUoClient();
	~cUoClient();

	void lock();
	void unlock();

	void run();
	void quit();

	void errorMessage(const QString &message, const QString &title = "Error");
};

extern cUoClient *Client;

#endif
