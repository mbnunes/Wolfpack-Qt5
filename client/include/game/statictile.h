
#if !defined(__STATICTILE_H__)
#define __STATICTILE_H__

#include "game/entity.h"
#include "muls/tiledata.h"
#include "utilities.h"
#include "texture.h"
#include "muls/art.h"
//Added by qt3to4:
#include <QMouseEvent>

class cStaticTile : virtual public cEntity {
Q_OBJECT
protected:
	unsigned short id_;
	unsigned short hue_;
	cItemTileInfo *tiledata_;
	cTexture *texture;
	bool drawStacked_;

	// For animated static tiles
	bool animated;
	cArtAnimation *animation; // Animation context
	unsigned short frame; // Current frame
	unsigned int nextFrame; // When this skips to the next frame

	cStaticTile();
public:
	cStaticTile(unsigned short x, unsigned short y, signed char z, enFacet facet);
	virtual ~cStaticTile();

	unsigned short id() const;
	virtual void setId(unsigned short data);
	unsigned short hue() const;
	virtual void setHue(unsigned short data);
	cItemTileInfo *tiledata() const;

	bool drawStacked() const;
	void setDrawStacked(bool data);

	// Override the draw method
	void draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip);

	// Do a sophisticated hittest on the internal surface
	bool hitTest(int x, int y);

	void onClick(QMouseEvent *e);
	
	// Update the priority of this static tile
	void updatePriority();
};

Q_DECLARE_METATYPE(cStaticTile*);

inline unsigned short cStaticTile::hue() const {
	return hue_;
}

inline unsigned short cStaticTile::id() const {
	return id_;
}

inline cItemTileInfo *cStaticTile::tiledata() const {
	return tiledata_;
}

inline bool cStaticTile::drawStacked() const {
	return drawStacked_;
}

inline void cStaticTile::setDrawStacked(bool data) {
	drawStacked_ = data;	
}

#endif
