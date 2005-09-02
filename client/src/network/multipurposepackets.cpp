
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "gui/worldview.h"
#include "gui/genericgump.h"
#include "game/mobile.h"
#include "game/dynamicitem.h"
#include "game/world.h"
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

		World->changeFacet((enFacet)facet);
		if (Player) {
			World->moveCenter(Player->x(), Player->y(), Player->z(), true);
			Player->move(Player->x(), Player->y(), Player->z());
		}

		/*Player->setSerial(serial);		
		Player->setBody(body);
		Player->setDirection(direction);
		World->moveCenter(posx, posy, posz, true);
		Player->move(posx, posy, posz);*/
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
			default:
				return 0;
		};
	}
};

AUTO_REGISTER_PACKET(0xbf, cMultiPurposePacket::creator);
