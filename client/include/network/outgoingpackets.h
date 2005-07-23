
#if !defined(__OUTGOINGPACKETS_H__)
#define __OUTGOINGPACKETS_H__

#include "network/outgoingpacket.h"

/*
	This file contains class definitions for outgoing packets.
*/

// 0x80: Attach to Loginserver
class cLoginPacket : public cOutgoingPacket {
public:
	cLoginPacket(const QString &username, const QString &password);
};

// 0x91 Attach to Gameserver
class cGameLoginPacket : public cOutgoingPacket {
public:
	cGameLoginPacket(unsigned int key, const QString &username, const QString &password);
};

// 0xa0 Request to be relayed to another server
class cRequestRelayPacket : public cOutgoingPacket {
public:
	cRequestRelayPacket(unsigned short id);
};

// 0x83 Request Character Deletion
class cDeleteCharacter : public cOutgoingPacket {
public:
	cDeleteCharacter(unsigned int id);
};

// 0x5d Play Character
class cPlayMobilePacket : public cOutgoingPacket {
public:
	cPlayMobilePacket(unsigned char id);
};

#endif
