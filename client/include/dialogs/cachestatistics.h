
#if !defined(__CACHESTATISTICS_H__)
#define __CACHESTATISTICS_H__

#include <qframe.h>

class QTextBrowser;

class cCacheStatistics : public QFrame {
Q_OBJECT
public:
	cCacheStatistics(QWidget *parent);
protected:
	QTextBrowser *browser;
public slots:
	void refresh();
};

#endif
