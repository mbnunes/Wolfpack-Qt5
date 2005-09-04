
#include "network/outgoingpackets.h"
#include "muls/speech.h"
#include "game/entity.h"
#include "game/mobile.h"
#include "game/statictile.h"
#include "game/groundtile.h"
#include "game/dynamicitem.h"
#include "log.h"

#include <qglobal.h>

cLoginPacket::cLoginPacket(const QString &username, const QString &password) : cOutgoingPacket(0x80, 62) {
	writeFixedAscii(username, 30);
	writeFixedAscii(password, 30);
	m_Stream << (unsigned char)0;
}

cGameLoginPacket::cGameLoginPacket(unsigned int key, const QString &username, const QString &password) : cOutgoingPacket(0x91, 65) {
	m_Stream << key;
	writeFixedAscii(username, 30);
	writeFixedAscii(password, 30);	
}

cRequestRelayPacket::cRequestRelayPacket(unsigned short id) : cOutgoingPacket(0xa0, 3) {
	m_Stream << id;
}

cDeleteCharacter::cDeleteCharacter(unsigned int id) : cOutgoingPacket(0x83, 39) {
	fill(30, 0); // Password...
	m_Stream << id;
	fill(4, 0); // "client-ip"
}

cPlayMobilePacket::cPlayMobilePacket(unsigned char id) : cOutgoingPacket(0x5d, 73) {
	m_Stream << 0xedededed; // "pattern"
	fill(60, 0); // name+password are blank since unused
	fill(3, 0); // Unknown. Maybe just the integer slot
	m_Stream << id;
	fill(4, 0); // "client-ip" but absolutly unneccesary
}

cDoubleClickPacket::cDoubleClickPacket(unsigned int serial) : cOutgoingPacket(0x06, 5) {
	m_Stream << serial;
}

cResyncPacket::cResyncPacket() : cOutgoingPacket(0x22, 3) {
	fill(2, 0);
}

unsigned int createOneMask(uint count) {
	uint result = 0;
	for (uint i = 0; i < count; ++i) {
		result |= 1 << i;
	}
	return result;
}

static void pushData(QByteArray &array, unsigned int &byteoffset, unsigned int &bitoffset, unsigned int data, unsigned char bit) {
    // We can at most save 8 - offset bit or bit bits
	while (bit > 0) {
		unsigned int bitCount = qMin<unsigned int>(8 - bitoffset, bit);
		unsigned int writeMask = (data >> (bit - bitCount)) & createOneMask(bitCount);
		
		data &= ~ (writeMask << (bit - bitCount)); // Clear out the data we write from the original data
		bit -= bitCount;

		array[byteoffset] = array[byteoffset] | (unsigned char)(writeMask << (8 - bitCount - bitoffset));
		bitoffset += bitCount;
		if (bitoffset > 7) {
			bitoffset = 0;
			++byteoffset;
		}
	}
}

cSendUnicodeSpeechPacket::cSendUnicodeSpeechPacket(enSpeechType type, const QString &message, unsigned short color, unsigned char font, const QString &language) : cOutgoingPacket(0xad, 14) {
	QVector<unsigned short> keywords = Speech->match(message); // Speech.mul keywords

	/*for (int i = 0; i < keywords.size(); ++i) {
		Log->print(LOG_MESSAGE, QString("Text '%1' contains Keyword %2.\n").arg(message).arg(keywords[i]));
	}*/

	// Prepare the language string for the speech.
	char lang[4] = "enu";
	if (!language.isEmpty()) {
		QByteArray qbaLang = language.toLatin1();
		memcpy(lang, qbaLang.constData(), qMin<size_t>(3, qbaLang.length()));
	}

	// Two different packet formats
	if (keywords.size() > 0) {
		m_Stream << (unsigned char)(type | 0xc0) << color << (unsigned short)font;
		m_Stream.writeRawData(lang, 4);

		// Build the encoded packet list
		unsigned int bitoffset = 0, byteoffset = 0;
		QByteArray array((12 + 5 * keywords.size() + 7) / 8, 0);
		pushData(array, byteoffset, bitoffset, keywords.size(), 12); // Push the keyword count (12 bit)
		for (int i = 0; i < keywords.size(); ++i) {
			pushData(array, byteoffset, bitoffset, keywords[i], 12); // 12 bit for each keyword
		}

		m_Stream.writeRawData(array.data(), array.size());

		writeUtf8Terminated(message);
	} else {
		m_Stream << (unsigned char)type << color << (unsigned short)font;
		m_Stream.writeRawData(lang, 4);
		
		// Simply dump the unicode string in big endian
		writeBigUnicodeTerminated(message);		
	}

	writeDynamicSize();
}

cTargetResponsePacket::cTargetResponsePacket(uint targetId, uchar targetType, uchar cursorType, cEntity *target) : cOutgoingPacket(0x6c, 19) {
	m_Stream << targetType << targetId << cursorType;

	// Target cancelled
	if (!target) {
		m_Stream << (unsigned int)0 << (unsigned int)~0 << (unsigned short)0 << (unsigned short)0;
	} else {
		// For Mobiles write all
		if (target->type() == MOBILE) {
			cMobile *mobile = dynamic_cast<cMobile*>(target);
			m_Stream << mobile->serial() << mobile->x() << mobile->y() << (short)mobile->z() << (unsigned short)0;
		} else if (target->type() == ITEM) {
			cDynamicItem *item = dynamic_cast<cDynamicItem*>(target);
			m_Stream << item->serial() << item->x() << item->y() << (short)item->z() << (unsigned short)item->id();
		} else if (target->type() == GROUND) {
			cGroundTile *item = dynamic_cast<cGroundTile*>(target);
			m_Stream << (unsigned int)0 << item->x() << item->y() << (short)item->z() << (unsigned short)0;
		} else if (target->type() == STATIC) {
			cStaticTile *item = dynamic_cast<cStaticTile*>(target);
			m_Stream << (unsigned int)0 << item->x() << item->y() << (short)item->z() << (unsigned short)item->id();
		}
	}
}

cGenericGumpResponsePacket::cGenericGumpResponsePacket(uint serial, uint type, uint button, QVector<uint> switches, QMap<uint, QString> strings) : cOutgoingPacket(0xb1, 23) {
	m_Stream << serial << type << button << (uint)switches.size();

	for (int i = 0; i < switches.size(); ++i) {
		m_Stream << switches[i];
	}

	m_Stream << (unsigned int)strings.size();

	QMap<uint, QString>::const_iterator it;
	for (it = strings.begin(); it != strings.end(); ++it) {
		m_Stream << (unsigned short)it.key() << (unsigned short)(it.value().length() + 1);
		writeBigUnicodeTerminated(it.value());
	}	

	writeDynamicSize();
}

cMoveRequestPacket::cMoveRequestPacket(uchar direction, uchar sequence, uint fastwalkKey) : cOutgoingPacket(0x02, 7) {
	m_Stream << direction << sequence << fastwalkKey;
}

cSingleClickPacket::cSingleClickPacket(uint serial) : cOutgoingPacket(0x09, 5) {
	m_Stream << serial;
}

cPingPacket::cPingPacket(uchar sequence) : cOutgoingPacket(0x73, 2) {
	m_Stream << sequence;
}

cRequestMultipleTooltipsPacket::cRequestMultipleTooltipsPacket(QVector<uint> tooltips) : cOutgoingPacket(0xd6, 3 + tooltips.size() * 4) {
	foreach (uint key, tooltips) {
		m_Stream << key;
	}
}
