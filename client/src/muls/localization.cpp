
#include "muls/localization.h"
#include "utilities.h"
#include "log.h"
#include <qfile.h>

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
	if (file.open(IO_ReadOnly)) {
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
				stream.readRawBytes(utf8Data, length); // Read the utf-8 data from the stream
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
	StringsMap::iterator sit = it.data().find(id);
	if (sit != it.data().end()) {
		return sit.data();
	}

	// Fall back to ENU
	if (language != "enu") {
		return get(id, "enu");
	}

	return tr("ERROR: Unknown Cliloc ID %1").arg(id);
}

cLocalization *Localization = 0;
