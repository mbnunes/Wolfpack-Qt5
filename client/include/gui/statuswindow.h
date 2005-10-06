
#if !defined(__STATUSWINDOW_H__)
#define __STATUSWINDOW_H__

#include "gui/window.h"

class cMobile;

class cStatusWindow : public cWindow {
Q_OBJECT
Q_PROPERTY(cMobile* mobile READ mobile WRITE setMobile)
Q_PROPERTY(bool smallstatus READ smallStatus WRITE setSmallStatus)
protected:
	cMobile *mobile_;
	bool smallStatus_;
public:
	cStatusWindow();
	~cStatusWindow();

	void processDoubleClick();
	void processDefinitionAttribute(QString name, QString value);
public slots:
	cMobile *mobile() const;
	void setMobile(cMobile *mobile);
	bool smallStatus() const;
	void setSmallStatus(bool data);

protected slots:
	void mobileDestroyed();
	void statsUpdated();
	void statLocksUpdated();
};

Q_DECLARE_METATYPE(cStatusWindow*)

inline bool cStatusWindow::smallStatus() const {
	return smallStatus_;
}

inline void cStatusWindow::setSmallStatus(bool data) {
	smallStatus_ = data;
}

inline cMobile *cStatusWindow::mobile() const {
	return mobile_;
}

#endif
