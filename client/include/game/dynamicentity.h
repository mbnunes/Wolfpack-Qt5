
#if !defined(__DYNAMICENTITY_H__)
#define __DYNAMICENTITY_H__

#include "game/entity.h"

class cDynamicEntity : public cEntity {
protected:
	unsigned int serial_;

public:
	cDynamicEntity(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial);
	virtual ~cDynamicEntity();

	unsigned int serial() const;
	
	// Move the entity to the given coordinates
	void move(unsigned short x, unsigned short y, signed char z);
};

inline unsigned int cDynamicEntity::serial() const {
    return serial_;
}

#endif
