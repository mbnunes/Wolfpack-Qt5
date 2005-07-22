
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
