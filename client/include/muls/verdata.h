
#if !defined(__VERDATA_H__)
#define __VERDATA_H__

#include <qmap.h>
#include <qfile.h>

enum VerFileIds {
	VERDATA_MAP0 = 0,
	VERDATA_STAIDX0,
	VERDATA_STATICS0,
	VERDATA_ARTIDX,
	VERDATA_ART, /* USED */
	VERDATA_ANIMIDX,
	VERDATA_ANIM,
	VERDATA_SOUNDIDX,
	VERDATA_SOUND,
	VERDATA_TEXIDX,
	VERDATA_TEXMAPS = 10,
	VERDATA_GUMPIDX,
	VERDATA_GUMPART, /* USED */
	VERDATA_MULTIIDX,
	VERDATA_MULTI,
	VERDATA_SKILLSIDX,
	VERDATA_SKILLS = 0x10,
	VERDATA_TILEDATA = 0x1E,
	VERDATA_ANIMDATA = 0x1F,
	VERDATA_COUNT = 0x20
};

struct stVerdataRecord {
	int offset;
	int length;
	short height;
	short width;
};

class cVerdata {
private:
	bool enabled; // Indicates whether the verdata is used at all	
	typedef QMap<unsigned int, stVerdataRecord> Container;
	QFile data;
	Container records[VERDATA_COUNT];

public:
	cVerdata();
	~cVerdata();

	void load();
	void unload();
	void reload();

	inline stVerdataRecord *getPatch(VerFileIds file, unsigned int block) {
		if (enabled && file < VERDATA_COUNT && records[file].contains(block)) {
			return &records[file][block];
		} else {
			return 0;
		}
	}

	inline QFile *getIoDevice() {
		return &data;
	}
};

extern cVerdata *Verdata;

#endif
