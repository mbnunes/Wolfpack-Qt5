
#if !defined(__UOSOCKET_H__)
#define __UOSOCKET_H__

#include <QList>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <QTcpSocket>
#include <QQueue>
#include <QVector>

#include "enums.h"
#include "network/uopacket.h"

class cStreamEncryption;
class cOutgoingPacket;

// Forward declaration for python objects
typedef struct _object PyObject;

typedef cIncomingPacket *(*fnIncomingPacketConstructor)(QDataStream &data, unsigned short size);

// Array with packet lengths.
extern ushort packetLengths[256];

// Features enabled trough the Enable Feature packet
enum enFeatures {
	// 0xb9 Features
	FEATURE_T2A = 0x0001 | 0x0004, // T2A complete
	FEATURE_LBR = 0x0008 | 0x0002, // LBR + Sound
	FEATURE_AOS = 0x0010, // AOS

	// Characterlist Features
	FEATURE_CONTEXTMENUS = 0x08,
	FEATURE_TOOLTIPS = 0x20,
	FEATURE_AOSPROFESSIONS = 0x20,
	FEATURE_SEPROFESSIONS = 0x80,
	FEATURE_SIXTHCHARSLOT = 0x40,
	FEATURE_ONECHARSLOT = 0x14,
};

// This is a starting location
struct stStartLocation {
    QString name;
	QString exactName;
	unsigned char index;
};

class cUoSocket : public QObject {
Q_OBJECT
Q_PROPERTY(uint outgoingBytes READ outgoingBytes)
Q_PROPERTY(uint incomingBytes READ incomingBytes)
Q_PROPERTY(uint incomingBytesCompressed READ incomingBytesCompressed)
Q_PROPERTY(uint features READ features)
Q_PROPERTY(uint charlistFeatures READ charlistFeatures)
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
	QString account_;

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
	uint features_;
	uint charlistFeatures_;
	QVector<stStartLocation> startLocations_;

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
	uint outgoingBytes();
	uint incomingBytes();
	uint incomingBytesCompressed();
	uint features() const;
	void setFeatures(uint data);
	void setCharlistFeatures(uint data);
	uint charlistFeatures() const;
	const QString &account() const;
	void setAccount(QString account);
	void setStartLocations(const QVector<stStartLocation> &locations);

	// Is-Getters for flags
	bool isAos() const;
	bool isT2a() const;
	bool isLbr() const;
	bool isContextMenus() const;
	bool isTooltips() const;
	bool isAosProfessions() const;
	bool isSeProfessions();
	bool isSixthCharslot() const;
	bool isOneCharslot() const;

	// Queue a given byte array for sending it to the server
	void sendRaw(const QByteArray &data);
	void send(const cOutgoingPacket &packet);
	void send(cOutgoingPacket *packet);

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
	void error(QAbstractSocket::SocketError error);
	void poll();
	void disconnect();
	void sendPing();
	void setPacketLength(uchar packet, ushort size);
	PyObject *startLocations() const;

	// Several convenience methods
	void sendUnicodeSpeech(const QString &text, ushort hue = 0x3b2, ushort font = 0, uchar type = SPEECH_REGULAR);
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

inline uint cUoSocket::outgoingBytes() {
	return outgoingBytes_;
}

inline uint cUoSocket::incomingBytes() {
	return incomingBytes_;
}

inline uint cUoSocket::incomingBytesCompressed() {
	return incomingBytesCompressed_;
}

inline void cUoSocket::clearSequenceQueue() {
	moveQueue.clear();
}

inline uint cUoSocket::sequenceQueueLength() {
	return moveQueue.size();
}

inline uint cUoSocket::features() const {
	return features_;
}

inline void cUoSocket::setFeatures(uint data) {
	features_ = data;
}

inline bool cUoSocket::isAos() const {
	return (features_ & FEATURE_AOS) != 0;
}

inline bool cUoSocket::isT2a() const {
	return (features_ & FEATURE_T2A) != 0;
}

inline bool cUoSocket::isLbr() const {
	return (features_ & FEATURE_LBR) != 0;
}

inline void cUoSocket::setCharlistFeatures(uint data) {
	charlistFeatures_ = data;
}

inline uint cUoSocket::charlistFeatures() const {
	return charlistFeatures_;
}

inline bool cUoSocket::isContextMenus() const {
	return (charlistFeatures_ & FEATURE_CONTEXTMENUS) != 0;
}

inline bool cUoSocket::isTooltips() const {
	return (charlistFeatures_ & FEATURE_TOOLTIPS) != 0;
}

inline bool cUoSocket::isAosProfessions() const {
	return (charlistFeatures_ & FEATURE_AOSPROFESSIONS) != 0;
}

inline bool cUoSocket::isSeProfessions() {
	return (charlistFeatures_ & FEATURE_SEPROFESSIONS) != 0;
}

inline bool cUoSocket::isSixthCharslot() const {
	return (charlistFeatures_ & FEATURE_SIXTHCHARSLOT) != 0;
}

inline bool cUoSocket::isOneCharslot() const {
	return (charlistFeatures_ & FEATURE_ONECHARSLOT) != 0;
}

inline const QString &cUoSocket::account() const {
	return account_;
}

inline void cUoSocket::setAccount(QString account) {
	account_ = account;
}

inline void cUoSocket::setStartLocations(const QVector<stStartLocation> &locations) {
	startLocations_ = locations;
}

#define AREGPREFIX fnc
#define AUTO_REGISTER_PACKET(ID, CONSTRUCTOR) static cUoSocketAutoRegisterPacket AREGPREFIX ## ID (ID, CONSTRUCTOR);

#endif
