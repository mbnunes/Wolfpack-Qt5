
#if !defined(__CONTAINER_H__)
#define __CONTAINER_H__

#include <qvaluelist.h>

#include "control.h"

class cContainer : public cControl, public IPaintable {
public:
	typedef QValueList<cControl*> Controls;
	typedef Controls::iterator Iterator;
protected:
	Controls controls;
	SDL_Surface *surface;
	unsigned char alpha_; // The alpha value of this container
public:
	inline Controls getControls() {
		return controls;
	}

	cContainer();
	virtual ~cContainer();

	// Delete the content of this container
	void clear();

	bool isContainer() const; // Indicates whether this is a container or not

	// Get the surface associated with this container
	inline SDL_Surface *getSurface() {
		return surface;
	}

	// Draw this control using OpenGL primitives
	void draw(int xoffset, int yoffset);

	// This method is called by the mainloop to ensure
	// every control is updated correctly (->valid)
	virtual void update();

	// Calculate and return the highest tab index in this control
	unsigned int getHighestTabIndex();
	cControl *getNextFocusControl(cControl *current);
	cControl *getPreviousFocusControl(cControl *current);

	// Get the control contained in this container at the given coordinates
	// the coordinates are relative to this upper-left corner
	virtual cControl *getControl(int x, int y);

	inline unsigned char alpha() const { return alpha_; }
	virtual void setAlpha(unsigned char data);

	void draw(IPaintable *target, const SDL_Rect *clipping = 0);

	// Add Control
	virtual void addControl(cControl *control, bool back = false);
	virtual void removeControl(cControl *control);

	// IPaintable implementation
	virtual void drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	virtual void drawSurface(int x, int y, SDL_Surface *surface, SDL_Rect *srcrect = 0);
	virtual void getPixel(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b);

	// Notify events
	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);

	virtual void alignControl(cControl *control); // Align a control in this container

	void invertPixel(int x, int y);
private:
	void doAlignment(enControlAlign align, cControl *control, SDL_Rect &clientRect);
	void doPositioning(enControlAlign align, cControl *control, SDL_Rect &clientRect);
	bool canAlignBefore(enControlAlign align, cControl *control1, cControl *control2);
};

#endif
