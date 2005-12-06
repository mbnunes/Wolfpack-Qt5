
#include <stdio.h>
#include <stdlib.h>

void readEntireFile(FILE *fp, char *&data, size_t &size) {
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	data = (char*)malloc(size);
	fseek(fp, 0, SEEK_SET);	
	
	size_t offset = 0;
	size_t readSize = ~0;

	while (readSize != 0) {
		readSize = fread(data + offset, 1, 4096, fp);
		offset += readSize;
	}
}

bool searchClientKeys(char *data, size_t size, int &key1, int &key2) {
	if (size < 100) {
		return false;
	}

	// Start byte: C1
	// At least 100 byte space

	// What we expect:
	/*
		0041A518  |. C1E2 1F        |SHL EDX,1F
		0041A51B  |. D1E8           |SHR EAX,1
		0041A51D  |. D1E9           |SHR ECX,1
		0041A51F  |. 0BC6           |OR EAX,ESI
		0041A521  |. 0BCA           |OR ECX,EDX
		0041A523  |. 35 EDA7AB2E    |XOR EAX,2EABA7ED
		0041A528  |. 81F1 7F7E41A2  |XOR ECX,A2417E7F
		0041A52E  |. 4D             |DEC EBP
	*/

	for (size_t i = 0; i < size - 100; ++i) {
		// Check the given position
		unsigned char *pos = (unsigned char*)data + i;

		if (*(pos++) != 0xc1) {
			continue; // No SHL instruction
		}

		if (*pos < 0xe0 || *pos > 0xe7) {
			continue; // Not a valid register number
		}

		// The first SHL register
		char firstRegister = (*pos) & 0xF;

		pos++; // Advance to next position

		// The 1f constant
		if (*(pos++) != 0x1f) {
			continue;
		}

		// **** LOOK FOR ****
		// 00425BBD  |. D1E9           |SHR ECX,1
		// **** LOOK FOR ****

		if (*(pos++) != 0xd1) {
			continue; // Not Rotate Right Once opcode
		}

		// Check for register type (Cannot be firstRegister)
		if ((*pos & 0xe8) != 0xe8) {
			continue; // Not the register extension operand
		}

		char secondRegister = *pos & 7;

		if (secondRegister == firstRegister) {
			continue; // Not what we want
		}

		pos++; // Next operation

		// **** LOOK FOR ****
		// 00425BBF  |. D1EA           |SHR EDX,1
		// **** LOOK FOR ****

		if (*(pos++) != 0xd1) {
			continue; // Not Rotate Right Once opcode
		}

		// Check for register type (Cannot be firstRegister or secondRegister)
		if ((*pos & 0xe8) != 0xe8) {
			continue; // Not the register extension operand
		}

		char thirdRegister = *pos & 7;

		if (thirdRegister == firstRegister || thirdRegister == secondRegister) {
			continue; // Not possible
		}

		pos++; // Next operation

		// **** LOOK FOR ****
		// 00425BC1  |. 0BCF           |OR ECX,EDI
		// **** LOOK FOR ****
		if (*(pos++) != 0x0B) {
			continue; // No OR operation
		}

		// Check for register extension byte
		if ((*pos & 0xC0) != 0xC0) {
			continue;
		}

		char operand1 = (*pos >> 3) & 7;
		char operand2 = *pos & 7;

		// Operand1 is supposed to be the secondRegister while 
		// operand2 may not be any of the three registers encountered yet
		if (operand1 != secondRegister || (operand2 == firstRegister || operand2 == secondRegister || operand2 == thirdRegister)) {
			continue;
		}

		pos++;

		// **** LOOK FOR ****
		// 00425BC3  |. 0BD6           |OR EDX,ESI
		// **** LOOK FOR ****
		if (*(pos++) != 0x0B) {
			continue; // No OR operation
		}

		// Check for register extension byte
		if ((*pos & 0xC0) != 0xC0) {
			continue;
		}

		operand1 = (*pos >> 3) & 7;
		operand2 = *pos & 7;

		// Operand1 is supposed to be the firstRegister while 
		// Operand2 should be the third register
		if (operand1 != thirdRegister || operand2 != firstRegister) {
			continue;
		}

		pos++;

		// Our keys
		// 00425BC5  |. 81F1 BD85F32D  |XOR ECX,2DF385BD
		// 0041A523  |. 35 EDA7AB2E    |XOR EAX,2EABA7ED
		if (*pos == 0x81) {
			pos++;
			if (*pos < 0xf0 || *pos > 0xf7) {
				continue;
			}

			operand1 = *pos & 7;
		} else if (*pos == 0x35) {
			operand1 = 0;
		} else {
			continue; // No XOR
		}

		// The XOR operand is supposed to be the second register
		if (operand1 != secondRegister) {
			continue;
		}

		pos++;

		// The first byte of the key should be != 0 Otherwise we assume it's too small
		if (pos[0] == 0) {
			continue;
		}

		// Next 4 bytes are the key
		int tempKey1 = pos[0] | (pos[1] << 8) | (pos[2] << 16) | (pos[3] << 24);

		pos += 4;

		// Canidates:
        // 00425BCB  |. 81F2 7F12EDA3  |XOR EDX,A3ED127F
		// 0041A523  |. 35 EDA7AB2E    |XOR EAX,2EABA7ED
		if (*pos == 0x81) {
			pos++;
			if (*pos < 0xf0 || *pos > 0xf7) {
				continue;
			}

			operand1 = *pos & 7;
		} else if (*pos == 0x35) {
			operand1 = 0;
		} else {
			continue; // No XOR
		}

		// The XOR operand is supposed to be the second register
		if (operand1 != thirdRegister) {
			continue;
		}

		pos++;

		// The first byte of the key should be != 0 Otherwise we assume it's too small
		if (pos[0] == 0) {
			continue;
		}

		// Next 4 bytes are the key
		int tempKey2 = pos[0] | (pos[1] << 8) | (pos[2] << 16) | (pos[3] << 24);

		pos += 4;

		// We expect some form of decrement here
		if (*pos < 0x48 || *pos > 0x4f) {
			continue;
		}

		operand1 = *pos - 0x48;

		// Should not be encountered until now
		if (operand1 == firstRegister || operand1 == secondRegister || operand1 == thirdRegister) {
			continue;
		}

		key1 = tempKey1;
		key2 = tempKey2;

		return true; // We reached a successfull pattern match
	}
    	
	return false;
}

int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Usage: %s <client.exe>\n", argv[0]);
		return -1;
	}

	// Treat argv[0] as the filename and slurp the file into memory
	size_t fileSize;
	char *fileData;
    
	FILE *fp = fopen(argv[1], "rb");

	if (!fp) {
		printf("Couldn't open %s for reading.\n", argv[1]);
	}

	readEntireFile(fp, fileData, fileSize);

	printf("Client Size: %u\n", fileSize);

	int key1, key2;
	if (searchClientKeys(fileData, fileSize, key1, key2)) {
		printf("Found client keys: 0x%x, 0x%x.\n", key1 - 1, key2);
	} else {
		printf("Unable to find encryption keys.\n");
	}

	free(fileData);

	return 0;
}
