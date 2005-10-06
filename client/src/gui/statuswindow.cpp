
#include "gui/statuswindow.h"
#include "gui/gui.h"
#include "gui/asciilabel.h"
#include "gui/imagebutton.h"
#include "gui/tiledgumpimage.h"
#include "game/mobile.h"

cStatusWindow::cStatusWindow() {
	mobile_ = 0;
	smallStatus_ = false;
}

cStatusWindow::~cStatusWindow() {
}

void cStatusWindow::mobileDestroyed() {
	mobile_ = 0;
	Gui->queueDelete(this);
}

void cStatusWindow::setMobile(cMobile *mobile) {
	// Disconnect us from the previous mobile
	if (mobile_) {
		mobile_->disconnect(this);
	}

	mobile_ = mobile;

	// Make sure this dialog closes when the mobile goes out of scope
	if (mobile_) {
		connect(mobile_, SIGNAL(destroyed(QObject*)), SLOT(mobileDestroyed()));
		connect(mobile_, SIGNAL(statsChanged()), SLOT(statsUpdated()));
		connect(mobile_, SIGNAL(statLocksChanged()), SLOT(statLocksUpdated()));
	}
}

// Update the labels in this status gump
#define SET_SAFE_TEXT(name, value) { \
	cAsciiLabel *label = dynamic_cast<cAsciiLabel*>(findByName(name)); \
	if (label) label->setText(value); \
}

void cStatusWindow::statsUpdated() {
	if (!mobile_) {
		return;
	}

	SET_SAFE_TEXT("HealthLabel", QString::number(mobile_->health()));
	SET_SAFE_TEXT("MaxHealthLabel", QString::number(mobile_->maxHealth()));
	SET_SAFE_TEXT("StaminaLabel", QString::number(mobile_->stamina()));
	SET_SAFE_TEXT("MaxStaminaLabel", QString::number(mobile_->maxStamina()));
	SET_SAFE_TEXT("ManaLabel", QString::number(mobile_->mana()));
	SET_SAFE_TEXT("MaxManaLabel", QString::number(mobile_->maxMana()));
	SET_SAFE_TEXT("NameLabel", mobile_->name());

	if (!smallStatus_) {
		const cExtendedStatus *status = mobile_->status();
		if (status) {
			SET_SAFE_TEXT("StrengthLabel", QString::number(status->strength()));
			SET_SAFE_TEXT("DexterityLabel", QString::number(status->dexterity()));
			SET_SAFE_TEXT("IntelligenceLabel", QString::number(status->intelligence()));

			SET_SAFE_TEXT("GoldLabel", QString::number(status->gold()));
			SET_SAFE_TEXT("WeightLabel", QString::number(status->weight()));

			ushort maxweight = 40 + (int)(3.5f * status->strength());

			SET_SAFE_TEXT("MaxWeightLabel", QString::number(maxweight));
			SET_SAFE_TEXT("LuckLabel", QString::number(status->luck()));
			SET_SAFE_TEXT("StatsCapLabel", QString::number(status->statsCap()));
			SET_SAFE_TEXT("FollowersLabel", QString("%1/%2").arg(status->followers()).arg(status->maxFollowers()));
			SET_SAFE_TEXT("PhysicalResistLabel", QString::number(status->physicalResist()));
			SET_SAFE_TEXT("FireResistLabel", QString::number(status->fireResist()));
			SET_SAFE_TEXT("ColdResistLabel", QString::number(status->coldResist()));
			SET_SAFE_TEXT("PoisonResistLabel", QString::number(status->poisonResist()));
			SET_SAFE_TEXT("EnergyResistLabel", QString::number(status->energyResist()));
			SET_SAFE_TEXT("DamageLabel", QString("%1-%2").arg(status->minDamage()).arg(status->maxDamage()));		
		}
	} else {
		cTiledGumpImage *gump;

		gump = dynamic_cast<cTiledGumpImage*>(findByName("HealthBar"));
		if (gump) {
			uint originalWidth = gump->getTag("maxwidth").toUInt();
			if (originalWidth != 0) {
				gump->setWidth((int)(mobile_->healthPercent() * originalWidth / 100.0));
			}
		}

		gump = dynamic_cast<cTiledGumpImage*>(findByName("StaminaBar"));
		if (gump) {
			uint originalWidth = gump->getTag("maxwidth").toUInt();
			if (originalWidth != 0) {
				gump->setWidth((int)(mobile_->staminaPercent() * originalWidth / 100.0));
			}
		}

		gump = dynamic_cast<cTiledGumpImage*>(findByName("ManaBar"));
		if (gump) {
			uint originalWidth = gump->getTag("maxwidth").toUInt();
			if (originalWidth != 0) {
				gump->setWidth((int)(mobile_->manaPercent() * originalWidth / 100.0));
			}
		}
	}
}

inline void setGumps(cImageButton *lock, uchar lockState) {
	switch (lockState) {
		default:
		case 0:
			// Raise
			lock->setStateGump(BS_UNPRESSED, 0x983);
			lock->setStateGump(BS_PRESSED, 0x984);
			lock->setStateGump(BS_HOVER, 0x984);
			break;		
		case 1:
			// Locked
			lock->setStateGump(BS_UNPRESSED, 0x82c);
			lock->setStateGump(BS_PRESSED, 0x82c);
			lock->setStateGump(BS_HOVER, 0x82c);
			break;
		case 2:
			// Fall
			lock->setStateGump(BS_UNPRESSED, 0x985);
			lock->setStateGump(BS_PRESSED, 0x986);
			lock->setStateGump(BS_HOVER, 0x986);
			break;
	}
}

void cStatusWindow::statLocksUpdated() {
	if (!mobile_) {
		return;
	}

	cImageButton *lock;
	const cExtendedStatus *status = mobile_->status();

	if (!status) {
		return;
	}

	// Change the locks. 
	// we can set them to visible here since we know the server supports 
	// the statlock stuff
    lock = dynamic_cast<cImageButton*>(findByName("StrengthLock"));
	if (lock) {
		lock->setVisible(true);
		setGumps(lock, status->strengthLock());
	}
	lock = dynamic_cast<cImageButton*>(findByName("DexterityLock"));
	if (lock) {
		lock->setVisible(true);
		setGumps(lock, status->dexterityLock());
	}
	lock = dynamic_cast<cImageButton*>(findByName("IntelligenceLock"));
	if (lock) {
		lock->setVisible(true);
		setGumps(lock, status->intelligenceLock());
	}
}

void cStatusWindow::processDefinitionAttribute(QString name, QString value) {
	if (name == "smallstatus") {
		smallStatus_ = Utilities::stringToBool(value);
	} else {
		cWindow::processDefinitionAttribute(name, value);
	}
}
