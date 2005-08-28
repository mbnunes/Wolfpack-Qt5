
#include "dialogs/cachestatistics.h"
#include "qtextbrowser.h"
#include <QVBoxLayout>
#include <QTimer>

#include "muls/gumpart.h"
#include "muls/art.h"
#include "muls/animations.h"

cCacheStatistics::cCacheStatistics(QWidget *parent) : QFrame(parent, Qt::Window) {
	resize(400, 250);
	setWindowTitle(tr("Cache Statistics"));

	QBoxLayout *layout = new QVBoxLayout(this, 0, 0);

	browser = new QTextBrowser();
	browser->setAlignment(Qt::AlignCenter);
	browser->setFrameShape(QFrame::NoFrame);
	layout->addWidget(browser);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
	timer->start(1000, false);
}


void cCacheStatistics::refresh() {
	if (!isVisible()) {
		return;
	}

	QString message = tr("<b>Gumpart Statistics:</b><br>");
	message += tr("Textures currently in use: %1<br>").arg(Gumpart->cacheSize());
	
	message += "<br>";

	message += tr("<b>Art Statistics:</b><br>");
	message += tr("Textures currently in use: %1<br>").arg(Art->cacheSize());

	message += "<br>";

	message += tr("<b>Animation Statistics:</b><br>");
	message += tr("Sequences currently in use: %1<br>").arg(Animations->cacheSize());


	browser->setText(message);
}
