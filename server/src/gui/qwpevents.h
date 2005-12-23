
#include <QEvent>
#include <QString>

class QWPConsoleSendEvent : public QEvent
{
	QString m;
public:
	QWPConsoleSendEvent( const QString& msg ) : QEvent( (QEvent::Type)(QEvent::User + 1) ), m( msg )
	{

	}

	QString message() const { return m;	}
};
