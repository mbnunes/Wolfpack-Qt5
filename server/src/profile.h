
#if !defined(__PROFILE_H__)
#define __PROFILE_H__

enum eProfileKeys {
	PF_NICENESS = 0,
	PF_SPAWNCHECK,
	PF_DECAYCHECK,
	PF_WORLDSAVE,
	PF_UOTIMECHECK,
	PF_COMBATCHECK,
	PF_TIMERSCHECK,
	PF_AICHECK,
	PF_NPCCHECK,
	PF_PLAYERCHECK,
	PF_REGENERATION,
	PF_AICHECKFINDACTION,
	PF_AICHECKEXECUTEACTION,
	PF_COUNT,
};

#define ENABLE_PROFILING 0

#if defined(ENABLE_PROFILING)
void startProfiling(eProfileKeys key);
void stopProfiling(eProfileKeys key);
void dumpProfilingInfo();
void clearProfilingInfo();
#else
#define startProfiling(key)
#define stopProfiling(key)
#define dumpProfilingInfo()
#define clearProfilingInfo()
#endif

#endif
