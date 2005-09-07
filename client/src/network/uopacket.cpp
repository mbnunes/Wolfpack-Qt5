
#include "network/uopacket.h"

/*
	Constructor for a basic incoming packet
*/
cIncomingPacket::cIncomingPacket(QDataStream &input, unsigned short size) {
	this->size = size;
	this->id = 0xFF; // Default to 0xFF
	safetyAssertSize(1); // Assume the packet has at least one uchar
	input >> id;
}

cIncomingPacket::cIncomingPacket() {
}

cIncomingPacket::~cIncomingPacket() {
}

/*
	Constructor for an incoming packet with a dynamic size
*/
cDynamicIncomingPacket::cDynamicIncomingPacket(QDataStream &input, unsigned short size) {
	this->size = size;
	this->id = 0xFF; // Default to 0xFF
	safetyAssertSize(3); // Assume a size of three
	input >> this->id;
	input >> this->size;
}

cDynamicIncomingPacket::~cDynamicIncomingPacket() {
}
