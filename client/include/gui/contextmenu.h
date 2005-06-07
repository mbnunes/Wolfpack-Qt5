
#if !defined(__CONTEXTMENU_H__)
#define __CONTEXTMENU_H__

#include "gui/bordergump.h"
#include "gui/label.h"
#include "gui/window.h"

#include <qvaluevector.h>

#if defined(__GNUC__)
#define __stdcall __attribute__((__stdcall__))
#endif
#define STDCALL __stdcall

class cContextMenuEntry;
class cContextMenu;

typedef void (STDCALL *fnContextMenuCallback)(cContextMenu *menu, int selected, void *customData);

class cContextMenu : public cWindow {
friend class cContextMenuEntry;

protected:
	typedef QValueVector<cContextMenuEntry*> Container;
	typedef Container::iterator Iterator;

	// Define some way of storing the context menu entries
	cBorderGump *border;	
	cTexture *checkerboard;
	Container entries;

	void *customData_;
	fnContextMenuCallback callback;
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

	void setCallback(fnContextMenuCallback callback, void *customData);
	void *customData() const;
};

inline void *cContextMenu::customData() const {
	return customData_;
}

extern cContextMenu *ContextMenu; // There can only be one context menu

#endif
