
#if !defined(__MOBILE_H__)
#define __MOBILE_H__

#include <qlist.h>
#include <qvector.h>

#include "enums.h"
#include "muls/animations.h"
#include "game/dynamicentity.h"

class cDynamicItem;

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
signals:
	void equipmentChanged();
	void bodyChanged();
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

extern cMobile *Player;

#endif
