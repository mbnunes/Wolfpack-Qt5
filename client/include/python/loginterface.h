
#if !defined(__LOGINTERFACE_H__)
#define __LOGINTERFACE_H__

/*
	This file defines an interface to allow python to write to our logfile.
	This includes a replacement for sys.stdout and sys.stderr.
*/

#include <Python.h>

void initializeLogInterface();

#endif
