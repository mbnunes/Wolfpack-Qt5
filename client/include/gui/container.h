
#if !defined(__CONTAINER_H__)
#define __CONTAINER_H__

#include <QRect>
#include <QList>
#include <QVector>

#include "control.h"

class cContainer : public cControl {
Q_OBJECT
public:
	typedef QVector<cControl*> Controls;
	typedef Controls::iterator Iterator;
protected:
	Controls controls;
	Controls tabControls;
	bool disableScissorBox_;	
public:
	inline Controls getControls() {
		return controls;
	}

	void sortTabControls();

	cContainer();
	virtual ~cContainer();

	// Delete the content of this container
	void clear();

	bool disableScissorBox() const;
	void setDisableScissorBox(bool data);

	bool isContainer() const; // Indicates whether this is a container or not

	// Draw this control using OpenGL primitives
	void draw(int xoffset, int yoffset);

	// Calculate and return the highest tab index in this control
	unsigned int getHighestTabIndex();
	cControl *getNextFocusControl(cControl *current);
	cControl *getPreviousFocusControl(cControl *current);

	// Get the control contained in this container at the given coordinates
	// the coordinates are relative to this upper-left corner
	virtual cControl *getControl(int x, int y);

	// Add Control
	virtual void addControl(cControl *control, bool back = false);
	virtual void removeControl(cControl *control);

	// Notify events
	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);

	virtual void alignControl(cControl *control); // Align a control in this container

	// Fill all non-container controls into the given vector
	void getContainment(Controls &result);
private:
	void doAlignment(enControlAlign align, cControl *control, QRect &clientRect);
	void doPositioning(enControlAlign align, cControl *control, QRect &clientRect);
	bool canAlignBefore(enControlAlign align, cControl *control1, cControl *control2);
};

inline bool cContainer::disableScissorBox() const {
	return disableScissorBox_;
}

inline void cContainer::setDisableScissorBox(bool data) {
	disableScissorBox_ = data;
}

#endif
