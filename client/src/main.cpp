
#include <QtCore>
#include <QPlastiqueStyle>

#include "uoclient.h"

/*!
	Entry Point.
	Please notice that the mainloop is inside cUoClient::run()
*/
int main( int argc, char** argv )
{    	
	QApplication app( argc, argv );	
	QApplication::setStyle(new QPlastiqueStyle);
	QStyle *style = QApplication::style();
	
	Client = new cUoClient; // Initialize UoClient
	Client->run(); // Run UoClient
	delete Client; // Free UoClient Instance
}
