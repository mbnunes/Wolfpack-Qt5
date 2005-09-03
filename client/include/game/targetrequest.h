
#if !defined(__TARGETREQUEST_H__)
#define __TARGETREQUEST_H__

#include "game/entity.h"

class cTargetRequest : public QObject {
Q_OBJECT
public:
	virtual bool isValidTarget(cEntity *selected) = 0;
	virtual void target(cEntity *selected) = 0;
	virtual void cancel() = 0;
};

class cGenericTargetRequest : public cTargetRequest {
Q_OBJECT
public:
	bool isValidTarget(cEntity *selected);
	void target(cEntity *selected);
	void cancel();
signals:
	void cancelled();
	void targetted(cEntity*);
};

// Implementation of the cTargetRequest class for 
// server sent target requests.
class cServerTargetRequest : public cTargetRequest {
Q_OBJECT
protected:
	unsigned int serial;
	unsigned char type;
	unsigned char cursor;
public:
	cServerTargetRequest(unsigned int serial, unsigned char type, unsigned char cursor);

	bool isValidTarget(cEntity *selected);
	void target(cEntity *selected);
	void cancel();
};

#endif
