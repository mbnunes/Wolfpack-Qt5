
#if !defined(__PROFILE_H__)
#define __PROFILE_H__

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QString>
#include <QKeySequence>
#include <QDomElement>
#include <QDomDocument>

class cBaseAction;

class cProfile : QObject {
Q_OBJECT
protected:
	ushort speechHue_; // Hue of normal speech sent to server
	ushort emoteHue_; // Hue of emotes sent to server
	uchar defaultFont_; // Overrides unicode only
	QDateTime lastChange_; // Indicates the last change to the profile
	QString currentFilename_; // Where the current profile has been loaded from
	QVector<cBaseAction*> keyBindings_; // Key bindings for this profile

	void loadKeyBindings(QDomElement &element);
	QDomElement saveKeyBindings(QDomDocument &document);

public:
	cProfile();
	~cProfile();

	void loadFromString(const QString &data);
    void loadFromFile(QString filename, bool dontPrependPath = false);
	void saveToFile(const QString &filename, bool dontPrependPath = false);
	void saveToFile();
	QString saveToString();

	ushort speechHue() const;
	void setSpeechHue(ushort data);
	ushort emoteHue() const;	
	void setEmoteHue(ushort data);
	uchar defaultFont() const;
	void setDefaultFont(uchar data);
	QDateTime lastChange();
	const QString &currentFilename() const;

	bool processShortcut(const QKeySequence &sequence);

	void clearProfile();
};

inline ushort cProfile::speechHue() const {
	return speechHue_;
}

inline void cProfile::setSpeechHue(ushort data) {
	speechHue_ = data;
	lastChange_ = QDateTime::currentDateTime();
}

inline ushort cProfile::emoteHue() const {
	return emoteHue_;
}

inline void cProfile::setEmoteHue(ushort data) {
	emoteHue_ = data;
	lastChange_= QDateTime::currentDateTime();
}

inline uchar cProfile::defaultFont() const {
	return defaultFont_;
}

inline void cProfile::setDefaultFont(uchar data) {
	defaultFont_ = data;
	lastChange_ = QDateTime::currentDateTime();
}

inline const QString &cProfile::currentFilename() const {
	return currentFilename_;
}

extern cProfile *Profile;

#endif
