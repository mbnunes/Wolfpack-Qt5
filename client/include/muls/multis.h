
#if !defined(__MULTIS_H__)
#define __MULTIS_H__

#include <QVector>
#include <QFile>
#include <QObject>
#include <QCache>

const uint multiCount = 0x1ff6;

struct stMultiItem {
	ushort id; // Item id
	short x, y, z; // X, Y and Z offsets
};

typedef QVector<stMultiItem> MultiData;

class cMultis : QObject {
Q_OBJECT
protected:
	QFile data;
	unsigned int lengths[multiCount]; // Total number of multis
	unsigned int offsets[multiCount]; // Offset for multi data

	QCache<ushort, MultiData> cache;
public:
	cMultis();
	
	void load();
	void unload();

	MultiData readMulti(ushort id);
};

extern cMultis *Multis;

#endif
