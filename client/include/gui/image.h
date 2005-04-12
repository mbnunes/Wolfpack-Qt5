
#if !defined(__IMAGE_H__)
#define __IMAGE_H__

#include "utilities.h"
#include "gui/control.h"
#include "SDL.h"

class cImage : public cControl {
Q_OBJECT

protected:
	SharedSurface *surface;
public:
	cImage();
	virtual ~cImage();

	virtual void update() = 0;

	// These methods don't need to be overridden in general
	cControl *getControl(int x, int y);
};

#endif
