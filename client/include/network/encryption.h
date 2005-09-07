
#if !defined(__ENCRYPTION_H__)
#define __ENCRYPTION_H__

#include <qstring.h>
#include "network/twofish2.h"

class cStreamEncryption {
public:
	virtual ~cStreamEncryption() {
	}

	virtual void encryptOutgoing( char *buffer, unsigned int length ) = 0;
	virtual void decryptIncoming( char* buffer, unsigned int length ) = 0;
};

// Used for Login Encryption
class cLoginEncryption : public cStreamEncryption {
private:
	unsigned int key1;
	unsigned int key2;
	unsigned int table1;
	unsigned int table2;
public:
	cLoginEncryption(unsigned int seed);

	void decryptIncoming( char* buffer, unsigned int length );
	void encryptOutgoing( char* buffer, unsigned int length );
};

// Used for GameServer Encryption
class cGameEncryption : public cStreamEncryption
{
private:
	unsigned short recvPos; // Position in our CipherTable (Recv)
	unsigned char sendPos; // Offset in our XOR Table (Send)
	unsigned char cipherTable[0x100];
	unsigned char xorData[16]; // This table is used for encrypting the server->client stream

	/*
		Note: Thanks to Negr0potence for the hint on uo.elitecoder.net.
		Crypting the initial twofish ciphertable... Man... This is typical...
	*/

	Twofish2::keyInstance ki;
	Twofish2::cipherInstance ci;

	void encryptByte( uchar & byte );
public:
	cGameEncryption(uint seed);

	void decryptIncoming( char* buffer, uint length );
	void encryptOutgoing( char* buffer, uint length );
};

// Used for GameServer Encryption
/*class cGameEncryption : public cClientEncryption
{
private:
	unsigned short recvPos; // Position in our CipherTable (Recv)
	unsigned char sendPos; // Offset in our XOR Table (Send)
	unsigned char cipherTable[0x100];
	unsigned char xorData[16]; // This table is used for encrypting the server->client stream
	
	//	Note: Thanks to Negr0potence for the hint on uo.elitecoder.net. 
	//	Crypting the initial twofish ciphertable... Man... This is typical...
	keyInstance ki;
	cipherInstance ci;

	void decryptByte( unsigned char& byte );

public:
	void init( unsigned int seed ); // Initialize this using the given seed
	void clientDecrypt( char* buffer, unsigned int length );
	void serverEncrypt( char* buffer, unsigned int length );
};*/

// Used for all No_Crypt_Clients
class cNoEncryption : public cStreamEncryption {
public:
	void encryptOutgoing(char *buffer, unsigned int length);
	void decryptIncoming(char* buffer, unsigned int length);
};

#endif
