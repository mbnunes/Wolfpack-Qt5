
#include "profile.h"
#include "console.h"

#if defined(ENABLE_PROFILING)

const char *profileNames[PF_COUNT] = {
	"Niceness/PythonThreads",
	"Spawnregion Checks",
	"DecayCheck",
	"WorldSave",
	"UO Time Check",
	"Combat Check",
	"Timers Check",
	"AI Check",
	"NPC Check",
	"Player Check",
	"Regeneration Check",
	"AI Check\\Search New Action",
	"AI Check\\Execute Action",
};

// Time in MS spent in the given profile keys
unsigned int profileData[PF_COUNT];
unsigned int profileStart[PF_COUNT];

// Time spent from first to last profiling
unsigned int startTime = 0;

void startProfiling(eProfileKeys key) {
	Server::instance()->refreshTime();
	unsigned int time = Server::instance()->time();

	// Let's just say this has been the server start
	if (!startTime) {
		startTime = time;
	}

	profileStart[key] = time;
}

void stopProfiling(eProfileKeys key) {
	Server::instance()->refreshTime();
	unsigned int time = Server::instance()->time();
	unsigned int diff = time - profileStart[key];
	profileData[key] += diff;
}

void clearProfilingInfo() {
	Server::instance()->refreshTime();
	startTime = Server::instance()->time();
	for (int i = 0; i < PF_COUNT; ++i) {
		profileData[i] = 0;
	}
}

void dumpProfilingInfo() {
	Server::instance()->refreshTime();
	unsigned int total = Server::instance()->time() - startTime;

	Console::instance()->send("PROFILING INFORMATION:\n");
	Console::instance()->send("TOTAL TIME: " + QString::number(total) + "\n");
	
	for (int i = 0; i < PF_COUNT; ++i) {
		Console::instance()->send(profileNames[i]);
		Console::instance()->send(QString(": %1\n").arg(profileData[i]));
	}

	// Clear Profiling Info
	clearProfilingInfo();
}

#endif
