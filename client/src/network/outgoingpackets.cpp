
#include "network/outgoingpackets.h"

cLoginPacket::cLoginPacket(const QString &username, const QString &password) : cOutgoingPacket(0x80, 62) {
	writeFixedAscii(username, 30);
	writeFixedAscii(password, 30);
	m_Stream << (unsigned char)0;
}

cGameLoginPacket::cGameLoginPacket(unsigned int key, const QString &username, const QString &password) : cOutgoingPacket(0x91, 65) {
	m_Stream << key;
	writeFixedAscii(username, 30);
	writeFixedAscii(password, 30);	
}

cRequestRelayPacket::cRequestRelayPacket(unsigned short id) : cOutgoingPacket(0xa0, 3) {
	m_Stream << id;
}

cDeleteCharacter::cDeleteCharacter(unsigned int id) : cOutgoingPacket(0x83, 39) {
	fill(30, 0); // Password...
	m_Stream << id;
	fill(4, 0); // "client-ip"
}

cPlayMobilePacket::cPlayMobilePacket(unsigned char id) : cOutgoingPacket(0x5d, 73) {
	m_Stream << 0xedededed; // "pattern"
	fill(60, 0); // name+password are blank since unused
	fill(3, 0); // Unknown. Maybe just the integer slot
	m_Stream << id;
	fill(4, 0); // "client-ip" but absolutly unneccesary
}

cDoubleClickPacket::cDoubleClickPacket(unsigned int serial) : cOutgoingPacket(0x06, 5) {
	m_Stream << serial;
}

cResyncPacket::cResyncPacket() : cOutgoingPacket(0x22, 3) {
	fill(2, 0);
}
