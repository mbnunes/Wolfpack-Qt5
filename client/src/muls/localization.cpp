
#include "muls/localization.h"
#include "utilities.h"
#include "log.h"
#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

cLocalization::cLocalization() {
	setObjectName("Localization");
}

void cLocalization::loadLanguage(QString language) {
	// Don't load a language twice
	if (loadedLanguages.contains(language)) {
		return;
	}

	StringsMap entries;

	// Try loading the file
	QFile file(Utilities::getUoFilename(QString("cliloc.%1").arg(language)));

	unsigned int clilocid;
	unsigned char unknown;
	unsigned short length;
	if (file.open(QFile::ReadOnly)) {
		QDataStream stream(&file);
		stream.setByteOrder(QDataStream::LittleEndian);

		unsigned int version;
		unsigned short subversion;
		stream >> version >> subversion;

		if (version != 2 || subversion != 1) {
			Log->print(LOG_ERROR, tr("Unable to read cliloc file %1. Unknown version %2.%3.\n").arg(file.fileName()).arg(version).arg(subversion));
		} else {
			// Start slurping in the entries in UTF-8
			while (!stream.atEnd()) {
				stream >> clilocid >> unknown >> length; // Read the entry data from the file
				char *utf8Data = new char[length+1]; // Create a new data array
				stream.readRawData(utf8Data, length); // Read the utf-8 data from the stream
				utf8Data[length] = 0; // Ensure null termination
				entries.insert(clilocid, QString::fromUtf8(utf8Data)); // Convert and insert data into the map
				delete [] utf8Data; // Free memory
			}

			Log->print(LOG_NOTICE, tr("Successfully loaded %1 strings from cliloc.%2.\n").arg(entries.size()).arg(language));
		}

        file.close(); // Close the cliloc file
	}

	languages.insert(language, entries); // Insert the entries into the language map
	loadedLanguages.append(language); // Denote that this file has been loaded
}

void cLocalization::load() {
	// Always load the ENU files
	loadLanguage("enu");
	
	// Get the local language and load it too
	loadLanguage(Utilities::localLanguage());
}

void cLocalization::unload() {
	languages.clear();
	loadedLanguages.clear();
}

QString cLocalization::get(unsigned int id, QString language) {
	if (language.isEmpty()) {
		language = Utilities::localLanguage();
	}

	language = language.toLower();

	LanguageMap::iterator it = languages.find(language);

	// Language has not been found in the list of loaded languages
	if (it == languages.end()) {
		// Try loading the language
		loadLanguage(language);

		// Retry getting the string
		it = languages.find(language);
		if (it == languages.end()) {
			return tr("ERROR: Unknown Language %1").arg(language);
		}
	}

	// Try to find a string with the given id
	StringsMap::iterator sit = it.value().find(id);
	if (sit != it.value().end()) {
		return sit.value();
	}

	// Fall back to ENU
	if (language != "enu") {
		return get(id, "enu");
	}

	return tr("ERROR: Unknown Cliloc ID %1").arg(id);
}

void cLocalization::export(const QString &filename, QString language) {
	QStringList lines;

	LanguageMap::iterator it = languages.find(language.toLower());

	if (it == languages.end()) {
		return;
	}

	StringsMap::iterator sit = it.value().begin();
	for ( ; sit != it.value().end(); ++sit) {
		lines << QString("%1 %2").arg(sit.key()).arg(sit.value());
	}

	lines.sort();

	QFile file(filename);
	
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream output(&file);
		foreach (QString line, lines) {
			output << line << "\n";
		}

		QMessageBox::information(MainWindow, tr("Localization Saved"), tr("The localization has been saved to %1.").arg(file.fileName()), QMessageBox::Ok, QMessageBox::NoButton);

		file.close();
	}
}

cLocalization *Localization = 0;
