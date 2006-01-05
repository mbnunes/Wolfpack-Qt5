
#include <stdio.h>
#include "md5.h"
#include "rsa.h"
#include "osrng.h"
#include "randpool.h"
#include "base64.h"
#include "cryptlib.h"
#include "filters.h"
#include "files.h"

/**
	Public key used for verifying updates.
	Generated on January 4th 2006
	by Sebastian Hartte
*/
static const char *PUBLICKEY =	"MIICIDANBgkqhkiG9w0BAQEFAAOCAg0AMIICCAKCAgEAld863xhbuWEBBZLkUqE1+6puKytG"
								"kF4hynpkfmaY5w1dT5ZT+KM8HY9w/6yy6epD6QLkQMfbufbADB1MSCbw+bV31rDUwT96L3I+"
								"UCzHeVSnZI/H0Vi01RCBV7petOS7j/LSjYKQMiA8Sn5UesqiBle4AxTFxOqHF7bnoOMH5dzP"
								"cJ+Au6i7wzHqmPAgESYVhL9qPx3ezLlxZVkGlrV5gaHUcfovfAQ9oq3t0j6KBE1lCSYO+O+E"
								"t8YE1rLFuKZDGgOhntuFF1FF0wQHgu/2sbVUziq3M1lyeEKUdhfprsiJiaIpW5CF6r9vHRSJ"
								"yvPgxkWnFyQTDSVlA1RNQ4mcSt+Z4qpXKdacyVRtC2q38KSRmRLFjBzUOkFA6cxxs3gVlZRy"
								"Txfc8Vj1RVyupF9LaAIuydkEFbMV0YgVgdsXugidpLZ5KLhf7LCGvoralim/Q+IZctXlHAkY"
								"ERlQflATZ1X9DK3gQp4KD8+FWlFrstSKLR4rKhBSTLf6VyKbln1SbV2USz0JdOTrjbj66iVt"
								"Ei2Vpbd6I0fgFCSzmYIF5kYhfe2Ph9d8WQAgCtg4LhBxpji6ac2NyNye/YvBYHfNQaOXlpZ9"
								"oKtPDcezn2Yzt9+1cVbHj6rPylaWrb9jgZmcygVVr9tgm38Gw1eHNehNBnm6ZABPH6QzP/iy"
								"WsJ5UqcCARE=";

using namespace CryptoPP;

#include "../codeauthentication/signature.h"
#pragma comment(lib, "../Release/codeauthentication.lib")

int main(int argc, char **argv) {
	bool result = verifySignature(PUBLICKEY, "C:/test.dat", "c:/test.dat.sig");

	if (result) {
		printf("Signature verification succeeded.");
	} else {
		printf("Signature verification failed.");
	}

	/*
	RSASS<PKCS1v15, SHA512>::Signer privkey(
     StringSource(PRIVATEKEY, true,
         new Base64Decoder));

	AutoSeededRandomPool rng;
	PK_MessageAccumulator *accum = privkey.NewSignatureAccumulator(rng);

	FILE *fp = 0;
	fopen_s(&fp, "C:/test.dat", "rb");	

	if (fp) {
		size_t read;
		char buffer[4096];
		while ((read = fread(buffer, 1, 4096, fp)) > 0) {
			accum->Update((byte*)buffer, read);
		}

		fclose(fp);
	}

	byte *signature = new byte[privkey.SignatureLength()];
	privkey.Sign(rng, accum, signature);

	Base64Encoder encoder(new FileSink("C:\\signed.dat"));
	encoder.Put(signature, privkey.SignatureLength());
	encoder.MessageEnd();

	// VERIFY
	RSASS<PKCS1v15, SHA512>::Verifier pubkey(
			StringSource(PUBLICKEY, true,
			new Base64Decoder));
	accum = pubkey.NewVerificationAccumulator();
	pubkey.InputSignature(*accum, signature, privkey.SignatureLength());

	// Accumulate stuff
	fp = 0;
	fopen_s(&fp, "C:/test.dat", "rb");	

	if (fp) {
		size_t read;
		char buffer[4096];
		while ((read = fread(buffer, 1, 4096, fp)) > 0) {
			accum->Update((byte*)buffer, read);
		}

		fclose(fp);
	}

	bool verification = pubkey.Verify(accum);

	delete[] signature;

	if (verification) {
		printf("File Verification succeeded!");
	} else {
		printf("File Verification failed!");
	}*/

/*
	byte bDigest[CryptoPP::MD5::DIGESTSIZE];
	CryptoPP::MD5 hashFunction;

	FILE *fp = fopen("C:/test.dat", "rb");	

	if (fp) {
		size_t read;
		char buffer[4096];
		while ((read = fread(buffer, 1, 4096, fp)) > 0) {
			hashFunction.Update((byte*)buffer, read);
		}

		fclose(fp);

		hashFunction.Final(bDigest);

		for (int i = 0; i < sizeof(bDigest); ++i) {
			printf("%02x", bDigest[i]);
		}
		printf("\n");
	}*/

	return 0;
}
