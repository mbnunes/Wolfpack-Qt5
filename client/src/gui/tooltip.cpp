
#include "gui/tooltip.h"
#include "game/entity.h"
#include "game/dynamicentity.h"
#include "game/tooltips.h"
#include "muls/localization.h"
#include "log.h"
#include "utilities.h"

cTooltip::cTooltip() {
	setSize(100, 100);
	
	cCheckerTrans *checkertrans = new cCheckerTrans(true);
	checkertrans->setAlign(CA_CLIENT);
	addControl(checkertrans);

	cTiledGumpImage *background = new cTiledGumpImage(0x243a);
	background->setAlign(CA_CLIENT);
	addControl(background);

	checkertrans = new cCheckerTrans(false);
	checkertrans->setAlign(CA_CLIENT);
	addControl(checkertrans);

	enableStencil_ = true;

	tooltip_ = 0;
	entity_ = 0;
}

cTooltip::~cTooltip() {	
}

void cTooltip::refreshTooltip() {
	cTooltipInfo *info = Tooltips->get(tooltip_);

	// Unknown Tooltip, request information about it
	// Tooltip information can only be requested from the server if this is a tooltip
	// for a dynamic object.
	if (!info && entity_) {
		cDynamicEntity *dynamic = dynamic_cast<cDynamicEntity*>(entity_);

		if (dynamic) {
			Tooltips->request(dynamic->serial(), tooltip_);
			Log->print(LOG_NOTICE, tr("Requesting tooltip for dynamic 0x%1.\n").arg(dynamic->serial(), 0, 16));
			return;
		}
	}

	if (info) {
		foreach (cLabel *label, lines) {
			removeControl(label);
			delete label;
		}
		lines.clear();

		int newWidth = 10;

		QVector<cTooltipInfo::Line> infolines = info->lines();
		foreach (cTooltipInfo::Line lineInfo, infolines) {
			QString localized = Localization->get(lineInfo.first);

			localized = Utilities::parseArguments(localized, lineInfo.second.split("\t"));

			// Capitalize everything after a space
			if (localized.length() > 0) {
				localized[0] = localized.at(0).toUpper();
			}
			for (int i = 0; i < localized.length() - 1; ++i) {
				if (localized.at(i).isSpace()) {
					localized[i+1] = localized.at(i+1).toUpper();
				}
			}

			ushort hue = 0x835;
			// The first line has a special meaning in most cases
			if (lines.isEmpty() && entity_) {
				if (entity_->type() == STATIC || entity_->type() == ITEM) {
					hue = 0x35; // Yellow color
				} else if (entity_->type() == MOBILE) {
					hue = 0x59; // Blue color
				}
			}

			cLabel *label = new cLabel(localized, 1, hue, true, ALIGN_CENTER);
			label->setHtmlMode(true);
			label->update();
			addControl(label);			
			lines.append(label);
			
			if (label->width() + 4 > newWidth) {
				newWidth = label->width() + 4;
			}
		}

		// calculate the width/height of the entire thing
		int currentY = 4;
		foreach (cLabel *label, lines) {
			label->setX(newWidth / 2 - label->width() / 2);
			label->setY(currentY);
			currentY += 4 + label->height();
		}

		setBounds(0, 0, newWidth, currentY);
	}
}

cTooltip *Tooltip = 0;
