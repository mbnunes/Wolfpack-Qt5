
#if !defined(__ACTION_H__)
#define __ACTION_H__

class cAction {
public:
	cAction();
	virtual ~cAction();

	// Execute the action
	virtual void execute() = 0;
};

class cActionReloadScripts : public cAction {
public:
	void execute();
};

class cActionReloadPython : public cAction {
public:
	void execute();
};

class cActionReloadAccounts : public cAction {
public:
	void execute();
};

class cActionReloadConfiguration : public cAction {
public:
	void execute();
};

class cActionSaveWorld : public cAction {
public:
	void execute();
};

class cActionSaveAccounts : public cAction {
public:
	void execute();
};

#endif
