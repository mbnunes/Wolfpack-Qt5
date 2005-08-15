
#if !defined(__DYNAMICENTITY_H__)
#define __DYNAMICENTITY_H__

#include <qvector.h>
#include <qdatetime.h>
#include "game/entity.h"

class cControl;

class cDynamicEntity : virtual public cEntity {
protected:
	unsigned int serial_;

	class cOverheadInfo {
	public:
		QTime timeout;
		cControl *control; // The control representing the text
	};

	QVector<cOverheadInfo> overheadText;

	// xbase/ybase are the coordinates of the topmost center of the entity (cellx, celly - height in reality)
	void drawOverheadText(int xbase, int ybase);
public:
	void addOverheadText(QString message, unsigned short color = 0x3b2, unsigned char font = 3);

	cDynamicEntity(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial);
	cDynamicEntity(unsigned int serial);
	virtual ~cDynamicEntity();

	unsigned int serial() const;
	
	// Move the entity to the given coordinates
	virtual void move(unsigned short x, unsigned short y, signed char z);

	void setFacet(enFacet facet);
};

inline unsigned int cDynamicEntity::serial() const {
    return serial_;
}

#endif
