
#if !defined(__ARGPARSER_H__)
#define __ARGPARSER_H__

#include <Python.h>
#include <stdarg.h>

bool parseArguments(PyObject *args, const char *format, va_list *p_va, int compat);

#endif
