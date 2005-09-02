/*
 * Parts of cGameEncryption are based on work done in injection
 * injection.sf.net
 */

#include <qstring.h>

#include "config.h"
#include "network/encryption.h"

/*!
	Initializes this "LoginCrypt" object.
	Seed is the DWORD sent by the client in the beginning of each connection.
	Buffer/Length should be the first 62 bytes received by the client.
*/
cLoginEncryption::cLoginEncryption( unsigned int seed ) {
	table1 = ( ( ( ~seed ) ^ 0x00001357 ) << 16 ) | ( ( seed ^ 0xffffaaaa ) & 0x0000ffff );
	table2 = ( ( seed ^ 0x43210000 ) >> 16 ) | ( ( ( ~seed ) ^ 0xabcdffff ) & 0xffff0000 );

	key1 = Config->encryptionLoginKey1();
	key2 = Config->encryptionLoginKey2();
}

/*!
	Encrypts the given buffer for sending it to the client.
	The encryption method used is "LoginCrypt".
*/
void cLoginEncryption::decryptIncoming( char* buffer, unsigned int length )
{
	Q_UNUSED( buffer );
	Q_UNUSED( length );
	return; // No Server->Client Encryption done sever-side
}

/*!
	Decrypts the given buffer received from the client.
	The decryption method used is "LoginCrypt".
*/
void cLoginEncryption::encryptOutgoing( char* buffer, unsigned int length )
{
	register uint eax, ecx, edx, esi;
	for ( uint i = 0; i < length; ++i )
	{
		buffer[i] = buffer[i] ^ ( uchar ) ( table1 & 0xFF );
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

void cNoEncryption::encryptOutgoing(char*, unsigned int) {
	// Do nothing
}

void cNoEncryption::decryptIncoming(char*, unsigned int) {
	// Do nothing
}
