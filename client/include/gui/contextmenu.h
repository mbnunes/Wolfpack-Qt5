
#if !defined(__CONTEXTMENU_H__)
#define __CONTEXTMENU_H__

#include "gui/bordergump.h"
#include "gui/label.h"
#include "gui/window.h"

#include <QList>

class cContextMenuEntry;
class cContextMenu;
class cCheckerTrans;

class cContextMenu : public cWindow {
Q_OBJECT
friend class cContextMenuEntry;

protected:
	typedef QList<cContextMenuEntry*> Container;
	typedef Container::iterator Iterator;

	// Define some way of storing the context menu entries
	cBorderGump *border;	
	cCheckerTrans *checkertransOn, *checkertransOff;
	Container entries;
	uint serial_; // Serial of object we're showing the contextmenu for

public:
	cContextMenu();
	virtual ~cContextMenu();

	cControl *getControl(int x, int y);

	void clear();
	void addEntry(const QString &name, unsigned short hue = 0x3b2, int id = -1);

	void draw(int xoffset, int yoffset);

	void show();
	void show(int x, int y);
	void hide();
	
	uint serial() const;
    void setSerial(uint serial);
public slots:
	void sendResponse(ushort id);

signals:
	void clicked(ushort id);
};

inline uint cContextMenu::serial() const {
	return serial_;
}

inline void cContextMenu::setSerial(uint data) {
	serial_ = data;
}

extern cContextMenu *ContextMenu; // There can only be one context menu

#endif
