
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "log.h"
#include "config.h"
#include "enums.h"

#include <QVector>
#include <QStringList>
#include <QList>

/*
	This packet is sent by the server is the login was accepted. Contains the list of possible
	gameservers.
*/
class cShardListPacket : public cDynamicIncomingPacket {
protected:
	QList<stShardEntry> shards;
	unsigned char listFlag;
public:
	cShardListPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(6); // At least 6 byte

		unsigned short count;

		// Get the data from the shardlist
		input >> listFlag >> count;

		// Assert that the packet is large enough
		safetyAssertSize(6 + count * 40); // 40 byte per server
		char shardname[33];
		shardname[32] = 0;

		for (unsigned int i = 0; i < count; ++i) {
			unsigned int pingIp;
			stShardEntry shard;
			input >> shard.id;			
			input.readRawData(shardname, 32);
			shard.name = QString::fromLatin1(shardname);
			input >> shard.percentFull >> shard.timezone >> pingIp;
			shard.pingAddress.setAddress(pingIp);
            shards.append(shard);
			
			//Log->print(LOG_MESSAGE, tr("Received shard entry #%1. Name: %2, Full %3, Timezone: %4, Ping Ip: %5.\n").arg(id).arg(serverName).arg(full).arg(timezone).arg(QHostAddress(pingIp).toString()));
		}
	}

	virtual void handle(cUoSocket *socket) {
		LoginDialog->clearShardList();

		QList<stShardEntry>::iterator it;
		for (it = shards.begin(); it != shards.end(); ++it) {
			LoginDialog->addShard(*it);
		}

		// If there is only one shard there, set Config->lastShardId() and call selectLastShard automatically
		if (shards.size() == 1) {
			Config->setLastShardId(shards[0].id);
			LoginDialog->selectLastShard();
		} else {
			LoginDialog->show(PAGE_SHARDLIST);
		}
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
				errorMessage = tr("Bad password");
				break;
			default:
				errorMessage = tr("Communication error");
				break;
		}

		LoginDialog->setStatusText(errorMessage);
		LoginDialog->setErrorStatus(true);
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
		UoSocket->setSeed(newCryptKey);
		UoSocket->connect(QHostAddress(gameServerIp).toString(), gameServerPort, true);		
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
		Log->print(LOG_NOTICE, tr("Server wants to enable client features: 0x%1.\n").arg(flags, 0, 16));
		socket->setFeatures(flags);
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
	QStringList characters;
	QVector<stStartLocation> startLocations;
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
			input.readRawData(name, 60);

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
			input.readRawData(name, 31);
			input.readRawData(exactName, 31);			
			location.name = name;
			location.exactName = exactName;
			
			startLocations.append(location);
		}

		input >> flags; // Read flags
	}

	virtual void handle(cUoSocket *socket) {
		socket->setCharlistFeatures(flags);
		LoginDialog->show(PAGE_SELECTCHAR);
		LoginDialog->setCharacterList(characters);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cCharacterListPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0xa9, cCharacterListPacket::creator);

class cCharacterListUpdatePacket : public cDynamicIncomingPacket {
protected:
	QStringList characters;
public:
	cCharacterListUpdatePacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(4); // At least 4 byte

		unsigned char charCount;

		// Get the data from the shardlist
		input >> charCount;

		// Assert that the packet is large enough
		safetyAssertSize(4 + charCount * 60); // 60 byte per character

		for (unsigned int i = 0; i < charCount; ++i) {
            char name[60];
			input.readRawData(name, 60);
			name[30] = 0; // ENSURE null termination

			// Only add if the name is non-null
			if (name[0]) {
				characters.append(name);
			}
		}
	}

	virtual void handle(cUoSocket *socket) {
		LoginDialog->show(PAGE_SELECTCHAR);
		LoginDialog->setCharacterList(characters);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cCharacterListUpdatePacket(input, size);
	}
};


AUTO_REGISTER_PACKET(0x86, cCharacterListUpdatePacket::creator);

/*
	This packet enables "locked" client features.
*/
class cIdleWarningPacket : public cIncomingPacket {
protected:
	uchar reason;
public:
	cIdleWarningPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> reason;
	}

	virtual void handle(cUoSocket *socket) {
		if (reason == 5) {
			LoginDialog->show(PAGE_ENTERING);
			LoginDialog->setErrorStatus(true);
			LoginDialog->setStatusText("There is already a character logged in.");
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cIdleWarningPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x53, cIdleWarningPacket::creator);
