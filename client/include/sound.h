
#if !defined(__SOUND_H__)
#define __SOUND_H__

#include <qobject.h>

// Forward declarations, other files don't need to know the real type of those
// pointers
#if !defined(ALAPI)
#define ALCdevice void
#define ALCcontext void
#endif

class cSound : QObject {
Q_OBJECT
protected:
	static const unsigned int NUM_BUFFERS = 16;

	// OpenAL Device + Context
	ALCdevice *device;
	ALCcontext *context;
	quint32 buffers[NUM_BUFFERS];
	qint32 bufferIds[NUM_BUFFERS];
	quint32 sources[NUM_BUFFERS];

public:
	cSound();

	void load();
	void unload();

	void playSound(unsigned short id); // Most simple of all sound playing methods
};

extern cSound *Sound;

#endif
