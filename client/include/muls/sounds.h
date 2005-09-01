
#if !defined(__SOUNDS_H__)
#define __SOUNDS_H__

#include <QObject>
#include <qfile.h>

// A single sample
class cSample {
public:
	QByteArray data;
    unsigned short id;
};

class cSounds : public QObject {
	Q_OBJECT

protected:
	QFile data;
	unsigned int lengths[4096]; // Total number of sounds
	unsigned int offsets[4096]; // Offset for sounds

public:
	cSounds();
	
	void load();
	void unload();

	cSample *readSound(unsigned short id);
};

extern cSounds *Sounds;

#endif
