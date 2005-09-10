
#include "profile.h"
#include "log.h"
#include "config.h"

#include <QDir>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QTextCodec>

cProfile::cProfile() {
	speechHue_ = 0x3b2;
	emoteHue_ = 0x3b2;
	defaultFont_ = 0;
	lastChange_ = QDateTime::currentDateTime();
}

cProfile::~cProfile() {
}

void cProfile::loadFromString(const QString &data) {
	QDomDocument document("profile");
	int errorLine, errorColumn;
	QString errorMessage;

	if (!document.setContent(data, false, &errorMessage, &errorLine, &errorColumn)) {
		Log->print(LOG_ERROR, tr("Error while loading profile data: %1 [Line %2, Column %3].\n").arg(errorMessage).arg(errorLine).arg(errorColumn));
		return;
	}

    QDomElement docElement = document.documentElement();

	QDomElement element = docElement.firstChildElement();
	while (!element.isNull()) {
		element = element.nextSiblingElement();
	}
}

void cProfile::loadFromFile(QString filename) {
	currentFilename_ = filename; // Save the filename for later use

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		// Note that the profile gets auto-created
		//Log->print(LOG_ERROR, tr("Error while loading profile data from %1.\n").arg(filename));
		return;
	}

	Log->print(LOG_NOTICE, tr("Loading profile data from %1.\n").arg(filename));

	QTextStream input(&file);
	input.setAutoDetectUnicode(true);
	input.setCodec(QTextCodec::codecForName("UTF-8"));

	loadFromString(input.readAll());
    file.close();
}

void cProfile::saveToFile() {
	saveToFile(currentFilename_);
}

void cProfile::saveToFile(const QString &filename) {
	QDir path(QDir::current().absolutePath() + Config->profilePath());

	if (!QDir().exists(path.absolutePath())) {
		QDir().mkpath(path.absolutePath());
	}

	QFile file(path.absoluteFilePath(filename));

	if (!file.open(QIODevice::WriteOnly)) {
		Log->print(LOG_ERROR, QString("Unable to save profile data to %1.\n").arg(filename));
		return;
	}

	Log->print(LOG_NOTICE, tr("Saving profile data to %1.\n").arg(filename));

	// Open a text stream
	QTextStream textStream(&file);
	textStream.setAutoDetectUnicode(true);
	textStream.setCodec(QTextCodec::codecForName("UTF-8"));
	textStream << saveToString(); // Save the document
	file.close();
}

QString cProfile::saveToString() {
	QDomDocument document("profile");
	QDomElement root = document.createElement("profile");	
    document.appendChild(root);

	// Set the lastchange attribute of our root node
	QDomAttr attr = document.createAttribute("lastchange");
	attr.setValue(lastChange_.toString());
	root.setAttributeNode(attr);

	QDomElement element;
	QDomText text;

	// Save speech hue
	element = document.createElement("speechhue");
	text = document.createTextNode(QString::number(speechHue_));
    element.appendChild(text);
	root.appendChild(element);

	element = document.createElement("emotehue");
	text = document.createTextNode(QString::number(emoteHue_));
    element.appendChild(text);
	root.appendChild(element);

	return document.toString();
}

cProfile *Profile = 0;
