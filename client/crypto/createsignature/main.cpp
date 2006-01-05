
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
	if (argc != 3) {
		fprintf(stderr, "Usage: createsignature <private key file> <file to sign>");
		return -1;
	}

	byte *signature = 0;
	int result = -1;

	try {
		// First argument should be a private key
		RSASS<PKCS1v15, SHA512>::Signer privkey(
			FileSource(argv[1], true,
			new Base64Decoder));

		// Second argument is the file to sign
		AutoSeededRandomPool rng;
		PK_MessageAccumulator *accum = privkey.NewSignatureAccumulator(rng);

		// Read and hash the input file 2
		FILE *fp = fopen(argv[2], "rb");

		if (!fp) {
			throw Exception(Exception::IO_ERROR, "Unable to open the data file.");
		}

		size_t read;
		byte buffer[4096];
		while ((read = fread(buffer, sizeof(byte), sizeof(buffer), fp)) > 0) {
			accum->Update(buffer, sizeof(buffer));
		}

		fclose(fp);

		// Signatur erstellen
		signature = new byte[privkey.SignatureLength()];
		privkey.Sign(rng, accum, signature);

		// Signatur ausgeben als Base64
		std::string b64Signature;
		Base64Encoder encoder(new StringSink(b64Signature), false);
		encoder.Put(signature, privkey.SignatureLength());
		encoder.MessageEnd();

		// Signatur auf der Konsole ausgeben
		printf(b64Signature.c_str());
	} catch(Exception e) {
		fprintf(stderr, e.GetWhat().c_str());
	}

	delete [] signature;

	return result;
}
