
#if !defined(__GENERICGUMP_H__)
#define __GENERICGUMP_H__

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include "gui/window.h"

class cGenericGump : public cWindow {
Q_OBJECT
protected:
	uint serial_;
	uint gumpType_;

	struct stLayoutContext {
        unsigned int page;
		unsigned int group;
	};

	void addControl(unsigned int page, cControl *control, bool back = false);
	void processCommand(stLayoutContext &context, QString token, QStringList strings);

	QMap<cControl*, uint> controlIds;
	QMap<uint, cContainer*> pages;
public:
	cGenericGump(int x, int y, unsigned int serial, unsigned int gumpType);
	void parseLayout(QString layout, QStringList strings);
	cControl *getControl(int x, int y);

	uint gumpType() const;
	uint serial() const;
	void setGumpType(uint data);
	void setSerial(uint data);
public slots:
	void onButtonPress(cControl *sender);
};

inline uint cGenericGump::gumpType() const {
	return gumpType_;
}

inline uint cGenericGump::serial() const {
	return serial_;
}

inline void cGenericGump::setGumpType(uint data) {
	gumpType_ = data;
}

inline void cGenericGump::setSerial(uint data) {
	serial_ = data;
}

#endif
