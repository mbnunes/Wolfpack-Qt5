
#if !defined(__SERVER_H__)
#define __SERVER_H__

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include "singleton.h"

enum enServerState {
	STARTUP = 0, RUNNING, SCRIPTRELOAD, SHUTDOWN
};

enum enActionType {
	RELOAD_SCRIPTS = 0,
	RELOAD_PYTHON,
	RELOAD_ACCOUNTS,
	RELOAD_CONFIGURATION,
	SAVE_WORLD,
	SAVE_ACCOUNTS
};

/*!
	\brief A server component.
*/
class cComponent {
private:
	QString depends;
	QString name;
	bool silent;
	bool reloadable;
	bool loaded;

public:
	cComponent();
	virtual ~cComponent();

	virtual void load();
	virtual void unload();
	virtual void reload();

	inline const QString &getName() const {
		return name;
	}

	inline void setName(const QString &name) {
		this->name = name;
	}

	inline const QString &getDepends() const {
		return depends;
	}

	inline void setDepends(const QString &depends) {
		this->depends = depends;
	}

	inline bool isReloadable() const {
		return reloadable;
	}

	inline void setRelodable(bool reloadable) {
		this->reloadable = reloadable;
	}

	inline bool isSilent() const {
		return silent;
	}

	inline void setSilent(bool silent) {
		this->silent = silent;
	}

	inline bool isLoaded() const {
		return loaded;
	}
};

class cServer {
private:
	class Private;
	Private *d;

	// Disallow copy constructor
	cServer(const cServer&) {}

	// Load a specific component
	void load(const QString &name);

	// Unload a specific component
	void unload(const QString &name);	

	// Load all components
	void load();

	// Unload all components
	void unload();

	// Setup the console window
	void setupConsole();
	void pollQueuedActions();
public:
	cServer();
	~cServer();

	void queueAction(enActionType type);
	void setState(enServerState state);
	enServerState getState();

	void setSecure(bool secure);
	bool getSecure();

	bool isRunning();
	void cancel();

	// Returns false if an error occured
	bool run(int argc, char **argv);

	// Reload a specific component
	void reload(const QString &name);

	// Component registry
	cComponent *findComponent(const QString &name);
	void registerComponent(cComponent *component, const QString &name, bool reloadable = true, bool silent = false, const QString &depends = QString::null);
};

typedef SingletonHolder<cServer> Server;

#endif
