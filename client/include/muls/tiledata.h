
#if !defined(__TILEDATA_H__)
#define __TILEDATA_H__

#include <QString>

// Utility defines
#define FLAG_GETTER(MASK, NAME) inline bool NAME() const { return (flags_ & MASK) != 0; }

/*
	Base class for both tile info classes.
	Both classes use the same flag. So this is just a better way of handling it.
*/
class cTileInfo {
friend class cTiledata;

protected:
	unsigned int refcount; // Simple reference count
	unsigned int flags_;
	QString name_;

public:
	cTileInfo();
	virtual ~cTileInfo();

	void incref(); // Increment reference count
	void decref(); // Decrement reference count (this delete the item if refcount <= 0)

	unsigned int flags() const;
    const QString &name() const;

	// These are convenient getters for certain properties
	FLAG_GETTER(0x00000001, isBackground)
	FLAG_GETTER(0x00000002, isWeapon)
	FLAG_GETTER(0x00000004, isTransparent)
	FLAG_GETTER(0x00000008, isTranslucent)
	FLAG_GETTER(0x00000010, isWall)
	FLAG_GETTER(0x00000020, isDamaging)
	FLAG_GETTER(0x00000040, isImpassable)
	FLAG_GETTER(0x00000080, isWet)
	FLAG_GETTER(0x00000100, isUnknown1)
	FLAG_GETTER(0x00000200, isSurface)
	FLAG_GETTER(0x00000400, isBridge)
	FLAG_GETTER(0x00000800, isGeneric)
	FLAG_GETTER(0x00001000, isWindow)
	FLAG_GETTER(0x00002000, isNoShoot)
	FLAG_GETTER(0x00004000, isArticleA)
	FLAG_GETTER(0x00008000, isArticleAn)
	FLAG_GETTER(0x00010000, isInternal)
	FLAG_GETTER(0x00020000, isFoliage)
	FLAG_GETTER(0x00040000, isPartialHue)
	FLAG_GETTER(0x00080000, isUnknown2)
	FLAG_GETTER(0x00100000, isMap)
	FLAG_GETTER(0x00200000, isContainer)
	FLAG_GETTER(0x00400000, isWearable)
	FLAG_GETTER(0x00800000, isLightSource)
	FLAG_GETTER(0x01000000, isAnimation)
	FLAG_GETTER(0x02000000, isNoDiagonal)
	FLAG_GETTER(0x04000000, isUnknown3)
	FLAG_GETTER(0x08000000, isArmor)
	FLAG_GETTER(0x10000000, isRoof)
	FLAG_GETTER(0x20000000, isDoor)
	FLAG_GETTER(0x40000000, isStairBack)
	FLAG_GETTER(0x80000000, isStairRight)
};

inline void cTileInfo::incref() {
	++refcount;
}

inline void cTileInfo::decref() {
	if (--refcount == 0) {
		delete this;
	}
}

inline unsigned int cTileInfo::flags() const {
	return flags_;
}

inline const QString &cTileInfo::name() const {
	return name_;
}

// This class represents tile information for land tiles
class cLandTileInfo : public cTileInfo {
friend class cTiledata;

protected:
	unsigned short texture_;
public:
	unsigned short texture() const;
};

inline unsigned short cLandTileInfo::texture() const {
	return texture_;
}

// This class represents tile information for item tiles
class cItemTileInfo : public cTileInfo {
friend class cTiledata;

protected:
	unsigned char weight_;
	unsigned char quality_;
	unsigned short unknown1_;
	unsigned char unknown2_;
	unsigned char quantity_;
	unsigned short animation_;
	unsigned char unknown3_;
	unsigned char hue_;
	unsigned char unknown4_;
	unsigned char unknown5_;
	unsigned char height_;
public:
	unsigned char weight() const;
	unsigned char quality() const;
	unsigned short unknown1() const;
	unsigned char unknown2() const;
	unsigned char quantity() const;
	unsigned short animation() const;
	unsigned char unknown3() const;
	unsigned char hue() const;
	unsigned char unknown4() const;
	unsigned char unknown5() const;
	unsigned char height() const;

	// Simple aliases
	unsigned char layer() const;
	unsigned char lightsource() const;
};

inline unsigned char cItemTileInfo::weight() const {
	return weight_;
}

inline unsigned char cItemTileInfo::quality() const {
	return quality_;
}

inline unsigned short cItemTileInfo::unknown1() const {
	return unknown1_;
}

inline unsigned char cItemTileInfo::unknown2() const {
	return unknown2_;
}

inline unsigned char cItemTileInfo::quantity() const {
	return quantity_;
}

inline unsigned short cItemTileInfo::animation() const {
	return animation_;
}

inline unsigned char cItemTileInfo::unknown3() const {
	return unknown3_;
}

inline unsigned char cItemTileInfo::hue() const {
	return hue_;
}

inline unsigned char cItemTileInfo::unknown4() const {
	return unknown4_;
}

inline unsigned char cItemTileInfo::unknown5() const {
	return unknown5_;
}

inline unsigned char cItemTileInfo::height() const {
	return height_;
}

inline unsigned char cItemTileInfo::layer() const {
	return quality_;
}

inline unsigned char cItemTileInfo::lightsource() const {
	return quality_;
}

// This is the real tiledata reader
class cTiledata {
protected:
	cItemTileInfo *items[0x4000];
	cLandTileInfo *land[0x4000];
public:
	cTiledata();

	void load();
	void unload();
	void reload();

	// This is ensured to return a pointer. If the id is out of range, it returns
	// an empty item info instead. The pointers are guaranteed to exist.
	cItemTileInfo *getItemInfo(unsigned short id) const;
	cLandTileInfo *getLandInfo(unsigned short id) const;
};

inline cItemTileInfo *cTiledata::getItemInfo(unsigned short id) const {
	return items[id & 0x3FFF];
}

inline cLandTileInfo *cTiledata::getLandInfo(unsigned short id) const {
return land[id & 0x3FFF];
}

extern cTiledata *Tiledata;

#endif
