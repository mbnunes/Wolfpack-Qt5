//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2003 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#if !defined(__ENCRYPTION_H__)
#define __ENCRYPTION_H__

#include <vector>
#include <qstring.h>
#include "singleton.h"

extern "C" {
#include "twofish/aes.h"
}

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

	void load();
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
	unsigned char cipherTable[0x100];

    keyInstance ki;
    cipherInstance ci;

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
