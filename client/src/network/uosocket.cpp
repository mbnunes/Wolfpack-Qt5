
#include <qdns.h>

#include "uoclient.h"
#include "network/encryption.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "log.h"

//#include <winsock2.h>

// Declare the variable
fnIncomingPacketConstructor cUoSocket::incomingPacketConstructors[256] = {0, };

// Packet Lengths
const Q_UINT16 packetLengths[256] = {
	0x0068, 0x0005, 0x0007, 0x0000, 0x0002, 0x0005, 0x0005, 0x0007, // 0x00
	0x000e, 0x0005, 0x000b, 0x010a, 0x0000, 0x0003, 0x0000, 0x003d, // 0x08
	0x00d7, 0x0000, 0x0000, 0x000a, 0x0006, 0x0009, 0x0001, 0x0000, // 0x10
	0x0000, 0x0000, 0x0000, 0x0025, 0x0000, 0x0005, 0x0004, 0x0008, // 0x18
	0x0013, 0x0008, 0x0003, 0x001a, 0x0007, 0x0014, 0x0005, 0x0002, // 0x20
	0x0005, 0x0001, 0x0005, 0x0002, 0x0002, 0x0011, 0x000f, 0x000a, // 0x28
	0x0005, 0x0001, 0x0002, 0x0002, 0x000a, 0x028d, 0x0000, 0x0008, // 0x30
	0x0007, 0x0009, 0x0000, 0x0000, 0x0000, 0x0002, 0x0025, 0x0000, // 0x38
	0x00c9, 0x0000, 0x0000, 0x0229, 0x02c9, 0x0005, 0x0000, 0x000b, // 0x40
	0x0049, 0x005d, 0x0005, 0x0009, 0x0000, 0x0000, 0x0006, 0x0002, // 0x48
	0x0000, 0x0000, 0x0000, 0x0002, 0x000c, 0x0001, 0x000b, 0x006e, // 0x50
	0x006a, 0x0000, 0x0000, 0x0004, 0x0002, 0x0049, 0x0000, 0x0031, // 0x58
	0x0005, 0x0009, 0x000f, 0x000d, 0x0001, 0x0004, 0x0000, 0x0015, // 0x60
	0x0000, 0x0000, 0x0003, 0x0009, 0x0013, 0x0003, 0x000e, 0x0000, // 0x68
	0x001c, 0x0000, 0x0005, 0x0002, 0x0000, 0x0023, 0x0010, 0x0011, // 0x70
	0x0000, 0x0009, 0x0000, 0x0002, 0x0000, 0x000d, 0x0002, 0x0000, // 0x78
	0x003e, 0x0000, 0x0002, 0x0027, 0x0045, 0x0002, 0x0000, 0x0000, // 0x80
	0x0042, 0x0000, 0x0000, 0x0000, 0x000b, 0x0000, 0x0000, 0x0000, // 0x88
	0x0013, 0x0041, 0x0000, 0x0063, 0x0000, 0x0009, 0x0000, 0x0002, // 0x90
	0x0000, 0x001a, 0x0000, 0x0102, 0x0135, 0x0033, 0x0000, 0x0000, // 0x98
	0x0003, 0x0009, 0x0009, 0x0009, 0x0095, 0x0000, 0x0000, 0x0004, // 0xA0
	0x0000, 0x0000, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x000d, // 0xA8
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0040, 0x0009, 0x0000, // 0xB0
	0x0000, 0x0003, 0x0006, 0x0009, 0x0003, 0x0000, 0x0000, 0x0000, // 0xB8
	0x0024, 0x0000, 0x0000, 0x0000, 0x0006, 0x00cb, 0x0001, 0x0031, // 0xC0
	0x0002, 0x0006, 0x0006, 0x0007, 0x0000, 0x0001, 0x0000, 0x004e, // 0xC8
	0x0000, 0x0002, 0x0019, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xD0
	0x0000, 0x010C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, // 0xD8
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, // 0xE0
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, // 0xE8
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, // 0xF0
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, // 0xF8
};

cUoSocket *UoSocket = 0; // Global cUoSocket instance

cUoSocket::cUoSocket() {
	encryption = 0;
	socket = new QSocket(this);

	// Connect the QSocket slots to this class
	QObject::connect(socket, SIGNAL(hostFound()), this, SLOT(hostFound()));
	QObject::connect(socket, SIGNAL(connected()), this, SLOT(connected()));
	QObject::connect(socket, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()));
	QObject::connect(socket, SIGNAL(delayedCloseFinished()), this, SLOT(delayedCloseFinished()));
	QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	QObject::connect(socket, SIGNAL(bytesWritten(int)), this, SLOT(bytesWritten(int)));
	QObject::connect(socket, SIGNAL(error(int)), this, SLOT(error(int)));

	/*
	Static -> Already Registered
	for (int i = 0; i < 256; ++i) {
		incomingPacketConstructors[i] = 0;
	}*/
}

cUoSocket::~cUoSocket() {
	delete socket;
}

void cUoSocket::connect(const QString &host, unsigned short port, bool gameServer) {
	if (!isIdle()) {
		// not yet connected
		return;
	}

	/*if (isConnected()) {
		disconnect(); // Disconnect an existing connection
	}*/
	seed = Random->randInt(); // Set a new seed

	this->gameServer = gameServer;
	this->hostname = host;
	this->hostport = port;

	// Connect to the Socket
	socket->connectToHost(host, port);

	/*QValueList<QHostAddress> addresses = dns.addresses();

	if (addresses.size() == 0) {
		onError(tr("Invalid hostname: %1").arg(host));
		return false;
	}

	QHostAddress hostAddress = addresses[Random->randInt(addresses.size())];

	if (!socketDevice.connect(hostAddress, port) && socketDevice.error() != QSocketDevice::NoError) {
		Log->print(tr("Error: %1\n").arg(convertError(socketDevice.error())));
		return false;
	}

    QByteArray uoHeader(4);
	uoHeader[0] = (unsigned char)( (seed >> 24) & 0xff );
	uoHeader[1] = (unsigned char)( (seed >> 16) & 0xff );
	uoHeader[2] = (unsigned char)( (seed >> 8) & 0xff );
	uoHeader[3] = (unsigned char)( seed & 0xff );
	outgoingQueue.push_back(uoHeader); // Send the uo header in the next iteration*/
}

void cUoSocket::disconnect() {
	// Clear incoming and outgoing queue
	incomingQueue.clear();
	outgoingQueue.clear();
}

bool cUoSocket::isIdle() {
	return socket->state() == QSocket::Idle;
}
bool cUoSocket::isConnecting() {
	return socket->state() == QSocket::Connecting;
}
bool cUoSocket::isClosing() {
	return socket->state() == QSocket::Closing;
}
bool cUoSocket::isResolvingHost() {
	return socket->state() == QSocket::HostLookup;
}

bool cUoSocket::isConnected() {
	return socket->state() == QSocket::Connected;
}

void cUoSocket::poll() {
	QSocket::State state = socket->state();

	switch (state) {
		case QSocket::Idle:
			break; // Do nothing

		case QSocket::HostLookup:
			break; // Do nothing

		case QSocket::Connecting:
			break; // Do nothing

		// Process incoming and outgoing packets if the socket is connected
		case QSocket::Connected:
		{
			while (socket->isWritable() && !outgoingQueue.isEmpty()) {
				QByteArray data = outgoingQueue.first();
				socket->writeBlock(data.data(), data.size());
				outgoingQueue.pop_front();
			}
		}
		break;

		case QSocket::Closing:
			break; // Do nothing

		default:
			break; // Do nothing
	}
}

void cUoSocket::send(const QByteArray &data) {
	QByteArray data2 = data.copy();
	
	if (encryption) {
		encryption->encryptOutgoing(data2.data(), data2.size());
	}

	outgoingQueue.push_back(data2);
}

void cUoSocket::buildPackets() {
	// Process the incoming buffer and split it into packets
	while (incomingBuffer.size() != 0) {
		unsigned char packetId = incomingBuffer[0];
		unsigned short size = packetLengths[packetId];

		if (size == 0xFFFF) {
			throw Exception(tr("Received unknown packet from server: %1\n").arg(packetId));
		} else if (size == 0 && incomingBuffer.size() >= 3) {
            unsigned short dynamicSize = (incomingBuffer[1] << 8) | incomingBuffer[2];
			if (dynamicSize <= incomingBuffer.size()) {				
				QByteArray packetData(dynamicSize);
				memcpy(packetData.data(), incomingBuffer.data(), dynamicSize);
				memcpy(incomingBuffer.data(), incomingBuffer.data() + dynamicSize, incomingBuffer.size() - dynamicSize);
				incomingBuffer.resize(incomingBuffer.size() - dynamicSize);
				
				QDataStream inputData(packetData, IO_ReadOnly);
				inputData.setByteOrder(QDataStream::BigEndian);
				if (incomingPacketConstructors[packetId]) {
					cIncomingPacket *packet = incomingPacketConstructors[packetId](inputData, dynamicSize);
					if (packet) {
						incomingQueue.append(packet);
					}
				}
			}
		} else if (size <= incomingBuffer.size()) {
			// Completed a packet
			QByteArray packetData(size);
			memcpy(packetData.data(), incomingBuffer.data(), size);
			memcpy(incomingBuffer.data(), incomingBuffer.data() + size, incomingBuffer.size() - size);
			incomingBuffer.resize(incomingBuffer.size() - size);

			QDataStream inputData(packetData, IO_ReadOnly);
			inputData.setByteOrder(QDataStream::BigEndian);
			if (incomingPacketConstructors[packetId]) {
				cIncomingPacket *packet = incomingPacketConstructors[packetId](inputData, size);
				if (packet) {
					incomingQueue.append(packet);
				}
			}
		}

		break;
	}
}

void cUoSocket::hostFound() {
	emit onHostFound();
}

void cUoSocket::connected() {
	// Send the UO header we have to send for every connection type
    QByteArray uoHeader(4);
	uoHeader[0] = (unsigned char)( (seed >> 24) & 0xff );
	uoHeader[1] = (unsigned char)( (seed >> 16) & 0xff );
	uoHeader[2] = (unsigned char)( (seed >> 8) & 0xff );
	uoHeader[3] = (unsigned char)( seed & 0xff );
	outgoingQueue.push_back(uoHeader);

	// Trigger the signal
	emit onConnect();
}

void cUoSocket::connectionClosed() {
}

void cUoSocket::delayedCloseFinished() {
	emit onDisconnect();
}

void cUoSocket::readyRead() {
	QByteArray data = socket->readAll();
	size_t offset = incomingBuffer.size();
	incomingBuffer.resize(incomingBuffer.size() + data.size());
	memcpy(incomingBuffer.data() + offset, data.data(), data.size());

	buildPackets(); // Try rebuilding incoming packets

	while (!incomingQueue.isEmpty()) {
		cIncomingPacket *packet = incomingQueue.front();
		incomingQueue.pop_front();
		packet->handle(this);
		delete packet;
	}
}

void cUoSocket::bytesWritten(int nbytes) {
}

void cUoSocket::error(int error) {
	QString message;

	switch (error) {
		case QSocket::ErrConnectionRefused:
			message = tr("Connection Refused");
			break;

		case QSocket::ErrHostNotFound:
			message = tr("Host not found");
			break;

		case QSocket::ErrSocketRead:
			message = tr("Could not read from socket");
			break;

		default:
			message = tr("Unknown socket error.");
			break;
	};

	emit onError(message);
}

