
#if !defined(__ENCRYPTION_H__)
#define __ENCRYPTION_H__

// Twofish is a C Header

extern "C" {
#include "twofish/aes.h"
}

#include <vector>
#include "qstring.h"
#include "singleton.h"

struct stLoginKey
{
	unsigned int key1;
	unsigned int key2;
};

// Key Manager
class cKeyManager
{
private:
	std::vector< stLoginKey > keys;

public:
	cKeyManager();
	unsigned int size() { return keys.size(); }
	stLoginKey *key( unsigned int id ) { if( id >= size() ) return 0; return &keys[id]; }
};

typedef SingletonHolder<cKeyManager> KeyManager;

// General Client Encryption Class
class cClientEncryption
{
public:
	virtual void serverEncrypt( char *buffer, unsigned int length ) = 0;
	virtual void clientDecrypt( char *buffer, unsigned int length ) = 0;
};

// Used for Login Encryption
class cLoginEncryption : public cClientEncryption
{
private:
	unsigned int key1;
	unsigned int key2;
	unsigned int table1;
	unsigned int table2;
public:
	bool init( unsigned int seed, const char *buffer, unsigned int length ); // Uses buffer as a way of identifying a valid encryption key
	void serverEncrypt( char *buffer, unsigned int length );
	void clientDecrypt( char *buffer, unsigned int length );
};

// Used for GameServer Encryption
class cGameEncryption : public cClientEncryption
{
private:
	unsigned short recvPos; // Position in our CipherTable (Recv)
	unsigned char sendPos; // Offset in our XOR Table (Send)
	unsigned char cipherTable[256];
	cipherInstance tfCipher;
	keyInstance tfKey;

	void decryptByte( unsigned char &byte );

public:
	void init( unsigned int seed ); // Initialize this using the given seed
	void clientDecrypt( char* buffer, unsigned int length );
	void serverEncrypt( char *buffer, unsigned int length );
};

// Used for all No_Crypt_Clients
class cNoEncryption : public cClientEncryption
{
public:
	void serverEncrypt( char *buffer, unsigned int length ) {}
	void clientDecrypt( char *buffer, unsigned int length ) {}
};

#endif
