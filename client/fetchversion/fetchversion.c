
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

// The pattern to look for. This rounded to the next 4 byte boundary is the offset to the version info
char *pattern = (char*)L"VS_VERSION_INFO";
size_t patternLength = 30;

// This represents the real version information
struct VS_FIXEDFILEINFO { 
  unsigned int dwSignature; 
  unsigned int dwStrucVersion; 
  unsigned int dwFileVersionMS; 
  unsigned int dwFileVersionLS; 
  unsigned int dwProductVersionMS; 
  unsigned int dwProductVersionLS; 
  unsigned int dwFileFlagsMask; 
  unsigned int dwFileFlags; 
  unsigned int dwFileOS; 
  unsigned int dwFileType; 
  unsigned int dwFileSubtype; 
  unsigned int dwFileDateMS; 
  unsigned int dwFileDateLS;
};

void processMemory(char *memory, size_t size) {
	// Scan trough the memory in search of the pattern (which is unicode by the way)
	// We look backwards here.
	size_t offset = size - (34 + sizeof(struct VS_FIXEDFILEINFO));
	struct VS_FIXEDFILEINFO *fileinfo;

	while (offset >= 0) {
		if (!memcmp(memory + offset, pattern, patternLength)) {
			// Use the offset and add 32. 
			fileinfo = (struct VS_FIXEDFILEINFO*)(memory + offset + 34);

			// Check the signature
			// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/resources/versioninformation/versioninformationreference/versioninformationstructures/vs_fixedfileinfo.asp
			// for details
			if (fileinfo->dwSignature == 0xFEEF04BD) {
				printf("%d", fileinfo->dwFileVersionLS & 0xFFFF);
				return;
			}
		}

		--offset;
	}
}

void grabVersion(char *filename) {
	FILE *fp; // File pointer
	char *data; // file data
	size_t dataSize = 0; // file size
	size_t readSize; // how much was read into the buffer
	char buffer[4096]; // read buffer
	struct stat fileStats; // file statistics

	// Open the file
	fp = fopen(filename, "rb");

	if (!fp) {
		fprintf(stderr, "Unable to open file: %s.\n", filename);
		return;
	}

	// Retrieve file stats
	fstat(fp, &fileStats);

	data = malloc(fileStats.st_size); // Allocate (?)

	// Slurp the file into a character array
	while ((readSize = fread(&buffer, 1, 4096, fp)) > 0) {
		// Append read data
		memcpy(data + dataSize, buffer, readSize);
		dataSize += readSize;
	}

	// Process memory
	processMemory(data, dataSize);

	free(data); // Free memory

	fclose(fp);
}

/*
	Open the filename passed as argument 1 and read to the end of it.
*/
int main(int argc, char **argv) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: fetchversion <filename>\n");
		return -1;
	}

	grabVersion(argv[1]);
	return 0;
}
