
#if !defined(__TOOLTIP_H__)
#define __TOOLTIP_H__

#include "gui/checkertrans.h"
#include "gui/label.h"
#include "gui/window.h"
#include "gui/tiledgumpimage.h"

#include <QVector>

class cEntity;

class cTooltip : public cWindow {
Q_OBJECT
protected:
	QVector<cLabel*> lines;

	uint tooltip_;
	cEntity *entity_;
public:
	cTooltip();
	~cTooltip();

	void setTooltip(uint key);
	uint tooltip() const;
	void refreshTooltip();
	cEntity *entity() const;
	void setEntity(cEntity *entity);
};

inline void cTooltip::setTooltip(uint key) {
	if (key != tooltip_) {
		tooltip_ = key;
		refreshTooltip();
	}
}

inline uint cTooltip::tooltip() const {
	return tooltip_;
}

inline cEntity *cTooltip::entity() const {
	return entity_;
}

inline void cTooltip::setEntity(cEntity *entity) {
	entity_ = entity;
}

extern cTooltip *Tooltip;

#endif
