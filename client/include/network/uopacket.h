
#if !defined(__UOPACKET_H__)
#define __UOPACKET_H__

#include <qdatastream.h>

#include "exceptions.h"
#include "utilities.h"

class cUoSocket;

/*
	Abstract base class for incoming packets.
*/
class cIncomingPacket {
protected:
	unsigned char id; // The packet id
	unsigned short size; // The packet size

	// These methods are only defined for debug builds
	void safetyAssertSize(unsigned short size);
	
	cIncomingPacket(); // This is only available to subclasses
public:
	cIncomingPacket(QDataStream &input, unsigned short size);
	virtual ~cIncomingPacket();
	virtual void handle(cUoSocket *socket) = 0;
};

#if defined(_DEBUG)
inline void cIncomingPacket::safetyAssertSize(unsigned short size) {
	if (this->size < size) {
		throw Exception(tr("Safety assertion for incoming packet 0x%1 size wasn't met.\nExpected: %2 byte. Received: %3 byte.").arg(id, 0, 16).arg(size).arg(this->size));
	}
}
#else
inline void cIncomingPacket::safetyAssertSize(unsigned short size) {
}
#endif

class cDynamicIncomingPacket : public cIncomingPacket {
public:
	cDynamicIncomingPacket(QDataStream &input, unsigned short size);
	virtual ~cDynamicIncomingPacket();
};

#endif
