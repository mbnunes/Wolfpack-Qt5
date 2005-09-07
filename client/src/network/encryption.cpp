/*
 * Parts of cGameEncryption are based on work done in injection
 * injection.sf.net
 */

#include <qstring.h>

#include "config.h"
#include "md5.h"
#include "network/encryption.h"

/*!
	Initializes this "LoginCrypt" object.
	Seed is the ulong sent by the client in the beginning of each connection.
	Buffer/Length should be the first 62 uchars received by the client.
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

/*!
	Initializes the gameserver encryption using the given seed
*/
cGameEncryption::cGameEncryption( unsigned int seed )
{
	// The seed is transmitted in little-endian uchar order
	// At least that is what sphere thinks
	seed = ((seed >> 24) & 0xFF) | ((seed >> 8) & 0xFF00) | ((seed << 8) & 0xFF0000) | ((seed << 24) & 0xFF000000);

	makeKey( &ki, DIR_ENCRYPT, 0x80, NULL );
	cipherInit( &ci, MODE_ECB, NULL );

	ki.key32[0] = ki.key32[1] = ki.key32[2] = ki.key32[3] = seed;
	reKey( &ki );

	for ( unsigned int i = 0; i < 256; ++i )
		cipherTable[i] = i;

	sendPos = 0x00;
	recvPos = 0;

	// We need to fill the table initially to calculate the MD5 hash of it
	unsigned char tmpBuffer[0x100];
	blockEncrypt( &ci, &ki, &cipherTable[0], 0x800, &tmpBuffer[0] );
	memcpy( &cipherTable[0], &tmpBuffer[0], 0x100 );

	// Create a MD5 hash of the twofish crypt data and use it as a 16-uchar xor table
	// for encrypting the server->client stream.
	cMd5 md5;
	md5.update(tmpBuffer, 0x100);
	md5.finalize();
	md5.rawDigest(xorData);
}

/*!
	Decrypts a single uchar sent by the client.
*/
void cGameEncryption::encryptuchar( uchar& uchar )
{
	// Recalculate table
	if ( recvPos >= 0x100 )
	{
		unsigned char tmpBuffer[0x100];
		blockEncrypt( &ci, &ki, &cipherTable[0], 0x800, &tmpBuffer[0] );
		memcpy( &cipherTable[0], &tmpBuffer[0], 0x100 );
		recvPos = 0;
	}

	// Simple XOR operation
	uchar ^= cipherTable[recvPos++];
}

/*!
	Decrypts a buffer sent by the client.
	Algorithm used is a slightly modified Twofish2 algorithm.
*/
void cGameEncryption::encryptOutgoing( char* buffer, unsigned int length )
{
	for ( unsigned int i = 0; i < length; ++i )
		encryptuchar( ( unsigned char & ) buffer[i] );
}

/*!
	Encrypts a buffer before sending it to the client.
	Encryption used is a table-based XOR encryption.
*/
void cGameEncryption::decryptIncoming( char* buffer, unsigned int length )
{
	for ( unsigned int i = 0; i < length; ++i ) {
		buffer[i] ^= xorData[sendPos++];
		sendPos &= 0x0F; // Maximum Value is 0xF = 15, then 0xF + 1 = 0 again
	}
}
