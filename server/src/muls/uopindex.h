/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2026 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#ifndef UOPINDEX_H
#define UOPINDEX_H

#include "../server.h"
#include <QtGlobal>
#include <QString>
#include <QVector>
#include <QByteArray>

class QFile;

class WPEXPORT cUOPIndex
{
public:
	struct Chunk
	{
		quint64 fileOffset;    // Physical offset in UOP file
		quint32 length;        // Uncompressed / decompressed length
		qint64 virtualOffset;  // Virtual starting offset across all chunks
	};

	cUOPIndex();
	~cUOPIndex();

	bool open( QFile* file, const QString& pattern );
	void close();

	bool isValid() const { return valid_; }
	qint64 lookup( qint64 virtualOffset ) const;
	quint32 chunkCount() const { return static_cast<quint32>( chunks_.size() ); }
	qint64 totalSize() const { return totalSize_; }
	const QVector<Chunk>& chunks() const { return chunks_; }

	static quint64 hashLittle2( const QString& s );
	static quint64 hashLittle2( const char* s, int length );
	static QByteArray extractEntry( QFile* file, quint64 chunkOffset, quint32 compressedLength, quint32 decompressedLength, quint16 flag );

private:
	bool valid_;
	qint64 totalSize_;
	QVector<Chunk> chunks_;
};

#endif // UOPINDEX_H
