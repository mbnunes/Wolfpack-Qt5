
#if !defined( __FLATSTOREKEYS_H__ )
#define __FLATSTOREKEYS_H__

// Object-Types
// Insert a unique key for each subclass here
// IF YOU DELETE A SUBCLASS DONT REUSE THE SAME KEY
// This is required for versioning to work
enum eObjectTypes
{
	CHUNK_UOBJECT = 0x0000,
	CHUNK_ITEM = 0x0001,
	CHUNK_CHAR = 0x0002,
	CHUNK_SKILLS = 0x0003,
	CHUNK_BOOK = 0x0004,
	CHUNK_NPC = 0x0005,
	CHUNK_PLAYER = 0x0006
};

#endif
