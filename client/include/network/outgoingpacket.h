
#if !defined(__OUTGOING_PACKET_H__)
#define __OUTGOING_PACKET_H__

#include <QByteArray>
#include <QDataStream>

class cOutgoingPacket {
protected:
	unsigned char m_packetId;
	QByteArray m_Data; // The underlying data.
	QDataStream m_Stream; // The underlying stream.

	void writeUtf8Terminated(const QString &text);
	void writeBigUnicodeTerminated(const QString &text);
	void writeFixedAscii(const QString &text, unsigned short length);
	void writeAscii(const QString &text);
	void fill(unsigned short size, unsigned char value);
	void writeDynamicSize();
public:
	cOutgoingPacket(const cOutgoingPacket &outgoing);
	bool isDynamicSize() const;

	cOutgoingPacket(unsigned char packetId);	
	cOutgoingPacket(unsigned char packetId, unsigned short size);

	unsigned int packetId() const;

	const QByteArray &data() const;

	// Return a dump version of this packet
	QString dump() const;
};

inline unsigned int cOutgoingPacket::packetId() const {
	return m_packetId;
}

inline const QByteArray &cOutgoingPacket::data() const {
	return m_Data;
}

#endif
