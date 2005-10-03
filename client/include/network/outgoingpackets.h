
#if !defined(__OUTGOINGPACKETS_H__)
#define __OUTGOINGPACKETS_H__

#include "network/outgoingpacket.h"
#include "enums.h"
#include <QMap>
#include <qvector.h>

/*
	This file contains class definitions for outgoing packets.
*/

// 0x80: Attach to Loginserver
class cLoginPacket : public cOutgoingPacket {
public:
	cLoginPacket(const QString &username, const QString &password);
};

// 0x91 Attach to Gameserver
class cGameLoginPacket : public cOutgoingPacket {
public:
	cGameLoginPacket(unsigned int key, const QString &username, const QString &password);
};

// 0xa0 Request to be relayed to another server
class cRequestRelayPacket : public cOutgoingPacket {
public:
	cRequestRelayPacket(unsigned short id);
};

// 0x83 Request Character Deletion
class cDeleteCharacter : public cOutgoingPacket {
public:
	cDeleteCharacter(unsigned int id);
};

// 0x5d Play Character
class cPlayMobilePacket : public cOutgoingPacket {
public:
	cPlayMobilePacket(unsigned char id);
};

// 0x06 Double Click
class cDoubleClickPacket : public cOutgoingPacket {
public:
	cDoubleClickPacket(unsigned int serial);
};

// 0x55 Resync with server
class cResyncPacket : public cOutgoingPacket {
public:
	cResyncPacket();
};

// 0xad	Send unicode speech
class cSendUnicodeSpeechPacket : public cOutgoingPacket {
public:
	cSendUnicodeSpeechPacket(enSpeechType type, const QString &message, ushort color = 0xFFFF, uchar font = 0, const QString &language = QString::null);
};

// 0x6c Targetting Packet
class cEntity;
class cTargetResponsePacket : public cOutgoingPacket {
public:
	cTargetResponsePacket(uint targetId, uchar targetType, uchar cursorType, cEntity *target);
};

// 0xb1 Generic gump response
class cGenericGumpResponsePacket : public cOutgoingPacket {
public:
	cGenericGumpResponsePacket(uint serial, uint type, uint button, QVector<uint> switches, QMap<uint, QString> strings);
};

// 0x02 Move Request
class cMoveRequestPacket : public cOutgoingPacket {
public:
	cMoveRequestPacket(uchar direction, uchar sequence, uint fastwalkKey = 0);
};

// 0x09 Single Click Request
class cSingleClickPacket : public cOutgoingPacket {
public:
	cSingleClickPacket(uint serial);
};

// 0x73 Ping Message
class cPingPacket : public cOutgoingPacket {
public:
	cPingPacket(uchar sequence);
};

// 0xd6 Request multiple tooltips
class cRequestMultipleTooltipsPacket : public cOutgoingPacket {
public:
	cRequestMultipleTooltipsPacket(QVector<uint> tooltips);
};

// Character Creation
class cCharacterCreationInfo {
public:
	QString name; // Character Name
	bool female; // Flag for gender
	uchar strength, dexterity, intelligence; // Statistics
	uchar skill1, skill2, skill3; // Ids for the 3 starting skills
	uchar skill1Value, skill2Value, skill3Value; // Values for the three starting skills
	ushort skinColor, hairColor, beardColor; // Color values
	ushort hairStyle, beardStyle; // Item ids for beard/hair. 0 for none.
	uchar startLocation; // Where to start
	uint characterSlot; // Which slot to create this one in (rather useless...)
	ushort shirtHue, pantsHue; // Hue of shirt and pants

	cCharacterCreationInfo();
};

class cCharacterCreationPacket : public cOutgoingPacket {
public:
	cCharacterCreationPacket(const cCharacterCreationInfo &info);
};

// Request Context Menu for object
class cRequestContextMenu : public cOutgoingPacket {
public:
	cRequestContextMenu(uint serial);
};

// Send Contextmenu Response
class cContextMenuResponsePacket : public cOutgoingPacket {
public:
	cContextMenuResponsePacket(uint serial, ushort id);
};

// Send warmode change request
class cWarmodeChangeRequest : public cOutgoingPacket {
public:
	cWarmodeChangeRequest(bool mode);
};

// Send the version of this client to the server
class cVersionPacket : public cOutgoingPacket {
public:
	cVersionPacket(const QString &version);
};

// Send the client language to the server
class cLanguagePacket : public cOutgoingPacket {
public:
	cLanguagePacket(const QString &language);
};

// Request help
class cRequestHelpPacket : public cOutgoingPacket {
public:
	cRequestHelpPacket();
};

// Attack a target
class cRequestAttackPacket : public cOutgoingPacket {
public:
	cRequestAttackPacket(uint serial);
};

// Grab an object
class cGrabItemPacket : public cOutgoingPacket {
public:
	cGrabItemPacket(uint serial, ushort amount = 1);
};

// Drop the object on the ground
class cDropItemPacket : public cOutgoingPacket {
public:
	cDropItemPacket(uint item, ushort x, ushort y, signed char z, uint container);
};

// Wear an item on the given layer
class cWearItemPacket : public cOutgoingPacket {
public:
	cWearItemPacket(uint item, uchar layer, uint mobile);
};

// Request the status of the given mobile
class cRequestStatusPacket : public cOutgoingPacket {
public:
	cRequestStatusPacket(uint serial);
};

// Request the skills of the given mobile
class cRequestSkillsPacket : public cOutgoingPacket {
public:
	cRequestSkillsPacket(uint serial);
};

#endif
