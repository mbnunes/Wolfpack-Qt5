
#if !defined(__MOBILE_H__)
#define __MOBILE_H__

#include <qlist.h>

#include "enums.h"
#include "muls/animations.h"
#include "game/dynamicentity.h"

class cMobile : public cDynamicEntity {
protected:
	unsigned short body_;
	unsigned short hue_;
	unsigned char direction_;
	bool partialHue_;

	unsigned char currentAction_;
	unsigned int currentActionEnd_;
	unsigned int nextFrame;
	unsigned int frame;
	cSequence *sequence_; // Current sequence
	cSequence *equipmentSequences[LAYER_VISIBLECOUNT]; // For every layer a possible sequence

	// UNFINISHED
	struct stEquipInfo {
        unsigned int serial;
		unsigned short hue;
		unsigned short id;
		unsigned char layer;
	};
	QList<stEquipInfo> equipment;

	void freeSequence();
	void refreshSequence();
	unsigned int getFrameDelay();
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
	bool hitTest(int x, int y);
	void updatePriority();

	// Getters and Setters
	unsigned short body() const;
	unsigned short hue() const;
	unsigned char direction() const;
	bool partialHue() const;
	void setBody(unsigned short data);
	void setHue(unsigned short data);
	void setDirection(unsigned char data);
	unsigned char currentAction() const;
	unsigned int currentActionEnd() const;
	cSequence *sequence() const;
	void setSerial(unsigned int serial); // Only use this on the player

	void playAction(unsigned char action, unsigned int duration = 0);

	void smoothMove(int xoffset, int yoffset, unsigned int duration);

	void addEquipment(unsigned int serial, unsigned short id, unsigned short hue, enLayer layer);
};

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
	body_ = data;
	freeSequence();
}

inline void cMobile::setHue(unsigned short data) {
	// Set partial hue flag
	if (data & 0x8000) {
		partialHue_ = true;
		data &= ~ 0x8000; // Clear the flag
	}
	hue_ = data;
	freeSequence();
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

extern cMobile *Player;

#endif
