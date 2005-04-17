
#if !defined(__UOSOCKET_H__)
#define __UOSOCKET_H__

#include <qstring.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qsocket.h>

#include "enums.h"
#include "network/uopacket.h"

class cStreamEncryption;
class QDns;

typedef cIncomingPacket *(*fnIncomingPacketConstructor)(QDataStream &data, unsigned short size);

class QSocket;

class cUoSocket : public QObject {
Q_OBJECT

protected:
	// TODO: !External event handlers!
	QSocket *socket;

	QByteArray pendingPacket;
	cStreamEncryption *encryption; // Encryption instance
	unsigned int seed; // Seed we sent to the server
	QString hostname; // The hostname we're connecting to
	bool gameServer; // Are we connecting to a gameserver?
	unsigned short hostport; // The port we're connecting to

	QValueList<QByteArray> outgoingQueue;
	QValueList<cIncomingPacket*> incomingQueue;
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

	// Poll for new packets and other events.
	void poll();

	// Queue a given byte array for sending it to the server
	void send(const QByteArray &data);

	// Checks if the socket is currently connected
	bool isConnected();
	bool isIdle();
	bool isConnecting();
	bool isClosing();
	bool isResolvingHost();
	bool isGameServer() const;

	// Event handler for errors
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

#define AREGPREFIX fnc
#define AUTO_REGISTER_PACKET(ID, CONSTRUCTOR) static cUoSocketAutoRegisterPacket AREGPREFIX ## ID (ID, CONSTRUCTOR);

#endif
