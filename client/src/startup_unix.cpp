
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qfileinfo.h>
#include <stdio.h>

#include "uoclient.h"

/*
	Unix Entry Point
*/

int main(int argc, char **argv) {
	QStringList args;
	
	for (int i = 0; i < argc; ++i) {
		args.append(argv[i]);
	}
	
	Client = new cUoClient; // Initialize UoClient
	Client->run(args); // Run UoClient
	delete Client; // Free UoClient Instance

	return 0;
}
