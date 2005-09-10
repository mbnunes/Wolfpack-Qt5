
#if !defined(__PROFILE_H__)
#define __PROFILE_H__

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QString>

class cProfile : QObject {
Q_OBJECT
protected:
	ushort speechHue_; // Hue of normal speech sent to server
	ushort emoteHue_; // Hue of emotes sent to server
	uchar defaultFont_; // Overrides unicode only
	QDateTime lastChange_; // Indicates the last change to the profile
	QString currentFilename_; // Where the current profile has been loaded from

public:
	cProfile();
	~cProfile();

	void loadFromString(const QString &data);
    void loadFromFile(QString filename);
	void saveToFile(const QString &filename);
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
