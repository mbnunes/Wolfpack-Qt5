
#if !defined( __BASEDEF_H__ )
#define __BASEDEF_H__

#include "singleton.h"

/*
	Class which contains information about characters that never changes.
 */
class cCharBaseDef
{
friend class cBaseDefManager;

protected:
	// Sounds
	unsigned short basesound_;
	unsigned char soundmode_;
	unsigned short shrinked_;
	unsigned int flags_;
	unsigned char type_;

public:
	cCharBaseDef() {}
	virtual ~cCharBaseDef() {}

	// Getters ONLY
	// These are READONLY PROPERTIES!
	unsigned short basesound() const { return basesound_; }
	unsigned char soundmode() const { return soundmode_; }
	unsigned short shrinked() const { return shrinked_; }
	unsigned int flags() const { return flags_; }
	unsigned char type() const { return type_; }
};

class cBaseDefManager
{
protected:
	cCharBaseDef *chardefs[0x400]; // 0x400 entries in anim.idx

public:
	cBaseDefManager() {}
	virtual ~cBaseDefManager() {}

	cCharBaseDef *getCharBaseDef( unsigned short id );

	void load();
	
	void reload()
	{
		unload();
		load();
	}

	void unload();
};

typedef SingletonHolder< cBaseDefManager > BaseDefManager;

#endif
