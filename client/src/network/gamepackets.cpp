
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "network/outgoingpackets.h"
#include "dialogs/login.h"
#include "gui/worldview.h"
#include "gui/genericgump.h"
#include "gui/containergump.h"
#include "game/mobile.h"
#include "game/dynamicitem.h"
#include "muls/localization.h"
#include "game/targetrequest.h"
#include "game/world.h"
#include "sound.h"
#include "mainwindow.h"
#include "log.h"
#include "enums.h"

#include <QCursor>
#include <qlist.h>

class cLoginConfirmPacket : public cIncomingPacket {
protected:
	unsigned int serial;
	unsigned short body;
	unsigned short posx, posy;
	signed short posz;
	unsigned char direction;
public:
	cLoginConfirmPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		safetyAssertSize(37);
		unsigned int unknown1;
		input >> serial >> unknown1 >> body >> posx >> posy >> posz >> direction;
	}

	virtual void handle(cUoSocket *socket) {
		Player->setSerial(serial);		
		Player->setBody(body);
		Player->setDirection(direction);
		World->moveCenter(posx, posy, posz, true);
		Player->move(posx, posy, posz);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cLoginConfirmPacket(input, size);
	}
};


AUTO_REGISTER_PACKET(0x1b, cLoginConfirmPacket::creator);

class cResetGamePacket : public cIncomingPacket {
public:
	cResetGamePacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {		
	}

	virtual void handle(cUoSocket *socket) {
		LoginDialog->hide();
		WorldView->setVisible(true);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cResetGamePacket(input, size);
	}
};


AUTO_REGISTER_PACKET(0x55, cResetGamePacket::creator);

class cUpdatePlayerPacket : public cIncomingPacket {
protected:
	unsigned int serial;
	unsigned short body;
	unsigned short hue;
	unsigned char flags;
	unsigned short posx, posy;
	signed char posz;
	unsigned char direction;
public:
	cUpdatePlayerPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		unsigned char unknown1;
		unsigned short unknown2;
		input >> serial >> body >> unknown1 >> hue >> flags >> posx >> posy >> unknown2 >> direction >> posz;
	}

	virtual void handle(cUoSocket *socket) {
		if (Player->serial() != serial) {
			return; // Do nothing if serials dont match
		}

		// Reset Walking Code
		UoSocket->clearSequenceQueue();
		UoSocket->setMoveSequence(0);

		if (body == 0x192 || body == 0x193) {
			Player->setDead(true);
			body -= 2;
		} else {
			Player->setDead(false);
		}

		// If the serial matches the player serial, update the player
		Player->setBody(body);
		Player->setHue(hue);
		Player->setDirection(direction);
		Player->processFlags(flags);
			
		// Update position if applicable
		if (posx != Player->x() || posy != Player->y() || posz != Player->z()) {
			World->moveCenter(posx, posy, posz, false);
			Player->move(posx, posy, posz);
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cUpdatePlayerPacket(input, size);
	}
};


AUTO_REGISTER_PACKET(0x20, cUpdatePlayerPacket::creator);

class cAddMobile : public cDynamicIncomingPacket {
protected:
	unsigned int serial;
	unsigned short body;
	unsigned short hue;
	unsigned char flags;
	unsigned short posx, posy;
	signed char posz;
	unsigned char direction;
	unsigned char notoriety;

	struct stEquipInfo {
		unsigned int serial;
		unsigned short id;
		unsigned short hue;
		unsigned char layer;
	};

	QList<stEquipInfo> equipment;
public:
	cAddMobile(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		input >> serial >> body;
		
		if (serial & 0x80000000) {
			unsigned short corpse; // or amount!
			input >> corpse;
			serial &= ~ 0x80000000; // Clear the flag
		}

		input >> posx >> posy;

		if (posx & 0x8000) {
			input >> direction;
			posx &= ~ 0x8000; // Clear the flag
		} else {
			direction = 0;
		}

		input >> posz >> direction >> hue >> flags >> notoriety;

		// Check for items
		stEquipInfo info;
		input >> info.serial;

		// Read until the serial is 0
		while (info.serial != 0) {
			input >> info.id >> info.layer;
			if (info.id & 0x8000) {
				input >> info.hue;
				info.id &= ~ 0x8000; // Clear the flag
			} else {
				info.hue = 0;
			}

            equipment.append(info);
			input >> info.serial; // Read next item serial
		}
	}

	virtual void handle(cUoSocket *socket) {
		// Normally we'd have to check if we have to create the mobile here
		cDynamicEntity *entity = World->findDynamic(serial);
		cMobile *mobile = 0;

		if (!entity && Utilities::isMobileSerial(serial)) {
			mobile = new cMobile(posx, posy, posz, World->facet(), serial);
			World->addEntity(mobile);
			entity = mobile;

			// Since the mobile is new, make sure to request it's name
			// Configure option here?
			UoSocket->send(cSingleClickPacket(serial));
		} else if (!entity) {
			return; // No support for items
		}

		if (entity->type() == MOBILE) {
			mobile = (cMobile*)entity;
			mobile->setHue(hue);
			mobile->setDirection(direction);

			if (body == 0x192 || body == 0x193) {
				mobile->setDead(true);
				body -= 2;
			} else {
				mobile->setDead(false);
			}

			mobile->setBody(body);

			if (posx != mobile->x() || posy != mobile->y() || posz != mobile->z()) {
				if (Player == mobile) {
					World->moveCenter(posx, posy, posz, false);
				}
				mobile->move(posx, posy, posz);
			}

			mobile->processFlags(flags);
		}

		if (mobile) {
			QList<stEquipInfo>::const_iterator it;
			for (it = equipment.begin(); it != equipment.end(); ++it) {
				cDynamicItem *item = World->findItem(it->serial);

				if (!item) {
					item = new cDynamicItem(mobile, (enLayer)it->layer, it->serial);
				}

				item->setHue(it->hue);
				item->setId(it->id);
			}
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cAddMobile(input, size);
	}
};


AUTO_REGISTER_PACKET(0x78, cAddMobile::creator);

class cAddGroundObject : public cDynamicIncomingPacket {
protected:
	unsigned int serial;
	unsigned short id;
	unsigned short hue;
	unsigned char flags;
	unsigned short posx, posy;
	signed char posz;
	unsigned char direction;
	unsigned short amount;
public:
	cAddGroundObject(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(size);

        input >> serial >> id;
		
		if (serial & 0x80000000) {
			input >> amount;
			serial &= ~ 0x80000000; // Clear the flag
		}

		// Whatever this is for, it's in the docs
		if (id & 0x8000) {
			unsigned char counter;
			input >> counter;
			id &= ~0x8000;
			id += counter;
		}

		input >> posx >> posy;

		if (posx & 0x8000) {
			input >> direction;
			posx &= ~ 0x8000; // Clear the flag
		} else {
			direction = 0;
		}

		input >> posz;

		if (posy & 0x8000) {
			input >> hue;
			posy &= ~ 0x8000; // Clear the flag
		} else {
			hue = 0;
		}

		if (posy & 0x4000) {
			input >> flags;
			posy &= ~ 0x4000; // Clear the flag
		} else {
			flags = 0;
		}
	}

	virtual void handle(cUoSocket *socket) {
		cDynamicItem *item = World->findItem(serial);

		if (!item) {
			item = new cDynamicItem(posx, posy, posz, Player->facet(), serial);
		} else {
			item->move(posx, posy, posz);
		}

		item->setHue(hue);
		item->setId(id);

		// TODO: Flags/Amount
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cAddGroundObject(input, size);
	}
};

AUTO_REGISTER_PACKET(0x1a, cAddGroundObject::creator);

class cDeleteObject : public cIncomingPacket {
protected:
	unsigned int serial;
public:
	cDeleteObject(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> serial;
	}

	virtual void handle(cUoSocket *socket) {
		if (Utilities::isItemSerial(serial)) {
			cDynamicItem *item = World->findItem(serial);

			if (item) {
				item->cleanPosition();
				item->decref();
			}
		} else if (Utilities::isMobileSerial(serial)) {
			cMobile  *mobile = World->findMobile(serial);

			if (mobile && Player != mobile) {
				World->removeEntity(mobile);
				mobile->decref();
			}
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cDeleteObject(input, size);
	}
};

AUTO_REGISTER_PACKET(0x1d, cDeleteObject::creator);

// Incoming unicode message
class cUnicodeMessagePacket : public cDynamicIncomingPacket {
protected:
	unsigned int serial; // Serial of source object
	unsigned short model; // Model of source object (whatever thats for..)
	unsigned char type; // Type of incoming message
	unsigned short hue;
	unsigned short font;
	QString language; // Pretty much unused
	QString name; // Name of source object
	QString message; // Message text
public:
	cUnicodeMessagePacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(50);
		input >> serial >> model >> type >> hue >> font;

		// Language
		char strLang[5];
		input.readRawBytes(strLang, 4);
		strLang[4] = 0;
		language = strLang;

		// Object Name
		char strName[31];
		input.readRawBytes(strName, 30);
		strName[30] = 0;
		name = strName;

		// Unicode Stuff
		unsigned int length = ((size - 48) / 2) + 1;
		ushort *strMessage = new unsigned short[length];
		
		// We need to swap the single characters
		for (unsigned int i = 0; i < (length - 1); ++i) {
			input >> strMessage[i];
		}

		strMessage[length-1] = 0;
        
		message = QString::fromUtf16(strMessage);

		delete [] strMessage;
	}

	virtual void handle(cUoSocket *socket) {
		if (!WorldView || !WorldView->isVisible()) {
			return; // Only process messages when ingame
		}

		cMobile *mobile = World->findMobile(serial);
		cDynamicItem *item = World->findItem(serial);

		// System Message
		if (!mobile && !item) {
			if (Utilities::isMobileSerial(serial) && !name.isEmpty()) {
				message.prepend("<");
				message.prepend(name);
				message.prepend("> ");
			}

			WorldView->addSysMessage(message, hue, font);
		} else if (mobile) {
			Gui->addOverheadText(0, 0, 3000, message, hue, font, mobile);
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cUnicodeMessagePacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0xae, cUnicodeMessagePacket::creator);

class cSoundEffectPacket : public cIncomingPacket {
protected:
	unsigned short model; // Sound id
	unsigned short posx, posy; // x,y position of source
	short posz; // z source
	unsigned short unknown;
	unsigned char mode; // Number of repeats?
public:
	cSoundEffectPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> mode >> model >> unknown >> posx >> posy >> posz;
	}

	virtual void handle(cUoSocket *socket) {
        Sound->playSound(model);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cSoundEffectPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x54, cSoundEffectPacket::creator);

// Incoming target request
class cRequestTargetPacket : public cIncomingPacket {
protected:
	unsigned int targetId; // Serial for the target command
	unsigned char targetType; // 0: Select Dynamic, 1: Select ground (allowed, not mandatory)
	unsigned char cursorType; // Criminal action?
public:
	cRequestTargetPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> targetType >> targetId >> cursorType;

		if (targetType > 1) {
			targetType = 1; // Only supported targettype for now
		}
	}

	virtual void handle(cUoSocket *socket) {
		if (!WorldView || !WorldView->isVisible()) {
			return; // Only process targets when ingame
		}

		// System Message
		Log->print(LOG_TRACE, tr("Received target request from server with serial 0x%1, type %2, cursor %3.\n").arg(targetId).arg(targetType).arg(cursorType));
		WorldView->requestTarget(new cServerTargetRequest(targetId, targetType, cursorType));
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cRequestTargetPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x6c, cRequestTargetPacket::creator);

// Generic Gump Packet
class cGenericGumpPacket : public cDynamicIncomingPacket {
protected:
	unsigned int serial, type;
	int x, y;
	QString layout;
	QStringList strings;

public:
	cGenericGumpPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(22);
		unsigned short length;
		input >> serial >> type >> x >> y >> length;
        
		char *strLayout = new char[length+1];
		input.readRawBytes(strLayout, length);
		strLayout[length] = 0; // Ensure null termination
		layout = strLayout; // Save

		unsigned short lineCount;
		input >> lineCount; // Number of lines

		for (int i = 0; i < lineCount; ++i) {   
			input >> length;
			ushort *strMessage = new unsigned short[length + 1];
			
			// We need to swap the single characters
			for (unsigned int j = 0; j < length; ++j) {
				input >> strMessage[j];
			}

			strMessage[length] = 0; // Ensure null termination
			strings.append(QString::fromUtf16(strMessage, length));

			delete [] strMessage;
		}
	}

	virtual void handle(cUoSocket *socket) {
		cGenericGump *gump = new cGenericGump(x, y, serial, type);
		gump->parseLayout(layout, strings);

		// TODO: Test if the gump has any visible controls and delete it
		// if it has not to prevent generic gumps from polluting the gui system.

		Gui->addControl(gump);
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cGenericGumpPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0xb0, cGenericGumpPacket::creator);


class cUpdateMobilePacket : public cIncomingPacket {
protected:
	unsigned int serial;
	unsigned short body;
	unsigned short hue;
	unsigned char flags;
	unsigned short posx, posy;
	signed char posz;
	unsigned char direction;
	unsigned char notoriety;
public:
	cUpdateMobilePacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> serial >> body >> posx >> posy >> posz >> direction >> hue >> flags >> notoriety;
	}

	virtual void handle(cUoSocket *socket) {
		cMobile *mobile = World->findMobile(serial);

		if (!mobile) {
			return;
		}

		if (body == 0x192 || body == 0x193) {
			mobile->setDead(true);
			body -= 2;
		} else {
			mobile->setDead(false);
		}

		bool running = (direction & 0x80) != 0; // see if the mobile is running
		direction &= ~ 0x80; // Clear the flag
		bool sameDirection = mobile->direction() == direction;

		mobile->setHue(hue);
		mobile->setDirection(direction);
		mobile->setBody(body);
		mobile->processFlags(flags);

		if ((posx != mobile->x() || posy != mobile->y() || posz != mobile->z()) && sameDirection) {
			// Can't move the player with this. Btw. this is _VERY_ "shaky"
			// using the wrong animation for certain mobiles etc. no running etc. etc.
			if (Player != mobile) {
				int diffx = mobile->x() - posx;
				int diffy = mobile->y() - posy;
				int diffz = mobile->z() - posz;
				int drawxoffset = (diffx - diffy) * 22;
				int drawyoffset = (diffx + diffy) * 22 - diffz * 4;

				// Calculate movement duration
				int duration = mobile->getMoveDuration(running);

				mobile->smoothMove(drawxoffset, drawyoffset, duration);
				mobile->move(posx, posy, posz);
				mobile->playMoveAnimation(duration, running);
			}
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cUpdateMobilePacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x77, cUpdateMobilePacket::creator);

// Move was rejected
class cRejectMovePacket : public cIncomingPacket {
protected:
	uchar sequence; // The sequence that was rejected
	ushort posx, posy; // The new x,y coordinates
	uchar direction; // The new direction
	signed char posz; // The new z position
public:
	cRejectMovePacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> sequence >> posx >> posy >> direction >> posz;
	}

	virtual void handle(cUoSocket *socket) {
		if (Player) {
			UoSocket->clearSequenceQueue();
			UoSocket->setMoveSequence(0);
			World->moveCenter(posx, posy, posz);
			Player->move(posx, posy, posz);
			Player->setDirection(direction);			
	
			Log->print(LOG_NOTICE, tr("Server rejected our move and bounced us to %1,%2,%3 direction %4.\n").arg(posx).arg(posy).arg(posz).arg(direction));
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cRejectMovePacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x21, cRejectMovePacket::creator);

// Move was accepted
class cAcceptMovePacket : public cIncomingPacket {
protected:
	uchar sequence; // The sequence that was accepted
	uchar notoriety; // Notoriety of the player
public:
	cAcceptMovePacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> sequence >> notoriety;
	}

	virtual void handle(cUoSocket *socket) {
		if (!Player) {
			return;
		}

		uint currentSequence = socket->popSequence();

		// Sequence mismatch
		if (currentSequence != sequence) {
			UoSocket->clearSequenceQueue();
			UoSocket->setMoveSequence(0);
		
			UoSocket->send(cResyncPacket()); // Send a resync request
			return;
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cAcceptMovePacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x22, cAcceptMovePacket::creator);

// Incoming predefined unicode message
class cPredefinedMessagePacket : public cDynamicIncomingPacket {
protected:
	unsigned int serial; // Serial of source object
	unsigned short model; // Model of source object (whatever thats for..)
	unsigned char type; // Type of incoming message
	unsigned short hue;
	unsigned short font;
	uint messageNumber; // Number of predefined message
	QString name; // Name of source object
	QString message; // Message arguments
public:
	cPredefinedMessagePacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(50);
		input >> serial >> model >> type >> hue >> font >> messageNumber;

		// Object Name
		char strName[31];
		input.readRawBytes(strName, 30);
		strName[30] = 0;
		name = strName;

		// Unicode Stuff
		// LITTLE ENDIAN (screw osi...)
		input.setByteOrder(QDataStream::LittleEndian);
		unsigned int length = ((size - 48) / 2) + 1;
		ushort *strMessage = new unsigned short[length];
		
		// We need to swap the single characters
		for (unsigned int i = 0; i < (length - 1); ++i) {
			input >> strMessage[i];
		}

		strMessage[length-1] = 0;
        
		message = QString::fromUtf16(strMessage);

		delete [] strMessage;
	}

	virtual void handle(cUoSocket *socket) {
		QString localized = Localization->get(messageNumber);

		// Parse in the ~1_...~ parts
		QStringList arguments = QStringList::split("\t", message, true);

		QRegExp pattern("~(\\d+)[^~]+~");
		int pos;
		while ((pos = pattern.indexIn(localized)) != -1 && !arguments.isEmpty()) {
			uint index = pattern.cap(1).toUInt();
			QString replacement;

			if (index < 1 || index > arguments.size()) {
				replacement = tr("[ERROR:%1]").arg(index);
			} else {
				replacement = arguments[index-1];
			}

			localized.replace(pos, pattern.matchedLength(), replacement);
		}

		if (Utilities::isItemSerial(serial)) {
			// "You see:" message
			if (type == 6) {
				cDynamicItem *item = World->findItem(serial);

				if (item) {
					Gui->addOverheadText(item->lastClickX(), item->lastClickY(), 3000, localized, hue, font, item);
				} else {
					serial = 0; // Set to invalid so it shows up as a sysmessage
				}
			}
		} else if (Utilities::isMobileSerial(serial)) {
			WorldView->addSysMessage(QString("<%1> %2").arg(name).arg(localized), hue, font);
		}
		
		if (Utilities::isInvalidSerial(serial)) {
			WorldView->addSysMessage(localized, hue, font);
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cPredefinedMessagePacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0xc1, cPredefinedMessagePacket::creator);

// Incoming ascii message
class cAsciiMessagePacket : public cDynamicIncomingPacket {
protected:
	unsigned int serial; // Serial of source object
	unsigned short model; // Model of source object (whatever thats for..)
	unsigned char type; // Type of incoming message
	unsigned short hue;
	unsigned short font;
	QString name; // Name of source object
	QString message; // Message content
public:
	cAsciiMessagePacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(45);
		input >> serial >> model >> type >> hue >> font;

		// Object Name
		char strName[31];
		input.readRawBytes(strName, 30);
		strName[30] = 0;
		name = strName;

		// Null terminated ascii string
		uint length = size - 44; // this includes the null byte
		char *strMessage = new char[length];
		input.readRawBytes(strMessage, length);
        strMessage[length-1] = 0; // ENSURE null termination
        
		message = QString::fromLatin1(strMessage);

		delete [] strMessage;
	}

	virtual void handle(cUoSocket *socket) {
		if (Utilities::isItemSerial(serial)) {
			// "You see:" message
			if (type == 6) {
				cDynamicItem *item = World->findItem(serial);

				if (item) {
					QPoint pos = GLWidget->mapFromGlobal(QCursor::pos());
					Gui->addOverheadText(item->lastClickX(), item->lastClickY(), 3000, message, hue, font, item);
				} else {
					serial = 0; // Set to invalid so it shows up as a sysmessage
				}
			}
		} else if (Utilities::isMobileSerial(serial)) {
			cMobile *mobile = World->findMobile(serial);

			if (mobile) {
				if (type == 6) {
					Gui->addOverheadText(0, 0, 3000, message, hue, font, mobile);
				} else {
					Gui->addOverheadText(0, 0, 3000, message, hue, font, mobile);
				}
			}
		}
		
		if (Utilities::isInvalidSerial(serial)) {
			WorldView->addSysMessage(message, hue, font);
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cAsciiMessagePacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x1c, cAsciiMessagePacket::creator);

class cCharacterEquipmentPacket : public cIncomingPacket {
protected:
	uint serial; // item serial
	ushort model;
	uchar unknown; // ?
	uint parent; // mobile equipping the item
	uchar layer; // layer to equip on
	ushort hue; // item hue
public:
	cCharacterEquipmentPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> serial >> model >> unknown >> layer >> parent >> hue;
	}

	virtual void handle(cUoSocket *socket) {
		cDynamicItem *equipment = World->findItem(serial);
		cMobile *mobile = World->findMobile(parent);

		if (mobile) {
			if (!equipment) {
				equipment = new cDynamicItem(mobile, layer, serial);
				equipment->setId(model);
				equipment->setHue(hue);
				return;
			} else {
				if (model != equipment->id()) {
					equipment->setId(model);
				}
				if (hue != equipment->hue()) {
					equipment->setHue(hue);
				}

				equipment->cleanPosition();
				equipment->move(mobile, layer);
			}
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cCharacterEquipmentPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x2e, cCharacterEquipmentPacket::creator);

class cDrawContainerPacket : public cIncomingPacket {
protected:
	uint serial; // item serial
	ushort gump; // container gump
public:
	cDrawContainerPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> serial >> gump;
	}

	virtual void handle(cUoSocket *socket) {
		cDynamicItem *container = World->findItem(serial);

		if (container) {
			container->showContent(gump);
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cDrawContainerPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x24, cDrawContainerPacket::creator);

class cAddItemToContainerPacket : public cIncomingPacket {
protected:
	uint serial; // item serial
	ushort id; // item id
	uchar unknown;
	ushort amount; // item amount
	short posx, posy; // position in container
	uint contserial; // container serial;
    ushort hue; // item color
public:
	cAddItemToContainerPacket(QDataStream &input, unsigned short size) : cIncomingPacket(input, size) {
		input >> serial >> id >> unknown >> amount >> posx >> posy >> contserial >> hue;
	}

	virtual void handle(cUoSocket *socket) {
		cDynamicItem *container = World->findItem(contserial);

		if (!container) {
			return;
		}

		cDynamicItem *item = World->findItem(serial);

		if (!item) {
			item = new cDynamicItem(container, serial);
		} else {
			item->move(container);
		}
		item->setId(id);
		item->setHue(hue);
		// TODO: Amount
		item->setContainerX(posx);
		item->setContainerY(posy);

		if (container->containerGump()) {
			container->containerGump()->flagContentChanged();
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cAddItemToContainerPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x25, cAddItemToContainerPacket::creator);

// Container content packet
class cContainerContentPacket : public cDynamicIncomingPacket {
protected:
	struct stItemInfo {
		uint serial; // item serial
		ushort id; // item model
		uchar unknown;
		ushort amount; // amount of items
		ushort posx, posy; // position in container
		uint contserial; // container serial
		ushort hue; // hue of item
	};
	QVector<stItemInfo> items;
public:
	cContainerContentPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		safetyAssertSize(5);
		ushort count; // number of items in packet
		input >> count;
		safetyAssertSize(5 + count * 19);

		for (int i = 0; i < count; ++i) {
			stItemInfo info;
			input >> info.serial >> info.id >> info.unknown 
				>> info.amount >> info.posx >> info.posy >> info.contserial >> info.hue;
			items.append(info);
		}
	}

	virtual void handle(cUoSocket *socket) {
		foreach (stItemInfo info, items) {
			cDynamicItem *container = World->findItem(info.contserial);
			if (!container) {
				continue; // Unknown container
			}

			cDynamicItem *item = World->findItem(info.serial);

			if (!item) {
				item = new cDynamicItem(container, info.serial);
			} else {
				item->move(container);
			}
			item->setId(info.id);
			item->setHue(info.hue);
			// TODO: Amount
			item->setContainerX(info.posx);
			item->setContainerY(info.posy);

			if (container->containerGump()) {
				container->containerGump()->flagContentChanged();
			}
		}
	}

	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		return new cContainerContentPacket(input, size);
	}
};

AUTO_REGISTER_PACKET(0x3c, cContainerContentPacket::creator);
