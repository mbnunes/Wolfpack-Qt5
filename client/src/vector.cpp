
#include "vector.h"
#include <qstring.h>

QString cVector::toString() {
	return QString("(%1, %2, %3)").arg(x).arg(y).arg(z);
}
