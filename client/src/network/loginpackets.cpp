
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "log.h"
#include "enums.h"

#include <qvaluevector.h>

/*
	This packet is sent by the server is the login was accepted. Contains the list of possible
	gameservers.
*/
class cShardListPacket : public cDynamicIncomingPacket {
protected:
	QValueList<stShardEntry> shards;
	unsigned char listFlag;
public:
	cShardListPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(6); // At least 6 byte

		unsigned short count;

		// Get the data from the shardlist
		input >> listFlag >> count;

		// Assert that the packet is large enough
		safetyAssertSize(6 + count * 40); // 40 byte per server

		for (unsigned int i = 0; i < count; ++i) {
			unsigned int pingIp;
			stShardEntry shard;
			input >> shard.id;
			shard.name.resize(33);
			shard.name[32] = 0;
			input.readRawBytes(shard.name.data(), 32);
			input >> shard.percentFull >> shard.timezone >> pingIp;
			shard.pingAddress.setAddress(pingIp);
            shards.append(shard);
			
			//Log->print(LOG_MESSAGE, tr("Received shard entry #%1. Name: %2, Full %3, Timezone: %4, Ping Ip: %5.\n").arg(id).arg(serverName).arg(full).arg(timezone).arg(QHostAddress(pingIp).toString()));
		}
	}

	virtual void handle(cUoSocket *socket) {
		LoginDialog->clearShardList();

		QValueList<stShardEntry>::iterator it;
		for (it = shards.begin(); it != shards.end(); ++it) {
			LoginDialog->addShard(*it);
		}

		LoginDialog->show(PAGE_SHARDLIST);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cShardListPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0xa8, cShardListPacket::creator);

/*
	This packet is receieved when the login has been denied by the server.
*/
class cLoginDeniedPacket : public cIncomingPacket {
protected:
	unsigned char reason;
public:
	cLoginDeniedPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		safetyAssertSize(2); // Exactly 2 byte

		// Get the data from the shardlist
		input >> reason;
	}

	virtual void handle(cUoSocket *socket) {
		QString errorMessage;
		switch (reason) {
			case 0:
				errorMessage = tr("Unknown User");
				break;
			case 1:
				errorMessage = tr("Account Already in Use");
				break;
			case 2:
				errorMessage = tr("Account disabled");
				break;
			case 3:
				errorMessage = tr("Password bad");
				break;
			default:
				errorMessage = tr("Communication error");
				break;
		}

		LoginDialog->setStatusText(errorMessage);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cLoginDeniedPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x82, cLoginDeniedPacket::creator);

/*
	This packet is receieved when the server relays us to a gameserver.
*/
class cRelayToServer : public cIncomingPacket {
protected:
	unsigned int gameServerIp;
	unsigned short gameServerPort;
	unsigned int newCryptKey;

public:
	cRelayToServer(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		safetyAssertSize(11); // Exactly 11 byte

		// Get the data from the shardlist
		input >> gameServerIp >> gameServerPort >> newCryptKey;
	}

	virtual void handle(cUoSocket *socket) {
		LoginDialog->show(PAGE_CONNECTING);
		LoginDialog->setStatusText("Connecting");

		UoSocket->disconnect();
		UoSocket->connect(QHostAddress(gameServerIp).toString(), gameServerPort, true);
		UoSocket->setSeed(newCryptKey);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cRelayToServer(input, size);
	}
};

AUTO_REGISTER_PACKET(0x8c, cRelayToServer::creator);

/*
	This packet enables "locked" client features.
*/
class cEnableFeatures : public cIncomingPacket {
protected:
	unsigned short flags;
public:
	cEnableFeatures(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		safetyAssertSize(3); // Exactly 3 byte
		input >> flags;
	}

	virtual void handle(cUoSocket *socket) {
		// Send this to the log for now
		Log->print(LOG_DEBUG, tr("Server wants to enable client features: 0x%1.\n").arg(flags, 0, 16));
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cEnableFeatures(input, size);
	}
};

AUTO_REGISTER_PACKET(0xb9, cEnableFeatures::creator);

/*
	This packet is sent by the server is the login was accepted. Contains the list of possible
	gameservers.
*/
class cCharacterListPacket : public cDynamicIncomingPacket {
protected:
	QValueVector<QString> characters;
	QValueVector<stStartLocation> startLocations;
	unsigned int flags;
public:
	cCharacterListPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(9); // At least 9 byte

		unsigned char charCount, townCount;

		// Get the data from the shardlist
		input >> charCount;

		if (charCount < 5) {
			charCount = 5; // At least 5 characters
		}

		// Assert that the packet is large enough
		safetyAssertSize(9 + charCount * 60); // 60 byte per character

		for (unsigned int i = 0; i < charCount; ++i) {
            char name[61];
			name[60] = 0;
			input.readRawBytes(name, 60);

			// Only add if the name is non-null
			if (name[0]) {
				characters.append(name);
			}
		}

		// Read the number of towns
		input >> townCount;

		safetyAssertSize(9 + charCount * 60 + townCount * 63); // 63 byte per town

		for (unsigned int i = 0; i < townCount; ++i) {
            char name[32], exactName[32];
			name[31] = 0; exactName[31] = 0;
			stStartLocation location;
			
			input >> location.index;
			input.readRawBytes(name, 31);
			input.readRawBytes(exactName, 31);			
			location.name = name;
			location.exactName = exactName;
			
			startLocations.append(location);
		}

		input >> flags; // Read flags
	}

	virtual void handle(cUoSocket *socket) {
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cCharacterListPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0xa9, cCharacterListPacket::creator);
