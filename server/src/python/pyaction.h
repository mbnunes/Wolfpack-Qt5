
#if !defined(__PYACTION_H__)
#define __PYACTION_H__

#include "../action.h"
#include "engine.h"

class cActionPythonCode : public cAction {
protected:
	PyObject *code;
	PyObject *args;
public:
	cActionPythonCode(PyObject *code, PyObject *args);
	virtual ~cActionPythonCode();

	void execute();
};

#endif
