
#if !defined(__UOSOCKET_H__)
#define __UOSOCKET_H__

#include <q3valuelist.h>
#include <qbytearray.h>

#include "enums.h"
#include "network/uopacket.h"

class cStreamEncryption;
class Q3Dns;
class cOutgoingPacket;
class Q3Socket;

typedef cIncomingPacket *(*fnIncomingPacketConstructor)(QDataStream &data, unsigned short size);

// Array with packet lengths.
extern const Q_UINT16 packetLengths[256];

class cUoSocket : public QObject {
Q_OBJECT

protected:
	// TODO: !External event handlers!
	Q3Socket *socket;

	QByteArray pendingPacket;
	cStreamEncryption *encryption; // Encryption instance
	unsigned int seed_; // Seed we sent to the server
	QString hostname; // The hostname we're connecting to
	bool gameServer; // Are we connecting to a gameserver?
	unsigned short hostport; // The port we're connecting to

	Q3ValueList<QByteArray> outgoingQueue;
	Q3ValueList<cIncomingPacket*> incomingQueue;
	QByteArray incomingBuffer;

	void buildPackets();

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
	void disconnect();
	void setSeed(unsigned int seed);
	unsigned int seed() const;

	// Poll for new packets and other events.
	void poll();

	// Queue a given byte array for sending it to the server
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
	void delayedCloseFinished();
	void readyRead();
	void bytesWritten(int nbytes);
	void error(int error);
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

#define AREGPREFIX fnc
#define AUTO_REGISTER_PACKET(ID, CONSTRUCTOR) static cUoSocketAutoRegisterPacket AREGPREFIX ## ID (ID, CONSTRUCTOR);

#endif
