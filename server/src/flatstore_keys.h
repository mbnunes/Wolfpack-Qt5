
#if !defined( __FLATSTOREKEYS_H__ )
#define __FLATSTOREKEYS_H__

// Defines a big list of keys used for subchunks, keep in mind
// That they ARE object dependant. We have at most 4096 keys
// per object. And yes i think we CAN reuse them.

	// Object-Types
	// Insert a unique key for each subclass here
	// IF YOU DELETE A SUBCLASS DONT REUSE THE SAME KEY!
	enum eObjectTypes
	{
		CHUNK_UOBJECT = 0x0000,
		CHUNK_ITEM = 0x0001,
		CHUNK_CHAR = 0x0002,
		CHUNK_SKILLS = 0x0003,
	};
	

#endif
