
#include "muls/speech.h"
#include <qfile.h>
#include <qdatastream.h>
#include "exceptions.h"
#include "utilities.h"

cSpeechEntry::cSpeechEntry(QString pattern, unsigned short id) {
	if (pattern.startsWith("*")) {
		atBeginning = false;
		pattern = pattern.right(pattern.length() - 1); // Strip * from the start
	} else {
		atBeginning = true;
	}

	if (pattern.endsWith("*")) {
		atEnd = false;
		pattern = pattern.left(pattern.length() - 1); // Strip * from the end
	} else {
		atEnd = true;
	}

	this->pattern = pattern.toLower();
	this->id = id;
}

bool cSpeechEntry::match(const QString &othertext) const {
	// See if the pattern is the text
	if (atEnd && atBeginning) {		
		return (pattern == othertext);

	// See if the pattern is at position 0 of othertext
	} else if (atBeginning) {
		return othertext.startsWith(pattern);

	// See if the pattern is at the end of othertext
	} else if (atEnd) {
		return othertext.endsWith(pattern);

	// See if othertext contains pattern
	} else {
		return othertext.contains(pattern);
	}	
}

cSpeechEntry::cSpeechEntry() {
}

QVector<unsigned short> cSpeech::match(const QString &text) {
	QVector<unsigned short> result;
	QVector<cSpeechEntry>::const_iterator it;

	for (it = entries.begin(); it != entries.end(); ++it) {
		if (result.contains(it->id)) {
			continue; // Skip if id already in keyword list
		}

		if (it->match(text)) {			
			result.append(it->id);
		}
	}	

	return result;
}

void cSpeech::load() {
	QFile data(Utilities::getUoFilename("speech.mul"));
	
	if (!data.open(QFile::ReadOnly)) {
		throw Exception(tr("Unable to open speech data at %1.").arg(data.fileName()));
	}

	QDataStream stream(&data);
	stream.setByteOrder(QDataStream::BigEndian);
    
	unsigned short id;
	unsigned short length;

	while (!stream.atEnd()) {
		stream >> id >> length;

		if (length != 0) {
			char *strUtf8 = new char[length+1]; // Create temporary string
			stream.readRawData(strUtf8, length);
			strUtf8[length] = 0; // Ensure null termination
	
			entries.append( cSpeechEntry(QString::fromUtf8(strUtf8), id) );
	
			delete [] strUtf8; // Free memory
		}
	};
}

void cSpeech::unload() {
	entries.clear();
}

cSpeech *Speech = 0;
