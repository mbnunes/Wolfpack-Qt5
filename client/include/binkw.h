
#if !defined(__BINKW_H__)
#define __BINKW_H__

#include <qstring.h>

class cBinkwFile {
public:
	cBinkwFile(const QString &filename);
	virtual ~cBinkwFile();

	void play();
};

#endif
