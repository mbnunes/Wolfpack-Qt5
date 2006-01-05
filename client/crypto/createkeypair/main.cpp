
#include <stdio.h>
#include "md5.h"
#include "rsa.h"
#include "osrng.h"
#include "randpool.h"
#include "base64.h"
#include "cryptlib.h"
#include "filters.h"
#include "files.h"

using namespace CryptoPP;

int main(int argc, char **argv) {
	int result = -1;

	if (argc != 3) {
		fprintf(stderr, "Usage: createkeypair <private key file> <public key file>");
		return result;
	}

	try {
		 // InvertibleRSAFunction is used directly only because the private key
		 // won't actually be used to perform any cryptographic operation;
		 // otherwise, an appropriate typedef'ed type from rsa.h would have been used.
		 AutoSeededRandomPool rng;
		 InvertibleRSAFunction privkey;
		 privkey.Initialize(rng, 4096);

		 // With the current version of Crypto++, MessageEnd() needs to be called
		 // explicitly because Base64Encoder doesn't flush its buffer on destruction.
		 Base64Encoder privkeysink(new FileSink(argv[1]));
		 privkey.DEREncode(privkeysink);
		 privkeysink.MessageEnd();
		 
		 // Suppose we want to store the public key separately,
		 // possibly because we will be sending the public key to a third party.
		 RSAFunction pubkey(privkey);
		 
		 Base64Encoder pubkeysink(new FileSink(argv[2]));
		 pubkey.DEREncode(pubkeysink);
		 pubkeysink.MessageEnd();

		 result = 0;
	} catch(Exception e) {
		fprintf(stderr, e.GetWhat().c_str());
		result = -1;
	}

	return result;
}
