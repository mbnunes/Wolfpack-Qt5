
#include "game/targetrequest.h"
#include "log.h"
#include "network/outgoingpackets.h"
#include "network/uosocket.h"

cServerTargetRequest::cServerTargetRequest(unsigned int serial, unsigned char type, unsigned char cursor) {
	this->serial = serial;
	this->type = type;
	this->cursor = cursor;
}

bool cServerTargetRequest::isValidTarget(cEntity *selected) {
	if (selected->type() == STATIC || selected->type() == ITEM || selected->type() == MOBILE) {
		return true;
	}
	
	if (type == 0x01) {
		return selected->type() == GROUND;
	} else {
		return false;
	}
}

void cServerTargetRequest::target(cEntity *selected) {
	if (!isValidTarget(selected)) {
		cancel();
		return;
	}

	UoSocket->send(cTargetResponsePacket(serial, type, cursor, selected));
}

void cServerTargetRequest::cancel() {
	UoSocket->send(cTargetResponsePacket(serial, type, cursor, 0));
}
