
#if !defined(__UOCLIENT_H__)
#define __UOCLIENT_H__

#include "SDL.h"

#include <qapplication.h>
#include <qstringlist.h>

class cUoClient {
private:
	void load();
	void unload();
	bool running_; // Indicates whether the client is still running

public:
	cUoClient();
	~cUoClient();

	void run(const QStringList &arguments);
	void processSdlEvent(const SDL_Event &event);

	bool running() { return running_; }
	void quit() { running_ = false; }
};

extern cUoClient *Client;
extern QApplication *App;

#endif
