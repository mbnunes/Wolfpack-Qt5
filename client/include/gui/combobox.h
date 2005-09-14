
#if !defined(__COMBOBOX_H__)
#define __COMBOBOX_H__

#include "gui/label.h"
#include "gui/container.h"
#include "gui/window.h"
#include <QStringList>
#include <QMap>

class cBorderGump;
class cCombobox;

/*
	This class represents the drop-down list shown by the combo box.
*/
class cCombolist : public cWindow {
Q_OBJECT
protected:
	cCombobox *combobox_;
	QMap<cLabel*, uint> labelIds;
public:
	cCombobox *combobox() const;
	void setCombobox(cCombobox *box);

	cCombolist();
	~cCombolist();

	void draw(int xoffset, int yoffset);

	void onMouseUp(QMouseEvent *e);

public slots:
	void scrolled(int pos);
	cControl *getControl(int x, int y);

signals:
	void itemSelected(int index);
};

inline cCombobox *cCombolist::combobox() const {
	return combobox_;
}

inline void cCombolist::setCombobox(cCombobox *box) {
	combobox_ = box;
}

/*
	This control has a fixed style
	Button id for opening the list: 0xa90/0xa91 and for closing the list: 0xa92/0xa93

*/
class cCombobox : public cContainer {
friend class cCombolist;

Q_OBJECT
private:
	bool dirty;
public:
	enum Style {
		Style1 = 0,
		StyleCount
	};

	cCombobox();
	~cCombobox();

protected:
	QStringList items_;
	int selectionIndex_;
	cLabel *selection_;
	Style style_;
	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);
	cCombolist *selectionList_;
	uint maxListHeight_;
	uint itemHeight_;

public slots:
	const QStringList &items() const;
	void setItems(const QStringList &items);
	QString selection() const;
	int selectionIndex() const;
	Style style() const;
	void setStyle(Style style);
	uint maxListHeight() const;
	void setMaxListHeight(uint maxListHeight);
	uint itemHeight() const;
	void setItemHeight(uint data);	

	void openSelectionList();
	void selectItem(int index);

signals:
	void selectionChanged();
};

inline const QStringList &cCombobox::items() const {
	return items_;
}

inline void cCombobox::setItems(const QStringList &items) {
	items_ = items;
	dirty = true;
}

inline QString cCombobox::selection() const {
	if (selection_) {
		return selection_->text();
	} else {
		return QString();
	}
}

inline int cCombobox::selectionIndex() const {
	return selectionIndex_;
}

inline cCombobox::Style cCombobox::style() const {
	return style_;
}

inline uint cCombobox::itemHeight() const {
	return itemHeight_;
}

inline void cCombobox::setItemHeight(uint height) {
	itemHeight_ = height;
}

inline uint cCombobox::maxListHeight() const {
	return maxListHeight_;
}

inline void cCombobox::setMaxListHeight(uint data) {
	maxListHeight_ = data;
}

#endif
