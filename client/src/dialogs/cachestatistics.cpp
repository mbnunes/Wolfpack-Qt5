
#include "dialogs/cachestatistics.h"
#include "qtextbrowser.h"
#include <QVBoxLayout>
#include <QTimer>

#include "muls/gumpart.h"
#include "muls/art.h"
#include "muls/animations.h"
#include "network/uosocket.h"

cCacheStatistics::cCacheStatistics(QWidget *parent) : QFrame(parent, Qt::Window) {
	resize(400, 250);
	setWindowTitle(tr("Cache Statistics"));

	QBoxLayout *layout = new QVBoxLayout(this);

	browser = new QTextBrowser();
	browser->setAlignment(Qt::AlignCenter);
	browser->setFrameShape(QFrame::NoFrame);
	layout->addWidget(browser);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
	timer->setSingleShot(false);
	timer->start(2500);
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
	message += tr("Memory used (estimated): %1 KiB<br>").arg(Animations->totalSequenceSize() / 1024.0f);

	message += "<br>";

	message += tr("<b>Network Statistics:</b><br>");
	message += tr("Data sent: %1 KiB<br>").arg(UoSocket->outgoinguchars() / 1024.0f);
	message += tr("Data received (compressed): %1 KiB<br>").arg(UoSocket->incomingucharsCompressed() / 1024.0f);
	message += tr("Data received: %1 KiB<br>").arg(UoSocket->incominguchars() / 1024.0f);

	int ratio = 100;
	if (UoSocket->incominguchars() > 0 && UoSocket->incomingucharsCompressed() > 0) {
		ratio = (int)(((float)UoSocket->incomingucharsCompressed() / (float)UoSocket->incominguchars()) * 100.0);
	}
	message += tr("Compression ratio: %1%<br>").arg(ratio);


	browser->setHtml(message);
}
