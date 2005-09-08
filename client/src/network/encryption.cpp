/*
 * Parts of cGameEncryption are based on work done in injection
 * injection.sf.net
 */

#include <QString>

#include "config.h"
#include "md5.h"
#include "network/encryption.h"
#include "network/twofish2.h"

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

class cGameEncryption::cGameEncryptionPrivate
{
public:
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
};

/*!
	Initializes the gameserver encryption using the given seed
*/
cGameEncryption::cGameEncryption( unsigned int seed ) : d( new cGameEncryptionPrivate )
{
	// The seed is transmitted in little-endian byte order
	// At least that is what sphere thinks
	seed = ((seed >> 24) & 0xFF) | ((seed >> 8) & 0xFF00) | ((seed << 8) & 0xFF0000) | ((seed << 24) & 0xFF000000);

	makeKey( &d->ki, DIR_ENCRYPT, 0x80, NULL );
	cipherInit( &d->ci, MODE_ECB, NULL );

	d->ki.key32[0] = d->ki.key32[1] = d->ki.key32[2] = d->ki.key32[3] = seed;
	reKey( &d->ki );

	for ( unsigned int i = 0; i < 256; ++i )
		d->cipherTable[i] = i;

	d->sendPos = 0x00;
	d->recvPos = 0;

	// We need to fill the table initially to calculate the MD5 hash of it
	unsigned char tmpBuffer[0x100];
	blockEncrypt( &d->ci, &d->ki, &d->cipherTable[0], 0x800, &tmpBuffer[0] );
	memcpy( &d->cipherTable[0], &tmpBuffer[0], 0x100 );

	// Create a MD5 hash of the twofish crypt data and use it as a 16-byte xor table
	// for encrypting the server->client stream.
	cMd5 md5;
	md5.update(tmpBuffer, 0x100);
	md5.finalize();
	md5.rawDigest(d->xorData);
}

/*!
	Decrypts a single byte sent by the client.
*/
void cGameEncryption::encryptByte( uchar& byte )
{
	// Recalculate table
	if ( d->recvPos >= 0x100 )
	{
		unsigned char tmpBuffer[0x100];
		blockEncrypt( &d->ci, &d->ki, &d->cipherTable[0], 0x800, &tmpBuffer[0] );
		memcpy( &d->cipherTable[0], &tmpBuffer[0], 0x100 );
		d->recvPos = 0;
	}

	// Simple XOR operation
	byte ^= d->cipherTable[d->recvPos++];
}

/*!
	Decrypts a buffer sent by the client.
	Algorithm used is a slightly modified Twofish2 algorithm.
*/
void cGameEncryption::encryptOutgoing( char* buffer, unsigned int length )
{
	for ( unsigned int i = 0; i < length; ++i )
		encryptByte( ( unsigned char & ) buffer[i] );
}

/*!
	Encrypts a buffer before sending it to the client.
	Encryption used is a table-based XOR encryption.
*/
void cGameEncryption::decryptIncoming( char* buffer, unsigned int length )
{
	for ( unsigned int i = 0; i < length; ++i ) {
		buffer[i] ^= d->xorData[d->sendPos++];
		d->sendPos &= 0x0F; // Maximum Value is 0xF = 15, then 0xF + 1 = 0 again
	}
}
