
#if !defined(__MAPS_H__)
#define __MAPS_H__

#include <QFile>
#include <QDataStream>
#include <QString>
#include <QVector>
#include <QCache>

#include "enums.h"

// Structure used for a single map cell
struct stMapCell {
	unsigned short id;
	signed char z;
};

struct stStaticItem {
	unsigned short id;
	unsigned short color;
	unsigned char xoffset;
	unsigned char yoffset;
	signed char z;
};

typedef QVector<stStaticItem> StaticBlock;

class cFacet {
private:	
	QFile mapData, staticData, staticIndex; // Input files
	QDataStream mapDataStream, staticDataStream, staticIndexStream; // Input streams
	QCache<int, stMapCell> mapCache; // This is actually a block cache for 64 cells at once
	QCache<int, StaticBlock> staticCache; // This is actually a block cache for all static items in a block
	stMapCell emptyCell;

protected:
	enFacet id_;
	QString name_;
	unsigned int width_;
	unsigned int height_;
	bool enabled_;
	bool optional_;

public:
	cFacet();

	const QString &name() const;
	unsigned int height() const;
	unsigned int width() const;
	enFacet id() const;
	bool optional() const;
	bool enabled() const;

	void setName(const QString &data);
	void setId(enFacet data);
	void setHeight(unsigned short height);
	void setWidth(unsigned short width);
	void setEnabled(bool data);
	void setOptional(bool data);

	// Loading this facet
	void load();
	void reload();
	void unload();

	// Retrieve a map cell from this block. This is guranteed to return something valid. Even if its crap data.
	stMapCell *getMapCell(unsigned short x, unsigned short y);
	signed char getAverageHeight(ushort x, ushort y);

	// If there aren't any static items in the block this returns NULL
	StaticBlock *getStaticBlock(unsigned short x, unsigned short y);
};

inline void cFacet::setEnabled(bool data) {
	enabled_ = data;
}

inline void cFacet::setOptional(bool data) {
	optional_ = data;
}

inline void cFacet::setName(const QString &data) {
	name_ = data;
}

inline void cFacet::setId(enFacet data) {
	id_ = data;
}

inline void cFacet::setHeight(unsigned short data) {
	height_ = data;
}

inline void cFacet::setWidth(unsigned short data) {
	width_ = data;
}

inline bool cFacet::optional() const {
	return optional_;
}

inline bool cFacet::enabled() const {
	return enabled_;
}

inline enFacet cFacet::id() const {
	return id_;
}

inline const QString &cFacet::name() const {
	return name_;
}

inline unsigned int cFacet::height() const {
	return height_;
}

inline unsigned int cFacet::width() const {
	return width_;
}

class cMaps {
private:
	cFacet facets[FACETCOUNT];
public:
	void load();
	void unload();
	void reload();

	unsigned int facetWidth(enFacet facet) const;
	unsigned int facetHeight(enFacet facet) const;

	// Retrieve a map cell from this block. This is guranteed to return something valid. Even if its crap data.
	stMapCell *getMapCell(enFacet facet, unsigned short x, unsigned short y);
	signed char getAverageHeight(enFacet facet, ushort x, ushort y);

	// If there aren't any static items in the block this returns NULL
	StaticBlock *getStaticBlock(enFacet facet, unsigned short x, unsigned short y);
};

inline unsigned int cMaps::facetWidth(enFacet facet) const {
	return facets[facet].width();
}

inline unsigned int cMaps::facetHeight(enFacet facet) const {
	return facets[facet].height();
}

inline signed char cMaps::getAverageHeight(enFacet facet, ushort x, ushort y) {
	return facets[facet].getAverageHeight(x, y);
}

inline stMapCell *cMaps::getMapCell(enFacet facet, unsigned short x, unsigned short y) {
	return facets[facet].getMapCell(x, y);
}

inline StaticBlock *cMaps::getStaticBlock(enFacet facet, unsigned short x, unsigned short y) {
	return facets[facet].getStaticBlock(x, y);
}

extern cMaps *Maps;

#endif
