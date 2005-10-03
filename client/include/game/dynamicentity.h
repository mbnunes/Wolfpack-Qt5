
#if !defined(__DYNAMICENTITY_H__)
#define __DYNAMICENTITY_H__

#include <qvector.h>
#include <qdatetime.h>
#include "game/entity.h"

class cControl;

class cDynamicEntity : virtual public cEntity {
Q_OBJECT
Q_PROPERTY(uint serial READ serial);
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
	cDynamicEntity(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial);
	cDynamicEntity(unsigned int serial);
	virtual ~cDynamicEntity();
public slots:
	unsigned int serial() const;
	
	// Move the entity to the given coordinates
	virtual void move(unsigned short x, unsigned short y, signed char z);

	void setFacet(enFacet facet);
};

inline unsigned int cDynamicEntity::serial() const {
    return serial_;
}

Q_DECLARE_METATYPE(cDynamicEntity*);

#endif
