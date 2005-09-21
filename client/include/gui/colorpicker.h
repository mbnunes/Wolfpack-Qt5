
#if !defined(__COLORPICKER_H__)
#define __COLORPICKER_H__

#include "gui/control.h"
#include "texture.h"

class cColorPicker : public cControl {
Q_OBJECT
Q_PROPERTY(ushort starthue READ startHue)
Q_PROPERTY(ushort columns READ columns WRITE setColumns)
Q_PROPERTY(ushort rows READ rows WRITE setRows)
protected:
	bool dirty;
	cTexture *texture;
    ushort startHue_;
	ushort endHue_;
	ushort columns_;
	ushort rows_;

	void onChangeBounds(int, int, int, int);
public:
	void update();

	cColorPicker();
	virtual ~cColorPicker();

	void draw(int xoffset, int yoffset);	
	void processDefinitionAttribute(QString name, QString value);
	void onMouseDown(QMouseEvent *e);
public slots:
	void setStartHue(ushort hue);
	void setEndHue(ushort hue);
	ushort startHue() const;
	ushort endHue() const;
	ushort rows() const;
	ushort columns() const;
	void setColumns(ushort data);
	void setRows(ushort data);
signals:
	void colorSelected(ushort color);
};

inline void cColorPicker::setStartHue(ushort hue) {
	dirty = true;
	startHue_ = hue;
}

inline ushort cColorPicker::startHue() const {
	return startHue_;
}

inline void cColorPicker::setEndHue(ushort hue) {
	dirty = true;
	endHue_ = hue;
}

inline ushort cColorPicker::endHue() const {
	return endHue_;
}

inline ushort cColorPicker::rows() const {
	return rows_;
}

inline ushort cColorPicker::columns() const {
	return columns_;
}

inline void cColorPicker::setColumns(ushort data) {
	columns_  = data;
	dirty = true;
}

inline void cColorPicker::setRows(ushort data) {
	rows_ = data;
	dirty = true;
}

#endif
