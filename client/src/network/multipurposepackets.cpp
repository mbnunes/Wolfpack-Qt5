
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "gui/contextmenu.h"
#include "gui/worldview.h"
#include "gui/genericgump.h"
#include "game/mobile.h"
#include "game/dynamicitem.h"
#include "game/world.h"
#include "muls/localization.h"
#include "sound.h"
#include "log.h"
#include "enums.h"

// Switch between facets
class cSwitchFacetPacket : public cDynamicIncomingPacket {
protected:
	unsigned char facet;
public:
	cSwitchFacetPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		unsigned short packetId;
		input >> packetId >> facet;
	}

	virtual void handle(cUoSocket *socket) {
		if (facet >= FACETCOUNT) {
			Log->print(LOG_ERROR, tr("Trying to change to an invalid facet %1.\n").arg(facet));
			return;
		}

		if (facet != World->facet()) {
			World->changeFacet((enFacet)facet);
			if (Player) {
				World->moveCenter(Player->x(), Player->y(), Player->z(), true);
				Player->move(Player->x(), Player->y(), Player->z());
			}
		}
	}
};

// Close Generic Gump Packet
class cCloseGenericGumpPacket : public cDynamicIncomingPacket {
protected:
	unsigned int type, button;
public:
	cCloseGenericGumpPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		unsigned short packetId;
		input >> packetId >> type >> button;
	}

	virtual void handle(cUoSocket *socket) {
		cGenericGump *gump = cGenericGump::findByType(type);

		if (gump) {
			Gui->removeControl(gump);
			gump->sendResponse(button);
			delete gump;
		}
	}
};

// Show Context Menu Packet
class cShowContextMenuPacket : public cDynamicIncomingPacket {
protected:
	ushort unknown;
	uint serial; // Object serial for contextmenu
	
	struct stEntry {
		ushort id; // Id that is returned to the server
		ushort localization; // This is the id of the localization string for this entry (+3000000?)
		ushort flags;
		ushort color; // Note that this is a direct 15-bit color value
	};

	QVector<stEntry> entries;
public:
	cShowContextMenuPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		ushort packetId;
		uchar count;
		input >> packetId >> unknown >> serial >> count;
		
		stEntry entry;
		for (int i = 0; i < count; ++i) {
			input >> entry.id >> entry.localization >> entry.flags;

			if (entry.flags & 0x20) {
				input >> entry.color;
			} else {
				entry.color = 0;
			}

			entries.append(entry);
		}
	}

	virtual void handle(cUoSocket *socket) {
		cDynamicEntity *entity = World->findDynamic(serial);

		if (entity) {
			ContextMenu->clear();
			foreach (const stEntry &entry, entries) {
				QString localization = Localization->get(3000000 + entry.localization);
				ContextMenu->addEntry(localization, 0x835, entry.id);
			}
			ContextMenu->setSerial(serial);
			cMobile *mobile = dynamic_cast<cMobile*>(entity);
			if (mobile) {
				ContextMenu->show(entity->drawx(), entity->drawy() - mobile->getCurrentHeight() / 2);
			} else {			
				ContextMenu->show(entity->drawx(), entity->drawy());
			}
		}		
	}
};

// Set the Statlocks
class cStatLocksPacket : public cDynamicIncomingPacket {
protected:
	uchar unknown1;
	uint serial;
	uchar unknown2;
	uchar locks;
public:
	cStatLocksPacket(QDataStream &input, unsigned short size) : cDynamicIncomingPacket(input, size) {
		ushort packetId;
		input >> packetId >> unknown1 >> serial >> unknown2 >> locks;		
	}

	virtual void handle(cUoSocket *socket) {
		cMobile *mobile = World->findMobile(serial);

		if (mobile) {
			mobile->setStatLocks((locks >> 4) & 3, (locks >> 2) & 3,locks & 3);
		}
	}
};

class cMultiPurposePacket {
public:
	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		if (size < 5) {
			return 0;
		}

		uint pos = input.device()->pos();
		input.device()->seek(3);
		unsigned short actionid;
		input >> actionid;
		input.device()->seek(pos);

		switch (actionid) {
			case 0x4:
				return new cCloseGenericGumpPacket(input, size);
			case 0x8:
				return new cSwitchFacetPacket(input, size);
			case 0x14:
				return new cShowContextMenuPacket(input, size);
			case 0x19:
				return new cStatLocksPacket(input, size);
			default:
				return 0;
		};
	}
};

AUTO_REGISTER_PACKET(0xbf, cMultiPurposePacket::creator);
