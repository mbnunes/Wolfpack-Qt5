
#if !defined(__ENTITY_H__)
#define __ENTITY_H__

#include "SDL.h"
#include "enums.h"

/*
	This is the base class for every world object in the game.
	Every object does have a position, so the entity provides the
	members for that. The entity class is using a simple reference
	counting scheme. The user is responsible for avoiding circular
	reference.
*/
class cEntity {
private:
	unsigned int refcount;

protected:
	unsigned short x_, y_; // x,y position
	signed char z_; // z position
	enFacet facet_; // facet
	int drawx_, drawy_; // last draw position
	int width_, height_; // last draw height/width
	enEntityType type_; // type of this entity
	int priority_; // sort priority

public:
	cEntity(unsigned short x, unsigned short y, signed char z, enFacet facet);
	virtual ~cEntity();

	unsigned short x() const;
	unsigned short y() const;
	signed char z() const;
	enFacet facet() const;
	enEntityType type() const;

	void incref();
	void decref();

	int drawx() const;
	int drawy() const;
	unsigned int width() const;
	unsigned int height() const;

	// Return the sort priority for this object
	int priority() const;

	// Returns true if the object should be drawn.
	virtual bool isInWorld() const;

	// Do a hit test on the entity (relative to the draw position)
	// A bounding box hit test has alredy been done here.
	virtual bool hitTest(int x, int y);

	// Event handler for drawing this entity. Only called if isInWorld is true.
	// cellx is the x coordinate of the cells center
	// celly is the y coordinate of the cells center
	virtual void draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip);

	// This entity has been clicked on
	virtual void onClick();
	virtual void onRightClick();

	// This method should be supplied by the subclass to update the tile priority
	virtual void updatePriority();
};

inline int cEntity::priority() const {
	return priority_;
}

inline void cEntity::incref() {
	++refcount;
}

inline void cEntity::decref() {
	if (--refcount == 0) {
		delete this;
	}
}

inline enEntityType cEntity::type() const {
	return type_;
}

inline unsigned short cEntity::x() const {
	return x_;
}

inline unsigned short cEntity::y() const {
	return y_;
}

inline signed char cEntity::z() const {
	return z_;
}

inline enFacet cEntity::facet() const {
	return facet_;
}

inline int cEntity::drawx() const {
	return drawx_;
}

inline int cEntity::drawy() const {
	return drawy_;
}

inline unsigned int cEntity::width() const {
	return width_;
}

inline unsigned int cEntity::height() const {
	return height_;
}

#endif
