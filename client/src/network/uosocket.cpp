
#include <qdns.h>

#include "uoclient.h"
#include "network/encryption.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "log.h"

#include <winsock2.h>

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

inline QString convertError(QSocketDevice::Error error) {
	switch (error) {
		case QSocketDevice::NoError:
			return "NoError";
		case QSocketDevice::AlreadyBound:
			return "AlreadyBound";
		case QSocketDevice::Inaccessible:
			return "Inaccessible";
		case QSocketDevice::NoResources:
			return "NoResources";
		case QSocketDevice::InternalError:
			return "InternalError";
		//case QSocketDevice::Bug:
		//	return "Bug";
		case QSocketDevice::Impossible:
			return "Impossible";
		case QSocketDevice::NoFiles:
			return "NoFiles";
		case QSocketDevice::ConnectionRefused:
			return "ConnectionRefused";
		case QSocketDevice::NetworkFailure:
			return "NetworkFailure";		
		case QSocketDevice::UnknownError:
		default:
			return "UnknownError";
	};
}

cUoSocket *UoSocket = 0; // Global cUoSocket instance

cUoSocket::cUoSocket() {
	socketState = SS_DISCONNECTED;
	socketDevice.setBlocking(false);
	socketDevice.setAddressReusable(true);
	encryption = 0;
	dns = new QDns;

	/*
	Static -> Already Registered
	for (int i = 0; i < 256; ++i) {
		incomingPacketConstructors[i] = 0;
	}*/
}

cUoSocket::~cUoSocket() {
	delete dns;
}

void cUoSocket::connect(const QString &host, unsigned short port, bool gameServer) {
	/*if (isConnected()) {
		disconnect(); // Disconnect an existing connection
	}*/
	seed = Random->randInt(); // Set a new seed

	this->gameServer = gameServer;
	this->hostname = host;
	this->hostport = port;

	dns->setRecordType();
	dns->setLabel(host);
	socketState = SS_DNSLOOKUP;
	
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
	socketDevice.close();

	// Clear incoming and outgoing queue
	incomingQueue.clear();
	outgoingQueue.clear();
}

bool cUoSocket::isConnected() {
	return socketDevice.isOpen(); 
}

void cUoSocket::poll() {
	switch (socketState) {
		case SS_DISCONNECTED:
			return;

		case SS_DNSLOOKUP:
			if (!dns->isWorking()) {
				// Get the list of addresses.
				QValueList<QHostAddress> addresses = dns->addresses();
				if (addresses.isEmpty()) {
					socketState = SS_DISCONNECTED;
					onError(tr("Unknown hostname: %1").arg(dns->label()));
					return;
				}

				// Get a random DNS A record.
				QHostAddress address = addresses[Random->randInt(addresses.size())];
				hostname = address;

				if (!socketDevice.connect(hostname, hostport)) {
					if (socketDevice.error() != QSocketDevice::NoError) {
						onError(convertError(socketDevice.error()));
						socketState = SS_DISCONNECTED;
						return;
					}
				}

				socketState = SS_CONNECTING;				
				onDnsLookupComplete(address, hostport); // Notify the event handler
			}
			return;

		case SS_CONNECTING:
			if (!socketDevice.connect(this->hostname, this->hostport)) {
				if (socketDevice.error() != QSocketDevice::NoError) {
					socketState = SS_DISCONNECTED;
					onError(convertError(socketDevice.error()));
				}
			} else {
				socketState = SS_CONNECTED;
				onConnect();
			}
			return;

		case SS_CONNECTED:
			{
				while (socketDevice.isWritable() && !outgoingQueue.isEmpty()) {
					QByteArray data = outgoingQueue.first();
					socketDevice.writeBlock(data.data(), data.size());
					outgoingQueue.pop_front();
				}

				Q_LONG avail = socketDevice.bytesAvailable();
				if (avail > 0) {
					Q_LONG offset = incomingBuffer.size();
					incomingBuffer.resize(incomingBuffer.size() + avail);
					socketDevice.readBlock(incomingBuffer.data() + offset, avail);
					Log->print(LOG_MESSAGE, tr("Read %1 bytes from the server.\n").arg(avail));
				}

				buildPackets(); // Try rebuilding incoming packets

				while (!incomingQueue.isEmpty()) {
					cIncomingPacket *packet = incomingQueue.front();
					incomingQueue.pop_front();
					packet->handle(this);
					delete packet;
				}
			}
			break;

		default:
			break;
	};
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

void cUoSocket::onConnect() {
    QByteArray uoHeader(4);
	uoHeader[0] = (unsigned char)( (seed >> 24) & 0xff );
	uoHeader[1] = (unsigned char)( (seed >> 16) & 0xff );
	uoHeader[2] = (unsigned char)( (seed >> 8) & 0xff );
	uoHeader[3] = (unsigned char)( seed & 0xff );
	outgoingQueue.push_back(uoHeader);

	if (!gameServer) {
		LoginDialog->onConnect();
	}
}

void cUoSocket::onDisconnect() {
}

void cUoSocket::onDnsLookupComplete(const QHostAddress &address, unsigned short port) {
	if (!gameServer) {
		LoginDialog->onDnsLookupComplete(address, port);
	}
}

void cUoSocket::onError(const QString &error) {
	if (!gameServer) {
		LoginDialog->onError(error);
	}
}
