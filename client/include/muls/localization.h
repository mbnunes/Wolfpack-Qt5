
#if !defined(__LOCALIZATION_H__)
#define __LOCALIZATION_H__

#include <qmap.h>
#include <qstring.h>
#include <qobject.h>
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
	void load();
	void unload();

	QString get(unsigned int id, QString language = QString::null);
};

extern cLocalization *Localization;

#endif