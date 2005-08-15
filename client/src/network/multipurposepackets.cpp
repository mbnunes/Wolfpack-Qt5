
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "dialogs/login.h"
#include "gui/worldview.h"
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

class cMultiPurposePacket {
public:
	static cIncomingPacket *creator(QDataStream &input, unsigned short size) {
		if (size < 5) {
			return 0;
		}

		QIODevice::Offset pos = input.device()->at();
		input.device()->at(3);
		unsigned short actionid;
		input >> actionid;
		input.device()->at(pos);

		switch (actionid) {
			case 0x8:
				return new cSwitchFacetPacket(input, size);
			default:
				return 0;
		};
	}
};

AUTO_REGISTER_PACKET(0xbf, cMultiPurposePacket::creator);