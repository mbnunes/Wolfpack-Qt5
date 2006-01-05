
#include "rsa.h"
#include "osrng.h"
#include "randpool.h"
#include "base64.h"
#include "cryptlib.h"
#include "filters.h"
#include "files.h"
#include <stdio.h>

using namespace CryptoPP;

bool __stdcall verifySignature(const char *publicKey, const char *dataFile, const char *signatureFile) {
	byte *signature = 0;
	bool result = false;

	try {
		// VERIFY
		RSASS<PKCS1v15, SHA512>::Verifier pubkey(
			StringSource(publicKey, true,
			new Base64Decoder));
		PK_MessageAccumulator *accum = pubkey.NewVerificationAccumulator();

		// Signatur lesen (Base64)
		signature = new byte[pubkey.SignatureLength()];	
			FileSource(signatureFile, true, new Base64Decoder(new ArraySink(signature, pubkey.SignatureLength())));
			pubkey.InputSignature(*accum, signature, pubkey.SignatureLength());

		// Hash aufbauen
		FILE *fp = fopen(dataFile, "rb");

		if (fp) {
			size_t read;
			char buffer[4096];
			while ((read = fread(buffer, 1, 4096, fp)) > 0) {
				accum->Update((byte*)buffer, read);
			}
			fclose(fp);
		} else {
			delete accum;
			throw Exception(Exception::IO_ERROR, "Unable to open data file.");						
		}

		result = pubkey.Verify(accum);
	} catch(Exception e) {
		result = false;
	}

	delete [] signature;

	return result;
}
