
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "log.h"

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
