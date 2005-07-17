
#if !defined(__GUI_H__)
#define __GUI_H__

#include <q3valuevector.h>

#include "control.h"
#include "container.h"
#include "window.h"

class cGui : public cContainer {
Q_OBJECT

private:
	cControl *inputFocus_; // Pointer to the control with the input focus
	cWindow *activeWindow_; // In principle this is the currently active window
public:
	cGui();
	virtual ~cGui();
	
	inline cWindow *activeWindow() { return activeWindow_; }
	inline void setActiveWindow(cWindow *data) { activeWindow_ = data; }

	inline cControl *inputFocus() { return inputFocus_; }
	void setInputFocus(cControl *focus);

	// This event should be issued by the cControl destructor to 
	// clear pointers
	void onDeleteControl(cControl *control);

	void invalidate();

	void addControl(cControl *control, bool back = false);

	// Draw the GUI
	void draw();
};

extern cGui *Gui;

#endif
