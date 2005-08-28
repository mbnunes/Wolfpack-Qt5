
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
	bool noclose;

	struct stLayoutContext {
        unsigned int page;
		unsigned int group;
	};

	void addControl(unsigned int page, cControl *control, bool back = false);
	void processCommand(stLayoutContext &context, QString token, QStringList strings);

	QMap<cControl*, uint> controlIds;
	QMap<uint, cContainer*> pages;

	static QMap<unsigned int, cGenericGump*> instances; // Instances of this class
public:
	cGenericGump(int x, int y, unsigned int serial, unsigned int gumpType);
	~cGenericGump();
	void parseLayout(QString layout, QStringList strings);
	cControl *getControl(int x, int y);
	void sendResponse(uint button);
	void onClick(QMouseEvent *e);

	static cGenericGump* findByType(uint type);
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

inline void cGenericGump::setSerial(uint data) {
	serial_ = data;
}

#endif
