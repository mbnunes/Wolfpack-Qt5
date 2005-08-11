
#if !defined(__SPEECH_H__)
#define __SPEECH_H__

#include <QRegExp>
#include <QString>
#include <QVector>

// This class is used for data representation
class cSpeechEntry {
public:
	QString pattern;
	unsigned short id;
	bool atBeginning;
	bool atEnd;

	cSpeechEntry(QString pattern, unsigned short id);
	cSpeechEntry();

	bool match(const QString &othertext) const;
};

class cSpeech {
protected:
	QVector<cSpeechEntry> entries;
public:
	void load();
	void unload();

	QVector<unsigned short> match(const QString &text);
};

extern cSpeech *Speech;

#endif
