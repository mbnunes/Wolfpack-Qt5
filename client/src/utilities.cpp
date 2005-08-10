
#include "utilities.h"
#include "config.h"
#include "uoclient.h"
#include <math.h>
#include <qmessagebox.h>

namespace Utilities {
	QString getUoFilename(const QString &filename) {
		QString basePath = Config->uoPath();

		// Translate Filename
		basePath.append(Config->getString("Filenames", filename.lower(), filename.lower()));

		return basePath;
	}

	void messageBox(QString message, QString caption, bool error) {
		//MessageBox(0, message.latin1(), caption.latin1(), MB_OK | (error ? MB_ICONERROR : MB_ICONINFORMATION));
		QMessageBox box(caption, message, ( error ? QMessageBox::Critical : QMessageBox::Information ), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		box.setModal(true);
		box.show();
	}

	QString dumpData(const QByteArray &data) {
		Q_INT32 length = data.size();
		QString dumped = QString( "\n[ packet: %1; length: %2 ]\n" ).arg( ( Q_UINT8 ) data[0], 2, 16 ).arg( data.count() );

		int lines = length / 16;
		if ( length % 16 ) // always round up.
			lines++;

		for ( int actLine = 0; actLine < lines; ++actLine )
		{
			QString line; //= QString("%1: ").arg(actLine*16, 4, 16); // Faster, but doesn't look so good
			line.sprintf( "%04x: ", actLine * 16 );
			int actRow = 0;
			for ( ; actRow < 16; ++actRow )
			{
				if ( actLine * 16 + actRow < length )
				{
					QString number = QString::number( static_cast<uint>( static_cast<Q_UINT8>( data[actLine*16 + actRow] ) ), 16 ) + QString( " " );
					//line += QString().sprintf( "%02x ", (unsigned int)((unsigned char)data[actLine * 16 + actRow]) );
					if ( number.length() < 3 )
						number.prepend( "0" );
					line += number;
				}
				else
					line += "-- ";
			}

			line += ": ";

			for ( actRow = 0; actRow < 16; ++actRow )
			{
				if ( actLine * 16 + actRow < length )
					line += ( isprint( static_cast<Q_UINT8>( data[actLine * 16 + actRow] ) ) ) ? data[actLine * 16 + actRow] : '.' ;
			}

			line += "\n";
			dumped += line;
		}
		return dumped;
	}
};

MTRand *Random = 0; // global rnd object
