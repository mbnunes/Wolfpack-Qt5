
#if !defined(__GUI_H__)
#define __GUI_H__


#include "control.h"
#include "container.h"
#include "window.h"

#include <QDateTime>
#include <QDomElement>

class cEntity;

class cGui : public cContainer {
Q_OBJECT

private:
	cControl *inputFocus_; // Pointer to the control with the input focus
	cWindow *activeWindow_; // In principle this is the currently active window

	// Private data class for overhead text information
	class cOverheadInfo {
	public:
		QTime timeout;
		cControl *control; // The control representing the text
        int centerx, centery; // If no entity is specified these are fixed coordinates
		cEntity *entity; // If only one text per entity should be allowed, set this to the pointer of the entity.
		bool itemName; // There will only be one item-name on screen at a time for an item. This is the flag for this.
	};

	QVector<cControl*> deleteQueue;
	QVector<cOverheadInfo> overheadText;
	bool cleaningUpOverheadText;

	QMap<QString, QDomElement> dialogTemplates; // Map for Dialog Templates
public:
	cGui();
	virtual ~cGui();

	void queueDelete(cControl *ctrl);
	void addItemNameText(int centerx, int centery, unsigned int timeout, QString message, unsigned short hue = 0x3b2, unsigned char font = 3, cEntity *source = 0, bool ascii = false);
	void addOverheadText(int centerx, int centery, unsigned int timeout, QString message, unsigned short hue = 0x3b2, unsigned char font = 3, cEntity *source = 0, bool ascii = false);
	void removeOverheadText(cEntity *source);
	
	inline cWindow *activeWindow() { return activeWindow_; }
	void setActiveWindow(cWindow *data);

	inline cControl *inputFocus() { return inputFocus_; }
	void setInputFocus(cControl *focus);

	// This event should be issued by the cControl destructor to 
	// clear pointers
	void onDeleteControl(cControl *control);

	void addControl(cControl *control, bool back = false);

	// Draw the GUI
	void draw();
	cTexture *checkerboard() const;

	void closeAllGumps();

	void load();
	void unload();

	// Check if the given template name exists
	bool isTemplateAvailable(QString name) const;
	cWindow *createDialog(QString templateName);
	cControl *createControl(QDomElement templateNode);
};

inline bool cGui::isTemplateAvailable(QString name) const {
	return dialogTemplates.find(name) != dialogTemplates.end();
}

extern cGui *Gui;

#endif
