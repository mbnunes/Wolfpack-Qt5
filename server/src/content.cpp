/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "content.h"
#include "items.h"
#include <qstring.h>

/*
	Iterate over the content of an item.
*/
ContainerIterator::ContainerIterator(const cItem *item) : content(item->content()), pos(0) {
}

ContainerCopyIterator::ContainerCopyIterator(const cItem *item) : pos(0) {
	content = item->content();
}

QString ContainerContent::dump() {
	QString result = "Content of array:\n";
	result += QString("Count: %1; MaxCount: %2\n").arg(count()).arg(maxCount());

	size_t count = this->count();
	for (size_t i = 2; i < 2 + count; ++i) {
		result += QString("At %1: 0x%2\n").arg(i - 2).arg(reinterpret_cast<size_t>(content[i]), 0, 16);
	}
	return result;
}
