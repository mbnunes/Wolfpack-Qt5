
#include <windows.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qfileinfo.h>
#include <stdio.h>

#include "uoclient.h"

/*
	Windows Entry Point
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#if defined(_DEBUG)
	// Switch to the directory containing the excutable
	char exeName[MAX_PATH];
	GetModuleFileName(0, exeName, MAX_PATH);
	QString dirPath = QFileInfo(exeName).dirPath(true);
	SetCurrentDirectory(dirPath.latin1());
#endif

	QStringList args;
	
	/*
		Since Windows programs don't get passed the command name as the
		first argument, we need to fetch it explicitly.
	*/
	static char appFileName[MAX_PATH];
	GetModuleFileNameA(0, appFileName, MAX_PATH);
	args.append(appFileName);

	/*
		Parse the Windows command line string.  If an argument begins with a
		double quote, then spaces are considered part of the argument until the
		next double quote.  The argument terminates at the second quote. Note
		that this is different from the usual Unix semantics.
	*/
	char* p = lpCmdLine;
	char* p_end = p + strlen( p );
	QValueVector<char*> argPointers;

	while ( *p && p < p_end ) {
		while (isspace((unsigned char)*p))	// Skip Whitespaces
			p++;

		if (*p == '\0') // End of String
			break;

		if (*p == '"') {
			p++;
			argPointers.append(p);
			while ( ( *p != '\0' ) && ( *p != '"' ) )
				p++;
		} else {
			argPointers.append(p);
			while ( *p != '\0' && !isspace( ( uchar ) * p ) )
				p++;
		}

		if (*p != '\0') {
			*p = '\0';
			p++;
		}
	}

	QValueVector<char*>::iterator it;
	for (it = argPointers.begin(); it != argPointers.end(); ++it) {
		args.append(*it);
	}

	Client = new cUoClient; // Initialize UoClient
	Client->run(args); // Run UoClient
	delete Client; // Free UoClient Instance

	return 0;
}
