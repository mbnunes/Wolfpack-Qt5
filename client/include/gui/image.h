
#if !defined(__IMAGE_H__)
#define __IMAGE_H__

#include "utilities.h"
#include "gui/control.h"
#include "SDL.h"

class cImage : public cControl {
protected:
	SharedSurface *surface;
public:
	cImage();
	virtual ~cImage();

	virtual void update() = 0;

	// These methods don't need to be overridden in general
	void draw(IPaintable *target, const SDL_Rect *clipping = 0);
	cControl *getControl(int x, int y);
};

class cCustomImage : public cImage {
protected:
	QString message;
public:
	cCustomImage(SharedSurface *surface);
	void update();
	void setMessage(const QString &data) {
		message = data;
	}

	void onMouseDown(int, int, unsigned char, bool);
};

#endif
