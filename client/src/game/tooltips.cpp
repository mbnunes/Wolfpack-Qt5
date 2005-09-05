
#include "game/tooltips.h"
#include "network/outgoingpackets.h"
#include "network/uosocket.h"

cTooltipInfo::cTooltipInfo(uint key, QVector<Line> lines) {
	key_ = key;
	lines_ = lines;
}

cTooltipInfo::~cTooltipInfo() {
}

cTooltips::cTooltips() {
}

cTooltips::~cTooltips() {
}

void cTooltips::clear() {
	foreach (cTooltipInfo *info, tooltips) {
		delete info;
	}
	tooltips.clear();
}

cTooltipInfo *cTooltips::get(uint key) const {
	Container::const_iterator it = tooltips.find(key);
	if (it != tooltips.end()) {
		return it.value();
	} else {
		return 0;
	}
}

void cTooltips::add(cTooltipInfo *info) {
	requested.remove(info->key()); // Make sure that we no longer think the tooltip is requested

	Iterator it = tooltips.find(info->key());
	if (it != tooltips.end()) {
		delete it.value();
		tooltips.erase(it);
	}

	tooltips.insert(info->key(), info);
}

void cTooltips::remove(uint key) {
	Iterator it = tooltips.find(key);
	if (it != tooltips.end()) {
		delete it.value();
		tooltips.erase(it);
	}
}

bool cTooltips::contains(uint key) const {
	Container::const_iterator it = tooltips.find(key);
	if (it != tooltips.end()) {
		return true;
	} else {
		return false;
	}
}

void cTooltips::request(uint serial, uint key) {
	// Check if the tooltip was already requested but we received no answer
	QMap<uint, QDateTime>::iterator it = requested.find(key);
	if (it != requested.end()) {
		QDateTime requestTime = it.value();

		// If the tooltip was requested but we didn't receive an answer for 15 seconds, request it again
		if (requestTime.secsTo(QDateTime::currentDateTime()) < 30) {
			requested.erase(it);
		} else {
			return; // Don't request it again
		}
	}

	// Queue the tooltip to be sent (option? since it's a new feature since 4.0-something)
	requestQueue.append(serial);

	requested.insert(key, QDateTime::currentDateTime());
}

void cTooltips::processRequests() {
	if (!requestQueue.isEmpty()) {
		// Compile a list of tooltips we want to request
		UoSocket->send(cRequestMultipleTooltipsPacket(requestQueue));
		requestQueue.clear();
	}
}

cTooltips *Tooltips = 0;
