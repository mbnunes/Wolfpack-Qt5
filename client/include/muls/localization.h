
#if !defined(__LOCALIZATION_H__)
#define __LOCALIZATION_H__

#include <QMap>
#include <qstring.h>
#include <QObject>
#include <qstringlist.h>

class cLocalization : public QObject {
Q_OBJECT
protected:
	typedef QMap<unsigned int, QString> StringsMap;
	typedef QMap<QString, StringsMap> LanguageMap;
	QStringList loadedLanguages;

	LanguageMap languages;

	void loadLanguage(QString language);
public:
	cLocalization();

	void load();
	void unload();

public slots:
	QString get(uint id, QString language = QString::null);
};

extern cLocalization *Localization;

#endif
