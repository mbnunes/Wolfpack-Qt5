
#if !defined(__LOG_H__)
#define __LOG_H__

#include <QString>
class QFile;

// Log Levels
enum eLogLevel
{
	LOG_MESSAGE		= 0x01,
	LOG_ERROR		= 0x02,
	// LOG_PYTHON		= 0x04, - UNUSED HERE
	LOG_WARNING		= 0x08,
	LOG_NOTICE		= 0x10,
	LOG_TRACE		= 0x20,
	LOG_DEBUG		= 0x40,
	LOG_SPEECH		= 0x80,
	LOG_ALL			= 0xFF
};

class cLog
{
private:
	QFile *logfile;
	bool checkLogFile();
	unsigned char currentday; // Day of the month our current logfile is for

public:
	cLog();
	~cLog();

	/*
		Print a line.
	*/
	void print( const QString &text, bool timestamp = true ) {
		print(LOG_MESSAGE, text, timestamp);
	}

	void print( eLogLevel, const QString&, bool timestamp = true );
};

extern cLog *Log;

#define DEBUG_LOG( value ) Log->print( LOG_DEBUG, QString( "%1 (%2:%3)" ).arg( value ).arg( __FILE__ ).arg( __LINE__ ) );

#endif
