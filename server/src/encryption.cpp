
#include "qstring.h"
#include "qdatastream.h"

#include "encryption.h"
#include "wpdefmanager.h"
#include "wpconsole.h"
#include "globals.h"
#include "prototypes.h"



/*!
	Initializes this "LoginCrypt" object. 
	Seed is the DWORD sent by the client in the beginning of each connection.
	Buffer/Length should be the first 62 bytes received by the client.
*/
bool cLoginEncryption::init( unsigned int seed, const char *buffer, unsigned int length )
{
	if( length < 62 )
		return false;
	
	// Try to find a valid key
	char packet[62];

	// Initialize our tables (cache them, they will be modified)
	unsigned int orgTable1 = ( ( ( ~seed ) ^ 0x00001357 ) << 16 ) | ( ( seed ^ 0xffffaaaa ) & 0x0000ffff );
	unsigned int orgTable2 = ( ( seed ^ 0x43210000 ) >> 16 ) | ( ( ( ~seed ) ^ 0xabcdffff ) & 0xffff0000 );

	for( unsigned int i = 0; i < KeyManager::instance()->size(); ++i )
	{
		stLoginKey *key = KeyManager::instance()->key( i );

		if( !key )
			break;

		// Check if this key works on this packet
		memcpy( packet, buffer, 62 );
		table1 = orgTable1;
		table2 = orgTable2;
		key1 = key->key1;
		key2 = key->key2;
        
		clientDecrypt( &packet[0], 62 );

		// Check if it decrypted correctly
		if( packet[0] == '\x80' && packet[30] == '\x00' && packet[60] == '\x00' )
		{
			// Reestablish our current state
			table1 = orgTable1;
			table2 = orgTable2;
			key1 = key->key1;
			key2 = key->key2;
			return true;
		}
	}

	return false;
}

/*!
	Encrypts the given buffer for sending it to the client.
	The encryption method used is "LoginCrypt".
*/
void cLoginEncryption::serverEncrypt( char *buffer, unsigned int length )
{
	return; // No Server->Client Encryption done sever-side
}

/*!
	Decrypts the given buffer received from the client.
	The decryption method used is "LoginCrypt".
*/
void cLoginEncryption::clientDecrypt( char *buffer, unsigned int length )
{
   register UINT32 eax, ecx, edx, esi;
   for( UINT32 i = 0; i < length; ++i )
   {
      buffer[i] = buffer[i] ^ (UINT8)( table1 & 0xFF );
      edx = table2;
      esi = table1 << 31;
      eax = table2 >> 1;
      eax |= esi;
      eax ^= key1 - 1;
      edx <<= 31;
      eax >>= 1;
      ecx = table1 >> 1;
      eax |= esi;
      ecx |= edx;
      eax ^= key1;
      ecx ^= key2;
      table1 = ecx;
      table2 = eax;
   }
}

/*!
	Initializes the gameserver encryption using the given seed
*/
void cGameEncryption::init( unsigned int seed )
{
	for( unsigned int i = 0; i < 256; ++i )
		cipherTable[i] = i;

	memset( &tfCipher, 0, sizeof( cipherInstance ) );
	memset( &tfKey, 0, sizeof( keyInstance ) );

	makeKey( &tfKey, DIR_DECRYPT, 0x80, 0 ); // 128 bit key (=4 dwords)
	

	// Byte-swap the seed
	// Seed is *fixed* to 0xFFFFFFFF
	tfKey.key32[0] = tfKey.key32[1] = tfKey.key32[2] = tfKey.key32[3] = 0xFFFFFFFF;
	
	cipherInit( &tfCipher, MODE_ECB, 0 );
	reKey( &tfKey );
	
	recvPos = 256;
	sendPos = 0x00;
}

/*!
	Decrypts a single byte sent by the client.
*/
void cGameEncryption::decryptByte( unsigned char &byte )
{
	// Recalculate table
	if( recvPos == 256 )
	{
		UINT8 tempBuffer[256];
		blockEncrypt( &tfCipher, &tfKey, cipherTable, 256*8, tempBuffer );
		memcpy( cipherTable, tempBuffer, 256 );
		recvPos = 0;
	}

	// Simple XOR operation
	byte ^= cipherTable[recvPos++];
}

/*!
	Decrypts a buffer sent by the client.
	Algorithm used is a slightly modified Twofish2 algorithm.
*/
void cGameEncryption::clientDecrypt( char* buffer, unsigned int length )
{
	for( unsigned int i = 0; i < length; ++i )
		decryptByte( (unsigned char&)buffer[i] );
}

/*!
	Encrypts a buffer before sending it to the client.
	Encryption used is a table-based XOR encryption.
*/
void cGameEncryption::serverEncrypt( char *buffer, unsigned int length )
{
   	static const UINT8 xorData[0x10] = 
	{
		// Seed: 7F000001
		//0x05, 0x92, 0x66, 0x23, 0x67, 0x14, 0xE3, 0x62, 0xDC, 0x60, 0x8C, 0xD6, 0xFE, 0x7C, 0x25, 0x69 
		
		// Seed: FFFFFFFF
		// Sniffed using a memory debugger
		0xa9, 0xd5, 0x7d, 0xa4, 0x3e, 0x0c, 0x22, 0xda, 0xde, 0x15, 0xe9, 0x92, 0xdd, 0x99, 0x98, 0x4d
	};

	for( unsigned int i = 0; i < length; ++i )
	{
		buffer[i] ^= xorData[ sendPos++ ];
		sendPos &= 0x0F; // Maximum Value is 0xF = 15, then 0xF + 1 = 0 again
	}
}

/*!
	Loads the key manager.
*/
cKeyManager::cKeyManager()
{
	QStringList list = DefManager->getList( "ENCRYPTION" );

	QStringList::const_iterator it;
	for( it = list.begin(); it != list.end(); ++it )
	{
		QStringList elements = QStringList::split( ";", *it, false );
        
		if( elements.size() < 3 )
		{
			clConsole.log( LOG_WARNING, QString( "Invalid encryption key line: %1" ).arg( *it ) );
			continue;
		}

        stLoginKey key;

		bool ok;

		key.key1 = hex2dec( elements[1].stripWhiteSpace() ).toUInt( &ok );

		if( !ok )
		{
			clConsole.log( LOG_WARNING, QString( "Couldn't parse key value: %1" ).arg( elements[1].stripWhiteSpace() ) );
			continue;
		}

		key.key2 = hex2dec( elements[2].stripWhiteSpace() ).toUInt( &ok );

		if( !ok )
		{
			clConsole.log( LOG_WARNING, QString( "Couldn't parse key value: %1" ).arg( elements[2].stripWhiteSpace() ) );
			continue;
		}
		
		keys.push_back( key );
	}
}
