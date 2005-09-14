
#include "macros.h"
#include "scripts.h"
#include "gui/worldview.h"
#include "game/mobile.h"
#include "mainwindow.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"

cBaseAction::cBaseAction() {
}

cBaseAction::~cBaseAction() {
}

QDomElement cCoreAction::save(QDomDocument &document) {
	QDomElement element = document.createElement("core");
	element.setAttribute("action", action_);
	return element;
}

void cCoreAction::load(QDomElement &element) {
	action_ = element.attribute("action");
}

void cCoreAction::perform() {
	perform(action_);
}

void cCoreAction::perform(const QString &action) {
	if (action == "togglewarmode") {
		toggleWarmode();
	} else if (action == "screenshot") {
		createScreenshot();
	} else {
		WorldView->addSysMessage(tr("Unknown macro action: %1.").arg(action));
	}
}

void cCoreAction::createScreenshot() {
	// Create a screenshot
	// Generate a nice filenamd and save the screenshot
	QDateTime current = QDateTime::currentDateTime();
	QString screenshotFilename = QString("screenshot-%1.png").arg(current.toString( "yyyyMMdd-hhmmss" ));
	GLWidget->createScreenshot(screenshotFilename);
}

void cCoreAction::toggleWarmode() {
	if (Player) {
		UoSocket->send(cWarmodeChangeRequest(!Player->isInWarmode()));
	}
}

QDomElement cMacroAction::save(QDomDocument &document) {
	QDomElement element = document.createElement("macro");
	QDomCDATASection section = document.createCDATASection(script_);
	element.appendChild(section);
	return element;
}

void cMacroAction::load(QDomElement &element) {
	// There should be a CDATA section there (hopefully?)
	QDomNode node = element.firstChild();
	while (!node.isNull()) {
		if (node.isCDATASection()) {
			script_ += node.toCDATASection().data();
		} else if (node.isText()) {
			script_ += node.toText().data();
		} else if (node.isCharacterData()) {
			script_ += node.toCharacterData().data();
		}
		node = node.nextSibling();
	}
}

void cMacroAction::perform() {

}
