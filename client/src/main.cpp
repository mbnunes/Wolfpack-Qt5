
#include <windows.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qfileinfo.h>
#include <stdio.h>
#include <qapplication.h>

#include "uoclient.h"

/*!
	Entry Point.
	Please notice that the mainloop is inside cUoClient::run()
*/
int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	Client = new cUoClient; // Initialize UoClient
	Client->run(); // Run UoClient
	delete Client; // Free UoClient Instance
}
