
#if !defined(__EXCEPTIONS_H__)
#define __EXCEPTIONS_H__

#include <qstring.h>

// Basic Exceptions
class Exception {
private:
	QString message_;
public:
	Exception(QString message) { message_ = message; }
	const QString &message() const { return message_; };
};

class SilentException {
};

#endif
