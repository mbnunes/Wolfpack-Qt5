
#include "profile.h"
#include "log.h"
#include "config.h"
#include "utilities.h"
#include "macros.h"

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
	clearProfile();
}

void cProfile::clearProfile() {
	foreach (cBaseAction *action, keyBindings_) {
		delete action;
	}
	keyBindings_.clear();
}

void cProfile::loadFromString(const QString &data) {
	QDomDocument document("profile");
	int errorLine, errorColumn;
	QString errorMessage;

	if (!document.setContent(data, false, &errorMessage, &errorLine, &errorColumn)) {
		Log->print(LOG_ERROR, tr("Error while loading profile data: %1 [Line %2, Column %3].\n").arg(errorMessage).arg(errorLine).arg(errorColumn));
		return;
	}

	clearProfile(); // Clear current profile

    QDomElement docElement = document.documentElement();

	if (docElement.hasAttribute("lastchange")) {
		lastChange_ = QDateTime::fromString(docElement.attribute("lastchange"), Qt::ISODate);
		if (!lastChange_.isValid()) {
			lastChange_ = QDateTime::currentDateTime();
		}
	}

	QDomElement element = docElement.firstChildElement();
	while (!element.isNull()) {
		if (element.nodeName() == "speechhue") {
			speechHue_ = Utilities::stringToUInt(element.text());
		} else if (element.nodeName() == "emotehue") {
			emoteHue_ = Utilities::stringToUInt(element.text());
		} else if (element.nodeName() == "keybindings") {
			// Process subnodes of this node as keys
			loadKeyBindings(element);
		}

		element = element.nextSiblingElement();
	}
}

void cProfile::loadFromFile(QString filename, bool dontPrependPath) {
	currentFilename_ = filename; // Save the filename for later use

	QFile file;

	if (!dontPrependPath) {
		QDir path(QDir::current().absolutePath() + Config->profilePath());
	
		if (!QDir().exists(path.absolutePath())) {
			QDir().mkpath(path.absolutePath());
		}

		file.setFileName(path.absoluteFilePath(filename));
	} else {
		file.setFileName(filename);
	}

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

void cProfile::saveToFile(const QString &filename, bool dontPrependPath) {
	QFile file;

	if (!dontPrependPath) {
		QDir path(QDir::current().absolutePath() + Config->profilePath());
	
		if (!QDir().exists(path.absolutePath())) {
			QDir().mkpath(path.absolutePath());
		}

		file.setFileName(path.absoluteFilePath(filename));
	} else {
		file.setFileName(filename);
	}

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
	attr.setValue(lastChange_.toString(Qt::ISODate));
	root.setAttributeNode(attr);

	QDomElement element;
	QDomText text;

	// Shortcut macro for creating an element with a text subnode
#define SAVE_TEXT(name, value) element = document.createElement(name);\
	text = document.createTextNode((value)); \
	element.appendChild(text); \
	root.appendChild(element);

	// Save speech hue
	SAVE_TEXT("speechhue", QString::number(speechHue_));
	SAVE_TEXT("emotehue", QString::number(emoteHue_));

	root.appendChild(saveKeyBindings(document));

	return document.toString();
}

bool cProfile::processShortcut(const QKeySequence &sequence) {
	foreach (cBaseAction *action, keyBindings_) {
		if (action->sequence() == sequence) {
			action->perform();
			return true;
		}
	}

	return false;
}

void cProfile::loadKeyBindings(QDomElement &element) {
	QDomElement child = element.firstChildElement("key");
	while (!child.isNull()) {
		bool altModifier = Utilities::stringToBool(child.attribute("alt", "false"));
		bool shiftModifier = Utilities::stringToBool(child.attribute("shift", "false"));
		bool ctrlModifier = Utilities::stringToBool(child.attribute("ctrl", "false"));
		bool metaModifier = Utilities::stringToBool(child.attribute("meta", "false"));
		uint key = Utilities::stringToUInt(child.attribute("id"));

		if (altModifier) {
			key |= Qt::ALT;
		}
		if (shiftModifier) {
			key |= Qt::SHIFT;
		}
		if (ctrlModifier) {
			key |= Qt::CTRL;
		}
		if (metaModifier) {
			key |= Qt::META;
		}

		QDomElement subchild = child.firstChildElement();
		while (!subchild.isNull()) {
			cBaseAction *action = 0;
			if (subchild.nodeName() == "core") {
				action = new cCoreAction;
			} else if (subchild.nodeName() == "macro") {
				action = new cCoreAction;
			} else {
				subchild = subchild.nextSiblingElement();
				continue;
			}
			action->setSequence(QKeySequence(key));
			action->load(subchild);
			keyBindings_.append(action);
			break;
		}

		child = child.nextSiblingElement("key");
	}
}

QDomElement cProfile::saveKeyBindings(QDomDocument &document) {
	QDomElement root = document.createElement("keybindings");

	foreach (cBaseAction *action, keyBindings_) {
		QDomElement element = document.createElement("key");
		uint key = action->sequence()[0];
		if (key & Qt::ALT) {
			key &= ~ Qt::ALT;
			element.setAttribute("alt", "true");
		}

		if (key & Qt::SHIFT) {
			key &= ~ Qt::SHIFT;
			element.setAttribute("shift", "true");
		}

		if (key & Qt::CTRL) {
			key &= ~ Qt::CTRL;
			element.setAttribute("ctrl", "true");
		}

		if (key & Qt::META) {
			key &= ~ Qt::META;
			element.setAttribute("meta", "true");
		}

		element.setAttribute("id", QString::number(key));

		element.appendChild(action->save(document));
		root.appendChild(element);
	}

	return root;
}

cProfile *Profile = 0;
