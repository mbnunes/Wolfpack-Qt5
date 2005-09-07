
#if !defined(__UOSOCKET_H__)
#define __UOSOCKET_H__

#include <QList>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <QTcpSocket>
#include <QQueue>

#include "enums.h"
#include "network/uopacket.h"

class cStreamEncryption;
class cOutgoingPacket;

typedef cIncomingPacket *(*fnIncomingPacketConstructor)(QDataStream &data, unsigned short size);

// Array with packet lengths.
extern const ushort packetLengths[256];

class cUoSocket : public QObject {
Q_OBJECT

protected:
	QTcpSocket *socket;

	QByteArray pendingPacket;
	cStreamEncryption *encryption; // Encryption instance
	unsigned int seed_; // Seed we sent to the server
	QString hostname; // The hostname we're connecting to
	bool gameServer; // Are we connecting to a gameserver?
	unsigned short hostport; // The port we're connecting to
	unsigned int lastDecodedPacketId_;
	uint moveSequence_;
	QQueue<int> moveQueue; // Not directly accessible from the outside

	QList<QByteArray> outgoingQueue;
	QList<cIncomingPacket*> incomingQueue;
	QByteArray incomingBuffer;

	void buildPackets();
	void logPacket(const QByteArray &data);
	void logPacket(cIncomingPacket *packet);
	QFile packetLog;
	QTextStream packetLogStream;
	uint outgoingBytes_;
	uint incomingBytes_;
	uint incomingBytesCompressed_;

	static fnIncomingPacketConstructor incomingPacketConstructors[256];
public:
	// Register a new packet.
	static void registerPacket(unsigned char id, fnIncomingPacketConstructor constructor) {
		incomingPacketConstructors[id] = constructor;
	}

	cUoSocket();
	virtual ~cUoSocket();

	// Initiate connection to a remote server
	void connect(const QString &host, unsigned short port, bool gameServer = false);	
	void setSeed(unsigned int seed);
	unsigned int seed() const;
	uint moveSequence() const;
	void setMoveSequence(uint data);
	void pushSequence(uint sequence);
	uint popSequence();
	void clearSequenceQueue();
	uint sequenceQueueLength();
	uint outgoinguchars();
	uint incominguchars();
	uint incomingucharsCompressed();

	// Queue a given uchar array for sending it to the server
	void sendRaw(const QByteArray &data);
	void send(const cOutgoingPacket &packet);

	// Checks if the socket is currently connected
	bool isConnected();
	bool isIdle();
	bool isConnecting();
	bool isClosing();
	bool isResolvingHost();
	bool isGameServer() const;

	// Event handler for errors

	// Game functionality
	void resync();
signals:
	void onError(const QString &error);
	void onHostFound();
	void onConnect();
	void onDisconnect();

public slots:
	// These will be connected to the QSocket signals
	void hostFound();
	void connected();
	void connectionClosed();
	void readyRead();
	void ucharsWritten(int nuchars);
	void error(QAbstractSocket::SocketError error);
	void poll();
	void disconnect();
	void sendPing();
};

// There is only one instance of the UoSocket class
extern cUoSocket *UoSocket;

// Utility class for auto registering a packet
class cUoSocketAutoRegisterPacket {
public:
	cUoSocketAutoRegisterPacket(unsigned char id, fnIncomingPacketConstructor constructor) {
		cUoSocket::registerPacket(id, constructor);
	}
};

inline bool cUoSocket::isGameServer() const {
	return gameServer;
}

inline unsigned int cUoSocket::seed() const {
	return seed_;
}

inline void cUoSocket::setSeed(unsigned int seed) {
	seed_ = seed;
}

inline uint cUoSocket::moveSequence() const {
	return moveSequence_;
}

inline void cUoSocket::setMoveSequence(uint data) {
	moveSequence_ = data;
}

inline void cUoSocket::pushSequence(uint sequence) {
	moveQueue.append(sequence);
}

inline uint cUoSocket::popSequence() {
	if (!moveQueue.isEmpty()) {
		return moveQueue.takeFirst();
	} else {
		return ~0;
	}
}

inline uint cUoSocket::outgoinguchars() {
	return outgoingBytes_;
}

inline uint cUoSocket::incominguchars() {
	return incomingBytes_;
}

inline uint cUoSocket::incomingucharsCompressed() {
	return incomingBytesCompressed_;
}

inline void cUoSocket::clearSequenceQueue() {
	moveQueue.clear();
}

inline uint cUoSocket::sequenceQueueLength() {
	return moveQueue.size();
}

#define AREGPREFIX fnc
#define AUTO_REGISTER_PACKET(ID, CONSTRUCTOR) static cUoSocketAutoRegisterPacket AREGPREFIX ## ID (ID, CONSTRUCTOR);

#endif
