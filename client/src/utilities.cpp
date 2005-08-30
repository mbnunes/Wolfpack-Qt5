
#include "utilities.h"
#include "config.h"
#include "uoclient.h"
#include <math.h>
#include <qlocale.h>
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
		QMessageBox box(caption, message, ( error ? QMessageBox::Critical : QMessageBox::Information ), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, qApp->mainWidget());
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

	QString localLanguage() {
		QLocale locale = QLocale::system();
		
		// This switch will translate to the correct locale
		switch (locale.language()) {
			case QLocale::German:
				return "deu";
			case QLocale::French:
				return "fra";
			case QLocale::Chinese:
				return "chs";
			case QLocale::Spanish:
				return "spa";
			case QLocale::Japanese:
				return "jpn";
			case QLocale::Korean:
				return "kor";
			default:
				return "enu";
		}
	}

	ushort getMountBody(ushort id) {
		switch (id) {
			case 0x3ea0:
				return 0xe2;

			case 0x3ea1:
				return 0xe4;

			case 0x3ea2:
				return 0xcc;

			case 0x3ea3:
				return 0xd2;

			case 0x3ea4:
				return 0xda;

			case 0x3ea5:
				return 0xdb;

			case 0x3ea6:
				return 0xdc;

			case 0x3ea7:
				return 0x74;

			case 0x3ea8:
				return 0x75;

			case 0x3ea9:
				return 0x72;

			case 0x3eaa:
				return 0x73;

			case 0x3eab:
				return 0xaa;

			case 0x3eac:
				return 0xab;

			case 0x3ead:
				return 0x84;

			case 0x3eb4:
				return 0x7a;

			case 0x3eaf:
				return 0x78;

			case 0x3eb0:
				return 0x79;

			case 0x3eb1:
				return 0x77;

			case 0x3eb2:
				return 0x76;

			case 0x3eb3:
				return 0x90;

			case 0x3eb5:
				return 0xb1;

			case 0x3eb6:
				return 0xb2;

			case 0x3eb7:
				return 0xb3;

			case 0x3eb8:
				return 0xbc;

			case 0x3eba:
				return 0xbb;

			case 0x3ebb:
				return 0x319;

			case 0x3ebc:
				return 0x317;

			case 0x3ebd:
				return 0x31a;

			case 0x3ebe:
				return 0x31f;

			case 0x3e9e:
				return 0xbe;

			case 0x3e9c:
				return 0xbf;

			case 0x3e9b:
			case 0x3e9d:
				return 0xc0;

			case 0x3e9a:
				return 0xc1;

			case 0x3e98:
				return 0xc2;

			case 0x3e97:
				return 0xc3;

			case 0x3e94:
				return 0xf3;

			case 0x3e95:
				return 0xa9;

			case 0x3e90:
				return 0x114;

			case 0x3e91:
				return 0x115;

			case 0x3e92:
				return 0x11c;

			default:
				return 0xc8;
		}
	}
};

MTRand *Random = 0; // global rnd object
