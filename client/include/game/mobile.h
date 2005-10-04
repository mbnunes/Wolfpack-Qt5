
#if !defined(__MOBILE_H__)
#define __MOBILE_H__

#include <qlist.h>
#include <qvector.h>

#include "enums.h"
#include "muls/animations.h"
#include "game/dynamicentity.h"

class cDynamicItem;

class cExtendedStatus : public QObject {
Q_OBJECT
protected:
	ushort strength_;
	ushort dexterity_;
	ushort intelligence_;
	ushort statsCap_;
	uint gold_;
	ushort weight_;
	ushort luck_;
	uchar followers_;
	uchar maxFollowers_;
	ushort minDamage_;
	ushort maxDamage_;
	ushort physicalResist_;
	ushort fireResist_;
	ushort coldResist_;
	ushort poisonResist_;
	ushort energyResist_;
	uchar strengthLock_;
	uchar dexterityLock_;
	uchar intelligenceLock_;
public:
	ushort strength() const {
		return strength_;
	}
	void setStrength(ushort data) {
		strength_ = data;
	}

	ushort dexterity() const {
		return dexterity_;
	}
	void setDexterity(ushort data) {
		dexterity_ = data;
	}

	ushort intelligence() const {
		return intelligence_;
	}
	void setIntelligence(ushort data) {
		intelligence_ = data;
	}

	ushort statsCap() const {
		return statsCap_;
	}
	void setStatsCap(ushort data) {
		statsCap_ = data;
	}

	uint gold() const {
		return gold_;
	}
	void setGold(ushort data) {
		gold_ = data;
	}

	ushort weight() const {
		return weight_;
	}
	void setWeight(ushort data) {
		weight_ = data;
	}

	ushort luck() const {
		return luck_;
	}
	void setLuck(ushort data) {
		luck_ = data;
	}

	uchar followers() const {
		return followers_;
	}
	void setFollowers(uchar data) {
		followers_ = data;
	}
	uchar maxFollowers() const {
		return maxFollowers_;
	}
	void setMaxFollowers(uchar data) {
		maxFollowers_ = data;
	}

	ushort maxDamage() const {
		return maxDamage_;
	}
	void setMaxDamage(uchar data) {
		maxDamage_ = data;
	}
	ushort minDamage() const {
		return minDamage_;
	}
	void setMinDamage(uchar data) {
		minDamage_ = data;
	}

	ushort physicalResist() const {
		return physicalResist_;
	}
	void setPhysicalResist(ushort data) {
		physicalResist_ = data;
	}
	ushort fireResist() const {
		return fireResist_;
	}
	void setFireResist(ushort data) {
		fireResist_ = data;
	}
	ushort coldResist() const {
		return coldResist_;
	}
	void setColdResist(ushort data) {
		coldResist_ = data;
	}
	ushort poisonResist() const {
		return poisonResist_;
	}
	void setPoisonResist(ushort data) {
		poisonResist_ = data;
	}
	ushort energyResist() const {
		return energyResist_;
	}
	void setEnergyResist(ushort data) {
		energyResist_ = data;
	}

	uchar strengthLock() const {
		return strengthLock_;
	}
	void setStrengthLock(uchar data) {
		strengthLock_ = data;
	}
	uchar dexterityLock() const {
		return dexterityLock_;
	}
	void setDexterityLock(uchar data) {
		dexterityLock_ = data;
	}
	uchar intelligenceLock() const {
		return intelligenceLock_;
	}
	void setIntelligenceLock(uchar data) {
		intelligenceLock_ = data;
	}
};

class cMobile : public cDynamicEntity {
Q_OBJECT
Q_PROPERTY(ushort body READ body WRITE setBody)
Q_PROPERTY(ushort hue READ hue WRITE setHue)
Q_PROPERTY(uchar direction READ direction WRITE setDirection)
Q_PROPERTY(bool partialhue READ partialHue)
Q_PROPERTY(uchar currentaction READ currentAction)
Q_PROPERTY(uchar currentmountaction READ currentMountAction)
Q_PROPERTY(Notoriety notoriety READ notoriety WRITE setNotoriety)
Q_PROPERTY(bool warmode READ isInWarmode WRITE setWarmode)
Q_PROPERTY(ushort health READ health WRITE setHealth)
Q_PROPERTY(ushort stamina READ stamina WRITE setStamina)
Q_PROPERTY(ushort mana READ mana WRITE setMana)
Q_PROPERTY(ushort maxhealth READ maxHealth WRITE setMaxHealth)
Q_PROPERTY(ushort maxstamina READ maxStamina WRITE setMaxStamina)
Q_PROPERTY(ushort maxmana READ maxMana WRITE setMaxMana)
Q_PROPERTY(QString name READ name WRITE setName)
Q_PROPERTY(bool renameable READ renameable WRITE setRenameable)
Q_ENUMS(Notoriety)
public:
	enum Notoriety {
		Unknown = 0,
		Innocent, // 1, Blue
		GuildAlly, // 2, Green
		Critter, // 3, Gray
		Criminal, // 4, Gray
		GuildEnemy, // 5, Orange
		Murderer, // 6, Red
		Invulnerable, // 7, Yellow
	};

protected:
	bool deleting;
	unsigned short body_;
	unsigned short hue_;
	unsigned char direction_;
	bool partialHue_;
	bool hidden;
	bool dead;
	bool warmode;
	Notoriety notoriety_;
	ushort health_;
	ushort maxHealth_;
	ushort mana_;
	ushort maxMana_;
	ushort stamina_;
	ushort maxStamina_;
	QString name_;
	bool renameable_;
	cExtendedStatus *status_;

	unsigned char currentAction_;
	unsigned int currentActionEnd_;
	unsigned int nextFrame, nextMountFrame;
	unsigned int frame, mountFrame;
	cSequence *sequence_; // Current sequence
	cSequence *equipmentSequences[LAYER_COUNT]; // For every layer a possible sequence
	cDynamicItem *equipment[LAYER_COUNT];
	QVector<cDynamicItem*> invisibleEquipment;

	void freeSequence();
	void refreshSequence();
	unsigned int getFrameDelay();
	unsigned int getMountFrameDelay();
	unsigned char getIdleAction();

	// This is used to allow smooth movement between two tiles
	float smoothMoveFactor; // Current smooth move factor
	int drawxoffset; // Draw x offset from new tile to old tile
	int drawyoffset; // Draw y offset from new tile to old tile
	unsigned int smoothMoveTime; // Duration of smooth move effect
	unsigned int smoothMoveEnd; // End time of the smooth move effect	

public:
	cMobile(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial);
	virtual ~cMobile();
	void draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip);
public slots:
	ushort getNameHue() const;
	bool hitTest(int x, int y);
	void updatePriority();
	
	// Getters and Setters
	enBodyType bodyType() const; // Translate the body into a body type
	unsigned short body() const;
	unsigned short hue() const;
	unsigned char direction() const;
	bool partialHue() const;
	void setBody(unsigned short data);
	void setHue(unsigned short data);
	void setDirection(unsigned char data);
	unsigned char currentAction() const;
	ushort currentMountAction() const;
	unsigned int currentActionEnd() const;
	cSequence *sequence() const;
	Notoriety notoriety() const;
	void setNotoriety(Notoriety data);
	void setSerial(unsigned int serial); // Only use this on the player
	void setHidden(bool data);
	bool isHidden() const;
	void setDead(bool data);
	bool isDead() const;
	bool isMoving() const;
	bool isInWarmode() const;
	void setWarmode(bool warmode);
	const cExtendedStatus *status() const;
	void setStatus(cExtendedStatus *status);

	void onDoubleClick(QMouseEvent *e);
	void onClick(QMouseEvent *e);

	void playAction(unsigned char action, unsigned int duration = 0);

	void smoothMove(int xoffset, int yoffset, unsigned int duration);
	uint getMoveDuration(bool running) const;
	void playMoveAnimation(uint duration, bool running);

	// Steals a reference
	void addEquipment(cDynamicItem *item);
	void removeEquipment(cDynamicItem *item);
	void refreshEquipment(enLayer layer);
	void clearEquipment();
	cDynamicItem *getEquipment(enLayer layer) const;

	void processFlags(uchar flags);

	uint getCurrentHeight();
	ushort health() const;
	ushort maxHealth() const;
	ushort stamina() const;
	ushort maxStamina() const;
	ushort mana() const;
	ushort maxMana() const;
	void setStatLocks(uchar strength, uchar dexterity, uchar intelligence);
	void setStats(ushort health, ushort maxHealth, ushort stamina, ushort maxStamina, ushort mana, ushort maxMana);
	void setHealth(ushort data);
	void setStamina(ushort data);
	void setMana(ushort data);
	void setMaxHealth(ushort data);
	void setMaxStamina(ushort data);
	void setMaxMana(ushort data);
	ushort healthPercent() const;
	ushort manaPercent() const;
	ushort staminaPercent() const;
	QString name() const;
	void setName(const QString &name);
	bool renameable() const;
	void setRenameable(bool data);
signals:
	void equipmentChanged();
	void bodyChanged();
	void statsChanged();
	void statLocksChanged();
};

Q_DECLARE_METATYPE(cMobile*);

inline cDynamicItem *cMobile::getEquipment(enLayer layer) const {
	if (layer < LAYER_COUNT) {
		return equipment[layer];
	} else {
		return 0;
	}
}

inline void cMobile::setDead(bool data) {
	dead = data;
}

inline bool cMobile::isDead() const {
	return dead;
}

inline void cMobile::setHidden(bool data) {
	hidden = data;
}

inline bool cMobile::isHidden() const {
	return hidden;
}

inline unsigned short cMobile::body() const {
	return body_;
}

inline unsigned short cMobile::hue() const {
	return hue_;
}

inline unsigned char cMobile::direction() const {
	return direction_;
}

inline bool cMobile::partialHue() const {
	return partialHue_;
}

inline void cMobile::setBody(unsigned short data) {
	if (data != body_) {
		body_ = data;
		currentAction_ = getIdleAction();
		freeSequence();
		emit bodyChanged();
	}
}

inline void cMobile::setHue(unsigned short data) {
	// Set partial hue flag
	if (data & 0x8000) {
		partialHue_ = true;
		data &= ~ 0x8000; // Clear the flag
	}	
	hue_ = data;	
	freeSequence();
	emit bodyChanged();
}

inline void cMobile::setDirection(unsigned char data) {
	direction_ = data;
	freeSequence();
}

inline unsigned char cMobile::currentAction() const {
	return currentAction_;
}

inline unsigned int cMobile::currentActionEnd() const {
	return currentActionEnd_;
}

inline cSequence *cMobile::sequence() const {
	return sequence_;
}

inline bool cMobile::isMoving() const {
	return smoothMoveEnd > 0;
}

inline bool cMobile::isInWarmode() const {
	return warmode;
};

inline cMobile::Notoriety cMobile::notoriety() const {
	return notoriety_;
}

inline ushort cMobile::health() const {
	return health_;
}

inline ushort cMobile::maxHealth() const {
	return maxHealth_;
}

inline ushort cMobile::stamina() const {
	return stamina_;
}

inline ushort cMobile::maxStamina() const {
	return maxStamina_;
}

inline ushort cMobile::mana() const {
	return mana_;
}

inline ushort cMobile::maxMana() const {
	return maxMana_;
}

inline void cMobile::setHealth(ushort data) {
	health_ = data;
	emit statsChanged();
}

inline void cMobile::setStamina(ushort data) {
	stamina_ = data;
	emit statsChanged();
}

inline void cMobile::setMana(ushort data) {
	mana_ = data;
	emit statsChanged();
}

inline void cMobile::setMaxHealth(ushort data) {
	maxHealth_ = data;
	emit statsChanged();
}

inline void cMobile::setMaxStamina(ushort data) {
	maxStamina_ = data;
	emit statsChanged();
}

inline void cMobile::setMaxMana(ushort data) {
	maxMana_ = data;
	emit statsChanged();
}

inline void cMobile::setStats(ushort health, ushort maxHealth, ushort stamina, ushort maxStamina, ushort mana, ushort maxMana) {
	health_ = health;
	stamina_ = stamina;
	mana_ = mana;
	maxHealth_ = maxHealth;
	maxStamina_ = maxStamina;
	maxMana_ = maxMana;
	emit statsChanged();
}

inline ushort cMobile::healthPercent() const {
	if (maxStamina_) {
		return qMin<ushort>(100, (health_ / maxHealth_) * 100);
	} else {
		return 0;
	}
}

inline ushort cMobile::manaPercent() const {
	if (maxStamina_) {
		return qMin<ushort>(100, (mana_ / maxMana_) * 100);
	} else {
		return 0;
	}
}

inline ushort cMobile::staminaPercent() const {
	if (maxStamina_) {
		return qMin<ushort>(100, (stamina_ / maxStamina_) * 100);
	} else {
		return 0;
	}
}

inline QString cMobile::name() const {
	return name_;
}

inline void cMobile::setName(const QString &name) {
	name_ = name;
}

inline bool cMobile::renameable() const {
	return renameable_;
}

inline void cMobile::setRenameable(bool data) {
	renameable_ = data;
}

inline const cExtendedStatus *cMobile::status() const {
	return status_;
}

extern cMobile *Player;

#endif
