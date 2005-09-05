
#if !defined(__TOOLTIPS_H__)
#define __TOOLTIPS_H__

#include <QDateTime>
#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>
#include <QPair>

class cTooltipInfo : public QObject {
Q_OBJECT
public:
	typedef QPair<uint, QString> Line;

	cTooltipInfo(uint key, QVector<Line> lines);
	~cTooltipInfo();

	uint key() const;	
	QVector<Line> lines() const;
protected:
	uint key_;
    QVector<Line> lines_;
};

inline QVector<cTooltipInfo::Line> cTooltipInfo::lines() const {
	return lines_;
}

inline uint cTooltipInfo::key() const {
	return key_;
}

class cTooltips : public QObject {
Q_OBJECT
public:
	cTooltips();
	~cTooltips();

	void clear(); // Clear all saved tooltips
	cTooltipInfo *get(uint key) const;
	void add(cTooltipInfo *info);
	void remove(uint key);
	bool contains(uint key) const;
	void request(uint serial, uint key);
	void processRequests();

protected:
	typedef QMap<uint, cTooltipInfo*> Container;
	typedef Container::iterator Iterator;

	QVector<uint> requestQueue;
	QMap<uint, QDateTime> requested; // Tooltips requested (and when)
	Container tooltips; // Known tooltips
};

extern cTooltips *Tooltips;

#endif
