
#include <qglobal.h>

// OpenAL Includes
// Include first to get the real functionality
#include "AL/al.h"
#include "AL/alc.h"
#if defined(Q_OS_WIN32)
#include "ALUT/alut.h"
#else
#include "AL/alut.h"
#endif

#include "exceptions.h"
#include "sound.h"
#include "muls/sounds.h"
#include "log.h"
#include "utilities.h"
#include "config.h"

#include <qfile.h>

cSound::cSound() {
	device = 0;
	context = 0;
	memset(buffers, 0, sizeof(buffers));
	memset(buffers, 0, sizeof(sources));
	memset(bufferIds, -1, sizeof(bufferIds));
}

void cSound::load() {
	if (Config->disableSound()) {
		return;
	}

	// Try to open the preferred device
#if defined(NO_DIRECTSOUND)
	device = alcOpenDevice("MMSYSTEM");
#else
	device = alcOpenDevice(0);
#endif

	// Unable to initialize?
	if (!device) {
		throw new Exception(tr("Unable to open the preferred OpenAL device."));
	}

	// Print the used device name to the logfile
	const char *deviceName = (char*)alcGetString(device, ALC_DEVICE_SPECIFIER);
	if (deviceName) {
		Log->print(LOG_NOTICE, tr("Using OpenAL device '%1' for sound output.\n").arg(deviceName));
	} else {
		Log->print(LOG_NOTICE, tr("Unable to obtain OpenAL device name for sound output.\n"));
	}

	const char *extensionString = (char*)alcGetString(device, ALC_EXTENSIONS);
	if (extensionString) {
		Log->print(LOG_NOTICE, tr("Supported OpenAL extensions: %1\n").arg(extensionString));
	} else {
		Log->print(LOG_NOTICE, tr("Unable to obtain list of supported OpenAL extensions.\n"));
	}

	// Try to create a context
	context = alcCreateContext(device, 0);

	if (!context) {
		throw new Exception(tr("Unable to create OpenAL context."));
	}

	alcGetError(device); // Clear current error
	alcMakeContextCurrent(context);
	if (alcGetError(device) != ALC_NO_ERROR) {
		throw new Exception(tr("Unable to make the created OpenAL context the current context."));
	}

	// Clear error codes
	alGetError();
	alcGetError(device);
    
	// Check OpenAL version and print it to the logfile
	ALint lMajor, lMinor;
	alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &lMajor);	
	alcGetIntegerv(device, ALC_MINOR_VERSION, 1, &lMinor);
	Log->print(LOG_NOTICE, tr("Using OpenAL Version %1.%2\n").arg(lMajor).arg(lMinor));

	// Generate buffers/sources
	alGenBuffers(NUM_BUFFERS, buffers);
	if (alcGetError(device) != ALC_NO_ERROR) {
		throw new Exception(tr("Unable to generate %1 OpenAL buffers.").arg(NUM_BUFFERS));
	}

	alGenSources(NUM_BUFFERS, sources);

	if (alcGetError(device) != ALC_NO_ERROR) {
		throw new Exception(tr("Unable to generate %1 OpenAL sources.").arg(NUM_BUFFERS));
	}

	alSourceStopv(NUM_BUFFERS, sources);
}

void cSound::unload() {
	if (Config->disableSound()) {
		return;
	}

	alSourceStopv(NUM_BUFFERS, sources);
	alDeleteBuffers(NUM_BUFFERS, buffers);
	alSourceStopv(NUM_BUFFERS, sources);
	alDeleteSources(NUM_BUFFERS, sources);

	if (context) {
		alcMakeContextCurrent(0); // Cancel current context assignment
		alcDestroyContext(context);
		context = 0;
	}

	if (device) {
		alcCloseDevice(device);
		device = 0;
	}
}

void cSound::playSound(unsigned short id) {
	if (Config->disableSound()) {
		return;
	}

	int bufferId = -1; // id of buffer with same id
	int sourceId = -1; // id of free source
	bool bufferUsed[NUM_BUFFERS];
	for (unsigned int i = 0; i < NUM_BUFFERS; ++i) {
		bufferUsed[i] = false; // Default to unused
	}

	// See which buffers are currently in used and see if a buffer is using the same id as the
	// sound that should be played. While at it, look for sources in use
	for (unsigned int i = 0; i < NUM_BUFFERS; ++i) {
		// See if the given buffer contains the same sound. A value of -1 means, 
		// that the played sound is "uncommon" and shouldn't be shared.
		if (id != ~0 && bufferIds[i] == id) {
			bufferId = i;
		}

        // Check the source if it's free. If not, flag the buffer as in-use
		ALint sourceState;
		alGetSourcei(sources[i], AL_SOURCE_STATE, &sourceState);

		if (sourceState != AL_STOPPED && sourceState != AL_INITIAL) {
			// Flag the used buffer as in-use
			ALint bufferId;
			alGetSourcei(sources[i], AL_BUFFER, &bufferId);

			if (bufferId >= 0 && bufferId < NUM_BUFFERS) {
				bufferUsed[bufferId] = true;
			}
		} else if (sourceId == -1) {
			sourceId = i;
		}
	}

	// If no source is free right now, don't play anything
	if (sourceId == -1) {
		return;
	}

	// If we don't have a bufferId yet, look for a non-playing source
	if (bufferId == -1) {
		// Look for a free buffer
		for (int i = 0; i < NUM_BUFFERS; ++i) {
			if (!bufferUsed[i]) {
				bufferId = i;
				break;
			}
		}

		// If no free buffer has been found, don't play any sound.
		if (bufferId == -1) {
			return;
		}

		// Read the data from disk
		cSample *sample = Sounds->readSound(id);
		
		if (!sample) {
			Log->print(LOG_WARNING, tr("Trying to play unknown sound with id %1.\n").arg(id, 0, 16));
			return;
		}
		
		const QByteArray &data = sample->data;

		// Find sources connected to our buffer and disconnect them
		for (int i = 0; i < NUM_BUFFERS; ++i) {
			ALint srcBufferId;
			alGetSourcei(sources[i], AL_BUFFER, &srcBufferId);

			if (buffers[bufferId] == srcBufferId) {
				alSourceStop(sources[i]);
				alSourcei(sources[i], AL_BUFFER, AL_NONE);
			}
		}

		// Otherwise load our data into the buffer
		alBufferData(buffers[bufferId], AL_FORMAT_MONO16, data.data(), data.size(), 22050);
		bufferIds[bufferId] = id; // Store the id of this sound

		delete sample;
	}

	// Assign the buffer to the selected source and play it
	alSourcei(sources[sourceId], AL_BUFFER, buffers[bufferId]);
	alSourcePlay(sources[sourceId]);
}

cSound *Sound = 0;
